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

#include "1905_tlvs.h"
#include "packet_tools.h"
#include "tlv.h"

static struct TLVDesc *getSubTLVDescExt(struct TLV *tlv, uint16_t subtype);
static struct TLVStruct *link_metric_query_decode(struct TLVDesc *desc, dlist_head *parent, uint8_t **pp, uint16_t *len);
static int link_metric_query_encode(struct TLVStruct *s, uint8_t **pp, uint16_t *len);
static uint16_t link_metric_query_length(struct TLVStruct *s);
static struct TLVStruct *ap_metrics_decode(struct TLVDesc *desc, dlist_head *parent, uint8_t **pp, uint16_t *len);
static int ap_metrics_encode(struct TLVStruct *s, uint8_t **pp, uint16_t *len);
static uint16_t ap_metrics_length(struct TLVStruct *s);
static struct TLVStruct *p2Error_code_decode(struct TLVDesc *desc, dlist_head *parent, uint8_t **pp, uint16_t *len);
static int p2Error_code_encode(struct TLVStruct *s, uint8_t **pp, uint16_t *len);
static uint16_t p2Error_code_length(struct TLVStruct *s);
static struct TLVStruct *channel_scan_result_decode(struct TLVDesc *desc, dlist_head *parent, uint8_t **pp, uint16_t *len);
static uint16_t channel_scan_result_length(struct TLVStruct *s);
static struct TLVStruct *super_tlv_decode(struct TLVDesc *desc, dlist_head *parent, uint8_t **pp, uint16_t *len);
static int super_tlv_encode(struct TLVStruct *s, uint8_t **pp, uint16_t *plen);
static uint16_t super_tlv_length(struct TLVStruct *s);
static void super_tlv_free(struct TLVStruct *s);


#define DECLARE_STRUCT_DESC(name) \
static struct TLVDesc SNAME(name)

#define TLVStructDeclare(_type, _parent) TLVStructNew(&SNAME(_type), _parent, 0)

DECLARE_STRUCT_DESC(serviceStruct) =
    TLV_DESC_1FIELD(serviceStruct, 1, service, fmt_unsigned, NULL);

DECLARE_STRUCT_DESC(txOpclassStruct) =
    TLV_DESC_3FIELDS(txOpclassStruct, 1, opclass, fmt_unsigned, max_tx_power, fmt_unsigned, non_op_chans, fmt_l1v, NULL);

DECLARE_STRUCT_DESC(apOperationBSSStruct) =
    TLV_DESC_2FIELDS(apOperationBSSStruct, 1, bssid, fmt_binary, ssid, fmt_l1v, NULL);

DECLARE_STRUCT_DESC(apOperationRadioStruct) =
    TLV_DESC_1FIELD(macStruct, 1, mac, fmt_binary, &SNAME(apOperationBSSStruct), NULL);

DECLARE_STRUCT_DESC(associatedClientStruct) =
    TLV_DESC_2FIELDS(associatedClientStruct, 2, mac, fmt_binary, age, fmt_binary, NULL);

DECLARE_STRUCT_DESC(associatedBSSStruct) =
    TLV_DESC_1FIELD(associatedBSSStruct, 1, bssid, fmt_binary, &SNAME(associatedClientStruct), NULL);

DECLARE_STRUCT_DESC(ipv4AddrStruct) =
    TLV_DESC_3FIELDS(ipv4AddrStruct, 1, proto, fmt_unsigned, ip, fmt_unsigned, dhcp_ip, fmt_unsigned, NULL);

DECLARE_STRUCT_DESC(interfaceipv4Struct) =
    TLV_DESC_1FIELD(macStruct, 1, mac, fmt_binary, &SNAME(ipv4AddrStruct), NULL);

DECLARE_STRUCT_DESC(ipv6AddrStruct) =
    TLV_DESC_3FIELDS(ipv6AddrStruct, 1, proto, fmt_unsigned, ip, fmt_binary, origin_ip, fmt_binary, NULL);

DECLARE_STRUCT_DESC(interfaceipv6Struct) =
    TLV_DESC_2FIELDS(interfaceipv6Struct, 1, mac, fmt_binary, local_ip, fmt_binary, &SNAME(ipv6AddrStruct), NULL);

DECLARE_STRUCT_DESC_GLOBAL(macStruct) =
    TLV_DESC_1FIELD(macStruct, 1, mac, fmt_binary, NULL);

DECLARE_STRUCT_DESC(non1905NeighborStruct) =
    TLV_DESC_1FIELD(macStruct, 0, mac, fmt_binary, NULL);

DECLARE_STRUCT_DESC(transmitterLinkMetricEntriesStruct) =
    TLV_DESC_9FIELDS(transmitterLinkMetricEntriesStruct, 0, local_interface_address, fmt_binary, neighbor_interface_address, fmt_binary,
                       intf_type, fmt_unsigned, bridge_flag, fmt_unsigned, tx_link_metrics.tx_packet_errors, fmt_unsigned, tx_link_metrics.tx_packet_ok, fmt_unsigned,
                       tx_link_metrics.tx_max_xput, fmt_unsigned, tx_link_metrics.tx_link_availability, fmt_unsigned, tx_link_metrics.tx_phy_rate, fmt_unsigned, NULL);

DECLARE_STRUCT_DESC(receiverLinkMetricEntriesStruct) =
    TLV_DESC_6FIELDS(receiverLinkMetricEntriesStruct, 0, local_interface_address, fmt_binary, neighbor_interface_address, fmt_binary,
                       intf_type, fmt_unsigned, rx_link_metrics.rx_packet_errors, fmt_unsigned, rx_link_metrics.rx_packet_ok, fmt_unsigned, rx_link_metrics.rx_rssi, fmt_unsigned, NULL);

DECLARE_STRUCT_DESC(i1905NeighborStruct) =
    TLV_DESC_2FIELDS(i1905NeighborStruct, 0, mac, fmt_binary, bridge, fmt_binary, NULL);

DECLARE_STRUCT_DESC(interfaceStruct) =
    TLV_DESC_3FIELDS(interfaceStruct, 1, mac, fmt_binary, media_type, fmt_unsigned, media_specific_info, fmt_l1v, NULL);

DECLARE_STRUCT_DESC(steerPolicyStruct) =
    TLV_DESC_4FIELDS(steerPolicyStruct, 1, rid, fmt_binary, policy, fmt_unsigned, chan_util, fmt_unsigned,
                        rcpi_thresh, fmt_unsigned, NULL);

DECLARE_STRUCT_DESC(metricRptPolicyStruct) =
    TLV_DESC_5FIELDS(metricRptPolicyStruct, 1, rid, fmt_binary, rcpi_thresh, fmt_unsigned,
                        rcpi_margin, fmt_unsigned, chan_util_thresh, fmt_unsigned, policy, fmt_unsigned, NULL);

DECLARE_STRUCT_DESC(opClassStruct) =
    TLV_DESC_3FIELDS(opClassStruct, 1, opclass, fmt_unsigned, non_operable_chans, fmt_l1v, pref_reason, fmt_unsigned, NULL);

DECLARE_STRUCT_DESC(opRestChanStruct) =
    TLV_DESC_2FIELDS(opRestChanStruct, 1, channel, fmt_unsigned, freq_sep, fmt_unsigned, NULL);

DECLARE_STRUCT_DESC(opRestOpclassStruct) =
    TLV_DESC_1FIELD(opRestOpclassStruct, 1, opclass, fmt_unsigned, &SNAME(opRestChanStruct), NULL);

DECLARE_STRUCT_DESC(opclassChanPairStruct) =
    TLV_DESC_2FIELDS(opclassChanPairStruct, 1, opclass, fmt_unsigned, channel, fmt_unsigned, NULL);

DECLARE_STRUCT_DESC(assocLinkMetricStruct) =
    TLV_DESC_5FIELDS(assocLinkMetricStruct, 1, bssid, fmt_binary, age, fmt_unsigned,
                        metrics.mac_rate_dl, fmt_unsigned, metrics.mac_rate_ul, fmt_unsigned,
                        metrics.rcpi_ul, fmt_unsigned, NULL);

DECLARE_STRUCT_DESC(unassocMetricQryStruct) =
    TLV_DESC_1FIELD(unassocMetricQryStruct, 1, channel, fmt_unsigned, &SNAME(macStruct), NULL);

DECLARE_STRUCT_DESC(unassocMetricRspStruct) =
    TLV_DESC_4FIELDS(unassocMetricRspStruct, 1, sta, fmt_binary, channel, fmt_unsigned, age, fmt_unsigned,
                        rcpi_ul, fmt_unsigned, NULL);

DECLARE_STRUCT_DESC(chanRptStruct) =
    TLV_DESC_1FIELD(chanRptStruct, 1, chanreport, fmt_l1vv, NULL);

DECLARE_STRUCT_DESC(measuredElemStruct) =
    TLV_DESC_1FIELD(measuredElemStruct, 1, elem, fmt_elem, NULL);

DECLARE_STRUCT_DESC(targetBssStruct) =
    TLV_DESC_3FIELDS(targetBssStruct, 1, target, fmt_binary, opclass, fmt_unsigned, channel, fmt_unsigned, NULL);

DECLARE_STRUCT_DESC(p2targetBssStruct) =
    TLV_DESC_4FIELDS(targetBssStruct, 1, target, fmt_binary, opclass, fmt_unsigned, channel, fmt_unsigned, reason, fmt_unsigned, NULL);


DECLARE_STRUCT_DESC(opClassChanStruct) =
    TLV_DESC_2FIELDS(opClassChanStruct, 1, opclass, fmt_unsigned, chan, fmt_l1v, NULL);

DECLARE_STRUCT_DESC(scanCapaRadioStruct) =
    TLV_DESC_3FIELDS(scanCapaRadioStruct, 1, ruid, fmt_binary, onboot_impact, fmt_unsigned, min_scan_interval, fmt_binary, &SNAME(opClassChanStruct), NULL);

DECLARE_STRUCT_DESC(scanReqRadioStruct) =
    TLV_DESC_1FIELD(macStruct, 1, mac, fmt_binary, &SNAME(opClassChanStruct), NULL);

DECLARE_STRUCT_DESC(bssLoadEleStruct) =
    TLV_DESC_2FIELDS(bssLoadEleStruct,  0, chann_utilize, fmt_unsigned, sta_cnt, fmt_unsigned, NULL);

DECLARE_STRUCT_DESC(scanNeighborStruct) =
    TLV_DESC_5FIELDS(scanNeighborStruct, 2, bssid, fmt_binary, ssid, fmt_l1v, signal_strength, fmt_unsigned, chanbw, fmt_l1v,
                        bss_field, fmt_unsigned, &SNAME(bssLoadEleStruct), NULL);

DECLARE_STRUCT_DESC(scanResultStruct) =
    TLV_DESC_3_2FIELDS(scanResultStruct, 0, timestamp, fmt_l1v, utilization, fmt_unsigned, noise, fmt_unsigned, aggre_scan_dur, fmt_unsigned,
                        scan_type, fmt_unsigned, &SNAME(scanNeighborStruct), NULL);

DECLARE_STRUCT_DESC(cacReqRadioStruct) =
    TLV_DESC_4FIELDS(cacRadioStruct, 1, ruid, fmt_binary, opclass, fmt_unsigned, channel, fmt_unsigned, cac_flag, fmt_unsigned, NULL);

DECLARE_STRUCT_DESC(cacTermRadioStruct) =
    TLV_DESC_3FIELDS(cacTermRadioStruct, 1, ruid, fmt_binary, opclass, fmt_unsigned, channel, fmt_unsigned, NULL);

DECLARE_STRUCT_DESC(cacReptRadioStruct) =
    TLV_DESC_4FIELDS(cacRadioStruct, 1, ruid, fmt_binary, opclass, fmt_unsigned, channel, fmt_unsigned, cac_flag, fmt_unsigned, &SNAME(opClassChanStruct), NULL);

DECLARE_STRUCT_DESC(cacStatChanStruct) =
    TLV_DESC_3FIELDS(cacStatChanStruct, 1, opclass, fmt_unsigned, channel, fmt_unsigned, time, fmt_unsigned, NULL);

DECLARE_STRUCT_DESC(cacStatCacChanStruct) =
    TLV_DESC_3FIELDS(cacStatCacChanStruct, 1, opclass, fmt_unsigned, channel, fmt_unsigned, cnt_down, fmt_binary, NULL);

