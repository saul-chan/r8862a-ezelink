#!/bin/bash



#root_path=$(cd $(dirname $0);pwd)
script_name=$0

echo ""
#echo "root_path: "${root_path}
#cd ${root_path}

usage(){
	echo "$script_name wlan type"
	echo "wlan    : 0 or 1"
	echo "type    : 0/1/2/3/4/..."
	echo " type=0 : enable_dubhe1000_test"
	echo " type=1 : disable_dubhe1000_test"
	echo " type=2 : disable set channel fix bw"
	echo " type=3 : set channel fix bw 20MHz"
	echo " type=4 : set channel fix bw 40MHz"
	echo " type=5 : set channel fix bw 80MHz"
	echo " type=6 : set channel fix bw 160MHz"
	echo " type=7 : set channel fix bw 80+80MHz"
	echo " type=8 : enable calib en"
	echo " type=9 : disable calib en"
	echo " type=10 : dump_reg_riu_0x4ac0a000"
	echo " type=11 : dump_reg_riu_0x4ac0c000"
	echo " type=12 : dump_reg_riu_0x4ac0b000"
	echo " type=13 : set channel fix bw 80MHz freq=5180 freq1=5210 sx=0"
	echo " type=14 : set channel fix bw 80MHz freq=5200 freq1=5210 sx=0"
	echo " type=15 : set channel fix bw 80MHz freq=5220 freq1=5210 sx=0"
	echo " type=16 : set channel fix bw 80MHz freq=5240 freq1=5210 sx=0"
	echo " type=19 : set channel fix bw 160MHz freq=5180 freq1=5250 sx=0"

	echo " type=20 : set agc snr 0 for nss=2"
	echo " type=21 : set agc snr 1 for nss=2 he-su"

	echo " type=30 : set wpu rx msg print len = 0"
	echo " type=31 : set wpu rx msg print len = 16"
	echo " type=32 : set wpu rx msg print len = 32"
	echo " type=33 : set wpu rx msg print len = 64"
	echo " type=34 : set wpu rx msg print len = 128"
	echo " type=35 : set wpu rx msg print len = 256"
	echo " type=36 : set wpu rx msg print len = 512"
	echo " type=37 : set wpu rx msg print len = 1024"
	echo " type=38 : set wpu rx msg print len = 2048"
	echo " type=39 : set wpu rx msg print len = 0xFFFFFFFFU"

	echo " type=50 : set wpu debug print level = 0"
	echo " type=51 : set wpu debug print level = 1"
	echo " type=52 : set wpu debug print level = 2"
	echo " type=53 : set wpu debug print level = 3"
	echo " type=54 : set wpu debug print level = 4"
	echo " type=55 : set wpu debug print level = 5"
	echo " type=56 : set wpu debug print level = 6"
	echo " type=57 : set wpu debug print level = 7"
	
	echo " type=60 : set wpu AMPDU = 0"
	echo " type=61 : set wpu AMPDU = 1"
	echo " type=62 : set wpu AMPDU = 2"
	echo " type=63 : set wpu AMPDU = 3"
	echo " type=64 : set wpu AMPDU = 4"
	echo " type=65 : set wpu AMPDU = 5"
	echo " type=66 : set wpu AMPDU = 6"
	echo " type=67 : set wpu AMPDU = 7"
	
	
	echo " type=100 : set fix rate VHT80 MCS9 NSS=2 LGI sta_idx=0"
}

show_help=1

