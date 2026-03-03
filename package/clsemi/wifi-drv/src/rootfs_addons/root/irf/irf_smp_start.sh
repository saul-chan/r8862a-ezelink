#!/bin/bash

usage(){
	echo "irf_smp_start node_mask [timeout]"
	echo "node_mask : bit0-node0,bit1-node1...bit3-node3;When bit set,start sample corresponding node!"
	echo "timeout : It's optional. Set the smp timeout value in unit of ms."
	echo "wlan    : 0 or 1"
}

wlan_type=0
timeout=0

if [ $# -eq 3 ]; then
	timeout=$2
	wlan_type=$3
fi



if [ $# -eq 3 ]; then
	echo "/sys/kernel/debug/ieee80211/phy$wlan_type/cls_wifi/irf/irf_smp_start"
	echo "timeout = $timeout"
	echo "$1 $timeout" > /sys/kernel/debug/ieee80211/phy$wlan_type/cls_wifi/irf/irf_smp_start
else
	usage;
fi
