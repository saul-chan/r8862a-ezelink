source "env.sh"

#check error if client not associated
ubus call clmesh.sigma dev_send_1905 "{\"DestALid\":\"${DUT_ALid}\",\"MessageTypeValue\":\"0x8001\"}"

ubus call clmesh.sigma dev_send_1905 "{\"DestALid\":\"${DUT_ALid}\",\"MessageTypeValue\":\"0x8009\",\"tlv_type\":\"0x90\",\"tlv_length\":\"0x000C\",\"tlv_value\":\"{${res_apbssid} ${res_stamac}}\"}"
