#!/bin/bash

set -e

echo "### Build cls_wifi_drv start ###"

if [ -z $PLAT_WORK_MODE ]
then
	export PLAT_WORK_MODE=EMU
fi

export ARCH=arm64
export CROSS_COMPILE=${CROSS_COMPILE:-/opt/gcc-arm-11.2-2022.02-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-}
export KERNELDIR=$1
export KERNELOUT=$2
export CONFIG_CLS_WIFI_HEMU_TX=y
export CONFIG_CLS_WIFI_DBG=y
export CONFIG_CLS_WIFI_BCMC=y
export CONFIG_DEBUG_IRF=y
export CONFIG_CLS_VBSS=n
export CONFIG_CLS_NAC=y
export CONFIG_CLS_SMTANT=y
if [ "$PLAT_WORK_MODE" == "EMU" ]
then
	export CONFIG_CLS_EMU=y
else
	export CONFIG_CLS_EMU=n
fi
export CONFIG_CLS_MSGQ=y
#export CONFIG_CLS_MSGQ_TEST=y
export CONFIG_CMCC_TEST=y
export CONFIG_CLS_SIGMA=y

if [ $# -lt 3 ]
then
	echo "Please input kernel path and kernel out path, something like:"
	echo "/.../kernel/build"
	echo "/.../kernel/build_out/modules"
	echo "arg3 is platform,dubhe2000 or merak2000"
	exit 1
fi

if [ $# -ge 3 ]; then
	case "$3" in
	dubhe2000)
		export CONFIG_PLATFORM_DUBHE2000=y
		export CONFIG_CLS_WIFI_VDEV_MAX=8
		export CONFIG_CLS_WIFI_STA_MAX=128
		export CONFIG_CLS_WIFI_USER_MAX=16
		export CONFIG_CLS_FWT=y
		export CONFIG_DYNAMIC_DEBUG=y
		export CONFIG_CLS_DUBHE_ETH=y
		export CONFIG_CLS_DBGCNT_HOST=y
		export CONFIG_CLS_WIFI_AMSDU_TX_OFFLOAD=0
		export CONFIG_CLS_HIGH_TEMP=y
		export CONFIG_CLS_3ADDR_BR=y
		;;
	merak2000)
		export CONFIG_PLATFORM_MERAK2000=y
		export CONFIG_CLS_WIFI_VDEV_MAX=4
		export CONFIG_CLS_WIFI_STA_MAX=20
		export CONFIG_CLS_WIFI_USER_MAX=16
		export CONFIG_CLS_FWT=n
		export CONFIG_CLS_DUBHE_ETH=y
		export CONFIG_CLS_DBGCNT_HOST=n
		export CONFIG_CLS_WIFI_AMSDU_TX_OFFLOAD=1
		;;
	*)
		echo "Unsupported project $3"
		exit 1
		;;
	esac
fi

if [ ! -d $KERNELDIR ]
then
	echo "Invalid kernel path: ${KERNELDIR}"
	exit 1
fi

if [ ! -d $KERNELOUT ]
then
	echo "Invalid kernel out path: ${KERNELOUT}"
	exit 1
fi


rm -rf *.o*
rm -rf .*.o*
rm -rf *.ko
make -j8
echo "### Build cls_wifi_drv done ###"
