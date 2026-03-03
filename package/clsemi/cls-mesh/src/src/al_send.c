/*
 *  Broadband Forum IEEE 1905.1/1a stack
 *  
 *  Copyright (c) 2017, Broadband Forum
 *  
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *  
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  
 *  Subject to the terms and conditions of this license, each copyright
 *  holder and contributor hereby grants to those receiving rights under
 *  this license a perpetual, worldwide, non-exclusive, no-charge,
 *  royalty-free, irrevocable (except for failure to satisfy the
 *  conditions of this license) patent license to make, have made, use,
 *  offer to sell, sell, import, and otherwise transfer this software,
 *  where such license applies only to those patent claims, already
 *  acquired or hereafter acquired, licensable by such copyright holder or
 *  contributor that are necessarily infringed by:
 *  
 *  (a) their Contribution(s) (the licensed copyrights of copyright holders
 *      and non-copyrightable additions of contributors, in source or binary
 *      form) alone; or
 *  
 *  (b) combination of their Contribution(s) with the work of authorship to
 *      which such Contribution(s) was added by such copyright holder or
 *      contributor, if, at the time the Contribution is added, such addition
 *      causes such combination to be necessarily infringed. The patent
 *      license shall not apply to any other combinations which include the
 *      Contribution.
 *  
 *  Except as expressly stated above, no rights or licenses from any
 *  copyright holder or contributor is granted under this license, whether
 *  expressly, by implication, estoppel or otherwise.
 *  
 *  DISCLAIMER
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 *  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 *  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 *  PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 *  DAMAGE.
 */

#include "platform.h"
#include "utils.h"

#include <stdarg.h>   // va_list
  // NOTE: This is part of the C standard, thus *all* platforms should have it
  // available... and that's why this include can exist in this "platform
  // independent" file

#include "al_send.h"
#include "al_utils.h"
#include "al_cmdu.h"

#include "1905_tlvs.h"
#include "1905_cmdus.h"
#include "1905_l2.h"
#include "lldp_tlvs.h"
#include "lldp_payload.h"

#include "platform_os.h"
#include "platform_interfaces.h"
#include "driver/cls/nl80211_cls.h"


////////////////////////////////////////////////////////////////////////////////
// Private functions and data
////////////////////////////////////////////////////////////////////////////////

static struct TLV *_obtainDeviceInfoTLV(struct al_device *d)
{
    struct TLV *tlv = TLVNew(NULL, TLV_TYPE_DEVICE_INFORMATION_TYPE, 0);

    if (tlv) {
        struct macAddressTLV *mac_tlv = (struct macAddressTLV *)tlv;
        struct interface *i;

        MACCPY(mac_tlv->mac, d->al_mac);
        dlist_for_each(i, d->interfaces, l) {
            deviceInfoAddInterface(tlv, i);
        }
    }
    return tlv;
}

static struct TLV *_obtainIPv4TLV()
{
    struct TLV *tlv = NULL;
    struct macStruct *mac_s;
    struct interface *intf;

    dlist_for_each(intf, local_device->interfaces, l) {
        struct ipv4_item *ip;
        mac_s = NULL;
        dlist_for_each(ip, intf->ipv4s, l) {
            if (!tlv) tlv = TLVNew(NULL, TLV_TYPE_IPV4, 0);
            if (!mac_s) mac_s = ipv4AddInterface(tlv, intf->mac);
            ipv4InterfaceAddIP(mac_s, ip->proto, &ip->ip, &ip->dhcp_server);
        }
    }
    return tlv;
}

static struct TLV *_obtainIPv6TLV()
{
    struct TLV *tlv = NULL;
    struct interfaceipv6Struct *intf_ipv6_s;
    struct interface *intf;

    dlist_for_each(intf, local_device->interfaces, l) {
        struct ipv6_item *ip;
        int n=0;
        intf_ipv6_s = NULL;
        if (IPV6_IS_VALID(intf->local_ipv6.ip)) {
            if (!tlv) tlv = TLVNew(NULL, TLV_TYPE_IPV6, 0);
            intf_ipv6_s = ipv6AddInterface(tlv, intf->mac, &intf->local_ipv6);
        }
        dlist_for_each(ip, intf->ipv6s, l) {
            if (!tlv) tlv = TLVNew(NULL, TLV_TYPE_IPV6, 0);
            if (!intf_ipv6_s) intf_ipv6_s = ipv6AddInterface(tlv, intf->mac, NULL);
            ipv6InterfaceAddIP(intf_ipv6_s, ip->proto, &ip->ip, &ip->ip_origin);
        }
        //fix, wireshark report error if only ipv6 local address and no other
        if ((intf_ipv6_s) && (!n))
            ipv6InterfaceAddIP(intf_ipv6_s, IP_PROTO_AUTO, &intf->local_ipv6, &intf->local_ipv6);
    }
    return tlv;
}


static struct TLV *_obtainNon1905NeighborListTLV(struct interface *i)
{
    struct macAddressTLV *tlv = NULL;
    struct neighbor *n;

    dlist_for_each(n, i->neighbors, l) {
        if (!alDeviceFind(n->al_mac)) {
            if (!tlv) {
                tlv = (struct macAddressTLV *)TLVNew(NULL, TLV_TYPE_NON_1905_NEIGHBOR_DEVICE_LIST, 0);
                MACCPY(tlv->mac, i->mac);
            }
            non1905AddNeighbor((struct TLV *)tlv, n);
        }
    }
    return (struct TLV *)tlv;
}

static struct TLV *_obtaini1905NeighborListTLV(struct interface *i)
{
    struct macAddressTLV *tlv = NULL;
    struct neighbor *n;

    dlist_for_each(n, i->neighbors, l) {
        if (alDeviceFind(n->al_mac)) {
            if (!tlv) {
                tlv = (struct macAddressTLV *)TLVNew(NULL, TLV_TYPE_NEIGHBOR_DEVICE_LIST, 0);
                MACCPY(tlv->mac, i->mac);
            }
            i1905AddNeighbor((struct TLV *)tlv, n);
        }
    }
    return (struct TLV *)tlv;
}

static struct TLV *_obtainLinkMetricsQueryTLV(uint8_t *neighbor_al_mac, uint8_t link_metrics_type)
{
    struct linkMetricQueryTLV *tlv = NULL;

    tlv = (struct linkMetricQueryTLV *)TLVNew(NULL, TLV_TYPE_LINK_METRIC_QUERY, 0);
    if (neighbor_al_mac) {
        tlv->neighbor = LINK_METRIC_QUERY_TLV_SPECIFIC_NEIGHBOR;
        MACCPY(tlv->specific_neighbor, neighbor_al_mac);
    } else {
        tlv->neighbor = LINK_METRIC_QUERY_TLV_ALL_NEIGHBORS;
    }
    tlv->link_metrics_type = link_metrics_type;
    return (struct TLV *)tlv;
}

static struct TLV *_obtain1905TransmitterLinkMetricsTLV(struct link_item *item)
{
    struct transmitterLinkMetricTLV *tlv = NULL;
    struct transmitterLinkMetricEntriesStruct entry;
    struct link_pair_item *pair = NULL;
    struct neighbor *n = NULL;
    struct interface *i = NULL;

    tlv = (struct transmitterLinkMetricTLV *)TLVNew(NULL, TLV_TYPE_TRANSMITTER_LINK_METRIC, 0);
    MACCPY(tlv->local_al_address, item->local_al_mac);
    MACCPY(tlv->neighbor_al_address, item->neighbor_al_mac);

    dlist_for_each(pair, item->links, l) {
        i = interfaceFindMAC(local_device, pair->local_intf_mac);
        if (i) {
            struct linkMetrics *link_metric = PLATFORM_GET_LINK_METRICS(i->name, pair->neighbor_intf_mac);
            if (link_metric) {
                entry.tx_link_metrics = link_metric->tx_link_metrics;
                PLATFORM_FREE_LINK_METRICS(link_metric);
            } else
                continue;
        } else {
            i = interfaceFindMAC(NULL, pair->local_intf_mac);
            if (i) {
                n = neighborFind(i, item->neighbor_al_mac, pair->neighbor_intf_mac);
                if (n)
                    entry.tx_link_metrics = n->link_metric.tx_link_metrics;
                else
                    continue;
            } else {
                continue;
            }
        }
        MACCPY(entry.local_interface_address, i->mac);
        MACCPY(entry.neighbor_interface_address, pair->neighbor_intf_mac);
        entry.intf_type = i->type;
        entry.bridge_flag = 0;//FIXME todo: get bridge information
        trasmitterLinkMetricAddEntry(&tlv->tlv, &entry);
    }
    return (struct TLV *)tlv;
}

static struct TLV *_obtain1905ReceiverLinkMetricsTLV(struct link_item *item)
{
    struct receiverLinkMetricTLV *tlv = NULL;
    struct receiverLinkMetricEntriesStruct entry;
    struct link_pair_item *pair = NULL;
    struct neighbor *n = NULL;
    struct interface *i = NULL;

    tlv = (struct receiverLinkMetricTLV *)TLVNew(NULL, TLV_TYPE_RECEIVER_LINK_METRIC, 0);
    MACCPY(tlv->local_al_address, item->local_al_mac);
    MACCPY(tlv->neighbor_al_address, item->neighbor_al_mac);
    dlist_for_each(pair, item->links, l) {
        i = interfaceFindMAC(local_device, pair->local_intf_mac);
        if (i) {
            struct linkMetrics *link_metric = PLATFORM_GET_LINK_METRICS(i->name, pair->neighbor_intf_mac);
            if (link_metric) {
                entry.rx_link_metrics = link_metric->rx_link_metrics;
                PLATFORM_FREE_LINK_METRICS(link_metric);
            } else
                continue;

        } else {
            i = interfaceFindMAC(NULL, pair->local_intf_mac);
            if (i) {
                n = neighborFind(i, item->neighbor_al_mac, pair->neighbor_intf_mac);
                if (n)
                    entry.rx_link_metrics = n->link_metric.rx_link_metrics;
                else
                    continue;
            } else {
                continue;
            }
        }
        MACCPY(entry.local_interface_address, i->mac);
        MACCPY(entry.neighbor_interface_address, pair->neighbor_intf_mac);
        entry.intf_type = i->type;
        receiverLinkMetricAddEntry(&tlv->tlv, &entry);
    }
    return (struct TLV *)tlv;
}

static struct TLV *_obtainServiceTLV(uint8_t type, uint8_t controller, uint8_t agent)
{
    struct TLV *tlv = TLVNew(NULL, type, 0);

    if (tlv) {
        if (controller) {
            serviceAddService(tlv, e_controller);
        }
        if (agent) {
            serviceAddService(tlv, e_agent);
        }
    }
    return tlv;
}

static struct TLV *_obtainU8TLV(uint8_t type, uint8_t v1)
{
    struct u8TLV *tlv = (struct u8TLV *)TLVNew(NULL, type, 0);

    if (tlv) {
        tlv->v1 = v1;
    }
    return (struct TLV *)tlv;
}

static struct TLV *_obtainMACTLV(uint8_t type, uint8_t *mac)
{
    struct macAddressTLV *tlv =
            (struct macAddressTLV *)TLVNew(NULL, type, 0);
    if (tlv) {
        MACCPY(tlv->mac, mac);
    }
    return (struct TLV *)tlv;
}

static struct TLV *_obtainapRadioBasicCapabilitiesTLV(struct radio *r)
{
    struct radioBasicCapabilityTLV *tlv =
            (struct radioBasicCapabilityTLV *)TLVNew(NULL, TLV_TYPE_AP_RADIO_BASIC_CAPABILITIES, 0);
    int i,j;

    if (tlv) {
        MACCPY(tlv->rid, r->uid);
        tlv->max_bss = r->max_bss;

        for (i=0;i<r->num_opc_support;i++) {
            struct operating_class *opclass = &r->opclasses[i];
            uint8_t non_op_chans[MAX_CHANNEL_PER_OPCLASS];
            uint8_t num = 0;

            for (j=0;j<opclass->num_support_chan;j++) {
                struct chan_info *info = &opclass->channels[j];

                if (info->disable)
                    non_op_chans[num++] = info->id;
            }
            apRadioBasicCapaTLVAddOpClass(&tlv->tlv, opclass->op_class, opclass->max_tx_power, num, non_op_chans);
        }
    }
    return (struct TLV *)tlv;
}

static struct TLV *_obtainHTCapabilityTLV(struct radio *r)
{
    struct htCapabilityTLV *tlv =
            (struct htCapabilityTLV *)TLVNew(NULL, TLV_TYPE_AP_HT_CAPABILITIES, 0);
    if (tlv) {
        MACCPY(tlv->rid, r->uid);
        tlv->capa = r->bands_capa[r->current_band_idx].ht_capa;
    }
    return (struct TLV *)tlv;
}

static struct TLV *_obtainVHTCapabilityTLV(struct radio *r)
{
    struct vhtCapabilityTLV *tlv =
            (struct vhtCapabilityTLV *)TLVNew(NULL, TLV_TYPE_AP_VHT_CAPABILITIES, 0);
    if (tlv) {
        MACCPY(tlv->rid, r->uid);
        tlv->capa = r->bands_capa[r->current_band_idx].vht_capa;
    }
    return (struct TLV *)tlv;
}

static struct TLV *_obtainHECapabilityTLV(struct radio *r)
{
    struct heCapabilityTLV *tlv =
            (struct heCapabilityTLV *)TLVNew(NULL, TLV_TYPE_AP_HE_CAPABILITIES, 0);
    if (tlv) {
        MACCPY(tlv->rid, r->uid);
        tlv->capa = r->bands_capa[r->current_band_idx].he_capa;
    }
    return (struct TLV *)tlv;
}

static struct TLV *_obtainAPOperationalBSSTLV(struct al_device *d)
{
    struct TLV *tlv = TLVNew(NULL, TLV_TYPE_AP_OPERATIONAL_BSS, 0);
    struct radio *r;

    if (tlv) {
        dlist_for_each(r, d->radios, l) {
            deviceAddOperationalRadio(tlv, d, r);
        }
    }

    return tlv;
}

static struct TLV *_obtainMapProfileTLV(struct al_device *d)
{
    struct TLV *tlv = NULL;

    if (d->profile>=profile_1)
        tlv = _obtainU8TLV(TLV_TYPE_MULTIAP_PROFILE, d->profile);

    return tlv;
}

static struct TLV *_obtainVbssConfigurationReportTLV(struct al_device *d)
{
    struct TLV *tlv = subTLVNew(NULL, TLV_TYPE_VBSS, TLV_SUB_TYPE_VBSS_CONFIGURATION_REPORT, 0);;
    struct radio *r;

    if (tlv) {
        dlist_for_each(r, d->radios, l) {
            deviceAddVbssConfigurationReportRadio(tlv, d, r);
        }
    }

    return tlv;
}

static struct TLV *_obtainAssociatedClientsTLV(struct al_device *d)
{
    struct TLV *tlv = TLVNew(NULL, TLV_TYPE_ASSOCIATED_CLIENTS, 0);
    uint32_t current = PLATFORM_GET_TIMESTAMP(0);
    struct interface *intf;
    bool no_clients = true;

    dlist_for_each(intf, d->interfaces, l) {
        if (intf->type==interface_type_wifi) {
            if (_deviceAddBSSAssociated(tlv, (struct wifi_interface *)intf, current)
                 && (true == no_clients))
                no_clients = false;
        }
    }
    if (true == no_clients) {
        TLVFree(tlv);
        return NULL;
    }
    return tlv;
}

static struct TLV *_obtainCacCapabilityTLV(struct al_device *d)
{
    struct cacCapaTLV *capa_tlv;
    struct radio *r;
    struct macStruct *radio_s;
    struct cacTypeStruct *cactype_s;
    struct cacCapaOpclassStruct *opclass_s;
    uint8_t i,j;

    capa_tlv = (struct cacCapaTLV *)TLVNew(NULL, TLV_TYPE_CAC_CAPABILITIES, 0);

    if (!capa_tlv)
        return NULL;

    capa_tlv->cn_code = (uint16_t)((d->country_code[1] << 8) | (d->country_code[0]));

