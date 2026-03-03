fast_path_flag="false"


#iw dev wlan0 info
iw dev wlan0 link;

# encrypted connect
#wpa_cli status -p /var/run/wpa_supplicant_wlan0/

sh /usr/psbin/ipv6_disable.sh;

ifconfig wlan0 192.168.52.2;

ifconfig lo up;

arp -s 192.168.52.1 D0:44:33:19:11:00;
arp -v;

#echo 793 > /sys/kernel/debug/ieee80211/phy0/cls_wifi/stations/d0:44:33:19:11:00/rc/fixed_rate_idx;
echo 1069 > /sys/kernel/debug/ieee80211/phy0/cls_wifi/stations/d0:44:33:19:11:00/rc/fixed_rate_idx;

if [ fast_path_flag = "true" ]; then
# DL
echo wlan0 eth0 > /sys/kernel/debug/ieee80211/phy0/cls_wifi/fp;
#echo wlan0 eth0 bypass_qdisc_en  > /sys/kernel/debug/ieee80211/phy0/cls_wifi/fp;
#echo wlan0 eth0 bypass_qdisc_dis  > /sys/kernel/debug/ieee80211/phy0/cls_wifi/fp;

# UL
echo "fp eth0 wlan0" > /sys/kernel/debug/cls_npe/command;
#echo "fp eth0 wlan0 bypass_qdisc_en" > /sys/kernel/debug/cls_npe/command;
#echo "fp eth0 wlan0 bypass_qdisc_dis" > /sys/kernel/debug/cls_npe/command;
fi


###debug log
#echo msg level 0 1007  > /sys/kernel/debug/cls_npe/command
echo msg level 0 7  > /sys/kernel/debug/cls_npe/command;

#hardware traffic limit (encrypted connect)
#devmem 0x4AC080D4 32 0x1EF

##FWD TX bind to core1 (DL)
echo 2 > /proc/irq/21/smp_affinity;
# unbind
#echo 3 > /proc/irq/21/smp_affinity;

###tx_wq and txfreeskb_wq to core1 (UL)
echo 2 > /sys/module/clsm_wifi/parameters/tx_wq;
echo 2 > /sys/module/clsm_wifi/parameters/txfree_wq;

###close cts2self
echo -n "44380E001000040001000000" > /sys/kernel/debug/ieee80211/phy0/cls_wifi/msg;
###enable cts2self
#echo -n "44380E001000040000000000" > /sys/kernel/debug/ieee80211/phy0/cls_wifi/msg;


#disable drv/fw pkt log
#echo 0 > /sys/module/clsm_wifi/parameters/process_logs;
#sh dht_msg.sh 0 57;

#enable drv/fw log
#sh dht_msg.sh 0 50;
#sh dht_msg.sh 0 37;
#echo 253 > /sys/module/clsm_wifi/parameters/process_logs;
