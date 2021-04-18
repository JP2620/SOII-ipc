#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <pthread.h>

#include "../include/cli.h"
#include "../include/srv_util.h"
#include "../../common/include/protocol.h"
#include "../../common/include/mq_util.h"




int main(int argc, char *argv[])
{
	int listenfd, sockfd, CLI_fd, retval;
	unsigned short puerto; unsigned int clilen;
	int *fd_ptr; 
	struct sockaddr_in serv_addr, cli_addr;
	struct rlimit lim;
	signal(SIGPIPE, SIG_IGN);

	lim.rlim_cur = 5100; // Aumentar limite para handlear 5000 conexiones
	lim.rlim_max = 6000; // Y unos cuantos mas para fifo/mqueue
	if (setrlimit(RLIMIT_NOFILE, &lim) == -1)
	{
		perror("setrlimit: ");
		exit(EXIT_FAILURE);		
	}
	fptr_log_clientes = fopen(LOG_CLIENTES, "a");
	fptr_log_productores = fopen(LOG_PRODUCTORES, "a");
	if (fptr_log_clientes == NULL || fptr_log_productores == NULL)
	{
		perror("fopen log");
		exit(EXIT_FAILURE);
	}
	setvbuf(fptr_log_clientes, NULL, _IOLBF, 512);

	if (argc < 2)
	{
		fprintf(stderr, "Uso: %s <puerto>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	// Server setup
	fprintf(stderr, "sobrevivi\n");
	puerto = (uint16_t) atoi(argv[1]);
	listenfd = setup_tcpsocket(puerto, &serv_addr);
	fprintf(stderr, "sobrevivi\n");
	if (listenfd == -1)
	{
		fprintf(stderr, "fallo setup del socket\n");
		exit(EXIT_FAILURE);
	}

	fprintf(fptr_log_clientes,"[Delivery manager] Proceso: %d - socket disponible: %d\n", getpid(),
				 ntohs(serv_addr.sin_port));

	CHECK(listen(listenfd, 10000));

	// Setea epoll
	struct epoll_event events[MAX_EVENT_NUMBER];
	int epollfd = epoll_create1(0);
	if (epollfd == -1)
	{
		perror("epoll ");
		exit(EXIT_FAILURE);
	}
	add_fd(epollfd, listenfd);

	// Crea lista de conexiones
	list_t *connections = list_create();
	connections->free_data = (void (*)(void *))conn_free;
	connections->compare_data = (int (*)(void *, void *))conn_compare;
	time_t last_time_cleanup;
	time(&last_time_cleanup);

	// Crea lista de paquetes para la reconexion
	list_t *buffer_packets = list_create();
	buffer_packets->free_data = (void (*)(void *))free;
	time_t last_gc_time;
	time(&last_gc_time);

	// Crea salas de difusión para cada productor
	list_t *susc_room[3];
	for (int j = 0; j < 3; j++)
	{
		susc_room[j] = list_create();
		susc_room[j]->free_data = (void (*)(void *))free;
		susc_room[j]->compare_data = (int (*)(void *, void *))compare_fd;
	}

	// Crea fifo para comunicarse con CLI
	char *CLI_fifo = "/tmp/cli_dm_fifo";
	mkfifo(CLI_fifo, 0666);
	CLI_fd = open(CLI_fifo, O_RDONLY | O_NONBLOCK);
	struct epoll_event events_CLI[5000];
	int epoll_CLI = epoll_create1(0);
	if (epoll_CLI == -1)
	{
		perror("epoll ");
		exit(EXIT_FAILURE);
	}
	add_fd(epoll_CLI, CLI_fd);

	// Crea mqueue para comunicarse con productores
	long QUEUE_MSGSIZE = sizeof(msg_producer_t);
	struct mq_attr attr = {
			.mq_maxmsg = QUEUE_MAXMSG,
			.mq_msgsize = QUEUE_MSGSIZE};

	mq = mq_open(QUEUE_NAME, O_CREAT | O_RDONLY | O_NONBLOCK,
							 QUEUE_PERMS, &attr);
	if (mq < 0)
	{
		perror("mq_open failed ");
		exit(1);
	}

	// Inicializa fuente de tokens
	unsigned int seed;
	int rand_fd = open("/dev/random", O_RDONLY);
	read(rand_fd, &seed, sizeof(int));
	close(rand_fd);
	srand(seed);

	// Server loop
	while (1)
	{
		int ret = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, 10); // polleo cada 25ms
		if (ret < 0)
		{
			perror("epoll ");
			continue;
		}

		for (int i = 0; i < ret; i++) /* Atiende eventos de clientes */
		{
			sockfd = events[i].data.fd;
			if (sockfd == listenfd) /* nueva conexion */
			{
				int connfd = accept(listenfd, (struct sockaddr *)&cli_addr, &clilen);
				if (connfd == -1)
				{
					perror("[Delivery manager] accept: ");
				}
				add_fd(epollfd, connfd);

				packet_t packet;
				bzero(&packet, sizeof(packet_t));
				int token = rand();
				gen_packet(&packet, M_TYPE_CONN_ACCEPTED, &token, sizeof(token));
				if (write(connfd, &packet, sizeof(packet)) == -1)
				{
					perror("write CLI_CONNECTED: ");
				}
				fprintf(fptr_log_clientes,"[Delivery manager] Cliente aceptado, socket: %d, token = %d\n", connfd, token);
				connection_t *new_conn = malloc(sizeof(connection_t));
				new_conn->sockfd = connfd;
				new_conn->susc_counter = 0;
				new_conn->token = token;
				time(&new_conn->timestamp);
				list_add_last(new_conn, connections);
			}
			else if (events[i].events & EPOLLHUP) /* Cerró el socket el cliente */
			{
				connection_t* conn = find_by_socket(sockfd, connections);
				epoll_ctl(epollfd, EPOLL_CTL_DEL, sockfd, NULL);
				fprintf(fptr_log_clientes,"[Delivery manager] Se desconecto un cliente, socket: %d y token: %d\n", conn->sockfd, conn->token);
			}
			else if (events[i].events & EPOLLIN) /* Recibo ACK */
			{
				packet_t packet;
				int retval = (int)read(sockfd, &packet, sizeof(packet_t));
				if (retval == -1)
				{
					perror("read: ");
					break;
				}
				if (check_packet_MD5(&packet))
				{
					connection_t aux_con;
					if (packet.mtype == M_TYPE_ACK)
					{
						aux_con.token = *((int*) packet.payload);
						int index = list_find(&aux_con, connections);
						connection_t *connection = (connection_t *)list_get(index, connections);
						time(&(connection->timestamp));
					}
					else if (packet.mtype == M_TYPE_AUTH) // Autenticación
					{
						int tokens[2] = {* ((int *) packet.payload), * ( ((int*) packet.payload) + 1)}; // Token viejo, seguido de token nuevo

						// Borra conexión nueva
						aux_con.token = tokens[1];
						int index = list_find(&aux_con, connections);
						if (index == -1)
						{
							fprintf(fptr_log_clientes, "list_find conexion con token nuevo\n");
							fprintf(fptr_log_clientes,"[Delivery manager] Fallo en la autenticación\n");
							send_fin(sockfd);
							continue;
						}
						list_delete(index, connections); // Borra entrada nueva, la que no se va a usar

						// updatea fd en listas de difusión
						aux_con.token = tokens[0];
						index = list_find(&aux_con, connections);
						if (index == -1)
						{
							fprintf(fptr_log_clientes,"[Delivery manager] Fallo en la autenticación\n");
							fprintf(fptr_log_clientes, "list_find conexion con token viejo\n");
							send_fin(sockfd);
							continue;
						}
						connection_t *orig_conn = (connection_t *) list_get(index, connections);
						for (int i = 0; i < 3; i++) 
						{
							int index_susc = list_find(&(orig_conn->sockfd), susc_room[i]);
							if (index_susc == -1)
								continue;
							int *fd_entry = (int*) list_get(index_susc, susc_room[i]);
							*fd_entry = sockfd; // Actualizo su fd
						}

						// Actualiza la entrada original con el nuevo fd
						if (close(orig_conn->sockfd) == -1)
						{
							perror("close conexion vieja: ");
						}
						orig_conn->sockfd = sockfd; 

						/* Reenvío de paquetes */
						node_t *iter = buffer_packets->head;
						while (iter->next != NULL)
						{
							time_t actual_time;
							time(&actual_time);
							packet_t* packet_to_resend = iter->data;
							if (actual_time - packet_to_resend->timestamp > 5)
								break;
							if (send(sockfd, packet_to_resend, sizeof(packet_t), MSG_NOSIGNAL) == -1)
							{
								perror("send reenvío ");
							}
							iter = iter->next;
						}

						fprintf(fptr_log_clientes,"[Delivery manager] Cliente reconectado con socket: %d y token: %d\n", sockfd, orig_conn->token);
					}

				}
			}
		}

		ret = epoll_wait(epoll_CLI, events_CLI, 5000, 10);
		if (ret < 0)
		{
			perror("epoll CLI");
			continue;
		}

		for (int i = 0; i < ret; i++)
		{
			int fifofd = events_CLI[i].data.fd;
			if (events_CLI[i].events & EPOLLIN)
			{
				command_t command;
				retval = (int)read(fifofd, &command, sizeof(command_t));
				if (retval == -1)
				{
					if (errno == EAGAIN)
						;
					else
					{
						perror("read CLI: ");
					}
				}
				else
				{
					connection_t *conn = find_by_socket(command.socket, connections);
					if (command.type == CMD_ADD) /* Agrega socket a una sala */
					{
						if (command.productor > 2 || command.productor < 0 || list_find(&command.socket, susc_room[command.productor]) != -1 || conn == NULL)
						{
							printf("Comando inválido\n");
							continue;
						}

						fd_ptr = malloc(sizeof(int));
						*fd_ptr = command.socket;
						list_add_last(fd_ptr, susc_room[command.productor]);
						conn->susc_counter++; // Una sala mas a la que esta suscripto

						// Notifico que fue aceptado
						packet_t packet;
						gen_packet(&packet, M_TYPE_CLI_ACCEPTED, "", 0);
						int retval = (int)send(command.socket, &packet, sizeof(packet_t), MSG_NOSIGNAL);
						if (retval == -1)
							perror("write CMD_ADD: ");

						fprintf(fptr_log_clientes,"[Delivery manager] Agregado cliente con socket: %d y token: %d a lista del productor %d\n",
									 command.socket, conn->token, command.productor);
					}
					else if (command.type == CMD_DEL) /* Elimina socket de una sala */
					{
						int index = list_find(&command.socket, susc_room[command.productor]);
						list_delete(index, susc_room[command.productor]);
						conn->susc_counter--;
						fprintf(fptr_log_clientes,"[Delivery manager] Eliminado cliente con socket: %d y token: %d de la lista del productor %d\n",
									 command.socket, conn->token, command.productor);
					}
					else if (command.type == CMD_LOG) /* Le envía el log comprimido al cliente */
					{
						struct sockaddr_in new_address; unsigned int new_addr_len;
						packet_t packet;
						uint16_t new_port = 12345;
						/* Nuevo socket para la cuestion */
						int socket_passive_ftransfer = setup_tcpsocket(new_port, &new_address);
						/* FT_SETUP, le aviso el puerto al que tiene que conectarse  */
						gen_packet(&packet, M_TYPE_FT_SETUP, &new_address.sin_port, sizeof(new_address.sin_port)); 
						/* Solo una conección */ 
						listen(socket_passive_ftransfer, 1); 
						if (write(command.socket, &packet, sizeof(packet)) == -1)
						{
							perror("Envío de M_TYPE_FT_SETUP\n");
							continue;
						} 
						fprintf(fptr_log_clientes, "Enviado M_TYPE_FT_SETUP, escuchando con socket %d en puerto %d\n", socket_passive_ftransfer, ntohs(new_address.sin_port));
						/* Espero a que se conecte, y guardo el fd */
						int fd_file_transfer = accept(socket_passive_ftransfer, 
								(struct sockaddr*) &new_address, &new_addr_len); 
						if (fd_file_transfer == -1)
						{
							perror("fd_file_transfer");
							continue;
						}
						close(socket_passive_ftransfer); // Ya no lo uso más.
						fprintf(fptr_log_clientes, "Cliente aceptado en fd: %d\n", fd_file_transfer);
						// struct zip_t *zip = zip_open("log_comprimido.zip", ZIP_DEFAULT_COMPRESSION_LEVEL, 'w'); // Comprimo log a enviar
						// {
						// 	zip_entry_open(zip, "log.txt");
						// 	{
						// 		zip_entry_fwrite(zip, LOG_CLIENTES);
						// 	}
						// 	zip_entry_close(zip);
						// }
						// zip_close(zip);
						// struct stat st;
						// CHECK(stat("log_comprimido.zip", &st));
						// gen_packet(&packet, M_TYPE_FT_BEGIN, st.st_size, sizeof(st.st_size)); // Paquete avisando el tamño del archivo
						// CHECK( write(fd_file_transfer, &packet, sizeof(packet)) ); // Envío el paquete

						// int fd_log = open("log_comprimido.zip", O_RDONLY);
						// char file_buf[100];
						// ssize_t nread;
						// packet = NULL;
						// while (nread = read(fd_log, file_buf, sizeof(file_buf)) > 0)
						// {
						// 	gen_packet()
						// }



					}
					conn->timestamp = time(NULL);

				}
			}
		}

		/* Limpieza de conexiones inactivas */
		garb_collec_old_conn(connections, susc_room, 
												 &last_time_cleanup, CONN_TIMEOUT, epollfd);

		/* Envío de paquetes */
		packet_t *packet = malloc(sizeof(packet_t));
		msg_producer_t msg_producer;
		char buffer_productores[TAM];

		if (mq_receive(mq, (char *)&msg_producer, sizeof(msg_producer), NULL) > 0)
		{
			memset(buffer_productores, '\0', sizeof(buffer_productores));
			fprintf(fptr_log_productores,"[Delivery manager] Mensaje de productor: %d con timestamp = %lu recibido\n",
						 msg_producer.id, msg_producer.timestamp);
			switch (msg_producer.id)
			{
			case 0:
				sprintf(buffer_productores, "Memoria disponible del sistema: %u kB", msg_producer.data.free_mem);
				break;
			case 1:
				sprintf(buffer_productores, "%s", msg_producer.data.random_msg);
				break;
			case 2:
				sprintf(buffer_productores, "Carga del sistema normalizada: %f", msg_producer.data.sysload);
				break;
			}
			gen_packet(packet, M_TYPE_DATA, buffer_productores, strlen(buffer_productores));
			packet->timestamp = msg_producer.timestamp; // Usamos timestamp del productor
			broadcast_room(susc_room[msg_producer.id], packet);
			list_add_start(packet, buffer_packets); // Quedan ordenados de mas reciente a mas viejo
		}
		else
		{
			if (errno == EAGAIN)
				;
			else
				perror("mq_receive ");
		}

		/* Limpieza buffer de paquetes */
		garb_collec_old_packets(buffer_packets, &last_gc_time, BUFFER_CLEANUP_PERIOD);
	}
	return 0;
}

