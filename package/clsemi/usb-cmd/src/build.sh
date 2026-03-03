#!/bin/bash

set -e
echo "### Build usbcmd start ###"
export ARCH=arm64
export CROSS_COMPILE=${CROSS_COMPILE:-/opt/gcc-arm-11.2-2022.02-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-}

mkdir -p build bin

rm -rf *.o
rm -rf usbcmd
make
echo "### Build usbcmd done ###"
