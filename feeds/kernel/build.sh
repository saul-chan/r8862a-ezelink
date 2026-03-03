#!/bin/bash
export PATH=/opt/gcc-arm-11.2-2022.02-x86_64-aarch64-none-linux-gnu/bin/:$PATH

CURR_DIR=$(pwd)
KERENL_DTS_DIR="$CURR_DIR/arch/arm64/boot/dts/clourney"
KERENL_DECFG_DIR="$CURR_DIR/arch/arm64/configs"
IMAGE_DIR="$CURR_DIR/build/arch/arm64/boot"
OUTPUT_DIR="$CURR_DIR/build_out"
BUILD_DIR="$CURR_DIR/build"
cpu_core=$(cat /proc/cpuinfo| grep "processor"| wc -l)
MakeMode="all"
PRJ_NAME="dubhe2000"
VENDER_INFO="clourney"
PLAT_NAME="emu"
FLASH_MODULE_TYPE="pnand"
WIFI_2G_MODULE_NAME="wifi0"
WIFI_5G_MODULE_NAME="wifi1"
NPE_MODULE_NAME="npe"
NPE_MODULE_FLAG=1
WIFI_2G_MODULE_FLAG=1
WIFI_5G_MODULE_FLAG=1

usage() {
	printf "Usage: %s -i kernel.bin" "$(basename "$0")"

	printf "\n\t all ==> build full kernel image with cleaning build folder"
	printf "\n\t only ==> build full kernel image without cleaning build folder"
	printf "\n\t menuconfig ==> build kernel menuconfig with parameter '-defconfig' using private deconfig file"
	printf "\n\t-defconfig: ==> using a private kernel deconfig file"
	printf "\n\t-prj: ==> set chip project name variant string 'dubhe1000' 'dubhe2000'"
	printf "\n\t-flash: ==> board flash variant string 'pnand' 'spinand' 'spinor'"
	printf "\n\t-npe: ==> kernel module npe enable flash 'yes' 'no' '1' '0'"
	printf "\n\t-wifi_2g: ==> kernel module wifi_2g enable flash 'yes' 'no' '1' '0'"
	printf "\n\t-wifi_5g: ==> kernel module wifi_5g enable flash 'yes' 'no' '1' '0'"
	printf "\n\r"
	exit 1
}


