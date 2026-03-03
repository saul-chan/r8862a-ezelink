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
#V1V2_DCDC
#V1V8_DCDC

# A[6] = 0, B = 1; A[6] = 1, B = -(64 -A)
LTC7106_SIGN_BIT=64

# Vol=1.2-(20*0.25*B/1000)
LTC7106_1V2_Unit=0.0050
# Vol=1.83-(20*0.25*B/1000)
LTC7106_1V8_Unit=0.0050
# Vol=0.829-(10*0.25*B/1000)
LTC7106_0V8_Unit=0.0025

LTC7106_1V8_Vol=1.8300
LTC7106_1V2_Vol=1.2000
LTC7106_0V8_Vol=0.8290

LTC7106_0V8_I2C_Addr='0x2a'
LTC7106_1V2_I2C_Addr='0x2e'
LTC7106_1V8_I2C_Addr='0x2c'

i2cdev_Port=2
i2cdev_No=2
Voltage_Debug=0

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
	i2ctransfer -f -y $i2cdev_Port w2@$LTC7106_0V8_I2C_Addr 0x01 0x98
        usleep 10000

	# Set IDAC
	i2ctransfer -f -y $i2cdev_Port w2@$LTC7106_0V8_I2C_Addr 0xE4 0x00
        usleep 10000

	# Enable IDAC MINUS
	i2ctransfer -f -y $i2cdev_Port w2@$LTC7106_0V8_I2C_Addr 0xE6 0x40
        usleep 10000
}

LTC7106_V0V8_DCDC_SETVOL () {
        vol_value=$(printf "0x%02x" $1)
	i2ctransfer -f -y $i2cdev_Port w2@$LTC7106_0V8_I2C_Addr 0xED $vol_value
        usleep 10000
}

LTC7106_V0V8_DCDC_GETVOL () {
	ltc7106_v0v8_reg=$(i2ctransfer -f -y $i2cdev_Port w1@$LTC7106_0V8_I2C_Addr 0xED r1 | awk '{print $1}')
	ltc7106_v0v8_val=$(calculate_ltc7106_b_value $ltc7106_v0v8_reg)
	# Vol=0.829-(10*0.25*B/1000)
	print_vol_value=$(awk 'BEGIN{printf "%.0f\n", ('0.829' - (('10' * '0.25' * '$ltc7106_v0v8_val') / '1000')) * '1000' }')
        printf "%-18s  %-8s  %-8s\r\n" $1 $ltc7106_v0v8_reg $print_vol_value
}

# V1V2_DCDC
LTC7106_V1V2_DCDC_INIT () {
        # NOMINAL Mode
	i2ctransfer -f -y $i2cdev_Port w2@$LTC7106_1V2_I2C_Addr 0x01 0x98
	usleep 10000

        # Set IDAC
	i2ctransfer -f -y $i2cdev_Port w2@$LTC7106_1V2_I2C_Addr 0xE4 0x00
	usleep 10000

        # Enable IDAC MINUS
	i2ctransfer -f -y $i2cdev_Port w2@$LTC7106_1V2_I2C_Addr 0xE6 0x40
	usleep 10000
}

LTC7106_V1V2_DCDC_SETVOL () {
        vol_value=$(printf "0x%02x" $1)
	i2ctransfer -f -y $i2cdev_Port w2@$LTC7106_1V2_I2C_Addr 0xED $vol_value
	usleep 10000
}

LTC7106_V1V2_DCDC_GETVOL () {
	ltc7106_ddr_vddq_reg=$(i2ctransfer -f -y $i2cdev_Port w1@$LTC7106_1V2_I2C_Addr 0xED r1 | awk '{print $1}')
	ltc7106_ddr_vddq_val=$(calculate_ltc7106_b_value $ltc7106_ddr_vddq_reg)
	# Vol=1.2-(20*0.25*B/1000)
	print_vol_value=$(awk 'BEGIN{printf "%.0f\n", ('1.2' - (('20' * '0.25' * '$ltc7106_ddr_vddq_val') / '1000')) * '1000' }')
        printf "%-18s  %-8s  %-8s\r\n" $1 $ltc7106_ddr_vddq_reg $print_vol_value
}

