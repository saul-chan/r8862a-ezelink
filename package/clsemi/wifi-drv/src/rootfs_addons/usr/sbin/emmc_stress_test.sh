#!/bin/bash

usage() {
        printf "Usage: %s" "$(basename "$0")"
        printf"\n\t-l ==> Set Emmc Stress Test Loop Times"
        printf "\n\t-s ==> Set Emmc Stress Test each Loop delay time"
        printf "\n\r"
        exit 1
}

EMMC_TEST_CASE=" 1 2 3 4 5 6 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 "
#EMMC_TEST_CASE=" 41 43 41 43 41 43 41 43 41 43 41 43 41 43 41 43 41 43 41 43 "

LOOPTIMES=10
DELAYSECOND=1

emmc_stress_test() {
	echo "0x90420030: $(devmem 0x90420030)"
	echo "0x90420034: $(devmem 0x90420034)"
	echo "0x90420038: $(devmem 0x90420038)"
	echo "0x9042003c: $(devmem 0x9042003c)"
	echo "0x90420040: $(devmem 0x90420040)"
	echo "0x90420044: $(devmem 0x90420044)"
	echo "0x90420048: $(devmem 0x90420048)"
	echo "0x9042004c: $(devmem 0x9042004c)"
	echo "0x90420050: $(devmem 0x90420050)"
	echo "0x90420054: $(devmem 0x90420054)"
	echo "0x8c00002c: $(devmem 0x8c00002c)"
	echo "0x8c000510: $(devmem 0x8c000510)"
        for case_no in $EMMC_TEST_CASE;
        do
                echo "$case_no" > /sys/kernel/debug/mmc0/mmc0:0001/test
		#ina226_show.sh
		echo "0x8c00002c: $(devmem 0x8c00002c)"
		cat  /sys/kernel/debug/cls_tsens/tsens_cmd
                #echo "Test Case:$case_no"
        done
	sleep 1
}


emmc_stress_cycle_test() {
        count=0
        while true
        do
		echo "============EMMC STRESS LOOP $count ============="
		emmc_stress_test
		sleep $1
		count=$((count + 1))
		if [ ${count} -eq ${LOOPTIMES} ]; then
			echo "============EMMC STRESS Test END============"
			break;
		fi
        done
}

while getopts "l:s:t:" OPTION
do
	case $OPTION in
		l )
			LOOPTIMES=$OPTARG
			;;
		s )
			DELAYSECOND=$OPTARG
			;;
		t )
			TEST_CASE=$OPTARG
			;;
		* )
			echo "Invalid option passed to '$0' (options:$*)"
			usage;;
	esac
done

echo "============EMMC STRESS Test START============"

#echo "file drivers/mmc/core/core.c +p" > /sys/kernel/debug/dynamic_debug/control
echo mmc0:0001 > /sys/bus/mmc/drivers/mmcblk/unbind
echo mmc0:0001 > /sys/bus/mmc/drivers/mmc_test/bind

emmc_stress_cycle_test $DELAYSECOND

#reboot