    dlist_for_each(r, d->radios, l) {
        uint8_t method;
        radio_s = NULL;

        for (method=CAC_METHOD_CONTINUOUS_CAC; method<CAC_METHOD_MAX; method++) {
            if (r->cac_capa[method].duration<0)
                continue;
            cactype_s = NULL;
            for (i=0;i<r->num_opc_support;i++) {
                struct operating_class *opclass = &r->opclasses[i];
                opclass_s = NULL;

                for (j=0;j<opclass->num_support_chan;j++) {
                    struct chan_info *info = &opclass->channels[j];

                    if (info->cac_method_mask & BIT(method)) {
                        if (!radio_s) {
                                radio_s = cacCapaTLVAddRadio(&capa_tlv->tlv, r->uid);
                        }
                        if (!cactype_s) {
                            cactype_s = cacCapaRadioAddType(radio_s, method, r->cac_capa[method].duration);
                        }
                        if (!opclass_s) {
                            opclass_s = cacCapaTypeAddOpclass(cactype_s, opclass->op_class);
                        }
                        opclass_s->chan[0]++;
                        opclass_s->chan[opclass_s->chan[0]] = info->id;
                    }
                }
            }
        }
    }
    return (struct TLV *)capa_tlv;
}

static struct TLV *_obtainProfile2ApCapabilityTLV(struct al_device *d)
{
    struct profile2ApCapabilityTLV *tlv =
            (struct profile2ApCapabilityTLV *)TLVNew(NULL, TLV_TYPE_PROFILE2_AP_CAPABILITY, 0);
    if (tlv) {
        tlv->cap = d->count_units<<P2_CAP_BYTE_COUNTER_UNIT_OFFSET;
        tlv->max_vid = d->max_vid;
        if (tlv->max_vid)
            tlv->cap |= P2_CAP_TRAFFIC_SEPARATION;
    }
    return (struct TLV *)tlv;
}

static struct TLV *_obtainSteeringPolicyTLV(struct dlist_head *local_disallow,
                                                    struct dlist_head *btm_disallow, struct dlist_head *policy)
{
    struct TLV *steer_policy;
    struct mac_item *item_mac;
    struct steer_policy_item *item_steer_policy;

    if (dlist_empty(local_disallow) && dlist_empty(btm_disallow) && dlist_empty(policy))
        return NULL;

    steer_policy = TLVNew(NULL, TLV_TYPE_STEERING_POLICY, 0);

    dlist_for_each(item_mac, *local_disallow, l) {
        TLVAddMac(&steer_policy->s.t.childs[0], item_mac->mac);
    }
    dlist_for_each(item_mac, *btm_disallow, l) {
        TLVAddMac(&steer_policy->s.t.childs[1], item_mac->mac);
    }
    dlist_for_each(item_steer_policy, *policy, l) {
        steeringPolicyTLVAddRadio(steer_policy, item_steer_policy->rid,
                                    item_steer_policy->params.agt_steer_mode,
                                    item_steer_policy->params.chan_util,
                                    item_steer_policy->params.rcpi_thresh);
    }
    return steer_policy;
}

static struct TLV *_obtainMetricsRptPolicyTLV(struct al_device *d, uint8_t interval)
{
    struct metricReportPolicyTLV *metrics_rpt =
                                    (struct metricReportPolicyTLV *)TLVNew(NULL, TLV_TYPE_METRIC_REPORTING_POLICY, 0);
    struct policy_param_metrics_rpt *rpt_policy = NULL;
    struct radio *r;

    metrics_rpt->interval = interval;
    if (d) {
        dlist_for_each(r, d->radios, l) {
            rpt_policy = findReportPolicy(r->current_band_idx);
            if (!rpt_policy)
                continue;
            metricReportPolicyTLVAddRadio(&metrics_rpt->tlv, r->uid,
                                            rpt_policy->sta_rcpi_thresh,
                                            rpt_policy->sta_rcpi_margin,
                                            rpt_policy->ap_chutil_thresh,
                                            rpt_policy->assoc_sta_inclusion_mode);
        }
    }
    return (struct TLV *)metrics_rpt;
}

static struct TLV *_obtainBackhaulBSSConfTLV(struct backhaul_bss_conf_item *bconf)
{
    struct backhaulBSSConfigTLV *tlv =
                                (struct backhaulBSSConfigTLV *)TLVNew(NULL, TLV_TYPE_BACKHAUL_BSS_CONFIGURATION, 0);

    MACCPY(tlv->bssid, bconf->bssid);
    tlv->config = bconf->config;
    return (struct TLV *)tlv;
}

static struct TLV *_obtainChanPrefTLV(struct radio *r)
{
    int i, j;
    struct macAddressTLV *ch_pref = (struct macAddressTLV *)TLVNew(NULL, TLV_TYPE_CHANNEL_PREFERENCE, 0);
    uint8_t num, pref, reason;
    uint8_t channel_set[MAX_CHANNEL_PER_OPCLASS];

    MACCPY(ch_pref->mac, r->uid);
    for (i = 0; i < r->num_opc_support; i++) {
        struct operating_class *opclass = &r->opclasses[i];
        num = 0;
        pref = 0;
        reason = 0;
        for (j = 0; j < opclass->num_support_chan; j++) {
            struct chan_info *info = &r->opclasses[i].channels[j];
            if (info->pref == MOST_PREF_SCORE) //skip best channels
                continue;
            if ((!info->pref) && (!info->reason)) //skip permanently non-operable channel
                continue;

            if ((info->pref!=pref) || (info->reason!=reason)) {
                if (num) {
                    chanPrefTLVAddOpclass(&ch_pref->tlv, opclass->op_class, num, channel_set, pref, reason);
                }
                num = 0;
                reason = info->reason;
                pref = info->pref;
            }
            channel_set[num++] = info->id;
        }
        if (num) {
            chanPrefTLVAddOpclass(&ch_pref->tlv, opclass->op_class, num, channel_set, pref, reason);
        }

    }

    return (struct TLV *)ch_pref;
}

static struct TLV *_obtainOpRestTLV(struct radio *r)
{
    int i, k;
    struct opRestOpclassStruct *op_rest_opc;
    struct macAddressTLV *op_rest = NULL;

    for (i = 0; i < r->num_opc_support; i++) {
        op_rest_opc = NULL;
        for (k = 0; k < r->opclasses[i].num_support_chan; k++) {
            if (r->opclasses[i].channels[k].freq_separation) {
                if (!op_rest) {
                    op_rest = (struct macAddressTLV *)
                                TLVNew(NULL, TLV_TYPE_RADIO_OPERATION_RESTRICTION, 0);
                    MACCPY(op_rest->mac, r->uid);
                }
                if (!op_rest_opc) {
                    op_rest_opc = operRestTLVAddOpclass(&op_rest->tlv, r->opclasses[i].op_class);
                }
                operRestOpcAddChan(op_rest_opc, r->opclasses[i].channels[k].id,
                                    r->opclasses[i].channels[k].freq_separation);
            }
        }
    }
    return (struct TLV *)op_rest;
}

static struct TLV *_obtainClientAssocControlReqTLV(uint8_t *req_bssid, uint8_t assoc_ctrl, uint16_t period, struct dlist_head *sta_list)
{
    struct clientAssocCtrlReqTLV *tlv = (struct clientAssocCtrlReqTLV *)TLVNew(NULL, TLV_TYPE_CLIENT_ASSOCIATION_CONTROL_REQUEST, 0);
    struct mac_item *item_mac;

    MACCPY(tlv->bssid, req_bssid);
    tlv->ctrl = assoc_ctrl;
    tlv->valid_period = period;

    if (sta_list) {
        dlist_for_each(item_mac, *sta_list, l) {
            clientAssocCtrlReqAddSta(&tlv->tlv, item_mac->mac);
        }
    }
    return (struct TLV *)tlv;
}

static struct TLV *_obtainChanScanReqTLV(struct al_device *d, uint8_t fresh_scan, uint8_t num, struct chscan_req *reqs)
{
    struct scanReqTLV *tlv = (struct scanReqTLV *)TLVNew(NULL, TLV_TYPE_CHANNEL_SCAN_REQUEST, 0);
    struct macStruct *rs = NULL;
    struct chscan_req_item *item = NULL;
    int i = 0;

    if (tlv) {
        if (fresh_scan)
            tlv->fresh_scan = FRESH_SCAN_RESULT;
        else
            tlv->fresh_scan = 0;
        for (i = 0; i < num; i++) {
            rs = scanReqTLVAddRadios(&tlv->tlv, reqs[i].r->uid);
            if (fresh_scan) {
                dlist_for_each(item, reqs[i].h, l) {
                    if(!item->ch_num) {
                        scanReqRadioAddOpclassChan(rs, item->opclass, 0, NULL);
                    } else {
                        scanReqRadioAddOpclassChan(rs, item->opclass, item->ch_num, &item->chans[0]);
                    }
                }
            }
        }
    }
    return (struct TLV *)tlv;
}

static struct TLV *_obtainTimeStampTLV(uint32_t ts)
{
    struct timeStampTLV *tlv = (struct timeStampTLV *)TLVNew(NULL, TLV_TYPE_TIMESTAMP, 0);
    uint32_t len = 0;
    char *timestamp = NULL;
    struct timeval tv;

    if (ts == 0) {
        timestamp = PLATFORM_GET_TIMESTAMP_STR(NULL);
    } else {
        tv.tv_sec = ts;
        tv.tv_usec = 0;
        timestamp = PLATFORM_GET_TIMESTAMP_STR(&tv);
    }
    if ((timestamp = PLATFORM_GET_TIMESTAMP_STR(NULL))) {
        len = PLATFORM_STRLEN(timestamp);
        tlv->timeStamp.len = len;
        PLATFORM_MEMCPY(tlv->timeStamp.data, timestamp, len);
        return (struct TLV *)tlv;
    } else {
        TLVFree((struct TLV *)tlv);
        return NULL;
    }
}

static struct TLV *_obtainScanResultTLV(mac_address ruid,
                    uint8_t opclass, struct chan_info *channel, uint8_t status)
{
    struct scanResultTLV *tlv = NULL;
    struct scanResultStruct *result = NULL;
    char *timestamp = NULL;
    struct neighbor_bss *neighbor = NULL;
    uint8_t ts_len = 0;

    tlv = (struct scanResultTLV *)TLVNew(NULL, TLV_TYPE_CHANNEL_SCAN_RESULT, 0);
    PLATFORM_MEMCPY(tlv->ruid, ruid, MACLEN);
    tlv->opclass = opclass;
    tlv->channel = channel->id;
    tlv->scan_status = status;
    if (status == SCAN_STATUS_SUCCESS) {
        timestamp = PLATFORM_GET_TIMESTAMP_STR(&channel->start_scan_ts);
        if (!timestamp) {
            DEBUG_WARNING("Internal error.\n");
            TLVFree((struct TLV *)tlv);
            return NULL;
        }
        ts_len = strlen(timestamp);
        result = scanResultTLVAddResult(&tlv->tlv, ts_len, (uint8_t *)timestamp, channel->utilization,
                                        channel->avg_noise, channel->chscan_dur, channel->chscan_type);
        dlist_for_each(neighbor, channel->neighbor_list, l) {
            scanResultAddNeighbors(result, neighbor);
        }
    }
    return (struct TLV *)tlv;
}

static struct TLV *_obtainCacRequestTLV(uint8_t num_reqs, struct cac_request *reqs)
{
    struct TLV *tlv = NULL;
    struct cac_request *tmp = NULL;
    struct radio *r = NULL;
    uint8_t i = 0;

    tlv = TLVNew(NULL, TLV_TYPE_CAC_REQUEST, 0);
    for (i = 0; i < num_reqs; i++) {
        tmp = &reqs[i];
        if (tmp->method > CAC_METHOD_TIME_SLICED_CAC) {
            DEBUG_WARNING("Wrong CAC method[%d]!\n", tmp->method);
            continue;
        }
        if (tmp->cmpl_action != CAC_ACTION_REMAIN_ON_CHAN
            && tmp->cmpl_action != CAC_ACTION_RETURN_RADIO) {
            DEBUG_WARNING("Wrong CAC action[%d]!\n", tmp->cmpl_action);
            continue;
        }
        r = radioFind(NULL, tmp->ruid);
        if (!r) {
            DEBUG_WARNING("Can NOT find radio["MACFMT"]\n", MACARG(tmp->ruid));
            continue;
        }
        cacReqTLVAddRadios(tlv, tmp->ruid, tmp->opclass, tmp->channel, tmp->method, tmp->cmpl_action);
    }
    return tlv;
}

static struct TLV *_obtainCacTerminationTLV(uint8_t num_terms, struct cac_termination *terms)
{
    struct TLV *tlv = NULL;
    struct cac_termination *tmp = NULL;
    struct radio *r = NULL;
    uint8_t i = 0;

    tlv = TLVNew(NULL, TLV_TYPE_CAC_TERMINATION, 0);
    for (i = 0; i < num_terms; i++) {
        tmp = &terms[i];
        r = radioFind(NULL, tmp->ruid);
        if (!r) {
            DEBUG_WARNING("Can NOT find radio["MACFMT"]\n", MACARG(tmp->ruid));
            continue;
        }
        cacTermTLVAddRadios(tlv, tmp->ruid, tmp->opclass, tmp->channel);
    }
    return tlv;
}

static struct TLV *_obtainCacCompletionReportTLV(struct radio *r,
    uint8_t opclass, uint8_t channel, uint8_t cac_status)
{
    struct TLV *tlv = NULL;
    struct cacRadioStruct *radio_s = NULL;
    int i = 0;
    int j = 0;

    tlv = TLVNew(NULL, TLV_TYPE_CAC_COMPLETION_REPORT, 0);
    radio_s = cacReptTLVAddRadios(tlv, r->uid, opclass, channel, cac_status);
    if (cac_status == CAC_CMPLT_STATUS_RADAR_DETECTED) {
        for (i = 0; i < r->num_opc_support; i++) {
            for (j = 0; j < r->opclasses[i].num_support_chan; j++) {
                if (r->opclasses[i].channels[j].radar_detected) {
                    cacReptRadioAddOpclsChan(radio_s, r->opclasses[i].op_class, r->opclasses[i].channels[j].id);
                }
            }
        }
    }
    return tlv;
}

static struct TLV *_obtainCacStatusReportTLV()
{
    struct TLV *tlv = NULL;
    struct radio *r = NULL;
    uint32_t minutes = 0;
    uint32_t cur_ts = 0;
    uint32_t unoccup = 0;
    uint32_t unoccup_dur = 0;
    uint32_t cac_countdown = 0;
    uint32_t cac_period = 0;
    uint8_t i = 0;
    uint8_t j = 0;

    tlv = TLVNew(NULL, TLV_TYPE_CAC_STATUS_REPORT, 0);
    cur_ts = PLATFORM_GET_TIMESTAMP(0);

    dlist_for_each(r, local_device->radios, l) {
        if (r->bands == band_2g)
            continue;
        for (i = 0; i < r->num_opc_support; i++) {
            for (j = 0; j < r->opclasses[i].num_support_chan; j++) {
                if ((r->opclasses[i].channels[j].cac_status == CAC_STATUS_AVAIL)
                    || (r->opclasses[i].channels[j].cac_status == CAC_STATUS_NON_DFS)) {
                    if (r->opclasses[i].channels[j].cac_status == CAC_STATUS_AVAIL)
                        minutes = (cur_ts - r->opclasses[i].channels[j].ts_cacstat_changed)/60000; /* minutes */
                    else /* Multi-AP Spec V2: Chap 17.2.45---availabel channel: minutes set to zero for non-DFS channel */
                        minutes = 0;
                    cacStatTLVAddAvailChan(tlv, r->opclasses[i].op_class, r->opclasses[i].channels[j].id, (uint16_t)minutes);
                }
            }
        }
    }
    dlist_for_each(r, local_device->radios, l) {
        if (r->bands == band_2g)
            continue;
        for (i = 0; i < r->num_opc_support; i++) {
            for (j = 0; j < r->opclasses[i].num_support_chan; j++) {
                if (r->opclasses[i].channels[j].cac_status == CAC_STATUS_NON_OCCUP) {
                    if (getPeriodForChannel(r->opclasses[i].channels[j].id, r->opclasses[i].op_class, r, &cac_period, &unoccup)) {
                        unoccup_dur = unoccup - (cur_ts - r->opclasses[i].channels[j].ts_cacstat_changed)/1000; /* seconds */
                        cacStatTLVAddUnoccupyChan(tlv, r->opclasses[i].op_class, r->opclasses[i].channels[j].id, (uint16_t)unoccup_dur);
                    }
                }
            }
        }
    }
    dlist_for_each(r, local_device->radios, l) {
        if (r->bands == band_2g)
            continue;
        if (r->cur_cac.cur_status == CAC_STATUS_ONGOING) {
            if (getPeriodForChannel(r->cur_cac.cur_scan.channel, r->cur_cac.cur_scan.opclass, r, &cac_period, &unoccup)) {
                cac_countdown = cac_period - (cur_ts - r->cur_cac.timestamp)/1000; /* seconds */
                cacStatTLVAddCacChan(tlv, r->cur_cac.cur_scan.opclass, r->cur_cac.cur_scan.channel, cac_countdown);
            }
        }
    }
    return (struct TLV *)tlv;
}

