#ifndef  PROTOCOL_H
#define  PROTOCOL_H

#include <stdio.h>
#include <string.h>
#include <openssl/md5.h>
#include <unistd.h>
#include <time.h>

#define PAYLOAD_SIZE 84

#define M_TYPE_ACK          1
#define M_TYPE_FIN          2
#define M_TYPE_DATA         3
#define M_TYPE_CLI_ACCEPTED 4

typedef struct packet {
    time_t timestamp;
    int mtype;
    char payload[PAYLOAD_SIZE];
    unsigned char hash[MD5_DIGEST_LENGTH];    
} packet;

int gen_packet(packet* new_packet, int type, char* payload,
                size_t payload_len);
int check_packet_MD5(packet* packet);
void dump_packet(packet*);
              
#endif