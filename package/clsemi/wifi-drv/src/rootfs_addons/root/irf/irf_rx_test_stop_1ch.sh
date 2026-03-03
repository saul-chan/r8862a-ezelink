#!/bin/sh

node_mask=0x1
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
	echo "Usage: $0 [-node_mask:<node_mask>] [-band:<band>] [-channel:<channel>] [-h]"
	echo "  -node_mask : bit0-node0, bit1-node1, bit2-node2, bit3-node3"
	echo "  -band      : 0-2.4G, 1-5G"
	echo "  -channel   : 0~1"
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
	-node_mask:*)
		node_mask=${1#-node_mask:}
		;;
	-band:*)
		band=${1#-band:}
		;;
	-channel:*)
		channel=${1#-channel:}
		;;
	-h)
		usage
		exit 0
		;;
	esac
	shift
done

do_cmd sh /root/irf/irf_send_stop.sh $node_mask $band $channel $src_node

echo "Done"
