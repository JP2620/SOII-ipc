#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h> 
#include <errno.h>
#include <fcntl.h>
#include "../../common/include/protocol.h"

void send_ack(int fd_sock);

int main( int argc, char *argv[] ) {

	// setup socket
	int sockfd, term;
	unsigned short puerto;
	long int n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	packet_t rcv_packet;

	if ( argc < 3 ) {
		fprintf( stderr, "Uso %s host puerto\n", argv[0]);
		exit( 0 );
	}

	puerto = (unsigned short) atoi( argv[2] );
	sockfd = socket( AF_INET, SOCK_STREAM, 0 );

	server = gethostbyname( argv[1] );

	memset( (char *) &serv_addr, '0', sizeof(serv_addr) );
	serv_addr.sin_family = AF_INET;
	bcopy( (char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
			 (long unsigned int) server->h_length );
	serv_addr.sin_port = htons( puerto );
	if ( connect( sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr ) ) < 0 ) {
		perror( "conexion" );
		exit( 1 );
	}

	term = 0;

	// Una vez el socket esta listo
	// fcntl(sockfd, F_SETFL, O_NONBLOCK);
	while(!term) {
		n = recv( sockfd, &rcv_packet, sizeof(rcv_packet), 0 );
		if (n == -1)
		{
			if (errno == EAGAIN)
				;
			else
				perror("recv: ");
		}

		if (check_packet_MD5(&rcv_packet))
		{
			switch (rcv_packet.mtype)
			{
			case M_TYPE_CONN_ACCEPTED:
				printf("Conexión aceptada, token es: %d\n", *((int*) rcv_packet.payload));
				break;

			case M_TYPE_CLI_ACCEPTED:
				printf("Me aceptaron en una sala\n");
				send_ack(sockfd);
				break;
			case M_TYPE_DATA:
				printf("Mensaje recibido íntegramente: %s\n", rcv_packet.payload);
				send_ack(sockfd);
				break;
			case M_TYPE_FIN:
				close(sockfd);
				term = 1;
				break;
			}
		}
		else
			continue;
	}
	return 0;
} 


void send_ack(int fd_sock)
{
	packet_t packet_ack;
	gen_packet(&packet_ack, M_TYPE_ACK, "", 0);
	if ( write(fd_sock, &packet_ack, sizeof(packet_t)) == -1)
	{
		if (errno == EAGAIN)
			;
		else
			perror("write: ");
	}
}
