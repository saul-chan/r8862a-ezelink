/*
 * Copyright (c) 2021-2024, Clourney Semiconductor. All rights reserved.
 *
 * This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
 * Reproduction and redistribution in binary or source form, with or without modification,
 * for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
 * must retain the above copyright notice.
 *
 * By using this software and/or documentation, you agree to the limited terms and conditions.
 */

/* dubhe1000_switch.c
 * Shared functions for accessing and configuring the Switch
 */

#include "dubhe2000_switch_regs.h"
#include "dubhe2000_switch.h"
#include "dubhe2000_acl.h"

struct acl_hash_setting acl_hash_settings[] = {
	{ 42, 330, 9, 2, 6, 2 }, //ingress acl0
	{ 17, 135, 6, 1, 2, 1 }, //ingress acl1
	{ 68, 540, 0, 0, 0, 0 }, //ingress acl2
	{ 10, 80, 0, 0, 0, 0 },	 //ingress acl3
};

u32 acl_max_entry[4][3] = {
	{ INGRESS_CONFIGURABLE_ACL_0_LARGE_TABLE_MAX, INGRESS_CONFIGURABLE_ACL_0_SMALL_TABLE_MAX,
	  INGRESS_CONFIGURABLE_ACL_0_TCAM_MAX },
	{ INGRESS_CONFIGURABLE_ACL_1_LARGE_TABLE_MAX, INGRESS_CONFIGURABLE_ACL_1_SMALL_TABLE_MAX,
	  INGRESS_CONFIGURABLE_ACL_1_TCAM_MAX },
	{ 0, 0, INGRESS_CONFIGURABLE_ACL_2_TCAM_MAX },
	{ 0, 0, INGRESS_CONFIGURABLE_ACL_3_TCAM_MAX },
};

int dubhe1000_acl_check_index(u8 aclN, u8 tbl_type, u16 index)
{
	if (aclN < 4 && (tbl_type <= SWITCH_ACL_TBL_TYPE_TCAM))
		return (index < acl_max_entry[aclN][tbl_type]);
	else
		return 0;
}

void dubhe1000_ingress_acl0_ipv4_nat_hashkey_build(u8 *compareData, u32 src_addr, u32 dest_addr, u16 src_port,
						   u16 dest_port, u8 ip_proto, u8 source_port)
{
	/*  compareData:
	 * | bit329-113 | bit112-110 | bit109-102 | bit101-86 |  bit85-70 | bit69-38 | bit37-6 | bit5-0 |
	 * |   unused   | source port| L4 protocol | dest port | src port  | dest addr| src addr|  valid |
	 */
	struct dubhe1000_tc_ipv4_nat_hashkey hashkey;

	hashkey.valid = 0x3F;
	hashkey.src_addr_part0 = src_addr & BITS_32(0, 24);
	hashkey.src_addr_part1 = (src_addr & BITS_32(25, 31)) >> 25;

	hashkey.dest_addr_part0 = dest_addr & BITS_32(0, 24);
	hashkey.dest_addr_part1 = (dest_addr & BITS_32(25, 31)) >> 25;

	hashkey.src_port = src_port;

	hashkey.dest_port_part0 = dest_port & BITS_32(0, 8);
	hashkey.dest_port_part1 = (dest_port & BITS_32(9, 15)) >> 9;

	hashkey.l4_proto = ip_proto;
	hashkey.source_port = source_port & 0x7;

	hashkey.reserved = 0;

	memcpy(compareData, &hashkey, sizeof(hashkey));
}

void dubhe2000_ingress_acl0_ipv6_router_hashkey_build(u8 *compareData, u32 *src_ipv6, u32 *dest_ipv6, u16 src_port,
						      u16 dest_port, u8 ip_proto, u8 source_port)
{
	/*  compareData:
	 * | bit329-305| bit304-302 | bit301-294 | bit293-278 | bit277-262 | bit261-134 | bit133-6 | bit5-0 |
	 * |   unused  | source port |L4 protocol|  dest port |  src port  |  dest_ipv6 | src_ipv6 | valid  |
	 */
	struct dubhe1000_tc_ipv6_router_hashkey hashkey;

	hashkey.valid = 0x3F;

	hashkey.src_ipv6_part0 = src_ipv6[0] & BITS_32(0, 24);
	hashkey.src_ipv6_part1 = ((src_ipv6[0] & BITS_32(25, 31)) >> 25) + ((src_ipv6[1] & BITS_32(0, 24)) << 7);
	hashkey.src_ipv6_part2 = ((src_ipv6[1] & BITS_32(25, 31)) >> 25) + ((src_ipv6[2] & BITS_32(0, 24)) << 7);
	hashkey.src_ipv6_part3 = ((src_ipv6[2] & BITS_32(25, 31)) >> 25) + ((src_ipv6[3] & BITS_32(0, 24)) << 7);
	hashkey.src_ipv6_part4 = ((src_ipv6[3] & BITS_32(25, 31)) >> 25);

	hashkey.dest_ipv6_part0 = dest_ipv6[0] & BITS_32(0, 24);
	hashkey.dest_ipv6_part1 = ((dest_ipv6[0] & BITS_32(25, 31)) >> 25) + ((dest_ipv6[1] & BITS_32(0, 24)) << 7);
	hashkey.dest_ipv6_part2 = ((dest_ipv6[1] & BITS_32(25, 31)) >> 25) + ((dest_ipv6[2] & BITS_32(0, 24)) << 7);
	hashkey.dest_ipv6_part3 = ((dest_ipv6[2] & BITS_32(25, 31)) >> 25) + ((dest_ipv6[3] & BITS_32(0, 24)) << 7);
	hashkey.dest_ipv6_part4 = ((dest_ipv6[3] & BITS_32(25, 31)) >> 25);

	hashkey.src_port = src_port;

	hashkey.dest_port_part0 = dest_port & BITS_32(0, 8);
	hashkey.dest_port_part1 = (dest_port & BITS_32(9, 15)) >> 9;

	hashkey.l4_proto = ip_proto;
	hashkey.source_port = source_port & 0x7;
	hashkey.reserved = 0;

	memcpy(compareData, &hashkey, sizeof(hashkey));
}

void dubhe2000_ingress_acl1_ip_hashkey_build(u8 *compareData, u32 sip, u32 dip, u16 src_port, u16 dst_port, u8 l4_proto,
					     u8 source_port)
{
	/* compareData:
	 * |bit127-114|bit113-111 |bit110-103|bit102-87|bit86-71|bit70-39|bit38-7|bit6-0|
	 * |  reserved|source_port| l4_proto |l4_dport |l4_sport|ipv4_da |ipv4_sa |valid |
	 */

	struct dubhe2000_tc_acl1_ip_hashkey hashkey;

	memset(&hashkey, 0, sizeof(hashkey));

	hashkey.valid = 0x0; //updated not here

	hashkey.ipv4_sa_part0 = sip & BITS_32(0, 24);
	hashkey.ipv4_sa_part1 = (sip >> 25) & BITS_32(0, 6);

	hashkey.ipv4_da_part0 = dip & BITS_32(0, 24);
	hashkey.ipv4_da_part1 = (dip >> 25) & BITS_32(0, 6);

	hashkey.l4_sport = src_port;
	hashkey.l4_dport_part0 = dst_port & BITS_32(0, 8);
	hashkey.l4_dport_part1 = (dst_port >> 9) & BITS_32(0, 6);
	hashkey.l4_proto = l4_proto;
	hashkey.source_port = source_port & 0x7;

	hashkey.reserved = 0;

	memcpy(compareData, &hashkey, sizeof(hashkey));
}

