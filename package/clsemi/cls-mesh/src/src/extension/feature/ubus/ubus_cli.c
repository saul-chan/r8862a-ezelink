#include <netinet/ether.h>
#include "platform.h"
#include "datamodel.h"
#include "feature/ubus/ubus_helper.h"
#include "driver/cls/nl80211_cls.h"
#include "version.h"
#include "al_send.h"
#include "al_driver.h"
#include "al_action.h"
#include "platform_os.h"

#define OBJ_NAME "clmesh.cli"

enum {
    ATTR_MSG_TYPE = 0,
    ATTR_DEV,
    ATTR_RADIO,
    ATTR_BSS,
    ATTR_WIPHY,
    ATTR_OPCLASS,
    ATTR_CHANNEL,
    ATTR_TXPOWER,
    ATTR_FRAME,
    ATTR_FRAME_TYPE,
    ATTR_MATCH,
    ATTR_CHPREF_RPT,
    ATTR_CLIENT,
    ATTR_IE,
    ATTR_OPCH_RPT,
    ATTR_CHANNELS,
    ATTR_BTM_DISASSOC_IMM,
    ATTR_BTM_MANDATE,
    ATTR_BTM_ABRIDGED,
    ATTR_BTM_OPPORTUNITY_WIN,
    ATTR_BTM_DISASSOC_TIMER,
    ATTR_BTM_STA_LIST,
    ATTR_BTM_TARGET_LIST,
    ATTR_CHSCAN,
    ATTR_STAMAC,
    ATTR_VBSSINFO,
    ATTR_LOG_LEVEL,
    ATTR_SYSLOG_SWITCH,
    ATTR_INTF,
    ATTR_ENABLE,
    ATTR_MAX,
};

static const struct blobmsg_policy _cli_attr_policy[] = {
    POLICY_ATTR(ATTR_MSG_TYPE, "msg_type", BLOBMSG_TYPE_STRING),
    POLICY_ATTR(ATTR_DEV, "dev", BLOBMSG_TYPE_STRING),
    POLICY_ATTR(ATTR_RADIO, "radio", BLOBMSG_TYPE_STRING),
    POLICY_ATTR(ATTR_BSS, "bss", BLOBMSG_TYPE_STRING),
    POLICY_ATTR(ATTR_WIPHY, "wiphy", BLOBMSG_TYPE_STRING),
    POLICY_ATTR(ATTR_OPCLASS, "opclass", BLOBMSG_TYPE_INT32),
    POLICY_ATTR(ATTR_CHANNEL, "channel", BLOBMSG_TYPE_INT32),
    POLICY_ATTR(ATTR_TXPOWER, "txpower", BLOBMSG_TYPE_INT32),
    POLICY_ATTR(ATTR_FRAME, "frame", BLOBMSG_TYPE_STRING),
    POLICY_ATTR(ATTR_FRAME_TYPE, "frame_type", BLOBMSG_TYPE_INT32),
    POLICY_ATTR(ATTR_MATCH, "match", BLOBMSG_TYPE_STRING),
    POLICY_ATTR(ATTR_CHPREF_RPT, "chan_pref", BLOBMSG_TYPE_TABLE),
    POLICY_ATTR(ATTR_CLIENT, "client", BLOBMSG_TYPE_STRING),
    POLICY_ATTR(ATTR_IE, "ie", BLOBMSG_TYPE_STRING),
    POLICY_ATTR(ATTR_OPCH_RPT, "opch_rpt", BLOBMSG_TYPE_TABLE),
    POLICY_ATTR(ATTR_CHANNELS, "channels", BLOBMSG_TYPE_STRING),
    POLICY_ATTR(ATTR_BTM_DISASSOC_IMM, "btm_imm", BLOBMSG_TYPE_INT32),
    POLICY_ATTR(ATTR_BTM_ABRIDGED, "btm_abridged", BLOBMSG_TYPE_INT32),
    POLICY_ATTR(ATTR_BTM_OPPORTUNITY_WIN, "btm_opportunity_window", BLOBMSG_TYPE_INT32),
    POLICY_ATTR(ATTR_BTM_DISASSOC_TIMER, "btm_disassoc_timer", BLOBMSG_TYPE_INT32),
    POLICY_ATTR(ATTR_BTM_STA_LIST, "btm_sta_list", BLOBMSG_TYPE_ARRAY),
    POLICY_ATTR(ATTR_BTM_TARGET_LIST, "btm_target_list", BLOBMSG_TYPE_ARRAY),
    POLICY_ATTR(ATTR_CHSCAN, "chan_scan", BLOBMSG_TYPE_TABLE),
    POLICY_ATTR(ATTR_STAMAC, "sta_mac", BLOBMSG_TYPE_STRING),
    POLICY_ATTR(ATTR_VBSSINFO, "vbss_info", BLOBMSG_TYPE_TABLE),
    POLICY_ATTR(ATTR_LOG_LEVEL, "log_level", BLOBMSG_TYPE_INT32),
    POLICY_ATTR(ATTR_INTF, "intf", BLOBMSG_TYPE_STRING),
    POLICY_ATTR(ATTR_ENABLE, "enable", BLOBMSG_TYPE_INT32),
};

enum {
    CHAN_SCAN_REQ_OPCLASS_ATTR_OPCLASS,
    CHAN_SCAN_REQ_OPCLASS_ATTR_NUM_CHANS,
    CHAN_SCAN_REQ_OPCLASS_ATTR_CHAN_LIST,
    NUM_CHAN_SCAN_REQ_OPCLASSES_ATTRS,
};

static const struct blobmsg_policy chan_scan_opclass_policy[] = {
    [CHAN_SCAN_REQ_OPCLASS_ATTR_OPCLASS] = { .name = "opclass", .type = BLOBMSG_TYPE_INT32 },
    [CHAN_SCAN_REQ_OPCLASS_ATTR_NUM_CHANS] = { .name = "num_chans", .type = BLOBMSG_TYPE_INT32 },
    [CHAN_SCAN_REQ_OPCLASS_ATTR_CHAN_LIST] = { .name = "chan_list", .type = BLOBMSG_TYPE_STRING },
};

enum {
    CHAN_SCAN_REQ_RADIO_ATTR_RADIO_UID,
    CHAN_SCAN_REQ_RADIO_ATTR_OPC_LIST,
    NUM_CHAN_SCAN_REQ_RADIO_ATTRS,
};

static const struct blobmsg_policy chan_scan_radio_policy[] = {
    [CHAN_SCAN_REQ_RADIO_ATTR_RADIO_UID] = { .name = "radio_uid", .type = BLOBMSG_TYPE_STRING },
    [CHAN_SCAN_REQ_RADIO_ATTR_OPC_LIST] = { .name = "opclass_list", .type = BLOBMSG_TYPE_ARRAY },
};

enum {
    CHAN_SCAN_REQ_ATTR_FRESH_SCAN = 0,
    CHAN_SCAN_REQ_ATTR_RADIO_LIST,
    NUM_CHAN_SCAN_REQ_ATTR_ATTRS,
};

static const struct blobmsg_policy chan_scan_req_policy[] = {
    [CHAN_SCAN_REQ_ATTR_FRESH_SCAN] = { .name = "fresh_scan", .type = BLOBMSG_TYPE_INT32 },
    [CHAN_SCAN_REQ_ATTR_RADIO_LIST] = { .name = "radio_list", .type = BLOBMSG_TYPE_ARRAY },
};

struct chan_scan_radio {
    struct radio *r;
    struct blob_attr *opc_reqs;
    dlist_item l;
};

