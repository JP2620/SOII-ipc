#include "../include/srv_util.h"



void broadcast_room(list_t* room, packet *msg)
{
	int n;
	for (node_t *iterator = room->head; iterator->next != NULL;
			 iterator = iterator->next)
	{
		n = write (* ( (int*)iterator->data ) , (char*) msg, sizeof(packet));
		if (n < 0) 
			perror("write ");
	}	
}
