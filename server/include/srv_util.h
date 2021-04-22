#include "./list.h"
#include "../../common/include/protocol.h"
#include "../include/zip.h"
#include <strings.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/stat.h>
#include <stdarg.h>

#define TAM 256
#define FT_PORT 12345
#define MAX_EVENT_NUMBER 10000 // Poco probable que ocurran 5000 eventos
#define LISTEN_BACKLOG 10000 // Máximo de encolados
#define CONN_TIMEOUT 5
#define BUFFER_CLEANUP_PERIOD 30
#define LOG_CLIENTES "logs/server/log_DM_clientes"
#define LOG_PRODUCTORES "logs/server/log_DM_productores"
#define NO_PRODUCTORES 3
#define POLLING_INTERVAL 10 // En ms

#define CHECK(x) do { \
  int retval = (x); \
  if (retval != 0) { \
    fprintf(stderr, "Runtime error: %s returned %d at %s:%d", #x, retval, __FILE__, __LINE__); \
    return retval/* or throw or whatever */; \
  } \
} while (0)

typedef struct connection {
  time_t timestamp;
  int susc_counter;
  int sockfd;
  int token;
} connection_t;

FILE *fptr_log_clientes, *fptr_log_productores;

int setup_tcpsocket(uint16_t port, struct sockaddr_in*);
int conn_compare(connection_t* con1, connection_t* con2);
void broadcast_room(list_t* room, packet_t *msg);
int compare_fd(int*, int*);

int set_non_blocking(int fd);
void add_fd(int epollfd, int fd);

void send_fin(int sockfd);
connection_t* find_by_socket(int fd, list_t* list);
void garb_collec_old_packets(list_t* buffered_packets, time_t *last_gc, unsigned int period);
void garb_collec_old_conn(list_t* connections, list_t* broadcast_rooms[NO_PRODUCTORES],
												  time_t *last_gc, unsigned int period, int epollfd);
void *handle_loq_req(void* args);
int log_event(FILE* log, char* event_str, ...);
int get_datetime(char* buf, size_t max_len);
