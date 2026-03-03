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

#ifndef _1905_TLVS_H_
#define _1905_TLVS_H_

#include "platform.h"
#include "tlv.h"
#include "datamodel.h"


#define SNAME(_struct) _desc_##_struct

#define DECLARE_STRUCT_DESC_GLOBAL(name) \
struct TLVDesc SNAME(name)

#define TLVStructDeclareGlobal(_type, _parent) TLVStructNew(&SNAME(_type), _parent, 0)

#define FIELD_DESC(structtype, field, type, ...) \
{ \
    .name = #field, \
    .size = sizeof(((structtype*)0)->field), \
    .offset = offsetof(structtype, field), \
    .fmt = type, \
    __VA_ARGS__ \
}

#define TERMINATOR {0}

#define EXTRA_FIELDS \
    .fields1 = { \
        TERMINATOR, \
    },

#define TLV_DESC_0FIELD(struct_name, _count_type, children, ...) \
    { \
        .name = #struct_name, \
        .size = sizeof(struct struct_name), \
        .count_type = _count_type, \
        .fields = { \
            TERMINATOR, \
        }, \
        .childs = { \
            children, \
            __VA_ARGS__ \
        }, \
        .ops = NULL, \
        EXTRA_FIELDS \
    }

#define TLV_DESC_1FIELD(struct_name, _count_type, field1, fmt1, children, ...) \
    { \
        .name = #struct_name, \
        .size = sizeof(struct struct_name), \
        .count_type = _count_type, \
        .fields = { \
            FIELD_DESC(struct struct_name, field1, fmt1), \
            TERMINATOR, \
        }, \
        .childs = { \
            children, \
            __VA_ARGS__ \
        }, \
        .ops = NULL, \
        EXTRA_FIELDS \
    }

#define TLV_DESC_1FIELD_NAMED(_myname, struct_name, _count_type, field1, fmt1, children, ...) \
    { \
        .name = #_myname, \
        .size = sizeof(struct struct_name), \
        .count_type = _count_type, \
        .fields = { \
            FIELD_DESC(struct struct_name, field1, fmt1), \
            TERMINATOR, \
        }, \
        .childs = { \
            children, \
            __VA_ARGS__ \
        }, \
        .ops = NULL, \
        EXTRA_FIELDS \
    }

#define TLV_DESC_1_1FIELDS(struct_name, _count_type, field1, fmt1, field2, fmt2, children, ...) \
    { \
        .name = #struct_name, \
        .size = sizeof(struct struct_name), \
        .count_type = _count_type, \
        .fields = { \
            FIELD_DESC(struct struct_name, field1, fmt1), \
            TERMINATOR, \
        }, \
        .childs = { \
            children, \
            __VA_ARGS__ \
        }, \
        .ops = NULL, \
        .fields1 = { \
            FIELD_DESC(struct struct_name, field2, fmt2), \
            TERMINATOR, \
        }, \
    }

#define TLV_DESC_2FIELDS(struct_name, _count_type, field1, fmt1, field2, fmt2, children, ...) \
    { \
        .name = #struct_name, \
        .size = sizeof(struct struct_name), \
        .count_type = _count_type, \
        .fields = { \
            FIELD_DESC(struct struct_name, field1, fmt1), \
            FIELD_DESC(struct struct_name, field2, fmt2), \
            TERMINATOR, \
        }, \
        .childs = { \
            children, \
            __VA_ARGS__ \
        }, \
        .ops = NULL, \
        EXTRA_FIELDS \
    }

#define TLV_DESC_2FIELDS_NAMED(_myname, struct_name, _count_type, field1, fmt1, field2, fmt2, children, ...) \
    { \
        .name = _myname, \
        .size = sizeof(struct struct_name), \
        .count_type = _count_type, \
        .fields = { \
            FIELD_DESC(struct struct_name, field1, fmt1), \
            FIELD_DESC(struct struct_name, field2, fmt2), \
            TERMINATOR, \
        }, \
        .childs = { \
            children, \
            __VA_ARGS__ \
        }, \
        .ops = NULL, \
        EXTRA_FIELDS \
    }

#define TLV_DESC_3FIELDS(struct_name, _count_type, field1, fmt1, field2, fmt2, field3, fmt3, children, ...) \
    { \
        .name = #struct_name, \
        .size = sizeof(struct struct_name), \
        .count_type = _count_type, \
        .fields = { \
            FIELD_DESC(struct struct_name, field1, fmt1), \
            FIELD_DESC(struct struct_name, field2, fmt2), \
            FIELD_DESC(struct struct_name, field3, fmt3), \
            TERMINATOR, \
        }, \
        .childs = { \
            children, \
            __VA_ARGS__ \
        }, \
        .ops = NULL, \
        EXTRA_FIELDS \
    }

#define TLV_DESC_4FIELDS(struct_name, _count_type, field1, fmt1, field2, fmt2, field3, fmt3, field4, fmt4, children, ...) \
    { \
        .name = #struct_name, \
        .size = sizeof(struct struct_name), \
        .count_type = _count_type, \
        .fields = { \
            FIELD_DESC(struct struct_name, field1, fmt1), \
            FIELD_DESC(struct struct_name, field2, fmt2), \
            FIELD_DESC(struct struct_name, field3, fmt3), \
            FIELD_DESC(struct struct_name, field4, fmt4), \
            TERMINATOR, \
        }, \
        .childs = { \
            children, \
            __VA_ARGS__ \
        }, \
        .ops = NULL, \
        EXTRA_FIELDS \
    }

#define TLV_DESC_5FIELDS(struct_name, _count_type, field1, fmt1, field2, fmt2, field3, fmt3, field4, fmt4, field5,fmt5, children, ...) \
    { \
        .name = #struct_name, \
        .size = sizeof(struct struct_name), \
        .count_type = _count_type, \
        .fields = { \
            FIELD_DESC(struct struct_name, field1, fmt1), \
            FIELD_DESC(struct struct_name, field2, fmt2), \
            FIELD_DESC(struct struct_name, field3, fmt3), \
            FIELD_DESC(struct struct_name, field4, fmt4), \
            FIELD_DESC(struct struct_name, field5, fmt5), \
            TERMINATOR, \
        }, \
        .childs = { \
            children, \
            __VA_ARGS__ \
        }, \
        .ops = NULL, \
        EXTRA_FIELDS \
    }

#define TLV_DESC_3_2FIELDS(struct_name, _count_type, field1, fmt1, field2, fmt2, field3, fmt3, field4, fmt4, field5, fmt5, children, ...) \
    { \
        .name = #struct_name, \
        .size = sizeof(struct struct_name), \
        .count_type = _count_type, \
        .fields = { \
            FIELD_DESC(struct struct_name, field1, fmt1), \
            FIELD_DESC(struct struct_name, field2, fmt2), \
            FIELD_DESC(struct struct_name, field3, fmt3), \
            TERMINATOR, \
        }, \
        .childs = { \
            children, \
            __VA_ARGS__ \
        }, \
        .ops = NULL, \
        .fields1 = { \
            FIELD_DESC(struct struct_name, field4, fmt4), \
            FIELD_DESC(struct struct_name, field5, fmt5), \
            TERMINATOR, \
        }, \
    }

#define TLV_DESC_6FIELDS(struct_name, _count_type, field1, fmt1, field2, fmt2, field3, fmt3, field4, fmt4, field5, fmt5, \
                                                               field6, fmt6, children, ...) \
    { \
        .name = #struct_name, \
        .size = sizeof(struct struct_name), \
        .count_type = _count_type, \
        .fields = { \
            FIELD_DESC(struct struct_name, field1, fmt1), \
            FIELD_DESC(struct struct_name, field2, fmt2), \
            FIELD_DESC(struct struct_name, field3, fmt3), \
            FIELD_DESC(struct struct_name, field4, fmt4), \
            FIELD_DESC(struct struct_name, field5, fmt5), \
            FIELD_DESC(struct struct_name, field6, fmt6), \
            TERMINATOR, \
        }, \
        .childs = { \
            children, \
            __VA_ARGS__ \
        }, \
        .ops = NULL, \
        EXTRA_FIELDS \
    }

#define TLV_DESC_6_1FIELDS(struct_name, _count_type, field1, fmt1, field2, fmt2, field3, fmt3, field4, fmt4, field5, fmt5, \
                                                               field6, fmt6, field7, fmt7, children, ...) \
    { \
        .name = #struct_name, \
        .size = sizeof(struct struct_name), \
        .count_type = _count_type, \
        .fields = { \
            FIELD_DESC(struct struct_name, field1, fmt1), \
            FIELD_DESC(struct struct_name, field2, fmt2), \
            FIELD_DESC(struct struct_name, field3, fmt3), \
            FIELD_DESC(struct struct_name, field4, fmt4), \
            FIELD_DESC(struct struct_name, field5, fmt5), \
            FIELD_DESC(struct struct_name, field6, fmt6), \
            TERMINATOR, \
        }, \
        .childs = { \
            children, \
            __VA_ARGS__ \
        }, \
        .ops = NULL, \
        .fields1 = { \
            FIELD_DESC(struct struct_name, field7, fmt7), \
            TERMINATOR, \
        }, \
    }

#define TLV_DESC_7FIELDS(struct_name, _count_type, field1, fmt1, field2, fmt2, field3, fmt3, field4, fmt4,\
                           field5, fmt5, field6, fmt6, field7, fmt7, children, ...) \
    { \
       .name = #struct_name, \
       .size = sizeof(struct struct_name), \
       .count_type = _count_type, \
       .fields = { \
           FIELD_DESC(struct struct_name, field1, fmt1), \
           FIELD_DESC(struct struct_name, field2, fmt2), \
           FIELD_DESC(struct struct_name, field3, fmt3), \
           FIELD_DESC(struct struct_name, field4, fmt4), \
           FIELD_DESC(struct struct_name, field5, fmt5), \
           FIELD_DESC(struct struct_name, field6, fmt6), \
           FIELD_DESC(struct struct_name, field7, fmt7), \
           TERMINATOR, \
       }, \
       .childs = { \
           children, \
           __VA_ARGS__ \
       }, \
       .ops = NULL, \
       EXTRA_FIELDS \
    }


