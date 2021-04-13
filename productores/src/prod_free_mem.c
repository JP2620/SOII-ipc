#include <stdio.h>
#include <stdlib.h>
#include "../include/mq_util.h"

unsigned int get_free_mem();

int main()
{
  mqd_t mq;
  join_existing_mq(QUEUE_NAME, &mq); // Consigo fd de la mqueue
  printf("[PUBLISHER]: Queue opened, queue descriptor: %d\n", mq);

  unsigned int prio = 0; // Seteo prio de mensajes
  int count = 1; // Contador de mensajes
  msg_producer msg;

  for (;;)
  {
    memset(&msg, '\0', sizeof(msg)); // Limpio buffer
    time(&(msg.timestamp)); // Seteo campos del msg
    msg.id = 0;
    msg.data.free_mem = get_free_mem();
    mq_send(mq, (const char*) &msg, sizeof(msg), prio); // Envío bloqueante de msg

    printf("[PUBLISHER]: Sending message %d\n", count);
    count++;
    fflush(stdout); // Para que se printee de a una linea.
    sleep(1);
  }
  return 0;
}

unsigned int get_free_mem()
{
  unsigned int mem_free;
  FILE *fptr = fopen("/proc/meminfo", "r");
	if (fscanf(fptr, "%*s %*u %*s %*s %*u %*s %*s %u", &mem_free) != 1)
  {
    fprintf(stderr, "Fallo el fscanf de /proc/meminfo");
  }
  return mem_free;
}
