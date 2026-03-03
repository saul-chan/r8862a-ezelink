#!/bin/bash

set -e

echo "### Build pvt_drv start ###"
export ARCH=arm64
export CROSS_COMPILE=${CROSS_COMPILE:-/opt/gcc-arm-11.2-2022.02-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-}
export KERNELDIR=$1
export KERNELOUT=$2
Project=${3:-dubhe2000}

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


if [ "$PRJ_NAME" == "Dubhe2000" ] ; then
	Project=dubhe2000
fi

if [ -d "$Project" ] ; then
	echo "pvt_drv Project: $Project"
	cd $Project
	rm -rf *.o *.ko .*cmd .tmp*
	make -j8
	cp dubhe*.ko ../
	echo "### Build $Project pvt_drv done ###"
else
	echo "Invalid pvt_drv Project"
	exit
fi


