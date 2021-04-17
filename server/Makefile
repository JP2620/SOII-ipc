CC = gcc
CFLAGS = -Wall -pedantic -Werror -Wextra -Wconversion -std=gnu11
COMMON_O = ../common/protocol.o ../common/mq_util.o
VPATH = ./src

all: server cli

server: srv.o list.o srv_util.o zip.o
	gcc $^ -o $@ $(COMMON_O) -lssl -lcrypto -lrt

cli: cli.o
	gcc $^ -o $@

zip.o: zip.c
	gcc -c $< -o $@

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@ -g

clean:
	rm *.o
	find . -maxdepth 1 -type f -executable -exec rm {} +
