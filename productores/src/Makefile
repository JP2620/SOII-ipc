CC = gcc

prod_free_mem: prod_free_mem.o
	$(CC) $^ -o $@

%.o: %.c
	$(CC) -c $< -o $@
