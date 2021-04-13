#include "../include/mq_util.h"


int gen_rand_msg(char* buff, size_t buf_len);


int main (void) {
	signal(SIGINT, handle_sigint);

	join_existing_mq(QUEUE_NAME, &mq);
  printf("[PUBLISHER]: Queue opened, queue descriptor: %d\n", mq);

	unsigned int prio = 0;
	int count = 1;
	msg_producer msg;

	for (;;)
	{
    sleep(1);
		memset(&msg, '\0', sizeof(msg));
    if (time(&msg.timestamp) == ((time_t) -1)) 
      continue;
		msg.id = 1;
		int retval = gen_rand_msg(msg.data.random_msg, sizeof(msg.data.random_msg));
		if (retval == -1)
			continue;
		
		if (mq_send(mq, (const char*) &msg, sizeof(msg), prio) == -1)
		{
			perror("mq_send: ");
			continue;
		}

		printf("[PUBLISHER]: Sending message %d\n", count);
		count++;
		fflush(stdout);
	}
	return 0;
}

int gen_rand_msg(char* buff, size_t buf_len)
{
	time_t my_seed;
	time(&my_seed);
	if (my_seed == ((time_t) -1))
	{
		perror("time: ");
		return -1;
	}
	srand((unsigned int) my_seed);
	char char1[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234"
        "56789/,.-+=~`<>:";
	for(size_t index = 0; index < buf_len; index++)
	{
		buff[index] = char1[ ((size_t) rand()) % (sizeof char1 - 1)];
	}
	return 0;
}