int parseChanScanOpclasses(struct blob_attr *opc_blob, struct chscan_req *req){
    struct blob_attr *attr = NULL;
    struct blob_attr *tb[NUM_CHAN_SCAN_REQ_OPCLASSES_ATTRS];
    char *ch_list = NULL, *hit = NULL;
    uint8_t channel_list[MAX_CHANNEL_PER_OPCLASS] = {0};
    uint8_t opclass = 0;
    int rem =0;
    int opnum = 0, chnum = 0, i = 0;

    if (!opc_blob || !req) {
        DEBUG_WARNING("No opclasses.\n");
        return 0;
    }

    dlist_head_init(&req->h);
    blobmsg_for_each_attr(attr, opc_blob, rem) {
        blobmsg_parse(chan_scan_opclass_policy, NUM_CHAN_SCAN_REQ_OPCLASSES_ATTRS, tb, blobmsg_data(attr), blobmsg_len(attr));
        if (!tb[CHAN_SCAN_REQ_OPCLASS_ATTR_OPCLASS]) {
            DEBUG_WARNING("No opclass-id in opclasses\n");
            return 0;
        }
        opnum++;
        opclass = blobmsg_get_u32(tb[CHAN_SCAN_REQ_OPCLASS_ATTR_OPCLASS]);
        chnum = blobmsg_get_u32(tb[CHAN_SCAN_REQ_OPCLASS_ATTR_NUM_CHANS]);
        if (chnum && tb[CHAN_SCAN_REQ_OPCLASS_ATTR_CHAN_LIST]) {
            ch_list = blobmsg_get_string(tb[CHAN_SCAN_REQ_OPCLASS_ATTR_CHAN_LIST]);
            hit = strtok(ch_list, ",");
            i = 0;
            while(hit && i < chnum) {
                channel_list[i] = atoi(hit);
                i++;
                hit = strtok(NULL, ",");
            }
            chscanReqItemNew(req, opclass, chnum, &channel_list[0]);
        }
    }
    return opnum;
}

int parseChanScanRadios(struct al_device *d, struct blob_attr *radio_blob, struct chscan_req **reqs, uint8_t fresh_scan) {
    struct blob_attr *attr = NULL;
    struct blob_attr *tb[NUM_CHAN_SCAN_REQ_RADIO_ATTRS];
    dlist_head reqs_list;
    mac_address radio_id = {0};
    struct radio *radio;
    struct chscan_req *reqs_tmp = NULL;
    struct chan_scan_radio *req_radio, *tmp;
    int rem = 0;
    int rnum = 0;

    dlist_head_init(&reqs_list);
    blobmsg_for_each_attr(attr, radio_blob, rem) {
        blobmsg_parse(chan_scan_radio_policy, NUM_CHAN_SCAN_REQ_RADIO_ATTRS, tb, blobmsg_data(attr), blobmsg_len(attr));
        if ((!tb[CHAN_SCAN_REQ_RADIO_ATTR_RADIO_UID])
            ||((fresh_scan) && (!tb[CHAN_SCAN_REQ_RADIO_ATTR_OPC_LIST]))) {
            continue;
        }
        blobmsg_get_mac(tb[CHAN_SCAN_REQ_RADIO_ATTR_RADIO_UID], radio_id);
        if (!(radio = radioFind(d, radio_id))) {
            DEBUG_WARNING("Can NOT find radio["MACFMT"]\n", MACARG(radio_id));
            continue;
        }

        if (!(req_radio = (struct chan_scan_radio *)calloc(1, sizeof(struct chan_scan_radio)))) {
            DEBUG_WARNING("Internal Error\n");
            continue;
        }
        
        req_radio->r = radio;
        if (fresh_scan) {
            req_radio->opc_reqs = tb[CHAN_SCAN_REQ_RADIO_ATTR_OPC_LIST];
        }
        dlist_add_tail(&reqs_list, &req_radio->l);
    }

    if (!dlist_count(&reqs_list))
        return rnum;

    if (!(reqs_tmp = (struct chscan_req *)calloc(dlist_count(&reqs_list), sizeof(struct chscan_req)))) {
        DEBUG_WARNING("Internal Error\n");
        goto bail;
    }

    dlist_for_each(tmp, reqs_list, l) {
        reqs_tmp[rnum].r = tmp->r;
        if (tmp->opc_reqs) {
            if (!parseChanScanOpclasses(tmp->opc_reqs, &reqs_tmp[rnum])) {
                DEBUG_WARNING("Parse opclasses failed.\n");
                PLATFORM_FREE(reqs_tmp);
                rnum = 0;
                goto bail;
            }
        }
        rnum++;
    }
    *reqs = reqs_tmp;

bail:
    dlist_free_items(&reqs_list, struct chan_scan_radio, l);
    return rnum;
}

uint8_t parseChanScanRequstMsg(struct al_device *d, struct blob_attr *chan_scan, uint8_t *fresh_scan, struct chscan_req **reqs) {
    struct blob_attr *tb[NUM_CHAN_SCAN_REQ_ATTR_ATTRS];
    int radio_num = 0;

    blobmsg_parse(chan_scan_req_policy, NUM_CHAN_SCAN_REQ_ATTR_ATTRS, tb, blobmsg_data(chan_scan), blobmsg_len(chan_scan));
    if (!tb[CHAN_SCAN_REQ_ATTR_FRESH_SCAN]
        || !tb[CHAN_SCAN_REQ_ATTR_RADIO_LIST]) {
        DEBUG_WARNING("No fresh_scan or radio_list\n");
        return 0;
    }

    *fresh_scan = blobmsg_get_u32(tb[CHAN_SCAN_REQ_ATTR_FRESH_SCAN]);
    radio_num = parseChanScanRadios(d, tb[CHAN_SCAN_REQ_ATTR_RADIO_LIST], reqs, *fresh_scan);
    if (!radio_num) {
        DEBUG_WARNING("Parse channel scan request failed\n");
    }
    return radio_num;
}

enum {
    CHAN_SCAN_NEIGHBOR_ATTR_BSSID = 0,
    CHAN_SCAN_NEIGHBOR_ATTR_SSID,
    CHAN_SCAN_NEIGHBOR_ATTR_SIGNALSTRENGTH,
    CHAN_SCAN_NEIGHBOR_ATTR_BANDWIDTH,
    CHAN_SCAN_NEIGHBOR_ATTR_ELE_PRESENT,
    CHAN_SCAN_NEIGHBOR_ATTR_BSS_COLOR,
    CHAN_SCAN_NEIGHBOR_ATTR_UTILIZATION,
    CHAN_SCAN_NEIGHBOR_ATTR_STA_COUNT,
    NUM_CHAN_SCAN_NEIGHBOR_ATTRS,
};

static const struct blobmsg_policy chan_scan_neighbor_policy[] = {
    [CHAN_SCAN_NEIGHBOR_ATTR_BSSID] = { .name = "bssid", .type = BLOBMSG_TYPE_STRING },
    [CHAN_SCAN_NEIGHBOR_ATTR_SSID] = { .name = "ssid", .type = BLOBMSG_TYPE_STRING },
    [CHAN_SCAN_NEIGHBOR_ATTR_SIGNALSTRENGTH] = { .name = "signalstrength", .type = BLOBMSG_TYPE_INT32 },
    [CHAN_SCAN_NEIGHBOR_ATTR_BANDWIDTH] = { .name = "bandwidth", .type = BLOBMSG_TYPE_STRING },
    [CHAN_SCAN_NEIGHBOR_ATTR_ELE_PRESENT] = { .name = "ele_present", .type = BLOBMSG_TYPE_INT32 },
    [CHAN_SCAN_NEIGHBOR_ATTR_BSS_COLOR] = { .name = "bss_color", .type = BLOBMSG_TYPE_INT32 },
    [CHAN_SCAN_NEIGHBOR_ATTR_UTILIZATION] = { .name = "utilization", .type = BLOBMSG_TYPE_INT32 },
    [CHAN_SCAN_NEIGHBOR_ATTR_STA_COUNT] = { .name = "sta_cnt", .type = BLOBMSG_TYPE_INT32 },
};

