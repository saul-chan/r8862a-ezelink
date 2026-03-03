#!/bin/bash

usage(){
	echo "irf_send_stop node_mask band CH src_position"
	echo "node_mask   : bit0-node0,bit1-node1...bit3-node3"
	echo "band        : 0-2G, 1-5G"
	echo "channel     : CH 0~1"
	echo "src_node    : -1-not SRC node, 0-hbf0, 1-hbf1, 2-src_st2"
}

if [ $# -ne 4 ]; then
	usage
else
	echo "$1 $2 $3 $4 0" > /sys/kernel/debug/ieee80211/phy0/cls_wifi/irf/irf_send_stop
fi
