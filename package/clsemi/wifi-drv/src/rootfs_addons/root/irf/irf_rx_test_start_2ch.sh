#!/bin/sh

# irf_send_config.sh
interval=1
trigger=0
delay=0x10
len=0
loop=0
ppdu_f_len=0x1ffff
ppdu_b_len=0x1ffff
ppdu_g_len=0x1ffff
phase=0
file=/tmp/test.dat

# irf_send_start.sh
node_mask=0x3
band=1
channel=0
src_node=-1

do_cmd()
{
	echo "$@"
	eval "$@"
}

usage()
{
	echo "Usage: $0 -len:<len> [-band:<band>] [-h]"
	echo "  -len       : length"
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

if [ $len -gt 65536 ]; then
	echo "Error: length can't be over 65536!"
	exit 0
fi

do_cmd sh /root/irf/irf_send_config.sh 0 $interval $trigger $delay $len $loop $ppdu_f_len $ppdu_b_len $ppdu_g_len $phase $file
do_cmd sh /root/irf/irf_send_config.sh 1 $interval $trigger $delay $len $loop $ppdu_f_len $ppdu_b_len $ppdu_g_len $phase $file

do_cmd sh /root/irf/irf_send_start.sh $node_mask $band $channel $src_node

echo "Done"
