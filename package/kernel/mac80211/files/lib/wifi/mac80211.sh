#!/bin/sh

append DRIVERS "mac80211"

lookup_phy() {
	[ -n "$phy" ] && {
		[ -d /sys/class/ieee80211/$phy ] && return
	}

	local devpath
	config_get devpath "$device" path
	[ -n "$devpath" ] && {
		phy="$(iwinfo nl80211 phyname "path=$devpath")"
		[ -n "$phy" ] && return
	}

	local macaddr="$(config_get "$device" macaddr | tr 'A-Z' 'a-z')"
	[ -n "$macaddr" ] && {
		for _phy in /sys/class/ieee80211/*; do
			[ -e "$_phy" ] || continue

			[ "$macaddr" = "$(cat ${_phy}/macaddress)" ] || continue
			phy="${_phy##*/}"
			return
		done
	}
	phy=
	return
}

find_mac80211_phy() {
	local device="$1"

	config_get phy "$device" phy
	lookup_phy
	[ -n "$phy" -a -d "/sys/class/ieee80211/$phy" ] || {
		echo "PHY for wifi device $1 not found"
		return 1
	}
	config_set "$device" phy "$phy"

	config_get macaddr "$device" macaddr
	[ -z "$macaddr" ] && {
		config_set "$device" macaddr "$(cat /sys/class/ieee80211/${phy}/macaddress)"
	}

	return 0
}

check_mac80211_device() {
	config_get phy "$1" phy
	[ -z "$phy" ] && {
		find_mac80211_phy "$1" >/dev/null || return 0
		config_get phy "$1" phy
	}
	[ "$phy" = "$dev" ] && found=1
}


__get_band_defaults() {
	local phy="$1"

	( iw phy "$phy" info; echo ) | awk '
BEGIN {
        bands = ""
}

($1 == "Band" || $1 == "") && band {
        if (channel) {
		mode="NOHT"
		if (ht) mode="HT20"
		if (vht && band != "1:") mode="VHT80"
		if (he) mode="HE80"
		if (he && band == "1:") mode="HE20"
                sub("\\[", "", channel)
                sub("\\]", "", channel)
                bands = bands band channel ":" mode " "
        }
        band=""
}

$1 == "Band" {
        band = $2
        channel = ""
	vht = ""
	ht = ""
	he = ""
}

$0 ~ "Capabilities:" {
	ht=1
}

$0 ~ "VHT Capabilities" {
	vht=1
}

$0 ~ "HE Iftypes" {
	he=1
}

$1 == "*" && $3 == "MHz" && $0 !~ /disabled/ && band && !channel {
        channel = $4
}

END {
        print bands
}'
}

get_band_defaults() {
	local phy="$1"

	for c in $(__get_band_defaults "$phy"); do
		local band="${c%%:*}"
		c="${c#*:}"
		local chan="${c%%:*}"
		c="${c#*:}"
		local mode="${c%%:*}"

		case "$band" in
			1) band=2g;;
			2) band=5g;;
			3) band=60g;;
			4) band=6g;;
			*) band="";;
		esac

		[ -n "$band" ] || continue
		[ -n "$mode_band" -a "$band" = "6g" ] && return

		mode_band="$band"
		channel="$chan"
		htmode="$mode"
	done
}

get_cls_band_boarddata() {
	ssid=$(fw_printsys -n ssid_$1 2>/dev/null)
	key=$(fw_printsys -n key_$1 2>/dev/null)
	if [ -z "$ssid" ]; then
		if [ "$1" == "2g" ]; then
			ssid="Clourney"
		else
			ssid="Clourney-5G"
		fi
	fi
	if [ -n "$key" ]; then
		encryption="psk2"
	else
		encryption="none"
	fi
}

get_cls_band_defaults() {
	local band="$1"
	local cal_state=$(fw_printenv -n cal_state 2>/dev/NULL)

	case "$band" in
	2g)
		country="CN"
		htmode="HE20"
		channel="1"
		get_cls_band_boarddata 2g
		;;
	5g)
		country="CN"
		if [ "$cal_state" == "3" ]; then
			htmode="HE80"
		else
			htmode="HE160"
		fi
		channel="36"
		get_cls_band_boarddata 5g
		;;
	esac
}

