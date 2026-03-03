source env.sh

FreqBand="0x00"
ubus call clmesh.sigma dev_send_1905 "{\"DestALid\":\"${DUT_ALid}\",\"MessageTypeValue\":\"0x000A\",\"tlv_type1\":\"0x01\",\"tlv_length1\":\"0x0006\",\"tlv_value1\":\"${CNTLR_ALid}\",\"tlv_type2\":\"0x0F\",\"tlv_length2\":\"0x0001\",\"tlv_value2\":\"{0x00}\",\"tlv_type3\":\"0x10\",\"tlv_length3\":\"0x0001\",\"tlv_value3\":\"${FreqBand}\"}"
