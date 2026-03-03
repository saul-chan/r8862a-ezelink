#include "platform.h"
#include "datamodel.h"
#include "al_msg.h"
#include "al_action.h"
#include "wifi.h"
#include "platform_interfaces.h"

int radioAddAP(struct radio *r, struct bss_info *bss, struct bss_info *backhaul_bss)
{
    uint8_t *buf = msgGetBuf(0);
    uint8_t *p = buf;
    int i=0;

    MSG_PUT_HEADER(p, msg_family_driver_cmd, drv_cmd_bss);

    p = msgaPutU8(p, attr_bss_role, role_ap);
    p = msgaPutBin(p, attr_radio_mac, r->uid, MACLEN);
    p = msgaPutBin(p, attr_ssid, bss->ssid.ssid, bss->ssid.len);
    p = msgaPutU8(p, attr_auth, bss->auth);
    p = msgaPutU8(p, attr_encrypt, bss->encrypt);

    if (bss->key.len)
        p = msgaPutBin(p, attr_key, bss->key.key, bss->key.len);

    if (bss->backhaul)
        p = msgaPutFlag(p, attr_backhaul);

    if ((bss->fronthaul) && (backhaul_bss)) {
        p = msgaPutFlag(p, attr_fronthaul);
        p = msgaPutBin(p, attr_backhaul_ssid, backhaul_bss->ssid.ssid,
                        backhaul_bss->ssid.len);
        p = msgaPutU8(p, attr_backhaul_auth, backhaul_bss->auth);
        p = msgaPutU8(p, attr_backhaul_encrypt, backhaul_bss->encrypt);

        if (backhaul_bss->key.len)
            p = msgaPutBin(p, attr_backhaul_key, backhaul_bss->key.key, backhaul_bss->key.len);
    }

    while (bss->vlan_map[i]) {
        p = msgaPutU16(p, attr_vlan_0+i, bss->vlan_map[i]);
        i++;
    }

    msgSend(buf, p-buf);

    msgPutBuf(buf);
    return 0;
}

int radioAddSTA(struct radio *r, struct bss_info *bss)
{
    uint8_t *buf = msgGetBuf(0);
    uint8_t *p = buf;

    MSG_PUT_HEADER(p, msg_family_driver_cmd, drv_cmd_bss);

    p = msgaPutU8(p, attr_bss_role, role_sta);
    p = msgaPutBin(p, attr_radio_mac, r->uid, MACLEN);
    p = msgaPutBin(p, attr_ssid, bss->ssid.ssid, bss->ssid.len);
    p = msgaPutU8(p, attr_auth, bss->auth);
    p = msgaPutU8(p, attr_encrypt, bss->encrypt);

    if (bss->key.len)
        p = msgaPutBin(p, attr_key, bss->key.key, bss->key.len);

    msgSend(buf, p-buf);

    msgPutBuf(buf);
    return 0;
}

int radioTeardown(struct radio *r)
{
    uint8_t *buf = msgGetBuf(0);
    uint8_t *p = buf;

    MSG_PUT_HEADER(p, msg_family_driver_cmd, drv_cmd_teardown);

    p = msgaPutBin(p, attr_radio_mac, r->uid, MACLEN);

    msgSend(buf, p-buf);
    msgPutBuf(buf);

    return 0;
}

int radioSetChannelTxPower(struct radio *r, int opclass, int channel, int power)
{
    uint8_t *buf = msgGetBuf(0);
    uint8_t *p = buf;

    MSG_PUT_HEADER(p, msg_family_driver_cmd, drv_cmd_set_channel_txpower);

    p = msgaPutBin(p, attr_radio_mac, r->uid, MACLEN);

    if ((opclass>=0) && (channel>=0)) {
        p = msgaPutU8(p, attr_opclass, opclass);
        p = msgaPutU8(p, attr_channel, channel);
    }

    if (power>=0)
        p = msgaPutU8(p, attr_power_lvl, power);

    msgSend(buf, p-buf);
    msgPutBuf(buf);

    return 0;
}

int bssSendMgmtFrame(int ifidx, uint8_t *frame, uint32_t frame_len)
{
    uint8_t *buf = msgGetBuf(0);
    uint8_t *p = buf;

    MSG_PUT_HEADER(p, msg_family_driver_cmd, drv_cmd_send_mgmt_frame);
    p = msgaPutU32(p, attr_if_idx, ifidx);
    p = msgaPutBin(p, attr_frame, frame, frame_len);

    msgSend(buf, p-buf);
    msgPutBuf(buf);

    return 0;
}

