#!/bin/bash

#set -e

root_path=$(cd $(dirname $0);pwd)
script_name=$0

echo ""
echo "root_path: "${root_path}
cd ${root_path}

show_help_info="true"
cmd_show_help_info="false"
wpa_supplicant_config_flie=${root_path}/wpa_supplicant_minimal.conf

usage() {
	echo "$0 help information"
	echo ""
	echo "启动wpa_supplicant"
	echo "默认使用的配置脚本： " $wpa_supplicant_config_flie
	echo "sh $0 help"
	echo "sh $0 start -i [ wlan0 | wlan1 | wlan2 | ...]  ===> start wpa"
	echo "sh $0 stop -i [ wlan0 | wlan1 | wlan2 | ...]  ===> stop wpa"
	echo ""
	echo "sh $0 status -i [ wlan0 | wlan1 | wlan2 | ...]  ===> status wpa"
	echo ""
	echo "sh $0 scan -i [ wlan0 | wlan1 | wlan2 | ...]  ===> scan"
	echo ""
	echo "sh $0 set_mac -i [ wlan0 | wlan1 ] -m mac ===> set mac"
	echo "sh $0 set_mac -i [ wlan0 | wlan1 ] ===> set random mac"
	echo "sh $0 env -i [ wlan0 | wlan1 ] ===> 为wpa启动复位环境 "
	echo ""
	echo "sh $0 wpa_cfg_file -i [ wlan0 | wlan1 ] ===> 生成wpa_supplicant配置文件 "
	echo ""
	echo "sh $script_name connect ssid -p pwd -i [ wlan0 | wlan1 | wlan2 | ...] ==> set connect ap information"
	echo "sh $script_name connect ssid -i [ wlan0 | wlan1 | wlan2 | ...] ==> set connect ap information"
	
	echo "sh $script_name connect-id netid -i [ wlan0 | wlan1 | wlan2 | ...] ==> connect netid ap(连接netid指示的AP)"
	echo ""
	echo "sh $script_name disconnect netid -i [ wlan0 | wlan1 | wlan2 | ...] ==> disconnect netid ap(断开连接netid指示的AP)"
	echo ""
	echo "sh $script_name list -i [ wlan0 | wlan1 | wlan2 | ...] ==> Show list of available APs"
	echo "sh $script_name ping -i [ wlan0 | wlan1 | wlan2 | ...] ==> wpa_cli ping wpa_supplicant(测试wpa_cli是否可以和wpa_supplicant通信)"
	echo ""
	echo "=====> wpa连接上AP之后，获取或者配置IP"
	echo "sh $script_name get-ip -i [ wlan0 | wlan1 | wlan2 | ...] ==> dhcp client get ip(动态获取IP)"
	echo "sh $script_name static-ip -ip 192.168.3.10 -nm 255.255.255.0 -i [ wlan0 | wlan1 | wlan2 | eth0 | ...] ==> set default static-ip(设置静态IP和掩码)"
	echo "sh $script_name del_ip -i [ wlan0 | wlan1 | wlan2 | ...] ==> 删除静态ip"
	echo "sh $script_name static-gw -gw 192.168.3.1 -i [ wlan0 | wlan1 | wlan2 | ...] ==> 设置默认网关"
	echo ""
	
	
	#echo "[=================== 调用的脚本 ===================]"
	#sh ./op_start_wpa.sh help;
	###. ./op_start_wpa.sh help;
	if [ $cmd_show_help_info = "true" ];then 
		exit 0
	else
		exit 1
	fi
}

start_wpa_type="wlan0"
set_wpa_sup_config_path_flag=0
start_wpa_type_flag=0
start_wpa_flag="false"
stop_wpa_flag="false"
set_mac_mac="false"
set_mac_value=$(echo $RANDOM | md5sum | sed 's/../&:/g' | cut -c1-17)
set_mac_arg_flag=0
env__flag="false"
scan_flag="false"
status_flag="false"

wpa_ping_flag="false"

