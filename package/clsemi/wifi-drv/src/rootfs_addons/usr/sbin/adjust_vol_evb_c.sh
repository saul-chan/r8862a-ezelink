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

#V0V8_DCDC
#DDR_VDDQ
#V1V8_DCDC

# A[6] = 0, B = 1; A[6] = 1, B = -(64 -A)
LTC7106_SIGN_BIT=64

# Vol=1.2-(30*0.25*B/1000)
LTC7106_1V2_Unit=0.0075
# Vol=1.8-(20*0.25*B/1000)
LTC7106_1V8_Unit=0.0050
# Vol=0.804-(3.4*1*B/1000)
LTC7106_0V8_Unit=0.0034

LTC7106_1V8_Vol=1.8000
LTC7106_1V2_Vol=1.2000
LTC7106_0V8_Vol=0.8040

Spidev_Port=0
Spidev_No=1
Voltage_Debug=0

spidevX_pinmux() {
	io_set.sh -i SPI -p $1 -n $2
}

calculate_ltc7106_b_value() {
	# A[7] is invalid
	LTC7106_A_VALUE=$(($1 % 128))
	if [ $LTC7106_A_VALUE -gt $LTC7106_SIGN_BIT ] ; then
		LTC7106_A_VALUE=$((LTC7106_A_VALUE % LTC7106_SIGN_BIT))
		LTC7106_B_VALUE=$((0 - (64 - LTC7106_A_VALUE)))
	else
		LTC7106_A_VALUE=$((LTC7106_A_VALUE % LTC7106_SIGN_BIT))
		LTC7106_B_VALUE=$LTC7106_A_VALUE
	fi
	echo "$LTC7106_B_VALUE"
}

calculate_ltc7106_a_value() {
	# A[7] is invalid
	LTC7106_B_VALUE=$1
	if [ $LTC7106_B_VALUE -ge 0 ] ; then
		LTC7106_A_VALUE=$LTC7106_B_VALUE
	else
		LTC7106_A_VALUE=$((LTC7106_SIGN_BIT + LTC7106_SIGN_BIT + LTC7106_B_VALUE))
	fi
	echo "$LTC7106_A_VALUE"
}

# V0V8_DCDC
LTC7106_V0V8_DCDC_INIT () {
	# NOMINAL Mode
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x01\x00\x00\x00\x00" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x02\x00\x00\x00\x10" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x03\x00\x00\x00\x01" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x04\x00\x00\x00\x80" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x01" -H 2 > /dev/null
        usleep 150
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x00" -H 2 > /dev/null

	# Set IDAC
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x01\x00\x00\x00\x00" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x02\x00\x00\x00\x10" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x03\x00\x00\x00\xE4" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x04\x00\x00\x00\x40" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x01" -H 2 > /dev/null
        usleep 150
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x00" -H 2 > /dev/null

	# Enable IDAC MINUS
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x01\x00\x00\x00\x00" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x02\x00\x00\x00\x10" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x03\x00\x00\x00\xE6" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x04\x00\x00\x00\x40" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x01" -H 2 > /dev/null
        usleep 150
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x00" -H 2 > /dev/null
}

LTC7106_V0V8_DCDC_SETVOL () {
        vol_value=$(printf "%02x" $1)
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x01\x00\x00\x00\x00" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x02\x00\x00\x00\x10" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x03\x00\x00\x00\xE8" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x04\x00\x00\x00\x$vol_value " -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x01" -H 2 > /dev/null
        usleep 150
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x00" -H 2 > /dev/null
}

