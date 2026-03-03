#!/bin/sh

catch_wps_credentials()
{
	local iface ifaces ifc ifname ssid encryption key radio radios

	. /usr/share/libubox/jshn.sh
	ubus -S -t 120 listen wps_credentials | while read creds; do
		json_init
		json_load "$creds"
		json_select wps_credentials || continue
		json_get_vars ifname ssid key encryption
		local ifcname="$ifname"
		json_init
		json_load "$(ubus -S call network.wireless status)"
		json_get_keys radios
		for radio in $radios; do
			json_select $radio
			json_select interfaces
			json_get_keys ifaces
			for ifc in $ifaces; do
				json_select $ifc
				json_get_vars ifname

				[ "$ifname" = "$ifcname" ] && {
					vif_info_file="/var/run/vif_info-$(basename $(readlink /sys/class/net/$ifcname/phy80211)).conf"
					section=$(grep -m1 -s " $ifcname " $vif_info_file | awk '{print $1}')

					ubus -S call uci set "{\"config\":\"wireless\", \"type\":\"wifi-iface\", \"section\": \"$section\", \
					\"values\": { \"encryption\": \"$encryption\", \
					\"ssid\": \"$ssid\", \
					\"key\": \"$key\" } }"
					uci commit wireless
				}
				json_select ..
			done
			json_select ..
			json_select ..
		done
	done
}

catch_wps_credentials &

return 0