DECLARE_STRUCT_DESC(cacCapaOpclassStruct) =
    TLV_DESC_2FIELDS(cacCapaOpclassStruct, 1, opclass, fmt_unsigned, chan, fmt_l1v, NULL);

DECLARE_STRUCT_DESC(cacTypeStruct) =
    TLV_DESC_2FIELDS(cacTypeStruct, 1, method, fmt_unsigned, dur, fmt_binary, &SNAME(cacCapaOpclassStruct), NULL);

DECLARE_STRUCT_DESC(cacCapaRadioStruct) =
    TLV_DESC_1FIELD(macStruct, 1, mac, fmt_binary, &SNAME(cacTypeStruct), NULL);

DECLARE_STRUCT_DESC(trafficPolicyStruct) =
    TLV_DESC_2FIELDS(trafficPolicyStruct, 1, ssid, fmt_l1v, vlanid, fmt_unsigned, NULL);

DECLARE_STRUCT_DESC(assocStatNotifyStruct) =
    TLV_DESC_2FIELDS(assocStatNotifyStruct, 1, bssid, fmt_binary, status, fmt_unsigned, NULL);

DECLARE_STRUCT_DESC(bStaMacStruct) =
    TLV_DESC_1FIELD(macStruct, 0, mac, fmt_binary, NULL);

DECLARE_STRUCT_DESC(associatedSTAExtLinkMetricsStruct) =
    TLV_DESC_5FIELDS(associatedSTAExtLinkMetricsStruct, 1, bssid, fmt_binary, metrics.last_data_rate_dl, fmt_unsigned,
                        metrics.last_data_rate_ul, fmt_unsigned, metrics.util_rx, fmt_unsigned, metrics.util_tx, fmt_unsigned, NULL);

DECLARE_STRUCT_DESC(wifi6RoleStruct) =
    TLV_DESC_6FIELDS(wifi6RoleStruct, 1, role_he_mcs, fmt_l4bitsv, bf_ofdma, fmt_unsigned, max_um_mimo, fmt_unsigned, max_dl_ofdma_tx, fmt_unsigned, max_ul_ofdma_rx, fmt_unsigned, other_capa, fmt_unsigned, NULL);

DECLARE_STRUCT_DESC(tidQueueSizeStruct) =
    TLV_DESC_2FIELDS(tidQueueSizeStruct, 1, tid, fmt_unsigned, queue_size, fmt_unsigned, NULL);

DECLARE_STRUCT_DESC(bssConfigBssStruct) =
    TLV_DESC_3FIELDS(bssConfigBssStruct, 1, bssid, fmt_binary, flag, fmt_binary, ssid, fmt_l1v, NULL);

DECLARE_STRUCT_DESC(bssConfigRadioStruct) =
    TLV_DESC_1FIELD(macStruct, 1, mac, fmt_binary, &SNAME(bssConfigBssStruct), NULL);

DECLARE_STRUCT_DESC(deviceInventoryRadioStruct) =
    TLV_DESC_2FIELDS(deviceInventoryRadioStruct, 1, ruid, fmt_binary, chip_vendor, fmt_l1vv, NULL);

DECLARE_STRUCT_DESC(agentListAgentStruct) =
    TLV_DESC_3FIELDS(agentListAgentStruct, 1, al_mac, fmt_binary, map_profile, fmt_unsigned, security, fmt_unsigned, NULL);

DECLARE_STRUCT_DESC(akmSuiteCapaStruct) =
    TLV_DESC_2FIELDS(akmSuiteCapaStruct, 1, cap.oui, fmt_binary, cap.akm_suite_type, fmt_unsigned, NULL);

DECLARE_STRUCT_DESC(anticipatedChannelPrefOpClassChanStruct) =
    TLV_DESC_3FIELDS(anticipatedChannelPrefOpClassChanStruct, 1, opclass, fmt_unsigned, chan, fmt_l1v, reserved, fmt_skip, NULL);

DECLARE_STRUCT_DESC(anticiaptedChannelUsageEntryStruct) =
    TLV_DESC_9FIELDS(anticiaptedChannelUsageEntryStruct, 1, burst_start, fmt_unsigned, burst_length, fmt_unsigned, repetitions, fmt_unsigned, burst_interval, fmt_unsigned, ru_bitmask, fmt_l1vv, transmitter_id, fmt_binary, power_level, fmt_unsigned, reason, fmt_unsigned, reserved, fmt_skip, NULL);

DECLARE_STRUCT_DESC(apOperationVBSSStruct) =
    TLV_DESC_2FIELDS(apOperationVBSSStruct, 1, bssid, fmt_binary, ssid, fmt_l1v, NULL);

DECLARE_STRUCT_DESC(vbssConfigurationReportRadioStruct) =
    TLV_DESC_1FIELD(macStruct, 1, mac, fmt_binary, &SNAME(apOperationVBSSStruct), NULL);

#define ONAME(_struct) _tlvops_##_struct
#define DECLARE_TLV_OPS(_name, _encode, _decode, _length, _free) \
static struct TLVOperator ONAME(_name) = {\
    .encode = _encode, \
    .decode = _decode, \
    .length = _length, \
    .free = _free, \
}


static struct TLVDesc _tlv_descs[TLV_TYPE_MAX] = {
    [TLV_TYPE_END_OF_MESSAGE] = {
        .name = "EOF",
        .size = sizeof(struct TLV),
    },
    [TLV_TYPE_AL_MAC_ADDRESS_TYPE] =
        TLV_DESC_1FIELD(macAddressTLV, 0, mac, fmt_binary, NULL),
    [TLV_TYPE_MAC_ADDRESS_TYPE] =
        TLV_DESC_1FIELD(macAddressTLV, 0, mac, fmt_binary, NULL),
    [TLV_TYPE_DEVICE_INFORMATION_TYPE] =
        TLV_DESC_1FIELD(macAddressTLV, 0, mac, fmt_binary, &SNAME(interfaceStruct), NULL),
    [TLV_TYPE_NON_1905_NEIGHBOR_DEVICE_LIST] =
        TLV_DESC_1FIELD(macAddressTLV, 0, mac, fmt_binary, &SNAME(non1905NeighborStruct), NULL),
    [TLV_TYPE_NEIGHBOR_DEVICE_LIST] =
        TLV_DESC_1FIELD(macAddressTLV, 0, mac, fmt_binary, &SNAME(i1905NeighborStruct), NULL),
    [TLV_TYPE_LINK_METRIC_QUERY] =
        TLV_DESC_3FIELDS(linkMetricQueryTLV, 0, neighbor, fmt_unsigned, specific_neighbor, fmt_binary, link_metrics_type, fmt_unsigned, NULL),
    [TLV_TYPE_TRANSMITTER_LINK_METRIC] =
        TLV_DESC_2FIELDS(transmitterLinkMetricTLV, 0, local_al_address, fmt_binary, neighbor_al_address, fmt_binary, &SNAME(transmitterLinkMetricEntriesStruct), NULL),
    [TLV_TYPE_RECEIVER_LINK_METRIC] =
        TLV_DESC_2FIELDS(receiverLinkMetricTLV, 0, local_al_address, fmt_binary, neighbor_al_address, fmt_binary, &SNAME(receiverLinkMetricEntriesStruct), NULL),
    [TLV_TYPE_SEARCHED_ROLE] =
        TLV_DESC_1FIELD(u8TLV, 0, v1, fmt_binary, NULL),
    [TLV_TYPE_SUPPORTED_ROLE] =
        TLV_DESC_1FIELD(u8TLV, 0, v1, fmt_binary, NULL),
    [TLV_TYPE_SUPPORTED_FREQ_BAND] =
        TLV_DESC_1FIELD(u8TLV, 0, v1, fmt_binary, NULL),
    [TLV_TYPE_AUTOCONFIG_FREQ_BAND] =
        TLV_DESC_1FIELD(u8TLV, 0, v1, fmt_binary, NULL),
    [TLV_TYPE_WSC] =
        TLV_DESC_1FIELD(wscTLV, 0, wsc, fmt_l0vv, NULL),
    [TLV_TYPE_DEVICE_IDENTIFICATION] =
        TLV_DESC_3FIELDS(deviceIDTLV, 0, friendly_name, fmt_binary, manufacturer, fmt_binary, model_name, fmt_binary, NULL),
    [TLV_TYPE_IPV4] =
        TLV_DESC_0FIELD(TLV, 0, &SNAME(interfaceipv4Struct), NULL),
    [TLV_TYPE_IPV6] =
        TLV_DESC_0FIELD(TLV, 0, &SNAME(interfaceipv6Struct), NULL),
    [TLV_TYPE_1905_PROFILE_VERSION]
        TLV_DESC_1FIELD(u8TLV, 0, v1, fmt_binary, NULL),
    [TLV_TYPE_AP_RADIO_IDENTIFIER] =
        TLV_DESC_1FIELD(macAddressTLV, 0, mac, fmt_binary, NULL),

    [TLV_TYPE_SUPPORTED_SERVICE] =
        TLV_DESC_0FIELD(TLV, 0, &SNAME(serviceStruct), NULL),
    [TLV_TYPE_SEARCHED_SERVICE] =
        TLV_DESC_0FIELD(TLV, 0, &SNAME(serviceStruct), NULL),
    [TLV_TYPE_AP_RADIO_IDENTIFIER] =
        TLV_DESC_1FIELD(macAddressTLV, 0, mac, fmt_binary, NULL),
    [TLV_TYPE_AP_OPERATIONAL_BSS] =
        TLV_DESC_0FIELD(TLV, 0, &SNAME(apOperationRadioStruct), NULL),
    [TLV_TYPE_ASSOCIATED_CLIENTS] =
        TLV_DESC_0FIELD(TLV, 0, &SNAME(associatedBSSStruct), NULL),

    [TLV_TYPE_AP_RADIO_BASIC_CAPABILITIES] =
        TLV_DESC_2FIELDS(radioBasicCapabilityTLV, 0, rid, fmt_binary, max_bss, fmt_unsigned,
            &SNAME(txOpclassStruct), NULL),
    [TLV_TYPE_AP_HT_CAPABILITIES] =
        TLV_DESC_2FIELDS(htCapabilityTLV, 0, rid, fmt_binary, capa.capa, fmt_unsigned, NULL),
    [TLV_TYPE_AP_VHT_CAPABILITIES] =
        TLV_DESC_4FIELDS(vhtCapabilityTLV, 0, rid, fmt_binary, capa.tx_mcs, fmt_unsigned, capa.rx_mcs, fmt_unsigned,
                            capa.capa, fmt_unsigned, NULL),

    [TLV_TYPE_AP_CAPABILITY] =
        TLV_DESC_1FIELD(u8TLV, 0, v1, fmt_binary, NULL),
    [TLV_TYPE_MULTIAP_PROFILE] =
        TLV_DESC_1FIELD(u8TLV, 0, v1, fmt_binary, NULL),
    [TLV_TYPE_PROFILE2_AP_CAPABILITY] =
        TLV_DESC_4FIELDS(profile2ApCapabilityTLV, 0, max_prio_rules, fmt_unsigned, reserved, fmt_unsigned, cap, fmt_unsigned, max_vid, fmt_unsigned, NULL),

