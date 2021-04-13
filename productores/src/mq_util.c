#include "../include/mq_util.h"

void join_existing_mq (char* mq_name, mqd_t* mq)
{
  do {
    mq = mq_open(QUEUE_NAME, O_WRONLY);
    if (mq < 0)
    {
      perror("mq_open: ");
      sleep(2);
    }
  } while (mq == -1);
  printf("[PUBLISHER]: Queue opened, queue descriptor: %d\n", mq);
}
