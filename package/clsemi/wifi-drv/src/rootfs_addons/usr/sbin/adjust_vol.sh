#!/bin/bash

usage() {
	printf "Usage: %s" "$(basename "$0")"
	printf "\n\t-D ==> Show Voltage Debug Log"
	printf "\n\t-N ==> Voltage BUS's NAME"
	printf "\n\t-V ==> Set BUS's Voltage"
	printf "\n\t-R ==> Read BUS's Voltage"
	printf "\n\t-p ==> SPI Interface Port Number"
	printf "\n\t-n ==> SPI Interface Pad Number"
	printf "\n\r"
	exit 1
}

#AVDD18_VCO
#AVDD18_CKI
#AVDD12_ABB
#AVDD18_RF
#V0V8_DCDC
#DDR_VDDQ
#V3V3_DCDC
#V1V8_DCDC

ISP1220_DC_3V3_BASE=0xFF
ISP1220_DC_1V8_BASE=0xFF
ISP1220_DC_1V2_BASE=0xFF
ISP1220_DC_1V5_BASE=0xFF
ISP1220_DC_0V8_BASE=0xFF

#ISP1220_Unit = (2 * Vol * 15%) / 255
ISP1220_3V3_Unit=0.003901
ISP1220_1V8_Unit=0.002114
ISP1220_1V2_Unit=0.001412
ISP1220_1V5_Unit=0.001764
ISP1220_0V8_Unit=0.000939

ISP1220_3V3_Float=$(awk 'BEGIN{printf "%.6f\n", 3.3161 * 0.15}')
ISP1220_1V8_Float=$(awk 'BEGIN{printf "%.6f\n", 1.7973 * 0.15}')
ISP1220_1V2_Float=$(awk 'BEGIN{printf "%.6f\n", 1.200 * 0.15}')
ISP1220_1V5_Float=$(awk 'BEGIN{printf "%.6f\n", 1.500 * 0.15}')
ISP1220_0V8_Float=$(awk 'BEGIN{printf "%.6f\n", 0.798 * 0.15}')

ISP1220_3V3_Vol=3.3161
ISP1220_1V8_Vol=1.7973
ISP1220_1V2_Vol=1.2000
ISP1220_1V5_Vol=1.5000
ISP1220_0V8_Vol=0.7980

Spidev_Port=0
Spidev_No=1
Voltage_Debug=0

spidevX_pinmux() {
	io_set.sh -i SPI -p $1 -n $2
}

# AVDD18_VCO
ISP1220_TRIM1_SETVOL () {
	vol_value=$(printf "%02x" $1)
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x01\x00\x00\x00\x00" -H 2 > /dev/null
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x02\x00\x00\x00\x0F" -H 2 > /dev/null
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x03\x00\x00\x00\x13" -H 2 > /dev/null
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x04\x00\x00\x00\x$vol_value " -H 2 > /dev/null
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x01" -H 2 > /dev/null
	usleep 150
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x00" -H 2 > /dev/null
}