    [TLV_TYPE_AP_HE_CAPABILITIES] =
        TLV_DESC_3FIELDS(heCapabilityTLV, 0, rid, fmt_binary, capa.mcs, fmt_l1v, capa.capa, fmt_unsigned, NULL),
    [TLV_TYPE_STEERING_POLICY] =
        TLV_DESC_0FIELD(TLV, 0, &SNAME(macStruct), &SNAME(macStruct), &SNAME(steerPolicyStruct), NULL),
    [TLV_TYPE_METRIC_REPORTING_POLICY] =
        TLV_DESC_1FIELD(metricReportPolicyTLV, 0, interval, fmt_unsigned, &SNAME(metricRptPolicyStruct), NULL),
    [TLV_TYPE_CHANNEL_PREFERENCE] =
        TLV_DESC_1FIELD(macAddressTLV, 0, mac, fmt_binary, &SNAME(opClassStruct), NULL),
    [TLV_TYPE_RADIO_OPERATION_RESTRICTION] =
        TLV_DESC_1FIELD(macAddressTLV, 0, mac, fmt_binary, &SNAME(opRestOpclassStruct), NULL),
    [TLV_TYPE_TRANSMIT_POWER_LIMIT] =
        TLV_DESC_2FIELDS(txPwrLimitTLV, 0, rid, fmt_binary, tx_pwr, fmt_unsigned, NULL),
    [TLV_TYPE_CHANNEL_SELECTION_RESPONSE] =
        TLV_DESC_2FIELDS(chanSelRspTLV, 0, rid, fmt_binary, code, fmt_unsigned, NULL),
    [TLV_TYPE_OPERATING_CHANNEL_REPORT] =
        TLV_DESC_1_1FIELDS(opChanReportTLV, 0, rid, fmt_binary, tx_power, fmt_unsigned,
                            &SNAME(opclassChanPairStruct), NULL),
    [TLV_TYPE_CLIENT_INFO] =
        TLV_DESC_2FIELDS(clientInfoTLV, 0, bssid, fmt_binary, client, fmt_binary, NULL),
    [TLV_TYPE_CLIENT_CAPABILITY_REPORT] =
        TLV_DESC_2FIELDS(clientCapabilityReportTLV, 0, result, fmt_unsigned, assc_req, fmt_l0vv, NULL),
    [TLV_TYPE_CLIENT_ASSOCIATION_EVENT] =
        TLV_DESC_3FIELDS(clientAssocEvtTLV, 0, client, fmt_binary, bssid, fmt_binary, evt, fmt_unsigned, NULL),
    [TLV_TYPE_AP_METRIC_QUERY] =
        TLV_DESC_0FIELD(TLV, 0, &SNAME(macStruct), NULL),
    [TLV_TYPE_AP_METRICS] =
        TLV_DESC_8FIELDS(apMetricsTLV, 0, bssid, fmt_binary, chan_util, fmt_unsigned, clients, fmt_unsigned,
                            includes, fmt_unsigned, espi_be, fmt_binary, espi_bk, fmt_binary, espi_vo, fmt_binary,
                            espi_vi, fmt_binary, NULL),
    [TLV_TYPE_STA_MAC_ADDRESS] =
        TLV_DESC_1FIELD(macAddressTLV, 0, mac, fmt_binary, NULL),
    [TLV_TYPE_ASSOCIATED_STA_LINK_METRICS] =
        TLV_DESC_1FIELD(macAddressTLV, 0, mac, fmt_binary, &SNAME(assocLinkMetricStruct), NULL),
    [TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_QUERY] =
        TLV_DESC_1FIELD(unassocStaLinkMetricsQryTLV, 0, opclass, fmt_unsigned,
                            &SNAME(unassocMetricQryStruct), NULL),
    [TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_RESPONSE] =
        TLV_DESC_1FIELD(unassocStaLinkMetricsRspTLV, 0, opclass, fmt_unsigned,
                            &SNAME(unassocMetricRspStruct), NULL),
    [TLV_TYPE_BEACON_METRICS_QUERY] =
        TLV_DESC_6_1FIELDS(bcnMetricQueryTLV, 0, sta, fmt_binary, opclass, fmt_unsigned, channel, fmt_unsigned,
                            bssid, fmt_binary, detail, fmt_unsigned, ssid, fmt_l1v, eids, fmt_l1vv,
                            &SNAME(chanRptStruct), NULL),
    [TLV_TYPE_BEACON_METRICS_RESPONSE] =
        TLV_DESC_2FIELDS(bcnMetricRspTLV, 0, sta, fmt_binary, reserved, fmt_unsigned,
                            &SNAME(measuredElemStruct), NULL),
    [TLV_TYPE_STEERING_REQUEST] =
        TLV_DESC_4FIELDS(steerReqTLV, 0, bssid, fmt_binary, mode, fmt_unsigned, window, fmt_unsigned,
                            disassoc, fmt_unsigned, &SNAME(macStruct), &SNAME(targetBssStruct), NULL),
    [TLV_TYPE_STEERING_BTM_REPORT] =
        TLV_DESC_4FIELDS(steerBtmReportTLV, 0, bssid, fmt_binary, sta, fmt_binary, status, fmt_unsigned,
                            target, fmt_l0vv, NULL),
    [TLV_TYPE_CLIENT_ASSOCIATION_CONTROL_REQUEST] =
        TLV_DESC_3FIELDS(clientAssocCtrlReqTLV, 0, bssid, fmt_binary, ctrl, fmt_unsigned, valid_period, fmt_unsigned,
                            &SNAME(macStruct), NULL),
    [TLV_TYPE_BACKHAUL_STEERING_REQUEST] =
        TLV_DESC_4FIELDS(bkhSteerReqTLV, 0, sta, fmt_binary, target, fmt_binary, opclass, fmt_unsigned,
                            channel, fmt_unsigned, NULL),
    [TLV_TYPE_BACKHAUL_STEERING_RESPONSE] =
        TLV_DESC_3FIELDS(bkhSteerRspTLV, 0, sta, fmt_binary, target, fmt_binary, result, fmt_unsigned, NULL),
    [TLV_TYPE_HIGHER_LAYER_DATA] =
        TLV_DESC_2FIELDS(higherLayerDataTLV, 0, protocol, fmt_unsigned, payload, fmt_l0vv, NULL),
    [TLV_TYPE_ASSOCIATED_STA_TRAFFIC_STATS] =
        TLV_DESC_8FIELDS(assocTrafficStatsTLV, 0, sta, fmt_binary,
                            stats.bytes_tx, fmt_unsigned, stats.bytes_rx, fmt_unsigned,
                            stats.packets_tx, fmt_unsigned, stats.packets_rx, fmt_unsigned,
                            stats.packets_err_tx, fmt_unsigned, stats.packets_err_tx, fmt_unsigned,
                            stats.retransmission, fmt_unsigned, NULL),
    [TLV_TYPE_ERROR_CODE] =
        TLV_DESC_2FIELDS(errorCodeTLV, 0, code, fmt_unsigned, sta, fmt_binary, NULL),
    /*Multi-AP TLV types as description in tables 47 to 76 of "Multi-AP_Specification_v2.0"*/
    [TLV_TYPE_CHANNEL_SCAN_REPORTING_POLOCY] =
        TLV_DESC_1FIELD(u8TLV, 0, v1, fmt_unsigned, NULL),
    [TLV_TYPE_CHANNEL_SCAN_CAPABILITIES] =
        TLV_DESC_0FIELD(TLV, 0, &SNAME(scanCapaRadioStruct), NULL),
    [TLV_TYPE_CHANNEL_SCAN_REQUEST] =
        TLV_DESC_1FIELD(scanReqTLV, 0, fresh_scan, fmt_unsigned, &SNAME(scanReqRadioStruct), NULL),
    [TLV_TYPE_CHANNEL_SCAN_RESULT] =
        TLV_DESC_4FIELDS(scanResultTLV, 0, ruid, fmt_binary, opclass, fmt_unsigned, channel, fmt_unsigned, scan_status, fmt_unsigned,
                            &SNAME(scanResultStruct), NULL),
    [TLV_TYPE_TIMESTAMP] =
        TLV_DESC_1FIELD(timeStampTLV, 0, timeStamp, fmt_l1v, NULL),
    [TLV_TYPE_CAC_REQUEST] =
        TLV_DESC_0FIELD(TLV, 0, &SNAME(cacReqRadioStruct), NULL),
    [TLV_TYPE_CAC_TERMINATION] =
        TLV_DESC_0FIELD(TLV, 0, &SNAME(cacTermRadioStruct), NULL),
    [TLV_TYPE_CAC_COMPLETION_REPORT] =
        TLV_DESC_0FIELD(TLV, 0, &SNAME(cacReptRadioStruct), NULL),
    [TLV_TYPE_CAC_STATUS_REPORT] =
        TLV_DESC_0FIELD(TLV, 0, &SNAME(cacStatChanStruct), &SNAME(cacStatChanStruct), &SNAME(cacStatCacChanStruct), NULL),
    [TLV_TYPE_CAC_CAPABILITIES] =
        TLV_DESC_1FIELD(cacCapaTLV, 0, cn_code, fmt_unsigned, &SNAME(cacCapaRadioStruct), NULL),
    [TLV_TYPE_DEFAULT_8021Q_SETTINGS] =
        TLV_DESC_2FIELDS(default80211QSetsTLV, 0, vlanid, fmt_unsigned, def_pcp, fmt_unsigned, NULL),
    [TLV_TYPE_TRAFFIC_SEPARATION_POLICY] =
        TLV_DESC_0FIELD(TLV, 0, &SNAME(trafficPolicyStruct), NULL),
    [TLV_TYPE_PROFILE2_ERROR_CODE] =
        TLV_DESC_4FIELDS(p2ErrorCodeTLV, 0, code, fmt_unsigned, bssid, fmt_binary, rule_id, fmt_unsigned, qmid, fmt_unsigned, NULL),
    [TLV_TYPE_AP_RADIO_ADVANCED_CAPABILITIES] =
        TLV_DESC_2FIELDS(radioAdvCapaTLV, 0, ruid, fmt_binary, flag, fmt_unsigned, NULL),
    [TLV_TYPE_ASSOCIATION_STATUS_NOTIFICATION] =
        TLV_DESC_0FIELD(TLV, 0, &SNAME(assocStatNotifyStruct), NULL),
    [TLV_TYPE_SOURCE_INFO] =
        TLV_DESC_1FIELD(macAddressTLV, 0, mac, fmt_binary, NULL),
    [TLV_TYPE_TUNNELED_MESSAGE_TYPE] =
        TLV_DESC_1FIELD(u8TLV, 0, v1, fmt_unsigned, NULL),
    [TLV_TYPE_TUNNELED] =
        TLV_DESC_1FIELD(tunnelTLV, 0, value, fmt_l0vv, NULL),
    [TLV_TYPE_PROFILE2_STEERING_REQUEST] =
        TLV_DESC_4FIELDS(steerReqTLV, 0, bssid, fmt_binary, mode, fmt_unsigned, window, fmt_unsigned,
                disassoc, fmt_unsigned, &SNAME(macStruct), &SNAME(p2targetBssStruct), NULL),
    [TLV_TYPE_UNSUCCESSFUL_ASSOCIATION_POLICY] =
        TLV_DESC_2FIELDS(unassocPolicyTLV, 0, report_flag, fmt_unsigned, max_rate, fmt_unsigned, NULL),
    [TLV_TYPE_METRIC_COLLECTION_INTERVAL] =
        TLV_DESC_1FIELD(metricCollectIntervalTLV, 0, interval, fmt_unsigned, NULL),

    [TLV_TYPE_BACKHAUL_BSS_CONFIGURATION] =
        TLV_DESC_2FIELDS(backhaulBSSConfigTLV, 0, bssid, fmt_binary, config, fmt_unsigned, NULL),
    [TLV_TYPE_BACKHAUL_STA_RADIO_CAPABILITIES] =
        TLV_DESC_2FIELDS(bSTARadioCapaTLV, 0, rid, fmt_binary, capa, fmt_unsigned, &SNAME(bStaMacStruct), NULL),
    [TLV_TYPE_STATUS_CODE] =
        TLV_DESC_1FIELD(codeTLV, 0, code, fmt_unsigned, NULL),
    [TLV_TYPE_REASON_CODE] =
        TLV_DESC_1FIELD(codeTLV, 0, code, fmt_unsigned, NULL),
    [TLV_TYPE_RADIO_METRICS] =
        TLV_DESC_5FIELDS(radioMetricsTLV, 0, rid, fmt_binary, metrics.noise, fmt_unsigned, metrics.transmit, fmt_unsigned,
                            metrics.receive_self, fmt_binary, metrics.receive_other, fmt_unsigned, NULL),
    [TLV_TYPE_AP_EXTENDED_METRICS] =
        TLV_DESC_7FIELDS(apExtMetricsTLV, 0, bssid, fmt_binary, metrics.uc_tx, fmt_unsigned, metrics.uc_rx, fmt_unsigned,
                            metrics.mc_tx, fmt_unsigned, metrics.mc_rx, fmt_unsigned,
                            metrics.bc_tx, fmt_unsigned, metrics.bc_rx, fmt_unsigned, NULL),
    [TLV_TYPE_ASSOCIATED_STA_EXTENDED_LINK_METRICS] =
        TLV_DESC_1FIELD(macAddressTLV, 0, mac, fmt_unsigned, &SNAME(associatedSTAExtLinkMetricsStruct), NULL),
    [TLV_TYPE_VENDOR_SPECIFIC] =
        TLV_DESC_1FIELD(vendorTLV, 0, oui, fmt_binary, NULL),