int bssRegisterMgmtFrame(int ifidx, uint16_t frame_type, uint8_t *match, uint16_t match_len)
{
    uint8_t *buf = msgGetBuf(0);
    uint8_t *p = buf;

    MSG_PUT_HEADER(p, msg_family_driver_cmd, drv_cmd_register_mgmt_frame);
    p = msgaPutU32(p, attr_if_idx, ifidx);
    p = msgaPutU16(p, attr_frame_type, frame_type);
    if (match_len > 0)
        p = msgaPutBin(p, attr_match, (uint8_t *)match, match_len);

    msgSend(buf, p-buf);
    msgPutBuf(buf);

    return 0;
}

int bssTriggerChannelScan(int ifidx, uint32_t opclass, uint8_t *channels, uint8_t chan_num)
{
    uint8_t *buf = msgGetBuf(0);
    uint8_t *p = buf;
    int i;

    MSG_PUT_HEADER(p, msg_family_driver_cmd, drv_cmd_trigger_scan);
    p = msgaPutU32(p, attr_if_idx, ifidx);
    p = msgaPutU32(p, attr_opclass, opclass);

    for (i = 0; i < chan_num; i++)
        p = msgaPutU8(p, attr_channel, channels[i]);

    msgSend(buf, p-buf);
    msgPutBuf(buf);

    return 0;
}

int bssGetLastChannelScanResult(int ifidx, int start_num)
{
    uint8_t *buf = msgGetBuf(0);
    uint8_t *p = buf;

    MSG_PUT_HEADER(p, msg_family_driver_cmd, drv_cmd_get_chan_scan_results);
    p = msgaPutU32(p, attr_if_idx, ifidx);
    p = msgaPutU32(p, attr_start_num, start_num);

    msgSend(buf, p-buf);
    msgPutBuf(buf);

    return 0;
}

int stationGetStats(int ifidx, struct client *c)
{
    uint8_t *buf = msgGetBuf(0);
    uint8_t *p = buf;

    MSG_PUT_HEADER(p, msg_family_driver_cmd, drv_cmd_get_station_stats);
    p = msgaPutU32(p, attr_if_idx, ifidx);
    if (c)
        p = msgaPutBin(p, attr_mac, c->mac, MACLEN);

    msgSend(buf, p-buf);
    msgPutBuf(buf);

    return 0;
}

int radioCommit(struct radio *r)
{
    uint8_t *buf = msgGetBuf(0);
    uint8_t *p = buf;

    MSG_PUT_HEADER(p, msg_family_driver_cmd, drv_cmd_commit);

    msgSend(buf, p-buf);

    msgPutBuf(buf);
    return 0;
}

int mappingConfAction(struct DSCP_mapping_conf *conf)
{
    uint8_t *buf = msgGetBuf(0);
    uint8_t *p = buf;
    struct DSCP_mapping_item *item;

    MSG_PUT_HEADER(p, msg_family_driver_cmd, drv_cmd_dscp_action);

    dlist_for_each(item, conf->dscp_list, l) {
        p = msgaPutU8(p, attr_dscp_value, item->dscp_value);
        p = msgaPutU8(p, attr_dscp_tid, item->tid);
        p = msgaPutU8(p, attr_dscp_qid, item->queue_id);
    }
    if (conf->dft_qid != 255 && conf->dft_tid != 255) {
        p = msgaPutU8(p, attr_dscp_dft_qid, conf->dft_qid);
        p = msgaPutU8(p, attr_dscp_dft_tid, conf->dft_tid);
    }

    msgSend(buf, p-buf);
    msgPutBuf(buf);
    return 0;
}

int queueConfAction(struct dlist_head *conf)
{
    uint8_t *buf = msgGetBuf(0);
    uint8_t *p = buf;
    struct queue_conf_item *item;

    MSG_PUT_HEADER(p, msg_family_driver_cmd, drv_cmd_queue_action);

    dlist_for_each(item, *conf, l) {
        p = msgaPutU8(p, attr_queue_port, item->port_id);
        p = msgaPutU8(p, attr_queue_id, item->queue_id);
        p = msgaPutU8(p, attr_queue_weight, item->weight);
    }

    msgSend(buf, p-buf);
    msgPutBuf(buf);
    return 0;
}

