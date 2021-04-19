all:
	$(MAKE) all -C ./common
	$(MAKE) all -C ./client
	$(MAKE) all -C ./server
	$(MAKE) all -C ./productores

clean:
	$(MAKE) clean -i -C ./common
	$(MAKE) clean -i -C ./client
	$(MAKE) clean -i -C ./productores
	$(MAKE) clean -i -C ./server

