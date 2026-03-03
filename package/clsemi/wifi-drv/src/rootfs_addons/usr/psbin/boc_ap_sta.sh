#!/bin/bash



if [ "$1" = "ap_br" ];then 
	brctl addbr br1
	brctl addif br1 wlan1
	brctl addif br1 eth0
	brctl show
	ifconfig br1 up
	ifconfig br1 192.168.3.1
elif [ "$1" = "ap_eth" ];then 
	ifconfig eth0 up
	arp -s 192.168.3.13 aa:c0:ab:39:11:00
	arp -s 192.168.3.110 20:AA:CC:33:55:20
	arp -v
	route add -net 192.168.5.0 netmask 255.255.255.0 dev wlan1
	route

	echo 1 > /proc/sys/net/ipv6/conf/all/disable_ipv6
elif [ "$1" = "ap_eth_log" ];then 
	echo msg level 0 7  > /sys/kernel/debug/dubhe1000/command
fi


if [ "$1" = "sta_cn" ];then 
	ifconfig wlan1 192.168.3.110
	iw dev wlan1 set power_save off
	iw dev wlan1 get power_save 
elif [ "$1" = "sta_eth" ];then 
	ifconfig eth0 up
	ifconfig eth0 192.168.5.110

	echo 1 > /proc/sys/net/ipv4/ip_forward

	arp -s 192.168.3.13 aa:c0:ab:39:11:00
	arp -s 192.168.5.13 00:22:22:33:44:22
	arp -s 192.168.3.1  20:11:CC:19:11:20
	arp -v

	echo 1 > /proc/sys/net/ipv6/conf/all/disable_ipv6
elif [ "$1" = "sta_eth_log" ];then 
	echo msg level 0 7  > /sys/kernel/debug/dubhe1000/command
elif [ "$1" = "sta_fp" ];then 
	echo eth0 > /sys/kernel/debug/ieee80211/phy1/cls_wifi/fp
fi
