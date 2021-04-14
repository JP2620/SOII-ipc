CC = gcc
CFLAGS = -Wall -pedantic -Werror -Wextra -Wconversion -std=gnu11
VPATH = ./src
COMMON_O = ../common/mq_util.o

all: prod_free_mem prod_rand_msg prod_sys_load

prod_free_mem: prod_free_mem.o
	$(CC) $(CFLAGS) $^ $(COMMON_O) -o $@ -lrt

prod_rand_msg: prod_rand_msg.o
	$(CC) $(CFLAGS) $^ $(COMMON_O) -o $@ -lrt

prod_sys_load: prod_sys_load.o
	$(CC) $(CFLAGS) $^ $(COMMON_O) -o $@ -lrt

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ 

clean:
	rm *.o prod*
