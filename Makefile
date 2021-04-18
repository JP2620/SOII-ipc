all: client producers server

client: common
	$(MAKE) all -C ./client

producers: common
	$(MAKE) all -C ./productores

server: common
	$(MAKE) all -C ./server

common:
	$(MAKE) all -C ./common



clean:
	$(MAKE) clean -i -C ./common
	$(MAKE) clean -i -C ./client
	$(MAKE) clean -i -C ./productores
	$(MAKE) clean -i -C ./server

