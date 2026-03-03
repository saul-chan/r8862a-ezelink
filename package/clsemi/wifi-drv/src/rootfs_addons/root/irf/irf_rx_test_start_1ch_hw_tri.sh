#!/bin/sh

# irf_send_config.sh
node=0
interval=1
trigger=1
# SIFS 16us
delay=10240
len=0
loop=0
# front of PPDU
ppdu_f_len=0
# back of PPDU, 1us
ppdu_b_len=640
# gap between PPDU, 16us
ppdu_g_len=12800
phase=0
file=/tmp/test.dat

# irf_send_start.sh
node_mask=0x1
band=1
channel=0
src_node=-1

# dif-sample
smp_len=0
smp_chan=1

do_cmd()
{
	echo "$@"
	eval "$@"
}

usage()
{
	echo "Usage: $0 -len:<len> [-node:<node>] [-band:<band>] [-h] [-smp_len:<smp_len>]"
	echo "  -len       : length"
	echo "  -smp_len   : sample length"
	echo "  -node      : 0~3"
	echo "  -band      : 0-2.4G, 1-5G"
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
	-node:*)
		node=${1#-node:}
		let "node_mask=1<<$node"
		;;
	-band:*)
		band=${1#-band:}
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

do_cmd sh /root/irf/irf_send_config.sh $node $interval $trigger $delay 4 $loop $ppdu_f_len $ppdu_b_len $ppdu_g_len $phase $file
do_cmd echo 0x40620074 $len > /sys/kernel/debug/ieee80211/phy0/cls_wifi/irf/irf_set_reg
do_cmd echo 0x40620044 0x5002 > /sys/kernel/debug/ieee80211/phy0/cls_wifi/irf/irf_set_reg

do_cmd sh /root/irf/irf_send_start.sh $node_mask $band $channel $src_node

echo "Done"
