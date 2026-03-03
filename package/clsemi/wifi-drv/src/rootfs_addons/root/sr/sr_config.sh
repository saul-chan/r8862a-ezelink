#!/bin/sh

band=1
enable=1
non_srg_offset=10
srg_max_offset=10
srg_min_offset=0
rate_adapt=1

hecfg_offset=`printf %d 0x108`
srcfg_offset=`printf %d 0x10C`
sr_pwr_offset=`printf %d 0x4070`
rate_adapt_offset=`printf %d 0x4074`
rate_adapt_ht1_offset=`printf %d 0x4078`
rate_adapt_ht2_offset=`printf %d 0x407C`
rate_adapt_leg_offset=`printf %d 0x4080`
rate_adapt_eht_offset=`printf %d 0x50D4`
sr_optm_ctrl_offset=`printf %d 0x50C4`
sr_ppdu_impr_offset=`printf %d 0x50CC`

srcfg_val=0

usage()
{
    echo "Usage: $0 [-band:<band>] [-enable:<enable>] [non_srg:<offset>] [srg_min:<offset>] [srg_max:<offset>] [rate_adapt:<enable>] [-h]"
    echo "  -band       : 0-2.4G, 1-5G"
    echo "  -enable     : 0-disable, 1-enable"
    echo "  -non_srg    : max offset from -82dBm for non-SRG"
    echo "  -srg_min    : min offset from -82dBm for SRG"
    echo "  -srg_max    : max offset from -82dBm for SRG"
    echo "  -rate_adapt : 0-disable, 1-enable"
    echo "  -h          : display help"
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
    -non_srg:*)
        non_srg_offset=${1#-non_srg:}
        ;;
    -srg_min:*)
        srg_min_offset=${1#-srg_min:}
        ;;
    -srg_max:*)
        srg_max_offset=${1#-srg_max:}
        ;;
    -rate_adapt:*)
        rate_adapt=${1#-rate_adapt:}
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

hecfg_reg_addr=`expr $base_reg + $hecfg_offset`
hecfg_reg=0x`printf %x $hecfg_reg_addr`
srcfg_reg_addr=`expr $base_reg + $srcfg_offset`
srcfg_reg=0x`printf %x $srcfg_reg_addr`
sr_pwr_addr=`expr $base_reg + $sr_pwr_offset`
sr_pwr_reg=0x`printf %x $sr_pwr_addr`
sr_optm_ctrl_addr=`expr $base_reg + $sr_optm_ctrl_offset`
sr_optm_ctrl_reg=0x`printf %x $sr_optm_ctrl_addr`
sr_ppdu_impr_addr=`expr $base_reg + $sr_ppdu_impr_offset`
sr_ppdu_impr_reg=0x`printf %x $sr_ppdu_impr_addr`
rate_adapt_addr=`expr $base_reg + $rate_adapt_offset`
rate_adapt_reg=0x`printf %x $rate_adapt_addr`
rate_adapt_ht1_addr=`expr $base_reg + $rate_adapt_ht1_offset`
rate_adapt_ht1_reg=0x`printf %x $rate_adapt_ht1_addr`
rate_adapt_ht2_addr=`expr $base_reg + $rate_adapt_ht2_offset`
rate_adapt_ht2_reg=0x`printf %x $rate_adapt_ht2_addr`
rate_adapt_leg_addr=`expr $base_reg + $rate_adapt_leg_offset`
rate_adapt_leg_reg=0x`printf %x $rate_adapt_leg_addr`
rate_adapt_eht_addr=`expr $base_reg + $rate_adapt_eht_offset`
rate_adapt_eht_reg=0x`printf %x $rate_adapt_eht_addr`

# set sr pwr offset
set_reg $sr_pwr_reg 0x09060300

if [ $enable -eq 0 ]; then
    set_reg $hecfg_reg 0x0
    set_reg $srcfg_reg 0x0
else
    non_srg_offset_val=$((non_srg_offset<<8))
    srg_max_offset_val=$((srg_max_offset<<24))
    srg_min_offset_val=$((srg_min_offset<<16))
    sr_enable_val=`printf %d 0x1D`
    srcfg_val=$(($sr_enable_val|srg_min_offset_val|srg_max_offset_val|non_srg_offset_val))
    srcfg_val_hex=0x`printf %x $srcfg_val`
    set_reg $hecfg_reg 0x2
    set_reg $srcfg_reg $srcfg_val_hex
    set_reg $rate_adapt_reg 0x9354991A
    set_reg $rate_adapt_leg_reg 0x64691
    set_reg $rate_adapt_ht1_reg 0x1220C8D2
    set_reg $rate_adapt_ht2_reg 0x803
	set_reg $rate_adapt_eht_reg 0x1B
fi

set_reg $sr_optm_ctrl_reg 0x1CC20000
set_reg $sr_ppdu_impr_reg 0x200

echo "Done"