void dubhe2000_ingress_acl1_nonip_hashkey_build(u8 *compareData, u8 *dmac, u8 *smac, bool has_vlans,
						bool outer_vlan_type, bool inner_vlan_type, u16 etype, u8 source_port)
{
	/* compareData:
	 * |127-125 |bit124-122 |  bit121-106 |  bit105   |   bit104  | bit103  |bit102-55|bit54-7|bit6-0|
	 * |reserved|source port|ethernet_type|inner_vtype|outer_vtype|has_vlans| mac sa  | mac da| valid|
	 */
	struct dubhe2000_tc_acl1_nonip_hashkey hashkey;
	u64 tmp;

	memset(&hashkey, 0, sizeof(hashkey));

	hashkey.valid = 0x0; //updated not here

	memcpy(&tmp, dmac, ETH_ALEN);
	hashkey.mac_da_part0 = tmp & BITS_32(0, 24);
	hashkey.mac_da_part1 = (tmp >> 25) & BITS_32(0, 22);

	memcpy(&tmp, smac, ETH_ALEN);
	hashkey.mac_sa_part0 = tmp & BITS_32(0, 8);
	hashkey.mac_sa_part1 = (tmp >> 9) & BITS_32(0, 31);
	hashkey.mac_sa_part2 = (tmp >> 41) & BITS_32(0, 6);

	hashkey.has_vlans = has_vlans;
	hashkey.outer_vlan_tag_type = outer_vlan_type;
	hashkey.inner_vlan_tag_type = inner_vlan_type;
	hashkey.etype = etype;
	hashkey.source_port = source_port & 0x7;

	hashkey.reserved = 0;

	memcpy(compareData, &hashkey, sizeof(hashkey));
}

void dubhe2000_ingress_acl2_ipv4_hashkey_build(u8 *compareData, u8 *dmac, u8 *smac, u32 *sip, u32 *dip, u8 tos,
					       u16 l4_sport, u16 l4_dport, u8 l4_proto, enum SWITCH_ACL_L4_TYPE l4_type,
					       u8 source_port)
{
	/* compareData:
	 * |bit287-269|bit268-266 |bit265-263 |bit262-255|bit254-239|bit238-223|bit222-215|
	 * | reserved |source_port|  l4 type  |l4_proto | l4 dport | l4 sport |    tos   |
	 *
	 * |bit214-183|bit182-151|  bit150 |bit149-147|bit146-135|  bit134 | bit133-131|  bit130  |
	 * |  ipv4_da | ipv4_sa  |inner_dei| inner_pcp| inner_vid|outer dei| outer_pcp |inner type|

	 * |  bit129  |  bit128  | bit127-116 | bit115-68 | bit67-20 | bit19-0 |
	 * |outer type|hash_vlans|  outer_vid |   mac sa  |  mac da  |  valid  |
	 */
	struct dubhe2000_tc_acl2_ipv4_hashkey hashkey;
	u64 tmp;

	memset(&hashkey, 0, sizeof(hashkey));

	hashkey.valid = 0x3FFFF; //need modified

	memcpy(&tmp, dmac, ETH_ALEN);
	hashkey.mac_da_part0 = tmp & BITS_32(0, 11);
	hashkey.mac_da_part1 = (tmp >> 12) & BITS_32(0, 31);
	hashkey.mac_da_part2 = (tmp >> 44) & BITS_32(0, 3);

	memcpy(&tmp, smac, ETH_ALEN);
	hashkey.mac_sa_part0 = tmp & BITS_32(0, 27);
	hashkey.mac_sa_part1 = tmp & BITS_32(0, 11);

	//ignore 802.1q part

	hashkey.ipv4_sa_part0 = sip[0] & BITS_32(0, 8);
	hashkey.ipv4_sa_part1 = (sip[0] & BITS_32(9, 31)) >> 9;

	hashkey.ipv4_da_part0 = dip[0] & BITS_32(0, 8);
	hashkey.ipv4_da_part1 = (dip[0] & BITS_32(9, 31)) >> 9;

	hashkey.tos = tos;

	hashkey.l4_sport_part0 = l4_sport & BITS_32(0, 0);
	hashkey.l4_sport_part1 = (l4_sport >> 1) & BITS_32(0, 14);
	hashkey.l4_dport = l4_dport;

	hashkey.l4_proto_part0 = l4_proto & BITS_32(0, 0);
	hashkey.l4_proto_part1 = (l4_proto >> 1) & BITS_32(0, 6);

	hashkey.l4_type = l4_type & 0x7;
	hashkey.source_port = source_port & 0x7;
	hashkey.reserved = 0;

	memcpy(compareData, &hashkey, sizeof(hashkey));
}

void dubhe2000_ingress_acl2_ipv6_hashkey_build(u8 *compareData, u8 *dmac, u8 *smac, u32 *sip, u32 *dip, u8 tos,
					       u16 l4_sport, u16 l4_dport, u32 ipv6_flow_lable, u8 l4_proto,
					       enum SWITCH_ACL_L4_TYPE l4_type, u8 source_port)
{
	/* compareData:
	 * |bit511-481|bit480-478 |bit477-475|bit474-467|   bit466-447   |bit446-431|bit430-415|bit414-407|
	 * | reserved |source_port| l4_type  | l4_proto |ipv6_flow_lable | l4 dport | l4 sport |    tos   |
	 *
	 * |bit406-279|bit278-151|  bit150 |bit149-147|bit146-135|  bit134 | bit133-131|  bit130  |
	 * |  ipv6_da | ipv6_sa  |inner_dei| inner_pcp| inner_vid|outer dei| outer_pcp |inner type|

	 * |  bit129  |  bit128  | bit127-116 | bit115-68 | bit67-20 | bit19-0 |
	 * |outer type|hash_vlans|  outer_vid |   mac sa  |  mac da  |  valid  |
	 */
	struct dubhe2000_tc_acl2_ipv6_hashkey hashkey;
	u64 tmp;

	memset(&hashkey, 0, sizeof(hashkey));

	hashkey.valid = 0x7FFFF; //need modified

	memcpy(&tmp, dmac, ETH_ALEN);
	hashkey.mac_da_part0 = tmp & BITS_32(0, 11);
	hashkey.mac_da_part1 = (tmp >> 12) & BITS_32(0, 31);
	hashkey.mac_da_part2 = (tmp >> 44) & BITS_32(0, 3);

	memcpy(&tmp, smac, ETH_ALEN);
	hashkey.mac_sa_part0 = tmp & BITS_32(0, 27);
	hashkey.mac_sa_part1 = tmp & BITS_32(0, 11);

	//ignore 802.1q part

	hashkey.ipv6_sa_part0 = sip[0] & BITS_32(0, 8);
	hashkey.ipv6_sa_part1 = (sip[0] >> 9) + ((sip[1] & BITS_32(0, 8)) << 23);
	hashkey.ipv6_sa_part2 = (sip[1] >> 9) + ((sip[2] & BITS_32(0, 8)) << 23);
	hashkey.ipv6_sa_part3 = (sip[2] >> 9) + ((sip[3] & BITS_32(0, 8)) << 23);
	hashkey.ipv6_sa_part4 = (sip[3] >> 9);

