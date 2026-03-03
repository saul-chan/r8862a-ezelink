#!/bin/bash

usage() {
	printf "Usage: %s" "$(basename "$0")"

	printf "\n\t-U ==> INA226Q Device's INA226_NUM"
	printf "\n\t-R ==> R_Shunt Sample Resistance Value 5mR"
	printf "\n\t-P ==> SPI Interface Port Number"
	printf "\n\t-N ==> SPI Interface Pad Number"
	printf "\n\r"
	exit 1
}

#D2K + M2K Tri-band Board
#0x00--INA226_0   ;     DDR_VDDQ
#0x01--INA226_1   ;     V0V8_DCDC
#0x02--INA226_2   ;     V5V0_DCDC
#0x03--INA226_3   ;     V1V8_DCDC
#0x04--INA226_4   ;     AVDD12_ABB

#Unit mV
Bus_Voltage="1.25"
#Unit mV
Shunt_Voltage="0.0025"
#Unit mR
R_Shunt=""
i2cdev_Port=0
i2cdev_addr="0x40"

check_i2c_ina226_num_bus() {
	if [ $1 -lt 0 ] || [ $1 -gt 4 ]; then
		usage
	fi
	ina226_num=$(printf "%0d" $1)
	case $ina226_num in
		"0" )
			BUS_NAME="DDR_VDDQ"
			R_Shunt=10
			i2cdev_Port=0
			i2cdev_addr="0x40"
			;;
		"1" )
			BUS_NAME="V0V8_DCDC"
			R_Shunt=5
			i2cdev_Port=0
			i2cdev_addr="0x41"
			;;
		"2" )
			BUS_NAME="V5V0_DCDC"
			R_Shunt=10
			i2cdev_Port=0
			i2cdev_addr="0x42"
			;;
		"3" )
			BUS_NAME="V1V8_DCDC"
			R_Shunt=10
			i2cdev_Port=0
			i2cdev_addr="0x44"
			;;
		"4" )
			BUS_NAME="AVDD12_ABB"
			R_Shunt=10
			i2cdev_Port=0
			i2cdev_addr="0x45"
			;;
		* ) echo "Invalid option passed to '$0' ina226_num"
			usage;;
	esac
}

INA226_SET_SAMPLE_TIMES() {
	i2ctransfer -f -y $i2cdev_Port w3@$i2cdev_addr 0x00 0x4B 0x27
	usleep 50000
}

INA226_READ_BUSVOL() {
	read_bus_vol=$(i2ctransfer -f -y $i2cdev_Port w1@$i2cdev_addr 0x02 r2)
	read_bus_value=0x$(printf "%02X%02X" $read_bus_vol)
	if [ "$read_bus_value" == "0xFFFF" ]; then
		echo "i2c_$i2cdev_Port doesn't work, read_bus_voltage $read_bus_value"
		exit 1
	fi
	print_bus_value=$(echo "$(printf "%d" $read_bus_value) * 1.25" | bc)
	usleep 50000
}

INA226_READ_SHUVOL() {
	read_shunt_vol=$(i2ctransfer -f -y $i2cdev_Port w1@$i2cdev_addr 0x01 r2)
	read_shunt_value=0x$(printf "%02X%02X" $read_shunt_vol)
	if [ "$read_bus_value" == "0xFFFF" ]; then
		echo "i2c_$i2cdev_Port doesn't work, read_shunt_voltage $read_shunt_value"
		exit 1
	fi
	print_shunt_value=$(echo "$(printf "%d" $read_shunt_value) * 0.0025" | bc)
	usleep 50000
}

while getopts "R:U:P:N:" OPTION
do
	case $OPTION in
	U )   INA226_NUM="$OPTARG"
		check_i2c_ina226_num_bus $INA226_NUM
		;;
	R )   R_Shunt="$OPTARG"
		;;
	P )   Spidev_Port="$OPTARG"
		;;
	N )   Spidev_No="$OPTARG"
		;;
	* ) echo "Invalid option passed to '$0' (options:$*)"
		usage;;
	esac
done

#echo "INA226 $BUS_NAME R_Shunt $R_Shunt mR"
INA226_SET_SAMPLE_TIMES $INA226_NUM $BUS_NAME
INA226_READ_BUSVOL $INA226_NUM $BUS_NAME
INA226_READ_SHUVOL $INA226_NUM $BUS_NAME
shunt_current=$(awk 'BEGIN{printf "%.3f\n",'$print_shunt_value'/'$R_Shunt'}')
#echo "$bus_name Current = $shunt_current A"
#bus_power=$(echo "$shunt_current * $print_bus_value" | bc)
bus_power=$(awk 'BEGIN{printf "%.3f\n",'$shunt_current'*'$print_bus_value'}')
#echo "$bus_name Power = $bus_power mW"

#printf "                  Voltage (mV)      Current (A)       Power (mW) \r\n"
printf "%-16s  %-16s  %-16s  %s \r\n" $BUS_NAME $print_bus_value $shunt_current $bus_power