connect_flag="false"
connect_ssid="SSPU_G"
connect_pwd="open"
get_ssid_flag="false"
get_ssid_pwd_flag=0

connect_netid_flag="false"
get__netid_flag=0
network_index=0

disconnect_flag="false"

show_network_list_flag="false"

get_ip_flag="false"
static_ip_flag="false"
static_ip="192.168.3.10"
static_ip_netmask="255.255.255.0"
static_ip_gw="192.168.3.1"
static_ip_flag=0
static_ip_nm_flag=0
static_ip_gw_flag=0
static_gw_flag="false"
del_static_ip="false"

wpa_cfg_file_flag="false"

for i in "$@"; do
	if [ $set_wpa_sup_config_path_flag = 1 ]; then
		set_wpa_sup_config_path_flag=2
		wpa_supplicant_config_flie=$i
		continue
	fi
	if [ $static_ip_gw_flag = 1 ]; then
		static_ip_gw_flag=0
		static_ip_gw="$i"
		continue
	fi
	if [ $static_ip_flag = 1 ]; then
		static_ip_flag=0
		static_ip="$i"
		continue
	fi
	if [ $static_ip_nm_flag = 1 ]; then
		static_ip_nm_flag=0
		static_ip_netmask="$i"
		continue
	fi
	if [ $get__netid_flag = 1 ]; then
		get__netid_flag=0
		network_index=$i
		continue
	fi
	if [ $start_wpa_type_flag = 1 ]; then
		start_wpa_type_flag=0;
		start_wpa_type=$i
		continue
	fi
	if [ $set_mac_arg_flag = 1 ]; then
		set_mac_arg_flag=0
		set_mac_value="$i"
		continue
	fi
	if [ $get_ssid_pwd_flag = 1 ]; then
		get_ssid_pwd_flag=2
		connect_pwd="$i"
		continue
	fi
	if [ $get_ssid_flag = "true" ]; then
		get_ssid_flag="false"
		connect_ssid="$i"
		continue
	fi
	if [ "$i" = "-ip" ] || [ "$i" = "-IP" ];then 
		static_ip_flag=1;
		continue;
	fi
	if [ "$i" = "-nm" ] || [ "$i" = "-NM" ];then 
		static_ip_nm_flag=1;
		continue;
	fi
	if [ "$i" = "-gw" ] || [ "$i" = "-GW" ];then 
		static_ip_gw_flag=1;
		continue;
	fi
	if [ "$i" = "del_ip" ];then
		del_static_ip="true"
		show_help_info="false"
		continue
	fi
	if [ "$i" = "list" ];then
		show_network_list_flag="true"
		show_help_info="false"
		continue
	fi
	if [ "$i" = "wpa_cfg_file" ];then
		wpa_cfg_file_flag="true"
		show_help_info="false"
		continue
	fi
	
	if [ "$i" = "connect-id" ];then
		connect_netid_flag="true"
		get__netid_flag=1
		show_help_info="false"
		continue
	fi
	if [ "$i" = "ping" ];then
		wpa_ping_flag="true"
		show_help_info="false"
		continue
	fi
	if [ "$i" = "connect" ];then
		connect_flag="true"
		show_help_info="false"
		get_ssid_flag="true"
		continue
	fi

	if [ "$i" = "disconnect" ];then
		disconnect_flag="true"
		show_help_info="false"
		get_ssid_flag="true"
		continue
	fi

	if [ "$i" = "env" ] || [ "$i" = "ENV" ] || [ "$i" = "Env" ];then 
		env__flag="true"
		show_help_info="false"
		continue
	fi

	if [ "$i" = "start" ] || [ "$i" = "START" ] || [ "$i" = "Start" ];then 
		start_wpa_flag="true"
		show_help_info="false"
		continue
	fi

	if [ "$i" = "status" ];then
		status_flag="true"
		show_help_info="false"
		continue
	fi
	
	if [ "$i" = "scan" ];then
		scan_flag="true"
		show_help_info="false"
		continue
	fi

	if [ "$i" = "stop" ] || [ "$i" = "STOP" ] || [ "$i" = "Stop" ];then 
		stop_wpa_flag="true"
		show_help_info="false"
		continue
	fi
	
	if [ "$i" = "set_mac" ] || [ "$i" = "SET_MAC" ] || [ "$i" = "set-mac" ];then 
		set_mac_mac="true"
		show_help_info="false"
		continue
	fi

	if [ "$i" = "-m" ] || [ "$i" = "-M" ];then 
		set_mac_arg_flag=1
		continue
	fi

	if [ "$i" = "-p" ] || [ "$i" = "-P" ];then 
		get_ssid_pwd_flag=1
		continue
	fi

	if [ "$i" = "help" ] || [ "$i" = "HELP" ];then 
		show_help_info="true"
		cmd_show_help_info="true"
		continue
	fi

	if [ "$i" = "-i" ] || [ "$i" = "-I" ];then 
		start_wpa_type_flag=1;
		continue;
	fi

	if [ "$i" = "get-ip" ];then
		get_ip_flag="true"
		show_help_info="false"
		continue
	fi
	
	if [ "$i" = "static-ip" ];then
		static_ip_flag="true"
		show_help_info="false"
		continue
	fi
	
	if [ "$i" = "static-gw" ];then
		static_gw_flag="true"
		show_help_info="false"
		continue
	fi
