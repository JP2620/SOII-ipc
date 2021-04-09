CC = gcc

prod_sys_load: prod_sys_load.o
	$(CC) $^ -o $@

%.o: %.c
	$(CC) -c $< -o $@

clean:
	rm *.o
	find . -maxdepth 1 -type f -executable -exec rm {} +	