LTC7106_V0V8_DCDC_GETVOL () {
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x01\x00\x00\x00\x01" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x02\x00\x00\x00\x10" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x03\x00\x00\x00\xE8" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x01" -H 2 > /dev/null
        usleep 150
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x00" -H 2 > /dev/null

	ltc7106_v0v8_reg=0x$(spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x00\x00\x00\x06\x00\x00\x00\x00" -H | grep RX | awk '{print $10}')
	ltc7106_v0v8_val=$(calculate_ltc7106_b_value $ltc7106_v0v8_reg)
	# Vol=0.804-(3.4*1*B/1000)
	print_vol_value=$(awk 'BEGIN{printf "%.0f\n", ('0.804' - (('3.4' * '$ltc7106_v0v8_val') / '1000')) * '1000' }')
        printf "%-18s  %-8s  %-8s\r\n" $1 $ltc7106_v0v8_reg $print_vol_value
}

# DDR_VDDQ
LTC7106_DDR_VDDQ_INIT () {
        # NOMINAL Mode
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x01\x00\x00\x00\x00" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x02\x00\x00\x00\x11" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x03\x00\x00\x00\x01" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x04\x00\x00\x00\x98" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x01" -H 2 > /dev/null
        usleep 150
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x00" -H 2 > /dev/null

        # Set IDAC
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x01\x00\x00\x00\x00" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x02\x00\x00\x00\x11" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x03\x00\x00\x00\xE4" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x04\x00\x00\x00\x00" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x01" -H 2 > /dev/null
        usleep 150
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x00" -H 2 > /dev/null

        # Enable IDAC MINUS
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x01\x00\x00\x00\x00" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x02\x00\x00\x00\x11" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x03\x00\x00\x00\xE6" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x04\x00\x00\x00\x40" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x01" -H 2 > /dev/null
        usleep 150
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x00" -H 2 > /dev/null
}

LTC7106_DDR_VDDQ_SETVOL () {
        vol_value=$(printf "%02x" $1)
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x01\x00\x00\x00\x00" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x02\x00\x00\x00\x11" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x03\x00\x00\x00\xED" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x04\x00\x00\x00\x$vol_value " -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x01" -H 2 > /dev/null
        usleep 150
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x00" -H 2 > /dev/null
}

LTC7106_DDR_VDDQ_GETVOL () {
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x01\x00\x00\x00\x01" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x02\x00\x00\x00\x11" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x03\x00\x00\x00\xED" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x01" -H 2 > /dev/null
        usleep 150
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x00" -H 2 > /dev/null

	ltc7106_ddr_vddq_reg=0x$(spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x00\x00\x00\x06\x00\x00\x00\x00" -H | grep RX | awk '{print $10}')
	ltc7106_ddr_vddq_val=$(calculate_ltc7106_b_value $ltc7106_ddr_vddq_reg)
	# Vol=1.2-(30*0.25*B/1000)
	print_vol_value=$(awk 'BEGIN{printf "%.0f\n", ('1.2' - (('30' * '0.25' * '$ltc7106_ddr_vddq_val') / '1000')) * '1000' }')
        printf "%-18s  %-8s  %-8s\r\n" $1 $ltc7106_ddr_vddq_reg $print_vol_value
}

# V1V8_DCDC
LTC7106_V1V8_DCDC_INIT () {
        # NOMINAL Mode
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x01\x00\x00\x00\x00" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x02\x00\x00\x00\x12" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x03\x00\x00\x00\x01" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x04\x00\x00\x00\x98" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x01" -H 2 > /dev/null
        usleep 150
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x00" -H 2 > /dev/null

        # Set IDAC
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x01\x00\x00\x00\x00" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x02\x00\x00\x00\x12" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x03\x00\x00\x00\xE4" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x04\x00\x00\x00\x00" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x01" -H 2 > /dev/null
        usleep 150
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x00" -H 2 > /dev/null

        # Enable IDAC MINUS
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x01\x00\x00\x00\x00" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x02\x00\x00\x00\x12" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x03\x00\x00\x00\xE6" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x04\x00\x00\x00\x40" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x01" -H 2 > /dev/null
        usleep 150
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x00" -H 2 > /dev/null
}

LTC7106_V1V8_DCDC_SETVOL () {
        vol_value=$(printf "%02x" $1)
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x01\x00\x00\x00\x00" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x02\x00\x00\x00\x12" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x03\x00\x00\x00\xED" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x04\x00\x00\x00\x$vol_value " -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x01" -H 2 > /dev/null
        usleep 150
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x00" -H 2 > /dev/null
}

LTC7106_V1V8_DCDC_GETVOL () {
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x01\x00\x00\x00\x01" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x02\x00\x00\x00\x12" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x03\x00\x00\x00\xED" -H 2 > /dev/null
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x01" -H 2 > /dev/null
        usleep 150
        spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x80\x00\x00\x05\x00\x00\x00\x00" -H 2 > /dev/null

	ltc7106_v1v8_reg=0x$(spidev_test --device $SPIDEV_NODE_NAME -s 5000000 -v -p "\x00\x00\x00\x06\x00\x00\x00\x00" -H | grep RX | awk '{print $10}')
	ltc7106_v1v8_val=$(calculate_ltc7106_b_value $ltc7106_v1v8_reg)
	# Vol=1.8-(20*0.25*B/1000)
	print_vol_value=$(awk 'BEGIN{printf "%.0f\n", ('1.8' - (('20' * '0.25' * '$ltc7106_v1v8_val') / '1000')) * '1000' }')
        printf "%-18s  %-8s  %-8s\r\n" $1 $ltc7106_v1v8_reg $print_vol_value
}

checkBusVoltage() {
	VoltageBus=$1
	AdjustVoltage=$2
	deltaVoltage=0

	case $VoltageBus in
                "V0V8_DCDC" )
			# Vol=0.804-(3.4*1*B/1000) LTC7106_0V8_Unit=0.0034 LTC7106_0V8_Vol=0.8040
			# B = ((0.804 - Vol) * 1000) / 3.4
			deltaVoltage=$(awk 'BEGIN{printf "%.0f\n", (('$LTC7106_0V8_Vol' - '$AdjustVoltage') * '1000') / '3.4' }')
			deltaVoltage=$(calculate_ltc7106_a_value $deltaVoltage)
			LTC7106_V0V8_DCDC_INIT
                        LTC7106_V0V8_DCDC_SETVOL $((deltaVoltage))
                        #echo "LTC7106_V0V8_DCDC_SETVOL $((deltaVoltage))"
                        ;;
                "DDR_VDDQ" )
			# Vol=1.2-(30*0.25*B/1000) LTC7106_1V2_Unit=0.0075 LTC7106_1V2_Vol=1.2000
			deltaVoltage=$(awk 'BEGIN{printf "%.0f\n", (('$LTC7106_1V2_Vol' - '$AdjustVoltage') * '1000') / '7.5' }')
			deltaVoltage=$(calculate_ltc7106_a_value $deltaVoltage)
			LTC7106_DDR_VDDQ_INIT
                        LTC7106_DDR_VDDQ_SETVOL $((deltaVoltage))
                        #echo "LTC7106_DDR_VDDQ_SETVOL $((deltaVoltage))"
                        ;;
                "V1V8_DCDC" )
			# Vol=1.8-(20*0.25*B/1000) LTC7106_1V8_Unit=0.0050 LTC7106_1V8_Vol=1.8000
			deltaVoltage=$(awk 'BEGIN{printf "%.0f\n", (('$LTC7106_1V8_Vol' - '$AdjustVoltage') * '1000') / '5' }')
			deltaVoltage=$(calculate_ltc7106_a_value $deltaVoltage)
			LTC7106_V1V8_DCDC_INIT
                        LTC7106_V1V8_DCDC_SETVOL $((deltaVoltage))
                        #echo "LTC7106_V1V8_DCDC_SETVOL $((deltaVoltage))"
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
		"V0V8_DCDC" )
			LTC7106_V0V8_DCDC_GETVOL $VoltageBus
			#echo "LTC7106_V0V8_DCDC_GETVOL $VoltageBus"
			;;
		"DDR_VDDQ" )
			LTC7106_DDR_VDDQ_GETVOL $VoltageBus
			#echo "LTC7106_DDR_VDDQ_GETVOL $VoltageBus"
			;;
		"V1V8_DCDC" )
			LTC7106_V1V8_DCDC_GETVOL $VoltageBus
			#echo "LTC7106_V1V8_DCDC_GETVOL $VoltageBus"
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
		#printf "%-18s  %-8s  %-8s\r\n" "VBUS"  "REG"  "(mV)"
		ShowBusVoltage $VOL_BUS_NAME
        else
		printf "%-18s  %-8s  %-8s\r\n" "VBUS"  "REG"  "(mV)"
		ShowBusVoltage "V0V8_DCDC"
		ShowBusVoltage "DDR_VDDQ"
		ShowBusVoltage "V1V8_DCDC"
        fi
fi
