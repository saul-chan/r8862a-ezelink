#!/bin/bash
usage() {
	printf "Usage: %s" "$(basename "$0")"
	printf "\n\t-p ==> I2C Interface Port Number"
	printf "\n\t-n ==> I2C Interface Pad Number"
	printf "\n\r"
	exit 1
}

POSITIVE_TEMP=0x800
NEGATIVE_TEMP=0x1000
TMP75_UNIT=0.0625

I2C_Port=2
I2C_No=1
i2cdevX_pinmux() {
	io_set.sh -i I2C -p $1 -n $2
}

while getopts "p:n:" OPTION
do
	case $OPTION in
		p )   I2C_Port="$OPTARG"
			;;
		n )   I2C_No="$OPTARG"
			;;
		* ) echo "Invalid option passed to '$0' (options:$*)"
			  usage;;
	esac
done

i2cdevX_pinmux $I2C_Port $I2C_No

i2cdevX_node=$(ls /dev/i2c-*)

i2cdev_no=2
for i2cdev in $i2cdevX_node;
do
	i2cdev_node=${i2cdev##*dev/}
	i2cdev_name=$(sed -n '1p' /sys/bus/i2c/devices/$i2cdev_node/of_node/label)
	i2cdev_no=${i2cdev_name: -1}
	if [ $i2cdev_no -eq 2 ]; then
		i2cdev_no=${i2cdev: -1}
		break
	fi
done

# Configuration register setting R1 R0 12-bits Converter Resolution
i2ctransfer -f -y $i2cdev_no w2@0x48 0x01 0x60

tempdata1=$(($(i2ctransfer -f -y $i2cdev_no w1@0x48 0x00 r2 | awk '{print $1}') * 16))
tempdata0=$(($(i2ctransfer -f -y $i2cdev_no w1@0x48 0x00 r2 | awk '{print $2}') / 16))
tempdata=$((tempdata1 + tempdata0))

if [ $((tempdata)) -lt $((POSITIVE_TEMP)) ]; then
	show_temp=$(awk 'BEGIN{printf "%.3f\n",'$tempdata'*'$TMP75_UNIT'}')
fi

if [ $((tempdata)) -gt $((POSITIVE_TEMP)) ]; then
        show_temp="-$(awk 'BEGIN{printf "%.3f\n",'$((NEGATIVE_TEMP - tempdata))'*'$TMP75_UNIT'}')"
fi

printf "TMP75 Temperature: %s C\r\n" $show_temp
