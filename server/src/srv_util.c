#include "../include/srv_util.h"

void broadcast_room(list_t* room, char* msg, size_t msg_len)
{
	int n;
	for (node_t *iterator = room->head; iterator->next != NULL;
			 iterator = iterator->next)
	{
		n = write (* ( (int*)iterator->data ) , msg, msg_len);
		if (n < 0) 
			perror(strerror(errno));
	}
}
