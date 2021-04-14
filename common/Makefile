CC = gcc
CFLAGS = -Wall -pedantic -Werror -Wextra -Wconversion -std=gnu11
VPATH = ./src

all: protocol.o mq_util.o


%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm *.o
