hostapd_cli status;

sleep 0.008;

#config DIF reg
echo "config DIF reg!!!"
echo -n "41380E001000040001000000" > /sys/kernel/debug/ieee80211/phy0/cls_wifi/msg;

sleep 0.004;

#disable WPU send beacon frame
echo "disable WPU send beacon frame!!!"
echo -n "17380E001000040001000000" > /sys/kernel/debug/ieee80211/phy0/cls_wifi/msg;
