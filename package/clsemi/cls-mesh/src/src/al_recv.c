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

#include "os_utils.h"
#include "datamodel.h"
#include "al_recv.h"
#include "al_utils.h"
#include "feature/feature_helper.h"
#include "feature/ubus/ubus_helper.h"
#include "feature/steering/record.h"
#include "al_send.h"
#include "al_wsc.h"
#include "1905_tlvs.h"
#include "1905_cmdus.h"
#include "1905_l2.h"
#include "lldp_tlvs.h"
#include "lldp_payload.h"
#include "al_action.h"
#include "al_driver.h"
#include "al_cmdu.h"

#include "platform_interfaces.h"
#include "platform_os.h"
#include "wifi.h"

////////////////////////////////////////////////////////////////////////////////
// Public functions (exported only to files in this same folder)
////////////////////////////////////////////////////////////////////////////////



static DEFINE_DLIST_HEAD(higher_layer_data_protocol_handlers);

int registerHigherLayerDataProtocol(uint8_t protocol, higherLayerDataFunc func)
{
    struct higherLayerDataProtocol *p;
    if (!func)
        return -1;

    if ((p = malloc(sizeof(struct higherLayerDataProtocol)))) {
        p->protocol = protocol;
        p->func = func;
        dlist_add_tail(&higher_layer_data_protocol_handlers, &p->l);
        return 0;
    }

    return -1;
}

static int processHigherLayerDataTLV(struct al_device *d, uint32_t intf_idx, struct higherLayerDataTLV *tlv)
{
    struct higherLayerDataProtocol *p;

    dlist_for_each(p, higher_layer_data_protocol_handlers, l) {
        if ((p->protocol==tlv->protocol) && (p->func)) {
            return p->func(d, intf_idx, tlv->payload.datap, tlv->payload.len);
        }
    }
    return 0;
}

static int _radioUpdateBasicCapability(struct radio *r, struct radioBasicCapabilityTLV *tlv)
{
    struct txOpclassStruct *opclass_s;

    r->max_bss = tlv->max_bss;

    dlist_for_each(opclass_s, tlv->tlv.s.t.childs[0], s.t.l) {
        struct operating_class *opclass = opclassAdd(r, opclass_s->opclass);

        r->current_band_idx = opclass2BandIdx(opclass_s->opclass);
        opclass->max_tx_power = opclass_s->max_tx_power;
        initOperatingClass(opclass, 0);
        if (opclass_s->non_op_chans[0]) {
            int i;
            for (i=0;i<opclass_s->non_op_chans[0];i++) {
                struct chan_info *ch_info;

                if ((ch_info = channelFind(opclass, opclass_s->non_op_chans[1+i])))
                    ch_info->disable = 1;
            }
        }
    }

    return 0;
}

static void _processAssocStaLinkMetricsTLV(struct client *c, struct dlist_head *list)
{
    struct assocLinkMetricStruct *item;

    dlist_for_each(item, *list, s.t.l) {
        if (MACCMP(item->bssid, c->wif->i.mac)) {
            //FIXME: mlo?
            DEBUG_WARNING("client["MACFMT"] report on ["MACFMT"] but associated on ["MACFMT"]\n",
                            MACARG(c->mac), MACARG(item->bssid), MACARG(c->wif->i.mac));
            continue;
        }
        c->link_metrics_ts = PLATFORM_GET_TIMESTAMP(item->age);
        c->link_metrics = item->metrics;
    }
}

#define UPDATE_ESPI_FEILDES(espi, value) do {   \
            espi.enabled = 1; \
            espi.data_fmt = (value[2] & ESPI_DATA_FORMAT_MASK) >> ESPI_DATA_FORMAT_SHIFT; \
            espi.ba_win = (value[2] & ESPI_BA_WINDOW_MASK) >> ESPI_BA_WINDOW_SHIFT; \
            espi.airtime_fraction = value[1]; \
            espi.ppdu_duration  = value[0]; \
        } while(0)

static void  _processAPMetricsTLV(struct wifi_interface *wif, struct apMetricsTLV *metrics)
{
    wif->metrics.ch_util = metrics->chan_util;

    if (metrics->includes & ESPI_BE_INCLUDED)
        UPDATE_ESPI_FEILDES(wif->metrics.espi[0], metrics->espi_be);
    if (metrics->includes & ESPI_BK_INCLUDED)
        UPDATE_ESPI_FEILDES(wif->metrics.espi[1], metrics->espi_bk);
    if (metrics->includes & ESPI_VO_INCLUDED)
        UPDATE_ESPI_FEILDES(wif->metrics.espi[2], metrics->espi_vo);
    if (metrics->includes & ESPI_VI_INCLUDED)
        UPDATE_ESPI_FEILDES(wif->metrics.espi[3], metrics->espi_vi);
}

static int clientUpdateMeasuredElem(struct client *sta, dlist_head *elem_list)
{
//  struct measure_elem_item *elem_original, *elem_additional;
    struct measuredElemStruct *elem_in_pkt;
//  bool flag_found = false;

    dlist_for_each(elem_in_pkt, *elem_list, s.t.l) {
#if 0
        dlist_for_each(elem_original, sta->measure_elem_list, l) {
            if (elem_in_pkt->elem.id == elem_original->id) {
                elem_original->len = elem_in_pkt->elem.len;
                free(elem_original->elem);
                elem_original->elem = calloc(1, elem_in_pkt->elem.len);
                memcpy(elem_original->elem, elem_in_pkt->elem.datap, elem_original->len);
                flag_found = true;
                break;
            }
        }
        if (false == flag_found) { /* new element id */
            elem_additional = calloc(1, sizeof(struct measure_elem_item));
            if (elem_additional) {
                elem_additional->id = elem_in_pkt->elem.id;
                elem_additional->len = elem_in_pkt->elem.len;
                elem_additional->elem = calloc(1, elem_in_pkt->elem.len);
                memcpy(elem_additional->elem, elem_in_pkt->elem.datap, elem_additional->len);
                dlist_add_tail(&sta->measure_elem_list, &elem_additional->l);
            }
        }
#endif
    }
    return 1;
}

static int _processSupportServiceTLV(struct al_device *d, struct TLV *tlv)
{
   struct serviceStruct *s;

   if (!d->profile)
       d->profile = 1;

   d->is_controller = 0;
   d->is_agent = 0;
   dlist_for_each(s, tlv->s.t.childs[0], s.t.l) {
       if (s->service==e_controller)
           d->is_controller = 1;
       else if (s->service == 0x01)
           d->is_agent = 1;
   }
   return 0;
}

static int _processMAPProfileTLV(struct al_device *d, struct TLV *tlv)
{
    struct u8TLV *u8_tlv = (struct u8TLV *)tlv;

    d->profile = u8_tlv->v1;
    return 0;
}

static int _processDeviceInformationTypeTLV(struct al_device *d, struct TLV *tlv)
{
    int ret = -1;

    struct interfaceStruct *i_s;

    dlist_for_each(i_s, tlv->s.t.childs[0], s.t.l) {
        if ((i_s->media_type & MEDIA_TYPE_MAIN_MASK) == 0x0100) {
            wifiInterfaceAdd(d, NULL, i_s->mac);
        } else if ((i_s->media_type & MEDIA_TYPE_MAIN_MASK) == 0) {
            struct interface *intf;

            if (!(intf = interfaceFind(d, i_s->mac, interface_type_ethernet)))
                intf = interfaceNew(d, i_s->mac, sizeof(struct interface));

            if (!intf)
                goto bail;

            intf->type = interface_type_ethernet;
        } else
            DEBUG_WARNING("ignore media type 0x%x\n", i_s->media_type);

    }

    ret = 0;
bail:
if (ret)
        DEBUG_ERROR("error in _processDeviceInfomationTypeTLV\n");
    return ret;
}

static int _process1905NeighborDeviceTLV(struct al_device *d, struct TLV *tlv, int query)
{
    int ret = 0;
    struct interface *intf;
    struct macAddressTLV *intf_tlv = (struct macAddressTLV *)tlv;

    if ((intf = interfaceFind(d, intf_tlv->mac, interface_type_unknown))) {
        struct i1905NeighborStruct *neighbor_s;

        dlist_for_each(neighbor_s, tlv->s.t.childs[0], s.t.l) {
            struct neighbor *neigh;
            struct al_device *peer;

            if ((query) && (!(peer = alDeviceFind(neighbor_s->mac))))
                sendTopologyQuery(idx2InterfaceName(d->recv_intf_idx), getNextMid(), neighbor_s->mac);
            if ((neigh = neighborAdd(intf, neighbor_s->mac, NULL)))
                neigh->is_1905 = 1;
        }
    }
    return ret;
}

static int _processnon1905NeighborDeviceTLV(struct al_device *d, struct TLV *tlv)
{
    int ret = 0;
    struct interface *intf;
    struct macAddressTLV *intf_tlv = (struct macAddressTLV *)tlv;

    if ((intf = interfaceFind(d, intf_tlv->mac, interface_type_unknown))) {
        struct macStruct *neighbor_s;

        dlist_for_each(neighbor_s, tlv->s.t.childs[0], s.t.l) {
            neighborAdd(intf, neighbor_s->mac, NULL);
        }
    }
    return ret;
}

static int _processTransmitterLinkMetricTLV(struct transmitterLinkMetricTLV *tlv)
{
    struct al_device *from = NULL;
    struct al_device *to = NULL;
    struct transmitterLinkMetricEntriesStruct *entry = NULL;
    struct interface *intf = NULL;
    struct neighbor *n = NULL;
    int ret = -1;

    from = alDeviceFindAny(tlv->local_al_address);
    if (!from) {
        DEBUG_WARNING("Can not find tx local_al_address ["MACFMT"] device for link metric response\n", MACARG(tlv->local_al_address));
        return ret;
    }
    to = alDeviceFindAny(tlv->neighbor_al_address);
    if (!to) {
        DEBUG_WARNING("Can not find tx neighbor_al_address ["MACFMT"] device for link metric response\n", MACARG(tlv->neighbor_al_address));
        return ret;
    }
    dlist_for_each(entry, tlv->tlv.s.t.childs[0], s.t.l) {
        intf = interfaceFindMAC(from, entry->local_interface_address);
        if(!intf) {
            DEBUG_WARNING("Can not find tx local_al_address ["MACFMT"] device interface ["MACFMT"]\n", MACARG(tlv->local_al_address),
                MACARG(entry->local_interface_address));
            return ret;
        }
        n = neighborAdd(intf, tlv->neighbor_al_address, entry->neighbor_interface_address);
        if (n) {
            n->link_metric.tx_link_metrics = entry->tx_link_metrics;
        }
    }
    ret = 0;
    return ret;
}

static int _processReceiverLinkMetricTLV(struct receiverLinkMetricTLV *tlv)
{
    struct al_device *from = NULL;
    struct al_device *to = NULL;
    struct receiverLinkMetricEntriesStruct *entry = NULL;
    struct interface *intf = NULL;
    struct neighbor *n = NULL;
    int ret = -1;

    from = alDeviceFindAny(tlv->local_al_address);
    if (!from) {
        DEBUG_WARNING("Can not find rx local_al_address ["MACFMT"] device for link metric response\n", MACARG(tlv->local_al_address));
        return ret;
    }
    to = alDeviceFindAny(tlv->neighbor_al_address);
    if (!to) {
        DEBUG_WARNING("Can not find rx neighbor_al_address ["MACFMT"] device for link metric response\n", MACARG(tlv->neighbor_al_address));
        return ret;
    }
    dlist_for_each(entry, tlv->tlv.s.t.childs[0], s.t.l) {
        intf = interfaceFindMAC(from, entry->local_interface_address);
        if(!intf) {
            DEBUG_WARNING("Can not find rx local_al_address ["MACFMT"] device interface ["MACFMT"]\n", MACARG(tlv->local_al_address),
                MACARG(entry->local_interface_address));
            return ret;
        }
        n = neighborAdd(intf, tlv->neighbor_al_address, entry->neighbor_interface_address);
        if (n) {
            n->link_metric.rx_link_metrics = entry->rx_link_metrics;
        }
    }
    ret = 0;
    return ret;
}

static int _processAPOperationalBSSTLV(struct al_device *d, struct TLV *tlv)
{
    int ret = -1;
    struct macStruct *radio_s;
    struct interface *intf;
    struct wifi_interface *wintf;

    dlist_for_each(intf, d->interfaces, l) {
        if (intf->type == interface_type_wifi) {
            wintf = (struct wifi_interface *)intf;
            wintf->mark.new = wintf->mark.change= wintf->mark.hit=0;
        }
    }

    dlist_for_each(radio_s, tlv->s.t.childs[0], s.t.l) {
        struct radio *r = radioAdd(d, radio_s->mac);
        struct apOperationBSSStruct *bss_s;

        if (!r)
            goto bail;

        if ((!r->opclass) || (!r->channel))
            continue;

        dlist_for_each(bss_s, radio_s->s.t.childs[0], s.t.l) {
            if ((wintf = (struct wifi_interface *)interfaceFind(d, bss_s->bssid, interface_type_wifi))) {
                wintf->mark.hit = 1;
                if (SSIDCMP(wintf->bssInfo.ssid, bss_s->ssid)) {
                    SSIDCPY(wintf->bssInfo.ssid, bss_s->ssid);
                    MACCPY(wintf->bssInfo.bssid, bss_s->bssid);
                    wintf->mark.change = 1;
                }
                wintf->radio = r;

            } else {
                if (!(wintf = wifiInterfaceAdd(d, r, bss_s->bssid))) {
                    DEBUG_ERROR("bss("MACFMT") add failed.\n", MACARG(bss_s->bssid));
                    goto bail;
                }
                SSIDCPY(wintf->bssInfo.ssid, bss_s->ssid);
                MACCPY(wintf->bssInfo.bssid, bss_s->bssid);
                wintf->mark.new = 1;
            }
        }
    }

    updateBSS(d);

    ret = 0;
bail:
    if (ret)
        DEBUG_ERROR("error in _processAPOperationalBSSTLV\n");
    return ret;
}

static int _processVBSSConfigurationReportTLV(struct al_device *d, struct TLV *tlv)
{
    int ret = -1;
    struct macStruct *radio_s;

    dlist_for_each(radio_s, tlv->s.t.childs[0], s.t.l) {
        struct radio *r = radioAdd(d, radio_s->mac);
        struct apOperationVBSSStruct *vbss_s;

        if (!r)
            goto bail;

        dlist_for_each(vbss_s, radio_s->s.t.childs[0], s.t.l) {
            struct wifi_interface *wintf = wifiInterfaceAdd(d, r, vbss_s->bssid);
            if (!wintf)
                goto bail;
            wintf->is_vbss = true;
            SSIDCPY(wintf->bssInfo.ssid, vbss_s->ssid);
            MACCPY(wintf->bssInfo.bssid, vbss_s->bssid);
        }
    }
    ret = 0;
bail:
    if (ret)
        DEBUG_ERROR("error in _processVBSSConfigurationReportTLV\n");
    return ret;
}

static int _processAssociatedClientsTLV(struct al_device *d, struct TLV *tlv)
{
    int ret = -1;
    struct associatedBSSStruct *assoc_bss_s;
    uint32_t current = PLATFORM_GET_TIMESTAMP(0);

    dlist_for_each(assoc_bss_s, tlv->s.t.childs[0], s.t.l) {
        struct wifi_interface *wintf =
                (struct wifi_interface *)interfaceFind(d, assoc_bss_s->bssid, interface_type_wifi);
        if (wintf) {
            struct associatedClientStruct *assoc_client_s;
            dlist_for_each(assoc_client_s, assoc_bss_s->s.t.childs[0], s.t.l) {
                struct client *c = clientAdd(wintf, assoc_client_s->mac);

                if (!c)
                    goto bail;

                c->last_assoc_ts = current-assoc_client_s->age*1000;
            }
        }
    }
    ret = 0;
bail:
    if (ret)
        DEBUG_ERROR("error in _processAssociatedClientsTLV\n");
    return ret;
}

static int _processClientAssociationEventTLV(struct al_device *d, struct TLV *tlv)
{
    struct wifi_interface *ifw;
    struct clientAssocEvtTLV *evt_tlv = (struct clientAssocEvtTLV *)tlv;
    struct client *client;

    if ((ifw = (struct wifi_interface *)interfaceFind(d, evt_tlv->bssid, interface_type_wifi))) {
        if (evt_tlv->evt & CLIENT_ASSOC_EVT_JOINED) {
            /* if client move from local device to others send deauth */
            client = clientFind(local_device, NULL, evt_tlv->client);
            if (client) {
                DEBUG_INFO("Receive asso notification but client["MACFMT"] find local. send deauth: WLAN_REASON_DEAUTH_LEAVING\n", MACARG(evt_tlv->client));
                bssDeauth(client->wif, client, WLAN_REASON_DEAUTH_LEAVING);
            }
            clientAdd(ifw, evt_tlv->client);
            if (isRegistrar()) {
                sendClientAssoEvent(evt_tlv->client, ifw->i.mac);
                sendClientCapabilityQuery(idx2InterfaceName(d->recv_intf_idx), getNextMid(), d->al_mac,
                        evt_tlv->bssid, evt_tlv->client);
            }
        }
        else if ((client = clientFind(NULL, ifw, evt_tlv->client)))
            clientDelete(client);
    }

    return 0;
}

static void _channelUpdateNeighbor(struct chan_info *ch, struct scanNeighborStruct *neighbor_s)
{
    struct neighbor_bss *neighbor_item = NULL;
    if (!ch || !neighbor_s){
        DEBUG_WARNING("Input NULL pointer\n");
        return;
    }

    neighbor_item = calloc(1, sizeof(struct neighbor_bss));
    if (!neighbor_item) {
        DEBUG_ERROR("Internal Error.\n");
        return;
    }
    MACCPY(neighbor_item->bssid, neighbor_s->bssid);
    SSIDCPY(neighbor_item->ssid, neighbor_s->ssid);
    neighbor_item->signal_stength = neighbor_s->signal_strength;
    if (neighbor_s->bss_field & BSS_LOAD_ELE_PRESENT) {
        neighbor_item->bss_load_element_present = 1;
    }
    neighbor_item->bss_color = (neighbor_s->bss_field & BSS_COLOR_MASK);
    if(neighbor_item->bss_load_element_present) {
        struct bssLoadEleStruct *bssload;
        dlist_for_each(bssload, neighbor_s->s.t.childs[0], s.t.l) {
            neighbor_item->chan_utilize = bssload->chann_utilize;
            neighbor_item->sta_cnt = bssload->sta_cnt;
        }
    }
    dlist_add_tail(&ch->neighbor_list, &neighbor_item->l);
}

