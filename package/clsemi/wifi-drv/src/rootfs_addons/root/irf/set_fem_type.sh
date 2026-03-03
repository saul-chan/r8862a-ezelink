#!/bin/bash

fem=$1
ver=$2
board_type=$3

if [ $# != 3 ]; then
		echo "parameter wrong. format: sh set_fem_type.sh fem_type version board_type"
		echo "fem_type: skywork/qorvo/nofem/kct"
		echo "version: xx/LAST (xx isthe number of version, LAST for select the last version)"
		echo "board_type: EVB or RDK"
	exit
fi


board_dir="EVB"


if [ $ver == "LAST" ]; then
	ver=$(ls $board_dir | sed 's/ver//' | sort -n | tail -n 1)
	echo "LAST ver: $ver"
fi


ver_dir="EVB/ver"$ver


if [ ! -d $ver_dir ];then
	echo "$ver_dir not exist"
	exit
fi

link() {
        local target=$1
        local linkname=$2
        if [ "$(readlink $linkname)" != "$target" ]; then
                ln -sf $target $linkname
        fi
}

echo "gain table select: $fem $ver $board_type"

if [ "$fem" != "skywork" ] && [ "$fem" != "qorvo" ] && [ "$fem" != "kct" ] && [ "$fem" != "nofem" ]; then
	echo "parameter wrong. format: sh set_fem_type.sh fem_type version board_type"
	echo "fem_type: skywork/qorvo/nofem/kct"
	exit
fi

if [ "$fem" == "skywork" ]; then
	link $ver_dir/rx_gain_level_2G_SKY85333.bin rx_gain_level_2G.bin
	link $ver_dir/rx_gain_level_5G_SKY85747.bin rx_gain_level_5G.bin
	link $ver_dir/tx_gain_level_2G.bin  tx_gain_level_2G.bin
	link $ver_dir/tx_gain_level_5G.bin  tx_gain_level_5G.bin
elif [ "$fem" == "qorvo" ]; then
	link $ver_dir/rx_gain_level_2G_QPF4288A.bin  rx_gain_level_2G.bin
	link $ver_dir/rx_gain_level_5G_QPF4588A.bin  rx_gain_level_5G.bin
	link $ver_dir/tx_gain_level_2G.bin  tx_gain_level_2G.bin
	link $ver_dir/tx_gain_level_5G.bin  tx_gain_level_5G.bin
elif [ "$fem" == "kct" ]; then
	link $ver_dir/rx_gain_level_2G_KCT8270.bin  rx_gain_level_2G.bin
	link $ver_dir/rx_gain_level_5G_KCT8570.bin  rx_gain_level_5G.bin
	link $ver_dir/tx_gain_level_2G_KCT8270.bin  	tx_gain_level_2G.bin
	link $ver_dir/tx_gain_level_5G_KCT8570.bin  	tx_gain_level_5G.bin
elif [ "$fem" == "nofem" ]; then
	link $ver_dir/rx_gain_level_2G_nofem.bin rx_gain_level_2G.bin
	link $ver_dir/rx_gain_level_5G_nofem.bin rx_gain_level_5G.bin
	link $ver_dir/tx_gain_level_2G.bin  tx_gain_level_2G.bin
	link $ver_dir/tx_gain_level_5G.bin  tx_gain_level_5G.bin
else
	echo "parameter wrong. format: sh set_fem_type.sh fem_type version board_type"
	echo "fem_type: skywork/qorvo/nofem/kct"
	echo "board_type: EVB or RDK"
	exit
fi

link $ver_dir/fb_gain_level_2G.bin  fb_gain_level_2G.bin
link $ver_dir/fb_gain_level_5G.bin  fb_gain_level_5G.bin

sync

exit

