#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "../include/mq_util.h"

#define MSG_LENGTH 50
void gen_rand_msg(char* buff, size_t buf_len);


int main (void) {
    char msg_buf[MSG_LENGTH];
    memset(msg_buf, '\0', sizeof(msg_buf));
    gen_rand_msg(msg_buf, sizeof(msg_buf) - 1);
    msg_buf[MSG_LENGTH] = '\0';
	return 0;
}

void gen_rand_msg(char* buff, size_t buf_len)
{
	srand((unsigned int)(time(NULL)));
	char char1[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234"
        "56789/,.-+=~`<>:";
	for(size_t index = 0; index < buf_len; index++)
	{
		buff[index] = char1[ ((size_t) rand()) % (sizeof char1 - 1)];
	}
}
