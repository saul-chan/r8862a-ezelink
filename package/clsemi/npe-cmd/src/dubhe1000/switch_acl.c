// SPDX-License-Identifier: GPL-2.0
// Copyright(c) 2023 Clourneysemi Corporation.

#include "npe_switch.h"
#include "switch_conf.h"
#include "switch_acl.h"
#include <arpa/inet.h>

/* NOTE: this enum should sync with switch_acl_fileds_all */
enum {
	ACL_FIELD_NO_FIELD,
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
	ACL_FIELD_Outer_Ethernet_Type,
	ACL_FIELD_Inner_Ethernet_Type,
	ACL_FIELD_ICMPv6_Type,
	ACL_FIELD_ICMPv6_Code,
	ACL_FIELD_Router_Next_Hop,
	ACL_FIELD_VRF,
	// Add a new item at the end 
};

struct acl_field_info switch_acl_fileds_info_all[] = {
	{"No_Field", 0, SWITCH_FIELD_TYPE_SHORT_VALUE},
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
	{"Outer_Ethernet_Type", 1, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"Inner_Ethernet_Type", 1, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"ICMPv6_Type", 8, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"ICMPv6_Code", 8, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"Router_Next_Hop", 10, SWITCH_FIELD_TYPE_SHORT_VALUE},
	{"VRF", 2, SWITCH_FIELD_TYPE_SHORT_VALUE},
	// Add a new item at the end
};

/* hash calculation */
u8  switch_ingress_acl0_index[] = {
	ACL_FIELD_NO_FIELD,
	ACL_FIELD_Source_Port,
	ACL_FIELD_MAC_DA,
	ACL_FIELD_MAC_SA,
	ACL_FIELD_Outer_VID,
	ACL_FIELD_Has_VLANs,
	ACL_FIELD_Outer_Ethernet_Type,
	ACL_FIELD_Inner_Ethernet_Type,
	ACL_FIELD_Outer_PCP,
	ACL_FIELD_Outer_DEI,
	ACL_FIELD_Inner_VID,
	ACL_FIELD_Inner_PCP,
	ACL_FIELD_Inner_DEI,
	ACL_FIELD_Ethernet_Type,
	ACL_FIELD_L3_Type,
	ACL_FIELD_IPV4_SA,
	ACL_FIELD_IPV4_DA,
	ACL_FIELD_IPV6_SA,
	ACL_FIELD_IPV6_DA,
	ACL_FIELD_Outer_MPLS,
	ACL_FIELD_TOS,
	ACL_FIELD_TTL,
	ACL_FIELD_L4_Protocol,
	ACL_FIELD_L4_Type,
	ACL_FIELD_L4_Source_Port,
	ACL_FIELD_L4_Destination_Port,
	ACL_FIELD_MLD_Address,
	ACL_FIELD_ICMP_Type,
	ACL_FIELD_ICMP_Code,
	ACL_FIELD_ICMPv6_Type,
	ACL_FIELD_ICMPv6_Code,
	ACL_FIELD_IGMP_Type,
	ACL_FIELD_IGMP_Group_Address,
	ACL_FIELD_Rule_Pointer,
};

u8 switch_ingress_acl1_index[] = {
};

u8 switch_ingress_acl2_index[] = {
};

u8 switch_ingress_acl3_index[] = {
};

u8 switch_egress_acl_index[] = {
	ACL_FIELD_NO_FIELD,
	ACL_FIELD_Destination_Portmask,
	ACL_FIELD_Ethernet_Type,
	ACL_FIELD_GID,
	ACL_FIELD_ICMP_Code,
	ACL_FIELD_ICMP_Type,
	ACL_FIELD_ICMPv6_Code,
	ACL_FIELD_ICMPv6_Type,
	ACL_FIELD_IGMP_Group_Address,
	ACL_FIELD_IGMP_Type,
	ACL_FIELD_IPV4_DA,
	ACL_FIELD_IPV4_SA,
	ACL_FIELD_IPV6_DA,
	ACL_FIELD_IPV6_SA,
	ACL_FIELD_L3_Type,
	ACL_FIELD_L4_Destination_Port,
	ACL_FIELD_L4_Protocol,
	ACL_FIELD_L4_Source_Port,
	ACL_FIELD_L4_Type,
	ACL_FIELD_MAC_DA,
	ACL_FIELD_MAC_SA,
	ACL_FIELD_MLD_Address,
	ACL_FIELD_L2_Multicast_Pointer,
	ACL_FIELD_Router_Next_Hop,
	ACL_FIELD_Rule_Pointer,
	ACL_FIELD_Source_Port,
	ACL_FIELD_TOS,
	ACL_FIELD_TTL,
	ACL_FIELD_VID,
	ACL_FIELD_VRF,
};

u8 *switch_acl_field_index_all[] = {
	switch_ingress_acl0_index,
	switch_ingress_acl1_index,
	switch_ingress_acl2_index,
	switch_ingress_acl3_index,
	switch_egress_acl_index,
};

struct switch_register_info reg_rules_setup[] = {
	{INGRESS_CONFIGURABLE_ACL_0_RULES_SETUP, INGRESS_CONFIGURABLE_ACL_0_RULES_SETUP_MAX, INGRESS_CONFIGURABLE_ACL_0_RULES_SETUP_ADDR_PER_ENTRY},
	{INGRESS_CONFIGURABLE_ACL_1_RULES_SETUP, INGRESS_CONFIGURABLE_ACL_1_RULES_SETUP_MAX, INGRESS_CONFIGURABLE_ACL_1_RULES_SETUP_ADDR_PER_ENTRY},
	{INGRESS_CONFIGURABLE_ACL_2_RULES_SETUP, INGRESS_CONFIGURABLE_ACL_2_RULES_SETUP_MAX, INGRESS_CONFIGURABLE_ACL_2_RULES_SETUP_ADDR_PER_ENTRY},
	{INGRESS_CONFIGURABLE_ACL_3_RULES_SETUP, INGRESS_CONFIGURABLE_ACL_3_RULES_SETUP_MAX, INGRESS_CONFIGURABLE_ACL_3_RULES_SETUP_ADDR_PER_ENTRY},
	{EGRESS_CONFIGURABLE_ACL_RULES_SETUP, EGRESS_CONFIGURABLE_ACL_RULES_SETUP_MAX, EGRESS_CONFIGURABLE_ACL_RULES_SETUP_ADDR_PER_ENTRY},
};

#define SWITCH_INGRESS_ACL_ENGINES		4
#define SWITCH_EGRESS_ACL_ENGINES		1
#define SWITCH_MAX_RULES_POINTER		16
#define SWITCH_RULES_POINTER_PER_ENTRY		6

