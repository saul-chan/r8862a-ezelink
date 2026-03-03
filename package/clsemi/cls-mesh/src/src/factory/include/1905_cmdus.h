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

#ifndef _1905_CDMUS_H_
#define _1905_CDMUS_H_

#include "platform.h"

////////////////////////////////////////////////////////////////////////////////
// CMDU message type as detailed in "IEEE Std 1905.1-2013, Table 6-4"
////////////////////////////////////////////////////////////////////////////////
#define CMDU_TYPE_TOPOLOGY_DISCOVERY               0x0000
#define CMDU_TYPE_TOPOLOGY_NOTIFICATION            0x0001
#define CMDU_TYPE_TOPOLOGY_QUERY                   0x0002
#define CMDU_TYPE_TOPOLOGY_RESPONSE                0x0003
#define CMDU_TYPE_VENDOR_SPECIFIC                  0x0004
#define CMDU_TYPE_LINK_METRIC_QUERY                0x0005
#define CMDU_TYPE_LINK_METRIC_RESPONSE             0x0006
#define CMDU_TYPE_AP_AUTOCONFIGURATION_SEARCH      0x0007
#define CMDU_TYPE_AP_AUTOCONFIGURATION_RESPONSE    0x0008
#define CMDU_TYPE_AP_AUTOCONFIGURATION_WSC         0x0009
#define CMDU_TYPE_AP_AUTOCONFIGURATION_RENEW       0x000a
#define CMDU_TYPE_PUSH_BUTTON_EVENT_NOTIFICATION   0x000b
#define CMDU_TYPE_PUSH_BUTTON_JOIN_NOTIFICATION    0x000c
#define CMDU_TYPE_HIGHER_LAYER_QUERY               0x000d
#define CMDU_TYPE_HIGHER_LAYER_RESPONSE            0x000e
#define CMDU_TYPE_INTERFACE_POWER_CHANGE_REQUEST   0x000f
#define CMDU_TYPE_INTERFACE_POWER_CHANGE_RESPONSE  0x0010
#define CMDU_TYPE_GENERIC_PHY_QUERY                0x0011
#define CMDU_TYPE_GENERIC_PHY_RESPONSE             0x0012
#define CMDU_TYPE_1905_LAST                        CMDU_TYPE_GENERIC_PHY_RESPONSE

//////////////////////////////////////////////////////////////////////////////////
// CMDU message type as detailed in "Multi-AP_Specification_v1.0, table 5"
//////////////////////////////////////////////////////////////////////////////////
#define CMDU_TYPE_ACK                                       0x8000
#define CMDU_TYPE_AP_CAPABILITY_QUERY                       0x8001
#define CMDU_TYPE_AP_CAPABILITY_REPORT                      0x8002
#define CMDU_TYPE_MAP_POLICY_CONFIG_REQUEST                 0x8003
#define CMDU_TYPE_CHANNEL_PREFERENCE_QUERY                  0x8004
#define CMDU_TYPE_CHANNEL_PREFERENCE_REPORT                 0x8005
#define CMDU_TYPE_CHANNEL_SELECTION_REQUEST                 0x8006
#define CMDU_TYPE_CHANNEL_SELECTION_RESPONSE                0x8007
#define CMDU_TYPE_OPERATING_CHANNEL_REPORT                  0x8008
#define CMDU_TYPE_CLIENT_CAPABILITY_QUERY                   0x8009
#define CMDU_TYPE_CLIENT_CAPABILITY_REPORT                  0x800A
#define CMDU_TYPE_AP_METRICS_QUERY                          0x800B
#define CMDU_TYPE_AP_METRICS_RESPONSE                       0x800C
#define CMDU_TYPE_ASSOCIATED_STA_LINK_METRICS_QUERY         0x800D
#define CMDU_TYPE_ASSOCIATED_STA_LINK_METRICS_RESPONSE      0x800E
#define CMDU_TYPE_UNASSOCIATED_STA_LINK_METRICS_QUERY       0x800F
#define CMDU_TYPE_UNASSOCIATED_STA_LINK_METRICS_RESPONSE    0x8010
#define CMDU_TYPE_BEACON_METRICS_QUERY                      0x8011
#define CMDU_TYPE_BEACON_METRICS_RESPONSE                   0x8012
#define CMDU_TYPE_COMBINED_INFRASTRUCTURE_METRICS           0x8013
#define CMDU_TYPE_CLIENT_STEERING_REQUEST                   0x8014
#define CMDU_TYPE_CLIENT_STEERING_BTM_REPORT                0x8015
#define CMDU_TYPE_CLIENT_ASSOCIATION_CONTROL                0x8016
#define CMDU_TYPE_STEERING_COMPLETED                        0x8017
#define CMDU_TYPE_HIGHER_LAYER_DATA                         0x8018
#define CMDU_TYPE_BACKHAUL_STEERING_REQUEST                 0x8019
#define CMDU_TYPE_BACKHAUL_STEERING_RESPONSE                0x801A

