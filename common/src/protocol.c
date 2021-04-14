#include "../include/protocol.h"
int gen_packet(packet_t* new_packet, int type, char* payload,
                size_t payload_len) 
{
    bzero(new_packet, sizeof(packet_t));
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
    MD5((unsigned char*) new_packet, sizeof(packet_t) -
                                     sizeof(new_packet->hash), new_packet->hash);
    return 0;
}

int check_packet_MD5(packet_t* inc_packet)
{
    unsigned char *packet_hash = inc_packet->hash;
    unsigned char calculated_hash [MD5_DIGEST_LENGTH];
    MD5((unsigned char*) inc_packet, sizeof(packet_t) -
                                     sizeof(inc_packet->hash), calculated_hash);
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
    {
        if (packet_hash[i] != calculated_hash[i])
        {
            return 0;
        }
    }
    return 1;
}

void dump_packet(packet_t* packet_to_dump)
{
    unsigned char *byte_ptr = (unsigned char*) packet_to_dump;
    for (unsigned int i = 0; i < sizeof(packet_t); i++)
    {
        printf("%02x ", byte_ptr[i]);
    }
    printf("\n");
}
