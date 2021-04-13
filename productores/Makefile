CC = gcc
CFLAGS = -Wall -pedantic -Werror -Wextra -Wconversion -std=gnu11
VPATH = ./src

all: prod_free_mem prod_rand_msg prod_sys_load

prod_free_mem: prod_free_mem.o mq_util.o
	$(CC) $(CFLAGS) $^ -o $@ -lrt

prod_rand_msg: prod_rand_msg.o
	$(CC) $(CFLAGS) $^ -o $@ -lrt

prod_sys_load: prod_sys_load.o
	$(CC) $(CFLAGS) $^ -o $@ -lrt

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ -lrt

clean:
	rm *.o prod*
	find . -maxdepth 1 -type f -executable -exec rm {} +	
