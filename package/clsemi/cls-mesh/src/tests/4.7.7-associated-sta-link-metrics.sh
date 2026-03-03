source "env.sh"

#associated sta link metrics query
ubus call clmesh.sigma dev_send_1905 "{\"DestALid\":\"$DUT_ALid\",\"MessageTypeValue\":\"0x800D\",\"tlv_type\":\"0x95\",\"tlv_length\":\"0x0006\",\"tlv_value\":\"{$res_stamac}\"}"
