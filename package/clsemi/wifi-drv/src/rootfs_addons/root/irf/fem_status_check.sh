#!/bin/bash

if [ $# != 2 ]; then
echo "sh fem_status.sh radio channel"
echo "radio: 2G/5G"
echo "channel: 0/1"
exit
fi

radio=$1
channel=$2

if [ $radio != "2G" ] && [ $radio != "5G" ]; then
echo "radio error, parameter: 2G/5G"
exit
fi

if [ $channel != 0 ] && [ $channel != 1 ]; then
echo "channel error, parameter: 0/1"
exit
fi

if [ $radio == "2G" ] ; then
	if [ $channel == 0 ]; then
		reg=0x4030d820
	else
		reg=0x4030e020
	fi

	reg_val=$(devmem $reg)

	if [ $reg_val == "0x00000000" ]; then
		echo "$radio channel[$channel] FEM: ANT TO RX[FEM RX mode, LNA disable]"

	elif [ $reg_val == "0x11201100" ]; then
		echo "$radio channel[$channel] FEM: Control by WMAC"

	elif [ $reg_val == "0x11111111" ]; then
		echo "$radio channel[$channel] FEM: TX to ANT[FEM TX mode]"

	elif [ $reg_val == "0x22222222" ]; then
		echo "$radio channel[$channel] FEM: ANT to RX[FEM RX mode, LNA enable]"

	else
		echo "$radio channel[$channel] FEM: unknow status, please check your config!"
	fi

	exit
fi

if [ $radio == "5G" ] ; then
	if [ $channel == 0 ]; then
		reg=0x4010d820
	else
		reg=0x4010e020
	fi


	reg_val=$(devmem $reg)

	if [ $reg_val == "0x00000000" ]; then
		echo "$radio channel[$channel] FEM: ANT TO RX[FEM RX mode, LNA disable]"

	elif [ $reg_val == "0x11201100" ]; then
		echo "$radio channel[$channel] FEM: Control by WMAC"

	elif [ $reg_val == "0x11111111" ]; then
		echo "$radio channel[$channel] FEM: TX to ANT[FEM TX mode]"

	elif [ $reg_val == "0x22222222" ]; then
		echo "$radio channel[$channel] FEM: ANT to RX[FEM RX mode, LNA enable]"

	else
		echo "$radio channel[$channel] FEM: unknow status, please check your config!"
	fi


	exit
fi
