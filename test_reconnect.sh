pid=$(pcregrep -o1 "pid\s=\s([0-9]+)" logs/clientes/cliente_$1)
token=$(pcregrep -o1 "token\s=\s([0-9]+)" logs/clientes/cliente_$1)
port=$(awk '/socket disponible/{print $NF}' ./logs/server/log_DM_clientes)
ip="localhost"

sudo kill -SIGTERM $pid
sleep 2s #espero
sudo ./client/cliente $ip $port -t $token
