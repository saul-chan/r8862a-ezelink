#!/bin/bash

if [ $# -lt 1 ] || [ $# -gt 2 ]; then
    echo "Usage: $0 <number_of_times> [interval_seconds]"
    exit 1
fi

i=1 # times
num_tests=$1
interval=${2:-1}  # default delay 1s

#power param
power_dev_path=$(ls /dev/spidev*.0)

if [ -n "$power_dev_path" ]; then
        is_support_spidev=1
else
        is_support_spidev=0
        echo "Unsupport spidev!"
fi

#temperature param
pvt_debugfs="/sys/kernel/debug/pvt/pvt_cmd"

if [ ! -e "$pvt_debugfs" ]; then
    echo "Please insmod pvt module!"
    exit
fi

dmesg -n 1
echo "w printk 0x3" > $pvt_debugfs

#calculate param
power_result_file="/tmp/power_test_results.txt"

if [ -e "$power_result_file" ]; then
    rm "$power_result_file"
fi

total_voltage_AVDD12_ABB=0
total_current_AVDD12_ABB=0
total_power_AVDD12_ABB=0

total_voltage_V0V8_DCDC=0
total_current_V0V8_DCDC=0
total_power_V0V8_DCDC=0

total_voltage_DDR_VDDQ=0
total_current_DDR_VDDQ=0
total_power_DDR_VDDQ=0

total_voltage_V1V8_DCDC=0
total_current_V1V8_DCDC=0
total_power_V1V8_DCDC=0

total_voltage_V3V3_DCDC=0
total_current_V3V3_DCDC=0
total_power_V3V3_DCDC=0
total_power=0

total_temp_ts0=0
total_temp_ts1=0
total_temp_ts2=0

while [ $i -le $num_tests ]
do
    echo "Test $i:" >> "$power_result_file"
    date  >> "$power_result_file"

    #power case
    if [ $is_support_spidev = 1 ] ; then
        test_output=$(/usr/sbin/ina226_show.sh)

        echo "----------------------------------" >> "$power_result_file"
        echo "$test_output" >> "$power_result_file"

        voltage_AVDD12_ABB=$(echo "$test_output" | awk '/AVDD12_ABB/ {print $2}')
        current_AVDD12_ABB=$(echo "$test_output" | awk '/AVDD12_ABB/ {print $3}')
        power_AVDD12_ABB=$(echo "$test_output" | awk '/AVDD12_ABB/ {print $4}')

        voltage_V0V8_DCDC=$(echo "$test_output" | awk '/V0V8_DCDC/ {print $2}')
        current_V0V8_DCDC=$(echo "$test_output" | awk '/V0V8_DCDC/ {print $3}')
        power_V0V8_DCDC=$(echo "$test_output" | awk '/V0V8_DCDC/ {print $4}')

        voltage_DDR_VDDQ=$(echo "$test_output" | awk '/DDR_VDDQ/ {print $2}')
        current_DDR_VDDQ=$(echo "$test_output" | awk '/DDR_VDDQ/ {print $3}')
        power_DDR_VDDQ=$(echo "$test_output" | awk '/DDR_VDDQ/ {print $4}')

        voltage_V1V8_DCDC=$(echo "$test_output" | awk '/V1V8_DCDC/ {print $2}')
        current_V1V8_DCDC=$(echo "$test_output" | awk '/V1V8_DCDC/ {print $3}')
        power_V1V8_DCDC=$(echo "$test_output" | awk '/V1V8_DCDC/ {print $4}')

        voltage_V3V3_DCDC=$(echo "$test_output" | awk '/V3V3_DCDC/ {print $2}')
        current_V3V3_DCDC=$(echo "$test_output" | awk '/V3V3_DCDC/ {print $3}')
        power_V3V3_DCDC=$(echo "$test_output" | awk '/V3V3_DCDC/ {print $4}')

        power=$(echo "$test_output" | awk '/total/ {print $2}')

        total_voltage_AVDD12_ABB=$(echo "$total_voltage_AVDD12_ABB + $voltage_AVDD12_ABB" | bc)
        total_current_AVDD12_ABB=$(echo "$total_current_AVDD12_ABB + $current_AVDD12_ABB" | bc)
        total_power_AVDD12_ABB=$(echo "$total_power_AVDD12_ABB + $power_AVDD12_ABB" | bc)

        total_voltage_V0V8_DCDC=$(echo "$total_voltage_V0V8_DCDC + $voltage_V0V8_DCDC" | bc)
        total_current_V0V8_DCDC=$(echo "$total_current_V0V8_DCDC + $current_V0V8_DCDC" | bc)
        total_power_V0V8_DCDC=$(echo "$total_power_V0V8_DCDC + $power_V0V8_DCDC" | bc)

        total_voltage_DDR_VDDQ=$(echo "$total_voltage_DDR_VDDQ + $voltage_DDR_VDDQ" | bc)
        total_current_DDR_VDDQ=$(echo "$total_current_DDR_VDDQ + $current_DDR_VDDQ" | bc)
        total_power_DDR_VDDQ=$(echo "$total_power_DDR_VDDQ + $power_DDR_VDDQ" | bc)

        total_voltage_V1V8_DCDC=$(echo "$total_voltage_V1V8_DCDC + $voltage_V1V8_DCDC" | bc)
        total_current_V1V8_DCDC=$(echo "$total_current_V1V8_DCDC + $current_V1V8_DCDC" | bc)
        total_power_V1V8_DCDC=$(echo "$total_power_V1V8_DCDC + $power_V1V8_DCDC" | bc)

        total_voltage_V3V3_DCDC=$(echo "$total_voltage_V3V3_DCDC + $voltage_V3V3_DCDC" | bc)
        total_current_V3V3_DCDC=$(echo "$total_current_V3V3_DCDC + $current_V3V3_DCDC" | bc)
        total_power_V3V3_DCDC=$(echo "$total_power_V3V3_DCDC + $power_V3V3_DCDC" | bc)

        total_power=$(echo "$total_power + $power" | bc)
    fi

    #temperature case
    test_output=$(dmesg -c > /dev/null && cat $pvt_debugfs && dmesg -c)

    echo "----------------------------------" >> "$power_result_file"
    echo "$test_output" >> "$power_result_file"

    temp_ts0=$(echo "$test_output" | grep 'TS0 Temperature' | awk -F'Temperature\\[' '{print $2}' | awk -F' C\\]' '{print $1}')
    temp_ts1=$(echo "$test_output" | grep 'TS1 Temperature' | awk -F'Temperature\\[' '{print $2}' | awk -F' C\\]' '{print $1}')
    temp_ts2=$(echo "$test_output" | grep 'TS2 Temperature' | awk -F'Temperature\\[' '{print $2}' | awk -F' C\\]' '{print $1}')

    total_temp_ts0=$(echo "$total_temp_ts0 + $temp_ts0" | bc)
    total_temp_ts1=$(echo "$total_temp_ts1 + $temp_ts1" | bc)
    total_temp_ts2=$(echo "$total_temp_ts2 + $temp_ts2" | bc)

    i=$((i + 1))

    echo "##################################" >> "$power_result_file"
    sleep $interval
done

echo "----------------------------------"

#power output
if [ $is_support_spidev = 1 ] ; then
    average_voltage_AVDD12_ABB=$(echo "scale=3; $total_voltage_AVDD12_ABB / $num_tests" | bc)
    average_current_AVDD12_ABB=$(echo "scale=3; $total_current_AVDD12_ABB / $num_tests" | bc)
    average_power_AVDD12_ABB=$(echo "scale=3; $total_power_AVDD12_ABB / $num_tests" | bc)

    average_voltage_V0V8_DCDC=$(echo "scale=3; $total_voltage_V0V8_DCDC / $num_tests" | bc)
    average_current_V0V8_DCDC=$(echo "scale=3; $total_current_V0V8_DCDC / $num_tests" | bc)
    average_power_V0V8_DCDC=$(echo "scale=3; $total_power_V0V8_DCDC / $num_tests" | bc)

    average_voltage_DDR_VDDQ=$(echo "scale=3; $total_voltage_DDR_VDDQ / $num_tests" | bc)
    average_current_DDR_VDDQ=$(echo "scale=3; $total_current_DDR_VDDQ / $num_tests" | bc)
    average_power_DDR_VDDQ=$(echo "scale=3; $total_power_DDR_VDDQ / $num_tests" | bc)

    average_voltage_V1V8_DCDC=$(echo "scale=3; $total_voltage_V1V8_DCDC / $num_tests" | bc)
    average_current_V1V8_DCDC=$(echo "scale=3; $total_current_V1V8_DCDC / $num_tests" | bc)
    average_power_V1V8_DCDC=$(echo "scale=3; $total_power_V1V8_DCDC / $num_tests" | bc)

    average_voltage_V3V3_DCDC=$(echo "scale=3; $total_voltage_V3V3_DCDC / $num_tests" | bc)
    average_current_V3V3_DCDC=$(echo "scale=3; $total_current_V3V3_DCDC / $num_tests" | bc)
    average_power_V3V3_DCDC=$(echo "scale=3; $total_power_V3V3_DCDC / $num_tests" | bc)

    average_power=$(echo "scale=3; $total_power / $num_tests" | bc)

    printf "              Voltage (mV)   Current (A)   Power (mW)\n"
    printf "AVDD12_ABB    %7.3f            %7.3f         %7.3f\n" $average_voltage_AVDD12_ABB $average_current_AVDD12_ABB $average_power_AVDD12_ABB
    printf "V0V8_DCDC     %7.3f            %7.3f         %7.3f\n" $average_voltage_V0V8_DCDC $average_current_V0V8_DCDC $average_power_V0V8_DCDC
    printf "DDR_VDDQ      %7.3f            %7.3f         %7.3f\n" $average_voltage_DDR_VDDQ $average_current_DDR_VDDQ $average_power_DDR_VDDQ
    printf "V1V8_DCDC     %7.3f            %7.3f         %7.3f\n" $average_voltage_V1V8_DCDC $average_current_V1V8_DCDC $average_power_V1V8_DCDC
    printf "V3V3_DCDC     %7.3f            %7.3f         %7.3f\n" $average_voltage_V3V3_DCDC $average_current_V3V3_DCDC $average_power_V3V3_DCDC
    printf "total                                        %7.3f\n" $average_power
    echo "----------------------------------"
fi

#temperature output
average_temp_ts0=$(echo "scale=3; $total_temp_ts0 / $num_tests" | bc)
average_temp_ts1=$(echo "scale=3; $total_temp_ts1 / $num_tests" | bc)
average_temp_ts2=$(echo "scale=3; $total_temp_ts2 / $num_tests" | bc)

printf "Average Temperature: TS0=%7.3f C, TS1=%7.3f C, TS2=%7.3f C\n" $average_temp_ts0 $average_temp_ts1 $average_temp_ts2
echo "##################################"
