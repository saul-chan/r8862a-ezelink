#!/bin/bash

usage(){
	echo "irf_send_config node intval trigger delay len loop ppdu_f_len ppdu_b_len ppdu_g_len phase file"
	echo "parameter number should be : 11"
	echo "node            : 0~3, send node"
	echo "interval        : 0~5, 0-640MHz,1-320MHz,2-160MHz,3-80MHz,4-40MHz,5-20MHz"
	echo "trigger         : 0-software trigger, 1-hardware trigger"
	echo "delay           : triger delay in software triger mode"
	echo "len             : send data sample number length,it's in unit of width,if width is set to 32bit,it means how many 32bit to send data"
	echo "loop            : times to send data in one cycle"
	echo "ppdu_f_len      : send_dat_ppdu_front_len"
	echo "ppdu_b_len      : send_dat_ppdu_back_len"
	echo "ppdu_g_len      : send_dat_ppdu_gap_len"
	echo "width           : send data width 0 - 32bit,1 - 64bit,2 - 128bit"
	echo "file            : data source file in file system"
	echo "dat_vld         : It's optional. 0-valid signal is always high, 1-valid signal changes by cycle"
	echo "snd_mod         : It's optional. 0-common send data mode, 1-DDR send data mode"
}

dat_vld=0
snd_mod=0

if [ $# -eq 13 ]; then
    dat_vld=${12}
    snd_mod=${13}
fi

if [ $# -eq 12 ]; then
    dat_vld=${12}
fi

if [ $# -lt 11 ]; then
	usage
else
	echo "$1 $2 $3 $4 $5 $6 $7 $8 $9 ${10} ${11} ${dat_vld} ${snd_mod}" > /sys/kernel/debug/ieee80211/phy0/cls_wifi/irf/irf_send_config
fi
