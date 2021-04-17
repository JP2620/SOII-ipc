#include "../include/srv_util.h"



void broadcast_room(list_t* room, packet_t *msg)
{
	long int n;
	for (node_t *iterator = room->head; iterator->next != NULL;
			 iterator = iterator->next)
	{
		int fd = * ((int*) iterator->data);
		
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

int conn_compare(connection_t* con1, connection_t* con2)
{
	if (con1->token == con2->token)
		return 0;
	else
		return -1;
}

void conn_free(connection_t* connection)
{
	free(connection);
	return;
}

int compare_fd(int* fd1, int* fd2)
{
	if (*fd1 == *fd2)
		return 0;
	else return -1;
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

connection_t* find_by_socket(int fd, list_t* list)
{
	for (node_t *iter = list->head; iter->next != NULL; iter = iter->next)
	{
		connection_t *conn = (connection_t*) iter->data;
		if (conn->sockfd == fd)
			return conn;
	}
	return NULL;
}

void garb_collec_old_packets(list_t* buffered_packets, time_t *last_gc, unsigned int period)
{
	time_t time_actual;
	time(&time_actual);
	packet_t *packet;
	time_t timestamp_packet;

	if (time_actual - *last_gc < period) // Si no paso el periodo, chau no limpio nada
		return;
	

	node_t *iter = buffered_packets->head;
	int index = 0;
	fprintf(fptr_log_productores, "[Delivery manager] Timestamp al limpiar buffer: %ld\n", time_actual);


	while (iter->next != NULL)
	{
		packet = (packet_t*) iter->data;
		timestamp_packet = packet->timestamp;
		// fprintf(fptr_log_productores, "[Delivery manager] Packet con timestamp: %ld\n", timestamp_packet);
		if (time_actual - timestamp_packet > CONN_TIMEOUT) // Si es paquete viejo, deleteo
		{
			iter = iter->next;
			fprintf(fptr_log_productores, "[Delivery manager] Packet con timestamp eliminado: %ld\n", timestamp_packet);
			list_delete(index, buffered_packets);
			continue;
		}
		iter = iter->next;
		index++;
	}
	*last_gc = time_actual; // Actualizo el tiempo de la ultima limpieza
}
