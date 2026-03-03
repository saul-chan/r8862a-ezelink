#!/bin/bash

band=$1
mod=$2

if [ "$band" == "2g" -a "$mod" == "11b" ]; then
	devmem 0x4030b304 32 0x30003000
	devmem 0x402c0098 32 0
	devmem 0x402c009c 32 0
	devmem 0x402c00a0 32 0
	devmem 0x402c00a4 32 0
	devmem 0x402c00a8 32 0
	devmem 0x402c00ac 32 0
	devmem 0x402c00b0 32 0
	devmem 0x402c00b4 32 1024
elif [ "$band" == "2g" ]; then
	devmem 0x4030b304 32 0x35003500
elif [ "$band" == "5g" ]; then
	devmem 0x4010b304 32 0x35003500
fi

echo "prepare aci done!"
