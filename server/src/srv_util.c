#include "../include/srv_util.h"

void broadcast_room(list_t *room, packet_t *msg)
{
	long int n;
	for (node_t *iterator = room->head; iterator->next != NULL;
			 iterator = iterator->next)
	{
		int fd = *((int *)iterator->data);

		n = send(fd, msg, sizeof(packet_t), MSG_NOSIGNAL);
		if (n == -1)
		{
			if (errno == EPIPE)
				;
			else
				perror("write broadcast");
		}
	}
}

int conn_compare(connection_t *con1, connection_t *con2)
{
	if (con1->token == con2->token)
		return 0;
	else
		return -1;
}

int compare_fd(int *fd1, int *fd2)
{
	if (*fd1 == *fd2)
		return 0;
	else
		return -1;
}

int set_non_blocking(int fd)
{
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return old_option;
}

void add_fd(int epollfd, int fd)
{
	struct epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLHUP;

	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	set_non_blocking(fd);
}

void send_fin(int sockfd)
{
	packet_t packet;
	gen_packet(&packet, M_TYPE_FIN, "", 0);
	write(sockfd, &packet, sizeof(packet_t));
}

connection_t *find_by_socket(int fd, list_t *list)
{
	for (node_t *iter = list->head; iter->next != NULL; iter = iter->next)
	{
		connection_t *conn = (connection_t *)iter->data;
		if (conn->sockfd == fd)
			return conn;
	}
	return NULL;
}

void garb_collec_old_packets(list_t *buffered_packets, time_t *last_gc, unsigned int period)
{
	time_t time_actual;
	time(&time_actual);
	packet_t *packet;
	time_t timestamp_packet;

	if (time_actual - *last_gc < period) // Si no paso el periodo, chau no limpio nada
		return;

	node_t *iter = buffered_packets->head;
	int index = 0;
	log_event(fptr_log_productores, "[Delivery manager] Timestamp al limpiar buffer: %ld\n", time_actual);

	while (iter->next != NULL)
	{
		packet = (packet_t *)iter->data;
		timestamp_packet = packet->timestamp;
		// fprintf(fptr_log_productores, "[Delivery manager] Packet con timestamp: %ld\n", timestamp_packet);
		if (time_actual - timestamp_packet > CONN_TIMEOUT) // Si es paquete viejo, deleteo
		{
			iter = iter->next;
			log_event(fptr_log_productores, "[Delivery manager] Packet con timestamp eliminado: %ld\n", timestamp_packet);
			list_delete(index, buffered_packets);
			continue;
		}
		iter = iter->next;
		index++;
	}
	*last_gc = time_actual; // Actualizo el tiempo de la ultima limpieza
}

void garb_collec_old_conn(list_t *connections, list_t *broadcast_rooms[NO_PRODUCTORES],
													time_t *last_gc, unsigned int period, int epollfd)
{
	time_t time_actual;
	time(&time_actual);
	node_t *iter = connections->head;
	int index = 0;

	if (time_actual - *last_gc < period)
		return;

	while (iter->next != NULL)
	{
		connection_t *conn = (connection_t *)iter->data;
		if (time_actual - conn->timestamp > CONN_TIMEOUT && conn->susc_counter > 0)
		{
			fprintf(fptr_log_clientes, "[Delivery manager] Conexion con cliente con "
																 "token: %d y socket: %d cerrada\n",
							conn->token, conn->sockfd);
			send_fin(conn->sockfd);
			if (epoll_ctl(epollfd, EPOLL_CTL_DEL, conn->sockfd, NULL) == -1)
			{
				if (EBADF) // Ignoro, ya lo eliminaron en la reconexión
					;
				else
					perror("epoll_ctl limpieza conexiones inactivas ");
			}
			for (int i = 0; i < NO_PRODUCTORES; i++)
			{
				int target_ind = list_find(&(conn->sockfd), broadcast_rooms[i]);
				list_delete(target_ind, broadcast_rooms[i]);
			}
			if (close(conn->sockfd) == -1)
			{
				if (errno == EBADF)
					fprintf(stderr, "Close fd de conexion inactiva");
				else
					perror("close ");
			}
			iter = iter->next;
			if (list_delete(index, connections) == -1)
			{
				fprintf(stderr, "Error al eliminar conexion, index = %d, "
												"connection.fd = %d\n",
								index, conn->sockfd);
			}
			continue;
		}
		iter = iter->next;
		index++;
	}
}

int setup_tcpsocket(uint16_t port, struct sockaddr_in *address)
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		perror("socket: ");
		return -1;
	}
	memset(address, 0, sizeof(struct sockaddr_in));
	address->sin_family = AF_INET;
	address->sin_addr.s_addr = INADDR_ANY;
	address->sin_port = htons(port);

	if (bind(sockfd, (struct sockaddr *)address, sizeof(struct sockaddr_in)) < 0)
	{
		perror("ligadura");
		return -1;
	}
	return sockfd;
}

