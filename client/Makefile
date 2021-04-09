CC = gcc

cliente: cliente.o
	gcc $^ -o $@
%.o: %.c
	gcc -c $< -o $@ -g

clean:
	rm *.o
	find . -maxdepth 1 -type f -executable -exec rm {} +
