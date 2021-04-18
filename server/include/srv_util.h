#include "./list.h"
#include "../../common/include/protocol.h"
#include "../include/zip.h"
#include <strings.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>

#define TAM 256
#define MAX_EVENT_NUMBER 10000 // Poco probable que ocurran 5000 eventos
#define CONN_TIMEOUT 5
#define BUFFER_CLEANUP_PERIOD 30
#define LOG_CLIENTES "logs/server/log_DM_clientes"
#define LOG_PRODUCTORES "logs/server/log_DM_productores"

typedef struct connection {
  time_t timestamp;
  int susc_counter;
  int sockfd;
  int token;
} connection_t;

FILE *fptr_log_clientes, *fptr_log_productores;

void conn_free(connection_t* connection);
int conn_compare(connection_t* con1, connection_t* con2);
void broadcast_room(list_t* room, packet_t *msg);
int compare_fd(int*, int*);

int set_non_blocking(int fd);
void add_fd(int epollfd, int fd);

void send_fin(int sockfd);
connection_t* find_by_socket(int fd, list_t* list);
void garb_collec_old_packets(list_t* buffered_packets, time_t *last_gc, unsigned int period);