ISP1220_TRIM1_GETVOL () {
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x01\x00\x00\x00\x01" -H 2 > /dev/null
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x02\x00\x00\x00\x0F" -H 2 > /dev/null
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x03\x00\x00\x00\x13" -H 2 > /dev/null
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x01" -H 2 > /dev/null
	usleep 150
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x00" -H 2 > /dev/null
	isp1220_trim1_val=0x$(spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x00\x00\x00\x06\x00\x00\x00\x00" -H | grep RX | awk '{print $10}')
	print_vol_value=$(awk 'BEGIN{printf "%.2f\n", (('$ISP1220_DC_1V8_BASE' - '$isp1220_trim1_val') * '$ISP1220_1V8_Unit' + '$ISP1220_1V8_Vol' - '$ISP1220_1V8_Float') * '1000'}')
	printf "%-18s  %-8s  %-8s\r\n" $1 $isp1220_trim1_val $print_vol_value
}

# AVDD18_CKI
ISP1220_TRIM2_SETVOL () {
	vol_value=$(printf "%02x" $1)
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x01\x00\x00\x00\x00" -H 2 > /dev/null
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x02\x00\x00\x00\x0F" -H 2 > /dev/null
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x03\x00\x00\x00\x14" -H 2 > /dev/null
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x04\x00\x00\x00\x$vol_value " -H 2 > /dev/null
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x01" -H 2 > /dev/null
	usleep 150
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x00" -H 2 > /dev/null
}

ISP1220_TRIM2_GETVOL () {
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x01\x00\x00\x00\x01" -H 2 > /dev/null
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x02\x00\x00\x00\x0F" -H 2 > /dev/null
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x03\x00\x00\x00\x14" -H 2 > /dev/null
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x01" -H 2 > /dev/null
	usleep 150
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x00" -H 2 > /dev/null
	isp1220_trim2_val=0x$(spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x00\x00\x00\x06\x00\x00\x00\x00" -H | grep RX | awk '{print $10}')
	print_vol_value=$(awk 'BEGIN{printf "%.2f\n", (('$ISP1220_DC_1V8_BASE' - '$isp1220_trim2_val') * '$ISP1220_1V8_Unit' + '$ISP1220_1V8_Vol' - '$ISP1220_1V8_Float') * '1000'}')
	printf "%-18s  %-8s  %-8s\r\n" $1 $isp1220_trim2_val $print_vol_value
}

# AVDD12_ABB
ISP1220_TRIM3_SETVOL () {
	vol_value=$(printf "%02x" $1)
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x01\x00\x00\x00\x00" -H 2 > /dev/null
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x02\x00\x00\x00\x0F" -H 2 > /dev/null
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x03\x00\x00\x00\x15" -H 2 > /dev/null
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x04\x00\x00\x00\x$vol_value " -H 2 > /dev/null
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x01" -H 2 > /dev/null
	usleep 150
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x00" -H 2 > /dev/null
}

ISP1220_TRIM3_GETVOL () {
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x01\x00\x00\x00\x01" -H 2 > /dev/null
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x02\x00\x00\x00\x0F" -H 2 > /dev/null
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x03\x00\x00\x00\x15" -H 2 > /dev/null
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x01" -H 2 > /dev/null
	usleep 150
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x00" -H 2 > /dev/null
	isp1220_trim3_val=0x$(spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x00\x00\x00\x06\x00\x00\x00\x00" -H | grep RX | awk '{print $10}')
	print_vol_value=$(awk 'BEGIN{printf "%.2f\n", (('$ISP1220_DC_1V2_BASE' - '$isp1220_trim3_val') * '$ISP1220_1V2_Unit' + '$ISP1220_1V2_Vol' - '$ISP1220_1V2_Float') * '1000'}')
	printf "%-18s  %-8s  %-8s\r\n" $1 $isp1220_trim3_val $print_vol_value
}

# AVDD18_RF
ISP1220_TRIM4_SETVOL () {
	vol_value=$(printf "%02x" $1)
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x01\x00\x00\x00\x00" -H 2 > /dev/null
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x02\x00\x00\x00\x0F" -H 2 > /dev/null
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x03\x00\x00\x00\x16" -H 2 > /dev/null
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x04\x00\x00\x00\x$vol_value " -H 2 > /dev/null
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x01" -H 2 > /dev/null
	usleep 150
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x00" -H 2 > /dev/null
}

ISP1220_TRIM4_GETVOL () {
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x01\x00\x00\x00\x01" -H 2 > /dev/null
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x02\x00\x00\x00\x0F" -H 2 > /dev/null
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x03\x00\x00\x00\x16" -H 2 > /dev/null
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x01" -H 2 > /dev/null
	usleep 150
	spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x00" -H 2 > /dev/null
	isp1220_trim4_val=0x$(spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x00\x00\x00\x06\x00\x00\x00\x00" -H | grep RX | awk '{print $10}')
	print_vol_value=$(awk 'BEGIN{printf "%.2f\n", (('$ISP1220_DC_1V8_BASE' - '$isp1220_trim4_val') * '$ISP1220_1V8_Unit' + '$ISP1220_1V8_Vol' - '$ISP1220_1V8_Float') * '1000'}')
	printf "%-18s  %-8s  %-8s\r\n" $1 $isp1220_trim4_val $print_vol_value
}

# V0V8_DCDC
ISP1220_TRIM5_SETVOL () {
        vol_value=$(printf "%02x" $1)
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x01\x00\x00\x00\x00" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x02\x00\x00\x00\x0F" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x03\x00\x00\x00\x17" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x04\x00\x00\x00\x$vol_value " -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x01" -H 2 > /dev/null
        usleep 150
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x00" -H 2 > /dev/null
}

ISP1220_TRIM5_GETVOL () {
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x01\x00\x00\x00\x01" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x02\x00\x00\x00\x0F" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x03\x00\x00\x00\x17" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x01" -H 2 > /dev/null
        usleep 150
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x00" -H 2 > /dev/null
        isp1220_trim5_val=0x$(spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x00\x00\x00\x06\x00\x00\x00\x00" -H | grep RX | awk '{print $10}')
	print_vol_value=$(awk 'BEGIN{printf "%.2f\n", (('$ISP1220_DC_0V8_BASE' - '$isp1220_trim5_val') * '$ISP1220_0V8_Unit' + '$ISP1220_0V8_Vol' - '$ISP1220_0V8_Float') * '1000' }')
        printf "%-18s  %-8s  %-8s\r\n" $1 $isp1220_trim5_val $print_vol_value
}

# DDR_VDDQ
ISP1220_TRIM6_SETVOL () {
        vol_value=$(printf "%02x" $1)
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x01\x00\x00\x00\x00" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x02\x00\x00\x00\x0F" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x03\x00\x00\x00\x18" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x04\x00\x00\x00\x$vol_value " -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x01" -H 2 > /dev/null
        usleep 150
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x00" -H 2 > /dev/null
}

ISP1220_TRIM6_GETVOL () {
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x01\x00\x00\x00\x01" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x02\x00\x00\x00\x0F" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x03\x00\x00\x00\x18" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x01" -H 2 > /dev/null
        usleep 150
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x00" -H 2 > /dev/null
        isp1220_trim6_val=0x$(spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x00\x00\x00\x06\x00\x00\x00\x00" -H | grep RX | awk '{print $10}')
	print_vol_value=$(awk 'BEGIN{printf "%.2f\n", (('$ISP1220_DC_1V5_BASE' - '$isp1220_trim6_val') * '$ISP1220_1V2_Unit' + '$ISP1220_1V2_Vol' - '$ISP1220_1V2_Float') * '1000'}')
        printf "%-18s  %-8s  %-8s\r\n" $1 $isp1220_trim6_val $print_vol_value
}

# V3V3_DCDC
ISP1220_TRIM7_SETVOL () {
        vol_value=$(printf "%02x" $1)
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x01\x00\x00\x00\x00" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x02\x00\x00\x00\x0F" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x03\x00\x00\x00\x19" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x04\x00\x00\x00\x$vol_value " -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x01" -H 2 > /dev/null
        usleep 150
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x00" -H 2 > /dev/null
}

ISP1220_TRIM7_GETVOL () {
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x01\x00\x00\x00\x01" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x02\x00\x00\x00\x0F" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x03\x00\x00\x00\x19" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x01" -H 2 > /dev/null
        usleep 150
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x00" -H 2 > /dev/null
        isp1220_trim7_val=0x$(spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x00\x00\x00\x06\x00\x00\x00\x00" -H | grep RX | awk '{print $10}')
	print_vol_value=$(awk 'BEGIN{printf "%.2f\n", (('$ISP1220_DC_3V3_BASE' - '$isp1220_trim7_val') * '$ISP1220_3V3_Unit' + '$ISP1220_3V3_Vol' - '$ISP1220_3V3_Float') * '1000'}')
        printf "%-18s  %-8s  %-8s\r\n" $1 $isp1220_trim7_val $print_vol_value
}

# V1V8_DCDC
ISP1220_TRIM8_SETVOL () {
        vol_value=$(printf "%02x" $1)
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x01\x00\x00\x00\x00" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x02\x00\x00\x00\x0F" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x03\x00\x00\x00\x1A" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x04\x00\x00\x00\x$vol_value " -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x01" -H 2 > /dev/null
        usleep 150
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x00" -H 2 > /dev/null
}

ISP1220_TRIM8_GETVOL () {
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x01\x00\x00\x00\x01" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x02\x00\x00\x00\x0F" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x03\x00\x00\x00\x1A" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x01" -H 2 > /dev/null
        usleep 150
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x00" -H 2 > /dev/null
        isp1220_trim8_val=0x$(spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x00\x00\x00\x06\x00\x00\x00\x00" -H | grep RX | awk '{print $10}')
	print_vol_value=$(awk 'BEGIN{printf "%.2f\n", (('$ISP1220_DC_1V8_BASE' - '$isp1220_trim8_val') * '$ISP1220_1V8_Unit' + '$ISP1220_1V8_Vol' - '$ISP1220_1V8_Float') * '1000'}')
        printf "%-18s  %-8s  %-8s\r\n" $1 $isp1220_trim8_val $print_vol_value
}

checkBusVoltage() {
	VoltageBus=$1
	AdjustVoltage=$2
	deltaVoltage=0

	case $VoltageBus in
		"AVDD18_VCO" )
			deltaVoltage=$(awk 'BEGIN{printf "%.0f\n", '$ISP1220_DC_1V8_BASE' - (('$AdjustVoltage' - '$ISP1220_1V8_Vol' + '$ISP1220_1V8_Float')/'$ISP1220_1V8_Unit')}')
			if [ $deltaVoltage -gt 255 ] || [ $deltaVoltage -lt 0 ]; then
				echo "$VoltageBus Setting $AdjustVoltage V is not in Setting Range"
				exit 1
			fi
			ISP1220_TRIM1_SETVOL $((deltaVoltage))
			#echo "ISP1220_TRIM1_SETVOL $((deltaVoltage))"
			;;
		"AVDD18_CKI" )
			deltaVoltage=$(awk 'BEGIN{printf "%.0f\n", '$ISP1220_DC_1V8_BASE' - (('$AdjustVoltage' - '$ISP1220_1V8_Vol' + '$ISP1220_1V8_Float')/'$ISP1220_1V8_Unit')}')
			if [ $deltaVoltage -gt 255 ] || [ $deltaVoltage -lt 0 ]; then
				echo "$VoltageBus Setting $AdjustVoltage V is not in Setting Range"
				exit 1
			fi
			ISP1220_TRIM2_SETVOL $((deltaVoltage))
			#echo "ISP1220_TRIM2_SETVOL $((deltaVoltage))"
			;;
		"AVDD12_ABB" )
			deltaVoltage=$(awk 'BEGIN{printf "%.0f\n", '$ISP1220_DC_1V2_BASE' - (('$AdjustVoltage' - '$ISP1220_1V2_Vol' + '$ISP1220_1V2_Float')/'$ISP1220_1V2_Unit')}')
			if [ $deltaVoltage -gt 255 ] || [ $deltaVoltage -lt 0 ]; then
				echo "$VoltageBus Setting $AdjustVoltage V is not in Setting Range"
				exit 1
			fi
			ISP1220_TRIM3_SETVOL $((deltaVoltage))
			#echo "ISP1220_TRIM3_SETVOL $((deltaVoltage))"
			;;
		"AVDD18_RF" )
			deltaVoltage=$(awk 'BEGIN{printf "%.0f\n", '$ISP1220_DC_1V8_BASE' - (('$AdjustVoltage' - '$ISP1220_1V8_Vol' + '$ISP1220_1V8_Float')/'$ISP1220_1V8_Unit')}')
			if [ $deltaVoltage -gt 255 ] || [ $deltaVoltage -lt 0 ]; then
				echo "$VoltageBus Setting $AdjustVoltage V is not in Setting Range"
				exit 1
			fi
			ISP1220_TRIM4_SETVOL $((deltaVoltage))
			#echo "ISP1220_TRIM4_SETVOL $((deltaVoltage))"
			;;
                "V0V8_DCDC" )
                        deltaVoltage=$(awk 'BEGIN{printf "%.0f\n", '$ISP1220_DC_0V8_BASE' - (('$AdjustVoltage' - '$ISP1220_0V8_Vol' + '$ISP1220_0V8_Float')/'$ISP1220_0V8_Unit')}')
                        if [ $deltaVoltage -gt 255 ] || [ $deltaVoltage -lt 0 ]; then
                                echo "$VoltageBus Setting $AdjustVoltage V is not in Setting Range"
                                exit 1
                        fi
                        ISP1220_TRIM5_SETVOL $((deltaVoltage))
                        #echo "ISP1220_TRIM5_SETVOL $((deltaVoltage))"
                        ;;
                "DDR_VDDQ" )
                        deltaVoltage=$(awk 'BEGIN{printf "%.0f\n", '$ISP1220_DC_1V2_BASE' - (('$AdjustVoltage' - '$ISP1220_1V2_Vol' + '$ISP1220_1V2_Float')/'$ISP1220_1V2_Unit')}')
                        if [ $deltaVoltage -gt 255 ] || [ $deltaVoltage -lt 0 ]; then
                                echo "$VoltageBus Setting $AdjustVoltage V is not in Setting Range"
                                exit 1
                        fi
                        ISP1220_TRIM6_SETVOL $((deltaVoltage))
                        #echo "ISP1220_TRIM6_SETVOL $((deltaVoltage))"
                        ;;
                "V3V3_DCDC" )
                        deltaVoltage=$(awk 'BEGIN{printf "%.0f\n", '$ISP1220_DC_3V3_BASE' - (('$AdjustVoltage' - '$ISP1220_3V3_Vol' + '$ISP1220_3V3_Float')/'$ISP1220_3V3_Unit')}')
                        if [ $deltaVoltage -gt 255 ] || [ $deltaVoltage -lt 0 ]; then
                                echo "$VoltageBus Setting $AdjustVoltage V is not in Setting Range"
                                exit 1
                        fi
                        ISP1220_TRIM7_SETVOL $((deltaVoltage))
                        #echo "ISP1220_TRIM7_SETVOL $((deltaVoltage))"
                        ;;
                "V1V8_DCDC" )
                        deltaVoltage=$(awk 'BEGIN{printf "%.0f\n", '$ISP1220_DC_1V8_BASE' - (('$AdjustVoltage' - '$ISP1220_1V8_Vol' + '$ISP1220_1V8_Float')/'$ISP1220_1V8_Unit')}')
                        if [ $deltaVoltage -gt 255 ] || [ $deltaVoltage -lt 0 ]; then
                                echo "$VoltageBus Setting $AdjustVoltage V is not in Setting Range"
                                exit 1
                        fi
                        ISP1220_TRIM8_SETVOL $((deltaVoltage))
                        #echo "ISP1220_TRIM8_SETVOL $((deltaVoltage))"
                        ;;
		* ) echo "Invalid option passed to '$0' $VoltageBus"
			usage;;
	esac
	if [ $Voltage_Debug -eq 1 ]; then
		echo "Setting $VoltageBus $AdjustVoltage V"
	fi
}

