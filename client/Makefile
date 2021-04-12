CC = gcc
VPATH = ./src

cliente: cliente.o protocol.o
	gcc $^ -o $@ -lssl -lcrypto

%.o: %.c
	gcc -c $< -o $@ -g

clean:
	rm *.o
	find . -maxdepth 1 -type f -executable -exec rm {} +
