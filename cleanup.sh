sudo kill -SIGTERM $(pidof server) $(pidof cli) $(pidof cliente) \
  $(pidof prod_free_mem) $(pidof prod_sys_load) $(pidof prod_rand_msg)
rm logs/clientes/* logs/server/* input.txt *.zip client/*.zip
	