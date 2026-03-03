#!/bin/bash

PATH_INA226_SHELL=$(which ina226_i2c.sh)
if [ $PATH_INA226_SHELL != "" ]; then
	RUN_INA226="ina226_triband.sh"
else
	RUN_INA226="./ina226_triband.sh"
fi

R_Shunt_Max=3
if [ $# != $R_Shunt_Max ] ; then
	U0_R_Shunt=5
	U1_R_Shunt=5
	U2_R_Shunt=5
	echo "R_shunt input parameter $# is less than $R_Shunt_Max, using default R_shunt Value!"
else
	U0_R_Shunt=$1
	U1_R_Shunt=$2
	U2_R_Shunt=$3
fi

devmem 0x90420030 32 0x00005019
devmem 0x90420034 32 0x00025019

printf "                  Voltage (mV)      Current (A)       Power (mW) \r\n" > /tmp/ina226_show_log

$RUN_INA226 -U 0 -R $U0_R_Shunt -P $Spidev_Port -N $Spidev_No >> /tmp/ina226_show_log
$RUN_INA226 -U 1 -R $U1_R_Shunt -P $Spidev_Port -N $Spidev_No >> /tmp/ina226_show_log
$RUN_INA226 -U 2 -R $U2_R_Shunt -P $Spidev_Port -N $Spidev_No >> /tmp/ina226_show_log

cat /tmp/ina226_show_log

total_val=0
for line_no in $(seq 2 13)
do
	tmp_val=$(sed -n ' '${line_no}'p ' /tmp/ina226_show_log | awk '{print $4}')
	total_val=$(awk 'BEGIN{printf "%.3f\n",'$tmp_val'+'$total_val'}')
done
printf "total %+53s%s \r\n" $total_val
