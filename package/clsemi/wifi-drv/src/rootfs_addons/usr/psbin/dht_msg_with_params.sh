#!/bin/bash

usage() {
	echo "params:"
	echo "        band: 0 or 1"
	echo "        id: refs dubhe1000_test.h"
	echo "        msg: bytes of struct, refs dubhe1000_test.h"
	echo "             add \":u\" to specify the value size"
	echo "             0x1:u32 means 0x1 is a 32bit value"
	echo "Example:"
	echo "        sh dht_msg_with_params.sh band=1 id=8 msg=7:u32"
	echo "        sh dht_msg_with_params.sh band=1 id=0xf msg=\"0x1:u32 0x4c820000 0:u32\""
}

parse_msg() {
	local len=0
	local sub_len
	local sub_msg

	for val in $@
	do
		if [ "$(echo $val | grep ":u")" != "" ]; then
			sub_len=${val##*:u}
			sub_len=$(($sub_len/8))
			sub_msg=${val%%:u*}
		else
			sub_len=${#val}
			if [ "$(echo $val | grep "0x")" != "" ]; then
				sub_len=$(($sub_len-2))
			fi
			sub_len=$((($sub_len+1)/2))
			sub_msg=$val
		fi
		sub_msg=$(($sub_msg&0xFFFFFFFF))
		print_len=$(($sub_len*2))
		sub_msg=`printf %0${print_len}x ${sub_msg}`
		sub_msg=`echo -n ${sub_msg} | awk '{for(i=length();i>0;i-=2) printf("%s",substr($0,i-1,2));print "";}'`
		len=$(($len+$sub_len))
		msg_str=${msg_str}${sub_msg}
	done

	return $len
}

while test $# -gt 0
do
        case "$1" in
	band=*)
		band=${1#band=}
		;;
        id=*)
		id=`printf %02x ${1#id=}`
		id_str="${id}38"
                ;;
        msg=*)
                msg=${1#msg=}
		parse_msg $msg
		len=$?
		len=`printf %02x ${len}`
		len_str="${len}00"
                ;;
        -h)
                usage
                exit 0
                ;;
        esac
        shift
done

dstid_str="0E00"
srcid_str="1000"
cmd_str=${id_str}${dstid_str}${srcid_str}${len_str}${msg_str}
cmd_path="/sys/kernel/debug/ieee80211/phy$band/cls_wifi/msg"

if [ -f $msg_path ]; then
	echo "=====>$cmd_path, write msg: $cmd_str"
	echo -n $cmd_str > $cmd_path
fi
