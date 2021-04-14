#ifndef MQ_UTIL_H
#define MQ_UTIL_H

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

#define QUEUE_NAME "/mq_DM_prod"
#define QUEUE_MAXMSG 16
#define QUEUE_PERMS ((int) (0666))
mqd_t mq;
typedef struct msg_producer
{
  int id;
  time_t timestamp;
  union Data
  {
    float sysload;
    char random_msg[40];
    unsigned int free_mem;
  } data;
} msg_producer_t;

void join_existing_mq (char* mq_name, mqd_t* mq);
void handle_sigint(int sig);

#endif