#!/bin/bash

#set -e

#root_path=$(cd $(dirname $0);pwd)
script_name=$0

echo ""
#echo "root_path: "${root_path}
#cd ${root_path}

usage(){
	echo "$script_name rate_idx mac_addr"
	echo "rate_idx    : 0/1/2/3/4/5/... ..."
	echo "mac_addr    : xx:xx:xx:xx:xx:xx  such as aa:00:cc:dd:ee:fe"
}

rate_idx=226
mac_addr=aa:00:cc:dd:ee:fe

path_env="/sys/kernel/debug/ieee80211/phy1/cls_wifi/stations"
if [ $# -eq 1 ];then 
rate_idx=$1
elif [ $# -eq 2 ];then 
rate_idx=$1
mac_addr=$2
elif [ $# -eq 0 ];then 
echo "default rate_idx = $rate_idx"
else
usage()
echo ""
echo "=========== $script_name done ==========="
echo ""
exit 1
fi

echo "echo $rate_idx >$path_env/$mac_addr/rc/fixed_rate_idx"
if [ -d $path_env/$mac_addr ]; then
	echo $rate_idx > $path_env/$mac_addr/rc/fixed_rate_idx
else
mac_addr=20:aa:cc:33:55:20
echo "[try]echo $rate_idx >$path_env/$mac_addr/rc/fixed_rate_idx"
if [ -d $path_env/$mac_addr  ]; then
	echo "echo $rate_idx > $path_env/$mac_addr/rc/fixed_rate_idx"
	echo $rate_idx > $path_env/$mac_addr/rc/fixed_rate_idx
fi
fi









echo ""
echo "=========== $script_name done ==========="
echo ""
