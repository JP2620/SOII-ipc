#include <stdio.h>
#include <stdlib.h>
#include "../include/mq_util.h"

#define MSG_LENGTH 50
unsigned int get_free_mem();

int main()
{
  char msg_buf[MSG_LENGTH];
  sprintf(msg_buf, "%u kB", get_free_mem());
  printf("%s\n", msg_buf);
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
