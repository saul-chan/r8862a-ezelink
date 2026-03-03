#!/bin/sh

band=1
enable=1
irf_debugfs_dir=/sys/kernel/debug/ieee80211/phy0/cls_wifi/irf

do_cmd()
{
	echo "$@"
	eval "$@"
}

usage()
{
	echo "Usage: $0 [-band:<band>] [-enable:<enable>] [-h]"
	echo "  -band	: 1 - 5G, 0 - 2.4G"
	echo "  -enable	: 1 - enable smp war, 0 - disable smp war"
	echo "  -h	: display this message"
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
	-enable:*)
		enable=${1#-enable:}
		;;
	-h)
		usage
		exit 0
		;;
	esac
	shift
done

if [ $enable -eq 1 ]; then
	#select node 0
	if [ $band -eq 1 ]; then
		#set data mux to dif160 cfr channel 0 node 0
		echo 0x4c400004 0x4000 > $irf_debugfs_dir/irf_set_reg
		#set trigger mux to dif160 fb channel 0 fb_corr_start
		echo 0x4c400008 0x4906 > $irf_debugfs_dir/irf_set_reg
	else
		#set data mux to dif40 cfr channel 0 node 0
		echo 0x4c400004 0x0000 > $irf_debugfs_dir/irf_set_reg
		#set trigger mux to dif40 fb channel 0 fb_corr_start
		echo 0x4c400008 0x0906 > $irf_debugfs_dir/irf_set_reg
	fi
	#set smp reg config1
	echo 0x4c4000d0 0x0 > $irf_debugfs_dir/irf_set_reg
	#set smp reg config2, length to 1
	echo 0x4c4000d4 0x0 > $irf_debugfs_dir/irf_set_reg
	#set smp reg config4, hardware trigger and delay to 0x10
	echo 0x4c4000dc 0x21 > $irf_debugfs_dir/irf_set_reg
	#start sample, smp reg config0
	echo 0x4c4000cc 0x1 > $irf_debugfs_dir/irf_set_reg
else
	#select node 0
	#clear data mux to 0
	echo 0x4c400004 0x0000 > $irf_debugfs_dir/irf_set_reg
	#clear trigger mux to 0
	echo 0x4c400008 0x0000 > $irf_debugfs_dir/irf_set_reg
	#clear smp reg config4 to 0
	echo 0x4c4000dc 0x00 > $irf_debugfs_dir/irf_set_reg
	#stop sample, smp reg config0
	echo 0x4c4000cc 0x0 > $irf_debugfs_dir/irf_set_reg
fi

echo "Done"
