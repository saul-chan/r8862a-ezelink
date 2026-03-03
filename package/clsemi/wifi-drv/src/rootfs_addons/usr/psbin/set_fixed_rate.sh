#!/bin/sh


root_path=$(cd $(dirname $0);pwd)
#echo "root_path: "${root_path}
cd ${root_path}

script_name=$0

show_script_log=0
force_arg_num=0

##################################
prot=HE_SU
rate=MCS0
BW=20
NSS=1
GI=0
RU=0

rate_index=-1
radio=0

use_rate_control="false"
set_rate="true"
all_station="false"
station_mac="00:00:00:00:00:00"
list_station_mac="00:00:00:00:00:00"
station_mac_num=0

rate_path=""
rate_path="/sys/kernel/debug/ieee80211/phy$radio/cls_wifi/stations"

get_input_parameters() {
	#echo "get_input_parameters($#): $@"
	while test $# -gt 0
	do
		case "$1" in
			rate=*)
				rate=${1#rate=}
				;;
			BW=*)
				BW=${1#BW=}
				;;
			prot=*)
				prot=${1#prot=}
				;;
			NSS=*)
				NSS=${1#NSS=}
				;;
			GI=*)
				GI=${1#GI=}
				;;
			RU=*)
				RU=${1#RU=}
				;;
			radio=*)
				radio=${1#radio=}
				;;
			station_mac=*)
				station_mac=${1#station_mac=}
				;;
			AUTO)
				use_rate_control="true"
				;;
			ALL_STA)
				all_station="true"
				;;
			set_rate)
				set_rate="true"
				;;
			calc)
				set_rate="false"
				;;
		esac
		shift
	done
	rate_path="/sys/kernel/debug/ieee80211/phy$radio/cls_wifi/stations"
}

show_help(){
	echo "[prot=NON-HT_CCK,NON-HT_OFDM,HT_MF,HT_GF,VHT,HE_SU,HE_MU,HE_ER,HE_TB] def: HE_SU"
	echo "[rate=1M,2M,5.5M,11M,6M,9M,...,MCS0,MCS1,...,MCS11]"
	echo "[BW=20,40,80,160],    other value: invalid"
	echo "[NSS=1,2],            0:invalid"
	echo "[GI=0,1,2],           -1:invalid"
	echo "[RU=RU26,RU106,RU242,RU52,RU484,RU996,RU2X996,...]"
	echo "[AUTO] use rate control"
	echo "[radio=0,1] 0 for 2.4G, 1 for 5G"
	echo "[station_mac=00:00:00:00:00:00]"
	echo "[ALL_STA]"
	echo "[calc]"
	echo "============================================================="
	echo "sh $script_name AUTO   #rate control"
	echo "sh $script_name prot=HE_SU rate=MCS11 GI=0 NSS=2 BW=160 radio=1"
	echo "sh $script_name rate=MCS11 GI=0 NSS=2 BW=160 radio=1 ALL_STA"
	echo "sh $script_name prot=HE_SU rate=MCS11 GI=0 NSS=2 BW=20 radio=0"
	echo "sh $script_name prot=HE_SU rate=MCS11 GI=0 NSS=2 BW=40 radio=0 ALL_STA"
	echo "sh $script_name prot=HE_SU rate=MCS11 GI=0 NSS=2 BW=40 radio=0 calc"
	echo "sh $script_name prot=NON-HT_CCK rate=1M radio=0 ALL_STA"
	echo "sh $script_name prot=NON-HT_OFDM rate=6M radio=0 ALL_STA"
	echo "sh $script_name prot=NON-HT_OFDM rate=9M radio=0"
	echo "sh $script_name prot=NON-HT_OFDM rate=18M radio=0"
	echo "sh $script_name prot=NON-HT_OFDM rate=54M radio=0 station_mac=d0:44:33:11:20:00"
	echo "sh $script_name prot=HT_MF rate=MCS0 GI=0 NSS=2 BW=40 radio=0 station_mac=d0:44:33:11:20:00"
	echo "sh $script_name prot=VHT rate=MCS0 GI=0 NSS=2 BW=160 radio=0 station_mac=d0:44:33:11:20:00"
	echo "sh $script_name prot=VHT rate=MCS9 GI=0 NSS=2 BW=160 radio=0"
	echo "sh $script_name prot=VHT rate=MCS9 GI=0 NSS=1 BW=160 radio=0"
	echo "sh $script_name prot=HE_ER rate=MCS0 GI=1 RU=RU242 radio=0"
	echo "sh $script_name prot=HE_ER rate=MCS1 GI=2 RU=RU242 radio=0"
	echo "sh $script_name prot=HE_ER rate=MCS1 GI=0 RU=RU242 radio=0"
	echo "sh $script_name prot=HE_ER rate=MCS2 GI=2 RU=RU242 radio=0"
	echo "sh $script_name prot=HE_ER rate=MCS0 GI=1 RU=RU106 radio=0"
	echo "sh $script_name prot=HE_ER rate=MCS0 GI=2 RU=RU106 radio=0"
	echo "sh $script_name prot=HE_ER rate=MCS0 GI=0 RU=RU106 radio=0"
	echo "sh $script_name prot=HE_ER rate=MCS0 GI=0 RU=RU242 radio=0"
	echo "============================================================="
}

