#!/bin/bash

usage() {
	printf "Usage: %s" "$(basename "$0")"
	printf "\n\t-D ==> Show Voltage Debug Log"
	printf "\n\t-H ==> Set Select Voltage BUS +5%"
	printf "\n\t-L ==> Set Select Voltage BUS -5%"
	printf "\n\t-R ==> Set Select Voltage BUS -5% ~ +5% Random Voltage"
	printf "\n\t-S ==> Select Voltage BUS with Mask 'V3V3 DDR_VDDQ V0V8_DCDC AVDD12_ABB V1V8_DCDC AVDD18_RF AVDD18_CKI AVDD18_VCO' 0xFF"
	printf "\n\r"
	exit 1
}

adjust_vol_high_flag=0
adjust_vol_low_flag=0
adjust_vol_high_low_loop=0
adjust_vol_random_flag=0
loop_delay=1
Voltage_Adjust_Range_Max=8
Voltage_Adjust_Percentage=0

AVDD18_VCO_Current_no=0
AVDD18_CKI_Current_no=0
AVDD18_RF_Current_no=0
V1V8_DCDC_Current_no=0
AVDD12_ABB_Current_no=0
V0V8_DCDC_Current_no=0
DDR_VDDQ_Current_no=0
V3V3_DCDC_Current_no=0

DEBUG_INFO_PARA=""

AVDD18_VCO_Fixed_VolArray="1.7074  1.7973  1.8872"
AVDD18_CKI_Fixed_VolArray="1.7074  1.7973  1.8872"
AVDD12_ABB_Fixed_VolArray="1.1400  1.2000  1.2600"
AVDD18_RF_Fixed_VolArray="1.7074  1.7973  1.8872"
V0V8_DCDC_Fixed_VolArray="0.7581  0.7980  0.8379"
DDR_VDDQ_Fixed_VolArray="1.4250  1.5000  1.5750"
V3V3_DCDC_Fixed_VolArray="3.1503  3.3161  3.4819"
V1V8_DCDC_Fixed_VolArray="1.7074  1.7973  1.8872"

AVDD18_VCO_SELECT_MASK=0x01
AVDD18_CKI_SELECT_MASK=0x02
AVDD18_RF_SELECT_MASK=0x04
V1V8_DCDC_SELECT_MASK=0x08
AVDD12_ABB_SELECT_MASK=0x10
V0V8_DCDC_SELECT_MASK=0x20
DDR_VDDQ_SELECT_MASK=0x40
V3V3_DCDC_SELECT_MASK=0x80
VOLTAGE_BUS_SELECT_MASK=0xF8

AVDD18_VCO_Current_Value="1.7973"
AVDD18_CKI_Current_Value="1.7973"
AVDD18_RF_Current_Value="1.7973"
V1V8_DCDC_Current_Value="1.7973"
AVDD12_ABB_Current_Value="1.2000"
DDR_VDDQ_Current_Value="1.5000"
V0V8_DCDC_Current_Value="0.7980"
V3V3_DCDC_Current_Value="3.3161"

AVDD18_VCO_CPLD_Value="1.7973"
AVDD18_CKI_CPLD_Value="1.7973"
AVDD18_RF_CPLD_Value="1.7973"
V1V8_DCDC_CPLD_Value="1.7973"
AVDD12_ABB_CPLD_Value="1.2000"
DDR_VDDQ_CPLD_Value="1.5000"
V0V8_DCDC_CPLD_Value="0.7980"
V3V3_DCDC_CPLD_Value="3.3161"

ISP1220_3V3_Unit="0.003901"
ISP1220_1V8_Unit="0.002114"
ISP1220_1V2_Unit="0.001418"
ISP1220_1V5_Unit="0.001764"
ISP1220_0V8_Unit="0.000939"

Check_Percentage_Range () {
	Set_Percentage_Range=$1
	Voltage_Max_Percentage_Range="0.120"
	if [ `echo "$Set_Percentage_Range > $Voltage_Max_Percentage_Range" |bc` -eq 1 ] ; then
		echo "Setting Adjustmemet exceed the max 12% range"
		exit 1
	fi
}

Get_Bus_Current_Voltag_Value() {
	VoltageBus=$1
        case $VoltageBus in
                "AVDD18_VCO" )
			echo $(awk 'BEGIN{printf "%.4f\n", ('$(ina226.sh -U 3 -R 10 |  awk '{print $2}')') /  '1000'}')
                        ;;
                "AVDD18_CKI" )
			echo $(awk 'BEGIN{printf "%.4f\n", ('$(ina226.sh -U 2 -R 20 |  awk '{print $2}')') /  '1000'}')
                        ;;
                "AVDD12_ABB" )
			echo $(awk 'BEGIN{printf "%.4f\n", ('$(ina226.sh -U 4 -R 10 |  awk '{print $2}')') /  '1000'}')
                        ;;
                "AVDD18_RF" )
			echo $(awk 'BEGIN{printf "%.4f\n", ('$(ina226.sh -U 1 -R 10 |  awk '{print $2}')') /  '1000'}')
                        ;;
                "V0V8_DCDC" )
			echo $(awk 'BEGIN{printf "%.4f\n", ('$(ina226.sh -U 5 -R 5 |  awk '{print $2}')') /  '1000'}')
                        ;;
                "DDR_VDDQ" )
			echo $(awk 'BEGIN{printf "%.4f\n", ('$(ina226.sh -U 6 -R 10 |  awk '{print $2}')') /  '1000'}')
                        ;;
                "V3V3_DCDC" )
			echo $(awk 'BEGIN{printf "%.4f\n", ('$(ina226.sh -U 8 -R 10 |  awk '{print $2}')') /  '1000'}')
                        ;;
                "V1V8_DCDC" )
			echo $(awk 'BEGIN{printf "%.4f\n", ('$(ina226.sh -U 7 -R 10 |  awk '{print $2}')') /  '1000'}')
                        ;;
                * ) echo "Invalid option passed to '$0' $VoltageBus"
                        usage;;
        esac
}