enum {
    CHAN_SCAN_REPORT_ATTR_RADIO_UID = 0,
    CHAN_SCAN_REPORT_ATTR_OPCLASS,
    CHAN_SCAN_REPORT_ATTR_CHANNEL,
    CHAN_SCAN_REPORT_ATTR_SCAN_STATUS,
    CHAN_SCAN_REPORT_ATTR_TIMESTAMP,
    CHAN_SCAN_REPORT_ATTR_UTILIZATION,
    CHAN_SCAN_REPORT_ATTR_NOISE,
    CHAN_SCAN_REPORT_ATTR_NEIGHBOR_LIST,
    CHAN_SCAN_REPORT_ATTR_SCAN_DURATION,
    CHAN_SCAN_REPORT_ATTR_SCAN_TYPE,
    NUM_CHAN_SCAN_REPORT_ATTR_ATTRS,
};

#if 0
static const struct blobmsg_policy chan_scan_report_policy[] = {
    [CHAN_SCAN_REPORT_ATTR_RADIO_UID] = { .name = "radio_uid", .type = BLOBMSG_TYPE_STRING },
    [CHAN_SCAN_REPORT_ATTR_OPCLASS] = { .name = "opclass", .type = BLOBMSG_TYPE_INT32 },
    [CHAN_SCAN_REPORT_ATTR_CHANNEL] = { .name = "channel", .type = BLOBMSG_TYPE_INT32 },
    [CHAN_SCAN_REPORT_ATTR_SCAN_STATUS] = { .name = "scan_status", .type = BLOBMSG_TYPE_INT32 },
    [CHAN_SCAN_REPORT_ATTR_TIMESTAMP] = { .name = "timestamp", .type = BLOBMSG_TYPE_STRING },
    [CHAN_SCAN_REPORT_ATTR_UTILIZATION] = { .name = "utilization", .type = BLOBMSG_TYPE_INT32 },
    [CHAN_SCAN_REPORT_ATTR_NOISE] = { .name = "noise", .type = BLOBMSG_TYPE_INT32 },
    [CHAN_SCAN_REPORT_ATTR_NEIGHBOR_LIST] = { .name = "neighbor_list", .type = BLOBMSG_TYPE_ARRAY },
    [CHAN_SCAN_REPORT_ATTR_SCAN_DURATION] = { .name = "scan_dur", .type = BLOBMSG_TYPE_INT32 },
    [CHAN_SCAN_REPORT_ATTR_SCAN_TYPE] = { .name = "scan_type", .type = BLOBMSG_TYPE_INT32 },
};
#endif

enum {
    VBSS_ATTR_BSSID = 0,
    VBSS_ATTR_SSID,
    VBSS_ATTR_PASSWORD,
    VBSS_ATTR_PTK,
    VBSS_ATTR_GTK,
    VBSS_ATTR_CLIENT_MAC,
    VBSS_ATTR_CLIENT_ASSOC,
    VBSS_ATTR_DISASSOC_CLIENT,
    VBSS_ATTR_TX_PACKET_NUMBER,
    VBSS_ATTR_GROUP_TX_PACKET_NUMBER,
    NUM_VBSS_ATTR_ATTRS,
};

static const struct blobmsg_policy vbss_policy[] = {
    [VBSS_ATTR_BSSID] = { .name = "bssid", .type = BLOBMSG_TYPE_STRING },
    [VBSS_ATTR_SSID] = { .name = "ssid", .type = BLOBMSG_TYPE_STRING },
    [VBSS_ATTR_PASSWORD] = { .name = "password", .type = BLOBMSG_TYPE_STRING },
    [VBSS_ATTR_PTK] = { .name = "ptk", .type = BLOBMSG_TYPE_STRING },
    [VBSS_ATTR_GTK] = { .name = "gtk", .type = BLOBMSG_TYPE_STRING },
    [VBSS_ATTR_CLIENT_MAC] = { .name = "client_mac", .type = BLOBMSG_TYPE_STRING },
    [VBSS_ATTR_CLIENT_ASSOC] = { .name = "client_assoc", .type = BLOBMSG_TYPE_INT8 },
    [VBSS_ATTR_DISASSOC_CLIENT] = { .name = "disassoc_client", .type = BLOBMSG_TYPE_INT8 },
    [VBSS_ATTR_TX_PACKET_NUMBER] = { .name = "tx_packet_number", .type = BLOBMSG_TYPE_INT64 },
    [VBSS_ATTR_GROUP_TX_PACKET_NUMBER] = { .name = "group_tx_packet_number", .type = BLOBMSG_TYPE_INT64 },
};

int parseChanScanNeighbors(struct blob_attr *neighbor_blob, struct chan_info *ch) {
    struct blob_attr *attr = NULL;
    struct blob_attr *tb[NUM_CHAN_SCAN_NEIGHBOR_ATTRS];
    struct neighbor_bss *neighbor = NULL;
    char *ssid = NULL;
    char *bw = NULL;
    int rem = 0;

    if(!dlist_empty(&ch->neighbor_list))
        dlist_free_items(&ch->neighbor_list, struct neighbor_bss, l);
    dlist_head_init(&ch->neighbor_list);
    blobmsg_for_each_attr(attr, neighbor_blob, rem) {
        blobmsg_parse(chan_scan_neighbor_policy, NUM_CHAN_SCAN_NEIGHBOR_ATTRS, tb, blobmsg_data(attr), blobmsg_len(attr));
        if (!tb[CHAN_SCAN_NEIGHBOR_ATTR_BSSID]
            || !tb[CHAN_SCAN_NEIGHBOR_ATTR_SSID]
            || !tb[CHAN_SCAN_NEIGHBOR_ATTR_SIGNALSTRENGTH]
            || !tb[CHAN_SCAN_NEIGHBOR_ATTR_BANDWIDTH]
            || !tb[CHAN_SCAN_NEIGHBOR_ATTR_ELE_PRESENT]
            || !tb[CHAN_SCAN_NEIGHBOR_ATTR_BSS_COLOR]
            || !tb[CHAN_SCAN_NEIGHBOR_ATTR_UTILIZATION]
            || !tb[CHAN_SCAN_NEIGHBOR_ATTR_STA_COUNT]) {
            DEBUG_WARNING("Parameters bssid, ssid, signalstrength, bandwidth, ele_present, bss_color, utilization, sta_cnt should be included\n");
            return 0;
        }
        neighbor = (struct neighbor_bss *)calloc(1, sizeof(struct neighbor_bss));
        blobmsg_get_mac(tb[CHAN_SCAN_NEIGHBOR_ATTR_BSSID], neighbor->bssid);
        ssid = blobmsg_get_string(tb[CHAN_SCAN_NEIGHBOR_ATTR_SSID]);
        neighbor->ssid.len = strlen(ssid);
        memcpy(neighbor->ssid.ssid, ssid, neighbor->ssid.len);
        neighbor->signal_stength = blobmsg_get_u32(tb[CHAN_SCAN_NEIGHBOR_ATTR_SIGNALSTRENGTH]);
        bw = blobmsg_get_string(tb[CHAN_SCAN_NEIGHBOR_ATTR_BANDWIDTH]);
        neighbor->bw = bandwidthStrToIndex(bw);
        neighbor->bss_load_element_present = blobmsg_get_u32(tb[CHAN_SCAN_NEIGHBOR_ATTR_ELE_PRESENT]);
        neighbor->bss_color = blobmsg_get_u32(tb[CHAN_SCAN_NEIGHBOR_ATTR_BSS_COLOR]);
        neighbor->chan_utilize = blobmsg_get_u32(tb[CHAN_SCAN_NEIGHBOR_ATTR_UTILIZATION]);
        neighbor->sta_cnt = blobmsg_get_u32(tb[CHAN_SCAN_NEIGHBOR_ATTR_STA_COUNT]);
        dlist_add_tail(&ch->neighbor_list, &(neighbor->l));
    }
    return 1;
}

#if 0
struct chscan_req *parseChanScanReportMsg(struct al_device *d, struct blob_attr *chan_scan) {
    struct blob_attr *tb[NUM_CHAN_SCAN_REPORT_ATTR_ATTRS];
    struct chscan_req *req;
    mac_address radio_id = {0};
    struct radio *r = NULL;
    struct chscan_req_item *item = NULL;
    struct operating_class *opc = NULL;
    struct chan_info *ch = NULL;
    uint8_t opclass = 0;
    uint8_t channel = 0;
    uint8_t scan_status = 0;
    char *timestamp = NULL;