# V1V8_DCDC
LTC7106_V1V8_DCDC_INIT () {
        # NOMINAL Mode
	i2ctransfer -f -y $i2cdev_Port w2@$LTC7106_1V8_I2C_Addr 0x01 0x98
	usleep 10000

        # Set IDAC
	i2ctransfer -f -y $i2cdev_Port w2@$LTC7106_1V8_I2C_Addr 0xE4 0x00
	usleep 10000

        # Enable IDAC MINUS
	i2ctransfer -f -y $i2cdev_Port w2@$LTC7106_1V8_I2C_Addr 0xE6 0x40
	usleep 10000
}

LTC7106_V1V8_DCDC_SETVOL () {
        vol_value=$(printf "0x%02x" $1)
	i2ctransfer -f -y $i2cdev_Port w2@$LTC7106_1V8_I2C_Addr 0xED $vol_value
	usleep 10000
}

LTC7106_V1V8_DCDC_GETVOL () {
	ltc7106_v1v8_reg=$(i2ctransfer -f -y $i2cdev_Port w1@$LTC7106_1V8_I2C_Addr 0xED r1 | awk '{print $1}')
	ltc7106_v1v8_val=$(calculate_ltc7106_b_value $ltc7106_v1v8_reg)
	# Vol=1.83-(20*0.25*B/1000)
	print_vol_value=$(awk 'BEGIN{printf "%.0f\n", ('1.83' - (('20' * '0.25' * '$ltc7106_v1v8_val') / '1000')) * '1000' }')
        printf "%-18s  %-8s  %-8s\r\n" $1 $ltc7106_v1v8_reg $print_vol_value
}

checkBusVoltage() {
	VoltageBus=$1
	AdjustVoltage=$2
	deltaVoltage=0

	case $VoltageBus in
                "V0V8_DCDC" )
			# Vol=0.829-(10*0.25*B/1000) LTC7106_0V8_Unit=0.0025 LTC7106_0V8_Vol=0.8290
			# B = ((0.829 - Vol) * 1000) / 2.5
			deltaVoltage=$(awk 'BEGIN{printf "%.0f\n", (('$LTC7106_0V8_Vol' - '$AdjustVoltage') * '1000') / '2.5' }')
			deltaVoltage=$(calculate_ltc7106_a_value $deltaVoltage)
			LTC7106_V0V8_DCDC_INIT
                        LTC7106_V0V8_DCDC_SETVOL $((deltaVoltage))
                        #echo "LTC7106_V0V8_DCDC_SETVOL $((deltaVoltage))"
                        ;;
                "V1V2_DCDC" )
			# Vol=1.2-(20*0.25*B/1000) LTC7106_1V2_Unit=0.0050 LTC7106_1V2_Vol=1.2000
			deltaVoltage=$(awk 'BEGIN{printf "%.0f\n", (('$LTC7106_1V2_Vol' - '$AdjustVoltage') * '1000') / '5' }')
			deltaVoltage=$(calculate_ltc7106_a_value $deltaVoltage)
			LTC7106_V1V2_DCDC_INIT
                        LTC7106_V1V2_DCDC_SETVOL $((deltaVoltage))
                        #echo "LTC7106_V1V2_DCDC_SETVOL $((deltaVoltage))"
                        ;;
                "V1V8_DCDC" )
			# Vol=1.83-(20*0.25*B/1000) LTC7106_1V8_Unit=0.0050 LTC7106_1V8_Vol=1.8300
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
		"V1V2_DCDC" )
			LTC7106_V1V2_DCDC_GETVOL $VoltageBus
			#echo "LTC7106_V1V2_DCDC_GETVOL $VoltageBus"
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
		ShowBusVoltage "V1V2_DCDC"
		ShowBusVoltage "V1V8_DCDC"
        fi
fi
