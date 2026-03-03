source env.sh

ubus call clmesh.sigma dev_send_1905 "{\"DestALid\":\"${DUT_ALid}\",\"MessageTypeValue\":\"0x8014\",\"tlv_type\":\"0xC3\",\"tlv_length\":\"0x001C\",\"tlv_value\":\"{${res_apbssid} 0xE0 0x0000 0x1388 0x01 {${res_stamac}} 0x01 {$res_agbssid 0x51 0x06 0x00}}\"}"
