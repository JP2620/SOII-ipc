#include "../include/mq_util.h"


void gen_rand_msg(char* buff, size_t buf_len);


int main (void) {
	mqd_t mq;
	join_existing_mq(QUEUE_NAME, &mq);
  printf("[PUBLISHER]: Queue opened, queue descriptor: %d\n", mq);

	unsigned int prio = 0;
	int count = 1;
	msg_producer msg;

	for (;;)
	{
		memset(&msg, '\0', sizeof(msg));
		time(&(msg.timestamp));
		msg.id = 1;
		gen_rand_msg(msg.data.random_msg, sizeof(msg.data.random_msg));
		if (mq_send(mq, (const char*) &msg, sizeof(msg), prio) == -1)
		{
			perror("mq_send: ");
			exit(EXIT_FAILURE);
		}

		printf("[PUBLISHER]: Sending message %d\n", count);
		count++;
		fflush(stdout);
		sleep(1);
	}
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