    blobmsg_parse(chan_scan_report_policy, NUM_CHAN_SCAN_REPORT_ATTR_ATTRS, tb, blobmsg_data(chan_scan), blobmsg_len(chan_scan));
    if (!tb[CHAN_SCAN_REPORT_ATTR_RADIO_UID]
        || !tb[CHAN_SCAN_REPORT_ATTR_OPCLASS]
        || !tb[CHAN_SCAN_REPORT_ATTR_CHANNEL]
        || !tb[CHAN_SCAN_REPORT_ATTR_SCAN_STATUS]) {
        DEBUG_WARNING("Parameters radio_uid, opclass, channel, scan_status should be included\n");
        return NULL;
    }
    blobmsg_get_mac(tb[CHAN_SCAN_REPORT_ATTR_RADIO_UID], radio_id);
    r = radioFind(d, radio_id);
    if (!r) {
        DEBUG_WARNING("Can NOT find radio["MACFMT"]\n", MACARG(radio_id));
        r = (struct radio *)calloc(1, sizeof(struct radio));
        if (!r)
        {
            DEBUG_WARNING("NO Space for chan scan report\n");
            return NULL;
        }
        memcpy(r->uid, radio_id, sizeof(mac_address));
        dlist_add_head(&d->radios, &r->l);
    }
    opclass = blobmsg_get_u32(tb[CHAN_SCAN_REPORT_ATTR_OPCLASS]);
    opc = opclassFind(r, opclass);
    if (!opc) {
        DEBUG_WARNING("Can NOT find opclass(%d) for radio["MACFMT"], use opclass[%d]\n", opclass,
            MACARG(r->uid), r->num_opc_support);
        opc = &r->opclasses[r->num_opc_support];
        r->num_opc_support++;
    }
    opc->op_class = opclass;
    channel = blobmsg_get_u32(tb[CHAN_SCAN_REPORT_ATTR_CHANNEL]);
    ch = channelFind(opc, channel);
    if (!ch) {
        DEBUG_WARNING("can NOT find ch[%d] in opc[%d] for radio["MACFMT"], use channels[%d]\n", channel,
            opclass, MACARG(r->uid), opc->num_support_chan);
        ch = &opc->channels[opc->num_support_chan];
        opc->num_support_chan++;
    }
    ch->id = channel;
    scan_status = blobmsg_get_u32(tb[CHAN_SCAN_REPORT_ATTR_SCAN_STATUS]);

    if (!scan_status) {
        if (!tb[CHAN_SCAN_REPORT_ATTR_TIMESTAMP]
            || !tb[CHAN_SCAN_REPORT_ATTR_UTILIZATION]
            || !tb[CHAN_SCAN_REPORT_ATTR_NOISE]
            || !tb[CHAN_SCAN_REPORT_ATTR_SCAN_DURATION]
            || !tb[CHAN_SCAN_REPORT_ATTR_SCAN_TYPE]) {
            DEBUG_WARNING("Parameters timestamp, utilization, noise, scan_dur, scan_type should be included\n");
            return NULL;
        }
        timestamp = blobmsg_get_string(tb[CHAN_SCAN_REPORT_ATTR_TIMESTAMP]);
        PLATFORM_GET_TIMESTAMP_TIMEVAL(timestamp, &ch->start_scan_ts);
        ch->utilization = blobmsg_get_u32(tb[CHAN_SCAN_REPORT_ATTR_UTILIZATION]);
        ch->avg_noise = blobmsg_get_u32(tb[CHAN_SCAN_REPORT_ATTR_NOISE]);
        if (tb[CHAN_SCAN_REPORT_ATTR_NEIGHBOR_LIST]) {
            parseChanScanNeighbors(tb[CHAN_SCAN_REPORT_ATTR_NEIGHBOR_LIST], ch);
        }
        ch->chscan_dur= blobmsg_get_u32(tb[CHAN_SCAN_REPORT_ATTR_SCAN_DURATION]);
        ch->chscan_type = blobmsg_get_u32(tb[CHAN_SCAN_REPORT_ATTR_SCAN_TYPE]);
    }
    req = chscanReqNew(r, d->recv_intf_idx, d->al_mac);
    if (req)
        req->status = scan_status;
    item = chscanReqItemNew(req, opclass, 1, &channel);
    item->status = scan_status;
    return req;
}
#endif

int parseVBSSRequestMsg(struct blob_attr *vbss_attrs, struct bss_info *bss, struct vbss_client_context_info *client_context,
                    uint8_t *desctruction_flag, uint8_t *client_assoc, uint8_t *disassociate_client) {
    struct blob_attr *tb[NUM_VBSS_ATTR_ATTRS];
    mac_address bssid = {0};
    mac_address client_mac = {0};

    blobmsg_parse(vbss_policy, NUM_VBSS_ATTR_ATTRS, tb, blobmsg_data(vbss_attrs), blobmsg_len(vbss_attrs));
    if (!tb[VBSS_ATTR_BSSID]) {
        DEBUG_WARNING("Parameters bssid should be included\n");
        return -1;
    }

    blobmsg_get_mac(tb[VBSS_ATTR_BSSID], bssid);
    MACCPY(bss->bssid, bssid);

    if (tb[VBSS_ATTR_CLIENT_MAC]) {
        blobmsg_get_mac(tb[VBSS_ATTR_CLIENT_MAC], client_mac);
        MACCPY(client_context->client_mac, client_mac);
        if (tb[VBSS_ATTR_SSID]) {
            bss->ssid.len = strlen(blobmsg_get_string(tb[VBSS_ATTR_SSID]));
            memcpy(bss->ssid.ssid, blobmsg_get_string(tb[VBSS_ATTR_SSID]), bss->ssid.len);
        }
        if (tb[VBSS_ATTR_PASSWORD]) {
            client_context->password.len = strlen(blobmsg_get_string(tb[VBSS_ATTR_PASSWORD]));
            client_context->password.datap = (uint8_t *)calloc(1, client_context->password.len);
            memcpy(client_context->password.datap, blobmsg_get_string(tb[VBSS_ATTR_PASSWORD]), client_context->password.len);
        }
        if (tb[VBSS_ATTR_PTK]) {
            client_context->ptk.len = strlen(blobmsg_get_string(tb[VBSS_ATTR_PTK])) + 1;
            client_context->ptk.datap = (uint8_t *)calloc(1, client_context->ptk.len);
            memcpy(client_context->ptk.datap, blobmsg_get_string(tb[VBSS_ATTR_PTK]), client_context->ptk.len);
        }
        if (tb[VBSS_ATTR_GTK]) {
            client_context->gtk.len = strlen(blobmsg_get_string(tb[VBSS_ATTR_GTK])) + 1;
            client_context->gtk.datap = (uint8_t *)calloc(1, client_context->gtk.len);
            memcpy(client_context->gtk.datap, blobmsg_get_string(tb[VBSS_ATTR_GTK]), client_context->gtk.len);
        }
        if (tb[VBSS_ATTR_TX_PACKET_NUMBER])
            client_context->tx_packet_number = blobmsg_get_u64(tb[VBSS_ATTR_TX_PACKET_NUMBER]);
        if (tb[VBSS_ATTR_GROUP_TX_PACKET_NUMBER])
            client_context->group_tx_packet_number = blobmsg_get_u64(tb[VBSS_ATTR_GROUP_TX_PACKET_NUMBER]);
        if (tb[VBSS_ATTR_CLIENT_ASSOC])
            *client_assoc = blobmsg_get_u8(tb[VBSS_ATTR_CLIENT_ASSOC]);
    }
    else {
        *desctruction_flag = 1;
        if (tb[VBSS_ATTR_DISASSOC_CLIENT])
            *disassociate_client = blobmsg_get_u8(tb[VBSS_ATTR_DISASSOC_CLIENT]);
    }

    return 0;
}


