#!/bin/bash
set -x
type=$1
ant=$2

if [ $# -ne 2 ]; then
    echo "parameter wrong!"
    echo "please input <board type> <antenna>"
    return;
fi

if [ $ant -ne 1 -a $ant -ne 3 ]; then
    echo "antenna configuration is not right."
    return;
fi

#fix channel
devmem 0x4010b004 32 $ant

if [ "$type" == "RDK" ]; then
    return;
fi

#close eth clock
devmem 0x9041220c 32 0x0
devmem 0x9041482c 32 0x0
devmem 0x9041402c 32 0x0