ShowBusVoltage() {
	VoltageBus=$1
	deltaVoltage=0

	case $VoltageBus in
		"AVDD18_VCO" )
			ISP1220_TRIM1_GETVOL $VoltageBus
			#echo "ISP1220_TRIM1_GETVOL $VoltageBus"
			;;
		"AVDD18_CKI" )
			ISP1220_TRIM2_GETVOL $VoltageBus
			#echo "ISP1220_TRIM2_GETVOL $VoltageBus"
			;;
		"AVDD12_ABB" )
			ISP1220_TRIM3_GETVOL $VoltageBus
			#echo "ISP1220_TRIM3_GETVOL $VoltageBus"
			;;
		"AVDD18_RF" )
			ISP1220_TRIM4_GETVOL $VoltageBus
			#echo "ISP1220_TRIM4_GETVOL $VoltageBus"
			;;
		"V0V8_DCDC" )
			ISP1220_TRIM5_GETVOL $VoltageBus
			#echo "ISP1220_TRIM5_GETVOL $VoltageBus"
			;;
		"DDR_VDDQ" )
			ISP1220_TRIM6_GETVOL $VoltageBus
			#echo "ISP1220_TRIM6_GETVOL $VoltageBus"
			;;
		"V3V3_DCDC" )
			ISP1220_TRIM7_GETVOL $VoltageBus
			#echo "ISP1220_TRIM7_GETVOL $VoltageBus"
			;;
		"V1V8_DCDC" )
			ISP1220_TRIM8_GETVOL $VoltageBus
			#echo "ISP1220_TRIM8_GETVOL $VoltageBus"
			;;
		* ) echo "Invalid option passed to '$0' $VoltageBus"
			usage;;
	esac
}