done


####生成默认的wpa_supplicant 配置脚本
if [[ $wpa_cfg_file_flag = "true" ]]; then
	echo "create wpa_supplicant config file"
	if [ $start_wpa_type != "NONE" ];then
		set_netinterface_vlaue=$start_wpa_type
	else
		set_netinterface_vlaue=wlan0
	fi

	##先过滤网卡名称,存到数组a中
	#a=(`ifconfig | grep ^[a-z] | awk -F: '{print $1}'`)
	##在拿到IP地址，存到数组b中
	#b=(`ifconfig | grep 'inet' | sed 's/^.*inet //g' | sed 's/ *netmask.*$//g'`)
	#for ((i=0;i<6;i++))
	  #do
		#echo ${a[$i]} ${b[$i]}
      #done
	i=1
	find_num=0
	have_set_card=0
	while(true)  
		do  
			 p=`ifconfig | grep ^[a-z] | awk -F: '{print $1}' | awk  -v num=${i} 'NR==num{print}'`
			 if [ ! $p ]; then
				echo "That's the last element,index ${i}"
				break
			 fi
			 echo "search ${i} \"${p}\""
			 if [[ `echo $p | grep "wlan"` ]];then
				let "find_num = find_num + 1"
				echo "${p} is true"
			 fi
			 if [ $p = $set_netinterface_vlaue ]; then
				let "have_set_card = have_set_card + 1"
			 fi
			 let "i = i + 1"
		done
	
	echo "find_num = $find_num"
	echo "have_set_card = $have_set_card"

	if [ -f ${root_path}/wpa_supplicant_minimal_$set_netinterface_vlaue.conf ];then
		rm -rf ${root_path}/wpa_supplicant_minimal_$set_netinterface_vlaue.conf
	fi
	wpa_supplicant_config_flie=${root_path}/wpa_supplicant_minimal.conf
	
	if [ $find_num -gt 1 ] || [ -f $wpa_supplicant_config_flie ];then
		wpa_supplicant_config_flie=${root_path}/wpa_supplicant_minimal_$set_netinterface_vlaue.conf
	fi

	echo wpa_supplicant_config_flie = $wpa_supplicant_config_flie
	touch $wpa_supplicant_config_flie
	chmod 777 $wpa_supplicant_config_flie

	echo "# wpa_supplicant minimal config" >> $wpa_supplicant_config_flie
	echo "update_config=1" >> $wpa_supplicant_config_flie
	#if [ $find_num -gt 1 ] && [ $wpa_supplicant_config_flie != "wlan0" ];then
	if [ $find_num != 1 ]; then
		echo "ctrl_interface=/var/run/wpa_supplicant_${set_netinterface_vlaue}" >> $wpa_supplicant_config_flie
	else
		echo "ctrl_interface=/var/run/wpa_supplicant" >> $wpa_supplicant_config_flie
	fi
	echo "ctrl_interface_group=0" >> $wpa_supplicant_config_flie
	echo "eapol_version=1" >> $wpa_supplicant_config_flie
	echo "country=CN" >> $wpa_supplicant_config_flie
	echo "p2p_disabled=1" >> $wpa_supplicant_config_flie
	echo "" >> $wpa_supplicant_config_flie
	echo "" >> $wpa_supplicant_config_flie
	echo "" >> $wpa_supplicant_config_flie
	echo "" >> $wpa_supplicant_config_flie

