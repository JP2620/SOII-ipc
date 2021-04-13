#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define QUEUE_NAME "/test_queue"
#define QUEUE_MAXMSG 16
#define QUEUE_MSGSIZE 50
#define QUEUE_PERMS ((int) (0666))

typedef struct msg_producer
{
  int id;
  union Data
  {
    float sysload;
    char random_msg[40];
    unsigned long free_mem;
  } data;
} msg_producer;

void join_existing_mq (char* mq_name);