static struct TLV *_obtainBackhaulSteeringRequestTLV(uint8_t *bkhSta, uint8_t *targetBss, uint8_t targetOpclass, uint8_t targetCh)
{
    struct bkhSteerReqTLV *tlv = (struct bkhSteerReqTLV *)TLVNew(NULL, TLV_TYPE_BACKHAUL_STEERING_REQUEST, 0);

    if (tlv) {
        MACCPY(tlv->sta, bkhSta);
        MACCPY(tlv->target, targetBss);
        tlv->opclass = targetOpclass;
        tlv->channel = targetCh;
    }
    return (struct TLV *)tlv;
}

static struct TLV *_obtainBackhaulSteeringResponseTLV(struct wifi_interface *wif, uint8_t result)
{
    struct bkhSteerRspTLV *tlv = (struct bkhSteerRspTLV *)TLVNew(NULL, TLV_TYPE_BACKHAUL_STEERING_RESPONSE, 0);

    if (tlv) {
        MACCPY(tlv->sta, wif->i.mac);
        MACCPY(tlv->sta, wif->last_steering_target);
        tlv->result = result;
    }
    return (struct TLV *)tlv;
}

static struct TLV *_obtainErrorCodeTLV(uint8_t *mac, uint8_t code)
{
    struct errorCodeTLV *tlv = (struct errorCodeTLV *)TLVNew(NULL, TLV_TYPE_ERROR_CODE, 0);

    if (tlv) {
        MACCPY(tlv->sta, mac);
        tlv->code = code;
    }
    return (struct TLV *)tlv;
}

static struct TLV *_obtainBackhaulSTARadioCapaTLV(struct wifi_interface *wif)
{
    struct bSTARadioCapaTLV *tlv = (struct bSTARadioCapaTLV *)TLVNew(NULL, TLV_TYPE_BACKHAUL_STA_RADIO_CAPABILITIES, 0);

    MACCPY(tlv->rid, wif->radio->uid);
    tlv->capa = MAC_INCLUDED;

    bStaCapabilityAddMac(tlv, wif->i.mac);
    return (struct TLV *)tlv;
}

static struct TLV *_obtainCodeTLV(uint16_t type, int16_t code)
{
    struct codeTLV *tlv = (struct codeTLV *)TLVNew(NULL, type, 0);
    tlv->code = code;

    return (struct TLV *)tlv;
}

static struct TLV *_obtainClientInfoTLV(uint8_t *bssid, uint8_t *sta)
{
    struct clientInfoTLV *client = (struct clientInfoTLV *)TLVNew(NULL, TLV_TYPE_CLIENT_INFO, 0);

    MACCPY(client->bssid, bssid);
    MACCPY(client->client, sta);
    return (struct TLV *)client;
}

static struct TLV *_obtainClientCapRptTLV(struct client *client, uint8_t result)
{
    struct clientCapabilityReportTLV *cap_rpt;

    cap_rpt = (struct clientCapabilityReportTLV *)TLVNew(NULL, TLV_TYPE_CLIENT_CAPABILITY_REPORT, 0);

    if (client)
        cap_rpt->assc_req = client->last_assoc;
    cap_rpt->result = result;

    return (struct TLV *)cap_rpt;
}


static struct TLV *_obtainBeaconMetricsQueryTLV(uint8_t *sta_request, uint8_t opclass_request, uint8_t chan_request,
                                                    uint8_t *bssid_request, uint8_t detail, uint8_t len_ssid, char *ssid,
                                                        dlist_head *list_chan_report, uint8_t *eid)
{
    struct bcnMetricQueryTLV *bcn_met_qry = (struct bcnMetricQueryTLV *)TLVNew(NULL, TLV_TYPE_BEACON_METRICS_QUERY, 0);
    struct requested_chrpt_item *item_chan_rpt;
    uint8_t len_rpt = 0;

    MACCPY(bcn_met_qry->sta, sta_request);
    MACCPY(bcn_met_qry->bssid, bssid_request);
    bcn_met_qry->opclass = opclass_request;
    bcn_met_qry->channel = chan_request;
    bcn_met_qry->detail = detail;
    bcn_met_qry->ssid.len = len_ssid;
    memcpy(bcn_met_qry->ssid.ssid, ssid, len_ssid);
    bcn_met_qry->eids.len = eid[0];
    //memcpy(bcn_met_qry->eids.data, &eid[1], eid[0]);

    dlist_for_each(item_chan_rpt, *list_chan_report, l) {
        len_rpt = item_chan_rpt->num_chs + 1;
        bcnMetricsQryAddChRpt(&bcn_met_qry->tlv, len_rpt, item_chan_rpt->opclass, item_chan_rpt->num_chs,
                                item_chan_rpt->ch_list);
    }
    return (struct TLV *)bcn_met_qry;
}

static struct TLV *_obtainBeaconMetricsRspTLV(uint8_t *sta_mac, uint8_t *report, uint16_t len)
{
    struct bcnMetricRspTLV *bcn_met_rsp = (struct bcnMetricRspTLV *)TLVNew(NULL, TLV_TYPE_BEACON_METRICS_RESPONSE, 0);

    MACCPY(bcn_met_rsp->sta, sta_mac);

    while (len>=2) {
        uint8_t elem_len = report[1];
        if (len<elem_len+2)
            break;
        bcnMetricsRspAddElem(&bcn_met_rsp->tlv, report, elem_len+2);
        report += (elem_len+2);
        len -= (elem_len+2);
    }

    return (struct TLV *)bcn_met_rsp;
}

static struct TLV *_obtainTxPwrTLV(struct radio *r)
{
    struct txPwrLimitTLV *pwr = (struct txPwrLimitTLV *)TLVNew(NULL, TLV_TYPE_TRANSMIT_POWER_LIMIT, 0);

    MACCPY(pwr->rid, r->uid);
    pwr->tx_pwr = r->tx_power;

    return (struct TLV *)pwr;
}

static struct TLV *_obtainChanSelRspTLV(struct radio *r, uint8_t code)
{
    struct chanSelRspTLV *rsp = (struct chanSelRspTLV *)TLVNew(NULL, TLV_TYPE_CHANNEL_SELECTION_RESPONSE, 0);

    MACCPY(rsp->rid, r->uid);
    rsp->code = code;

    return (struct TLV *)rsp;
}

static struct TLV *_obtainOperatingChanRptTLV(struct radio *r)
{
    struct opChanReportTLV *rpt = (struct opChanReportTLV *)TLVNew(NULL, TLV_TYPE_OPERATING_CHANNEL_REPORT, 0);
    int i;

    MACCPY(rpt->rid, r->uid);
    for (i = 0; i < ARRAY_SIZE(r->current_opclass); i++) {
        if (!r->current_opclass[i]) {
            if (i==0)
                DEBUG_WARNING("radio["MACFMT"] channel/opclass not ready\n", MACARG(r->uid));
            break;
        }
        opChanReportLVAddPair(&rpt->tlv, r->current_opclass[i], r->current_channel[i]);
    }
    rpt->tx_power = r->tx_power;
    return (struct TLV *)rpt;
}

static struct TLV *_obtainApMetricsQryTLV(struct dlist_head *list)
{
    struct TLV *ap_metric = (struct TLV*)TLVNew(NULL, TLV_TYPE_AP_METRIC_QUERY, 0);
    struct mac_item *item;

    dlist_for_each(item, *list, l)
    {
        TLVAddMac(&ap_metric->s.t.childs[0], item->mac);
    }
    return ap_metric;
}

#define SET_ESPI_BITS(byte, ac, value) \
        byte[0] = value.ppdu_duration;   \
        byte[1] = value.airtime_fraction;   \
        SET_BITS(byte[2], ac, ESPI_ACCESS_CATEGORY_MASK, ESPI_ACCESS_CATEGORY_SHIFT); \
        SET_BITS(byte[2], value.data_fmt, ESPI_DATA_FORMAT_MASK, ESPI_DATA_FORMAT_SHIFT); \
        SET_BITS(byte[2], value.ba_win, ESPI_BA_WINDOW_MASK, ESPI_BA_WINDOW_SHIFT);

static struct TLV *_obtainApMetricsTLV(struct wifi_interface *wif)
{
    struct apMetricsTLV *metric = (struct apMetricsTLV*)TLVNew(NULL, TLV_TYPE_AP_METRICS, 0);

    MACCPY(metric->bssid, wif->i.mac);
    metric->chan_util = wif->radio->ch_util;
    metric->clients = dlist_count(&wif->clients);

    metric->includes |= ESPI_BE_INCLUDED;
    SET_ESPI_BITS(metric->espi_be, 1, wif->metrics.espi[0]);
    if (wif->metrics.espi[1].enabled) {
        metric->includes |= ESPI_BK_INCLUDED;
        SET_ESPI_BITS(metric->espi_bk, 0, wif->metrics.espi[1]);
    }
    if (wif->metrics.espi[2].enabled) {
        metric->includes |= ESPI_VO_INCLUDED;
        SET_ESPI_BITS(metric->espi_vo, 3, wif->metrics.espi[2]);
    }
    if (wif->metrics.espi[3].enabled) {
        metric->includes |= ESPI_VI_INCLUDED;
        SET_ESPI_BITS(metric->espi_vi, 2, wif->metrics.espi[3]);
    }
    return (struct TLV *)metric;
}

static struct TLV *_obtainAssocStaTrafStatTLV(struct client *sta_info)
{
    struct assocTrafficStatsTLV *traf_stats =
                        (struct assocTrafficStatsTLV*)TLVNew(NULL, TLV_TYPE_ASSOCIATED_STA_TRAFFIC_STATS, 0);

    MACCPY(traf_stats->sta, sta_info->mac);
    traf_stats->stats = sta_info->traffic_stats;
    return (struct TLV *)traf_stats;
}

static struct TLV *_obtainAssocStaLinkMetricsTLV(struct client *sta_info)
{
    struct macAddressTLV *link_metrics = (struct macAddressTLV*)TLVNew(NULL, TLV_TYPE_ASSOCIATED_STA_LINK_METRICS, 0);

    MACCPY(link_metrics->mac, sta_info->mac);
    assocLinkMetricsTLVAddBssid(&link_metrics->tlv, sta_info);

    return (struct TLV *)link_metrics;
}

static struct TLV *_obtainUnassocStaLinkMetricQryTLV(uint8_t opclass, dlist_head *channel_sta_list)
{

    struct unassoc_sta_metrics_query_per_chan_item *per_chan_i;
    struct mac_item *mac_i;
    struct unassocMetricQryStruct *per_chan_s;
    struct unassocStaLinkMetricsQryTLV *unassoc_linkmetric_qry =
        (struct unassocStaLinkMetricsQryTLV *)TLVNew(NULL, TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_QUERY, 0);

    unassoc_linkmetric_qry->opclass = opclass;
    dlist_for_each(per_chan_i, *channel_sta_list, l) {
        per_chan_s = unassocStaLinkQueryAddChan(&unassoc_linkmetric_qry->tlv, per_chan_i->chan);
        dlist_for_each(mac_i, per_chan_i->sta_list, l) {
            unassocStaLinkChanAddSta(per_chan_s, mac_i->mac);
        }
    }

    return (struct TLV *)unassoc_linkmetric_qry;
}

static struct TLV *_obtainUnassocStaLinkMetricRspTLV(dlist_head *channel_list, dlist_head *unassoc_list,
                                                                uint8_t opclass)
{
    struct unassocMetricQryStruct *channel_item;
    struct macStruct *item_mac;
    uint8_t ul_rcpi = 0;
    struct unassocStaLinkMetricsRspTLV *unassoc_linkmetric_rsp =
                (struct unassocStaLinkMetricsRspTLV *)TLVNew(NULL, TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_RESPONSE, 0);

    unassoc_linkmetric_rsp->opclass = opclass;

    dlist_for_each(channel_item, *channel_list, s.t.l) {
        dlist_for_each(item_mac, channel_item->s.t.childs[0], s.t.l) {
            ul_rcpi = nl80211GetNacStaInfo(channel_item->channel, item_mac->mac);
            unassocStaLinkRspAddSta(&unassoc_linkmetric_rsp->tlv, item_mac->mac, channel_item->channel,
                                            PLATFORM_GET_TIMESTAMP(0), ul_rcpi);
            DEBUG_INFO("unassoc sta link response add sta("MACFMT"), rcpi: %u success\n", MACARG(item_mac->mac), ul_rcpi);
        }
    }

    return (struct TLV *)unassoc_linkmetric_rsp;
}

static struct TLV *_obtainSteeringReqTLV(uint8_t *bssid, dlist_head *sta_list, dlist_head *target_list,
                                            uint16_t disassoc_timer, uint16_t steering_window, bool mandatory,
                                            bool imminent, bool abridged)
{
    struct mac_item *mac_i, *target_i;
    struct steerReqTLV *req = (struct steerReqTLV *)TLVNew(NULL, TLV_TYPE_STEERING_REQUEST, 0);

    req->disassoc = disassoc_timer;
    MACCPY(req->bssid, bssid);
    if (mandatory)
        SET_BIT(req->mode, STEER_REQ_MODE_MANDATE_SHIFT);
    else
        req->window = steering_window;
    if (imminent)
        SET_BIT(req->mode, STEER_REQ_MODE_DISASSOC_IMM_SHIFT);
    if (abridged)
        SET_BIT(req->mode, STEER_REQ_MODE_ABRIDGED_SHIFT);

    dlist_for_each(mac_i, *sta_list, l) {
        TLVAddMac(&req->tlv.s.t.childs[0], mac_i->mac);
    }
    if (mandatory) {
        dlist_for_each(target_i, *target_list, l) {
            struct wifi_interface *wif;

            if (IS_WILDCARD_MAC(target_i->mac)) {
                steerReqAddTargetBss(&req->tlv, target_i->mac, 0, 0);
            } else {
                if ((wif = (struct wifi_interface *)interfaceFind(NULL, target_i->mac, interface_type_wifi)) &&
                    (wif->radio)) {
                    steerReqAddTargetBss(&req->tlv, target_i->mac, wif->radio->opclass, wif->radio->channel);
                }
            }
        }
    }
    return (struct TLV *)req;
}

static struct TLV *_obtainBtmReportTLV(uint8_t *bssid_rpt, uint8_t *sta, uint8_t status, uint8_t *target_bssid)
{
    struct steerBtmReportTLV *report = (struct steerBtmReportTLV *)TLVNew(NULL, TLV_TYPE_STEERING_BTM_REPORT,
                                            sizeof(struct steerBtmReportTLV)+MACLEN);

    report->status = status;
    MACCPY(report->bssid, bssid_rpt);
    MACCPY(report->sta, sta);
    if (target_bssid) {
        report->target.datap = (uint8_t *)(report+1);
        report->target.len = MACLEN;
        MACCPY(report->target.datap, target_bssid);
    }
    return (struct TLV *)report;
}

static struct TLV *_obtainClientAssocEvtTLV(struct client *sta, uint8_t join)
{
    struct clientAssocEvtTLV *tlv = (struct clientAssocEvtTLV *)TLVNew(NULL, TLV_TYPE_CLIENT_ASSOCIATION_EVENT, 0);

    MACCPY(tlv->client, sta->mac);
    MACCPY(tlv->bssid, sta->wif->i.mac);
    if (join)
        tlv->evt = CLIENT_ASSOC_EVT_JOINED;

    return (struct TLV *)tlv;
}

static struct TLV *_obtainTunneldTLV(uint8_t *frame, uint16_t frame_len)
{
    struct tunnelTLV *tlv = (struct tunnelTLV *)TLVNew(NULL, TLV_TYPE_TUNNELED, 0);

    tlv->value.len = frame_len;
    tlv->value.datap = frame;

    return (struct TLV *)tlv;
}