DECLARE_FUNC_UBUS_METHOD(_getVersion)
{
    blob_buf_init(&b, 0);
    blobmsg_add_string(&b, "version", CLMESH_VERSION"-"CLMESH_COMMIT);
    return ubus_send_reply(ctx, req, b.head);
}

enum {
    ATTR_BTM_TARGET_BSSID = 0,
    ATTR_BTM_TARGET_OPCLASS,
    ATTR_BTM_TARGET_CHANNEL,

    ATTR_BTM_TARGET_MAX,
};


DECLARE_FUNC_UBUS_METHOD(_send)
{
    struct blob_attr *tb[ATTR_MAX];
    struct ether_addr macaddr;
    const char *msg_type;
    struct al_device *d;
//    uint8_t bc_addr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    blobmsg_parse(_cli_attr_policy, ATTR_MAX, tb, blob_data(msg), blob_len(msg));
    blob_buf_init(&b, 0);

    if ((!tb[ATTR_MSG_TYPE]) || (!tb[ATTR_DEV])) {
        goto invalid;
    }

    if (!ether_aton_r(blobmsg_get_string(tb[ATTR_DEV]), &macaddr)) {
        goto invalid;
    }
    d = alDeviceFindAny((uint8_t *)&macaddr);

    msg_type = blobmsg_get_string(tb[ATTR_MSG_TYPE]);
    if (!strcmp(msg_type, "APCapabilityQuery")) {
        struct al_device *d = alDeviceFindAny((uint8_t *)&macaddr);
        if (d)
            sendAPCapabilityQuery(idx2InterfaceName(d->recv_intf_idx), getNextMid(), d->al_mac);
    } else if (!strcmp(msg_type, "TopologyQuery")) {
        if (d)
            sendTopologyQuery(idx2InterfaceName(d->recv_intf_idx), getNextMid(), d->al_mac);
    } else if (!strcmp(msg_type, "MapPolicyConfigRequest")) {
        if (d)
            sendMapPolicyConfigRequest(idx2InterfaceName(d->recv_intf_idx), getNextMid(), d->al_mac, &local_policy);
    } else if (!strcmp(msg_type, "ChanPrefQuery")) {
            if (d)
                sendChannelPreferenceQuery(idx2InterfaceName(d->recv_intf_idx), getNextMid(), d->al_mac);
    } else if (!strcmp(msg_type, "ClientCapaQuery")) {
        if (d) {
            uint8_t vap[6] = {0};
            uint8_t sta[6] = {0};
            blobmsg_get_mac(tb[ATTR_BSS], vap);
            blobmsg_get_mac(tb[ATTR_CLIENT], sta);
            sendClientCapabilityQuery(idx2InterfaceName(d->recv_intf_idx), getNextMid(), d->al_mac, vap, sta);
        }
    } else if (!strcmp(msg_type, "ChannelSelReq")) {
        if (d)
            sendChannelSelectionRequest(idx2InterfaceName(d->recv_intf_idx), getNextMid(), d->al_mac, d);
    } else if (!strcmp(msg_type, "vbssCapabilityReq")) {
        if (d)
            sendVBSSCapabilitiesRequest(idx2InterfaceName(d->recv_intf_idx), getNextMid(), d->al_mac);
    } else if (!strcmp(msg_type, "vbssReq")) {
        mac_address ruid;
        struct bss_info bss = {0};
        struct vbss_client_context_info client_context = {0};
        uint8_t disassociate_flag = 0;
        uint8_t client_assoc = 1;
        uint8_t disassociate_client = 1;
        if (!tb[ATTR_RADIO] || !tb[ATTR_VBSSINFO])
            goto invalid;

        blobmsg_get_mac(tb[ATTR_RADIO], ruid);
        if (parseVBSSRequestMsg(tb[ATTR_VBSSINFO], &bss, &client_context, &disassociate_flag, &client_assoc, &disassociate_client))
            goto invalid;

        if (d)
            sendVBSSCreationRequest(idx2InterfaceName(d->recv_intf_idx), getNextMid(), d->al_mac, ruid, &bss, &client_context,
                        NULL, NULL);

        if (client_context.password.datap)
            free(client_context.password.datap);
        if (client_context.ptk.datap)
            free(client_context.ptk.datap);
        if (client_context.gtk.datap)
            free(client_context.gtk.datap);
    }else if (!strcmp(msg_type, "ClientSecurityContextReq")) {
        mac_address bssid;
        mac_address sta_mac;
        if ((!tb[ATTR_BSS]) || (!tb[ATTR_STAMAC]))
            goto invalid;
        blobmsg_get_mac(tb[ATTR_BSS], bssid);
        blobmsg_get_mac(tb[ATTR_STAMAC], sta_mac);
        if (d)
            sendClientSecurityContextRequest(idx2InterfaceName(d->recv_intf_idx), getNextMid(), d->al_mac, bssid, sta_mac);
    } else if (!strcmp(msg_type, "TriggerCSAReq")) {
        mac_address ruid;
        mac_address bssid;
        mac_address sta_mac;
        if (!tb[ATTR_RADIO] || !tb[ATTR_BSS] || !tb[ATTR_STAMAC] || !tb[ATTR_CHANNEL] || !tb[ATTR_OPCLASS])
            goto invalid;
        blobmsg_get_mac(tb[ATTR_RADIO], ruid);
        blobmsg_get_mac(tb[ATTR_BSS], bssid);
        blobmsg_get_mac(tb[ATTR_STAMAC], sta_mac);
        if (d)
            sendTriggerCSARequest(idx2InterfaceName(d->recv_intf_idx), getNextMid(), d->al_mac, ruid, bssid, sta_mac,
                blobmsg_get_u32(tb[ATTR_CHANNEL]), blobmsg_get_u32(tb[ATTR_OPCLASS]));
    } else if (!strcmp(msg_type, "VbssMovePreparationReq")) {
        mac_address bssid;
        mac_address sta_mac;
        if (!tb[ATTR_BSS] || !tb[ATTR_STAMAC])
            goto invalid;
        blobmsg_get_mac(tb[ATTR_BSS], bssid);
        blobmsg_get_mac(tb[ATTR_STAMAC], sta_mac);
        if (d)
            sendVbssMovePreparationRequest(idx2InterfaceName(d->recv_intf_idx), getNextMid(), d->al_mac, bssid, sta_mac);
    } else if (!strcmp(msg_type, "VbssMoveCancelReq")) {
        mac_address bssid;
        mac_address sta_mac;
        if (!tb[ATTR_BSS] || !tb[ATTR_STAMAC])
            goto invalid;
        blobmsg_get_mac(tb[ATTR_BSS], bssid);
        blobmsg_get_mac(tb[ATTR_STAMAC], sta_mac);
        if (d)
            sendVbssMoveCancelRequest(idx2InterfaceName(d->recv_intf_idx), getNextMid(), d->al_mac, bssid, sta_mac);
    } else if (!strcmp(msg_type, "ChanScanRequest")) {
        if (d) {
            if (tb[ATTR_CHSCAN]) {
                uint8_t fresh_scan = 0;
                uint8_t radio_num = 0;
                struct chscan_req *reqs = NULL;
                uint8_t i = 0;

                radio_num = parseChanScanRequstMsg(d, tb[ATTR_CHSCAN], &fresh_scan, &reqs);
                if (!radio_num || !reqs) {
                    goto invalid;
                }
                sendChannelScanRequest(idx2InterfaceName(d->recv_intf_idx), getNextMid(), d->al_mac, fresh_scan, radio_num, reqs);
                for (i = 0; i < radio_num; i++) {
                    if (!dlist_empty(&reqs[i].h))
                        dlist_free_items(&reqs[i].h, struct chscan_req_item, l);
                }
                PLATFORM_FREE(reqs);
            }
        }
#if 0
    } else if (!strcmp(msg_type, "ChanScanReport")) {
        if (d) {
            if (tb[ATTR_CHSCAN]) {
                struct chscan_req *req = NULL;
                req = parseChanScanReportMsg(d, tb[ATTR_CHSCAN]);
                if (req) {
                    sendChannelScanReport(idx2InterfaceName(d->recv_intf_idx), getNextMid(), d->al_mac, req->r, req);
                }
            }
        }
#endif
    } else if (!strcmp(msg_type, "DeauthSTA")) {
        if (d) {
            if (tb[ATTR_STAMAC]) {
                struct client *sta = NULL;
                uint8_t sta_mac[6] = {0};
                blobmsg_get_mac(tb[ATTR_STAMAC], sta_mac);
                sta = clientFind(local_device, NULL, sta_mac);
                if (sta) {
                    doSteerByDeauth(sta, 12);
                    doStaDenyAdd(sta->wif, sta_mac);
                }
            }
        }
    } else if (!strcmp(msg_type, "DenySTADel")) {
        if (d) {
            if (tb[ATTR_STAMAC]) {
                uint8_t sta_mac[6] = {0};
                blobmsg_get_mac(tb[ATTR_STAMAC], sta_mac);
                struct wifi_interface *wif = (struct wifi_interface *)interfaceFindName(d, "wlan0");
                doStaDenyDel(wif, sta_mac);
            }
        }
    } else {
        goto invalid;
    }
    addResult(RESULT_OK);
    return ubus_send_reply(ctx, req, b.head);
invalid:
    addResult(RESULT_INVALID_ARGUMENT);
    return ubus_send_reply(ctx, req, b.head);
}

