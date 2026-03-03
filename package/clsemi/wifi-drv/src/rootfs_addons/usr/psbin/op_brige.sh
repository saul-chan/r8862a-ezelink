#!/bin/bash




#set -e

root_path=$(cd $(dirname $0);pwd)
script_name=$0
echo ""
#echo "root_path: "${root_path}
cd ${root_path}

show_help_info="true"
cmd_show_help_info="false"

usage() {
	echo "$0 help information"
	echo ""
	echo "$0 addbr br-name -i1 eth0 -i2 wlan0 -ip 192.168.3.1 -nw 255.255.255.0 ===>create brige"
	echo "$0 delbr br-name ===>delete brige"
	echo "$0 delif br-name -i1 [ eth0 | wlan0 | ...] -i2 [ eth0 | wlan0 | ...]===>delete brige if"
	echo ""
	
	if [ $cmd_show_help_info = "true" ];then 
		exit 0
	else
		exit 1
	fi
}

create_brige="false"
brige_name="br0"
brige_name_flag=0

brige_inter1="eth0"
brige_inter1_flag=0
brige_inter2="wlan0"
brige_inter2_flag=0

brige_ip=192.168.3.1
brige_ip_flag=0
brige_netmask=255.255.255.0
brige_netmask_flag=0
set_brige_ip_info="false"

delif_flag="false"
delbr_flag="false"

for i in "$@"; do
	if [ $brige_name_flag = 1 ]; then
		brige_name_flag=2
		brige_name=$i
		continue
	fi
	if [ $brige_ip_flag = 1 ]; then
		brige_ip_flag=2
		brige_ip=$i
		continue
	fi
	if [ $brige_netmask_flag = 1 ]; then
		brige_netmask_flag=2
		brige_netmask=$i
		continue
	fi
	if [ $brige_inter1_flag = 1 ]; then
		brige_inter1_flag=2
		brige_inter1="$i"
		continue
	fi
	if [ $brige_inter2_flag = 1 ]; then
		brige_inter2_flag=2
		brige_inter2="$i"
		continue
	fi

	if [ "$i" = "addbr" ];then
		brige_name_flag=1
		create_brige="true"
		show_help_info="false"
		continue
	fi

	if [ "$i" = "delif" ];then
		brige_name_flag=1
		delif_flag="true"
		show_help_info="false"
		continue
	fi

	if [ "$i" = "delbr" ];then
		brige_name_flag=1
		delbr_flag="true"
		show_help_info="false"
		continue
	fi
	
	if [ "$i" = "-i1" ];then
		brige_inter1_flag=1
		continue
	fi
	if [ "$i" = "-i" ];then
		brige_inter1_flag=1
		continue
	fi
	if [ "$i" = "-i2" ];then
		brige_inter2_flag=1
		continue
	fi
	if [ "$i" = "-ip" ];then
		brige_ip_flag=1
		set_brige_ip_info="true"
		continue
	fi
	if [ "$i" = "-nw" ];then
		brige_netmask_flag=1
		continue
	fi
done


##############
###帮助
if [ $cmd_show_help_info = "true" ];then 
show_help_info="true"
fi
if [ ${show_help_info} = "true" ]; then
usage;

fi

###建桥
if [ $create_brige = "true" ]; then
	if [ $brige_name_flag = 2 ]; then
		#brctl addbr br0
		i=1
		find_num=0
		have_set_card=0
		while(true)  
			do  
				 p=`ifconfig -a | grep ^[a-z] | awk -F: '{print $1}' | awk  -v num=${i} 'NR==num{print}'`
				 if [ ! $p ]; then
					echo "That's the last element,index ${i}"
					break
				 fi
				 echo "search ${i} \"${p}\""
				 if [[ `echo $p | grep "br"` ]];then
					let "find_num = find_num + 1"
					echo "${p} is true"
				 fi
				 if [ $p = $brige_name ]; then
					let "have_set_card = have_set_card + 1"
				 fi
				 let "i = i + 1"
			done

		echo "br* find_num = $find_num"
		echo "have_set_card = $have_set_card"
		if [ $have_set_card = 0 ]; then
			echo "create new brige $brige_name"
			brctl addbr $brige_name
		fi
	else
		echo "no set brige name ,exit"
		exit 1
	fi

	if [ $brige_inter1_flag = 2 ]; then
		#brctl addif br0 eth0
		brctl addif $brige_name $brige_inter1
	fi

	if [ $brige_inter2_flag = 2 ]; then
		#brctl addif br0 wlan0
		brctl addif $brige_name $brige_inter2
	fi

	brctl show

	if [ $set_brige_ip_info = "true" ] && [ $brige_ip_flag = 2 ]; then
		#ifconfig br0 192.168.3.1 netmask 255.255.255.0]
		ifconfig $brige_name $brige_ip netmask $brige_netmask
	fi
fi

###删除桥臂的一端
if [ $delif_flag = "true" ]; then
	if [ ! $brige_name_flag = 2 ]; then
		echo "no set brige name ,exit"
		exit 1
	fi
	echo "delete $brige_name brige interface"
	if [ $brige_inter1_flag = 2 ]; then
		#brctl delif br0 eth0
		brctl delif $brige_name $brige_inter1
	fi

	if [ $brige_inter2_flag = 2 ]; then
		#brctl delif br0 wlan0
		brctl delif $brige_name $brige_inter2
	fi
	
	brctl show
fi




###删除桥
if [ $delbr_flag = "true" ]; then
	if [ ! $brige_name_flag = 2 ]; then
		echo "no set brige name ,exit"
		exit 1
	fi
	echo "delete $brige_name brige"
	ifconfig $brige_name down
	brctl delbr $brige_name
	brctl show
fi