Get_Bus_CPLD_Set_Voltag_Value() {
        VoltageBus=$1
        case $VoltageBus in
                "AVDD18_VCO" )
                        echo $(awk 'BEGIN{printf "%.4f\n", ('$(adjust_vol_evb_b.sh  -N AVDD18_VCO -R yes | awk '{print $3}')') /  '1000'}')
                        ;;
                "AVDD18_CKI" )
                        echo $(awk 'BEGIN{printf "%.4f\n", ('$(adjust_vol_evb_b.sh  -N AVDD18_CKI -R yes | awk '{print $3}')') /  '1000'}')
                        ;;
                "AVDD12_ABB" )
                        echo $(awk 'BEGIN{printf "%.4f\n", ('$(adjust_vol_evb_b.sh  -N AVDD12_ABB -R yes | awk '{print $3}')') /  '1000'}')
                        ;;
                "AVDD18_RF" )
                        echo $(awk 'BEGIN{printf "%.4f\n", ('$(adjust_vol_evb_b.sh  -N AVDD18_RF -R yes | awk '{print $3}')') /  '1000'}')
                        ;;
                "V0V8_DCDC" )
                        echo $(awk 'BEGIN{printf "%.4f\n", ('$(adjust_vol_evb_b.sh  -N V0V8_DCDC -R yes | awk '{print $3}')') /  '1000'}')
                        ;;
                "DDR_VDDQ" )
                        echo $(awk 'BEGIN{printf "%.4f\n", ('$(adjust_vol_evb_b.sh  -N DDR_VDDQ -R yes | awk '{print $3}')') /  '1000'}')
                        ;;
                "V3V3_DCDC" )
                        echo $(awk 'BEGIN{printf "%.4f\n", ('$(adjust_vol_evb_b.sh  -N V3V3_DCDC -R yes | awk '{print $3}')') /  '1000'}')
                        ;;
                "V1V8_DCDC" )
                        echo $(awk 'BEGIN{printf "%.4f\n", ('$(adjust_vol_evb_b.sh  -N V1V8_DCDC -R yes | awk '{print $3}')') /  '1000'}')
                        ;;
                * ) echo "Invalid option passed to '$0' $VoltageBus"
                        usage;;
        esac
}

