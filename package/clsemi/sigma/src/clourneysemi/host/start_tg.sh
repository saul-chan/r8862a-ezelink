#!/bin/bash

tg_sighandler()
{
	echo "signal received"

	for p in $dut_pid $ca_pid
	do
		echo "try to kill $p"
		kill -9 $p > /dev/null 2>&1
	done

	echo "exit"
	exit 1
}

trap 'tg_sighandler' QUIT
trap 'tg_sighandler'  HUP
trap 'tg_sighandler'  INT
trap 'tg_sighandler' TERM

bin_dir="/usr/local/clourneysemi/sigma_ca/tg"
core_dir="/var/log/sigma_ca"
cd $bin_dir

ulimit -c unlimited

cur_dir=`pwd`

echo "$core_dir/core_%e_%p_%s_%c_%P" > /proc/sys/kernel/core_pattern

wfa_ca_bin=wfa_ca_tg
wfa_dut_bin=wfa_dut_tg

pkill -9 -f $wfa_ca_bin
pkill -9 -f $wfa_dut_bin

PATH="$PATH":$bin_dir/scripts
chmod +x $bin_dir/scripts/*
chmod +x $wfa_dut_bin $wfa_ca_bin

echo $PATH


while [ true ]
do

echo "run traffic generator"

pwd

./$wfa_dut_bin lo 9090 &
dut_pid=$!
sleep 1

./$wfa_ca_bin lo 9099 127.0.0.1 9090 &
ca_pid=$!
sleep 1



# monitor processes state

while [ true ]
do
	ps -p $dut_pid	> /dev/null || break
	ps -p $ca_pid > /dev/null || break

	sleep 1
done


echo "some of monitored processes stopped, kill other and do full restart"

for p in $dut_pid $ca_pid
do
	echo "try to kill $p"
	kill -9 $p > /dev/null 2>&1
done


done
