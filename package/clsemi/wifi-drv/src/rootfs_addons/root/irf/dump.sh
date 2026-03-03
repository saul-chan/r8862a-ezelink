#!/bin/bash
reg=$1

devmem $reg

for i in $(seq $2)
do
reg=$(($reg + 4))
hexreg=`echo "obase=16;$reg" | bc`
devmem 0x$hexreg
done
