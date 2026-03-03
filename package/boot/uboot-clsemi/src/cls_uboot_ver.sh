#!/bin/bash -x

CLS_UBOOT_VER=V1.0.4
FULL_TYPE=$1

if [ "$1" == "syncconfig" ];then
	exit
fi

export board_type="$(echo $FULL_TYPE | awk -F'_' '{print $2}')"
echo ${board_type}
sed -i '/^CONFIG_LOCALVERSION=/d' .config
sed -i '/^CONFIG_LOCALVERSION_AUTO=/d' .config
echo ${CLS_UBOOT_VER}
echo ${board_type}
echo "CONFIG_LOCALVERSION=\"${CLS_UBOOT_VER}-${board_type}\"" >> .config
echo "CONFIG_LOCALVERSION_AUTO=y" >> .config
