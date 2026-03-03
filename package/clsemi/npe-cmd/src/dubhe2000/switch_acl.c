// SPDX-License-Identifier: GPL-2.0
// Copyright(c) 2023 Clourneysemi Corporation.

#include "npe_switch.h"
#include "switch_conf.h"
#include "switch_acl.h"
#include <arpa/inet.h>

/* NOTE: this enum should sync with switch_acl_fileds_all */
enum {
	ACL_FIELD_MAC_DA,
	ACL_FIELD_MAC_SA,
	ACL_FIELD_IPV4_SA,
	ACL_FIELD_IPV4_DA,
	ACL_FIELD_IPV6_SA,
	ACL_FIELD_IPV6_DA,
	ACL_FIELD_L4_Source_Port,
	ACL_FIELD_L4_Destination_Port,
	ACL_FIELD_L4_Protocol,
	ACL_FIELD_Ethernet_Type,
	ACL_FIELD_L4_Type,
	ACL_FIELD_L3_Type,
	ACL_FIELD_Source_Port,
	ACL_FIELD_From_Crypto,
	ACL_FIELD_From_Crypto_Operation_Data,
	ACL_FIELD_From_Crypto_Security_Association_Ptr,
	ACL_FIELD_Rule_Pointer,
	ACL_FIELD_TCP_Flags,
	ACL_FIELD_Outer_VID,
	ACL_FIELD_Has_VLANs,
	ACL_FIELD_Outer_VLAN_Tag_Type,
	ACL_FIELD_Inner_VLAN_Tag_Type,
	ACL_FIELD_Outer_PCP,
	ACL_FIELD_Outer_DEI,
	ACL_FIELD_Inner_VID,
	ACL_FIELD_Inner_PCP,
	ACL_FIELD_Inner_DEI,
	ACL_FIELD_Outer_MPLS,
	ACL_FIELD_TOS,
	ACL_FIELD_TTL,
	ACL_FIELD_IPSEC_SPI,
	ACL_FIELD_IPSEC_SEQ,
	ACL_FIELD_MACsec_Key,
	ACL_FIELD_MACsec_Control_Byte,
	ACL_FIELD_MLD_Address,
	ACL_FIELD_ICMP_Type,
	ACL_FIELD_ICMP_Code,
	ACL_FIELD_IGMP_Type,
	ACL_FIELD_IGMP_Group_Address,
	ACL_FIELD_IPv6_Flow_Label,
	ACL_FIELD_L2_Packet_Flags,
	ACL_FIELD_IPv4_Options,
	ACL_FIELD_GID,
	ACL_FIELD_VID,
	ACL_FIELD_L2_Multicast_Pointer,
	ACL_FIELD_Destination_Portmask,
	ACL_FIELD_L2_Packet_Processing_Flags,
	// Add a new item at the end 
};

struct acl_field_info switch_acl_fileds_info_all[] = {
	{"MAC_DA", 48, SWITCH_FIELD_TYPE_MACADDR},
	{"MAC_SA", 48, SWITCH_FIELD_TYPE_MACADDR},
	{"IPv4_SA", 32, SWITCH_FIELD_TYPE_IPV4},
	{"IPv4_DA", 32, SWITCH_FIELD_TYPE_IPV4},
	{"IPv6_SA", 128, SWITCH_FIELD_TYPE_IPV6},
	{"IPv6_DA", 128, SWITCH_FIELD_TYPE_IPV6},
	{"L4_Source_Port", 16, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"L4_Destination_Port", 16, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"L4_Protocol", 8, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"Ethernet_Type", 16, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"L4_Type", 3, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"L3_Type", 2, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"Source_Port", 3, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"From_Crypto", 1, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"From_Crypto_Operation_Data", 5, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"From_Crypto_Security_Association_Ptr", 5, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"Rule_Pointer", 3, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"TCP_Flags", 9, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"Outer_VID", 12, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"Has_VLANs", 1, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"Outer_VLAN_Tag_Type", 1, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"Inner_VLAN_Tag_Type", 1, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"Outer_PCP", 3, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"Outer_DEI", 1, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"Inner_VID", 12, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"Inner_PCP", 3, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"Inner_DEI", 1, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"Outer_MPLS", 20, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"TOS", 8, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"TTL", 8, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"IPSEC_SPI", 32, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"IPSEC_SEQ", 32, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"MACsec_Key", 64, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"MACsec_Control_Byte", 18, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"MLD_Address", 128, SWITCH_FIELD_TYPE_IPV6},
	{"ICMP_Type", 8, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"ICMP_Code", 8, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"IGMP_Type", 8, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"IGMP_Group_Address", 32, SWITCH_FIELD_TYPE_IPV4},
	{"IPv6_Flow_Label", 20, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"L2_Packet_Flags", 4, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"IPv4_Options", 5, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"GID", 12, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"VID", 12, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"L2_Multicast_Pointer", 6, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"Destination_Portmask", 6, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"L2_Packet_Processing_Flags", 2, SWITCH_FIELD_TYPE_SHORT_VALUE},
	// Add a new item at the end
};

/* hash calculation */
u8  switch_ingress_acl0_index[] = {
	ACL_FIELD_MAC_DA,
	ACL_FIELD_MAC_SA,
	ACL_FIELD_IPV4_SA,
	ACL_FIELD_IPV4_DA,
	ACL_FIELD_IPV6_SA,
	ACL_FIELD_IPV6_DA,
	ACL_FIELD_L4_Source_Port,
	ACL_FIELD_L4_Destination_Port,
	ACL_FIELD_L4_Protocol,
	ACL_FIELD_Ethernet_Type,
	ACL_FIELD_L4_Type,
	ACL_FIELD_L3_Type,
	ACL_FIELD_Source_Port,
	ACL_FIELD_Rule_Pointer,
};

u8 switch_ingress_acl1_index[] = {
        ACL_FIELD_TCP_Flags,
	ACL_FIELD_MAC_DA,
	ACL_FIELD_MAC_SA,
        ACL_FIELD_Outer_VID,
        ACL_FIELD_Has_VLANs,
        ACL_FIELD_Outer_VLAN_Tag_Type,
        ACL_FIELD_Inner_VLAN_Tag_Type,
        ACL_FIELD_Outer_PCP,
        ACL_FIELD_Outer_DEI,
        ACL_FIELD_Inner_VID,
        ACL_FIELD_Inner_PCP,
        ACL_FIELD_Inner_DEI,
	ACL_FIELD_IPV4_SA,
	ACL_FIELD_IPV4_DA,
	ACL_FIELD_IPV6_SA,
	ACL_FIELD_IPV6_DA,
        ACL_FIELD_Outer_MPLS,
        ACL_FIELD_TOS,
        ACL_FIELD_TTL,
	ACL_FIELD_L4_Source_Port,
	ACL_FIELD_L4_Destination_Port,
        ACL_FIELD_MLD_Address,
        ACL_FIELD_ICMP_Type,
        ACL_FIELD_ICMP_Code,
        ACL_FIELD_IGMP_Type,
        ACL_FIELD_IGMP_Group_Address,
        ACL_FIELD_IPv6_Flow_Label,
        ACL_FIELD_L4_Protocol,
        ACL_FIELD_Ethernet_Type,
        ACL_FIELD_L4_Type,
        ACL_FIELD_L3_Type,
        ACL_FIELD_Source_Port,
        ACL_FIELD_Rule_Pointer,
};

u8 switch_ingress_acl2_index[] = {
        ACL_FIELD_TCP_Flags,
	ACL_FIELD_MAC_DA,
	ACL_FIELD_MAC_SA,
        ACL_FIELD_Outer_VID,
        ACL_FIELD_Has_VLANs,
        ACL_FIELD_Outer_VLAN_Tag_Type,
        ACL_FIELD_Inner_VLAN_Tag_Type,
        ACL_FIELD_Outer_PCP,
        ACL_FIELD_Outer_DEI,
        ACL_FIELD_Inner_VID,
        ACL_FIELD_Inner_PCP,
        ACL_FIELD_Inner_DEI,
	ACL_FIELD_IPV4_SA,
	ACL_FIELD_IPV4_DA,
	ACL_FIELD_IPV6_SA,
	ACL_FIELD_IPV6_DA,
        ACL_FIELD_Outer_MPLS,
        ACL_FIELD_TOS,
        ACL_FIELD_TTL,
	ACL_FIELD_L4_Source_Port,
	ACL_FIELD_L4_Destination_Port,
        ACL_FIELD_IPv6_Flow_Label,
        ACL_FIELD_L4_Protocol,
        ACL_FIELD_Ethernet_Type,
        ACL_FIELD_L4_Type,
        ACL_FIELD_L3_Type,
        ACL_FIELD_Source_Port,
        ACL_FIELD_Rule_Pointer,
};

u8 switch_ingress_acl3_index[] = {
        ACL_FIELD_L2_Packet_Flags,
        ACL_FIELD_IPv4_Options,
        ACL_FIELD_TCP_Flags,
        ACL_FIELD_TOS,
	ACL_FIELD_L4_Source_Port,
	ACL_FIELD_L4_Destination_Port,
        ACL_FIELD_L4_Protocol,
        ACL_FIELD_Ethernet_Type,
        ACL_FIELD_L4_Type,
        ACL_FIELD_L3_Type,
};

u8 switch_egress_acl0_index[] = {
	ACL_FIELD_MAC_DA,
	ACL_FIELD_MAC_SA,
	ACL_FIELD_IPV4_SA,
	ACL_FIELD_IPV4_DA,
	ACL_FIELD_IPV6_SA,
	ACL_FIELD_IPV6_DA,
	ACL_FIELD_L4_Source_Port,
	ACL_FIELD_L4_Destination_Port,
	ACL_FIELD_GID,
	ACL_FIELD_VID,
	ACL_FIELD_L2_Multicast_Pointer,
	ACL_FIELD_Destination_Portmask,
	ACL_FIELD_L4_Protocol,
	ACL_FIELD_Ethernet_Type,
	ACL_FIELD_L4_Type,
	ACL_FIELD_L3_Type,
	ACL_FIELD_Source_Port,
	ACL_FIELD_Rule_Pointer,
};

u8 switch_egress_acl1_index[] = {
	ACL_FIELD_L2_Packet_Processing_Flags,
	ACL_FIELD_MAC_DA,
	ACL_FIELD_MAC_SA,
	ACL_FIELD_IPV4_SA,
	ACL_FIELD_IPV4_DA,
	ACL_FIELD_IPV6_SA,
	ACL_FIELD_IPV6_DA,
	ACL_FIELD_TOS,
	ACL_FIELD_L4_Source_Port,
	ACL_FIELD_L4_Destination_Port,
	ACL_FIELD_GID,
	ACL_FIELD_VID,
	ACL_FIELD_L2_Multicast_Pointer,
	ACL_FIELD_Destination_Portmask,
	ACL_FIELD_L4_Protocol,
	ACL_FIELD_Ethernet_Type,
	ACL_FIELD_L4_Type,
	ACL_FIELD_L3_Type,
	ACL_FIELD_Source_Port,
	ACL_FIELD_Rule_Pointer,
};

u8 *switch_acl_field_index_all[] = {
	switch_ingress_acl0_index,
	switch_ingress_acl1_index,
	switch_ingress_acl2_index,
	switch_ingress_acl3_index,
	switch_egress_acl0_index,
	switch_egress_acl1_index,
};

struct switch_register_info reg_rules_setup[] = {
	{INGRESS_CONFIGURABLE_ACL_0_RULES_SETUP, INGRESS_CONFIGURABLE_ACL_0_RULES_SETUP_MAX, INGRESS_CONFIGURABLE_ACL_0_RULES_SETUP_ADDR_PER_ENTRY},
	{INGRESS_CONFIGURABLE_ACL_1_RULES_SETUP, INGRESS_CONFIGURABLE_ACL_1_RULES_SETUP_MAX, INGRESS_CONFIGURABLE_ACL_1_RULES_SETUP_ADDR_PER_ENTRY},
	{INGRESS_CONFIGURABLE_ACL_2_RULES_SETUP, INGRESS_CONFIGURABLE_ACL_2_RULES_SETUP_MAX, INGRESS_CONFIGURABLE_ACL_2_RULES_SETUP_ADDR_PER_ENTRY},
	{INGRESS_CONFIGURABLE_ACL_3_RULES_SETUP, INGRESS_CONFIGURABLE_ACL_3_RULES_SETUP_MAX, INGRESS_CONFIGURABLE_ACL_3_RULES_SETUP_ADDR_PER_ENTRY},
	{EGRESS_CONFIGURABLE_ACL_0_RULES_SETUP, EGRESS_CONFIGURABLE_ACL_0_RULES_SETUP_MAX, EGRESS_CONFIGURABLE_ACL_0_RULES_SETUP_ADDR_PER_ENTRY},
	{EGRESS_CONFIGURABLE_ACL_1_RULES_SETUP, EGRESS_CONFIGURABLE_ACL_1_RULES_SETUP_MAX, EGRESS_CONFIGURABLE_ACL_1_RULES_SETUP_ADDR_PER_ENTRY},
};

struct acl_engine_setting acl_settings[] = {
	//{154, 8, 2, 6, 2, ARRAY_SIZE(switch_ingress_acl0_index), 6},//dubhe1000 ingress_acl0 test
	{330, 9, 2, 6, 2, ARRAY_SIZE(switch_ingress_acl0_index), 7},
	{135, 6, 1, 2, 1, ARRAY_SIZE(switch_ingress_acl1_index), 7},
	{540, 0, 0, 0, 0, ARRAY_SIZE(switch_ingress_acl2_index), 20},
	{80, 0, 0, 0, 0, ARRAY_SIZE(switch_ingress_acl3_index), 10},
	{135, 8, 2, 6, 2, ARRAY_SIZE(switch_egress_acl0_index), 7},
	{540, 0, 0, 0, 0, ARRAY_SIZE(switch_egress_acl1_index), 20},
};

#define SWITCH_INGRESS_ACL_ENGINES	4
#define SWITCH_EGRESS_ACL_ENGINES	2

void switch_acl_hash_cal_usage(void)
{
	printf("Usage: npecmd switch acl hash_cal <is_egress> <AclN> <rulePtr> <--arg=value>\n");

	printf("is_egress := 0 1\n");

	for (int i = 0; i < ARRAY_SIZE(reg_rules_setup); i++) {
		if (reg_rules_setup[i].entries) {
			if (i < SWITCH_INGRESS_ACL_ENGINES)
				printf("maximum rulePtr of Ingress-Acl%d: %d\n", i, reg_rules_setup[i].entries - 1);
			else
				printf("maximum rulePtr of Egress-Acl%d: %d\n", i - SWITCH_INGRESS_ACL_ENGINES, reg_rules_setup[i].entries - 1);
		}
	}

	printf("arg := Field Name in ACL Field\n");
}

struct switch_field_def *switch_setup_acl_filed_def(u8 AclN, u64 fields, int *size)
{
	struct switch_field_def *switch_acl_hash = NULL;
	int i, j;

	//check AclN
	if (AclN >= ARRAY_SIZE(acl_settings)) {
		printf("[%s] Failed! Invalid AclN\n", __func__);
		goto ERR;
	}

	if (!fields) {
		printf("[%s] Failed! Empty Rule Setup\n", __func__);
		goto ERR;
	}

	switch_acl_hash	= malloc(sizeof(struct switch_field_def) * (acl_settings[AclN].bit_max_use + 1));
	if (!switch_acl_hash) {
		printf("[%s] Malloc Failed!\n", __func__);
		goto ERR;
	}

	switch_acl_hash[0].index = 0;
	switch_acl_hash[0].name = "Valid";
	switch_acl_hash[0].width = acl_settings[AclN].bit_max_use;
	switch_acl_hash[0].start = 0;
	switch_acl_hash[0].flag = SWITCH_FIELD_TYPE_SHORT_VALUE;

	for (i = 0, j = 1; i < acl_settings[AclN].bitmask_width; i++) {
		if (((i <= 31) && (fields & (1U << i)))
				|| (i > 31 && ((fields >> 32) & (1U << (i - 32))))) {
			switch_acl_hash[j].index = j;
			switch_acl_hash[j].name = switch_acl_fileds_info_all[switch_acl_field_index_all[AclN][i]].field_name;
			switch_acl_hash[j].width = switch_acl_fileds_info_all[switch_acl_field_index_all[AclN][i]].size;
			switch_acl_hash[j].flag = switch_acl_fileds_info_all[switch_acl_field_index_all[AclN][i]].flag;
			switch_acl_hash[j].start = switch_acl_hash[j - 1].start + switch_acl_hash[j - 1].width;
			j++;
		}

		if (j > acl_settings[AclN].bit_max_use) 
			break;
	}

	if (j == 1) {
		printf("[%s] Failed! Invalid Rule Setup\n", __func__);
		goto ERR;
	}

	*size = j;

	return switch_acl_hash;

ERR:
	if (switch_acl_hash)
		free(switch_acl_hash);