    [TLV_TYPE_1905_LAYER_SECURITY_CAPABILITY] =
        TLV_DESC_3FIELDS(securityCapabilityTLV, 0, onboarding_protocol, fmt_unsigned, mic_alg, fmt_unsigned, encrypt_alg, fmt_unsigned, NULL),
    [TLV_TYPE_MIC] =
        TLV_DESC_4FIELDS(micTLV, 0, mic_misc, fmt_unsigned, integrity_tx_counter, fmt_binary, src_mac, fmt_binary, mic, fmt_l2vv, NULL),
    [TLV_TYPE_ENCRYPTED_PAYLOAD] =
        TLV_DESC_4FIELDS(encryptedPayloadTLV, 0, encryt_tx_counter, fmt_binary, src_mac, fmt_binary, dst_mac, fmt_binary, aes_siv, fmt_l2vv, NULL),
    [TLV_TYPE_SERVICE_PRIORITIZATION_RULE] =
        TLV_DESC_5FIELDS(servicePriorityRuleTLV, 0, rule_id, fmt_unsigned, flag, fmt_unsigned, precedence, fmt_unsigned, output, fmt_unsigned, match, fmt_unsigned, NULL),
    [TLV_TYPE_DSCP_MAPPING_TABLE] =
        TLV_DESC_1FIELD(dscpMappingTableTLV, 0, dscp_pcp_mapping, fmt_binary, NULL),
    [TLV_TYPE_AP_WIFI6_CAPABILITIES] =
        TLV_DESC_1FIELD(macAddressTLV, 0, mac, fmt_binary, &SNAME(wifi6RoleStruct), NULL),
    [TLV_TYPE_ASSOCIATED_WIFI6_STA_STATUS_REPORT] =
        TLV_DESC_1FIELD(macAddressTLV, 0, mac, fmt_binary, &SNAME(tidQueueSizeStruct), NULL),
    [TLV_TYPE_BSS_CONFIGURATION_REPORT] =
        TLV_DESC_0FIELD(TLV, 0, &SNAME(bssConfigRadioStruct), NULL),
    [TLV_TYPE_BSSID] =
        TLV_DESC_1FIELD(macAddressTLV, 0, mac, fmt_binary, NULL),
    [TLV_TYPE_DEVICE_INVENTORY] =
        TLV_DESC_3FIELDS(deviceInventoryTLV, 0, serial_no, fmt_l1vv, software_ver, fmt_l1vv, exec_env, fmt_l1vv, &SNAME(deviceInventoryRadioStruct), NULL),
    [TLV_TYPE_AGENT_LIST] =
        TLV_DESC_0FIELD(TLV, 0, &SNAME(agentListAgentStruct), NULL),
    [TLV_TYPE_AKM_SUITE_CAPABILITIES] =
        TLV_DESC_0FIELD(TLV, 0, &SNAME(akmSuiteCapaStruct), &SNAME(akmSuiteCapaStruct), NULL),
    [TLV_TYPE_1905_ENCAP_DPP] =
        TLV_DESC_4FIELDS(encapDppTLV, 0, flag, fmt_unsigned, dst_sta_mac, fmt_binary, frame_type, fmt_unsigned, frame, fmt_l2vv, NULL),
    [TLV_TYPE_1905_ENCAP_EAPOL] =
        TLV_DESC_1FIELD(l0vvTLV, 0, data, fmt_l0vv, NULL),
    [TLV_TYPE_DPP_BOOTSTRAPPING_URI_NOTIFICATION] =
        TLV_DESC_4FIELDS(dppBootstrapUriNotificationTLV, 0, ruid, fmt_binary, bssid, fmt_binary, bsta_mac, fmt_binary, dpp_uri, fmt_l0vv, NULL),
    [TLV_TYPE_DPP_CCE_INDICATION] =
        TLV_DESC_1FIELD(u8TLV, 0, v1, fmt_unsigned, NULL),
    [TLV_TYPE_DPP_CHIRP_VALUE] =
        TLV_DESC_3FIELDS(dppChirpValueTLV, 0, flag, fmt_unsigned, dst_sta_mac, fmt_binary, hash, fmt_l1vv, NULL),
    [TLV_TYPE_BSS_CONFIGURATION_REQUEST] =
        TLV_DESC_1FIELD(l0vvTLV, 0, data, fmt_l0vv, NULL),
    [TLV_TYPE_BSS_CONFIGURATION_RESPONSE] =
        TLV_DESC_1FIELD(l0vvTLV, 0, data, fmt_l0vv, NULL),
    [TLV_TYPE_DPP_MESSAGE] =
        TLV_DESC_1FIELD(l0vvTLV, 0, data, fmt_l0vv, NULL),
    [TLV_TYPE_ANTICIPATED_CHANNEL_PREFERENCE] =
        TLV_DESC_0FIELD(TLV, 0, &SNAME(anticipatedChannelPrefOpClassChanStruct), NULL),
    [TLV_TYPE_ANTICIPATED_CHANNEL_USAGE] =
        TLV_DESC_3FIELDS(anticiaptedChannelUsageTLV, 0, opclass, fmt_unsigned, channel, fmt_unsigned, reference_bssid, fmt_binary, &SNAME(anticiaptedChannelUsageEntryStruct), NULL),

    [TLV_TYPE_SPATIAL_REUSE_REQUEST] =
        TLV_DESC_8FIELDS(spacialReuseRequestTLV, 0, ruid, fmt_binary, bss_color, fmt_unsigned, flag, fmt_unsigned, non_srg_obsspd_max_offset, fmt_unsigned, srg_obsspd_min_offset, fmt_unsigned, srg_obsspd_max_offset, fmt_unsigned, srg_bss_color_bitmap, fmt_binary, srg_partial_bssid_bitmap, fmt_binary, NULL),
    [TLV_TYPE_SPATIAL_REUSE_REPORT] =
        TLV_DESC_9FIELDS(spacialReuseReportTLV, 0, ruid, fmt_binary, bss_color, fmt_unsigned, flag, fmt_unsigned, non_srg_obsspd_max_offset, fmt_unsigned, srg_obsspd_min_offset, fmt_unsigned, srg_obsspd_max_offset, fmt_unsigned, srg_bss_color_bitmap, fmt_binary, srg_partial_bssid_bitmap, fmt_binary, neighbor_bss_color_in_use_bitmap, fmt_binary, NULL),
    [TLV_TYPE_SPATIAL_REUSE_CONFIG_RESPONSE] =
        TLV_DESC_2FIELDS(spatialReuseConfigResponseTLV, 0, ruid, fmt_binary, response_code, fmt_unsigned, NULL),
    [TLV_TYPE_QOS_MANAGEMENT_POLICY] =
        TLV_DESC_0FIELD(TLV, 0, &SNAME(macStruct), &SNAME(macStruct), NULL),
    [TLV_TYPE_QOS_MANAGEMENT_DESCRIPTOR] =
        TLV_DESC_4FIELDS(qosManagementDescTLV, 0, qmid, fmt_binary, bssid, fmt_binary, client_mac, fmt_binary, desc_element, fmt_l0vv, NULL),
    [TLV_TYPE_CONTROLLER_CAPABILITY] =
        TLV_DESC_1FIELD(u8TLV, 0, v1, fmt_unsigned, NULL),
    [TLV_TYPE_VBSS] =
        TLV_DESC_0FIELD(superTLV, 0, NULL)
};

static struct TLVDesc _vbss_tlv_descs[TLV_SUB_TYPE_VBSS_MAX] = {
    [TLV_SUB_TYPE_VBSS_AP_RADIO_CAPABILITIES] =
        TLV_DESC_5FIELDS(vbssCapabilitiesTLV, 0, ruid, fmt_binary, max_vbss, fmt_unsigned,
                    flag, fmt_unsigned, fixed_bits_mask, fmt_binary, fixed_bits_value, fmt_binary, NULL),
    [TLV_SUB_TYPE_VBSS_CREATION] =
        TLV_DESC_11FIELDS(vbssCreationTLV, 0, ruid, fmt_binary, bssid, fmt_binary, ssid, fmt_l2vv,
                    wpa_password, fmt_l2vv, dpp_connector, fmt_l2vv, client_mac, fmt_binary, client_assoc,
                    fmt_unsigned, ptk, fmt_l2vv, tx_packet_number, fmt_unsigned, gtk, fmt_l2vv, group_tx_packet_number,
                    fmt_unsigned, NULL),
    [TLV_SUB_TYPE_VBSS_DESTRUCTION] =
        TLV_DESC_3FIELDS(vbssDestructionTLV, 0, ruid, fmt_binary, bssid, fmt_binary, disassociate_client, fmt_unsigned, NULL),
    [TLV_SUB_TYPE_VBSS_EVENT] =
        TLV_DESC_3FIELDS(vbssEventTLV, 0, ruid, fmt_binary, success, fmt_unsigned, bssid, fmt_binary, NULL),
    [TLV_SUB_TYPE_CLIENT_SECURITY_CONTEXT] =
        TLV_DESC_5FIELDS(clientSecurityContextTLV, 0, flag, fmt_unsigned, ptk, fmt_l2vv, tx_packet_number, fmt_unsigned,
                    gtk, fmt_l2vv, group_tx_packet_number, fmt_unsigned, NULL),
    [TLV_SUB_TYPE_TRIGGER_CSA] =
        TLV_DESC_3FIELDS(triggerCSATLV, 0, ruid, fmt_binary, csa_channel, fmt_unsigned, opclass, fmt_unsigned, NULL),
    [TLV_SUB_TYPE_VBSS_CONFIGURATION_REPORT] =
        TLV_DESC_0FIELD(TLV, 0, &SNAME(vbssConfigurationReportRadioStruct), NULL),

};

void register_non_std_tlv_ops(uint8_t type, void *_decode, void *_encode, void *_length, void *_free)
{
    struct TLVOperator *operation = (struct TLVOperator *)calloc(1, sizeof(struct TLVOperator));
    struct TLVDesc *desc = getTLVDesc(type);

    if (!operation || !desc)
        goto bail;
    if (!_encode && !_decode && !_length && !_free)
        goto bail;

    if (_encode)
        operation->encode = _encode;
    if (_decode)
        operation->decode = _decode;
    if (_length)
        operation->length = _length;
    if (_free)
        operation->free = _free;
    desc->ops = operation;
    desc->tag = type;
    return;
bail:
    if(operation)
        free(operation);
    return;
}

void unregister_non_std_tlv_ops(uint8_t type)
{
    struct TLVDesc *desc = getTLVDesc(type);

    if (!desc)
        return;
    if (desc->ops)
        free(desc->ops);
}

void init_non_std_tlv_ops(void)
{
    register_non_std_tlv_ops(TLV_TYPE_LINK_METRIC_QUERY, link_metric_query_decode, link_metric_query_encode, link_metric_query_length, NULL);
    register_non_std_tlv_ops(TLV_TYPE_AP_METRICS, ap_metrics_decode, ap_metrics_encode, ap_metrics_length, NULL);
    register_non_std_tlv_ops(TLV_TYPE_PROFILE2_ERROR_CODE, p2Error_code_decode, p2Error_code_encode, p2Error_code_length, NULL);
    register_non_std_tlv_ops(TLV_TYPE_CHANNEL_SCAN_RESULT, channel_scan_result_decode, NULL, channel_scan_result_length, NULL);
    register_non_std_tlv_ops(TLV_TYPE_VENDOR_SPECIFIC, super_tlv_decode, super_tlv_encode, super_tlv_length, super_tlv_free);
    register_non_std_tlv_ops(TLV_TYPE_VBSS, super_tlv_decode, super_tlv_encode, super_tlv_length, super_tlv_free);
}