#define TLV_DESC_8FIELDS(struct_name, _count_type, field1, fmt1, field2, fmt2, field3, fmt3, field4, fmt4,\
                                field5, fmt5, field6, fmt6, field7, fmt7, field8, fmt8, children, ...) \
    { \
        .name = #struct_name, \
        .size = sizeof(struct struct_name), \
        .count_type = _count_type, \
        .fields = { \
            FIELD_DESC(struct struct_name, field1, fmt1), \
            FIELD_DESC(struct struct_name, field2, fmt2), \
            FIELD_DESC(struct struct_name, field3, fmt3), \
            FIELD_DESC(struct struct_name, field4, fmt4), \
            FIELD_DESC(struct struct_name, field5, fmt5), \
            FIELD_DESC(struct struct_name, field6, fmt6), \
            FIELD_DESC(struct struct_name, field7, fmt7), \
            FIELD_DESC(struct struct_name, field8, fmt8), \
            TERMINATOR, \
        }, \
        .childs = { \
            children, \
            __VA_ARGS__ \
        }, \
        .ops = NULL, \
        EXTRA_FIELDS \
    }

#define TLV_DESC_9FIELDS(struct_name, _count_type, field1, fmt1, field2, fmt2, field3, fmt3, field4, fmt4,\
                                field5, fmt5, field6, fmt6, field7, fmt7, field8, fmt8, field9, fmt9, children, ...) \
    { \
        .name = #struct_name, \
        .size = sizeof(struct struct_name), \
        .count_type = _count_type, \
        .fields = { \
            FIELD_DESC(struct struct_name, field1, fmt1), \
            FIELD_DESC(struct struct_name, field2, fmt2), \
            FIELD_DESC(struct struct_name, field3, fmt3), \
            FIELD_DESC(struct struct_name, field4, fmt4), \
            FIELD_DESC(struct struct_name, field5, fmt5), \
            FIELD_DESC(struct struct_name, field6, fmt6), \
            FIELD_DESC(struct struct_name, field7, fmt7), \
            FIELD_DESC(struct struct_name, field8, fmt8), \
            FIELD_DESC(struct struct_name, field9, fmt9), \
            TERMINATOR, \
        }, \
        .childs = { \
            children, \
            __VA_ARGS__ \
        }, \
        .ops = NULL, \
        EXTRA_FIELDS \
    }

#define TLV_DESC_11FIELDS(struct_name, _count_type, field1, fmt1, field2, fmt2, field3, fmt3, field4, fmt4,\
                                field5, fmt5, field6, fmt6, field7, fmt7, field8, fmt8, field9, fmt9, field10, fmt10,\
                                field11, fmt11, children, ...) \
    { \
        .name = #struct_name, \
        .size = sizeof(struct struct_name), \
        .count_type = _count_type, \
        .fields = { \
            FIELD_DESC(struct struct_name, field1, fmt1), \
            FIELD_DESC(struct struct_name, field2, fmt2), \
            FIELD_DESC(struct struct_name, field3, fmt3), \
            FIELD_DESC(struct struct_name, field4, fmt4), \
            FIELD_DESC(struct struct_name, field5, fmt5), \
            FIELD_DESC(struct struct_name, field6, fmt6), \
            FIELD_DESC(struct struct_name, field7, fmt7), \
            FIELD_DESC(struct struct_name, field8, fmt8), \
            FIELD_DESC(struct struct_name, field9, fmt9), \
            FIELD_DESC(struct struct_name, field10, fmt10), \
            FIELD_DESC(struct struct_name, field11, fmt11), \
            TERMINATOR, \
        }, \
        .childs = { \
            children, \
            __VA_ARGS__ \
        }, \
        .ops = NULL, \
        EXTRA_FIELDS \
    }

#define TLV_DESC_OPS(struct_name, _ops) \
    { \
        .name = #struct_name, \
        .size = sizeof(struct struct_name), \
        .ops = _ops, \
    }


// In the comments below, every time a reference is made (ex: "See Section 6.4"
// or "See Table 6-11") we are talking about the contents of the following
// document:
//
//   "IEEE Std 1905.1-2013"

////////////////////////////////////////////////////////////////////////////////
// TLV types as detailed in "Section 6.4"
////////////////////////////////////////////////////////////////////////////////
enum e_tlv_type {
	TLV_TYPE_END_OF_MESSAGE                       = 0,
	TLV_TYPE_AL_MAC_ADDRESS_TYPE                  = 1,
	TLV_TYPE_MAC_ADDRESS_TYPE                     = 2,
	TLV_TYPE_DEVICE_INFORMATION_TYPE              = 3,
	TLV_TYPE_DEVICE_BRIDGING_CAPABILITIES         = 4,
	TLV_TYPE_NON_1905_NEIGHBOR_DEVICE_LIST        = 6,
	TLV_TYPE_NEIGHBOR_DEVICE_LIST                 = 7,
	TLV_TYPE_LINK_METRIC_QUERY                    = 8,
	TLV_TYPE_TRANSMITTER_LINK_METRIC              = 9,
	TLV_TYPE_RECEIVER_LINK_METRIC                 = 10,
	TLV_TYPE_VENDOR_SPECIFIC                      = 11,
	TLV_TYPE_LINK_METRIC_RESULT_CODE              = 12,
	TLV_TYPE_SEARCHED_ROLE                        = 13,
	TLV_TYPE_AUTOCONFIG_FREQ_BAND                 = 14,
	TLV_TYPE_SUPPORTED_ROLE                       = 15,
	TLV_TYPE_SUPPORTED_FREQ_BAND                  = 16,
	TLV_TYPE_WSC                                  = 17,
	TLV_TYPE_PUSH_BUTTON_EVENT_NOTIFICATION       = 18,
	TLV_TYPE_PUSH_BUTTON_JOIN_NOTIFICATION        = 19,
	TLV_TYPE_GENERIC_PHY_DEVICE_INFORMATION       = 20,
	TLV_TYPE_DEVICE_IDENTIFICATION                = 21,
	TLV_TYPE_CONTROL_URL                          = 22,
	TLV_TYPE_IPV4                                 = 23,
	TLV_TYPE_IPV6                                 = 24,
	TLV_TYPE_GENERIC_PHY_EVENT_NOTIFICATION       = 25,
	TLV_TYPE_1905_PROFILE_VERSION                 = 26,
	TLV_TYPE_POWER_OFF_INTERFACE                  = 27,
	TLV_TYPE_INTERFACE_POWER_CHANGE_INFORMATION   = 28,
	TLV_TYPE_INTERFACE_POWER_CHANGE_STATUS        = 29,
	TLV_TYPE_L2_NEIGHBOR_DEVICE                   = 30,

/** @brief Multi-AP TLV types as detailed in tables 6 to 41 of "Multi-AP_Specification_v1.0"
 *
 * @{
 */

	TLV_TYPE_SUPPORTED_SERVICE                    = 0x80,
	TLV_TYPE_SEARCHED_SERVICE                     = 0x81,
	TLV_TYPE_AP_RADIO_IDENTIFIER                  = 0x82,
	TLV_TYPE_AP_OPERATIONAL_BSS                   = 0x83,
	TLV_TYPE_ASSOCIATED_CLIENTS                   = 0x84,
	TLV_TYPE_AP_RADIO_BASIC_CAPABILITIES          = 0x85,
	TLV_TYPE_AP_HT_CAPABILITIES                   = 0x86,
	TLV_TYPE_AP_VHT_CAPABILITIES                  = 0x87,
	TLV_TYPE_AP_HE_CAPABILITIES                   = 0x88,
	TLV_TYPE_STEERING_POLICY                      = 0x89,
	TLV_TYPE_METRIC_REPORTING_POLICY              = 0x8A,
	TLV_TYPE_CHANNEL_PREFERENCE                   = 0x8B,
	TLV_TYPE_RADIO_OPERATION_RESTRICTION          = 0x8C,
	TLV_TYPE_TRANSMIT_POWER_LIMIT                 = 0x8D,
	TLV_TYPE_CHANNEL_SELECTION_RESPONSE           = 0x8E,
	TLV_TYPE_OPERATING_CHANNEL_REPORT             = 0x8F,
	TLV_TYPE_CLIENT_INFO                          = 0x90,
	TLV_TYPE_CLIENT_CAPABILITY_REPORT             = 0x91,
	TLV_TYPE_CLIENT_ASSOCIATION_EVENT             = 0x92,
	TLV_TYPE_AP_METRIC_QUERY                      = 0x93,
	TLV_TYPE_AP_METRICS                           = 0x94,
	TLV_TYPE_STA_MAC_ADDRESS                      = 0x95,
	TLV_TYPE_ASSOCIATED_STA_LINK_METRICS          = 0x96,
	TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_QUERY  = 0x97,
	TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_RESPONSE  = 0x98,
	TLV_TYPE_BEACON_METRICS_QUERY                 = 0x99,
	TLV_TYPE_BEACON_METRICS_RESPONSE              = 0x9A,
	TLV_TYPE_STEERING_REQUEST                     = 0x9B,
	TLV_TYPE_STEERING_BTM_REPORT                  = 0x9C,
	TLV_TYPE_CLIENT_ASSOCIATION_CONTROL_REQUEST   = 0x9D,
	TLV_TYPE_BACKHAUL_STEERING_REQUEST            = 0x9E,
	TLV_TYPE_BACKHAUL_STEERING_RESPONSE           = 0x9F,
	TLV_TYPE_HIGHER_LAYER_DATA                    = 0xA0,
	TLV_TYPE_AP_CAPABILITY                        = 0xA1,
	TLV_TYPE_ASSOCIATED_STA_TRAFFIC_STATS         = 0xA2,
	TLV_TYPE_ERROR_CODE                           = 0xA3,
/**_@}_*/

/** @brief Multi-AP TLV types as detailed in tables 47 to 76 of "Multi-AP_Specification_v2.0"
 *
 * @{
 */

