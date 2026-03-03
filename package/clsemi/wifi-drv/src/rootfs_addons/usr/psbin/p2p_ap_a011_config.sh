four_addr=1
ip_addr="192.168.11.1"
board_num="A011"
skip_eth_insmod=0
skip_wifi_insmod=0
skip_fwt_insmod=0
bands=2
wifi_intface=wlan0
phy_intface=phy0
# parse input parameters
while test $# -gt 0
do
	case "$1" in
		4addr=*)
			four_addr=${1#4addr=}
			;;
		ip_addr=*)
			ip_addr=${1#ip_addr=}
			;;
		board_num=*)
			board_num=${1#board_num=}
			;;
		bands=*)
			bands=${1#bands=}
			;;
		wifi_intface=*)
			wifi_intface=${1#wifi_intface=}
			;;
		phy_intface=*)
			phy_intface=${1#phy_intface=}
			;;
		skip_eth_insmod=*)
			skip_eth_insmod=${1#skip_eth_insmod=}
			;;
		skip_wifi_insmod=*)
			skip_wifi_insmod=${1#skip_wifi_insmod=}
			;;
		skip_fwt_insmod=*)
			skip_fwt_insmod=${1#skip_fwt_insmod=}
			;;
		-h)
			exit 0
			;;
	esac
	shift
done

set -x

cd /usr/psbin
sh mount_debugfs.sh

if [ $skip_fwt_insmod -eq 0 ]; then
insmod /lib/firmware/fwt_drv.ko
sleep 5
fi

echo "============================ FWT done ======================================"


if [ $skip_eth_insmod -eq 0 ]; then
insmod /lib/firmware/cls_npe.ko verbose=1
sleep 5

ifconfig eth0 hw ether d0:44:33:50:00:00
ifconfig eth1 hw ether d0:44:33:50:00:10
ifconfig eth2 hw ether D0:44:33:50:00:20
ifconfig eth3 hw ether d0:44:33:50:00:30
ifconfig eth4 hw ether D0:44:33:50:00:40

sleep 1
fi

brctl addbr br0
brctl addif br0 eth4
brctl addif br0 eth1
ifconfig eth4 up
ifconfig eth1 up
ifconfig br0 $ip_addr

brctl setageing br0 500000


echo "============================ ETH done ======================================"
sleep 10

if [ $skip_wifi_insmod -eq 0 ]; then
	insmod /lib/firmware/clsm_wifi.ko bands_enable=$bands ps_on=0 boot_cali_enable=0
	sleep 1
	insmod /lib/firmware/clsm_wifi_soc.ko;
	sleep 2
fi

ifconfig $wifi_intface down
ifconfig $wifi_intface hw ether d0:44:33:19:11:20

echo "============================ WIFI-drv done ======================================"

ifconfig $wifi_intface up;

echo "****** fix ch/freq/bw ******"
ifconfig $wifi_intface up
sleep 1
echo -n "05380E00100010000F000000030000003C148214" > /sys/kernel/debug/ieee80211/$phy_intface/cls_wifi/msg
sleep 1
ifconfig $wifi_intface down

echo "****** disable ipv6 ******"
sh /usr/psbin/ipv6_disable.sh

echo > /sys/kernel/debug/ieee80211/$phy_intface/cls_wifi/fwlog

if [ "$wifi_intface" = "wlan0" ]; then
sed -i 's/interface=wlan1/interface=wlan0/g' /usr/psbin/hostapd_he160.conf;
else
sed -i 's/interface=wlan0/interface=wlan1/g' /usr/psbin/hostapd_he160.conf;
fi

echo "****** start hostapd ******"
if [ $four_addr -gt 0 ]
then
	sed -i 's/#wds_sta=1/wds_sta=1/g' /usr/psbin/hostapd_he160.conf
	sed -i 's/#wds_bridge=br0/wds_bridge=br0/g' /usr/psbin/hostapd_he160.conf
fi
sync
#sed -i 's/\/usr\/psbin\/hostapd_he160.conf -d/\/usr\/psbin\/hostapd_he160.conf -ddd/g' start_hostapd.sh;
sh start_hostapd.sh he160;
sleep 10
if [ "$wifi_intface" = "wlan0" ]; then
	hostapd_state=`hostapd_cli -i wlan0 status | head -n 1`
else
	hostapd_state=`hostapd_cli -i wlan1 status | head -n 1`
fi
while [ "$hostapd_state" != "state=ENABLED" ]
do
	sleep 5
	if [ "$wifi_intface" = "wlan0" ]; then
		hostapd_state=`hostapd_cli -i wlan0 status | head -n 1`
	else
		hostapd_state=`hostapd_cli -i wlan1 status | head -n 1`
	fi
done

sh /usr/psbin/ipv6_disable.sh;

brctl addif br0 $wifi_intface
sleep 1
iw dev $wifi_intface set txpower fixed 1300

devmem 0x4010b304 32 0x42004200
sleep 0.01

#write 1
devmem 0x4010b390 32 0x00011103
sleep 0.01
devmem 0x4010b390 32 0x00010103

###if [ "$board_num" = "A008" ]; then
### RX环境频偏配置（A-8‘板子）
###devmem 0x90422034 32 0x0F250000
##else
#### A'-011的频偏配置(TX方向)：
##devmem 0x90422034 32 0x0f290000
##fi

echo "============================ AP done ======================================"


