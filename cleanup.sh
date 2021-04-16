sudo kill -SIGTERM $(pidof server) $(pidof cli) $(pidof cliente) \
  $(pidof prod_free_mem) $(pidof prod_sys_load) $(pidof prod_rand_msg)
rm logs/clientes/* logs/server_log.txt input.txt logs/log_cli.txt
	