static struct TLV *_obtainAssocStatusTLV(uint8_t *bssid, uint8_t status)
{
    struct TLV *tlv = TLVNew(NULL, TLV_TYPE_ASSOCIATION_STATUS_NOTIFICATION, 0);

    assocStatNotifyTLVAddBssid(tlv, bssid, status);
    return tlv;
}

static struct TLV *_obtainClsCapabilitiesTLV(void)
{
    struct clsCapTLV *tlv = (struct clsCapTLV *)
        CLSTLVNew(NULL, CLS_TLV_TYPE_CLS_CAPABILITIES, sizeof(struct clsCapTLV)+1);

    tlv->cap.datap = (uint8_t *)(tlv+1);
    tlv->cap.len = 1;

    tlv->cap.datap[0] = local_device->cls_cap.vip_max;

    return (struct TLV *)tlv;
}

static struct TLV *_obtainDSCPMappingTLV(struct DSCP_mapping_conf *mapping_conf)
{
    struct DSCP_mapping_item *item;
    struct DSCPMappingTLV *mapping_tlv = (struct DSCPMappingTLV *)CLSTLVNew(NULL, CLS_TLV_TYPE_DSCP_MAPPING_CONF, 0);

    mapping_tlv->dft_tid = mapping_conf->dft_tid;
    mapping_tlv->dft_swq = mapping_conf->dft_qid;

    if (!dlist_empty(&mapping_conf->dscp_list)) {
        dlist_for_each(item, mapping_conf->dscp_list, l) {
            mappingConfAddEntry(&mapping_tlv->tlv, item);
        }
    }
    return (struct TLV *)mapping_tlv;
}

static struct TLV *_obtainSwQueueTLV(struct dlist_head *queue)
{
    struct TLV *tlv;
    struct queue_conf_item *item;

    tlv = CLSTLVNew(NULL, CLS_TLV_TYPE_EGRESS_CONF, 0);

    if (dlist_empty(queue)) {
        TLVFree(tlv);
        return NULL;
    }
    dlist_for_each(item, *queue, l) {
        egressQConfAddQueue(tlv, item);
    }
    return tlv;
}

static struct TLV *_obtainVIPTLV(struct dlist_head *vip_list, uint8_t num)
{
    struct TLV *tlv;
    struct mac_item *vip;
    int i = 0;

    tlv = CLSTLVNew(NULL, CLS_TLV_TYPE_VIP_CONF, 0);
    /* We still need include this TLV even local VIP list is empty. */
    /* Becasue agent need to del the local vips */

    dlist_for_each(vip, *vip_list, l) {
        TLVAddMac(&tlv->s.t.childs[0], vip->mac);

        if ((++i) >= num)
            break;
    }

    return tlv;
}

static struct TLV *_obtainTCMappingTLV(struct tc_mapping_conf *conf)
{
    struct tc_mapping_item *item;
    struct TcMappingTLV *mapping_tlv = (struct TcMappingTLV *)CLSTLVNew(NULL, CLS_TLV_TYPE_TC_MAPPING_CONF, 0);

    mapping_tlv->dft_tid = conf->dft_tid;
    mapping_tlv->dft_qid = conf->dft_qid;

    if (!dlist_empty(&conf->mapping_list)) {
        dlist_for_each(item, conf->mapping_list, l) {
            TcConfAddEntry(&mapping_tlv->tlv, item);
        }
    }
    return (struct TLV *)mapping_tlv;
}

struct TLV *_obtainVBSSRadioCapabilitiesTLV(struct radio *r)
{
    struct vbssCapabilitiesTLV *tlv = (struct vbssCapabilitiesTLV *)subTLVNew(NULL,
                                        TLV_TYPE_VBSS, TLV_SUB_TYPE_VBSS_AP_RADIO_CAPABILITIES, 0);
    if (tlv) {
        MACCPY(tlv->ruid, r->uid);
        tlv->max_vbss = r->vbss_capa.max_vbss;
        if (r->vbss_capa.vbss_subtract)
            SET_BIT(tlv->flag, VBSSs_SUBTRACT_SHIFT);
        if (r->vbss_capa.vbssid_restrictions)
            SET_BIT(tlv->flag, VBSSID_RESTRICTIONS_SHIFT);
        if (r->vbss_capa.match_and_mask_restrictions)
            SET_BIT(tlv->flag, VBSSID_MATCH_AND_MASK_RESTRICTIONS_SHIFT);
        if (r->vbss_capa.fixed_bits_restrictions)
            SET_BIT(tlv->flag, FIXED_BITS_RESTRICTIONS_SHIFT);
        MACCPY(tlv->fixed_bits_mask, r->vbss_capa.fixed_bits_mask);
        MACCPY(tlv->fixed_bits_value, r->vbss_capa.fixed_bits_value);
    }
    return (struct TLV *)tlv;
}

struct TLV *_obtainVBSSCreationTLV(uint8_t *ruid, struct bss_info *bss,
                struct vbss_client_context_info *client_context, uint8_t client_assoc, char *dpp_connection)
{
    struct vbssCreationTLV *tlv = (struct vbssCreationTLV *)subTLVNew(NULL,
                                        TLV_TYPE_VBSS, TLV_SUB_TYPE_VBSS_CREATION, 0);

    if (tlv) {
        MACCPY(tlv->ruid, ruid);
        MACCPY(tlv->bssid, bss->bssid);

        tlv->ssid.len = bss->ssid.len;
        if (tlv->ssid.len > 0) {
            tlv->ssid.datap = requestBuf(&tlv->tlv.s, tlv->ssid.len);
            memcpy(tlv->ssid.datap, bss->ssid.ssid, tlv->ssid.len);
        }

        if (dpp_connection) {
            tlv->dpp_connector.len = strlen(dpp_connection) + 1;
            tlv->dpp_connector.datap = requestBuf(&tlv->tlv.s, tlv->dpp_connector.len);
            memcpy(tlv->dpp_connector.datap, dpp_connection, tlv->dpp_connector.len - 1);
            tlv->dpp_connector.datap[tlv->dpp_connector.len - 1] = 0;
        }

        if (client_context) {
            if (client_assoc) {
                MACCPY(tlv->client_mac, client_context->client_mac);
                tlv->client_assoc = client_assoc;
            }

            tlv->wpa_password.len = client_context->password.len;
            if (tlv->wpa_password.len > 0) {
                tlv->wpa_password.datap = requestBuf(&tlv->tlv.s, tlv->wpa_password.len);
                memcpy(tlv->wpa_password.datap, client_context->password.datap, tlv->wpa_password.len);
            }

            tlv->ptk.len = client_context->ptk.len;
            if (tlv->ptk.len > 0) {
                tlv->ptk.datap = requestBuf(&tlv->tlv.s, tlv->ptk.len);
                memcpy(tlv->ptk.datap, client_context->ptk.datap, tlv->ptk.len);
            }

            tlv->tx_packet_number = client_context->tx_packet_number;

            tlv->gtk.len = client_context->gtk.len;
            if (tlv->gtk.len > 0) {
                tlv->gtk.datap = requestBuf(&tlv->tlv.s, tlv->gtk.len);
                memcpy(tlv->gtk.datap, client_context->gtk.datap, tlv->gtk.len);
            }

            tlv->group_tx_packet_number = client_context->group_tx_packet_number;
        }
        /* first creation vbss when get vbss capability */
        else {
            tlv->ptk.len = bss->key.len;
            if (tlv->ptk.len > 0) {
                tlv->ptk.datap = requestBuf(&tlv->tlv.s, tlv->ptk.len);
                memcpy(tlv->ptk.datap, bss->key.key, tlv->ptk.len);
            }
        }
    }
    return (struct TLV *)tlv;
}

struct TLV *_obtainVBSSDestructionTLV(uint8_t *ruid, uint8_t *bssid, uint8_t disassociate_client)
{
    struct vbssDestructionTLV *tlv = (struct vbssDestructionTLV *)subTLVNew(NULL,
                                        TLV_TYPE_VBSS, TLV_SUB_TYPE_VBSS_DESTRUCTION, 0);

    if (tlv) {
        MACCPY(tlv->ruid, ruid);
        MACCPY(tlv->bssid, bssid);
        tlv->disassociate_client = disassociate_client;
    }
    return (struct TLV *)tlv;
}

struct TLV *_obtainVBSSEventTLV(uint8_t *ruid, uint8_t *bssid, uint8_t success)
{
    struct vbssEventTLV *tlv = (struct vbssEventTLV *)subTLVNew(NULL,
                                        TLV_TYPE_VBSS, TLV_SUB_TYPE_VBSS_EVENT, 0);

    if (tlv) {
        MACCPY(tlv->ruid, ruid);
        MACCPY(tlv->bssid, bssid);
        tlv->success = success;
    }
    return (struct TLV *)tlv;
}

struct TLV *_obtainClientSecurityContextTLV(struct vvData *ptk, struct vvData *gtk,
                                                uint64_t tx_packet_number, uint64_t group_tx_packet_number)
{
    struct clientSecurityContextTLV *tlv = (struct clientSecurityContextTLV *)subTLVNew(NULL,
                                        TLV_TYPE_VBSS, TLV_SUB_TYPE_CLIENT_SECURITY_CONTEXT, 0);

    if (tlv) {
        SET_BIT(tlv->flag, CLIENT_CONNECTED_SHIFT);

        if (ptk && ptk->len > 0) {
            tlv->ptk.len = ptk->len;
            tlv->ptk.datap = requestBuf(&tlv->tlv.s, tlv->ptk.len);
            memcpy(tlv->ptk.datap, ptk->datap, tlv->ptk.len);
        }

        tlv->tx_packet_number = tx_packet_number;

        if (gtk && gtk->len > 0) {
            tlv->gtk.len = gtk->len;
            tlv->gtk.datap = requestBuf(&tlv->tlv.s, tlv->gtk.len);
            memcpy(tlv->gtk.datap, gtk->datap, tlv->gtk.len);
        }

        tlv->group_tx_packet_number = group_tx_packet_number;
    }
    return (struct TLV *)tlv;
}

struct TLV *_obtainTriggerCSATLV(uint8_t *ruid, uint8_t csa_channel, uint8_t opclass)
{
    struct triggerCSATLV *tlv = (struct triggerCSATLV *)subTLVNew(NULL,
                                        TLV_TYPE_VBSS, TLV_SUB_TYPE_TRIGGER_CSA, 0);

    if (tlv) {
        MACCPY(tlv->ruid, ruid);
        tlv->csa_channel = csa_channel;
        tlv->opclass = opclass;
    }
    return (struct TLV *)tlv;
}

struct TLV *_obtainDeviceIDTLV()
{
    struct device_info *info = &local_device->device_info;
    struct deviceIDTLV *tlv = (struct deviceIDTLV *)TLVNew(NULL, TLV_TYPE_DEVICE_IDENTIFICATION, 0);

    if (info->friendly_name)
        strncpy(tlv->friendly_name, info->friendly_name, 64);
    if (info->manufacturer)
        strncpy(tlv->manufacturer, info->manufacturer, 64);
    if (info->model_name)
        strncpy(tlv->model_name, info->model_name, 64);
    return (struct TLV *)tlv;
}

static struct TLV *_obtainTrafficSeparationTLV(struct al_device *d)
{
    struct TLV *tlv = TLVNew(NULL, TLV_TYPE_TRAFFIC_SEPARATION_POLICY, 0);;
    struct vlan_config_item *vlan;

    dlist_for_each(vlan, local_policy.vlans, l2.l) {
        trafficPolicyTLVAddSsid(tlv, vlan->ssid.len, vlan->ssid.ssid, vlan->vlan);
    }

    return tlv;
}

static struct TLV *_obtainDefault8021QSettingTLV()
{
    struct default80211QSetsTLV *tlv = (struct default80211QSetsTLV *)TLVNew(NULL, TLV_TYPE_DEFAULT_8021Q_SETTINGS, 0);

    tlv->vlanid = local_policy.def_vlan;
    tlv->def_pcp = local_policy.def_pcp;

    return (struct TLV *)tlv;
}

static struct TLV *_obtainDSCP2UPTLV(uint8_t *dscp2up_table)
{
    struct dscpMappingTableTLV *tlv = (struct dscpMappingTableTLV *)TLVNew(NULL, TLV_TYPE_DSCP_MAPPING_TABLE, 0);

    memcpy(tlv->dscp_pcp_mapping, dscp2up_table, DSCP2UP_SIZE);

    return (struct TLV *)tlv;
}

static struct TLV *_obtainClsControllerWeightTLV(void)
{
    struct controllerWeightTLV *tlv = (struct controllerWeightTLV *)
        CLSTLVNew(NULL, CLS_TLV_TYPE_CONTROLLER_WEIGHT, sizeof(struct controllerWeightTLV)+1);

    tlv->weight = local_device->controller_weight;

    return (struct TLV *)tlv;
}

static int _needSendTrafficSeperation(struct al_device *d)
{
    if ((d->profile<profile_2) || (d->max_vid<dlist_count(&local_policy.vlans)))
        return 0;

    return 1;
}


////////////////////////////////////////////////////////////////////////////////
// Public functions (exported only to files in this same folder)
////////////////////////////////////////////////////////////////////////////////
uint8_t sendRawPacket2(struct CMDU2 *cmdu, char *interface_name, uint8_t *dst, uint8_t *src)
{
    struct cmdu_buf *buf;
    //send1905CmduExtensions(cmdu);
    if (!cmdu) {
        DEBUG_WARNING("empty CMDU\n");
        return 0;
    }

    if (encodeCMDU(cmdu)) {
        // Could not forge the packet. Error?
        DEBUG_WARNING("encodeCMDU() failed!\n");
        return 0;
    }

    if (!dst)
        dst = (uint8_t *)MCAST_1905;
    if (!src)
        src = DMalMacGet();

    DEBUG_INFO("--%s-> [mid=%04x, dst="MACFMT"] %s\n",
        interface_name, cmdu->id, MACARG(dst), convert_1905_CMDU_type_to_string(cmdu->type));

    dlist_for_each(buf, cmdu->streams, l) {
        if (0 == PLATFORM_SEND_RAW_PACKET2(interface_name,
                    dst,
                    src,
                    ETHERTYPE_1905,
                    buf)) {
            DEBUG_ERROR("[%s]Packet couldn't sent[type=0x%x, mid=%04x]!\n", interface_name, cmdu->type, cmdu->id);
            return 0;
        }
    }

    return 1;
}

uint8_t sendRawPacketRetry2(struct CMDU2 *cmdu, char *interface_name, uint8_t *dst, uint8_t *src)
{
    if (0==sendRawPacket2(cmdu, interface_name, dst, src)) {
        return 0;
    }

    return cmdu2AddRetry(cmdu, interface_name, dst);
}

uint8_t sendRawPacketAuto(struct CMDU2 *cmdu, char *interface_name, uint8_t *dst, uint8_t *src)
{
    struct cmdu_msg_desc *desc = CMDU_type_to_desc(cmdu->type);

    if (desc->resp_type!=CMDU_TYPE_INVALID)
        return sendRawPacketRetry2(cmdu, interface_name, dst, src);
    else
        return sendRawPacket2(cmdu, interface_name, dst, src);
}

uint8_t sendMulticast(struct CMDU2 *cmdu, uint32_t exclude, uint8_t *dst, uint8_t *src)
{
    struct interface_list_item *item;
    dlist_head *ifs_names;

    //FIXME: use local interface information
    ifs_names = PLATFORM_GET_LIST_OF_1905_INTERFACES();
    dlist_for_each(item, *ifs_names, l) {
        struct interfaceInfo *x = PLATFORM_GET_1905_INTERFACE_INFO(item->intf_name);
        if (NULL == x) continue;

        if ((0 == x->is_secured) ||
            ((x->power_state != INTERFACE_POWER_STATE_ON)
            && (x->power_state!= INTERFACE_POWER_STATE_SAVE))
            || (exclude && x->index == exclude)) {
            goto next;
        }

        if (0 == sendRawPacket2(cmdu, item->intf_name, dst, src)) {
            DEBUG_WARNING("Could not send packet in sendMulticast\n");
            PLATFORM_FREE_1905_INTERFACE_INFO(x);
            return 0;
        }
next:
        PLATFORM_FREE_1905_INTERFACE_INFO(x);
    }
    return 1;
}

struct TLV *obtainErrCodeTLV(uint8_t err_code, uint8_t *sta)
{
    struct errorCodeTLV *err = (struct errorCodeTLV *)TLVNew(NULL, TLV_TYPE_ERROR_CODE, 0);

