#!/bin/sh
antenna=$1
freq=$2
bw=$3
stage=$4

#reference power config, change it when needed
#calcmd irf tx_power $antenna $power

#load dif data source
calcmd irf load 0 0

#enter equipment mode
calcmd irf mode 1

#start calibration
calcmd irf dif_equip $antenna $freq $bw $stage 0x2
