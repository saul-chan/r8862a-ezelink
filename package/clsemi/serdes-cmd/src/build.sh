#!/bin/bash

set -e
echo "### Build serdes_cmd start ###"
export ARCH=arm64
export CROSS_COMPILE=${CROSS_COMPILE:-/opt/gcc-arm-11.2-2022.02-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-}

mkdir -p build rtk/build bin

make clean
make
cd rtk
make clean
make
#cp -f npecmd ../
echo "### Build serdes_cmd done ###"