int tcConfAction(struct tc_mapping_conf *conf)
{
    uint8_t *buf = msgGetBuf(0);
    uint8_t *p = buf;
    struct tc_mapping_item *item;

    MSG_PUT_HEADER(p, msg_family_driver_cmd, drv_cmd_tc_action);

    dlist_for_each(item, conf->mapping_list, l) {
        p = msgaPutU8(p, attr_tc_value, item->tc_value);
        p = msgaPutU8(p, attr_tc_tid, item->tid);
        p = msgaPutU8(p, attr_queue_id, item->queue_id);
    }

    if (conf->dft_qid != 255 && conf->dft_tid != 255) {
        p = msgaPutU8(p, attr_tc_dft_qid, conf->dft_qid);
        p = msgaPutU8(p, attr_tc_dft_tid, conf->dft_tid);
    }
    msgSend(buf, p-buf);
    msgPutBuf(buf);
    return 0;
}

int bssDeauth(struct wifi_interface *wif, struct client *c, int16_t reason)
{
    uint8_t *buf = msgGetBuf(0);
    uint8_t *p = buf;

    MSG_PUT_HEADER(p, msg_family_driver_cmd, drv_cmd_deauth_sta);
    p = msgaPutU32(p, attr_if_idx, wif->i.index);
    p = msgaPutBin(p, attr_mac, c->mac, MACLEN);
    p = msgaPutU16(p, attr_reason_code, reason);

    msgSend(buf, p-buf);
    msgPutBuf(buf);
    return 0;
}

int staDeny(struct wifi_interface *wif, uint8_t *mac, uint8_t flag)
{
    uint8_t *buf = msgGetBuf(0);
    uint8_t *p = buf;

    if (flag)
        MSG_PUT_HEADER(p, msg_family_driver_cmd, drv_cmd_deny_sta_add);
    else
        MSG_PUT_HEADER(p, msg_family_driver_cmd, drv_cmd_deny_sta_del);

    p = msgaPutU32(p, attr_if_idx, wif->i.index);
    p = msgaPutBin(p, attr_mac, mac, MACLEN);

    msgSend(buf, p-buf);
    msgPutBuf(buf);
    return 0;
}

int bssStartWPS(int32_t ifindex, uint8_t role, char *ifname)
{
    uint8_t *buf = msgGetBuf(0);
    uint8_t *p = buf;

    MSG_PUT_HEADER(p, msg_family_driver_cmd, drv_cmd_start_wps);
    p = msgaPutU32(p, attr_if_idx, ifindex);
    p = msgaPutU8(p, attr_bss_role, role);
    if (ifname)
         p = msgaPutStr(p, attr_if_name, ifname);

    msgSend(buf, p-buf);
    msgPutBuf(buf);
    return 0;
}

int VipStaAction(void)
{
    uint8_t *buf = msgGetBuf(0);
    uint8_t *p = buf;
    struct mac_item *vip;;

    if (!dlist_empty(&local_policy.vips)) {
        MSG_PUT_HEADER(p, msg_family_driver_cmd, drv_cmd_vip_add);

        dlist_for_each(vip, local_policy.vips, l) {
            p = msgaPutBin(p, attr_mac, vip->mac, MACLEN);
        }
    }
    else
        MSG_PUT_HEADER(p, msg_family_driver_cmd, drv_cmd_vip_clear);

    msgSend(buf, p-buf);
    msgPutBuf(buf);
    return 0;
}

char *vip_conf_keywords[VIP_QOS_KEYWORDS_NUM] = {
    "Port",
    "Queue",
    "Priority",
    "DSCP",
    "TID",
    "TC",
};

int jumpFirstLine(char *line_conf)
{
    char *key_words = NULL;
    int i;

    for(i = 0; i < VIP_QOS_KEYWORDS_NUM; i++) {
        key_words = strstr(line_conf, vip_conf_keywords[i]);
        if (key_words)
            return 1;
    }
    return 0;
}

int parseVipQueueConf(char *line_conf)
{
    int qid = 0, weight = 0, port = 0, num = 0;
    int port_begin = 0, port_end = 0, i;
    char *sep = NULL;
    struct queue_conf_item *item;

    sep = strchr(line_conf, '~');
    if (!sep) {
        num = sscanf(line_conf, "%d %d %d", &port, &qid, &weight);
        if (VIP_QUEUE_PARAMS_NUM == num) { /* queue id will be the index */
            item = calloc(1, sizeof(struct queue_conf_item));
            if (!item) {
                DEBUG_WARNING("no space for vip queue conf\n");
                return -1;
            }

            item->port_id = port;
            item->queue_id = qid;
            item->weight = weight;
            dlist_add_tail(&local_policy.queue_conf, &item->l);
        }
    }
    else {
        num = sscanf(line_conf, "%d~%d %d %d", &port_begin, &port_end, &qid, &weight);
        if ((VIP_QUEUE_PARAMS_NUM + 1)!= num)
            return -1;
        for (i = port_begin; (i <= port_end && i < APPLY_PORT_NUM); i++) {
            item = calloc(1, sizeof(struct queue_conf_item));
            if (!item) {
                DEBUG_WARNING("no space for vip queue conf\n");
                return -1;
            }

            item->port_id = i;
            item->queue_id = qid;
            item->weight = weight;
            dlist_add_tail(&local_policy.queue_conf, &item->l);
        }
    }
    return 0;
}

