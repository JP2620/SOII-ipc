#include "../../common/include/mq_util.h"

#define MSG_LENGTH 50

int get_sys_load(float* sys_load_1min);

int main()
{
  signal(SIGINT, handle_sigint);

  join_existing_mq(QUEUE_NAME, &mq); // Consigo fd de la mqueue
  printf("[PUBLISHER]: Queue opened, queue descriptor: %d\n", mq);

  unsigned int prio = 0; // Seteo prio de mensajes
  int count = 1; // Contador de mensajes
  msg_producer_t msg;

  for (;;)
  {
    sleep(3);
    memset(&msg, '\0', sizeof(msg)); // Limpio buffer
    if (time(&(msg.timestamp)) == ((time_t) -1)) // Seteo campos del msg
      continue;
    msg.id = 2;
    int retval = get_sys_load(&msg.data.sysload);
    if (retval == -1)
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


int get_sys_load(float* sys_load_1min) {
  FILE *fptr = fopen("/proc/loadavg", "r");
  if (fptr == NULL) {
    fprintf(stderr, "Falló al abrir /proc/loadavg");
    return -1;
  }

  if (fscanf(fptr, "%f", sys_load_1min) != 1)
  {
    fprintf(stderr, "fallo al buscar system load");
    return -1;
  }

  return 0;

}