	return NULL;
}

static u16 switch_acl_hash_value_cal(u32 *hashkey, u8 AclN, u8 len, bool is_small)
{
        u16 hashval, base_value;
        u8 shift, fold_count;
        int i;

	if (is_small)
		shift = acl_settings[AclN].small_hash_size;
	else
               shift = acl_settings[AclN].large_hash_size;

        fold_count = acl_settings[AclN].compareData_width / shift;
	if (acl_settings[AclN].compareData_width % shift)
		fold_count += 1;

        base_value = get_one_bits(0, shift -1 );

	npecmd_dbg("[%s] acl%d %s-table: shift=%d fold_count=%d base_value=0x%x\n", __func__,
                       AclN, is_small ? "small" : "large", shift, fold_count, base_value);

        // hashval = hashkey & base_value
        hashval = hashkey[0] & base_value;
	npecmd_dbg("[%s] time=1-%d hashkey[bit0 - bit%d]=0x%x\n", __func__, fold_count, shift - 1, hashval);
        for (i = 1; i < fold_count; i++) {
                switch_u32_array_right_shift(hashkey, len, shift);
                npecmd_dbg("[%s] hashval=0x%x time=%d-%d hashkey[bit%d - bit%d]=0x%x\n",
                                        __func__,
                                        hashval, i + 1, fold_count, shift * i, shift * (i + 1) - 1, hashkey[0] & base_value);
                // hashval = hashval ˆ (hashkey >> shift))
                hashval = hashval ^ hashkey[0];
                // hashval = hashval & base_value
                hashval = hashval & base_value;
        }

        //hashval = (hashval & base_value);

        return hashval;
}

#define SWITCH_HASHKEY_LENGTH_MAX	20
static u8 siwtch_acl_modify_hashkey(u32 *hashkey, u16 size)
{
	int i, m, n;

	if (size >= SWITCH_HASHKEY_LENGTH_MAX * 32)
		return 0;

	m = size / 32;
	n = size % 32;

	if (n) {
		hashkey[m] &= get_one_bits(0, n - 1);
		m++;
	}

	for (i = m; i < SWITCH_HASHKEY_LENGTH_MAX; i++)
		hashkey[i] = 0;

	return m;
}

