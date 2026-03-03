#!/bin/sh

if [ $# != 3 ]; then
	echo "Usage: $(basename $0) <mode:ap|sta> <radio:0|1> <band:2G|5G>"
	exit
fi

mode=$1
radio=$2
band=$3

band=$(echo $band | tr 'g' 'G')

lab_mode_value="$(fw_printenv -n lab_mode 2>/dev/null)"
vip_node_enable=0
vip_node_5G_tx=0
vip_node_5G_rx=1
vip_node_2G_tx=1
vip_node_2G_rx=1

if [ "$lab_mode_value" = "1" -o "$lab_mode_value" = "2" ] && [ ! -f /tmp/.wlan${radio}_quick_mode_configured ]; then
	while [ -z "$(ip link show wlan$radio | grep -w UP)" ]; do
		sleep 1
	done
	if [ "$lab_mode_value" = "1" ]; then
		if [ "$vip_node_enable" = "1" ]; then
			if [ "$band" = "5G" ]; then
				echo "enable VIP node on 5G, tx:$vip_node_5G_tx rx:$vip_node_5G_rx" > /dev/console
				sh /usr/psbin/dht_msg_with_params.sh band=$radio id=0x83 msg="$vip_node_5G_tx:u32 $vip_node_5G_rx:u32 0x20:u32 0x8:u32 0x64:u32 0x0:u32 0x0:u32"
			else
				echo "enable VIP node on 2G, tx:$vip_node_2G_tx rx:$vip_node_2G_rx" > /dev/console
				sh /usr/psbin/dht_msg_with_params.sh band=$radio id=0x83 msg="$vip_node_2G_tx:u32 $vip_node_2G_rx:u32 0x20:u32 0x8:u32 0x64:u32 0x0:u32 0x0:u32"
				fi
		fi
	elif [ "$lab_mode_value" = "2" ]; then
		sh /usr/psbin/dht_msg_with_params.sh band=$radio id=0x32 msg="0x80000000:u32"
	fi

	touch /tmp/.wlan${radio}_quick_mode_configured
fi

low_power_enable=""
if [ ! -f "/etc/ignore_low_power" ] && [ "$mode" == "ap" ]; then
	low_power_enable="1"
fi

echo "****** config registers for $band ******" > /dev/console
if [ $band == "2G" ]; then
	echo "todo: sh /root/irf/cfr_sw_bypass_2g.sh"
	echo "todo: sh /usr/psbin/rx_sens_test_2g_phy.sh"

	if [ "$low_power_enable" == "1" ]; then
		echo "config $band $mode dynamic low power" > /dev/console
		/usr/psbin/low_power.sh 2g_dyn
	fi
elif [ $band == "5G" ]; then
	echo "todo: sh /root/irf/cfr_sw_bypass_5g.sh"
	echo "todo: sh /usr/psbin/rx_sens_test_5g_phy.sh"

	if [ "$low_power_enable" == "1" ]; then
		echo "config $band $mode dynamic low power" > /dev/console
		/usr/psbin/low_power.sh 5g_dyn
	fi
fi

#npe txdone intr
npe_txdone_irq=`cat /proc/interrupts | grep "GICv3 170"`
npe_txdone_irq_idx=`eval echo ${npe_txdone_irq%%:*}`
echo 2 > /proc/irq/$npe_txdone_irq_idx/smp_affinity
