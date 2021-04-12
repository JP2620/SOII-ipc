CC = gcc
VPATH = ./src

all: server cli

server: srv.o list.o protocol.o srv_util.o
	gcc $^ -o $@ -lssl -lcrypto

cli: cli.o
	gcc $^ -o $@

%.o: %.c
	gcc -c $< -o $@ -g

clean:
	rm *.o
	find . -maxdepth 1 -type f -executable -exec rm {} +
	rm /tmp/cli_dm_fifo