int switch_acl_hash_cal_config(int argc, char **argv)
{
	u64 address, tmp;
	/* only ACL1 need value[2], other value[1] */
	u32 value[INGRESS_CONFIGURABLE_ACL_1_RULES_SETUP_ADDR_PER_ENTRY] = {0}, AclN ,index, is_egress;
	u32 hashkey[SWITCH_HASHKEY_LENGTH_MAX] = {0}, hashtmp[SWITCH_HASHKEY_LENGTH_MAX] = {0};
	struct switch_field_def *switch_acl_hash = NULL;
	int i, size;
	int high_valid = 0;
	u16 hashval;
	u8 len;

	if (argc <= 3) {
		npecmd_err("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}


        //check is_egress
        if ((sscanf(argv[0], "%i", &is_egress) != 1) || (is_egress > 1)) {
                printf("[%s] Failed! Invalid is_egress\n", __func__);
                goto ERR;
        }

        //check AclN
        if (sscanf(argv[1], "%i", &AclN) != 1
                        || (!is_egress && AclN >= SWITCH_INGRESS_ACL_ENGINES)
                        ||(is_egress && AclN >= SWITCH_EGRESS_ACL_ENGINES)) {
                printf("[%s] Failed! Invalid AclN value\n", __func__);
                goto ERR;
        }

        if (is_egress)
                AclN += SWITCH_INGRESS_ACL_ENGINES;

        //checl acl entry
        if (!reg_rules_setup[AclN].entries) {
                printf("[%s] Failed! Invalid AclN entry\n", __func__);
                goto ERR;
        }

	//check index
	if ((sscanf(argv[2], "%i", &index) != 1) || (index >= reg_rules_setup[AclN].entries)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = reg_rules_setup[AclN].address + index * reg_rules_setup[AclN].addr_per_entry;

	for (i = 0; i < reg_rules_setup[AclN].addr_per_entry; i++)
		switch_read(address + i, &value[i]);

	tmp = value[1];
	tmp = (tmp << 32) + value[0];

	if (!tmp) {
		printf("[%s] Failed! Empty Rule Setup\n", __func__);
		goto ERR;
	}

	switch_acl_hash = switch_setup_acl_filed_def(AclN, tmp, &size);
	if (!switch_acl_hash) {
		printf("[%s] Failed! Invalid acl_filed_def!\n", __func__);
		goto ERR;
	}

	//update
	if (!switch_field_setup(argc - 2, argv + 2, switch_acl_hash, size, hashkey)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	len = siwtch_acl_modify_hashkey(hashkey, acl_settings[AclN].compareData_width);
	if (!len) {
		printf("[%s] Failed! Invalid compareData_width\n", __func__);
		goto ERR;
	}

       printf("[%s] hashkey = 0x", __func__);
	for (i = len - 1; i >=0 ; i--) {
		if (!high_valid && !hashkey[i])
			continue;
		else {
			high_valid = 1;
                       printf("%08x", hashkey[i]);
		}

	}
       printf("\n");

       if (acl_settings[AclN].large_hash_size) {
		memcpy(hashtmp, hashkey, sizeof(hashkey));
		hashval = switch_acl_hash_value_cal(hashtmp, AclN, len, 0);
		printf("[%s] large hashvalue = 0x%x\n", __func__, hashval);
	}

	if (acl_settings[AclN].small_hash_size) {
		memcpy(hashtmp, hashkey, sizeof(hashkey));
		hashval = switch_acl_hash_value_cal(hashtmp, AclN, len, 1);
		printf("[%s] small hashvalue = 0x%x\n", __func__, hashval);
	}

	free(switch_acl_hash);

	return 1;

ERR:
	if (switch_acl_hash)
		free(switch_acl_hash);

	switch_acl_hash_cal_usage();
	return 0;
}

/* Ingress/Egress ACL */
struct switch_register_info reg_selection[] = {
	{INGRESS_CONFIGURABLE_ACL_0_SELECTION, INGRESS_CONFIGURABLE_ACL_0_SELECTION_MAX, INGRESS_CONFIGURABLE_ACL_0_SELECTION_ADDR_PER_ENTRY},
	{INGRESS_CONFIGURABLE_ACL_1_SELECTION, INGRESS_CONFIGURABLE_ACL_1_SELECTION_MAX, INGRESS_CONFIGURABLE_ACL_1_SELECTION_ADDR_PER_ENTRY},
	{0, 0, 0},
	{0, 0, 0},
	{EGRESS_CONFIGURABLE_ACL_0_SELECTION, EGRESS_CONFIGURABLE_ACL_0_SELECTION_MAX, EGRESS_CONFIGURABLE_ACL_0_SELECTION_ADDR_PER_ENTRY},
	{0, 0, 0},
};

struct switch_field_def switch_ingress_acl_selection[] = {
	{INGRESS_ACL_SELECTION_ARG_SELECTTCAMORTABLE, "selectTcamOrTable", 1, 0},
	{INGRESS_ACL_SELECTION_ARG_SELECTSMALLORLARGE, "selectSmallOrLarge", 1, 1},
};

void switch_acl_selection_usage(void)
{
	int i;

	printf("Usage: npecmd switch acl selection <is_egress> <AclN> <--arg=value>\n");

	printf("is_egress := 0 1\n");

	printf("Ingress-AclN :=");
	for (i = 0; i < SWITCH_INGRESS_ACL_ENGINES; i++) {
		if (reg_selection[i].entries)
			printf(" %d", i);
	}
	printf("\n");

	printf("Egress-AclN :=");
	for (i = SWITCH_INGRESS_ACL_ENGINES; i < ARRAY_SIZE(reg_selection); i++) {
		if (reg_selection[i].entries)
			printf(" %d", i - SWITCH_INGRESS_ACL_ENGINES);
	}
	printf("\n");

	printf("arg :=");
	for (i = 0; i < ARRAY_SIZE(switch_ingress_acl_selection); i++)
		printf(" %s", switch_ingress_acl_selection[i].name);

	printf("\n");
}

int switch_acl_selection_config(int argc, char **argv)
{
	u64 address;
	u32 value[INGRESS_CONFIGURABLE_ACL_0_SELECTION_ADDR_PER_ENTRY] = {0}, AclN, is_egress;
	int i;

	if (argc <= 2) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check is_egress
	if ((sscanf(argv[0], "%i", &is_egress) != 1) || (is_egress > 1)) {
		printf("[%s] Failed! Invalid is_egress\n", __func__);
		goto ERR;
	}

	//check AclN
	if (sscanf(argv[1], "%i", &AclN) != 1 
			|| (!is_egress && AclN >= SWITCH_INGRESS_ACL_ENGINES)
			||(is_egress && AclN >= SWITCH_EGRESS_ACL_ENGINES)) {
		printf("[%s] Failed! Invalid AclN value\n", __func__);
		goto ERR;
	}

	if (is_egress)
		AclN += SWITCH_INGRESS_ACL_ENGINES;

	//checl acl entry
	if (!reg_selection[AclN].entries) {
		printf("[%s] Failed! Invalid AclN entry\n", __func__);
		goto ERR;
	}

	address = reg_selection[AclN].address;

	//get old value
	for (i = 0; i < reg_selection[AclN].addr_per_entry; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc - 1, argv + 1, switch_ingress_acl_selection, ARRAY_SIZE(switch_ingress_acl_selection), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < reg_selection[AclN].addr_per_entry; i++)
		switch_write(address + i, value[i]);

	printf("[%s] %s-Acl%d addr 0x%lx value 0x%08x\n", __func__,
			is_egress ? "Egress" : "Ingress", is_egress ? (AclN - SWITCH_INGRESS_ACL_ENGINES): AclN ,
			address, value[0]);

	return 1;

ERR:
	switch_acl_selection_usage();
	return 0;
}

void switch_acl_rules_setup_usage(void)
{
	printf("Usage: npecmd switch acl rules_setup <is_egress> <AclN> <index> <value>\n");

	printf("is_egress := 0 1\n");

	for (int i = 0; i < ARRAY_SIZE(reg_rules_setup); i++) {
		if (reg_rules_setup[i].entries)
			if (i < SWITCH_INGRESS_ACL_ENGINES)
				printf("maximum index of Ingress-Acl%d: %d\n", i, reg_rules_setup[i].entries - 1);
			else
				printf("maximum index of Egress-Acl%d: %d\n", i - SWITCH_INGRESS_ACL_ENGINES, reg_rules_setup[i].entries - 1);
	}

}

int switch_acl_rules_setup_config(int argc, char **argv)
{
	u64 address, tmp;
	/* only ACL1 need value[2], other value[1] */
	u32 value[INGRESS_CONFIGURABLE_ACL_1_RULES_SETUP_ADDR_PER_ENTRY] = {0}, AclN ,index, is_egress;
	struct switch_field_def *switch_acl_hash = NULL;
	int i, size;

	if (argc != 4) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

        //check is_egress
        if ((sscanf(argv[0], "%i", &is_egress) != 1) || (is_egress > 1)) {
                printf("[%s] Failed! Invalid is_egress\n", __func__);
                goto ERR;
        }

        //check AclN
        if (sscanf(argv[1], "%i", &AclN) != 1
                        || (!is_egress && AclN >= SWITCH_INGRESS_ACL_ENGINES)
                        ||(is_egress && AclN >= (ARRAY_SIZE(reg_selection) - SWITCH_INGRESS_ACL_ENGINES))) {
                printf("[%s] Failed! Invalid AclN value\n", __func__);
                goto ERR;
        }

        if (is_egress)
                AclN += SWITCH_INGRESS_ACL_ENGINES;

        //checl acl entry
        if (!reg_rules_setup[AclN].entries) {
                printf("[%s] Failed! Invalid AclN entry\n", __func__);
                goto ERR;
        }

	//check index
	if ((sscanf(argv[2], "%i", &index) != 1) || (index >= reg_rules_setup[AclN].entries)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = reg_rules_setup[AclN].address + index * reg_rules_setup[AclN].addr_per_entry;

	//check value
	if ((isHex(argv[3]) && sscanf(argv[3], "%lx", &tmp) != 1)
				|| (!isHex(argv[3]) && sscanf(argv[3], "%lu", &tmp) != 1)) {
		printf("[%s] Failed! Invalid value\n", __func__);
		goto ERR;
	}

	if (acl_settings[AclN].bitmask_width <= 32) {
		value[0] =  tmp & get_one_bits(0, acl_settings[AclN].bitmask_width - 1);
		value[1] = 0;
	} else {
		value[0] =  tmp & get_one_bits(0, 31);
		value[1] =  (tmp >> 32) & get_one_bits(0, acl_settings[AclN].bitmask_width - 33);
	}

	//config
	for (i = 0; i < reg_rules_setup[AclN].addr_per_entry; i++)
		switch_write(address + i, value[i]);

	printf("[%s] %s-Acl%d index %d addr 0x%lx value 0x[%08x %08x]\n", __func__,
			is_egress ? "Egress" : "Ingress", is_egress ? (AclN - SWITCH_INGRESS_ACL_ENGINES): AclN ,
			index, address, value[1], value[0]);

#if 1
	switch_acl_hash = switch_setup_acl_filed_def(AclN, tmp, &size);
	if (!switch_acl_hash) {
		printf("[%s] Failed! Invalid acl_filed_def!\n", __func__);
		goto ERR;
	}

	printf("[%s] setup arg:=", __func__);
	for (i = 0; i < size; i++) {
		if (switch_acl_hash[i].width)
			printf(" %s", switch_acl_hash[i].name);
	}
	printf("\n");
	free(switch_acl_hash);
#endif

	return 1;

ERR:
	switch_acl_rules_setup_usage();
	return 0;
}

/*INGRESS ACL*/
struct switch_register_info reg_pre_lookup[] = {
	{INGRESS_CONFIGURABLE_ACL_0_PRE_LOOKUP, INGRESS_CONFIGURABLE_ACL_0_PRE_LOOKUP_MAX, INGRESS_CONFIGURABLE_ACL_0_PRE_LOOKUP_ADDR_PER_ENTRY},
	{INGRESS_CONFIGURABLE_ACL_1_PRE_LOOKUP, INGRESS_CONFIGURABLE_ACL_1_PRE_LOOKUP_MAX, INGRESS_CONFIGURABLE_ACL_1_PRE_LOOKUP_ADDR_PER_ENTRY},
	{INGRESS_CONFIGURABLE_ACL_2_PRE_LOOKUP, INGRESS_CONFIGURABLE_ACL_2_PRE_LOOKUP_MAX, INGRESS_CONFIGURABLE_ACL_2_PRE_LOOKUP_ADDR_PER_ENTRY},
};

struct switch_field_def switch_ingress_acl_pre_lookup[] = {
	{INGRESS_ACL_PRE_LOOKUP_ARG_VALID, "valid", 1, 0},
	{INGRESS_ACL_PRE_LOOKUP_ARG_RULEPTR, "rulePtr", 3, 1},//Note: size is 2 in Ingress Acl2
};

void switch_ingress_acl_pre_lookup_usage(void)
{
	int i;

	printf("Usage: npecmd switch acl ingress_prelookup <Ingress AclN> <index> <--arg=value>\n");

	for (i = 0; i < ARRAY_SIZE(reg_pre_lookup); i++) {
		if (reg_pre_lookup[i].entries)
			printf("maximum index of Ingress Acl%d: %d\n", i, reg_pre_lookup[i].entries - 1);
	}

	printf("arg :=");
	for (i = 0; i < ARRAY_SIZE(switch_ingress_acl_pre_lookup); i++)
		printf(" %s", switch_ingress_acl_pre_lookup[i].name);

	printf("\n");
}

int switch_ingress_acl_pre_lookup_config(int argc, char **argv)
{
	u64 address;
	u32 value[INGRESS_CONFIGURABLE_ACL_0_PRE_LOOKUP_ADDR_PER_ENTRY] = {0}, index, AclN;
	int i;

	if (argc <= 2) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check AclN
	if ((sscanf(argv[0], "%i", &AclN) != 1) || (AclN >= ARRAY_SIZE(reg_pre_lookup)) || !reg_pre_lookup[AclN].entries) {
		printf("[%s] Failed! Invalid Ingress AclN\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[1], "%i", &index) != 1) || (index >= reg_pre_lookup[AclN].entries)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = reg_pre_lookup[AclN].address + index * reg_pre_lookup[AclN].addr_per_entry;

	//get old value
	for (i = 0; i < reg_pre_lookup[AclN].addr_per_entry; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc - 1, argv + 1, switch_ingress_acl_pre_lookup, ARRAY_SIZE(switch_ingress_acl_pre_lookup), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	/* Note: The width of rulePtr in Acl3 pre lookup is 2 */
	if (AclN == 2)
		value[0] = value[0] & 0x7;

	//config
	for (i = 0; i < reg_pre_lookup[AclN].addr_per_entry; i++)
		switch_write(address + i, value[i]);

	printf("[%s] Ingress acl%d index %d addr 0x%lx value 0x%08x\n", __func__, AclN, index, address, value[0]);

	return 1;

ERR:
	switch_ingress_acl_pre_lookup_usage();
	return 0;
}

struct switch_field_def switch_acl_ingress_0_large_table[] = {
	{INGRESS_ACL0_LARGE_ARG_VALID, "valid", 1, 0},
	//{INGRESS_ACL0_LARGE_ARG_COMPAREDATA, "compareData", 330, 1},
	{INGRESS_ACL0_LARGE_ARG_COMPAREDATA0, "compareData0", 64, 1},
	{INGRESS_ACL0_LARGE_ARG_COMPAREDATA1, "compareData1", 64, 65},
	{INGRESS_ACL0_LARGE_ARG_COMPAREDATA2, "compareData2", 64, 129},
	{INGRESS_ACL0_LARGE_ARG_COMPAREDATA3, "compareData3", 64, 193},
	{INGRESS_ACL0_LARGE_ARG_COMPAREDATA4, "compareData4", 64, 257},
	{INGRESS_ACL0_LARGE_ARG_COMPAREDATA5, "compareData5", 10, 321},

	{INGRESS_ACL0_LARGE_ARG_SENDTOCPU, "sendToCpu", 1, 331},
	{INGRESS_ACL0_LARGE_ARG_FORCESENDTOCPUORIGPKT, "forceSendToCpuOrigPkt", 1, 332},
	{INGRESS_ACL0_LARGE_ARG_METADATAVALID, "metaDataValid", 1, 333},
	{INGRESS_ACL0_LARGE_ARG_METADATA, "metaData", 16, 334},
	{INGRESS_ACL0_LARGE_ARG_METADATAPRIO, "metaDataPrio", 1, 350},
	{INGRESS_ACL0_LARGE_ARG_DROPENABLE, "dropEnable", 1, 351},
	{INGRESS_ACL0_LARGE_ARG_SENDTOPORT, "sendToPort", 1, 352},
	{INGRESS_ACL0_LARGE_ARG_DESTPORT, "destPort", 3, 353},
	{INGRESS_ACL0_LARGE_ARG_INPUTMIRROR, "inputMirror", 1, 356},
	{INGRESS_ACL0_LARGE_ARG_DESTINPUTMIRROR, "destInputMirror", 3, 357},
	{INGRESS_ACL0_LARGE_ARG_IMPRIO, "imPrio", 1, 360},
	{INGRESS_ACL0_LARGE_ARG_UPDATECOUNTER, "updateCounter", 1, 361},
	{INGRESS_ACL0_LARGE_ARG_COUNTER, "counter", 6, 362},
	{INGRESS_ACL0_LARGE_ARG_UPDATETOSEXP, "updateTosExp", 1, 368},
	{INGRESS_ACL0_LARGE_ARG_NEWTOSEXP, "newTosExp", 8, 369},
	{INGRESS_ACL0_LARGE_ARG_TOSMASK, "tosMask", 8, 377},
	{INGRESS_ACL0_LARGE_ARG_ENABLEUPDATEIP, "enableUpdateIp", 1, 385},
	{INGRESS_ACL0_LARGE_ARG_UPDATESAORDA, "updateSaOrDa", 1, 386},
	{INGRESS_ACL0_LARGE_ARG_NEWIPVALUE, "newIpValue", 32, 387, SWITCH_FIELD_TYPE_IPV4},
	{INGRESS_ACL0_LARGE_ARG_ENABLEUPDATEL4, "enableUpdateL4", 1, 419},
	{INGRESS_ACL0_LARGE_ARG_UPDATEL4SPORDP, "updateL4SpOrDp", 1, 420},
	{INGRESS_ACL0_LARGE_ARG_NEWL4VALUE, "newL4Value", 16, 421},
	{INGRESS_ACL0_LARGE_ARG_NATOPVALID, "natOpValid", 1, 437},
	{INGRESS_ACL0_LARGE_ARG_NATOPPTR, "natOpPtr", 11, 438},
	{INGRESS_ACL0_LARGE_ARG_NATOPPRIO, "natOpPrio", 1, 449},
	{INGRESS_ACL0_LARGE_ARG_FORCECOLOR, "forceColor", 1, 450},
	{INGRESS_ACL0_LARGE_ARG_COLOR, "color", 2, 451},
	{INGRESS_ACL0_LARGE_ARG_FORCECOLORPRIO, "forceColorPrio", 1, 453},
	{INGRESS_ACL0_LARGE_ARG_MMPVALID, "mmpValid", 1, 454},
	{INGRESS_ACL0_LARGE_ARG_MMPPTR, "mmpPtr", 5, 455},
	{INGRESS_ACL0_LARGE_ARG_MMPORDER, "mmpOrder", 2, 460},
	{INGRESS_ACL0_LARGE_ARG_FORCEQUEUE, "forceQueue", 1, 462},
	{INGRESS_ACL0_LARGE_ARG_EQUEUE, "eQueue", 3, 463},
	{INGRESS_ACL0_LARGE_ARG_FORCEQUEUEPRIO, "forceQueuePrio", 1, 466},
};

void switch_acl_ingress_0_large_table_usage(void)
{
	printf("Usage: npecmd switch acl ingress0_large <index> <--arg=value>\n");

	printf("maximum index: %d\n", INGRESS_CONFIGURABLE_ACL_0_LARGE_TABLE_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_acl_ingress_0_large_table); i++)
		printf(" %s", switch_acl_ingress_0_large_table[i].name);

	printf("\n");
}

int switch_acl_ingress_0_large_table_config(int argc, char **argv)
{
	u64 address;
	u32 value[INGRESS_CONFIGURABLE_ACL_0_LARGE_TABLE_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= INGRESS_CONFIGURABLE_ACL_0_LARGE_TABLE_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = INGRESS_CONFIGURABLE_ACL_0_LARGE_TABLE + index * INGRESS_CONFIGURABLE_ACL_0_LARGE_TABLE_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < INGRESS_CONFIGURABLE_ACL_0_LARGE_TABLE_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_acl_ingress_0_large_table, ARRAY_SIZE(switch_acl_ingress_0_large_table), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < INGRESS_CONFIGURABLE_ACL_0_LARGE_TABLE_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x %08x %08x]\n", __func__, index, address, value[3], value[2], value[1], value[0]);

	return 1;

ERR:
	switch_acl_ingress_0_large_table_usage();
	return 0;
}

struct switch_field_def switch_acl_ingress_0_small_table[] = {
	{INGRESS_ACL0_SMALL_ARG_VALID, "valid", 1, 0},
	//{INGRESS_ACL0_SMALL_ARG_COMPAREDATA, "compareData", 330, 1},
	{INGRESS_ACL0_SMALL_ARG_COMPAREDATA0, "compareData0", 64, 1},
	{INGRESS_ACL0_SMALL_ARG_COMPAREDATA1, "compareData1", 64, 65},
	{INGRESS_ACL0_SMALL_ARG_COMPAREDATA2, "compareData2", 64, 129},
	{INGRESS_ACL0_SMALL_ARG_COMPAREDATA3, "compareData3", 64, 193},
	{INGRESS_ACL0_SMALL_ARG_COMPAREDATA4, "compareData4", 64, 257},
	{INGRESS_ACL0_SMALL_ARG_COMPAREDATA5, "compareData5", 10, 321},

	{INGRESS_ACL0_SMALL_ARG_SENDTOCPU, "sendToCpu", 1, 331},
	{INGRESS_ACL0_SMALL_ARG_FORCESENDTOCPUORIGPKT, "forceSendToCpuOrigPkt", 1, 332},
	{INGRESS_ACL0_SMALL_ARG_METADATAVALID, "metaDataValid", 1, 333},
	{INGRESS_ACL0_SMALL_ARG_METADATA, "metaData", 16, 334},
	{INGRESS_ACL0_SMALL_ARG_METADATAPRIO, "metaDataPrio", 1, 350},
	{INGRESS_ACL0_SMALL_ARG_DROPENABLE, "dropEnable", 1, 351},
	{INGRESS_ACL0_SMALL_ARG_SENDTOPORT, "sendToPort", 1, 352},
	{INGRESS_ACL0_SMALL_ARG_DESTPORT, "destPort", 3, 353},
	{INGRESS_ACL0_SMALL_ARG_INPUTMIRROR, "inputMirror", 1, 356},
	{INGRESS_ACL0_SMALL_ARG_DESTINPUTMIRROR, "destInputMirror", 3, 357},
	{INGRESS_ACL0_SMALL_ARG_IMPRIO, "imPrio", 1, 360},
	{INGRESS_ACL0_SMALL_ARG_UPDATECOUNTER, "updateCounter", 1, 361},
	{INGRESS_ACL0_SMALL_ARG_COUNTER, "counter", 6, 362},
	{INGRESS_ACL0_SMALL_ARG_UPDATETOSEXP, "updateTosExp", 1, 368},
	{INGRESS_ACL0_SMALL_ARG_NEWTOSEXP, "newTosExp", 8, 369},
	{INGRESS_ACL0_SMALL_ARG_TOSMASK, "tosMask", 8, 377},
	{INGRESS_ACL0_SMALL_ARG_ENABLEUPDATEIP, "enableUpdateIp", 1, 385},
	{INGRESS_ACL0_SMALL_ARG_UPDATESAORDA, "updateSaOrDa", 1, 386},
	{INGRESS_ACL0_SMALL_ARG_NEWIPVALUE, "newIpValue", 32, 387, SWITCH_FIELD_TYPE_IPV4},
	{INGRESS_ACL0_SMALL_ARG_ENABLEUPDATEL4, "enableUpdateL4", 1, 419},
	{INGRESS_ACL0_SMALL_ARG_UPDATEL4SPORDP, "updateL4SpOrDp", 1, 420},
	{INGRESS_ACL0_SMALL_ARG_NEWL4VALUE, "newL4Value", 16, 421},
	{INGRESS_ACL0_SMALL_ARG_NATOPVALID, "natOpValid", 1, 437},
	{INGRESS_ACL0_SMALL_ARG_NATOPPTR, "natOpPtr", 11, 438},
	{INGRESS_ACL0_SMALL_ARG_NATOPPRIO, "natOpPrio", 1, 449},
	{INGRESS_ACL0_SMALL_ARG_FORCECOLOR, "forceColor", 1, 450},
	{INGRESS_ACL0_SMALL_ARG_COLOR, "color", 2, 451},
	{INGRESS_ACL0_SMALL_ARG_FORCECOLORPRIO, "forceColorPrio", 1, 453},
	{INGRESS_ACL0_SMALL_ARG_MMPVALID, "mmpValid", 1, 454},
	{INGRESS_ACL0_SMALL_ARG_MMPPTR, "mmpPtr", 5, 455},
	{INGRESS_ACL0_SMALL_ARG_MMPORDER, "mmpOrder", 2, 460},
	{INGRESS_ACL0_SMALL_ARG_FORCEQUEUE, "forceQueue", 1, 462},
	{INGRESS_ACL0_SMALL_ARG_EQUEUE, "eQueue", 3, 463},
	{INGRESS_ACL0_SMALL_ARG_FORCEQUEUEPRIO, "forceQueuePrio", 1, 466},
};

void switch_acl_ingress_0_small_table_usage(void)
{
	printf("Usage: npecmd switch acl ingress0_small <index> <--arg=value>\n");

	printf("maximum index: %d\n", INGRESS_CONFIGURABLE_ACL_0_SMALL_TABLE_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_acl_ingress_0_small_table); i++)
		printf(" %s", switch_acl_ingress_0_small_table[i].name);

	printf("\n");
}

int switch_acl_ingress_0_small_table_config(int argc, char **argv)
{
	u64 address;
	u32 value[INGRESS_CONFIGURABLE_ACL_0_SMALL_TABLE_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= INGRESS_CONFIGURABLE_ACL_0_SMALL_TABLE_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = INGRESS_CONFIGURABLE_ACL_0_SMALL_TABLE + index * INGRESS_CONFIGURABLE_ACL_0_SMALL_TABLE_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < INGRESS_CONFIGURABLE_ACL_0_SMALL_TABLE_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_acl_ingress_0_small_table, ARRAY_SIZE(switch_acl_ingress_0_small_table), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < INGRESS_CONFIGURABLE_ACL_0_SMALL_TABLE_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x %08x %08x]\n", __func__, index, address, value[3], value[2], value[1], value[0]);


	return 1;

ERR:
	switch_acl_ingress_0_small_table_usage();
	return 0;
}

struct switch_field_def switch_acl_ingress_0_mask[] = {
       //{INGRESS_ACL0_MASK_ARG_MASK_SMALL, "mask_small", 330, 0},
       {INGRESS_ACL0_MASK_ARG_MASK_SMALL0, "mask_small0", 64, 0},
       {INGRESS_ACL0_MASK_ARG_MASK_SMALL1, "mask_small1", 64, 64},
       {INGRESS_ACL0_MASK_ARG_MASK_SMALL2, "mask_small2", 64, 128},
       {INGRESS_ACL0_MASK_ARG_MASK_SMALL3, "mask_small3", 64, 192},
       {INGRESS_ACL0_MASK_ARG_MASK_SMALL4, "mask_small4", 64, 256},
       {INGRESS_ACL0_MASK_ARG_MASK_SMALL5, "mask_small5", 10, 320},
       //{INGRESS_ACL0_MASK_ARG_MASK_LARGE, "mask_large", 330, 330},
       {INGRESS_ACL0_MASK_ARG_MASK_LARGE0, "mask_large0", 64, 330},
       {INGRESS_ACL0_MASK_ARG_MASK_LARGE1, "mask_large1", 64, 394},
       {INGRESS_ACL0_MASK_ARG_MASK_LARGE2, "mask_large2", 64, 458},
       {INGRESS_ACL0_MASK_ARG_MASK_LARGE3, "mask_large3", 64, 522},
       {INGRESS_ACL0_MASK_ARG_MASK_LARGE4, "mask_large4", 64, 586},
       {INGRESS_ACL0_MASK_ARG_MASK_LARGE5, "mask_large5", 10, 650},
};

void switch_acl_ingress_0_mask_usage(void)
{
       printf("Usage: npecmd switch acl ingress0_mask <index> <--arg=value>\n");

       printf("maximum index: %d\n", INGRESS_CONFIGURABLE_ACL_0_SEARCH_MASK_MAX - 1);
       printf("arg :=");
       for (int i = 0; i < ARRAY_SIZE(switch_acl_ingress_0_mask); i++)
               printf(" %s", switch_acl_ingress_0_mask[i].name);

       printf("\n");
}

int switch_acl_ingress_0_mask_config(int argc, char **argv)
{
       u64 address;
       u32 value[INGRESS_CONFIGURABLE_ACL_0_SEARCH_MASK_ADDR_PER_ENTRY] = {0}, index;
       int i;
       int high_valid = 0;

       if (argc <= 1) {
               printf("[%s] Failed! Invalid argc\n", __func__);
               goto ERR;
       }

       //check index
       if ((sscanf(argv[0], "%i", &index) != 1) || (index >= INGRESS_CONFIGURABLE_ACL_0_SEARCH_MASK_MAX)) {
               printf("[%s] Failed! Invalid index\n", __func__);
               goto ERR;
       }

       address = INGRESS_CONFIGURABLE_ACL_0_SEARCH_MASK + index * INGRESS_CONFIGURABLE_ACL_0_SEARCH_MASK_ADDR_PER_ENTRY;

       //get old value
       for (i = 0; i < INGRESS_CONFIGURABLE_ACL_0_SEARCH_MASK_ADDR_PER_ENTRY; i++)
               switch_read(address + i, &value[i]);

       //update
       if (!switch_field_setup(argc, argv, switch_acl_ingress_0_mask, ARRAY_SIZE(switch_acl_ingress_0_mask), value)) {
               printf("[%s] Failed! Invalid arg list\n", __func__);
               goto ERR;
       }

       //config
       for (i = 0; i < INGRESS_CONFIGURABLE_ACL_0_SEARCH_MASK_ADDR_PER_ENTRY; i++)
               switch_write(address + i, value[i]);

       printf("[%s] index %d addr 0x%lx value %08x\n", __func__, index, address, value[0]);

       npecmd_dbg("[%s] new value = 0x", __func__);
       for (i = INGRESS_CONFIGURABLE_ACL_0_SEARCH_MASK_ADDR_PER_ENTRY - 1; i >= 0 ; i--) {
               if (!high_valid && !value[i])
                       continue;
               else {
                       high_valid = 1;
                       npecmd_dbg("%08x", value[i]);
               }

       }
       npecmd_dbg("\n");


       return 1;

ERR:
       switch_acl_ingress_0_mask_usage();
       return 0;
}

struct switch_field_def switch_acl_ingress_0_tcam[] = {
	{INGRESS_ACL0_TCAM_ARG_VALID, "valid", 1, 0},
	//{INGRESS_ACL0_TCAM_ARG_MASK, "mask", 330, 1},
	{INGRESS_ACL0_TCAM_ARG_MASK0, "mask0", 64, 1},
	{INGRESS_ACL0_TCAM_ARG_MASK1, "mask1", 64, 65},
	{INGRESS_ACL0_TCAM_ARG_MASK2, "mask2", 64, 129},
	{INGRESS_ACL0_TCAM_ARG_MASK3, "mask3", 64, 193},
	{INGRESS_ACL0_TCAM_ARG_MASK4, "mask4", 64, 257},
	{INGRESS_ACL0_TCAM_ARG_MASK5, "mask5", 10, 321},
	//{INGRESS_ACL0_TCAM_ARG_COMPAREDATA, "compareData", 330, 331},
	{INGRESS_ACL0_TCAM_ARG_COMPAREDATA0, "compareData0", 64, 331},
	{INGRESS_ACL0_TCAM_ARG_COMPAREDATA1, "compareData1", 64, 395},
	{INGRESS_ACL0_TCAM_ARG_COMPAREDATA2, "compareData2", 64, 459},
	{INGRESS_ACL0_TCAM_ARG_COMPAREDATA3, "compareData3", 64, 523},
	{INGRESS_ACL0_TCAM_ARG_COMPAREDATA4, "compareData4", 64, 587},
	{INGRESS_ACL0_TCAM_ARG_COMPAREDATA5, "compareData5", 10, 651},
};

void switch_acl_ingress_0_tcam_usage(void)
{
        printf("Usage: npecmd switch acl ingress0_tcam <index> <--arg=value>\n");

	printf("maximum index: %d\n", INGRESS_CONFIGURABLE_ACL_0_TCAM_MAX - 1);
        printf("arg :=");
        for (int i = 0; i < ARRAY_SIZE(switch_acl_ingress_0_tcam); i++)
                printf(" %s", switch_acl_ingress_0_tcam[i].name);

        printf("\n");
}

int switch_acl_ingress_0_tcam_config(int argc, char **argv)
{
	u64 address;
	u32 value[INGRESS_CONFIGURABLE_ACL_0_TCAM_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= INGRESS_CONFIGURABLE_ACL_0_TCAM_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = INGRESS_CONFIGURABLE_ACL_0_TCAM + index * INGRESS_CONFIGURABLE_ACL_0_TCAM_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < INGRESS_CONFIGURABLE_ACL_0_TCAM_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_acl_ingress_0_tcam, ARRAY_SIZE(switch_acl_ingress_0_tcam), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < INGRESS_CONFIGURABLE_ACL_0_TCAM_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x %08x %08x]\n", __func__, index, address, value[3], value[2], value[1], value[0]);

        return 1;

ERR:
        switch_acl_ingress_0_tcam_usage();
        return 0;
}

struct switch_field_def switch_acl_ingress_0_answer[] = {
	{INGRESS_ACL0_TCAM_ANSWER_ARG_SENDTOCPU, "sendToCpu", 1, 0},
	{INGRESS_ACL0_TCAM_ANSWER_ARG_FORCESENDTOCPUORIGPKT, "forceSendToCpuOrigPkt", 1, 1},
	{INGRESS_ACL0_TCAM_ANSWER_ARG_METADATAVALID, "metaDataValid", 1, 2},
	{INGRESS_ACL0_TCAM_ANSWER_ARG_METADATA, "metaData", 16, 3},
	{INGRESS_ACL0_TCAM_ANSWER_ARG_METADATAPRIO, "metaDataPrio", 1, 19},
	{INGRESS_ACL0_TCAM_ANSWER_ARG_DROPENABLE, "dropEnable", 1, 20},
	{INGRESS_ACL0_TCAM_ANSWER_ARG_SENDTOPORT, "sendToPort", 1, 21},
	{INGRESS_ACL0_TCAM_ANSWER_ARG_DESTPORT, "destPort", 3, 22},
	{INGRESS_ACL0_TCAM_ANSWER_ARG_INPUTMIRROR, "inputMirror", 1, 25},
	{INGRESS_ACL0_TCAM_ANSWER_ARG_DESTINPUTMIRROR, "destInputMirror", 3, 26},
	{INGRESS_ACL0_TCAM_ANSWER_ARG_IMPRIO, "imPrio", 1, 29},
	{INGRESS_ACL0_TCAM_ANSWER_ARG_UPDATECOUNTER, "updateCounter", 1, 30},
	{INGRESS_ACL0_TCAM_ANSWER_ARG_COUNTER, "counter", 6, 31},
	{INGRESS_ACL0_TCAM_ANSWER_ARG_UPDATETOSEXP, "updateTosExp", 1, 37},
	{INGRESS_ACL0_TCAM_ANSWER_ARG_NEWTOSEXP, "newTosExp", 8, 38},
	{INGRESS_ACL0_TCAM_ANSWER_ARG_TOSMASK, "tosMask", 8, 46},
	{INGRESS_ACL0_TCAM_ANSWER_ARG_ENABLEUPDATEIP, "enableUpdateIp", 1, 54},
	{INGRESS_ACL0_TCAM_ANSWER_ARG_UPDATESAORDA, "updateSaOrDa", 1, 55},
	{INGRESS_ACL0_TCAM_ANSWER_ARG_NEWIPVALUE, "newIpValue", 32, 56, SWITCH_FIELD_TYPE_IPV4},
	{INGRESS_ACL0_TCAM_ANSWER_ARG_ENABLEUPDATEL4, "enableUpdateL4", 1, 88},
	{INGRESS_ACL0_TCAM_ANSWER_ARG_UPDATEL4SPORDP, "updateL4SpOrDp", 1, 89},
	{INGRESS_ACL0_TCAM_ANSWER_ARG_NEWL4VALUE, "newL4Value", 16, 90},
	{INGRESS_ACL0_TCAM_ANSWER_ARG_NATOPVALID, "natOpValid", 1, 106},
	{INGRESS_ACL0_TCAM_ANSWER_ARG_NATOPPTR, "natOpPtr", 11, 107},
	{INGRESS_ACL0_TCAM_ANSWER_ARG_NATOPPRIO, "natOpPrio", 1, 118},
	{INGRESS_ACL0_TCAM_ANSWER_ARG_FORCECOLOR, "forceColor", 1, 119},
	{INGRESS_ACL0_TCAM_ANSWER_ARG_COLOR, "color", 2, 120},
	{INGRESS_ACL0_TCAM_ANSWER_ARG_FORCECOLORPRIO, "forceColorPrio", 1, 122},
	{INGRESS_ACL0_TCAM_ANSWER_ARG_MMPVALID, "mmpValid", 1, 123},
	{INGRESS_ACL0_TCAM_ANSWER_ARG_MMPPTR, "mmpPtr", 5, 124},
	{INGRESS_ACL0_TCAM_ANSWER_ARG_MMPORDER, "mmpOrder", 2, 129},
	{INGRESS_ACL0_TCAM_ANSWER_ARG_FORCEQUEUE, "forceQueue", 1, 131},
	{INGRESS_ACL0_TCAM_ANSWER_ARG_EQUEUE, "eQueue", 3, 132},
	{INGRESS_ACL0_TCAM_ANSWER_ARG_FORCEQUEUEPRIO, "forceQueuePrio", 1, 135},
};

void switch_acl_ingress_0_answer_usage(void)
{
        printf("Usage: npecmd switch acl ingress0_answer <index> <--arg=value>\n");

	printf("maximum index: %d\n", INGRESS_CONFIGURABLE_ACL_0_TCAM_ANSWER_MAX - 1);
        printf("arg :=");
        for (int i = 0; i < ARRAY_SIZE(switch_acl_ingress_0_answer); i++)
                printf(" %s", switch_acl_ingress_0_answer[i].name);

        printf("\n");
}

int switch_acl_ingress_0_answer_config(int argc, char **argv)
{
	u64 address;
	u32 value[INGRESS_CONFIGURABLE_ACL_0_TCAM_ANSWER_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= INGRESS_CONFIGURABLE_ACL_0_TCAM_ANSWER_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = INGRESS_CONFIGURABLE_ACL_0_TCAM_ANSWER + index * INGRESS_CONFIGURABLE_ACL_0_TCAM_ANSWER_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < INGRESS_CONFIGURABLE_ACL_0_TCAM_ANSWER_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_acl_ingress_0_answer, ARRAY_SIZE(switch_acl_ingress_0_answer), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < INGRESS_CONFIGURABLE_ACL_0_TCAM_ANSWER_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x %08x %08x %08x]\n", __func__,
				index, address, value[4], value[3], value[2], value[1], value[0]);

        return 1;

ERR:
        switch_acl_ingress_0_answer_usage();
        return 0;
}


struct switch_field_def switch_acl_ingress_1_large_table[] = {
	{INGRESS_ACL1_LARGE_ARG_VALID, "valid", 1, 0},
	//{INGRESS_ACL1_LARGE_ARG_COMPAREDATA, "compareData", 135, 1},
	{INGRESS_ACL1_LARGE_ARG_COMPAREDATA0, "compareData0", 64, 1},
	{INGRESS_ACL1_LARGE_ARG_COMPAREDATA1, "compareData1", 64, 65},
	{INGRESS_ACL1_LARGE_ARG_COMPAREDATA2, "compareData2", 7, 129},

	{INGRESS_ACL1_LARGE_ARG_SENDTOCPU, "sendToCpu", 1, 136},
	{INGRESS_ACL1_LARGE_ARG_FORCESENDTOCPUORIGPKT, "forceSendToCpuOrigPkt", 1, 137},
	{INGRESS_ACL1_LARGE_ARG_METADATAVALID, "metaDataValid", 1, 138},
	{INGRESS_ACL1_LARGE_ARG_METADATA, "metaData", 16, 139},
	{INGRESS_ACL1_LARGE_ARG_METADATAPRIO, "metaDataPrio", 1, 155},
	{INGRESS_ACL1_LARGE_ARG_DROPENABLE, "dropEnable", 1, 156},
	{INGRESS_ACL1_LARGE_ARG_SENDTOPORT, "sendToPort", 1, 157},
	{INGRESS_ACL1_LARGE_ARG_DESTPORT, "destPort", 3, 158},
	{INGRESS_ACL1_LARGE_ARG_INPUTMIRROR, "inputMirror", 1, 161},
	{INGRESS_ACL1_LARGE_ARG_DESTINPUTMIRROR, "destInputMirror", 3, 162},
	{INGRESS_ACL1_LARGE_ARG_IMPRIO, "imPrio", 1, 165},
	{INGRESS_ACL1_LARGE_ARG_NOLEARNING, "noLearning", 1, 166},
	{INGRESS_ACL1_LARGE_ARG_UPDATECOUNTER, "updateCounter", 1, 167},
	{INGRESS_ACL1_LARGE_ARG_COUNTER, "counter", 6, 168},
	{INGRESS_ACL1_LARGE_ARG_UPDATETOSEXP, "updateTosExp", 1, 174},
	{INGRESS_ACL1_LARGE_ARG_NEWTOSEXP, "newTosExp", 8, 175},
	{INGRESS_ACL1_LARGE_ARG_TOSMASK, "tosMask", 8, 183},
	{INGRESS_ACL1_LARGE_ARG_UPDATECFIDEI, "updateCfiDei", 1, 191},
	{INGRESS_ACL1_LARGE_ARG_NEWCFIDEIVALUE, "newCfiDeiValue", 1, 192},
	{INGRESS_ACL1_LARGE_ARG_UPDATEPCP, "updatePcp", 1, 193},
	{INGRESS_ACL1_LARGE_ARG_NEWPCPVALUE, "newPcpValue", 3, 194},
	{INGRESS_ACL1_LARGE_ARG_UPDATEVID, "updateVid", 1, 197},
	{INGRESS_ACL1_LARGE_ARG_NEWVIDVALUE, "newVidValue", 12, 198},
	{INGRESS_ACL1_LARGE_ARG_UPDATEETYPE, "updateEType", 1, 210},
	{INGRESS_ACL1_LARGE_ARG_NEWETHTYPE, "newEthType", 2, 211},
	{INGRESS_ACL1_LARGE_ARG_CFIDEIPRIO, "cfiDeiPrio", 1, 213},
	{INGRESS_ACL1_LARGE_ARG_PCPPRIO, "pcpPrio", 1, 214},
	{INGRESS_ACL1_LARGE_ARG_VIDPRIO, "vidPrio", 1, 215},
	{INGRESS_ACL1_LARGE_ARG_ETHPRIO, "ethPrio", 1, 216},
	{INGRESS_ACL1_LARGE_ARG_ENABLEUPDATEIP, "enableUpdateIp", 1, 217},
	{INGRESS_ACL1_LARGE_ARG_UPDATESAORDA, "updateSaOrDa", 1, 218},
	{INGRESS_ACL1_LARGE_ARG_NEWIPVALUE, "newIpValue", 32, 219, SWITCH_FIELD_TYPE_IPV4},
	{INGRESS_ACL1_LARGE_ARG_ENABLEUPDATEL4, "enableUpdateL4", 1, 251},
	{INGRESS_ACL1_LARGE_ARG_UPDATEL4SPORDP, "updateL4SpOrDp", 1, 252},
	{INGRESS_ACL1_LARGE_ARG_NEWL4VALUE, "newL4Value", 16, 253},
	{INGRESS_ACL1_LARGE_ARG_NATOPVALID, "natOpValid", 1, 269},
	{INGRESS_ACL1_LARGE_ARG_NATOPPTR, "natOpPtr", 11, 270},
	{INGRESS_ACL1_LARGE_ARG_NATOPPRIO, "natOpPrio", 1, 281},
	{INGRESS_ACL1_LARGE_ARG_PTP, "ptp", 1, 282},
	{INGRESS_ACL1_LARGE_ARG_TUNNELENTRY, "tunnelEntry", 1, 283},
	{INGRESS_ACL1_LARGE_ARG_TUNNELENTRYUCMC, "tunnelEntryUcMc", 1, 284},
	{INGRESS_ACL1_LARGE_ARG_TUNNELENTRYPTR, "tunnelEntryPtr", 4, 285},
	{INGRESS_ACL1_LARGE_ARG_TUNNELENTRYPRIO, "tunnelEntryPrio", 1, 289},
	{INGRESS_ACL1_LARGE_ARG_FORCECOLOR, "forceColor", 1, 290},
	{INGRESS_ACL1_LARGE_ARG_COLOR, "color", 2, 291},
	{INGRESS_ACL1_LARGE_ARG_FORCECOLORPRIO, "forceColorPrio", 1, 293},
	{INGRESS_ACL1_LARGE_ARG_MMPVALID, "mmpValid", 1, 294},
	{INGRESS_ACL1_LARGE_ARG_MMPPTR, "mmpPtr", 5, 295},
	{INGRESS_ACL1_LARGE_ARG_MMPORDER, "mmpOrder", 2, 300},
	{INGRESS_ACL1_LARGE_ARG_FORCEQUEUE, "forceQueue", 1, 302},
	{INGRESS_ACL1_LARGE_ARG_EQUEUE, "eQueue", 3, 303},
	{INGRESS_ACL1_LARGE_ARG_FORCEQUEUEPRIO, "forceQueuePrio", 1, 306},
	{INGRESS_ACL1_LARGE_ARG_FORCEVIDVALID, "forceVidValid", 1, 307},
	{INGRESS_ACL1_LARGE_ARG_FORCEVID, "forceVid", 12, 308},
	{INGRESS_ACL1_LARGE_ARG_FORCEVIDPRIO, "forceVidPrio", 1, 320},
};

void switch_acl_ingress_1_large_table_usage(void)
{
	printf("Usage: npecmd switch acl ingress1_large <index> <--arg=value>\n");

	printf("maximum index: %d\n", INGRESS_CONFIGURABLE_ACL_1_LARGE_TABLE_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_acl_ingress_1_large_table); i++)
		printf(" %s", switch_acl_ingress_1_large_table[i].name);

	printf("\n");
}

int switch_acl_ingress_1_large_table_config(int argc, char **argv)
{
	u64 address;
	u32 value[INGRESS_CONFIGURABLE_ACL_1_LARGE_TABLE_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= INGRESS_CONFIGURABLE_ACL_1_LARGE_TABLE_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = INGRESS_CONFIGURABLE_ACL_1_LARGE_TABLE + index * INGRESS_CONFIGURABLE_ACL_1_LARGE_TABLE_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < INGRESS_CONFIGURABLE_ACL_1_LARGE_TABLE_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_acl_ingress_1_large_table, ARRAY_SIZE(switch_acl_ingress_1_large_table), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < INGRESS_CONFIGURABLE_ACL_1_LARGE_TABLE_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x %08x %08x]\n", __func__, index, address, value[3], value[2], value[1], value[0]);

	return 1;

ERR:
	switch_acl_ingress_1_large_table_usage();
	return 0;
}

struct switch_field_def switch_acl_ingress_1_small_table[] = {
	{INGRESS_ACL1_SMALL_ARG_VALID, "valid", 1, 0},
	//{INGRESS_ACL1_SMALL_ARG_COMPAREDATA, "compareData", 135, 1},
	{INGRESS_ACL1_SMALL_ARG_COMPAREDATA0, "compareData0", 64, 1},
	{INGRESS_ACL1_SMALL_ARG_COMPAREDATA1, "compareData1", 64, 65},
	{INGRESS_ACL1_SMALL_ARG_COMPAREDATA2, "compareData2", 7, 129},

	{INGRESS_ACL1_SMALL_ARG_SENDTOCPU, "sendToCpu", 1, 136},
	{INGRESS_ACL1_SMALL_ARG_FORCESENDTOCPUORIGPKT, "forceSendToCpuOrigPkt", 1, 137},
	{INGRESS_ACL1_SMALL_ARG_METADATAVALID, "metaDataValid", 1, 138},
	{INGRESS_ACL1_SMALL_ARG_METADATA, "metaData", 16, 139},
	{INGRESS_ACL1_SMALL_ARG_METADATAPRIO, "metaDataPrio", 1, 155},
	{INGRESS_ACL1_SMALL_ARG_DROPENABLE, "dropEnable", 1, 156},
	{INGRESS_ACL1_SMALL_ARG_SENDTOPORT, "sendToPort", 1, 157},
	{INGRESS_ACL1_SMALL_ARG_DESTPORT, "destPort", 3, 158},
	{INGRESS_ACL1_SMALL_ARG_INPUTMIRROR, "inputMirror", 1, 161},
	{INGRESS_ACL1_SMALL_ARG_DESTINPUTMIRROR, "destInputMirror", 3, 162},
	{INGRESS_ACL1_SMALL_ARG_IMPRIO, "imPrio", 1, 165},
	{INGRESS_ACL1_SMALL_ARG_NOLEARNING, "noLearning", 1, 166},
	{INGRESS_ACL1_SMALL_ARG_UPDATECOUNTER, "updateCounter", 1, 167},
	{INGRESS_ACL1_SMALL_ARG_COUNTER, "counter", 6, 168},
	{INGRESS_ACL1_SMALL_ARG_UPDATETOSEXP, "updateTosExp", 1, 174},
	{INGRESS_ACL1_SMALL_ARG_NEWTOSEXP, "newTosExp", 8, 175},
	{INGRESS_ACL1_SMALL_ARG_TOSMASK, "tosMask", 8, 183},
	{INGRESS_ACL1_SMALL_ARG_UPDATECFIDEI, "updateCfiDei", 1, 191},
	{INGRESS_ACL1_SMALL_ARG_NEWCFIDEIVALUE, "newCfiDeiValue", 1, 192},
	{INGRESS_ACL1_SMALL_ARG_UPDATEPCP, "updatePcp", 1, 193},
	{INGRESS_ACL1_SMALL_ARG_NEWPCPVALUE, "newPcpValue", 3, 194},
	{INGRESS_ACL1_SMALL_ARG_UPDATEVID, "updateVid", 1, 197},
	{INGRESS_ACL1_SMALL_ARG_NEWVIDVALUE, "newVidValue", 12, 198},
	{INGRESS_ACL1_SMALL_ARG_UPDATEETYPE, "updateEType", 1, 210},
	{INGRESS_ACL1_SMALL_ARG_NEWETHTYPE, "newEthType", 2, 211},
	{INGRESS_ACL1_SMALL_ARG_CFIDEIPRIO, "cfiDeiPrio", 1, 213},
	{INGRESS_ACL1_SMALL_ARG_PCPPRIO, "pcpPrio", 1, 214},
	{INGRESS_ACL1_SMALL_ARG_VIDPRIO, "vidPrio", 1, 215},
	{INGRESS_ACL1_SMALL_ARG_ETHPRIO, "ethPrio", 1, 216},
	{INGRESS_ACL1_SMALL_ARG_ENABLEUPDATEIP, "enableUpdateIp", 1, 217},
	{INGRESS_ACL1_SMALL_ARG_UPDATESAORDA, "updateSaOrDa", 1, 218},
	{INGRESS_ACL1_SMALL_ARG_NEWIPVALUE, "newIpValue", 32, 219, SWITCH_FIELD_TYPE_IPV4},
	{INGRESS_ACL1_SMALL_ARG_ENABLEUPDATEL4, "enableUpdateL4", 1, 251},
	{INGRESS_ACL1_SMALL_ARG_UPDATEL4SPORDP, "updateL4SpOrDp", 1, 252},
	{INGRESS_ACL1_SMALL_ARG_NEWL4VALUE, "newL4Value", 16, 253},
	{INGRESS_ACL1_SMALL_ARG_NATOPVALID, "natOpValid", 1, 269},
	{INGRESS_ACL1_SMALL_ARG_NATOPPTR, "natOpPtr", 11, 270},
	{INGRESS_ACL1_SMALL_ARG_NATOPPRIO, "natOpPrio", 1, 281},
	{INGRESS_ACL1_SMALL_ARG_PTP, "ptp", 1, 282},
	{INGRESS_ACL1_SMALL_ARG_TUNNELENTRY, "tunnelEntry", 1, 283},
	{INGRESS_ACL1_SMALL_ARG_TUNNELENTRYUCMC, "tunnelEntryUcMc", 1, 284},
	{INGRESS_ACL1_SMALL_ARG_TUNNELENTRYPTR, "tunnelEntryPtr", 4, 285},
	{INGRESS_ACL1_SMALL_ARG_TUNNELENTRYPRIO, "tunnelEntryPrio", 1, 289},
	{INGRESS_ACL1_SMALL_ARG_FORCECOLOR, "forceColor", 1, 290},
	{INGRESS_ACL1_SMALL_ARG_COLOR, "color", 2, 291},
	{INGRESS_ACL1_SMALL_ARG_FORCECOLORPRIO, "forceColorPrio", 1, 293},
	{INGRESS_ACL1_SMALL_ARG_MMPVALID, "mmpValid", 1, 294},
	{INGRESS_ACL1_SMALL_ARG_MMPPTR, "mmpPtr", 5, 295},
	{INGRESS_ACL1_SMALL_ARG_MMPORDER, "mmpOrder", 2, 300},
	{INGRESS_ACL1_SMALL_ARG_FORCEQUEUE, "forceQueue", 1, 302},
	{INGRESS_ACL1_SMALL_ARG_EQUEUE, "eQueue", 3, 303},
	{INGRESS_ACL1_SMALL_ARG_FORCEQUEUEPRIO, "forceQueuePrio", 1, 306},
	{INGRESS_ACL1_SMALL_ARG_FORCEVIDVALID, "forceVidValid", 1, 307},
	{INGRESS_ACL1_SMALL_ARG_FORCEVID, "forceVid", 12, 308},
	{INGRESS_ACL1_SMALL_ARG_FORCEVIDPRIO, "forceVidPrio", 1, 320},
};

void switch_acl_ingress_1_small_table_usage(void)
{
	printf("Usage: npecmd switch acl ingress1_small <index> <--arg=value>\n");

	printf("maximum index: %d\n", INGRESS_CONFIGURABLE_ACL_1_SMALL_TABLE_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_acl_ingress_1_small_table); i++)
		printf(" %s", switch_acl_ingress_1_small_table[i].name);

	printf("\n");
}

int switch_acl_ingress_1_small_table_config(int argc, char **argv)
{
	u64 address;
	u32 value[INGRESS_CONFIGURABLE_ACL_1_SMALL_TABLE_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= INGRESS_CONFIGURABLE_ACL_1_SMALL_TABLE_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = INGRESS_CONFIGURABLE_ACL_1_SMALL_TABLE + index * INGRESS_CONFIGURABLE_ACL_1_SMALL_TABLE_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < INGRESS_CONFIGURABLE_ACL_1_SMALL_TABLE_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_acl_ingress_1_small_table, ARRAY_SIZE(switch_acl_ingress_1_small_table), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < INGRESS_CONFIGURABLE_ACL_1_SMALL_TABLE_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x %08x %08x]\n", __func__, index, address, value[3], value[2], value[1], value[0]);


	return 1;

ERR:
	switch_acl_ingress_1_small_table_usage();
	return 0;
}

struct switch_field_def switch_acl_ingress_1_mask[] = {
       //{INGRESS_ACL1_MASK_ARG_MASK_SMALL, "mask_small", 135, 0},
       {INGRESS_ACL1_MASK_ARG_MASK_SMALL0, "mask_small0", 64, 0},
       {INGRESS_ACL1_MASK_ARG_MASK_SMALL1, "mask_small1", 64, 64},
       {INGRESS_ACL1_MASK_ARG_MASK_SMALL2, "mask_small2", 7, 128},
       //{INGRESS_ACL1_MASK_ARG_MASK_LARGE, "mask_large", 135, 135},
       {INGRESS_ACL1_MASK_ARG_MASK_LARGE0, "mask_large0", 64, 135},
       {INGRESS_ACL1_MASK_ARG_MASK_LARGE1, "mask_large1", 64, 199},
       {INGRESS_ACL1_MASK_ARG_MASK_LARGE2, "mask_large2", 7, 263},
};

void switch_acl_ingress_1_mask_usage(void)
{
       printf("Usage: npecmd switch acl ingress1_mask <index> <--arg=value>\n");

       printf("maximum index: %d\n", INGRESS_CONFIGURABLE_ACL_1_SEARCH_MASK_MAX - 1);
       printf("arg :=");
       for (int i = 0; i < ARRAY_SIZE(switch_acl_ingress_1_mask); i++)
               printf(" %s", switch_acl_ingress_1_mask[i].name);

       printf("\n");
}

int switch_acl_ingress_1_mask_config(int argc, char **argv)
{
       u64 address;
       u32 value[INGRESS_CONFIGURABLE_ACL_1_SEARCH_MASK_ADDR_PER_ENTRY] = {0}, index;
       int i;
       int high_valid = 0;

       if (argc <= 1) {
               printf("[%s] Failed! Invalid argc\n", __func__);
               goto ERR;
       }

       //check index
       if ((sscanf(argv[0], "%i", &index) != 1) || (index >= INGRESS_CONFIGURABLE_ACL_1_SEARCH_MASK_MAX)) {
               printf("[%s] Failed! Invalid index\n", __func__);
               goto ERR;
       }

       address = INGRESS_CONFIGURABLE_ACL_1_SEARCH_MASK + index * INGRESS_CONFIGURABLE_ACL_1_SEARCH_MASK_ADDR_PER_ENTRY;

       //get old value
       for (i = 0; i < INGRESS_CONFIGURABLE_ACL_1_SEARCH_MASK_ADDR_PER_ENTRY; i++)
               switch_read(address + i, &value[i]);

       //update
       if (!switch_field_setup(argc, argv, switch_acl_ingress_1_mask, ARRAY_SIZE(switch_acl_ingress_1_mask), value)) {
               printf("[%s] Failed! Invalid arg list\n", __func__);
               goto ERR;
       }

       //config
       for (i = 0; i < INGRESS_CONFIGURABLE_ACL_1_SEARCH_MASK_ADDR_PER_ENTRY; i++)
               switch_write(address + i, value[i]);

       printf("[%s] index %d addr 0x%lx value %08x\n", __func__, index, address, value[0]);

       npecmd_dbg("[%s] new value = 0x", __func__);
       for (i = INGRESS_CONFIGURABLE_ACL_1_SEARCH_MASK_ADDR_PER_ENTRY - 1; i >= 0 ; i--) {
               if (!high_valid && !value[i])
                       continue;
               else {
                       high_valid = 1;
                       npecmd_dbg("%08x", value[i]);
               }

       }
       npecmd_dbg("\n");

       return 1;

ERR:
       switch_acl_ingress_1_mask_usage();
       return 0;
}

struct switch_field_def switch_acl_ingress_1_tcam[] = {
	{INGRESS_ACL1_TCAM_ARG_VALID, "valid", 1, 0},
	//{INGRESS_ACL1_TCAM_ARG_MASK, "mask", 135, 1},
	{INGRESS_ACL1_TCAM_ARG_MASK0, "mask0", 64, 1},
	{INGRESS_ACL1_TCAM_ARG_MASK1, "mask1", 64, 65},
	{INGRESS_ACL1_TCAM_ARG_MASK2, "mask2", 7, 129},
	//{INGRESS_ACL1_TCAM_ARG_COMPAREDATA, "compareData", 135, 136},
	{INGRESS_ACL1_TCAM_ARG_COMPAREDATA0, "compareData0", 64, 136},
	{INGRESS_ACL1_TCAM_ARG_COMPAREDATA1, "compareData1", 64, 200},
	{INGRESS_ACL1_TCAM_ARG_COMPAREDATA2, "compareData2", 7, 264},
};

void switch_acl_ingress_1_tcam_usage(void)
{
        printf("Usage: npecmd switch acl ingress1_tcam <index> <--arg=value>\n");

	printf("maximum index: %d\n", INGRESS_CONFIGURABLE_ACL_1_TCAM_MAX - 1);
        printf("arg :=");
        for (int i = 0; i < ARRAY_SIZE(switch_acl_ingress_1_tcam); i++)
                printf(" %s", switch_acl_ingress_1_tcam[i].name);

        printf("\n");
}

int switch_acl_ingress_1_tcam_config(int argc, char **argv)
{
	u64 address;
	u32 value[INGRESS_CONFIGURABLE_ACL_1_TCAM_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= INGRESS_CONFIGURABLE_ACL_1_TCAM_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = INGRESS_CONFIGURABLE_ACL_1_TCAM + index * INGRESS_CONFIGURABLE_ACL_1_TCAM_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < INGRESS_CONFIGURABLE_ACL_1_TCAM_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_acl_ingress_1_tcam, ARRAY_SIZE(switch_acl_ingress_1_tcam), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < INGRESS_CONFIGURABLE_ACL_1_TCAM_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x %08x %08x]\n", __func__, index, address, value[3], value[2], value[1], value[0]);

        return 1;

ERR:
        switch_acl_ingress_1_tcam_usage();
        return 0;
}

struct switch_field_def switch_acl_ingress_1_answer[] = {
	{INGRESS_ACL1_TCAM_ANSWER_ARG_SENDTOCPU, "sendToCpu", 1, 0},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_FORCESENDTOCPUORIGPKT, "forceSendToCpuOrigPkt", 1, 1},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_METADATAVALID, "metaDataValid", 1, 2},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_METADATA, "metaData", 16, 3},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_METADATAPRIO, "metaDataPrio", 1, 19},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_DROPENABLE, "dropEnable", 1, 20},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_SENDTOPORT, "sendToPort", 1, 21},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_DESTPORT, "destPort", 3, 22},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_INPUTMIRROR, "inputMirror", 1, 25},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_DESTINPUTMIRROR, "destInputMirror", 3, 26},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_IMPRIO, "imPrio", 1, 29},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_NOLEARNING, "noLearning", 1, 30},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_UPDATECOUNTER, "updateCounter", 1, 31},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_COUNTER, "counter", 6, 32},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_UPDATETOSEXP, "updateTosExp", 1, 38},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_NEWTOSEXP, "newTosExp", 8, 39},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_TOSMASK, "tosMask", 8, 47},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_UPDATECFIDEI, "updateCfiDei", 1, 55},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_NEWCFIDEIVALUE, "newCfiDeiValue", 1, 56},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_UPDATEPCP, "updatePcp", 1, 57},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_NEWPCPVALUE, "newPcpValue", 3, 58},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_UPDATEVID, "updateVid", 1, 61},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_NEWVIDVALUE, "newVidValue", 12, 62},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_UPDATEETYPE, "updateEType", 1, 74},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_NEWETHTYPE, "newEthType", 2, 75},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_CFIDEIPRIO, "cfiDeiPrio", 1, 77},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_PCPPRIO, "pcpPrio", 1, 78},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_VIDPRIO, "vidPrio", 1, 79},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_ETHPRIO, "ethPrio", 1, 80},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_ENABLEUPDATEIP, "enableUpdateIp", 1, 81},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_UPDATESAORDA, "updateSaOrDa", 1, 82},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_NEWIPVALUE, "newIpValue", 32, 83, SWITCH_FIELD_TYPE_IPV4},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_ENABLEUPDATEL4, "enableUpdateL4", 1, 115},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_UPDATEL4SPORDP, "updateL4SpOrDp", 1, 116},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_NEWL4VALUE, "newL4Value", 16, 117},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_NATOPVALID, "natOpValid", 1, 133},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_NATOPPTR, "natOpPtr", 11, 134},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_NATOPPRIO, "natOpPrio", 1, 145},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_PTP, "ptp", 1, 146},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_TUNNELENTRY, "tunnelEntry", 1, 147},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_TUNNELENTRYUCMC, "tunnelEntryUcMc", 1, 148},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_TUNNELENTRYPTR, "tunnelEntryPtr", 4, 149},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_TUNNELENTRYPRIO, "tunnelEntryPrio", 1, 153},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_FORCECOLOR, "forceColor", 1, 154},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_COLOR, "color", 2, 155},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_FORCECOLORPRIO, "forceColorPrio", 1, 157},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_MMPVALID, "mmpValid", 1, 158},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_MMPPTR, "mmpPtr", 5, 159},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_MMPORDER, "mmpOrder", 2, 164},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_FORCEQUEUE, "forceQueue", 1, 166},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_EQUEUE, "eQueue", 3, 167},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_FORCEQUEUEPRIO, "forceQueuePrio", 1, 170},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_FORCEVIDVALID, "forceVidValid", 1, 171},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_FORCEVID, "forceVid", 12, 172},
	{INGRESS_ACL1_TCAM_ANSWER_ARG_FORCEVIDPRIO, "forceVidPrio", 1, 184},
};

