#!/bin/bash

set -e


if [ -f /proc/sys/net/ipv6/conf/wlan0/disable_ipv6 ]; then
echo 1 > /proc/sys/net/ipv6/conf/wlan0/disable_ipv6
echo "wlan0 disable_ipv6"
cat /proc/sys/net/ipv6/conf/wlan0/disable_ipv6
fi

if [ -f /proc/sys/net/ipv6/conf/wlan1/disable_ipv6 ]; then
echo 1 > /proc/sys/net/ipv6/conf/wlan1/disable_ipv6
echo "wlan1 disable_ipv6"
cat /proc/sys/net/ipv6/conf/wlan1/disable_ipv6
fi

if [ -f /proc/sys/net/ipv6/conf/eth0/disable_ipv6 ]; then
echo 1 > /proc/sys/net/ipv6/conf/eth0/disable_ipv6
echo "eth0 disable_ipv6"
cat /proc/sys/net/ipv6/conf/eth0/disable_ipv6
fi

if [ -f /proc/sys/net/ipv6/conf/eth1/disable_ipv6 ]; then
echo 1 > /proc/sys/net/ipv6/conf/eth1/disable_ipv6
echo "eth1 disable_ipv6"
cat /proc/sys/net/ipv6/conf/eth1/disable_ipv6
fi

if [ -f /proc/sys/net/ipv6/conf/br0/disable_ipv6 ]; then
echo "br0 disable_ipv6"
echo 1 > /proc/sys/net/ipv6/conf/br0/disable_ipv6
cat /proc/sys/net/ipv6/conf/br0/disable_ipv6
fi

if [ -f /proc/sys/net/ipv6/conf/br1/disable_ipv6 ]; then
echo "br1 disable_ipv6"
echo 1 > /proc/sys/net/ipv6/conf/br1/disable_ipv6
cat /proc/sys/net/ipv6/conf/br1/disable_ipv6
fi


if [ -f /proc/sys/net/ipv6/conf/wlan0.sta1/disable_ipv6 ]; then
echo "wlan0.sta1 disable_ipv6"
echo 1 > /proc/sys/net/ipv6/conf/wlan0.sta1/disable_ipv6
cat /proc/sys/net/ipv6/conf/wlan0.sta1/disable_ipv6
fi



if [ -f /proc/sys/net/ipv6/conf/wlan1.sta1/disable_ipv6 ]; then
echo "wlan1.sta1 disable_ipv6"
echo 1 > /proc/sys/net/ipv6/conf/wlan1.sta1/disable_ipv6
cat /proc/sys/net/ipv6/conf/wlan1.sta1/disable_ipv6
fi

if [ -f /proc/sys/net/ipv6/conf/sit0/disable_ipv6 ]; then
echo 1 > /proc/sys/net/ipv6/conf/sit0/disable_ipv6
echo "sit0 disable_ipv6"
cat /proc/sys/net/ipv6/conf/sit0/disable_ipv6
fi

if [ -f /proc/sys/net/ipv6/conf/all/disable_ipv6 ]; then
echo 1 > /proc/sys/net/ipv6/conf/all/disable_ipv6
echo "all disable_ipv6"
cat /proc/sys/net/ipv6/conf/all/disable_ipv6
fi