static int _processChannelResultTLV(struct al_device *d, struct scanResultTLV *result_tlv, char *tsstr, uint8_t len)
{
    struct radio *radio = NULL;
    struct operating_class *opc = NULL;
    struct chan_info *ch = NULL;
    int ret = -1;

    radio = radioFind(d, result_tlv->ruid);
    if (!radio) {
        DEBUG_WARNING("can NOT find radio["MACFMT"]\n", MACARG(result_tlv->ruid));
        goto bail;
    }
    if (tsstr && len)
    {
        PLATFORM_GET_TIMESTAMP_TIMEVAL(tsstr, &radio->chscan_result_ts);
    }

    opc = opclassFind(radio, result_tlv->opclass);
    if (!opc) {
        DEBUG_WARNING("can NOT find opclass(%d) for radio["MACFMT"]\n", result_tlv->opclass, MACARG(radio->uid));
        goto bail;
    }
    ch = channelFind(opc, result_tlv->channel);
    if(!ch) {
        DEBUG_WARNING("can NOT find ch[%d] in opc[%d] for radio["MACFMT"]\n", result_tlv->channel,
                    result_tlv->opclass, MACARG(radio->uid));
        goto bail;
    }
    ch->scan_status = result_tlv->scan_status;
    if(result_tlv->scan_status == SCAN_STATUS_SUCCESS) {
        struct scanResultStruct *result_s = NULL;
        struct scanNeighborStruct *neighbor_s = NULL;
        result_s = container_of(dlist_get_first(&result_tlv->tlv.s.t.childs[0]), struct scanResultStruct, s.t.l);
        if (result_s) {
            PLATFORM_GET_TIMESTAMP_TIMEVAL((char *)result_s->timestamp.data, &ch->start_scan_ts);
            ch->utilization = result_s->utilization;
            ch->avg_noise = result_s->noise;
            ch->chscan_dur = result_s->aggre_scan_dur;
            ch->chscan_type = result_s->scan_type;
            if (!dlist_empty(&ch->neighbor_list))
                dlist_free_items(&ch->neighbor_list, struct neighbor_bss, l);
            dlist_head_init(&ch->neighbor_list);
            dlist_for_each(neighbor_s, result_s->s.t.childs[0], s.t.l) {
                _channelUpdateNeighbor(ch, neighbor_s);
            }
        }
    }
    ret = 0;
bail:
    if (ret)
        DEBUG_ERROR("error in _processChannelResultTLV\n");
    return ret;
}

static void _resetRadioOperatingClass(struct radio *r)
{
    struct operating_class *opclass;
    int i;

    for (i=0;i<r->num_opc_support;i++) {
        opclass = &r->opclasses[i];
        resetOperatingClass(opclass, MOST_PREF_SCORE, REASON_NON_SPECIFIC);
    }

}

static void _resetRadioCACCapability(struct radio *r)
{
    int i,j;

    for (i=0;i<CAC_METHOD_MAX;i++)
        r->cac_capa[i].duration = -1;

    for (i=0;i<r->num_opc_support;i++) {
        struct operating_class *opclass = &r->opclasses[i];
        for (j=0;j<opclass->num_support_chan;j++) {
            struct chan_info *info = &opclass->channels[j];
            info->cac_method_mask = 0;
        }
    }
}

static uint8_t _processChannelPreferenceTLV(struct radio *r, struct macAddressTLV *tlv, uint8_t select)
{
    struct opClassStruct *item_opc;
    int i;
    struct operating_class *opc;
    uint8_t ret = 0;

    _resetRadioOperatingClass(r);

    dlist_for_each(item_opc, tlv->tlv.s.t.childs[0], s.t.l) {
        opc = opclassFind(r, item_opc->opclass);
        if (!opc) {
            DEBUG_WARNING("can NOT find opclass(%d) for radio["MACFMT"]\n",
                            item_opc->opclass, MACARG(r->uid));
            continue;
        }
        if (select) {
            if ((ret = checkChanSelRequestValid(opc, item_opc->non_operable_chans)))
                return ret;
        }
        if (0 == item_opc->non_operable_chans[0]) {
            resetOperatingClass(opc, GET_CHAN_PREF(item_opc->pref_reason),
                                    GET_CHAN_REASON(item_opc->pref_reason));
        } else {
            if (item_opc->non_operable_chans[0]) {
                for (i=0;i<item_opc->non_operable_chans[0];i++) {
                    struct chan_info *info;

                    if ((info = channelFind(opc, item_opc->non_operable_chans[1+i]))) {
                        info->pref = GET_CHAN_PREF(item_opc->pref_reason);
                        info->reason = GET_CHAN_REASON(item_opc->pref_reason);
                    }
                }
            }
        }
    }
    return ret;
}

static void _processRadioOperRestrictTLV(struct radio *r, struct macAddressTLV *op_rest)
{
    struct opRestOpclassStruct *item_opc;
    struct opRestChanStruct *item_ch;
    struct operating_class *opc;
    struct chan_info *ch;

    dlist_for_each(item_opc, op_rest->tlv.s.t.childs[0], s.t.l) {
        opc = opclassFind(r, item_opc->opclass);
        if (!opc) {
            DEBUG_WARNING("can NOT find opclass(%d) for radio["MACFMT"]\n", item_opc->opclass, MACARG(r->uid));
            continue;
        }
        dlist_for_each(item_ch, item_opc->s.t.childs[0], s.t.l) {
            ch = channelFind(opc, item_ch->channel);
            if (ch)
                ch->freq_separation = item_ch->freq_sep;
            else
                DEBUG_WARNING("can NOT find ch[%d] in opc[%d] for radio["MACFMT"]\n", item_ch->channel,
                                item_opc->opclass, MACARG(r->uid));
        }
    }
}

void _processUnassociatedStaLinkMetricsTLV(struct al_device *d, struct unassocStaLinkMetricsRspTLV *tlv)
{
    struct unassocMetricRspStruct *item;
    dlist_for_each(item, tlv->tlv.s.t.childs[0], s.t.l) {
        struct client *sta;

        if ((sta = clientFind(NULL, NULL, item->sta))) {
            struct radio *r;
            dlist_for_each(r, d->radios, l) {
                if (!validateOperatingChannel(r, tlv->opclass, item->channel)) {
                    struct sta_seen *seen = seenAdd(sta, r->uid);

                    if (seen) {
                        seen->opclass = tlv->opclass;
                        seen->channel = item->channel;
                        seen->rcpi_ul = item->rcpi_ul;
                        seen->rcpi_ul_ts = PLATFORM_GET_TIMESTAMP(item->age);
                    }
                }
            }
        }
    }
}

static void _processOperatingChannelReportTLV(struct radio *r, struct opChanReportTLV *rpt)
{
    struct opclassChanPairStruct *item;
    int idx;

    RESET_CURRENT_CHANNEL(r->current_opclass, r->current_channel);
    r->opclass = r->channel = 0;

    dlist_for_each(item, rpt->tlv.s.t.childs[0], s.t.l) {
        if (validateOperatingChannel(r, item->opclass, item->channel)) {
            DEBUG_WARNING("opclass=%d, channel=%d is invalid in radio["MACFMT"]\n",
                item->opclass, item->channel, MACARG(r->uid));
            continue;
        }
        idx = nlbw2Idx(opclass2nlBandwidth(item->opclass, NULL));
        r->current_opclass[idx] = item->opclass;
        r->current_channel[idx] = item->channel;
    }

    for (idx=bw_idx_320Mhz;idx>=0;idx--) {
        if ((r->current_opclass[idx]) && (r->current_channel[idx])) {
            r->opclass = r->current_opclass[idx];
            break;
        }
    }
    r->bw = idx;
    r->channel = r->current_channel[bw_idx_20MHz];
    r->tx_power = rpt->tx_power;
}

static void _processP2APCapabilityTLV(struct al_device *d, struct profile2ApCapabilityTLV *tlv)
{
    if ((tlv->max_vid) && (tlv->cap & P2_CAP_TRAFFIC_SEPARATION))
        d->max_vid = tlv->max_vid;
    d->count_units = (tlv->cap >> P2_CAP_BYTE_COUNTER_UNIT_OFFSET);
}

static int _processControllerWeightTLV(struct al_device *d, struct TLV *tlv)
{
    struct controllerWeightTLV *weight_tlv = (struct controllerWeightTLV *)tlv;

    d->controller_weight = weight_tlv->weight;
    return 0;
}

uint8_t processTopologyDiscovery(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct macAddressTLV *almac_tlv, *mac_tlv;
    struct interface *intf;

    struct al_device *d;

    almac_tlv = (struct macAddressTLV *)getTypedTLV(c, TLV_TYPE_AL_MAC_ADDRESS_TYPE, 0);
    if (!almac_tlv)
        return PROCESS_CMDU_KO;

    mac_tlv = (struct macAddressTLV *)getTypedTLV(c, TLV_TYPE_MAC_ADDRESS_TYPE, 0);
    if (!mac_tlv)
        return PROCESS_CMDU_KO;

    DM_CHECK_DEVICE(d, almac_tlv->mac, intf_idx);

    intf = interfaceFindIdx(intf_idx);
    if (!intf) {
        DEBUG_ERROR("Can not find local interface(idx=%d)\n", intf_idx);
        return PROCESS_CMDU_KO;
    }

    dmUpdateNeighbor(intf, almac_tlv->mac, mac_tlv->mac, IS_1905);

    return PROCESS_CMDU_OK;
}

uint8_t processLinkMetricQuery(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct linkMetricQueryTLV *tlv = NULL;
    uint8_t *dst_mac;
    struct al_device *d = alDeviceFindAny(src);
    uint8_t *neighbor_al_mac = NULL;

    if (!d) {
        DEBUG_WARNING("Unknown dst AL MAC, using src("MACFMT") for send ap capability report \n", MACARG(src));
        dst_mac = src;
    } else
        dst_mac = d->al_mac;
    tlv = (struct linkMetricQueryTLV *)getTypedTLV(c, TLV_TYPE_LINK_METRIC_QUERY, 0);

    if (!tlv) {
        DEBUG_ERROR("No TLV_TYPE_LINK_METRIC_QUERY in this CMDU\n");
        return PROCESS_CMDU_KO;
    }
    if (tlv->neighbor == LINK_METRIC_QUERY_TLV_ALL_NEIGHBORS) {
        DEBUG_DETAIL("tlv->neighbor is all neighbor\n");
    } else if (tlv->neighbor == LINK_METRIC_QUERY_TLV_SPECIFIC_NEIGHBOR) {
        DEBUG_DETAIL("tlv->neighbor is specific neighbor["MACFMT"]\n", MACARG(tlv->specific_neighbor));
        neighbor_al_mac = tlv->specific_neighbor;
    } else {
        DEBUG_ERROR("Unexpected 'neighbor': %d\n", tlv->neighbor);
        return PROCESS_CMDU_KO;
    }
    if (tlv->link_metrics_type == TX_LINK_METRICS_ONLY || tlv->link_metrics_type == RX_LINK_METRICS_ONLY
        || tlv->link_metrics_type == TX_AND_RX_LINK_METRICS) {
        DEBUG_DETAIL("Type is %d\n", tlv->link_metrics_type);
    } else {
        DEBUG_ERROR("Unexpected 'type': %d\n", tlv->link_metrics_type);
        return PROCESS_CMDU_KO;
    }
    if (0 == sendLinkMetricsResponse(idx2InterfaceName(intf_idx), c->id, dst_mac, neighbor_al_mac, tlv->link_metrics_type)) {
        DEBUG_WARNING("Could not send 'link metric response' message\n");
    }
    return PROCESS_CMDU_OK;
}

uint8_t processLinkMetricResponse(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct al_device *d = NULL;
    struct transmitterLinkMetricTLV *txlinkmetric_tlv = NULL;
    struct receiverLinkMetricTLV *rxlinkmetric_tlv = NULL;
    uint8_t i = 0;

    d = alDeviceFindAny(src);
    if (!d) {
        DEBUG_WARNING("Can not find device ["MACFMT"] for link metric response\n", MACARG(src));
        return PROCESS_CMDU_KO;
    }

    i = 0;
    while((txlinkmetric_tlv = (struct transmitterLinkMetricTLV *)getTypedTLV(c, TLV_TYPE_TRANSMITTER_LINK_METRIC, i++))) {
        _processTransmitterLinkMetricTLV(txlinkmetric_tlv);
    }
    i = 0;
    while((rxlinkmetric_tlv = (struct receiverLinkMetricTLV *)getTypedTLV(c, TLV_TYPE_RECEIVER_LINK_METRIC, i++))) {
        _processReceiverLinkMetricTLV(rxlinkmetric_tlv);
    }
    return PROCESS_CMDU_OK;
}

uint8_t processAPAutoconfigurationSearch(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct macAddressTLV *mac_tlv;
    struct u8TLV *u8_tlv;
    struct TLV *tlv;
    struct serviceStruct *service_s;
    struct al_device *d;

    mac_tlv = (struct macAddressTLV *)getTypedTLV(c, TLV_TYPE_AL_MAC_ADDRESS_TYPE, 0);
    if (!mac_tlv)
        return PROCESS_CMDU_KO;

    DM_CHECK_DEVICE(d, mac_tlv->mac, intf_idx);

    u8_tlv = (struct u8TLV *)getTypedTLV(c, TLV_TYPE_SEARCHED_ROLE, 0);
    if ((!u8_tlv) || (u8_tlv->v1!=0x00))
        return PROCESS_CMDU_KO;

    u8_tlv = (struct u8TLV *)getTypedTLV(c, TLV_TYPE_AUTOCONFIG_FREQ_BAND, 0);
    if (!u8_tlv)
        return PROCESS_CMDU_KO;

    if ((tlv = getTypedTLV(c, TLV_TYPE_MULTIAP_PROFILE, 0)))
        _processMAPProfileTLV(d, tlv);

    if ((tlv = getTypedTLV(c, TLV_TYPE_SUPPORTED_SERVICE, 0)))
        _processSupportServiceTLV(d, tlv);

    if (d->is_controller) {
        if ((tlv = getCLSTypedTLV(c, CLS_TLV_TYPE_CONTROLLER_WEIGHT, 0)))
            _processControllerWeightTLV(d, tlv);
        else
            d->controller_weight = 255;

        if (!registrar && local_config.auto_role &&
            (local_device->controller_weight < d->controller_weight)) {
            DEBUG_INFO("local controller_weight(%u) < d("MACFMT"): controller_weight(%u) cancel auto role\n",
                local_device->controller_weight, MACARG(d->al_mac), d->controller_weight);
            PLATFORM_CANCEL_TIMER(local_device->autorole_timer);
        }
    }

    tlv = getTypedTLV(c, TLV_TYPE_SEARCHED_SERVICE, 0);
    if (!tlv)
        return PROCESS_CMDU_KO;
    service_s = (struct serviceStruct *)getChildTLVStruct(&tlv->s, 0, 0);
    if ((!service_s) || (service_s->service!=e_controller))
        return PROCESS_CMDU_KO;

    if (!isRegistrar())
        return PROCESS_CMDU_OK;

    /* auto controller receive other auto role device search change to agent */
    if (local_config.auto_role &&
        (local_device->controller_weight < d->controller_weight)) {
        DEBUG_INFO("local controller_weight(%u) < d("MACFMT"): controller_weight(%u) change from Controller to Agent\n",
                local_device->controller_weight, MACARG(d->al_mac), d->controller_weight);
        struct mesh_ubus_event_request req;
        req.u.role_change.old_role = 1; // controller
        req.u.role_change.new_role = 2; // agent
        sendUbusEvent(EVENT_ROLE_CHANGE, &req);
        setRegistrar(NULL);
        return PROCESS_CMDU_OK;
    }

    if (d->inited_map_policy)
        d->inited_map_policy = 0;

    if (0 == sendAPAutoconfigurationResponse(idx2InterfaceName(intf_idx), c->id, mac_tlv->mac, u8_tlv->v1)) {
        DEBUG_WARNING("Could not send 'AP autoconfiguration response' message\n");
    }

    return PROCESS_CMDU_OK;
}

uint8_t processAPAutoconfigurationResponse(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    uint8_t ret = PROCESS_CMDU_OK;
    struct TLV *tlv;
    struct serviceStruct *service_s;
    uint8_t is_controller = 0;
    int i = 0;
    struct al_device *d;

    tlv = getTypedTLV(c, TLV_TYPE_SUPPORTED_SERVICE, 0);
    if (!tlv)
        return PROCESS_CMDU_KO;

    while ((service_s = (struct serviceStruct *)getChildTLVStruct(&tlv->s, 0, (i++)))) {
        if (service_s->service == e_controller) {
            is_controller = 1;
            break;
        }
    }

    if (!is_controller)
        return PROCESS_CMDU_KO;

    d = alDeviceFindAny(src);

    if (!d) {
        sendTopologyQuery(idx2InterfaceName(intf_idx), getNextMid(), src);
        return ret;
    }

    if ((tlv = getCLSTypedTLV(c, CLS_TLV_TYPE_CONTROLLER_WEIGHT, 0)))
        _processControllerWeightTLV(d, tlv);
    else
        d->controller_weight = 255;

    if ((tlv = getTypedTLV(c, TLV_TYPE_MULTIAP_PROFILE, 0)))
        _processMAPProfileTLV(d, tlv);

    /* controller detected other controllers if local is auto role change to agent,
     * if local is not auto role send event to upper
     */
    if (isRegistrar()) {
        if (!local_config.auto_role) {
            DEBUG_INFO("Detected other Controller send event!\n");
            goto detected_other_controller;
        } else {
            if (local_device->controller_weight > d->controller_weight) {
                DEBUG_INFO("local controller_weight(%u) > d("MACFMT"): controller_weight(%u) send event!\n",
                    local_device->controller_weight, MACARG(d->al_mac), d->controller_weight);
                goto detected_other_controller;
            } else {
                DEBUG_INFO("local controller_weight(%u) <= d("MACFMT"): controller_weight(%u) change to agent!\n",
                    local_device->controller_weight, MACARG(d->al_mac), d->controller_weight);
                goto bail;
            }
        }
    } else {
        goto bail;
    }

detected_other_controller:
    struct mesh_ubus_event_request req;
    MACCPY(req.u.other_controller.al_mac, d->al_mac);
    sendUbusEvent(EVENT_DETECT_OTHER_CONTROLLER, &req);
    return PROCESS_CMDU_KO;

bail:
    ret = PROCESS_CMDU_OK_TRIGGER_CONTROLLER_SEARCH;
    setRegistrar(d);
    return ret;
}