void switch_acl_ingress_1_answer_usage(void)
{
        printf("Usage: npecmd switch acl ingress1_answer <index> <--arg=value>\n");

	printf("maximum index: %d\n", INGRESS_CONFIGURABLE_ACL_1_TCAM_ANSWER_MAX - 1);
        printf("arg :=");
        for (int i = 0; i < ARRAY_SIZE(switch_acl_ingress_1_answer); i++)
                printf(" %s", switch_acl_ingress_1_answer[i].name);

        printf("\n");
}

int switch_acl_ingress_1_answer_config(int argc, char **argv)
{
	u64 address;
	u32 value[INGRESS_CONFIGURABLE_ACL_1_TCAM_ANSWER_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= INGRESS_CONFIGURABLE_ACL_1_TCAM_ANSWER_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = INGRESS_CONFIGURABLE_ACL_1_TCAM_ANSWER + index * INGRESS_CONFIGURABLE_ACL_1_TCAM_ANSWER_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < INGRESS_CONFIGURABLE_ACL_1_TCAM_ANSWER_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_acl_ingress_1_answer, ARRAY_SIZE(switch_acl_ingress_1_answer), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < INGRESS_CONFIGURABLE_ACL_1_TCAM_ANSWER_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x %08x %08x %08x]\n", __func__,
				index, address, value[4], value[3], value[2], value[1], value[0]);

        return 1;

