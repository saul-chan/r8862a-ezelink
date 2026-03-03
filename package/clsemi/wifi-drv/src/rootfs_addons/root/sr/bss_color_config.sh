#!/bin/sh

band=1
bss_color=6
enable=1
partial_enable=0
bss_color_offset=`printf %d 0x104`

usage()
{
    echo "Usage: $0 [-band:<band>] [-enable:<enable>] [-partial_enable:<enable>] [-bss_color:<bss_color>] [-h]"
    echo "  -band            : 0-2.4G, 1-5G"
    echo "  -enable          : 0-disable, 1-enable"
    echo "  -partial_enable  : 0-disable, 1-enable"
    echo "  -bss_color       : bss color from 0~63"
    echo "  -h               : display help"
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
    -enable:*)
        enable=${1#-enable:}
        ;;
    -partial_enable:*)
        partial_enable=${1#-partial_enable:}
        ;;
    -bss_color:*)
        bss_color=${1#-bss_color:}
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

bss_color_reg_addr=`expr $base_reg + $bss_color_offset`
bss_color_reg=0x`printf %x $bss_color_reg_addr`
bss_color_enable_val=$((enable<<8))
partial_bss_color_enable_val=$((partial_enable<<9))
val=$((bss_color|$bss_color_enable_val|$partial_bss_color_enable_val))
val_hex=0x`printf %x $val`
set_reg $bss_color_reg $val_hex

echo "Done"