uint8_t processAPAutoconfigurationRenew(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    uint8_t ret = PROCESS_CMDU_OK_TRIGGER_CONTROLLER_SEARCH;
    struct macAddressTLV *mac_tlv;
    struct al_device *d;

    if (isRegistrar())
        return PROCESS_CMDU_KO;

    if (!(mac_tlv = (struct macAddressTLV *)getTypedTLV(c, TLV_TYPE_AL_MAC_ADDRESS_TYPE, 0)))
        return PROCESS_CMDU_KO;

    if ((!(d = alDeviceFind(mac_tlv->mac))) || (registrar != NULL && d!=registrar))
        return PROCESS_CMDU_KO;

    alDeviceSetConfigured(local_device, 0);

    if (local_config.auto_role && (local_device->controller_weight <= d->controller_weight))
        PLATFORM_CANCEL_TIMER(local_device->autorole_timer);

    return ret;
}

uint8_t processHigherLayerQuery(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    uint8_t ret = PROCESS_CMDU_OK;

    sendHigherLayerResponse(idx2InterfaceName(intf_idx), c->id, src);

    return ret;
}

uint8_t processHigherLayerResponse(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct macAddressTLV *mac_tlv;
    struct al_device *d;
    struct deviceIDTLV *deviceid_tlv;
    struct TLV *ip_tlv;

    if (!(mac_tlv = (struct macAddressTLV *)getTypedTLV(c, TLV_TYPE_AL_MAC_ADDRESS_TYPE, 0)))
        return PROCESS_CMDU_KO;

    if (!(d = alDeviceFind(mac_tlv->mac)))
        return PROCESS_CMDU_KO;

    if ((deviceid_tlv = (struct deviceIDTLV *)getTypedTLV(c, TLV_TYPE_DEVICE_IDENTIFICATION, 0))) {
        struct device_info *info = &d->device_info;

        if (deviceid_tlv->friendly_name[0] != '\0')
            REPLACE_STR(info->friendly_name, strdup(deviceid_tlv->friendly_name));
        if (deviceid_tlv->manufacturer[0] != '\0')
            REPLACE_STR(info->manufacturer, strdup(deviceid_tlv->manufacturer));
        if (deviceid_tlv->model_name[0] != '\0')
            REPLACE_STR(info->model_name, strdup(deviceid_tlv->model_name));
    }

    if ((ip_tlv = (struct TLV *)getTypedTLV(c, TLV_TYPE_IPV4, 0))) {
        struct macStruct *ipv4_s;
        struct interface *intf;

        dlist_for_each(intf, d->interfaces, l) {
            dlist_free_items(&intf->ipv4s, struct ipv4_item, l);
        }
        dlist_for_each(ipv4_s, ip_tlv->s.t.childs[0], s.t.l) {
            if ((intf = interfaceFind(d, ipv4_s->mac, interface_type_unknown))) {
                struct ipv4AddrStruct *ipv4_ss;

                dlist_for_each(ipv4_ss, ipv4_s->s.t.childs[0], s.t.l) {
                    struct ipv4_item *ipv4 = malloc(sizeof(struct ipv4_item));

                    ipv4->proto = ipv4_ss->proto;
                    ipv4->ip = ipv4_ss->ip;
                    ipv4->dhcp_server = ipv4_ss->dhcp_ip;
                    dlist_add_tail(&intf->ipv4s, &ipv4->l);
                }
            }
        }
    }

    if ((ip_tlv = (struct TLV *)getTypedTLV(c, TLV_TYPE_IPV6, 0))) {
        struct interfaceipv6Struct *ipv6_s;
        struct interface *intf;

        dlist_for_each(intf, d->interfaces, l) {
            dlist_free_items(&intf->ipv6s, struct ipv6_item, l);
        }
        dlist_for_each(ipv6_s, ip_tlv->s.t.childs[0], s.t.l) {
            if ((intf = interfaceFind(d, ipv6_s->mac, interface_type_unknown))) {
                struct ipv6AddrStruct *ipv6_ss;

                intf->local_ipv6 = ipv6_s->local_ip;
                dlist_for_each(ipv6_ss, ipv6_s->s.t.childs[0], s.t.l) {
                    struct ipv6_item *ipv6 = malloc(sizeof(struct ipv6_item));

                    ipv6->proto = ipv6_ss->proto;
                    ipv6->ip = ipv6_ss->ip;
                    ipv6->ip_origin = ipv6_ss->origin_ip;
                    dlist_add_tail(&intf->ipv6s, &ipv6->l);
                }
            }
        }
    }

    return PROCESS_CMDU_OK;
}

uint8_t wifiConfigMatch(struct wifi_config *wcfg, struct wsc_m1_info *info)
{
   return (wcfg->bands & info->bands);
}

int _handleClsCapTLV(struct al_device *d, struct TLV *tlv)
{
    struct clsCapTLV *cap_tlv = (struct clsCapTLV *)tlv;

    if (cap_tlv->cap.len>=1) {
        d->cls_cap.vip_max = cap_tlv->cap.datap[0];
        DEBUG_INFO("agent["MACFMT"] support %d vips.\n", MACARG(d->al_mac), d->cls_cap.vip_max);
    }
    return 0;
}

static uint8_t processAPAutoconfiguratioWSCM1(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct wsc_m1_info info = {0};
    struct wscTLV *m1 = (struct wscTLV *)getTypedTLV(c, TLV_TYPE_WSC, 0);
    struct TLV *m2, *tlv;
    struct radioBasicCapabilityTLV *radio_basic_capa;
    struct wifi_config *wcfg;
    struct al_device *d;
    struct radio *r;
    DEFINE_DLIST_HEAD(m2s);

    if (!m1)
        return PROCESS_CMDU_KO;

    wscProcessM1((struct TLV *)m1, &info);
    if ((!info.al_mac) || (!info.nonce) || (!info.pub))
        return PROCESS_CMDU_KO;

    DM_CHECK_DEVICE(d, info.al_mac, intf_idx);

    radio_basic_capa = (struct radioBasicCapabilityTLV *)getTypedTLV(c, TLV_TYPE_AP_RADIO_BASIC_CAPABILITIES, 0);
    if (!radio_basic_capa)
        return PROCESS_CMDU_KO;

    {
        if ((tlv = getTypedTLV(c, TLV_TYPE_PROFILE2_AP_CAPABILITY, 0)))
            _processP2APCapabilityTLV(d, (struct profile2ApCapabilityTLV *)tlv);
    }

    r = radioAdd(d, radio_basic_capa->rid);
    _radioUpdateBasicCapability(r, radio_basic_capa);

    dlist_for_each(wcfg, local_config.wifi_config.bsses, l) {
        if ((wcfg->bss.role==role_ap) && (wifiConfigMatch(wcfg, &info))) {
            m2 = wscBuildM2(wcfg, &info);
            if (m2)
                dlist_add_tail(&m2s, &m2->s.t.l);
        }
    }
    if (dlist_empty(&m2s)) {
        // build tear down m2
        m2 = wscBuildM2(NULL, &info);
        dlist_add_tail(&m2s, &m2->s.t.l);
    }

    {
        struct TLV *cap_tlv;
        if ((cap_tlv = getCLSTypedTLV(c, CLS_TLV_TYPE_CLS_CAPABILITIES, 0))) {
            _handleClsCapTLV(d, cap_tlv);
        }
    }

    int old_status = d->status;
    sendAPAutoconfigurationWSCM2(idx2InterfaceName(intf_idx), c->id, d->al_mac, r, &m2s);

    // when agent onboading. send initiated map policy
    if (!d->inited_map_policy) {
        sendMapPolicyConfigRequest(idx2InterfaceName(d->recv_intf_idx), getNextMid(), d->al_mac, &local_policy);
        d->inited_map_policy = 1;
    }

    if (local_policy.vbss_conf.enable && (old_status != d->status && d->status == STATUS_CONFIGURED)) {
        /* Agent status changed to CONFIGURED then send vbss capability request */
        DEBUG_INFO("send VBSS Capability Request to Agent("MACFMT")\n", MACARG(d->al_mac));
        sendVBSSCapabilitiesRequest(idx2InterfaceName(intf_idx), getNextMid(), d->al_mac);
    }

    return PROCESS_CMDU_OK;
}


int compareVlan(dlist_item *target, dlist_item *src)
{
    struct vlan_config_item *config = container_of(target, struct vlan_config_item, l2.l);
    struct trafficPolicyStruct *policy = container_of(src, struct trafficPolicyStruct, s.t.l);

    return (SSIDCMP(config->ssid, policy->ssid) || (config->vlan!=policy->vlanid));
}

dlist_item *newVlanConfig(dlist_item *src)
{
    struct trafficPolicyStruct *policy = container_of(src, struct trafficPolicyStruct, s.t.l);
    struct vlan_config_item *config = calloc(1, sizeof(struct vlan_config_item));

    SSIDCPY(config->ssid, policy->ssid);
    config->vlan = policy->vlanid;

    return &config->l2.l;
}

int processTrafficSeparation(struct default80211QSetsTLV *qos, struct TLV *vlan)
{
    int ret = 0;

    if ((vlan) && (!qos))
        return -1;

    if (!qos) {
        if (local_policy.def_vlan!=0) {
            local_policy.def_vlan = 0;
            ret = 1;
        }
    } else {
        if (qos->vlanid!=local_policy.def_vlan) {
            local_policy.def_vlan = qos->vlanid;
            ret = 1;
        }
    }
    if (!vlan) {
        if (!dlist_empty(&local_policy.vlans)) {
            dlist_free_items(&local_policy.vlans, struct vlan_config_item, l2.l);
            ret = 1;
        }
    } else {
        ret = listMerge(&local_policy.vlans, &vlan->s.t.childs[0], compareVlan, newVlanConfig, NULL);
    }
    return ret;
}

int _handleVipStaTLV(struct TLV *vip_tlv)
{
    struct macStruct *vip_s;
    struct mac_item *vip_local;

    if (!vip_tlv)
        return -1;

    dlist_free_items(&local_policy.vips, struct mac_item , l);
    if (dlist_empty(&vip_tlv->s.t.childs[0]))
        goto action;

    dlist_for_each(vip_s, vip_tlv->s.t.childs[0], s.t.l) {
        vip_local = calloc(1, sizeof(struct mac_item));
        if (!vip_local) {
            DEBUG_ERROR("no space for VIP STA TLV handle\n");
            return -1;
        }
        MACCPY(vip_local->mac, vip_s->mac);
        dlist_add_tail(&local_policy.vips, &vip_local->l);
    }
action:
    doVipAction();
    return 0;
}

int _handleMappingTLV(struct TLV *mapping_tlv)
{
    struct DSCPMappingStruct *mapping;
    struct DSCPMappingTLV *parent = NULL;
    struct DSCP_mapping_item *local;

    parent = container_of(mapping_tlv, struct DSCPMappingTLV, tlv);
    dlist_for_each(mapping, mapping_tlv->s.t.childs[0], s.t.l) {
        local = calloc(1, sizeof(struct DSCP_mapping_item));
        if (!local) {
            DEBUG_WARNING("no space for tc mapping conf\n");
            return -1;
        }
        local->dscp_value = mapping->dscp_value;
        local->tid = mapping->wmm_tid;
        local->queue_id = mapping->swq_id;
        dlist_add_tail(&local_policy.dscp_conf.dscp_list, &local->l);
    }

    if ((parent->dft_swq != 255) && (parent->dft_tid != 255)) {
        local_policy.dscp_conf.dft_tid = parent->dft_tid;
        local_policy.dscp_conf.dft_qid = parent->dft_swq;
    }
    doMappingConfAction(&local_policy.dscp_conf);
    return 0;
}

int _handleQueueTLV(struct TLV *queue_tlv)
{
    struct swQueueStruct *queue;
    struct queue_conf_item *item;

    if (dlist_empty(&queue_tlv->s.t.childs[0])) {
        DEBUG_WARNING("wrong sw queue config num\n");
        return -1;
    }

    dlist_for_each(queue, queue_tlv->s.t.childs[0], s.t.l) {
        item = calloc(1, sizeof(struct queue_conf_item));
        if (!item) {
            DEBUG_WARNING("no space for storing VIP queue conf\n");
            return -1;
        }
        item->port_id = queue->port;
        item->queue_id = queue->qid;
        item->weight = queue->weight;
        dlist_add_tail(&local_policy.queue_conf, &item->l);
    }
    doQueueConfAction(&local_policy.queue_conf);
    return 0;
}

int _handleTcTLV(struct TLV *mapping_tlv)
{
    struct TcMappingStruct *mapping;
    struct tc_mapping_item *local;
    struct TcMappingTLV *parent = NULL;

    parent = container_of(mapping_tlv, struct TcMappingTLV, tlv);
    dlist_for_each(mapping, mapping_tlv->s.t.childs[0], s.t.l) {
        local = calloc(1, sizeof(struct tc_mapping_item));
        if (!local) {
            DEBUG_WARNING("no space for tc mapping conf\n");
            return -1;
        }
        local->tc_value = mapping->tc;
        local->tid = mapping->tid;
        local->queue_id = mapping->qid;
        dlist_add_tail(&local_policy.tc_conf.mapping_list, &local->l);
    }

    if ((parent->dft_qid != 255) && (parent->dft_tid != 255)) {
        local_policy.tc_conf.dft_tid = parent->dft_tid;
        local_policy.tc_conf.dft_qid = parent->dft_qid;
    }

    doTcConfAction(&local_policy.tc_conf);

    return 0;
}

void wifiConfigApplyTrafficSeperation(dlist_head *bsses)
{
    struct wifi_config *config;
    struct vlan_config_item *vlan;

    dlist_for_each(config, *bsses, l) {
        if (config->bss.backhaul) {
            int i = 1;

            config->bss.vlan_map[0] = local_policy.def_vlan;
            dlist_for_each(vlan, local_policy.vlans, l2.l) {
                if (local_policy.def_vlan!=vlan->vlan) {
                    config->bss.vlan_map[i++] = vlan->vlan;
                }
                if (i>=VLAN_MAX)
                    break;
            }
        } else {
            dlist_for_each(vlan, local_policy.vlans, l2.l) {
                if (!SSIDCMP(config->bss.ssid, vlan->ssid)) {
                    config->bss.vlan_map[0] = vlan->vlan;
                }
            }
        }
    }
}

static uint8_t processAPAutoconfiguratioWSCM2(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct radio *r;
    int i = 0;
    struct wifi_config *wcfg;
    struct bss_info *backhaul_bss = NULL;

    DEFINE_DLIST_HEAD(bsses);
    struct macAddressTLV *radio_identifier = (struct macAddressTLV *)getTypedTLV(c, TLV_TYPE_AP_RADIO_IDENTIFIER, 0);
    struct TLV *m2;
    bool skip_qos_table = false;
    struct radio *local_radio;
    struct default80211QSetsTLV *qos=NULL;
    struct TLV *vlan = NULL;
    int need_set_vlan = 0;


    if (!radio_identifier)
        return PROCESS_CMDU_KO;

    if (!(r = radioFind(local_device, radio_identifier->mac)))
        return PROCESS_CMDU_KO;

    if (!r->wsc)
        return PROCESS_CMDU_KO;

    while ((m2 = getTypedTLV(c, TLV_TYPE_WSC, i++))) {
        uint8_t teardown = 0;
        wcfg = wscProcessM2(m2, r, &teardown);
        if (wcfg) {
            if (wcfg->bss.backhaul)
                backhaul_bss = &wcfg->bss;
            wcfg->bands = r->bands;
            dlist_add_tail(&bsses, &wcfg->l);
        } else {
            if (teardown) {
                DEBUG_INFO("teardown "MACFMT"\n", MACARG(radio_identifier->mac));
                break;
            }
            else
                return PROCESS_CMDU_KO;
        }
    }

    updateLocalWifiConfig(r, &bsses);

    r->configured = 1;
    if ((qos = (struct default80211QSetsTLV *)getTypedTLV(c, TLV_TYPE_DEFAULT_8021Q_SETTINGS, 0))) {
        vlan = getTypedTLV(c, TLV_TYPE_TRAFFIC_SEPARATION_POLICY, 0);
    }
    need_set_vlan = processTrafficSeparation(qos, vlan);

    if (need_set_vlan>0) {
        DEBUG_INFO("reset etherent vlan setting\n");
        ethVlanSet();
    } else
        DEBUG_INFO("vlan setting unchanged\n");

    wifiConfigApplyTrafficSeperation(&bsses);
    {
        struct interface *intf = interfaceFindIdx(intf_idx);
        uint8_t sync_sta = 0;

        if (!intf) {
            DEBUG_ERROR("can not find m2 recv interface info\n");
            return PROCESS_CMDU_KO;
        }

        if ((local_config.sync_sta) && (intf->type==interface_type_ethernet))
            sync_sta = 1;

        radioApplyBsses(r, &bsses, backhaul_bss, sync_sta);
    }
    dlist_free_items(&bsses, struct wifi_config, l);

    if (r->wsc) {
        //clean up wsc context
        if (r->wsc->priv_key)
            free(r->wsc->priv_key);
        if (r->wsc->m1)
            TLVFree(r->wsc->m1);
        free(r->wsc);
        r->wsc = NULL;
    }

    {
        struct TLV *vips =  getCLSTypedTLV(c, CLS_TLV_TYPE_VIP_CONF, 0);
        if (vips)
            _handleVipStaTLV(vips);
    }

    dlist_for_each(local_radio, local_device->radios, l) {
        if (local_radio->configured) {
            if (MACCMP(local_radio->uid, r->uid)) { /* this means one WSC-M2 is already received, qos table is configured */
                skip_qos_table = true;
                break;
            }
        }
    }

    if (false == skip_qos_table) { /* cls-qos config is global config, not belong to vap */
        {
            struct TLV *mapping_conf =  getCLSTypedTLV(c, CLS_TLV_TYPE_DSCP_MAPPING_CONF, 0);
            if (mapping_conf)
                _handleMappingTLV(mapping_conf);
        }

        {
            struct TLV *queue_conf =  getCLSTypedTLV(c, CLS_TLV_TYPE_EGRESS_CONF, 0);
            if (queue_conf)
                _handleQueueTLV(queue_conf);
        }

        {
            struct TLV *tc_conf =  getCLSTypedTLV(c, CLS_TLV_TYPE_TC_MAPPING_CONF, 0);
            if (tc_conf)
                _handleTcTLV(tc_conf);
        }
    }
    return PROCESS_CMDU_OK_TRIGGER_CONTROLLER_SEARCH;
}