ERR:
        switch_acl_ingress_1_answer_usage();
        return 0;
}

struct switch_field_def switch_acl_ingress_2_tcam[] = {
	{INGRESS_ACL2_TCAM_ARG_VALID, "valid", 1, 0},
	//{INGRESS_ACL2_TCAM_ARG_MASK, "mask", 540, 1},
	{INGRESS_ACL2_TCAM_ARG_MASK0, "mask0", 64, 1},
	{INGRESS_ACL2_TCAM_ARG_MASK1, "mask1", 64, 65},
	{INGRESS_ACL2_TCAM_ARG_MASK2, "mask2", 64, 129},
	{INGRESS_ACL2_TCAM_ARG_MASK3, "mask3", 64, 193},
	{INGRESS_ACL2_TCAM_ARG_MASK4, "mask4", 64, 257},
	{INGRESS_ACL2_TCAM_ARG_MASK5, "mask5", 64, 321},
	{INGRESS_ACL2_TCAM_ARG_MASK6, "mask6", 64, 385},
	{INGRESS_ACL2_TCAM_ARG_MASK7, "mask7", 64, 449},
	{INGRESS_ACL2_TCAM_ARG_MASK8, "mask8", 28, 513},
	//{INGRESS_ACL2_TCAM_ARG_COMPAREDATA, "compareData", 540, 541},
	{INGRESS_ACL2_TCAM_ARG_COMPAREDATA0, "compareData0", 64, 541},
	{INGRESS_ACL2_TCAM_ARG_COMPAREDATA1, "compareData1", 64, 605},
	{INGRESS_ACL2_TCAM_ARG_COMPAREDATA2, "compareData2", 64, 669},
	{INGRESS_ACL2_TCAM_ARG_COMPAREDATA3, "compareData3", 64, 733},
	{INGRESS_ACL2_TCAM_ARG_COMPAREDATA4, "compareData4", 64, 797},
	{INGRESS_ACL2_TCAM_ARG_COMPAREDATA5, "compareData5", 64, 861},
	{INGRESS_ACL2_TCAM_ARG_COMPAREDATA6, "compareData6", 64, 925},
	{INGRESS_ACL2_TCAM_ARG_COMPAREDATA7, "compareData7", 64, 989},
	{INGRESS_ACL2_TCAM_ARG_COMPAREDATA8, "compareData8", 28, 1053},
};

