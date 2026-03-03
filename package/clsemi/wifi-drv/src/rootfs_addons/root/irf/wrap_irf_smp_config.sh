#!/bin/sh

# irf_smp_config.sh
mux_data=0xd000
mux_trg=0x4000
len=0
width=0
sync=0
intval=1
trg_sel=0
delay=0x10
wlan=0
smp_mod=0
# temp
band=1
chan=1

do_cmd()
{
	echo "$@"
	eval "$@"
}

usage()
{
	echo "Usage: $0 -len:<len> [-band:<band>] [-chan:<chan>] [-h]"
	echo "  -len       : length"
	echo "  -band      : 0-2.4G, 1-5G"
	echo "  -chan      : number of channel to use"
	echo "  -wlan      : 0, 1"
	echo "  -smp_mod   : 0-Legacy sample data mode, 1-DDR sample data mode"
	echo "  -h         : display this message"
}

# debugfs check
if [ ! -d /sys/kernel/debug ]; then
	echo "Error: please mount debugfs first!"
	exit 0
fi

# parse input parameters
while test $# -gt 0
do
	case "$1" in
	-len:*)
		len=${1#-len:}
		;;
	-band:*)
		band=${1#-band:}
		if [ $band -eq 0 ]; then
			mux_data=0
			mux_trg=0
		fi
		;;
	-chan:*)
		chan=${1#-chan:}
		;;
	-wlan:*)
		wlan=${1#-wlan:}
		;;
	-h)
		usage
		exit 0
		;;
	esac
	shift
done

if [ $len -eq 0 ]; then
	echo "Error: please input IQ data length!"
	exit 0
fi

do_cmd sh /root/irf/irf_smp_config.sh 2 $mux_data $mux_trg $len $width $sync $intval $trg_sel $delay $wlan $smp_mod
if [ $chan -gt 1 ]; then
	let "mux_data=mux_data+0x100"
	let "mux_trg=mux_trg+0x100"
	do_cmd sh /root/irf/irf_smp_config.sh 3 $mux_data $mux_trg $len $width $sync $intval $trg_sel $delay $wlan $smp_mod
fi

echo "Done"
