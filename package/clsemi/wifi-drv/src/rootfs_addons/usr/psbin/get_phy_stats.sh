#!/bin/bash

# need one parameter
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <0|1>"
    echo "0: 2g4"
    echo "1: 5g"
    exit 1
fi

# get parameter
band="$1"

# check parameter
if [ "$band" != "0" ] && [ "$band" != "1" ]; then
    echo "Invalid parameter: $band. Parameter must be 0 or 1."
	exit 1
fi

#set path
base_path="/sys/kernel/debug/ieee80211"

sta_base_path="${base_path}/phy${band}/cls_wifi/stations"

dbgcnt_path="${base_path}/phy${band}/cls_wifi/dbgcnt"
mib_path="${base_path}/phy${band}/cls_wifi/mib"
phydfx_path="${base_path}/phy${band}/cls_wifi/phydfx"
txq_path="${base_path}/phy${band}/cls_wifi/txq"


# dump radio info
echo "cat ${dbgcnt_path}"
echo;
cat "${dbgcnt_path}"
echo;

echo "cat ${mib_path}"
echo;
cat "${mib_path}"
echo;

echo "cat ${phydfx_path}"
echo;
cat "${phydfx_path}"
echo;

echo "cat ${txq_path}"
echo;
cat "${txq_path}"
echo;


# dump all sta info
ls -d "${sta_base_path}"/*/ | while read -r folder; do
    # get folder name
    sta="$(basename "$folder")"
	sta_path="${sta_base_path}/${sta}"

	echo "cat ${sta_path}/rc/stats"
	cat "${sta_path}/rc/stats"
	echo;

	echo "cat ${sta_path}/rc/tx_rate"
	cat "${sta_path}/rc/tx_rate"
	echo;

	echo "cat ${sta_path}/rc/rx_rate"
	cat "${sta_path}/rc/rx_rate"
	echo;
done