void switch_acl_ingress_2_tcam_usage(void)
{
        printf("Usage: npecmd switch acl ingress2_tcam <index> <--arg=value>\n");

	printf("maximum index: %d\n", INGRESS_CONFIGURABLE_ACL_2_TCAM_MAX - 1);
        printf("arg :=");
        for (int i = 0; i < ARRAY_SIZE(switch_acl_ingress_2_tcam); i++)
                printf(" %s", switch_acl_ingress_2_tcam[i].name);

        printf("\n");
}

int switch_acl_ingress_2_tcam_config(int argc, char **argv)
{
	u64 address;
	u32 value[INGRESS_CONFIGURABLE_ACL_2_TCAM_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= INGRESS_CONFIGURABLE_ACL_2_TCAM_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = INGRESS_CONFIGURABLE_ACL_2_TCAM + index * INGRESS_CONFIGURABLE_ACL_2_TCAM_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < INGRESS_CONFIGURABLE_ACL_2_TCAM_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_acl_ingress_2_tcam, ARRAY_SIZE(switch_acl_ingress_2_tcam), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < INGRESS_CONFIGURABLE_ACL_2_TCAM_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x %08x %08x]\n", __func__, index, address, value[3], value[2], value[1], value[0]);

        return 1;

ERR:
        switch_acl_ingress_2_tcam_usage();
        return 0;
}

struct switch_field_def switch_acl_ingress_2_answer[] = {
	{INGRESS_ACL2_TCAM_ANSWER_ARG_SENDTOCPU, "sendToCpu", 1, 0},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_FORCESENDTOCPUORIGPKT, "forceSendToCpuOrigPkt", 1, 1},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_METADATAVALID, "metaDataValid", 1, 2},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_METADATA, "metaData", 16, 3},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_METADATAPRIO, "metaDataPrio", 1, 19},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_DROPENABLE, "dropEnable", 1, 20},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_SENDTOPORT, "sendToPort", 1, 21},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_DESTPORT, "destPort", 3, 22},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_INPUTMIRROR, "inputMirror", 1, 25},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_DESTINPUTMIRROR, "destInputMirror", 3, 26},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_IMPRIO, "imPrio", 1, 29},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_NOLEARNING, "noLearning", 1, 30},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_UPDATECOUNTER, "updateCounter", 1, 31},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_COUNTER, "counter", 6, 32},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_UPDATETOSEXP, "updateTosExp", 1, 38},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_NEWTOSEXP, "newTosExp", 8, 39},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_TOSMASK, "tosMask", 8, 47},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_UPDATECFIDEI, "updateCfiDei", 1, 55},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_NEWCFIDEIVALUE, "newCfiDeiValue", 1, 56},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_UPDATEPCP, "updatePcp", 1, 57},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_NEWPCPVALUE, "newPcpValue", 3, 58},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_UPDATEVID, "updateVid", 1, 61},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_NEWVIDVALUE, "newVidValue", 12, 62},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_UPDATEETYPE, "updateEType", 1, 74},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_NEWETHTYPE, "newEthType", 2, 75},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_CFIDEIPRIO, "cfiDeiPrio", 1, 77},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_PCPPRIO, "pcpPrio", 1, 78},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_VIDPRIO, "vidPrio", 1, 79},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_ETHPRIO, "ethPrio", 1, 80},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_ENABLEUPDATEIP, "enableUpdateIp", 1, 81},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_UPDATESAORDA, "updateSaOrDa", 1, 82},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_NEWIPVALUE, "newIpValue", 32, 83, SWITCH_FIELD_TYPE_IPV4},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_ENABLEUPDATEL4, "enableUpdateL4", 1, 115},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_UPDATEL4SPORDP, "updateL4SpOrDp", 1, 116},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_NEWL4VALUE, "newL4Value", 16, 117},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_NATOPVALID, "natOpValid", 1, 133},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_NATOPPTR, "natOpPtr", 11, 134},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_NATOPPRIO, "natOpPrio", 1, 145},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_PTP, "ptp", 1, 146},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_TUNNELENTRY, "tunnelEntry", 1, 147},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_TUNNELENTRYUCMC, "tunnelEntryUcMc", 1, 148},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_TUNNELENTRYPTR, "tunnelEntryPtr", 4, 149},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_TUNNELENTRYPRIO, "tunnelEntryPrio", 1, 153},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_FORCECOLOR, "forceColor", 1, 154},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_COLOR, "color", 2, 155},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_FORCECOLORPRIO, "forceColorPrio", 1, 157},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_MMPVALID, "mmpValid", 1, 158},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_MMPPTR, "mmpPtr", 5, 159},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_MMPORDER, "mmpOrder", 2, 164},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_FORCEQUEUE, "forceQueue", 1, 166},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_EQUEUE, "eQueue", 3, 167},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_FORCEQUEUEPRIO, "forceQueuePrio", 1, 170},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_FORCEVIDVALID, "forceVidValid", 1, 171},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_FORCEVID, "forceVid", 12, 172},
	{INGRESS_ACL2_TCAM_ANSWER_ARG_FORCEVIDPRIO, "forceVidPrio", 1, 184},
};

void switch_acl_ingress_2_answer_usage(void)
{
        printf("Usage: npecmd switch acl ingress2_answer <index> <--arg=value>\n");

	printf("maximum index: %d\n", INGRESS_CONFIGURABLE_ACL_2_TCAM_ANSWER_MAX - 1);
        printf("arg :=");
        for (int i = 0; i < ARRAY_SIZE(switch_acl_ingress_2_answer); i++)
                printf(" %s", switch_acl_ingress_2_answer[i].name);

        printf("\n");
}

int switch_acl_ingress_2_answer_config(int argc, char **argv)
{
	u64 address;
	u32 value[INGRESS_CONFIGURABLE_ACL_2_TCAM_ANSWER_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= INGRESS_CONFIGURABLE_ACL_2_TCAM_ANSWER_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = INGRESS_CONFIGURABLE_ACL_2_TCAM_ANSWER + index * INGRESS_CONFIGURABLE_ACL_2_TCAM_ANSWER_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < INGRESS_CONFIGURABLE_ACL_2_TCAM_ANSWER_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_acl_ingress_2_answer, ARRAY_SIZE(switch_acl_ingress_2_answer), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < INGRESS_CONFIGURABLE_ACL_2_TCAM_ANSWER_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x %08x %08x %08x]\n", __func__,
				index, address, value[4], value[3], value[2], value[1], value[0]);

        return 1;

ERR:
        switch_acl_ingress_2_answer_usage();
        return 0;
}

struct switch_field_def switch_acl_ingress_3_tcam[] = {
	{INGRESS_ACL3_TCAM_ARG_VALID, "valid", 1, 0},
	//{INGRESS_ACL3_TCAM_ARG_MASK, "mask", 80, 1},
	{INGRESS_ACL3_TCAM_ARG_MASK0, "mask0", 64, 1},
	{INGRESS_ACL3_TCAM_ARG_MASK1, "mask1", 16, 65},
	//{INGRESS_ACL3_TCAM_ARG_COMPAREDATA, "compareData", 80, 81},
	{INGRESS_ACL3_TCAM_ARG_COMPAREDATA0, "compareData0", 64, 81},
	{INGRESS_ACL3_TCAM_ARG_COMPAREDATA1, "compareData1", 16, 145},
};

void switch_acl_ingress_3_tcam_usage(void)
{
        printf("Usage: npecmd switch acl ingress3_tcam <index> <--arg=value>\n");

	printf("maximum index: %d\n", INGRESS_CONFIGURABLE_ACL_3_TCAM_MAX - 1);
        printf("arg :=");
        for (int i = 0; i < ARRAY_SIZE(switch_acl_ingress_3_tcam); i++)
                printf(" %s", switch_acl_ingress_3_tcam[i].name);

        printf("\n");
}

int switch_acl_ingress_3_tcam_config(int argc, char **argv)
{
	u64 address;
	u32 value[INGRESS_CONFIGURABLE_ACL_3_TCAM_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= INGRESS_CONFIGURABLE_ACL_3_TCAM_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = INGRESS_CONFIGURABLE_ACL_3_TCAM + index * INGRESS_CONFIGURABLE_ACL_3_TCAM_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < INGRESS_CONFIGURABLE_ACL_3_TCAM_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_acl_ingress_3_tcam, ARRAY_SIZE(switch_acl_ingress_3_tcam), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < INGRESS_CONFIGURABLE_ACL_3_TCAM_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x %08x %08x]\n", __func__, index, address, value[3], value[2], value[1], value[0]);

        return 1;

ERR:
        switch_acl_ingress_3_tcam_usage();
        return 0;
}

struct switch_field_def switch_acl_ingress_3_answer[] = {
	{INGRESS_ACL3_TCAM_ANSWER_ARG_SENDTOCPU, "sendToCpu", 1, 0},
	{INGRESS_ACL3_TCAM_ANSWER_ARG_FORCESENDTOCPUORIGPKT, "forceSendToCpuOrigPkt", 1, 1},
	{INGRESS_ACL3_TCAM_ANSWER_ARG_METADATAVALID, "metaDataValid", 1, 2},
	{INGRESS_ACL3_TCAM_ANSWER_ARG_METADATA, "metaData", 16, 3},
	{INGRESS_ACL3_TCAM_ANSWER_ARG_METADATAPRIO, "metaDataPrio", 1, 19},
	{INGRESS_ACL3_TCAM_ANSWER_ARG_DROPENABLE, "dropEnable", 1, 20},
	{INGRESS_ACL3_TCAM_ANSWER_ARG_SENDTOPORT, "sendToPort", 1, 21},
	{INGRESS_ACL3_TCAM_ANSWER_ARG_DESTPORT, "destPort", 3, 22},
	{INGRESS_ACL3_TCAM_ANSWER_ARG_FORCECOLOR, "forceColor", 1, 25},
	{INGRESS_ACL3_TCAM_ANSWER_ARG_COLOR, "color", 2, 26},
	{INGRESS_ACL3_TCAM_ANSWER_ARG_FORCECOLORPRIO, "forceColorPrio", 1, 28},
	{INGRESS_ACL3_TCAM_ANSWER_ARG_MMPVALID, "mmpValid", 1, 29},
	{INGRESS_ACL3_TCAM_ANSWER_ARG_MMPPTR, "mmpPtr", 5, 30},
	{INGRESS_ACL3_TCAM_ANSWER_ARG_MMPORDER, "mmpOrder", 2, 35},
	{INGRESS_ACL3_TCAM_ANSWER_ARG_FORCEQUEUE, "forceQueue", 1, 37},
	{INGRESS_ACL3_TCAM_ANSWER_ARG_EQUEUE, "eQueue", 3, 38},
	{INGRESS_ACL3_TCAM_ANSWER_ARG_FORCEQUEUEPRIO, "forceQueuePrio", 1, 41},
};

void switch_acl_ingress_3_answer_usage(void)
{
        printf("Usage: npecmd switch acl ingress3_answer <index> <--arg=value>\n");

	printf("maximum index: %d\n", INGRESS_CONFIGURABLE_ACL_3_TCAM_ANSWER_MAX - 1);
        printf("arg :=");
        for (int i = 0; i < ARRAY_SIZE(switch_acl_ingress_3_answer); i++)
                printf(" %s", switch_acl_ingress_3_answer[i].name);

        printf("\n");
}

int switch_acl_ingress_3_answer_config(int argc, char **argv)
{
	u64 address;
	u32 value[INGRESS_CONFIGURABLE_ACL_3_TCAM_ANSWER_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= INGRESS_CONFIGURABLE_ACL_3_TCAM_ANSWER_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = INGRESS_CONFIGURABLE_ACL_3_TCAM_ANSWER + index * INGRESS_CONFIGURABLE_ACL_3_TCAM_ANSWER_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < INGRESS_CONFIGURABLE_ACL_3_TCAM_ANSWER_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_acl_ingress_3_answer, ARRAY_SIZE(switch_acl_ingress_3_answer), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < INGRESS_CONFIGURABLE_ACL_3_TCAM_ANSWER_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x %08x %08x %08x]\n", __func__,
				index, address, value[4], value[3], value[2], value[1], value[0]);

        return 1;

ERR:
        switch_acl_ingress_3_answer_usage();
        return 0;
}