    err->code = err_code;
    MACCPY(err->sta, sta);
    return (struct TLV *)err;
}

struct TLV *obtainP2ErrCodeTLV(struct p2_ec_item *p2err_code)
{
    struct p2ErrorCodeTLV *p2err = (struct p2ErrorCodeTLV *)TLVNew(NULL, TLV_TYPE_PROFILE2_ERROR_CODE, 0);

    p2err->code = p2err_code->reason_code;
    if ((p2err_code->reason_code == REASON_COMBINED_BSS_TRAFFIC_SEP_UNSUPPORT)
        || (p2err_code->reason_code == REASON_MIX_BACKHAUL_UNSUPPORT))
        MACCPY(p2err->bssid, p2err_code->mac);
    /* Fill the rule_id and qmid if R3/R4 is supported */
    return (struct TLV *)p2err;
}

uint8_t sendZeroTLV(uint16_t type, char *interface_name, uint16_t mid, uint8_t *dst)
{
    uint8_t ret = 0;
    struct CMDU2 *c = cmdu2New(type, mid);

    if (!c)
        return ret;

    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL)) {
        DEBUG_WARNING("Could not send %s\n", convert_1905_CMDU_type_to_string(type));
        goto bail;
    }
    ret = 1;
bail:
    cmdu2Free(c);
    return ret;
}

uint8_t reportChanScanResult(struct chscan_req *req)
{
    uint8_t ret = 0;

    if (!req || !req->r)
        return 0;

    ret = sendChannelScanReport(idx2InterfaceName(req->ifindex), getNextMid(), req->src, req->r, req);
    chscanReqDelete(req);

    return ret;
}

uint8_t sendTopologyDiscovery(char *interface_name, uint16_t mid)
{
    // - One 1905.1 AL MAC address type TLV (see Table 6-8)
    // - One 1905.1 MAC address type TLV (see Table 6-9)

    uint8_t  ret = 0;
    uint8_t  mcast_address[] = MCAST_1905;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_TOPOLOGY_DISCOVERY, mid);

    if (!c) {
        return ret;
    }

    cmdu2AddTlv(c, _obtainMACTLV(TLV_TYPE_AL_MAC_ADDRESS_TYPE, DMalMacGet()));
    cmdu2AddTlv(c, _obtainMACTLV(TLV_TYPE_MAC_ADDRESS_TYPE, interfaceName2MAC(interface_name)));

    if (0 == sendRawPacket2(c, interface_name, mcast_address, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;

    cmdu2Free(c);
    return ret;
}

uint8_t sendTopologyNotification(uint16_t mid, struct client *sta, uint8_t join)
{
    uint8_t ret = 0;
    uint8_t  mcast_address[] = MCAST_1905;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_TOPOLOGY_NOTIFICATION, mid);

    if (!c)
        return ret;

    cmdu2AddTlv(c, _obtainMACTLV(TLV_TYPE_AL_MAC_ADDRESS_TYPE, DMalMacGet()));
    cmdu2AddTlv(c, _obtainClientAssocEvtTLV(sta, join));

    if (0 == sendMulticast(c, 0, mcast_address, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;

    cmdu2Free(c);
    return ret;
}

uint8_t sendTopologyQuery(char *interface_name, uint16_t mid, uint8_t *dst)
{
    uint8_t ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_TOPOLOGY_QUERY, mid);

    if (!c)
        return ret;

    cmdu2AddTlv(c, _obtainMapProfileTLV(local_device));

    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;

    cmdu2Free(c);
    return ret;
}

uint8_t sendTopologyResponse(char *interface_name, uint16_t mid, uint8_t* dst)
{
    // - One device information type TLV (see Table 6-10)
    // - Zero or more device bridging capability TLVs (see Table 6-11)
    // - Zero or more non-1905 neighbor device list TLVs (see Table 6-14)
    // - Zero or more 1905.1 neighbor device TLVs (see Table 6-15)

    // - Zero or one SupportedService TLV (see section 17.2.1).
    // - One AP Operational BSS TLV (see section 17.2.4).
    // - Zero or one Associated Clients TLV (see section 17.2.5).
    // - One Multi-AP Profile TLV (see section 17.2.47) [Profile-2].
    // - One BSS Configuration Report TLV (see section 17.2.75) [Profile-3].

    uint8_t  ret = 0;
    struct interface *i;
    struct radio *r;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_TOPOLOGY_RESPONSE, mid);

    if (!c) {
        return ret;
    }

    cmdu2AddTlv(c, _obtainDeviceInfoTLV(local_device));
    dlist_for_each(i, local_device->interfaces, l) {
        cmdu2AddTlv(c, _obtainNon1905NeighborListTLV(i));
    }
    dlist_for_each(i, local_device->interfaces, l) {
        cmdu2AddTlv(c, _obtaini1905NeighborListTLV(i));
    }

    cmdu2AddTlv(c, _obtainServiceTLV(TLV_TYPE_SUPPORTED_SERVICE, local_device->is_controller, local_device->is_agent));

    dlist_for_each(r, local_device->radios, l) {
        cmdu2AddTlv(c, _obtainOperatingChanRptTLV(r));
    }

    cmdu2AddTlv(c, _obtainAPOperationalBSSTLV(local_device));
    cmdu2AddTlv(c, _obtainAssociatedClientsTLV(local_device));
    cmdu2AddTlv(c, superTLVNew(TLV_TYPE_VBSS, _obtainVbssConfigurationReportTLV(local_device)));

    cmdu2AddTlv(c, _obtainMapProfileTLV(local_device));

    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;

    cmdu2Free(c);
    return ret;
}

uint8_t sendLinkMetricsQuery(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t *neighbor_al_mac, uint8_t link_metrics_type)
{
    uint8_t ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_LINK_METRIC_QUERY, mid);

    if (!c)
        return ret;
    cmdu2AddTlv(c, _obtainLinkMetricsQueryTLV(neighbor_al_mac, link_metrics_type));
    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;

    cmdu2Free(c);
    return ret;
}

uint8_t getListOfLinkWithNeighbors(struct interface *i, dlist_head *link_list)
{
    struct neighbor *n;
    struct link_item *item = NULL;
    struct link_pair_item *pair = NULL;
    uint8_t num = 0;
    uint8_t exist = 0;

    if (dlist_count(&i->neighbors) == 0) {
        DEBUG_INFO("No neighbors for interface: ["MACFMT"]\n", MACARG(i->mac));
        return num;
    }
    dlist_for_each(n, i->neighbors, l) {
        exist = 0;
        if (dlist_count(link_list)) {
            dlist_for_each(item, *link_list, l) {
                if ((MACCMP(item->local_al_mac, i->owner->al_mac) == 0) &&
                    (MACCMP(item->neighbor_al_mac, n->al_mac) == 0)) {
                    exist = 1;
                    break;
                }
            }
        }
        if (!exist) {
            DEBUG_INFO("New device pair: i->owner->al_mac:["MACFMT"], n->al_mac:["MACFMT"]\n", MACARG(i->owner->al_mac), MACARG(n->al_mac));
            item = calloc(1, sizeof(struct link_item));
            MACCPY(item->local_al_mac, i->owner->al_mac);
            MACCPY(item->neighbor_al_mac, n->al_mac);
            dlist_head_init(&item->links);
            pair = calloc(1, sizeof(struct link_pair_item));
            MACCPY(pair->local_intf_mac, i->mac);
            MACCPY(pair->neighbor_intf_mac, n->intf_mac);
            dlist_add_tail(&item->links, &pair->l);
            dlist_add_tail(link_list, &item->l);
            num++;
        } else {
            DEBUG_INFO("New link pair: i->mac:["MACFMT"], n->intf_mac:["MACFMT"]\n", MACARG(i->mac), MACARG(n->intf_mac));
            pair = calloc(1, sizeof(struct link_pair_item));
            MACCPY(pair->local_intf_mac, i->mac);
            MACCPY(pair->neighbor_intf_mac, n->intf_mac);
            dlist_add_tail(&item->links, &pair->l);
        }
    }
    return num;
}

uint8_t sendLinkMetricsResponse(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t *neighbor_al_mac, uint8_t link_metrics_type)
{
    // 'neighbor' == LINK_METRIC_QUERY_TLV_SPECIFIC_NEIGHBOR
    //   A) A CMDU containing either:
    //      - One Tx link metrics
    //      - One Rx link metrics
    //      - One Rx and one Tx link metrics
    // 'neighbor' == LINK_METRIC_QUERY_TLV_ALL_NEIGHBORS
    //   B) A CMDU made by concatenating many CMDUs of "type A" (one for each of its 1905 neighbors).
    struct CMDU2 *c = NULL;
    struct interface *i;
    dlist_head neighbor_list;
    struct link_item *item = NULL;
    uint8_t neighbor = LINK_METRIC_QUERY_TLV_ALL_NEIGHBORS;
    uint8_t ret = 0;

    c = cmdu2New(CMDU_TYPE_LINK_METRIC_RESPONSE, mid);
    if (!c)
        return ret;
    if (neighbor_al_mac)
        neighbor = LINK_METRIC_QUERY_TLV_SPECIFIC_NEIGHBOR;
    dlist_head_init(&neighbor_list);
    dlist_for_each(i, local_device->interfaces, l) {
        DEBUG_WARNING("Get neighbor list for interface: ["MACFMT"]\n", MACARG(i->mac));
        getListOfLinkWithNeighbors(i, &neighbor_list);
    }
    dlist_for_each(item, neighbor_list, l) {
        if (neighbor == LINK_METRIC_QUERY_TLV_SPECIFIC_NEIGHBOR && MACCMP(item->neighbor_al_mac, neighbor_al_mac) != 0) {
            continue;
        }
        if (link_metrics_type == TX_LINK_METRICS_ONLY || link_metrics_type == TX_AND_RX_LINK_METRICS) {
           cmdu2AddTlv(c, _obtain1905TransmitterLinkMetricsTLV(item));
        }
        if (link_metrics_type == RX_LINK_METRICS_ONLY || link_metrics_type == TX_AND_RX_LINK_METRICS) {
            cmdu2AddTlv(c, _obtain1905ReceiverLinkMetricsTLV(item));
        }
    }
    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;

    cmdu2Free(c);
    if (dlist_count(&neighbor_list)) {
        dlist_for_each(item, neighbor_list, l) {
            dlist_free_items(&item->links, struct link_pair_item, l);
        }
        dlist_free_items(&neighbor_list, struct link_item, l);
    }
    return ret;
}

uint8_t sendAPAutoconfigurationSearch(uint16_t mid, uint8_t freq_band)
{
    // - One 1905.1 AL MAC address type TLV (see Table 6-8)
    // - One SearchedRole TLV (see Table 6-22)
    // - One AutoconfigFreqBand TLV (see Table 6-23)

    // - Zero or one SupportedService TLV (see section 17.2.1).
    // - Zero or one SearchedService TLV (see section 17.2.2).
    // - One Multi-AP Profile TLV (see section 17.2.47) [Profile-2].
    // - Zero or one DPP Chirp Value TLV (see section 17.2.83) [Profile-3].

    uint8_t  ret = 0;
    uint8_t  mcast_address[] = MCAST_1905;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_AP_AUTOCONFIGURATION_SEARCH, mid);

    if (!c) {
        return ret;
    }
    c->relay = 1;

    cmdu2AddTlv(c, _obtainMACTLV(TLV_TYPE_AL_MAC_ADDRESS_TYPE, DMalMacGet()));
    cmdu2AddTlv(c, _obtainU8TLV(TLV_TYPE_SEARCHED_ROLE, 0));
    cmdu2AddTlv(c, _obtainU8TLV(TLV_TYPE_AUTOCONFIG_FREQ_BAND, freq_band));
    cmdu2AddTlv(c, _obtainServiceTLV(TLV_TYPE_SEARCHED_SERVICE, 1, 0));
    cmdu2AddTlv(c, _obtainMapProfileTLV(local_device));

    if (local_config.auto_role) {
        cmdu2AddTlv(c, _obtainServiceTLV(TLV_TYPE_SUPPORTED_SERVICE, 1, 1));
        cmdu2AddTlv(c, CLSVendorTLVNew(_obtainClsControllerWeightTLV()));
    }
    else
        cmdu2AddTlv(c, _obtainServiceTLV(TLV_TYPE_SUPPORTED_SERVICE, local_device->is_controller, local_device->is_agent));

    if (0 == sendMulticast(c, 0, mcast_address, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;

    cmdu2Free(c);
    return ret;
}

uint8_t sendAPAutoconfigurationResponse(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t freq_band)
{
    // - One SupportedRole TLV (see Table 6-24)
    // - One SupportedFreqBand TLV (see in Table 6-25)

    // - Zero or one SupportedService TLV (see section 17.2.1).
    // - One Device 1905 Layer Security Capability TLV (see section 17.2.67) [Profile-3].
    // - One Multi-AP Profile TLV (see section 17.2.47) [Profile-2].
    // - Zero or one DPP Chirp Value TLV (see section 17.2.83) [Profile-3].
    // - Zero or one Controller Capability TLV (see section 17.2.94).
    uint8_t  ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_AP_AUTOCONFIGURATION_RESPONSE, mid);

    if (!c) {
        return ret;
    }

    cmdu2AddTlv(c, _obtainU8TLV(TLV_TYPE_SUPPORTED_ROLE, 0));
    cmdu2AddTlv(c, _obtainU8TLV(TLV_TYPE_AUTOCONFIG_FREQ_BAND, freq_band));

    cmdu2AddTlv(c, _obtainServiceTLV(TLV_TYPE_SUPPORTED_SERVICE, local_device->is_controller, local_device->is_agent));
    cmdu2AddTlv(c, _obtainMapProfileTLV(local_device));
    if (local_config.auto_role) {
        cmdu2AddTlv(c, CLSVendorTLVNew(_obtainClsControllerWeightTLV()));
    }

    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;

    cmdu2Free(c);
    return ret;
}

uint8_t sendAPAutoconfigurationRenew(uint16_t mid, uint8_t freq_band, struct al_device *dst)
{
    uint8_t  ret = 0;
    uint8_t  mcast_address[] = MCAST_1905;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_AP_AUTOCONFIGURATION_RENEW, mid);

    if (!c) {
        return ret;
    }

    c->relay = 1;
    cmdu2AddTlv(c, _obtainMACTLV(TLV_TYPE_AL_MAC_ADDRESS_TYPE, DMalMacGet()));

    cmdu2AddTlv(c, _obtainU8TLV(TLV_TYPE_SUPPORTED_ROLE, 0));
    cmdu2AddTlv(c, _obtainU8TLV(TLV_TYPE_SUPPORTED_FREQ_BAND, freq_band));

    if (!dst) {
        ret = sendMulticast(c, 0, mcast_address, NULL);
    } else {
        ret = sendRawPacketAuto(c, idx2InterfaceName(dst->recv_intf_idx), dst->al_mac, NULL);
    }

    if (0 == ret) {
        DEBUG_WARNING("Could not send packet\n");
    } else {
        ret = 1;
    }

    cmdu2Free(c);
    return ret;
}

uint8_t sendHigherLayerResponse(char *interface_name, uint16_t mid, uint8_t *dst)
{
    uint8_t  ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_HIGHER_LAYER_RESPONSE, mid);

    if (!c)
        return ret;

    cmdu2AddTlv(c, _obtainMACTLV(TLV_TYPE_AL_MAC_ADDRESS_TYPE, DMalMacGet()));
    cmdu2AddTlv(c, _obtainU8TLV(TLV_TYPE_1905_PROFILE_VERSION, 0x01));
    cmdu2AddTlv(c, _obtainDeviceIDTLV());

    dmUpdateIPInfo();
    cmdu2AddTlv(c, _obtainIPv4TLV());
    cmdu2AddTlv(c, _obtainIPv6TLV());

    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;

    cmdu2Free(c);
    return ret;

}

uint8_t sendAPAutoconfigurationWSCM1(char *interface_name, uint16_t mid, uint8_t *dst, struct radio *r, struct TLV *m1)
{
    uint8_t  ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_AP_AUTOCONFIGURATION_WSC, mid);

    if (!c)
        return ret;

    //adjust tlv order for cmcc
    cmdu2AddTlv(c, m1);
    cmdu2AddTlv(c, _obtainapRadioBasicCapabilitiesTLV(r));
    cmdu2AddTlv(c, _obtainProfile2ApCapabilityTLV(local_device));
    if (local_device->cls_cap.vip_max)
        cmdu2AddTlv(c, CLSVendorTLVNew(_obtainClsCapabilitiesTLV()));

    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;

    cmdu2Free(c);
    return ret;
}

