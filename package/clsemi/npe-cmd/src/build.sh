#!/bin/bash

set -e
echo "### Build npecmd start ###"
export ARCH=arm64
export CROSS_COMPILE=${CROSS_COMPILE:-/opt/gcc-arm-11.2-2022.02-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-}
Project=$1

if [ -d "$Project" ] ; then
	echo "npecmd Project: $Project"
	cd $Project
	rm -rf *.o 
	rm -rf npecmd
	make
	cp -f npecmd ../
	echo "### Build $Project npecmd done ###"
else
	echo "Invalid npecmd Project"
	exit 1
fi
