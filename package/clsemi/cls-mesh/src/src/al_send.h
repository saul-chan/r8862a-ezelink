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

#ifndef _AL_SEND_H_
#define _AL_SEND_H_

#include "1905_cmdus.h"
#include "1905_tlvs.h"
#include "al_utils.h"

////////////////////////////////////////////////////////////////////////////////
// Functions to send "raw" 1905 messages
////////////////////////////////////////////////////////////////////////////////

uint8_t sendRawPacket2(struct CMDU2 *cmdu, char *interface_name, uint8_t *dst, uint8_t *src);
uint8_t sendRawPacketAuto(struct CMDU2 *cmdu, char *interface_name, uint8_t *dst, uint8_t *src);
uint8_t sendMulticast(struct CMDU2 *cmdu, uint32_t exclude, uint8_t *dst, uint8_t *src);

struct TLV *obtainErrCodeTLV(uint8_t err_code, uint8_t *sta);
uint8_t reportChanScanResult(struct chscan_req *req);

uint8_t sendTopologyDiscovery(char *interface_name, uint16_t mid);
uint8_t sendTopologyNotification(uint16_t mid, struct client *sta, uint8_t join);
uint8_t sendZeroTLV(uint16_t type, char *interface_name, uint16_t mid, uint8_t *dst);
uint8_t sendTopologyQuery(char *interface_name, uint16_t mid, uint8_t *dst);
uint8_t sendTopologyResponse(char *interface_name, uint16_t mid, uint8_t *dst);
uint8_t sendLinkMetricsQuery(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t *neighbor_al_mac, uint8_t link_metrics_type);
uint8_t sendLinkMetricsResponse(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t *neighbor_al_mac, uint8_t link_metrics_type);
uint8_t sendAPAutoconfigurationSearch(uint16_t mid, uint8_t freq_band);
uint8_t sendAPAutoconfigurationResponse(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t freq_band);
uint8_t sendAPAutoconfigurationRenew(uint16_t mid, uint8_t freq_band, struct al_device *dst);
uint8_t sendHigherLayerResponse(char *interface_name, uint16_t mid, uint8_t *dst);
uint8_t sendAPAutoconfigurationWSCM1(char *interface_name, uint16_t mid, uint8_t *dst, struct radio *r, struct TLV *m1);
uint8_t sendAPAutoconfigurationWSCM2(char *interface_name, uint16_t mid, uint8_t *dst, struct radio *r, dlist_head *m2s);
#define sendHighLayerQuery(...) sendZeroTLV(CMDU_TYPE_HIGHER_LAYER_QUERY, __VA_ARGS__)
#define sendAPCapabilityQuery(...) sendZeroTLV(CMDU_TYPE_AP_CAPABILITY_QUERY, __VA_ARGS__)
uint8_t sendAPCapabilityReport(char *interface_name, uint16_t mid, uint8_t *dst);
uint8_t sendMapPolicyConfigRequest(char *interface_name, uint16_t mid, uint8_t *dst, struct map_policy *policy);
uint8_t sendClientCapabilityQuery(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t *bssid, uint8_t *sta);
uint8_t sendClientCapabilityReport(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t *bssid, uint8_t *sta);
#define sendChannelPreferenceQuery(...) sendZeroTLV(CMDU_TYPE_CHANNEL_PREFERENCE_QUERY, __VA_ARGS__)
uint8_t sendChannelPreferenceReport(char *interface_name, uint16_t mid, uint8_t * dst, struct al_device *dev);
uint8_t sendCacScanChannelPreferenceReport(char *interface_name, uint16_t mid, uint8_t *dst, struct radio *r);
uint8_t sendSingleClientAssocControl(struct al_device *d, uint8_t *bssid, uint8_t *mac, uint16_t period, uint8_t control);
uint8_t sendClientAssocControlRequest(char *interface_name, uint16_t mid, uint8_t *dst, struct dlist_head *req_list);
uint8_t sendChannelScanRequest(char *interface_name, uint16_t mid, uint8_t *dst,
                                      uint8_t fresh_scan, uint8_t num, struct chscan_req *reqs);
uint8_t sendChannelScanReport(char *interface_name, uint16_t mid, uint8_t *dst, struct radio *r, struct chscan_req *req);
uint8_t sendCacRequest(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t num_reqs, struct cac_request *reqs);
uint8_t sendCacTermination(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t num_terms, struct cac_termination *terms);
uint8_t sendCombinedInfrastructMetrics(char *interface_name, uint16_t mid, uint8_t *dst);
uint8_t sendBackhaulSteeringRequest(char *interface_name, uint16_t mid, uint8_t *dst,
                                            uint8_t *sta, uint8_t *target, uint8_t opclass, uint8_t channel);
uint8_t sendBackhaulSteeringResponse(char *interface_name, uint16_t mid, uint8_t *dst,
                                             struct wifi_interface *wif, uint8_t result, uint8_t code);