uint8_t sendAPAutoconfigurationWSCM2(char *interface_name, uint16_t mid, uint8_t *dst, struct radio *r, dlist_head *m2s)
{
    uint8_t  ret = 0;
    struct TLV *tlv;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_AP_AUTOCONFIGURATION_WSC, mid);
    struct al_device *dev;

    if (!c)
        return ret;

    if (!(dev = alDeviceFind(dst))) {
        goto bail;
    }

    cmdu2AddTlv(c, _obtainMACTLV(TLV_TYPE_AP_RADIO_IDENTIFIER, r->uid));

    while ((tlv = container_of(dlist_get_first(m2s), struct TLV, s.t.l))) {
        dlist_remove(&tlv->s.t.l);
        cmdu2AddTlv(c, tlv);
    }

    if (_needSendTrafficSeperation(dev)) {
        cmdu2AddTlv(c, _obtainDefault8021QSettingTLV());
        cmdu2AddTlv(c, _obtainTrafficSeparationTLV(dev));
    }

    if ((dev->cls_cap.vip_max) && (!dlist_empty(&local_policy.vips))) {
        cmdu2AddTlv(c, CLSVendorTLVNew(_obtainVIPTLV(&local_policy.vips, dev->cls_cap.vip_max)));
    }
    cmdu2AddTlv(c, CLSVendorTLVNew(_obtainDSCPMappingTLV(&local_policy.dscp_conf)));
    cmdu2AddTlv(c, CLSVendorTLVNew(_obtainSwQueueTLV(&local_policy.queue_conf)));
    cmdu2AddTlv(c, CLSVendorTLVNew(_obtainTCMappingTLV(&local_policy.tc_conf)));

    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else {
        dev->status = STATUS_CONFIGURED;
        dev->set_unconfigured_ts = 0;
        ret = 1;
    }
bail:
    cmdu2Free(c);
    return ret;
}


uint8_t sendAPCapabilityReport(char *interface_name, uint16_t mid, uint8_t *dst)
{
    uint8_t  ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_AP_CAPABILITY_REPORT, mid);
    struct radio *r;

    if (!c)
        return ret;

    cmdu2AddTlv(c, _obtainU8TLV(TLV_TYPE_AP_CAPABILITY, local_device->ap_capability));

    dlist_for_each(r, local_device->radios, l) {
        cmdu2AddTlv(c, _obtainapRadioBasicCapabilitiesTLV(r));
        if (r->bands_capa[r->current_band_idx].ht_capa_valid)
            cmdu2AddTlv(c, _obtainHTCapabilityTLV(r));
        if (r->bands_capa[r->current_band_idx].vht_capa_valid)
            cmdu2AddTlv(c, _obtainVHTCapabilityTLV(r));
        if (r->bands_capa[r->current_band_idx].he_capa_valid)
            cmdu2AddTlv(c, _obtainHECapabilityTLV(r));
    }
    if (local_device->profile >= profile_2)
    {
        cmdu2AddTlv(c, _obtainProfile2ApCapabilityTLV(local_device));
        cmdu2AddTlv(c, _obtainCacCapabilityTLV(local_device));
    }

    if (local_device->cls_cap.vip_max)
        cmdu2AddTlv(c, CLSVendorTLVNew(_obtainClsCapabilitiesTLV()));

    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;
    cmdu2Free(c);
    return ret;

}

uint8_t sendLLDPDiscovery(char *interface_name)
{
    uint8_t  al_mac_address[6];
    uint8_t  interface_mac_address[6];

    struct chassisIdTLV      chassis_id_tlv;
    struct portIdTLV         port_id_tlv;
    struct timeToLiveTypeTLV time_to_live_tlv;

    struct PAYLOAD payload;

    uint8_t  *stream;
    uint16_t  stream_len;
    struct cmdu_buf *buf;

    DEBUG_INFO("--%s-> LLDP_BRIDGE_DISCOVERY\n", interface_name);

    PLATFORM_MEMCPY(al_mac_address,        DMalMacGet(),                         6);
    PLATFORM_MEMCPY(interface_mac_address, interfaceName2MAC(interface_name), 6);

    // Fill the chassis ID TLV
    //
    chassis_id_tlv.tlv_type           = TLV_TYPE_CHASSIS_ID;
    MACCPY(chassis_id_tlv.chassis_id, al_mac_address);

    // Fill the port ID TLV
    //
    port_id_tlv.tlv_type            = TLV_TYPE_PORT_ID;
    MACCPY(port_id_tlv.port_id, interface_mac_address);

    // Fill the time to live TLV
    //
    time_to_live_tlv.tlv_type       = TLV_TYPE_TIME_TO_LIVE;
    time_to_live_tlv.ttl            = TIME_TO_LIVE_TLV_1905_DEFAULT_VALUE;

    // Forge the LLDP payload containing all these TLVs
    //
    payload.list_of_TLVs[0] = (uint8_t *)&chassis_id_tlv;
    payload.list_of_TLVs[1] = (uint8_t *)&port_id_tlv;
    payload.list_of_TLVs[2] = (uint8_t *)&time_to_live_tlv;
    payload.list_of_TLVs[3] = NULL;

    stream = forge_lldp_PAYLOAD_from_structure(&payload, &stream_len);
    buf = cmduBufNew(stream_len + 14, 14);
    memcpy(buf->data, stream, stream_len);
    free_lldp_PAYLOAD_packet(stream);
    cmduBufPut(buf, stream_len);

    // Finally, send the packet!
    //
    {
        uint8_t   mcast_address[] = MCAST_LLDP;

        DEBUG_DETAIL("Sending LLDP bridge discovery message on interface %s\n", interface_name);
        if (0 == PLATFORM_SEND_RAW_PACKET2(interface_name,
                                          mcast_address,
                                          interface_mac_address,
                                          ETHERTYPE_LLDP,
                                          buf))
        {
            DEBUG_ERROR("Packet could not be sent!\n");
        }
    }

    // Free memory
    //
    cmduBufFree(buf);

    return 1;
}

uint8_t sendMapPolicyConfigRequest(char *interface_name, uint16_t mid, uint8_t *dst, struct map_policy* policy)
{
    // - Zero or one steering policy TLV (see section 17.2.11).
    // - Zero or one metrics reporting policy TLV (see section 17.2.12).

    uint8_t  ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_MAP_POLICY_CONFIG_REQUEST, mid);
    struct backhaul_bss_conf_item *bconf;
    struct al_device *dev;

    if (!c) {
        return ret;
    }

    if (!(dev = alDeviceFind(dst))) {
        goto bail;
    }

    DEBUG_INFO("--%s-> MAP_POLICY_CONFIG_REQUEST\n", interface_name);

    cmdu2AddTlv(c, _obtainSteeringPolicyTLV(&policy->stalist_local_steer_disallow,
                                            &policy->stalist_btm_steer_disallow,
                                            &policy->steer));
    cmdu2AddTlv(c, _obtainMetricsRptPolicyTLV(dev, policy->metrics_rpt_intvl));

    dlist_for_each(bconf, policy->backhaul_bss_configs, l) {
        cmdu2AddTlv(c, _obtainBackhaulBSSConfTLV(bconf));
    }

    if (_needSendTrafficSeperation(dev)) {
        cmdu2AddTlv(c, _obtainDefault8021QSettingTLV());
        cmdu2AddTlv(c, _obtainTrafficSeparationTLV(dev));
    }

    if ((dev->cls_cap.vip_max) && (BIT_IS_SET(local_device->vip_conf_changed, VIP_CONF_CHANGED_STA_SHIFT))) {
        cmdu2AddTlv(c, CLSVendorTLVNew(_obtainVIPTLV(&policy->vips, dev->cls_cap.vip_max)));
        CLR_BIT(local_device->vip_conf_changed, VIP_CONF_CHANGED_STA_SHIFT);
    }

    if (BIT_IS_SET(local_device->vip_conf_changed, VIP_CONF_CHANGED_MAPPING_SHIFT)) {
        cmdu2AddTlv(c, CLSVendorTLVNew(_obtainDSCPMappingTLV(&policy->dscp_conf)));
        CLR_BIT(local_device->vip_conf_changed, VIP_CONF_CHANGED_MAPPING_SHIFT);
    }

    if (BIT_IS_SET(local_device->vip_conf_changed, VIP_CONF_CHANGED_QUEUE_SHIFT)) {
        cmdu2AddTlv(c, CLSVendorTLVNew(_obtainSwQueueTLV(&policy->queue_conf)));
        CLR_BIT(local_device->vip_conf_changed, VIP_CONF_CHANGED_QUEUE_SHIFT);
    }

    if (BIT_IS_SET(local_device->vip_conf_changed, VIP_CONF_CHANGED_TC_SHIFT)) {
        cmdu2AddTlv(c, CLSVendorTLVNew(_obtainTCMappingTLV(&policy->tc_conf)));
        CLR_BIT(local_device->vip_conf_changed, VIP_CONF_CHANGED_TC_SHIFT);
    }

    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;

bail:
    cmdu2Free(c);
    return ret;
}

void _buildChannelPreferenceReport(struct CMDU2 *c, struct al_device *dev, struct radio *r, uint16_t mid, uint8_t opclass,
        uint8_t channel, uint8_t cac_status, bool is_unsolicited)
{
    // - Zero or more channel preference TLVs (see section 17.2.13).
    // - Zero or more radio operation restriction TLVs (see section 17.2.14).
    // - Zero or one CAC Completion Report TLV (see section 17.2.44) [Profile-2].
    // - One CAC Status Reprot TLV (see section 17.2.45) [Profile-2].

    if (r) {
        cmdu2AddTlv(c, _obtainChanPrefTLV(r));
        cmdu2AddTlv(c, _obtainOpRestTLV(r));
        if ((local_device->profile >= profile_2)
            && (!is_unsolicited || (cac_status == CAC_CMPLT_STATUS_RADAR_DETECTED)))
            cmdu2AddTlv(c, _obtainCacCompletionReportTLV(r, opclass, channel, cac_status));
    } else {
        dlist_for_each(r, dev->radios, l)
        {
            cmdu2AddTlv(c, _obtainChanPrefTLV(r));
            cmdu2AddTlv(c, _obtainOpRestTLV(r));
        }
    }

    if (local_device->profile >= profile_2) {
        cmdu2AddTlv(c, _obtainCacStatusReportTLV(r));
    }
    return;
}

uint8_t sendChannelPreferenceReport(char *interface_name, uint16_t mid, uint8_t *dst, struct al_device *dev)
{
    struct CMDU2 *c = NULL;
    uint8_t ret = 0;

    c = cmdu2New(CMDU_TYPE_CHANNEL_PREFERENCE_REPORT, mid);
    if (!c) {
        DEBUG_WARNING("Could not build channel preference message\n");
        return ret;
    }
    _buildChannelPreferenceReport(c, dev, NULL, mid, 0, 0, 0, 0);
    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;
    cmdu2Free(c);
    return ret;
}

uint8_t sendCacScanChannelPreferenceReport(char *interface_name, uint16_t mid, uint8_t *dst, struct radio *r)
{
    struct CMDU2 *c = NULL;
    uint8_t ret = 0;

    if(!r || !registrar)
        return ret;

    c = cmdu2New(CMDU_TYPE_CHANNEL_PREFERENCE_REPORT, mid);
    if (!c) {
        DEBUG_WARNING("Could not build channel preference message\n");
        return ret;
    }
    _buildChannelPreferenceReport(c, NULL, r, mid, r->cur_cac.cur_scan.opclass, r->cur_cac.cur_scan.channel, CAC_CMPLT_STATUS_OTHER_ERR, true);
    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;
    cmdu2Free(c);
    return ret;
}

uint8_t sendClientCapabilityQuery(char *interface_name, uint16_t mid, uint8_t * dst, uint8_t *bssid, uint8_t *sta)
{
    // - One client info TLV (see section 17.2.18).

    uint8_t  ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_CLIENT_CAPABILITY_QUERY, mid);
    if (!c) {
        return ret;
    }
    cmdu2AddTlv(c, _obtainClientInfoTLV(bssid, sta));
    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;
    cmdu2Free(c);
    return ret;
}

uint8_t sendClientCapabilityReport(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t *bssid, uint8_t *sta)
{
    // - One client info TLV (see section 17.2.18)
    // - One client capability report TLV (see section 17.2.19).
    // - zero or one error code TLV (see section 17.2.36).

    struct wifi_interface *wif;
    struct client *client = NULL;
    uint8_t ret = 0;
    uint8_t result = CAP_REPORT_SUCCESS;
    uint8_t err_code = 0;

    struct CMDU2 *c = cmdu2New(CMDU_TYPE_CLIENT_CAPABILITY_REPORT, mid);
    if (!c)
        return ret;
    cmdu2AddTlv(c, _obtainClientInfoTLV(bssid, sta));

    wif = (struct wifi_interface *)interfaceFind(local_device, bssid, interface_type_wifi);

    if ((wif) && (wif->role == role_ap)) {
        client = clientFind(NULL, wif, sta);
    }

    if (client) {
        if (!client->last_assoc.datap || !client->last_assoc.len) {
            result = CAP_REPORT_FAILURE;
            err_code = EC_CLIENT_CAPRPT_FAILED_UNSPEC;
        }
    } else {
        result = CAP_REPORT_FAILURE;
        err_code = EC_STA_UNASSOICITE;
    }

    cmdu2AddTlv(c, _obtainClientCapRptTLV(client, result));

    if (err_code)
        cmdu2AddTlv(c, obtainErrCodeTLV(err_code, sta));

    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;
    cmdu2Free(c);
    return ret;
}

uint8_t sendSingleClientAssocControl(struct al_device *d, uint8_t *bssid, uint8_t *mac, uint16_t period, uint8_t control)
{
    struct client_assoc_ctrl_item *item;
    struct mac_item *item_mac;

    if (!d || !bssid || !mac)
        return 0;

    DEFINE_DLIST_HEAD(req_head);
    item_mac = calloc(1, sizeof(struct mac_item));
    if (!item_mac) {
        DEBUG_ERROR("mac_item calloc failed\n");
        return 0;
    }
    MACCPY(item_mac->mac, mac);

    item = calloc(1, sizeof(struct client_assoc_ctrl_item));
    if (!item) {
        PLATFORM_FREE(item_mac);
        return 0;
    }
    dlist_head_init(&item->sta_list);
    MACCPY(item->req_bssid, bssid);
    item->assoc_ctrl = control;
    item->period = period;
    dlist_add_tail(&item->sta_list, &item_mac->l);
    dlist_add_tail(&req_head, &item->l);

    sendClientAssocControlRequest(idx2InterfaceName(d->recv_intf_idx),
            getNextMid(), d->al_mac, &req_head);

    dlist_free_items(&item->sta_list, struct mac_item, l);
    dlist_free_items(&req_head, struct client_assoc_ctrl_item, l);

    return 1;
}

uint8_t sendClientAssocControlRequest(char *interface_name, uint16_t mid, uint8_t *dst, struct dlist_head *req_list)
{
    struct CMDU2 *c = NULL;
    struct al_device *d = NULL;
    struct client_assoc_ctrl_item *item;
    uint8_t ret = 0;

    if ((!interface_name) || (!dst))
        return ret;

    if ((!req_list) || dlist_empty(req_list))
        return ret;


    d = alDeviceFindAny(dst);
    if (!d) {
        DEBUG_WARNING("Can NOT find device ["MACFMT"]\n", MACARG(dst));
        return ret;
    }
    c = cmdu2New(CMDU_TYPE_CLIENT_ASSOCIATION_CONTROL, mid);
    if (!c)
        return ret;

    dlist_for_each(item, *req_list, l) {
        cmdu2AddTlv(c, _obtainClientAssocControlReqTLV(item->req_bssid, item->assoc_ctrl, item->period, &item->sta_list));
    }

    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;

    cmdu2Free(c);
    return ret;
}

