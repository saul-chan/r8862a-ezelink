#!/bin/bash

band=$1
enable=$2

set_cfr_reg()
{
	reg=`devmem $1`
	reg_dec=$(($reg|0x1005000))
	reg_hex=0x`printf %x $reg_dec`
	devmem $1 32 $reg_hex
}

clr_cfr_reg()
{
	reg=`devmem $1`
	reg_dec=$(($reg&0xfeffafff ))
	reg_hex=0x`printf %x $reg_dec`
	devmem $1 32 $reg_hex
}

if [ "$band" == "2g" ]; then
	base_reg_ch0=0x40280008
	base_reg_ch1=0x40288008
elif [ "$band" == "5g" ]; then
	base_reg_ch0=0x40080008
	base_reg_ch1=0x40088008
fi

if [ "$enable" == "enable" ]; then
	set_cfr_reg $base_reg_ch0
	set_cfr_reg $base_reg_ch1
elif [ "$enable" == "disable" ]; then
	clr_cfr_reg $base_reg_ch0
	clr_cfr_reg $base_reg_ch1
fi

echo "CFR config done!"
