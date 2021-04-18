#ifndef  PROTOCOL_H
#define  PROTOCOL_H

#include <stdio.h>
#include <string.h>
#include <openssl/md5.h>
#include <unistd.h>
#include <time.h>

#define PAYLOAD_SIZE 84
#define FT_PAYLOAD_SIZE 256

#define M_TYPE_ACK            1
#define M_TYPE_FIN            2
#define M_TYPE_DATA           3
#define M_TYPE_CLI_ACCEPTED   4
#define M_TYPE_CONN_ACCEPTED  5
#define M_TYPE_AUTH           6
#define M_TYPE_FT_SETUP       7 // File transfer setup
#define M_TYPE_FT_BEGIN       8
#define M_TYPE_FT_DATA        9
#define M_TYPE_FT_FIN         10

typedef struct packet {
    time_t timestamp;
    int mtype;
    char payload[PAYLOAD_SIZE];
    unsigned char hash[MD5_DIGEST_LENGTH];    
} packet_t;

typedef struct file_packet {
    int mtype; // Tipo de mensaje
    union data {
        ssize_t fsize;
        int nro_chunk;
    };
    char payload[FT_PAYLOAD_SIZE]; // Bytes del archivo
} ft_packet;

int gen_packet(packet_t* new_packet, int type, void* payload,
                size_t payload_len);
int check_packet_MD5(packet_t* packet);
void dump_packet(packet_t*);
              
#endif