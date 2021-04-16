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

void send_ack(int fd_sock, int token);

int main( int argc, char *argv[] ) {

	// setup socket
	int sockfd, term, token;
	unsigned short puerto;
	long int n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	packet_t rcv_packet;
	int tflag = 0;

	int opt = getopt(argc, argv, "t:");
	if (opt == 't')
	{
		tflag = 1;
		token = atoi(optarg);
		printf("token = %d\n", token);
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
				printf("Conexión aceptada");
				if (tflag)
				{
					packet_t packet;
					int tokens[2] = {token, *((int*) rcv_packet.payload)}; // Token viejo, seguido de token nuevo
					gen_packet(&packet, M_TYPE_AUTH, tokens, sizeof(tokens));
					if (write(sockfd, &packet, sizeof(packet_t)) == -1)
						perror("write: ");
					printf(", reconectando\n");
				}
				else
				{
					token = *((int*) rcv_packet.payload);
					printf(", token es: %d\n", token);
				}
				break;

			case M_TYPE_CLI_ACCEPTED:
				printf("Me aceptaron en una sala\n");
				send_ack(sockfd, token);
				break;
			case M_TYPE_DATA:
				printf("Mensaje recibido íntegramente: %s\n", rcv_packet.payload);
				send_ack(sockfd, token);
				break;
			case M_TYPE_FIN:
				printf("Server cerró la conexion, terminando proceso\n");
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


void send_ack(int fd_sock, int token)
{
	packet_t packet_ack;
	gen_packet(&packet_ack, M_TYPE_ACK, &token, sizeof(token));
	if ( write(fd_sock, &packet_ack, sizeof(packet_t)) == -1)
	{
		if (errno == EAGAIN)
			;
		else
			perror("write: ");
	}
}
