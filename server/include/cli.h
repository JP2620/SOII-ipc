// C program to implement one side of FIFO
// This side writes first, then reads
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#define CMD_ADD 1
#define CMD_DEL 2
#define CMD_LOG 3

typedef struct command {
  int type;
  int socket;
  int productor;
} command_t;