DECLARE_FUNC_UBUS_METHOD(_setChannel)
{
    struct blob_attr *tb[ATTR_MAX];
    struct ether_addr macaddr;
    struct radio *r;
    int opclass = -1, channel = -1, power = -1;

    blobmsg_parse(_cli_attr_policy, ATTR_MAX, tb, blob_data(msg), blob_len(msg));
    blob_buf_init(&b, 0);

    if (!tb[ATTR_RADIO])
        goto invalid;

    if ((!tb[ATTR_OPCLASS]) &&
        (!tb[ATTR_CHANNEL]) &&
        (!tb[ATTR_TXPOWER]))
        goto invalid;

    if (!ether_aton_r(blobmsg_get_string(tb[ATTR_RADIO]), &macaddr))
        goto invalid;

    if (!(r = radioFind(local_device, (uint8_t *)&macaddr)))
        goto invalid;

    if (tb[ATTR_OPCLASS])
        opclass = blobmsg_get_u32(tb[ATTR_OPCLASS]);
    if (tb[ATTR_CHANNEL])
        channel = blobmsg_get_u32(tb[ATTR_CHANNEL]);
    if (tb[ATTR_TXPOWER])
        power = blobmsg_get_u32(tb[ATTR_TXPOWER]);

    radioSetChannelTxPower(r, opclass, channel, power);

    addResult(RESULT_OK);
    return ubus_send_reply(ctx, req, b.head);
invalid:
    addResult(RESULT_INVALID_ARGUMENT);
    return ubus_send_reply(ctx, req, b.head);
}

static int _string2Binary(const char *bin_str, uint8_t **bins, uint32_t *len)
{
    uint32_t val, buf_len, i = 0;
    const char *p = bin_str;
    char *endpr;
    uint8_t *buf;

    if ((!bins) || (!len))
        return -1;

    *bins = NULL;
    *len = 0;

    buf_len = strlen(bin_str) / 3 + 1;
    buf = (uint8_t *)malloc(buf_len);
    while (p - bin_str < strlen(bin_str) && i < buf_len) {
        val = strtoul(p, &endpr, 16);
        if ((endpr - p) != 2 || val > 255) {
            free(buf);
            return -1;
        }
        buf[i++] = val;
        p = endpr + 1;
    }

    *bins = buf;
    *len = i;
    return 0;
}

DECLARE_FUNC_UBUS_METHOD(_sendFrame)
{
    struct blob_attr *tb[ATTR_MAX];
    struct ether_addr macaddr;
    struct wifi_interface *wif;
    uint8_t *frame = NULL;
    uint32_t frame_len = 0;

    blobmsg_parse(_cli_attr_policy, ATTR_MAX, tb, blob_data(msg), blob_len(msg));
    blob_buf_init(&b, 0);

    if ((!tb[ATTR_BSS]) ||
            (!tb[ATTR_FRAME])) {
        goto invalid;
    }

    if (!ether_aton_r(blobmsg_get_string(tb[ATTR_BSS]), &macaddr))
        goto invalid;

    wif = (struct wifi_interface *)interfaceFind(local_device,
            (uint8_t *)&macaddr, interface_type_wifi);
    if (!wif)
        goto invalid;

    if (_string2Binary(blobmsg_get_string(tb[ATTR_FRAME]), &frame, &frame_len) < 0)
        goto invalid;

    bssSendMgmtFrame(wif->i.index, frame, frame_len);
    free(frame);

    addResult(RESULT_OK);
    return ubus_send_reply(ctx, req, b.head);

invalid:
    addResult(RESULT_INVALID_ARGUMENT);
    return ubus_send_reply(ctx, req, b.head);
}

DECLARE_FUNC_UBUS_METHOD(_registerFrame)
{
    struct blob_attr *tb[ATTR_MAX];
    struct ether_addr macaddr;
    struct wifi_interface *wif;

    uint16_t frame_type;
    char *match = NULL;
    uint32_t match_len = 0;

    blobmsg_parse(_cli_attr_policy, ATTR_MAX, tb, blob_data(msg), blob_len(msg));
    blob_buf_init(&b, 0);

    if ((!tb[ATTR_BSS]) ||
            (!tb[ATTR_FRAME_TYPE])) {
        goto invalid;
    }

    if (!ether_aton_r(blobmsg_get_string(tb[ATTR_BSS]), &macaddr))
        goto invalid;

    wif = (struct wifi_interface *)interfaceFind(local_device,
            (uint8_t *)&macaddr, interface_type_wifi);
    if (!wif)
        goto invalid;

    frame_type = blobmsg_get_u32(tb[ATTR_FRAME_TYPE]);
    if (tb[ATTR_MATCH]) {
        match = blobmsg_get_string(tb[ATTR_MATCH]);
        match_len = strlen(match);
    }

    bssRegisterMgmtFrame(wif->i.index, frame_type, (uint8_t *)match, (uint16_t)match_len);

    addResult(RESULT_OK);
    return ubus_send_reply(ctx, req, b.head);

invalid:
    addResult(RESULT_INVALID_ARGUMENT);
    return ubus_send_reply(ctx, req, b.head);
}

DECLARE_FUNC_UBUS_METHOD(_getStaStats)
{
    struct blob_attr *tb[ATTR_MAX];
    struct ether_addr macaddr;
    struct wifi_interface *wif;

    blobmsg_parse(_cli_attr_policy, ATTR_MAX, tb, blob_data(msg), blob_len(msg));
    blob_buf_init(&b, 0);

    if (!tb[ATTR_BSS]) {
        goto invalid;
    }

    if (!ether_aton_r(blobmsg_get_string(tb[ATTR_BSS]), &macaddr))
        goto invalid;

    wif = (struct wifi_interface *)interfaceFind(local_device,
            (uint8_t *)&macaddr, interface_type_wifi);
    if (!wif)
        goto invalid;

    stationGetStats(wif->i.index, NULL);

    addResult(RESULT_OK);
    return ubus_send_reply(ctx, req, b.head);

invalid:
    addResult(RESULT_INVALID_ARGUMENT);
    return ubus_send_reply(ctx, req, b.head);
}

