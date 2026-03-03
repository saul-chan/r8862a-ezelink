#!/bin/sh /etc/rc.common

if [ -f $WAN_DETERMINED ];then
	echo "WAN_DETERMINED exist"
	exit 0
fi

wan_port=$(jsonfilter -i /etc/board.json -e '@.network.wan.device' | xargs)
lan_port=$(jsonfilter -i /etc/board.json -e '@.network.lan.ports[*]' | xargs)
all_ports="${wan_port} ${lan_port}"

# read last count from carrier_up_count
for iface in $all_ports; do
    if [ -f "/sys/class/net/$iface/carrier_up_count" ]; then
        eval LAST_COUNT_$iface=$(cat "/sys/class/net/$iface/carrier_up_count")
    fi
done

while true; do
	for iface in $all_ports; do
		if [ -f "/sys/class/net/$iface/carrier_up_count" ]; then
			current_count=$(cat "/sys/class/net/$iface/carrier_up_count")

			eval last_count=\$LAST_COUNT_$iface

			# if count changed print ethX up
			if [[ "$last_count" != "$current_count" ]]; then
				echo "$iface up"
				/etc/init.d/dhcp_wan_detection start
				exit
			fi

			# update last count
			eval LAST_COUNT_$iface=$current_count
		fi
	done
	sleep 5
done

