#include "../include/protocol.h"
int gen_packet(packet_t* new_packet, int type, void* payload,
                size_t payload_len) 
{
    bzero(new_packet, sizeof(packet_t));
    while (time(& (new_packet->timestamp) ) == -1)
    {
        perror("time "); // Vuelve a intentar
    }
    new_packet->mtype = type;
    if (payload_len > PAYLOAD_SIZE)
    {
        fprintf(stderr, "No se permiten payloads mayores a %d bytes", PAYLOAD_SIZE);
        return -1;
    }
    memcpy(new_packet->payload, payload, payload_len);
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
    if (memcmp(calculated_hash, packet_hash, MD5_DIGEST_LENGTH) == 0) 
        return 1;
    else
        return 0;
}

int get_file_MD5(int fd_file, unsigned char out[MD5_DIGEST_LENGTH])
{
	MD5_CTX c;
	ssize_t nread;
	char buf[512];

	MD5_Init(&c);
	nread = read(fd_file, buf, sizeof(buf));
    if (nread == -1)
    {
        perror("Read file");
        return -1;
    }
	while (nread > 0)
	{
		MD5_Update(&c, buf, (size_t)nread);
		nread = read(fd_file, buf, sizeof(buf));
        if (nread == -1)
        {
            perror("Read file");
            return -1;
        }
	}
	MD5_Final(out, &c);
	return 0;
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
