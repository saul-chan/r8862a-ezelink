#!/bin/bash

bw=$1
enable_send=$2
mount -t debugfs none /sys/kernel/debug
calcmd set radio 5G

if [ $bw == "20M" ]; then
    calcmd set bss bw 20
elif [ $bw == "40M" ]; then
    calcmd set bss bw 40
elif [ $bw == "80M" ]; then
    calcmd set bss bw 80
elif [ $bw == "160M" ]; then
    calcmd set bss bw 160
else
    echo "bandwidth error!" 
    return 1
fi
sleep 1

calcmd update

sleep 10

#QUC Bypass
devmem 0x40420284 32 1

# 退模拟功率
devmem 0x4040a008 32 0x371F1301
echo "TX Power setting done"

# 停止发数
sh /root/irf/irf_send_stop.sh 0xf 1 0 -1

#TX always ON, close LNA and RXON
devmem 0x4010D820 32 0x11111111

# Config sending data for TXEQ Path
devmem 0x40080000 32 0
devmem 0x4042c06c 32 1
devmem 0x404201c4 32 4
devmem 0x4042c070 32 0
devmem 0x4042c050 32 3

# TXEQ send data
if [ $bw == "20M" -o $bw == "40M" ]; then
    sh /root/irf/irf_send_config.sh 0 1 0 0x10 8192 0 5000 5000 5000 0 /root/irf/Tx_160M_PN_AddNoise-17dBfs.dat
else
    sh /root/irf/irf_send_config.sh 0 0 0 0x10 8192 0 5000 5000 5000 0 /root/irf/Tx_160M_PN_AddNoise-17dBfs.dat
fi
sleep 1
sh /root/irf/irf_send_start.sh 0x1 1 0 -1
sleep 2
echo "sending TXEQ data done"

# DIF test mode
calcmd irf mode 1
sleep 5
calcmd irf task 0 1 init
sleep 5
calcmd irf task 0 1 looppaout
sleep 10
echo "start ffeq task"
calcmd irf task 0 1 ffEq
sleep 30
calcmd irf table 0 0 5250 3 0
sleep 5

# Enable DCC
devmem 0x4c82c0a4 32 0x1

# Enable CGL
devmem 0x4042c098 32 0x1

# Enable LMS
devmem 0x4042c098 32 0x1

# Bypass PDCore
devmem 0x404201c4 32 0x1
echo "DPD configuration done!"
echo "Start sending CFR data"

devmem 0x40080000 32 1
if [ $bw == "20M" ]; then
    devmem 0x40000010 32 0x200054
elif [ $bw == "40M" ]; then
    devmem 0x40000010 32 0x2000d5
elif [ $bw == "80M" ]; then
    devmem 0x40000010 32 0x2003D6
elif [ $bw == "160M" ]; then
    devmem 0x40080008 32 0xa80
    devmem 0x40088008 32 0xa80
    devmem 0x40000010 32 0x203FD7
else
    echo "bandwidth error!" 
fi
devmem 0x404201c4 32 0
devmem 0x4042c06c 32 0
devmem 0x4042c070 32 1
devmem 0x4042c050 32 0x3c

sh /root/irf/irf_send_stop.sh 0xf 1 0 -1

if [ $enable_send == 1 ]; then
    sleep 1

    if [ $bw == "20M" ]; then
        sh /root/irf/irf_send_config.sh 0 4 0 0x10 16384 0 5000 5000 5000  0 /root/irf/CFRIn_With_DataIndStartAt2000.dat
    elif [ $bw == "40M" ]; then
        sh /root/irf/irf_send_config.sh 0 3 0 0x10 16384 0 5000 5000 5000  0 /root/irf/CFRIn_With_DataIndStartAt2000.dat
    elif [ $bw == "80M" ]; then
        sh /root/irf/irf_send_config.sh 0 2 0 0x10 16384 0 5000 5000 5000  0 /root/irf/CFRIn_With_DataIndStartAt2000.dat
    elif [ $bw == "160M" ]; then
        sh /root/irf/irf_send_config.sh 0 1 0 0x10 16384 0 5000 5000 5000  0 /root/irf/CFRIn_With_DataIndStartAt2000.dat
    else
        echo "bandwidth error!"
    fi
    sleep 1

    sh /root/irf/irf_send_start.sh 0x1 1 0 -1
    echo "CFR preparation done!"
fi

sleep 5

sh /root/irf/irf_send_stop.sh 0xf 1 0 -1
sleep 1

devmem 0x4042C070 32 1
if [ $bw == "20M" ]; then
    devmem 0x40080000 32 0x190
else
    devmem 0x40080000 32 0xB0
fi
devmem 0x40088000 32 1
devmem 0x404201c4 32 1
devmem 0x4010D820 32 0x11201100
devmem 0x4008000c 32 0x5a67
devmem 0x4042b000 32 0xa5294a
#devmem 0x4ac00010 32 0xc40
devmem 0x4040a064 32 0x00300201
devmem 0x4040a068 32 0x00300201

echo "DPD preparation done!"
