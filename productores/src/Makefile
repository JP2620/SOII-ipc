CC = gcc

all: prod_rand_msg

prod_rand_msg: prod_rand_msg.o
	$(CC) $< -o $@

%.o: %.c
	$(CC) -c $< -o $@ -g

clean:
	rm *.o
	find . -maxdepth 1 -type f -executable -exec rm {} +