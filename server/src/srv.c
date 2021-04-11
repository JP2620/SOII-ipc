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


int main( int argc, char *argv[] ) {
	int listenfd, sockfd, puerto, clilen, pid;
	char buffer[TAM];
	int *fd_ptr;
	struct sockaddr_in serv_addr, cli_addr;
	int n;

	if ( argc < 2 ) {
        	fprintf( stderr, "Uso: %s <puerto>\n", argv[0] );
		exit( 1 );
	}

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

	struct  epoll_event events[MAX_EVENT_NUMBER];
	int epollfd = epoll_create1(0);
	if (epollfd == -1)
	{
		perror("epoll ");
		exit(EXIT_FAILURE);
	}
	add_fd(epollfd, listenfd);

	list_t *connections = list_create();
	connections->free_data = (void (*)(void*)) conn_free;
	connections->compare_data = (int (*)(void *, void *)) conn_compare;

	list_t *susc_room[3];
	for (int j = 0; j < 3; j++)
	{
		susc_room[j] = list_create();
		susc_room[j]->free_data = (void (*)(void *))free;
		susc_room[j]->compare_data = compare_fd;
	}

	while( 1 ) {
		int ret = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, 25); // -1 -> espera bloq
		if (ret < 0)
		{
			perror("epoll ");
			continue;
		}

		for (int i = 0; i < ret; i++)
		{
			sockfd = events[i].data.fd;
			if (sockfd == listenfd) /* pollea ese socket */
			{
				fprintf(stderr, "Cliente aceptado\n");
				int connfd = accept(listenfd, (struct sockaddr*) &cli_addr, &clilen);
				add_fd(epollfd, connfd);

				connection_t *new_conn = malloc(sizeof (connection_t));
				new_conn->sockfd = connfd;
				new_conn->susc_counter = 0;
				time(&new_conn->timestamp);

				int r = rand() % 3;
				fd_ptr = malloc(sizeof (int));
				*fd_ptr = connfd;
				list_add_last(fd_ptr, susc_room[r]);
			}

			else if (events[i].events & EPOLLHUP)
			{
				/* Cierra conexión ROTO POR AHORA */
				epoll_ctl(epollfd, EPOLL_CTL_DEL, sockfd, &events[i]);
				connection_t aux_con;
				aux_con.sockfd = sockfd;
				for (int i = 0; i < 3; i++)
				{
					list_delete(list_find(&sockfd, susc_room[i]), susc_room[i]);	
				}
				list_delete(list_find(&aux_con, connections), connections);
				fprintf(stderr, "Se desconecto un cliente\n");
				
			}

			else if (events[i].events & EPOLLIN)
			{
				/* recibe ack */
				fprintf(stderr, "Recibi el ack\n");
				connection_t aux_con;
				aux_con.sockfd = sockfd;
				connection_t* connection = list_get(list_find(&aux_con, connections), connections);
				time(&(connection->timestamp));
				fprintf(stderr, "%lu\n", connection->timestamp);
			}
		}

		sleep(2);
		packet_t packet;
		for (int j = 0; j < 3; j++)
		{
			memset(buffer, '\0', sizeof(buffer));
			sprintf(buffer, "Hola sala %d", j);
			gen_packet(&packet, M_TYPE_DATA, buffer, strlen(buffer));
			broadcast_room(susc_room[j], &packet);
		}
	}
	return 0;
} 
