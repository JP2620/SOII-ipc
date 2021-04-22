#include "../../common/include/mq_util.h"

unsigned int get_free_mem();

int main()
{
  signal(SIGTERM, handle_SIGTERM);
  

  join_existing_mq(QUEUE_NAME, &mq); // Consigo fd de la mqueue
  printf("[PUBLISHER]: Queue opened, queue descriptor: %d\n", mq);

  unsigned int prio = 0; // Seteo prio de mensajes
  int count = 1; // Contador de mensajes
  msg_producer_t msg;

  for (;;)
  {
    sleep(1);
    memset(&msg, '\0', sizeof(msg)); // Limpio buffer
    if (time(&(msg.timestamp)) == ((time_t) -1)) // Seteo campos del msg
      continue;
    msg.id = 0;
    msg.data.free_mem = get_free_mem();
    if (msg.data.free_mem == 0)
      continue;
    if (mq_send(mq, (const char*) &msg, sizeof(msg), prio) == -1) // Envío de msg
    {
      perror("mq_send: ");
      continue;
    }
    printf("[PUBLISHER]: Sending message %d\n", count);
    count++;
    fflush(stdout); // Para que se printee de a una linea.

  }
  return 0;
}

unsigned int get_free_mem()
{
  unsigned int mem_free;
  FILE *fptr = fopen("/proc/meminfo", "r");
  if (fptr == NULL)
  {
    perror("fopen: ");
    return 0;
  }
	if (fscanf(fptr, "%*s %*u %*s %*s %*u %*s %*s %u", &mem_free) != 1)
  {
    fprintf(stderr, "Fallo el fscanf de /proc/meminfo");
    fclose(fptr);
    return 0;
  }
  while (fclose(fptr) == EOF)
  {
    perror("fclose: ");
    sleep(1); // Vuelve a intentar cerrarlo en 1 segundo.
  }
  return mem_free;
}
