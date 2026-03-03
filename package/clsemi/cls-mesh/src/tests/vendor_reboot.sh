source env.sh

ubus call clmesh.sigma dev_send_1905 "{\"DestALid\":\"${DUT_ALid}\",\"MessageTypeValue\":\"0x8018\",\"tlv_type1\":\"0xa0\",\"tlv_length1\":\"0x0005\",\"tlv_value1\":\"{0xcc 0x63 0x0001 0x01}\"}"
