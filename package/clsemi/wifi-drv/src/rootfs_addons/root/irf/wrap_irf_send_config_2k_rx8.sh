#!/bin/sh

# irf_send_config.sh
interval=1
trigger=0
delay=10240
len=0
loop=0
send_frm_inner_num=0
send_frm_outer_num=0
ppdu_f_len=0
ppdu_b_len=0x1FFFF
ppdu_g_len=0x1FFFF
phase=0
file=/tmp/test.dat
snd_mod=0
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
	echo "  -send_frm_inner_num: 0-infinite"
	echo "  -send_frm_outer_num: 0-infinite"
	echo "  -file      : IQ data file for chan 0"
	echo "  -file1     : IQ data file for chan 1"
	echo "  -snd_mod   : 0-Legacy send data mode, 1-DDR send data mode"
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
	-send_frm_inner_num:*)
		send_frm_inner_num=${1#-send_frm_inner_num:}
		;;
	-send_frm_outer_num:*)
		send_frm_outer_num=${1#-send_frm_outer_num:}
		;;
	-ppdu_f_len:*)
		ppdu_f_len=${1#-ppdu_f_len:}
		;;
	-ppdu_b_len:*)
		ppdu_b_len=${1#-ppdu_b_len:}
		;;
	-ppdu_g_len:*)
		ppdu_g_len=${1#-ppdu_g_len:}
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
	-snd_mod:*)
		snd_mod=${1#-snd_mod:}
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

if [ $chan -gt 1 ] && [ $len -gt 2097152 ]; then
	echo "Error: length can't be over 2097152!"
	exit 0
fi

if [ $band -eq 0 ]; then
	delay=6400
fi

loop=$(((send_frm_outer_num<<16)|$send_frm_inner_num))

do_cmd sh /root/irf/irf_send_config.sh 0 $interval $trigger $delay $len $loop $ppdu_f_len $ppdu_b_len $ppdu_g_len $phase $file 1 $snd_mod
if [ $band -eq 0 ]; then
    devmem 0x402C0534 32 0x1
else
    devmem 0x400C0534 32 0x1
fi

if [ $chan -gt 1 ]; then
    do_cmd sh /root/irf/irf_send_config.sh 1 $interval $trigger $delay $len $loop $ppdu_f_len $ppdu_b_len $ppdu_g_len $phase ${file1} 1 $snd_mod
    if [ $band -eq 0 ]; then
        devmem 0x402E0534 32 0x1
    else
        devmem 0x400E0534 32 0x1
    fi
fi

if [ $trigger -eq 1 ]; then
	SMP_MUX_TRG_SEL_REG=0x40620044
	SMP_DAT_CFG1_REG=0x40620048
	SMP_DAT_TRIG_VAL=0x40
	if [ $band -eq 0 ]; then
		SMP_MUX_TRG_SEL_VAL=0x9803
	else
		SMP_MUX_TRG_SEL_VAL=0xd803
	fi
	do_cmd echo $SMP_MUX_TRG_SEL_REG $SMP_MUX_TRG_SEL_VAL > /sys/kernel/debug/ieee80211/phy0/cls_wifi/irf/irf_set_reg
	do_cmd echo $SMP_DAT_CFG1_REG $SMP_DAT_TRIG_VAL > /sys/kernel/debug/ieee80211/phy0/cls_wifi/irf/irf_set_reg
	if [ $chan -gt 1 ]; then
		SMP_MUX_TRG_SEL_REG=0x406200C4
		SMP_DAT_CFG1_REG=0x406200C8
		SMP_DAT_TRIG_VAL=0x40
		if [ $band -eq 0 ]; then
			SMP_MUX_TRG_SEL_VAL=0x9803
		else
			SMP_MUX_TRG_SEL_VAL=0xd803
		fi
		do_cmd echo $SMP_MUX_TRG_SEL_REG $SMP_MUX_TRG_SEL_VAL > /sys/kernel/debug/ieee80211/phy0/cls_wifi/irf/irf_set_reg
		do_cmd echo $SMP_DAT_CFG1_REG $SMP_DAT_TRIG_VAL > /sys/kernel/debug/ieee80211/phy0/cls_wifi/irf/irf_set_reg
	fi
fi

echo "Done"
