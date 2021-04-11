#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h> 
#include "../include/protocol.h"
#define TAM 256

int main( int argc, char *argv[] ) {
	int sockfd, puerto, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	packet rcv_packet;

	char buffer[TAM];
	if ( argc < 3 ) {
		fprintf( stderr, "Uso %s host puerto\n", argv[0]);
		exit( 0 );
	}

	puerto = atoi( argv[2] );
	sockfd = socket( AF_INET, SOCK_STREAM, 0 );

	server = gethostbyname( argv[1] );

	memset( (char *) &serv_addr, '0', sizeof(serv_addr) );
	serv_addr.sin_family = AF_INET;
	bcopy( (char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length );
	serv_addr.sin_port = htons( puerto );
	if ( connect( sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr ) ) < 0 ) {
		perror( "conexion" );
		exit( 1 );
	}

	while(1) {
		n = read( sockfd, &rcv_packet, sizeof(packet) );
		unsigned char* byte_ptr = rcv_packet.hash;
		if (check_packet_MD5(&rcv_packet))
		{
			printf("Hash correcto, mensaje recibido: %s\n", rcv_packet.payload);
		}
		else
		{
			printf("Hashes no iguales\n");
		}
	}
	return 0;
} 
