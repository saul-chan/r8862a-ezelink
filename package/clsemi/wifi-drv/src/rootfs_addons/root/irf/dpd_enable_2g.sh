#!/bin/bash

opt=$1

if [ "$opt" == "enable" ]; then
    devmem 0x405201c4 32 0x0
	devmem 0x405221c4 32 0x0
    echo "Enable pdcore!"
elif [ "$opt" == "disable" ]; then
    devmem 0x405201c4 32 0x1
	devmem 0x405221c4 32 0x1
    echo "Disable pdcore!"
else
    echo "Wrong parmeter!"
fi
