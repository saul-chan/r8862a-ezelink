#!/bin/bash

usage(){
	echo "PARA: node mux_data mux_trg len width sync intval trg_sel delay"
	echo "parameter number should be 12"
	echo "node    : 0~3, sample node"
	echo "mux_data: lv0 ~ lv4 smp_dat_sel"
	echo "mux_trg : lv0 ~ lv4 smp_trg_sel"
	echo "len     : sample data len"
	echo "width   : sample data width, 0-32bit, 1-16bit"
	echo "sync    : sync clock, 0-640MHz, 1-320MHz"
	echo "intval  : sample data interval, 0/1/2/3"
	echo "trg_sel : 0-software triger, 1-hardware triger"
	echo "delay   : triger delay in software triger mode"
	echo "update  : sample dpdlms update, 0-don't sample, 1-sample"
	echo "sub     : 0-2G, 1-5G"
	echo "channel : 0-CH0, 1-CH1"
}

if [ $# -ne 9 ]; then
	usage
else
	echo "$1 $2 $3 $4 $5 $6 $7 $8 $9 ${10} ${11} ${12}" > /sys/kernel/debug/ieee80211/phy0/cls_wifi/irf/irf_smp_lms
fi
