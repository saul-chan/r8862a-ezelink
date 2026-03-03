#!/bin/bash

set -e

echo "### Build fwt_drv start ###"
export ARCH=arm64
export CROSS_COMPILE=${CROSS_COMPILE:-/opt/gcc-arm-11.2-2022.02-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-}
export KERNELDIR=$1
export KERNELOUT=$2

if [ $# -lt 2 ]
then
	echo "Please input kernel path and kernel out path, something like:"
	echo "/.../kernel/build"
	echo "/.../kernel/build_out/modules"
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

rm -rf *.o *.ko .*cmd .tmp*
make -j8
echo "### Build fwt_drv done ###"


