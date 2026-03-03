#!/bin/bash

set -e



echo 8 > /proc/sysrq-trigger

if [ ! -d /sys/kernel/debug/ieee80211/phy0/cls_wifi/irf ] && [ ! -d /sys/kernel/debug/ieee80211/phy1/cls_wifi/irf ];then
sh mount_debugfs.sh
fi

if [ -d /sys/kernel/debug/ieee80211/phy0/cls_wifi/irf ];then
ls /sys/kernel/debug/ieee80211/phy0/cls_wifi/irf
cd /sys/kernel/debug/ieee80211/phy0/cls_wifi/irf
# CFR pregain
echo 0x4c81000c > irf_get_reg
fi

if [ -d /sys/kernel/debug/ieee80211/phy1/cls_wifi/irf ];then
ls /sys/kernel/debug/ieee80211/phy1/cls_wifi/irf
cd /sys/kernel/debug/ieee80211/phy1/cls_wifi/irf
# CFR pregain
echo 0x4c81000c > irf_get_reg
fi

ls /root/irf
