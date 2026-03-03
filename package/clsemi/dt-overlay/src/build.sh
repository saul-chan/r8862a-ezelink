#!/bin/bash

set -e

echo "### Build dtoverlay_driver start ###"
export ARCH=arm64
export CROSS_COMPILE=${CROSS_COMPILE:-/opt/gcc-arm-11.2-2022.02-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-}
export KERNELDIR=$1
export KERNELOUT=$2
export CONFIG_DUBHE1000_MODULE=y
export CONFIG_ARM_CCI=y
#export CONFIG_NET_POLL_CONTROLLER=y
#export CONFIG_DUBHE1000_ENABLE_FWD_TEST=y
#export CONFIG_DUBHE1000_ENABLE_LOOPBACK=y

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
make clean
make -j8
echo "### Build dtoverlay_driver done ###"

