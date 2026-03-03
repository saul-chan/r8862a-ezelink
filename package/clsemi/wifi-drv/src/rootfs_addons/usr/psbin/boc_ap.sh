#!/bin/bash

#set -e

root_path=$(cd $(dirname $0);pwd)
script_name=$0

echo ""
echo "root_path: "${root_path}
cd ${root_path}



sh op_wpa.sh set_mac -i wlan0 -m 22:33:C0:55:22:22
sh op_wpa.sh set_mac -i wlan1 -m 22:33:44:55:22:22
sh op_wpa.sh set_mac -i eth0 -m A0:C0:AB:50:CC:22  
sh op_wpa.sh set_mac -i eth1 -m A0:C0:AB:50:C5:22  





echo ""
echo "=========== $script_name done ==========="
echo ""