DECLARE_FUNC_UBUS_METHOD(_triggerChanScan)
{
    struct blob_attr *tb[ATTR_MAX];
    struct ether_addr macaddr;
    struct wifi_interface *wif;

    int i = 0;
    uint8_t opclass;
    uint8_t chan_num = 0;
    uint8_t op_chan_num = 0;
    uint8_t channels[32];
    uint8_t op_channels[32];

    blobmsg_parse(_cli_attr_policy, ATTR_MAX, tb, blob_data(msg), blob_len(msg));
    blob_buf_init(&b, 0);

    if (!tb[ATTR_BSS] ||
        !tb[ATTR_OPCLASS]) {
        goto invalid;
    }

    if (!ether_aton_r(blobmsg_get_string(tb[ATTR_BSS]), &macaddr))
        goto invalid;

    wif = (struct wifi_interface *)interfaceFind(local_device,
            (uint8_t *)&macaddr, interface_type_wifi);
    if (!wif)
        goto invalid;

    opclass = blobmsg_get_u32(tb[ATTR_OPCLASS]);
    if (wif->radio) {
        struct operating_class *c = opclassFind(wif->radio, opclass);
        if (!c)
            goto invalid;
        for (i = 0; i < c->num_support_chan; i++)
            op_channels[op_chan_num++] = c->channels[i].id;
    }

    if (tb[ATTR_CHANNELS]) {
        char *ch_list, *hit;

        ch_list = blobmsg_get_string(tb[ATTR_CHANNELS]);
        hit = strtok(ch_list, ",");
        while(hit)
        {
            for (i = 0; i < op_chan_num; i++) {
                if (op_channels[i] == atoi(hit))
                    channels[chan_num++] = atoi(hit);
            }
            hit = strtok(NULL, ",");
            if (chan_num >= 31)
                break;
        }
        if (chan_num == 0)
            goto invalid;
        bssTriggerChannelScan(wif->i.index, opclass, channels, chan_num);
    }
    else {
        bssTriggerChannelScan(wif->i.index, opclass, op_channels, op_chan_num);
    }

    addResult(RESULT_OK);
    return ubus_send_reply(ctx, req, b.head);

invalid:
    addResult(RESULT_INVALID_ARGUMENT);
    return ubus_send_reply(ctx, req, b.head);
}

DECLARE_FUNC_UBUS_METHOD(_setLog)
{
    struct blob_attr *tb[ATTR_MAX];

    blobmsg_parse(_cli_attr_policy, ATTR_MAX, tb, blob_data(msg), blob_len(msg));
    blob_buf_init(&b, 0);

    if (!tb[ATTR_SYSLOG_SWITCH] && !tb[ATTR_LOG_LEVEL]) {
        goto invalid;
    }

    if (tb[ATTR_LOG_LEVEL])
        DEBUG_SET(debug_param_level, blobmsg_get_u32(tb[ATTR_LOG_LEVEL]));

    if (tb[ATTR_SYSLOG_SWITCH])
        DEBUG_SET(debug_param_syslog, blobmsg_get_u32(tb[ATTR_SYSLOG_SWITCH]));

    addResult(RESULT_OK);
    return ubus_send_reply(ctx, req, b.head);

invalid:
    addResult(RESULT_INVALID_ARGUMENT);
    return ubus_send_reply(ctx, req, b.head);
}

DECLARE_FUNC_UBUS_METHOD(_setCtrler)
{
    struct blob_attr *tb[ATTR_MAX];
    struct ether_addr macaddr;
    struct al_device *d;

    blobmsg_parse(_cli_attr_policy, ATTR_MAX, tb, blob_data(msg), blob_len(msg));
    blob_buf_init(&b, 0);

    if ((!tb[ATTR_DEV]) || (!tb[ATTR_INTF])) {
        goto invalid;
    }

    if (!ether_aton_r(blobmsg_get_string(tb[ATTR_DEV]), &macaddr)) {
        goto invalid;
    }


    if (!(d = calloc(1, sizeof(struct al_device))))
        goto invalid;

    MACCPY(d->al_mac, &macaddr);
    d->recv_intf_idx = getInterfaceIndex(blobmsg_get_string(tb[ATTR_INTF]));
    setRegistrar(d);
    local_device->configured = 1;

addResult(RESULT_OK);
    return ubus_send_reply(ctx, req, b.head);
invalid:
    addResult(RESULT_INVALID_ARGUMENT);
    return ubus_send_reply(ctx, req, b.head);
}

DECLARE_FUNC_UBUS_METHOD(_addNac)
{
    struct blob_attr *tb[ATTR_MAX];
    uint8_t macaddr[MACLEN];

    blobmsg_parse(_cli_attr_policy, ATTR_MAX, tb, blob_data(msg), blob_len(msg));
    blob_buf_init(&b, 0);

    if (!tb[ATTR_CLIENT]) {
        goto invalid;
    }

    blobmsg_get_mac(tb[ATTR_CLIENT], macaddr);

    addNacMonitorSta(1, macaddr);

    addResult(RESULT_OK);
    return ubus_send_reply(ctx, req, b.head);

invalid:
    addResult(RESULT_INVALID_ARGUMENT);
    return ubus_send_reply(ctx, req, b.head);
}

DECLARE_FUNC_UBUS_METHOD(_delNac)
{
    struct blob_attr *tb[ATTR_MAX];
    uint8_t macaddr[MACLEN];

    blobmsg_parse(_cli_attr_policy, ATTR_MAX, tb, blob_data(msg), blob_len(msg));
    blob_buf_init(&b, 0);

    if (!tb[ATTR_CLIENT]) {
        goto invalid;
    }

    blobmsg_get_mac(tb[ATTR_CLIENT], macaddr);

    delNacMonitorSta(1, macaddr);

    addResult(RESULT_OK);
    return ubus_send_reply(ctx, req, b.head);

invalid:
    addResult(RESULT_INVALID_ARGUMENT);
    return ubus_send_reply(ctx, req, b.head);
}

DECLARE_FUNC_UBUS_METHOD(_getNacInfo)
{
    struct blob_attr *tb[ATTR_MAX];
    uint8_t macaddr[MACLEN];

    blobmsg_parse(_cli_attr_policy, ATTR_MAX, tb, blob_data(msg), blob_len(msg));
    blob_buf_init(&b, 0);

    if (!tb[ATTR_CLIENT]) {
        goto invalid;
    }

    blobmsg_get_mac(tb[ATTR_CLIENT], macaddr);

    nl80211GetNacStaInfo(1, macaddr);

    addResult(RESULT_OK);
    return ubus_send_reply(ctx, req, b.head);

invalid:
    addResult(RESULT_INVALID_ARGUMENT);
    return ubus_send_reply(ctx, req, b.head);
}

DECLARE_FUNC_UBUS_METHOD(_enableNac)
{
    struct blob_attr *tb[ATTR_MAX];

    blobmsg_parse(_cli_attr_policy, ATTR_MAX, tb, blob_data(msg), blob_len(msg));
    blob_buf_init(&b, 0);

    if (!tb[ATTR_ENABLE]) {
        goto invalid;
    }

    setNacMonitorEnable(1, blobmsg_get_u32(tb[ATTR_ENABLE]));

    addResult(RESULT_OK);
    return ubus_send_reply(ctx, req, b.head);

invalid:
    addResult(RESULT_INVALID_ARGUMENT);
    return ubus_send_reply(ctx, req, b.head);
}