fi

## 查找wpa_supplicant 配置脚本
if [ $set_wpa_sup_config_path_flag = 0 ] && [ ! $start_wpa_type = "NONE" ] && [ $start_wpa_flag = "true" ]; then
	if [ -f ${root_path}/wpa_supplicant_minimal_$start_wpa_type.conf ];then
		wpa_supplicant_config_flie=${root_path}/wpa_supplicant_minimal_$start_wpa_type.conf
	elif [ -f ${root_path}/wpa_supplicant_minimal.conf ];then
		cp -rf ${root_path}/wpa_supplicant_minimal.conf ${root_path}/wpa_supplicant_minimal_$start_wpa_type.conf
		chmod 777 ${root_path}/wpa_supplicant_minimal_$start_wpa_type.conf
		wpa_supplicant_config_flie=${root_path}/wpa_supplicant_minimal_$start_wpa_type.conf
	else
		echo "wpa_supplicant 没有找到默认的配置脚本"
		exit
	fi
fi

############

##启动wpa_supplicant
if [ ${start_wpa_type} != "NONE" ] && [ $start_wpa_flag = "true" ]; then
	echo ""
	if [ ! -f $wpa_supplicant_config_flie ]; then
		echo "wpa_supplicant 没有找到 $wpa_supplicant_config_flie 配置脚本"
	fi
	echo "rfkill unblock wifi"
	rfkill unblock wifi
	echo "ifconfig ${start_wpa_type} up"
	ifconfig ${start_wpa_type} up
	echo "wpa_supplicant ${start_wpa_type} config file: "$wpa_supplicant_config_flie
	#sudo wpa_supplicant -D nl80211 -i wlan0 -c ./wpa_supplicant_minimal_start.conf -B
	wpa_supplicant -D nl80211 -i ${start_wpa_type} -c $wpa_supplicant_config_flie -dddd &
	echo ""
	wpa_comm_path=/var/run/wpa_supplicant
	if [ -d /var/run/wpa_supplicant_${start_wpa_type} ]; then
		wpa_comm_path=/var/run/wpa_supplicant_${start_wpa_type}
	fi
	echo "wpa_sup communication path: "$wpa_comm_path
	#chmod 777 $wpa_comm_path
	echo ""
	##ps -e | grep wpa_supplicant
	echo ""
fi

############

##停止wpa_supplicant
if [ ${start_wpa_type} != "NONE" ] && [  $stop_wpa_flag = "true" ]; then
	echo ""
	ps -e | grep wpa_supplicant
	
	echo ""
	echo "killall wpa_supplicant ==>"" 杀死同一进程组内的所有进程"
	echo "kill -9 PID ==>" " 迫使进程在运行时突然终止，进程在结束后不能自我清理。危害是导致系统资源无法正常释放，一般不推荐使用，除非其他办法都无效。  "
	echo "kill -l PID ==>" " -l选项告诉kill命令用好像启动进程的用户已注销的方式结束进程。当使用该选项时，kill命令也试图杀死所留下的子进程。但这个命令也不是总能成功--或许仍然需要先手工杀死子进程，然后再杀死父进程。 "
	echo "kill -TERM PID  ==>" " 给父进程发送一个TERM信号，试图杀死它和它的子进程。  "

	###killall命令杀死同一进程组内的所有进程。其允许指定要终止的进程的名称，而非PID。
	##killall wpa_supplicant
