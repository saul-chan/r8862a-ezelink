#ifndef _FEATURE_HELPER_H_
#define _FEATURE_HELPER_H_

int alDeviceUpdateEvt(uint8_t *mac);
int sendBtmResponseEvent(uint8_t *al_mac, uint8_t *mac, uint8_t *wif_mac, uint8_t status_code, uint8_t *target);
int stationUpdateEvent(uint8_t *mac, uint8_t *wif_mac, uint8_t ul_rcpi, uint8_t last_rcpi);
int sendClientAssoEvent(uint8_t *mac, uint8_t *wif_mac);
int mapSteeringPolicyChangedEvent(uint8_t *mac, uint8_t rcpi_thresh, uint8_t chan_util, uint8_t policy);
int assocStaLinkMetricsQueryRspEvent(uint8_t *al_mac, uint8_t *mac, uint8_t rcpi_ul, uint8_t mac_rate_ul, uint8_t mac_rate_dl);
int sendVbssStartEvent(uint8_t *sta_mac, uint8_t *src_agent, uint8_t *target_ruid);
int sendVbssClientSecurityContextReqEvent(uint16_t cid, uint8_t *al_mac, uint8_t *bssid, uint8_t *client_mac);
int sendVbssCreationEvent(uint16_t cid, uint8_t *al_mac, uint8_t *ruid, uint8_t *bssid, struct vvData *ssid,
            struct vvData *wpa_password, uint8_t *client_mac, uint8_t client_assoc, struct vvData *ptk,
            uint64_t tx_packet_number, struct vvData *gtk, uint64_t group_tx_packet_number, struct vvData *sta_assoc_req);
int sendVbssDestructionEvent(uint16_t cid, uint8_t *al_mac, uint8_t *ruid, uint8_t *bssid, uint8_t client_disassoc);
int sendVbssResponseEvent(uint8_t *al_mac, uint8_t *ruid, uint8_t *bssid, uint8_t success);
int sendVbssTriggerCSARspEvent(uint8_t *al_mac, uint8_t *bssid, uint8_t *client_mac);

#endif