set_global_config() {
	uci -q batch <<-EOF
		set wireless.globals=globals
	EOF
	uci -q commit wireless
}

cls_get_opmode()
{
	local mode

	# If cls-opmode package is not enabled, return directly
	config_load cls-opmode || return
	config_get mode globals mode

	echo $mode
}

# Mesh solution default UCI
gen_mesh_solution_def_uci() {
	local radio_idx="$1"
	local band="$2"
	local ssid_fh=$(fw_printsys -n ssid_${band} 2>/dev/null)
	local ssid_bh=$(fw_printsys -n ssid_${band}_bh 2>/dev/null)
	local key_bh=$(fw_printsys -n key_${band}_bh 2>/dev/null)
	local last_4mac=$(fw_printsys -n macwlan0 2>/dev/null | awk -F: '{print toupper($(NF-1) $NF)}')
	local def_key_bh=$(echo $last_4mac | md5sum | awk '{print substr($0, 1, 9)}')
	local def_ssid_bh="EZE-TGW3130-${last_4mac}-bh"
	local ap_idx=1
	local mode

	if [ -z "$ssid_fh" ]; then
		if [ "$band" == "2g" ]; then
			ssid_fh="Clourney-${last_4mac}"
		else
			ssid_fh="Clourney-${last_4mac}-5G"
		fi
	fi

	if [ $radio_idx -gt 0 ]; then
		uci -q batch <<-EOF
			# STA
			set wireless.station_iface=wifi-iface
			set wireless.station_iface.ifname=wlan${radio_idx}-sta
			set wireless.station_iface.device=radio${radio_idx}
			set wireless.station_iface.network=lan
			set wireless.station_iface.mode=sta
			set wireless.station_iface.ssid=${ssid_bh:-$def_ssid_bh}
			set wireless.station_iface.encryption=psk2
			set wireless.station_iface.key=${key_bh:-${def_key_bh}}
			set wireless.station_iface.wds=1
			set wireless.station_iface.multi_ap=1

			# Backhaul BSS
			set wireless.wifinet${radio_idx}_${ap_idx}=wifi-iface
			set wireless.wifinet${radio_idx}_${ap_idx}.device=radio${radio_idx}
			set wireless.wifinet${radio_idx}_${ap_idx}.network=lan
			set wireless.wifinet${radio_idx}_${ap_idx}.mode=ap
			set wireless.wifinet${radio_idx}_${ap_idx}.ssid=${ssid_bh:-$def_ssid_bh}
			set wireless.wifinet${radio_idx}_${ap_idx}.encryption=psk2
			set wireless.wifinet${radio_idx}_${ap_idx}.key=${key_bh:-$def_key_bh}
			set wireless.wifinet${radio_idx}_${ap_idx}.wds=1
			set wireless.wifinet${radio_idx}_${ap_idx}.multi_ap=1
			set wireless.wifinet${radio_idx}_${ap_idx}.hidden=1
		EOF
		ap_idx=$(($ap_idx + 1))
		mode=$(cls_get_opmode)
		if [ "$mode" != "repeater" ]; then
			uci set wireless.station_iface.disabled=1
		fi
	fi

	uci -q batch <<-EOF
		# Fronthaul BSS
		set wireless.wifinet${radio_idx}_${ap_idx}=wifi-iface
		set wireless.wifinet${radio_idx}_${ap_idx}.device=radio${radio_idx}
		set wireless.wifinet${radio_idx}_${ap_idx}.network=lan
		set wireless.wifinet${radio_idx}_${ap_idx}.mode=ap
		set wireless.wifinet${radio_idx}_${ap_idx}.ssid=${ssid_fh}
		set wireless.wifinet${radio_idx}_${ap_idx}.encryption=psk2
		set wireless.wifinet${radio_idx}_${ap_idx}.key=${key:-ezelink3120}
		set wireless.wifinet${radio_idx}_${ap_idx}.wps_pushbutton=1
	EOF
	if [ $radio_idx -gt 0 ]; then
		uci -q batch <<-EOF
			# Backhaul SSID/Key in Fronthaul BSS
			set wireless.wifinet${radio_idx}_${ap_idx}.multi_ap=2
			set wireless.wifinet${radio_idx}_${ap_idx}.multi_ap_backhaul_ssid=${ssid_bh:-$def_ssid_bh}
			set wireless.wifinet${radio_idx}_${ap_idx}.multi_ap_backhaul_key=${key_bh:-$def_key_bh}
		EOF
	fi
	ap_idx=$(($ap_idx + 1))

	uci -q batch <<-EOF
		# Guest BSS
		set wireless.wifinet${radio_idx}_${ap_idx}=wifi-iface
		set wireless.wifinet${radio_idx}_${ap_idx}.device=radio${radio_idx}
		set wireless.wifinet${radio_idx}_${ap_idx}.network=lan
		set wireless.wifinet${radio_idx}_${ap_idx}.mode=ap
		set wireless.wifinet${radio_idx}_${ap_idx}.ssid=${ssid_fh}-Guest
		set wireless.wifinet${radio_idx}_${ap_idx}.encryption=psk2
		set wireless.wifinet${radio_idx}_${ap_idx}.key=ezelink3120
		set wireless.wifinet${radio_idx}_${ap_idx}.disabled=1
	EOF
}

