#include "../include/protocol.h"
int gen_packet(packet* new_packet, int type, char* payload,
                size_t payload_len) 
{
    bzero(new_packet, sizeof(packet));
    if (time(& (new_packet->timestamp) ) == -1)
    {
        perror("time ");
        return -1;
    }
    new_packet->mtype = type;
    if (payload_len > PAYLOAD_SIZE)
    {
        fprintf(stderr, "No se permiten payloads mayores a %d bytes", PAYLOAD_SIZE);
        return -1;
    }
    strncpy(new_packet->payload, payload, PAYLOAD_SIZE);
    MD5((unsigned char*) new_packet, sizeof(packet) -
                                     sizeof(new_packet->hash), new_packet->hash);
}