//////////////////////////////////////////////////////////////////////////////////
// CMDU message type as detailed in "Multi-AP_Specification_v2.0, table 9"
//////////////////////////////////////////////////////////////////////////////////

#define CMDU_TYPE_CHANNEL_SCAN_REQUEST                      0x801B
#define CMDU_TYPE_CHANNEL_SCAN_REPORT                       0x801C
#define CMDU_TYPE_CAC_REQUEST                               0x8020
#define CMDU_TYPE_CAC_TERMINATION                           0x8021
#define CMDU_TYPE_CLIENT_DISASSOCIATION_STATS               0x8022
#define CMDU_TYPE_ERROR_RESPONSE                            0x8024
#define CMDU_TYPE_ASSOCIATION_STATUS_NOTIFICATION           0x8025
#define CMDU_TYPE_TUNNELED                                  0x8026
#define CMDU_TYPE_BACKHAUL_STA_CAPABILITY_QUERY             0x8027
#define CMDU_TYPE_BACKHAUL_STA_CAPABILITY_REPORT            0x8028
#define CMDU_TYPE_FAILED_CONNECTION                         0x8033

//////////////////////////////////////////////////////////////////////////////////
// CMDU message type as detailed in "Wi-Fi_EasyMesh_Specification_v3, table 16"
//////////////////////////////////////////////////////////////////////////////////
#define CMDU_TYPE_DPP_CCE_INDICATION                        0x801D
#define CMDU_TYPE_1905_REKEY_REQUEST                        0x801E
#define CMDU_TYPE_1905_DECRYPTION_FAILURE                   0x801F
#define CMDU_TYPE_SERVICE_PRIORITIZATION_REQUEST            0x8023
#define CMDU_TYPE_PROXIED_ENCAP_DPP                         0x8029
#define CMDU_TYPE_DIRECT_ENCAP_DPP                          0x802A
#define CMDU_TYPE_RECONFIGURATION_TRIGGER                   0x802B
#define CMDU_TYPE_BSS_CONFIGURATION_REQUEST                 0x802C
#define CMDU_TYPE_BSS_CONFIGURATION_RESPONSE                0x802D
#define CMDU_TYPE_BSS_CONFIGURATION_RESULT                  0x802E
#define CMDU_TYPE_CHIRP_NOTIFICATION                        0x802F
#define CMDU_TYPE_1905_ENCAP_EAPOL                          0x8030
#define CMDU_TYPE_DPP_BOOTSTRAPPING_URI_NOTIFICATION        0x8031
#define CMDU_TYPE_DPP_BOOTSTRAPPING_URI_QUERY               0x8032
#define CMDU_TYPE_AGENT_LIST                                0x8035

