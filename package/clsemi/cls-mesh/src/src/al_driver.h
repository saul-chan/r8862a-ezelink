#ifndef _AL_DRIVER_H_
#define _AL_DRIVER_H_

int radioAddAP(struct radio *r, struct bss_info *bss, struct bss_info *backhaul_bss);
int radioAddSTA(struct radio *r, struct bss_info *bss);
int radioTeardown(struct radio *r);
int radioCommit(struct radio *r);
int radioSetChannelTxPower(struct radio *r, int opclass, int channel, int power);
int bssSendMgmtFrame(int ifidx, uint8_t *frame, uint32_t frame_len);
int bssRegisterMgmtFrame(int ifidx, uint16_t frame_type, uint8_t *match, uint16_t match_len);
int bssTriggerChannelScan(int ifidx, uint32_t opclass, uint8_t *channels, uint8_t chan_num);
int bssGetLastChannelScanResult(int ifidx, int start_num);
int bssDeauth(struct wifi_interface *wif, struct client *c, int16_t reason);
int bssStartWPS(int32_t ifindex, uint8_t role, char *ifname);
int stationGetStats(int ifidx, struct client *c);
int staDeny(struct wifi_interface *wif, uint8_t *mac, uint8_t flag);
int updateVipDSCPConf(void);
int updateVipQueueConf(void);
int updateVipTcConf(void);
int updateVipSTAConf(void);
int mappingConfAction(struct DSCP_mapping_conf *conf);
int queueConfAction(struct dlist_head *conf);
int tcConfAction(struct tc_mapping_conf *conf);
int VipStaAction(void);
int InitVipConf(struct map_policy *policy);
int updateBSS(struct al_device *d);
int ethVlanSet();
int addNacMonitorSta(uint8_t channel, uint8_t *mac);
int delNacMonitorSta(uint8_t channel, uint8_t *mac);
int setNacMonitorEnable(uint8_t channel, uint8_t enable);
int flushNacSta(uint8_t channel);
int bssGetChannelSurvey(int ifidx);

#endif
