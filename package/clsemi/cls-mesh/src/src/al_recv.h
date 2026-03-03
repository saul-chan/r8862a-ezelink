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

#ifndef _AL_RECV_H_
#define _AL_RECV_H_

#include "1905_cmdus.h"
#include "lldp_payload.h"

typedef int (*higherLayerDataFunc)(struct al_device *, uint32_t, uint8_t *, uint16_t);

struct higherLayerDataProtocol {
    dlist_item l;
    higherLayerDataFunc func;
    uint8_t protocol;
};

int registerHigherLayerDataProtocol(uint8_t protocol, higherLayerDataFunc func);

// This function does "whatever needs to be done" as a result of receiving a
// CMDU: for example, some CMDU trigger a response, others are used to update
// the topology data base, etc...
//
// This function does *not* deal with "discarding" or "forwarding" the packet
// (that should have already been taken care of before this function is called)
//
// 'c' is the just received CMDU structure.
//
// 'receiving_interface_addr' is the MAC address of the local interface where
// the CMDU packet was received
//
// 'src_addr' is the MAC address contained in the 'src' field of the ethernet
// frame which contained the just received CMDU payload.
//
// 'queue_id' is the ID of the queue where messages to the AL entity should be
// posted in case new actions are derived from the processing of the current
// message.
//
// Return values:
//   PROCESS_CMDU_KO:
//     There was problem processing the CMDU
//   PROCESS_CMDU_OK:
//     The CMDU was correctly processed. No further action required.
//   PROCESS_CMDU_OK_TRIGGER_AP_AUTOCONF:
//     The CMDU was correctly processed. The caller should now trigger an "AP
//     auto config" process if there is still an unconfigured AP local interface.
//
#define PROCESS_CMDU_KO                             (0)
#define PROCESS_CMDU_OK                             (1)
#define PROCESS_CMDU_OK_TRIGGER_CONTROLLER_SEARCH   (2)

uint8_t process1905Cmdu(struct CMDU2 *c, uint32_t recv_intf_idx, uint8_t *src, uint8_t *dst);

// Call this function when receiving an LLPD "bridge discovery" message so that
// the topology database is properly updated.
//
uint8_t processLlpdPayload(struct PAYLOAD *payload, uint32_t intf_idx);

// Call this function when receiving an ALME REQUEST message. It will take
// action depending on the actual contents of this message (ie. "shut down an
// interface", "add a new bridge configuration", etc...)
//

uint8_t processTopologyDiscovery(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processTopologyNotification(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processLinkMetricQuery(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processLinkMetricResponse(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processAPAutoconfigurationSearch(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processTopologyQuery(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processAPAutoconfigurationResponse(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processAPAutoconfigurationRenew(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processHigherLayerQuery(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processHigherLayerResponse(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processTopologyResponse(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processAPAutoconfiguratioWSC(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processAPCapabilityQuery(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processAPCapabilityReport(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processMapPolicyConfigRequest(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processClientAssocControlRequest(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processCombinedInfrastructureMetrics(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processHigherLayerData(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processBackhaulSteeringRequest(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processBackhaulSteeringResponse(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);

uint8_t processClientCapabilityQuery(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processClientCapabilityReport(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processChannelScanRequest(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processChannelScanReport(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processCacRequest(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processCacTermination(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processBackhaulStaCapabilityQuery(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processBackhaulStaCapabilityReport(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processChannelPreferenceQuery(struct CMDU2 * c, uint32_t intf_idx, uint8_t * src, uint8_t * dst);
uint8_t processChannelPreferenceReport(struct CMDU2 * c, uint32_t intf_idx, uint8_t * src, uint8_t * dst);
uint8_t processFailedConnection(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processChannelSelectionRequest(struct CMDU2 * c, uint32_t intf_idx, uint8_t * src, uint8_t * dst);
uint8_t processChannelSelectionResponse(struct CMDU2 * c, uint32_t intf_idx, uint8_t * src, uint8_t * dst);
uint8_t processOperatingChannelReport(struct CMDU2 * c, uint32_t intf_idx, uint8_t * src, uint8_t * dst);
uint8_t processAPMetricsQuery(struct CMDU2 * c, uint32_t intf_idx, uint8_t * src, uint8_t * dst);
uint8_t processAPMetricsResponse(struct CMDU2 * c, uint32_t intf_idx, uint8_t * src, uint8_t * dst);
uint8_t processAssociatedStaLinkMetricsQuery(struct CMDU2 * c, uint32_t intf_idx, uint8_t * src, uint8_t * dst);
uint8_t processAssociatedStaLinkMetricsResponse(struct CMDU2 * c, uint32_t intf_idx, uint8_t * src, uint8_t * dst);
uint8_t processUnassociatedStaLinkMetricsQuery(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processUnassociatedStaLinkMetricsResponse(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processClientSteeringRequest(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processSteeringBTMReport(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processSteeringComplete(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processBeaconMetricsQuery(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processBeaconMetricsResponse(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processAssocStatusNotification(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processClientDisassocStat(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processErrorResponse(struct CMDU2 * c, uint32_t intf_idx, uint8_t * src, uint8_t * dst);
uint8_t processVBSSCapabilitiesRequest(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processVBSSCapabilitiesResponse(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processVBSSRequest(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processVBSSResponse(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processClientSecurityContextRequest(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processClientSecurityContextResponse(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processTriggerCSARequest(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processTriggerCSAResponse(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processVbssMovePreparationRequest(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processVbssMovePreparationResponse(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processVbssMoveCancelRequest(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processVbssMoveCancelResponse(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);
uint8_t processServicePrioritizationRequest(struct CMDU2 *c, uint32_t intf_idx, uint8_t *src, uint8_t *dst);

#endif


