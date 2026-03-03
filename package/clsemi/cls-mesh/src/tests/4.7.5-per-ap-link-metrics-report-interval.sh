source "env.sh"

#config policy with metrics report interval 5s
ubus call clmesh.sigma dev_send_1905 "{\"DestALid\":\"$DUT_ALid\",\"MessageTypeValue\":\"0x8003\",\"tlv_type\":\"0x8A\",\"tlv_length\":\"0x000C\",\"tlv_value\":\"{0x05 0x01 {$res_ruid 0x00 0x00 0x00 0xE0}}\"}"

sleep 20
#see metrics report for every 5s and sta traffic at least 10s age


#config policy with metrics report interval 0s, which disable periodriclly metrics report
ubus call clmesh.sigma dev_send_1905 "{\"DestALid\":\"$DUT_ALid\",\"MessageTypeValue\":\"0x8003\",\"tlv_type\":\"0x8A\",\"tlv_length\":\"0x0002\",\"tlv_value\":\"{0x00 0x00}\"}"
