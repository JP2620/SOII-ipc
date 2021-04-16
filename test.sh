#! /bin/bash
port=$(shuf -i 2000-64000 -n 1)
n=1024


./server/server $port >> logs/server_log.txt &
sudo ./productores/prod_free_mem > /dev/null &
sudo ./productores/prod_rand_msg > /dev/null &
sudo ./productores/prod_sys_load > /dev/null &
touch input.txt
chmod 777 input.txt
for i in $(seq 1 $((n)))
do
  ./client/cliente localhost $port > ./logs/clientes/cliente_$((7+i)) &
  for j in $(seq 0 $(($RANDOM % 3)))
  do
    echo "add $((7+i)) $((j))" >> input.txt
  done
done
sed -i '$d' input.txt
sudo ./server/cli < input.txt > logs/log_cli.txt &