int parseVipDSCPConf(char *line_conf)
{
    int qid = 0, dscp = 0, tid = 0, num = 0;
    struct DSCP_mapping_item *item;

    num = sscanf(line_conf, "%d %d %d", &dscp, &tid, &qid);
    if (VIP_DSCP_PARAMS_NUM == num) {
        if (-1 != dscp) { /* specifical config, dscp value will be index */
            item = calloc(1, sizeof(struct tc_mapping_item));
            if (!item) {
                DEBUG_WARNING("no space for parse dscp conf\n");
                return -1;
            }
            item->queue_id = qid;
            item->dscp_value = dscp;
            item->tid = tid;
            dlist_add_tail(&local_policy.dscp_conf.dscp_list, &item->l);
        }
        else { /* default config, index will be the dscp value */
            local_policy.dscp_conf.dft_tid = tid;
            local_policy.dscp_conf.dft_qid = qid;
        }
        return 0;
    }
    else /* get params wrong */
        return -1;
}

int parseVipTcConf(char *line_conf)
{
    int qid = 0, tc = 0, tid = 0, num = 0;
    struct tc_mapping_item *item;

    num = sscanf(line_conf, "%d %d %d", &tc, &tid, &qid);
    if (VIP_TC_PARAMS_NUM == num) {
        if (-1 != tc) {
            item = calloc(1, sizeof(struct tc_mapping_item));
            if (!item) {
                DEBUG_WARNING("no space for parse tc conf\n");
                return -1;
            }
            item->queue_id = qid;
            item->tc_value = tc;
            item->tid = tid;
            dlist_add_tail(&local_policy.tc_conf.mapping_list, &item->l);
        }
        else { /* default config*/
            local_policy.tc_conf.dft_qid = qid;
            local_policy.tc_conf.dft_tid = tid;
        }
        return 0;
    }
    else /* get params wrong */
        return -1;
}


int parseVipConf(char *line_conf, enum vip_conf_type type)
{
    int ret = 0;

    switch(type) {
        case VIP_CONF_TYPE_QUEUE:
            ret = parseVipQueueConf(line_conf);
            break;

        case VIP_CONF_TYPE_DSCP:
            ret = parseVipDSCPConf(line_conf);
            break;

        case VIP_CONF_TYPE_TC:
            ret = parseVipTcConf(line_conf);
            break;

        default:
            ret  = -1;
            break;
    }
    return ret;
}

int getVipConf(struct map_policy *policy, enum vip_conf_type type)
{
    char conf_path[64] = {0};
    FILE *conf = NULL;
    char *one_line = NULL;
    size_t len = 0;
    ssize_t nread;
    int jumped = 0;

    if (!policy)
        return -1;

    switch(type) {
        case VIP_CONF_TYPE_QUEUE:
            strcpy(conf_path, VIP_QUEUE_CONF_PATH);
            break;

        case VIP_CONF_TYPE_DSCP:
            strcpy(conf_path, VIP_DSCP_CONF_PATH);
            break;

        case VIP_CONF_TYPE_TC:
            strcpy(conf_path, VIP_TC_CONF_PATH);
            break;

        default:
            strcpy(conf_path, "/etc/cls-qos/dummy.conf");
            break;
    }

    conf = fopen(conf_path, "r");
    if (!conf) {
        DEBUG_INFO("can NOT open file(%s)\n", conf_path);
        return -1;
    }
    one_line = calloc(1, 128);
    if (!one_line) {
        DEBUG_WARNING("NO space for parsing VIP conf\n");
        fclose(conf);
        return -1;
    }
    while((nread = getline(&one_line, &len, conf)) != -1)
    {
        if (0 == nread)
            break;
        if (!jumped) {
            jumped = jumpFirstLine(one_line);
            if (jumped)
                continue; /* jump the first line, it's the title */
        }
        parseVipConf(one_line, type);
    }
    free(one_line);
    fclose(conf);
    return 0;
}

