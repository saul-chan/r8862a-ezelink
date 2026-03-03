#!/bin/bash

set -e

echo "### Build uboot start ###"

PLAT_WORK_MODE="EMU"
PLAT_NAME="emu"
DECONFIG_PATH="configs"
BOARD_FLASH_VARIANTS="pnand"
ROOTFS_INITRD_VARIANT="buildroot"
BUILD_ALL_UBOOT_IMAGE=0
COMPILE_CLEAN=1
PRJ_NAME="dubhe1000"

check_board_flash_variants() {
	if [ "$1" == "pnand" ] || [ "$1" == "spinor" ] || [ "$1" == "spinand" ];  then
		BOARD_FLASH_VARIANTS="$1"
	else
		echo "Invalid flash variants '$1'"
		exit 1;
	fi
}

check_board_rootfs_variants() {
	if [ "$1" == "buildroot" ] ;  then
		ROOTFS_INITRD_VARIANT="$1"
	else
		echo "Invalid rootfs variants '$1'"
		exit 1;
	fi
}

check_platform_work_mode() {
        if [ "$1" == "EMU" ] || [ "$1" == "FPGA" ] || [ "$1" == "SoC" ] ||
	   [ "$1" == "EVB-A" ] || [ "$1" == "EVB-B" ] ||
	   [ "$1" == "EVB-E" ] || [ "$1" == "EVB-N" ] || [ "$1" == "EVB-M3K" ] ||
	   [ "$1" == "RDK" ] || [ "$1" == "RDK-T" ] || [ "$1" == "RDK-S" ] || [ "$1" == "RDK-C" ] ||
	   [ "$1" == "DEMO-A" ] || [ "$1" == "DEMO-B" ] || [ "$1" == "DEMO-R" ] ||
	   [ "$1" == "DEMO-O" ] ;  then
                PLAT_WORK_MODE="$1"
		PLAT_NAME=$(echo $PLAT_WORK_MODE | tr 'A-Z' 'a-z')
        else
                echo "Invalid platform work mode '$1'"
                exit 1;
        fi

}

check_project() {
	PRJ_NAME=$(echo $1 | tr 'A-Z' 'a-z')
	if [ "$PRJ_NAME" != "dubhe1000" ] && [ "$PRJ_NAME" != "dubhe2000" ];  then
		echo "Invalid project '$1'"
		exit 1;
	fi
}
compile_process() {
deconfig_file="$DECONFIG_PATH/$1"

if [ ${COMPILE_CLEAN} -eq 1 ]; then
	make clean
fi
	make $1
	make -j8
}

usage() {
	printf "Usage: ./%s -f pnand  -r busybox" "$(basename "$0")"
	printf "\n\t-c ==> set uboot complime with make clean fisrt "
	printf "\n\t-f ==> set uboot board flash variant string 'pnand' 'spinand' 'spinor'"
	printf "\n\t-m ==> set platform work mode variant string 'EMU' 'FPGA' 'SOC'"
	printf "\n\t-p ==> set project variant string 'dubhe1000' 'dubhe2000'"
	printf "\n\t-r ==> set uboot rootfs initrd variant string 'busybox' 'buildroot'"
	printf "\n\t\t\n"
	exit 1
}

while getopts "f:m:p:r:c" OPTION
do
        case $OPTION in
		c ) COMPILE_CLEAN=1 ;;
		f ) check_board_flash_variants  "$OPTARG" ;;
		m ) check_platform_work_mode    "$OPTARG" ;;
		p ) check_project               "$OPTARG" ;;
		r ) check_board_rootfs_variants "$OPTARG" ;;
		* ) echo "Invalid build.sh option passed to '$0' (options:$*)"
		usage;;
        esac
done

export ARCH=arm64
export CROSS_COMPILE=/opt/gcc-arm-11.2-2022.02-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-
rm -rf output
mkdir -p output

