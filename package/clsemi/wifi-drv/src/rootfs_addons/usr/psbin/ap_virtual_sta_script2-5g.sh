
radio=0

cd /usr/psbin
#only for specific mac addr
echo -n "b0003a01d04433191120665544332211d044331911201000000001000000" > /usr/psbin/auth_1;
echo -n "00003c00d04433191120665544332211d04433191120902221150500000b436c6f75726e65792d354701088c129824b048606c2102f41424022408460570000000007f0b04004a02004040c00141203b0d817374757677787c7d7e7f8082dd070050f2020001002d1aef091fffff00000100000000002c010100000000000000000000bf0cf6098903f2ff1806f2ff1806c70113ff25230100020200001ee02bff00c9cf84d3b000fafffafffafffafffefffeff791cc7711cc771" > /usr/psbin/assoc_he_1;


cp /usr/psbin/auth_1 /tmp/auth
cp /usr/psbin/assoc_he_1 /tmp/assoc


/usr/psbin/hostapd_cli auth
#等待结束
sleep 5
/usr/psbin/hostapd_cli all_sta
sleep 5
/usr/psbin/hostapd_cli assoc
sleep 15
/usr/psbin/hostapd_cli all_sta
sleep 10

echo ""
echo "==========================================="
echo "!!!set arp"
arp -s 192.168.52.5 66:55:44:33:22:11
arp -v
echo "==========================================="
echo ""
sleep 2

if [ ! -d /sys/kernel/debug/ieee80211/phy$radio/cls_wifi/stations ]; then
mount -t debugfs none /sys/kernel/debug
fi

echo "!!!set fix rate"
if [ -f /sys/kernel/debug/ieee80211/phy$radio/cls_wifi/stations/66:55:44:33:22:11/rc/fixed_rate_idx ]; then
	echo 793 > /sys/kernel/debug/ieee80211/phy$radio/cls_wifi/stations/66:55:44:33:22:11/rc/fixed_rate_idx
	sleep 2
else
	echo "set fix rate failed!!!!!!!"
fi


echo ""
echo "================================================"
#brctl show
echo "================================================"
echo ""
sleep 4

ifconfig
sleep 2


if [ -f /usr/psbin/disable_ipv6.sh ]; then
sh  /usr/psbin/disable_ipv6.sh
fi


###
echo ""
echo "============= $0 done ============="
echo ""