uint8_t processAPAutoconfiguratioWSC(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    uint8_t i = 0, wsc_type;
    struct wscTLV *wsc_tlv;
    uint8_t ret = PROCESS_CMDU_OK;

    wsc_tlv = (struct wscTLV *)getTypedTLV(c, TLV_TYPE_WSC, i++);
    if (!wsc_tlv) {
        DEBUG_ERROR("Malformed structure.");
        return PROCESS_CMDU_KO;
    }

    wsc_type = wscGetType(wsc_tlv->wsc.datap, wsc_tlv->wsc.len);

    while ((wsc_tlv = (struct wscTLV *)getTypedTLV(c, TLV_TYPE_WSC, i++))) {

        if (wscGetType(wsc_tlv->wsc.datap, wsc_tlv->wsc.len)!=wsc_type) {
            DEBUG_ERROR("wsc type is not consist, %d!=%d\n",
                             wscGetType(wsc_tlv->wsc.datap, wsc_tlv->wsc.len), wsc_type);
            return PROCESS_CMDU_KO;
        }
    }

    if ((wsc_type==WPS_M1) && (isRegistrar())) {
        ret = processAPAutoconfiguratioWSCM1(c, intf_idx, src, dst);
    } else if ((wsc_type==WPS_M2) && (!isRegistrar())) {
        ret = processAPAutoconfiguratioWSCM2(c, intf_idx, src, dst);
    } else {
        DEBUG_ERROR("wsc type unknown(%d)\n", wsc_type);
        return PROCESS_CMDU_KO;
    }

    return ret;
}

uint8_t processAPCapabilityQuery(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    uint8_t *dst_mac;
    struct al_device *d = alDeviceFindAny(src);

    if (!d) {
        DEBUG_WARNING("Unknown dst AL MAC, using src("MACFMT") for send ap capability report \n", MACARG(src));
        dst_mac = src;
    } else
        dst_mac = d->al_mac;

    if (0 == sendAPCapabilityReport(idx2InterfaceName(intf_idx), c->id, dst_mac)) {
        DEBUG_WARNING("Could not send packet\n");
    }

    return PROCESS_CMDU_OK;
}

static void _handleCACCapabilitiesTLV(struct al_device *d, struct cacCapaTLV *cac)
{
    struct macStruct *radio_s;
    struct cacTypeStruct *cactype_s;
    struct cacCapaOpclassStruct *opc_s;
    struct radio *r;
    struct operating_class *opc;
    struct chan_info *ch;
    int i;

    d->country_code[0] = (uint8_t)(cac->cn_code & 0x00ff);
    d->country_code[1] = (uint8_t)(cac->cn_code & 0xff00);

    dlist_for_each(radio_s, cac->tlv.s.t.childs[0], s.t.l) {
        r = radioFind(d, radio_s->mac);
        if (!r) {
            DEBUG_WARNING("can NOT find radio"MACFMT"\n", MACARG(radio_s->mac));
            continue;
        }

        _resetRadioCACCapability(r);

        dlist_for_each(cactype_s, radio_s->s.t.childs[0], s.t.l) {
            if (cactype_s->method >= CAC_METHOD_MAX)
                continue;

            r->cac_capa[cactype_s->method].duration = GET_DUR_VALUE(cactype_s->dur);
            dlist_for_each(opc_s, cactype_s->s.t.childs[0], s.t.l) {
                opc = opclassFind(r, opc_s->opclass);
                if (!opc) {
                    DEBUG_WARNING("can NOT find opclass(%d) for radio["MACFMT"]\n", opc_s->opclass, MACARG(r->uid));
                    continue;
                }

                for (i=1;i<=opc_s->chan[0];i++) {
                    ch = channelFind(opc, opc_s->chan[i]);
                    if(!ch) {
                        DEBUG_WARNING("can NOT find ch[%d] in opc[%d] for radio["MACFMT"]\n", opc_s->chan[i], opc_s->opclass, MACARG(r->uid));
                        continue;
                    }
                    ch->cac_method_mask |= BIT(cactype_s->method);
                }
            }
        }
    }
    return;
}

uint8_t processAPCapabilityReport(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct al_device *d = alDeviceFindAny(src);
    int i;
    struct TLV *tlv;

    if (!d) {
        DEBUG_WARNING("can not find device for ap capability report\n");
        return PROCESS_CMDU_KO;
    }

    d->ap_basic_capaed = 1;

    {
        struct u8TLV *apcapa = (struct u8TLV *)getTypedTLV(c, TLV_TYPE_AP_CAPABILITY, 0);
        if (!apcapa)
            return PROCESS_CMDU_KO;
        d->ap_capability = apcapa->v1;
    }

    {
        struct radioBasicCapabilityTLV *basic_capa;
        i = 0;

        while ((basic_capa = (struct radioBasicCapabilityTLV *)getTypedTLV(c, TLV_TYPE_AP_RADIO_BASIC_CAPABILITIES, i++))) {
            struct radio *r = radioAdd(d, basic_capa->rid);
            _radioUpdateBasicCapability(r, basic_capa);
            if (!r->channel || !r->opclass) {
                d->ap_basic_capaed = 0;
                DEBUG_WARNING("r->opclass(%u) or r->channel(%u) is 0, set ap_basic_capaed 0\n", r->opclass, r->channel);
            }
        }
    }

    {
        if ((tlv = getTypedTLV(c, TLV_TYPE_PROFILE2_AP_CAPABILITY, 0)))
            _processP2APCapabilityTLV(d, (struct profile2ApCapabilityTLV *)tlv);
    }

    {
        struct htCapabilityTLV *ht;
        i = 0;

        while ((ht = (struct htCapabilityTLV *)getTypedTLV(c, TLV_TYPE_AP_HT_CAPABILITIES, i++))) {
            struct radio *r = radioAdd(d, ht->rid);
            r->bands_capa[r->current_band_idx].ht_capa_valid = 1;
            r->bands_capa[r->current_band_idx].ht_capa = ht->capa;
        }
    }

    {
        struct vhtCapabilityTLV *vht;
        i = 0;

        while ((vht = (struct vhtCapabilityTLV *)getTypedTLV(c, TLV_TYPE_AP_VHT_CAPABILITIES, i++))) {
            struct radio *r = radioAdd(d, vht->rid);
            r->bands_capa[r->current_band_idx].vht_capa_valid = 1;
            r->bands_capa[r->current_band_idx].vht_capa = vht->capa;
        }
    }

    {
        struct heCapabilityTLV *he;
        i = 0;

        while ((he = (struct heCapabilityTLV *)getTypedTLV(c, TLV_TYPE_AP_HE_CAPABILITIES, i++))) {
            struct radio *r = radioAdd(d, he->rid);
            r->bands_capa[r->current_band_idx].he_capa_valid = 1;
            r->bands_capa[r->current_band_idx].he_capa = he->capa;
        }
    }

    {
        struct cacCapaTLV *cac;

        if ((cac = (struct cacCapaTLV *)getTypedTLV(c, TLV_TYPE_CAC_CAPABILITIES, 0))) {
            _handleCACCapabilitiesTLV(d, cac);
        }
    }

    {
        struct TLV *cap_tlv;

        if ((cap_tlv = getCLSTypedTLV(c, CLS_TLV_TYPE_CLS_CAPABILITIES, 0))) {
            _handleClsCapTLV(d, cap_tlv);
        }
    }

    return PROCESS_CMDU_OK;
}

uint8_t processChannelPreferenceQuery(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    return sendChannelPreferenceReport(idx2InterfaceName(intf_idx), c->id, dst, local_device);
}

uint8_t processChannelPreferenceReport(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    int i = 0, k = 0;
    struct al_device *d = alDeviceFindAny(src);
    struct macAddressTLV *ch_pref;
    struct macAddressTLV *op_rest;

    {
        while ((ch_pref = (struct macAddressTLV *)getTypedTLV(c, TLV_TYPE_CHANNEL_PREFERENCE, i++))) {
            struct radio *r = radioAdd(d, ch_pref->mac);
            _processChannelPreferenceTLV(r, ch_pref, 0);
        }
    }
    {
        while ((op_rest = (struct macAddressTLV *)getTypedTLV(c, TLV_TYPE_RADIO_OPERATION_RESTRICTION, k++))) {
            struct radio *r = radioAdd(d, op_rest->mac);
            _processRadioOperRestrictTLV(r, op_rest);
        }
    }
    return PROCESS_CMDU_OK;
}

uint8_t processMapPolicyConfigRequest(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct al_device *d = alDeviceFindAny(src);

    if ((!d) || (d!=registrar)) {
        DEBUG_WARNING("can not find device for Map policy config\n");
        return PROCESS_CMDU_KO;
    }

    {
        /* steering policy TLV */
        struct TLV *steer;
        struct macStruct *mac_s;
        struct mac_item *mac_i;
        struct steerPolicyStruct *item_policy;
        struct radio *local_radio;

        if ((steer = getTypedTLV(c, TLV_TYPE_STEERING_POLICY, 0))) {
            DISALLOW_LIST_RESET(&local_device->local_disallow_list);
            DISALLOW_LIST_RESET(&local_device->btm_disallow_list);

            dlist_for_each(mac_s, steer->s.t.childs[0], s.t.l) {
                if ((mac_i = calloc(1, sizeof(struct mac_item)))) {
                    MACCPY(mac_i->mac, mac_s->mac);
                    dlist_add_tail(&local_device->local_disallow_list, &mac_i->l);
                }
            }

            dlist_for_each(mac_s, steer->s.t.childs[1], s.t.l) {
                if ((mac_i = calloc(1, sizeof(struct mac_item)))) {
                    MACCPY(mac_i->mac, mac_s->mac);
                    dlist_add_tail(&local_device->btm_disallow_list, &mac_i->l);
                }
            }

            dlist_for_each(item_policy, steer->s.t.childs[2], s.t.l) {
                local_radio = radioFind(local_device, item_policy->rid);
                if (!local_radio) {
                    DEBUG_WARNING("can NOT find local radio"MACFMT"\n", MACARG(item_policy->rid));
                    continue;
                }
                // logic model store the configuration to datamodel.
                mapSteeringPolicyChangedEvent(local_radio->uid, item_policy->rcpi_thresh, item_policy->chan_util,
                                    item_policy->policy);
            }
        }
    }

    {
        /* metrics reporting policy */
        struct metricReportPolicyTLV *metrics_rpt;
        struct metricRptPolicyStruct *item_policy;
        struct radio *local_radio;

        if ((metrics_rpt = (struct metricReportPolicyTLV *)getTypedTLV(c, TLV_TYPE_METRIC_REPORTING_POLICY, 0))) {
            local_device->metrics_rpt_interval = metrics_rpt->interval;
            dlist_for_each(item_policy, metrics_rpt->tlv.s.t.childs[0], s.t.l) {
                local_radio = radioFind(local_device, item_policy->rid);
                if (!local_radio) {
                    DEBUG_WARNING("can NOT find local radio"MACFMT"\n", MACARG(item_policy->rid));
                    continue;
                }
                local_radio->metrics_rpt_policy.sta_rcpi_margin = item_policy->rcpi_margin;
                local_radio->metrics_rpt_policy.sta_rcpi_thresh = item_policy->rcpi_thresh;
                local_radio->metrics_rpt_policy.ap_chutil_thresh = item_policy->chan_util_thresh;
                local_radio->metrics_rpt_policy.assoc_sta_inclusion_mode = item_policy->policy;
                DEBUG_INFO("Radio("MACFMT") metrics report policy updated:\n", MACARG(local_radio->uid));
                DEBUG_INFO("    sta_rcpi_thresh: %u\n", local_radio->metrics_rpt_policy.sta_rcpi_thresh);
                DEBUG_INFO("    sta_rcpi_margin: %u\n", local_radio->metrics_rpt_policy.sta_rcpi_margin);
                DEBUG_INFO("    ap_chutil_thresh: %u\n", local_radio->metrics_rpt_policy.ap_chutil_thresh);
                DEBUG_INFO("    assoc_sta_inclusion_mode: %u\n", local_radio->metrics_rpt_policy.assoc_sta_inclusion_mode);
            }
            doMetricsReport(local_device);
        }
    }

    {
        /* backhaul bss configuration */
        struct backhaulBSSConfigTLV *bconf_tlv;
        int i=0;

        while ((bconf_tlv = (struct backhaulBSSConfigTLV *)getTypedTLV(c, TLV_TYPE_BACKHAUL_BSS_CONFIGURATION, i++))) {
            struct wifi_interface *wif = (struct wifi_interface *)interfaceFind(local_device, bconf_tlv->bssid, interface_type_wifi);

            if ((wif) && (wif->role == role_ap)) {
                DEBUG_INFO("set backhaul bss disallow 0x%x\n", bconf_tlv->config);
                //FIXME: To do setting bss for disallow
            }
        }
    }

    {
        struct TLV *traffic_sepa_tlv;

        if ((traffic_sepa_tlv = getTypedTLV(c, TLV_TYPE_TRAFFIC_SEPARATION_POLICY, 0))) {
        }
    }

    {
        struct TLV *vips =  getCLSTypedTLV(c, CLS_TLV_TYPE_VIP_CONF, 0);
        if (vips)
            _handleVipStaTLV(vips);
    }

    {
        struct TLV *mapping_conf =  getCLSTypedTLV(c, CLS_TLV_TYPE_DSCP_MAPPING_CONF, 0);
        if (mapping_conf)
            _handleMappingTLV(mapping_conf);
    }

    {
        struct TLV *queue_conf =  getCLSTypedTLV(c, CLS_TLV_TYPE_EGRESS_CONF, 0);
        if (queue_conf)
            _handleQueueTLV(queue_conf);
    }

    {
        struct TLV *tc_conf =  getCLSTypedTLV(c, CLS_TLV_TYPE_TC_MAPPING_CONF, 0);
        if (tc_conf)
            _handleTcTLV(tc_conf);
    }

    return PROCESS_CMDU_OK;
}

uint8_t processClientAssocControlRequest(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct al_device *d = alDeviceFindAny(src);
    struct clientAssocCtrlReqTLV *assocCtrlReq_tlv = NULL;
    struct wifi_interface *wif = NULL;
    struct client *client = NULL;
    struct macStruct *mac = NULL;
    int i = 0;

    if (!d) {
        DEBUG_WARNING("Can not find device for client assoc control request\n");
        return PROCESS_CMDU_KO;
    }
    while((assocCtrlReq_tlv =
            (struct clientAssocCtrlReqTLV *)getTypedTLV(c, TLV_TYPE_CLIENT_ASSOCIATION_CONTROL_REQUEST, i++))) {
        wif = (struct wifi_interface *)interfaceFind(local_device, assocCtrlReq_tlv->bssid, interface_type_wifi);
        if (!wif || (wif->role != role_ap)) {
            DEBUG_WARNING("Can not find AP interface for["MACFMT"]\n", MACARG(assocCtrlReq_tlv->bssid));
            return PROCESS_CMDU_KO;
        }
        dlist_for_each(mac, assocCtrlReq_tlv->tlv.s.t.childs[0], s.t.l) {
            DEBUG_DETAIL("Client association control reqesut for client["MACFMT"]\n", MACARG(mac->mac));
            client = clientFind(local_device, wif, mac->mac);
            if (client) {
                doClientAssociationControl(wif, client, assocCtrlReq_tlv->ctrl, assocCtrlReq_tlv->valid_period);
            }
        }
    }
    return PROCESS_CMDU_OK;
}

struct chscan_req *processChannelScanOnRadio(uint32_t intf_idx, uint8_t *src, struct scanReqTLV *req_tlv,
                                              struct macStruct *radio_s, struct radio *radio)
{
    struct opClassChanStruct *opchan_tlv = NULL;
    struct operating_class *opc = NULL;
    struct chscan_req *req = NULL;
    uint8_t chans[MAX_CHANNEL_PER_OPCLASS] = {0};
    int i = 0;

    if (req_tlv->fresh_scan == 0) {
        sendChannelScanReport(idx2InterfaceName(intf_idx), getNextMid(), src, radio, NULL);
        return NULL;
    }
    if (req_tlv->fresh_scan != FRESH_SCAN_RESULT) {
        DEBUG_WARNING("Invalid scan mode: %d\n", req_tlv->fresh_scan);
        return NULL;
    }
    req = chscanReqNew(radio, intf_idx, src);
    if (!req) {
        DEBUG_ERROR("Internal Error.\n");
        return NULL;
    }
    dlist_for_each(opchan_tlv, radio_s->s.t.childs[0], s.t.l) {
        opc = opclassFind(radio, opchan_tlv->opclass);
        if (!opc) {
            DEBUG_WARNING("can NOT find opclass(%d) for radio["MACFMT"]\n", opchan_tlv->opclass, MACARG(radio->uid));
            continue;
        }
        if(opchan_tlv->chan.len) {
            for (i = 0; i < opchan_tlv->chan.len; i++) {
                DEBUG_DETAIL("Do scan on radio:["MACFMT"], opclass: %d, chan: %d\n", MACARG(radio_s->mac),
                    opchan_tlv->opclass, opchan_tlv->chan.data[i]);
            }
            chscanReqItemNew(req, opchan_tlv->opclass, opchan_tlv->chan.len, opchan_tlv->chan.data);
        }
        else { /*number of channels is 0, scan on all channels in this opclass*/
            for (i = 0; i < opc->num_support_chan; i++) {
                chans[i] = opc->channels[i].id;
            }
            chscanReqItemNew(req, opchan_tlv->opclass, opc->num_support_chan, &chans[0]);
        }
    }

