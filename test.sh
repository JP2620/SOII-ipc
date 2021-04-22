#! /bin/bash
port=$(shuf -i 2000-64000 -n 1)
n=5000


sudo ./server/server $port &
sudo ./productores/prod_free_mem > /dev/null &
sudo ./productores/prod_rand_msg > /dev/null &
sudo ./productores/prod_sys_load > /dev/null &
touch input.txt
chmod 777 input.txt
sleep 1
for i in $(seq 1 $((n)))
do
  ./client/cliente localhost $port > ./logs/clientes/cliente_$((9+i)) &
  for j in $(seq 0 $(($RANDOM % 3)))
  do
    echo "add $((9+i)) $((j))" >> input.txt
  done
done
(cat input.txt ; cat) | ./server/cli > /dev/null

