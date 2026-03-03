npe_dtb_path="/etc/npe_dtb/"

if `lsmod | grep -q dts_overlay`; then
	echo "dts_overlay already load"
	exit
fi

if [ $# -gt 0 ]; then
    ver_str=$1
else
	ver=`devmem 0x90444404`
	ver_str=`printf "C%02XV%02XP%02XS%02X.dtb" "$((($ver >> 24) & 0xFF))" "$((($ver >> 16) & 0xFF))" "$((($ver >> 8) & 0xFF))" "$((($ver) & 0xFF))"`
fi

if [ -f "$npe_dtb_path/$ver_str" ]; then

	echo "============load npe config file $ver_str============="

	modprobe dts_overlay
	mount -t configfs none /sys/kernel/config/
	mkdir /sys/kernel/config/device-tree/overlays/npe
	cat $npe_dtb_path/$ver_str >  /sys/kernel/config/device-tree/overlays/npe/dtbo

fi

modprobe cls_npe
