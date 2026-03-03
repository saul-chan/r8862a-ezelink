#config DIF reg again
echo "config DIF reg again!!!"
echo -n "41380E001000040001000000" > /sys/kernel/debug/ieee80211/phy0/cls_wifi/msg;

sleep 0.004;

#scan again
echo "scan after AP enabled!!!"
iw dev wlan0 scan freq 5500;
