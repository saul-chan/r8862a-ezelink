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

#0x00--INA226_0	  ; 	AVDD18_ABB
#0x01--INA226_1	  ;	AVDD18_RF
#0x02--INA226_2	  ;	AVDD18_CKI
#0x03--INA226_3	  ;	AVDD18_VCO
#0x04--INA226_4	  ;	AVDD12_ABB
#0x05--INA226_5	  ;	V0V8_DCDC
#0x06--INA226_6	  ; 	DDR_VDDQ
#0x07--INA226_7	  ;	V1V8_DCDC
#0x08--INA226_8	  ;	V3V3_DCDC

#Unit mV
Bus_Voltage="1.25"
#Unit mV
Shunt_Voltage="0.0025"
#Unit mR
R_Shunt=""
Spidev_Port=0

check_ina226_num_bus() {
	if [ $1 -lt 0 ] || [ $1 -gt 8 ]; then
		usage
	fi
	ina226_num=$(printf "%0d" $1)
	case $ina226_num in
		"0" )
			BUS_NAME="AVDD18_ABB"
			R_Shunt=20
			;;
		"1" )
			BUS_NAME="AVDD18_RF"
			R_Shunt=10
			;;
		"2" )
			BUS_NAME="AVDD18_CKI"
			R_Shunt=20
			;;
		"3" )
			BUS_NAME="AVDD18_VCO"
			R_Shunt=10
			;;
		"4" )
			BUS_NAME="AVDD12_ABB"
			R_Shunt=10
			;;
		"5" )
			BUS_NAME="V0V8_DCDC"
			R_Shunt=5
			;;
		"6" )
			BUS_NAME="DDR_VDDQ"
			R_Shunt=10
			;;
		"7" )
			BUS_NAME="V1V8_DCDC"
			R_Shunt=10
			;;
		"8" )
			BUS_NAME="V3V3_DCDC "
			R_Shunt=10
			;;
		* ) echo "Invalid option passed to '$0' ina226_num"
			usage;;
	esac
}

check_evb_c_ina226_num_bus() {
	if [ $1 -lt 0 ] || [ $1 -gt 4 ]; then
		usage
	fi
	ina226_num=$(printf "%0d" $1)
	case $ina226_num in
		"0" )
			BUS_NAME="DDR_VDDQ"
			R_Shunt=10
			;;
		"1" )
			BUS_NAME="V0V8_DCDC"
			R_Shunt=5
			;;
		"2" )
			BUS_NAME="V5V0_DCDC"
			R_Shunt=10
			;;
		"3" )
			BUS_NAME="V1V8_DCDC"
			R_Shunt=10
			;;
		"4" )
			BUS_NAME="AVDD12_ABB"
			R_Shunt=10
			;;
		* ) echo "Invalid option passed to '$0' ina226_num"
			usage;;
	esac
}

INA226_SET_SAMPLE_TIMES() {
	ina226_num=$(printf "%02x" $1)
	bus_name=$2
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x01\x00\x00\x00\x00" -H 2 > /dev/null
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x02\x00\x00\x00\x$ina226_num" -H 2 > /dev/null
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x03\x00\x00\x00\x00" -H 2 > /dev/null
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x04\x00\x00\x4B\x27" -H 2 > /dev/null
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x01" -H 2 > /dev/null
	usleep 600
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x00" -H 2 > /dev/null
	usleep 200
}

INA226_READ_BUSVOL() {
	ina226_num=$(printf "%02x" $1)
	bus_name=$2
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x01\x00\x00\x00\x01" -H 2 > /dev/null
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x02\x00\x00\x00\x$ina226_num" -H 2 > /dev/null
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x03\x00\x00\x00\x02" -H 2 > /dev/null
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x01" -H 2 > /dev/null
	usleep 150
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x00" -H 2 > /dev/null
	read_bus_hval=$(spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x00\x00\x00\x06\x00\x00\x00\x00" -H | grep RX | awk '{print $9}')
	read_bus_lval=$(spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x00\x00\x00\x06\x00\x00\x00\x00" -H | grep RX | awk '{print $10}')
	read_bus_value=0x"$read_bus_hval$read_bus_lval"
	if [ $read_bus_value == "0xFFFF" ] || [ $read_bus_hval == "" ] || [ $read_bus_lval == "" ] ; then
		echo "$SPIDEV_NODE_NAME doesn't work, read_bus_voltage $read_bus_value"
		exit 1
	fi
	print_bus_value=$(echo "$(printf "%d" $read_bus_value) * 1.25" | bc)
	#echo "$bus_name Bus Voltage = $print_bus_value mV"
}

INA226_READ_SHUVOL() {
	ina226_num=$(printf "%02x" $1)
	bus_name=$2
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x01\x00\x00\x00\x01" -H 2 > /dev/null
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x02\x00\x00\x00\x$ina226_num" -H 2 > /dev/null
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x03\x00\x00\x00\x01" -H 2 > /dev/null
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x01" -H 2 > /dev/null
	usleep 150
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x00" -H 2 > /dev/null
	read_shunt_hval=$(spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x00\x00\x00\x06\x00\x00\x00\x00" -H | grep RX | awk '{print $9}')
	read_shunt_lval=$(spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x00\x00\x00\x06\x00\x00\x00\x00" -H | grep RX | awk '{print $10}')
	read_shunt_value=0x"$read_shunt_hval$read_shunt_lval"
	if [ $read_bus_value == "0xFFFF" ] || [ $read_shunt_hval == "" ] || [ $read_shunt_lval == "" ] ; then
		echo "$SPIDEV_NODE_NAME doesn't work, read_shunt_voltage $read_shunt_value"
		exit 1
	fi
	print_shunt_value=$(echo "$(printf "%d" $read_shunt_value) * 0.0025" | bc)
	#echo "$bus_name Shunt Voltage = $print_shunt_value mV"
}

get_cpld_version() {
        read_cpld_ver=$(spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x00\x00\x00\xdd\x00\x00\x00\x00" -H | grep RX | awk '{print $8}')
}

while getopts "R:U:P:N:" OPTION
do
	case $OPTION in
	U )   INA226_NUM="$OPTARG"
		SPIDEV_NODE_NAME="/dev/spidev""$Spidev_Port.0"
		get_cpld_version
		if [ $read_cpld_ver == "41" ] ; then
			check_evb_c_ina226_num_bus $INA226_NUM
		else
			check_ina226_num_bus $INA226_NUM
		fi
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

SPIDEV_NODE_NAME="/dev/spidev""$Spidev_Port.0"
get_cpld_version

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
printf "%-16s  %-16s  %-16s  %s \r\n" $bus_name $print_bus_value $shunt_current $bus_power

