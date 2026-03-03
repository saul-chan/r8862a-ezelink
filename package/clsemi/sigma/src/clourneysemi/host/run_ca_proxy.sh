#!/bin/bash

sigma_ca_proxy="/usr/local/clourneysemi/sigma_ca/sigma_ca_proxy.py"
sigma_tg="/usr/local/clourneysemi/sigma_ca/tg/start_tg.sh"
sigma_ca_config_file="/usr/local/clourneysemi/sigma_ca/sigma_ca.conf"

log_dir='/var/log/sigma_ca'
log_base="${log_dir}/$(date +%Y-%m-%d-%H-%M-%s)"


ca_ip='192.165.123.40'
ca_port=9000

server_ip='0.0.0.0'
server_port=9000

read_config()
{
    while IFS='= ' read var val
    do
        if [[ $var == \#* ]]
        then
            true # ignore comment
        elif [[ $var == \[*] ]]
        then
            section=`echo "$var" | tr -d "[] "`
        elif [[ $val ]]
        then
            eval $section$var="$val"
        fi
    done < $1
}

if [ -e "$sigma_ca_config_file" ]
then
    read_config "$sigma_ca_config_file"
fi

if [ -n "$boardnc_host" ]
then
    ca_ip="$boardnc_host"
fi

if [ -n "$boardnc_port" ]
then
    ca_port="$boardnc_port"
fi

if [ -n "$serverca_port" ]
then
    server_port="$serverca_port"
fi

if [ -n "$serverca_host" ]
then
    server_ip="$serverca_host"
fi


sigma_ca_cmd="python $sigma_ca_proxy --tg-ip=127.0.0.1 --tg-port=9099 --server-ip=$server_ip --server-port=$server_port --ca-ip=$ca_ip  --ca-port=$ca_port"
sigma_tg_cmd="$sigma_tg"

start_service() {
	mkdir -p $log_dir
	(cd ${log_dir} ; touch ${log_base}_sigma_ca.log ; rm -f sigma_ca.log ; ln ${log_base}_sigma_ca.log sigma_ca.log; chmod 666 ${log_base}_sigma_ca.log sigma_ca.log )
    (cd ${log_dir} ; touch ${log_base}_sigma_tg.log ; rm -f sigma_tg.log ; ln ${log_base}_sigma_tg.log sigma_tg.log; chmod 666 ${log_base}_sigma_tg.log sigma_tg.log )

	nohup ${sigma_ca_cmd} > ${log_base}_sigma_ca.log 2>&1 &
	nohup ${sigma_tg_cmd} > ${log_base}_sigma_tg.log 2>&1 &
}

stop_service() {
	pkill -9 -f sigma_ca_proxy.py
	pkill -f start_tg.sh
}

case "$1" in
	start)
		echo "Starting Sigma services:"
		start_service
		;;
	stop)
		echo "Stopping Sigma Services"
		stop_service
		;;
	restart)
		echo "Restarting Sigma Services"
		stop_service
		sleep 3
		start_service
		;;
	*)
		echo "usage: $0 start|stop|restart"
		;;
esac