while test $# -gt 0
do
    case "$1" in
        all*)
                MakeMode="all"
            ;;
        only*)
                MakeMode="only"
            ;;
	menuconfig*)
		MakeMode="menuconfig"
	    ;;
	-defconfig:*) kernel_deconfig=( ${1#-defconfig:} )
		if [ ! -f "$KERENL_DECFG_DIR/$kernel_deconfig" ]; then
			echo "$kernel_deconfig is not exist, will use default dubhe1000_tiny_defconfig"
			kernel_deconfig="dubhe2000_tiny_defconfig"
		fi
	   ;;
        -flash:*)  FLASH_MODULE_TYPE=( ${1#-flash:} )
		if [ "$FLASH_MODULE_TYPE" == "pnand" ] || [ "$FLASH_MODULE_TYPE" == "spinor" ] || [ "$FLASH_MODULE_TYPE" == "spinand" ];  then
			echo "flash variants '$FLASH_MODULE_TYPE'"
		else
			echo "Invalid flash variants '$FLASH_MODULE_TYPE'"
			usage
		fi
            ;;
        -plat:*)  PLAT_WORK_MODE=( ${1#-plat:} )
                if [ "$PLAT_WORK_MODE" == "EMU" ] || [ "$PLAT_WORK_MODE" == "FPGA" ] || [ "$PLAT_WORK_MODE" == "SoC" ] || \
			[ "$PLAT_WORK_MODE" == "EVB-A" ] || [ "$PLAT_WORK_MODE" == "EVB-B" ] || \
			[ "$PLAT_WORK_MODE" == "RDK" ] || \
			[ "$PLAT_WORK_MODE" == "DEMO-R" ] || [ "$PLAT_WORK_MODE" == "DEMO-O" ] ;  then
                        echo "Compile for $PLAT_WORK_MODE platform"
			PLAT_NAME=$(echo $PLAT_WORK_MODE | tr 'A-Z' 'a-z')
                else
                        echo "Invalid platfomr variants '$PLAT_WORK_MODE'"
                        usage
                fi
            ;;
	-prj:*)  PRJ_STRING=( ${1#-prj:} )
		PRJ_NAME=$(echo $PRJ_STRING | tr 'A-Z' 'a-z')
		if [ "$PRJ_NAME" != "dubhe1000" ] && [ "$PRJ_NAME" != "dubhe2000" ];  then
			echo "Invalid project '$PRJ_STRING'"
			exit 1;
		fi
	    ;;
	-npe:*)  NPE_MODULE_FLAG=( ${1#-npe:} )
	        if [ "$NPE_MODULE_FLAG" == "yes" ] || [ "$NPE_MODULE_FLAG" == "1" ]; then
			NPE_MODULE_FLAG=1
		else
		        NPE_MODULE_FLAG=0
	        fi
	    ;;
	-wifi_2g:*)  WIFI_2G_MODULE_FLAG=( ${1#-wifi_2g:} )
                if [ "$WIFI_2G_MODULE_FLAG" == "yes" ] || [ "$WIFI_2G_MODULE_FLAG" == "1" ] ; then
                        WIFI_2G_MODULE_FLAG=1
                else
                        WIFI_2G_MODULE_FLAG=0
                fi
            ;;
	-wifi_5g:*)  WIFI_5G_MODULE_FLAG=( ${1#-wifi_5g:} )
                if [ "$WIFI_5G_MODULE_FLAG" == "yes" ] || [ "$WIFI_5G_MODULE_FLAG" == "1" ] ; then
                        WIFI_5G_MODULE_FLAG=1
                else
                        WIFI_5G_MODULE_FLAG=0
                fi
	    ;;
		* ) echo "Invalid option passed to '$0' (options:$*)"
		usage
		exit 1;;
    esac
    shift
done

kernel_deconfig="$PRJ_NAME"_"tiny_defconfig"
kernel_dtb="$VENDER_INFO-$PRJ_NAME-*.dtb"

if [ "$PLAT_WORK_MODE" == "EVB-A" ] || [ "$PLAT_WORK_MODE" == "EVB-B" ] || [ "$PLAT_WORK_MODE" == "RDK" ] || [ "$PLAT_WORK_MODE" == "DEMO-R" ] || [ "$PLAT_WORK_MODE" == "DEMO-O" ] ;  then
	kernel_dts="$VENDER_INFO"-"$PRJ_NAME"-"$PLAT_NAME".dts
	echo "using dts $kernel_dts"
else
	kernel_dts="$VENDER_INFO"-"$PRJ_NAME"-"$PLAT_NAME"-"$FLASH_MODULE_TYPE".dts
	echo "using dts $kernel_dts"
fi

update_kernel_modules_status_info() {
string_line_no=0
dts_file="$KERENL_DTS_DIR/$kernel_dts"

	if [ "$1" == "$WIFI_2G_MODULE_NAME" ] || [ "$1" == "$WIFI_5G_MODULE_NAME" ] || [ "$1" == "$NPE_MODULE_NAME" ]; then
		match_string="$1"
		string_line_no="$(awk '/'${match_string}'/ {print NR}' "$dts_file" )"
		#echo "string_line_no $string_line_no"
		if [ "$string_line_no" != "" ]; then
			if [ $2 -eq 1 ]; then
				replace_string=$(sed -n ' '${string_line_no}'p ' $dts_file | sed 's/disable/ok/g')
				sed -i ' '${string_line_no}'s/disable/ok/g' $dts_file
				echo "enable $1 module in dts"
			else
				replace_string=$(sed -n ' '${string_line_no}'p ' $dts_file | sed 's/ok/disable/g')
				sed -i ' '${string_line_no}'s/ok/disable/g' $dts_file
				echo "disable $1 module in dts"
			fi
			#echo $replace_string
		else
			echo "$kernel_dts $1 dtb node not find "
			exit 1
		fi
	else
		echo "keep $kernel_dts old info"
	fi
}

if [ "$MakeMode" == "all" ]; then
#clean build
rm -rf  build build_out
mkdir -p build_out
if [ -f .config ]; then
	echo "kernel source code is not clean, make distclean first"
	make ARCH=arm64 distclean
	make ARCH=arm64 mrproper
fi

echo "building kernel $kernel_deconfig ... this can take a while"
make $kernel_deconfig ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- -C ./ O=build > $OUTPUT_DIR/kernel_defconfig.log 2>&1
fi

if [ "$MakeMode" == "only" ] || [ "$MakeMode" == "menuconfig" ]; then

	if [[ -d build ]] && [[ -d build_out ]]; then
		echo "alreay have build and build_out folder"
		rm -rf build_out/modules
	else
		rm -rf  build build_out
		mkdir -p build_out
		if [ -f .config ]; then
			echo "kernel source code is not clean, make distclean first"
			make ARCH=arm64 distclean
			make ARCH=arm64 mrproper
		fi
		echo "building kernel $kernel_deconfig ... this can take a while"
		make $kernel_deconfig ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- -C ./ O=build > $OUTPUT_DIR/kernel_defconfig.log 2>&1
	fi

	if [ "$MakeMode" == "menuconfig" ]; then
		make menuconfig ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- -C ./ O=build
		echo "copy build/.config to $KERENL_DECFG_DIR/$kernel_deconfig"
		cp -rf build/.config $KERENL_DECFG_DIR/$kernel_deconfig
		exit 0
	fi
fi

echo "building kernel ... this can take a while"
echo "compile kernel with $cpu_core thread"

update_kernel_modules_status_info npe $NPE_MODULE_FLAG

update_kernel_modules_status_info wifi0 $WIFI_2G_MODULE_FLAG

update_kernel_modules_status_info wifi1 $WIFI_5G_MODULE_FLAG

make -j$cpu_core ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- -C ./ O=build  > $OUTPUT_DIR/kernel_compile.log 2>&1

echo "build kernel image successful"

make -j$cpu_core modules ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- -C ./ O=build >> $OUTPUT_DIR/kernel_compile.log 2>&1

echo "build kernel modules successful"

make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- -C ./ O=build modules_install INSTALL_MOD_PATH=../build_out/modules >> $OUTPUT_DIR/kernel_compile.log 2>&1

echo "done: building kernel and modules"

cd $IMAGE_DIR
cp Image Image.gz dts/clourney/$kernel_dtb $OUTPUT_DIR
cp $BUILD_DIR/System.map $BUILD_DIR/vmlinux $OUTPUT_DIR

echo "List $OUTPUT_DIR:"
cd $OUTPUT_DIR
ls -al