    if (radio->scan_capa.scan_bootonly) {
        DEBUG_WARNING("Reject fresh scan: only support on boot scan.\n");
        req->status = SCAN_STATUS_BOOTSCAN_ONLY;
        reportChanScanResult(req);
        return NULL;
    }

    if (radio->chscan_req && ((req->ts - radio->chscan_req->ts) < radio->scan_capa.min_scan_interval)) {
        DEBUG_WARNING("Reject fresh scan: too soon\n");
        req->status = SCAN_STATUS_TOO_SOON;
        reportChanScanResult(req);
        return NULL;
    }

    req->status = SCAN_STATUS_SUCCESS;
    /* if ongoing scan does not finished. then replace the pending one */
    if (radio->chscan_req && !dlist_empty(&radio->chscan_req->h)) {
        if(radio->chscan_req_pend) {
            radio->chscan_req_pend->status = SCAN_STATUS_SCAN_ABORT;
            reportChanScanResult(radio->chscan_req_pend);
        }
        radio->chscan_req_pend = req;
        return NULL;
    }
    chscanReqDelete(radio->chscan_req);
    radio->chscan_req = req;

    return req;
}

uint8_t processChannelScanRequest(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct al_device *d = local_device;

    struct scanReqTLV *req_tlv = NULL;
    struct macStruct *radio_s = NULL;
    struct radio *radio = NULL;
    struct chscan_req *req = NULL;

    req_tlv = (struct scanReqTLV *)getTypedTLV(c, TLV_TYPE_CHANNEL_SCAN_REQUEST, 0);
    if (!req_tlv) {
        DEBUG_WARNING("Invalid format: No TLV_TYPE_CHANNEL_SCAN_REQUEST tlv\n");
        return PROCESS_CMDU_KO;
    }
    dlist_for_each(radio_s, req_tlv->tlv.s.t.childs[0], s.t.l) {
        radio = radioFind(d, radio_s->mac);
        if (!radio) {
            DEBUG_WARNING("Can NOT find local radio["MACFMT"]\n", MACARG(radio_s->mac));
            continue;
        }

        req = processChannelScanOnRadio(intf_idx, src, req_tlv, radio_s, radio);
        if (req)
            startChannelScan(req);
    }
    return PROCESS_CMDU_OK;
}

uint8_t processChannelScanReport(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct al_device *d = alDeviceFindAny(src);
    struct timeStampTLV *ts_tlv = NULL;
    struct scanResultTLV *result_tlv = NULL;
    char ts_str[TIMESTAMP_MAX_LEN+1] = {0};
    int i = 0;

    if (!d) {
        DEBUG_WARNING("Can not find device for channel scan report\n");
        return PROCESS_CMDU_KO;
    }
    {
        if((ts_tlv = (struct timeStampTLV *)getTypedTLV(c, TLV_TYPE_TIMESTAMP, 0))) {
            PLATFORM_MEMCPY(ts_str, ts_tlv->timeStamp.data, ts_tlv->timeStamp.len);
            DEBUG_DETAIL("Scan report timestamp: %s\n", ts_str);
        } else {
            DEBUG_WARNING("Invalid format: No TLV_TYPE_TIMESTAMP tlv\n");
            return PROCESS_CMDU_KO;
        }
    }
    {
        while ((result_tlv = (struct scanResultTLV *)getTypedTLV(c, TLV_TYPE_CHANNEL_SCAN_RESULT, i++))) {
            _processChannelResultTLV(d, result_tlv, ts_str, ts_tlv->timeStamp.len);
        }
    }
    return PROCESS_CMDU_OK;
}

static void _processCacScanOperateTimer(void *p)
{
    struct radio *r = (struct radio *)(p);
    if (!r)
        return;

    r->cur_cac.cur_status = CAC_STATUS_FAIL;
    r->cacscan_operate_timer = NULL;
    if(registrar)
        sendCacScanChannelPreferenceReport(idx2InterfaceName(registrar->recv_intf_idx), getNextMid(), registrar->al_mac, r);
    return;
}

static void _registerCacScanOperateTimer(struct radio *r, uint8_t opclass, uint8_t channel)
{
    uint32_t cac_period = CAC_PERIODIC_INTERVAL; /*default value for cac period */
    uint32_t unoccup = 0;

    PLATFORM_CANCEL_TIMER(r->cacscan_operate_timer);
    getPeriodForChannel(channel, opclass, r, &cac_period, &unoccup);
    if (!(r->cacscan_operate_timer = platformAddTimer((cac_period + 2) * 1000, 0, _processCacScanOperateTimer, r)))
        DEBUG_WARNING("Set CAC operation timer failed\n");
    return;
}

static void _processCacReqTLVRadio(struct cacRadioStruct *cac_radio)
{
    struct radio *r = NULL;
    struct operating_class *opc = NULL;
    struct chan_info *ch = NULL;
    uint8_t cac_method = 0;
    uint8_t action = 0;

    r = radioFind(local_device, cac_radio->ruid);
    if (!r) {
        DEBUG_WARNING("Can NOT find local radio["MACFMT"]\n", MACARG(cac_radio->ruid));
        return;
    }
    opc = opclassFind(r, cac_radio->opclass);
    if (!opc) {
        DEBUG_WARNING("can NOT find opclass(%d) for radio["MACFMT"]\n", cac_radio->opclass, MACARG(r->uid));
        return;
    }
    ch = channelFind(opc, cac_radio->channel);
    if(!ch) {
        DEBUG_WARNING("can NOT find ch[%d] in opc[%d] for radio["MACFMT"]\n", cac_radio->channel, cac_radio->opclass, MACARG(r->uid));
        return;
    }
    cac_method = ((cac_radio->cac_flag >> CAC_METHOD_SHIFT) & CAC_METHOD_MASK);
    if (r->cur_cac.cur_status == CAC_STATUS_ONGOING) {
        if (r->cur_cac.cur_scan.opclass == cac_radio->opclass
            && r->cur_cac.cur_scan.channel == cac_radio->channel
            && r->cur_cac.cur_scan.method == cac_method) {
            DEBUG_WARNING("new incoming CAC, paramters same as ongoing cac\n");
            return;
        }
        DEBUG_WARNING("new incoming CAC, should stop the current one\n");
        //FIXME todo: stop CAC scan.
    }
    action = ((cac_radio->cac_flag >> CAC_ACTION_SHIFT) & CAC_ACTION_MASK);
    MACCPY(r->cur_cac.cur_scan.ruid, cac_radio->ruid);
    r->cur_cac.cur_scan.opclass = cac_radio->opclass;
    r->cur_cac.cur_scan.channel = cac_radio->channel;
    r->cur_cac.cur_scan.method = cac_method;
    r->cur_cac.cur_scan.cmpl_action = action;
    DEBUG_DETAIL("CAC request content: ruid=["MACFMT"], opclass=%d, channel=%d, method=%d, cmpl_action=%d\n", MACARG(r->cur_cac.cur_scan.ruid),
        r->cur_cac.cur_scan.opclass, r->cur_cac.cur_scan.channel, r->cur_cac.cur_scan.method, r->cur_cac.cur_scan.cmpl_action);
    _registerCacScanOperateTimer(r, cac_radio->opclass, cac_radio->channel);
    //FIXME todo: start CAC scan.
    return;
}

uint8_t processCacRequest(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct al_device *d = NULL;
    struct TLV *tlv;
    struct cacRadioStruct *cac_radio;
    uint8_t i = 0;

    d = alDeviceFindAny(src);
    if (!d) {
        DEBUG_WARNING("Can not find device for cac request\n");
        return PROCESS_CMDU_KO;
    }
    tlv = getTypedTLV(c, TLV_TYPE_CAC_REQUEST, 0);
    if (!tlv) {
        DEBUG_WARNING("Invalid format: No TLV_TYPE_CAC_REQUEST tlv\n");
        return PROCESS_CMDU_KO;
    }
    while ((cac_radio = (struct cacRadioStruct *)getChildTLVStruct(&tlv->s, 0, (i++)))) {
        _processCacReqTLVRadio(cac_radio);
    }
    return PROCESS_CMDU_OK;
}

static void _processCacTerminationTLVRadio(struct cacTermRadioStruct *cac_term)
{
    struct radio *r = NULL;
    struct operating_class *opc = NULL;
    struct chan_info *ch = NULL;

    r = radioFind(local_device, cac_term->ruid);
    if (!r) {
        DEBUG_WARNING("Can NOT find local radio["MACFMT"]\n", MACARG(cac_term->ruid));
        return;
    }
    opc = opclassFind(r, cac_term->opclass);
    if (!opc) {
        DEBUG_WARNING("can NOT find opclass(%d) for radio["MACFMT"]\n", cac_term->opclass, MACARG(r->uid));
        return;
    }
    ch = channelFind(opc, cac_term->channel);
    if(!ch) {
        DEBUG_WARNING("can NOT find ch[%d] in opc[%d] for radio["MACFMT"]\n", cac_term->channel, cac_term->opclass, MACARG(r->uid));
        return;
    }
    DEBUG_WARNING("receive CAC termination: ch[%d] in opc[%d] for radio["MACFMT"]\n", cac_term->channel, cac_term->opclass, MACARG(r->uid));
    //FIXME todo:stop CAC scan
    return;
}

uint8_t processCacTermination(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct al_device *d = NULL;
    struct TLV *tlv = NULL;
    struct cacTermRadioStruct *cac_term = NULL;
    uint8_t i = 0;

    d = alDeviceFindAny(src);
    if (!d) {
        DEBUG_WARNING("Can not find device for cac termination\n");
        return PROCESS_CMDU_KO;
    }
    tlv = getTypedTLV(c, TLV_TYPE_CAC_TERMINATION, 0);
    if (!tlv) {
        DEBUG_WARNING("Invalid format: No TLV_TYPE_CAC_TERMINATION tlv\n");
        return PROCESS_CMDU_KO;
    }
    while ((cac_term = (struct cacTermRadioStruct *)getChildTLVStruct(&tlv->s, 0, (i++)))) {
        _processCacTerminationTLVRadio(cac_term);
    }
    return PROCESS_CMDU_OK;
}

uint8_t processCombinedInfrastructureMetrics(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct al_device *d = NULL;
    struct wifi_interface *wif = NULL;
    struct apMetricsTLV *metric = NULL;
    struct transmitterLinkMetricTLV *txlinkmetric_tlv = NULL;
    struct receiverLinkMetricTLV *rxlinkmetric_tlv = NULL;
    uint8_t i = 0;

    d = alDeviceFindAny(src);
    if (!d) {
        DEBUG_WARNING("Can not find device for combined infrastructure metrics\n");
        return PROCESS_CMDU_KO;
    }
    while((metric = (struct apMetricsTLV *)getTypedTLV(c, TLV_TYPE_AP_METRICS, i++))) {
        if ((wif = (struct wifi_interface *)interfaceFind(d, metric->bssid, interface_type_wifi)))
            _processAPMetricsTLV(wif, metric);
    }

    i = 0;
    while((txlinkmetric_tlv = (struct transmitterLinkMetricTLV *)getTypedTLV(c, TLV_TYPE_TRANSMITTER_LINK_METRIC, i++))) {
        _processTransmitterLinkMetricTLV(txlinkmetric_tlv);
    }
    i = 0;
    while((rxlinkmetric_tlv = (struct receiverLinkMetricTLV *)getTypedTLV(c, TLV_TYPE_RECEIVER_LINK_METRIC, i++))) {
        _processReceiverLinkMetricTLV(rxlinkmetric_tlv);
    }
    return PROCESS_CMDU_OK;
}

uint8_t processHigherLayerData(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct al_device *d = alDeviceFindAny(src);
    struct higherLayerDataTLV *tlv;

    if (!d) {
        DEBUG_WARNING("Can not find device for backhaul steering request\n");
        return PROCESS_CMDU_KO;
    }

    if ((tlv = (struct higherLayerDataTLV *)getTypedTLV(c, TLV_TYPE_HIGHER_LAYER_DATA, 0))) {
        processHigherLayerDataTLV(d, intf_idx, tlv);
    }

    return PROCESS_CMDU_OK;
}

uint8_t processBackhaulSteeringRequest(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct al_device *d = alDeviceFindAny(src);
    struct bkhSteerReqTLV *bkhSteerReq_tlv = NULL;
    struct wifi_interface *wif = NULL;
    struct operating_class *opc = NULL;
    struct chan_info *ch = NULL;

    if (!d) {
        DEBUG_WARNING("Can not find device for backhaul steering request\n");
        return PROCESS_CMDU_KO;
    }
    if (!d->is_controller) {
        DEBUG_WARNING("Device ["MACFMT"] is NOT controller\n", MACARG(src));
        return PROCESS_CMDU_KO;
    }
    if((bkhSteerReq_tlv = (struct bkhSteerReqTLV *)getTypedTLV(c, TLV_TYPE_BACKHAUL_STEERING_REQUEST, 0))) {
        wif = (struct wifi_interface *)interfaceFind(local_device, bkhSteerReq_tlv->sta, interface_type_wifi);
        if (!wif || (wif->role != role_sta)) {
            DEBUG_WARNING("Can not find backhaul sta interface for["MACFMT"]\n", MACARG(bkhSteerReq_tlv->sta));
            return PROCESS_CMDU_KO;
        }
        if (wif->steering_timer) {
            DEBUG_WARNING("Last backhaul steering hasn't been finished\n");
            return PROCESS_CMDU_KO;
        }
        MACCPY(wif->last_steering_target, bkhSteerReq_tlv->target);
        if (wif->radio) {
            opc = opclassFind(wif->radio, bkhSteerReq_tlv->opclass);
            if (!opc) {
                DEBUG_WARNING("can NOT find opclass(%d) for radio["MACFMT"]\n", bkhSteerReq_tlv->opclass, MACARG(wif->radio->uid));
                sendBackhaulSteeringResponse(idx2InterfaceName(intf_idx), c->id, src, wif, BKH_STEER_CODE_FAILURE, EC_BKHSTEERREQ_REJECT_CHANNEL);
                return PROCESS_CMDU_KO;
            }
            ch = channelFind(opc, bkhSteerReq_tlv->channel);
            if(!ch) {
                DEBUG_WARNING("can NOT find ch[%d] in opc[%d] for radio["MACFMT"]\n", bkhSteerReq_tlv->channel,
                            bkhSteerReq_tlv->opclass, MACARG(wif->radio->uid));
                sendBackhaulSteeringResponse(idx2InterfaceName(intf_idx), c->id, src, wif, BKH_STEER_CODE_FAILURE, EC_BKHSTEERREQ_REJECT_CHANNEL);
                return PROCESS_CMDU_KO;
            }
            if(!ch->pref) {
                DEBUG_WARNING("Channel[%d] in opc[%d] not available for radio["MACFMT"]: pref %d\n", bkhSteerReq_tlv->channel,
                            bkhSteerReq_tlv->opclass, MACARG(wif->radio->uid), ch->pref);
                sendBackhaulSteeringResponse(idx2InterfaceName(intf_idx), c->id, src, wif, BKH_STEER_CODE_FAILURE, EC_BKHSTEERREQ_REJECT_CHANNEL);
                return PROCESS_CMDU_KO;
            }
            //FIXME todo: do steering and sendBackhaulSteeringResponse()
        }
    } else {
        DEBUG_WARNING("Invalid format: No TLV_TYPE_BACKHAUL_STEERING_REQUEST tlv\n");
        return PROCESS_CMDU_KO;
    }
    return PROCESS_CMDU_OK;
}

uint8_t processBackhaulSteeringResponse(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct al_device *d = alDeviceFindAny(src);
    struct bkhSteerRspTLV *bkhSteerRsp_tlv = NULL;
    struct errorCodeTLV *errCode_tlv = NULL;

    if (!d) {
        DEBUG_WARNING("Can not find device for backhaul steering response\n");
        return PROCESS_CMDU_KO;
    }
    {
        if((bkhSteerRsp_tlv = (struct bkhSteerRspTLV *)getTypedTLV(c, TLV_TYPE_BACKHAUL_STEERING_RESPONSE, 0))) {
            DEBUG_DETAIL("Receive backhaul steering response TLV, result: %d\n", bkhSteerRsp_tlv->result);
        } else {
            DEBUG_WARNING("Invalid format: No TLV_TYPE_BACKHAUL_STEERING_RESPONSE tlv\n");
            return PROCESS_CMDU_KO;
        }
    }
    {
        if((errCode_tlv = (struct errorCodeTLV *)getTypedTLV(c, TLV_TYPE_ERROR_CODE, 0))) {
            DEBUG_DETAIL("Receive error code TLV: STA:["MACFMT"], error code:%d\n", MACARG(errCode_tlv->sta), errCode_tlv->code);
        } else {
            DEBUG_DETAIL("No TLV_TYPE_ERROR_CODE tlv\n");
        }
    }

    return PROCESS_CMDU_OK;
}

uint8_t processBackhaulStaCapabilityQuery(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    uint8_t *dst_mac;
    struct al_device *d = alDeviceFindAny(src);

    if (d)
        dst_mac = d->al_mac;
    else
        dst_mac = src;

    if (0 == sendBackhaulSTACapabilityReport(idx2InterfaceName(intf_idx), c->id, dst_mac)) {
       DEBUG_WARNING("Could not send packet\n");
    }
    return PROCESS_CMDU_OK;
}

