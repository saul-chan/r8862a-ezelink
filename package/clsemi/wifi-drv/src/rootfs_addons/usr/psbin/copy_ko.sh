#!/bin/bash

set -e

cd /lib/modules/*/
kernel_path=$(pwd)
echo "ko_path: "$kernel_path

check_md5="false"

if [ $# -eq 1 ]; then
check_md5=$1
fi

if [ ! -f $kernel_path/extra/trans.ko ]; then
if [ -f /lib/firmware/trans.ko ]; then
cp -r /lib/firmware/trans.ko $kernel_path/extra/trans.ko
fi
fi

if [ $check_md5 = "true" ];then
	if [ -f /lib/firmware/trans.ko ]; then
	md5sum /lib/firmware/trans.ko
	fi
	if [ -f $kernel_path/extra/trans.ko ]; then
	md5sum $kernel_path/extra/trans.ko
	fi
fi


if [ ! -f $kernel_path/extra/fullmac/clsm_wifi.ko ]; then
if [ -f /lib/firmware/clsm_wifi.ko ]; then
cp -r /lib/firmware/clsm_wifi.ko $kernel_path/extra/fullmac/clsm_wifi.ko
fi
fi

if [ $check_md5 = "true" ];then
	if [ -f /lib/firmware/clsm_wifi.ko ]; then
	md5sum /lib/firmware/clsm_wifi.ko
	fi
	if [ -f $kernel_path/extra/fullmac/clsm_wifi.ko ]; then
	md5sum $kernel_path/extra/fullmac/clsm_wifi.ko
	fi
fi

if [ ! -f $kernel_path/extra/fullmac/clsm_wifi_soc.ko ]; then
if [ -f /lib/firmware/clsm_wifi_soc.ko ]; then
cp -r /lib/firmware/clsm_wifi_soc.ko $kernel_path/extra/fullmac/clsm_wifi_soc.ko
fi
fi

if [ $check_md5 = "true" ];then
	if [ -f /lib/firmware/clsm_wifi_soc.ko ]; then
	md5sum /lib/firmware/clsm_wifi_soc.ko
	fi
	if [ -f $kernel_path/extra/fullmac/clsm_wifi_soc.ko ]; then
	md5sum $kernel_path/extra/fullmac/clsm_wifi_soc.ko
	fi
fi


if [ ! -f $kernel_path/extra/dubhe1000_eth.ko ]; then
if [ -f /lib/firmware/dubhe1000_eth.ko ]; then
cp -r /lib/firmware/dubhe1000_eth.ko $kernel_path/extra/dubhe1000_eth.ko
fi
fi

if [ $check_md5 = "true" ];then
	if [ -f /lib/firmware/dubhe1000_eth.ko ]; then
	md5sum /lib/firmware/dubhe1000_eth.ko
	fi
	if [ -f $kernel_path/extra/dubhe1000_eth.ko ]; then
	md5sum $kernel_path/extra/dubhe1000_eth.ko
	fi
fi

