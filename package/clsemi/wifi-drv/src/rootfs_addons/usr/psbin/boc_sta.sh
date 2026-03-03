#!/bin/bash

#set -e

root_path=$(cd $(dirname $0);pwd)
script_name=$0

echo ""
echo "root_path: "${root_path}
cd ${root_path}


#modprobe dubhe1000_eth
#modprobe clsm_wifi_soc

sh op_wpa.sh set_mac -i wlan0 -m aa:00:11:dd:ee:22
sh op_wpa.sh set_mac -i wlan1 -m aa:00:11:dd:ee:ff
sh op_wpa.sh set_mac -i eth0 -m A0:C0:AB:39:CC:F0  
sh op_wpa.sh set_mac -i eth1 -m A0:C0:AB:39:CC:F2  


ifconfig wlan1 up
exit
sh op_wpa.sh start -i wlan1
sleep 2
ifconfig wlan1 192.168.3.110
sh op_wpa.sh connect open -p open -i wlan1

ifconfig eth0 up
ifconfig eth0 192.168.5.110

sh ipv6_disable.sh

arp -s 192.168.5.13 00:22:22:33:44:22
arp -s 192.168.3.1  22:33:44:55:22:22  

echo ""
echo "=========== $script_name done ==========="
echo ""
