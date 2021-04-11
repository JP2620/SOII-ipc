#include "../include/srv_util.h"



void broadcast_room(list_t* room, packet_t *msg)
{
	int n;
	for (node_t *iterator = room->head; iterator->next != NULL;
			 iterator = iterator->next)
	{
		n = write (* ( (int*)iterator->data ) , (char*) msg, sizeof(packet_t));
		if (n < 0) 
			perror("write ");
	}	
}

int conn_compare(connection_t* con1, connection_t* con2)
{
	if (con1->sockfd == con2->sockfd)
		return 0;
	else
		return -1;
}

void conn_free(connection_t* connection)
{
	return;
}