	TLV_TYPE_CHANNEL_SCAN_REPORTING_POLOCY        = 0xA4,
	TLV_TYPE_CHANNEL_SCAN_CAPABILITIES            = 0xA5,
	TLV_TYPE_CHANNEL_SCAN_REQUEST                 = 0xA6,
	TLV_TYPE_CHANNEL_SCAN_RESULT                  = 0xA7,
	TLV_TYPE_TIMESTAMP                            = 0xA8,
	TLV_TYPE_CAC_REQUEST                          = 0xAD,
	TLV_TYPE_CAC_TERMINATION                      = 0xAE,
	TLV_TYPE_CAC_COMPLETION_REPORT                = 0xAF,
	TLV_TYPE_CAC_STATUS_REPORT                    = 0xB1,
	TLV_TYPE_CAC_CAPABILITIES                     = 0xB2,
	TLV_TYPE_MULTIAP_PROFILE                      = 0xB3,
	TLV_TYPE_PROFILE2_AP_CAPABILITY               = 0xB4,
	TLV_TYPE_DEFAULT_8021Q_SETTINGS               = 0xB5,
	TLV_TYPE_TRAFFIC_SEPARATION_POLICY            = 0xB6,
	TLV_TYPE_PROFILE2_ERROR_CODE                  = 0xBC,
	TLV_TYPE_AP_RADIO_ADVANCED_CAPABILITIES       = 0xBE,
	TLV_TYPE_ASSOCIATION_STATUS_NOTIFICATION      = 0xBF,
	TLV_TYPE_SOURCE_INFO                          = 0xC0,
	TLV_TYPE_TUNNELED_MESSAGE_TYPE                = 0xC1,
	TLV_TYPE_TUNNELED                             = 0xC2,
	TLV_TYPE_PROFILE2_STEERING_REQUEST            = 0xC3,
	TLV_TYPE_UNSUCCESSFUL_ASSOCIATION_POLICY      = 0xC4,
	TLV_TYPE_METRIC_COLLECTION_INTERVAL           = 0xC5,
	TLV_TYPE_RADIO_METRICS                        = 0xC6,
	TLV_TYPE_AP_EXTENDED_METRICS                  = 0xC7,
	TLV_TYPE_ASSOCIATED_STA_EXTENDED_LINK_METRICS  = 0xC8,
	TLV_TYPE_STATUS_CODE                          = 0xC9,
	TLV_TYPE_REASON_CODE                          = 0xCA,
	TLV_TYPE_BACKHAUL_STA_RADIO_CAPABILITIES      = 0xCB,
	TLV_TYPE_BACKHAUL_BSS_CONFIGURATION           = 0xD0,
/**_@}_*/

/** @brief EasyMesh TLV types as detailed in tables 85 to 103 of "Wi-Fi_EasyMesh_Specification_v3"
 *
 * @{
 */

	TLV_TYPE_1905_LAYER_SECURITY_CAPABILITY       = 0xA9,
	TLV_TYPE_AP_WIFI6_CAPABILITIES                = 0xAA,
	TLV_TYPE_MIC                                  = 0xAB,
	TLV_TYPE_ENCRYPTED_PAYLOAD                    = 0xAC,
	TLV_TYPE_ASSOCIATED_WIFI6_STA_STATUS_REPORT   = 0xB0,
	TLV_TYPE_BSS_CONFIGURATION_REPORT             = 0xB7,
	TLV_TYPE_BSSID                                = 0xB8,
	TLV_TYPE_SERVICE_PRIORITIZATION_RULE          = 0xB9,
	TLV_TYPE_DSCP_MAPPING_TABLE                   = 0xBA,
	TLV_TYPE_BSS_CONFIGURATION_REQUEST            = 0xBB,
	TLV_TYPE_BSS_CONFIGURATION_RESPONSE           = 0xBD,
	TLV_TYPE_AKM_SUITE_CAPABILITIES               = 0xCC,
	TLV_TYPE_1905_ENCAP_DPP                       = 0xCD,
	TLV_TYPE_1905_ENCAP_EAPOL                     = 0xCE,
	TLV_TYPE_DPP_BOOTSTRAPPING_URI_NOTIFICATION   = 0xCF,
	TLV_TYPE_DPP_MESSAGE                          = 0xD1,
	TLV_TYPE_DPP_CCE_INDICATION                   = 0xD2,
	TLV_TYPE_DPP_CHIRP_VALUE                      = 0xD3,
	TLV_TYPE_DEVICE_INVENTORY                     = 0xD4,
	TLV_TYPE_AGENT_LIST                           = 0xD5,

	TLV_TYPE_ANTICIPATED_CHANNEL_PREFERENCE	      = 0xD6,
	TLV_TYPE_ANTICIPATED_CHANNEL_USAGE            = 0xD7,
	TLV_TYPE_SPATIAL_REUSE_REQUEST                = 0xD8,
	TLV_TYPE_SPATIAL_REUSE_REPORT                 = 0xD9,
	TLV_TYPE_SPATIAL_REUSE_CONFIG_RESPONSE        = 0xDA,
	TLV_TYPE_QOS_MANAGEMENT_POLICY                = 0xDB,
	TLV_TYPE_QOS_MANAGEMENT_DESCRIPTOR            = 0xDC,
	TLV_TYPE_CONTROLLER_CAPABILITY                = 0xDD,

	TLV_TYPE_VBSS                                 = 0xDE,

    //add new tlv type before

    TLV_TYPE_MAX,
};


enum e_tlv_sub_type_vbss {
    TLV_SUB_TYPE_VBSS_AP_RADIO_CAPABILITIES = 0x0001,
    TLV_SUB_TYPE_VBSS_CREATION              = 0x0002,
    TLV_SUB_TYPE_VBSS_DESTRUCTION           = 0x0003,
    TLV_SUB_TYPE_VBSS_EVENT                 = 0x0004,
    TLV_SUB_TYPE_CLIENT_SECURITY_CONTEXT    = 0x0005,
    TLV_SUB_TYPE_TRIGGER_CSA                = 0x0006,
    TLV_SUB_TYPE_VBSS_CONFIGURATION_REPORT  = 0x0007,
    TLV_SUB_TYPE_VBSS_MAX
};

////////////////////////////////////////////////////////////////////////////////
// Media types as detailed in "Table 6-12"
////////////////////////////////////////////////////////////////////////////////
#define MEDIA_TYPE_IEEE_802_3U_FAST_ETHERNET       (0x0000)
#define MEDIA_TYPE_IEEE_802_3AB_GIGABIT_ETHERNET   (0x0001)
#define MEDIA_TYPE_IEEE_802_11B_2_4_GHZ            (0x0100)
#define MEDIA_TYPE_IEEE_802_11G_2_4_GHZ            (0x0101)
#define MEDIA_TYPE_IEEE_802_11A_5_GHZ              (0x0102)
#define MEDIA_TYPE_IEEE_802_11N_2_4_GHZ            (0x0103)
#define MEDIA_TYPE_IEEE_802_11N_5_GHZ              (0x0104)
#define MEDIA_TYPE_IEEE_802_11AC_5_GHZ             (0x0105)
#define MEDIA_TYPE_IEEE_802_11AD_60_GHZ            (0x0106)
#define MEDIA_TYPE_IEEE_802_11AF_GHZ               (0x0107)
#define MEDIA_TYPE_IEEE_802_11AX                   (0x0108)
#define MEDIA_TYPE_IEEE_1901_WAVELET               (0x0200)
#define MEDIA_TYPE_IEEE_1901_FFT                   (0x0201)
#define MEDIA_TYPE_MOCA_V1_1                       (0x0300)
#define MEDIA_TYPE_UNKNOWN                         (0xFFFF)


////////////////////////////////////////////////////////////////////////////////
// IEEE802.11 frequency bands used in "Tables 6-22 and 6-24"
////////////////////////////////////////////////////////////////////////////////
#define IEEE80211_ROLE_REGISTRAR                   (0x00)


////////////////////////////////////////////////////////////////////////////////
// IEEE802.11 frequency bands used in "Tables 6-23 and 6-25"
////////////////////////////////////////////////////////////////////////////////
#define IEEE80211_FREQUENCY_BAND_2_4_GHZ           (0x00)
#define IEEE80211_FREQUENCY_BAND_5_GHZ             (0x01)
#define IEEE80211_FREQUENCY_BAND_60_GHZ            (0x02)


enum service {
    e_controller = 0x00,
    e_agent = 0x01,
};

#define IEEE80211_SPECIFIC_INFO_ROLE_AP                   (0x0)
#define IEEE80211_SPECIFIC_INFO_ROLE_NON_AP_NON_PCP_STA   (0x4)
#define IEEE80211_SPECIFIC_INFO_ROLE_WIFI_P2P_CLIENT      (0x8)
#define IEEE80211_SPECIFIC_INFO_ROLE_WIFI_P2P_GROUP_OWNER (0x9)
#define IEEE80211_SPECIFIC_INFO_ROLE_AD_PCP               (0xa)

////////////////////////////////////////////////////////////////////////////////
// Generic phy common structure used in "Tables 6.29, 6.36 and 6.38"
////////////////////////////////////////////////////////////////////////////////
struct _genericPhyCommonData
{
    uint8_t   oui[3];                   // OUI of the generic phy networking
                                      // technology of the local interface

    uint8_t   variant_index;            // Variant index of the generic phy
                                      // networking technology of the local
                                      // interface

    uint8_t   media_specific_bytes_nr;
    uint8_t  *media_specific_bytes;     // Media specific information of the
                                      // variant.
                                      // This field contains
                                      // "media_specific_bytes_nr" bytes.
};