Get_All_Bus_Compensation_Value() {
        if [ $((AVDD18_VCO_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((AVDD18_VCO_SELECT_MASK)) ]; then
                # AVDD18_VCO 3 R_Shunt=10
                AVDD18_VCO_Current_Value=$(Get_Bus_Current_Voltag_Value AVDD18_VCO)
                AVDD18_VCO_CPLD_Value=$(Get_Bus_CPLD_Set_Voltag_Value AVDD18_VCO)
                AVDD18_VCO_Compensation_Value=$(awk 'BEGIN{printf "%.4f\n", '$AVDD18_VCO_CPLD_Value' - '$AVDD18_VCO_Current_Value'}')
                #echo "AVDD18_VCO_Compensation_Value $AVDD18_VCO_Compensation_Value"
        fi

        if [ $((AVDD18_CKI_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((AVDD18_CKI_SELECT_MASK)) ]; then
                # AVDD18_CKI 2 R_Shunt=20
                AVDD18_CKI_Current_Value=$(Get_Bus_Current_Voltag_Value AVDD18_CKI)
                AVDD18_CKI_CPLD_Value=$(Get_Bus_CPLD_Set_Voltag_Value AVDD18_CKI)
                AVDD18_CKI_Compensation_Value=$(awk 'BEGIN{printf "%.4f\n", '$AVDD18_CKI_CPLD_Value' - '$AVDD18_RF_Current_Value'}')
                #echo "AVDD18_CKI_Compensation_Value $AVDD18_CKI_Compensation_Value"
        fi

        if [ $((AVDD18_RF_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((AVDD18_RF_SELECT_MASK)) ]; then
                # AVDD18_RF 1 R_Shunt=10
                AVDD18_RF_Current_Value=$(Get_Bus_Current_Voltag_Value AVDD18_RF)
                AVDD18_RF_CPLD_Value=$(Get_Bus_CPLD_Set_Voltag_Value AVDD18_RF)
                AVDD18_RF_Compensation_Value=$(awk 'BEGIN{printf "%.4f\n", '$AVDD18_RF_CPLD_Value' - '$AVDD18_RF_Current_Value'}')
                #echo "AVDD18_RF_Compensation_Value $AVDD18_RF_Compensation_Value"
        fi

        if [ $((V1V8_DCDC_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((V1V8_DCDC_SELECT_MASK)) ]; then
                # V1V8_DCDC 7 R_Shunt=10
                V1V8_DCDC_Current_Value=$(Get_Bus_Current_Voltag_Value V1V8_DCDC)
                V1V8_DCDC_CPLD_Value=$(Get_Bus_CPLD_Set_Voltag_Value V1V8_DCDC)
                V1V8_DCDC_Compensation_Value=$(awk 'BEGIN{printf "%.4f\n", '$V1V8_DCDC_CPLD_Value' - '$V1V8_DCDC_Current_Value'}')
                #echo "V1V8_DCDC_Compensation_Value $V1V8_DCDC_Compensation_Value"
        fi

        if [ $((AVDD12_ABB_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((AVDD12_ABB_SELECT_MASK)) ]; then
                # AVDD12_ABB 4 R_Shunt=10
                AVDD12_ABB_Current_Value=$(Get_Bus_Current_Voltag_Value AVDD12_ABB)
                AVDD12_ABB_CPLD_Value=$(Get_Bus_CPLD_Set_Voltag_Value AVDD12_ABB)
                AVDD12_ABB_Compensation_Value=$(awk 'BEGIN{printf "%.4f\n", '$AVDD12_ABB_CPLD_Value' - '$AVDD12_ABB_Current_Value'}')
                #echo "AVDD12_ABB_Compensation_Value $AVDD12_ABB_Compensation_Value"
        fi

        if [ $((DDR_VDDQ_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((DDR_VDDQ_SELECT_MASK)) ]; then
                # DDR_VDDQ 6 R_Shunt=10
                DDR_VDDQ_Current_Value=$(Get_Bus_Current_Voltag_Value DDR_VDDQ)
                DDR_VDDQ_CPLD_Value=$(Get_Bus_CPLD_Set_Voltag_Value DDR_VDDQ)
                DDR_VDDQ_Compensation_Value=$(awk 'BEGIN{printf "%.4f\n", '$DDR_VDDQ_CPLD_Value' - '$DDR_VDDQ_Current_Value'}')
                #echo "DDR_VDDQ_Compensation_Value $DDR_VDDQ_Compensation_Value"
        fi

        if [ $((V0V8_DCDC_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((V0V8_DCDC_SELECT_MASK)) ]; then
                # V0V8_DCDC 5 R_Shunt=5
                V0V8_DCDC_Current_Value=$(Get_Bus_Current_Voltag_Value V0V8_DCDC)
                V0V8_DCDC_CPLD_Value=$(Get_Bus_CPLD_Set_Voltag_Value V0V8_DCDC)
                V0V8_DCDC_Compensation_Value=$(awk 'BEGIN{printf "%.4f\n", '$V0V8_DCDC_CPLD_Value' - '$V0V8_DCDC_Current_Value'}')
                #echo "V0V8_DCDC_Compensation_Value $V0V8_DCDC_Compensation_Value"
        fi

        if [ $((V3V3_DCDC_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((V3V3_DCDC_SELECT_MASK)) ]; then
                # V3V3_DCDC 8 R_Shunt=10
                V3V3_DCDC_Current_Value=$(Get_Bus_Current_Voltag_Value V3V3_DCDC)
                V3V3_DCDC_CPLD_Value=$(Get_Bus_CPLD_Set_Voltag_Value V3V3_DCDC)
                V3V3_DCDC_Compensation_Value=$(awk 'BEGIN{printf "%.4f\n", '$V3V3_DCDC_CPLD_Value' - '$V3V3_DCDC_Current_Value'}')
                #echo "V3V3_DCDC_Compensation_Value $V3V3_DCDC_Compensation_Value"
        fi
}

Auto_Compensation_Bus_Vol_By_Step() {
	VoltageBus=$1
	Bus_Voltage_Set_Value=$2
	Bus_Voltage_Current_Value=$3
	Bus_Voltage_Unit_Value=$4
	Bus_Voltage_Compensation_Value=$5
	Bus_Voltage_Set_Step_No=0
	Big_Range_Adjust_Time=0
	Little_Range_Adjust_Time=0

	if [ `echo "$Bus_Voltage_Set_Value > $Bus_Voltage_Current_Value" |bc` -eq 1 ] ; then
		Bus_Voltage_Set_Step_Num=$(awk 'BEGIN{printf "%d\n", ('$Bus_Voltage_Set_Value' - '$Bus_Voltage_Current_Value') / '$Bus_Voltage_Unit_Value'}')
		Big_Range_Adjust_Time=$((Bus_Voltage_Set_Step_Num / Voltage_Adjust_Range_Max))
		Little_Range_Adjust_Time=$((Bus_Voltage_Set_Step_Num & Voltage_Adjust_Range_Max))

		#echo "High Bus_Voltage_Set_Step_Num $Bus_Voltage_Set_Step_Num"
		while [ $Bus_Voltage_Set_Step_No -lt $Bus_Voltage_Set_Step_Num ]
		do
                        if [ $Bus_Voltage_Set_Step_No -lt $((Big_Range_Adjust_Time * Voltage_Adjust_Range_Max)) ] ; then
                                Bus_Voltage_Set_Step_No=$((Bus_Voltage_Set_Step_No + Voltage_Adjust_Range_Max))
                        else
                                Bus_Voltage_Set_Step_No=$((Bus_Voltage_Set_Step_No + 1))
                        fi

			#echo "Bus_Voltage_Set_Step_No $Bus_Voltage_Set_Step_No"
			Bus_Voltage_Setp_Set_Value=$(awk 'BEGIN{printf "%.4f\n", '$Bus_Voltage_Current_Value' + ('$Bus_Voltage_Set_Step_No' * '$Bus_Voltage_Unit_Value')}')
			#echo "Auto_Compensation_Bus_Vol $VoltageBus $Bus_Voltage_Setp_Set_Value $Bus_Voltage_Compensation_Value"
			Set_Compensation_Vol_Value=$(awk 'BEGIN{printf "%.4f\n", '$Bus_Voltage_Setp_Set_Value' + '$Bus_Voltage_Compensation_Value'}')
			adjust_vol_evb_b.sh -N $VoltageBus -V $Set_Compensation_Vol_Value
			usleep 400000

			# Check Step adjust Current Voltage
			Bus_Voltage_Setp_Current_Value=$(Get_Bus_Current_Voltag_Value $VoltageBus)
			Bus_Voltage_Setp_CPLD_Value=$(Get_Bus_CPLD_Set_Voltag_Value $VoltageBus)
			Bus_Voltage_Compensation_Value=$(awk 'BEGIN{printf "%.4f\n", '$Bus_Voltage_Setp_CPLD_Value' - '$Bus_Voltage_Setp_Current_Value'}')

			#echo "Bus_Voltage_Setp_Current_Value $Bus_Voltage_Setp_Current_Value Compensation $Bus_Voltage_Compensation_Value"
                        if [ `echo "$Bus_Voltage_Set_Value < $Bus_Voltage_Setp_Current_Value" |bc` -eq 1 ] ; then
                                break
                        fi
		done
	else
                Bus_Voltage_Set_Step_Num=$(awk 'BEGIN{printf "%d\n", ('$Bus_Voltage_Current_Value' - '$Bus_Voltage_Set_Value') / '$Bus_Voltage_Unit_Value'}')
                Big_Range_Adjust_Time=$((Bus_Voltage_Set_Step_Num / Voltage_Adjust_Range_Max))
                Little_Range_Adjust_Time=$((Bus_Voltage_Set_Step_Num & Voltage_Adjust_Range_Max))

		#echo "Low Bus_Voltage_Set_Step_Num $Bus_Voltage_Set_Step_Num"
                while [ $Bus_Voltage_Set_Step_No -lt $Bus_Voltage_Set_Step_Num ]
                do
			if [ $Bus_Voltage_Set_Step_No -lt $((Big_Range_Adjust_Time * Voltage_Adjust_Range_Max)) ] ; then
				Bus_Voltage_Set_Step_No=$((Bus_Voltage_Set_Step_No + Voltage_Adjust_Range_Max))
			else
				Bus_Voltage_Set_Step_No=$((Bus_Voltage_Set_Step_No + 1))
			fi

			#echo "Bus_Voltage_Set_Step_No $Bus_Voltage_Set_Step_No"
			Bus_Voltage_Setp_Set_Value=$(awk 'BEGIN{printf "%.4f\n", '$Bus_Voltage_Current_Value' - ('$Bus_Voltage_Set_Step_No' * '$Bus_Voltage_Unit_Value')}')
			#echo "Auto_Compensation_Bus_Vol $VoltageBus $Bus_Voltage_Setp_Set_Value $Bus_Voltage_Compensation_Value"
                        Set_Compensation_Vol_Value=$(awk 'BEGIN{printf "%.4f\n", '$Bus_Voltage_Setp_Set_Value' + '$Bus_Voltage_Compensation_Value'}')
                        adjust_vol_evb_b.sh -N $VoltageBus -V $Set_Compensation_Vol_Value
                        usleep 400000

                        # Check Step adjust Current Voltage
                        Bus_Voltage_Setp_Current_Value=$(Get_Bus_Current_Voltag_Value $VoltageBus)
                        Bus_Voltage_Setp_CPLD_Value=$(Get_Bus_CPLD_Set_Voltag_Value $VoltageBus)
                        Bus_Voltage_Compensation_Value=$(awk 'BEGIN{printf "%.4f\n", '$Bus_Voltage_Setp_CPLD_Value' - '$Bus_Voltage_Setp_Current_Value'}')

			#echo "Bus_Voltage_Setp_Current_Value $Bus_Voltage_Setp_Current_Value Compensation $Bus_Voltage_Compensation_Value"
                        if [ `echo "$Bus_Voltage_Set_Value > $Bus_Voltage_Setp_Current_Value" |bc` -eq 1 ] ; then
                                break
                        fi
                done
        fi

	if [ $DEBUG_INFO_PARA != "" ]; then
		echo "$VoltageBus Set_Voltage_Value $Bus_Voltage_Set_Value Get_Current_Value $(Get_Bus_Current_Voltag_Value $VoltageBus)"
	fi
}

Set_All_Bus_High_Vol() {

	if [ $((AVDD18_VCO_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((AVDD18_VCO_SELECT_MASK)) ]; then
		Set_High_Vol_Value=$(echo $AVDD18_VCO_Fixed_VolArray | awk '{print $2}')
		Set_High_Vol_Value=$(awk 'BEGIN{printf "%.4f\n", '$Set_High_Vol_Value' * ('1' + '$Voltage_Adjust_Percentage')}')
                Get_Current_Value=$(Get_Bus_Current_Voltag_Value AVDD18_VCO)
                Auto_Compensation_Bus_Vol_By_Step AVDD18_VCO $Set_High_Vol_Value $Get_Current_Value $ISP1220_1V8_Unit $AVDD18_VCO_Compensation_Value
	fi

	if [ $((AVDD18_CKI_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((AVDD18_CKI_SELECT_MASK)) ]; then
		Set_High_Vol_Value=$(echo $AVDD18_CKI_Fixed_VolArray) | awk '{print $2}'
		Set_High_Vol_Value=$(awk 'BEGIN{printf "%.4f\n", '$Set_High_Vol_Value' * ('1' + '$Voltage_Adjust_Percentage')}')
                Get_Current_Value=$(Get_Bus_Current_Voltag_Value AVDD18_CKI)
                Auto_Compensation_Bus_Vol_By_Step AVDD18_CKI $Set_High_Vol_Value $Get_Current_Value $ISP1220_1V8_Unit $AVDD18_CKI_Compensation_Value
	fi

	if [ $((AVDD18_RF_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((AVDD18_RF_SELECT_MASK)) ]; then
		Set_High_Vol_Value=$(echo $AVDD18_RF_Fixed_VolArray | awk '{print $2}')
		Set_High_Vol_Value=$(awk 'BEGIN{printf "%.4f\n", '$Set_High_Vol_Value' * ('1' + '$Voltage_Adjust_Percentage')}')
		Get_Current_Value=$(Get_Bus_Current_Voltag_Value AVDD18_RF)
                Auto_Compensation_Bus_Vol_By_Step AVDD18_RF $Set_High_Vol_Value $Get_Current_Value $ISP1220_1V8_Unit $AVDD18_RF_Compensation_Value
	fi

	if [ $((V1V8_DCDC_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((V1V8_DCDC_SELECT_MASK)) ]; then
		Set_High_Vol_Value=$(echo $V1V8_DCDC_Fixed_VolArray | awk '{print $2}')
		Set_High_Vol_Value=$(awk 'BEGIN{printf "%.4f\n", '$Set_High_Vol_Value' * ('1' + '$Voltage_Adjust_Percentage')}')
		Get_Current_Value=$(Get_Bus_Current_Voltag_Value V1V8_DCDC)
		Auto_Compensation_Bus_Vol_By_Step V1V8_DCDC $Set_High_Vol_Value $Get_Current_Value $ISP1220_1V8_Unit $V1V8_DCDC_Compensation_Value
	fi

	if [ $((AVDD12_ABB_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((AVDD12_ABB_SELECT_MASK)) ]; then
		Set_High_Vol_Value=$(echo $AVDD12_ABB_Fixed_VolArray | awk '{print $2}')
		Set_High_Vol_Value=$(awk 'BEGIN{printf "%.4f\n", '$Set_High_Vol_Value' * ('1' + '$Voltage_Adjust_Percentage')}')
                Get_Current_Value=$(Get_Bus_Current_Voltag_Value AVDD12_ABB)
                Auto_Compensation_Bus_Vol_By_Step AVDD12_ABB $Set_High_Vol_Value $Get_Current_Value $ISP1220_1V2_Unit $AVDD12_ABB_Compensation_Value
	fi

	if [ $((V0V8_DCDC_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((V0V8_DCDC_SELECT_MASK)) ]; then
		Set_High_Vol_Value=$(echo $V0V8_DCDC_Fixed_VolArray | awk '{print $2}')
		Set_High_Vol_Value=$(awk 'BEGIN{printf "%.4f\n", '$Set_High_Vol_Value' * ('1' + '$Voltage_Adjust_Percentage')}')
                Get_Current_Value=$(Get_Bus_Current_Voltag_Value V0V8_DCDC)
                Auto_Compensation_Bus_Vol_By_Step V0V8_DCDC $Set_High_Vol_Value $Get_Current_Value $ISP1220_0V8_Unit $V0V8_DCDC_Compensation_Value
	fi

	if [ $((DDR_VDDQ_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((DDR_VDDQ_SELECT_MASK)) ]; then
		Set_High_Vol_Value=$(echo $DDR_VDDQ_Fixed_VolArray | awk '{print $2}')
		Set_High_Vol_Value=$(awk 'BEGIN{printf "%.4f\n", '$Set_High_Vol_Value' * ('1' + '$Voltage_Adjust_Percentage')}')
                Get_Current_Value=$(Get_Bus_Current_Voltag_Value DDR_VDDQ)
                Auto_Compensation_Bus_Vol_By_Step DDR_VDDQ $Set_High_Vol_Value $Get_Current_Value $ISP1220_1V5_Unit $DDR_VDDQ_Compensation_Value
	fi

	if [ $((V3V3_DCDC_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((V3V3_DCDC_SELECT_MASK)) ]; then
		Set_High_Vol_Value=$(echo $V3V3_DCDC_Fixed_VolArray | awk '{print $2}')
		Set_High_Vol_Value=$(awk 'BEGIN{printf "%.4f\n", '$Set_High_Vol_Value' * ('1' + '$Voltage_Adjust_Percentage')}')
                Get_Current_Value=$(Get_Bus_Current_Voltag_Value V3V3_DCDC)
                Auto_Compensation_Bus_Vol_By_Step V3V3_DCDC $Set_High_Vol_Value $Get_Current_Value $ISP1220_3V3_Unit $V3V3_DCDC_Compensation_Value
	fi
}

Set_All_Bus_Low_Vol() {
	if [ $((AVDD18_VCO_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((AVDD18_VCO_SELECT_MASK)) ]; then
		Set_Low_Vol_Value=$(echo $AVDD18_VCO_Fixed_VolArray | awk '{print $2}')
		Set_Low_Vol_Value=$(awk 'BEGIN{printf "%.4f\n", '$Set_Low_Vol_Value' * ('1' - '$Voltage_Adjust_Percentage')}')
                Get_Current_Value=$(Get_Bus_Current_Voltag_Value AVDD18_VCO)
                Auto_Compensation_Bus_Vol_By_Step AVDD18_VCO $Set_Low_Vol_Value $Get_Current_Value $ISP1220_1V8_Unit $AVDD18_VCO_Compensation_Value
	fi

	if [ $((AVDD18_CKI_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((AVDD18_CKI_SELECT_MASK)) ]; then
		Set_Low_Vol_Value=$(echo $AVDD18_CKI_Fixed_VolArray | awk '{print $2}')
		Set_Low_Vol_Value=$(awk 'BEGIN{printf "%.4f\n", '$Set_Low_Vol_Value' * ('1' - '$Voltage_Adjust_Percentage')}')
                Get_Current_Value=$(Get_Bus_Current_Voltag_Value AVDD18_CKI)
                Auto_Compensation_Bus_Vol_By_Step AVDD18_CKI $Set_Low_Vol_Value $Get_Current_Value $ISP1220_1V8_Unit $AVDD18_CKI_Compensation_Value
	fi

	if [ $((AVDD18_RF_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((AVDD18_RF_SELECT_MASK)) ]; then
		Set_Low_Vol_Value=$(echo $AVDD18_RF_Fixed_VolArray | awk '{print $2}')
		Set_Low_Vol_Value=$(awk 'BEGIN{printf "%.4f\n", '$Set_Low_Vol_Value' * ('1' - '$Voltage_Adjust_Percentage')}')
                Get_Current_Value=$(Get_Bus_Current_Voltag_Value AVDD18_RF)
                Auto_Compensation_Bus_Vol_By_Step AVDD18_RF $Set_Low_Vol_Value $Get_Current_Value $ISP1220_1V8_Unit $AVDD18_RF_Compensation_Value
	fi

	if [ $((V1V8_DCDC_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((V1V8_DCDC_SELECT_MASK)) ]; then
		Set_Low_Vol_Value=$(echo $V1V8_DCDC_Fixed_VolArray | awk '{print $2}')
		Set_Low_Vol_Value=$(awk 'BEGIN{printf "%.4f\n", '$Set_Low_Vol_Value' * ('1' - '$Voltage_Adjust_Percentage')}')
                Get_Current_Value=$(Get_Bus_Current_Voltag_Value V1V8_DCDC)
		Auto_Compensation_Bus_Vol_By_Step V1V8_DCDC $Set_Low_Vol_Value $Get_Current_Value $ISP1220_1V8_Unit $V1V8_DCDC_Compensation_Value
	fi

	if [ $((AVDD12_ABB_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((AVDD12_ABB_SELECT_MASK)) ]; then
		Set_Low_Vol_Value=$(echo $AVDD12_ABB_Fixed_VolArray | awk '{print $2}')
		Set_Low_Vol_Value=$(awk 'BEGIN{printf "%.4f\n", '$Set_Low_Vol_Value' * ('1' - '$Voltage_Adjust_Percentage')}')
                Get_Current_Value=$(Get_Bus_Current_Voltag_Value AVDD12_ABB)
                Auto_Compensation_Bus_Vol_By_Step AVDD12_ABB $Set_Low_Vol_Value $Get_Current_Value $ISP1220_1V2_Unit $AVDD12_ABB_Compensation_Value
	fi

	if [ $((V0V8_DCDC_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((V0V8_DCDC_SELECT_MASK)) ]; then
		Set_Low_Vol_Value=$(echo $V0V8_DCDC_Fixed_VolArray | awk '{print $2}')
		Set_Low_Vol_Value=$(awk 'BEGIN{printf "%.4f\n", '$Set_Low_Vol_Value' * ('1' - '$Voltage_Adjust_Percentage')}')
                Get_Current_Value=$(Get_Bus_Current_Voltag_Value V0V8_DCDC)
                Auto_Compensation_Bus_Vol_By_Step V0V8_DCDC $Set_Low_Vol_Value $Get_Current_Value $ISP1220_0V8_Unit $V0V8_DCDC_Compensation_Value
	fi

	if [ $((DDR_VDDQ_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((DDR_VDDQ_SELECT_MASK)) ]; then
		Set_Low_Vol_Value=$(echo $DDR_VDDQ_Fixed_VolArray | awk '{print $2}')
		Set_Low_Vol_Value=$(awk 'BEGIN{printf "%.4f\n", '$Set_Low_Vol_Value' * ('1' - '$Voltage_Adjust_Percentage')}')
                Get_Current_Value=$(Get_Bus_Current_Voltag_Value DDR_VDDQ)
                Auto_Compensation_Bus_Vol_By_Step DDR_VDDQ $Set_Low_Vol_Value $Get_Current_Value $ISP1220_1V5_Unit $DDR_VDDQ_Compensation_Value
	fi

	if [ $((V3V3_DCDC_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((V3V3_DCDC_SELECT_MASK)) ]; then
		Set_Low_Vol_Value=$(echo $V3V3_DCDC_Fixed_VolArray | awk '{print $2}')
		Set_Low_Vol_Value=$(awk 'BEGIN{printf "%.4f\n", '$Set_Low_Vol_Value' * ('1' - '$Voltage_Adjust_Percentage')}')
                Get_Current_Value=$(Get_Bus_Current_Voltag_Value V3V3_DCDC)
                Auto_Compensation_Bus_Vol_By_Step V3V3_DCDC $Set_Low_Vol_Value $Get_Current_Value $ISP1220_3V3_Unit $V3V3_DCDC_Compensation_Value
	fi
}

Set_All_Bus_Normal_Vol() {
        if [ $((AVDD18_VCO_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((AVDD18_VCO_SELECT_MASK)) ]; then
                Set_Low_Vol_Value=$(echo $AVDD18_VCO_Fixed_VolArray | awk '{print $2}')
                Get_Current_Value=$(Get_Bus_Current_Voltag_Value AVDD18_VCO)
                Auto_Compensation_Bus_Vol_By_Step AVDD18_VCO $Set_Low_Vol_Value $Get_Current_Value $ISP1220_1V8_Unit $AVDD18_VCO_Compensation_Value
        fi

        if [ $((AVDD18_CKI_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((AVDD18_CKI_SELECT_MASK)) ]; then
                Set_Low_Vol_Value=$(echo $AVDD18_CKI_Fixed_VolArray | awk '{print $2}')
                Get_Current_Value=$(Get_Bus_Current_Voltag_Value AVDD18_CKI)
                Auto_Compensation_Bus_Vol_By_Step AVDD18_CKI $Set_Low_Vol_Value $Get_Current_Value $ISP1220_1V8_Unit $AVDD18_CKI_Compensation_Value
        fi

        if [ $((AVDD18_RF_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((AVDD18_RF_SELECT_MASK)) ]; then
                Set_Low_Vol_Value=$(echo $AVDD18_RF_Fixed_VolArray | awk '{print $2}')
                Get_Current_Value=$(Get_Bus_Current_Voltag_Value AVDD18_RF)
                Auto_Compensation_Bus_Vol_By_Step AVDD18_RF $Set_Low_Vol_Value $Get_Current_Value $ISP1220_1V8_Unit $AVDD18_RF_Compensation_Value
        fi

        if [ $((V1V8_DCDC_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((V1V8_DCDC_SELECT_MASK)) ]; then
                Set_Low_Vol_Value=$(echo $V1V8_DCDC_Fixed_VolArray | awk '{print $2}')
                Get_Current_Value=$(Get_Bus_Current_Voltag_Value V1V8_DCDC)
                Auto_Compensation_Bus_Vol_By_Step V1V8_DCDC $Set_Low_Vol_Value $Get_Current_Value $ISP1220_1V8_Unit $V1V8_DCDC_Compensation_Value
        fi

        if [ $((AVDD12_ABB_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((AVDD12_ABB_SELECT_MASK)) ]; then
                Set_Low_Vol_Value=$(echo $AVDD12_ABB_Fixed_VolArray | awk '{print $2}')
                Get_Current_Value=$(Get_Bus_Current_Voltag_Value AVDD12_ABB)
                Auto_Compensation_Bus_Vol_By_Step AVDD12_ABB $Set_Low_Vol_Value $Get_Current_Value $ISP1220_1V2_Unit $AVDD12_ABB_Compensation_Value
        fi

        if [ $((V0V8_DCDC_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((V0V8_DCDC_SELECT_MASK)) ]; then
                Set_Low_Vol_Value=$(echo $V0V8_DCDC_Fixed_VolArray | awk '{print $2}')
                Get_Current_Value=$(Get_Bus_Current_Voltag_Value V0V8_DCDC)
                Auto_Compensation_Bus_Vol_By_Step V0V8_DCDC $Set_Low_Vol_Value $Get_Current_Value $ISP1220_0V8_Unit $V0V8_DCDC_Compensation_Value
        fi

        if [ $((DDR_VDDQ_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((DDR_VDDQ_SELECT_MASK)) ]; then
                Set_Low_Vol_Value=$(echo $DDR_VDDQ_Fixed_VolArray | awk '{print $2}')
                Get_Current_Value=$(Get_Bus_Current_Voltag_Value DDR_VDDQ)
                Auto_Compensation_Bus_Vol_By_Step DDR_VDDQ $Set_Low_Vol_Value $Get_Current_Value $ISP1220_1V5_Unit $DDR_VDDQ_Compensation_Value
        fi

        if [ $((V3V3_DCDC_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((V3V3_DCDC_SELECT_MASK)) ]; then
                Set_Low_Vol_Value=$(echo $V3V3_DCDC_Fixed_VolArray | awk '{print $2}')
                Get_Current_Value=$(Get_Bus_Current_Voltag_Value V3V3_DCDC)
                Auto_Compensation_Bus_Vol_By_Step V3V3_DCDC $Set_Low_Vol_Value $Get_Current_Value $ISP1220_3V3_Unit $V3V3_DCDC_Compensation_Value
        fi
}

Rand() {
	min=$1
	max=$(($2 - min + 1))
	num=$(date +%s)
	echo $((num % max + min))
}

Set_VoltageBus_Random_Vol() {
	last_random=$1
	current_random=$2
	VoltageBus=$3

	if [ $last_random -gt $current_random ] ; then

		case $last_random in
			"2" )
				delta=1
				;;
			"1" )
				delta=0
				;;
		esac

	elif [ $current_random -gt $last_random ] ; then

		case $last_random in
			"0" )
				delta=1
				;;
			"1" )
				delta=2
				;;
		esac

	elif [ $current_random -eq $last_random ] ; then

		case $last_random in
			"0" )
				delta=1
				;;
			"1" )
				rnd=$(Rand 0 1)
				if [ $rnd -eq 0 ]; then
					delta=0
				else
					delta=2
				fi
				;;
			"2" )
				delta=1
				;;
		esac

	fi

	#echo "$VoltageBus last_random=$last_random current_random=$current_random delta=$delta"

	case $VoltageBus in
		"AVDD18_VCO" )
			setVoltage=$(echo $AVDD18_VCO_Fixed_VolArray | awk '{print $'$((delta + 1))' }')
			AVDD18_VCO_Current_no=$delta
			VoltageUnitVal=$ISP1220_1V8_Unit
			VoltageCompensationValue=$AVDD18_VCO_Compensation_Value
			;;
		"AVDD18_CKI" )
			setVoltage=$(echo $AVDD18_CKI_Fixed_VolArray | awk '{print $'$((delta + 1))' }')
			AVDD18_CKI_Current_no=$delta
			VoltageUnitVal=$ISP1220_1V8_Unit
			VoltageCompensationValue=$AVDD18_CKI_Compensation_Value
			;;
		"AVDD12_ABB" )
			setVoltage=$(echo $AVDD12_ABB_Fixed_VolArray | awk '{print $'$((delta + 1))' }')
			AVDD12_ABB_Current_no=$delta
			VoltageUnitVal=$ISP1220_1V2_Unit
			VoltageCompensationValue=$AVDD12_ABB_Compensation_Value
			;;
		"AVDD18_RF" )
			setVoltage=$(echo $AVDD18_RF_Fixed_VolArray | awk '{print $'$((delta + 1))' }')
			AVDD18_RF_Current_no=$delta
			VoltageUnitVal=$ISP1220_1V8_Unit
			VoltageCompensationValue=$AVDD18_RF_Compensation_Value
			;;
                "V0V8_DCDC" )
			setVoltage=$(echo $V0V8_DCDC_Fixed_VolArray | awk '{print $'$((delta + 1))' }')
			V0V8_DCDC_Current_no=$delta
			VoltageUnitVal=$ISP1220_0V8_Unit
			VoltageCompensationValue=$V0V8_DCDC_Compensation_Value
                        ;;
                "DDR_VDDQ" )
			setVoltage=$(echo $DDR_VDDQ_Fixed_VolArray | awk '{print $'$((delta + 1))' }')
			DDR_VDDQ_Current_no=$delta
			VoltageUnitVal=$ISP1220_1V5_Unit
			VoltageCompensationValue=$DDR_VDDQ_Compensation_Value
                        ;;
                "V3V3_DCDC" )
			setVoltage=$(echo $V3V3_DCDC_Fixed_VolArray | awk '{print $'$((delta + 1))' }')
			V3V3_DCDC_Current_no=$delta
			VoltageUnitVal=$ISP1220_3V3_Unit
			VoltageCompensationValue=$V3V3_DCDC_Compensation_Value
                        ;;
                "V1V8_DCDC" )
			setVoltage=$(echo $V1V8_DCDC_Fixed_VolArray | awk '{print $'$((delta + 1))' }')
			V1V8_DCDC_Current_no=$delta
			VoltageUnitVal=$ISP1220_1V8_Unit
			VoltageCompensationValue=$V1V8_DCDC_Compensation_Value
                        ;;
		* ) echo "Invalid option passed to '$0' $VoltageBus"
			usage;;
	esac

	getVoltage=$(Get_Bus_Current_Voltag_Value $VoltageBus)
	Auto_Compensation_Bus_Vol_By_Step $VoltageBus $setVoltage $getVoltage $VoltageUnitVal $VoltageCompensationValue
}

Set_All_Bus_Random_Vol() {

	if [ $((AVDD18_VCO_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((AVDD18_VCO_SELECT_MASK)) ]; then
		Rnd=$(Rand 0 2)
		Set_VoltageBus_Random_Vol $AVDD18_VCO_Current_no $Rnd  "AVDD18_VCO"
	fi

	if [ $((AVDD18_CKI_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((AVDD18_CKI_SELECT_MASK)) ]; then
		Rnd=$(Rand 0 2)
		Set_VoltageBus_Random_Vol $AVDD18_CKI_Current_no $Rnd  "AVDD18_CKI"
	fi

	if [ $((AVDD18_RF_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((AVDD18_RF_SELECT_MASK)) ]; then
		Rnd=$(Rand 0 2)
		Set_VoltageBus_Random_Vol $AVDD18_RF_Current_no $Rnd  "AVDD18_RF"
	fi

	if [ $((V1V8_DCDC_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((V1V8_DCDC_SELECT_MASK)) ]; then
		Rnd=$(Rand 0 2)
		Set_VoltageBus_Random_Vol $V1V8_DCDC_Current_no $Rnd  "V1V8_DCDC"
	fi

	if [ $((AVDD12_ABB_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((AVDD12_ABB_SELECT_MASK)) ]; then
		Rnd=$(Rand 0 2)
		Set_VoltageBus_Random_Vol $AVDD12_ABB_Current_no $Rnd  "AVDD12_ABB"
	fi

	if [ $((V0V8_DCDC_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((V0V8_DCDC_SELECT_MASK)) ]; then
		Rnd=$(Rand 0 2)
		Set_VoltageBus_Random_Vol $V0V8_DCDC_Current_no $Rnd  "V0V8_DCDC"
	fi

	if [ $((DDR_VDDQ_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((DDR_VDDQ_SELECT_MASK)) ]; then
		Rnd=$(Rand 0 2)
		Set_VoltageBus_Random_Vol $DDR_VDDQ_Current_no $Rnd  "DDR_VDDQ"
	fi

	if [ $((V3V3_DCDC_SELECT_MASK & VOLTAGE_BUS_SELECT_MASK)) -eq $((V3V3_DCDC_SELECT_MASK)) ]; then
		Rnd=$(Rand 0 2)
		Set_VoltageBus_Random_Vol $V3V3_DCDC_Current_no $Rnd  "V3V3_DCDC"
	fi
}

Set_All_Bus_Random_Vol_in_loop() {
	while true
	do
	    Set_All_Bus_Random_Vol
	    echo "======== Set_All_Bus_Random_Vol One time Complete!!! ========"
	    sleep $1
	done
}

Set_All_Bus_High_Vol_in_loop() {
        while true
        do
            Set_All_Bus_High_Vol
            Save_Adjust_Percentage=$Voltage_Adjust_Percentage
            Voltage_Adjust_Percentage=0
            sleep $1
            Set_All_Bus_High_Vol
            Voltage_Adjust_Percentage=$Save_Adjust_Percentage
            sleep $1
	    echo "======== Set_All_Bus_High_Vol_in_loop One Loop Complete!!! ========"
        done
}

Set_All_Bus_Low_Vol_in_loop() {
        while true
        do
            Set_All_Bus_Low_Vol
            Save_Adjust_Percentage=$Voltage_Adjust_Percentage
            Voltage_Adjust_Percentage=0
            sleep $1
            Set_All_Bus_Low_Vol
            Voltage_Adjust_Percentage=$Save_Adjust_Percentage
            sleep $1
	    echo "======== Set_All_Bus_Low_Vol_in_loop One Loop Complete!!! ========"
        done
}

while getopts "DH:l:L:R:S:" OPTION
do
	case $OPTION in
                D )
                       DEBUG_INFO_PARA="-D"
                        ;;
		H )
		       Check_Percentage_Range "$OPTARG"
		       Voltage_Adjust_Percentage="$OPTARG"
		       adjust_vol_high_flag=1
			;;
		L )
		       Check_Percentage_Range "$OPTARG"
		       Voltage_Adjust_Percentage="$OPTARG"
		       adjust_vol_low_flag=1
			;;
                l )
                        loop_delay="$OPTARG"
                        adjust_vol_high_low_loop=1
                        ;;
		R )    loop_delay="$OPTARG"
		       adjust_vol_random_flag=1
			;;
		S )    VOLTAGE_BUS_SELECT_MASK="$OPTARG"
			;;
		* ) echo "Invalid option passed to '$0' (options:$*)"
			  usage;;
	esac
done

Get_All_Bus_Compensation_Value

if [ $adjust_vol_high_flag -eq 1 ] && [ $adjust_vol_low_flag -eq 0 ] && [ $adjust_vol_high_low_loop -eq 0 ]; then
        Set_All_Bus_High_Vol
fi

if [ $adjust_vol_low_flag -eq 1 ] && [ $adjust_vol_high_flag -eq 0 ] && [ $adjust_vol_high_low_loop -eq 0 ]; then
        Set_All_Bus_Low_Vol
fi

if [ $adjust_vol_random_flag -eq 1 ]; then
        Set_All_Bus_Random_Vol_in_loop $loop_delay
fi

if [ $adjust_vol_high_flag -eq 1 ] && [ $adjust_vol_low_flag -eq 0 ] && [ $adjust_vol_high_low_loop -eq 1 ]; then
        Set_All_Bus_High_Vol_in_loop $loop_delay
fi

if [ $adjust_vol_low_flag -eq 1 ] && [ $adjust_vol_high_flag -eq 0 ] && [ $adjust_vol_high_low_loop -eq 1 ]; then
        Set_All_Bus_Low_Vol_in_loop $loop_delay
fi
