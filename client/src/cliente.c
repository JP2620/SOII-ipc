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

int main(int argc, char *argv[])
{

	// setup socket
	int sockfd, term, token;
	unsigned short puerto;
	long int n;
	struct sockaddr_in serv_addr, ft_serv_addr;
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
			printf("Conexión aceptada, pid = %d ", getpid());
			if (tflag)
			{
				packet_t packet;
				int tokens[2] = {token, *((int *)rcv_packet.payload)}; // Token viejo, seguido de token nuevo
				gen_packet(&packet, M_TYPE_AUTH, tokens, sizeof(tokens));
				if (write(sockfd, &packet, sizeof(packet_t)) == -1)
					perror("write CONN_ACCEPTED: ");
				printf("reconectando ||| timestamp: %ld\n", time(NULL));
				send_ack(sockfd, tokens[0]);
			}
			else
			{
				token = *((int *)rcv_packet.payload);
				printf("y token = %d ||| timestamp: %ld\n", token, rcv_packet.timestamp);
				send_ack(sockfd, token);
			}
		}
		else if (rcv_packet.mtype == M_TYPE_CLI_ACCEPTED)
		{
			printf("Me aceptaron en una sala ||| timestamp: %ld\n", rcv_packet.timestamp);
			send_ack(sockfd, token);
		}
		else if (rcv_packet.mtype == M_TYPE_DATA)
		{
			printf("Mensaje recibido íntegramente: %s ||| timestamp: %ld\n", rcv_packet.payload, rcv_packet.timestamp);
			send_ack(sockfd, token);
		}
		else if (rcv_packet.mtype == M_TYPE_FIN)
		{
			printf("Server cerró la conexion, terminando proceso ||| timestamp: %ld\n", rcv_packet.timestamp);
			close(sockfd);
			term = 1;
		}
		else if (rcv_packet.mtype == M_TYPE_FT_SETUP)
		{
			printf("Recibido M_TYPE_FT_SETUP\n");
			// recibe en formato de la network
			unsigned short int new_port = *((unsigned short int *)&rcv_packet.payload); 
			int new_sock = socket(AF_INET, SOCK_STREAM, 0);
			memset(&ft_serv_addr, '\0', sizeof(ft_serv_addr));
			ft_serv_addr.sin_family = AF_INET;
			bcopy((char *)(server->h_addr), (char *)&(ft_serv_addr.sin_addr.s_addr),
						(long unsigned int)server->h_length);
			ft_serv_addr.sin_port = new_port;
			printf("Intentando conectarme al puerto: %d\n", ntohs(ft_serv_addr.sin_port));
			if (connect(new_sock, (struct sockaddr *)&ft_serv_addr, sizeof(ft_serv_addr)) < 0)
			{
				perror("Conexion a ft_server");
				exit(EXIT_FAILURE);
			}
			printf("Me pude conectar al puerto: %d\n", ntohs(ft_serv_addr.sin_port));

			// int end = 0;
			// int recv_bytes;
			// packet_t packet;
			// int recv_file = open("log.zip", O_WRONLY | O_CREAT | O_EXCL, 0666);
			// if (recv_file == -1)
			// {
			// 	perror("Fallo al crear log.zip\n");
			// 	continue;
			// }
			// while (!end)
			// {
			// 	recv_bytes = recv(new_sock, &packet, sizeof(packet_t), 0);
			// }
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