////////////////////////////////////////////////////////////////////////////////
// Link metric query TLV associated structures ("Section 6.4.10")
////////////////////////////////////////////////////////////////////////////////
struct linkMetricQueryTLV
{
    struct TLV tlv;                                  /**< @brief TLV type, must always be set to TLV_TYPE_LINK_METRIC_QUERY. */
    #define LINK_METRIC_QUERY_TLV_ALL_NEIGHBORS      (0x00)
    #define LINK_METRIC_QUERY_TLV_SPECIFIC_NEIGHBOR  (0x01)
    uint8_t neighbor;                                /**< @brief neighbor type: special neighbor or all neighbors. */
    uint8_t specific_neighbor[6];                    /**< @brief Only significant when 'neighbor' field is set to
                                                       * 'LINK_METRIC_QUERY_TLV_SPECIFIC_NEIGHBOR'. */
    #define TX_LINK_METRICS_ONLY                     (0x00)
    #define RX_LINK_METRICS_ONLY                     (0x01)
    #define TX_AND_RX_LINK_METRICS                   (0x02)
    uint8_t link_metrics_type;                      /**< @brief link metric type of above value. */
};

////////////////////////////////////////////////////////////////////////////////
// Transmitter link metric TLV associated structures ("Section 6.4.11")
////////////////////////////////////////////////////////////////////////////////
struct transmitterLinkMetricEntriesStruct
{
    struct TLVStruct s;
    uint8_t local_interface_address[6];    /**< @brief MAC address of an interface in the receiving AL,
                                             * which connects to an interface in the neighbor AL */
    uint8_t neighbor_interface_address[6]; /**< @brief  MAC addres of an interface in a neighbor AL,
                                             * which connects to an interface in the receiving AL. */
    uint16_t intf_type;                    /**< @brief Underlaying network technology.
                                             * One of the MEDIA_TYPE_* values. */
    uint8_t bridge_flag;                   /**< @brief Indicates whether or not the 1905 link includes one or more IEEE 802.11 bridges. */
    struct tx_linkMetrics tx_link_metrics;
};
struct transmitterLinkMetricTLV
{
    struct TLV tlv;                  /**< @brief TLV type, TLV_TYPE_TRANSMITTER_LINK_METRIC. */
    uint8_t local_al_address[6];     /**< @brief AL MAC address of the device that transmits the response message that contains this TLV. */
    uint8_t neighbor_al_address[6];  /**< @brief AL MAC address of the neighbor whose link metric is reported in this TLV. */
};
struct transmitterLinkMetricEntriesStruct *trasmitterLinkMetricAddEntry(struct TLV *tlv, struct transmitterLinkMetricEntriesStruct *entry);

////////////////////////////////////////////////////////////////////////////////
// Receiver link metric TLV associated structures ("Section 6.4.12")
////////////////////////////////////////////////////////////////////////////////
struct  receiverLinkMetricEntriesStruct
{
    struct TLVStruct s;
    uint8_t local_interface_address[6];    /**< @brief MAC address of an interface in the receiving AL,
                                             * which connects to an interface in the neighbor AL. */
    uint8_t neighbor_interface_address[6]; /**< @brief MAC addres of an interface in a neighbor AL,
                                             * which connects to an interface in the receiving AL. */
    uint16_t intf_type;                    /**< @brief Underlaying network technology. */
    struct rx_linkMetrics rx_link_metrics;
};
struct receiverLinkMetricTLV
{
    struct TLV tlv;                  /**< @brief TLV type, TLV_TYPE_RECEIVER_LINK_METRIC. */
    uint8_t local_al_address[6];     /**< @brief AL MAC address of the device that transmits the response message that contains this TLV. */
    uint8_t neighbor_al_address[6];  /**< @brief AL MAC address of the neighbor whose link metric is reported in this TLV. */
};
struct receiverLinkMetricEntriesStruct *receiverLinkMetricAddEntry(struct TLV *tlv, struct receiverLinkMetricEntriesStruct *entry);


////////////////////////////////////////////////////////////////////////////////
// Generic PHY device information TLV associated structures ("Section 6.4.21")
////////////////////////////////////////////////////////////////////////////////
struct _genericPhyDeviceEntries
{
    uint8_t   local_interface_address[6];
                                      // MAC address of the local interface

    struct _genericPhyCommonData generic_phy_common_data;
                                      // This structure contains the OUI,
                                      // variant index and media specific
                                      // information of the local interface

    uint8_t   variant_name[32];         // Variant name UTF-8 string (NULL
                                      // terminated)

    uint8_t   generic_phy_description_xml_url_len;
    char   *generic_phy_description_xml_url;
                                      // URL to the "Generic Phy XML Description
                                      // Document" of the variant.
                                      // The string is
                                      // 'generic_phy_description_xml_url_len'
                                      // bytes long including the final NULL
                                      // character.

};


// IPv4 type TLV associated structures ("Section 6.4.24")
////////////////////////////////////////////////////////////////////////////////
struct _ipv4Entries
{
    #define IPV4_PROTO_UNKNOWN (0)
    #define IPV4_PROTO_DHCP    (1)
    #define IPV4_PROTO_STATIC  (2)
    #define IPV4_PROTO_AUTOIP  (3)
    uint8_t type;                     // One of the values from above

    uint8_t ipv4_address[4];          // IPv4 address associated to the interface

    uint8_t ipv4_dhcp_server[4];      // IPv4 address of the DHCP server (if
                                    // known, otherwise set to all zeros)
};
struct _ipv4InterfaceEntries
{
    uint8_t   mac_address[6];          // MAC address of the interface whose IPv4s
                                     // are going to be reported.
                                     //
                                     //   NOTE: The standard says it can also
                                     //   be an AL MAC address instead of an
                                     //   interface MAC address.
                                     //   In that case I guess *all* IPv4s of
                                     //   the device (no matter the interface
                                     //   they are "binded" to) are reported.

    uint8_t                   ipv4_nr;
    struct  _ipv4Entries   *ipv4;    // List of IPv4s associated to this
                                     // interface
};

////////////////////////////////////////////////////////////////////////////////
// IPv6 type TLV associated structures ("Section 6.4.25")
////////////////////////////////////////////////////////////////////////////////
struct _ipv6Entries
{
    #define IPV6_TYPE_UNKNOWN (0)
    #define IPV6_TYPE_DHCP    (1)
    #define IPV6_TYPE_STATIC  (2)
    #define IPV6_TYPE_SLAAC   (3)
    uint8_t type;                     // One of the values from above

    uint8_t ipv6_address[16];         // IPv6 address associated to the interface

    uint8_t ipv6_address_origin[16];  // If type == IPV6_TYPE_DHCP, this field
                                    // contains the IPv6 address of the DHCPv6
                                    // server.
                                    // If type == IPV6_TYPE_SLAAC, this field
                                    // contains the IPv6 address of the router
                                    // that provided the SLAAC address.
                                    // In any other case this field is set to
                                    // all zeros.
};
struct _ipv6InterfaceEntries
{
    uint8_t   mac_address[6];          // MAC address of the interface whose IPv4s
                                     // are going to be reported.
                                     //
                                     //   NOTE: The standard says it can also
                                     //   be an AL MAC address instead of an
                                     //   interface MAC address.
                                     //   In that case I guess *all* IPv4s of
                                     //   the device (no matter the interface
                                     //   they are "binded" to) are reported.

    uint8_t  ipv6_link_local_address[16];
                                     // IPv6 link local address corresponding to
                                     // this interface

    uint8_t                   ipv6_nr;
    struct  _ipv6Entries   *ipv6;    // List of IPv4s associated to this
                                     // interface
};


////////////////////////////////////////////////////////////////////////////////
// Power off interface TLV associated structures ("Section 6.4.28")
////////////////////////////////////////////////////////////////////////////////
struct _powerOffInterfaceEntries
{
    uint8_t   interface_address[6];     // MAC address of an interface in the
                                      // "power off" state
                                             
    uint16_t  media_type;               // Underlaying network technology
                                      // One of the MEDIA_TYPE_* values

    struct _genericPhyCommonData generic_phy_common_data;
                                      // If 'media_type' is MEDIA_TYPE_UNKNOWN,
                                      // this structure contains the vendor OUI,
                                      // variant index and media specific
                                      // information of the interface
                                      // Otherwise, it is set to all zeros
};

////////////////////////////////////////////////////////////////////////////////
// Interface power change information TLV associated structures ("Section
// 6.4.29")
////////////////////////////////////////////////////////////////////////////////
struct _powerChangeInformationEntries
{
    uint8_t   interface_address[6];     // MAC address of an interface in the
                                      // "power off" state
                                             
    #define POWER_STATE_REQUEST_OFF  (0x00)
    #define POWER_STATE_REQUEST_ON   (0x01)
    #define POWER_STATE_REQUEST_SAVE (0x02)
    uint8_t   requested_power_state;    // One of the values from above
};

////////////////////////////////////////////////////////////////////////////////
// Interface power change status TLV associated structures ("Section 6.4.29")
////////////////////////////////////////////////////////////////////////////////
struct _powerChangeStatusEntries
{
    uint8_t   interface_address[6];     // MAC address of an interface in the
                                      // "power off" state
                                             
    #define POWER_STATE_RESULT_COMPLETED          (0x00)
    #define POWER_STATE_RESULT_NO_CHANGE          (0x01)
    #define POWER_STATE_RESULT_ALTERNATIVE_CHANGE (0x02)
    uint8_t   result;                   // One of the values from above
};




////////////////////////////////////////////////////////////////////////////////
// Main API functions
////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////
// Utility API functions
////////////////////////////////////////////////////////////////////////////////

char *convert_1905_TLV_type_to_string(uint8_t tlv_type);

////////////////////////////////////////////////////////////////////////////////
//New TLV type defines
////////////////////////////////////////////////////////////////////////////////
struct u8TLV {
    struct TLV tlv;
    uint8_t v1;
};

struct macAddressTLV {
    struct TLV tlv;
    mac_address mac;
};

struct macStruct {
    struct TLVStruct s;
    mac_address mac;
};

struct ipv4AddrStruct {
    struct TLVStruct s;
    uint8_t proto;
    struct ipv4 ip;
    struct ipv4 dhcp_ip;
};

struct ipv6AddrStruct {
    struct TLVStruct s;
    uint8_t proto;
    struct ipv6 ip;
    struct ipv6 origin_ip;
};

