#!/bin/bash

usage(){
	echo "command example: ./irf_hw_init <bw> <bitmap>"
	echo "bw: bandwidth from 0 to 3 (0 - 20MHz, 1 - 40MHz, 2 - 80MHz, 3 - 160MHz)"
	echo "bitmap: 1 or 255"
}

if [ $# -ne 2 ]; then
	usage
else
	echo "$1 $2" > /sys/kernel/debug/ieee80211/phy0/cls_wifi/irf/irf_hw_init
fi
