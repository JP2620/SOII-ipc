#include "../include/mq_util.h"

void join_existing_mq (char* mq_name, mqd_t* mqd)
{
  do {
    *mqd = mq_open(mq_name, O_WRONLY);
    if (*mqd < 0)
    {
      perror("mq_open: ");
      sleep(2);
    }
  } while (*mqd == -1);
  printf("[PUBLISHER]: Queue opened, queue descriptor: %d\n", *mqd);
}

void handle_sigint(__attribute__((unused)) int sig) /* Suppress*/
{
  char notif[] = "Cerrando mqueue\n";
  write(STDOUT_FILENO, notif, sizeof(notif));
  fflush(stdout);
  if (mq_close(mq) == -1)
   perror("mq_close: ");
  exit(EXIT_SUCCESS);
}