uint8_t processBackhaulStaCapabilityReport(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct al_device *d = alDeviceFindAny(src);

    if (d) {
        int i = 0;
        struct bSTARadioCapaTLV *tlv;
        while ((tlv =
               (struct bSTARadioCapaTLV *)getTypedTLV(c, TLV_TYPE_BACKHAUL_STA_RADIO_CAPABILITIES, i++))) {
            struct radio *r = radioAdd(d, tlv->rid);
            if ((tlv->capa & MAC_INCLUDED) && (dlist_count(&tlv->tlv.s.t.childs[0])==1)) {
                struct macStruct *mac_s =
                    container_of(dlist_get_first(&tlv->tlv.s.t.childs[0]), struct macStruct, s.t.l);
                struct wifi_interface *wif = wifiInterfaceAdd(d, r, mac_s->mac);
                if (wif)
                    wif->role = role_sta;
            }
        }
    }
    return PROCESS_CMDU_OK;
}

uint8_t processFailedConnection(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    //FIXME todo: add notification for features
    return PROCESS_CMDU_OK;
}

uint8_t processChannelSelectionRequest(struct CMDU2 * c, uint32_t intf_idx, uint8_t * src, uint8_t * dst)
{
    int i = 0, k = 0;
    struct al_device *d = alDeviceFindAny(src);
    struct macAddressTLV *ch_pref;
    struct txPwrLimitTLV *pwr;
    struct radio *r;
    uint8_t *dst_mac;

    if (!d) {
        DEBUG_WARNING("Unknown dst AL MAC, using src("MACFMT") for send ap capability report \n", MACARG(src));
        dst_mac = src;
    } else
        dst_mac = d->al_mac;

    while ((pwr = (struct txPwrLimitTLV *)getTypedTLV(c, TLV_TYPE_TRANSMIT_POWER_LIMIT, k++))) {
        if (((r = radioFind(local_device, pwr->rid))) && (r->tx_power!=pwr->tx_pwr)) {
            r->tx_power = pwr->tx_pwr;
            r->change_power = 1;
        }
    }

    while ((ch_pref = (struct macAddressTLV *)getTypedTLV(c, TLV_TYPE_CHANNEL_PREFERENCE, i++))) {
        if ((r = radioFind(local_device, ch_pref->mac))) {
            r->channel_selection_code = _processChannelPreferenceTLV(r, ch_pref, 1);
            if (!r->channel_selection_code)
                r->change_channel = 1;
        }
    }

    dlist_for_each(r, local_device->radios, l) {
        if ((r->change_channel) || (r->change_power)) {
            doChannelSelection(r);
        }
    }
    sendChannelSelectionResponse(idx2InterfaceName(intf_idx), c->id, dst_mac);

    return PROCESS_CMDU_OK;
}

uint8_t processChannelSelectionResponse(struct CMDU2 * c, uint32_t intf_idx, uint8_t * src, uint8_t * dst)
{
        /* no more actions for now */
#if 0
    struct chanSelRspTLV *rsp;
    struct al_device *d = alDeviceFindAny(src);
    int i = 0;

    while ((rsp = (struct chanSelRspTLV *)getTypedTLV(c, TLV_TYPE_CHANNEL_SELECTION_RESPONSE, i++))) {
        struct radio *r = radioAdd(d, rsp->rid);
        r->ret_ch_sel = rsp->code;
    }
#endif
    return PROCESS_CMDU_OK;
}


uint8_t processOperatingChannelReport(struct CMDU2 * c, uint32_t intf_idx, uint8_t * src, uint8_t * dst)
{
    struct opChanReportTLV *rpt;
    struct al_device *d = alDeviceFindAny(src);
    int i = 0;

    if (!d)
        return PROCESS_CMDU_KO;

    while ((rpt = (struct opChanReportTLV *)getTypedTLV(c, TLV_TYPE_OPERATING_CHANNEL_REPORT, i++))) {
        struct radio *r = radioAdd(d, rpt->rid);
        _processOperatingChannelReportTLV(r, rpt);
    }

    d->ap_basic_capaed = 0;

    return PROCESS_CMDU_OK;
}

uint8_t processAssociatedStaLinkMetricsQuery(struct CMDU2 * c, uint32_t intf_idx, uint8_t * src, uint8_t * dst)
{
    struct macAddressTLV *qry;
    struct al_device *d = alDeviceFindAny(src);

    if (!d) {
        DEBUG_WARNING("Unknown dst AL MAC("MACFMT") for assoc sta link metric query \n", MACARG(src));
        return PROCESS_CMDU_KO;
    }

    qry = (struct macAddressTLV *)getTypedTLV(c, TLV_TYPE_STA_MAC_ADDRESS, 0);
    if (qry) {
        sendAssociatedStaLinkMetricsResponse(idx2InterfaceName(intf_idx), c->id, src, qry->mac);
        return PROCESS_CMDU_OK;
    }
    else
        return PROCESS_CMDU_KO;
}

uint8_t processAssociatedStaLinkMetricsResponse(struct CMDU2 * c, uint32_t intf_idx, uint8_t * src, uint8_t * dst)
{
    struct macAddressTLV *rsp;
    struct errorCodeTLV *err;
    struct al_device *d = alDeviceFindAny(src);
    struct client *sta = NULL;
    int i = 0;

    if (!d) {
        DEBUG_WARNING("Unknown dst AL MAC("MACFMT") \n", MACARG(src));
        return PROCESS_CMDU_KO;
    }

    while ((rsp = (struct macAddressTLV *)getTypedTLV(c, TLV_TYPE_ASSOCIATED_STA_LINK_METRICS, i++))) {
        if ((sta = clientFind(d, NULL, rsp->mac))) {
            _processAssocStaLinkMetricsTLV(sta, &rsp->tlv.s.t.childs[0]);
            /* send associated sta link query response event to logic module */
            assocStaLinkMetricsQueryRspEvent(d->al_mac, sta->mac, sta->link_metrics.rcpi_ul,
                sta->link_metrics.mac_rate_ul, sta->link_metrics.mac_rate_dl);
        }
    }

    if ((err = (struct errorCodeTLV *)getTypedTLV(c, TLV_TYPE_ERROR_CODE, 0))) {
        DEBUG_WARNING("AssociatedStaLinkMetricsResponse failed for ["MACFMT"] code=%d\n", MACARG(err->sta), err->code);
        /* if sta not exist in mesh nework. delete it. */
        if (err->code == EC_STA_UNASSOICITE) {
            sta = clientFind(d, NULL, err->sta);
            if (sta) {
                DEBUG_WARNING("AssociatedStaLinkMetricsResponse failed for ["MACFMT"] code=%d, then delete sta\n",
                    MACARG(err->sta), err->code);
                clientDelete(sta);
            }
        }
    }
    return PROCESS_CMDU_OK;
}

uint8_t processTopologyQuery(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    // When a "topology query" is received we must obtain a series of
    // information from the platform and then package and send it back
    // in a "topology response" message.
    uint8_t *dst_mac;
    struct al_device *d = alDeviceFindAny(src);
    struct TLV *tlv;

    if (!d) {
        DEBUG_WARNING("Unknown dst AL MAC, using src("MACFMT") for send topology response\n", MACARG(src));
        dst_mac = src;
    } else {
        if ((tlv = getTypedTLV(c, TLV_TYPE_MULTIAP_PROFILE, 0)))
            _processMAPProfileTLV(d, tlv);

        dst_mac = d->al_mac;
    }

    if ( 0 == sendTopologyResponse(idx2InterfaceName(intf_idx), c->id, dst_mac))
    {
        DEBUG_WARNING("Could not send 'topology query' message\n");
    }

    return PROCESS_CMDU_OK;
}

uint8_t processTopologyResponse(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct macAddressTLV *deviceinfo_tlv;
    struct TLV *tlv;
    struct al_device *d;
    struct opChanReportTLV *rpt;
    int i;

    deviceinfo_tlv = (struct macAddressTLV *)getTypedTLV(c, TLV_TYPE_DEVICE_INFORMATION_TYPE, 0);

    if (!deviceinfo_tlv)
        return PROCESS_CMDU_KO;

    DM_CHECK_DEVICE(d, deviceinfo_tlv->mac, intf_idx);

    d->ts_topo_resp = PLATFORM_GET_TIMESTAMP(0);

    if ((tlv = getTypedTLV(c, TLV_TYPE_SUPPORTED_SERVICE, 0)))
        _processSupportServiceTLV(d, tlv);

    /* 1. if Controller receive topology response said it support controller.
     *    send a AP-Autoconfiguration search msg;
     * 2. if Agent receive topology response said it support controller but registrar isn't it.
     *    set registrar to NULL to search new controller
     */
    if (d->is_controller) {
        if (isRegistrar()) {
            DEBUG_INFO("Receive other Controller("MACFMT") topology response. trigger once auto search\n",
                MACARG(d->al_mac));
            sendAPAutoconfigurationSearch(getNextMid(), IEEE80211_FREQUENCY_BAND_2_4_GHZ);
        }
        else if (registrar && (registrar != d)) {
            DEBUG_INFO("Receive other Controller("MACFMT") topology response. try onboarding again\n",
                MACARG(d->al_mac));
            setRegistrar(NULL);
        }
    }
    else {
        if (registrar == d) {
            DEBUG_INFO("Detect Controller("MACFMT") become to agent. set Registrar to NULL\n",
                MACARG(d->al_mac));
            setRegistrar(NULL);
        }
    }

    i = 0;
    while ((rpt = (struct opChanReportTLV *)getTypedTLV(c, TLV_TYPE_OPERATING_CHANNEL_REPORT, i++))) {
        struct radio *r = radioAdd(d, rpt->rid);
        _processOperatingChannelReportTLV(r, rpt);
    }

    if ((tlv = getTypedTLV(c, TLV_TYPE_AP_OPERATIONAL_BSS, 0)))
        _processAPOperationalBSSTLV(d, tlv);

    if ((tlv = getSubTypedTLV(c, TLV_TYPE_VBSS, TLV_SUB_TYPE_VBSS_CONFIGURATION_REPORT, 0)))
        _processVBSSConfigurationReportTLV(d, tlv);

    if ((tlv = getTypedTLV(c, TLV_TYPE_ASSOCIATED_CLIENTS, 0)))
        _processAssociatedClientsTLV(d, tlv);

    if ((tlv = getTypedTLV(c, TLV_TYPE_DEVICE_INFORMATION_TYPE, 0)))
        _processDeviceInformationTypeTLV(d, tlv);

    if ((tlv = getTypedTLV(c, TLV_TYPE_MULTIAP_PROFILE, 0)))
        _processMAPProfileTLV(d, tlv);

    i = 0;
    while ((tlv = getTypedTLV(c, TLV_TYPE_NEIGHBOR_DEVICE_LIST, i++))) {
        _process1905NeighborDeviceTLV(d, tlv, 1);
    }

    i = 0;
    while ((tlv = getTypedTLV(c, TLV_TYPE_NON_1905_NEIGHBOR_DEVICE_LIST, i++))) {
        _processnon1905NeighborDeviceTLV(d, tlv);
    }

    if (!local_device->configured)
        return PROCESS_CMDU_OK_TRIGGER_CONTROLLER_SEARCH;
    else
        return PROCESS_CMDU_OK;
}

uint8_t processTopologyNotification(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct al_device *d;
    struct TLV *tlv;
    struct macAddressTLV *mac_tlv = (struct macAddressTLV *)getTypedTLV(c, TLV_TYPE_AL_MAC_ADDRESS_TYPE, 0);

    if (!mac_tlv)
        return PROCESS_CMDU_KO;

    DM_CHECK_DEVICE(d, mac_tlv->mac, intf_idx);

    if ((tlv = getTypedTLV(c, TLV_TYPE_CLIENT_ASSOCIATION_EVENT, 0)))
        _processClientAssociationEventTLV(d, tlv);
    return PROCESS_CMDU_OK;
}

uint8_t processClientCapabilityQuery(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct al_device *d = alDeviceFindAny(src);
    struct clientInfoTLV *info;

    if (!d) {
        DEBUG_WARNING("can not find device for client capability query\n");
        return PROCESS_CMDU_KO;
    }
    if ((info = (struct clientInfoTLV *)getTypedTLV(c, TLV_TYPE_CLIENT_INFO, 0)))
        return sendClientCapabilityReport(idx2InterfaceName(intf_idx), c->id, src, info->bssid, info->client);
    return PROCESS_CMDU_KO;
}

uint8_t processClientCapabilityReport(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct al_device *d = alDeviceFindAny(src);
    struct clientInfoTLV *info;
    struct clientCapabilityReportTLV *cap;
    struct errorCodeTLV *err_code;
    struct wifi_interface *wif = NULL;
    struct client *client = NULL;

    if (!d) {
        DEBUG_WARNING("can not find device for client capability query\n");
        return PROCESS_CMDU_KO;
    }

    if ((info = (struct clientInfoTLV *)getTypedTLV(c, TLV_TYPE_CLIENT_INFO, 0))) {
        wif = wifiInterfaceAdd(d, NULL, info->bssid);
        client = clientAdd(wif, info->client);
    }

    if (!client) {
        DEBUG_WARNING("can not find client\n");
        return PROCESS_CMDU_KO;
    }

    if ((cap = (struct clientCapabilityReportTLV *)getTypedTLV(c, TLV_TYPE_CLIENT_CAPABILITY_REPORT, 0))) {
        if (CAP_REPORT_SUCCESS == cap->result) {
            REPLACE_VVDATA(client->last_assoc, cap->assc_req.datap, cap->assc_req.len);
            parseAssocFrame(client, 0);
        }
        if ((err_code = (struct errorCodeTLV *)getTypedTLV(c, TLV_TYPE_ERROR_CODE, 0)))
            client->last_capa_rpt_result = err_code->code;
    }
    return PROCESS_CMDU_OK;
}

uint8_t process1905Cmdu(struct CMDU2 *c, uint32_t recv_intf_idx, uint8_t *src, uint8_t *dst)
{
    struct cmdu_msg_desc *desc;
    struct al_device *d;

    if (!c) {
        return PROCESS_CMDU_KO;
    }

    // Third party implementations maybe need to process some protocol
    // extensions
    //
    //process1905CmduExtensions(c);

	DEBUG_INFO("<-%s-- [mid=%04x, src="MACFMT"] %s\n",
        idx2InterfaceName(recv_intf_idx), c->id, MACARG(src), convert_1905_CMDU_type_to_string(c->type));

    if ((d = alDeviceFindAny(src)))
        DM_UPDATE_DEVICE(d, recv_intf_idx);

    desc = CMDU_type_to_desc(c->type);

    if (!desc)
        return PROCESS_CMDU_KO;

    if (checkAck(c, src))
        sendAck(idx2InterfaceName(recv_intf_idx), c->id, src, NULL);

    checkRetry(c, src);

    if ((desc) && (desc->process))
        return desc->process(c, recv_intf_idx, src, dst);

    return PROCESS_CMDU_OK;
}

uint8_t processLlpdPayload(struct PAYLOAD *payload, uint32_t intf_idx)
{
    uint8_t *p;
    uint8_t  i;
    struct interface *intf;

    uint8_t  *al_mac_address=NULL, *mac_address = NULL;

    if (NULL == payload)
    {
        return 0;
    }

    DEBUG_INFO("<-%s-- LLDP_BRIDGE_DISCOVERY\n", idx2InterfaceName(intf_idx));

    // We need to update the data model structure, which keeps track
    // of local interfaces, neighbors, and neighbors' interfaces, and
    // what type of discovery messages ("topology discovery" and/or
    // "bridge discovery") have been received on each link.

    // First, extract the AL MAC and MAC addresses of the interface
    // which transmitted this bridge discovery message
    //
    i = 0;
    while (NULL != (p = payload->list_of_TLVs[i]))
    {
        switch (*p)
        {
            case TLV_TYPE_CHASSIS_ID:
            {
                struct chassisIdTLV *t = (struct chassisIdTLV *)p;
                al_mac_address = t->chassis_id;
                break;
            }
            case TLV_TYPE_MAC_ADDRESS_TYPE:
            {
                struct portIdTLV *t = (struct portIdTLV *)p;
                mac_address = t->port_id;
                break;
            }
            case TLV_TYPE_TIME_TO_LIVE:
            {
                break;
            }
            default:
            {
                DEBUG_DETAIL("Ignoring TLV type %d\n", *p);
                break;
            }
        }
        i++;
    }

    if (!al_mac_address) {
        DEBUG_WARNING("More TLVs were expected inside this LLDP message\n");
        return 0;
    }

    // Finally, update the data model
    //
    //
    intf = interfaceFindIdx(intf_idx);
    if (!intf) {
        DEBUG_ERROR("Can not find local interface(idx=%d)\n", intf_idx);
        return PROCESS_CMDU_KO;
    }

    dmUpdateNeighbor(intf, al_mac_address, mac_address, IS_LLDP);

    return 1;
}

uint8_t processAPMetricsQuery(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    uint8_t *dst_mac;
    struct TLV *mac_tlv;
    struct al_device *d = alDeviceFindAny(src);

    if (!d) {
        DEBUG_WARNING("Unknown dst AL MAC, using src("MACFMT") for send ap metric response \n", MACARG(src));
        dst_mac = src;
    } else
        dst_mac = d->al_mac;

    {
        mac_tlv = getTypedTLV(c, TLV_TYPE_AP_METRIC_QUERY, 0);
        if (!mac_tlv)
            return PROCESS_CMDU_KO;
        if (dlist_count(&mac_tlv->s.t.childs[0])) {
            if (0 == sendApMetricsResponse(idx2InterfaceName(intf_idx), c->id, dst_mac, &mac_tlv->s.t.childs[0], 0)) {
                DEBUG_WARNING("Could not send packet\n");
            }
        }
    }
    return PROCESS_CMDU_OK;
}

