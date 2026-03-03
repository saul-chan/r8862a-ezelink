#!/bin/bash

set -e

root_path=$(cd $(dirname $0);pwd)
#echo "root_path: "${root_path}
cd ${root_path}

conf_path=/lib/firmware/clsm_wifi_settings.ini
if [ ! -f /lib/firmware/clsm_wifi_settings.ini ];then
conf_path=./sta_clsm_wifi_settings.ini
if [ ! -f $conf_path ];then
touch $conf_path
echo -n "MAC_ADDR=00:AA:cc:33:55:00" > $conf_path
fi
fi

if [ -f $conf_path ]; then
echo "set wlan0 | wlan1 mac"
sed -i '/MAC_ADDR1=/d'  $conf_path
sed -i "/MAC_ADDR=/c\MAC_ADDR=00:AA:cc:33:55:00"  $conf_path
sed -i "/MAC_ADDR=/a\MAC_ADDR1=20:AA:cc:33:55:20"  $conf_path
fi


