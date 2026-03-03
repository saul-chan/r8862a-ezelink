#!/bin/bash

usage(){
	echo "command example: ./irf_set_reg 0x40000 0x1"
}

if [ $# -ne 2 ]; then
	usage
else
	echo "$1 $2" > /sys/kernel/debug/ieee80211/phy0/cls_wifi/irf/irf_set_reg
fi
