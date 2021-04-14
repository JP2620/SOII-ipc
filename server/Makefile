CC = gcc
CFLAGS = -Wall -pedantic -Werror -Wextra -Wconversion -std=gnu11
VPATH = ./src

all: server cli

server: srv.o list.o protocol.o srv_util.o
	gcc $^ -o $@ -lssl -lcrypto -lrt

cli: cli.o
	gcc $^ -o $@

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@ -g

clean:
	rm *.o
	find . -maxdepth 1 -type f -executable -exec rm {} +