uint8_t sendCombinedInfrastructMetrics(char *interface_name, uint16_t mid, uint8_t *dst)
{
    struct CMDU2 *c = NULL;
    struct al_device *d = NULL;
    struct al_device *alldev = NULL;
    struct interface *intf = NULL;
    dlist_head neighbor_list;
    struct link_item *item = NULL;
    struct wifi_interface *wif = NULL;
    uint8_t ret = 0;

    if (!local_device->is_controller) {
        DEBUG_WARNING("Can NOT find device ["MACFMT"]\n", MACARG(dst));
        return ret;
    }
    if (!interface_name || !dst) {
        return ret;
    }
    d = alDeviceFindAny(dst);
    if (!d) {
        DEBUG_WARNING("Can NOT find device ["MACFMT"]\n", MACARG(dst));
        return ret;
    }
    c = cmdu2New(CMDU_TYPE_COMBINED_INFRASTRUCTURE_METRICS, mid);
    if (!c)
        return ret;
    dlist_head_init(&neighbor_list);
    dlist_for_each(alldev, local_network, l) {
        dlist_for_each(intf, alldev->interfaces, l) {
            if (intf->type != interface_type_wifi)
                continue;
            wif = (struct wifi_interface *)intf;
            if (wif->role == role_ap)
                cmdu2AddTlv(c, _obtainApMetricsTLV(wif));
            if (wif->bssInfo.backhaul == 1 || wif->bssInfo.backhaul_sta == 1) {
                intf = &wif->i;
                getListOfLinkWithNeighbors(intf, &neighbor_list);
            }
        }
    }
    dlist_for_each(item, neighbor_list, l) {
        cmdu2AddTlv(c, _obtain1905TransmitterLinkMetricsTLV(item));
    }
    dlist_for_each(item, neighbor_list, l) {
        cmdu2AddTlv(c, _obtain1905ReceiverLinkMetricsTLV(item));
    }

    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;

    cmdu2Free(c);
    if (dlist_count(&neighbor_list)) {
        dlist_for_each(item, neighbor_list, l) {
            dlist_free_items(&item->links, struct link_pair_item, l);
        }
        dlist_free_items(&neighbor_list, struct link_item, l);
    }
    return ret;
}

uint8_t sendBackhaulSteeringRequest(char *interface_name, uint16_t mid, uint8_t *dst,
                                            uint8_t *sta, uint8_t *target, uint8_t opclass, uint8_t channel)
{
    struct CMDU2 *c = NULL;
    struct al_device *d = NULL;
    uint8_t  ret = 0;

    if (!interface_name || !dst) {
        return ret;
    }
    d = alDeviceFindAny(dst);
    if (!d) {
        DEBUG_WARNING("Can NOT find device ["MACFMT"]\n", MACARG(dst));
        return ret;
    }

    c = cmdu2New(CMDU_TYPE_BACKHAUL_STEERING_REQUEST, mid);
    if (!c)
        return ret;
    cmdu2AddTlv(c, _obtainBackhaulSteeringRequestTLV(sta, target, opclass, channel));
    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;

    cmdu2Free(c);
    return ret;
}

uint8_t sendChannelScanRequest(char *interface_name, uint16_t mid, uint8_t *dst,
                                      uint8_t fresh_scan, uint8_t num, struct chscan_req *reqs)
{
    struct CMDU2 *c = NULL;
    struct al_device *d = NULL;
    uint8_t ret = 0;

    if (!interface_name || !dst || !reqs) {
        return ret;
    }
    d = alDeviceFindAny(dst);
    if (!d) {
        DEBUG_WARNING("Can NOT find device ["MACFMT"]\n", MACARG(dst));
        return ret;
    }
    if (!d->is_agent) {
        DEBUG_WARNING("Device ["MACFMT"] is NOT agent\n", MACARG(dst));
        return ret;
    }
    if (d->profile < profile_2) {
        DEBUG_WARNING("Device ["MACFMT"] is NOT a profile-2 device\n", MACARG(dst));
        return ret;
    }

    c = cmdu2New(CMDU_TYPE_CHANNEL_SCAN_REQUEST, mid);
    if (!c) {
        return ret;
    }
    cmdu2AddTlv(c, _obtainChanScanReqTLV(d, fresh_scan, num, reqs));
    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;

    cmdu2Free(c);
    return ret;
}

uint8_t sendChannelScanReport(char *interface_name, uint16_t mid, uint8_t *dst,
            struct radio *r, struct chscan_req *req)
{
    struct CMDU2 *c = NULL;
    struct TLV *tlv;
    uint32_t tlv_num = 0;
    uint8_t result_num = 0;
    struct chscan_req_item *item = NULL;
    struct operating_class *opc = NULL;
    struct chan_info *ch = NULL;
    uint8_t status;
    uint8_t ret = 0;
    uint8_t i, j;

    if (!interface_name || !dst) {
        return ret;
    }
    c = cmdu2New(CMDU_TYPE_CHANNEL_SCAN_REPORT, mid);
    if (!c) {
        return ret;
    }

    if (req) {
        cmdu2AddTlv(c, _obtainTimeStampTLV(req->ts));
        result_num = dlist_count(&req->h);
        if (!result_num) {
            DEBUG_WARNING("No result for this request.\n");
            goto bail;
        }
        dlist_for_each(item, req->h, l) {
            opc = opclassFind(r, item->opclass);
            if (!opc) {
                DEBUG_WARNING("can NOT find opclass(%d) for radio["MACFMT"]\n", item->opclass, MACARG(r->uid));
                continue;
            }
            for (i = 0; i < item->ch_num; i++) {
                ch = channelFind(opc, item->chans[i]);
                if (!ch) {
                    DEBUG_WARNING("can NOT find ch[%d] in opc[%d] for radio["MACFMT"]\n", item->chans[i], item->opclass, MACARG(r->uid));
                    continue;
                }
                status = (req->status != SCAN_STATUS_SUCCESS) ? req->status : chanScanItemStatusTransfer(item);
                ch->scan_status = status;
                tlv = _obtainScanResultTLV(r->uid, item->opclass, ch, status);
                if (!tlv) {
                    DEBUG_WARNING("ch[%d] in opc[%d] for radio["MACFMT"] get result tlv failed\n",
                        item->chans[i], item->opclass, MACARG(r->uid));
                    continue;
                }
                cmdu2AddTlv(c, tlv);
                tlv_num++;
            }
        }
    } else {
        cmdu2AddTlv(c, _obtainTimeStampTLV(PLATFORM_GET_TIMESTAMP(0)));
        for (i = 0; i < r->num_opc_support; i++) {
            opc = &r->opclasses[i];
            for (j = 0; opc->num_support_chan; j++) {
                ch = &opc->channels[j];
                tlv = _obtainScanResultTLV(r->uid, opc->op_class, ch, SCAN_STATUS_SUCCESS);
                if (!tlv) {
                    DEBUG_WARNING("ch[%d] in opc[%d] for radio["MACFMT"] get result tlv failed\n",
                        ch->id, opc->op_class, MACARG(r->uid));
                    continue;
                }
                cmdu2AddTlv(c, tlv);
                tlv_num++;
            }
        }
    }

    if (tlv_num < 1) {
        DEBUG_WARNING("No scan result tlvs to send on radio(%s).\n", r->name);
        goto bail;
    }

    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;

bail:
    cmdu2Free(c);
    return ret;
}

uint8_t sendCacRequest(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t num_reqs, struct cac_request *reqs)
{
    struct CMDU2 *c = NULL;
    struct al_device *d = NULL;
    int ret = 0;

    if (!local_device->is_controller) {
        DEBUG_WARNING("Device ["MACFMT"] is NOT controller\n", MACARG(local_device->al_mac));
        return ret;
    }
    if (!interface_name || !dst || !reqs)
        return ret;
    d = alDeviceFindAny(dst);
    if (!d) {
        DEBUG_WARNING("Can NOT find device ["MACFMT"]\n", MACARG(dst));
        return ret;
    }

    c = cmdu2New(CMDU_TYPE_CAC_REQUEST, mid);
    if (!c)
        return ret;
    cmdu2AddTlv(c, _obtainCacRequestTLV(num_reqs, reqs));
    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet for CAC request\n");
    else
        ret = 1;

    cmdu2Free(c);
    return ret;
}

uint8_t sendCacTermination(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t num_terms, struct cac_termination *terms)
{
    struct CMDU2 *c = NULL;
    struct al_device *d = NULL;
    int ret = 0;

    if (!local_device->is_controller) {
        DEBUG_WARNING("Device ["MACFMT"] is NOT controller\n", MACARG(local_device->al_mac));
        return ret;
    }
    if (!interface_name || !dst) {
        return ret;
    }
    d = alDeviceFindAny(dst);
    if (!d) {
        DEBUG_WARNING("Can NOT find device ["MACFMT"]\n", MACARG(dst));
        return ret;
    }

    c = cmdu2New(CMDU_TYPE_CAC_TERMINATION, mid);
    if (!c) {
        return ret;
    }
    cmdu2AddTlv(c, _obtainCacTerminationTLV(num_terms, terms));
    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet for CAC termination\n");
    else
        ret = 1;

    cmdu2Free(c);
    return ret;
}

uint8_t sendBackhaulSteeringResponse(char *interface_name, uint16_t mid, uint8_t *dst,
                                             struct wifi_interface *wif, uint8_t result, uint8_t code)
{
    struct CMDU2 *c = NULL;
    struct al_device *d = NULL;
    uint8_t  ret = 0;

    if (!interface_name || !dst || !wif)
        return ret;
    d = alDeviceFindAny(dst);
    if (!d) {
        DEBUG_WARNING("Can NOT find device ["MACFMT"]\n", MACARG(dst));
        return ret;
    }

    c = cmdu2New(CMDU_TYPE_BACKHAUL_STEERING_RESPONSE, mid);
    if (!c)
        return ret;
    cmdu2AddTlv(c, _obtainBackhaulSteeringResponseTLV(wif, result));
    if (result)
        cmdu2AddTlv(c, _obtainErrorCodeTLV(wif->i.mac, code));

    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;

    cmdu2Free(c);
    return ret;
}
uint8_t sendBackhaulSTACapabilityReport(char *interface_name, uint16_t mid, uint8_t * dst)
{
    uint8_t  ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_BACKHAUL_STA_CAPABILITY_REPORT, mid);
    struct interface *intf;

    if (!c)
        return ret;
    dlist_for_each(intf, local_device->interfaces, l) {
        if (intf->type == interface_type_wifi) {
            struct wifi_interface *wif = (struct wifi_interface *)intf;
            if ((wif->role == role_sta) && (wif->radio)) {
                cmdu2AddTlv(c, _obtainBackhaulSTARadioCapaTLV(wif));
            }
        }
    }
    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;
    cmdu2Free(c);
    return ret;
}

uint8_t sendFailedConnection(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t *sta, uint16_t status, uint16_t reason)
{
    uint8_t  ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_FAILED_CONNECTION, mid);

    if (!c) {
        return ret;
    }

    cmdu2AddTlv(c, _obtainMACTLV(TLV_TYPE_STA_MAC_ADDRESS, sta));
    cmdu2AddTlv(c, _obtainCodeTLV(TLV_TYPE_STATUS_CODE, status));
    if (reason>0)
        cmdu2AddTlv(c, _obtainCodeTLV(TLV_TYPE_REASON_CODE, reason));

    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;
    cmdu2Free(c);

    return ret;
}

uint8_t sendChannelSelectionRequest(char *interface_name, uint16_t mid, uint8_t * dst, struct al_device *dev)
{
    // - Zero or more channel preference TLVs (see section 17.2.13).
    // - Zero or more transmit power limit TLVs (see section 17.2.15).

    struct radio* r;
    uint8_t  ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_CHANNEL_SELECTION_REQUEST, mid);

    if (!c) {
        return ret;
    }

    dlist_for_each(r, dev->radios, l) {
        cmdu2AddTlv(c, _obtainChanPrefTLV(r));
        cmdu2AddTlv(c, _obtainTxPwrTLV(r));
    }
    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;

    cmdu2Free(c);
    return ret;
}


uint8_t sendChannelSelectionResponse(char *interface_name, uint16_t mid, uint8_t * dst)
{
    // - One or more channel selection response TLVs (see section 17.2.16).

    struct radio* r;
    uint8_t  ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_CHANNEL_SELECTION_RESPONSE, mid);

    if (!c) {
        return ret;
    }
    dlist_for_each(r, local_device->radios, l) {
        if ((r->change_power) || (r->change_channel) || (r->channel_selection_code)) {
            cmdu2AddTlv(c, _obtainChanSelRspTLV(r, r->channel_selection_code));
            r->change_power = 0;
            r->change_channel = 0;
            r->channel_selection_code = 0;
        }
    }
    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;

    cmdu2Free(c);
    return ret;
}

uint8_t sendOperatingChannelReport(char *interface_name, uint16_t mid, uint8_t * dst, struct al_device *dev, int force)
{
    // - One or more operating channel report TLVs (see section 17.2.17).
    uint8_t  ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_OPERATING_CHANNEL_REPORT, mid);
    struct radio *r;

    if (!c) {
        return ret;
    }

    dlist_for_each(r, dev->radios, l) {
        if ((force) || (r->change_mask & OPERATING_CHANNEL_CHANGE_MASK)) {
            cmdu2AddTlv(c, _obtainOperatingChanRptTLV(r));
            r->change_mask &= (~OPERATING_CHANNEL_CHANGE_MASK);
            ret++;
        }
    }
    if (!ret)
        goto bail;

    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL)) {
        DEBUG_WARNING("Could not send packet\n");
        ret = 0;
    } else {
        ret = 1;
    }
bail:
    cmdu2Free(c);
    return ret;
}

uint8_t sendApMetricsQuery(char *interface_name, uint16_t mid, uint8_t *dst, struct dlist_head *vap_list)
{
    // - One AP metric query TLV (see section 17.2.21).

    uint8_t  ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_AP_METRICS_QUERY, mid);

    if (!c) {
        return ret;
    }
    cmdu2AddTlv(c, _obtainApMetricsQryTLV(vap_list));

    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;

    cmdu2Free(c);
    return ret;
}

void  _addBSSMetricsTLV(struct CMDU2 *c, struct wifi_interface *wif)
{
    struct client *sta;
    uint8_t includes = 0;
    uint32_t ts = PLATFORM_GET_TIMESTAMP(0);

    if ((!wif) || (!wif->radio))
        return;

    includes = wif->radio->metrics_rpt_policy.assoc_sta_inclusion_mode;

    cmdu2AddTlv(c, _obtainApMetricsTLV(wif));

    if (!includes)
        return;

    dlist_for_each(sta, wif->clients, l) {
        if ((includes & REPORT_STA_TRAFFIC_STATS)
                && (ts-sta->last_traffic_status_report_ts>MIN_STA_TRAFFIC_STATUS_REPORT_INTERVAL_MS)) {
            sta->last_traffic_status_report_ts = ts;
            cmdu2AddTlv(c, _obtainAssocStaTrafStatTLV(sta));
        }
        if (includes & REPORT_STA_LINK_METRICS) {
            cmdu2AddTlv(c, _obtainAssocStaLinkMetricsTLV(sta));
        }
    }
}

uint8_t sendApMetricsResponse(char *interface_name, uint16_t mid, uint8_t *dst, struct dlist_head *vap_list, uint8_t crossed)
{
    // - One or more AP metric query TLVs (see section 17.2.22).
    // - Zero or moreAssociated STA Traffic Stats TLVs (see section 17.2.35).
    // - Zero or moreAssociated STA Link Metrics TLVs (see section 17.2.24).
    struct wifi_interface *wif;
    uint8_t  ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_AP_METRICS_RESPONSE, mid);

    if (!c) {
        return ret;
    }

    if (vap_list) {
        struct macStruct *mac_s;
        dlist_for_each(mac_s, *vap_list, s.t.l) {
            if ((wif = (struct wifi_interface *)interfaceFind(local_device, mac_s->mac, interface_type_wifi))
                && (wif->role==role_ap)) {
                _addBSSMetricsTLV(c, wif);
            }
        }
    } else { /* report the metrics periodically or if crossed utiliziation threshold */
        struct radio *r;
        dlist_for_each(r, local_device->radios, l) {
            if ((!crossed) || (r->ch_util_crossed)) {
                int i;
                for (i=0;i<r->configured_bsses.len;i++) {
                    if ((wif = r->configured_bsses.p[i])) {
                        _addBSSMetricsTLV(c, wif);
                    }
                }
            }
            r->ch_util_crossed = 0;
        }
    }

    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;
    cmdu2Free(c);
    return ret;
}

uint8_t sendAssociatedStaLinkMetricsQuery(char *interface_name, uint16_t mid, uint8_t * dst, uint8_t *sta_mac)
{
    uint8_t ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_ASSOCIATED_STA_LINK_METRICS_QUERY, mid);

    if (!c) {
        return ret;
    }
    cmdu2AddTlv(c, _obtainMACTLV(TLV_TYPE_STA_MAC_ADDRESS, sta_mac));

    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;

    cmdu2Free(c);
    return ret;
}