/*EGRESS ACL*/
struct switch_field_def switch_egress_acl_rule_pointer_tcam[] = {
	{EGRESS_ACL_RULE_POINTER_TCAM_ARG_VALID, "valid", 1, 0},
	{EGRESS_ACL_RULE_POINTER_TCAM_ARG_DESTPORTMASK_MASK, "destPortMask_mask", 6, 1},
	{EGRESS_ACL_RULE_POINTER_TCAM_ARG_DESTPORTMASK, "destPortMask", 6, 7},
	{EGRESS_ACL_RULE_POINTER_TCAM_ARG_ROUTED_MASK, "routed_mask", 1, 13},
	{EGRESS_ACL_RULE_POINTER_TCAM_ARG_ROUTED, "routed", 1, 14},
	{EGRESS_ACL_RULE_POINTER_TCAM_ARG_VRF_MASK, "vrf_mask", 2, 15},
	{EGRESS_ACL_RULE_POINTER_TCAM_ARG_VRF, "vrf", 2, 17},
	{EGRESS_ACL_RULE_POINTER_TCAM_ARG_FLOODED_MASK, "flooded_mask", 1, 19},
	{EGRESS_ACL_RULE_POINTER_TCAM_ARG_FLOODED, "flooded", 1, 20},
	{EGRESS_ACL_RULE_POINTER_TCAM_ARG_UCSWITCHED_MASK, "ucSwitched_mask", 1, 21},
	{EGRESS_ACL_RULE_POINTER_TCAM_ARG_UCSWITCHED, "ucSwitched", 1, 22},
	{EGRESS_ACL_RULE_POINTER_TCAM_ARG_MCSWITCHED_MASK, "mcSwitched_mask", 1, 23},
	{EGRESS_ACL_RULE_POINTER_TCAM_ARG_MCSWITCHED, "mcSwitched", 1, 24},
	{EGRESS_ACL_RULE_POINTER_TCAM_ARG_VID_MASK, "vid_mask", 12, 25},
	{EGRESS_ACL_RULE_POINTER_TCAM_ARG_VID, "vid", 12, 37},
	{EGRESS_ACL_RULE_POINTER_TCAM_ARG_L3TYPE_MASK, "l3Type_mask", 2, 49},
	{EGRESS_ACL_RULE_POINTER_TCAM_ARG_L3TYPE, "l3Type", 2, 51},
	{EGRESS_ACL_RULE_POINTER_TCAM_ARG_L4TYPE_MASK, "l4Type_mask", 3, 53},
	{EGRESS_ACL_RULE_POINTER_TCAM_ARG_L4TYPE, "l4Type", 3, 56},
	{EGRESS_ACL_RULE_POINTER_TCAM_ARG_SRCPORT_MASK, "srcPort_mask", 3, 59},
	{EGRESS_ACL_RULE_POINTER_TCAM_ARG_SRCPORT, "srcPort", 3, 62},
};

void switch_egress_acl_rule_pointer_tcam_usage(void)
{
	printf("Usage: npecmd switch acl rule_tcam <index> <rulePtr0> <rulePtr1> <--arg=value>\n");

	printf("maximum index: %d\n", EGRESS_ACL_RULE_POINTER_TCAM_MAX - 1);
	printf("maximum rulePtr0: %d\n", EGRESS_CONFIGURABLE_ACL_0_RULES_SETUP_MAX - 1);
	printf("maximum rulePtr1: %d\n", EGRESS_CONFIGURABLE_ACL_1_RULES_SETUP_MAX - 1);

	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_egress_acl_rule_pointer_tcam); i++)
		printf(" %s", switch_egress_acl_rule_pointer_tcam[i].name);

	printf("\n");
}

int switch_egress_acl_rule_pointer_tcam_config(int argc, char **argv)
{
	u64 address, answer_address;
	u32 value[EGRESS_ACL_RULE_POINTER_TCAM_ADDR_PER_ENTRY] = {0}, index, rulePtr0, rulePtr1;
	u32 answer[EGRESS_ACL_RULE_POINTER_TCAM_ANSWER_ADDR_PER_ENTRY];
	int i;

	if (argc <= 3) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= EGRESS_ACL_RULE_POINTER_TCAM_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	//check rulePtr0/rulePtr1
	if ((sscanf(argv[1], "%i", &rulePtr0) != 1) || (rulePtr0 >= EGRESS_CONFIGURABLE_ACL_0_RULES_SETUP_MAX)) {
		printf("[%s] Failed! Invalid rulePtr0\n", __func__);
		goto ERR;
	}

	if ((sscanf(argv[2], "%i", &rulePtr1) != 1) || (rulePtr1 >= EGRESS_CONFIGURABLE_ACL_1_RULES_SETUP_MAX)) {
		printf("[%s] Failed! Invalid rulePtr1\n", __func__);
		goto ERR;
	}

	answer[0] = rulePtr0 + (rulePtr1 << 3);

	address = EGRESS_ACL_RULE_POINTER_TCAM + index * EGRESS_ACL_RULE_POINTER_TCAM_ADDR_PER_ENTRY;
	answer_address = EGRESS_ACL_RULE_POINTER_TCAM_ANSWER + index * EGRESS_ACL_RULE_POINTER_TCAM_ANSWER_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < EGRESS_ACL_RULE_POINTER_TCAM_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc - 2, argv + 2, switch_egress_acl_rule_pointer_tcam, ARRAY_SIZE(switch_egress_acl_rule_pointer_tcam), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	switch_write(answer_address, answer[0]);

	for (i = 0; i < EGRESS_ACL_RULE_POINTER_TCAM_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);


	printf("[%s] TCAM-index %d addr 0x%lx value 0x[%08x %08x %08x]\n", __func__, index, address, value[2], value[1], value[0]);
	printf("[%s] Answer-index %d addr 0x%lx answer 0x%08x\n", __func__, index, answer_address, answer[0]);

	return 1;

ERR:
	switch_egress_acl_rule_pointer_tcam_usage();
	return 0;
}

struct switch_field_def switch_acl_egress_0_large_table[] = {
	{EGRESS_ACL0_LARGE_ARG_VALID, "valid", 1, 0},
	//{EGRESS_ACL0_LARGE_ARG_COMPAREDATA, "compareData", 135, 1},
	{EGRESS_ACL0_LARGE_ARG_COMPAREDATA0, "compareData0", 64, 1},
	{EGRESS_ACL0_LARGE_ARG_COMPAREDATA1, "compareData1", 64, 65},
	{EGRESS_ACL0_LARGE_ARG_COMPAREDATA2, "compareData2", 7, 129},

	{EGRESS_ACL0_LARGE_ARG_SENDTOCPU, "sendToCpu", 1, 136},
	{EGRESS_ACL0_LARGE_ARG_FORCESENDTOCPUORIGPKT, "forceSendToCpuOrigPkt", 1, 137},
	{EGRESS_ACL0_LARGE_ARG_METADATAVALID, "metaDataValid", 1, 138},
	{EGRESS_ACL0_LARGE_ARG_METADATA, "metaData", 16, 139},
	{EGRESS_ACL0_LARGE_ARG_METADATAPRIO, "metaDataPrio", 1, 155},
	{EGRESS_ACL0_LARGE_ARG_DROPENABLE, "dropEnable", 1, 156},
	{EGRESS_ACL0_LARGE_ARG_SENDTOPORT, "sendToPort", 1, 157},
	{EGRESS_ACL0_LARGE_ARG_DESTPORT, "destPort", 3, 158},
	{EGRESS_ACL0_LARGE_ARG_UPDATECOUNTER, "updateCounter", 1, 161},
	{EGRESS_ACL0_LARGE_ARG_COUNTER, "counter", 6, 162},
	{EGRESS_ACL0_LARGE_ARG_NATOPVALID, "natOpValid", 1, 168},
	{EGRESS_ACL0_LARGE_ARG_NATOPPTR, "natOpPtr", 10, 169},
	{EGRESS_ACL0_LARGE_ARG_NATOPPRIO, "natOpPrio", 1, 179},
	{EGRESS_ACL0_LARGE_ARG_TUNNELENTRY, "tunnelEntry", 1, 180},
	{EGRESS_ACL0_LARGE_ARG_TUNNELENTRYUCMC, "tunnelEntryUcMc", 1, 181},
	{EGRESS_ACL0_LARGE_ARG_TUNNELENTRYPTR, "tunnelEntryPtr", 4, 182},
	{EGRESS_ACL0_LARGE_ARG_TUNNELENTRYPRIO, "tunnelEntryPrio", 1, 186},
};

void switch_acl_egress_0_large_table_usage(void)
{
	printf("Usage: npecmd switch acl egress0_large <index> <--arg=value>\n");

	printf("maximum index: %d\n", EGRESS_CONFIGURABLE_ACL_0_LARGE_TABLE_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_acl_egress_0_large_table); i++)
		printf(" %s", switch_acl_egress_0_large_table[i].name);

	printf("\n");
}

int switch_acl_egress_0_large_table_config(int argc, char **argv)
{
	u64 address;
	u32 value[EGRESS_CONFIGURABLE_ACL_0_LARGE_TABLE_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= EGRESS_CONFIGURABLE_ACL_0_LARGE_TABLE_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = EGRESS_CONFIGURABLE_ACL_0_LARGE_TABLE + index * EGRESS_CONFIGURABLE_ACL_0_LARGE_TABLE_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < EGRESS_CONFIGURABLE_ACL_0_LARGE_TABLE_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_acl_egress_0_large_table, ARRAY_SIZE(switch_acl_egress_0_large_table), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < EGRESS_CONFIGURABLE_ACL_0_LARGE_TABLE_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x %08x %08x]\n", __func__, index, address, value[3], value[2], value[1], value[0]);

	return 1;

ERR:
	switch_acl_egress_0_large_table_usage();
	return 0;
}

struct switch_field_def switch_acl_egress_0_small_table[] = {
	{EGRESS_ACL0_SMALL_ARG_VALID, "valid", 1, 0},
	//{EGRESS_ACL0_SMALL_ARG_COMPAREDATA, "compareData", 135, 1},
	{EGRESS_ACL0_SMALL_ARG_COMPAREDATA0, "compareData0", 64, 1},
	{EGRESS_ACL0_SMALL_ARG_COMPAREDATA1, "compareData1", 64, 65},
	{EGRESS_ACL0_SMALL_ARG_COMPAREDATA2, "compareData2", 7, 129},

	{EGRESS_ACL0_SMALL_ARG_SENDTOCPU, "sendToCpu", 1, 136},
	{EGRESS_ACL0_SMALL_ARG_FORCESENDTOCPUORIGPKT, "forceSendToCpuOrigPkt", 1, 137},
	{EGRESS_ACL0_SMALL_ARG_METADATAVALID, "metaDataValid", 1, 138},
	{EGRESS_ACL0_SMALL_ARG_METADATA, "metaData", 16, 139},
	{EGRESS_ACL0_SMALL_ARG_METADATAPRIO, "metaDataPrio", 1, 155},
	{EGRESS_ACL0_SMALL_ARG_DROPENABLE, "dropEnable", 1, 156},
	{EGRESS_ACL0_SMALL_ARG_SENDTOPORT, "sendToPort", 1, 157},
	{EGRESS_ACL0_SMALL_ARG_DESTPORT, "destPort", 3, 158},
	{EGRESS_ACL0_SMALL_ARG_UPDATECOUNTER, "updateCounter", 1, 161},
	{EGRESS_ACL0_SMALL_ARG_COUNTER, "counter", 6, 162},
	{EGRESS_ACL0_SMALL_ARG_NATOPVALID, "natOpValid", 1, 168},
	{EGRESS_ACL0_SMALL_ARG_NATOPPTR, "natOpPtr", 10, 169},
	{EGRESS_ACL0_SMALL_ARG_NATOPPRIO, "natOpPrio", 1, 179},
	{EGRESS_ACL0_SMALL_ARG_TUNNELENTRY, "tunnelEntry", 1, 180},
	{EGRESS_ACL0_SMALL_ARG_TUNNELENTRYUCMC, "tunnelEntryUcMc", 1, 181},
	{EGRESS_ACL0_SMALL_ARG_TUNNELENTRYPTR, "tunnelEntryPtr", 4, 182},
	{EGRESS_ACL0_SMALL_ARG_TUNNELENTRYPRIO, "tunnelEntryPrio", 1, 186},
};

void switch_acl_egress_0_small_table_usage(void)
{
	printf("Usage: npecmd switch acl egress0_small <index> <--arg=value>\n");

	printf("maximum index: %d\n", EGRESS_CONFIGURABLE_ACL_0_SMALL_TABLE_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_acl_egress_0_small_table); i++)
		printf(" %s", switch_acl_egress_0_small_table[i].name);

	printf("\n");
}

int switch_acl_egress_0_small_table_config(int argc, char **argv)
{
	u64 address;
	u32 value[EGRESS_CONFIGURABLE_ACL_0_SMALL_TABLE_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= EGRESS_CONFIGURABLE_ACL_0_SMALL_TABLE_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = EGRESS_CONFIGURABLE_ACL_0_SMALL_TABLE + index * EGRESS_CONFIGURABLE_ACL_0_SMALL_TABLE_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < EGRESS_CONFIGURABLE_ACL_0_SMALL_TABLE_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_acl_egress_0_small_table, ARRAY_SIZE(switch_acl_egress_0_small_table), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < EGRESS_CONFIGURABLE_ACL_0_SMALL_TABLE_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x %08x %08x]\n", __func__, index, address, value[3], value[2], value[1], value[0]);

	return 1;

ERR:
	switch_acl_egress_0_small_table_usage();
	return 0;
}

struct switch_field_def switch_acl_egress_0_mask[] = {
       //{EGRESS_ACL0_MASK_ARG_MASK_SMALL, "mask_small", 135, 0},
       {EGRESS_ACL0_MASK_ARG_MASK_SMALL0, "mask_small0", 64, 0},
       {EGRESS_ACL0_MASK_ARG_MASK_SMALL1, "mask_small1", 64, 64},
       {EGRESS_ACL0_MASK_ARG_MASK_SMALL2, "mask_small2", 7, 128},
       //{EGRESS_ACL0_MASK_ARG_MASK_LARGE, "mask_large", 135, 135},
       {EGRESS_ACL0_MASK_ARG_MASK_LARGE0, "mask_large0", 64, 135},
       {EGRESS_ACL0_MASK_ARG_MASK_LARGE1, "mask_large1", 64, 199},
       {EGRESS_ACL0_MASK_ARG_MASK_LARGE2, "mask_large2", 7, 263},
};

void switch_acl_egress_0_mask_usage(void)
{
       printf("Usage: npecmd switch acl egress0_mask <index> <--arg=value>\n");

       printf("maximum index: %d\n", EGRESS_CONFIGURABLE_ACL_0_SEARCH_MASK_MAX - 1);
       printf("arg :=");
       for (int i = 0; i < ARRAY_SIZE(switch_acl_egress_0_mask); i++)
               printf(" %s", switch_acl_egress_0_mask[i].name);

       printf("\n");
}

int switch_acl_egress_0_mask_config(int argc, char **argv)
{
       u64 address;
       u32 value[EGRESS_CONFIGURABLE_ACL_0_SEARCH_MASK_ADDR_PER_ENTRY] = {0}, index;
       int i;
       int high_valid = 0;

       if (argc <= 1) {
               printf("[%s] Failed! Invalid argc\n", __func__);
               goto ERR;
       }

       //check index
       if ((sscanf(argv[0], "%i", &index) != 1) || (index >= EGRESS_CONFIGURABLE_ACL_0_SEARCH_MASK_MAX)) {
               printf("[%s] Failed! Invalid index\n", __func__);
               goto ERR;
       }

       address = EGRESS_CONFIGURABLE_ACL_0_SEARCH_MASK + index * EGRESS_CONFIGURABLE_ACL_0_SEARCH_MASK_ADDR_PER_ENTRY;

       //get old value
       for (i = 0; i < EGRESS_CONFIGURABLE_ACL_0_SEARCH_MASK_ADDR_PER_ENTRY; i++)
               switch_read(address + i, &value[i]);

       //update
       if (!switch_field_setup(argc, argv, switch_acl_egress_0_mask, ARRAY_SIZE(switch_acl_egress_0_mask), value)) {
               printf("[%s] Failed! Invalid arg list\n", __func__);
               goto ERR;
       }

       //config
       for (i = 0; i < EGRESS_CONFIGURABLE_ACL_0_SEARCH_MASK_ADDR_PER_ENTRY; i++)
               switch_write(address + i, value[i]);

       printf("[%s] index %d addr 0x%lx value %08x\n", __func__, index, address, value[0]);

       npecmd_dbg("[%s] new value = 0x", __func__);
       for (i = EGRESS_CONFIGURABLE_ACL_0_SEARCH_MASK_ADDR_PER_ENTRY - 1; i >= 0 ; i--) {
               if (!high_valid && !value[i])
                       continue;
               else {
                       high_valid = 1;
                       npecmd_dbg("%08x", value[i]);
               }

       }
       npecmd_dbg("\n");

       return 1;

ERR:
       switch_acl_egress_0_mask_usage();
       return 0;
}