uint8_t processAPMetricsResponse(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    int i;
    struct apMetricsTLV *metrics;
    struct macAddressTLV *link;
    struct assocTrafficStatsTLV *traf;
    struct al_device *d = alDeviceFindAny(src);

    if (!d) {
        DEBUG_WARNING("Unknown src AL MAC("MACFMT") in Ap metric response \n", MACARG(src));
    }
    {
        i = 0;
        while ((metrics = (struct apMetricsTLV *)getTypedTLV(c, TLV_TYPE_AP_METRICS, i++))) {
            struct wifi_interface *wif;
            if ((wif = (struct wifi_interface *)interfaceFind(d, metrics->bssid, interface_type_wifi)))
                _processAPMetricsTLV(wif, metrics);
        }
    }

    {
        i = 0;
        while ((link = (struct macAddressTLV *)getTypedTLV(c, TLV_TYPE_ASSOCIATED_STA_LINK_METRICS, i++))) {
            struct client *sta;
            if ((sta = clientFind(d, NULL, link->mac)))
                _processAssocStaLinkMetricsTLV(sta, &link->tlv.s.t.childs[0]);
        }
    }

    {
        i = 0;
        while ((traf = (struct assocTrafficStatsTLV *)getTypedTLV(c, TLV_TYPE_ASSOCIATED_STA_TRAFFIC_STATS, i++))) {
            struct client *sta;
            if ((sta = clientFind(d, NULL, traf->sta)))
                sta->traffic_stats = traf->stats;
        }
    }
    return PROCESS_CMDU_OK;
}

struct unassociatedStaMetricsTimerParam {
    struct unassocStaLinkMetricsQryTLV *query_tlv;
    uint32_t recv_ifidx;
    uint32_t index;
    mac_address src_addr;
};

void _delNacStas(struct unassocStaLinkMetricsQryTLV *query)
{
    struct unassocMetricQryStruct *channel_item;
    struct macStruct *mac;

    // notify driver del nac sta
    dlist_for_each(channel_item, query->tlv.s.t.childs[0], s.t.l) {
        dlist_for_each(mac, channel_item->s.t.childs[0], s.t.l) {
            DEBUG_INFO("del NAC station: "MACFMT" info, channel: %u\n", MACARG(mac->mac), channel_item->channel);
            setNacMonitorEnable(channel_item->channel, 0);
            delNacMonitorSta(channel_item->channel, mac->mac);
        }
    }
}

void _unassociatedStaMetricsTimerHandler(void *data)
{
    struct unassociatedStaMetricsTimerParam *param = (struct unassociatedStaMetricsTimerParam *)data;

    sendUnassociatedStaLinkMetricsResponse(idx2InterfaceName(param->recv_ifidx), getNextMid(), param->src_addr,
        param->query_tlv->opclass, NULL, &param->query_tlv->tlv.s.t.childs[0]);

    TLVFree((struct TLV *)param->query_tlv);
    free(param);
}

void _regitsterUnassociatedStaMetricsTimer(struct unassocStaLinkMetricsQryTLV *query,
                uint32_t recv_ifidx, uint8_t *src_addr)
{
    struct unassocMetricQryStruct *channel_item;
    struct macStruct *mac;
    struct unassociatedStaMetricsTimerParam *param =
        (struct unassociatedStaMetricsTimerParam *)calloc(1, sizeof(*param));

    if (!param)
        return;

    param->query_tlv = query;
    param->recv_ifidx = recv_ifidx;
    MACCPY(param->src_addr, src_addr);

    // notify driver add nac sta
    dlist_for_each(channel_item, query->tlv.s.t.childs[0], s.t.l) {
        dlist_for_each(mac, channel_item->s.t.childs[0], s.t.l) {
            DEBUG_INFO("add NAC station: "MACFMT" , channel: %u\n", MACARG(mac->mac), channel_item->channel);
            addNacMonitorSta(channel_item->channel, mac->mac);
            setNacEnable(channel_item->channel);
        }
    }

    if (!platformAddTimer(1500, 0, _unassociatedStaMetricsTimerHandler, param))
        goto fail;

    return;

fail:
    // notify driver del nac sta
    _delNacStas(query);
    TLVFree((struct TLV *)param->query_tlv);
    free(param);

    return;
}

uint8_t processUnassociatedStaLinkMetricsQuery(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct al_device *d = alDeviceFindAny(src);
    struct unassocStaLinkMetricsQryTLV *query_tlv = NULL;

    if (!d) {
        DEBUG_WARNING("Unknown dst AL MAC("MACFMT") for unassoc sta link metrics query \n", MACARG(src));
        return PROCESS_CMDU_KO;
    }
    {
        //FIXME: TODO done the action
        //start timer to get latest metrics from driver
        query_tlv = (struct unassocStaLinkMetricsQryTLV *)getTypedTLV(c, TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_QUERY, 0);
        if (!query_tlv)
            return PROCESS_CMDU_KO;
        // store tlv untill send query response
        TLVRef((struct TLV *)query_tlv);
        _regitsterUnassociatedStaMetricsTimer(query_tlv, intf_idx, src);
    }
    return PROCESS_CMDU_OK;
}

uint8_t processUnassociatedStaLinkMetricsResponse(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct unassocStaLinkMetricsRspTLV *rsp_tlv;
    struct al_device *d = alDeviceFindAny(src);

    if (!d) {
        DEBUG_WARNING("Unknown agent AL MAC("MACFMT") for unassociated sta link metrics response \n", MACARG(src));
        return PROCESS_CMDU_KO;
    }

    {
        if ((rsp_tlv =
                (struct unassocStaLinkMetricsRspTLV *)getTypedTLV(c, TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_RESPONSE, 0)))
        _processUnassociatedStaLinkMetricsTLV(d, rsp_tlv);
    }
    return PROCESS_CMDU_OK;
}

uint8_t processClientSteeringRequest(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct al_device *d = alDeviceFindAny(src);
    struct steerReqTLV *req;
    struct macStruct *mac_s;
    struct targetBssStruct *target_s;
    struct wifi_interface *wif;
    struct client *sta;
    uint8_t ret = PROCESS_CMDU_KO;
    uint8_t mandate;
    struct TLV *err_tlv;

    DEFINE_DLIST_HEAD(err_codes);

    if (!d) {
        DEBUG_WARNING("can not find device for client steering request\n");
        goto fail;
    }

    if ((!(req = (struct steerReqTLV *)getTypedTLV(c, TLV_TYPE_STEERING_REQUEST, 0))) &&
        (!(req = (struct steerReqTLV *)getTypedTLV(c, TLV_TYPE_PROFILE2_STEERING_REQUEST, 0)))) {
        DEBUG_WARNING("parse TLV error for client steering request\n");
        goto fail;
    }

    if (!(wif = (struct wifi_interface *)interfaceFind(local_device, req->bssid, interface_type_wifi))) {
        DEBUG_WARNING("can not find wifi interface\n");
        goto fail;
    }

    mandate = !!BIT_IS_SET(req->mode, STEER_REQ_MODE_MANDATE_SHIFT);
    if (mandate) {
        if (!dlist_count(&req->tlv.s.t.childs[1])) {
            DEBUG_WARNING("mandate steering: no target spcified\n");
            goto fail;
        } else if ((!dlist_count(&req->tlv.s.t.childs[0])) && (dlist_count(&req->tlv.s.t.childs[1])!=1)) {
            DEBUG_WARNING("mandate steering: wild station with more than 1 targets\n");
            goto fail;
        } else if (dlist_count(&req->tlv.s.t.childs[1])!=dlist_count(&req->tlv.s.t.childs[0])) {
            DEBUG_WARNING("mandate steering: target number not match sta number\n");
            goto fail;
        }
        target_s = container_of(dlist_get_first(&req->tlv.s.t.childs[1]), struct targetBssStruct, s.t.l);

        if (dlist_count(&req->tlv.s.t.childs[0])) {
            dlist_for_each(mac_s, req->tlv.s.t.childs[0], s.t.l) {
                if ((sta = clientFind(local_device, wif, mac_s->mac))) {
                    doMandantorySteer(sta, target_s->target, target_s->opclass, target_s->channel,
                            req->mode, req->disassoc, target_s->reason);
                } else {
                    DEBUG_WARNING("can not find sta["MACFMT"]\n", MACARG(mac_s->mac));
                    err_tlv = obtainErrCodeTLV(EC_STA_UNASSOICITE, mac_s->mac);
                    dlist_add_tail(&err_codes, &err_tlv->s.t.l);
                }
                if (target_s->s.t.l.next != &req->tlv.s.t.childs[1])
                    target_s = container_of(target_s->s.t.l.next, struct targetBssStruct, s.t.l);
            }
        } else {
            dlist_for_each(sta, wif->clients, l) {
                doMandantorySteer(sta, target_s->target, target_s->opclass, target_s->channel,
                    req->mode, req->disassoc, target_s->reason);
            }
        }

    } else {
        if (dlist_count(&req->tlv.s.t.childs[1])) {
            DEBUG_WARNING("opportunity steering: non empty target\n");
            goto fail;
        }
        if (dlist_count(&req->tlv.s.t.childs[0])) {
            dlist_for_each(mac_s, req->tlv.s.t.childs[0], s.t.l) {
                if ((sta = clientFind(local_device, wif, mac_s->mac))) {
                    doOpportunitySteer(sta, req->mode, req->disassoc, req->window);
                } else {
                    DEBUG_WARNING("can not find sta["MACFMT"]\n", MACARG(mac_s->mac));
                    err_tlv = obtainErrCodeTLV(EC_STA_UNASSOICITE, mac_s->mac);
                    dlist_add_tail(&err_codes, &err_tlv->s.t.l);
                }
            }
        } else {
            dlist_for_each(sta, wif->clients, l) {
                doOpportunitySteer(sta, req->mode, req->disassoc, req->window);
            }
        }
    }
    ret = PROCESS_CMDU_OK;
fail:
    sendAck(idx2InterfaceName(intf_idx), c->id, dst, &err_codes);
    return ret;
}

uint8_t processSteeringBTMReport(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct al_device *d = alDeviceFindAny(src);
    struct steerBtmReportTLV *report;
    uint8_t ret = PROCESS_CMDU_KO;

    if (!d) {
        DEBUG_WARNING("can not find device for client capability query\n");
        return PROCESS_CMDU_KO;
    }
    /* FIXME: how to use the measurement report IE */
    if ((!(report = (struct steerBtmReportTLV *)getTypedTLV(c, TLV_TYPE_STEERING_BTM_REPORT, 0)))) {
        DEBUG_WARNING("parse TLV error for client steering BTM report\n");
        goto fail;
    }

    /* send event to logic module */
    sendBtmResponseEvent(d->al_mac, report->sta, report->bssid, report->status, report->target.datap);

    ret = PROCESS_CMDU_OK;

fail:
    return ret;
}

uint8_t processSteeringComplete(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct al_device *d = alDeviceFindAny(src);

    if (!d) {
        DEBUG_WARNING("can not find device for client capability query\n");
        return PROCESS_CMDU_KO;
    }
//FIXME: todo clear local steering context
    return PROCESS_CMDU_OK;
}

uint8_t processBeaconMetricsQuery(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct bcnMetricQueryTLV *qry;
    struct al_device *d = alDeviceFindAny(src);
    struct TLV *err_tlv;
    struct client* sta_info;

    DEFINE_DLIST_HEAD(err_codes);

    if (!d) {
        DEBUG_WARNING("Unknown DEV, using src("MACFMT") for beacon metrics query \n", MACARG(src));
        return PROCESS_CMDU_KO;
    }
    qry = (struct bcnMetricQueryTLV *)getTypedTLV(c, TLV_TYPE_BEACON_METRICS_QUERY, 0);
    if (!qry) {
        DEBUG_WARNING("can NOT parse beacon metrics query \n");
        return PROCESS_CMDU_KO;
    }
    dlist_head_init(&err_codes);
    sta_info = check_beacon_request_params(qry->bssid, qry->sta);
    if (!sta_info) {
        err_tlv = obtainErrCodeTLV(EC_STA_UNASSOICITE, qry->sta);
        dlist_add_tail(&err_codes, &err_tlv->s.t.l);
    }
    sendAck(idx2InterfaceName(intf_idx), c->id, d->al_mac, &err_codes);
    if (sta_info)
        doBeaconRequest(sta_info, qry);
    return PROCESS_CMDU_OK;
}

uint8_t processBeaconMetricsResponse(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct bcnMetricRspTLV *rsp;
    struct client *sta;
    struct al_device *d = alDeviceFindAny(src);

    if (!d) {
        DEBUG_WARNING("Unknown src AL MAC["MACFMT"] \n", MACARG(src));
        return PROCESS_CMDU_KO;
    }
    rsp = (struct bcnMetricRspTLV *)getTypedTLV(c, TLV_TYPE_BEACON_METRICS_RESPONSE, 0);
    if (!rsp) {
        DEBUG_WARNING("Unknown dst AL MAC, using src("MACFMT") for send ap capability report \n", MACARG(src));
        return PROCESS_CMDU_KO;
    }
    sta = clientFind(local_device, NULL, rsp->sta);
    if (sta)
        return clientUpdateMeasuredElem(sta, &rsp->tlv.s.t.childs[0]);
    return PROCESS_CMDU_KO;
}

uint8_t processAssocStatusNotification(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct al_device *d = alDeviceFindAny(src);
    struct TLV *notification;
    struct assocStatNotifyStruct *stat;
    struct wifi_interface *wif;

    if (!d) {
        DEBUG_WARNING("Unknown src AL MAC["MACFMT"] \n", MACARG(src));
        return PROCESS_CMDU_KO;
    }
    notification = getTypedTLV(c, TLV_TYPE_ASSOCIATION_STATUS_NOTIFICATION, 0);
    if (!notification) {
        DEBUG_WARNING("there is NO ASSOCIATION_STATUS_NOTIFICATION_TLV\n");
        return PROCESS_CMDU_KO;
    }
    dlist_for_each (stat, notification->s.t.childs[0], s.t.l) {
        wif = (struct wifi_interface *)interfaceFind(d, stat->bssid, interface_type_wifi);
        if (!wif) {
            DEBUG_WARNING("can NOT find vap["MACFMT"] in agent["MACFMT"]\n", MACARG(stat->bssid), MACARG(d->al_mac));
            continue;
        }
        if (stat->status != NO_MORE_ASSOC && stat->status != ASSOC_ALLOW) {
            DEBUG_WARNING("wrong assoc_status=%d for bssid["MACFMT"]\n", stat->status, MACARG(stat->bssid));
            continue;
        }
        wif->assoc_allowance = stat->status;
    }
    return PROCESS_CMDU_OK;
}

uint8_t processClientDisassocStat(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct macAddressTLV *mac;
    struct codeTLV *reason;
    struct assocTrafficStatsTLV *traf_stat;
    struct client *sta;
    struct al_device *d = alDeviceFind(src);

    mac = (struct macAddressTLV *)getTypedTLV(c, TLV_TYPE_MAC_ADDRESS_TYPE, 0);
    if (!mac) {
        DEBUG_WARNING("NO STA MAC in client disassociation stat msg\n");
        return PROCESS_CMDU_KO;
    }
    sta = clientFind(d, NULL, mac->mac);
    if (!sta) {
        DEBUG_WARNING("can NOT find sta["MACFMT"] in agent["MACFMT"]\n", MACARG(mac->mac), MACARG(src));
        return PROCESS_CMDU_KO;
    }
    reason = (struct codeTLV *)getTypedTLV(c, TLV_TYPE_REASON_CODE, 0);
    if (!reason) {
        DEBUG_WARNING("NO reason code in client disassociation stat msg\n");
        return PROCESS_CMDU_KO;
    }
    traf_stat = (struct assocTrafficStatsTLV *)getTypedTLV(c, TLV_TYPE_ASSOCIATED_STA_TRAFFIC_STATS, 0);
    if (!traf_stat) {
        DEBUG_WARNING("NO traffic statistic in client disassociation stat msg\n");
        return PROCESS_CMDU_KO;
    }
    /* Todo: controller may store the station info for a while because of roaming */
    /* Also the controller should do garbage collecting for the aged disassoc/deauth clients */
    return PROCESS_CMDU_OK;
}

uint8_t processErrorResponse(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct p2ErrorCodeTLV *err_code;
    struct al_device *d = alDeviceFindAny(src);
    int i = 0;

    if (!d) {
        DEBUG_WARNING("Unknown src AL MAC["MACFMT"] \n", MACARG(src));
        return PROCESS_CMDU_KO;
    }
    while ((err_code = (struct p2ErrorCodeTLV *)getTypedTLV(c, TLV_TYPE_PROFILE2_ERROR_CODE, i++))) {
        if (!err_code) {
            DEBUG_WARNING("can NOT find err_code in error response\n");
            continue;
        }
        /* FIXME if Traffic Separation feature done. */
    }
    return PROCESS_CMDU_KO;
}

uint8_t processVBSSCapabilitiesRequest(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct al_device *d = alDeviceFindAny(src);

    if (!d) {
        DEBUG_WARNING("Unknown src AL MAC["MACFMT"] \n", MACARG(src));
        return PROCESS_CMDU_KO;
    }

    return sendVBSSCapabilitiesResponse(idx2InterfaceName(d->recv_intf_idx), c->id, d->al_mac);
}

struct vbss_timer_param
{
    struct al_device *agent;
    struct radio *r;
};

void _vbssFirstCreationTimerHandler(void *data)
{
    struct vbss_timer_param *param = (struct vbss_timer_param *)data;
    struct bss_info new_bss_info;

    if (!param) {
        DEBUG_ERROR("param or param->agent or param->r is NULL.\n");
        return;
    }

    if (!param->agent || !param->r) {
        DEBUG_ERROR("param->agent or param->r is NULL.\n");
        free(param);
        return;
    }

    /* generate new bss info */
    generateNewBssid(param->agent, param->r, new_bss_info.bssid);
    SSIDCPY(new_bss_info.ssid, local_policy.vbss_conf.vbss_ssid);
    new_bss_info.auth = local_policy.vbss_conf.auth;
    new_bss_info.encrypt = local_policy.vbss_conf.encrypt;
    new_bss_info.key.len = local_policy.vbss_conf.k.len;
    if (new_bss_info.key.len > 0)
        memcpy(new_bss_info.key.key, local_policy.vbss_conf.k.key, new_bss_info.key.len);

    DEBUG_INFO("generate new vbss for radio("MACFMT"):\n", MACARG(param->r->uid));
    DEBUG_INFO("    bssid   : "MACFMT"\n", MACARG(new_bss_info.bssid));
    DEBUG_INFO("    ssid    : %s\n", new_bss_info.ssid.ssid);
    DEBUG_INFO("    ssid len: %u\n", new_bss_info.ssid.len);
    DEBUG_INFO("    encrypt : %u\n", new_bss_info.encrypt);
    DEBUG_INFO("    key_len : %u, key: %s\n", new_bss_info.key.len, new_bss_info.key.key);

    /* send vbss creation request */
    sendVBSSCreationRequest(idx2InterfaceName(param->agent->recv_intf_idx), getNextMid(),
        param->agent->al_mac, param->r->uid, &new_bss_info, NULL, NULL, NULL);

    free(param);
    return;
}