int updateVipDSCPConf(void)
{
    if (isRegistrar()) {
        dlist_free_items(&local_policy.dscp_conf.dscp_list, struct DSCP_mapping_item, l);
        local_policy.dscp_conf.dft_qid = 255;
        local_policy.dscp_conf.dft_tid = 255;
        getVipConf(&local_policy, VIP_CONF_TYPE_DSCP);
        doMappingConfAction(&local_policy.dscp_conf);
        syncMappingConf();
    }
    return 0;
}

int updateVipQueueConf(void)
{
    if (isRegistrar()) {
        dlist_free_items(&local_policy.queue_conf, struct queue_conf_item, l);
        getVipConf(&local_policy, VIP_CONF_TYPE_QUEUE);
        doQueueConfAction(&local_policy.queue_conf);
        syncQueueConf();
    }
    return 0;
}

int updateVipTcConf(void)
{
    if (isRegistrar()) {
        dlist_free_items(&local_policy.tc_conf.mapping_list, struct queue_conf_item, l);
        local_policy.tc_conf.dft_qid = 255;
        local_policy.tc_conf.dft_tid = 255;
        getVipConf(&local_policy, VIP_CONF_TYPE_TC);
        doTcConfAction(&local_policy.tc_conf);
        syncTcConf();
    }
    return 0;
}

int updateVipSTAConf(void)
{
    if (isRegistrar()) {
        doVipAction();
        syncVIPSta();
        return 0;
    }
    return -1;
}

int InitVipConf(struct map_policy *policy)
{
#define MAX_VIP_STA 128
    local_device->cls_cap.vip_max = MAX_VIP_STA;
    local_device->max_vid = 8;
    updateVipQueueConf();
    updateVipDSCPConf();
    updateVipTcConf();
    updateVipSTAConf();
    return 0;
}

static uint32_t _getBSSInfo(struct radio *r, struct wifi_interface *wintf)
{
    uint32_t bssinfo = NEI_REP_BSSID_INFO_AP_REACHABLE;
    struct band_capability *band_cap = &r->bands_capa[r->current_band_idx];

    if (band_cap->ht_capa_valid) {
        bssinfo |= NEI_REP_BSSID_INFO_HT|NEI_REP_BSSID_INFO_DELAYED_BA;
    }
    if (band_cap->vht_capa_valid) {
        bssinfo |= NEI_REP_BSSID_INFO_VHT;
    }
    if (band_cap->he_capa_valid) {
        bssinfo |= NEI_REP_BSSID_INFO_HE;
    }
    return bssinfo;
}

static uint8_t _getPHYType(struct radio *r)
{
    struct band_capability *band_cap = &r->bands_capa[r->current_band_idx];

    if (band_cap->he_capa_valid) {
        return PHY_TYPE_HE;
    } else if (band_cap->vht_capa_valid) {
        return PHY_TYPE_VHT;
    } else if (band_cap->ht_capa_valid) {
        return PHY_TYPE_HT;
    } else
        return PHY_TYPE_UNSPECIFIED;
}


int updateBSS(struct al_device *d)
{
#define FILL_SET_NEIGHBOR(_radio, _intf)\
    do {\
        p = msgaPutBin(p, attr_bssid, (_intf)->i.mac, MACLEN);\
        p = msgaPutU8(p, attr_flag, FLAG_ADD);\
        p = msgaPutStr(p, attr_ssid, (char *)(_intf)->bssInfo.ssid.ssid);\
        p_end = addNeighborReport(p+2, (_intf)->i.mac, (_radio)->opclass, (_radio)->channel, _getBSSInfo((_radio),(_intf)),\
            _getPHYType((_radio)), NULL, 0);\
        msgaPutAttrHeader(p, attr_nr, p_end-p-4);\
        p = p_end;\
    }while(0)
#define FILL_DEL_NEIGHBOR(_intf)\
    do {\
        p = msgaPutBin(p, attr_bssid, (_intf)->i.mac, MACLEN);\
        p = msgaPutU8(p, attr_flag, FLAG_DEL);\
    }while(0)
    struct radio *r;
    struct wifi_interface *wintf;
    uint8_t *buf = msgGetBuf(0);
    uint8_t *p = buf, *p_end;

    MSG_PUT_HEADER(p, msg_family_driver_cmd, drv_cmd_set_neighbor);

    dlist_for_each(r, d->radios, l) {

        if ((!r->channel) || (!r->opclass))
            continue;

        struct interface *intf, *tmp;
        dlist_for_each_safe(intf, tmp, d->interfaces, l) {
            if (intf->type == interface_type_wifi) {
                wintf=(struct wifi_interface *)intf;
                if (wintf->radio==r) {

                if (wintf->mark.new) {
                    wintf->neighbor_set = 1;
                    FILL_SET_NEIGHBOR(r, wintf);
                } else if (wintf->mark.change) {
                    if (wintf->neighbor_set) {
                        FILL_DEL_NEIGHBOR(wintf);
                    }
                    wintf->neighbor_set = 1;
                    FILL_SET_NEIGHBOR(r, wintf);
                } else if (!wintf->mark.hit) {
                    if (wintf->neighbor_set) {
                        FILL_DEL_NEIGHBOR(wintf);
                    }
                    wifiInterfaceDelete(wintf);
                    continue;
                }
            }
            }
        }
    }
    msgSend(buf, p-buf);
    msgPutBuf(buf);
    return 0;
}