uint8_t sendAck(char *interface_name, uint16_t mid, uint8_t *dst, dlist_head *err_codes)
{
    uint8_t ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_ACK, mid);
    struct TLV *err_code, *tmp;

    if (!c)
        return ret;
    if (err_codes) {
        dlist_for_each_safe(err_code, tmp, *err_codes, s.t.l) {
            dlist_remove(&err_code->s.t.l);
            cmdu2AddTlv(c, err_code);
        }
    }

    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send %s\n", convert_1905_CMDU_type_to_string(CMDU_TYPE_ACK));
    else
        ret = 1;

    cmdu2Free(c);
    return ret;
}

uint8_t sendAssociatedStaLinkMetricsResponse(char *interface_name, uint16_t mid, uint8_t * dst, uint8_t *sta_mac)
{
    // - One or More Associated STA Link Metrics Response TLVs (see section 17.2.24).
    // - zero or one Error code TLV (see section 17.2.36).
    uint8_t ret = 0;
    struct client *sta;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_ASSOCIATED_STA_LINK_METRICS_RESPONSE, mid);

    if (!c) {
        return ret;
    }

    sta = clientFind(local_device, NULL, sta_mac);
    if (sta)
        cmdu2AddTlv(c, _obtainAssocStaLinkMetricsTLV(sta));
    else
        cmdu2AddTlv(c, obtainErrCodeTLV(EC_STA_UNASSOICITE, sta_mac));

    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;

    cmdu2Free(c);
    return ret;
}

uint8_t sendUnassociatedStaLinkMetricsQuery(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t opclass,
                                                    dlist_head *sta_list)
{
    // - One Unassociated STA Link Metrics query TLV (see section 17.2.25).
    uint8_t  ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_UNASSOCIATED_STA_LINK_METRICS_QUERY, mid);

    if (!c) {
        return ret;
    }
    cmdu2AddTlv(c, _obtainUnassocStaLinkMetricQryTLV(opclass, sta_list));
    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;
    cmdu2Free(c);
    return ret;
}

uint8_t sendUnassociatedStaLinkMetricsResponse(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t opclass,
                                                    dlist_head *sta_list, dlist_head *target_list)
{
    // - One Unassociated STA Link Metrics response TLV (see section 17.2.26).

    uint8_t  ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_UNASSOCIATED_STA_LINK_METRICS_RESPONSE, mid);

    if (!c) {
        return ret;
    }

    cmdu2AddTlv(c, _obtainUnassocStaLinkMetricRspTLV(target_list, sta_list, opclass));
    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;

    cmdu2Free(c);
    return ret;
}

uint8_t sendClientSteeringRequest(char *interface_name, uint16_t mid, uint8_t * dst, dlist_head *target_list,
                                            dlist_head *sta_list, uint16_t disassoc_timer, uint16_t steering_window,
                                            bool mandatory, bool imminent, bool abridged, uint8_t *bssid)
{
    // - One steering request TLV (see section 17.2.29).
    struct al_device *dev = alDeviceFind(dst);
    uint8_t  ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_CLIENT_STEERING_REQUEST, mid);
    if (!c || !dev) {
        goto bail;
    }

    cmdu2AddTlv(c, _obtainSteeringReqTLV(bssid, sta_list, target_list, disassoc_timer, steering_window,
                    mandatory, imminent, abridged));
    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;

    //FIXME: todo process following steer response
bail:
    if (c)
        cmdu2Free(c);
    return ret;
}

uint8_t sendClientSteeringBTMReport(char *interface_name, uint16_t mid, uint8_t * dst, uint8_t *bssid_rpt,
                                                uint8_t *sta, uint8_t status, uint8_t *target_bssid)
{
    // - One steering BTM report TLV (see section 17.2.30).

    uint8_t  ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_CLIENT_STEERING_BTM_REPORT, mid);
    if (!c) {
        return ret;
    }
    cmdu2AddTlv(c, _obtainBtmReportTLV(bssid_rpt, sta, status, target_bssid));
    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;
    cmdu2Free(c);
    return ret;
}

uint8_t sendBeaconMetricsQuery(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t *sta_request,
                                        uint8_t opclass_request, uint8_t chan_request,
                                        uint8_t *bssid_request, uint8_t detail, uint8_t len_ssid, char *ssid,
                                        dlist_head *list_chan_report, uint8_t *eid)
{
    // - One Beacon metrics query TLVs (see section 17.2.27).

    uint8_t  ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_BEACON_METRICS_QUERY, mid);

    if (!c) {
        return ret;
    }
    cmdu2AddTlv(c, _obtainBeaconMetricsQueryTLV(sta_request, opclass_request, chan_request, bssid_request, detail,
                                                len_ssid, ssid, list_chan_report, eid));

    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;

    cmdu2Free(c);
    return ret;
}

uint8_t sendBeaconMetricsResponse(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t *sta, uint8_t *report,
                                    uint16_t len)
{
    // - One Beacon metrics query TLVs (see section 17.2.28).

    uint8_t  ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_BEACON_METRICS_RESPONSE, mid);

    if (!c) {
        return ret;
    }

    cmdu2AddTlv(c, _obtainBeaconMetricsRspTLV(sta, report, len));

    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;

    /* timer operation will be re-design in action operation */
//    if (sta->bcn_meas_ctx.beacon_meas_timer)
//        platformCancelTimer(sta->bcn_meas_ctx.beacon_meas_timer);

    cmdu2Free(c);
    return ret;
}

uint8_t sendTunnledMessage(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t *client, uint8_t type,
                            uint8_t *frame, uint16_t frame_len)
{
    uint8_t  ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_TUNNELED, mid);

    if (!c) {
        return ret;
    }

    cmdu2AddTlv(c, _obtainMACTLV(TLV_TYPE_SOURCE_INFO, client));
    cmdu2AddTlv(c, _obtainU8TLV(TLV_TYPE_TUNNELED_MESSAGE_TYPE, type));
    cmdu2AddTlv(c, _obtainTunneldTLV(frame, frame_len));

    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;

    cmdu2Free(c);
    return ret;
}

uint8_t sendAssociationStatNotification(char *interface_name, uint16_t mid, uint8_t *dst,
                                                            uint8_t *bssid, uint8_t status)
{
    // - One Association Status Notification TLV (see section 17.2.53).

    uint8_t  ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_ASSOCIATION_STATUS_NOTIFICATION, mid);

    if (!c) {
        return ret;
    }
    cmdu2AddTlv(c, _obtainAssocStatusTLV(bssid, status));
    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;

    cmdu2Free(c);
    return ret;
}

uint8_t sendClientDisassocStatsMessage(char *interface_name, uint16_t mid, uint8_t *dst,
                                                        struct client *sta, uint16_t reason)
{
    // - One STA MAC Address TLV (see section 17.2.23).
    // - One Reason Code TLV (see section 17.2.64).
    // - One Associated STA Traffic Stats TLV (see section 17.2.35).

    uint8_t ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_CLIENT_DISASSOCIATION_STATS, mid);

    if (!c)
        return ret;

    cmdu2AddTlv(c, _obtainMACTLV(TLV_TYPE_AL_MAC_ADDRESS_TYPE, sta->mac));
    cmdu2AddTlv(c, _obtainCodeTLV(TLV_TYPE_REASON_CODE, reason));
    cmdu2AddTlv(c, _obtainAssocStaTrafStatTLV(sta));

    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;

    cmdu2Free(c);
    return ret;
}

uint8_t sendErrorResponse(char *interface_name, uint16_t mid, uint8_t *dst, dlist_head *ec_list)
{
    // - One or more profile-2 error code TLVs (see section 17.2.51).

    uint8_t  ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_ERROR_RESPONSE, mid);
    struct p2_ec_item *p2err_code;

    if (!c) {
        return ret;
    }

    dlist_for_each(p2err_code, *ec_list, l) {
        cmdu2AddTlv(c, obtainP2ErrCodeTLV(p2err_code));
    }

    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;

    cmdu2Free(c);
    return ret;
}


uint8_t sendVBSSCapabilitiesResponse(char *interface_name, uint16_t mid, uint8_t * dst)
{
    uint8_t  ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_VBSS_CAPABILITIES_RESPONSE, mid);
    struct radio *r = NULL;

    if (!c)
        return ret;

    if (!local_device)
        goto bail;

    dlist_for_each(r, local_device->radios, l) {
        cmdu2AddTlv(c, superTLVNew(TLV_TYPE_VBSS, _obtainVBSSRadioCapabilitiesTLV(r)));
    }

    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;

bail:
    cmdu2Free(c);
    return ret;
}

uint8_t sendVBSSCreationRequest(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t *ruid, struct bss_info *bss,
    struct vbss_client_context_info *client_context, struct client *sta, char *dpp_connection)
{
    uint8_t ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_VBSS_REQUEST, mid);

    if (!c)
        return ret;

    if (!bss)
        goto bail;

    if (sta) {
        cmdu2AddTlv(c, superTLVNew(TLV_TYPE_VBSS, _obtainVBSSCreationTLV(ruid, bss, client_context, 1, dpp_connection)));
        cmdu2AddTlv(c, _obtainClientCapRptTLV(sta, sta->last_capa_rpt_result));
    }
    else {
        cmdu2AddTlv(c, superTLVNew(TLV_TYPE_VBSS, _obtainVBSSCreationTLV(ruid, bss, client_context, 0, dpp_connection)));
    }

    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet(VBSS Creation Request)\n");
    else
        ret = 1;

bail:
    cmdu2Free(c);
    return ret;
}

uint8_t sendVBSSDestructionRequest(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t *ruid,
            uint8_t *bssid, uint8_t disassoc_client, char *dpp_connection)
{
    uint8_t  ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_VBSS_REQUEST, mid);

    if (!c)
        return ret;

    cmdu2AddTlv(c, superTLVNew(TLV_TYPE_VBSS, _obtainVBSSDestructionTLV(ruid, bssid, disassoc_client)));

    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet(VBSS Request)\n");
    else
        ret = 1;

    cmdu2Free(c);
    return ret;
}

uint8_t sendVBSSResponse(char *interface_name, uint16_t mid, uint8_t * dst, uint8_t *ruid,
            uint8_t *bssid, uint8_t success)
{
    uint8_t  ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_VBSS_RESPONSE, mid);

    if (!c)
        return ret;

    cmdu2AddTlv(c, superTLVNew(TLV_TYPE_VBSS, _obtainVBSSEventTLV(ruid, bssid, success)));

    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet(VBSS Response)\n");
    else
        ret = 1;

    cmdu2Free(c);
    return ret;
}

uint8_t sendClientSecurityContextRequest(char *interface_name, uint16_t mid, uint8_t *dst,
                uint8_t *bssid, uint8_t *sta_mac)
{
    uint8_t  ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_CLIENT_SECURITY_CONTEXT_REQUEST, mid);

    if (!c)
        return ret;

    if (!bssid || !sta_mac)
        goto bail;

    cmdu2AddTlv(c, _obtainClientInfoTLV(bssid, sta_mac));

    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet(client security context request)\n");
    else
        ret = 1;

bail:
    cmdu2Free(c);
    return ret;
}

uint8_t sendClientSecurityContextResponse(char *interface_name, uint16_t mid, uint8_t *dst,
                uint8_t *bssid, uint8_t *sta_mac, struct vvData *ptk, struct vvData *gtk,
                uint64_t tx_packet_number, uint64_t group_tx_packet_number)
{
    uint8_t  ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_CLIENT_SECURITY_CONTEXT_RESPONSE, mid);

    if (!c)
        return ret;

    if (!bssid || !sta_mac)
        goto bail;

    cmdu2AddTlv(c, _obtainClientInfoTLV(bssid, sta_mac));
    //TODO: get client security info and fill it in response message
    cmdu2AddTlv(c, superTLVNew(TLV_TYPE_VBSS, _obtainClientSecurityContextTLV(ptk, gtk,
                            tx_packet_number, group_tx_packet_number)));

    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet(client security context response)\n");
    else
        ret = 1;

bail:
    cmdu2Free(c);
    return ret;
}

uint8_t sendTriggerCSARequest(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t *ruid,
                uint8_t *bssid, uint8_t *sta_mac, uint8_t csa_channel, uint8_t opclass)
{
    uint8_t  ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_TRIGGER_CSA_REQUEST, mid);

    if (!c)
        return ret;

    if (!ruid || !bssid || !sta_mac)
        goto bail;

    cmdu2AddTlv(c, _obtainClientInfoTLV(bssid, sta_mac));
    cmdu2AddTlv(c, superTLVNew(TLV_TYPE_VBSS, _obtainTriggerCSATLV(ruid, csa_channel, opclass)));

    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet(trigger csa request)\n");
    else
        ret = 1;

bail:
    cmdu2Free(c);
    return ret;
}

uint8_t sendTriggerCSAResponse(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t *ruid,
                uint8_t *bssid, uint8_t *sta_mac, uint8_t csa_channel, uint8_t opclass)
{
    uint8_t  ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_TRIGGER_CSA_RESPONSE, mid);

    if (!c)
        return ret;

    if (!ruid || !bssid || !sta_mac)
        goto bail;

    cmdu2AddTlv(c, _obtainClientInfoTLV(bssid, sta_mac));
    cmdu2AddTlv(c, superTLVNew(TLV_TYPE_VBSS, _obtainTriggerCSATLV(ruid, csa_channel, opclass)));

    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet(trigger csa response)\n");
    else
        ret = 1;

bail:
    cmdu2Free(c);
    return ret;
}

uint8_t sendVbssMovePreparationRequest(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t *bssid, uint8_t *sta_mac)
{
    uint8_t  ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_VBSS_MOVE_PREPARATION_REQUEST, mid);

    if (!c)
        return ret;

    if (!bssid || !sta_mac)
        goto bail;

    cmdu2AddTlv(c, _obtainClientInfoTLV(bssid, sta_mac));

    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet(vbss move preparation request)\n");
    else
        ret = 1;

bail:
    cmdu2Free(c);
    return ret;
}

uint8_t sendVbssMovePreparationResponse(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t *bssid, uint8_t *sta_mac)
{
    uint8_t  ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_VBSS_MOVE_PREPARATION_RESPONSE, mid);

    if (!c)
        return ret;

    if (!bssid || !sta_mac)
        goto bail;

    cmdu2AddTlv(c, _obtainClientInfoTLV(bssid, sta_mac));

    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet(vbss move preparation response)\n");
    else
        ret = 1;

bail:
    cmdu2Free(c);
    return ret;
}

uint8_t sendVbssMoveCancelRequest(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t *bssid, uint8_t *sta_mac)
{
    uint8_t  ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_VBSS_MOVE_CANCEL_REQUEST, mid);

    if (!c)
        return ret;

    if (!bssid || !sta_mac)
        goto bail;

    cmdu2AddTlv(c, _obtainClientInfoTLV(bssid, sta_mac));

    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet(vbss move cancel request)\n");
    else
        ret = 1;

bail:
    cmdu2Free(c);
    return ret;
}

uint8_t sendVbssMoveCancelResponse(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t *bssid, uint8_t *sta_mac)
{
    uint8_t  ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_VBSS_MOVE_CANCEL_RESPONSE, mid);

    if (!c)
        return ret;

    if (!bssid || !sta_mac)
        goto bail;

    cmdu2AddTlv(c, _obtainClientInfoTLV(bssid, sta_mac));

    if (0 == sendRawPacketAuto(c, interface_name, dst, NULL))
        DEBUG_WARNING("Could not send packet(vbss move cancel response)\n");
    else
        ret = 1;

bail:
    cmdu2Free(c);
    return ret;
}

uint8_t sendServicePrioritizationRequest(char *intf_name, uint16_t mid, uint8_t *dst)
{
    uint8_t  ret = 0;
    struct CMDU2 *c = cmdu2New(CMDU_TYPE_SERVICE_PRIORITIZATION_REQUEST, mid);

    if (!c)
        return ret;

    if (local_policy.dscp2up_set) {
        cmdu2AddTlv(c, _obtainDSCP2UPTLV(local_policy.dscp2up_table));
    }

    if (0 == sendRawPacketAuto(c, intf_name, dst, NULL))
        DEBUG_WARNING("Could not send packet\n");
    else
        ret = 1;

    cmdu2Free(c);
    return ret;
}
