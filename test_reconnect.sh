pid=$(awk -F ' ' -v row=1 -v col=5 'NR==row{print $col}' logs/clientes/cliente_$1)
token=$(awk -F ' ' -v row=1 -v col=9 'NR==row{print $col}' logs/clientes/cliente_$1)
port=$(awk '/socket disponible/{print $NF}' logs/server_log.txt)
ip="localhost"

sudo kill -SIGTERM $pid
sleep 4s #espero
sudo ./client/cliente $ip $port -t $token
