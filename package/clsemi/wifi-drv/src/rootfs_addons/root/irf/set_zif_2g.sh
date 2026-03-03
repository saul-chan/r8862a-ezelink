#!/bin/bash
set -x
sw=$1
ch=$2

if [ $# -ne 2 ]; then
    echo "parameter wrong!"
    echo "please input <enable/disable> <channel>"
    return;
fi

if [ $ch -eq 0 ]; then
    base=0x40520284
elif [ $ch -eq 1 ]; then
    base=0x40522284
else
    echo "channel is not correct!"
    return;
fi

if [ "$sw" == "enable" ]; then
    devmem $base 32 0
elif [ "$sw" == "disable" ]; then
    devmem $base 32 2
fi