struct interfaceipv6Struct {
    struct TLVStruct s;
    mac_address mac;
    struct ipv6 local_ip;
};

struct i1905NeighborStruct {
    struct TLVStruct s;
    mac_address mac;
    uint8_t bridge;
};

struct deviceIDTLV {
    struct TLV tlv;
    char friendly_name[64];
    char padding[2];
    char manufacturer[64];
    char padding1[2];
    char model_name[64];
    char padding2[2];
};

struct serviceStruct {
    struct TLVStruct s;
    uint8_t service;
};

struct wscTLV {
    struct TLV tlv;
    struct vvData wsc;
};

struct codeTLV {
    struct TLV tlv;
    uint16_t code;
};

struct radioBasicCapabilityTLV {
    struct TLV tlv;
    mac_address rid;
    uint8_t max_bss;
};

struct htCapabilityTLV {
    struct TLV tlv;
    mac_address rid;
#define HT_CAPA_TXSS_SHIFT      (6)
#define HT_CAPA_TXSS_MASK       (0x03)
#define HT_CAPA_RXSS_SHIFT      (4)
#define HT_CAPA_RXSS_MASK       (0x03)
#define HT_CAPA_SGI_20M         BIT(3)
#define HT_CAPA_SGI_40M         BIT(2)
#define HT_CAPA_BW_40M          BIT(1)
    struct ht_capability capa;
};

struct vhtCapabilityTLV {
    struct TLV tlv;
    mac_address rid;
#define VHT_CAPA_TXSS_SHIFT     (5)
#define VHT_CAPA_TXSS_MASK      (0x07)
#define VHT_CAPA_RXSS_SHIFT     (2)
#define VHT_CAPA_RXSS_MASK      (0x07)
#define VHT_CAPA_SGI_80M        BIT(1)
#define VHT_CAPA_SGI_160M       BIT(0)
#define VHT_CAPA_BW_8080M       BIT(7)
#define VHT_CAPA_BW_160M        BIT(6)
#define VHT_CAPA_SU_BF          BIT(5)
#define VHT_CAPA_MU_BF          BIT(4)
    struct vht_capability capa;
};


struct heCapabilityTLV {
    struct TLV  tlv;
    mac_address rid;
    struct he_capability capa;
#define HE_CAPA_TXSS_SHIFT      (5)
#define HE_CAPA_TXSS_MASK       (0x07)
#define HE_CAPA_RXSS_SHIFT      (2)
#define HE_CAPA_RXSS_MASK       (0x07)
#define HE_CAPA_BW_8080M        BIT(1)
#define HE_CAPA_BW_160M         BIT(0)
#define HE_CAPA_SU_BF           BIT(7)
#define HE_CAPA_MU_BF           BIT(6)
#define HE_CAPA_UL_MUMIMO       BIT(5)
#define HE_CAPA_UL_MUMIMO_OFDMA BIT(4)
#define HE_CAPA_DL_MUMIMO_OFDMA BIT(3)
#define HE_CAPA_UL_OFDMA        BIT(2)
#define HE_CAPA_DL_OFDMA        BIT(1)
};

struct profile2ApCapabilityTLV {
    struct TLV tlv;
    uint8_t max_prio_rules;
#define P2_CAP_PRIORITIZATION   BIT(5)
#define P2_CAP_DPP              BIT(4)
#define P2_CAP_TRAFFIC_SEPARATION BIT(3)
#define P2_CAP_BYTE_COUNTER_UNIT_OFFSET (6)
    uint8_t cap;
    uint8_t max_vid;
    uint8_t reserved;
};

struct txOpclassStruct {
    struct TLVStruct s;
    uint8_t opclass;
    uint8_t max_tx_power;
    uint8_t non_op_chans[1+MAX_CHANNEL_PER_OPCLASS];
};

struct apOperationBSSStruct {
    struct TLVStruct s;
    mac_address bssid;
    struct ssid ssid;
};

struct associatedBSSStruct {
    struct TLVStruct s;
    mac_address bssid;
};

struct associatedClientStruct {
    struct TLVStruct s;
    mac_address mac;
    uint16_t age;
};

struct chanStruct {
    struct TLVStruct s;
    uint8_t chan;
};

struct cacCapaOpclassStruct {
    struct TLVStruct s;
    uint8_t opclass;
    uint8_t chan[MAX_CHANNEL_PER_OPCLASS+1];
};

struct opClassChanStruct {
    struct TLVStruct s;
    uint8_t opclass;
    struct l1vData chan;
};

#define MAX_MEDIA_SPECIFIC_INFO_LEN (10)
struct interfaceStruct {
    struct TLVStruct s;
    mac_address mac;
#define MEDIA_TYPE_MAIN_MASK        (0xff00)
    uint16_t media_type;
    uint8_t media_specific_info[MAX_MEDIA_SPECIFIC_INFO_LEN+1];
};


struct steerPolicyStruct {
    struct TLVStruct s;
    mac_address rid;
#define POLICY_AGENT_DISALLOWED        0x00
#define POLICY_AGENT_RCPI_MANDATED     0x01
#define POLICY_AGENT_RCPI_ALLOWED      0x02
    uint8_t policy;
    uint8_t chan_util;
    uint8_t rcpi_thresh;
};

struct macStruct *TLVAddMac(struct dlist_head * parent, mac_address mac);
struct steerPolicyStruct *steeringPolicyTLVAddRadio(struct TLV *tlv,
                                        uint8_t *rid, uint8_t policy, uint8_t chan_util, uint8_t rcpi_threshold);

/****************************** TLV-17.2.12 *****************************/
struct metricRptPolicyStruct {
    struct TLVStruct s;
    mac_address rid;
    uint8_t rcpi_thresh;
    uint8_t rcpi_margin;
    uint8_t chan_util_thresh;
#define RPTPOLICY_INCLUDE_ASSOC_TRAFFIC_STATS       BIT(7)
#define RPTPOLICY_INCLUDE_ASSOC_LINK_METRICS        BIT(6)
#define RPTPOLICY_INCLUDE_ASSOC_WIFI6_STA_STATUS    BIT(5)
    uint8_t policy;
};

struct metricReportPolicyTLV {
    struct TLV  tlv;
    uint8_t interval;
};
struct metricRptPolicyStruct *metricReportPolicyTLVAddRadio(struct TLV *tlv, uint8_t *rid,
                                uint8_t rcpi_threshold, uint8_t rcpi_margin, uint8_t ch_util_threshold, uint8_t policy);

/****************************** TLV-17.2.13 *****************************/
struct opClassStruct {
    struct TLVStruct s;
    uint8_t opclass;
    uint8_t non_operable_chans[1 + MAX_CHANNEL_PER_OPCLASS];
#define CHAN_PREF_SHIFT         (4)
#define CHAN_PREF_MASK          (0x0f)
#define CHAN_PREF_NONOPERABLE    0
#define CHAN_PREF_HIGHEST        15
#define CHAN_PREF_REASON_SHIFT   (0)
#define CHAN_PREF_REASON_MASK    (0x0f)
#define GET_CHAN_PREF(_value) GET_BITS((_value), 4, 7)
#define GET_CHAN_REASON(_value) GET_BITS((_value), 0, 3)
    uint8_t pref_reason;
};

struct opClassStruct *chanPrefTLVAddOpclass(struct TLV *tlv, uint8_t opclass, uint8_t num_chans, uint8_t *p_chans,
                                                            uint8_t preference, uint8_t reason);

/****************************** TLV-17.2.14 *****************************/
struct opRestChanStruct {
    struct TLVStruct s;
    uint8_t channel;
    uint8_t freq_sep;
};

struct opRestOpclassStruct {
    struct TLVStruct s;
    uint8_t opclass;
};

struct opRestChanStruct *operRestOpcAddChan(struct opRestOpclassStruct *opc, uint8_t channel, uint8_t freq_sep);
struct opRestOpclassStruct *operRestTLVAddOpclass(struct TLV *tlv, uint8_t opclass);

struct txPwrLimitTLV {
    struct TLV  tlv;
    mac_address rid;
    uint8_t tx_pwr;
};

struct chanSelRspTLV {
    struct TLV  tlv;
    mac_address rid;
#define CHAN_SEL_RESPCODE_ACCEPT (0x00)
#define CHAN_SEL_RESPCODE_VIOLATE_CURRENT (0x01)
#define CHAN_SEL_RESPCODE_VIOLATE_RECENT (0x02)
#define CHAN_SEL_RESPCODE_PREVENT (0x03)
    uint8_t code;
};


/****************************** TLV-17.2.17 *****************************/
struct opclassChanPairStruct {
    struct TLVStruct s;
    uint8_t opclass;
    uint8_t channel;
};

struct opChanReportTLV {
    struct TLV  tlv;
    mac_address rid;
    uint8_t tx_power;
};

struct opclassChanPairStruct *opChanReportLVAddPair(struct TLV *tlv, uint8_t opclass, uint8_t channel);

/****************************** TLV-17.2.18 *****************************/
struct clientInfoTLV {
    struct TLV  tlv;
    mac_address bssid;
    mac_address client;
};


struct clientCapabilityReportTLV {
    struct TLV  tlv;
    uint8_t result;
    struct vvData assc_req;
};

#define CAP_REPORT_SUCCESS     (0x00)
#define CAP_REPORT_FAILURE     (0x01)

/****************************** TLV-17.2.20 *****************************/
struct clientAssocEvtTLV {
    struct TLV  tlv;
    mac_address client;
    mac_address bssid;
#define CLIENT_ASSOC_EVT_JOINED   BIT(7)
    uint8_t evt;
};

