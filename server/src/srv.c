#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "../include/cli.h"
#include "../include/srv_util.h"
#include "../include/protocol.h"
#include "../include/mq_util.h"

#define TAM 256
#define MAX_EVENT_NUMBER 5000 // Poco probable que ocurran 5000 eventos
#define CONN_TIMEOUT 15


int main( int argc, char *argv[] ) {
	int listenfd, sockfd, puerto, clilen, pid, CLI_fd;
	char buffer[TAM];
	int *fd_ptr;
	struct sockaddr_in serv_addr, cli_addr;
	int n;

	if ( argc < 2 ) {
        	fprintf( stderr, "Uso: %s <puerto>\n", argv[0] );
		exit( 1 );
	}

	// Server setup
	listenfd = socket( AF_INET, SOCK_STREAM, 0);

	memset( (char *) &serv_addr, 0, sizeof(serv_addr) );
	puerto = atoi( argv[1] );
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons( puerto );

	if ( bind(listenfd, ( struct sockaddr *) &serv_addr, sizeof( serv_addr ) ) < 0 ) {
		perror( "ligadura" );
		exit( 1 );
	}

	printf( "Proceso: %d - socket disponible: %d\n", getpid(), 
			ntohs(serv_addr.sin_port) );

	listen( listenfd, 5 );

	// Setea epoll
	struct  epoll_event events[MAX_EVENT_NUMBER];
	int epollfd = epoll_create1(0);
	if (epollfd == -1)
	{
		perror("epoll ");
		exit(EXIT_FAILURE);
	}
	add_fd(epollfd, listenfd);

	// Crea lista de conexiones
	list_t *connections = list_create();
	connections->free_data = (void (*)(void*)) conn_free;
	connections->compare_data = (int (*)(void *, void *)) conn_compare;

	// Crea salas de difusión para cada productor
	list_t *susc_room[3];
	for (int j = 0; j < 3; j++)
	{
		susc_room[j] = list_create();
		susc_room[j]->free_data = (void (*)(void *))free;
		susc_room[j]->compare_data = (int (*)(void *, void *)) compare_fd;
	}

	// Crea fifo para comunicarse con CLI
	char * CLI_fifo = "/tmp/cli_dm_fifo";
	mkfifo(CLI_fifo, 0666);
	CLI_fd = open(CLI_fifo, O_RDONLY | O_NONBLOCK);

	// Crea mqueue para comunicarse con productores
	long QUEUE_MSGSIZE = sizeof (msg_producer_t);
	struct mq_attr attr = {
		.mq_maxmsg = QUEUE_MAXMSG,
		.mq_msgsize = QUEUE_MSGSIZE
	};

  mq = mq_open(QUEUE_NAME, O_CREAT | O_RDONLY | O_NONBLOCK,
                  QUEUE_PERMS, &attr);
  if (mq < 0)
  {
    perror("mq_open failed ");
    exit(1);
  }


	// Server loop
	while( 1 ) {
		int ret = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, 25); // polleo cada 25ms
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
				int connfd = accept(listenfd, (struct sockaddr*) &cli_addr, &clilen);
				fprintf(stderr, "Cliente aceptado, socket: %d\n", connfd);
				add_fd(epollfd, connfd);

				connection_t *new_conn = malloc(sizeof (connection_t));
				new_conn->sockfd = connfd;
				new_conn->susc_counter = 0;
				time(&new_conn->timestamp);
				list_add_last(new_conn, connections);
			}
			else if (events[i].events & EPOLLHUP) /* Cerró el socket el cliente */
			{
				/* Cierra conexión ROTO POR AHORA */
				fprintf(stderr, "Se desconecto un cliente\n");
				epoll_ctl(epollfd, EPOLL_CTL_DEL, sockfd, &events[i]);
				close(sockfd);
				
			}
			else if (events[i].events & EPOLLIN) /* Recibo ACK */
			{
				packet_t packet;
				int retval = read(sockfd, &packet, sizeof(packet_t));
				if (retval == -1)
				{
					perror("read: ");
					break;
				}
				if (check_packet_MD5(&packet))
				{
					connection_t aux_con;
					aux_con.sockfd = sockfd;
					int index = list_find(&aux_con, connections);
					connection_t* connection = (connection_t *) list_get(index, connections);
					time(&(connection->timestamp));
				}
			}
		}

		/* Ejecucion comandos de la CLI */
		command_t command;
		if ( read(CLI_fd, &command, sizeof(command_t)) == -1 )
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
			if (command.type == CMD_ADD) /* Agrega socket a una sala */
			{
				fd_ptr = malloc(sizeof(int));
				*fd_ptr = command.socket;
				list_add_last(fd_ptr, susc_room[command.productor]);
				
				// Notifico que fue aceptado
				packet_t packet;
				gen_packet(&packet, M_TYPE_CLI_ACCEPTED, "", 0);
				int retval = send(sockfd, &packet, sizeof(packet_t), MSG_NOSIGNAL);
				if (retval == -1)
					perror("write: ");
				printf("Agregado socket %d a lista del productor %d",
				 command.socket, command.productor);
			}
			else if (command.type == CMD_DEL) /* Elimina socket de una sala */
			{
				int index = list_find(&command.socket, susc_room[command.productor]);
				list_delete(index, susc_room[command.productor]);
				printf("Eliminado socket %d de la lista del productor %d",
				 command.socket, command.productor);
			}
			else if (command.type == CMD_LOG) /* Te lo debo */
			{

			}
		}
		
		/* Limpieza de conexiones inactivas */
		time_t new_timestamp;
		time(&new_timestamp);
		for (node_t *iterator = connections->head; iterator != NULL && iterator->next != NULL;
		 iterator = iterator->next)
		{
			connection_t* connection = (connection_t*) iterator->data;
			if (new_timestamp - connection->timestamp >= CONN_TIMEOUT) /* Chequea timeout */
			{
				send_fin(connection->sockfd);
				for (int i = 0; i < 3; i++)
				{
					int index = list_find(&(connection->sockfd), susc_room[i]);
					list_delete(index, susc_room[i]);
				}
				if (close(connection->sockfd) == -1)
				{
					if (errno == EBADF)
						;
					else
						perror("close ");
				}
				int index = list_find(connection, connections);
				iterator = iterator->next;
				if (list_delete(index, connections) == -1)
					fprintf(stderr, "Error al eliminar conexion, index = %d, connection.fd = %d\n", index, connection->sockfd);
				printf("Conexion cerrada\n");
			}
		}

		/* Envío de paquetes */
		packet_t packet;
		msg_producer_t msg_producer;

		if (mq_receive(mq, (char*) &msg_producer, sizeof(msg_producer), NULL) > 0)
		{
			memset(buffer, '\0', sizeof(buffer));
			printf("Mensaje recibido: id = %d, timestamp = %lu\n",
			 msg_producer.id, msg_producer.timestamp);
			switch (msg_producer.id)
			{
				case 0:
					sprintf(buffer, "Memoria disponible del sistema: %u kB", msg_producer.data.free_mem);
					break;
				case 1:
					sprintf(buffer, "%s", msg_producer.data.random_msg);
					break;
				case 2:
					sprintf(buffer, "Carga del sistema normalizada: %f", msg_producer.data.sysload);
					break;
			}
			gen_packet(&packet, M_TYPE_DATA, buffer, strlen(buffer));
			broadcast_room(susc_room[msg_producer.id], &packet);
		}
		else
		{
			if (errno == EAGAIN)
				;
			else
				perror("mq_receive ");
		}


		// memset (buffer, '\0', sizeof(buffer));

		// for (int j = 0; j < 3; j++)
		// {
		// 	memset(buffer, '\0', sizeof(buffer));
		// 	sprintf(buffer, "Hola sala %d", j);
		// 	gen_packet(&packet, M_TYPE_DATA, buffer, strlen(buffer));
		// 	broadcast_room(susc_room[j], &packet);
		// }
	}
	return 0;
} 
