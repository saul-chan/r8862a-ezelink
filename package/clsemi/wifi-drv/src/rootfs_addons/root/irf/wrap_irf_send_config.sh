#!/bin/sh

# irf_send_config.sh
interval=1
trigger=0
delay=10240
len=0
loop=0
ppdu_f_len=0
ppdu_b_len=0x1FFFF
ppdu_g_len=0x1FFFF
phase=0
file=/tmp/test.dat
# temp
chan=1
band=1
file1=/tmp/test.dat

do_cmd()
{
	echo "$@"
	eval "$@"
}

usage()
{
	echo "Usage: $0 -len:<len> [-chan:<chan>] [-h]"
	echo "  -len       : length"
	echo "  -chan      : number of channel to use"
	echo "  -band      : 0-2.4G, 1-5G"
	echo "  -trigger   : 0-software 1-hardware"
	echo "  -file      : IQ data file for chan 0"
	echo "  -file1     : IQ data file for chan 1"
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
	-chan:*)
		chan=${1#-chan:}
		;;
	-band:*)
		band=${1#-band:}
		;;
	-trigger:*)
		trigger=${1#-trigger:}
		;;
	-file:*)
		file=${1#-file:}
		if [ ! -f $file ]; then
			echo "Error: please check IQ data file $file"
			exit 0
		fi
		;;
	-file1:*)
		file1=${1#-file1:}
		if [ ! -f ${file1} ]; then
			echo "Error: please check IQ data file ${file1}"
			exit 0
		fi
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

if [ $chan -gt 1 ] && [ $len -gt 65536 ]; then
	echo "Error: length can't be over 65536!"
	exit 0
fi

do_cmd sh /root/irf/irf_send_config.sh 0 $interval $trigger $delay $len $loop $ppdu_f_len $ppdu_b_len $ppdu_g_len $phase $file
if [ $chan -gt 1 ]; then
do_cmd sh /root/irf/irf_send_config.sh 1 $interval $trigger $delay $len $loop $ppdu_f_len $ppdu_b_len $ppdu_g_len $phase ${file1}
fi

if [ $trigger -eq 1 ]; then
	SMP_MUX_TRG_SEL_REG=0x4c400008
	if [ $band -eq 0 ]; then
		SMP_MUX_TRG_SEL_VAL=0x1002
	else
		SMP_MUX_TRG_SEL_VAL=0x5002
	fi
	do_cmd echo $SMP_MUX_TRG_SEL_REG $SMP_MUX_TRG_SEL_VAL > /sys/kernel/debug/ieee80211/phy0/cls_wifi/irf/irf_set_reg
	if [ $chan -gt 1 ]; then
		SMP_MUX_TRG_SEL_REG=0x4c400048
		if [ $band -eq 0 ]; then
			SMP_MUX_TRG_SEL_VAL=0x1003
		else
			SMP_MUX_TRG_SEL_VAL=0x5003
		fi
		do_cmd echo $SMP_MUX_TRG_SEL_REG $SMP_MUX_TRG_SEL_VAL > /sys/kernel/debug/ieee80211/phy0/cls_wifi/irf/irf_set_reg
	fi
fi

echo "Done"
