#!/bin/bash

usage(){
	echo "PARA: node mux_data mux_trg len width sync intval trg_sel delay"
	echo "parameter number should be 10"
	echo "node    : 0~3, sample node"
	echo "mux_data: lv0 ~ lv4 smp_dat_sel"
	echo "mux_trg : lv0 ~ lv4 smp_trg_sel"
	echo "len     : sample number length,it's in unit of width,if width is set to 32bit,it means how many 32bit to sample"
	echo "width   : sample data width, 0-32bit, 1-64bit, 2-128bit"
	echo "smp_trg : 0-high level, 1-low level, 2-positive edge, 3-negtive edge"
	echo "intval  : sample data interval, 0/1/2/3"
	echo "trg_sel : 0-software triger, 1-hardware triger"
	echo "delay   : triger delay in software triger mode"
	echo "wlan    : 0 or 1"
	echo "smp_mod : It's optional. 0-common sample data mode, 1-DDR sample data mode"
}

smp_mod=0

if [ $# -eq 11 ]; then
    smp_mod=${11}
fi

if [ $# -lt 10 ]; then
	usage
else
	echo "/sys/kernel/debug/ieee80211/phy${10}/cls_wifi/irf/irf_smp_config"
	echo "$1 $2 $3 $4 $5 $6 $7 $8 $9 ${smp_mod}" > /sys/kernel/debug/ieee80211/phy${10}/cls_wifi/irf/irf_smp_config
fi