struct acl_engine_setting acl_settings[] = {
	{154, 8, 2, 6, 2, ARRAY_SIZE(switch_ingress_acl0_index), SWITCH_RULES_POINTER_PER_ENTRY},
	{214, 7, 1, 2, 1, ARRAY_SIZE(switch_ingress_acl1_index), SWITCH_RULES_POINTER_PER_ENTRY},
	{214, 7, 1, 2, 1, ARRAY_SIZE(switch_ingress_acl2_index), SWITCH_RULES_POINTER_PER_ENTRY},
	{214, 7, 1, 1, 1, ARRAY_SIZE(switch_ingress_acl3_index), SWITCH_RULES_POINTER_PER_ENTRY},
	{214, 8, 2, 6, 2, ARRAY_SIZE(switch_egress_acl_index), SWITCH_RULES_POINTER_PER_ENTRY},
};

void switch_acl_hash_cal_usage(void)
{
	printf("Usage: npecmd switch acl hash_cal <is_egress> <AclN> <rulePtr> <--arg=value>\n");

	printf("is_egress := 0 1\n");

	printf("maximum Ingress AclN: %d\n", SWITCH_INGRESS_ACL_ENGINES - 1);
	printf("maximum Egress AclN: %d\n", SWITCH_EGRESS_ACL_ENGINES - 1);

	printf("maximum rulePtr: %d\n", SWITCH_MAX_RULES_POINTER - 1);

	printf("arg := Field Name in ACL Field\n");
}

struct switch_field_def *switch_setup_acl_filed_def(u8 AclN, u8 *fields, int size)
{
	struct switch_field_def *switch_acl_hash = NULL;
	struct acl_field_info *info = NULL;
	int i, j;

	//check AclN
	if (AclN >= ARRAY_SIZE(acl_settings)) {
		printf("[%s] Failed! Invalid AclN\n", __func__);
		goto ERR;
	}

	switch_acl_hash	= malloc(sizeof(struct switch_field_def) * (size + 1));
	if (!switch_acl_hash) {
		printf("[%s] Malloc Failed!\n", __func__);
		goto ERR;
	}

	switch_acl_hash[0].index = 0;
	switch_acl_hash[0].name = "Valid";
	switch_acl_hash[0].width = acl_settings[AclN].bit_max_use;
	switch_acl_hash[0].start = 0;
	switch_acl_hash[0].flag = SWITCH_FIELD_TYPE_SHORT_VALUE;

	for (i = 0, j = 1; i < size; i++) {
			info = &switch_acl_fileds_info_all[switch_acl_field_index_all[AclN][fields[i]]]; 

			switch_acl_hash[j].index = j;
			switch_acl_hash[j].name = info->field_name;
			switch_acl_hash[j].width = info->size;
			switch_acl_hash[j].flag = info->flag;
			switch_acl_hash[j].start = switch_acl_hash[j - 1].start + switch_acl_hash[j - 1].width;
			j++;
	}

