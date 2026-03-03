#!/bin/sh

# irf_send_start.sh
node_mask=0x1
band=1
channel=0
src_node=-1
# temp
chan=1

do_cmd()
{
	echo "$@"
	eval "$@"
}

usage()
{
	echo "Usage: $0 [-band:<band>] [-chan:<chan>] [-h]"
	echo "  -band      : 0-2.4G, 1-5G"
	echo "  -chan      : number of channel to use"
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
	-band:*)
		band=${1#-band:}
		;;
	-chan:*)
		chan=${1#-chan:}
		if [ $chan -gt 1 ]; then
			node_mask=0x3
		fi
		;;
	-h)
		usage
		exit 0
		;;
	esac
	shift
done

do_cmd sh /root/irf/irf_send_start.sh $node_mask $band $channel $src_node

echo "Done"