uint8_t processVBSSCapabilitiesResponse(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct vbssCapabilitiesTLV *vbss_capability;
    struct al_device *d = alDeviceFindAny(src);
    struct radio *r = NULL;
    struct vbss_timer_param *param = NULL;
    int i = 0;

    if (!d) {
        DEBUG_WARNING("Unknown src AL MAC["MACFMT"] \n", MACARG(src));
        return PROCESS_CMDU_KO;
    }

    while ((vbss_capability = (struct vbssCapabilitiesTLV *)getSubTypedTLV(c, TLV_TYPE_VBSS, TLV_SUB_TYPE_VBSS_AP_RADIO_CAPABILITIES, i++))) {
        if (!vbss_capability) {
            DEBUG_WARNING("can NOT find vbss_capability in vbss capability response\n");
            continue;
        }

        r = radioFind(d, vbss_capability->ruid);
        if (!r) {
            DEBUG_WARNING("can NOT find radio("MACFMT") in datamodel\n", MACARG(vbss_capability->ruid));
            continue;
        }

        r->vbss_capa.max_vbss = vbss_capability->max_vbss;
        r->vbss_capa.vbss_subtract = GET_BIT(vbss_capability->flag, VBSSs_SUBTRACT_SHIFT);
        r->vbss_capa.vbssid_restrictions = GET_BIT(vbss_capability->flag, VBSSID_RESTRICTIONS_SHIFT);
        r->vbss_capa.match_and_mask_restrictions = GET_BIT(vbss_capability->flag, VBSSID_MATCH_AND_MASK_RESTRICTIONS_SHIFT);
        r->vbss_capa.fixed_bits_restrictions = GET_BIT(vbss_capability->flag, FIXED_BITS_RESTRICTIONS_SHIFT);
        MACCPY(r->vbss_capa.fixed_bits_mask, vbss_capability->fixed_bits_mask);
        MACCPY(r->vbss_capa.fixed_bits_value, vbss_capability->fixed_bits_value);

        /* if supported vbss add timer to nofify agent create first vbss */
        if (r->vbss_capa.max_vbss > 0) {
            param = (struct vbss_timer_param *)calloc(1, sizeof(struct vbss_timer_param));
            if (param) {
                param->agent = d;
                param->r = r;
                /* delay 30s(wait bss applied) to create first vbss on the radio */
                platformAddTimer(30000, 0, _vbssFirstCreationTimerHandler, param);
            }
        }
        else {
            DEBUG_INFO("radio("MACFMT"): r->vbss_capa.max_vbss(%u) <= 0. \n",
                MACARG(vbss_capability->ruid), r->vbss_capa.max_vbss);
        }

        DEBUG_DETAIL("radio("MACFMT") vbss_capability: (%u, %u, %u, %u, %u, "MACFMT", "MACFMT")\n",
            MACARG(vbss_capability->ruid), r->vbss_capa.max_vbss, r->vbss_capa.vbss_subtract,
            r->vbss_capa.vbssid_restrictions, r->vbss_capa.match_and_mask_restrictions,
            r->vbss_capa.fixed_bits_restrictions, MACARG(r->vbss_capa.fixed_bits_mask),
            MACARG(r->vbss_capa.fixed_bits_value));
    }

    return PROCESS_CMDU_OK;
}

uint8_t processVBSSRequest(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct al_device *d = alDeviceFindAny(src);
    struct vbssCreationTLV *vbss_creation = NULL;
    struct vbssDestructionTLV *vbss_destruction = NULL;
    struct clientCapabilityReportTLV *client_capa = NULL;

    if (!d) {
        DEBUG_WARNING("Unknown src AL MAC["MACFMT"] \n", MACARG(src));
        return PROCESS_CMDU_KO;
    }

    vbss_creation = (struct vbssCreationTLV *)getSubTypedTLV(c, TLV_TYPE_VBSS, TLV_SUB_TYPE_VBSS_CREATION, 0);
    if (vbss_creation) {
        DEBUG_INFO("Radio["MACFMT"] receive vbss creation request, bssid["MACFMT"], client_assoc: %u\n",
            MACARG(vbss_creation->ruid), MACARG(vbss_creation->bssid), vbss_creation->client_assoc);

        client_capa = (struct clientCapabilityReportTLV *)getTypedTLV(c, TLV_TYPE_CLIENT_CAPABILITY_REPORT, 0);
        if (client_capa && CAP_REPORT_SUCCESS == client_capa->result) {
            sendVbssCreationEvent(c->id, src, vbss_creation->ruid, vbss_creation->bssid, &vbss_creation->ssid,
                &vbss_creation->wpa_password, vbss_creation->client_mac, vbss_creation->client_assoc,
                &vbss_creation->ptk, vbss_creation->tx_packet_number, &vbss_creation->gtk,
                vbss_creation->group_tx_packet_number, &client_capa->assc_req);
        }
        else {
            sendVbssCreationEvent(c->id, src, vbss_creation->ruid, vbss_creation->bssid, &vbss_creation->ssid,
                &vbss_creation->wpa_password, vbss_creation->client_mac, vbss_creation->client_assoc,
                &vbss_creation->ptk, vbss_creation->tx_packet_number, &vbss_creation->gtk,
                vbss_creation->group_tx_packet_number, NULL);
        }
        return PROCESS_CMDU_OK;
    }

    vbss_destruction = (struct vbssDestructionTLV *)getSubTypedTLV(c, TLV_TYPE_VBSS, TLV_SUB_TYPE_VBSS_DESTRUCTION, 0);
    if (vbss_destruction) {
        DEBUG_INFO("Radio["MACFMT"] receive vbss destruction request, bssid["MACFMT"], disassociate_client: %u\n",
            MACARG(vbss_destruction->ruid), MACARG(vbss_destruction->bssid), vbss_destruction->disassociate_client);

        sendVbssDestructionEvent(c->id, src, vbss_destruction->ruid, vbss_destruction->bssid,
            vbss_destruction->disassociate_client);
        return PROCESS_CMDU_OK;
    }

    return PROCESS_CMDU_KO;
}

uint8_t processVBSSResponse(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct vbssEventTLV *vbss_event = NULL;
    struct al_device *d = alDeviceFindAny(src);
    int i = 0;

    if (!d) {
        DEBUG_WARNING("Unknown src AL MAC["MACFMT"] \n", MACARG(src));
        return PROCESS_CMDU_KO;
    }

    vbss_event = (struct vbssEventTLV *)getSubTypedTLV(c, TLV_TYPE_VBSS, TLV_SUB_TYPE_VBSS_EVENT, i++);
    if (vbss_event) {
        DEBUG_INFO("receive vbss response. radio["MACFMT"], bssid["MACFMT"], success: %u\n",
            MACARG(vbss_event->ruid), MACARG(vbss_event->bssid), vbss_event->success);

        sendVbssResponseEvent(src, vbss_event->ruid, vbss_event->bssid, vbss_event->success);

        return PROCESS_CMDU_OK;
    }

    return PROCESS_CMDU_KO;
}

uint8_t processClientSecurityContextRequest(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct clientInfoTLV *client_info = NULL;
    struct al_device *d = alDeviceFindAny(src);

    if (!d) {
        DEBUG_WARNING("Unknown src AL MAC["MACFMT"] \n", MACARG(src));
        return PROCESS_CMDU_KO;
    }

    client_info = (struct clientInfoTLV *)getTypedTLV(c, TLV_TYPE_CLIENT_INFO, 0);
    if (client_info) {
        DEBUG_INFO("receive client security context request. bssid["MACFMT"], sta_mac["MACFMT"]\n",
            MACARG(client_info->bssid), MACARG(client_info->client));

        sendVbssClientSecurityContextReqEvent(c->id, src, client_info->bssid, client_info->client);
        return PROCESS_CMDU_OK;
    }

    return PROCESS_CMDU_KO;
}

uint8_t processClientSecurityContextResponse(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct clientInfoTLV *client_info = NULL;
    struct clientSecurityContextTLV *client_security_info = NULL;
    struct client *sta = NULL;
    struct wifi_interface *wif = NULL;
    struct al_device *d = alDeviceFindAny(src);

    if (!d) {
        DEBUG_WARNING("Unknown src AL MAC["MACFMT"] \n", MACARG(src));
        return PROCESS_CMDU_KO;
    }

    client_info = (struct clientInfoTLV *)getTypedTLV(c, TLV_TYPE_CLIENT_INFO, 0);
    client_security_info = (struct clientSecurityContextTLV *)getSubTypedTLV(c, TLV_TYPE_VBSS,
                TLV_SUB_TYPE_CLIENT_SECURITY_CONTEXT, 0);

    if (!client_info || !client_security_info)
        return PROCESS_CMDU_KO;

    DEBUG_INFO("receive client security context response. bssid["MACFMT"], sta_mac["MACFMT"]\n",
        MACARG(client_info->bssid), MACARG(client_info->client));

    wif = (struct wifi_interface *)interfaceFind(NULL, client_info->bssid, interface_type_wifi);
    if (!wif) {
        DEBUG_ERROR("VBSS["MACFMT"] not found or vbss is NULL\n", MACARG(client_info->bssid));
        return PROCESS_CMDU_KO;
    }

    sta = clientFind(NULL, wif, client_info->client);
    if (!sta)
        return PROCESS_CMDU_KO;

    wifiInterfaceAddVbssClientContext(wif, sta->mac, client_security_info->tx_packet_number,
        client_security_info->group_tx_packet_number, client_security_info->ptk.datap,
        client_security_info->ptk.len, client_security_info->gtk.datap, client_security_info->gtk.len);

    sendVbssMovePreparationRequest(idx2InterfaceName(d->recv_intf_idx), getNextMid(), d->al_mac,
            client_info->bssid, client_info->client);

    return PROCESS_CMDU_OK;
}

uint8_t processTriggerCSARequest(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct clientInfoTLV *client_info = NULL;
    struct triggerCSATLV *csa_info = NULL;
    struct al_device *d = alDeviceFindAny(src);

    if (!d) {
        DEBUG_WARNING("Unknown src AL MAC["MACFMT"] \n", MACARG(src));
        return PROCESS_CMDU_KO;
    }

    client_info = (struct clientInfoTLV *)getTypedTLV(c, TLV_TYPE_CLIENT_INFO, 0);
    csa_info = (struct triggerCSATLV *)getSubTypedTLV(c, TLV_TYPE_VBSS, TLV_SUB_TYPE_TRIGGER_CSA, 0);

    if (!client_info || !csa_info)
        return PROCESS_CMDU_KO;

    DEBUG_INFO("receive trigger csa request msg. bssid["MACFMT"], sta_mac["MACFMT"], ruid["MACFMT"], csa_channel: %u, opclass: %u\n",
            MACARG(client_info->bssid), MACARG(client_info->client), MACARG(csa_info->ruid), csa_info->csa_channel, csa_info->opclass);

    //1. send Trigger CSA response containing the same received CSA TLV & Client info TLV
    sendTriggerCSAResponse(idx2InterfaceName(d->recv_intf_idx), c->id, d->al_mac, csa_info->ruid, client_info->bssid,
            client_info->client, csa_info->csa_channel, csa_info->opclass);
    //2. send csa frame to sta with csa element -TODO

    return PROCESS_CMDU_OK;
}

uint8_t processTriggerCSAResponse(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct clientInfoTLV *client_info = NULL;
    struct triggerCSATLV *csa_info = NULL;

    client_info = (struct clientInfoTLV *)getTypedTLV(c, TLV_TYPE_CLIENT_INFO, 0);
    csa_info = (struct triggerCSATLV *)getSubTypedTLV(c, TLV_TYPE_VBSS, TLV_SUB_TYPE_TRIGGER_CSA, 0);

    if (!client_info || !csa_info)
        return PROCESS_CMDU_KO;

    DEBUG_INFO("receive trigger csa response msg. bssid["MACFMT"], sta_mac["MACFMT"], ruid["MACFMT"], csa_channel: %u, opclass: %u\n",
            MACARG(client_info->bssid), MACARG(client_info->client), MACARG(csa_info->ruid), csa_info->csa_channel, csa_info->opclass);

    //send trigger csa response event
    sendVbssTriggerCSARspEvent(src, client_info->bssid, client_info->client);

    return PROCESS_CMDU_OK;
}

uint8_t processVbssMovePreparationRequest(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct clientInfoTLV *client_info = NULL;
    struct al_device *d = alDeviceFindAny(src);

    if (!d) {
        DEBUG_WARNING("Unknown src AL MAC["MACFMT"] \n", MACARG(src));
        return PROCESS_CMDU_KO;
    }

    client_info = (struct clientInfoTLV *)getTypedTLV(c, TLV_TYPE_CLIENT_INFO, 0);

    if (!client_info)
        return PROCESS_CMDU_KO;

    DEBUG_INFO("receive vbss move preparation request msg. bssid["MACFMT"], sta_mac["MACFMT"]\n",
            MACARG(client_info->bssid), MACARG(client_info->client));

    //1. send Vbss Move Preparation response containing the same Client info TLV
    sendVbssMovePreparationResponse(idx2InterfaceName(d->recv_intf_idx), c->id, d->al_mac, client_info->bssid, client_info->client);
    //2. if the client of the VBSS has any negotiated TBTT agreements, should teardown them by sending a TWT Teardown frame(the Negotiation Type subfield 1)
    //TODO
    //3. if the client of the VBSS has any individual TWT agreements, should teardown then by sending a TWT Teardown frame.
    //TODO
    //4. if the client of the VBSS has any broadcast TWT memberships, should send a Reject Broadcast TWT with the TWT ID
    //TODO

    return PROCESS_CMDU_OK;
}

uint8_t processVbssMovePreparationResponse(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct clientInfoTLV *client_info = NULL;
    struct radio *target_radio = NULL;
    struct al_device  *target_agent = NULL;
    struct wifi_interface *wif = NULL;
    struct client *sta = NULL;

    client_info = (struct clientInfoTLV *)getTypedTLV(c, TLV_TYPE_CLIENT_INFO, 0);

    if (!client_info)
        return PROCESS_CMDU_KO;

    DEBUG_INFO("receive vbss move preparation response msg. bssid["MACFMT"], sta_mac["MACFMT"]\n",
            MACARG(client_info->bssid), MACARG(client_info->client));

    wif = (struct wifi_interface *)interfaceFind(NULL, client_info->bssid, interface_type_wifi);
    if (!wif)
        return PROCESS_CMDU_KO;

    sta = clientFind(NULL, wif, client_info->client);
    if (!sta)
        return PROCESS_CMDU_KO;

    // find target agent
    target_radio  = radioFind(NULL, sta->vbss_ctx.target_ruid);
    if (!target_radio) {
        DEBUG_ERROR("bssid["MACFMT"], sta_mac["MACFMT"] vbss but cannot find target ruid["MACFMT"]!\n",
            MACARG(client_info->bssid), MACARG(client_info->client), MACARG(sta->vbss_ctx.target_ruid));
        return PROCESS_CMDU_KO;
    }

    target_agent = target_radio->d;
    if (!target_agent)
        return PROCESS_CMDU_KO;

    // send VBSS request msg to dst agent
    sendVBSSCreationRequest(idx2InterfaceName(target_agent->recv_intf_idx), getNextMid(), target_agent->al_mac,
        target_radio->uid, &sta->wif->bssInfo, sta->wif->vbss_client_context, sta, NULL);

    return PROCESS_CMDU_OK;
}

uint8_t processVbssMoveCancelRequest(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct clientInfoTLV *client_info = NULL;
    struct al_device *d = alDeviceFindAny(src);

    if (!d) {
        DEBUG_WARNING("Unknown src AL MAC["MACFMT"] \n", MACARG(src));
        return PROCESS_CMDU_KO;
    }

    client_info = (struct clientInfoTLV *)getTypedTLV(c, TLV_TYPE_CLIENT_INFO, 0);

    if (!client_info)
        return PROCESS_CMDU_KO;

    DEBUG_INFO("receive vbss move cancel request msg. bssid["MACFMT"], sta_mac["MACFMT"]\n",
            MACARG(client_info->bssid), MACARG(client_info->client));

    //1. send Vbss Move Cancel response containing the same Client info TLV
    sendVbssMoveCancelResponse(idx2InterfaceName(d->recv_intf_idx), c->id, d->al_mac, client_info->bssid, client_info->client);
    //2. may renegotiate block ack mode with the client
    //TODO
    //3. may renegotiate TBTT or TWT or other agreements with the client
    //TODO

    return PROCESS_CMDU_OK;
}

uint8_t processVbssMoveCancelResponse(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct clientInfoTLV *client_info = NULL;

    client_info = (struct clientInfoTLV *)getTypedTLV(c, TLV_TYPE_CLIENT_INFO, 0);

    if (!client_info)
        return PROCESS_CMDU_KO;

    DEBUG_INFO("receive vbss move cancel response msg. bssid["MACFMT"], sta_mac["MACFMT"]\n",
            MACARG(client_info->bssid), MACARG(client_info->client));

    //nothing to do

    return PROCESS_CMDU_OK;
}

uint8_t processServicePrioritizationRequest(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst)
{
    struct dscpMappingTableTLV *dscpmap;
    struct al_device *d;

    if ((!(d = alDeviceFindAny(src))) || (d!=registrar))
        return PROCESS_CMDU_OK;

    if ((dscpmap = (struct dscpMappingTableTLV *)getTypedTLV(c, TLV_TYPE_DSCP_MAPPING_TABLE, 0))){
       local_policy.dscp2up_set = 1;
       memcpy(local_policy.dscp2up_table, dscpmap->dscp_pcp_mapping, DSCP2UP_SIZE);
    }

    if (local_policy.dscp2up_set) {
    //do set dscp2up to local
    }

    return PROCESS_CMDU_OK;
}