fi

############


if [[ $env__flag = "true" ]]; then
	if [ $start_wpa_type != "NONE" ];then
		set_netinterface_vlaue=$start_wpa_type
	else
		set_netinterface_vlaue=wlan0
	fi

	#sudo nmcli device disconnect wlan0
	#sudo nmcli radio wifi off
	echo "nmcli device disconnect ${start_wpa_type}"
	echo "nmcli radio wifi off"

	rfkill unblock wifi

	ifconfig $set_netinterface_vlaue down
	sleep 0.5

	ifconfig $set_netinterface_vlaue up
	sleep 0.5

	rfkill unblock wifi
	#sudo ifconfig wlan0 192.168.0.1/24
	#sudo ifconfig wlan0 192.168.0.1/24 up

	echo ""
	ifconfig $set_netinterface_vlaue
	echo ""

fi

############
if [ ${scan_flag} = "true" ]; then
	if [ ${start_wpa_type} != "NONE" ];then
		wpa_interf=${start_wpa_type}
	else
		wpa_interf=wlan0
	fi
	if [ -d /var/run/wpa_supplicant_${wpa_interf} ]; then
		wpa_cli -p /var/run/wpa_supplicant_${wpa_interf} -i ${wpa_interf} scan
		wpa_cli -p /var/run/wpa_supplicant_${wpa_interf} -i ${wpa_interf} scan_results
	else
		wpa_cli -i ${wpa_interf} scan
		sleep 5
		wpa_cli -i ${wpa_interf} scan_results
	fi
fi

############
if [ ${status_flag} = "true" ]; then
	if [ ${start_wpa_type} != "NONE" ];then
		wpa_interf=${start_wpa_type}
	else
		wpa_interf=wlan0
	fi

	if [ -d /var/run/wpa_supplicant_${wpa_interf} ]; then
		wpa_cli -p /var/run/wpa_supplicant_${wpa_interf} status
	else
		wpa_cli status
	fi
fi

############
if [ ${connect_flag} = "true" ]; then
	if [ ${start_wpa_type} != "NONE" ];then
		wpa_interf=$start_wpa_type
	else
		wpa_interf=wlan0
	fi

	wpa_sup_comm_path=/var/run/wpa_supplicant
	if [ -d /var/run/wpa_supplicant_${wpa_interf} ]; then
		wpa_sup_comm_path=/var/run/wpa_supplicant_${wpa_interf}
	fi

	# 添加一个网络连接,会返回<network id>
	###network_id=$(wpa_cli -i wlan0 add_network -p $wpa_sup_comm_path)
	echo "wpa_cli -i ${wpa_interf} add_network -p $wpa_sup_comm_path"
	network_id=$(wpa_cli -i ${wpa_interf} add_network -p $wpa_sup_comm_path)
	echo "network_id = " $network_id
	# sudo chmod 777 /var/run/wpa_supplicant/ -R
	## //ssid名称
	# sudo wpa_cli set_network $network_id ssid '"SSPU_G"'
	# sudo wpa_cli set_network $network_id ssid "\"wlin\""
	echo "set-ssid: " "\"${connect_ssid}\""
	wpa_cli set_network $network_id  ssid "\"${connect_ssid}\"" -p $wpa_sup_comm_path

	# 密码
	echo "set-psk: " "\"${connect_pwd}\""
	# sudo wpa_cli set_network 0  psk '"tplink@2020"'
	#sudo wpa_cli set_network $network_id  psk '"tplink2022lin"'
	if [ ! ${connect_pwd} = "open" ]; then
		wpa_cli set_network $network_id  psk "\"${connect_pwd}\"" -p $wpa_sup_comm_path
	else
	# open ap:
		wpa_cli set_network $network_id  key_mgmt  NONE -p $wpa_sup_comm_path
	fi

	#echo "scan_ssid"
	#wpa_cli set_network $network_id  scan_ssid 1 -p $wpa_sup_comm_path

	## 优先级
	echo "set priority"
	# sudo wpa_cli set_network 0  priority  1
	# sudo wpa_cli set_network $network_id  priority  1
	if [ $network_id = 0 ] ; then
		wpa_cli set_network $network_id  priority  0 -p $wpa_sup_comm_path
	else
		wpa_cli set_network $network_id  priority  1 -p $wpa_sup_comm_path
	fi

	## 使能制定的ssid
	##wpa_cli -i wlan0 enable_network $network_id -p $wpa_sup_comm_path
	wpa_cli -i ${wpa_interf} enable_network $network_id -p $wpa_sup_comm_path

	echo "save_config"
	## 信息保存到默认的配置文件中，前面提到的/etc/wpa_supplicant.conf
	##wpa_cli -i wlan0 save_config -p $wpa_sup_comm_path
	wpa_cli -i ${wpa_interf} save_config -p $wpa_sup_comm_path
	
	###若是第一个SSID和PWD，则立即进行连接
	if [ $network_id = 0 ]; then
		#sudo wpa_cli -i wlan0 select_network $network_index
		wpa_cli -i ${wpa_interf} select_network $network_index -p $wpa_sup_comm_path
	fi
