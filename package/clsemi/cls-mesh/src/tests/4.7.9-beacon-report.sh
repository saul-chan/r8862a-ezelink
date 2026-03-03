source "env.sh"
sta1_mac="0x020000000100"
#associate client to agent

#client query to get client capability
ubus call clmesh.sigma dev_send_1905 "{\"DestALid\":\"${DUT_ALid}\",\"MessageTypeValue\":\"0x8009\",\"tlv_type\":\"0x90\",\"tlv_length\":\"0x000C\",\"tlv_value\":\"{${res_apbssid} ${res_stamac}}\"}"

#send beacon report query
ubus call clmesh.sigma dev_send_1905 "{\"DestALid\":\"$DUT_ALid\",\"MessageTypeValue\":\"0x8011\",\"tlv_type\":\"0x99\",\"tlv_length\":\"0x0016\",\"tlv_value\":\"{$sta1_mac 0x73 0xFF 0xFFFFFFFFFFFF 0x02 0x00 0x01 0x03 0x73 0x24 0x30 0x00}\"}"
