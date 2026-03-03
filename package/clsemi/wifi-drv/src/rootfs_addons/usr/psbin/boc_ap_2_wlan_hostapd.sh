cd /usr/psbin;

sh hostapd_env.sh;

insmod /lib/firmware/clsm_wifi.ko bands_enable=2;
insmod /lib/firmware/clsm_wifi_soc.ko;

ifconfig wlan0 hw ether d0:44:33:19:11:00;

# fix bw
echo "fix bw!!!"
ifconfig wlan0 up;
sh dht_msg.sh 0 18;
ifconfig wlan0 down;

# dma & eco reg
echo "set dma and eco reg!!!"
devmem 0x4AB0860C 32 0x30080100;
devmem 0x90444608 32 0x00000003;

# log level
#sh dht_msg.sh 0 50;
#sh dht_msg.sh 0 57;

# encrypt
#sed -i 's/wpa=0/wpa=2/g' /usr/psbin/hostapd_he160.conf
#sed -i 's/vht_capab=\[MAX-AMSDU-3839\]/vht_capab=\[MAX-A-MPDU-LEN-EXP7\]\[MAX-MPDU-7991\]\[RXLDPC\]\[SHORT-GI-80\]\[SHORT-GI-160\]\[RX-STBC-1\]/g' /usr/psbin/hostapd_he160.conf
#sed -i 's/ht_capab=\[HT40+\]\[SHORT-GI-20\]\[SHORT-GI-40\]\[DSSS_CCK-40\]/ht_capab=\[HT40+\]\[SHORT-GI-20\]\[SHORT-GI-40\]\[DSSS_CCK-40\]\[MAX-AMSDU-7935\]/g' /usr/psbin/hostapd_he160.conf

sed -i 's/interface=wlan1/interface=wlan0/g' /usr/psbin/hostapd_he160.conf;
sh start_hostapd.sh he160;

echo "wait for hostapd!!!"