//////////////////////////////////////////////////////////////////////////////////
// CMDU message type as detailed in "Wi-Fi_EasyMesh_Specification_v5, table 16"
//////////////////////////////////////////////////////////////////////////////////
#define CMDU_TYPE_VBSS_CAPABILITIES_REQUEST                 0x8038
#define CMDU_TYPE_VBSS_CAPABILITIES_RESPONSE                0x8039
#define CMDU_TYPE_VBSS_REQUEST                              0x803a
#define CMDU_TYPE_VBSS_RESPONSE                             0x803b
#define CMDU_TYPE_CLIENT_SECURITY_CONTEXT_REQUEST           0x803c
#define CMDU_TYPE_CLIENT_SECURITY_CONTEXT_RESPONSE          0x803d
#define CMDU_TYPE_TRIGGER_CSA_REQUEST                       0x803e
#define CMDU_TYPE_TRIGGER_CSA_RESPONSE                      0x803f
#define CMDU_TYPE_VBSS_MOVE_PREPARATION_REQUEST             0x8040
#define CMDU_TYPE_VBSS_MOVE_PREPARATION_RESPONSE            0x8041
#define CMDU_TYPE_VBSS_MOVE_CANCEL_REQUEST                  0x8042
#define CMDU_TYPE_VBSS_MOVE_CANCEL_RESPONSE                 0x8043

#define CMDU_TYPE_MAP_FIRST                            CMDU_TYPE_ACK
#define CMDU_TYPE_MAP_LAST                             CMDU_TYPE_VBSS_MOVE_CANCEL_RESPONSE

#define CMDU_TYPE_INVALID                                   0xffff

////////////////////////////////////////////////////////////////////////////////
// CMDU message version
////////////////////////////////////////////////////////////////////////////////
#define CMDU_MESSAGE_VERSION_1905_1_2013  (0x00)


////////////////////////////////////////////////////////////////////////////////
// CMDU associated structures
////////////////////////////////////////////////////////////////////////////////

struct CMDU2 {
    uint8_t   ref;
    uint8_t   version;       // One of "CMD_t_MESSAGE_VERSION_*" values
    uint16_t  type;          // Any of the CMD_t_TYPE_* types
    uint16_t  id;            // Identifies the message
    uint8_t   relay;         // Set to '1' to indicate that his packet
    dlist_head tlvs;
    dlist_head streams;
};


typedef uint8_t (*cmdu_process_func)(struct CMDU2 *, uint32_t intf_idx, uint8_t *src, uint8_t *dst);

struct cmdu_msg_desc {
    const char *name;
    cmdu_process_func process;
    uint16_t resp_type;
#define FLAG_RESPONSE_TYPE  BIT(0) //as a response
#define FLAG_UNSOLICITED    BIT(1) //solicited as response and unsolicited need ack
    uint8_t flag;
};


////////////////////////////////////////////////////////////////////////////////
// Main API functions
////////////////////////////////////////////////////////////////////////////////

struct CMDU2 *parse_1905_CMDU_from_packets(uint8_t **packet_streams, uint16_t *lens);


////////////////////////////////////////////////////////////////////////////////
// Utility API functions
////////////////////////////////////////////////////////////////////////////////

// Return the 'mid', 'fragment_id' and 'last_fragment_indicator' of the CMDU
// contained in the given 'stream' in the provided output variables.
//
// Return "0" if an error preventing the parsing takes place, "1" otherwise.
//
uint8_t parse_1905_CMDU_header_from_packet(uint8_t *stream, uint16_t *mid, uint8_t *fragment_id, uint8_t *last_fragment_indicator);

const char *convert_1905_CMDU_type_to_string(uint16_t cmdu_type);
struct CMDU2 *cmdu2New(uint16_t type, uint16_t mid);
void cmdu2Free(struct CMDU2 *c);
struct CMDU2 *cmdu2Ref(struct CMDU2 *c);
struct TLV *getTypedTLV(struct CMDU2 *c, uint8_t type, int idx);
struct TLV *getSubTypedTLV(struct CMDU2 *c, uint8_t type, uint16_t subtype, int idx);
struct TLV *getCLSTypedTLV(struct CMDU2 *c, uint8_t type, int idx);
struct TLVStruct *getChildTLVStruct(struct TLVStruct *parent, uint8_t order, int idx);
struct cmdu_msg_desc *CMDU_type_to_desc(uint16_t cmdu_type);
void cmdu2AddTlv(struct CMDU2 *c, struct TLV *tlv);

int encodeCMDU(struct CMDU2 *c);
int updateCMDU(struct CMDU2 *c, uint16_t mid);
#endif
