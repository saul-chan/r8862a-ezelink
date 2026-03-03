#/bin/sh

##输出当前进程PID
echo "PID = "$$

root_path=$(cd $(dirname $0);pwd)
#echo "root_path: "${root_path}
cd ${root_path}

script_name=$0

hostapd_interf="wlan0"
hostapd_interf_flag="false"
status_flag="false"

show_all_sta_flag="false"

for i in "$@"; do
	if [ ${hostapd_interf_flag} = "true" ]; then
		hostapd_interf_flag="false"
		hostapd_interf="$i"
		continue
	fi

	if [ "$i" = "-i" ];then
		hostapd_interf_flag="true"
		show_help_info="false"
	fi

	if [ "$i" = "status" ];then
		status_flag="true"
		show_help_info="false"
	fi
	if [ "$i" = "all_sta" ];then
		show_all_sta_flag="true"
		show_help_info="false"
	fi
done

hostapd_cli_path=hostapd_cli

if [ -f /usr/psbin/hostapd_cli ]; then
hostapd_cli_path=/usr/psbin/hostapd_cli
fi

if [ ${status_flag} = "true" ]; then
	if [ -f /var/run/hostapd_${hostapd_interf} ]; then
		$hostapd_cli_path -p /var/run/hostapd_${hostapd_interf} status
	else
		$hostapd_cli_path status
	fi
fi

if [ ${show_all_sta_flag} = "true" ]; then
	if [ -f /var/run/hostapd_${hostapd_interf} ]; then
		$hostapd_cli_path -p /var/run/hostapd_${hostapd_interf} all_sta
	else
		$hostapd_cli_path all_sta
	fi
fi


echo "============= $script_name ============="
