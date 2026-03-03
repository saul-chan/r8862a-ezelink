#!/bin/bash

band=$1

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <band: 5g/2g/all>"
    exit 1
fi

rm /tmp/reg_*.dat 2>/dev/null

dif_com_base=0x40600000
dif_com_len=65536
dif_smp_fft_base=0x40620000
dif_smp_fft_len=131072

dif_sub_5g=0x40000000
dif_sub_5g_len=1572864

dif_sub_2g=0x40200000
dif_sub_2g_len=1109504

dpd_sub_5g=0x40400000
dpd_sub_5g_len=196608

dpd_sub_2g=0x40500000
dpd_sub_2g_len=196608

if [ "$band" == "5G" -o "$band" == "5g" ]; then
    memdump $dif_com_base $dif_com_len hex 0 /tmp/reg_${dif_com_base}.dat
    memdump $dif_sub_5g $dif_sub_5g_len hex 0 /tmp/reg_${dif_sub_5g}.dat
    memdump $dpd_sub_5g $dpd_sub_5g_len hex 0 /tmp/reg_${dpd_sub_5g}.dat
elif [ "$band" == "2G" -o "$band" == "2g" ]; then
    memdump $dif_com_base $dif_com_len hex 0 /tmp/reg_${dif_com_base}.dat
    memdump $dif_sub_2g $dif_sub_2g_len hex 0 /tmp/reg_${dif_sub_2g}.dat
    memdump $dpd_sub_2g $dpd_sub_2g_len hex 0 /tmp/reg_${dpd_sub_2g}.dat
else
    memdump $dif_com_base $dif_com_len hex 0 /tmp/reg_${dif_com_base}.dat
    memdump $dif_sub_5g $dif_sub_5g_len hex 0 /tmp/reg_${dif_sub_5g}.dat
    memdump $dpd_sub_5g $dpd_sub_5g_len hex 0 /tmp/reg_${dpd_sub_5g}.dat
    memdump $dif_sub_2g $dif_sub_2g_len hex 0 /tmp/reg_${dif_sub_2g}.dat
    memdump $dpd_sub_2g $dpd_sub_2g_len hex 0 /tmp/reg_${dpd_sub_2g}.dat
fi
