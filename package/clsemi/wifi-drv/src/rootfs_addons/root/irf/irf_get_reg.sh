#!/bin/bash

usage(){
	echo "command example: ./irf_get_reg.sh 0x40000"
}

if [ $# -ne 1 ]; then
	usage
else
	echo "$1" > /sys/kernel/debug/ieee80211/phy0/cls_wifi/irf/irf_get_reg
fi
