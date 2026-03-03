#!/bin/bash
Spidev_Port=0
Spidev_No=1

if [ $# != 9 ] ; then
U0_R_Shunt=20
U1_R_Shunt=10
U2_R_Shunt=20
U3_R_Shunt=10
U4_R_Shunt=10
U5_R_Shunt=5
U6_R_Shunt=10
U7_R_Shunt=10
U8_R_Shunt=10
echo "R_shunt input parameter $# is less than 9, using default R_shunt Value!"
else
U0_R_Shunt=$1
U1_R_Shunt=$2
U2_R_Shunt=$3
U3_R_Shunt=$4
U4_R_Shunt=$5
U5_R_Shunt=$6
U6_R_Shunt=$7
U7_R_Shunt=$8
U8_R_Shunt=$9
fi

spidevX_pinmux() {
       io_set.sh -i SPI -p $1 -n $2
}

get_cpld_version() {
        read_cpld_ver=$(spidev_test --device /dev/spidev$Spidev_Port.0 -s 5000000 -v -p "\x00\x00\x00\xdd\x00\x00\x00\x00" -H | grep RX | awk '{print $8}')
}

PATH_INA226_SHELL=$(which ina226.sh)
if [ $PATH_INA226_SHELL != "" ]; then
	RUN_INA226="ina226.sh"
else
	RUN_INA226="./ina226.sh"
fi

spidevX_pinmux $Spidev_Port $Spidev_No

get_cpld_version

printf "                  Voltage (mV)      Current (A)       Power (mW) \r\n" > /tmp/ina226_show_log

if [ $read_cpld_ver == "41" ] ; then
$RUN_INA226 -U 0 -R $U0_R_Shunt -P $Spidev_Port -N $Spidev_No >> /tmp/ina226_show_log
$RUN_INA226 -U 1 -R $U1_R_Shunt -P $Spidev_Port -N $Spidev_No >> /tmp/ina226_show_log
$RUN_INA226 -U 2 -R $U2_R_Shunt -P $Spidev_Port -N $Spidev_No >> /tmp/ina226_show_log
$RUN_INA226 -U 3 -R $U3_R_Shunt -P $Spidev_Port -N $Spidev_No >> /tmp/ina226_show_log
$RUN_INA226 -U 4 -R $U4_R_Shunt -P $Spidev_Port -N $Spidev_No >> /tmp/ina226_show_log
else
if [ $read_cpld_ver == "01" ] || [ $read_cpld_ver == "31" ] ; then
$RUN_INA226 -U 0 -R $U0_R_Shunt -P $Spidev_Port -N $Spidev_No >> /tmp/ina226_show_log
$RUN_INA226 -U 1 -R $U1_R_Shunt -P $Spidev_Port -N $Spidev_No >> /tmp/ina226_show_log
$RUN_INA226 -U 2 -R $U2_R_Shunt -P $Spidev_Port -N $Spidev_No >> /tmp/ina226_show_log
$RUN_INA226 -U 3 -R $U3_R_Shunt -P $Spidev_Port -N $Spidev_No >> /tmp/ina226_show_log
fi
$RUN_INA226 -U 4 -R $U4_R_Shunt -P $Spidev_Port -N $Spidev_No >> /tmp/ina226_show_log
$RUN_INA226 -U 5 -R $U5_R_Shunt -P $Spidev_Port -N $Spidev_No >> /tmp/ina226_show_log
$RUN_INA226 -U 6 -R $U6_R_Shunt -P $Spidev_Port -N $Spidev_No >> /tmp/ina226_show_log
$RUN_INA226 -U 7 -R $U7_R_Shunt -P $Spidev_Port -N $Spidev_No >> /tmp/ina226_show_log
$RUN_INA226 -U 8 -R $U8_R_Shunt -P $Spidev_Port -N $Spidev_No >> /tmp/ina226_show_log
fi
cat /tmp/ina226_show_log

total_val=0
for line_no in $(seq 2 13)
do
	tmp_val=$(sed -n ' '${line_no}'p ' /tmp/ina226_show_log | awk '{print $4}')
	total_val=$(awk 'BEGIN{printf "%.3f\n",'$tmp_val'+'$total_val'}')
done
printf "total %+53s%s \r\n" $total_val