/****************************** TLV-17.2.22 *****************************/
struct apMetricsTLV {
    struct TLV  tlv;
    mac_address bssid;
    uint8_t chan_util;
    uint16_t clients;
#define ESPI_BE_INCLUDED        (1 << 7)
#define ESPI_BK_INCLUDED        (1 << 6)
#define ESPI_VO_INCLUDED        (1 << 5)
#define ESPI_VI_INCLUDED        (1 << 4)
    uint8_t includes;
#define ESPI_ACCESS_CATEGORY_MASK   0x3
#define ESPI_ACCESS_CATEGORY_SHIFT  0
#define ESPI_DATA_FORMAT_MASK       0x18
#define ESPI_DATA_FORMAT_SHIFT      3
#define ESPI_BA_WINDOW_MASK         0xc0
#define ESPI_BA_WINDOW_SHIFT        6
#define ESPI_FIELD_LEN              3
    uint8_t espi_be[ESPI_FIELD_LEN];
    uint8_t espi_bk[ESPI_FIELD_LEN];
    uint8_t espi_vo[ESPI_FIELD_LEN];
    uint8_t espi_vi[ESPI_FIELD_LEN];
};


/****************************** TLV-17.2.24 *****************************/
struct assocLinkMetricStruct {
    struct TLVStruct s;
    mac_address bssid;
    uint32_t age;
    struct associated_sta_link_metrics metrics;
};

struct assocLinkMetricStruct *assocLinkMetricsTLVAddBssid(struct TLV *tlv, struct client *sta);

/****************************** TLV-17.2.25 *****************************/
struct unassocMetricQryStruct {
    struct TLVStruct s;
    uint8_t channel;
};

struct unassocStaLinkMetricsQryTLV {
    struct TLV  tlv;
    uint8_t opclass;
};

struct macStruct *unassocStaLinkChanAddSta(struct unassocMetricQryStruct *chan, mac_address sta);
struct unassocMetricQryStruct *unassocStaLinkQueryAddChan(struct TLV *tlv, uint8_t channel);

/****************************** TLV-17.2.26 *****************************/
struct unassocMetricRspStruct {
    struct TLVStruct s;
    mac_address sta;
    uint8_t channel;
    uint32_t age;
    uint8_t rcpi_ul;
};

struct unassocStaLinkMetricsRspTLV {
    struct TLV  tlv;
    uint8_t opclass;
};

struct unassocMetricRspStruct *unassocStaLinkRspAddSta(struct TLV *tlv, mac_address sta, uint8_t channel,
                                                                    uint32_t age, uint8_t rcpi_ul);

/****************************** TLV-17.2.27 *****************************/
struct chanRptStruct {
    struct TLVStruct s;
    struct vvData chanreport;
};

struct bcnMetricQueryTLV {
    struct TLV  tlv;
    mac_address sta;
    uint8_t opclass;
    uint8_t channel;
    mac_address bssid;
    uint8_t detail;
    struct ssid ssid;
    struct vvData eids;
};
struct chanRptStruct *bcnMetricsQryAddChRpt(struct TLV *tlv, uint8_t len, uint8_t opclass,
                                                            uint8_t num_chans, uint8_t *channel);

/****************************** TLV-17.2.28 *****************************/
struct measuredElemStruct {
    struct TLVStruct s;
    struct vvData elem;
};

struct bcnMetricRspTLV {
    struct TLV  tlv;
    mac_address sta;
    uint8_t reserved;
};
struct measuredElemStruct *bcnMetricsRspAddElem(struct TLV *tlv, uint8_t* elem, uint16_t elem_len);

/****************************** TLV-17.2.29/57 ***************************/
struct targetBssStruct {
    struct TLVStruct s;
#define TARGET_WILDCARD     "FF:FF:FF:FF:FF:FF"
    mac_address target;
    uint8_t opclass;
    uint8_t channel;
    uint8_t reason;//profile-2 steering request
};

struct steerReqTLV {
    struct TLV  tlv;
    mac_address bssid;
#define STEER_REQ_MODE_MANDATE_SHIFT            7
#define STEER_REQ_MODE_DISASSOC_IMM_SHIFT       6
#define STEER_REQ_MODE_ABRIDGED_SHIFT           5
    uint8_t mode;
    uint16_t window;
    uint16_t disassoc;
};
struct targetBssStruct *steerReqAddTargetBss(struct TLV *tlv, uint8_t *bssid, uint8_t opclass, uint8_t channel);

/****************************** TLV-17.2.30 *****************************/
struct steerBtmReportTLV {
    struct TLV  tlv;
    mac_address bssid;
    mac_address sta;
    uint8_t status;
    struct vvData target;
};

/****************************** TLV-17.2.31 *****************************/
struct clientAssocCtrlReqTLV {
    struct TLV  tlv;
    mac_address bssid;
#define ASSOC_CTRL_BLOCK        (0x00)
#define ASSOC_CTRL_UNBLOCK      (0x01)
#define ASSOC_CTRL_TIMED_BLOCK  (0X02)
    uint8_t ctrl;
    uint16_t valid_period;
};
struct macStruct *clientAssocCtrlReqAddSta(struct TLV *tlv, mac_address sta);

/****************************** TLV-17.2.32 *****************************/
struct bkhSteerReqTLV {
    struct TLV  tlv;
    mac_address sta;
    mac_address target;
    uint8_t opclass;
    uint8_t channel;
};

/****************************** TLV-17.2.33 *****************************/
struct bkhSteerRspTLV {
    struct TLV  tlv;
    mac_address sta;
    mac_address target;
#define BKH_STEER_CODE_SUCCESS     (0x00)
#define BKH_STEER_CODE_FAILURE     (0x01)
    uint8_t result;
};

/****************************** TLV-17.2.34 *****************************/
struct higherLayerDataTLV {
    struct TLV  tlv;
    uint8_t protocol;
    struct vvData payload;
};

/****************************** TLV-17.2.35 *****************************/
struct assocTrafficStatsTLV {
    struct TLV  tlv;
    mac_address sta;
    struct associated_sta_traffic_stats stats;
};

/****************************** TLV-17.2.36 *****************************/
struct errorCodeTLV {
    struct TLV  tlv;
#define EC_STA_ASSOCITED                   (0x1)
#define EC_STA_UNASSOICITE                 (0x2)
#define EC_CLIENT_CAPRPT_FAILED_UNSPEC     (0x3)
#define EC_BKHSTEERREQ_REJECT_CHANNEL      (0x4)
#define EC_BKHSTEERREQ_REJECT_TARGETBSS    (0x5)
#define EC_BKHSTEERREQ_AUTH_ASSOC_REJECT   (0x6)
    uint8_t code;
    mac_address sta;
};

/****************************** TLV-17.2.37 *****************************/
#define REPORT_INDEPENDENT_CHAN_SCAN  (BIT(7))

/****************************** TLV-17.2.38 *****************************/
struct scanCapaRadioStruct {
    struct TLVStruct s;
    mac_address ruid;
#define SCAN_ON_BOOT_ONLY       (1)
#define SCAN_NO_IMPACT          (0x00)
#define SCAN_REDUCE_NUM_SS      (0x01)
#define SCAN_TIME_SLICING       (0x02)
#define SCAN_RADIO_UNAVAILABLE  (0x03)
#define SCAN_ONBOOT_SHIFT       (7)
#define SCAN_ONBOOT_MASK        (0x01)
#define SCAN_IMPACT_SHIFT       (5)
#define SCAN_IMPACT_MASK        (0x03)
    uint8_t onboot_impact;
    uint32_t min_scan_interval;
};
struct opClassChanStruct *scanCapaRadioAddOpclassChan(struct scanCapaRadioStruct *radios, uint8_t opclass,
                                                                  uint8_t chan_num, uint8_t *chan);
struct scanCapaRadioStruct *scanCapaTLVAddRadios(struct TLV *tlv, uint8_t *ruid, uint8_t onboot_impact,
                                                                  uint8_t impact, uint32_t min_scan_interval);

/****************************** TLV-17.2.39 *****************************/
struct scanReqTLV {
    struct TLV tlv;
#define FRESH_SCAN_RESULT   (BIT(7))
    uint8_t fresh_scan;
};
struct opClassChanStruct *scanReqRadioAddOpclassChan(struct macStruct *radios, uint8_t opclass,
                                                                  uint8_t chan_num, uint8_t *chan);
struct macStruct *scanReqTLVAddRadios(struct TLV *tlv, uint8_t *ruid);

/****************************** TLV-17.2.40 *****************************/
struct bssLoadEleStruct {
    struct TLVStruct s;
    uint8_t chann_utilize;
    uint16_t sta_cnt;
};
struct scanNeighborStruct {
    struct TLVStruct s;
    mac_address bssid;
    struct ssid ssid;
    uint8_t signal_strength;
    struct l1vData chanbw;
#define BSS_LOAD_ELE_PRESENT    (BIT(7))
#define BSS_COLOR_MASK          (0x1F)
    uint8_t bss_field;
};
struct scanResultStruct {
    struct TLVStruct s;
    struct l1vData timestamp;
    uint8_t utilization;
    uint8_t noise;
    uint32_t aggre_scan_dur;
#define ACTIVE_SCAN     (BIT(7))
    uint8_t scan_type;
};
struct scanResultTLV {
    struct TLV tlv;
    mac_address ruid;
    uint8_t opclass;
    uint8_t channel;
#define SCAN_STATUS_SUCCESS       (0x0)
#define SCAN_STATUS_NOT_SUPPORT   (0x01)
#define SCAN_STATUS_TOO_SOON      (0x02)
#define SCAN_STATUS_RADIO_BUSY    (0x03)
#define SCAN_STATUS_NOT_COMPLETE  (0x04)
#define SCAN_STATUS_SCAN_ABORT    (0x05)
#define SCAN_STATUS_BOOTSCAN_ONLY (0x06)
#define SCAN_STATUS_INIT          (0xff)
    uint8_t scan_status;
};
struct scanNeighborStruct *scanResultAddNeighbors(struct scanResultStruct *result,struct neighbor_bss *neighbor);
struct scanResultStruct *scanResultTLVAddResult(struct TLV *tlv, uint8_t ts_len, uint8_t *ts, uint8_t utilization,
                                                     uint8_t noise, uint32_t scan_dur, uint8_t type);

/****************************** TLV-17.2.41 *****************************/
struct timeStampTLV {
    struct TLV tlv;
    struct l1vData timeStamp;
};