void deinit_non_std_tlv_ops(void)
{
    unregister_non_std_tlv_ops(TLV_TYPE_LINK_METRIC_QUERY);
    unregister_non_std_tlv_ops(TLV_TYPE_AP_METRICS);
    unregister_non_std_tlv_ops(TLV_TYPE_STEERING_BTM_REPORT);
    unregister_non_std_tlv_ops(TLV_TYPE_PROFILE2_ERROR_CODE);
    unregister_non_std_tlv_ops(TLV_TYPE_CHANNEL_SCAN_RESULT);
    unregister_non_std_tlv_ops(TLV_TYPE_VENDOR_SPECIFIC);
    unregister_non_std_tlv_ops(TLV_TYPE_VBSS);
}

static struct TLVStruct *super_tlv_decode(struct TLVDesc *desc, dlist_head *parent, uint8_t **pp, uint16_t *plen)
{
    struct superTLV *self = (struct superTLV *)TLVStructNew(desc, parent, 0);
    uint16_t sub_type;
    struct TLVStruct *s;
    struct TLVDesc *sub_desc;
    int i;

    for (i=0; (i<ARRAY_SIZE(desc->fields)) && (desc->fields[i].name); i++) {
        if (decodeTLVField(&desc->fields[i], &self->tlv.s, pp, plen)) {
            DEBUG_WARNING("decode field %s failed\n", desc->fields[i].name);
            goto fail;
        }
    }

    _E2B(pp, &sub_type);

    *plen -= 2;

    if ((!(sub_desc = getSubTLVDescExt((struct TLV *)self, sub_type))) ||
        (!(s = decodeTLVOne(sub_desc, NULL, pp, plen))))
        goto fail;

    self->sub_tlv = container_of(s, struct TLV, s);
    self->sub_tlv->tlv_subtype = sub_type;

    return &self->tlv.s;
fail:
    tlist_delete_item(&self->tlv.s.t);
    return NULL;
}

static int super_tlv_encode(struct TLVStruct *s, uint8_t **pp, uint16_t *plen)
{
    struct superTLV *self = container_of(s, struct superTLV, tlv.s);
    struct TLV *sub_tlv = self->sub_tlv;
    int i;

    for (i=0; i<ARRAY_SIZE(s->desc->fields) && (s->desc->fields[i].name); i++) {
        encodeTLVField(&s->desc->fields[i], s, pp, plen);
    }

    _I2B(&sub_tlv->tlv_subtype, pp);
    *plen -= 2;

    encodeTLVOne(&sub_tlv->s, pp, plen);

    return 1;
}

static uint16_t super_tlv_length(struct TLVStruct *s)
{
    struct superTLV *self = container_of(s, struct superTLV, tlv.s);
    uint16_t len = 2; //2 otctets subtype
    struct TLV *sub_tlv = self->sub_tlv;
    int i;

    for (i=0; i<ARRAY_SIZE(s->desc->fields) && (s->desc->fields[i].name); i++) {
        len += getLenField(s, &(s->desc->fields[i]));
    }

    len += getLenTLVOne(&sub_tlv->s);
    return len;
}

static void super_tlv_free(struct TLVStruct *s)
{
    struct superTLV *self = container_of(s, struct superTLV, tlv.s);

    if (self->sub_tlv)
        TLVFree(self->sub_tlv);
}

static struct TLVStruct *link_metric_query_decode(struct TLVDesc *desc, dlist_head *parent, uint8_t **pp, uint16_t *len)
{
    struct linkMetricQueryTLV *self = (struct linkMetricQueryTLV *)TLVNew(parent, TLV_TYPE_LINK_METRIC_QUERY, 0);
    int len_variable = 0;

    len_variable += _E1B(pp, &self->neighbor);
    if (self->neighbor == LINK_METRIC_QUERY_TLV_SPECIFIC_NEIGHBOR)
        len_variable += _EnB(pp, (void *)self->specific_neighbor, MACLEN);
    len_variable += _E1B(pp, &self->link_metrics_type);

    *len -= len_variable;
    return &self->tlv.s;
}

static int link_metric_query_encode(struct TLVStruct *s, uint8_t **pp, uint16_t *len)
{
    struct linkMetricQueryTLV *self = container_of(s, struct linkMetricQueryTLV, tlv.s);
    uint8_t len_variable = 0;

    _I1B(&self->neighbor, pp);
    len_variable += 1;
    if (self->neighbor == LINK_METRIC_QUERY_TLV_SPECIFIC_NEIGHBOR) {
        _InB((void *)self->specific_neighbor, pp, MACLEN);
        len_variable += MACLEN;
    }
    _I1B(&self->link_metrics_type, pp);
    len_variable += 1;

    *len -= len_variable;
    return 1;
}

static uint16_t link_metric_query_length(struct TLVStruct *s)
{
    uint16_t size;
    struct linkMetricQueryTLV *self = container_of(s, struct linkMetricQueryTLV, tlv.s);

    size = 1 + 1; /* neighbor + link_metrics_type */
    if (self->neighbor == LINK_METRIC_QUERY_TLV_SPECIFIC_NEIGHBOR)
        size += MACLEN;

    return size;
}

static struct TLVStruct *ap_metrics_decode(struct TLVDesc *desc, dlist_head *parent, uint8_t **pp, uint16_t *len)
{
    struct apMetricsTLV *self = (struct apMetricsTLV *)TLVNew(parent, TLV_TYPE_AP_METRICS, 0);
    int i, len_variable = 0;

    for (i = 0; i < 5; i++)
    {
        if (-1 == decodeTLVField(&desc->fields[i], &self->tlv.s, pp, len))
            goto err;
    }
    if (self->includes & ESPI_BK_INCLUDED)
        len_variable += _EnB(pp, self->espi_bk, ESPI_FIELD_LEN);
    if (self->includes & ESPI_VO_INCLUDED)
        len_variable += _EnB(pp, self->espi_vo, ESPI_FIELD_LEN);
    if (self->includes & ESPI_VI_INCLUDED)
        len_variable += _EnB(pp, self->espi_vi, ESPI_FIELD_LEN);

    *len -= len_variable;
    return &self->tlv.s;

err:
    tlist_delete_item(&self->tlv.s.t);
    return NULL;
}

static int ap_metrics_encode(struct TLVStruct *s, uint8_t **pp, uint16_t *len)
{
    int i;
    struct apMetricsTLV *self = container_of(s, struct apMetricsTLV, tlv.s);
    uint8_t len_variable = 0;

    /* Use the normal forge functions to forge the first 5 fields. */
    for (i = 0; i < 5; i++)
    {
        if (encodeTLVField(&s->desc->fields[i], s, pp, len))
            return 0;
    }
    if (self->includes & ESPI_BK_INCLUDED) {
        _InB((void *)self->espi_bk, pp, ESPI_FIELD_LEN);
        len_variable += ESPI_FIELD_LEN;
    }
    if (self->includes & ESPI_VO_INCLUDED) {
        _InB((void *)self->espi_vo, pp, ESPI_FIELD_LEN);
        len_variable += ESPI_FIELD_LEN;
    }
    if (self->includes & ESPI_VI_INCLUDED) {
        _InB((void *)self->espi_vi, pp, ESPI_FIELD_LEN);
        len_variable += ESPI_FIELD_LEN;
    }

    *len -= len_variable;
    return 1;
}

static uint16_t ap_metrics_length(struct TLVStruct *s)
{
    uint16_t size;
    struct apMetricsTLV *self = container_of(s, struct apMetricsTLV, tlv.s);

    size = 6 + 1 + 2 + 1 + ESPI_FIELD_LEN;

    if (self->includes & ESPI_BK_INCLUDED)
        size += ESPI_FIELD_LEN;
    if (self->includes & ESPI_VO_INCLUDED)
        size += ESPI_FIELD_LEN;
    if (self->includes & ESPI_VI_INCLUDED)
        size += ESPI_FIELD_LEN;

    return size;
}

static struct TLVStruct *p2Error_code_decode(struct TLVDesc *desc, dlist_head *parent, uint8_t **pp, uint16_t *len)
{
    struct p2ErrorCodeTLV *self = (struct p2ErrorCodeTLV *)TLVNew(parent, TLV_TYPE_PROFILE2_ERROR_CODE, 0);

    self->tlv.s.desc = desc;
    if (-1 == decodeTLVField(&desc->fields[0], &self->tlv.s, pp, len))
        goto err;
    if(self->code == REASON_COMBINED_BSS_TRAFFIC_SEP_UNSUPPORT ||
       self->code == REASON_MIX_BACKHAUL_UNSUPPORT)
       _EnB(pp, (void *)self->bssid, MACLEN);
    if(self->code == REASON_SERVICE_RULE_NOT_FOUND || self->code == REASON_SERVICE_RULE_NUM_EXCEED)
       _E4B(pp, &self->rule_id);
    if(self->code == REASON_QOS_MGMT_POLICY_UNCONFIG)
       _E2B(pp, &self->qmid);
    return &self->tlv.s;
err:
    tlist_delete_item(&self->tlv.s.t);
    return NULL;
}

static int p2Error_code_encode(struct TLVStruct *s, uint8_t **pp, uint16_t *len)
{
    struct p2ErrorCodeTLV *self = container_of(s, struct p2ErrorCodeTLV, tlv.s);

    if (encodeTLVField(&s->desc->fields[0], s, pp, len))
        return 0;
    if(self->code == REASON_COMBINED_BSS_TRAFFIC_SEP_UNSUPPORT ||
       self->code == REASON_MIX_BACKHAUL_UNSUPPORT)
        _InB((void *)self->bssid, pp, MACLEN);
    if(self->code == REASON_SERVICE_RULE_NOT_FOUND || self->code == REASON_SERVICE_RULE_NUM_EXCEED)
        _I4B(&self->rule_id, pp);
    if(self->code == REASON_QOS_MGMT_POLICY_UNCONFIG)
        _I2B(&self->qmid, pp);
    return 1;
}

static uint16_t p2Error_code_length(struct TLVStruct *s)
{
    uint16_t size = 1; /* code */

    struct p2ErrorCodeTLV *self = container_of(s, struct p2ErrorCodeTLV, tlv.s);

    if(self->code == REASON_COMBINED_BSS_TRAFFIC_SEP_UNSUPPORT ||
       self->code == REASON_MIX_BACKHAUL_UNSUPPORT)
        size += 6;
    if(self->code == REASON_SERVICE_RULE_NOT_FOUND || self->code == REASON_SERVICE_RULE_NUM_EXCEED)
        size += 4;
    if(self->code == REASON_QOS_MGMT_POLICY_UNCONFIG)
        size += 2;
    return size;
}

static struct TLVStruct *channel_scan_result_decode(struct TLVDesc *desc, dlist_head *parent, uint8_t **pp, uint16_t *len)
{
    struct scanResultTLV *self = (struct scanResultTLV *)TLVNew(parent, TLV_TYPE_CHANNEL_SCAN_RESULT, 0);
    struct scanResultStruct *result = NULL;
    struct scanNeighborStruct *neighbor = NULL;
    struct bssLoadEleStruct *bssfield = NULL;
    uint16_t neighbor_num;
    int i = 0;

    self->tlv.s.desc = desc;
    for (i = 0; i < 4; i++) {
        if (-1 == decodeTLVField(&desc->fields[i], &self->tlv.s, pp, len))
            goto err;
    }
    if (!self->scan_status) {
        result = (struct scanResultStruct *)TLVStructDeclare(scanResultStruct, &self->tlv.s.t.childs[0]);
        _E1B(pp, &result->timestamp.len);
        _EnB(pp, (void *)result->timestamp.data, result->timestamp.len);
        _E1B(pp, &result->utilization);
        _E1B(pp, &result->noise);
        _E2B(pp, &neighbor_num);
        if (neighbor_num) {
            for (i = 0; i < neighbor_num; i++) {
                neighbor = (struct scanNeighborStruct *)TLVStructDeclare(scanNeighborStruct, &result->s.t.childs[0]);
                _EnB(pp, (void *)neighbor->bssid, MACLEN);
                _E1B(pp, &neighbor->ssid.len);
                _EnB(pp, (void *)neighbor->ssid.ssid, neighbor->ssid.len);
                _E1B(pp, &neighbor->signal_strength);
                _E1B(pp, &neighbor->chanbw.len);
                _EnB(pp, (void *)neighbor->chanbw.data, neighbor->chanbw.len);
                _E1B(pp, &neighbor->bss_field);
                if (neighbor->bss_field & BSS_LOAD_ELE_PRESENT) {
                    bssfield = (struct bssLoadEleStruct *)TLVStructDeclare(bssLoadEleStruct, &neighbor->s.t.childs[0]);
                    _E1B(pp, &bssfield->chann_utilize);
                    _E2B(pp, &bssfield->sta_cnt);
                }
            }
        }
        _E4B(pp, &result->aggre_scan_dur);
        _E1B(pp, &result->scan_type);
    }
    return &self->tlv.s;
err:
    tlist_delete_item(&self->tlv.s.t);
    return NULL;
}