VOL_BUS_NAME=""
Set_Voltage=""

while getopts "DN:R:V:p:n:" OPTION
do
	case $OPTION in
		D )   Voltage_Debug=1
			;;
		N )   VOL_BUS_NAME="$OPTARG"
			;;
		R )   Read_Bus_Flag="$OPTARG"
			;;
		V )   Set_Voltage="$OPTARG"
			;;
		p )   Spidev_Port="$OPTARG"
			;;
		n )   Spidev_No="$OPTARG"
			;;
		* ) echo "Invalid option passed to '$0' (options:$*)"
			  usage;;
	esac
done

SPIDEV_NODE_NAME="/dev/spidev""$Spidev_Port.0"

spidevX_pinmux $Spidev_Port $Spidev_No

if [ "$Set_Voltage" != "" ] ; then
        checkBusVoltage $VOL_BUS_NAME $Set_Voltage
fi

Read_Bus_Flag=$(echo $Read_Bus_Flag | tr 'A-Z' 'a-z')
if [ "$Read_Bus_Flag" == "yes" ] || [ "$Read_Bus_Flag" == "y" ] ; then
        if [ "$VOL_BUS_NAME" != "ALL" ] && [ "$VOL_BUS_NAME" != "all" ]  ; then
		#printf "%-18s  %-8s  %-8s\r\n" "VBUS"  "REG"  "(V)"
		ShowBusVoltage $VOL_BUS_NAME
        else
		printf "%-18s  %-8s  %-8s\r\n" "VBUS"  "REG"  "(mV)"
		ShowBusVoltage "AVDD18_VCO"
		ShowBusVoltage "AVDD18_CKI"
		ShowBusVoltage "AVDD12_ABB"
		ShowBusVoltage "AVDD18_RF"
		ShowBusVoltage "V0V8_DCDC"
		ShowBusVoltage "DDR_VDDQ"
		ShowBusVoltage "V3V3_DCDC"
		ShowBusVoltage "V1V8_DCDC"
        fi
fi