	if (j == 1) {
		printf("[%s] Failed! Invalid Rule Setup\n", __func__);
		goto ERR;
	}

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
	u64 address;
	/* only ACL1 need value[2], other value[1] */
	u32 value[SWITCH_RULES_POINTER_PER_ENTRY] = {0}, AclN ,index, is_egress;
	u32 hashkey[SWITCH_HASHKEY_LENGTH_MAX] = {0}, hashtmp[SWITCH_HASHKEY_LENGTH_MAX] = {0};
	struct switch_field_def *switch_acl_hash = NULL;
	int i, j, size;
	int high_valid = 0;
	u16 hashval;
	u8 len, tmp[SWITCH_RULES_POINTER_PER_ENTRY] = {0};

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
	if ((sscanf(argv[2], "%i", &index) != 1) || (index >= SWITCH_MAX_RULES_POINTER)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = reg_rules_setup[AclN].address +
			index * reg_rules_setup[AclN].addr_per_entry * SWITCH_RULES_POINTER_PER_ENTRY;

	for (i = 0, size = 0; i < SWITCH_RULES_POINTER_PER_ENTRY; i++) {
		switch_read(address + i, &value[i]);
		if (!(value[i] & 0x3F)) {
			printf("[%s] field num %d is No_Field\n", __func__, i);
			break;
		}

		tmp[size++]=(value[i] & 0x3F);
	}

	if (!size) {
		printf("[%s] Failed! Empty Rule Setup\n", __func__);
		goto ERR;
	}

	switch_acl_hash = switch_setup_acl_filed_def(AclN, tmp, size);
	if (!switch_acl_hash) {
		printf("[%s] Failed! Invalid acl_filed_def!\n", __func__);
		goto ERR;
	}

	printf("[%s] arg list:", __func__);
	for (i = 0; i < size + 1; i++) {
		printf(" %s", switch_acl_hash[i].name);
	}
	printf("\n");

	//update
	if (!switch_field_setup(argc - 2, argv + 2, switch_acl_hash, size + 1, hashkey)) {
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
	{INGRESS_CONFIGURABLE_ACL_2_SELECTION, INGRESS_CONFIGURABLE_ACL_2_SELECTION_MAX, INGRESS_CONFIGURABLE_ACL_2_SELECTION_ADDR_PER_ENTRY},
	{INGRESS_CONFIGURABLE_ACL_3_SELECTION, INGRESS_CONFIGURABLE_ACL_3_SELECTION_MAX, INGRESS_CONFIGURABLE_ACL_3_SELECTION_ADDR_PER_ENTRY},
	{EGRESS_CONFIGURABLE_ACL_SELECTION, EGRESS_CONFIGURABLE_ACL_SELECTION_MAX, EGRESS_CONFIGURABLE_ACL_SELECTION_ADDR_PER_ENTRY},
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

struct switch_field_def switch_acl_ingress_rules_setup[] = {
	{INGRESS_ACL_RULES_SETUP_ARG_FIELDSELECT, "fieldSelect", 6, 0},
	{INGRESS_ACL_RULES_SETUP_ARG_BITSSELECTED, "bitsSelected", 8, 6},
	{INGRESS_ACL_RULES_SETUP_ARG_LSHIFT, "lShift", 8, 14},
};

struct switch_field_def switch_acl_egress_rules_setup[] = {
	{EGRESS_ACL_RULES_SETUP_ARG_FIELDSELECT, "fieldSelect", 5, 0},
	{EGRESS_ACL_RULES_SETUP_ARG_BITSSELECTED, "bitsSelected", 8, 5},
	{EGRESS_ACL_RULES_SETUP_ARG_LSHIFT, "lShift", 8, 13},
};

void switch_acl_rules_setup_usage(void)
{
	printf("Usage: npecmd switch acl rules_setup <is_egress> <AclN> <index> <--arg=value>\n");

	printf("is_egress := 0 1\n");

	printf("maximum index: %d\n", reg_rules_setup[0].entries - 1);

        printf("arg :=");
        for (int i = 0; i < ARRAY_SIZE(switch_acl_ingress_rules_setup); i++)
                printf(" %s", switch_acl_ingress_rules_setup[i].name);

        printf("\n");

}

int switch_acl_rules_setup_config(int argc, char **argv)
{
	u64 address, tmp;
	/* only ACL1 need value[2], other value[1] */
	u32 value[INGRESS_CONFIGURABLE_ACL_1_RULES_SETUP_ADDR_PER_ENTRY] = {0}, AclN ,index, is_egress;
	struct switch_field_def *switch_acl_hash = NULL;
	int i, size, field_len;
	struct switch_field_def *rules_setup_field;

	if (argc <= 3) {
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

        if (is_egress) {
                AclN += SWITCH_INGRESS_ACL_ENGINES;
		rules_setup_field = switch_acl_egress_rules_setup;
		field_len = ARRAY_SIZE(switch_acl_egress_rules_setup);
	} else {
		rules_setup_field = switch_acl_ingress_rules_setup;
		field_len = ARRAY_SIZE(switch_acl_ingress_rules_setup);
	}

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

	//get old value
	for (i = 0; i < reg_rules_setup[AclN].addr_per_entry; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc - 2, argv + 2, rules_setup_field, field_len, value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//check fieldSelect
	if ((value[0] & 0x3F) >= ARRAY_SIZE(switch_acl_fileds_info_all)) {
		printf("[%s] Failed! Invalid fieldSelect\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < reg_rules_setup[AclN].addr_per_entry; i++)
		switch_write(address + i, value[i]);

	printf("[%s] %s-Acl%d index %d addr 0x%lx value 0x%08x\n", __func__,
			is_egress ? "Egress" : "Ingress", is_egress ? (AclN - SWITCH_INGRESS_ACL_ENGINES): AclN ,
			index, address, value[0]);

#if 0
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

struct switch_field_def switch_acl_ingress_0_large_table[] = {
	{INGRESS_ACL0_LARGE_ARG_VALID, "valid", 1, 0},
	//{INGRESS_ACL0_LARGE_ARG_COMPAREDATA, "compareData", 154, 1},
	{INGRESS_ACL0_LARGE_ARG_COMPAREDATA0, "compareData0", 64, 1},
	{INGRESS_ACL0_LARGE_ARG_COMPAREDATA1, "compareData1", 64, 65},
	{INGRESS_ACL0_LARGE_ARG_COMPAREDATA2, "compareData2", 26, 129},
	{INGRESS_ACL0_LARGE_ARG_DROPENABLE, "dropEnable", 1, 155},
	{INGRESS_ACL0_LARGE_ARG_SENDTOCPU, "sendToCpu", 1, 156},
	{INGRESS_ACL0_LARGE_ARG_SENDTOPORT, "sendToPort", 1, 157},
	{INGRESS_ACL0_LARGE_ARG_DESTPORT, "destPort", 3, 158},
	{INGRESS_ACL0_LARGE_ARG_FORCEVIDVALID, "forceVidValid", 1, 161},
	{INGRESS_ACL0_LARGE_ARG_FORCEVID, "forceVid", 12, 162},
	{INGRESS_ACL0_LARGE_ARG_FORCEQUEUE, "forceQueue", 1, 174},
	{INGRESS_ACL0_LARGE_ARG_EQUEUE, "eQueue", 3, 175},
	{INGRESS_ACL0_LARGE_ARG_FORCEVIDPRIO, "forceVidPrio", 1, 178},
	{INGRESS_ACL0_LARGE_ARG_FORCEQUEUEPRIO, "forceQueuePrio", 1, 179},
	{INGRESS_ACL0_LARGE_ARG_FORCECOLOR, "forceColor", 1, 180},
	{INGRESS_ACL0_LARGE_ARG_COLOR, "color", 2, 181},
	{INGRESS_ACL0_LARGE_ARG_FORCECOLORPRIO, "forceColorPrio", 1, 183},
	{INGRESS_ACL0_LARGE_ARG_MMPVALID, "mmpValid", 1, 184},
	{INGRESS_ACL0_LARGE_ARG_MMPPTR, "mmpPtr", 5, 185},
	{INGRESS_ACL0_LARGE_ARG_MMPORDER, "mmpOrder", 2, 190},
	{INGRESS_ACL0_LARGE_ARG_UPDATECOUNTER, "updateCounter", 1, 192},
	{INGRESS_ACL0_LARGE_ARG_COUNTER, "counter", 6, 193},
	{INGRESS_ACL0_LARGE_ARG_NOLEARNING, "noLearning", 1, 199},
	{INGRESS_ACL0_LARGE_ARG_UPDATECFIDEI, "updateCfiDei", 1, 200},
	{INGRESS_ACL0_LARGE_ARG_NEWCFIDEIVALUE, "newCfiDeiValue", 1, 201},
	{INGRESS_ACL0_LARGE_ARG_UPDATEPCP, "updatePcp", 1, 202},
	{INGRESS_ACL0_LARGE_ARG_NEWPCPVALUE, "newPcpValue", 3, 203},
	{INGRESS_ACL0_LARGE_ARG_UPDATEVID, "updateVid", 1, 206},
	{INGRESS_ACL0_LARGE_ARG_NEWVIDVALUE, "newVidValue", 12, 207},
	{INGRESS_ACL0_LARGE_ARG_UPDATEETYPE, "updateEType", 1, 219},
	{INGRESS_ACL0_LARGE_ARG_NEWETHTYPE, "newEthType", 2, 220},
	{INGRESS_ACL0_LARGE_ARG_CFIDEIPRIO, "cfiDeiPrio", 1, 222},
	{INGRESS_ACL0_LARGE_ARG_PCPPRIO, "pcpPrio", 1, 223},
	{INGRESS_ACL0_LARGE_ARG_VIDPRIO, "vidPrio", 1, 224},
	{INGRESS_ACL0_LARGE_ARG_ETHPRIO, "ethPrio", 1, 225},
	{INGRESS_ACL0_LARGE_ARG_INPUTMIRROR, "inputMirror", 1, 226},
	{INGRESS_ACL0_LARGE_ARG_DESTINPUTMIRROR, "destInputMirror", 3, 227},
	{INGRESS_ACL0_LARGE_ARG_IMPRIO, "imPrio", 1, 230},
	{INGRESS_ACL0_LARGE_ARG_PTP, "ptp", 1, 231},
	{INGRESS_ACL0_LARGE_ARG_NATOPVALID, "natOpValid", 1, 232},
	{INGRESS_ACL0_LARGE_ARG_NATOPPTR, "natOpPtr", 10, 233},
	{INGRESS_ACL0_LARGE_ARG_NATOPPRIO, "natOpPrio", 1, 243},
	{INGRESS_ACL0_LARGE_ARG_TUNNELENTRY, "tunnelEntry", 1, 244},
	{INGRESS_ACL0_LARGE_ARG_TUNNELENTRYUCMC, "tunnelEntryUcMc", 1, 245},
	{INGRESS_ACL0_LARGE_ARG_TUNNELENTRYPTR, "tunnelEntryPtr", 6, 246},
	{INGRESS_ACL0_LARGE_ARG_TUNNELEXIT, "tunnelExit", 1, 252},
	{INGRESS_ACL0_LARGE_ARG_TUNNELEXITPTR, "tunnelExitPtr", 7, 253},
	{INGRESS_ACL0_LARGE_ARG_TUNNELENTRYPRIO, "tunnelEntryPrio", 1, 260},
	{INGRESS_ACL0_LARGE_ARG_TUNNELEXITPRIO, "tunnelExitPrio", 1, 261},
	{INGRESS_ACL0_LARGE_ARG_ENABLEUPDATEIP, "enableUpdateIp", 1, 262},
	{INGRESS_ACL0_LARGE_ARG_UPDATESAORDA, "updateSaOrDa", 1, 263},
	{INGRESS_ACL0_LARGE_ARG_NEWIPVALUE, "newIpValue", 32, 264, SWITCH_FIELD_TYPE_IPV4},
	{INGRESS_ACL0_LARGE_ARG_ENABLEUPDATEL4, "enableUpdateL4", 1, 296},
	{INGRESS_ACL0_LARGE_ARG_UPDATEL4SPORDP, "updateL4SpOrDp", 1, 297},
	{INGRESS_ACL0_LARGE_ARG_NEWL4VALUE, "newL4Value", 16, 298},
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
	//{INGRESS_ACL0_SMALL_ARG_COMPAREDATA, "compareData", 154, 1},
	{INGRESS_ACL0_SMALL_ARG_COMPAREDATA0, "compareData0", 64, 1},
	{INGRESS_ACL0_SMALL_ARG_COMPAREDATA1, "compareData1", 64, 65},
	{INGRESS_ACL0_SMALL_ARG_COMPAREDATA2, "compareData2", 26, 129},
	{INGRESS_ACL0_SMALL_ARG_DROPENABLE, "dropEnable", 1, 155},
	{INGRESS_ACL0_SMALL_ARG_SENDTOCPU, "sendToCpu", 1, 156},
	{INGRESS_ACL0_SMALL_ARG_SENDTOPORT, "sendToPort", 1, 157},
	{INGRESS_ACL0_SMALL_ARG_DESTPORT, "destPort", 3, 158},
	{INGRESS_ACL0_SMALL_ARG_FORCEVIDVALID, "forceVidValid", 1, 161},
	{INGRESS_ACL0_SMALL_ARG_FORCEVID, "forceVid", 12, 162},
	{INGRESS_ACL0_SMALL_ARG_FORCEQUEUE, "forceQueue", 1, 174},
	{INGRESS_ACL0_SMALL_ARG_EQUEUE, "eQueue", 3, 175},
	{INGRESS_ACL0_SMALL_ARG_FORCEVIDPRIO, "forceVidPrio", 1, 178},
	{INGRESS_ACL0_SMALL_ARG_FORCEQUEUEPRIO, "forceQueuePrio", 1, 179},
	{INGRESS_ACL0_SMALL_ARG_FORCECOLOR, "forceColor", 1, 180},
	{INGRESS_ACL0_SMALL_ARG_COLOR, "color", 2, 181},
	{INGRESS_ACL0_SMALL_ARG_FORCECOLORPRIO, "forceColorPrio", 1, 183},
	{INGRESS_ACL0_SMALL_ARG_MMPVALID, "mmpValid", 1, 184},
	{INGRESS_ACL0_SMALL_ARG_MMPPTR, "mmpPtr", 5, 185},
	{INGRESS_ACL0_SMALL_ARG_MMPORDER, "mmpOrder", 2, 190},
	{INGRESS_ACL0_SMALL_ARG_UPDATECOUNTER, "updateCounter", 1, 192},
	{INGRESS_ACL0_SMALL_ARG_COUNTER, "counter", 6, 193},
	{INGRESS_ACL0_SMALL_ARG_NOLEARNING, "noLearning", 1, 199},
	{INGRESS_ACL0_SMALL_ARG_UPDATECFIDEI, "updateCfiDei", 1, 200},
	{INGRESS_ACL0_SMALL_ARG_NEWCFIDEIVALUE, "newCfiDeiValue", 1, 201},
	{INGRESS_ACL0_SMALL_ARG_UPDATEPCP, "updatePcp", 1, 202},
	{INGRESS_ACL0_SMALL_ARG_NEWPCPVALUE, "newPcpValue", 3, 203},
	{INGRESS_ACL0_SMALL_ARG_UPDATEVID, "updateVid", 1, 206},
	{INGRESS_ACL0_SMALL_ARG_NEWVIDVALUE, "newVidValue", 12, 207},
	{INGRESS_ACL0_SMALL_ARG_UPDATEETYPE, "updateEType", 1, 219},
	{INGRESS_ACL0_SMALL_ARG_NEWETHTYPE, "newEthType", 2, 220},
	{INGRESS_ACL0_SMALL_ARG_CFIDEIPRIO, "cfiDeiPrio", 1, 222},
	{INGRESS_ACL0_SMALL_ARG_PCPPRIO, "pcpPrio", 1, 223},
	{INGRESS_ACL0_SMALL_ARG_VIDPRIO, "vidPrio", 1, 224},
	{INGRESS_ACL0_SMALL_ARG_ETHPRIO, "ethPrio", 1, 225},
	{INGRESS_ACL0_SMALL_ARG_INPUTMIRROR, "inputMirror", 1, 226},
	{INGRESS_ACL0_SMALL_ARG_DESTINPUTMIRROR, "destInputMirror", 3, 227},
	{INGRESS_ACL0_SMALL_ARG_IMPRIO, "imPrio", 1, 230},
	{INGRESS_ACL0_SMALL_ARG_PTP, "ptp", 1, 231},
	{INGRESS_ACL0_SMALL_ARG_NATOPVALID, "natOpValid", 1, 232},
	{INGRESS_ACL0_SMALL_ARG_NATOPPTR, "natOpPtr", 10, 233},
	{INGRESS_ACL0_SMALL_ARG_NATOPPRIO, "natOpPrio", 1, 243},
	{INGRESS_ACL0_SMALL_ARG_TUNNELENTRY, "tunnelEntry", 1, 244},
	{INGRESS_ACL0_SMALL_ARG_TUNNELENTRYUCMC, "tunnelEntryUcMc", 1, 245},
	{INGRESS_ACL0_SMALL_ARG_TUNNELENTRYPTR, "tunnelEntryPtr", 6, 246},
	{INGRESS_ACL0_SMALL_ARG_TUNNELEXIT, "tunnelExit", 1, 252},
	{INGRESS_ACL0_SMALL_ARG_TUNNELEXITPTR, "tunnelExitPtr", 7, 253},
	{INGRESS_ACL0_SMALL_ARG_TUNNELENTRYPRIO, "tunnelEntryPrio", 1, 260},
	{INGRESS_ACL0_SMALL_ARG_TUNNELEXITPRIO, "tunnelExitPrio", 1, 261},
	{INGRESS_ACL0_SMALL_ARG_ENABLEUPDATEIP, "enableUpdateIp", 1, 262},
	{INGRESS_ACL0_SMALL_ARG_UPDATESAORDA, "updateSaOrDa", 1, 263},
	{INGRESS_ACL0_SMALL_ARG_NEWIPVALUE, "newIpValue", 32, 264, SWITCH_FIELD_TYPE_IPV4},
	{INGRESS_ACL0_SMALL_ARG_ENABLEUPDATEL4, "enableUpdateL4", 1, 296},
	{INGRESS_ACL0_SMALL_ARG_UPDATEL4SPORDP, "updateL4SpOrDp", 1, 297},
	{INGRESS_ACL0_SMALL_ARG_NEWL4VALUE, "newL4Value", 16, 298},
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
	//{INGRESS_ACL0_MASK_ARG_MASK_SMALL, "mask_small", 154, 0},
	{INGRESS_ACL0_MASK_ARG_MASK_SMALL0, "mask_small0", 64, 0},
	{INGRESS_ACL0_MASK_ARG_MASK_SMALL1, "mask_small1", 64, 64},
	{INGRESS_ACL0_MASK_ARG_MASK_SMALL2, "mask_small2", 26, 128},
	//{INGRESS_ACL0_MASK_ARG_MASK_LARGE, "mask_large", 154, 154},
	{INGRESS_ACL0_MASK_ARG_MASK_LARGE0, "mask_large0", 64, 154},
	{INGRESS_ACL0_MASK_ARG_MASK_LARGE1, "mask_large1", 64, 218},
	{INGRESS_ACL0_MASK_ARG_MASK_LARGE2, "mask_large2", 26, 282},
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
	//{INGRESS_ACL0_TCAM_ARG_MASK, "mask", 154, 1},
	{INGRESS_ACL0_TCAM_ARG_MASK0, "mask0", 64, 1},
	{INGRESS_ACL0_TCAM_ARG_MASK1, "mask1", 64, 65},
	{INGRESS_ACL0_TCAM_ARG_MASK2, "mask2", 26, 129},
	//{INGRESS_ACL0_TCAM_ARG_COMPAREDATA, "compareData", 154, 155},
	{INGRESS_ACL0_TCAM_ARG_COMPAREDATA0, "compareData0", 64, 155},
	{INGRESS_ACL0_TCAM_ARG_COMPAREDATA1, "compareData1", 64, 219},
	{INGRESS_ACL0_TCAM_ARG_COMPAREDATA2, "compareData2", 26, 283},
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
	{INGRESS_ACL0_ANSWER_ARG_DROPENABLE, "dropEnable", 1, 0},
	{INGRESS_ACL0_ANSWER_ARG_SENDTOCPU, "sendToCpu", 1, 1},
	{INGRESS_ACL0_ANSWER_ARG_SENDTOPORT, "sendToPort", 1, 2},
	{INGRESS_ACL0_ANSWER_ARG_DESTPORT, "destPort", 3, 3},
	{INGRESS_ACL0_ANSWER_ARG_FORCEVIDVALID, "forceVidValid", 1, 6},
	{INGRESS_ACL0_ANSWER_ARG_FORCEVID, "forceVid", 12, 7},
	{INGRESS_ACL0_ANSWER_ARG_FORCEQUEUE, "forceQueue", 1, 19},
	{INGRESS_ACL0_ANSWER_ARG_EQUEUE, "eQueue", 3, 20},
	{INGRESS_ACL0_ANSWER_ARG_FORCEVIDPRIO, "forceVidPrio", 1, 23},
	{INGRESS_ACL0_ANSWER_ARG_FORCEQUEUEPRIO, "forceQueuePrio", 1, 24},
	{INGRESS_ACL0_ANSWER_ARG_FORCECOLOR, "forceColor", 1, 25},
	{INGRESS_ACL0_ANSWER_ARG_COLOR, "color", 2, 26},
	{INGRESS_ACL0_ANSWER_ARG_FORCECOLORPRIO, "forceColorPrio", 1, 28},
	{INGRESS_ACL0_ANSWER_ARG_MMPVALID, "mmpValid", 1, 29},
	{INGRESS_ACL0_ANSWER_ARG_MMPPTR, "mmpPtr", 5, 30},
	{INGRESS_ACL0_ANSWER_ARG_MMPORDER, "mmpOrder", 2, 35},
	{INGRESS_ACL0_ANSWER_ARG_UPDATECOUNTER, "updateCounter", 1, 37},
	{INGRESS_ACL0_ANSWER_ARG_COUNTER, "counter", 6, 38},
	{INGRESS_ACL0_ANSWER_ARG_NOLEARNING, "noLearning", 1, 44},
	{INGRESS_ACL0_ANSWER_ARG_UPDATECFIDEI, "updateCfiDei", 1, 45},
	{INGRESS_ACL0_ANSWER_ARG_NEWCFIDEIVALUE, "newCfiDeiValue", 1, 46},
	{INGRESS_ACL0_ANSWER_ARG_UPDATEPCP, "updatePcp", 1, 47},
	{INGRESS_ACL0_ANSWER_ARG_NEWPCPVALUE, "newPcpValue", 3, 48},
	{INGRESS_ACL0_ANSWER_ARG_UPDATEVID, "updateVid", 1, 51},
	{INGRESS_ACL0_ANSWER_ARG_NEWVIDVALUE, "newVidValue", 12, 52},
	{INGRESS_ACL0_ANSWER_ARG_UPDATEETYPE, "updateEType", 1, 64},
	{INGRESS_ACL0_ANSWER_ARG_NEWETHTYPE, "newEthType", 2, 65},
	{INGRESS_ACL0_ANSWER_ARG_CFIDEIPRIO, "cfiDeiPrio", 1, 67},
	{INGRESS_ACL0_ANSWER_ARG_PCPPRIO, "pcpPrio", 1, 68},
	{INGRESS_ACL0_ANSWER_ARG_VIDPRIO, "vidPrio", 1, 69},
	{INGRESS_ACL0_ANSWER_ARG_ETHPRIO, "ethPrio", 1, 70},
	{INGRESS_ACL0_ANSWER_ARG_INPUTMIRROR, "inputMirror", 1, 71},
	{INGRESS_ACL0_ANSWER_ARG_DESTINPUTMIRROR, "destInputMirror", 3, 72},
	{INGRESS_ACL0_ANSWER_ARG_IMPRIO, "imPrio", 1, 75},
	{INGRESS_ACL0_ANSWER_ARG_PTP, "ptp", 1, 76},
	{INGRESS_ACL0_ANSWER_ARG_NATOPVALID, "natOpValid", 1, 77},
	{INGRESS_ACL0_ANSWER_ARG_NATOPPTR, "natOpPtr", 10, 78},
	{INGRESS_ACL0_ANSWER_ARG_NATOPPRIO, "natOpPrio", 1, 88},
	{INGRESS_ACL0_ANSWER_ARG_TUNNELENTRY, "tunnelEntry", 1, 89},
	{INGRESS_ACL0_ANSWER_ARG_TUNNELENTRYUCMC, "tunnelEntryUcMc", 1, 90},
	{INGRESS_ACL0_ANSWER_ARG_TUNNELENTRYPTR, "tunnelEntryPtr", 6, 91},
	{INGRESS_ACL0_ANSWER_ARG_TUNNELEXIT, "tunnelExit", 1, 97},
	{INGRESS_ACL0_ANSWER_ARG_TUNNELEXITPTR, "tunnelExitPtr", 7, 98},
	{INGRESS_ACL0_ANSWER_ARG_TUNNELENTRYPRIO, "tunnelEntryPrio", 1, 105},
	{INGRESS_ACL0_ANSWER_ARG_TUNNELEXITPRIO, "tunnelExitPrio", 1, 106},
	{INGRESS_ACL0_ANSWER_ARG_ENABLEUPDATEIP, "enableUpdateIp", 1, 107},
	{INGRESS_ACL0_ANSWER_ARG_UPDATESAORDA, "updateSaOrDa", 1, 108},
	{INGRESS_ACL0_ANSWER_ARG_NEWIPVALUE, "newIpValue", 32, 109, SWITCH_FIELD_TYPE_IPV4},
	{INGRESS_ACL0_ANSWER_ARG_ENABLEUPDATEL4, "enableUpdateL4", 1, 141},
	{INGRESS_ACL0_ANSWER_ARG_UPDATEL4SPORDP, "updateL4SpOrDp", 1, 142},
	{INGRESS_ACL0_ANSWER_ARG_NEWL4VALUE, "newL4Value", 16, 143},
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
	{EGRESS_ACL_RULE_POINTER_TCAM_ARG_SRCPORT_MASK, "srcPort_mask", 3, 49},
	{EGRESS_ACL_RULE_POINTER_TCAM_ARG_SRCPORT, "srcPort", 3, 52},
};

void switch_egress_acl_rule_pointer_tcam_usage(void)
{
	printf("Usage: npecmd switch acl rule_tcam <index> <rulePtr> <--arg=value>\n");

	printf("maximum index: %d\n", EGRESS_ACL_RULE_POINTER_TCAM_MAX - 1);
	printf("maximum rulePtr: %d\n", SWITCH_MAX_RULES_POINTER - 1);

	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_egress_acl_rule_pointer_tcam); i++)
		printf(" %s", switch_egress_acl_rule_pointer_tcam[i].name);

	printf("\n");
}

int switch_egress_acl_rule_pointer_tcam_config(int argc, char **argv)
{
	u64 address, answer_address;
	u32 value[EGRESS_ACL_RULE_POINTER_TCAM_ADDR_PER_ENTRY] = {0}, index, rulePtr;
	u32 answer[EGRESS_ACL_RULE_POINTER_TCAM_ANSWER_ADDR_PER_ENTRY];
	int i;

	if (argc <= 2) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= EGRESS_ACL_RULE_POINTER_TCAM_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	//check rulePtr
	if ((sscanf(argv[1], "%i", &rulePtr) != 1) || (rulePtr >= SWITCH_MAX_RULES_POINTER)) {
		printf("[%s] Failed! Invalid rulePtr0\n", __func__);
		goto ERR;
	}

	answer[0] = rulePtr;

	address = EGRESS_ACL_RULE_POINTER_TCAM + index * EGRESS_ACL_RULE_POINTER_TCAM_ADDR_PER_ENTRY;
	answer_address = EGRESS_ACL_RULE_POINTER_TCAM_ANSWER + index * EGRESS_ACL_RULE_POINTER_TCAM_ANSWER_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < EGRESS_ACL_RULE_POINTER_TCAM_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc - 1, argv + 1, switch_egress_acl_rule_pointer_tcam, ARRAY_SIZE(switch_egress_acl_rule_pointer_tcam), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	switch_write(answer_address, answer[0]);

	for (i = 0; i < EGRESS_ACL_RULE_POINTER_TCAM_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);


	printf("[%s] TCAM-index %d addr 0x%lx value 0x[%08x %08x]\n", __func__, index, address, value[1], value[0]);
	printf("[%s] Answer-index %d addr 0x%lx answer 0x%08x\n", __func__, index, answer_address, answer[0]);

	return 1;

ERR:
	switch_egress_acl_rule_pointer_tcam_usage();
	return 0;
}

struct switch_field_def switch_acl_egress_large_table[] = {
	{EGRESS_ACL_LARGE_ARG_VALID, "valid", 1, 0},
	//{EGRESS_ACL_LARGE_ARG_COMPAREDATA, "compareData", 214, 1},
	{EGRESS_ACL_LARGE_ARG_COMPAREDATA0, "compareData0", 64, 1},
	{EGRESS_ACL_LARGE_ARG_COMPAREDATA1, "compareData1", 64, 65},
	{EGRESS_ACL_LARGE_ARG_COMPAREDATA2, "compareData2", 64, 129},
	{EGRESS_ACL_LARGE_ARG_COMPAREDATA3, "compareData3", 22, 193},
	{EGRESS_ACL_LARGE_ARG_DROPENABLE, "dropEnable", 1, 215},
	{EGRESS_ACL_LARGE_ARG_SENDTOCPU, "sendToCpu", 1, 216},
	{EGRESS_ACL_LARGE_ARG_SENDTOPORT, "sendToPort", 1, 217},
	{EGRESS_ACL_LARGE_ARG_DESTPORT, "destPort", 3, 218},
	{EGRESS_ACL_LARGE_ARG_NATOPVALID, "natOpValid", 1, 221},
	{EGRESS_ACL_LARGE_ARG_NATOPPTR, "natOpPtr", 10, 222},
	{EGRESS_ACL_LARGE_ARG_UPDATECOUNTER, "updateCounter", 1, 232},
	{EGRESS_ACL_LARGE_ARG_COUNTER, "counter", 6, 233},
	{EGRESS_ACL_LARGE_ARG_TUNNELENTRY, "tunnelEntry", 1, 239},
	{EGRESS_ACL_LARGE_ARG_TUNNELENTRYUCMC, "tunnelEntryUcMc", 1, 240},
	{EGRESS_ACL_LARGE_ARG_TUNNELENTRYPTR, "tunnelEntryPtr", 6, 241},
	{EGRESS_ACL_LARGE_ARG_TUNNELEXIT, "tunnelExit", 1, 247},
	{EGRESS_ACL_LARGE_ARG_TUNNELEXITPTR, "tunnelExitPtr", 7, 248},
};

void switch_acl_egress_large_table_usage(void)
{
	printf("Usage: npecmd switch acl egress_large <index> <--arg=value>\n");

	printf("maximum index: %d\n", EGRESS_CONFIGURABLE_ACL_LARGE_TABLE_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_acl_egress_large_table); i++)
		printf(" %s", switch_acl_egress_large_table[i].name);

	printf("\n");
}

int switch_acl_egress_large_table_config(int argc, char **argv)
{
	u64 address;
	u32 value[EGRESS_CONFIGURABLE_ACL_LARGE_TABLE_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= EGRESS_CONFIGURABLE_ACL_LARGE_TABLE_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = EGRESS_CONFIGURABLE_ACL_LARGE_TABLE + index * EGRESS_CONFIGURABLE_ACL_LARGE_TABLE_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < EGRESS_CONFIGURABLE_ACL_LARGE_TABLE_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_acl_egress_large_table, ARRAY_SIZE(switch_acl_egress_large_table), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < EGRESS_CONFIGURABLE_ACL_LARGE_TABLE_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x %08x %08x]\n", __func__, index, address, value[3], value[2], value[1], value[0]);

	return 1;

ERR:
	switch_acl_egress_large_table_usage();
	return 0;
}

struct switch_field_def switch_acl_egress_small_table[] = {
	{EGRESS_ACL_SMALL_ARG_VALID, "valid", 1, 0},
	//{EGRESS_ACL_SMALL_ARG_COMPAREDATA, "compareData", 214, 1},
	{EGRESS_ACL_SMALL_ARG_COMPAREDATA0, "compareData0", 64, 1},
	{EGRESS_ACL_SMALL_ARG_COMPAREDATA1, "compareData1", 64, 65},
	{EGRESS_ACL_SMALL_ARG_COMPAREDATA2, "compareData2", 64, 129},
	{EGRESS_ACL_SMALL_ARG_COMPAREDATA3, "compareData3", 22, 193},
	{EGRESS_ACL_SMALL_ARG_DROPENABLE, "dropEnable", 1, 215},
	{EGRESS_ACL_SMALL_ARG_SENDTOCPU, "sendToCpu", 1, 216},
	{EGRESS_ACL_SMALL_ARG_SENDTOPORT, "sendToPort", 1, 217},
	{EGRESS_ACL_SMALL_ARG_DESTPORT, "destPort", 3, 218},
	{EGRESS_ACL_SMALL_ARG_NATOPVALID, "natOpValid", 1, 221},
	{EGRESS_ACL_SMALL_ARG_NATOPPTR, "natOpPtr", 10, 222},
	{EGRESS_ACL_SMALL_ARG_UPDATECOUNTER, "updateCounter", 1, 232},
	{EGRESS_ACL_SMALL_ARG_COUNTER, "counter", 6, 233},
	{EGRESS_ACL_SMALL_ARG_TUNNELENTRY, "tunnelEntry", 1, 239},
	{EGRESS_ACL_SMALL_ARG_TUNNELENTRYUCMC, "tunnelEntryUcMc", 1, 240},
	{EGRESS_ACL_SMALL_ARG_TUNNELENTRYPTR, "tunnelEntryPtr", 6, 241},
	{EGRESS_ACL_SMALL_ARG_TUNNELEXIT, "tunnelExit", 1, 247},
	{EGRESS_ACL_SMALL_ARG_TUNNELEXITPTR, "tunnelExitPtr", 7, 248},
};

void switch_acl_egress_small_table_usage(void)
{
	printf("Usage: npecmd switch acl egress_small <index> <--arg=value>\n");

	printf("maximum index: %d\n", EGRESS_CONFIGURABLE_ACL_SMALL_TABLE_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_acl_egress_small_table); i++)
		printf(" %s", switch_acl_egress_small_table[i].name);

	printf("\n");
}

int switch_acl_egress_small_table_config(int argc, char **argv)
{
	u64 address;
	u32 value[EGRESS_CONFIGURABLE_ACL_SMALL_TABLE_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= EGRESS_CONFIGURABLE_ACL_SMALL_TABLE_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = EGRESS_CONFIGURABLE_ACL_SMALL_TABLE + index * EGRESS_CONFIGURABLE_ACL_SMALL_TABLE_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < EGRESS_CONFIGURABLE_ACL_SMALL_TABLE_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_acl_egress_small_table, ARRAY_SIZE(switch_acl_egress_small_table), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < EGRESS_CONFIGURABLE_ACL_SMALL_TABLE_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x %08x %08x]\n", __func__, index, address, value[3], value[2], value[1], value[0]);

	return 1;

ERR:
	switch_acl_egress_small_table_usage();
	return 0;
}

struct switch_field_def switch_acl_egress_mask[] = {
	//{EGRESS_ACL_SEARCH_MASK_ARG_MASK_SMALL, "mask_small", 214, 0},
	{EGRESS_ACL_SEARCH_MASK_ARG_MASK_SMALL0, "mask_small0", 64, 0},
	{EGRESS_ACL_SEARCH_MASK_ARG_MASK_SMALL1, "mask_small1", 64, 64},
	{EGRESS_ACL_SEARCH_MASK_ARG_MASK_SMALL2, "mask_small2", 64, 128},
	{EGRESS_ACL_SEARCH_MASK_ARG_MASK_SMALL3, "mask_small3", 22, 192},
	//{EGRESS_ACL_SEARCH_MASK_ARG_MASK_LARGE, "mask_large", 214, 214},
	{EGRESS_ACL_SEARCH_MASK_ARG_MASK_LARGE0, "mask_large0", 64, 214},
	{EGRESS_ACL_SEARCH_MASK_ARG_MASK_LARGE1, "mask_large1", 64, 278},
	{EGRESS_ACL_SEARCH_MASK_ARG_MASK_LARGE2, "mask_large2", 64, 342},
	{EGRESS_ACL_SEARCH_MASK_ARG_MASK_LARGE3, "mask_large3", 22, 406},
};

void switch_acl_egress_mask_usage(void)
{
       printf("Usage: npecmd switch acl egress_mask <index> <--arg=value>\n");

       printf("maximum index: %d\n", EGRESS_CONFIGURABLE_ACL_SEARCH_MASK_MAX - 1);
       printf("arg :=");
       for (int i = 0; i < ARRAY_SIZE(switch_acl_egress_mask); i++)
               printf(" %s", switch_acl_egress_mask[i].name);

       printf("\n");
}

int switch_acl_egress_mask_config(int argc, char **argv)
{
       u64 address;
       u32 value[EGRESS_CONFIGURABLE_ACL_SEARCH_MASK_ADDR_PER_ENTRY] = {0}, index;
       int i;
       int high_valid = 0;

       if (argc <= 1) {
               printf("[%s] Failed! Invalid argc\n", __func__);
               goto ERR;
       }

       //check index
       if ((sscanf(argv[0], "%i", &index) != 1) || (index >= EGRESS_CONFIGURABLE_ACL_SEARCH_MASK_MAX)) {
               printf("[%s] Failed! Invalid index\n", __func__);
               goto ERR;
       }

       address = EGRESS_CONFIGURABLE_ACL_SEARCH_MASK + index * EGRESS_CONFIGURABLE_ACL_SEARCH_MASK_ADDR_PER_ENTRY;

       //get old value
       for (i = 0; i < EGRESS_CONFIGURABLE_ACL_SEARCH_MASK_ADDR_PER_ENTRY; i++)
               switch_read(address + i, &value[i]);

       //update
       if (!switch_field_setup(argc, argv, switch_acl_egress_mask, ARRAY_SIZE(switch_acl_egress_mask), value)) {
               printf("[%s] Failed! Invalid arg list\n", __func__);
               goto ERR;
       }

       //config
       for (i = 0; i < EGRESS_CONFIGURABLE_ACL_SEARCH_MASK_ADDR_PER_ENTRY; i++)
               switch_write(address + i, value[i]);

       printf("[%s] index %d addr 0x%lx value %08x\n", __func__, index, address, value[0]);

       npecmd_dbg("[%s] new value = 0x", __func__);
       for (i = EGRESS_CONFIGURABLE_ACL_SEARCH_MASK_ADDR_PER_ENTRY - 1; i >= 0 ; i--) {
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
       switch_acl_egress_mask_usage();
       return 0;
}

struct switch_field_def switch_acl_egress_tcam[] = {
	{EGRESS_ACL_TCAM_ARG_VALID, "valid", 1, 0},
	//{EGRESS_ACL_TCAM_ARG_MASK, "mask", 214, 1},
	{EGRESS_ACL_TCAM_ARG_MASK0, "mask0", 64, 1},
	{EGRESS_ACL_TCAM_ARG_MASK1, "mask1", 64, 65},
	{EGRESS_ACL_TCAM_ARG_MASK2, "mask2", 64, 129},
	{EGRESS_ACL_TCAM_ARG_MASK3, "mask3", 22, 193},
	//{EGRESS_ACL_TCAM_ARG_COMPAREDATA, "compareData", 214, 215},
	{EGRESS_ACL_TCAM_ARG_COMPAREDATA0, "compareData0", 64, 215},
	{EGRESS_ACL_TCAM_ARG_COMPAREDATA1, "compareData1", 64, 279},
	{EGRESS_ACL_TCAM_ARG_COMPAREDATA2, "compareData2", 64, 343},
	{EGRESS_ACL_TCAM_ARG_COMPAREDATA3, "compareData3", 22, 407},
};

void switch_acl_egress_tcam_usage(void)
{
        printf("Usage: npecmd switch acl egress_tcam <index> <--arg=value>\n");

	printf("maximum index: %d\n", EGRESS_CONFIGURABLE_ACL_TCAM_MAX - 1);
        printf("arg :=");
        for (int i = 0; i < ARRAY_SIZE(switch_acl_egress_tcam); i++)
                printf(" %s", switch_acl_egress_tcam[i].name);

        printf("\n");
}

int switch_acl_egress_tcam_config(int argc, char **argv)
{
	u64 address;
	u32 value[EGRESS_CONFIGURABLE_ACL_TCAM_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= EGRESS_CONFIGURABLE_ACL_TCAM_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = EGRESS_CONFIGURABLE_ACL_TCAM + index * EGRESS_CONFIGURABLE_ACL_TCAM_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < EGRESS_CONFIGURABLE_ACL_TCAM_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_acl_egress_tcam, ARRAY_SIZE(switch_acl_egress_tcam), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < EGRESS_CONFIGURABLE_ACL_TCAM_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x %08x %08x]\n", __func__, index, address, value[3], value[2], value[1], value[0]);

        return 1;

ERR:
        switch_acl_egress_tcam_usage();
        return 0;
}

struct switch_field_def switch_acl_egress_answer[] = {
	{EGRESS_ACL_TCAM_ANSWER_ARG_DROPENABLE, "dropEnable", 1, 0},
	{EGRESS_ACL_TCAM_ANSWER_ARG_SENDTOCPU, "sendToCpu", 1, 1},
	{EGRESS_ACL_TCAM_ANSWER_ARG_SENDTOPORT, "sendToPort", 1, 2},
	{EGRESS_ACL_TCAM_ANSWER_ARG_DESTPORT, "destPort", 3, 3},
	{EGRESS_ACL_TCAM_ANSWER_ARG_NATOPVALID, "natOpValid", 1, 6},
	{EGRESS_ACL_TCAM_ANSWER_ARG_NATOPPTR, "natOpPtr", 10, 7},
	{EGRESS_ACL_TCAM_ANSWER_ARG_UPDATECOUNTER, "updateCounter", 1, 17},
	{EGRESS_ACL_TCAM_ANSWER_ARG_COUNTER, "counter", 6, 18},
	{EGRESS_ACL_TCAM_ANSWER_ARG_TUNNELENTRY, "tunnelEntry", 1, 24},
	{EGRESS_ACL_TCAM_ANSWER_ARG_TUNNELENTRYUCMC, "tunnelEntryUcMc", 1, 25},
	{EGRESS_ACL_TCAM_ANSWER_ARG_TUNNELENTRYPTR, "tunnelEntryPtr", 6, 26},
	{EGRESS_ACL_TCAM_ANSWER_ARG_TUNNELEXIT, "tunnelExit", 1, 32},
	{EGRESS_ACL_TCAM_ANSWER_ARG_TUNNELEXITPTR, "tunnelExitPtr", 7, 33},
};

void switch_acl_egress_answer_usage(void)
{
        printf("Usage: npecmd switch acl egress_answer <index> <--arg=value>\n");

	printf("maximum index: %d\n", EGRESS_CONFIGURABLE_ACL_TCAM_ANSWER_MAX - 1);
        printf("arg :=");
        for (int i = 0; i < ARRAY_SIZE(switch_acl_egress_answer); i++)
                printf(" %s", switch_acl_egress_answer[i].name);

        printf("\n");
}

int switch_acl_egress_answer_config(int argc, char **argv)
{
	u64 address;
	u32 value[EGRESS_CONFIGURABLE_ACL_TCAM_ANSWER_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= EGRESS_CONFIGURABLE_ACL_TCAM_ANSWER_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = EGRESS_CONFIGURABLE_ACL_TCAM_ANSWER + index * EGRESS_CONFIGURABLE_ACL_TCAM_ANSWER_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < EGRESS_CONFIGURABLE_ACL_TCAM_ANSWER_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_acl_egress_answer, ARRAY_SIZE(switch_acl_egress_answer), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < EGRESS_CONFIGURABLE_ACL_TCAM_ANSWER_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x]\n", __func__,
				index, address, value[1], value[0]);

        return 1;

ERR:
        switch_acl_egress_answer_usage();
        return 0;
}


struct cmd_module switch_acl_cmd[] = {
	{"hash_cal", switch_acl_hash_cal_config, switch_acl_hash_cal_usage},
	/* multi ACLs */
	{"selection", switch_acl_selection_config, switch_acl_selection_usage},
	{"rules_setup", switch_acl_rules_setup_config, switch_acl_rules_setup_usage},

	/*Ingress ACLN*/
	/* Ingress ACL0 */
	{"ingress0_large", switch_acl_ingress_0_large_table_config, switch_acl_ingress_0_large_table_usage},
	{"ingress0_small", switch_acl_ingress_0_small_table_config, switch_acl_ingress_0_small_table_usage},
	{"ingress0_mask", switch_acl_ingress_0_mask_config, switch_acl_ingress_0_mask_usage},
	{"ingress0_tcam", switch_acl_ingress_0_tcam_config, switch_acl_ingress_0_tcam_usage},
	{"ingress0_answer", switch_acl_ingress_0_answer_config, switch_acl_ingress_0_answer_usage},

	/*Egress ACLN*/
	{"rule_tcam", switch_egress_acl_rule_pointer_tcam_config, switch_egress_acl_rule_pointer_tcam_usage},
	/* Egress ACL0 */
	{"egress_large", switch_acl_egress_large_table_config, switch_acl_egress_large_table_usage},
	{"egress_small", switch_acl_egress_small_table_config, switch_acl_egress_small_table_usage},
	{"egress_mask", switch_acl_egress_mask_config, switch_acl_egress_mask_usage},
	{"egress_tcam", switch_acl_egress_tcam_config, switch_acl_egress_tcam_usage},
	{"egress_answer", switch_acl_egress_answer_config, switch_acl_egress_answer_usage},

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
