#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/mq_util.h"

#define MSG_LENGTH 50

float get_sys_load();

int main()
{
  char msg_buf[MSG_LENGTH];
  memset(msg_buf, '\0', sizeof(msg_buf));
  sprintf(msg_buf, "%2.3f load avg 1min", get_sys_load());
  printf("%s\n", msg_buf);
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