get_station_mac_dir() {
	#echo "get_station_mac_dir"
	_list_dir=""
	first_do_it=1
	show_list_log=0
	regex_pattern='^([0-9a-fA-F]{2}:){5}[0-9a-fA-F]{2}$'


	#cd /sys/kernel/debug/ieee80211/phy$radio/cls_wifi/stations
	if [ ! $rate_path = "" ]; then
		if [ -d $rate_path ]; then
		cd $rate_path
		fi
	fi


	# 使用 find 命令获取当前路径下的所有文件，并将它们存放到数组中
	directories_array=$(find . -maxdepth 1 -type d ! -name '.*')
	directories_array=$(echo $directories_array | tr '\n' ' ')
	# 输出数组中的所有子目录
	#echo "directories_array=${directories_array}"
	#echo "Subdirectories in the current directory:"
	for dir in ${directories_array}; do
		#echo "$dir"
		case "$dir" in
			./*)
				dir=${dir#./}
				;;
		esac
		#echo "new_dir: $dir"
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

	if [ ! "$rate_path" = "" ]; then
		cd -
	fi
}

he_su_check_rate_arg(){
	if  [ $BW != 20 ] && [ $BW != 40 ] && [ $BW != 80 ] && [ $BW != 160 ]; then
	echo "BW $BW error!"
	__exit_scrpit
	fi
	if  [ $NSS != 1 ] && [ $NSS != 2 ] ; then
	echo "NSS $NSS error!"
	__exit_scrpit
	fi
	if  [ $GI != 1 ] && [ $GI != 2 ] && [ $GI != 0 ]; then
	echo "GI $GI error!"
	__exit_scrpit
	fi

	if  [ "$prot" != "NON-HT_CCK" ] && [ "$prot" != "NON-HT_OFDM" ]  && [ "$prot" != "HT_MF" ] && [ "$prot" != "VHT" ] && [ "$prot" != "HE_SU" ]&& [ "$prot" != "HE_MU" ] && [ "$prot" != "HE_ER" ] && [ "$prot" != "HE_TB" ]; then
	echo "prot $prot error!"
	__exit_scrpit
	fi
}

non_ht_cck_check_rate_arg(){
	if  [ $GI != 1 ] && [ $GI != 0 ]; then
	echo "Non-ht GI $GI error,GI value select 0/1!"
	__exit_scrpit
	fi
	if  [ "$rate" != "1M" ] && [ "$rate" != "2M" ] && [ "$rate" != "5.5M" ] && [ "$rate" != "11M" ]; then
	echo "Non-ht rate $rate error,rate value select 1M/2M/5.5M/11M!"
	__exit_scrpit
	fi
}

non_ht_ofdm_check_rate_arg(){
	if  [ "$rate" != "6M" ] && [ "$rate" != "9M" ] && [ "$rate" != "12M" ] && [ "$rate" != "18M" ] && [ "$rate" != "24M" ] && [ "$rate" != "36M" ] && [ "$rate" != "48M" ] && [ "$rate" != "54M" ]; then
	echo "Non-ht rate $rate error,rate value select 6M/9M/12/18M/...!"
	__exit_scrpit
	fi
}

ht_check_rate_arg(){
	if  [ $BW != 20 ] && [ $BW != 40 ]; then
	echo "BW $BW error!only suport 20M/40M"
	__exit_scrpit
	fi
	if  [ $NSS != 1 ] && [ $NSS != 2 ] ; then
	echo "NSS $NSS error!only suport 1SS/2SS"
	__exit_scrpit
	fi
	if  [ $GI != 1 ] && [ $GI != 0 ]; then
	echo "GI $GI error!only suport 0/1"
	__exit_scrpit
	fi
	mcs_num=${rate#MCS}
	if [ $mcs_num -lt 0 ] || [ $mcs_num -gt 7 ];then
		echo "$prot MCS set $rate error!only support MCS0~MCS7"
		__exit_scrpit
	fi
	if  [ "$prot" != "NON-HT_CCK" ] && [ "$prot" != "NON-HT_OFDM" ]  && [ "$prot" != "HT_MF" ] && [ "$prot" != "VHT" ] && [ "$prot" != "HE_SU" ]&& [ "$prot" != "HE_MU" ] && [ "$prot" != "HE_ER" ] && [ "$prot" != "HE_TB" ]; then
	echo "prot $prot error!"
	__exit_scrpit
	fi
}

vht_check_rate_arg(){
	if  [ $BW != 20 ] && [ $BW != 40 ] && [ $BW != 80 ] && [ $BW != 160 ]; then
		echo "BW $BW error!only suport 20M/40M/80M/160M"
		__exit_scrpit
	fi
	if  [ $NSS != 1 ] && [ $NSS != 2 ] ; then
		echo "NSS $NSS error!only suport 1SS/2SS"
		__exit_scrpit
	fi
	if  [ $GI != 1 ] && [ $GI != 0 ]; then
		echo "GI $GI error!only suport 0/1"
		__exit_scrpit
	fi
	mcs_num=${rate#MCS}
	if [ $mcs_num -lt 0 ] || [ $mcs_num -gt 9 ];then
		echo "$prot MCS set $rate error!only support MCS0~MCS9"
		__exit_scrpit
	fi
	if  [ "$prot" != "NON-HT_CCK" ] && [ "$prot" != "NON-HT_OFDM" ]  && [ "$prot" != "HT_MF" ] && [ "$prot" != "VHT" ] && [ "$prot" != "HE_SU" ]&& [ "$prot" != "HE_MU" ] && [ "$prot" != "HE_ER" ] && [ "$prot" != "HE_TB" ]; then
		echo "prot $prot error!"
		__exit_scrpit
	fi
}

he_er_check_rate_arg(){
	if  [ $GI != 1 ] && [ $GI != 0 ] && [ $GI != 2 ]; then
		echo "GI $GI error!only suport 0/1/2"
		__exit_scrpit
	fi
	mcs_num=${rate#MCS}
	if [ $mcs_num -lt 0 ] || [ $mcs_num -gt 2 ];then
		echo "$prot MCS set $rate error!only support MCS0~MCS9"
		__exit_scrpit
	fi
	if  [ "$RU" != "RU242" ] && [ "$RU" != "RU106" ]; then
		echo "HE ERRU $RU error!"
		__exit_scrpit
	fi
	if  [ "$prot" != "NON-HT_CCK" ] && [ "$prot" != "NON-HT_OFDM" ]  && [ "$prot" != "HT_MF" ] && [ "$prot" != "VHT" ] && [ "$prot" != "HE_SU" ]&& [ "$prot" != "HE_MU" ] && [ "$prot" != "HE_ER" ] && [ "$prot" != "HE_TB" ]; then
		echo "prot $prot error!"
		__exit_scrpit
	fi
}


##################
bw_num=1

calc_rate_index() {
	rate_index=-1
	if [ "$prot" = "HE_SU" ]; then
		he_su_check_rate_arg
		if [ $BW = 20 ] || [ $BW = 40 ];then
		bw_num=$((BW / 20))
		bw_num=$((bw_num - 1))
		elif [ $BW = 80 ];then
		bw_num=2
		else
		bw_num=3
		fi
		nss_num=$((NSS - 1))
		gi_num=$GI
		mcs_num=${rate#MCS}
		#echo "$prot set rate($rate) ==> mcs_num: $mcs_num"
		#echo "$prot set BW($BW) ==> bw_num: $bw_num"
		#echo "$prot set NSS($NSS) ==> nss_num: $nss_num"
		#echo "$prot set GI($GI) ==> gi_num: $gi_num"
		if [ $mcs_num -lt 0 ] || [ $mcs_num -gt 11 ];then
		echo "$prot MCS set $rate error!"
		__exit_scrpit
		fi
		
		rate_index=$((784 + 12 * 3 * 4 * nss_num + 4 * 3 * mcs_num + gi_num + 3 * bw_num))
	fi
	if [ "$prot" = "NON-HT_CCK" ]; then
		non_ht_cck_check_rate_arg
		gi_num=$GI
		echo "GI: $GI,rate: $rate"
		if  [ "$rate" = "1M" ] ; then
			mcs_num=0
		elif  [ "$rate" = "2M" ] ; then
			mcs_num=1
		elif  [ "$rate" = "5.5M" ] ; then
			mcs_num=2
		elif  [ "$rate" = "11M" ] ; then
			mcs_num=3
		else
			echo "Non-ht rate $rate error,rate value select 1M/2M/5.5M/11M!!"
			__exit_scrpit
		fi
		rate_index=$((mcs_num * 2 + gi_num))
		if [ $rate_index -gt 8 ]; then
			echo "non-ht cck rate_index($rate_index >= 8) error!"
			__exit_scrpit
		fi
	fi
	if [ "$prot" = "NON-HT_OFDM" ]; then
		non_ht_ofdm_check_rate_arg
		gi_num=$GI
		echo "GI: $GI,rate: $rate"
		if  [ "$rate" = "6M" ] ; then
			mcs_num=0
		elif  [ "$rate" = "9M" ] ; then
			mcs_num=1
		elif  [ "$rate" = "12M" ] ; then
			mcs_num=2
		elif  [ "$rate" = "18M" ] ; then
			mcs_num=3
		elif  [ "$rate" = "24M" ] ; then
			mcs_num=4
		elif  [ "$rate" = "36M" ] ; then
			mcs_num=5
		elif  [ "$rate" = "48M" ] ; then
			mcs_num=6
		elif  [ "$rate" = "54M" ] ; then
			mcs_num=7
		else
			echo "Non-HT rate $rate error,rate value select 6M/9M/12/18M/...!!"
			__exit_scrpit
		fi
		rate_index=$((mcs_num + 8))
		if [ $rate_index -gt 15 ]; then
			echo "non-ht ofdm rate_index($rate_index >= 16) error!"
			__exit_scrpit
		fi
	fi
	if [ "$prot" = "HT_MF" ]; then
		ht_check_rate_arg
		mcs_num=${rate#MCS}
		if [ $mcs_num -lt 0 ] || [ $mcs_num -gt 7 ];then
			echo "$prot MCS set $rate error!only support MCS0~MCS7"
			__exit_scrpit
		fi
		if [ $BW = 20 ] || [ $BW = 40 ];then
			bw_num=$((BW / 20))
			bw_num=$((bw_num - 1))
		else
			echo "BW $BW error!only suport 20M/40M"
			__exit_scrpit
		fi
		nss_num=$((NSS - 1))
		gi_num=$GI
		rate_index=$((16 + 32 * nss_num + 4 * mcs_num + 2 * bw_num + gi_num))
		if [ $rate_index -gt 144 ]; then
			echo "HT rate_index($rate_index >= 144) error!"
			__exit_scrpit
		fi
	fi
	if [ "$prot" = "HT_GF" ]; then
		echo "no support"
		__exit_scrpit
	fi
	if [ "$prot" = "VHT" ]; then
		vht_check_rate_arg
		mcs_num=${rate#MCS}
		if [ $mcs_num -lt 0 ] || [ $mcs_num -gt 9 ];then
			echo "$prot MCS set $rate error!only support MCS0~MCS7"
			__exit_scrpit
		fi
		if [ $BW = 20 ] || [ $BW = 40 ];then
			bw_num=$((BW / 20))
			bw_num=$((bw_num - 1))
		elif [ $BW = 80 ];then
			bw_num=2
		elif [ $BW = 160 ];then
			bw_num=3
		else
			echo "BW $BW error!only suport 20M/40M"
			__exit_scrpit
		fi
		nss_num=$((NSS - 1))
		gi_num=$GI
		rate_index=$((144 + gi_num + 10 * 4 * 2 *nss_num + 2 * bw_num + 8 * mcs_num))
		if [ $rate_index -gt 784 ]; then
			echo "HT rate_index($rate_index >= 784) error!"
			__exit_scrpit
		fi
	fi

	if [ "$prot" = "HE_MU" ]; then
		echo "TODO"
		__exit_scrpit
	fi
	if [ "$prot" = "HE_ER" ]; then
		he_er_check_rate_arg
		mcs_num=${rate#MCS}
		if [ $mcs_num -lt 0 ] || [ $mcs_num -gt 2 ];then
			echo "$prot MCS set $rate error!only support MCS0~MCS7"
			__exit_scrpit
		fi
		gi_num=$GI
		if  [ "$RU" = "RU242" ] ; then
			mcs_num=$((mcs_num + 1))
			gi_num=$((gi_num + 1))
			rate_index=$((3664 + mcs_num * gi_num - 1))
		elif [ "$RU" = "RU106" ]; then
			if [ $mcs_num -eq 0 ];then
			rate_index=$((3664 + 9 + gi_num))
			else
				echo "HT mcs($rate != MCS0) error!"
				__exit_scrpit
			fi
		else
			echo "RU $RU error!"
			__exit_scrpit
		fi
		
			
	fi
	if [ "$prot" = "HE_TB" ]; then
		echo "TODO"
		__exit_scrpit
	fi
}



#########################################################################
__exit_scrpit(){
	echo "============> $script_name Done <============"
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
#########################################################################
if [ $# -lt $force_arg_num ]; then
	__show_help
	echo "============> $script_name Done <============"
	exit
fi
show_log

#########################################################################

if [ $use_rate_control = "true" ];then
rate_index=-1
else
calc_rate_index
echo "rate_index = $rate_index"
fi

echo "set_rate:$set_rate"
echo "all_station:$all_station"
if [ $set_rate = "true" ];then
if [ $all_station = "true" ];then
	echo "set all station"
	if [ $list_station_mac = "00:00:00:00:00:00" ] || [ $station_mac_num -lt 1 ];then
		get_station_mac_dir
	fi

	if [ $station_mac_num -gt 0 ]; then
		sta_config_offset=0
		for sta_mac in ${list_station_mac}; do
			###set rate
			#echo "sta_mac:$sta_mac"
			if [ -f $rate_path/$sta_mac/rc/fixed_rate_idx ]; then
				echo "[ALL STA]echo $rate_index > $rate_path/$sta_mac/rc/fixed_rate_idx"
				echo $rate_index > $rate_path/$sta_mac/rc/fixed_rate_idx
			else
				echo "[ALL STA]echo $rate_index > $rate_path/$sta_mac/rc/fixed_rate_idx"
			fi
			sta_config_offset=$((sta_config_offset + 1))
			if [ $sta_config_offset -ge $station_mac_num ]; then
				break
			fi
		done
	else
		echo "[warn]ALL_STA echo $rate_index > ?"
	fi
else
	if [ $station_mac = "00:00:00:00:00:00" ];then
		get_station_mac_dir
	fi
	#echo "station_mac:$station_mac"
	if [ $station_mac != "00:00:00:00:00:00" ];then
		###set rate
		sta_mac=$station_mac
		echo "sta_mac:$sta_mac"
		if [ -f $rate_path/$sta_mac/rc/fixed_rate_idx ]; then
			echo "echo $rate_index > $rate_path/$sta_mac/rc/fixed_rate_idx"
			echo $rate_index > $rate_path/$sta_mac/rc/fixed_rate_idx
		else
			echo "echo $rate_index > $rate_path/$sta_mac/rc/fixed_rate_idx"
		fi
	else
		echo "[warn]echo $rate_index > ?"
	fi
fi
fi

echo ""
echo "============> $script_name Done <============"
echo ""
exit


