#include "../include/srv_util.h"



void broadcast_room(list_t* room, packet_t *msg)
{
	int n;
	for (node_t *iterator = room->head; iterator->next != NULL;
			 iterator = iterator->next)
	{
		n = write (* ( (int*)iterator->data ) , (char*) msg, sizeof(packet_t));
		if (n < 0) 
			perror("write ");
	}	
}

int conn_compare(connection_t* con1, connection_t* con2)
{
	if (con1->sockfd == con2->sockfd)
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
  event.events = EPOLLIN | EPOLLOUT | EPOLLHUP | EPOLLET; // edge trigger

  epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
  set_non_blocking(fd);
}
