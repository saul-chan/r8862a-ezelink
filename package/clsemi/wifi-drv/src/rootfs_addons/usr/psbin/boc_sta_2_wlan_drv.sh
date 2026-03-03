cd /usr/psbin;

sh wpa_env.sh;

insmod /lib/firmware/clsm_wifi.ko bands_enable=2;
insmod /lib/firmware/clsm_wifi_soc.ko;

ifconfig wlan0 hw ether d0:44:33:19:11:20;

# fix bw
echo "fix bw!!!"
ifconfig wlan0 up;
sh dht_msg.sh 0 18;

# dma & eco reg
echo "set dma and eco reg!!!"
devmem 0x4AB0860C 32 0x30080100;
devmem 0x90444608 32 0x00000003;

#config DIF reg
echo "config DIF reg!!!"
echo -n "41380E001000040001000000" > /sys/kernel/debug/ieee80211/phy0/cls_wifi/msg;

sleep 0.004;

#force to send probe req
echo "force to send probe req!!!"
echo -n "21380E001000040000000000"  > /sys/kernel/debug/ieee80211/phy0/cls_wifi/msg;

#scan
echo "scan before AP enabled!!!"
iw dev wlan0 scan freq 5500;

