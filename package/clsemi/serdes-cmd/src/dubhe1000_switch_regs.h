/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 1999 - 2006 Clourneysemi Corporation. */

/* dubhe1000_switch.h
 * Structures, enums, and macros for the Switch
 */

#ifndef _DUBHE1000_SWITCH_REGS_H_
#define _DUBHE1000_SWITCH_REGS_H_


/* Global Register */
#define MAC_RX_MAX_PACKET_LEN_TBL                           0x003C
#define MAC_RX_MAX_PACKET_LEN_TBL_MAX                       6

#define INGRESS_ADMISSION_CTRL_RESET                        0x014A
#define INGRESS_ADMISSION_CTRL_RESET_MAX                    32

#define INGR_ADMIS_CTRL_TOKEN_BUCKET_CONF                   0x00CA
#define INGR_ADMIS_CTRL_TOKEN_BUCKET_CONF_MAX               32
#define INGR_ADMIS_CTRL_TOKEN_BUCKET_CONF_ADDR_PER_ENTRY    4

#define SOURCE_PORT_TBL                                     0x1E164
#define SOURCE_PORT_TBL_MAX                                 6
#define SOURCE_PORT_NUM_ADDR_PER_ENTRY                      8

/* L2 Protocol */
#define ARP_PACKET_DECODER_OPTIONS                          0x1E129
#define AH_HEADER_PACKET_DECODER_OPTIONS                    0x1E12E
#define BOOTP_DHCP_PACKET_DECODER_OPTIONS                   0x1E446
#define CAPWAP_PACKET_DECODER_OPTIONS                       0x1E448
#define DNS_PACKET_DECODER_OPTIONS                          0x1E130
#define ESP_HEADER_PACKET_DECODER_OPTIONS                   0x1E12F
#define GRE_PACKET_DECODER_OPTIONS                          0x1E442
#define L2_1588_PACKET_DECODER_OPTIONS                      0x1E12B
#define L4_1588_PACKET_DECODER_OPTIONS                      0x1E540
#define EAPOL_PACKET_DECODER_OPTIONS                        0x1E12C
#define IKE_PACKET_DECODER_OPTIONS                          0x1E44A
#define LACP_PACKET_DECODER_OPTIONS                         0x1E444
#define RARP_PACKET_DECODER_OPTIONS                         0x1E12A
#define SCTP_PACKET_DECODER_OPTIONS                         0x1E12D

/* L2 Table/Register */
#define L2_DESTINATION_TBL                                  0x1611E
#define L2_DESTINATION_TBL_MAX                              4128

#define L2_AGING_STATUS_SHADOW_TBL                          0x1311E
#define L2_AGING_STATUS_SHADOW_TBL_MAX                      4096

#define L2_AGING_TBL                                       0x01B3
#define L2_AGING_TBL_MAX                                   4096

#define L2_DA_HASH_LOOKUP_TBL                              0x1411E
#define L2_DA_HASH_LOOKUP_TBL_MAX                          4096
#define L2_DA_HASH_LOOKUP_TBL_ADDR_PER_ENTRY               2

/* L3 Table/Register */
#define ROUTER_PORT_MACADDR_TABLE                           0x1E194
#define ROUTER_PORT_MACADDR_TABLE_MAX                       16
#define ROUTER_PORT_MACADDR_TABLE_ADDR_PER_ENTRY            8

#define L3_ROUTING_DEFAULT_TBL                            0x1DE7A
#define L3_ROUTING_DEFAULT_TBL_MAX                        4

#define INGRESS_ROUTER_TBL                                0xE10A
#define INGRESS_ROUTER_TBL_MAX                            4

#define NEXT_HOP_TBL                                      0x1211E
#define NEXT_HOP_TBL_MAX                                  1024
#define NEXT_HOP_TBL_ADDR_PER_ENTRY                       2

#define INGR_EGR_PORT_PKT_TYPE_FILTER_TBL                 0x1DAD6
#define INGR_EGR_PORT_PKT_TYPE_FILTER_TBL_MAX             6

