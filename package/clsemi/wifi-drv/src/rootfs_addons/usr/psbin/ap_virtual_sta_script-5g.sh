cd /usr/psbin
sh /usr/psbin/copy_ko.sh

radio=0




echo "============= modprobe cls_wifi_fdrv done ============="
insmod /lib/firmware/clsm_wifi.ko bands_enable=2
sleep 2
insmod /lib/firmware/clsm_wifi_soc.ko
sleep 5
echo "============= modprobe cls_wifi_fdrv done ============="

echo "sh mount_debugfs.sh"
sh mount_debugfs.sh

ifconfig wlan0 hw ether d0:44:33:19:11:20

if [ -f /usr/psbin/ipv6_disable.sh ]; then
sh /usr/psbin/ipv6_disable.sh
fi
sleep 2

ifconfig wlan0 up
sleep 5
echo "bw160 5250 5180"
echo -n "05380E00100010000F000000030000003C148214" > /sys/kernel/debug/ieee80211/phy$radio/cls_wifi/msg
sleep 5

sed -i 's/ssid=d2k/ssid=Clourney-5G/g' /usr/psbin/hostapd_he160.conf

if [ $radio -eq 0 ]; then
sed -i 's/interface=wlan1/interface=wlan0/g' /usr/psbin/hostapd_he160.conf
fi

echo "sh start_hostapd.sh he160"
sh start_hostapd.sh he160


sleep 20
echo "wait hostapd dfs done"
sleep 20
echo "wait hostapd dfs done"
sleep 20
echo "wait hostapd dfs done"
hostapd_cli status
sleep 10

echo "sh dht_msg.sh 0 0"
sh dht_msg.sh 0 0
sleep 2


ifconfig wlan0 192.168.52.1
ifconfig wlan0 up
ifconfig lo up
sleep 1


echo "disable WPU send beacon frame!!!!!!!!!!!!!"
echo -n "17380E001000040001000000" > /sys/kernel/debug/ieee80211/phy$radio/cls_wifi/msg

sleep 1

###
echo ""
echo "============= $0 done ============="
echo ""