DECLARE_FUNC_UBUS_METHOD(_getSurvey)
{
    struct blob_attr *tb[ATTR_MAX];
    struct ether_addr al_mac;
    struct al_device *d;
    struct radio *r;
    struct operating_class *opc;
    struct neighbor_bss *nei;
    void *result, *r_info, *c_info, *n_info, *ie_info;
    int i,k;

    blob_buf_init(&b, 0);
    blobmsg_parse(_cli_attr_policy, ATTR_MAX, tb, blob_data(msg), blob_len(msg));

    if (!tb[ATTR_DEV]) {
        addResult(RESULT_INVALID_ARGUMENT);
        goto bail;
    }
    if (!ether_aton_r(blobmsg_get_string(tb[ATTR_DEV]), &al_mac)) {
        addResult(RESULT_INVALID_ARGUMENT);
        goto bail;
    }
    if (local_device->is_agent && !local_device->is_controller) {
        addResult(RESULT_INVALID_ARGUMENT);
        goto bail;
    }
    d = alDeviceFind((uint8_t *)&al_mac);
    if (!d) {
        addResult(RESULT_INVALID_ARGUMENT);
        goto bail;
    }

    result= blobmsg_open_table(&b, "scan_result");
    r_info = blobmsg_open_table(&b, NULL);
    dlist_for_each(r, d->radios, l) {
        blobmsgAddMacStr(&b, "radio-uid", r->uid);
        for (i = 0; i < r->num_opc_support && i < MAX_OPCLASS; i++) {
            opc = &r->opclasses[i];
            for (k = 0; k < opc->num_support_chan && k < MAX_CHANNEL_PER_OPCLASS; k++) {
                DEBUG_WARNING("opc[%d]+ch[%d] scan_status=%d\n", opc->op_class, opc->channels[k].id, opc->channels[k].scan_status);
                if (0xff == opc->channels[k].scan_status) /* skip which scan status is initial */
                    continue;
                c_info = blobmsg_open_table(&b, "channel");
                blobmsg_add_u32(&b, "channel_id", opc->channels[k].id);
                blobmsg_add_string(&b, "scan_status", opc->channels[k].scan_status ? "fail" :"success");
                if (opc->channels[k].scan_status)
                    continue;
                blobmsg_add_string(&b, "scan_timestamp", PLATFORM_GET_TIMESTAMP_STR(&opc->channels[k].start_scan_ts));
                blobmsg_add_u32(&b, "channel_util", opc->channels[k].utilization);
                blobmsg_add_u32(&b, "agg_duration", opc->channels[k].chscan_dur);
                blobmsg_add_u32(&b, "noise", opc->channels[k].avg_noise);
                blobmsg_add_string(&b, "scan_type", opc->channels[k].chscan_type ? "activie" : "passive");
                if (dlist_empty(&opc->channels[k].neighbor_list))
                    blobmsg_add_u32(&b, "neighbor_num", 0);
                else {
                    blobmsg_add_u32(&b, "neighbor_num", dlist_count(&opc->channels[k].neighbor_list));
                    n_info = blobmsg_open_table(&b, "neighbor_info");
                    dlist_for_each(nei, opc->channels[k].neighbor_list, l) {
                        blobmsgAddMacStr(&b, "bssid", nei->bssid);
                        blobmsg_add_string(&b, "ssid", (char *)nei->ssid.ssid);
                        blobmsg_add_string(&b, "bw", bandwidth2String(nei->bw));
                        if (nei->bss_load_element_present) {
                            ie_info = blobmsg_open_table(&b, "bss_load_IE");
                            blobmsg_add_u32(&b, "bss_color", nei->bss_color);
                            blobmsg_add_u32(&b, "utilization", nei->chan_utilize);
                            blobmsg_add_u32(&b, "station_num", nei->sta_cnt);
                            blobmsg_close_table(&b, ie_info);
                        }
                    }
                    blobmsg_close_table(&b, n_info);
                }
                blobmsg_close_table(&b, c_info);
            }
        }
    }
    blobmsg_close_table(&b, r_info);
    blobmsg_close_table(&b, result);
    addResult(RESULT_OK);

bail:
    return ubus_send_reply(ctx, req, b.head);
}

struct local_scan_req {
    uint8_t radio_num;
    struct chscan_req *reqs;
};

struct local_scan_req local_req = {0};

void _clearLastScanReq(void)
{
    int i;

    if (local_req.reqs) {
        for (i = 0; i < local_req.radio_num; i++) {
            if (!dlist_empty(&local_req.reqs[i].h))
                dlist_free_items(&local_req.reqs[i].h, struct chscan_req_item, l);
        }
        PLATFORM_FREE(local_req.reqs);
    }
}

DECLARE_FUNC_UBUS_METHOD(_triggerSurvey)
{
    struct blob_attr *tb[ATTR_MAX];
    struct ether_addr al_mac;
    struct al_device *d;

    if (local_device->is_agent && !local_device->is_controller) {
        addResult(RESULT_INVALID_ARGUMENT);
        goto invalid;
    }

    blobmsg_parse(_cli_attr_policy, ATTR_MAX, tb, blob_data(msg), blob_len(msg));
    blob_buf_init(&b, 0);

    if (!ether_aton_r(blobmsg_get_string(tb[ATTR_DEV]), &al_mac)) {
        addResult(RESULT_INVALID_ARGUMENT);
        goto invalid;
    }

    d = alDeviceFind((uint8_t *)&al_mac);
    if (!d) {
        addResult(RESULT_INVALID_ARGUMENT);
        goto invalid;
    }

    if (tb[ATTR_CHSCAN]) {
        uint8_t fresh_scan = 0;
        uint8_t radio_num = 0;
        struct chscan_req *reqs = NULL;
        struct radio *r;
        uint8_t i = 0;

        if (0 == memcmp(local_device->al_mac, d->al_mac, MACLEN)) {
            _clearLastScanReq();

            local_req.radio_num = parseChanScanRequstMsg(local_device, tb[ATTR_CHSCAN], &fresh_scan, &local_req.reqs);
            if (!local_req.radio_num || !local_req.reqs)
                goto invalid;
            if (0 == fresh_scan)
                goto success;

            for (i = 0; i < local_req.radio_num; i++) {
                dlist_for_each(r, local_device->radios, l) {
                    if (0 == MACCMP(r->uid, local_req.reqs[i].r->uid)) {
                        r->chscan_req = &local_req.reqs[i];
                        startChannelScan(&local_req.reqs[i]);
                        break;
                    }
                }
            }
        } else {
            radio_num = parseChanScanRequstMsg(d, tb[ATTR_CHSCAN], &fresh_scan, &reqs);
            if (!radio_num || !reqs)
                goto invalid;
            sendChannelScanRequest(idx2InterfaceName(d->recv_intf_idx), getNextMid(), d->al_mac, fresh_scan, radio_num, reqs);
            for (i = 0; i < radio_num; i++) {
                if (!dlist_empty(&reqs[i].h))
                    dlist_free_items(&reqs[i].h, struct chscan_req_item, l);
            }
            PLATFORM_FREE(reqs);
        }
success:
        addResult(RESULT_OK);
        return ubus_send_reply(ctx, req, b.head);
    }

invalid:
    addResult(RESULT_INVALID_ARGUMENT);
    return ubus_send_reply(ctx, req, b.head);
}

static const struct ubus_method _cli_obj_methods[] = {
    UBUS_METHOD_NOARG("version", _getVersion),
    UBUS_METHOD("send", _send, _cli_attr_policy),
    UBUS_METHOD("set_channel", _setChannel, _cli_attr_policy),
    UBUS_METHOD("send_frame", _sendFrame, _cli_attr_policy),
    UBUS_METHOD("register_frame", _registerFrame, _cli_attr_policy),
    UBUS_METHOD("get_sta_stats", _getStaStats, _cli_attr_policy),
    UBUS_METHOD("chan_scan", _triggerChanScan, _cli_attr_policy),
    UBUS_METHOD("set_log", _setLog, _cli_attr_policy),
    UBUS_METHOD("set_controller", _setCtrler, _cli_attr_policy),
    UBUS_METHOD("add_nac", _addNac, _cli_attr_policy),
    UBUS_METHOD("del_nac", _delNac, _cli_attr_policy),
    UBUS_METHOD("get_nac_info", _getNacInfo, _cli_attr_policy),
    UBUS_METHOD("enable_nac", _enableNac, _cli_attr_policy),
    UBUS_METHOD("trigger_survey", _triggerSurvey, _cli_attr_policy),
    UBUS_METHOD("get_survey", _getSurvey, _cli_attr_policy),
};

static struct ubus_object_type _cli_obj_type =
    UBUS_OBJECT_TYPE(OBJ_NAME, _cli_obj_methods);

DECLARE_UBUS_OBJ(_cli_obj, OBJ_NAME, &_cli_obj_type, _cli_obj_methods);

struct ubus_object *getCliObj(void)
{
    return &_cli_obj;
}