/****************************** TLV-17.2.42 *****************************/

#define CAC_ACTION_REMAIN_ON_CHAN		(0x0)
#define CAC_ACTION_RETURN_RADIO		(0x01)

#define CAC_METHOD_SHIFT	(5)
#define CAC_METHOD_MASK		(0x07)
#define CAC_ACTION_SHIFT	(3)
#define CAC_ACTION_MASK		(0x03)

struct cacRadioStruct {
    struct TLVStruct s;
    mac_address ruid;
    uint8_t opclass;
    uint8_t channel;
    uint8_t cac_flag;
};
struct cacRadioStruct *cacReqTLVAddRadios(struct TLV *tlv, uint8_t *ruid, uint8_t opclass, uint8_t channel,
                                                  uint8_t cac_method, uint8_t cac_action);

/****************************** TLV-17.2.43 *****************************/
struct cacTermRadioStruct {
    struct TLVStruct s;
    mac_address ruid;
    uint8_t opclass;
    uint8_t channel;
};
struct cacTermRadioStruct *cacTermTLVAddRadios(struct TLV *tlv, uint8_t *ruid, uint8_t opclass, uint8_t channel);

/****************************** TLV-17.2.44 *****************************/
#define CAC_CMPLT_STATUS_SUCCESS			(0x0)
#define CAC_CMPLT_STATUS_RADAR_DETECTED	(0x01)
#define CAC_CMPLT_STATUS_NOT_SUPPORT		(0x02)
#define CAC_CMPLT_STATUS_RADIO_BUSY		(0x03)
#define CAC_CMPLT_STATUS_REQ_INVALID		(0x04)
#define CAC_CMPLT_STATUS_OTHER_ERR		(0x05)

struct opclassChanPairStruct *cacReptRadioAddOpclsChan(struct cacRadioStruct *radio, uint8_t opclass, uint8_t chan);
struct cacRadioStruct *cacReptTLVAddRadios(struct TLV *tlv, uint8_t *ruid, uint8_t opclass, uint8_t channel, uint8_t cac_stat);

/****************************** TLV-17.2.45 *****************************/
struct cacStatChanStruct {
    struct TLVStruct s;
    uint8_t opclass;
    uint8_t channel;
    uint16_t time;
};
struct cacStatCacChanStruct {
    struct TLVStruct s;
    uint8_t opclass;
    uint8_t channel;
    uint8_t cnt_down[3];
};
struct cacStatChanStruct *cacStatTLVAddAvailChan(struct TLV *tlv, uint8_t opclass, uint8_t channel, uint16_t time);
struct cacStatChanStruct *cacStatTLVAddUnoccupyChan(struct TLV *tlv, uint8_t opclass, uint8_t channel, uint16_t dur);
struct cacStatCacChanStruct *cacStatTLVAddCacChan(struct TLV *tlv, uint8_t opclass, uint8_t channel, uint32_t countdown);

/****************************** TLV-17.2.46 *****************************/
struct cacTypeStruct {
    struct TLVStruct s;
    uint8_t method;
#define GET_DUR_VALUE(dur) ((dur[2] << 16 | dur[1] << 8 | dur[0]) & 0x00ffffff);
    uint8_t dur[3];
};

struct cacCapaTLV {
    struct TLV tlv;
    uint16_t cn_code;
};

struct bSTARadioCapaTLV {
    struct TLV tlv;
    mac_address rid;
#define MAC_INCLUDED    BIT(7)
    uint8_t capa;
};

struct radioMetricsTLV {
    struct TLV tlv;
    mac_address rid;
    struct radio_metrics metrics;
};

struct apExtMetricsTLV {
    struct TLV tlv;
    mac_address bssid;
    struct ap_ext_metrics metrics;
};

struct associatedSTAExtLinkMetricsStruct {
    struct TLVStruct s;
    mac_address bssid;
    struct associated_sta_ext_link_metrics metrics;
};

struct cacCapaOpclassStruct *cacCapaTypeAddOpclass(struct cacTypeStruct *type, uint8_t opclass);
struct cacTypeStruct *cacCapaRadioAddType(struct macStruct *radio, uint8_t method, uint32_t dur);
struct macStruct *cacCapaTLVAddRadio(struct TLV *tlv, uint8_t *ruid);


/****************************** TLV-17.2.48 *****************************/
#define BYTE_UNITS_KIBIBYTES    (1)
#define BYTE_UNITS_MEBIBYTES    (2)
#define BYTE_UNITS_SHIFT        (6)
#define BYTE_UNITS_MASK         (0x03)
#define PRIORITY_SHIFT          (5)
#define PRIORITY_MASK           (0x01)
#define DPP_ONBOARDING_SHIFT    (4)
#define DPP_ONBOARDING__MASK    (0x01)
#define TRAFFIC_SEPARATE_SHIFT  (3)
#define TRAFFIC_SEPARATE_MASK   (0x01)


/****************************** TLV-17.2.49 *****************************/
struct default80211QSetsTLV {
    struct TLV tlv;
    uint16_t vlanid;
#define DEFAULT_PCP_SHIFT   (5)
#define DEFAULT_PCP_MASK    (0x7)
    uint8_t def_pcp;
};

/****************************** TLV-17.2.50 *****************************/
struct trafficPolicyStruct {
    struct TLVStruct s;
    struct ssid ssid;
    uint16_t vlanid;
};
struct trafficPolicyStruct *trafficPolicyTLVAddSsid(struct TLV *tlv, uint8_t len, uint8_t *ssid, uint16_t vlan_id);

/****************************** TLV-17.2.51 *****************************/
struct p2ErrorCodeTLV {
    struct TLV tlv;
#define REASON_SERVICE_RULE_NOT_FOUND             (0x01)
#define REASON_SERVICE_RULE_NUM_EXCEED            (0x02)
#define REASON_NO_PCP_OR_VLANID                   (0x03)
#define REASON_VLAN_ID_NUM_EXCEED                 (0x05)
#define REASON_COMBINED_BSS_TRAFFIC_SEP_UNSUPPORT (0x07)
#define REASON_MIX_BACKHAUL_UNSUPPORT             (0x08)
#define REASON_TRAFFIC_SE_UNSUPPORT               (0x0A)
#define REASON_QOS_MGMT_POLICY_UNCONFIG           (0x0B)
#define REASON_QOS_MGMT_DSCP_POLICY_REJECT        (0x0C)
#define REASON_AGETN_CANNOT_ONBOARD               (0x0D)
    uint8_t code;
    mac_address bssid;
    uint32_t rule_id;
    uint16_t qmid;
};

/****************************** TLV-17.2.52 *****************************/
struct radioAdvCapaTLV {
    struct TLV tlv;
    mac_address ruid;
#define SUPPORT_COMBINED_FRONT_BACK BIT(7)
#define SUPPORT_COMBINED_P1_P2      BIT(6)
#define SUPPORT_MSCS                BIT(5)
#define SUPPORT_SCS                 BIT(4)
#define SUPPORT_QOS_MAP             BIT(3)
#define SUPPORT_DSCP_POLICY         BIT(2)
    uint8_t flag;
};

/****************************** TLV-17.2.53 *****************************/
struct assocStatNotifyStruct {
    struct TLVStruct s;
    mac_address bssid;
#define NO_MORE_ASSOC   (0x0)
#define ASSOC_ALLOW     (0x01)
    uint8_t status;
};
struct assocStatNotifyStruct *assocStatNotifyTLVAddBssid(struct TLV *tlv, uint8_t *ssid, uint8_t status);

/****************************** TLV-17.2.55 *****************************/
// This is for tunnled message
#define MSG_ASSOC_REQUEST           (0x0)
#define MSG_REASSOC_REQUEST         (0x01)
#define MSG_BTM_QUERY               (0x02)
#define MSG_WNM_REQUEST             (0x03)
#define MSG_ANQP_REQUEST            (0x04)
#define MSG_DSCP_POLICY_QUERY       (0x05)
#define MSG_DSCP_POLICY_RESPONSE    (0x06)
// This is for register mgmt frame
#define MSG_INTERESTING             (0xfe)
// This is for terminal of mgmt frame register table
#define MSG_NULL                    (0xff)

/****************************** TLV-17.2.56 *****************************/
struct tunnelTLV {
    struct TLV tlv;
    struct vvData value;
};

/****************************** TLV-17.2.58 *****************************/
struct unassocPolicyTLV {
    struct TLV tlv;
#define REPORT_UNASSOC  BIT(7)
    uint8_t report_flag;
    uint32_t max_rate;
};

/****************************** TLV-17.2.59 *****************************/
struct metricCollectIntervalTLV {
    struct TLV tlv;
    uint32_t interval;
};

struct macStruct *bStaCapabilityAddMac(struct bSTARadioCapaTLV *tlv, uint8_t *mac);

/****************************** TLV-17.2.66 *****************************/
struct backhaulBSSConfigTLV {
    struct TLV tlv;
    mac_address bssid;
#define P1_bSTA_DISALLOWED  BIT(7)
#define P2_bSTA_DISALLOWED  BIT(6)
    uint8_t config;
};

struct securityCapabilityTLV {
    struct TLV tlv;
    uint8_t onboarding_protocol;
    uint8_t mic_alg;
    uint8_t encrypt_alg;
};

struct micTLV {
    struct TLV tlv;
    uint8_t mic_misc;
    uint8_t integrity_tx_counter[6];
    mac_address src_mac;
    struct vvData mic;
};

struct encryptedPayloadTLV {
    struct TLV tlv;
    uint8_t encryt_tx_counter[6];
    mac_address src_mac;
    mac_address dst_mac;
    struct vvData aes_siv;
};

struct servicePriorityRuleTLV {
    struct TLV tlv;
    uint32_t rule_id;
    uint8_t flag;
    uint8_t precedence;
    uint8_t output;
    uint8_t match;
};

struct dscpMappingTableTLV {
    struct TLV tlv;
    uint8_t dscp_pcp_mapping[64];
};

