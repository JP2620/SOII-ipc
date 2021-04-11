#include "./list.h"
#include "./protocol.h"
#include <strings.h>
#include <errno.h>

typedef struct connection {
  time_t timestamp;
  int susc_counter;
  int sockfd;
} connection_t;


void conn_free(connection_t* connection);
int conn_compare(connection_t* con1, connection_t* con2);
void broadcast_room(list_t* room, packet_t *msg);

int set_non_blocking(int fd);
void add_fd(int epollfd, int fd);
