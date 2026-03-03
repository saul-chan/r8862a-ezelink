#ifndef _AL_ACTION_H_
#define _AL_ACTION_H_

#include "platform.h"
#include "datamodel.h"
#include "1905_tlvs.h"

struct frame_match_desc {
    uint8_t tag;
    uint8_t type;
    uint16_t match_len;
    uint8_t *match;
    int (*process)(struct frame_match_desc *,uint8_t *, uint16_t);
};

#define HAL_ADD_VIP 0
#define HAL_DEL_ALLVIP 1

int doSteerByDeauth(struct client *sta, uint16_t reason_code);
int doSteerByBTM(struct client *sta, uint8_t *target, uint8_t opclass, uint8_t channel,
                        uint8_t disassoc_imm, uint8_t abridged, uint16_t disassoc_timer);
int doMandantorySteer(struct client *sta, uint8_t *target, uint8_t opclass, uint8_t channel, uint8_t param,
                        uint16_t disassoc_timer, uint8_t reason);
int doOpportunitySteer(struct client *sta, uint8_t param, uint16_t disassoc_timer, uint16_t window);
int doStaDenyAdd(struct wifi_interface *wif, uint8_t *sta_mac);
int doStaDenyDel(struct wifi_interface *wif, uint8_t *sta_mac);
int doClientAssociationControl(struct wifi_interface *wif, struct client *sta, uint8_t mode, uint16_t time);
int doMetricsReport(struct al_device *d);
int doTrafficSeperation();
int doBeaconRequest(struct client *sta, struct bcnMetricQueryTLV *qry);
int cbBTMResponse(struct frame_match_desc *desc, uint8_t *p, uint16_t len);
int cbDisassociateSta(struct frame_match_desc *desc, uint8_t *p, uint16_t len);
int cbBeaconReport(struct frame_match_desc *desc, uint8_t *p, uint16_t len);

void startChannelScan(struct chscan_req *req);
void chanScanTimerHandle(void *data);
void metricsReportTimerHandler(void *data);
int doChannelSelection(struct radio *r);
int doVipAction(void);
int doMappingConfAction(struct DSCP_mapping_conf *conf);
void syncMappingConf(void);
int doQueueConfAction(struct dlist_head *conf);
void syncQueueConf(void);
int doTcConfAction(struct tc_mapping_conf *conf);
void syncTcConf(void);
void syncVIPSta(void);
void doAutoRole(void);


#endif
