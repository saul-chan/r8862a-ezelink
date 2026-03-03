#. ./env.sh
#source "./env.sh"
source "env.sh"

DUT1_ALid="00:33:33:33:33:44"
AGT1_ALid="00:33:33:33:33:44"
DUT2_ALid="00:33:33:33:66:44"
AGT2_ALid="00:33:33:33:66:44"

#send AP Metrics Query message to agent1
ubus call clmesh.sigma dev_send_1905 "{\"DestALid\":\"${DUT1_ALid}\",\"MessageTypeValue\":\"0x800B\",\"tlv_type\":\"0x93\",\"tlv_length\":\"0x0007\",\"tlv_value\":\"{0x01 {$res_apbssid}}\"}"
#send Link Metrics Query message to agent1
ubus call clmesh.sigma dev_send_1905 "{\"DestALid\":\"${DUT1_ALid}\",\"MessageTypeValue\":\"0x0005\",\"tlv_type\":\"0x08\",\"tlv_length\":\"0x0008\",\"tlv_value\":\"{0x01 {$AGT2_ALid} 0x02}\"}"
#send AP Metrics Query message to agent1
ubus call clmesh.sigma dev_send_1905 "{\"DestALid\":\"${DUT2_ALid}\",\"MessageTypeValue\":\"0x800B\",\"tlv_type\":\"0x93\",\"tlv_length\":\"0x0007\",\"tlv_value\":\"{0x01 {$res_apbssid}}\"}"
#send Link Metrics Query message to agent1
ubus call clmesh.sigma dev_send_1905 "{\"DestALid\":\"${DUT2_ALid}\",\"MessageTypeValue\":\"0x0005\",\"tlv_type\":\"0x08\",\"tlv_length\":\"0x0008\",\"tlv_value\":\"{0x01 {$AGT1_ALid} 0x02}\"}"
#send Combined Infrastructure metrics message
sleep 2
ubus call clmesh.sigma dev_send_1905 "{\"DestALid\":\"${DUT1_ALid}\",\"MessageTypeValue\":\"0x8013\"}"