static uint16_t channel_scan_result_length(struct TLVStruct *s)
{
    struct scanResultStruct *result = NULL;
    struct scanNeighborStruct *neighbor = NULL;
    uint16_t size = 6 + 1 + 1 + 1; /* ruid + opclass + channel + scan_status */
    uint16_t nsize = 0;
    struct scanResultTLV *self = container_of(s, struct scanResultTLV, tlv.s);

    if (!self->scan_status) {
        result = container_of(dlist_get_first(&self->tlv.s.t.childs[0]), struct scanResultStruct, s.t.l);
        size += 1; /* timestamp length */
        size += result->timestamp.len; /* timestamp */
        size += (1 + 1 + 2 + 4 + 1); /* utilization + noise + neighbor_num + aggre_scan_dur + scan_type */
        dlist_for_each(neighbor, result->s.t.childs[0], s.t.l) {
            nsize = 6 + 1; /*bssid + ssid length*/
            if (neighbor->ssid.len)
                nsize += neighbor->ssid.len;
            nsize += 2; /*signal_strength + chanbw_len*/
            if (neighbor->chanbw.len)
                nsize += neighbor->chanbw.len;
            nsize += 1; /*bss_field */
            if (neighbor->bss_field & BSS_LOAD_ELE_PRESENT) {
                nsize += 3; /* chann_utilize + sta_cnt */
            }
            size += nsize;
        }
        size += 5; /* aggre_scan_dur + scan_type */
    }
    return size;
}

struct serviceStruct *serviceAddService(struct TLV *tlv, uint8_t service)
{
    struct serviceStruct *s =
         (struct serviceStruct *)TLVStructDeclare(serviceStruct, &tlv->s.t.childs[0]);
    s->service = service;
    return s;
}

#define FILL_80211_SPECIFIC_INFO(_info, _bssid, _role, _bw, _channel1, _channel2) \
do {\
    MACCPY((_info), (_bssid));\
    (_info)[6] = (_role);\
    (_info)[7] = (_bw);\
    (_info)[8] = (_channel1);\
    (_info)[9] = (_channel2);\
} while (0)

struct interfaceStruct *deviceInfoAddInterface(struct TLV *tlv, struct interface *i)
{
    struct interfaceStruct *s =
        (struct interfaceStruct *)TLVStructDeclare(interfaceStruct, &tlv->s.t.childs[0]);

    MACCPY(s->mac, i->mac);
    if (i->type == interface_type_wifi) {
        struct wifi_interface *wif = (struct wifi_interface *)i;
        struct radio *r;
        struct band_capability *capa;

        if ((r = wif->radio)) {
            capa = &r->bands_capa[r->current_band_idx];
            if (capa->he_capa_valid) {
                s->media_type = MEDIA_TYPE_IEEE_802_11AX;
            } else if (capa->vht_capa_valid) {
                s->media_type = MEDIA_TYPE_IEEE_802_11AC_5_GHZ;
            } else if (capa->ht_capa_valid) {
                if (r->current_band_idx==band_2g_idx)
                    s->media_type = MEDIA_TYPE_IEEE_802_11N_2_4_GHZ;
                else
                    s->media_type = MEDIA_TYPE_IEEE_802_11N_5_GHZ;
            } else {
                if (r->current_band_idx==band_2g_idx)
                    s->media_type = MEDIA_TYPE_IEEE_802_11G_2_4_GHZ;
                else
                    s->media_type = MEDIA_TYPE_IEEE_802_11AC_5_GHZ;
            }
            if (s->media_type != MEDIA_TYPE_IEEE_802_11AX) {
                s->media_specific_info[0] = 10;
                //FIXME need check bssid for sta
                FILL_80211_SPECIFIC_INFO(s->media_specific_info+1, i->mac,
                    (wif->role==role_ap?IEEE80211_SPECIFIC_INFO_ROLE_AP:IEEE80211_SPECIFIC_INFO_ROLE_NON_AP_NON_PCP_STA), r->bw, r->channel, 0);
            }
        }
    } else if (i->type == interface_type_ethernet)
        s->media_type = MEDIA_TYPE_IEEE_802_3AB_GIGABIT_ETHERNET;
    return s;
}

static struct apOperationBSSStruct *_radioAddOperationalBSS(struct TLV *tlv, struct wifi_interface *wi)
{
    struct apOperationBSSStruct *s =
        (struct apOperationBSSStruct *)TLVStructDeclare(apOperationBSSStruct, &tlv->s.t.childs[0]);

    MACCPY(s->bssid, wi->i.mac);
    SSIDCPY(s->ssid, wi->bssInfo.ssid);

    return s;
}

struct macStruct *deviceAddOperationalRadio(struct TLV *tlv, struct al_device *d, struct radio *r)
{
    struct macStruct *s =
        (struct macStruct *)TLVStructDeclare(apOperationRadioStruct, &tlv->s.t.childs[0]);
    struct interface *intf;

    MACCPY(s->mac, r->uid);

    dlist_for_each(intf, d->interfaces, l) {
        if (intf->type==interface_type_wifi) {
            struct wifi_interface *wi = (struct wifi_interface *)intf;
            if (wi->radio==r && !wi->is_vbss)
                _radioAddOperationalBSS((struct TLV *)s, wi);
        }
    }

    return s;
}

static struct apOperationVBSSStruct *_radioAddOperationalVBSS(struct TLV *tlv, struct wifi_interface *wi)
{
    struct apOperationVBSSStruct *s =
        (struct apOperationVBSSStruct *)TLVStructDeclare(apOperationVBSSStruct, &tlv->s.t.childs[0]);

    MACCPY(s->bssid, wi->i.mac);
    SSIDCPY(s->ssid, wi->bssInfo.ssid);

    return s;
}

struct macStruct *deviceAddVbssConfigurationReportRadio(struct TLV *tlv, struct al_device *d, struct radio *r)
{
    struct macStruct *s = (struct macStruct *)TLVStructDeclare(vbssConfigurationReportRadioStruct, &tlv->s.t.childs[0]);
    struct interface *intf;

    MACCPY(s->mac, r->uid);

    dlist_for_each(intf, d->interfaces, l) {
        if (intf->type==interface_type_wifi) {
            struct wifi_interface *wi = (struct wifi_interface *)intf;
            if (wi->radio==r && wi->is_vbss) {
                _radioAddOperationalVBSS((struct TLV *)s, wi);
            }
        }
    }

    return s;
}

struct txOpclassStruct *apRadioBasicCapaTLVAddOpClass(struct TLV *tlv, uint8_t opclass, uint8_t max_tx_power,
                                                    uint8_t num, uint8_t *non_op_chans)
{
    struct txOpclassStruct *s =
        (struct txOpclassStruct *)TLVStructDeclare(txOpclassStruct, &tlv->s.t.childs[0]);

    s->opclass = opclass;
    s->max_tx_power = max_tx_power;

    s->non_op_chans[0] = num;
    if (num)
        memcpy(&s->non_op_chans[1], non_op_chans, num);
    return s;
}

static struct associatedClientStruct *_bssAddClient(struct TLV *tlv, struct client *client, uint32_t current)
{
    struct associatedClientStruct *s =
        (struct associatedClientStruct *)TLVStructDeclare(associatedClientStruct, &tlv->s.t.childs[0]);

    MACCPY(s->mac, client->mac);
    s->age = (current-client->last_assoc_ts)/1000;

    return s;
}

struct associatedBSSStruct *_deviceAddBSSAssociated(struct TLV *tlv, struct wifi_interface *wi, uint32_t current)
{
    struct associatedBSSStruct *s;
    struct client *client;

    if (dlist_empty(&wi->clients))
        return NULL;

    s = (struct associatedBSSStruct *)TLVStructDeclare(associatedBSSStruct, &tlv->s.t.childs[0]);
    MACCPY(s->bssid, wi->i.mac);
    dlist_for_each(client, wi->clients, l) {
        _bssAddClient((struct TLV *)s, client, current);
    }

    return s;
}

struct i1905NeighborStruct *i1905AddNeighbor(struct TLV *tlv, struct neighbor *n)
{
    struct i1905NeighborStruct *s =
        (struct i1905NeighborStruct *)TLVStructDeclare(i1905NeighborStruct, &tlv->s.t.childs[0]);

    MACCPY(s->mac, n->al_mac);
    s->bridge = 0;//FIXME
    return s;
}

struct macStruct *non1905AddNeighbor(struct TLV *tlv, struct neighbor *n)
{
    struct macStruct *s =
        (struct macStruct *)TLVStructDeclare(macStruct, &tlv->s.t.childs[0]);

    MACCPY(s->mac, n->al_mac);
    return s;
}

struct transmitterLinkMetricEntriesStruct *trasmitterLinkMetricAddEntry(struct TLV *tlv, struct transmitterLinkMetricEntriesStruct *entry)
{
    struct transmitterLinkMetricEntriesStruct *tx =
        (struct transmitterLinkMetricEntriesStruct *)TLVStructDeclare(transmitterLinkMetricEntriesStruct, &tlv->s.t.childs[0]);

    MACCPY(tx->local_interface_address, entry->local_interface_address);
    MACCPY(tx->neighbor_interface_address, entry->neighbor_interface_address);
    tx->tx_link_metrics = entry->tx_link_metrics;
    return tx;
}

struct receiverLinkMetricEntriesStruct *receiverLinkMetricAddEntry(struct TLV *tlv, struct receiverLinkMetricEntriesStruct *entry)
{
    struct receiverLinkMetricEntriesStruct *rx =
        (struct receiverLinkMetricEntriesStruct *)TLVStructDeclare(receiverLinkMetricEntriesStruct, &tlv->s.t.childs[0]);

    MACCPY(rx->local_interface_address, entry->local_interface_address);
    MACCPY(rx->neighbor_interface_address, entry->neighbor_interface_address);
    rx->rx_link_metrics = entry->rx_link_metrics;
    return rx;
}

struct steerPolicyStruct *steeringPolicyTLVAddRadio(struct TLV *tlv, uint8_t * rid, uint8_t policy,
                                                                uint8_t chan_util, uint8_t rcpi_threshold)
{
    struct steerPolicyStruct *s=
        (struct steerPolicyStruct *)TLVStructDeclare(steerPolicyStruct, &tlv->s.t.childs[2]);

    s->chan_util = chan_util;
    s->policy = policy;
    s->rcpi_thresh = rcpi_threshold;
    MACCPY(s->rid, rid);
    return s;
}

struct macStruct *TLVAddMac(struct dlist_head *parent, mac_address mac)
{
    struct macStruct *s =
        (struct macStruct *)TLVStructDeclare(macStruct, parent);

    MACCPY(s->mac, mac);
    return s;
}

struct metricRptPolicyStruct *metricReportPolicyTLVAddRadio(struct TLV *tlv, uint8_t *rid,
                            uint8_t rcpi_threshold, uint8_t rcpi_margin, uint8_t ch_util_threshold, uint8_t policy)
{
    struct metricRptPolicyStruct *s =
        (struct metricRptPolicyStruct *)TLVStructDeclare(metricRptPolicyStruct, &tlv->s.t.childs[0]);

    s->chan_util_thresh = ch_util_threshold;
    s->rcpi_margin = rcpi_margin;
    s->rcpi_thresh = rcpi_threshold;
    s->policy = policy;
    MACCPY(s->rid, rid);
    return s;
}

