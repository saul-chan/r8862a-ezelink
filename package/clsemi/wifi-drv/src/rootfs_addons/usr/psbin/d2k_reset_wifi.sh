#!/bin/sh

#########################################################################
root_path=$(cd $(dirname $0);pwd)
#echo "root_path: "${root_path}
cd ${root_path}
script_name=$0
show_script_log=0
force_arg_num=0
version="false"
verison_info="2023/12/8 version 0.01"

#########################################################################
reset_hw_type="RST_D"
WMAC_STAT="def"
reset_hw_comm_flag="NONE"
op_send_msg=0
op_dbg_cb=0
msg_body_string=''
msg_string=''
msg_id=-1
msg_task_id=0x30
dst_task_id=0x000E
src_task_id=0x0011
param_len=0x0004


radio=0
msg_path="/sys/kernel/debug/ieee80211/phy$radio/cls_wifi/msg"

#########################################################################
get_input_parameters() {
	#echo "get_input_parameters($#): $@"
	while test $# -gt 0
	do
		case "$1" in
			version)
				version="true"
				;;
			dbg=*)
				op_dbg_cb=${1#dbg=}
				;;
			op_msg=*)
				op_send_msg=${1#op_msg=}
				;;
			wmac_stat=*)
				WMAC_STAT=${1#wmac_stat=}
				;;
			RST=*)
				reset_hw_type=${1#RST=}
				;;
			COMM=*)
				reset_hw_comm_flag=${1#COMM=}
				;;
			radio=*)
				radio=${1#radio=}
				msg_path="/sys/kernel/debug/ieee80211/phy$radio/cls_wifi/msg"
				;;
		esac
		shift
	done
}

show_help(){
	echo "+++++--------------------------------------------------------------+++++"
	echo "[version]"
	echo "[RST=RST_A,RST_B,RST_C,RST_D,RST_E,RST_F]"
	echo "[dbg=0,1]0: none, 1: reset hw cb"
	echo "[op_msg=0,1]0: none, 1: send msg to ke_msg do it"
	echo "[wmac_stat=def,idle,doze,active],def: standard procedure"
	echo "[COMM=0,1,2,...,F]bit0: SMP, bit1: DPD_FB_LMS, bit2: INTFDET, bit3: bistfft"
	echo "[radio=0,1]0: 2G4(only 2g4 or 2g4_5g) or 5G(only 5G); 1: 5G"
	echo "============================================================="
	echo "sh $script_name version           #get script version"
	echo "sh $script_name RST=RST_D"
	echo "sh $script_name RST=RST_D op_msg=1"
	echo "sh $script_name RST=RST_D dbg=1"
	echo "sh $script_name RST=RST_D dbg=1 wmac_stat=active"
	echo "============================================================="
}

