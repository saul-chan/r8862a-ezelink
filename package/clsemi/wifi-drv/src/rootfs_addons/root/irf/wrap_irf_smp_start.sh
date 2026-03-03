#!/bin/sh

# irf_smp_start.sh
node_mask=0x4
timeout=0
wlan=0
# temp
chan=1

do_cmd()
{
	echo "$@"
	eval "$@"
}

usage()
{
	echo "Usage: $0 [-chan:<chan>] [-timeout:<timeout>] [-h]"
	echo "  -chan      : number of channel to use"
	echo "  -timeout   : timeout (ms)"
	echo "  -wlan      : 0, 1"
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
	-chan:*)
		chan=${1#-chan:}
		if [ $chan -gt 1 ]; then
			node_mask=0xc
		fi
		;;
	-timeout:*)
		timeout=${1#-timeout:}
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

do_cmd sh /root/irf/irf_smp_start.sh $node_mask $timeout $wlan

echo "Done"
