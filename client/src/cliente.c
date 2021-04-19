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
#include <pthread.h>
#include "../../common/include/protocol.h"

void send_ack(int fd_sock, int token);
void *recv_file(void *args);

int main(int argc, char *argv[])
{

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

#ifndef TEST
		printf("token = %d\n", token);
#endif
	}

	puerto = (unsigned short)atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	server = gethostbyname(argv[1]);

	memset((char *)&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
				(long unsigned int)server->h_length);
	serv_addr.sin_port = htons(puerto);
	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("conexion");
		exit(1);
	}

	term = 0;

	// Una vez el socket esta listo
	// fcntl(sockfd, F_SETFL, O_NONBLOCK);
	while (!term)
	{
		n = recv(sockfd, &rcv_packet, sizeof(rcv_packet), 0);
		if (n == -1)
		{
			if (errno == EAGAIN)
				;
			else
				perror("recv: ");
		}

		if (!check_packet_MD5(&rcv_packet))
			continue;
		if (rcv_packet.mtype == M_TYPE_CONN_ACCEPTED)
		{
#ifndef TEST
			printf("Conexión aceptada, pid = %d ", getpid());
#endif
			if (tflag)
			{
				packet_t packet;
				int tokens[2] = {token, *((int *)rcv_packet.payload)}; // Token viejo, seguido de token nuevo
				gen_packet(&packet, M_TYPE_AUTH, tokens, sizeof(tokens));
				if (write(sockfd, &packet, sizeof(packet_t)) == -1)
					perror("write CONN_ACCEPTED: ");
#ifndef TEST
				printf("reconectando ||| timestamp: %ld\n", time(NULL));
#endif
				send_ack(sockfd, tokens[0]);
			}
			else
			{
				token = *((int *)rcv_packet.payload);
#ifndef TEST
				printf("y token = %d ||| timestamp: %ld\n", token, rcv_packet.timestamp);
#endif
				send_ack(sockfd, token);
			}
		}
		else if (rcv_packet.mtype == M_TYPE_CLI_ACCEPTED)
		{
#ifndef TEST
			printf("Me aceptaron en una sala ||| timestamp: %ld\n", rcv_packet.timestamp);
#endif
			send_ack(sockfd, token);
		}
		else if (rcv_packet.mtype == M_TYPE_DATA)
		{
#ifndef TEST
			printf("Mensaje recibido íntegramente: %s ||| timestamp: %ld\n", rcv_packet.payload, rcv_packet.timestamp);
#endif
			send_ack(sockfd, token);
		}
		else if (rcv_packet.mtype == M_TYPE_FIN)
		{
#ifndef TEST
			printf("Server cerró la conexion, terminando proceso ||| timestamp: %ld\n", rcv_packet.timestamp);
#endif
			close(sockfd);
			term = 1;
		}
		else if (rcv_packet.mtype == M_TYPE_FT_SETUP)
		{
			struct sockaddr_in ft_serv_addr; // Parámetro para el hilo
			unsigned short int new_port = *((unsigned short int *)&rcv_packet.payload);
			memset(&ft_serv_addr, '\0', sizeof(ft_serv_addr));
			ft_serv_addr.sin_family = AF_INET;
			bcopy((char *)(server->h_addr), (char *)&(ft_serv_addr.sin_addr.s_addr),
						(long unsigned int)server->h_length);
			ft_serv_addr.sin_port = new_port;
			pthread_t ft_thread;
			pthread_create(&ft_thread, NULL, recv_file, &ft_serv_addr);
		}
	}
	return 0;
}

void send_ack(int fd_sock, int token)
{
	packet_t packet_ack;
	gen_packet(&packet_ack, M_TYPE_ACK, &token, sizeof(token));
	if (write(fd_sock, &packet_ack, sizeof(packet_t)) == -1)
	{
		if (errno == EAGAIN)
			;
		else
			perror("write ack: ");
	}
}

void *recv_file(void *args)
{
	// recibe en formato de la network
	struct sockaddr_in ft_serv_addr = *((struct sockaddr_in*) args);
	int recv_file;
	int new_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (new_sock == -1)
	{
		perror("Fallo al abrir nuevo socket para recibir archivo");
		goto terminate;
	}
#ifndef TEST
	printf("Intentando conectarme al puerto: %d\n", ntohs(ft_serv_addr.sin_port));
#endif
	if (connect(new_sock, (struct sockaddr *)&ft_serv_addr, sizeof(ft_serv_addr)) < 0)
	{
		perror("Conexion a ft_server");
		goto terminate;
	}
#ifndef TEST
	printf("Me pude conectar al puerto: %d\n", ntohs(ft_serv_addr.sin_port));
#endif
	int end = 0;
	ssize_t recv_bytes;
	ft_packet_t packet;
#ifndef TEST
	ssize_t fsize;
#endif
	recv_file = open("rcv_file.zip", O_WRONLY | O_CREAT | O_EXCL, 0666);
	if (recv_file == -1)
	{
		perror("rcv_file.zip\n");
		goto terminate;
	}
	while (!end)
	{
		recv_bytes = recv(new_sock, &packet, sizeof(ft_packet_t), 0);
		if (recv_bytes == -1)
		{
			perror("recv_bytes");
			break;
		}
		if (packet.mtype == M_TYPE_FT_BEGIN)
		{
#ifndef TEST
			fsize = packet.data.fsize;
			printf("Tamaño del archivo: %ld\n", fsize);
#endif
		}
		else if (packet.mtype == M_TYPE_FT_DATA)
		{
			if (write(recv_file, packet.payload, packet.nbytes) == -1)
			{
				perror("Write archivo nuevo, archivo corrupto");
				end = 1;
			}
		}
		else if (packet.mtype == M_TYPE_FT_FIN)
		{
			end = 1;
		}
	}
terminate: ;
	close(new_sock);
	close(recv_file);
	pthread_exit(NULL);
	return NULL;
}
