#!/bin/sh


root_path=$(cd $(dirname $0);pwd)
#echo "root_path: "${root_path}
cd ${root_path}

script_name=$0


show_script_log=0
force_arg_num=0

script_version="false"
verison_info="2023/12/21 version 0.01"

radio=0
basic_path="/sys/kernel/debug/ieee80211/phy$radio/cls_wifi"
txq_flag="false"
version="false"
show_band_stats="false"
show_sys_stats="false"
show_dbg_cnt="false"
show_sta_rx_cnt="false"
show_sta_tx_cnt="false"
station_mac="00:00:00:00:00:00"
list_station_mac="00:00:00:00:00:00"
station_mac_num=0
all_station="false"

################
get_input_parameters() {
	while test $# -gt 0
	do
		case "$1" in
			version)
				version="true"
				;;
			txq)
				txq_flag="true"
				;;
			radio=*)
				radio=${1#radio=}
				;;
			stats)
				show_band_stats="true"
				;;
			sys_stats)
				show_sys_stats="true"
				;;
			dbg_cnt)
				show_dbg_cnt="true"
				;;
			sta_tx)
				show_sta_tx_cnt="true"
				;;
			sta_rx)
				show_sta_rx_cnt="true"
				;;
			station_mac=*)
				station_mac=${1#station_mac=}
				;;
			ALL_STA)
				all_station="true"
				;;
		esac
		shift
	done
	basic_path="/sys/kernel/debug/ieee80211/phy$radio/cls_wifi"
}

show_help(){
	echo "[version]show firmware information"
	echo "[txq]show wifi-drv tx queue information,VIF[0]... ..."
	echo "[stats]show band tx or rx AMSDU/AMPDU information"
	echo "[radio=0,1]0: 2G4(only 2g4 or 2g4_5g) or 5G(only 5G); 1: 5G"
	echo "[sys_stats]show WPU CPU Idle information"
	echo "[dbg_cnt]show statistic count"
	echo "[sta_tx]show station tx rate succeed information"
	echo "[sta_rx]show station rx rate succeed information"
	echo "[station_mac=00:00:00:00:00:00]"
	echo "[ALL_STA]select all station"
	echo "============================================================="
	echo "sh $script_name version           #get firmware version"
	echo "sh $script_name stats             #show band tx or rx AMSDU/AMPDU information"
	echo "sh $script_name txq               #show wifi-drv tx queue information"
	echo "sh $script_name sys_stats         #show WPU CPU Idle information"
	echo "sh $script_name sys_stats radio=1 #show WPU CPU Idle information"
	echo "sh $script_name dbg_cnt           #show wifi system statistic count"
	echo "sh $script_name sta_tx ALL_STA    #show all station tx rate succeed information"
	echo "sh $script_name sta_rx ALL_STA    #show all station rx rate succeed information"
	echo "sh $script_name sta_rx station_mac=d0:44:33:10:19:20    #show station(d0:44:33:10:19:20) rx rate succeed information"
	echo "sh $script_name sta_tx station_mac=d0:44:33:10:19:20    #show station(d0:44:33:10:19:20) tx rate succeed information"
	echo "============================================================="
}