fi

##############
###选择网络连接
if [ ${connect_netid_flag} = "true" ]; then
	if [ ${start_wpa_type} != "NONE" ];then
		wpa_interf=$start_wpa_type
	else
		wpa_interf=wlan0
	fi
	wpa_sup_comm_path=/var/run/wpa_supplicant
	if [ -d /var/run/wpa_supplicant_${wpa_interf} ]; then
		wpa_sup_comm_path=/var/run/wpa_supplicant_${wpa_interf}
	fi
	echo "Tabular information"
	#sudo wpa_cli -i wlan0 list_network
	wpa_cli -i $wpa_interf list_network  -p $wpa_sup_comm_path
	echo ""
	# 使用list_network命令，会打印所有已添加成功的WiFi热点，
	# 如果有多个WiFi热点，可以使用select_network命令选择使用哪个热点，可实现WiFi热点的切换。
	# sudo wpa_cli -i wlan0 select_network 0
	echo "select_network indx : " $network_index
	#sudo wpa_cli -i wlan0 select_network $network_index
	wpa_cli -i $wpa_interf select_network $network_index -p $wpa_sup_comm_path
	echo ""
fi

##############
####断开网络
if [ $disconnect_flag = "true" ]; then
	if [ ${start_wpa_type} != "NONE" ];then
		wpa_interf=$start_wpa_type
	else
		wpa_interf=wlan0
	fi
	wpa_sup_comm_path=/var/run/wpa_supplicant
	if [ -d /var/run/wpa_supplicant_${wpa_interf} ]; then
		wpa_sup_comm_path=/var/run/wpa_supplicant_${wpa_interf}
	fi
	# sudo wpa_cli -i wlan0 disable_network 0
	wpa_cli -i $wpa_interf disable_network $network_index -p $wpa_sup_comm_path
fi

###########
####显示已经配置的SSID
if [ ${show_network_list_flag} = "true" ]; then
	if [ ${start_wpa_type} != "NONE" ];then
		wpa_interf=$start_wpa_type
	else
		wpa_interf=wlan0
	fi
	wpa_sup_comm_path=/var/run/wpa_supplicant
	if [ -d /var/run/wpa_supplicant_${wpa_interf} ]; then
		wpa_sup_comm_path=/var/run/wpa_supplicant_${wpa_interf}
	fi
	## 使用list_network命令，会打印所有已添加成功的WiFi热点
	#sudo wpa_cli -i wlan0 list_network
	wpa_cli -i $wpa_interf list_network -p $wpa_sup_comm_path
fi

