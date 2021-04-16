all: common client producers server

common:
	$(MAKE) -C ./common

client:
	$(MAKE) -C ./client

producers:
	$(MAKE) -C ./productores

common:
	$(MAKE) -C ./server

clean:
	$(MAKE) clean -i -C ./common
	$(MAKE) clean -i -C ./client
	$(MAKE) clean -i -C ./productores
	$(MAKE) clean -i -C ./server

