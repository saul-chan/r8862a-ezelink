#!/bin/bash

set -e

root_path=$(cd $(dirname $0);pwd)
script_name=$0
#echo "root_path: "${root_path}
cd ${root_path}

show_help_info="true"
cmd_show_help_info="false"
usage() {
	echo "\t$0 help information\r\n"
	echo "sh $0 help"
	echo "sh $0 [he | vht | ht | non-ht | non-ht-2g4 | ht-2g4 | he-2g4]  ===> start hostapd"
	exit 1
}

start_hostapd_type="NONE"

for i in "$@"; do
	if [ "$i" = "help" ] || [ "$i" = "HELP" ];then 
		show_help_info="true"
		cmd_show_help_info="true"
	fi

	if [ "$i" = "HE" ] || [ "$i" = "he" ];then 
		start_hostapd_type="HE_AP"
		show_help_info="false"
	fi
	
	if [ "$i" = "HE160" ] || [ "$i" = "he160" ];then 
		start_hostapd_type="HE160_AP"
		show_help_info="false"
	fi

	if [ "$i" = "VHT" ] || [ "$i" = "vht" ];then 
		start_hostapd_type="VHT_AP"
		show_help_info="false"
	fi
	
	if [ "$i" = "VHT160" ] || [ "$i" = "vht160" ];then 
		start_hostapd_type="VHT160_AP"
		show_help_info="false"
	fi

	if [ "$i" = "HT" ] || [ "$i" = "ht" ];then 
		start_hostapd_type="HT_AP"
		show_help_info="false"
	fi

	if [ "$i" = "NON-HT" ] || [ "$i" = "non-ht" ];then 
		start_hostapd_type="NON-HT_AP"
		show_help_info="false"
	fi
	if [ "$i" = "NON-HT-2G4" ] || [ "$i" = "non-ht-2g4" ];then 
		start_hostapd_type="NON-HT_AP-2G4"
		show_help_info="false"
	fi
	if [ "$i" = "HT-2G4" ] || [ "$i" = "ht-2g4" ];then 
		start_hostapd_type="HT_AP-2G4"
		show_help_info="false"
	fi
	if [ "$i" = "HE-2G4" ] || [ "$i" = "he-2g4" ];then 
		start_hostapd_type="HE_AP-2G4"
		show_help_info="false"
	fi
done

if [ ${start_hostapd_type} = "HE_AP" ]; then
echo "start he 5G AP"
/usr/psbin/hostapd /usr/psbin/hostapd_he.conf -ddd &
elif [ ${start_hostapd_type} = "HE160_AP" ]; then
echo "start he 160M 5G AP"
/usr/psbin/hostapd /usr/psbin/hostapd_he160.conf -d &
elif [ ${start_hostapd_type} = "VHT_AP" ]; then
echo "start vht 5G AP"
/usr/psbin/hostapd /usr/psbin/hostapd_vht.conf -ddd &
elif [ ${start_hostapd_type} = "VHT160_AP" ]; then
echo "start vht 5G AP"
/usr/psbin/hostapd /usr/psbin/hostapd_vht160.conf -ddd &
elif [ ${start_hostapd_type} = "HT_AP" ]; then
echo "start ht 5G AP"
/usr/psbin/hostapd /usr/psbin/hostapd_ht.conf -ddd &
elif [ ${start_hostapd_type} = "NON-HT_AP" ]; then
echo "start non-ht 5G AP"
/usr/psbin/hostapd /usr/psbin/hostapd_minimal_open.conf -ddd &
elif [ ${start_hostapd_type} = "NON-HT_AP-2G4" ]; then
echo "start non-ht 2.4G AP"
/usr/psbin/hostapd /usr/psbin/hostapd_2g_non_ht.conf -ddd &
elif [ ${start_hostapd_type} = "HT_AP-2G4" ]; then
echo "start ht 2.4G AP"
/usr/psbin/hostapd /usr/psbin/hostapd_2g_ht.conf -ddd &
elif [ ${start_hostapd_type} = "HE_AP-2G4" ]; then
echo "start he 2.4G AP"
/usr/psbin/hostapd /usr/psbin/hostapd_2g_he.conf -ddd &
else
echo "arg error"
exit
fi

if [ ${show_help_info} = "true" ]; then
usage;



fi

echo "=========== $script_name ==========="
