#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include "../include/srv_util.h"
#include "../include/protocol.h"

#define TAM 256
#define MAX_EVENT_NUMBER 5000 // Poco probable que ocurran 5000 eventos


int SetNonblocking(int fd)
{
  int old_option = fcntl(fd, F_GETFL);
  int new_option = old_option | O_NONBLOCK;
  fcntl(fd, F_SETFL, new_option);
  return old_option;
}

void AddFd(int epollfd, int fd)
{
  struct epoll_event event;
  event.data.fd = fd;
  event.events = EPOLLIN | EPOLLET;

  epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
  SetNonblocking(fd);
}



int main( int argc, char *argv[] ) {
	int sockfd, newsockfd, puerto, clilen, pid;
	char buffer[TAM];
	int *fd_ptr;
	int id;
	struct sockaddr_in serv_addr, cli_addr;
	int n;

	if ( argc < 2 ) {
        	fprintf( stderr, "Uso: %s <puerto>\n", argv[0] );
		exit( 1 );
	}

	list_t *susc_room[3];
	for (int j = 0; j < 3; j++)
	{
		susc_room[j] = list_create();
	}

	sockfd = socket( AF_INET, SOCK_STREAM, 0);

	memset( (char *) &serv_addr, 0, sizeof(serv_addr) );
	puerto = atoi( argv[1] );
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons( puerto );

	if ( bind(sockfd, ( struct sockaddr *) &serv_addr, sizeof( serv_addr ) ) < 0 ) {
		perror( "ligadura" );
		exit( 1 );
	}

	printf( "Proceso: %d - socket disponible: %d\n", getpid(), 
			ntohs(serv_addr.sin_port) );

	listen( sockfd, 5 );

	struct  epoll_event events[MAX_EVENT_NUMBER];
	int epollfd = epoll_create1(0);
	if (epollfd == -1)
	{
		perror("epoll ");
		exit(EXIT_FAILURE);
	}


	

	id = 0;
	while( 1 ) {

		// Simula evento
		sleep(5);

		// Acepta socket
		newsockfd = accept( sockfd, (struct sockaddr *) &cli_addr, &clilen );
		if (newsockfd == -1)
		{
			fprintf(stderr, "Fallo establecimiento de la conexión con suscriptor");
			continue;
		}

		// Asigna socket a lista de suscriptores
		fd_ptr = malloc(sizeof(int));
		*fd_ptr = newsockfd;
		list_add_last(fd_ptr, susc_room[id % 3]);
		id++;
		packet paquete;
		// Envía mensajes a las salas
		for (int j = 0; j < 3; j++)
		{
			memset(buffer, '\0', sizeof(buffer));
			sprintf(buffer, "Hola sala %d", j);
			gen_packet(&paquete, M_TYPE_DATA, buffer, strlen(buffer));
			broadcast_room(susc_room[j], &paquete);
		}
	}
	return 0;
} 
