iw dev wlan0 set power_save off;
iw dev wlan0 get power_save;

#encrypted connect
#sh op_wpa.sh start -i wlan0
#sh op_wpa.sh connect open -p 123456789 -i wlan0

#connect
echo "start connect!!!"
iw dev wlan0 connect open;