wlan_type=0
msg_type=0
msg_string="00380E001000040001000000"
if [ $# -eq 2 ]; then
	wlan_type=$1
	msg_type=$2
	show_help=0
	if [ $msg_type -eq 0 ];then
	msg_string="00380E001000040001000000"
	elif [ $msg_type -eq 1 ];then
	###msg_string="13380E00 10000400 01000400"
	msg_string="00380E001000040000000000"
	elif [ $msg_type -eq 2 ];then
	msg_string="05380E00100008000000000000000000"
	elif [ $msg_type -eq 3 ];then
	msg_string="05380E00100008000100000000000000"
	elif [ $msg_type -eq 4 ];then
	msg_string="05380E00100008000100000001000000"
	elif [ $msg_type -eq 5 ];then
	msg_string="05380E00100008000100000002000000"
	elif [ $msg_type -eq 6 ];then
	msg_string="05380E00100008000100000003000000"
	elif [ $msg_type -eq 7 ];then
	msg_string="05380E00100008000100000004000000"
	elif [ $msg_type -eq 8 ];then
	msg_string="00380E001000040001000000"
	elif [ $msg_type -eq 9 ];then
	msg_string="00380E001000040000000000"
	elif [ $msg_type -eq 10 ];then
	###dump_reg_riu_0x4ac0a000   0000c04a c0040000
	msg_string="03380E001000080000a0c04a00040000"
	elif [ $msg_type -eq 11 ];then
	msg_string="03380E001000080000c0c04a40000000"
	elif [ $msg_type -eq 12 ];then
	msg_string="03380E001000080000b0c04a00040000"
	elif [ $msg_type -eq 13 ];then
	###msg_string="05380E0010001000 0F000000 0200 0000 3C14 5A14" #set channel fix bw 80MHz freq=5180 freq1=5210 sx=0
	msg_string="05380E00100010000F000000020000003C145A14"
	elif [ $msg_type -eq 14 ];then
	###msg_string="05380E0010001000 0F000000 0200 0000 5014 5A14" #set channel fix bw 80MHz freq=5200 freq1=5210 sx=0
	msg_string="05380E00100010000F0000000200000050145A14"
	elif [ $msg_type -eq 15 ];then
	###msg_string="05380E0010001000 0F000000 0200 0000 6414 5A14" #set channel fix bw 80MHz freq=5220 freq1=5210 sx=0
	msg_string="05380E00100010000F0000000200000064145A14"
	elif [ $msg_type -eq 16 ];then
	###msg_string="05380E0010001000 0F000000 0200 0000 7814 5A14" #set channel fix bw 80MHz freq=5240 freq1=5210 sx=0
	msg_string="05380E00100010000F0000000200000078145A14"
	elif [ $msg_type -eq 17 ];then
	###msg_string="05380E0010001000 0F000000 0300 0000 3C14 5A14" #set channel fix bw 160MHz freq=5180 freq1=5210 sx=0
	msg_string="05380E00100010000F000000020000003C145A14"
	elif [ $msg_type -eq 18 ];then
	###msg_string="05380E0010001000 0F000000 0300 0000 7C15 C215" #set channel fix bw 160MHz freq=5500 freq1=5570 sx=0
	msg_string="05380E00100010000F000000030000007C15C215"
	elif [ $msg_type -eq 19 ];then
	#set channel fix bw 160MHz freq=5180 freq1=5250 sx=0
	msg_string="05380E00100010000F000000030000003C148214"

	elif [ $msg_type -eq 20 ];then
	###msg_string="06380E0010000C00 01000000 00000000 00000000" #set agc snr 0 for nss=2
	msg_string="06380E0010000C00010000000000000000000000"
	elif [ $msg_type -eq 21 ];then
	###msg_string="06380E0010000C00 02000000 00000000 00000000" #set agc snr 1 for nss=2 he-su
	msg_string="06380E0010000C00020000000000000000000000"

	elif [ $msg_type -eq 30 ]; then
	###msg_string="07380E0010000400 00000000" #set wpu rx msg print len = 0
	msg_string="07380E001000040000000000"
	elif [ $msg_type -eq 31 ]; then
	###msg_string="07380E0010000400 10000000" #set wpu rx msg print len = 16
	msg_string="07380E001000040010000000"
	elif [ $msg_type -eq 32 ]; then
	###msg_string="07380E0010000400 20000000" #set wpu rx msg print len = 32
	msg_string="07380E001000040020000000"
	elif [ $msg_type -eq 33 ]; then
	###msg_string="07380E0010000400 40000000" #set wpu rx msg print len = 64
	msg_string="07380E001000040040000000"
	elif [ $msg_type -eq 34 ]; then
	###msg_string="07380E0010000400 80000000" #set wpu rx msg print len = 128
	msg_string="07380E001000040080000000"
	elif [ $msg_type -eq 35 ]; then
	###msg_string="07380E0010000400 00010000" #set wpu rx msg print len = 256
	msg_string="07380E001000040000010000"
	elif [ $msg_type -eq 36 ]; then
	###msg_string="07380E0010000400 00020000" #set wpu rx msg print len = 512
	msg_string="07380E001000040000020000"
	elif [ $msg_type -eq 37 ]; then
	###msg_string="07380E0010000400 00040000" #set wpu rx msg print len = 1024
	msg_string="07380E00100004000004000000040000"
	elif [ $msg_type -eq 38 ]; then
	###msg_string="07380E0010000400 00080000" #set wpu rx msg print len = 2048
	msg_string="07380E001000040000080000"
	elif [ $msg_type -eq 39 ]; then
	###msg_string="07380E0010000400 FFFFFFFF" #set wpu rx msg print len = FFFFFFFF
	msg_string="07380E0010000400FFFFFFFF"

	elif [ $msg_type -eq 50 ]; then
	#set wpu debug print level = 0
	msg_string="08380E001000040000000000"
	elif [ $msg_type -eq 51 ]; then
	#set wpu debug print level = 1
	msg_string="08380E001000040001000000"
	elif [ $msg_type -eq 52 ]; then
	#set wpu debug print level = 2
	msg_string="08380E001000040002000000"
	elif [ $msg_type -eq 53 ]; then
	#set wpu debug print level = 3
	msg_string="08380E001000040003000000"
	elif [ $msg_type -eq 54 ]; then
	#set wpu debug print level = 4
	msg_string="08380E001000040004000000"
	elif [ $msg_type -eq 55 ]; then
	#set wpu debug print level = 5
	msg_string="08380E001000040005000000"
	elif [ $msg_type -eq 56 ]; then
	#set wpu debug print level = 6
	msg_string="08380E001000040006000000"
	elif [ $msg_type -eq 57 ]; then
	#set wpu debug print level = 7
	msg_string="08380E001000040007000000"

	elif [ $msg_type -eq 60 ]; then
	#set wpu AMPDU = 0
	msg_string="0A380E0010000400FF030000"
	elif [ $msg_type -eq 61 ]; then
	#set wpu AMPDU = 1
	msg_string="0A380E0010000400FF030100"
	elif [ $msg_type -eq 62 ]; then
	#set wpu AMPDU = 2
	msg_string="0A380E0010000400FF030200"
	elif [ $msg_type -eq 63 ]; then
	#set wpu AMPDU = 3
	msg_string="0A380E0010000400FF030300"
	elif [ $msg_type -eq 64 ]; then
	#set wpu AMPDU = 4
	msg_string="0A380E0010000400FF030400"
	elif [ $msg_type -eq 65 ]; then
	#set wpu AMPDU = 5
	msg_string="0A380E0010000400FF030500"
	elif [ $msg_type -eq 66 ]; then
	#set wpu AMPDU = 6
	msg_string="0A380E0010000400FF030600"
	elif [ $msg_type -eq 67 ]; then
	#set wpu AMPDU = 7
	msg_string="0A380E0010000400FF030700"
	elif [ $msg_type -eq 68 ]; then
	#set wpu AMPDU = 8
	msg_string="0A380E0010000400FF030800"
	elif [ $msg_type -eq 69 ]; then
	#set wpu AMPDU = 9
	msg_string="0A380E0010000400FF030900"
	elif [ $msg_type -eq 70 ]; then
	#set wpu AMPDU = 10
	msg_string="0A380E0010000400FF030A00"
	elif [ $msg_type -eq 79 ]; then
	#set wpu AMPDU = 7
	msg_string="0A380E0010000400FF03FF00"
	
	elif [ $msg_type -eq 209 ];then
	###set fix rate VHT80 MCS9 NSS=2 LGI sta_idx=0  ==>0x00008194
	msg_string="0E380E00100008000000010094810000"
	elif [ $msg_type -eq 1209 ];then
	###set fix rate VHT80 MCS9 NSS=2 LGI sta_idx=1  ==>0x00008194
	msg_string="0E380E00100008000100010094810000"
	elif [ $msg_type -eq 208 ];then
	###set fix rate VHT80 MCS8 NSS=2 LGI sta_idx=0   ==>0x00008184
	msg_string="0E380E00100008000000010084810000"
	elif [ $msg_type -eq 1208 ];then
	###set fix rate VHT80 MCS8 NSS=2 LGI sta_idx=1  ==>0x00008184
	msg_string="0E380E00100008000100010084810000"
	fi

fi




msg_path="/sys/kernel/debug/ieee80211/phy$wlan_type/cls_wifi/msg"

if [ ! -f $msg_path ];then
sh mount_debugfs.sh
if [ ! -f $msg_path ];then
msg_path=./dht_msg.txt
if [ ! -f $msg_path ];then
touch $msg_path
fi
fi
fi

if [ $show_help -eq 1 ]; then
	usage;
else
	if [ -f $msg_path ];then
		echo "=====>$msg_path, write msg: 0x$msg_string"
		echo -n "$msg_string" > $msg_path
	else
		echo "$msg_path is not exist!!!!"
	fi
fi




echo ""
echo "=========== $script_name done ==========="
echo ""