int ethVlanSet()
{
    struct interface_list_item *item;
    dlist_head *interfaces = PLATFORM_GET_LIST_OF_1905_INTERFACES();

    uint8_t *buf = msgGetBuf(0);
    uint8_t *p = buf;

    MSG_PUT_HEADER(p, msg_family_driver_cmd, drv_cmd_eth_vlan);

    dlist_for_each(item, *interfaces, l) {
        if (item->fix)
            p = msgaPutStr(p, attr_if_name, item->intf_name);
    }

    if (local_policy.def_vlan) {
        struct vlan_config_item *vlan;
        uint16_t attr_vlan = attr_vlan_0+1;

        p = msgaPutU16(p, attr_vlan_0, local_policy.def_vlan);
        dlist_for_each(vlan, local_policy.vlans, l2.l) {
            if (vlan->vlan!=local_policy.def_vlan) {
                p = msgaPutU16(p, attr_vlan++, vlan->vlan);
            }
        }
    }
    msgSend(buf, p-buf);
    msgPutBuf(buf);
    return 0;
}

int addNacMonitorSta(uint8_t channel, uint8_t *mac)
{
    uint8_t *buf = msgGetBuf(0);
    uint8_t *p = buf;

    MSG_PUT_HEADER(p, msg_family_driver_cmd, drv_cmd_set_nac_sta);
    p = msgaPutU8(p, attr_channel, channel);
    p = msgaPutBin(p, attr_mac, mac, MACLEN);
    p = msgaPutU8(p, attr_flag, 0);

    msgSend(buf, p-buf);
    msgPutBuf(buf);

    return 0;
}

int delNacMonitorSta(uint8_t channel, uint8_t *mac)
{
    uint8_t *buf = msgGetBuf(0);
    uint8_t *p = buf;

    MSG_PUT_HEADER(p, msg_family_driver_cmd, drv_cmd_set_nac_sta);
    p = msgaPutU8(p, attr_channel, channel);
    p = msgaPutBin(p, attr_mac, mac, MACLEN);
    p = msgaPutU8(p, attr_flag, 1);

    msgSend(buf, p-buf);
    msgPutBuf(buf);

    return 0;
}

int setNacMonitorEnable(uint8_t channel, uint8_t enable)
{
    uint8_t *buf = msgGetBuf(0);
    uint8_t *p = buf;

    MSG_PUT_HEADER(p, msg_family_driver_cmd, drv_cmd_set_nac_enable);
    p = msgaPutU8(p, attr_channel, channel);
    p = msgaPutU8(p, attr_flag, enable);

    msgSend(buf, p-buf);
    msgPutBuf(buf);

    return 0;
}

int flushNacSta(uint8_t channel)
{
    uint8_t *buf = msgGetBuf(0);
    uint8_t *p = buf;

    MSG_PUT_HEADER(p, msg_family_driver_cmd, drv_cmd_flush_nac_sta);
    p = msgaPutU8(p, attr_channel, channel);

    msgSend(buf, p-buf);
    msgPutBuf(buf);

    return 0;
}

int bssGetChannelSurvey(int ifidx)
{
    uint8_t *buf = msgGetBuf(0);
    uint8_t *p = buf;

    MSG_PUT_HEADER(p, msg_family_driver_cmd, drv_cmd_get_chan_survey);
    p = msgaPutU32(p, attr_if_idx, ifidx);

    msgSend(buf, p-buf);
    msgPutBuf(buf);

    return 0;
}

