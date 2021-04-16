#! /bin/bash
port=5000
n=50


./server/server $port >> log.txt &
sudo ./productores/prod_free_mem > /dev/null &
sudo ./productores/prod_rand_msg > /dev/null &
sudo ./productores/prod_sys_load > /dev/null &
touch input.txt
chmod 777 input.txt
for i in $(seq 1 $((n)))
do
  ./client/cliente localhost $port > ./logs/clientes/cliente_$i &
  for j in $(seq 0 $(($RANDOM % 3)))
  do
    echo "add $((6+i)) $((j))" >> input.txt
  done
done
sed -i '$d' input.txt
sudo ./server/cli < input.txt > log_cli.txt &
