CC = gcc
CFLAGS = -Wall -pedantic -Werror -Wextra -Wconversion -std=gnu11 -pthread
COMMON_O = ../common/protocol.o
VPATH = ./src

all: cliente
cliente: cliente.o
	gcc -pthread $^ $(COMMON_O) -o $@ -lssl -lcrypto

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@ -g

clean:
	rm *.o
	find . -maxdepth 1 -type f -executable -exec rm {} +