	hashkey.ipv6_da_part0 = dip[0] & BITS_32(0, 8);
	hashkey.ipv6_da_part1 = (dip[0] >> 9) + ((dip[1] & BITS_32(0, 8)) << 23);
	hashkey.ipv6_da_part2 = (dip[1] >> 9) + ((dip[2] & BITS_32(0, 8)) << 23);
	hashkey.ipv6_da_part3 = (dip[2] >> 9) + ((dip[3] & BITS_32(0, 8)) << 23);
	hashkey.ipv6_da_part4 = (dip[3] >> 9);

	hashkey.tos = tos;
	hashkey.l4_sport_part0 = l4_sport & BITS_32(0, 0);
	hashkey.l4_sport_part1 = (l4_sport >> 1) & BITS_32(0, 14);
	hashkey.l4_dport = l4_dport;
	hashkey.ipv6_flow_lable_part0 = ipv6_flow_lable & BITS_32(0, 0);
	hashkey.ipv6_flow_lable_part1 = (l4_sport >> 1) & BITS_32(0, 18);
	hashkey.l4_proto = l4_proto;
	hashkey.l4_type = l4_type & 0x7;
	hashkey.source_port_part0 = source_port & BITS_32(0, 0);
	hashkey.source_port_part1 = (source_port >> 1) & BITS_32(0, 1);
	hashkey.reserved = 0;

	memcpy(compareData, &hashkey, sizeof(hashkey));
}

void dubhe2000_ingress_acl3_hashkey_build(u8 *compareData, u8 l2_packet_flags, u8 ipv4_options, u16 tcp_flags,
					  u8 l4_proto, u16 etype, enum SWITCH_ACL_L3_TYPE l4_type,
					  enum SWITCH_ACL_L3_TYPE l3_type)
{
	/* compareData:
	 * |bit63-57| bit56-55| bit54-52| bit51-36 |bit35-28| bit27-19 |   bit18-14 |    bit13-10   | bit9-0 |
	 * |reserved| l3 type | l4 type |  etype   |L4 proto| tcp_flags|ipv4_options|l2_packet_flags| valid  |
	 */
	struct dubhe2000_tc_acl3_hashkey hashkey;

	memset(&hashkey, 0, sizeof(hashkey));

	hashkey.valid = 0x7F; //updated not here

	hashkey.l2_packet_flags = l2_packet_flags & 0xF;
	hashkey.ipv4_options = ipv4_options & 0x1F;
	hashkey.tcp_flags = tcp_flags & 0x1FF;
	hashkey.l4_proto_part0 = l4_proto & 0xF;

	hashkey.l4_proto_part1 = (l4_proto >> 4) & 0xF;
	hashkey.ethernet_type = etype;
	hashkey.l4_type = l4_type & 0x7;
	hashkey.l3_type = l3_type & 0x3;
	hashkey.reserved = 0;

	memcpy(compareData, &hashkey, sizeof(hashkey));
}

#define DUBHE1000_ACLN_MAX 4
int dubhe1000_acl_hashval_cal(u8 aclN, u8 *compareData, bool is_large)
{
	u16 hashval, base_value, key_size;
	u8 shift, fold_count, key8_len;
	u32 hashkey[11]; //max length is 330(acl0)
	int i, len32;

	if (aclN >= DUBHE1000_ACLN_MAX) {
		pr_err("[%s] invalid aclN\n", __func__);

		return -1;
	}

	if (is_large)
		shift = acl_hash_settings[aclN].large_hash_size;
	else
		shift = acl_hash_settings[aclN].small_hash_size;

	base_value = BITS_32(0, shift - 1);

	key_size = acl_hash_settings[aclN].compareData_width;

	fold_count = (key_size / shift) + ((key_size % shift) ? 1 : 0);

	key8_len = (key_size / 8) + ((key_size % 8) ? 1 : 0);

	memset(hashkey, 0, sizeof(hashkey));
	memcpy(hashkey, compareData, key8_len);

	len32 = (key_size / 32) + ((key_size % 32) ? 1 : 0);

	hashval = hashkey[0] & base_value;
	for (i = 1; i < fold_count; i++) {
		switch_u32_array_right_shift(hashkey, len32, shift);
		hashval = hashval ^ hashkey[0];
		hashval = hashval & base_value;
	}

	return hashval;
}

