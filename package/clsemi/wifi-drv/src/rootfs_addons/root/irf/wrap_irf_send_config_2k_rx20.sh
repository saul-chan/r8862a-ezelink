#!/bin/sh

# irf_send_config.sh
interval=1
trigger=0
delay=0x10
len=0
loop=0
ppdu_f_len=10
ppdu_b_len=0x200
ppdu_g_len=0x2800
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
	echo "  -interval  : 0~5, 0-640MHz,1-320MHz,2-160MHz,3-80MHz,4-40MHz,5-20MHz"
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
	-interval:*)
		interval=${1#-interval:}
		if [ $interval -lt 0 ] || [ $interval -gt 5 ]; then
			echo "Error: please check interval: 0~5, 0-640MHz,1-320MHz,2-160MHz,3-80MHz,4-40MHz,5-20MHz"
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

# Just for Rx20 experiment: set node_mask=0 in wrap_irf_send_start.sh
sed -i "s/node_mask=0x1/node_mask=0x0/g" /root/irf/wrap_irf_send_start.sh

do_cmd sh /root/irf/irf_send_config.sh 0 $interval $trigger $delay $len $loop $ppdu_f_len $ppdu_b_len $ppdu_g_len $phase $file
if [ $chan -gt 1 ]; then
do_cmd sh /root/irf/irf_send_config.sh 1 $interval $trigger $delay $len $loop $ppdu_f_len $ppdu_b_len $ppdu_g_len $phase ${file1}
fi

if [ $band -eq 1 ]; then
	# accept RTS, accept ACK, do not accept other BSSID's frame
	devmem 0x4ab00060 32 0x35ffedac
	devmem 0x4010d21c 32 8
	if [ $chan -gt 1 ]; then
		devmem 0x4010d51c 32 8
	fi
	# riurx_send_pot_sel
	devmem 0x4010d2d0 32 0
	if [ $chan -gt 1 ]; then
		devmem 0x4010d5d0 32 1
	fi
	# Tooling mode; riurx_ofdmfe_sften
	devmem 0x4010e600 32 1
	devmem 0x4010e608 32 0x70004
	# active antenna
	if [ $chan -eq 1 ]; then
		devmem 0x4010b004 32 1
	else
		devmem 0x4010b004 32 3
	fi
	# PHY take over mode
	devmem 0x4ac08084 32 0x8
	# config noise
	devmem 0x4ac08078 32 0x128
	devmem 0x4ac0807c 32 0x25252525
	# rx wait out rxvector2 time: 15us
	devmem 0x4ab04064 32 0xcf10c210
elif [ $band -eq 0 ]; then
	# accept RTS, accept ACK, do not accept other BSSID's frame
	devmem 0x48b00060 32 0x35ffedac
	devmem 0x4030d21c 32 8
	if [ $chan -gt 1 ]; then
		devmem 0x4030d51c 32 8
	fi
	# riurx_send_pot_sel
	devmem 0x4030d2d0 32 0
	if [ $chan -gt 1 ]; then
		devmem 0x4030d5d0 32 1
	fi
	# Tooling mode; riurx_ofdmfe_sften
	devmem 0x4030e600 32 1
	devmem 0x4030e608 32 0x70004
	# active antenna
	if [ $chan -eq 1 ]; then
		devmem 0x4030b004 32 1
	else
		devmem 0x4030b004 32 3
	fi
	# PHY take over mode
	devmem 0x48c08084 32 0x8
	# config noise
	devmem 0x48c08078 32 0x128
	devmem 0x48c0807c 32 0x25252525
	# rx wait out rxvector2 time: 15us
	devmem 0x48b04064 32 0xcf10c210
fi

# send_dat_intval 5:20MHz, 4:40MHz, 3:80MHz, 2:160MHz, 1:320MHz, 0:640MHz
# send_dat_vld_mode == 1
devmem 0x40620068 32 $((0x20+$interval))
if [ $chan -gt 1 ]; then
	devmem 0x406200e8 32 $((0x20+$interval))
fi

# set send_frm_inner_num=1, send_frm_outer_num=1
devmem 0x40620080 32 0x00010001
if [ $chan -gt 1 ]; then
	devmem 0x40620100 32 0x00010001
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
