#!/bin/sh

wps_catch_credentials() {
	local iface ifaces ifc ifname ssid encryption key radio radios
	local found=0

	. /usr/share/libubox/jshn.sh
	ubus -S -t 30 listen wps_credentials | while read creds; do
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
					ubus -S call uci set "{\"config\":\"wireless\", \"type\":\"wifi-iface\",		\
								\"match\": { \"device\": \"$radio\", \"encryption\": \"wps\" },	\
								\"values\": { \"encryption\": \"$encryption\", 			\
										\"ssid\": \"$ssid\", 				\
										\"key\": \"$key\" } }"
					ubus -S call uci commit '{"config": "wireless"}'
					ubus -S call uci apply
				}
				json_select ..
			done
			json_select ..
			json_select ..
		done
	done
}

get_primary_iface_with_wps() {
	config_file="$1"
	interface_name=""
	has_wps_push_button=0

	while IFS= read -r line || [ -n "$line" ]; do

		trimmed_line=$(echo "$line" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')

		case "$trimmed_line" in
			interface=*)
			interface_name="${trimmed_line#*=}"
		;;
			bss=*)
			break
		;;
			config_methods=*push_button*)
			has_wps_push_button=1
		;;
		esac
	done < "$config_file"

	if [ -n "$interface_name" ] && [ "$has_wps_push_button" -eq 1 ]; then
		echo "$interface_name"
	else
		echo ""
	fi
}

if [ "$ACTION" = "released" ] && [ "$BUTTON" = "wps" ]; then
	mesh_solution="$(clsapi get board_info | grep "release_version" | grep "Mesh")"
	if [ -n "$mesh_solution" ]; then
		return 0
	fi
	# If the button was pressed for 3 seconds or more, trigger WPS on
	# wpa_supplicant only, no matter if hostapd is running or not.  If
	# was pressed for less than 3 seconds, try triggering on
	# hostapd. If there is no hostapd instance to trigger it on or WPS
	# is not enabled on them, trigger it on wpa_supplicant.
	if [ "$SEEN" -lt 3 ] ; then
		wps_done=0
		for config_file in /var/run/hostapd-phy*.conf; do
			iface=$(get_primary_iface_with_wps "$config_file")
			if [ -n "$iface" ]; then
				ubus -S call "hostapd.$iface" wps_start && wps_done=1
			fi
		done

		[ $wps_done = 0 ] || return 0
	fi
	wps_done=0
	ubusobjs="$( ubus -S list wpa_supplicant.* )"
	for ubusobj in $ubusobjs; do
		ifname="$(echo $ubusobj | cut -d'.' -f2 )"
		multi_ap=""
		if [ -e "/var/run/wpa_supplicant-${ifname}.conf.is_multiap" ]; then
			ubus -S call $ubusobj wps_start '{ "multi_ap": true }' && wps_done=1
		else
			ubus -S call $ubusobj wps_start && wps_done=1
		fi
	done

#	[ $wps_done = 0 ] || wps_catch_credentials &
fi

return 0
