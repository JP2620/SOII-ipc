#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#define TAM 256

int main( int argc, char *argv[] ) {
	int sockfd, newsockfd, puerto, clilen, pid;
	char buffer[TAM];
	int suscriptores[TAM];
	struct sockaddr_in serv_addr, cli_addr;
	int n;

	if ( argc < 2 ) {
        	fprintf( stderr, "Uso: %s <puerto>\n", argv[0] );
		exit( 1 );
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
	clilen = sizeof( cli_addr );

	int id = 0;
	while( 1 ) {
		sleep(5);
		newsockfd = accept( sockfd, (struct sockaddr *) &cli_addr, &clilen );
		if (newsockfd == -1)
		{
			fprintf(stderr, "Fallo establecimiento de la conexión con suscriptor");
			continue;
		}

		suscriptores[id] = newsockfd;
		id++;
		for (int i = 0; i < id; i++)
		{
			n = write (suscriptores[i], "Hola pibes", 11);
		}
	}
	return 0;
} 