struct opClassStruct *chanPrefTLVAddOpclass(struct TLV *tlv, uint8_t opclass, uint8_t num_chans, uint8_t *p_chans,
                                                            uint8_t preference, uint8_t reason)
{
    struct opClassStruct *s =
        (struct opClassStruct *)TLVStructDeclare(opClassStruct, &tlv->s.t.childs[0]);
    uint8_t value = ((preference & CHAN_PREF_MASK) << CHAN_PREF_SHIFT)
                | ((reason & CHAN_PREF_REASON_MASK) << CHAN_PREF_REASON_SHIFT);

    s->opclass = opclass;
    s->non_operable_chans[0] = num_chans;
    memcpy(&s->non_operable_chans[1], p_chans, num_chans);
    s->pref_reason = value;
    return s;
}

struct opRestChanStruct *operRestOpcAddChan(struct opRestOpclassStruct *opc, uint8_t channel, uint8_t freq_sep)
{
    struct opRestChanStruct *c =
        (struct opRestChanStruct *)TLVStructDeclare(opRestChanStruct, &opc->s.t.childs[0]);
    c->channel = channel;
    c->freq_sep = freq_sep;
    return c;
}

struct opRestOpclassStruct *operRestTLVAddOpclass(struct TLV *tlv, uint8_t opclass)
{
    struct opRestOpclassStruct *s =
        (struct opRestOpclassStruct *)TLVStructDeclare(opRestOpclassStruct, &tlv->s.t.childs[0]);

    s->opclass = opclass;
    return s;
}

struct opclassChanPairStruct *opChanReportLVAddPair(struct TLV *tlv, uint8_t opclass, uint8_t channel)
{
    struct opclassChanPairStruct *s =
        (struct opclassChanPairStruct *)TLVStructDeclare(opclassChanPairStruct, &tlv->s.t.childs[0]);

    s->opclass = opclass;
    s->channel = channel;
    return s;
}

struct assocLinkMetricStruct *assocLinkMetricsTLVAddBssid(struct TLV *tlv, struct client *sta)
{
    struct assocLinkMetricStruct *s;

    if ((!sta) || (!sta->wif))
        return NULL;
    s = (struct assocLinkMetricStruct *)TLVStructDeclare(assocLinkMetricStruct, &tlv->s.t.childs[0]);

    MACCPY(s->bssid, sta->wif->i.mac);
    s->age = PLATFORM_GET_AGE(sta->link_metrics_ts);
    s->metrics = sta->link_metrics;

    return s;
}

struct macStruct *unassocStaLinkChanAddSta(struct unassocMetricQryStruct *chan, mac_address sta)
{
    struct macStruct *c =
        (struct macStruct *)TLVStructDeclare(macStruct, &chan->s.t.childs[0]);

    MACCPY(c->mac, sta);
    return c;
}

struct unassocMetricQryStruct *unassocStaLinkQueryAddChan(struct TLV *tlv, uint8_t channel)
{
    struct unassocMetricQryStruct *s =
        (struct unassocMetricQryStruct *)TLVStructDeclare(unassocMetricQryStruct, &tlv->s.t.childs[0]);

    s->channel = channel;
    return s;
}

struct unassocMetricRspStruct *unassocStaLinkRspAddSta(struct TLV *tlv, mac_address sta, uint8_t channel,
                                                                    uint32_t age, uint8_t rcpi_ul)
{
    struct unassocMetricRspStruct *s =
        (struct unassocMetricRspStruct *)TLVStructDeclare(unassocMetricRspStruct, &tlv->s.t.childs[0]);

    s->age = age;
    s->channel = channel;
    s->rcpi_ul = rcpi_ul;
    MACCPY(s->sta, sta);
    return s;
}

struct chanRptStruct *bcnMetricsQryAddChRpt(struct TLV *tlv, uint8_t len, uint8_t opclass,
                                                            uint8_t num_chans, uint8_t *channel)
{
    struct chanRptStruct *s =
        (struct chanRptStruct *)TLVStructNew(&SNAME(chanRptStruct), &tlv->s.t.childs[0], sizeof(struct chanRptStruct)+num_chans+1);

    s->chanreport.datap = (uint8_t *)(s+1);
    s->chanreport.len = num_chans+1;
    s->chanreport.datap[0] = opclass;
    if (channel && num_chans)
        memcpy(&s->chanreport.datap[1], channel, num_chans);
    return s;
}

struct targetBssStruct *steerReqAddTargetBss(struct TLV *tlv, uint8_t *bssid, uint8_t opclass, uint8_t channel)
{
    struct targetBssStruct *s =
        (struct targetBssStruct *)TLVStructDeclare(targetBssStruct, &tlv->s.t.childs[1]);

    MACCPY(s->target, bssid);
    s->channel = channel;
    s->opclass = opclass;
    return s;
}

struct macStruct *clientAssocCtrlReqAddSta(struct TLV *tlv, mac_address sta)
{
    struct macStruct *s =
         (struct macStruct *)TLVStructDeclare(macStruct, &tlv->s.t.childs[0]);

    MACCPY(s->mac, sta);
    return s;
}

struct measuredElemStruct *bcnMetricsRspAddElem(struct TLV *tlv, uint8_t* elem, uint16_t elem_len)
{
    struct measuredElemStruct *s =
        (struct measuredElemStruct *)TLVStructDeclare(measuredElemStruct, &tlv->s.t.childs[0]);

    s->elem.len = elem_len;
    s->elem.datap = elem;

    return s;
}

struct opClassChanStruct *opClassChanSet(struct opClassChanStruct *opc, uint8_t opclass, uint8_t chan_num, uint8_t *chan)
{
    if (opc == NULL) {
        return NULL;
    }
    opc->opclass = opclass;
    opc->chan.len = chan_num;
    if (chan && chan_num) {
        memcpy(opc->chan.data, chan, chan_num);
    }
    return opc;
}

struct opClassChanStruct *scanCapaRadioAddOpclassChan(struct scanCapaRadioStruct *radios, uint8_t opclass,
	                                                           uint8_t chan_num, uint8_t *chan)
{
    struct opClassChanStruct *opc =
        (struct opClassChanStruct *)TLVStructDeclare(opClassChanStruct, &radios->s.t.childs[0]);
    return opClassChanSet(opc, opclass, chan_num, chan);
}

struct scanCapaRadioStruct *scanCapaTLVAddRadios(struct TLV *tlv, uint8_t *ruid, uint8_t onboot,
	                                                        uint8_t impact, uint32_t min_scan_interval)
{
    struct scanCapaRadioStruct *r =
        (struct scanCapaRadioStruct *)TLVStructDeclare(scanCapaRadioStruct, &tlv->s.t.childs[0]);
    uint8_t value = ((onboot & SCAN_ONBOOT_MASK) << SCAN_ONBOOT_SHIFT)
                    | ((impact & SCAN_IMPACT_MASK) << SCAN_IMPACT_SHIFT);
    MACCPY(r->ruid, ruid);
    r->onboot_impact = value;
    r->min_scan_interval = min_scan_interval;
    return r;
}

struct opClassChanStruct *scanReqRadioAddOpclassChan(struct macStruct *radios, uint8_t opclass,
	                                                         uint8_t chan_num, uint8_t *chan)
{
    struct opClassChanStruct *opc =
        (struct opClassChanStruct *)TLVStructDeclare(opClassChanStruct, &radios->s.t.childs[0]);
    return opClassChanSet(opc, opclass, chan_num, chan);
}

struct macStruct *scanReqTLVAddRadios(struct TLV *tlv, uint8_t *ruid)
{
    struct macStruct *r =
        (struct macStruct *)TLVStructDeclare(scanReqRadioStruct, &tlv->s.t.childs[0]);

    MACCPY(r->mac, ruid);
    return r;
}

struct bssLoadEleStruct *scanNeighborAddBssloadEle(struct scanNeighborStruct *neighbor, uint8_t util, uint8_t cnt)
{
    struct bssLoadEleStruct *bssload = (struct bssLoadEleStruct *)TLVStructDeclare(bssLoadEleStruct, &neighbor->s.t.childs[0]);
    bssload->chann_utilize = util;
    bssload->sta_cnt = cnt;
    return bssload;
}
struct scanNeighborStruct *scanResultAddNeighbors(struct scanResultStruct *result,struct neighbor_bss *neighbor)
{
    uint8_t value = 0;
    char *bw_str = NULL;

    struct scanNeighborStruct *neighbor_s =
        (struct scanNeighborStruct *)TLVStructDeclare(scanNeighborStruct, &result->s.t.childs[0]);

    MACCPY(neighbor_s->bssid, neighbor->bssid);
    neighbor_s->ssid.len = neighbor->ssid.len;
    if (neighbor->ssid.len) {
        memcpy(neighbor_s->ssid.ssid, neighbor->ssid.ssid, neighbor_s->ssid.len);
    }
    neighbor_s->signal_strength = neighbor->signal_stength;
    bw_str = bandwidth2String(neighbor->bw);

    if (bw_str)
        neighbor_s->chanbw.len = strlen(bw_str);
    else
        DEBUG_ERROR("bw_str is NULL in scanResultAddNeighbors\n");

    if (neighbor_s->chanbw.len && bw_str) {
        memcpy(neighbor_s->chanbw.data, bw_str, neighbor_s->chanbw.len);
    }
    value = ((neighbor->bss_load_element_present & BSS_LOAD_ELE_PRESENT)
             | (neighbor->bss_color & BSS_COLOR_MASK));
    neighbor_s->bss_field = value;
    if (neighbor->bss_load_element_present) {
        scanNeighborAddBssloadEle(neighbor_s, neighbor->chan_utilize, neighbor->sta_cnt);
    }
    return neighbor_s;
}

struct scanResultStruct *scanResultTLVAddResult(struct TLV *tlv, uint8_t ts_len, uint8_t *ts, uint8_t utilization,
                                                     uint8_t noise, uint32_t scan_dur, uint8_t type)
{
    struct scanResultStruct *r =
        (struct scanResultStruct *)TLVStructDeclare(scanResultStruct, &tlv->s.t.childs[0]);

    r->timestamp.len = ts_len;
    if (ts_len && ts) {
        memcpy(r->timestamp.data, ts, ts_len);
    }
    r->utilization = utilization;
    r->noise = noise;
    r->aggre_scan_dur = scan_dur;
    r->scan_type = type;
    return r;
}

struct cacRadioStruct *cacReqTLVAddRadios(struct TLV *tlv, uint8_t *ruid, uint8_t opclass, uint8_t channel,
                                                  uint8_t cac_method, uint8_t cac_action)
{
    struct cacRadioStruct *r =
        (struct cacRadioStruct *)TLVStructDeclare(cacReqRadioStruct, &tlv->s.t.childs[0]);
    uint8_t value = ((cac_method & CAC_METHOD_MASK) << CAC_METHOD_SHIFT)
                    | ((cac_action & CAC_ACTION_MASK) << CAC_ACTION_SHIFT);
    MACCPY(r->ruid, ruid);
    r->opclass = opclass;
    r->channel = channel;
    r->cac_flag = value;
    return r;
}

struct cacTermRadioStruct *cacTermTLVAddRadios(struct TLV *tlv, uint8_t *ruid, uint8_t opclass, uint8_t channel)
{
    struct cacTermRadioStruct *r =
        (struct cacTermRadioStruct *)TLVStructDeclare(cacTermRadioStruct, &tlv->s.t.childs[0]);
    MACCPY(r->ruid, ruid);
    r->opclass = opclass;
    r->channel = channel;
    return r;
}

struct opclassChanPairStruct *cacReptRadioAddOpclsChan(struct cacRadioStruct *radio, uint8_t opclass, uint8_t chan)
{
    struct opclassChanPairStruct *r =
        (struct opclassChanPairStruct *)TLVStructDeclare(opclassChanPairStruct, &radio->s.t.childs[0]);
    r->opclass = opclass;
    r->channel = chan;
    return r;
}

struct cacRadioStruct *cacReptTLVAddRadios(struct TLV *tlv, uint8_t *ruid, uint8_t opclass, uint8_t channel, uint8_t cac_stat)
{
    struct cacRadioStruct *r =
        (struct cacRadioStruct *)TLVStructDeclare(cacReptRadioStruct, &tlv->s.t.childs[0]);
    MACCPY(r->ruid, ruid);
    r->opclass = opclass;
    r->channel = channel;
    r->cac_flag = cac_stat;
    return r;
}

