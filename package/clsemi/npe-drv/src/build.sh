#!/bin/bash

set -e

echo "### Build npe_drv start ###"
export ARCH=arm64
export CROSS_COMPILE=${CROSS_COMPILE:-/opt/gcc-arm-11.2-2022.02-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-}
export KERNELDIR=$1
export KERNELOUT=$2
export CONFIG_DUBHE1000_MODULE=y
export CONFIG_ARM_CCI=y
#export CONFIG_NET_POLL_CONTROLLER=y
#export CONFIG_DUBHE1000_ENABLE_FWD_TEST=y
#export CONFIG_DUBHE1000_ENABLE_LOOPBACK=y

Project=$3

if [ $# -lt 2 ]
then
	echo "Please input kernel path and kernel out path, something like:"
	echo "/.../kernel/build"
	echo "/.../kernel/build_out/modules"
	echo "arg3 is platform,dubhe1000 or dubhe2000"
	exit 1
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

if [ -z $PLAT_WORK_MODE ]
then
	export PLAT_WORK_MODE=EMU
fi
if [ -d "$Project" ] ; then
	echo "npe_drv Project: $Project"
	cd $Project
	rm -rf *.o *.ko .*cmd .tmp*
	find ../rtk/ -name "*.o" -exec rm {} \;
	find ../rtk/ -name ".*cmd" -exec rm {} \;
	find ../rtk/ -name ".*tmp*" -exec rm {} \;
	make -j8
	cp cls_npe*.ko ../
	echo "### Build $Project npe_drv done ###"
else
	echo "Invalid npe_drv Project"
	exit 1
fi