#define NEXT_HOP_PACKET_MODIFY_TBL                        0x1291E
#define NEXT_HOP_PACKET_MODIFY_TBL_MAX                    1024
#define NEXT_HOP_PACKET_MODIFY_TBL_ADDR_PER_ENTRY         2

#define NEXT_HOP_DA_MAC_TBL                               0x1F8D0
#define NEXT_HOP_DA_MAC_TBL_MAX                           1024
#define NEXT_HOP_DA_MAC_TBL_ADDR_PER_ENTRY                2

#define HASH_BASED_L3_ROUTING_TBL                         0xE11E
#define HASH_BASED_L3_ROUTING_TBL_MAX                     2048
#define HASH_BASED_L3_ROUTING_TBL_ADDR_PER_ENTRY          8


/* VLAN */
#define VLAN_TABLE                                        0xA0FA
#define VLAN_TABLE_MAX                                    4096
#define VLAN_TABLE_ADDR_PER_ENTRY                         4

/* ACL */
#define CFG_ACL_MATCH_COUNTER_MAX				64
/* INGRESS ACL */
#define INGRESS_CFG_ACL_NUMBER					6
#define INGRESS_CFG_ACL_ENGINE_NUMBER				5

#define INGRESS_CFG_ACL_TABLE_ADDR_PER_ENTRY			16

#define INGRESS_CFG_ACL_0_LARGE_TABLE				0x1E9A
#define INGRESS_CFG_ACL_0_LARGE_TABLE_MAX			1024
#define INGRESS_CFG_ACL_1_LARGE_TABLE				0x6F1A
#define INGRESS_CFG_ACL_2_LARGE_TABLE				0x7FBA
#define INGRESS_CFG_ACL_3_LARGE_TABLE				0x905A
#define INGRESS_CFG_ACL_LARGE_TABLE_MAX			256

#define INGRESS_CFG_ACL_0_RULES_SETUP				0x1DFB6
#define INGRESS_CFG_ACL_1_RULES_SETUP				0x1DF56
#define INGRESS_CFG_ACL_2_RULES_SETUP				0x1DEF6
#define INGRESS_CFG_ACL_3_RULES_SETUP				0x1DE96
#define ACL_RULE_POINTER_NUMBER		16
#define ACL_RULE_FIELD_NUMBER		6

#define INGRESS_CFG_ACL_0_SEARCH_MASK				0x1E794
#define INGRESS_CFG_ACL_1_SEARCH_MASK				0x1E7A4
#define INGRESS_CFG_ACL_2_SEARCH_MASK				0x1E7B4
#define INGRESS_CFG_ACL_3_SEARCH_MASK				0x1E7C4
#define INGRESS_CFG_ACL_SEARCH_MASK_MAX			1
#define INGRESS_CFG_ACL_SEARCH_MASK_ADDR_PER_ENTRY		16

#define INGRESS_CFG_ACL_0_SELECTION				0x1E133
#define INGRESS_CFG_ACL_1_SELECTION				0x1E134
#define INGRESS_CFG_ACL_2_SELECTION				0x1E135
#define INGRESS_CFG_ACL_3_SELECTION				0x1E136
#define INGRESS_CFG_ACL_SELECTION_MASK_MAX			1
#define INGRESS_CFG_ACL_SELECTION_MASK_ADDR_PER_ENTRY	1

#define INGRESS_CFG_ACL_0_SMALL_TABLE				0x5E9A
#define INGRESS_CFG_ACL_0_SMALL_TABLE_MAX			256
#define INGRESS_CFG_ACL_1_SMALL_TABLE				0x7F1A
#define INGRESS_CFG_ACL_2_SMALL_TABLE				0x8FBA
#define INGRESS_CFG_ACL_3_SMALL_TABLE				0xA05A
#define INGRESS_CFG_ACL_SMALL_TABLE_MAX			8

#define ACL_0_HASH_KEY_SIZE						154
#define ACL_OTHER_HASH_KEY_SIZE						214

