#!/bin/sh


root_path=$(cd $(dirname $0);pwd)
#echo "root_path: "${root_path}
cd ${root_path}

script_name=$0
show_script_log=0
force_arg_num=0
version="false"
verison_info="2023/12/14 version 0.01"
radio=0
msg_path="/sys/kernel/debug/ieee80211/phy$radio/cls_wifi/msg"
msg_string="08380E001000040007000000"
log="no"
rx_msg_len=-1

get_input_parameters() {
	while test $# -gt 0
	do
		case "$1" in
			version)
				version="true"
				;;
			radio=*)
				radio=${1#radio=}
				msg_path="/sys/kernel/debug/ieee80211/phy$radio/cls_wifi/msg"
				;;
			log=*)
				log=${1#log=}
				;;
			rx_msg_len=*)
				rx_msg_len=${1#rx_msg_len=}
				;;
		esac
		shift
	done
}

show_help(){
	echo "============================================================="
	echo "[version]"
	echo "[radio=0,1]0: 2G4(only 2g4 or 2g4_5g) or 5G(only 5G); 1: 5G"
	echo "[log=ALL,LOG1,LOG2,LOG3,WARN1,WARN2,ERR,NONE], NONE: no debug log; ALL: print all log"
	echo "[rx_msg_len=0,1,2,3,...,2147483647]recevice msg output length Byte"
	echo "============================================================="
	echo "sh $script_name version                   #get script version"
	echo "sh $script_name radio=1                   #No log output"
	echo "sh $script_name log=ALL radio=1           #output ALL,LOG1,LOG2,LOG3,WARN1,WARN2 and ERR log"
	echo "sh $script_name log=ERR radio=1           #only output ERR log"
	echo "sh $script_name log=WARN1 radio=1         #output WARN1,WARN2 and ERR log"
	echo "sh $script_name log=WARN2 radio=1         #output WARN2 and ERR log"
	echo "sh $script_name log=LOG1 radio=1          #output LOG1,LOG2,LOG3,WARN1,WARN2 and ERR log"
	echo "sh $script_name log=LOG2 radio=1          #output LOG2,LOG3,WARN1,WARN2 and ERR log"
	echo "sh $script_name log=LOG3 radio=1          #output LOG3,WARN1,WARN2 and ERR log"
	echo "sh $script_name log=NONE radio=1          #No log output"
	echo "sh $script_name rx_msg_len=0              #recevice msg output 0Byte"
	echo "sh $script_name rx_msg_len=16             #recevice msg output 16Byte"
	echo "sh $script_name rx_msg_len=32             #recevice msg output 32Byte"
	echo "sh $script_name rx_msg_len=64             #recevice msg output 64Byte"
	echo "sh $script_name rx_msg_len=256            #recevice msg output 256Byte"
	echo "sh $script_name rx_msg_len=512            #recevice msg output 512Byte"
	echo "sh $script_name rx_msg_len=1024           #recevice msg output 1024Byte"
	echo "sh $script_name rx_msg_len=2048           #recevice msg output 2048Byte"
	echo "sh $script_name rx_msg_len=4096           #recevice msg output 4096Byte"
	echo "sh $script_name rx_msg_len=8192           #recevice msg output 8192Byte"
	echo "============================================================="
}


__exit_scrpit(){
	echo ""
	echo "============> $script_name Done <============"
	echo ""
	if [ $# -gt 0 ];then
		exit $1
	else
		exit
	fi
}

__show_help() {
	echo "$0 help information"
	if [ $force_arg_num -gt 0 ]; then
	echo "The minimum number of input parameters is $force_arg_num"
	fi
	echo "$script_name input arg:"
	echo "[scriptlog=0/1],0: default,1: output log"
	echo "[-h]: output help inforation"
	echo "[help]: output help inforation"
	echo ""
	show_help
}

show_log() {
if [ $show_script_log -gt 0 ]
then
	set -x
fi
}

__get_input_parameters() {
	#echo "get_input_parameters"
	while test $# -gt 0
	do
		case "$1" in
			-h)
				__show_help
				__exit_scrpit 0
				;;
			help)
				__show_help
				__exit_scrpit 0
				;;
			scriptlog=*)
				show_script_log=${1#scriptlog=}
				;;
		esac
		get_input_parameters $1
		shift
	done
}


__get_input_parameters $@
show_log

if [ $version = "true" ]; then
	echo $verison_info
	__exit_scrpit
fi

if [ $# -lt $force_arg_num ]; then
if [ $force_arg_num -gt 0 ]; then
echo "Please input [arg1] [arg2], The minimum number of input parameters is $force_arg_num"
fi
	__show_help
	echo "============> $script_name Done <============"
	exit
fi

####============================= MAIN ===============
log_level_skip="false"
log=$(echo "$log" | tr 'a-z' 'A-Z')
if [ "$log" = "ALL" ]; then
	#set wpu debug print level = 0
	msg_string="08380E001000040000000000"
elif [ "$log" = "LOG1" ]; then
	#set wpu debug print level = 1
	msg_string="08380E001000040001000000"
elif [ "$log" = "LOG2" ]; then
	#set wpu debug print level = 2
	msg_string="08380E001000040002000000"
elif [ "$log" = "LOG3" ]; then
	#set wpu debug print level = 3
	msg_string="08380E001000040003000000"
elif [ "$log" = "WARN1" ]; then
	#set wpu debug print level = 4
	msg_string="08380E001000040004000000"
elif [ "$log" = "WARN2" ]; then
	#set wpu debug print level = 5
	msg_string="08380E001000040005000000"
elif [ "$log" = "ERR" ]; then
	#set wpu debug print level = 6
	msg_string="08380E001000040006000000"
elif [ "$log" = "NONE" ]; then
	#set wpu debug print level = 7
	msg_string="08380E001000040007000000"
else
	log_level_skip="true"
fi

if [ "$log_level_skip" = "false" ];then
if [ -f $msg_path ]; then
	echo "echo -n \"$msg_string\" > $msg_path"
	echo -n "$msg_string" > $msg_path
else
	echo "[warn]echo -n \"$msg_string\" > $msg_path"
fi
fi

if [[ $rx_msg_len =~ ^[+]?[0-9]+$ ]] || [[ $rx_msg_len =~ ^[-]?[0-9]+$ ]] || [[  $rx_msg_len =~ ^[0-9]+$ ]]; then
	if [ $rx_msg_len -gt -1 ]; then
		rx_msg_len_string=$(printf "%08x" $rx_msg_len)
		last_two_chars="${rx_msg_len_string:6:2}"
		three_two_chars="${rx_msg_len_string:4:2}"
		second_two_chars="${rx_msg_len_string:2:2}"
		first_two_chars="${rx_msg_len_string:0:2}"
		#echo "last_two_chars=${last_two_chars}"
		#echo "three_two_chars=${three_two_chars}"
		#echo "second_two_chars=${second_two_chars}"
		#echo "first_two_chars=${first_two_chars}"
		rx_msg_len_string="${last_two_chars}${three_two_chars}${second_two_chars}${first_two_chars}"
		#echo "rx_msg_len_string = ${rx_msg_len_string}"
		msg_string="07380E0010000400${rx_msg_len_string}"
		if [ -f $msg_path ]; then
			echo "echo -n \"$msg_string\" > $msg_path"
			echo -n "$msg_string" > $msg_path
		else
			echo "[warn]echo -n \"$msg_string\" > $msg_path"
		fi
	fi
fi




echo ""
echo "============> $script_name Done <============"
echo ""
exit