if [ "$PLAT_WORK_MODE" == "EVB-A" ] || [ "$PLAT_WORK_MODE" == "EVB-B" ] ||
	[ "$PLAT_WORK_MODE" == "EVB-E" ] || [ "$PLAT_WORK_MODE" == "EVB-N" ] || [ "$PLAT_WORK_MODE" == "EVB-M3K" ] ||
	[ "$PLAT_WORK_MODE" == "RDK" ] || [ "$PLAT_WORK_MODE" == "RDK-T" ] || [ "$PLAT_WORK_MODE" == "RDK-S" ] || [ "$PLAT_WORK_MODE" == "RDK-C" ] ||
	[ "$PLAT_WORK_MODE" == "DEMO-A" ] || [ "$PLAT_WORK_MODE" == "DEMO-B" ] || [ "$PLAT_WORK_MODE" == "DEMO-R" ] ||
	[ "$PLAT_WORK_MODE" == "DEMO-O" ] ;  then
	PRJ_UBOOT_DECONFIG_FILE="$PRJ_NAME"_"$PLAT_NAME"_defconfig
	OUTPUT_UBOOT_BINARY=u-boot-"$PRJ_NAME"-"$PLAT_NAME".bin
	echo "$PRJ_UBOOT_DECONFIG_FILE"
	echo "$OUTPUT_UBOOT_BINARY"
else
	PRJ_UBOOT_DECONFIG_FILE="$PRJ_NAME"_"$PLAT_NAME"_"$BOARD_FLASH_VARIANTS"_"$ROOTFS_INITRD_VARIANT"_defconfig
	OUTPUT_UBOOT_BINARY=u-boot-"$PRJ_NAME"-"$PLAT_NAME"-"$BOARD_FLASH_VARIANTS"-"$ROOTFS_INITRD_VARIANT".bin
fi

compile_process $PRJ_UBOOT_DECONFIG_FILE
cp -rf u-boot.bin output/$OUTPUT_UBOOT_BINARY

if [ "$PLAT_WORK_MODE" == "EVB-A" ] ; then
	make clean
	PRJ_UBOOT_DECONFIG_FILE="$PRJ_NAME"_"$PLAT_NAME"_defconfig
	PRJ_SIGN_UBOOT_DECONFIG_FILE="$PRJ_NAME"-append-sign-check-config
	OUTPUT_UBOOT_SIGN_BINARY=u-boot-"$PRJ_NAME"-"$PLAT_NAME"-sign.bin
	#backup config and dts
	cp configs/$PRJ_UBOOT_DECONFIG_FILE configs/$PRJ_UBOOT_DECONFIG_FILE.bak
	cp arch/arm/dts/clourneysemi-"$PRJ_NAME"-"$PLAT_NAME".dts arch/arm/dts/clourneysemi-"$PRJ_NAME"-"$PLAT_NAME".dts.bak

	#append config and dts
	cat configs/$PRJ_SIGN_UBOOT_DECONFIG_FILE >> configs/$PRJ_UBOOT_DECONFIG_FILE
	cat arch/arm/dts/clourneysemi-"$PRJ_NAME"-append-sign-check.dts >> arch/arm/dts/clourneysemi-"$PRJ_NAME"-"$PLAT_NAME".dts
	echo "$PRJ_SIGN_UBOOT_DECONFIG_FILE"
	echo "$OUTPUT_UBOOT_SIGN_BINARY"
	compile_process $PRJ_UBOOT_DECONFIG_FILE
	cp -rf u-boot.bin output/$OUTPUT_UBOOT_SIGN_BINARY
	#restore config and dts
	mv configs/$PRJ_UBOOT_DECONFIG_FILE.bak configs/$PRJ_UBOOT_DECONFIG_FILE
	mv arch/arm/dts/clourneysemi-"$PRJ_NAME"-"$PLAT_NAME".dts.bak arch/arm/dts/clourneysemi-"$PRJ_NAME"-"$PLAT_NAME".dts
fi

echo "### Build uboot done ###"
