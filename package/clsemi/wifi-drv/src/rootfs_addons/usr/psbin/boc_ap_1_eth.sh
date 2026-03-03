cd /usr/psbin;

insmod /lib/firmware/cls_npe.ko

echo "sh mount_debugfs.sh!!!"
sh /usr/psbin/mount_debugfs.sh;

echo "disable ipv6!!!"
sh /usr/psbin/ipv6_disable.sh;

###debug log
#echo msg level 0 1007  > /sys/kernel/debug/cls_npe/command
#echo msg level 0 7  > /sys/kernel/debug/cls_npe/command

# AP chip
ifconfig eth0 hw ether d0:44:33:50:00:20;
ifconfig eth1 hw ether D0:44:33:50:01:20;

ifconfig eth0 up;
ifconfig eth1 up;

ifconfig eth0 192.168.52.110;
ifconfig eth1 192.168.52.111;

brctl addbr br0;

brctl addif br0 eth0;
brctl addif br0 eth1;

ifconfig br0 up;
ifconfig br0 192.168.52.1;

echo "arp!!!"
arp -s 192.168.52.5 D0:44:33:A0:A0:20;
arp -s 192.168.52.50 D0:44:33:A0:A1:20;
arp -v;


### accept IPV4 pkt
npecmd switch l3 ingress_router 0 --acceptIPv4=1;
## default route
npecmd switch l3 routing_default 0 --sendToCpu=1;

echo "eth done!!!"