#define INGRESS_CFG_ACL_0_TCAM					0x1EA64
#define INGRESS_CFG_ACL_0_TCAM_MAX				16
#define INGRESS_CFG_ACL_1_TCAM					0x1E9E4
#define INGRESS_CFG_ACL_2_TCAM					0x1E964
#define INGRESS_CFG_ACL_3_TCAM					0x1E8E4
#define INGRESS_CFG_ACL_TCAM_MAX				8
#define INGRESS_CFG_ACL_TCAM_ADDR_PER_ENTRY			16

#define INGRESS_CFG_ACL_0_TCAM_ANSWER				0x6E9A
#define INGRESS_CFG_ACL_0_TCAM_ANSWER_MAX			16
#define INGRESS_CFG_ACL_0_TCAM_ANSWER_ADDR_PER_ENTRY		8
#define INGRESS_CFG_ACL_1_TCAM_ANSWER				0x7F9A
#define INGRESS_CFG_ACL_2_TCAM_ANSWER				0x903A
#define INGRESS_CFG_ACL_3_TCAM_ANSWER				0xA0DA
#define INGRESS_CFG_ACL_TCAM_ANSWER_MAX			8
#define INGRESS_CFG_ACL_TCAM_ANSWER_ADDR_PER_ENTRY		4

#define INGRESS_CFG_ACL_MATCH_COUNTER				0x1ECA5

#define INGRESS_CFG_ACL_DROP					0x1227

/* EGRESS ACL */
#define EGRESS_ACL_RULE_POINTER_TCAM					0x1E450
#define EGRESS_ACL_RULE_POINTER_TCAM_ANSWER				0x1B15E
#define EGRESS_ACL_RULE_POINTER_TCAM_MAX				64
#define EGRESS_ACL_RULE_POINTER_TCAM_ADDR_PER_ENTRY			2

#define EGRESS_CFG_ACL_LARGE_TABLE				0x1B19E
#define EGRESS_CFG_ACL_SMALL_TABLE				0x1D19E

#define EGRESS_CFG_ACL_TABLE_ADDR_PER_ENTRY			8

#define EGRESS_CFG_ACL_RULES_SETUP				0x1DB5C

#define EGRESS_CFG_ACL_SEARCH_MASK				0x1E7D4
#define EGRESS_CFG_ACL_SEARCH_MASK_MAX				1
#define EGRESS_CFG_ACL_SEARCH_MASK_ADDR_PER_ENTRY		16

#define EGRESS_CFG_ACL_SELECTION				0x1E149

#define EGRESS_CFG_ACL_MATCH_COUNTER				0x1F0E9

#define EGRESS_CFG_ACL_DROP					0x1228

/* NAT */
#define NAT_ADD_EGRESS_PORT_FOR_NAT_CAL					0x21DF7

#define EGRESS_NAT_OPERATION						0x20D6C
#define EGRESS_NAT_OPERATION_ADDR_PER_ENTRY				2
#define INGRESS_NAT_OPERATION						0x2056C
#define INGRESS_NAT_OPERATION_ADDR_PER_ENTRY				2
#define NAT_OPERATION_MAX						1024

#define NAT_OPERATION_ADDR_PER_ENTRY					2

#define EGRESS_NAT_HIT_STATUS						0x2230D
#define EGRESS_NAT_HIT_STATUS_ADDR_PER_ENTRY				1
#define INGRESS_NAT_HIT_STATUS						0x21F0D
#define INGRESS_NAT_HIT_STATUS_ADDR_PER_ENTRY				1

#define EGRESS_PORT_NAT_STATE						0x1E14A

#define NAT_ACTION_TABLE						0x1DADC
#define NAT_ACTION_TABLE_MAX						(1 << 7)
#define NAT_ACTION_TABLE_ADDR_PER_ENTRY					1
#define NAT_ACTION_TABLE_DROP						0x1237

#endif /* _DUBHE1000_SWITCH_REGS_H_ */