calc_msg_string() {
	reset_type="00"
	sub_type=''
	op_type=0
	msg_id_string=''
	show_log=0
	if [ "$reset_hw_type" = "RST_A" ]; then
		reset_type="01"
	elif [ "$reset_hw_type" = "RST_B" ]; then
		reset_type="02"	
	elif [ "$reset_hw_type" = "RST_C" ]; then
		reset_type="03"
	elif [ "$reset_hw_type" = "RST_D" ]; then
		reset_type="04"
	elif [ "$reset_hw_type" = "RST_E" ]; then
		reset_type="05"
	elif [ "$reset_hw_type" = "RST_F" ]; then
		reset_type="06"
	else
		echo "RST= msg error"
		show_help
		__exit_scrpit
	fi

	if [ "$WMAC_STAT" = "def" ]; then
		sub_type="0"
	elif [ "$WMAC_STAT" = "idle" ]; then
		sub_type="1"	
	elif [ "$WMAC_STAT" = "doze" ]; then
		sub_type="2"
	elif [ "$WMAC_STAT" = "active" ]; then
		sub_type="3"
	else
		echo "wmac_stat= msg error"
		show_help
		__exit_scrpit
	fi

	length=${#reset_hw_comm_flag}
	if [[ $reset_hw_comm_flag =~ ^[0-9a-fA-F]+$ ]] && [ $length -eq 1 ]; then
		sub_type="$reset_hw_comm_flag$sub_type"
		if [ $show_log -gt 0 ]; then
			echo "reset_hw_comm_flag: $reset_hw_comm_flag"
		fi
	elif [ "$reset_hw_comm_flag" = "NONE" ]; then
		sub_type="0$sub_type"
	else
		echo "COMM= msg error,input value: $reset_hw_comm_flag"
		show_help
		__exit_scrpit
	fi

	if [ $op_send_msg -eq 0 ]; then
		op_type=$((op_type + 0))
	elif [ $op_send_msg -eq 1 ]; then
		op_type=$((op_type + 1))
	else
		echo "op_msg= msg error"
		show_help
		__exit_scrpit
	fi

	if [ $op_dbg_cb -eq 0 ]; then
		op_type=$((op_type + 0))
	elif [ $op_dbg_cb -eq 1 ]; then
		op_type=$((op_type + 2))
	else
		echo "dbg= msg error"
		show_help
		__exit_scrpit
	fi

	op_type_string=$(printf "%04x" $op_type)
	last_two_chars="${op_type_string: -2}"
	first_two_chars="${op_type_string:0:2}"
	op_type_string="${last_two_chars}${first_two_chars}"
	msg_body_string="$reset_type$sub_type$op_type_string"
	if [ $show_log -gt 0 ]; then
		echo "reset_type:       $reset_type"
		echo "sub_type:         $sub_type"
		echo "op_type:          $op_type_string($op_type)"
		echo "msg_body_string:  $msg_body_string"
	fi

	dst_task_id_string=$(printf "%04x" $dst_task_id)
	last_two_chars="${dst_task_id_string: -2}"
	first_two_chars="${dst_task_id_string:0:2}"
	dst_task_id_string="${last_two_chars}${first_two_chars}"

	src_task_id_string=$(printf "%04x" $src_task_id)
	last_two_chars="${src_task_id_string: -2}"
	first_two_chars="${src_task_id_string:0:2}"
	src_task_id_string="${last_two_chars}${first_two_chars}"

	param_len_string=$(printf "%04x" $param_len)
	last_two_chars="${param_len_string: -2}"
	first_two_chars="${param_len_string:0:2}"
	param_len_string="${last_two_chars}${first_two_chars}"

	if [ $msg_id -gt -1 ]; then
		g_msg_task_id=$msg_id
		msg_id_string=$(printf "%04x" $msg_id)
		last_two_chars="${msg_id_string: -2}"
		first_two_chars="${msg_id_string:0:2}"
		msg_id_string="${last_two_chars}${first_two_chars}"
	else
		g_msg_task_id=$((dst_task_id << 10))
		g_msg_task_id=$((g_msg_task_id + msg_task_id))
		msg_id_string=$(printf "%04x" $g_msg_task_id)
		last_two_chars="${msg_id_string: -2}"
		first_two_chars="${msg_id_string:0:2}"
		msg_id_string="${last_two_chars}${first_two_chars}"
	fi

	msg_head_string="$msg_id_string$dst_task_id_string$src_task_id_string$param_len_string"
	msg_string="$msg_head_string$msg_body_string"
	if [ $show_log -gt 0 ]; then
		g_msg_task_id_hex_string=$(printf "0x%04X" "$g_msg_task_id")
		echo "msg_id:         $msg_id_string($g_msg_task_id_hex_string)"
		echo "dst_task_id:    $dst_task_id_string($dst_task_id)"
		echo "src_task_id:    $src_task_id_string($src_task_id)"
		echo "param_len:      $param_len_string"
		echo "msg_head:       $msg_head_string"
		echo "msg:            $msg_string"
	fi
}


#########################################################################
__exit_scrpit(){
	echo ""
	echo "============> $script_name Done <============"
	echo ""
	exit
}

__show_help() {
	echo "$0 help information"
	if [ $force_arg_num -gt 0 ]; then
	echo "The minimum number of input parameters is $force_arg_num"
	fi
	echo "$script_name input arg:"
	echo "[log=0/1],0: default,1: output log"
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
				exit 0
				;;
			help)
				__show_help
				exit 0
				;;
			log=*)
				show_script_log=${1#log=}
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
calc_msg_string

if [ -f $msg_path ]; then
	echo "echo -n \"$msg_string\" > $msg_path"
	echo -n "$msg_string" > $msg_path
else
	echo "echo -n \"$msg_string\" > $msg_path"
fi




####============================= DONE ===============
__exit_scrpit

####============================= FILE END ===============