struct switch_field_def switch_acl_egress_0_tcam[] = {
	{EGRESS_ACL0_TCAM_ARG_VALID, "valid", 1, 0},
	//{EGRESS_ACL0_TCAM_ARG_MASK, "mask", 135, 1},
	{EGRESS_ACL0_TCAM_ARG_MASK0, "mask0", 64, 1},
	{EGRESS_ACL0_TCAM_ARG_MASK1, "mask1", 64, 65},
	{EGRESS_ACL0_TCAM_ARG_MASK2, "mask2", 7, 129},
	//{EGRESS_ACL0_TCAM_ARG_COMPAREDATA, "compareData", 135, 136},
	{EGRESS_ACL0_TCAM_ARG_COMPAREDATA0, "compareData0", 64, 136},
	{EGRESS_ACL0_TCAM_ARG_COMPAREDATA1, "compareData1", 64, 200},
	{EGRESS_ACL0_TCAM_ARG_COMPAREDATA2, "compareData2", 7, 264},
};

void switch_acl_egress_0_tcam_usage(void)
{
        printf("Usage: npecmd switch acl egress0_tcam <index> <--arg=value>\n");

	printf("maximum index: %d\n", EGRESS_CONFIGURABLE_ACL_0_TCAM_MAX - 1);
        printf("arg :=");
        for (int i = 0; i < ARRAY_SIZE(switch_acl_egress_0_tcam); i++)
                printf(" %s", switch_acl_egress_0_tcam[i].name);

        printf("\n");
}

int switch_acl_egress_0_tcam_config(int argc, char **argv)
{
	u64 address;
	u32 value[EGRESS_CONFIGURABLE_ACL_0_TCAM_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= EGRESS_CONFIGURABLE_ACL_0_TCAM_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = EGRESS_CONFIGURABLE_ACL_0_TCAM + index * EGRESS_CONFIGURABLE_ACL_0_TCAM_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < EGRESS_CONFIGURABLE_ACL_0_TCAM_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_acl_egress_0_tcam, ARRAY_SIZE(switch_acl_egress_0_tcam), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < EGRESS_CONFIGURABLE_ACL_0_TCAM_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x %08x %08x]\n", __func__, index, address, value[3], value[2], value[1], value[0]);

        return 1;

ERR:
        switch_acl_egress_0_tcam_usage();
        return 0;
}

struct switch_field_def switch_acl_egress_0_answer[] = {
	{EGRESS_ACL0_TCAM_ANSWER_ARG_SENDTOCPU, "sendToCpu", 1, 0},
	{EGRESS_ACL0_TCAM_ANSWER_ARG_FORCESENDTOCPUORIGPKT, "forceSendToCpuOrigPkt", 1, 1},
	{EGRESS_ACL0_TCAM_ANSWER_ARG_METADATAVALID, "metaDataValid", 1, 2},
	{EGRESS_ACL0_TCAM_ANSWER_ARG_METADATA, "metaData", 16, 3},
	{EGRESS_ACL0_TCAM_ANSWER_ARG_METADATAPRIO, "metaDataPrio", 1, 19},
	{EGRESS_ACL0_TCAM_ANSWER_ARG_DROPENABLE, "dropEnable", 1, 20},
	{EGRESS_ACL0_TCAM_ANSWER_ARG_SENDTOPORT, "sendToPort", 1, 21},
	{EGRESS_ACL0_TCAM_ANSWER_ARG_DESTPORT, "destPort", 3, 22},
	{EGRESS_ACL0_TCAM_ANSWER_ARG_UPDATECOUNTER, "updateCounter", 1, 25},
	{EGRESS_ACL0_TCAM_ANSWER_ARG_COUNTER, "counter", 6, 26},
	{EGRESS_ACL0_TCAM_ANSWER_ARG_NATOPVALID, "natOpValid", 1, 32},
	{EGRESS_ACL0_TCAM_ANSWER_ARG_NATOPPTR, "natOpPtr", 10, 33},
	{EGRESS_ACL0_TCAM_ANSWER_ARG_NATOPPRIO, "natOpPrio", 1, 43},
	{EGRESS_ACL0_TCAM_ANSWER_ARG_TUNNELENTRY, "tunnelEntry", 1, 44},
	{EGRESS_ACL0_TCAM_ANSWER_ARG_TUNNELENTRYUCMC, "tunnelEntryUcMc", 1, 45},
	{EGRESS_ACL0_TCAM_ANSWER_ARG_TUNNELENTRYPTR, "tunnelEntryPtr", 4, 46},
	{EGRESS_ACL0_TCAM_ANSWER_ARG_TUNNELENTRYPRIO, "tunnelEntryPrio", 1, 50},
};

void switch_acl_egress_0_answer_usage(void)
{
        printf("Usage: npecmd switch acl egress0_answer <index> <--arg=value>\n");

	printf("maximum index: %d\n", EGRESS_CONFIGURABLE_ACL_0_TCAM_ANSWER_MAX - 1);
        printf("arg :=");
        for (int i = 0; i < ARRAY_SIZE(switch_acl_egress_0_answer); i++)
                printf(" %s", switch_acl_egress_0_answer[i].name);

        printf("\n");
}

int switch_acl_egress_0_answer_config(int argc, char **argv)
{
	u64 address;
	u32 value[EGRESS_CONFIGURABLE_ACL_0_TCAM_ANSWER_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= EGRESS_CONFIGURABLE_ACL_0_TCAM_ANSWER_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = EGRESS_CONFIGURABLE_ACL_0_TCAM_ANSWER + index * EGRESS_CONFIGURABLE_ACL_0_TCAM_ANSWER_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < EGRESS_CONFIGURABLE_ACL_0_TCAM_ANSWER_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_acl_egress_0_answer, ARRAY_SIZE(switch_acl_egress_0_answer), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < EGRESS_CONFIGURABLE_ACL_0_TCAM_ANSWER_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x %08x %08x]\n", __func__,
				index, address, value[3], value[2], value[1], value[0]);

        return 1;

ERR:
        switch_acl_egress_0_answer_usage();
        return 0;
}

struct switch_field_def switch_acl_egress_1_tcam[] = {
	{EGRESS_ACL1_TCAM_ARG_VALID, "valid", 1, 0},
	//{EGRESS_ACL1_TCAM_ARG_MASK, "mask", 540, 1},
	{EGRESS_ACL1_TCAM_ARG_MASK0, "mask0", 64, 1},
	{EGRESS_ACL1_TCAM_ARG_MASK1, "mask1", 64, 65},
	{EGRESS_ACL1_TCAM_ARG_MASK2, "mask2", 64, 129},
	{EGRESS_ACL1_TCAM_ARG_MASK3, "mask3", 64, 193},
	{EGRESS_ACL1_TCAM_ARG_MASK4, "mask4", 64, 257},
	{EGRESS_ACL1_TCAM_ARG_MASK5, "mask5", 64, 321},
	{EGRESS_ACL1_TCAM_ARG_MASK6, "mask6", 64, 385},
	{EGRESS_ACL1_TCAM_ARG_MASK7, "mask7", 64, 449},
	{EGRESS_ACL1_TCAM_ARG_MASK8, "mask8", 28, 513},
	//{EGRESS_ACL1_TCAM_ARG_COMPAREDATA, "compareData", 540, 541},
	{EGRESS_ACL1_TCAM_ARG_COMPAREDATA0, "compareData0", 64, 541},
	{EGRESS_ACL1_TCAM_ARG_COMPAREDATA1, "compareData1", 64, 605},
	{EGRESS_ACL1_TCAM_ARG_COMPAREDATA2, "compareData2", 64, 669},
	{EGRESS_ACL1_TCAM_ARG_COMPAREDATA3, "compareData3", 64, 733},
	{EGRESS_ACL1_TCAM_ARG_COMPAREDATA4, "compareData4", 64, 793},
	{EGRESS_ACL1_TCAM_ARG_COMPAREDATA5, "compareData5", 64, 861},
	{EGRESS_ACL1_TCAM_ARG_COMPAREDATA6, "compareData6", 64, 925},
	{EGRESS_ACL1_TCAM_ARG_COMPAREDATA7, "compareData7", 64, 989},
	{EGRESS_ACL1_TCAM_ARG_COMPAREDATA8, "compareData8", 28, 1053},
};

void switch_acl_egress_1_tcam_usage(void)
{
        printf("Usage: npecmd switch acl egress1_tcam <index> <--arg=value>\n");

	printf("maximum index: %d\n", EGRESS_CONFIGURABLE_ACL_1_TCAM_MAX - 1);
        printf("arg :=");
        for (int i = 0; i < ARRAY_SIZE(switch_acl_egress_1_tcam); i++)
                printf(" %s", switch_acl_egress_1_tcam[i].name);

        printf("\n");
}

int switch_acl_egress_1_tcam_config(int argc, char **argv)
{
	u64 address;
	u32 value[EGRESS_CONFIGURABLE_ACL_1_TCAM_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= EGRESS_CONFIGURABLE_ACL_1_TCAM_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = EGRESS_CONFIGURABLE_ACL_1_TCAM + index * EGRESS_CONFIGURABLE_ACL_1_TCAM_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < EGRESS_CONFIGURABLE_ACL_1_TCAM_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_acl_egress_1_tcam, ARRAY_SIZE(switch_acl_egress_1_tcam), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < EGRESS_CONFIGURABLE_ACL_1_TCAM_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x %08x %08x]\n", __func__, index, address, value[3], value[2], value[1], value[0]);

        return 1;

ERR:
        switch_acl_egress_1_tcam_usage();
        return 0;
}

struct switch_field_def switch_acl_egress_1_answer[] = {
	{EGRESS_ACL1_TCAM_ANSWER_ARG_SENDTOCPU, "sendToCpu", 1, 0},
	{EGRESS_ACL1_TCAM_ANSWER_ARG_FORCESENDTOCPUORIGPKT, "forceSendToCpuOrigPkt", 1, 1},
	{EGRESS_ACL1_TCAM_ANSWER_ARG_METADATAVALID, "metaDataValid", 1, 2},
	{EGRESS_ACL1_TCAM_ANSWER_ARG_METADATA, "metaData", 16, 3},
	{EGRESS_ACL1_TCAM_ANSWER_ARG_METADATAPRIO, "metaDataPrio", 1, 19},
	{EGRESS_ACL1_TCAM_ANSWER_ARG_DROPENABLE, "dropEnable", 1, 20},
	{EGRESS_ACL1_TCAM_ANSWER_ARG_SENDTOPORT, "sendToPort", 1, 21},
	{EGRESS_ACL1_TCAM_ANSWER_ARG_DESTPORT, "destPort", 3, 22},
	{EGRESS_ACL1_TCAM_ANSWER_ARG_UPDATECOUNTER, "updateCounter", 1, 25},
	{EGRESS_ACL1_TCAM_ANSWER_ARG_COUNTER, "counter", 6, 26},
	{EGRESS_ACL1_TCAM_ANSWER_ARG_TUNNELENTRY, "tunnelEntry", 1, 32},
	{EGRESS_ACL1_TCAM_ANSWER_ARG_TUNNELENTRYUCMC, "tunnelEntryUcMc", 1, 33},
	{EGRESS_ACL1_TCAM_ANSWER_ARG_TUNNELENTRYPTR, "tunnelEntryPtr", 4, 34},
	{EGRESS_ACL1_TCAM_ANSWER_ARG_TUNNELENTRYPRIO, "tunnelEntryPrio", 1, 38},
};

void switch_acl_egress_1_answer_usage(void)
{
        printf("Usage: npecmd switch acl egress1_answer <index> <--arg=value>\n");

	printf("maximum index: %d\n", EGRESS_CONFIGURABLE_ACL_1_TCAM_ANSWER_MAX - 1);
        printf("arg :=");
        for (int i = 0; i < ARRAY_SIZE(switch_acl_egress_1_answer); i++)
                printf(" %s", switch_acl_egress_1_answer[i].name);

        printf("\n");
}

int switch_acl_egress_1_answer_config(int argc, char **argv)
{
	u64 address;
	u32 value[EGRESS_CONFIGURABLE_ACL_1_TCAM_ANSWER_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= EGRESS_CONFIGURABLE_ACL_1_TCAM_ANSWER_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = EGRESS_CONFIGURABLE_ACL_1_TCAM_ANSWER + index * EGRESS_CONFIGURABLE_ACL_1_TCAM_ANSWER_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < EGRESS_CONFIGURABLE_ACL_1_TCAM_ANSWER_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_acl_egress_1_answer, ARRAY_SIZE(switch_acl_egress_1_answer), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < EGRESS_CONFIGURABLE_ACL_1_TCAM_ANSWER_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x]\n", __func__,
				index, address, value[1], value[0]);

        return 1;

ERR:
        switch_acl_egress_1_answer_usage();
        return 0;
}

struct cmd_module switch_acl_cmd[] = {
	{"hash_cal", switch_acl_hash_cal_config, switch_acl_hash_cal_usage},
	/* multi ACLs */
	{"selection", switch_acl_selection_config, switch_acl_selection_usage},
	{"rules_setup", switch_acl_rules_setup_config, switch_acl_rules_setup_usage},

	/*Ingress ACLN*/
	{"ingress_prelookup", switch_ingress_acl_pre_lookup_config, switch_ingress_acl_pre_lookup_usage},
	/* Ingress ACL0 */
	{"ingress0_large", switch_acl_ingress_0_large_table_config, switch_acl_ingress_0_large_table_usage},
	{"ingress0_small", switch_acl_ingress_0_small_table_config, switch_acl_ingress_0_small_table_usage},
	{"ingress0_mask", switch_acl_ingress_0_mask_config, switch_acl_ingress_0_mask_usage},
	{"ingress0_tcam", switch_acl_ingress_0_tcam_config, switch_acl_ingress_0_tcam_usage},
	{"ingress0_answer", switch_acl_ingress_0_answer_config, switch_acl_ingress_0_answer_usage},

	/* Ingress ACL1 */
	{"ingress1_large", switch_acl_ingress_1_large_table_config, switch_acl_ingress_1_large_table_usage},
	{"ingress1_small", switch_acl_ingress_1_small_table_config, switch_acl_ingress_1_small_table_usage},
	{"ingress1_mask", switch_acl_ingress_1_mask_config, switch_acl_ingress_1_mask_usage},
	{"ingress1_tcam", switch_acl_ingress_1_tcam_config, switch_acl_ingress_1_tcam_usage},
	{"ingress1_answer", switch_acl_ingress_1_answer_config, switch_acl_ingress_1_answer_usage},

	/* Ingress ACL2 */
	{"ingress2_tcam", switch_acl_ingress_2_tcam_config, switch_acl_ingress_2_tcam_usage},
	{"ingress2_answer", switch_acl_ingress_2_answer_config, switch_acl_ingress_2_answer_usage},

	/* Ingress ACL3 */
	{"ingress3_tcam", switch_acl_ingress_3_tcam_config, switch_acl_ingress_3_tcam_usage},
	{"ingress3_answer", switch_acl_ingress_3_answer_config, switch_acl_ingress_3_answer_usage},

	/*Egress ACLN*/
	{"rule_tcam", switch_egress_acl_rule_pointer_tcam_config, switch_egress_acl_rule_pointer_tcam_usage},
	/* Egress ACL0 */
	{"egress0_large", switch_acl_egress_0_large_table_config, switch_acl_egress_0_large_table_usage},
	{"egress0_small", switch_acl_egress_0_small_table_config, switch_acl_egress_0_small_table_usage},
	{"egress0_mask", switch_acl_egress_0_mask_config, switch_acl_egress_0_mask_usage},
	{"egress0_tcam", switch_acl_egress_0_tcam_config, switch_acl_egress_0_tcam_usage},
	{"egress0_answer", switch_acl_egress_0_answer_config, switch_acl_egress_0_answer_usage},

	/* Egress ACL1 */
	{"egress1_tcam", switch_acl_egress_1_tcam_config, switch_acl_egress_1_tcam_usage},
	{"egress1_answer", switch_acl_egress_1_answer_config, switch_acl_egress_1_answer_usage},
};

void switch_acl_usage(void)
{
	printf("Usage: npecmd switch acl SUB-FUNCTION {COMMAND}\n");
	printf("where  SUB-FUNCTION :=");
	for (int i = 0; i < ARRAY_SIZE(switch_acl_cmd); i++)
		printf(" %s", switch_acl_cmd[i].name);

	printf("\n");
}

int switch_acl_config(int argc, char **argv)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(switch_acl_cmd); i++) {
		if (!strcasecmp(argv[0], switch_acl_cmd[i].name)) {
			if (argc <= 1) {
				if (switch_acl_cmd[i].usage)
					switch_acl_cmd[i].usage();

				return 0;
			}

			if (switch_acl_cmd[i].func)
				return switch_acl_cmd[i].func(argc - 1, argv + 1);
		}
	}

	switch_acl_usage();
	return 0;
}
