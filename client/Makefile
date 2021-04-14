CC = gcc
CFLAGS = -Wall -pedantic -Werror -Wextra -Wconversion -std=gnu11
VPATH = ./src


cliente: cliente.o protocol.o
	gcc $^ -o $@ -lssl -lcrypto

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@ -g

clean:
	rm *.o
	find . -maxdepth 1 -type f -executable -exec rm {} +
