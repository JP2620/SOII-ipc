sudo kill -SIGTERM $(pidof server) $(pidof cli) $(pidof cliente)
rm logs/clientes/* log.txt input.txt log_cli.txt
	