#define sendBackhaulSTACapabilityQuery(...) sendZeroTLV(CMDU_TYPE_BACKHAUL_STA_CAPABILITY_QUERY, __VA_ARGS__)
uint8_t sendBackhaulSTACapabilityReport(char *interface_name, uint16_t mid, uint8_t * dst);
uint8_t sendFailedConnection(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t *sta, uint16_t status, uint16_t reason);
uint8_t sendChannelSelectionRequest(char *interface_name, uint16_t mid, uint8_t * dst, struct al_device *dev);
uint8_t sendChannelSelectionResponse(char *interface_name, uint16_t mid, uint8_t * dst);
uint8_t sendOperatingChannelReport(char *interface_name, uint16_t mid, uint8_t * dst, struct al_device *dev, int force);
uint8_t sendApMetricsQuery(char * interface_name, uint16_t mid, uint8_t * dst, struct dlist_head * vap_list);
uint8_t sendApMetricsResponse(char *interface_name, uint16_t mid, uint8_t * dst, struct dlist_head *vap_list, uint8_t crossed);
uint8_t sendAssociatedStaLinkMetricsQuery(char * interface_name, uint16_t mid, uint8_t * dst, uint8_t * sta_mac);
uint8_t sendAssociatedStaLinkMetricsResponse(char * interface_name, uint16_t mid, uint8_t * dst, uint8_t * sta_mac);
uint8_t sendUnassociatedStaLinkMetricsQuery(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t opclass,
                                                    dlist_head *unassoc_list);
uint8_t sendUnassociatedStaLinkMetricsResponse(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t opclass,
                                                    dlist_head *sta_list, dlist_head *channel_list);
uint8_t sendAck(char *interface_name, uint16_t mid, uint8_t *dst, dlist_head *err_codes);
uint8_t sendClientSteeringRequest(char *interface_name, uint16_t mid, uint8_t * dst, dlist_head *target_list,
                                            dlist_head *sta_list, uint16_t disassoc_timer, uint16_t steering_window,
                                            bool mandatory, bool imminent, bool abridged, uint8_t *bssid);
uint8_t sendClientSteeringBTMReport(char *interface_name, uint16_t mid, uint8_t * dst, uint8_t *bssid_rpt,
                                                uint8_t *sta, uint8_t status, uint8_t *target_bssid);
#define sendClientSteeringComplete(...) sendZeroTLV(CMDU_TYPE_STEERING_COMPLETED, __VA_ARGS__)
uint8_t sendBeaconMetricsQuery(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t *sta_request,
                                        uint8_t opclass_request, uint8_t chan_request,
                                        uint8_t *bssid_request, uint8_t detail, uint8_t len_ssid, char *ssid,
                                        dlist_head *list_chan_report, uint8_t *eid);
uint8_t sendBeaconMetricsResponse(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t *sta, uint8_t *report,
                                    uint16_t len);
uint8_t sendTunnledMessage(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t *client, uint8_t type,
                            uint8_t *frame, uint16_t frame_len);
uint8_t sendAssociationStatNotification(char *interface_name, uint16_t mid, uint8_t *dst,
                                                        uint8_t *bssid, uint8_t status);
uint8_t sendClientDisassocStatsMessage(char *interface_name, uint16_t mid, uint8_t *dst, struct client *sta, uint16_t reason);
uint8_t sendErrorResponse(char *interface_name, uint16_t mid, uint8_t *dst, dlist_head *ec_list);
#define sendVBSSCapabilitiesRequest(...) sendZeroTLV(CMDU_TYPE_VBSS_CAPABILITIES_REQUEST, __VA_ARGS__);
uint8_t sendVBSSCapabilitiesResponse(char *interface_name, uint16_t mid, uint8_t * dst);
uint8_t sendVBSSCreationRequest(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t *ruid, struct bss_info *bss,
    struct vbss_client_context_info *client_context, struct client *sta, char *dpp_connection);
uint8_t sendVBSSDestructionRequest(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t *ruid,
            uint8_t *bssid, uint8_t disassoc_client, char *dpp_connection);
uint8_t sendVBSSResponse(char *interface_name, uint16_t mid, uint8_t * dst, uint8_t *ruid,
            uint8_t *bssid, uint8_t success);
uint8_t sendClientSecurityContextRequest(char *interface_name, uint16_t mid, uint8_t * dst,
                uint8_t *bssid, uint8_t *sta_mac);
uint8_t sendClientSecurityContextResponse(char *interface_name, uint16_t mid, uint8_t * dst,
                uint8_t *bssid, uint8_t *sta_mac, struct vvData *ptk, struct vvData *gtk,
                uint64_t tx_packet_number, uint64_t group_tx_packet_number);
uint8_t sendTriggerCSARequest(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t *ruid,
                uint8_t *bssid, uint8_t *sta_mac, uint8_t csa_channel, uint8_t opclass);
uint8_t sendTriggerCSAResponse(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t *ruid,
                uint8_t *bssid, uint8_t *sta_mac, uint8_t csa_channel, uint8_t opclass);
uint8_t sendVbssMovePreparationRequest(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t *bssid, uint8_t *sta_mac);
uint8_t sendVbssMovePreparationResponse(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t *bssid, uint8_t *sta_mac);
uint8_t sendVbssMoveCancelRequest(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t *bssid, uint8_t *sta_mac);
uint8_t sendVbssMoveCancelResponse(char *interface_name, uint16_t mid, uint8_t *dst, uint8_t *bssid, uint8_t *sta_mac);
uint8_t sendServicePrioritizationRequest(char *intf_name, uint16_t mid, uint8_t *dst);

////////////////////////////////////////////////////////////////////////////////
// Functions to send LLDP messages
////////////////////////////////////////////////////////////////////////////////

// This function sends a "LLDP bridge discovery packet" on the provided
// interface.
//
// 'interface_name' is one of the values returned by
// "PLATFORM_GET_LIST_OF_1905_INTERFACES()" and must refer to the interface we
// want to use to send the packet.
//
//
// The format of this packet is detailed in "Section 6.1"
//
// Return "0" if a problem was found. "1" otherwise.
//
uint8_t sendLLDPDiscovery(char *interface_name);


#endif
