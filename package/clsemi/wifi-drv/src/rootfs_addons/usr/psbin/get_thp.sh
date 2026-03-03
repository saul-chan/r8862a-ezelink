#!/bin/bash

#set -e

#root_path=$(cd $(dirname $0);pwd)
script_name=$0

echo ""
#echo "root_path: "${root_path}
#cd ${root_path}



#/sys/kernel/debug/ieee80211/phy1/cls_wifi/stations/aa:00:cc:dd:ee:fe/rc/fixed_rate_idx

path_env1="/sys/kernel/debug/ieee80211/phy1/cls_wifi/stations/aa:00:cc:dd:ee:fe"
path_env2="/sys/kernel/debug/ieee80211/phy1/cls_wifi/stations/20:aa:cc:33:55:20"
if [ -d $path_env1 ]; then
	echo "/sys/kernel/debug/ieee80211/phy1/cls_wifi/stats"
	cat /sys/kernel/debug/ieee80211/phy1/cls_wifi/stats
	echo ""
	echo "$path_env1/rc/rx_rate"
	cat $path_env1/rc/rx_rate
	echo ""
	echo "$path_env1/rc/tx_rate"
	cat $path_env1/rc/tx_rate
	echo ""
	echo "$path_env1/rc/stats"
	cat $path_env1/rc/stats
	echo ""
	echo "/sys/kernel/debug/ieee80211/phy1/cls_wifi/dbg_cnt"
	cat /sys/kernel/debug/ieee80211/phy1/cls_wifi/dbg_cnt
	echo ""
	ifconfig
elif [ -d $path_env2 ]; then
	echo "/sys/kernel/debug/ieee80211/phy1/cls_wifi/stats"
	cat /sys/kernel/debug/ieee80211/phy1/cls_wifi/stats
	echo ""
	echo "$path_env2/rc/rx_rate"
	cat $path_env2/rc/rx_rate
	echo ""
	echo "$path_env2/rc/tx_rate"
	cat $path_env2/rc/tx_rate
	echo ""
	echo "$path_env2/rc/stats"
	cat $path_env2/rc/stats
	echo ""
	echo "/sys/kernel/debug/ieee80211/phy1/cls_wifi/dbg_cnt"
	cat /sys/kernel/debug/ieee80211/phy1/cls_wifi/dbg_cnt
	echo ""
	ifconfig
else
	path_env2="/sys/kernel/debug/ieee80211/phy1/cls_wifi/stations/*"
	echo "/sys/kernel/debug/ieee80211/phy1/cls_wifi/stats"
	cat /sys/kernel/debug/ieee80211/phy1/cls_wifi/stats
	echo ""
	echo "$path_env2/rc/rx_rate"
	cat $path_env2/rc/rx_rate
	echo ""
	echo "$path_env2/rc/tx_rate"
	cat $path_env2/rc/tx_rate
	echo ""
	echo "$path_env2/rc/stats"
	cat $path_env2/rc/stats
	echo ""
	echo "/sys/kernel/debug/ieee80211/phy1/cls_wifi/dbg_cnt"
	cat /sys/kernel/debug/ieee80211/phy1/cls_wifi/dbg_cnt
	echo ""
	ifconfig
fi

echo dump switch > /sys/kernel/debug/dubhe1000/command

echo ""
echo "=========== $script_name done ==========="
echo ""
