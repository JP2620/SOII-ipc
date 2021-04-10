CC = gcc
VPATH = ./src

server: srv.o list.o protocol.o srv_util.o
	gcc $^ -o $@ -lssl -lcrypto
%.o: %.c
	gcc -c $< -o $@ -g

clean:
	rm *.o
	find . -maxdepth 1 -type f -executable -exec rm {} +