get_station_mac_dir() {
	#echo "get_station_mac_dir"
	_list_dir=""
	first_do_it=1
	show_list_log=0
	regex_pattern='^([0-9a-fA-F]{2}:){5}[0-9a-fA-F]{2}$'
	station_path=$basic_path/stations/


	#cd /sys/kernel/debug/ieee80211/phy$radio/cls_wifi/stations
	if [ ! $station_path = "" ]; then
		if [ -d $station_path ]; then
		cd $station_path
		fi
	fi


	# 使用 find 命令获取当前路径下的所有文件，并将它们存放到数组中
	directories_array=$(find . -maxdepth 1 -type d ! -name '.*')
	directories_array=$(echo $directories_array | tr '\n' ' ')
	# 输出数组中的所有子目录
	if [ $show_list_log -gt 0 ];then
		echo "directories_array=${directories_array}"
		echo "Subdirectories in the current directory:"
	fi
	for dir in ${directories_array}; do
		echo "$dir"
		case "$dir" in
			./*)
				dir=${dir#./}
				;;
		esac
		echo "new_dir: $dir"
		if [[ $dir =~ $regex_pattern ]]; then
			if [ $first_do_it -gt 0 ]; then
				first_do_it=0
				_list_dir=$dir
			else
				_list_dir+=" $dir"
			fi
		fi
	done


	if [ $first_do_it -lt 1 ]; then
		if [ $show_list_log -gt 0 ]; then
			echo "list_dir数组元素个数为: ${_list_dir}"
		fi
		for item in ${_list_dir}; do
			if [ $show_list_log -gt 0 ];then
				echo "$item"
			fi

			if [ $station_mac_num -lt 1 ]; then
				list_station_mac="$item"
				if [ $station_mac = "00:00:00:00:00:00" ];then
					station_mac=$list_station_mac
				fi
			else
				list_station_mac+=" $item"
			fi
			station_mac_num=$((station_mac_num + 1))
		done

		#if [ $station_mac = "00:00:00:00:00:00" ];then
		#	station_mac=${list_station_mac[0]}
		#fi

		if [ $show_list_log -gt 0 ];then
			echo "station_mac: $station_mac"

			echo "list_station_mac数组元素个数为: $station_mac_num"
			for item in ${list_station_mac}; do
				echo "$item"
			done
		fi
	fi

	if [ ! "$station_path" = "" ]; then
		cd -
	fi
}

show_sta_rx_cnt_info(){
	if [ "$show_sta_rx_cnt" = "true" ]; then
		if [ "$all_station" = "true" ];then
			echo "set all station"
			if [ "$list_station_mac" = "00:00:00:00:00:00" ] || [ $station_mac_num -lt 1 ];then
				get_station_mac_dir
			fi

			if [ $station_mac_num -gt 0 ]; then
				sta_config_offset=0
				for sta_mac in ${list_station_mac}; do
					###set rate
					#echo "sta_mac=$sta_mac"
					msg_path=$basic_path/stations/$sta_mac/rc/rx_rate
					if [ -f $msg_path ]; then
						echo "cat $msg_path"
						cat $msg_path
					else
						echo "cat $msg_path"
					fi
					sta_config_offset=$((sta_config_offset + 1))
					if [ $sta_config_offset -ge $station_mac_num ]; then
						break
					fi
				done
			else
				echo "[warn]ALL_STA cat ?"
			fi
		else
			if [ $station_mac = "00:00:00:00:00:00" ];then
				get_station_mac_dir
			fi
			if [ $station_mac != "00:00:00:00:00:00" ];then
				###set rate
				sta_mac=$station_mac
				msg_path=$basic_path/stations/$sta_mac/rc/rx_rate
				if [ -f $msg_path ]; then
					echo "cat $msg_path"
					cat $msg_path
				else
					echo "[warn]cat $msg_path failed"
				fi
			else
				echo "[warn]cat ?"
			fi
		fi
	fi
}

show_sta_tx_cnt_info(){
	if [ "$show_sta_tx_cnt" = "true" ]; then
		if [ "$all_station" = "true" ];then
			echo "set all station"
			if [ "$list_station_mac" = "00:00:00:00:00:00" ] || [ $station_mac_num -lt 1 ];then
				get_station_mac_dir
			fi

			if [ $station_mac_num -gt 0 ]; then
				sta_config_offset=0
				for sta_mac in ${list_station_mac}; do
					###set rate
					echo "sta_mac=$sta_mac"
					msg_path=$basic_path/stations/$sta_mac/rc/tx_rate
					if [ -f $msg_path ]; then
						echo "cat $msg_path"
						cat $msg_path
					else
						echo "cat $msg_path"
					fi
					sta_config_offset=$((sta_config_offset + 1))
					if [ $sta_config_offset -ge $station_mac_num ]; then
						break
					fi
				done
			else
				echo "[warn]ALL_STA cat ?"
			fi
		else
			if [ $station_mac = "00:00:00:00:00:00" ];then
				get_station_mac_dir
			fi
			if [ $station_mac != "00:00:00:00:00:00" ];then
				###set rate
				sta_mac=$station_mac
				msg_path=$basic_path/stations/$sta_mac/rc/tx_rate
				if [ -f $msg_path ]; then
					echo "cat $msg_path"
					cat $msg_path
				else
					echo "[warn]cat $msg_path failed"
				fi
			else
				echo "[warn]cat ?"
			fi
		fi
	fi
}

show_dbg_cnt_info(){
	msg_path=$basic_path/dbg_cnt
	if [ $show_dbg_cnt = "true" ]; then
		if [ -f $msg_path ];then 
			cat $msg_path
		else
			echo "[warn]cat $msg_path failed,The $msg_path does not exist"
		fi
	fi
}

show_sys_stats_info(){
	msg_path=$basic_path/sys_stats
	if [ $show_sys_stats = "true" ]; then
		if [ -f $msg_path ];then 
			cat $msg_path
		else
			echo "[warn]cat $msg_path failed,The $msg_path does not exist"
		fi
	fi
}

show_bands_stats_info(){
	msg_path=$basic_path/stats
	if [ $show_band_stats = "true" ]; then
		if [ -f $msg_path ];then 
			cat $msg_path
		else
			echo "[warn]cat $msg_path failed,The $msg_path does not exist"
		fi
	fi
}

show_bands_stats_info(){
	msg_path=$basic_path/stats
	if [ $show_band_stats = "true" ]; then
		if [ -f $msg_path ];then 
			cat $msg_path
		else
			echo "[warn]cat $msg_path failed,The $msg_path does not exist"
		fi
	fi
}

show_txq_info(){
	if [ $txq_flag = "true" ]; then
		if [ -f $basic_path/txq ];then 
			cat $basic_path/txq
		else
			echo "[warn]cat $basic_path/txq failed,The $basic_path/txq does not exist"
		fi
	fi
}

show_vresion_info(){
	if [ $version = "true" ]; then
		if [ -f $basic_path/version ];then 
			cat $basic_path/version
		else
			echo "[warn]cat $basic_path/version failed,The $basic_path/version does not exist"
		fi
	fi
}

#########################################################################
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
	echo "[script_version]show script version inforation"
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
			script_version)
				script_version="true"
				;;
		esac
		get_input_parameters $1
		shift
	done
}

__get_input_parameters $@
show_log

if [ $script_version = "true" ]; then
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

show_vresion_info
show_txq_info
show_bands_stats_info
show_sys_stats_info
show_dbg_cnt_info
show_sta_rx_cnt_info
show_sta_tx_cnt_info



echo ""
echo "============> $script_name Done <============"
echo ""
exit