#define CLS_ACL_TABLE_PINTER(engine, tbl_type, acl_tbl, member)                                                        \
	(((t_IngressConfigurableACL##engine##tbl_type *)acl_tbl)->member)

// please check all params before calling this function
void cls_switch_get_acl_tbl(u8 aclN, u8 tbl_type, u16 index, void *tbl, u8 **compareData, u8 *is_valid)
{
	if (aclN == 0) {
		if (tbl_type == SWITCH_ACL_TBL_TYPE_LARGE) {
			rd_IngressConfigurableACL0LargeTable(g_adapter, index, tbl);
			*is_valid = !!CLS_ACL_TABLE_PINTER(0, LargeTable, tbl, valid);
			*compareData = CLS_ACL_TABLE_PINTER(0, LargeTable, tbl, compareData);
		} else if (tbl_type == SWITCH_ACL_TBL_TYPE_SMALL) {
			rd_IngressConfigurableACL0SmallTable(g_adapter, index, tbl);
			*is_valid = !!CLS_ACL_TABLE_PINTER(0, SmallTable, tbl, valid);
			*compareData = CLS_ACL_TABLE_PINTER(0, SmallTable, tbl, compareData);
		} else if (tbl_type == SWITCH_ACL_TBL_TYPE_TCAM) {
			rd_IngressConfigurableACL0TCAM(g_adapter, index, tbl);
			*is_valid = !!CLS_ACL_TABLE_PINTER(0, TCAM, tbl, valid);
			*compareData = CLS_ACL_TABLE_PINTER(0, TCAM, tbl, compareData);
		}
	} else if (aclN == 1) {
		if (tbl_type == SWITCH_ACL_TBL_TYPE_LARGE) {
			rd_IngressConfigurableACL1LargeTable(g_adapter, index, tbl);
			*is_valid = !!CLS_ACL_TABLE_PINTER(1, LargeTable, tbl, valid);
			*compareData = CLS_ACL_TABLE_PINTER(1, LargeTable, tbl, compareData);
		} else if (tbl_type == SWITCH_ACL_TBL_TYPE_SMALL) {
			rd_IngressConfigurableACL1SmallTable(g_adapter, index, tbl);
			*is_valid = !!CLS_ACL_TABLE_PINTER(1, SmallTable, tbl, valid);
			*compareData = CLS_ACL_TABLE_PINTER(1, SmallTable, tbl, compareData);
		} else if (tbl_type == SWITCH_ACL_TBL_TYPE_TCAM) {
			rd_IngressConfigurableACL1TCAM(g_adapter, index, tbl);
			*is_valid = !!CLS_ACL_TABLE_PINTER(1, TCAM, tbl, valid);
			*compareData = CLS_ACL_TABLE_PINTER(1, TCAM, tbl, compareData);
		}
	} else if (aclN == 2) {
		if (tbl_type == SWITCH_ACL_TBL_TYPE_TCAM) {
			rd_IngressConfigurableACL2TCAM(g_adapter, index, tbl);
			*is_valid = !!CLS_ACL_TABLE_PINTER(2, TCAM, tbl, valid);
			*compareData = CLS_ACL_TABLE_PINTER(2, TCAM, tbl, compareData);
		}
	} else if (aclN == 3) {
		if (tbl_type == SWITCH_ACL_TBL_TYPE_TCAM) {
			rd_IngressConfigurableACL3TCAM(g_adapter, index, tbl);
			*is_valid = !!CLS_ACL_TABLE_PINTER(3, TCAM, tbl, valid);
			*compareData = CLS_ACL_TABLE_PINTER(3, TCAM, tbl, compareData);
		}
	}
}

// Get the type of new acl0 table and table index
//tbl_type: 0-large 1-small 2-tcam
int dubhe2000_ingress_aclN_get_free_index(u8 aclN, u8 *hashkey, u8 *tbl_type)
{
	u16 index;
	u8 bucket_size, hash_size, is_valid;
	u8 large_tbl[64], small_tbl[64], tcam_tbl[256]; // should cover all acl table
	u8 *compareData;
	int i;

	//check aclN/tbl_type
	if (aclN >= ARRAY_SIZE(acl_hash_settings)) {
		pr_err("[%s] invalid ACLN\n", __func__);
		return -EINVAL;
	}

	//check large table
	if (acl_hash_settings[aclN].large_hash_size) {
		bucket_size = acl_hash_settings[aclN].large_bucket_size;
		hash_size = acl_hash_settings[aclN].large_hash_size;
		index = dubhe1000_acl_hashval_cal(aclN, hashkey, 1);

		for (i = 0; i <= BITS_32(0, bucket_size - 1); i++) {
			memset(large_tbl, 0, sizeof(large_tbl));
			cls_switch_get_acl_tbl(aclN, SWITCH_ACL_TBL_TYPE_LARGE, index + (i << hash_size), large_tbl,
					       &compareData, &is_valid);

			if (!is_valid) {
				*tbl_type = SWITCH_ACL_TBL_TYPE_LARGE;

				return (index + (i << hash_size)); //empty entry found
			}
		}
	}

	//check small table
	if (acl_hash_settings[aclN].small_hash_size) {
		bucket_size = acl_hash_settings[aclN].small_bucket_size;
		hash_size = acl_hash_settings[aclN].small_hash_size;
		index = dubhe1000_acl_hashval_cal(aclN, hashkey, 0);

		for (i = 0; i <= BITS_32(0, bucket_size - 1); i++) {
			memset(small_tbl, 0, sizeof(small_tbl));
			cls_switch_get_acl_tbl(aclN, SWITCH_ACL_TBL_TYPE_SMALL, index + (i << hash_size), small_tbl,
					       &compareData, &is_valid);

			if (!is_valid) {
				*tbl_type = SWITCH_ACL_TBL_TYPE_SMALL;

				return (index + (i << hash_size)); //empty entry found
			}
		}
	}

	//check tcam table
	for (i = 0; i < acl_max_entry[aclN][2]; i++) {
		memset(tcam_tbl, 0, sizeof(tcam_tbl));
		cls_switch_get_acl_tbl(aclN, SWITCH_ACL_TBL_TYPE_TCAM, i, tcam_tbl, &compareData, &is_valid);

		if (!is_valid) {
			*tbl_type = SWITCH_ACL_TBL_TYPE_TCAM;

			return i; //empty entry found
		}
	}

	return -EINVAL;
}

int dubhe2000_ingress_aclN_get_existed_index(u8 aclN, u8 *hashkey, u8 *mask, u8 *tbl_type)
{
	u16 index;
	u8 bucket_size, hash_size, size_u8, is_valid;
	u8 large_tbl[64], small_tbl[64], tcam_tbl[256]; // should cover all acl table
	u8 *compareData;
	int i;

	//check aclN/tbl_type
	if (aclN >= ARRAY_SIZE(acl_hash_settings)) {
		pr_err("[%s] invalid ACLN\n", __func__);
		return -EINVAL;
	}

	size_u8 = acl_hash_settings[aclN].compareData_u8_size;

	//check large table
	if (acl_hash_settings[aclN].large_hash_size) {
		bucket_size = acl_hash_settings[aclN].large_bucket_size;
		hash_size = acl_hash_settings[aclN].large_hash_size;
		index = dubhe1000_acl_hashval_cal(aclN, hashkey, 1);

		for (i = 0; i <= BITS_32(0, bucket_size - 1); i++) {
			memset(large_tbl, 0, sizeof(large_tbl));
			cls_switch_get_acl_tbl(aclN, SWITCH_ACL_TBL_TYPE_LARGE, index + (i << hash_size), large_tbl,
					       &compareData, &is_valid);

			if (is_valid && !memcmp(hashkey, compareData, size_u8)) {
				*tbl_type = SWITCH_ACL_TBL_TYPE_LARGE;

				return (index + (i << hash_size)); //empty entry found
			}
		}
	}

	//check small table
	if (acl_hash_settings[aclN].small_hash_size) {
		bucket_size = acl_hash_settings[aclN].small_bucket_size;
		hash_size = acl_hash_settings[aclN].small_hash_size;
		index = dubhe1000_acl_hashval_cal(aclN, hashkey, 0);

		for (i = 0; i <= BITS_32(0, bucket_size - 1); i++) {
			memset(small_tbl, 0, sizeof(small_tbl));
			cls_switch_get_acl_tbl(aclN, SWITCH_ACL_TBL_TYPE_SMALL, index + (i << hash_size), small_tbl,
					       &compareData, &is_valid);

			if (is_valid && !memcmp(hashkey, compareData, size_u8)) {
				*tbl_type = SWITCH_ACL_TBL_TYPE_SMALL;

				return (index + (i << hash_size)); //empty entry found
			}
		}
	}

	//check tcam table
	for (i = 0; i < acl_max_entry[aclN][2]; i++) {
		memset(tcam_tbl, 0, sizeof(tcam_tbl));
		cls_switch_get_acl_tbl(aclN, SWITCH_ACL_TBL_TYPE_TCAM, i, tcam_tbl, &compareData, &is_valid);

		//TODO: we should compare mask in acl2/acl3?
		if (is_valid && !memcmp(hashkey, compareData, size_u8)) {
			*tbl_type = SWITCH_ACL_TBL_TYPE_TCAM;

			return i; //empty entry found
		}
	}

	return -EINVAL;
}

// Ingress ACL0 - NAT
void dubhe1000_ingress_acl0_config_nat(bool is_snat, u16 index, u8 tbl_type, u8 *hashkey, u16 nat_index,
				       const struct hw_nat_key *key, const struct hw_nat_act *act)
{
	t_IngressConfigurableACL0LargeTable large_tbl;
	t_IngressConfigurableACL0SmallTable small_tbl;
	t_IngressConfigurableACL0TCAM tcam_tbl;
	t_IngressConfigurableACL0TCAMAnswer tcam_answer;

	if (tbl_type == SWITCH_ACL_TBL_TYPE_LARGE) {
		memset(&large_tbl, 0, sizeof(large_tbl));

		large_tbl.valid = 1;
		memcpy(large_tbl.compareData, hashkey, sizeof(large_tbl.compareData));
		large_tbl.natOpValid = 1;
		large_tbl.natOpPtr = nat_index;

		if (act->opt_queue) {
			large_tbl.forceQueue = 1;
			large_tbl.eQueue =
				act->opt_queue == 1 ? act->queue_value : g_adapter->dscp_map[act->dscp_value].queue;
		}

		if (act->replace_dscp == 1) {
			large_tbl.updateTosExp = 1;
			large_tbl.tosMask = 0xfc;
			large_tbl.newTosExp = act->dscp_value << 2;
		}

		if (!is_snat) {
			// update ipaddr
			if (act->replace_addr) {
				large_tbl.enableUpdateIp = 1;
				large_tbl.newIpValue = ntohl(act->addr);
				large_tbl.updateSaOrDa = !act->replace_src;
			}

			// update port
			if (act->replace_port) {
				large_tbl.enableUpdateL4 = 1;
				large_tbl.newL4Value = ntohs(act->port);
				large_tbl.updateL4SpOrDp = !act->replace_src;
			}
		}
		wr_IngressConfigurableACL0LargeTable(g_adapter, index, &large_tbl);
	} else if (tbl_type == SWITCH_ACL_TBL_TYPE_SMALL) {
		memset(&small_tbl, 0, sizeof(small_tbl));

		small_tbl.valid = 1;
		memcpy(small_tbl.compareData, hashkey, sizeof(small_tbl.compareData));
		small_tbl.natOpValid = 1;
		small_tbl.natOpPtr = nat_index;

		if (act->opt_queue) {
			small_tbl.forceQueue = 1;
			small_tbl.eQueue =
				act->opt_queue == 1 ? act->queue_value : g_adapter->dscp_map[act->dscp_value].queue;
		}

		if (act->replace_dscp == 1) {
			small_tbl.updateTosExp = 1;
			small_tbl.tosMask = 0xfc;
			small_tbl.newTosExp = act->dscp_value << 2;
		}

		if (!is_snat) {
			// update ipaddr
			if (act->replace_addr) {
				small_tbl.enableUpdateIp = 1;
				small_tbl.newIpValue = ntohl(act->addr);
				small_tbl.updateSaOrDa = !act->replace_src;
			}

			// update port
			if (act->replace_port) {
				small_tbl.enableUpdateL4 = 1;
				small_tbl.newL4Value = ntohs(act->port);
				small_tbl.updateL4SpOrDp = !act->replace_src;
			}
		}

		wr_IngressConfigurableACL0SmallTable(g_adapter, index, &small_tbl);
	} else if (tbl_type == SWITCH_ACL_TBL_TYPE_TCAM) {
		memset(&tcam_tbl, 0, sizeof(tcam_tbl));
		memset(&tcam_answer, 0, sizeof(tcam_answer));

		tcam_tbl.valid = 1;
		memset(tcam_tbl.mask, 0xff, sizeof(tcam_tbl.mask));
		memcpy(tcam_tbl.compareData, hashkey, sizeof(tcam_tbl.compareData));
		wr_IngressConfigurableACL0TCAM(g_adapter, index, &tcam_tbl);

		tcam_answer.natOpValid = 1;
		tcam_answer.natOpPtr = nat_index;

		if (act->opt_queue) {
			tcam_answer.forceQueue = 1;
			tcam_answer.eQueue =
				act->opt_queue == 1 ? act->queue_value : g_adapter->dscp_map[act->dscp_value].queue;
		}

		if (act->replace_dscp == 1) {
			tcam_answer.updateTosExp = 1;
			tcam_answer.tosMask = 0xfc;
			tcam_answer.newTosExp = act->dscp_value << 2;
		}

		if (!is_snat) {
			// update ipaddr
			if (act->replace_addr) {
				tcam_answer.enableUpdateIp = 1;
				tcam_answer.newIpValue = ntohl(act->addr);
				tcam_answer.updateSaOrDa = !act->replace_src;
			}

			// update port
			if (act->replace_port) {
				tcam_answer.enableUpdateL4 = 1;
				tcam_answer.newL4Value = ntohs(act->port);
				tcam_answer.updateL4SpOrDp = !act->replace_src;
			}
		}

		wr_IngressConfigurableACL0TCAMAnswer(g_adapter, index, &tcam_answer);

	} else { //never happened
		pr_err("[%s] invalid ACL0 Table Type\n", __func__);
	}
}

void dubhe2000_ingress_acl0_config_ipv6_router(u16 index, u8 tbl_type, u8 *hashkey, u16 nat_index,
					       const struct hw_ipv6_key *key, const struct hw_ipv6_act *act)
{
	t_IngressConfigurableACL0LargeTable large_tbl;
	t_IngressConfigurableACL0SmallTable small_tbl;
	t_IngressConfigurableACL0TCAM tcam_tbl;
	t_IngressConfigurableACL0TCAMAnswer tcam_answer;

	if (tbl_type == SWITCH_ACL_TBL_TYPE_LARGE) {
		memset(&large_tbl, 0, sizeof(large_tbl));

		large_tbl.valid = 1;
		memcpy(large_tbl.compareData, hashkey, sizeof(large_tbl.compareData));
		large_tbl.natOpValid = 1;
		large_tbl.natOpPtr = nat_index;

		if (act->opt_queue) {
			large_tbl.forceQueue = 1;
			large_tbl.eQueue =
				act->opt_queue == 1 ? act->queue_value : g_adapter->dscp_map[act->dscp_value].queue;
		}

		if (act->replace_dscp == 1) {
			large_tbl.updateTosExp = 1;
			large_tbl.tosMask = 0xfc;
			large_tbl.newTosExp = act->dscp_value << 2;
		}

		wr_IngressConfigurableACL0LargeTable(g_adapter, index, &large_tbl);
	} else if (tbl_type == SWITCH_ACL_TBL_TYPE_SMALL) {
		memset(&small_tbl, 0, sizeof(small_tbl));

		small_tbl.valid = 1;
		memcpy(small_tbl.compareData, hashkey, sizeof(small_tbl.compareData));
		small_tbl.natOpValid = 1;
		small_tbl.natOpPtr = nat_index;

		if (act->opt_queue) {
			small_tbl.forceQueue = 1;
			small_tbl.eQueue =
				act->opt_queue == 1 ? act->queue_value : g_adapter->dscp_map[act->dscp_value].queue;
		}

		if (act->replace_dscp == 1) {
			small_tbl.updateTosExp = 1;
			small_tbl.tosMask = 0xfc;
			small_tbl.newTosExp = act->dscp_value << 2;
		}

		wr_IngressConfigurableACL0SmallTable(g_adapter, index, &small_tbl);
	} else if (tbl_type == SWITCH_ACL_TBL_TYPE_TCAM) {
		memset(&tcam_tbl, 0, sizeof(tcam_tbl));
		memset(&tcam_answer, 0, sizeof(tcam_answer));

		tcam_tbl.valid = 1;
		memset(tcam_tbl.mask, 0xff, sizeof(tcam_tbl.mask));
		memcpy(tcam_tbl.compareData, hashkey, sizeof(tcam_tbl.compareData));
		wr_IngressConfigurableACL0TCAM(g_adapter, index, &tcam_tbl);

		tcam_answer.natOpValid = 1;
		tcam_answer.natOpPtr = nat_index;

		if (act->opt_queue) {
			tcam_answer.forceQueue = 1;
			tcam_answer.eQueue =
				act->opt_queue == 1 ? act->queue_value : g_adapter->dscp_map[act->dscp_value].queue;
		}

		if (act->replace_dscp == 1) {
			tcam_answer.updateTosExp = 1;
			tcam_answer.tosMask = 0xfc;
			tcam_answer.newTosExp = act->dscp_value << 2;
		}

		wr_IngressConfigurableACL0TCAMAnswer(g_adapter, index, &tcam_answer);
	} else {
		pr_err("[%s] invalid ACL0 Table Type\n", __func__);
	}
}

// Ingress ACL1 - IPV4 (non NAT)
void dubhe1000_ingress_acl1_config(u16 index, u8 tbl_type, u8 *hashkey, const struct hw_acl_match *match,
				   const struct hw_acl_act *act)
{
	t_IngressConfigurableACL1LargeTable large_tbl;
	t_IngressConfigurableACL1SmallTable small_tbl;
	t_IngressConfigurableACL1TCAM tcam_tbl;
	t_IngressConfigurableACL1TCAMAnswer tcam_answer;

	if (tbl_type == SWITCH_ACL_TBL_TYPE_LARGE) {
		memset(&large_tbl, 0, sizeof(large_tbl));

		large_tbl.valid = 1;
		memcpy(large_tbl.compareData, hashkey, sizeof(large_tbl.compareData));

		wr_IngressConfigurableACL1LargeTable(g_adapter, index, &large_tbl);
	} else if (tbl_type == SWITCH_ACL_TBL_TYPE_SMALL) {
		memset(&small_tbl, 0, sizeof(small_tbl));

		small_tbl.valid = 1;
		memcpy(small_tbl.compareData, hashkey, sizeof(small_tbl.compareData));

		wr_IngressConfigurableACL1SmallTable(g_adapter, index, &small_tbl);
	} else if (tbl_type == SWITCH_ACL_TBL_TYPE_TCAM) {
		memset(&tcam_tbl, 0, sizeof(tcam_tbl));
		memset(&tcam_answer, 0, sizeof(tcam_answer));

		tcam_tbl.valid = 1;
		memset(tcam_tbl.mask, 0xff, sizeof(tcam_tbl.mask));
		memcpy(tcam_tbl.compareData, hashkey, sizeof(tcam_tbl.compareData));
		wr_IngressConfigurableACL1TCAM(g_adapter, index, &tcam_tbl);

		wr_IngressConfigurableACL1TCAMAnswer(g_adapter, index, &tcam_answer);
	} else {
		pr_err("[%s] invalid ACL Table Type\n", __func__);
	}
}

// Ingress ACL2 - IPV6 (non Routing) only TCAM
void dubhe1000_ingress_acl2_config(u16 index, u8 tbl_type, u8 *hashkey, u8 *mask, const struct hw_acl_match *match,
				   const struct hw_acl_act *act)
{
	t_IngressConfigurableACL2TCAM tcam;
	t_IngressConfigurableACL2TCAMAnswer answer;

	//TCAM
	memset(&tcam, 0, sizeof(tcam));

	tcam.valid = 1;
	memcpy(tcam.mask, hashkey, sizeof(tcam.mask));
	memcpy(tcam.compareData, mask, sizeof(tcam.compareData));

	wr_IngressConfigurableACL2TCAM(g_adapter, index, &tcam);

	//Answer
	memset(&answer, 0, sizeof(answer));

	//TODO: parse act and update answer

	wr_IngressConfigurableACL2TCAMAnswer(g_adapter, index, &answer);
}

// Ingress ACL3 - special packet only TCAM
void dubhe1000_ingress_acl3_config(u16 index, u8 tbl_type, u8 *hashkey, u8 *mask, const struct hw_acl_match *match,
				   const struct hw_acl_act *act)
{
	t_IngressConfigurableACL3TCAM tcam;
	t_IngressConfigurableACL3TCAMAnswer answer;

	//TCAM
	memset(&tcam, 0, sizeof(tcam));

	tcam.valid = 1;
	memcpy(tcam.mask, hashkey, sizeof(tcam.mask));
	memcpy(tcam.compareData, mask, sizeof(tcam.compareData));

	wr_IngressConfigurableACL3TCAM(g_adapter, index, &tcam);

	//Answer
	memset(&answer, 0, sizeof(answer));

	//TODO: parse act and update answer

	wr_IngressConfigurableACL3TCAMAnswer(g_adapter, index, &answer);
}

//the ACL rule pointer of IPV4_NAT is 0
//the ACL rule pointer of IPV6_ROUTER is 1
void dubhe2000_acl0_pre_lookup_init_config(void)
{
	t_IngressConfigurableACL0PreLookup data;
	u8 index, prelookupaclbits, l3_type;

	//the ACL rule pointer of IPV4_NAT is 0
	prelookupaclbits = 0; // Not yet enabled.
	l3_type = 0;	      // ipv4
	index = prelookupaclbits + (l3_type << 2);
	data.valid = 1;
	data.rulePtr = 0;
	wr_IngressConfigurableACL0PreLookup(g_adapter, index, &data);

	//the ACL rule pointer of IPV6_ROUTER is 1
	prelookupaclbits = 0; // Not yet enabled
	l3_type = 1;	      // ipv6
	index = prelookupaclbits + (l3_type << 2);
	data.valid = 1;
	data.rulePtr = 1;
	wr_IngressConfigurableACL0PreLookup(g_adapter, index, &data);
}

/* ingress acl0 for ipv4 nat and ipv6 router*/
void dubhe2000_acl0_init_config(void)
{
	t_IngressConfigurableACL0RulesSetup acl0_rule;
	t_IngressConfigurableACL0SearchMask acl0_mask;
	t_IngressConfigurableACL0Selection acl0_sel;
	u8 rule_pointer;

	dubhe2000_acl0_pre_lookup_init_config();

	/* Note: there is not quintet(5 elements), but 6 elements.
	 * switch's source port will be added in acl rules.
	 * The purpose of doing this is to make the upper level aware of portmove
	 */

	// ipv4 nat: sip/dip/sport/dport/l4_protocol/source_port
	rule_pointer = 0;
	memset(&acl0_rule, 0, sizeof(t_IngressConfigurableACL0RulesSetup));
	acl0_rule.fieldSelectBitmask = 1 << 2;	 //IPv4 SA
	acl0_rule.fieldSelectBitmask += 1 << 3;	 //IPv4 DA
	acl0_rule.fieldSelectBitmask += 1 << 6;	 //L4 Source Port
	acl0_rule.fieldSelectBitmask += 1 << 7;	 //L4 Destination Port
	acl0_rule.fieldSelectBitmask += 1 << 8;	 //L4 Protocol
	acl0_rule.fieldSelectBitmask += 1 << 12; //Switch Source Port
	wr_IngressConfigurableACL0RulesSetup(g_adapter, rule_pointer, &acl0_rule);

	// ipv6 router: sip/dip/l4_protocol/sport/dport
	rule_pointer = 1;
	memset(&acl0_rule, 0, sizeof(t_IngressConfigurableACL0RulesSetup));
	acl0_rule.fieldSelectBitmask = 1 << 4;	 //IPv6 SA
	acl0_rule.fieldSelectBitmask += 1 << 5;	 //IPv6 DA
	acl0_rule.fieldSelectBitmask += 1 << 6;	 //L4 Source Port
	acl0_rule.fieldSelectBitmask += 1 << 7;	 //L4 Destination Port
	acl0_rule.fieldSelectBitmask += 1 << 8;	 //L4 Protocol
	acl0_rule.fieldSelectBitmask += 1 << 12; //Switch Source Port
	wr_IngressConfigurableACL0RulesSetup(g_adapter, rule_pointer, &acl0_rule);

	//acl0 mask: Note: TCAM mask will be uodated when config TCAM compareData, not here
	memset(acl0_mask.masklarge, 0xFF, sizeof(acl0_mask.masklarge));
	memset(acl0_mask.masksmall, 0xFF, sizeof(acl0_mask.masksmall));
	wr_IngressConfigurableACL0SearchMask(g_adapter, &acl0_mask);

	acl0_sel.selectTcamOrTable = 0;	 // select Tcam
	acl0_sel.selectSmallOrLarge = 0; // select Small
	wr_IngressConfigurableACL0Selection(g_adapter, &acl0_sel);
}

void dubhe2000_acl1_pre_lookup_init_config(void)
{
	t_IngressConfigurableACL1PreLookup data;
	u16 index;
	u8 prelookupaclbits, l3_type, l4_type, vlans;

	prelookupaclbits = 0; // Not yet enabled.
	l3_type = 0;	      // ipv4
	data.valid = 1;
	data.rulePtr = 0;
	for (vlans = 0; vlans < 4; vlans++) {
		for (l4_type = 0; l4_type < 8; l4_type++) {
			index = prelookupaclbits + (vlans << 2) + (l3_type << 4) + (l4_type << 6);
			wr_IngressConfigurableACL1PreLookup(g_adapter, index, &data);
		}
	}

	prelookupaclbits = 0; // Not yet enabled.
	l3_type = 1;	      // ipv6
	data.valid = 1;
	data.rulePtr = 0;
	for (vlans = 0; vlans < 4; vlans++) {
		for (l4_type = 0; l4_type < 8; l4_type++) {
			index = prelookupaclbits + (vlans << 2) + (l3_type << 4) + (l4_type << 6);
			wr_IngressConfigurableACL1PreLookup(g_adapter, index, &data);
		}
	}

	prelookupaclbits = 0; // Not yet enabled.
	l3_type = 3;	      // Not IPv4, IPv6 or MPLS
	data.valid = 1;
	data.rulePtr = 1;
	for (vlans = 0; vlans < 4; vlans++) {
		for (l4_type = 0; l4_type < 8; l4_type++) {
			index = prelookupaclbits + (vlans << 2) + (l3_type << 4) + (l4_type << 6);
			wr_IngressConfigurableACL1PreLookup(g_adapter, index, &data);
		}
	}
}

/* ingress acl1: other case and action case >= acl0 */
void dubhe2000_acl1_init_config(void)
{
	t_IngressConfigurableACL1RulesSetup acl1_rule;
	t_IngressConfigurableACL1SearchMask acl1_mask;
	u8 rule_pointer;

	dubhe2000_acl1_pre_lookup_init_config();

	rule_pointer = 0;
	memset(&acl1_rule, 0, sizeof(t_IngressConfigurableACL1RulesSetup));
	/* for IP packet. And ipv6, IPv4 address is unused */
	acl1_rule.fieldSelectBitmask += 1 << 12; //IPv4 SA
	acl1_rule.fieldSelectBitmask += 1 << 13; //IPv4 DA
	acl1_rule.fieldSelectBitmask += 1 << 19; //L4 SPORT
	acl1_rule.fieldSelectBitmask += 1 << 20; //L4 DPORT
	acl1_rule.fieldSelectBitmask += 1 << 27; //L4 Protocol
	acl1_rule.fieldSelectBitmask += 1 << 31; //SrcPort
	wr_IngressConfigurableACL1RulesSetup(g_adapter, rule_pointer, &acl1_rule);

	rule_pointer = 1;
	memset(&acl1_rule, 0, sizeof(t_IngressConfigurableACL1RulesSetup));
	/* for non-ip Packet */
	acl1_rule.fieldSelectBitmask += 1 << 1;	 //MAC DA
	acl1_rule.fieldSelectBitmask += 1 << 2;	 //MAC SA
	acl1_rule.fieldSelectBitmask += 1 << 4;	 //Has VLANs
	acl1_rule.fieldSelectBitmask += 1 << 5;	 //Outer VLAN Tag Type
	acl1_rule.fieldSelectBitmask += 1 << 6;	 //Inner VLAN Tag Type
	acl1_rule.fieldSelectBitmask += 1 << 28; //Ethernet Type
	acl1_rule.fieldSelectBitmask += 1 << 31; //SrcPort
	wr_IngressConfigurableACL1RulesSetup(g_adapter, rule_pointer, &acl1_rule);

	//acl1 mask: Note: TCAM mask will be updated when config TCAM compareData, not here
	memset(acl1_mask.masklarge, 0xFF, sizeof(acl1_mask.masklarge));
	memset(acl1_mask.masksmall, 0xFF, sizeof(acl1_mask.masksmall));
	wr_IngressConfigurableACL1SearchMask(g_adapter, &acl1_mask);
}

void dubhe2000_acl2_pre_lookup_init_config(void)
{
	t_IngressConfigurableACL2PreLookup data;
	u8 index, prelookupaclbits, l3_type, vlans;

	//the ACL rule pointer of IPV4 is 0
	prelookupaclbits = 0; // Not yet enabled.
	data.valid = 1;
	data.rulePtr = 0;
	l3_type = 0; // ipv4
	for (vlans = 0; vlans < 4; vlans++) {
		index = prelookupaclbits + (vlans << 2) + (l3_type << 4);
		wr_IngressConfigurableACL2PreLookup(g_adapter, index, &data);
	}

	//the ACL rule pointer of IPV6 is 1
	prelookupaclbits = 0; // Not yet enabled.
	l3_type = 1;	      // ipv6
	data.valid = 1;
	data.rulePtr = 1;
	for (vlans = 0; vlans < 4; vlans++) {
		index = prelookupaclbits + (vlans << 2) + (l3_type << 4);
		wr_IngressConfigurableACL2PreLookup(g_adapter, index, &data);
	}
}

/* ingress acl2: ipv6(non router) only can use acl2 */
void dubhe2000_acl2_init_config(void)
{
	t_IngressConfigurableACL2RulesSetup acl2_rule;
	u8 rule_pointer;

	dubhe2000_acl2_pre_lookup_init_config();

	rule_pointer = 0; //IPv4
	memset(&acl2_rule, 0, sizeof(t_IngressConfigurableACL2RulesSetup));
	//acl2_rule.fieldSelectBitmask += 1 << 0;//TCP Flags
	acl2_rule.fieldSelectBitmask += 1 << 1;	 //MAC DA
	acl2_rule.fieldSelectBitmask += 1 << 2;	 //MAC SA
	acl2_rule.fieldSelectBitmask += 1 << 3;	 //Outer VID
	acl2_rule.fieldSelectBitmask += 1 << 4;	 //Has VLANs
	acl2_rule.fieldSelectBitmask += 1 << 5;	 //Outer VLAN Tag Type
	acl2_rule.fieldSelectBitmask += 1 << 6;	 //Inner VLAN Tag Type
	acl2_rule.fieldSelectBitmask += 1 << 7;	 //Outer PCP
	acl2_rule.fieldSelectBitmask += 1 << 8;	 //Outer DEI
	acl2_rule.fieldSelectBitmask += 1 << 9;	 //Inner VID
	acl2_rule.fieldSelectBitmask += 1 << 10; //Inner PCP
	acl2_rule.fieldSelectBitmask += 1 << 11; //Inner DEI
	acl2_rule.fieldSelectBitmask += 1 << 12; //IPv4 SA
	acl2_rule.fieldSelectBitmask += 1 << 13; //IPv4 DA
	//acl2_rule.fieldSelectBitmask += 1 << 14;//IPv6 SA
	//acl2_rule.fieldSelectBitmask += 1 << 15;//IPv6 DA
	//acl2_rule.fieldSelectBitmask += 1 << 16;//Outer MPLS
	acl2_rule.fieldSelectBitmask += 1 << 17; //TOS
	//acl2_rule.fieldSelectBitmask += 1 << 18;//TTL
	acl2_rule.fieldSelectBitmask += 1 << 19; //L4 Source Port
	acl2_rule.fieldSelectBitmask += 1 << 20; //L4 Destination Port
	//acl2_rule.fieldSelectBitmask += 1 << 21;//IPv6 Flow Label
	acl2_rule.fieldSelectBitmask += 1 << 22; //L4 Protocol
	/*
	 * EType: already known IP, there is a speical cases:
	 * MAC + VLAN + pppoe + IP
	 * If tunnel work, Ethernet Type is IP. If not, etype is pppoe
	 */
	//acl2_rule.fieldSelectBitmask += 1 << 23;//Ethernet Type
	acl2_rule.fieldSelectBitmask += 1 << 24; //L4 Type: L4 Protocol can cover all cases
	//acl2_rule.fieldSelectBitmask += 1 << 25;//L3 Type: already known IPV4 from pre lookup
	acl2_rule.fieldSelectBitmask += 1 << 26; //Source Port
	//acl2_rule.fieldSelectBitmask += 1 << 27;//Rule Pointer
	wr_IngressConfigurableACL2RulesSetup(g_adapter, rule_pointer, &acl2_rule);

	rule_pointer = 1; //IPV6
	memset(&acl2_rule, 0, sizeof(t_IngressConfigurableACL2RulesSetup));
	//acl2_rule.fieldSelectBitmask += 1 << 0;//TCP Flags
	acl2_rule.fieldSelectBitmask += 1 << 1;	 //MAC DA
	acl2_rule.fieldSelectBitmask += 1 << 2;	 //MAC SA
	acl2_rule.fieldSelectBitmask += 1 << 3;	 //Outer VID
	acl2_rule.fieldSelectBitmask += 1 << 4;	 //Has VLANs
	acl2_rule.fieldSelectBitmask += 1 << 5;	 //Outer VLAN Tag Type
	acl2_rule.fieldSelectBitmask += 1 << 6;	 //Inner VLAN Tag Type
	acl2_rule.fieldSelectBitmask += 1 << 7;	 //Outer PCP
	acl2_rule.fieldSelectBitmask += 1 << 8;	 //Outer DEI
	acl2_rule.fieldSelectBitmask += 1 << 9;	 //Inner VID
	acl2_rule.fieldSelectBitmask += 1 << 10; //Inner PCP
	acl2_rule.fieldSelectBitmask += 1 << 11; //Inner DEI
	//acl2_rule.fieldSelectBitmask += 1 << 12;//IPv4 SA
	//acl2_rule.fieldSelectBitmask += 1 << 13;//IPv4 DA
	acl2_rule.fieldSelectBitmask += 1 << 14; //IPv6 SA
	acl2_rule.fieldSelectBitmask += 1 << 15; //IPv6 DA
	//acl2_rule.fieldSelectBitmask += 1 << 16;//Outer MPLS
	acl2_rule.fieldSelectBitmask += 1 << 17; //TOS
	//acl2_rule.fieldSelectBitmask += 1 << 18;//TTL
	acl2_rule.fieldSelectBitmask += 1 << 19; //L4 Source Port
	acl2_rule.fieldSelectBitmask += 1 << 20; //L4 Destination Port
	acl2_rule.fieldSelectBitmask += 1 << 21; //IPv6 Flow Label
	acl2_rule.fieldSelectBitmask += 1 << 22; //L4 Protocol
	/*
	 * EType: already known IPV6, there is a speical cases:
	 * MAC + VLAN + pppoe + IP
	 * If tunnel work, Ethernet Type is IP. If not, etype is pppoe
	 */
	//acl2_rule.fieldSelectBitmask += 1 << 23;//Ethernet Type
	acl2_rule.fieldSelectBitmask += 1 << 24; //L4 Type: L4 Protocol can cover all cases
	//acl2_rule.fieldSelectBitmask += 1 << 25;//L3 Type: already known IPV4 from pre lookup
	acl2_rule.fieldSelectBitmask += 1 << 26; //Source Port
	//acl2_rule.fieldSelectBitmask += 1 << 27;//Rule Pointer
	wr_IngressConfigurableACL2RulesSetup(g_adapter, rule_pointer, &acl2_rule);

	rule_pointer = 2; // for non-ip or MPLS?
	memset(&acl2_rule, 0, sizeof(t_IngressConfigurableACL2RulesSetup));
	wr_IngressConfigurableACL2RulesSetup(g_adapter, rule_pointer, &acl2_rule);

	//acl2 mask: Note: TCAM mask will be updated when config TCAM compareData, not here
}

/* drop/send2port for special packet type */
void dubhe2000_acl3_init_config(void)
{
	t_IngressConfigurableACL3RulesSetup acl3_rule;
	u8 rule_pointer;

	rule_pointer = 0;

	memset(&acl3_rule, 0, sizeof(t_IngressConfigurableACL3RulesSetup));

	// Width of Search Data is 80. we should ignore L4 SPort/L4 DPort(acl1/acl2 will cover it)
	acl3_rule.fieldSelectBitmask = 1 << 0;	//L2 Packet Flags
	acl3_rule.fieldSelectBitmask += 1 << 1; //IPv4 Option
	acl3_rule.fieldSelectBitmask += 1 << 2; //TCP Flags
	//acl3_rule.fieldSelectBitmask += 1 << 3;//TOS
	//acl3_rule.fieldSelectBitmask += 1 << 4;//L4 SPORT
	//acl3_rule.fieldSelectBitmask += 1 << 5;//L4 DPORT
	acl3_rule.fieldSelectBitmask += 1 << 6; //L4 Protocol
	acl3_rule.fieldSelectBitmask += 1 << 7; //EthernetType
	acl3_rule.fieldSelectBitmask += 1 << 8; //L4 Type
	acl3_rule.fieldSelectBitmask += 1 << 9; //L3 Type

	wr_IngressConfigurableACL3RulesSetup(g_adapter, rule_pointer, &acl3_rule);

	//acl3 mask: Note: TCAM mask will be updated when config TCAM compareData, not here
}

void dubhe2000_switch_ingress_aclN_init(struct dubhe1000_adapter *adapter)
{
	int i;
	t_SourcePortTable source_port;

	//step1: config source_port_table:useaclN/aclRuleN
	for (i = 0; i <= DUBHE2000_PORT_MAX; i++) {
		memset(&source_port, 0, sizeof(source_port));
		rd_SourcePortTable(adapter, i, &source_port);

		source_port.useAcl0 = 1;
		//invalid acl0 rule pointer, will be re-writed by pre_lookup
		source_port.aclRule0 = IngressConfigurableACL0RulesSetup_nr_entries - 1;

		if (i != DUBHE2000_CPU_PORT) {
			source_port.useAcl1 = 1;
			//invalid acl1 rule pointer, will be re-writed by pre_lookup
			source_port.aclRule1 = IngressConfigurableACL1RulesSetup_nr_entries - 1;

			source_port.useAcl2 = 1;
			//invalid acl2 rule pointer, will be re-writed by pre_lookup
			source_port.aclRule2 = IngressConfigurableACL2RulesSetup_nr_entries - 1;

			source_port.useAcl3 = 1;
			source_port.aclRule3 = 0;
		}

		wr_SourcePortTable(adapter, i, &source_port);
	}

	//setp2: config acl engine
	dubhe2000_acl0_init_config();
	dubhe2000_acl1_init_config();
	dubhe2000_acl2_init_config();
	dubhe2000_acl3_init_config();
}
