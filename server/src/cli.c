// C program to implement one side of FIFO
// This side writes first, then reads
// Quiero lectura bloqueante de stdin
// Seguido de escritura bloqueante al pipe

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "../include/cli.h"

int parse_command(char *string, command_t *command);

int main()
{
	// FIFO file path
	char * myfifo = "/tmp/cli_dm_fifo";

	// Creating the named file(FIFO)
	// mkfifo(<pathname>, <permission>)
	mkfifo(myfifo, 0666);

	char buf[80];
	while (1)
	{
		// Open FIFO for write only
		int fd = open(myfifo, O_WRONLY);

		// Leo input de stdin
		memset(buf, '\0', 80);
		fgets(buf, 80, stdin);

		// Si sale todo bien guardo en command
		// La info relevante
		command_t command;
		if (parse_command(buf, &command) == -1)
		{
			printf("Mal uso de la CLI\n");
			fflush(stdout);
			sleep(1);
			continue; // Si sale mal, siguiente iteracion
		}

		// Envío y chau
		write(fd, &command, sizeof(command_t));
		close(fd);
	}
	return 0;
}

int parse_command(char *string, command_t *command)
{
	char *token = strtok(string, " ");
	char *tokens[3];
	memset(tokens, 0, sizeof(char*) * 3);
	
	tokens[0] = token;
	int i;
	for (i = 1; (token = strtok(NULL, " ")) ; i++)
	{
		if (i == 3)
			return -1; // No deberian haber mas de 3 tokens
		tokens[i] = token;
	}

	if (i == 1)
		return -1;

	if (strcmp(tokens[0], "add") == 0 && i == 3)
	{
		command->type = CMD_ADD;
		command->socket = atoi(tokens[1]);
		command->productor = atoi(tokens[2]);
	}
	else if (strcmp(tokens[0], "delete") == 0 && i == 3)
	{
		command->type = CMD_DEL;
		command->socket = atoi(tokens[1]);
		command->productor = atoi(tokens[2]);
	}
	else if (strcmp(tokens[0], "log") == 0 && i == 2)
	{
		command->type = CMD_LOG;
		command->socket = atoi(tokens[1]);
	}
	else
		return -1;
	return 0;
}


