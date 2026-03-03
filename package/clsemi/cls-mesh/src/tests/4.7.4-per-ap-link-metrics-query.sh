source "env.sh"

#ap metrics query
ubus call clmesh.sigma dev_send_1905 "{\"DestALid\":\"$DUT_ALid\",\"MessageTypeValue\":\"0x800B\",\"tlv_type\":\"0x93\",\"tlv_length\":\"0x0007\",\"tlv_value\":\"{0x01 {$res_apbssid}}\"}"