detect_mac80211() {
	devidx=0
	config_load wireless
	while :; do
		config_get type "radio$devidx" type
		[ -n "$type" ] || break
		devidx=$(($devidx + 1))
	done

	for _dev in /sys/class/ieee80211/*; do
		[ -e "$_dev" ] || continue

		dev="${_dev##*/}"

		found=0
		config_foreach check_mac80211_device wifi-device
		[ "$found" -gt 0 ] && continue

		mode_band=""
		channel=""
		htmode=""
		ht_capab=""

		get_band_defaults "$dev"
		get_cls_band_defaults "$mode_band"

		path="$(iwinfo nl80211 path "$dev")"
		if [ -n "$path" ]; then
			dev_id="set wireless.radio${devidx}.path='$path'"
		else
			dev_id="set wireless.radio${devidx}.macaddr=$(cat /sys/class/ieee80211/${dev}/macaddress)"
		fi

		uci -q batch <<-EOF
			set wireless.radio${devidx}=wifi-device
			set wireless.radio${devidx}.type=mac80211
			${dev_id}
			set wireless.radio${devidx}.channel=${channel}
			set wireless.radio${devidx}.country=${country:-CN}
			set wireless.radio${devidx}.band=${mode_band}
			set wireless.radio${devidx}.htmode=$htmode
		EOF

		mesh_solution="$(clsapi get board_info | grep "release_version" | grep "Mesh")"
		if [ -z "$mesh_solution" ]; then
			local last_4mac=$(fw_printsys -n macwlan0 2>/dev/null | awk -F: '{print toupper($(NF-1) $NF)}')
			local def_ssid="EZE-TGW3130-${last_4mac}"
			
			uci -q batch <<-EOF
				set wireless.default_radio${devidx}=wifi-iface
				set wireless.default_radio${devidx}.device=radio${devidx}
				set wireless.default_radio${devidx}.network=lan
				set wireless.default_radio${devidx}.mode=ap
				set wireless.default_radio${devidx}.ssid=${def_ssid:-OpenWrt}
				set wireless.default_radio${devidx}.encryption=${encryption:-psk2}
				set wireless.default_radio${devidx}.key=${key:-ezelink3120}
				set wireless.default_radio${devidx}.wps_pushbutton=1
			EOF
			if [ $devidx -eq 1 ]; then
				uci -q batch <<-EOF
					set wireless.station_iface=wifi-iface
					set wireless.station_iface.device=radio${devidx}
					set wireless.station_iface.network=lan
					set wireless.station_iface.ifname=wlan${devidx}-sta
					set wireless.station_iface.mode=sta
					set wireless.station_iface.ssid=Clourney_RootAP
					set wireless.station_iface.encryption=none
					set wireless.station_iface.disabled=1
				EOF
			fi
		else
			gen_mesh_solution_def_uci "$devidx" "$mode_band"
		fi

		[ "$mode_band" = "5g" ] && {
			uci -q batch <<-EOF
				add_list wireless.radio${devidx}.channels=36-64
				add_list wireless.radio${devidx}.channels=149-165
			EOF
		}
		uci -q commit wireless

		devidx=$(($devidx + 1))
	done

	set_global_config
}