void *handle_loq_req(void *args)
{
	int sock_req = *((int *)args);
	struct sockaddr_in new_address;
	ft_packet_t ft_packet;
	unsigned int new_addr_len;
	packet_t packet;
	uint16_t new_port = FT_PORT;
	/* Nuevo socket para la cuestion */
	int socket_passive_ftransfer = setup_tcpsocket(new_port, &new_address);
	if (socket_passive_ftransfer == -1)
	{
		perror("socket_pasivo_ft");
		goto terminate;
	}
	/* FT_SETUP, le aviso el puerto al que tiene que conectarse  */
	gen_packet(&packet, M_TYPE_FT_SETUP, &new_address.sin_port, sizeof(new_address.sin_port));
	/* Solo una conección */
	if (listen(socket_passive_ftransfer, 1) == -1)
	{
		perror("listen ft");
		goto terminate;
	}
	if (write(sock_req, &packet, sizeof(packet)) == -1)
	{
		perror("Envío de M_TYPE_FT_SETUP\n");
		goto terminate;
	}
	log_event(fptr_log_clientes, "Enviado M_TYPE_FT_SETUP, escuchando con socket %d "
														 "en puerto %d\n",
					socket_passive_ftransfer, ntohs(new_address.sin_port));
	/* Espero a que se conecte, y guardo el fd */
	int fd_file_transfer = accept(socket_passive_ftransfer,
																(struct sockaddr *)&new_address, &new_addr_len);
	if (fd_file_transfer == -1)
	{
		perror("fd_file_transfer");
		goto terminate;
	}
	log_event(fptr_log_clientes, "Cliente aceptado para enviar archivo en fd: %d\n", fd_file_transfer);
	struct zip_t *zip = zip_open("log_comprimido.zip", ZIP_DEFAULT_COMPRESSION_LEVEL, 'w'); // Comprimo log a enviar
	if (zip == NULL)
	{
		fprintf(stderr, "Error al crear log comprimido\n");
		goto terminate;
	}
	{
		int retval = zip_entry_open(zip, "log_DM_Clientes");
		if (retval < 0)
		{
			fprintf(stderr, "zip_entry_open: %s", zip_strerror(retval));
			goto terminate;
		}
		{
			retval = zip_entry_fwrite(zip, LOG_CLIENTES);
			if (retval < 0)
			{
				fprintf(stderr, "zip_entry_write: %s", zip_strerror(retval));
				goto terminate;
			}
		}
		retval = zip_entry_close(zip);
		if (retval == -1)
		{
			fprintf(stderr, "zip_entry_close: %s", zip_strerror(retval));
			goto terminate;
		}
	}
	zip_close(zip);

	/* Envío el tamaño y MD5 del archivo al cliente */
	u_char hash[MD5_DIGEST_LENGTH];
	int fd_log_MD5 = open("log_comprimido.zip", O_RDONLY);
	struct stat st;
	if (stat(LOG_CLIENTES, &st) == -1)
	{
		perror("stat");
		goto terminate;
	}
	bzero(&ft_packet, sizeof(ft_packet));
	ft_packet.mtype = M_TYPE_FT_BEGIN;
	ft_packet.fsize = st.st_size;
	if (get_file_MD5(fd_log_MD5, hash) == -1)
	{
		fprintf(stderr, "get_file_MD5 falló\n");
		goto terminate;
	}
	memcpy(ft_packet.payload, hash, MD5_DIGEST_LENGTH);
	log_event(fptr_log_clientes, "Tamaño del archivo a mandar: %ld\n", ft_packet.fsize);
	if (write(fd_file_transfer, &ft_packet, sizeof(ft_packet)) == -1)
	{
		perror("write file_transfer");
		goto terminate;
	}
	int fd_log = open("log_comprimido.zip", O_RDONLY);
	if (fd_log == -1)
	{
		perror("fd_log");
		goto terminate;
	}

	/* Envío los bytes del archivo */
	bzero(&ft_packet, sizeof(ft_packet));
	ssize_t nread;
	while ((nread = read(fd_log, ft_packet.payload, sizeof(ft_packet.payload))) > 0)
	{
		ft_packet.mtype = M_TYPE_FT_DATA;
		ft_packet.nbytes = (size_t)nread;
		if (write(fd_file_transfer, &ft_packet, sizeof(ft_packet)) == -1)
		{
			perror ("write fd_file_transfer");
			goto terminate;
		}
		bzero(&ft_packet, sizeof(ft_packet));
	}

	/* Aviso que terminamos */
terminate: ;
	ft_packet.mtype = M_TYPE_FT_FIN;
	write(fd_file_transfer, &ft_packet, sizeof(ft_packet));
	close(fd_file_transfer);
	close(socket_passive_ftransfer);
	free(args);
	pthread_exit(NULL);
	return NULL;
}

void *srv_on_exit(void* args)
{
	struct srv_exit_args stuff_to_release = *((struct srv_exit_args*) args);
	list_free(stuff_to_release.buffer_packets);
	list_free(stuff_to_release.connections);
	mq_close(stuff_to_release.mq);
	for (int i = 0; i < NO_PRODUCTORES; i++)
	{
		list_free(stuff_to_release.susc_rooms[i]);
	}
	return NULL;
}
int get_datetime(char* buf, size_t max_len)
{
	time_t now = time(NULL);
	if (now == ((time_t) -1))
		return -1;
	strftime(buf, max_len, "%D %H:%M:%S ", localtime(&now));
	return 0;
}


int log_event(FILE* log, char* event_str, ...)
{
	va_list format_specs;
	va_start(format_specs, event_str);
	char datetime[200];
	if (get_datetime(datetime, 50) == -1)
		return -1;
	char string_to_print[128];
	vsnprintf(string_to_print, 128, event_str, format_specs);
	strncat(datetime, string_to_print, 200);
	fprintf(log, "%s", datetime);
	return 0;
	
}