##############
######测试wpa_cli是否可以和wpa_supplicant通信
if [ $wpa_ping_flag = "true" ]; then
	if [ ${start_wpa_type} != "NONE" ];then
		wpa_interf=$start_wpa_type
	else
		wpa_interf=wlan0
	fi
	wpa_sup_comm_path=/var/run/wpa_supplicant
	if [ -d /var/run/wpa_supplicant_${wpa_interf} ]; then
		wpa_sup_comm_path=/var/run/wpa_supplicant_${wpa_interf}
	fi
	echo "[$wpa_interf]wpa_cli ping wpa_supplicant"
	# pings wpa_supplicant
	#sudo wpa_cli -i wlan0 ping
	wpa_cli -i $wpa_interf ping -p $wpa_sup_comm_path
fi

################
###动态ip
if [ ${get_ip_flag} = "true" ]; then
	#sudo dhclient
	i=1
	find_num=0
	#	ip a | grep "state UP" | awk '{print $2}'| awk -F: '{print $1}'
	while(true)  
	do  
		 #echo "i = ${i}"
		 #p=`ip a | grep "state UP" | awk '{print $2}'| awk -F: '{print $1}' | awk 'NR==/'$i/'{print}'   `
		 p=`ps -e | grep "dhclient" | awk '{print $4}' | awk  -v num=${i} 'NR==num{print}'   `
		 if [ ! $p ]; then
			echo "no find dhclient,index ${i}"
			break
		 fi
		 echo "search ${i} \"${p}\""
		 if [ $p == "dhclient" ];then
			let "find_num = find_num + 1"
			echo "${p} is true"
		 fi
		 
		 let "i = i + 1"
		 #i=`expr ${i} + 1`
	done
	if [ ${find_num} -eq 0 ]; then
		echo "no find dhclient,start dhclient"
		dhclient
	fi
fi

###############
##设置静态IP和掩码
if [ ${static_ip_flag} = "true" ]; then
	if [ $start_wpa_type != "NONE" ];then
		wpa_interf=$start_wpa_type
	else
		wpa_interf=wlan0
	fi
	
	echo "$wpa_interf SET IP: "$static_ip "netmask $static_ip_netmask"
	#ifconfig wlan0 192.168.1.119 netmask 255.255.255.0
	#route add default gw 192.168.1.1
	ifconfig $wpa_interf $static_ip netmask $static_ip_netmask
	ifconfig $wpa_interf
	#route add default gw $static_ip_gw
fi

#########
##删除静态IP和掩码
if [ ${del_static_ip} = "true" ]; then
	if [ $start_wpa_type != "NONE" ];then
		wpa_interf=$start_wpa_type
	else
		wpa_interf=wlan0
	fi

	#ifconfig wlan0 192.168.1.119 netmask 255.255.255.0
	#route add default gw 192.168.1.1
	ifconfig $wpa_interf 0.0.0.0
	ifconfig $wpa_interf
	#route add default gw $static_ip_gw
fi


###############
##设置默认网关
if [ $static_gw_flag = "true" ]; then
	echo "route add default gw $static_ip_gw"
	#ifconfig wlan0 192.168.1.119 netmask 255.255.255.0
	#route add default gw 192.168.1.1
	#ifconfig $wpa_interf $static_ip netmask $static_ip_netmask
	route add default gw $static_ip_gw
fi


##############
###帮助
if [ ${show_help_info} = "true" ]; then
usage;

fi


### 设置网卡的MAC地址
set_netinterface_vlaue=${start_wpa_type}
if [ ${set_netinterface_vlaue} != "NONE" ] && [ $set_mac_mac = "true" ]; then
	echo "set ${set_netinterface_vlaue} mac: "$set_mac_value
	ifconfig $set_netinterface_vlaue down
	ifconfig $set_netinterface_vlaue hw ether $set_mac_value
	#ifconfig $set_netinterface_vlaue up
	ifconfig $set_netinterface_vlaue
fi







echo ""
echo "=========== $script_name done ==========="
echo ""
