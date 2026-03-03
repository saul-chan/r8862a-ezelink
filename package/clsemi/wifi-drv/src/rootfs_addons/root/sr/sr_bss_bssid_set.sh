#!/bin/sh

band=1
bss_color=0
partial_bssid=0
clear_flag=0

bss_color_low_offset=`printf %d 0x110`
bss_color_high_offset=`printf %d 0x114`
partial_bssid_low_offset=`printf %d 0x118`
partial_bssid_high_offset=`printf %d 0x11C`

usage()
{
    echo "Usage: $0 [-band:<band>] [-bss_color:<bss_color>] [-partial_bssid:<partial_bssid>] [-clear:<clear>] [-h]"
    echo "  -band          : 0-2.4G, 1-5G"
    echo "  -bss_color     : bss color from 0~63"
    echo "  -partial_bssid : partial bssid from 0~63"
    echo "  -clear         : 1 - clear bss color and partial bssid, 0 - not clear"
    echo "  -h             : display help"
}

set_reg()
{
    set -x
    devmem $1 32 $2
    set +x
}

# parse input parameters
while test $# -gt 0
do
    case "$1" in
    -band:*)
        band=${1#-band:}
        ;;
    -bss_color:*)
        bss_color=${1#-bss_color:}
        ;;
    -partial_bssid:*)
        partial_bssid=${1#-partial_bssid:}
        ;;
    -clear:*)
        clear_flag=${1#-clear:}
        ;;
    -h)
        usage
        exit 0
        ;;
    esac
    shift
done

if [ $band -eq 1 ]; then
    base_reg=`printf %d 0xacb00000`
elif [ $band -eq 0 ]; then
    base_reg=`printf %d 0xadb00000`
else
    echo "radio not support!"
    exit 0;
fi

bss_color_low_addr=`expr $base_reg + $bss_color_low_offset`
bss_color_low_reg=0x`printf %x $bss_color_low_addr`
bss_color_high_addr=`expr $base_reg + $bss_color_high_offset`
bss_color_high_reg=0x`printf %x $bss_color_high_addr`

partial_bssid_low_addr=`expr $base_reg + $partial_bssid_low_offset`
partial_bssid_low_reg=0x`printf %x $partial_bssid_low_addr`
partial_bssid_high_addr=`expr $base_reg + $partial_bssid_high_offset`
partial_bssid_high_reg=0x`printf %x $partial_bssid_high_addr`

if [ $clear_flag -eq 1 ]; then
    set_reg $bss_color_low_reg 0x0
    set_reg $bss_color_high_reg 0x0
    set_reg $partial_bssid_low_reg 0x0
    set_reg $partial_bssid_high_reg 0x0
    exit 0;
fi

if [ $bss_color -gt 31 ]; then
    bss_color_tmp=`expr $bss_color - 31`
    bss_color_hex=$((1<<$bss_color_tmp))
    bss_color_val=0x`printf %x $bss_color_hex`
    set_reg $bss_color_low_reg 0x0
    set_reg $bss_color_high_reg $bss_color_val
else
    bss_color_hex=$((1<<$bss_color))
    bss_color_val=0x`printf %x $bss_color_hex`
    set_reg $bss_color_low_reg $bss_color_val
    set_reg $bss_color_high_reg 0x0
fi

if [ $partial_bssid -gt 31 ]; then
    partial_bssid_tmp=`expr $partial_bssid - 31`
    partial_bssid_hex=$((1<<$partial_bssid_tmp))
    partial_bssid_val=0x`printf %x $partial_bssid_hex`
    set_reg $partial_bssid_low_reg 0x0
    set_reg $partial_bssid_high_reg $partial_bssid_val
else
    partial_bssid_hex=$((1<<$partial_bssid))
    partial_bssid_val=0x`printf %x $partial_bssid_hex`
    set_reg $partial_bssid_low_reg $partial_bssid_val
    set_reg $partial_bssid_high_reg 0x0
fi

echo "Done"
