#!/bin/bash

band=$1
ch=$2
bw=$3

if [ $# == 4 ]; then
	target_power=$4
else
	if [ $band == "5G" ]; then
		target_power=21
	else
		if [ $bw == "20" ]; then
			target_power=22
		else
			target_power=21
		fi
	fi
fi

if [ $ch != 0 ] && [ $ch != 1 ]; then
echo "ch error, ch : 0/1"
exit
fi

echo "0" >/sys/kernel/debug/ieee80211/phy0/cls_wifi/irf/g_pwr_ctrl_en
calcmd irf status $ch 7 0
calcmd irf task $ch 1 loopPaout
calcmd irf loop_power_init $ch
calcmd irf tx_power $ch $target_power
usleep 50000
calcmd irf status 0 2 0xf
calcmd irf status 0 0 0
usleep 100000
calcmd irf status 0 0 0

for i in `seq 10`
do
    calcmd irf status $ch 7 0
	usleep 100000
	calcmd irf tx_loop_power $ch

	if [ $band == "5G" ]; then
	    ret=`logread | grep fmac1 | tail -n 5 | grep "loop close" | wc -l`
    else
	    ret=`logread | grep fmac0 | tail -n 5 | grep "loop close" | wc -l`
	fi
	if [ $ret -eq 1 ] ; then
		break
	fi
done

calcmd irf task $ch 1 fbFuncPD

if [ $ch == "0" ]; then
	calcmd irf task 0 1 loopPaout
	calcmd irf status 0 7 0
	calcmd irf status 0 0 0
elif [ $ch == "1" ]; then
	calcmd irf task 1 1 loopPaout
	calcmd irf status 1 7 0
	calcmd irf status 0 0 0
else
	echo "ch error!"
	return 1
fi

echo "loopPaout done"

if [ $ch == "0" ]; then
	for i in `seq 10`
	do
		calcmd irf task 0 1 fbDelay_s1
		sleep 1
		calcmd irf task 0 1 fbDelay_s2
		if [ $band == "5G" ]; then
		    ret=`logread | grep fmac1 | tail -n 5 | grep "fbDelay_s2 run success" | wc -l`
		else
		    ret=`logread | grep fmac0 | tail -n 5 | grep "fbDelay_s2 run success" | wc -l`
		fi
		if [ $ret -eq 1 ] ; then
			break
		fi
	done

elif [ $ch == "1" ]; then
	for i in `seq 10`
	do
		calcmd irf task 1 1 fbDelay_s1
		sleep 1
		calcmd irf task 1 1 fbDelay_s2
		if [ $band == "5G" ]; then
		    ret=`logread | grep fmac1 | tail -n 5 | grep "fbDelay_s2 run success" | wc -l`
		else
		    ret=`logread | grep fmac0 | tail -n 5 | grep "fbDelay_s2 run success" | wc -l`
		fi
		if [ $ret -eq 1 ] ; then
			break
		fi
	done

else
	echo "ch error!"
	return 1
fi

echo "fbDelay done"

#CGL移位
if [ $band == "5G" ]; then

	if [ $ch == "0" ]; then
		devmem 0x40424804 32 5
	elif [ $ch == "1" ]; then
		devmem 0x40426804 32 5
	fi

elif [ $band == "2G" ]; then

	if [ $ch == "0" ]; then
		devmem 0x40524804 32 5
	elif [ $ch == "1" ]; then
		devmem 0x40526804 32 5
	fi

else
	echo "band error!"
	return 1
fi

if [ $ch == "0" ]; then
	calcmd irf task 0 1 fbCgl_s1
	sleep 1
	calcmd irf task 0 1 fbCgl_s2

elif [ $ch == "1" ]; then
	calcmd irf task 1 1 fbCgl_s1
	sleep 1
	calcmd irf task 1 1 fbCgl_s2

else
	echo "ch error!"
	return 1
fi

echo "fbCgl done"

if [ $band == "5G" ]; then

	if [ $ch == "0" ]; then
		#PD err(pd in)
		devmem 0x40424810 32 1
		#LMS model
		devmem 0x404201bc 32 0x34567
		devmem 0x404201c0 32 0x34567
		devmem 0x4042a000 32 0x76543
		devmem 0x4042a004 32 0x1f76543
		#block size
		devmem 0x4042a034 32 0x120
		#u
		devmem 0x4042a058 32 0x842108
		echo "set 5G dpd success!"
	elif [ $ch == "1" ]; then
		#PD err(pd in)
		devmem 0x40426810 32 1
		#LMS model
		devmem 0x404221bc 32 0x34567
		devmem 0x404221c0 32 0x34567
		devmem 0x4042b000 32 0x76543
		devmem 0x4042b004 32 0x1f76543
		#block size
		devmem 0x4042b034 32 0x120
		#u
		devmem 0x4042b058 32 0x842108
		echo "set 5G dpd success!"
	fi

elif [ $band == "2G" ]; then

	if [ $ch == "0" ]; then
		#PD err(pd in)
		devmem 0x40524810 32 1
		#LMS model
		devmem 0x405201bc 32 0x34567
		devmem 0x405201c0 32 0x34567
		devmem 0x4052a000 32 0x76543
		devmem 0x4052a004 32 0x1f76543
		#block size
		devmem 0x4052a034 32 0x120
		#u
		devmem 0x4052a058 32 0x842108
		echo "set 2G dpd success!"
	elif [ $ch == "1" ]; then
		#PD err(pd in)
		devmem 0x40526810 32 1
		#LMS model
		devmem 0x405221bc 32 0x34567
		devmem 0x405221c0 32 0x34567
		devmem 0x4052b000 32 0x76543
		devmem 0x4052b004 32 0x1f76543
		#block size
		devmem 0x4052b034 32 0x120
		#u
		devmem 0x4052b058 32 0x842108
		echo "set 2G dpd success!"
	fi

else
	echo "band error!"
	return 1
fi

#启动PD task前打开CGL
if [ $band == "5G" ]; then
	if [ $ch == "0" ]; then
		devmem 0x4042c098 32 0x1
	elif [ $ch == "1" ]; then
		devmem 0x4042c198 32 0x1
	fi
elif [ $band == "2G" ]; then
	if [ $ch == "0" ]; then
		devmem 0x4052c098 32 0x1
	elif [ $ch == "1" ]; then
		devmem 0x4052c198 32 0x1
	fi
fi

if [ $ch == "0" ]; then
	calcmd irf task 0 1 pd_s1
	usleep 500000
	calcmd irf task 0 1 pd_s2

elif [ $ch == "1" ]; then
	calcmd irf task 1 1 pd_s1
	usleep 500000
	calcmd irf task 1 1 pd_s2

else
	echo "ch error!"
	return 1
fi

calcmd irf status 0 2 0

echo "1" >/sys/kernel/debug/ieee80211/phy0/cls_wifi/irf/g_pwr_ctrl_en
echo "DPD task done!"