struct cacStatChanStruct *cacStatTLVAddAvailChan(struct TLV *tlv, uint8_t opclass, uint8_t channel, uint16_t time)
{
    struct cacStatChanStruct *r =
        (struct cacStatChanStruct *)TLVStructDeclare(cacStatChanStruct, &tlv->s.t.childs[0]);
    r->opclass = opclass;
    r->channel =channel;
    r->time = time;
    return r;
}

struct cacStatChanStruct *cacStatTLVAddUnoccupyChan(struct TLV *tlv, uint8_t opclass, uint8_t channel, uint16_t dur)
{
    struct cacStatChanStruct *r =
        (struct cacStatChanStruct *)TLVStructDeclare(cacStatChanStruct, &tlv->s.t.childs[1]);
    r->opclass = opclass;
    r->channel =channel;
    r->time = dur;
    return r;
}

struct cacStatCacChanStruct *cacStatTLVAddCacChan(struct TLV *tlv, uint8_t opclass, uint8_t channel, uint32_t countdown)
{
    struct cacStatCacChanStruct *r =
        (struct cacStatCacChanStruct *)TLVStructDeclare(cacStatCacChanStruct, &tlv->s.t.childs[2]);
    r->opclass = opclass;
    r->channel =channel;
    r->cnt_down[0] = countdown & 0x000000ff;
    r->cnt_down[1] = (countdown & 0x0000ff00) >> 8;
    r->cnt_down[2] = (countdown & 0x00ff0000) >> 16;
    return r;
}

struct cacCapaOpclassStruct *cacCapaTypeAddOpclass(struct cacTypeStruct *type, uint8_t opclass)

{
    struct cacCapaOpclassStruct *r =
        (struct cacCapaOpclassStruct *)TLVStructDeclare(cacCapaOpclassStruct, &type->s.t.childs[0]);
    r->opclass = opclass;
    return r;
}

struct cacTypeStruct *cacCapaRadioAddType(struct macStruct *radio, uint8_t method, uint32_t dur)
{
    struct cacTypeStruct *r =
        (struct cacTypeStruct *)TLVStructDeclare(cacTypeStruct, &radio->s.t.childs[0]);
    r->method = method;
    r->dur[0] = dur & 0x000000ff;
    r->dur[1] = (dur & 0x0000ff00) >> 8;
    r->dur[2] = (dur & 0x00ff0000) >> 16;
    return r;
}

struct macStruct *cacCapaTLVAddRadio(struct TLV *tlv, uint8_t *ruid)
{
    struct macStruct *r =
        (struct macStruct *)TLVStructDeclare(cacCapaRadioStruct, &tlv->s.t.childs[0]);

    MACCPY(r->mac, ruid);
    return r;
}

struct trafficPolicyStruct *trafficPolicyTLVAddSsid(struct TLV *tlv, uint8_t len, uint8_t *ssid, uint16_t vlan_id)
{
    struct trafficPolicyStruct *r =
        (struct trafficPolicyStruct *)TLVStructDeclare(trafficPolicyStruct, &tlv->s.t.childs[0]);
    r->ssid.len = len;
    if (len && ssid) {
        memcpy(r->ssid.ssid, ssid, len);
    }
    r->vlanid = vlan_id;
    return r;
}

struct assocStatNotifyStruct *assocStatNotifyTLVAddBssid(struct TLV *tlv,  uint8_t *bssid, uint8_t status)
{
    struct assocStatNotifyStruct *r =
        (struct assocStatNotifyStruct *)TLVStructDeclare(assocStatNotifyStruct, &tlv->s.t.childs[0]);
    MACCPY(r->bssid, bssid);
    r->status = status;
    return r;
}

struct macStruct *bStaCapabilityAddMac(struct bSTARadioCapaTLV *tlv, uint8_t *mac)
{
    struct macStruct *s =
        (struct macStruct *)TLVStructDeclare(bStaMacStruct, &tlv->tlv.s.t.childs[0]);
    MACCPY(s->mac, mac);
    return s;
}

struct macStruct *ipv4AddInterface(struct TLV *tlv, uint8_t *mac)
{
    struct macStruct *s =
        (struct macStruct *)TLVStructDeclare(interfaceipv4Struct, &tlv->s.t.childs[0]);

    MACCPY(s->mac, mac);
    return s;
}

struct ipv4AddrStruct *ipv4InterfaceAddIP(struct macStruct *s, uint8_t proto, struct ipv4 *ip, struct ipv4 *dhcp)
{
    struct ipv4AddrStruct *ipv4_s =
        (struct ipv4AddrStruct *)TLVStructDeclare(ipv4AddrStruct, &s->s.t.childs[0]);
    ipv4_s->proto = proto;

    if (ip)
        ipv4_s->ip = *ip;
    if (dhcp)
        ipv4_s->dhcp_ip = *dhcp;
    return ipv4_s;
}

struct interfaceipv6Struct *ipv6AddInterface(struct TLV *tlv, uint8_t *mac, struct ipv6 *local_ip)
{
    struct interfaceipv6Struct *s =
        (struct interfaceipv6Struct *)TLVStructDeclare(interfaceipv6Struct, &tlv->s.t.childs[0]);

    MACCPY(s->mac, mac);
    if (local_ip)
        s->local_ip = *local_ip;
    return s;
}

struct ipv6AddrStruct *ipv6InterfaceAddIP(struct interfaceipv6Struct *s, uint8_t proto, struct ipv6 *ip, struct ipv6 *origin)
{
    struct ipv6AddrStruct *ipv6_s =
        (struct ipv6AddrStruct *)TLVStructDeclare(ipv6AddrStruct, &s->s.t.childs[0]);
    ipv6_s->proto = proto;

    if (ip)
        ipv6_s->ip = *ip;
    if (origin)
        ipv6_s->origin_ip = *origin;
    return ipv6_s;
}

struct TLVDesc *getDesc(struct TLVDesc *table, uint16_t max, uint16_t type)
{
    struct TLVDesc *desc = NULL;
    if (type<max) {
        desc = &table[type];
    }
    return (((desc) && (desc->name)) ? desc : NULL);
}


struct TLVDesc *getTLVDesc(uint8_t type)
{
    return getDesc(_tlv_descs, TLV_TYPE_MAX, type);
}

struct TLVDesc *getSubTLVDesc(uint8_t type, uint16_t subtype)
{
    switch (type) {
        case TLV_TYPE_VBSS:
            return getDesc(_vbss_tlv_descs, TLV_SUB_TYPE_VBSS_MAX, subtype);
            break;
        default:
            break;
    }
    return NULL;
}

static struct TLVDesc *getSubTLVDescExt(struct TLV *tlv, uint16_t subtype)
{
    struct TLVDesc *desc;

    if (!(desc = tlv->s.desc))
        return NULL;

    if (desc->tag == TLV_TYPE_VENDOR_SPECIFIC) {
        struct vendorTLV *vendor_tlv = (struct vendorTLV *)tlv;
        if (!OUICMP(vendor_tlv->oui, CLS_OUI)) {
            return getCLSTLVDesc(subtype);
        }
    } else {
            return getSubTLVDesc(desc->tag, subtype);
    }
    return NULL;
}

struct TLV *subTLVNew(dlist_head *parent, uint8_t type, uint16_t subtype, uint32_t size)
{
    struct TLVDesc *desc = getSubTLVDesc(type, subtype);
    struct TLV *tlv = (struct TLV *)TLVStructNew(desc, parent, size);

    if (tlv) {
        tlv->tlv_subtype = subtype;
    }
    return tlv;
}

struct TLV *superTLVNew(uint8_t type, struct TLV *sub_tlv)
{
    struct superTLV *tlv = NULL;

    if (sub_tlv) {
        tlv = (struct superTLV *)TLVNew(NULL, type, 0);
        tlv->sub_tlv = sub_tlv;
    }

    return (struct TLV *)tlv;
}

////////////////////////////////////////////////////////////////////////////////
// Actual API functions
////////////////////////////////////////////////////////////////////////////////


char *convert_1905_TLV_type_to_string(uint8_t tlv_type)
{
    switch (tlv_type)
    {
        case TLV_TYPE_END_OF_MESSAGE:
            return "TLV_TYPE_END_OF_MESSAGE";
        case TLV_TYPE_VENDOR_SPECIFIC:
            return "TLV_TYPE_VENDOR_SPECIFIC";
        case TLV_TYPE_AL_MAC_ADDRESS_TYPE:
            return "TLV_TYPE_AL_MAC_ADDRESS_TYPE";
        case TLV_TYPE_MAC_ADDRESS_TYPE:
            return "TLV_TYPE_MAC_ADDRESS_TYPE";
        case TLV_TYPE_DEVICE_INFORMATION_TYPE:
            return "TLV_TYPE_DEVICE_INFORMATION_TYPE";
        case TLV_TYPE_DEVICE_BRIDGING_CAPABILITIES:
            return "TLV_TYPE_DEVICE_BRIDGING_CAPABILITIES";
        case TLV_TYPE_NON_1905_NEIGHBOR_DEVICE_LIST:
            return "TLV_TYPE_NON_1905_NEIGHBOR_DEVICE_LIST";
        case TLV_TYPE_NEIGHBOR_DEVICE_LIST:
            return "TLV_TYPE_NEIGHBOR_DEVICE_LIST";
        case TLV_TYPE_LINK_METRIC_QUERY:
            return "TLV_TYPE_LINK_METRIC_QUERY";
        case TLV_TYPE_TRANSMITTER_LINK_METRIC:
            return "TLV_TYPE_TRANSMITTER_LINK_METRIC";
        case TLV_TYPE_RECEIVER_LINK_METRIC:
            return "TLV_TYPE_RECEIVER_LINK_METRIC";
        case TLV_TYPE_LINK_METRIC_RESULT_CODE:
            return "TLV_TYPE_LINK_METRIC_RESULT_CODE";
        case TLV_TYPE_SEARCHED_ROLE:
            return "TLV_TYPE_SEARCHED_ROLE";
        case TLV_TYPE_AUTOCONFIG_FREQ_BAND:
            return "TLV_TYPE_AUTOCONFIG_FREQ_BAND";
        case TLV_TYPE_SUPPORTED_ROLE:
            return "TLV_TYPE_SUPPORTED_ROLE";
        case TLV_TYPE_SUPPORTED_FREQ_BAND:
            return "TLV_TYPE_SUPPORTED_FREQ_BAND";
        case TLV_TYPE_WSC:
            return "TLV_TYPE_WSC";
        case TLV_TYPE_PUSH_BUTTON_EVENT_NOTIFICATION:
            return "TLV_TYPE_PUSH_BUTTON_EVENT_NOTIFICATION";
        case TLV_TYPE_PUSH_BUTTON_JOIN_NOTIFICATION:
            return "TLV_TYPE_PUSH_BUTTON_JOIN_NOTIFICATION";
        case TLV_TYPE_GENERIC_PHY_DEVICE_INFORMATION:
            return "TLV_TYPE_GENERIC_PHY_DEVICE_INFORMATION";
        case TLV_TYPE_DEVICE_IDENTIFICATION:
            return "TLV_TYPE_DEVICE_IDENTIFICATION";
        case TLV_TYPE_CONTROL_URL:
            return "TLV_TYPE_CONTROL_URL";
        case TLV_TYPE_IPV4:
            return "TLV_TYPE_IPV4";
        case TLV_TYPE_IPV6:
            return "TLV_TYPE_IPV6";
        case TLV_TYPE_GENERIC_PHY_EVENT_NOTIFICATION:
            return "TLV_TYPE_GENERIC_PHY_EVENT_NOTIFICATION";
        case TLV_TYPE_1905_PROFILE_VERSION:
            return "TLV_TYPE_1905_PROFILE_VERSION";
        case TLV_TYPE_POWER_OFF_INTERFACE:
            return "TLV_TYPE_POWER_OFF_INTERFACE";
        case TLV_TYPE_INTERFACE_POWER_CHANGE_INFORMATION:
            return "TLV_TYPE_INTERFACE_POWER_CHANGE_INFORMATION";
        case TLV_TYPE_INTERFACE_POWER_CHANGE_STATUS:
            return "TLV_TYPE_INTERFACE_POWER_CHANGE_STATUS";
        case TLV_TYPE_L2_NEIGHBOR_DEVICE:
            return "TLV_TYPE_L2_NEIGHBOR_DEVICE";
        default:
            return "Unknown";
    }

    // This code cannot be reached
    //
    return "";
}

