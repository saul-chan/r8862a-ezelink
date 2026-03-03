#!/bin/bash

set -e

#root_path=$(cd $(dirname $0);pwd)
script_name=$0

echo ""
#echo "root_path: "${root_path}
#cd ${root_path}

if [ $# -eq 1 ]; then
	if [ "$1" = "wifi1" ];then
		cp -rf /tmp/iram /lib/firmware/wifi1.bin
		md5sum /lib/firmware/wifi1.bin
		exit 0
	fi

	if [ "$1" = "wifi0" ];then
		cp -rf /tmp/iram /lib/firmware/wifi0.bin
		md5sum /lib/firmware/wifi0.bin
		exit 0
	fi

	if [ "$1" = "clsm_wifi" ]; then
		cp -rf /tmp/iram /lib/modules/*/extra/fullmac/clsm_wifi.ko
		md5sum /lib/modules/*/extra/fullmac/clsm_wifi.ko
		exit 0
	fi

	if [ "$1" = "clsm_wifi_soc" ]; then
		cp -rf /tmp/iram /lib/modules/*/extra/fullmac/clsm_wifi_soc.ko
		md5sum /lib/modules/*/extra/fullmac/clsm_wifi_soc.ko
		exit 0
	fi
fi

cd /tmp
if [ -f /tmp/iram ]; then
	tar -xf /tmp/iram
fi

if [ -f /tmp/wifi0.bin ]; then
echo "cp -rf /tmp/wifi0.bin /lib/firmware/wifi0.bin"
cp -rf /tmp/wifi0.bin /lib/firmware/wifi0.bin
#md5sum /tmp/wifi0.bin
md5sum /lib/firmware/wifi0.bin
rm -rf /tmp/wifi0.bin
fi

if [ -f /tmp/wifi1.bin ]; then
echo "cp -rf /tmp/wifi1.bin /lib/firmware/wifi1.bin"
cp -rf /tmp/wifi1.bin /lib/firmware/wifi1.bin
#md5sum /tmp/wifi1.bin
md5sum /lib/firmware/wifi1.bin
rm -rf /tmp/wifi1.bin
fi

if [ -f /tmp/clsm_wifi.ko ]; then
echo "cp -rf clsm_wifi.ko /lib/modules/*/extra/fullmac/clsm_wifi.ko"
cp -rf /tmp/clsm_wifi.ko /lib/modules/*/extra/fullmac/clsm_wifi.ko
#md5sum /tmp/clsm_wifi.ko
md5sum /lib/modules/*/extra/fullmac/clsm_wifi.ko
rm -rf /tmp/clsm_wifi.ko
fi

if [ -f /tmp/clsm_wifi_soc.ko ]; then
echo "cp -rf clsm_wifi_soc.ko /lib/modules/*/extra/fullmac/clsm_wifi_soc.ko"
cp -rf /tmp/clsm_wifi_soc.ko /lib/modules/*/extra/fullmac/clsm_wifi_soc.ko
#md5sum /tmp/clsm_wifi_soc.ko
md5sum /lib/modules/*/extra/fullmac/clsm_wifi_soc.ko
rm -rf /tmp/clsm_wifi_soc.ko
fi

if [ -f /tmp/dubhe1000_eth.ko ]; then
echo "cp -rf /tmp/dubhe1000_eth.ko /lib/modules/*/extra/dubhe1000_eth.ko"
cp -rf /tmp/dubhe1000_eth.ko /lib/modules/*/extra/dubhe1000_eth.ko
#md5sum /tmp/dubhe1000_eth.ko
md5sum /lib/modules/*/extra/dubhe1000_eth.ko
rm -rf /tmp/dubhe1000_eth.ko
fi

echo ""
echo "=========== $script_name done ==========="
echo ""
