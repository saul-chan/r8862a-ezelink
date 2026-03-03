. /lib/functions.sh
. /lib/netifd/netifd-proto.sh

cls_get_opmode()
{
    local mode

    # If cls-opmode package is not enabled, return directly
    config_load cls-opmode || return
    config_get mode globals mode

    echo $mode
}

cls_lan_wan_ip_conflict_fix() {
    local mode
    mode=$(cls_get_opmode)

    if [[ "$mode" == "bridge" || "$mode" == "repeater" ]]; then
        #echo "mode is bridge or repeater, do nothing"
        return 0
    fi

    if [[ "$IPLOCAL" == "" ]]; then
        #echo "pppoe not get ipaddr"
        return 0
    fi
    #cls default lan ip is 192.168.1.1, need change to other subnet if wan port is also 192.168.1.1
    WANADDR=`echo $IPLOCAL | cut -d '.' -f 1-3`
    LANADDR=`ubus call network.interface.lan status | grep \"address\" | awk -F '"' '{print $4}'| head -n 1| cut -d '.' -f 1-3`

    if [ "$WANADDR" == "$LANADDR" ]; then
        logger -p notice -t dhcpc "WAN/LAN IP conflict, change LAN IP automatically"
        echo "mode is router, try fix ip conflict" >> /dev/console

        if [ "$WANADDR" == "192.168.1" ]; then
            uci set network.lan.ipaddr='192.168.10.1'
            uci commit
            /etc/init.d/network restart
        else
            uci set network.lan.ipaddr='192.168.1.1'
            uci commit
            /etc/init.d/network restart
        fi
    fi

}

cls_lan_wan_ip_conflict_fix
