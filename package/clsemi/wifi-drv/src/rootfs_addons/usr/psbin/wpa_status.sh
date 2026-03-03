#/bin/sh

##输出当前进程PID
echo "PID = "$$

root_path=$(cd $(dirname $0);pwd)
#echo "root_path: "${root_path}
cd ${root_path}

script_name=$0

wpa_interf="wlan0"
wpa_interf_flag="false"
status_flag="false"

scan_flag="false"



for i in "$@"; do
	if [ ${wpa_interf_flag} = "true" ]; then
		wpa_interf_flag="false"
		wpa_interf="$i"
		continue
	fi

	if [ "$i" = "-i" ];then
		wpa_interf_flag="true"
		show_help_info="false"
	fi

	if [ "$i" = "status" ];then
		status_flag="true"
		show_help_info="false"
	fi

done

if [ ${status_flag} = "true" ]; then
	if [ -f /var/run/wpa_supplicant_${wpa_interf} ]; then
		wpa_cli -p /var/run/wpa_supplicant_${wpa_interf} status
	else
		wpa_cli status
	fi
fi

if [ ${scan_flag} = "true" ]; then
	if [ -f /var/run/wpa_supplicant_${wpa_interf} ]; then
		wpa_cli -p /var/run/wpa_supplicant_${wpa_interf} -i ${wpa_interf} scan
		wpa_cli -p /var/run/wpa_supplicant_${wpa_interf} -i ${wpa_interf} scan_results
	else
		wpa_cli -i ${wpa_interf} scan
		wpa_cli -i ${wpa_interf} scan_results
	fi
fi



echo "============= $script_name ============="