struct deviceInventoryRadioStruct {
    struct TLVStruct s;
    mac_address ruid;
    struct vvData chip_vendor;
};

struct deviceInventoryTLV {
    struct TLV tlv;
    struct vvData serial_no;
    struct vvData software_ver;
    struct vvData exec_env;
};

struct agentListAgentStruct {
    struct TLVStruct s;
    mac_address al_mac;
    uint8_t map_profile;
    uint8_t security;
};

/**************************** vendor specific TLV **************************/
struct superTLV {
    struct TLV tlv;
    struct TLV *sub_tlv;
};


struct vendorTLV {
    struct superTLV super;
    uint8_t oui[OUILEN];
};

struct wifi6RoleStruct {
    struct TLVStruct s;
    uint8_t role_he_mcs[13];//role, he160/80+80, mcs nss len (1+4/8/12)
    uint8_t bf_ofdma;
    uint8_t max_um_mimo;
    uint8_t max_dl_ofdma_tx;
    uint8_t max_ul_ofdma_rx;
    uint8_t other_capa;
};

struct tidQueueSizeStruct {
    struct TLVStruct s;
    uint8_t tid;
    uint8_t queue_size;
};

struct bssConfigBssStruct {
    struct TLVStruct s;
    mac_address bssid;
    uint8_t flag[2];
    struct ssid ssid;
};

struct controllerWeightTLV {
    struct TLV tlv;
    uint8_t weight;
};

/**************************** sw-queue child TLV **************************/
struct swQueueStruct {
    struct TLVStruct s;
    uint8_t qid;
    uint8_t weight;
    uint8_t port;
};

struct swQueueStruct *egressQConfAddQueue(struct TLV *tlv, struct queue_conf_item *conf);


/**************************** DSCP-mapping child TLV **************************/
struct DSCPMappingStruct {
    struct TLVStruct s;
    uint8_t dscp_value;
    uint8_t swq_id;
    uint8_t wmm_tid;
};

struct DSCPMappingTLV {
    struct TLV tlv;
    uint8_t dft_swq;
    uint8_t dft_tid;
};

struct DSCPMappingStruct *mappingConfAddEntry(struct TLV * tlv, struct DSCP_mapping_item * conf);

/**************************** TC-mapping TLV **************************/
struct TcMappingStruct {
    struct TLVStruct s;
    uint8_t tc;
    uint8_t tid;
    uint8_t qid;
};

struct TcMappingTLV {
    struct TLV tlv;
    uint8_t dft_qid;
    uint8_t dft_tid;
};
struct TcMappingStruct *TcConfAddEntry(struct TLV *tlv, struct tc_mapping_item *conf);
/***************************************************************************/

struct clsCapTLV {
    struct TLV tlv;
    struct vvData cap;
};

struct akmSuiteCapaStruct {
    struct TLVStruct s;
    struct akm_suite_cap cap;
};

struct encapDppTLV {
    struct TLV tlv;
    uint8_t flag;
    mac_address dst_sta_mac;
    uint8_t frame_type;
    struct vvData frame;
};

struct l0vvTLV {
    struct TLV tlv;
    struct vvData data;
};

struct dppBootstrapUriNotificationTLV {
    mac_address ruid;
    mac_address bssid;
    mac_address bsta_mac;
    struct vvData dpp_uri;
};

struct dppChirpValueTLV {
    struct TLV *tlv;
    uint8_t flag;
    mac_address dst_sta_mac;
    struct vvData hash;
};

struct anticipatedChannelPrefOpClassChanStruct {
    struct TLVStruct s;
    uint8_t opclass;
    struct l1vData chan;
    uint8_t reserved[4];
};

struct anticiaptedChannelUsageEntryStruct {
    struct TLVStruct s;
    uint32_t burst_start;
    uint32_t burst_length;
    uint32_t repetitions;
    uint32_t burst_interval;
    struct vvData ru_bitmask;
    mac_address transmitter_id;
    uint8_t power_level;
    uint8_t reason;
    uint8_t reserved[4];
};

struct anticiaptedChannelUsageTLV {
    struct TLV *tlv;
    uint8_t opclass;
    uint8_t channel;
    mac_address reference_bssid;
};

struct spacialReuseRequestTLV {
    struct TLV *tlv;
    mac_address ruid;
    uint8_t bss_color;
    uint8_t flag;
    uint8_t non_srg_obsspd_max_offset;
    uint8_t srg_obsspd_min_offset;
    uint8_t srg_obsspd_max_offset;
    uint8_t srg_bss_color_bitmap[8];
    uint8_t srg_partial_bssid_bitmap[8];
};

struct spacialReuseReportTLV {
    struct TLV *tlv;
    mac_address ruid;
    uint8_t bss_color;
    uint8_t flag;
    uint8_t non_srg_obsspd_max_offset;
    uint8_t srg_obsspd_min_offset;
    uint8_t srg_obsspd_max_offset;
    uint8_t srg_bss_color_bitmap[8];
    uint8_t srg_partial_bssid_bitmap[8];
    uint8_t neighbor_bss_color_in_use_bitmap[8];
};

struct spatialReuseConfigResponseTLV {
    struct TLV *tlv;
    mac_address ruid;
    uint8_t response_code;
};

struct qosManagementDescTLV {
    struct TLV *tlv;
    uint8_t qmid[2];
    mac_address bssid;
    mac_address client_mac;
    struct vvData desc_element;
};


struct TLVDesc *getSubTLVDesc(uint8_t type, uint16_t subtype);

/***************** TLV-appendix B VBSS TLV ******************/

/* appendix B.5.1 AP Radio VBSS Capabilities TLV */
struct vbssCapabilitiesTLV {
    struct TLV tlv;
    mac_address ruid;
    uint8_t max_vbss;
#define VBSSs_SUBTRACT_SHIFT 7
#define VBSSID_RESTRICTIONS_SHIFT 6
#define VBSSID_MATCH_AND_MASK_RESTRICTIONS_SHIFT 5
#define FIXED_BITS_RESTRICTIONS_SHIFT 4
    uint8_t flag;
    mac_address fixed_bits_mask;
    mac_address fixed_bits_value;
};

/* appendix B.5.2 Virtual BSS Creation TLV */
struct vbssCreationTLV {
    struct TLV tlv;
    mac_address ruid;
    mac_address bssid;
    struct vvData ssid;
    struct vvData wpa_password;
    struct vvData dpp_connector;
    mac_address client_mac;
    uint8_t client_assoc;
    struct vvData ptk;
    uint64_t tx_packet_number;
    struct vvData gtk;
    uint64_t group_tx_packet_number;
};

/* appendix B.5.3 Virtual BSS Destruction TLV */
struct vbssDestructionTLV {
    struct TLV tlv;
    mac_address ruid;
    mac_address bssid;
    uint8_t disassociate_client;
};

/* appendix B.5.4 Virtual BSS Event TLV */
struct vbssEventTLV {
    struct TLV tlv;
    mac_address ruid;
    uint8_t success;
    mac_address bssid;
};

/* appendix B.5.5 Client Security Context TLV */
struct clientSecurityContextTLV {
    struct TLV tlv;
#define CLIENT_CONNECTED_SHIFT 7
    uint8_t flag;
    struct vvData ptk;
    uint64_t tx_packet_number;
    struct vvData gtk;
    uint64_t group_tx_packet_number;
};

/* appendix B.5.6 Client Security Context TLV */
struct triggerCSATLV {
    struct TLV tlv;
    mac_address ruid;
    uint8_t csa_channel;
    uint8_t opclass;
};

/* appendix B.5.7 VBSS Configuration Report TLV */
struct apOperationVBSSStruct {
    struct TLVStruct s;
    mac_address bssid;
    struct ssid ssid;
};

struct TLVDesc *getDesc(struct TLVDesc *table, uint16_t max, uint16_t type);
struct TLVDesc *getTLVDesc(uint8_t type);
struct TLV *subTLVNew(dlist_head *parent, uint8_t type, uint16_t subtype, uint32_t size);
struct TLV *superTLVNew(uint8_t type, struct TLV *sub_tlv);
struct serviceStruct *serviceAddService(struct TLV *tlv, uint8_t service);
struct interfaceStruct *deviceInfoAddInterface(struct TLV *tlv, struct interface *i);
struct macStruct *deviceAddOperationalRadio(struct TLV *tlv, struct al_device *d, struct radio *r);
struct macStruct *deviceAddVbssConfigurationReportRadio(struct TLV *tlv, struct al_device *d, struct radio *r);
struct txOpclassStruct *apRadioBasicCapaTLVAddOpClass(struct TLV *tlv, uint8_t opclass, uint8_t max_tx_power,
                                                    uint8_t num, uint8_t *non_op_chans);
struct associatedBSSStruct *_deviceAddBSSAssociated(struct TLV *tlv, struct wifi_interface *wi, uint32_t current);
struct i1905NeighborStruct *i1905AddNeighbor(struct TLV *tlv, struct neighbor *n);
struct macStruct *non1905AddNeighbor(struct TLV *tlv, struct neighbor *n);
struct macStruct *ipv4AddInterface(struct TLV *tlv, uint8_t *mac);
struct ipv4AddrStruct *ipv4InterfaceAddIP(struct macStruct *s, uint8_t proto, struct ipv4 *ip, struct ipv4 *dhcp);
struct interfaceipv6Struct *ipv6AddInterface(struct TLV *tlv, uint8_t *mac, struct ipv6 *local_ip);
struct ipv6AddrStruct *ipv6InterfaceAddIP(struct interfaceipv6Struct *s, uint8_t proto, struct ipv6 *ip, struct ipv6 *origin);

void init_non_std_tlv_ops(void);
void deinit_non_std_tlv_ops(void);
extern DECLARE_STRUCT_DESC_GLOBAL(macStruct);

struct TLV *subTLVNew(dlist_head *parent, uint8_t type, uint16_t subtype, uint32_t size);
struct TLV *superTLVNew(uint8_t type, struct TLV *sub_tlv);


#include "1905_tlvs_cls.h"

#endif
