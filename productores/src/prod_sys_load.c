#include "../include/mq_util.h"

#define MSG_LENGTH 50

float get_sys_load();

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
    msg.id = 2;
    msg.data.sysload = get_sys_load();
    if (mq_send(mq, (const char*) &msg, sizeof(msg), prio) == -1) // Envío de msg
    {
      perror("mq_send: ");
      exit(EXIT_FAILURE);
    }
    printf("[PUBLISHER]: Sending message %d\n", count);
    count++;
    fflush(stdout); // Para que se printee de a una linea.
    sleep(1);
  }

  return 0;
}


float get_sys_load() {
  float sys_load_1min;
  FILE *fptr = fopen("/proc/loadavg", "r");
  if (fptr == NULL) {
    fprintf(stderr, "Falló al abrir /proc/loadavg");
    exit(EXIT_FAILURE);
  }

  if (fscanf(fptr, "%f", &sys_load_1min) != 1)
  {
    fprintf(stderr, "fallo al buscar system load");
    exit(EXIT_FAILURE);
  }

  return sys_load_1min; 

}
