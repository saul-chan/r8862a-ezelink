// SPDX-License-Identifier: GPL-2.0
// Copyright(c) 2023 Clourneysemi Corporation.

#include "npe_switch.h"
#include "switch_conf.h"
#include "switch_tunnel.h"

struct switch_field_def switch_l2_tunnel_decoder[] = {
	{L2_TUNNEL_DECODER_ARG_DEFAULTETHCTYPEVALID, "defaultEthCTypeValid", 1, 0},
	{L2_TUNNEL_DECODER_ARG_DEFAULTETHCTYPE, "defaultEthCType", 16, 1},
	{L2_TUNNEL_DECODER_ARG_DEFAULTETHSTYPEVALID, "defaultEthSTypeValid", 1, 17},
	{L2_TUNNEL_DECODER_ARG_DEFAULTETHSTYPE, "defaultEthSType", 16, 18},
};

void switch_l2_tunnel_decoder_usage(void)
{
	printf("Usage: npecmd switch tunnel l2_decoder <index> <--arg=value>\n");
	printf("maximum index: %d\n", L2_TUNNEL_DECODER_SETUP_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_l2_tunnel_decoder); i++)
		printf(" %s", switch_l2_tunnel_decoder[i].name);

	printf("\n");
}

int switch_l2_tunnel_decoder_config(int argc, char **argv)
{
	u64 address;
	u32 value[L2_TUNNEL_DECODER_SETUP_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= L2_TUNNEL_DECODER_SETUP_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = L2_TUNNEL_DECODER_SETUP + index * L2_TUNNEL_DECODER_SETUP_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < L2_TUNNEL_DECODER_SETUP_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_l2_tunnel_decoder, ARRAY_SIZE(switch_l2_tunnel_decoder), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < L2_TUNNEL_DECODER_SETUP_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x]\n", __func__, index, address, value[1], value[0]);

	return 1;

ERR:
	switch_l2_tunnel_decoder_usage();
	return 0;
}

struct switch_field_def switch_tunnel_exit_lookup_tcam[] = {
	{TUNNEL_EXIT_LOOKUP_TCAM_ARG_VALID, "valid", 1, 0},
	{TUNNEL_EXIT_LOOKUP_TCAM_ARG_SNAPLLC_MASK, "snapLlc_mask", 1, 1},
	{TUNNEL_EXIT_LOOKUP_TCAM_ARG_SNAPLLC, "snapLlc", 1, 2},
	{TUNNEL_EXIT_LOOKUP_TCAM_ARG_ETHTYPE_MASK, "ethType_mask", 16, 3},
	{TUNNEL_EXIT_LOOKUP_TCAM_ARG_ETHTYPE, "ethType", 16, 19},
	{TUNNEL_EXIT_LOOKUP_TCAM_ARG_L3TYPE_MASK, "l3Type_mask", 3, 35},
	{TUNNEL_EXIT_LOOKUP_TCAM_ARG_L3TYPE, "l3Type", 3, 38},
	{TUNNEL_EXIT_LOOKUP_TCAM_ARG_IPSA_MASK, "ipSa_mask", 128, 41},
	{TUNNEL_EXIT_LOOKUP_TCAM_ARG_IPSA, "ipSa", 128, 169},
	{TUNNEL_EXIT_LOOKUP_TCAM_ARG_IPDA_MASK, "ipDa_mask", 128, 297},
	{TUNNEL_EXIT_LOOKUP_TCAM_ARG_IPDA, "ipDa", 128, 425},
	{TUNNEL_EXIT_LOOKUP_TCAM_ARG_L4TYPE_MASK, "l4Type_mask", 2, 553},
	{TUNNEL_EXIT_LOOKUP_TCAM_ARG_L4TYPE, "l4Type", 2, 555},
	{TUNNEL_EXIT_LOOKUP_TCAM_ARG_L4PROTOCOL_MASK, "l4Protocol_mask", 8, 557},
	{TUNNEL_EXIT_LOOKUP_TCAM_ARG_L4PROTOCOL, "l4Protocol", 8, 565},
	{TUNNEL_EXIT_LOOKUP_TCAM_ARG_L4SP_MASK, "l4Sp_mask", 16, 573},
	{TUNNEL_EXIT_LOOKUP_TCAM_ARG_L4SP, "l4Sp", 16, 589},
	{TUNNEL_EXIT_LOOKUP_TCAM_ARG_L4DP_MASK, "l4Dp_mask", 16, 605},
	{TUNNEL_EXIT_LOOKUP_TCAM_ARG_L4DP, "l4Dp", 16, 621},
};

void switch_tunnel_exit_lookup_tcam_usage(void)
{
	printf("Usage: npecmd switch tunnel exit_tcam <index> <--arg=value>\n");
	printf("maximum index: %d\n", TUNNEL_EXIT_LOOKUP_TCAM_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_tunnel_exit_lookup_tcam); i++)
		printf(" %s", switch_tunnel_exit_lookup_tcam[i].name);

	printf("\n");
}

#define TUNNEL_L3_TYPE_IPV4			0
#define TUNNEL_L3_TYPE_IPV6			1
#define TUNNEL_L3_TYPE_ONE_MPLS_LABEL		2
#define TUNNEL_L3_TYPE_TWO_MPLS_LABELS		3
#define TUNNEL_L3_TYPE_THREE_MPLS_LABELS	4
#define TUNNEL_L3_TYPE_FOUR_MPLS_LABELS		5
#define TUNNEL_L3_TYPE_UNKNOWN			7

int switch_tunnel_exit_lookup_tcam_config(int argc, char **argv)
{
	u64 address;
	u32 value[TUNNEL_EXIT_LOOKUP_TCAM_ADDR_PER_ENTRY] = {0}, index, tmp;
	u8 l3Type, iptype;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= TUNNEL_EXIT_LOOKUP_TCAM_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = TUNNEL_EXIT_LOOKUP_TCAM + index * TUNNEL_EXIT_LOOKUP_TCAM_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < TUNNEL_EXIT_LOOKUP_TCAM_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//get l3Type and update iptype
	l3Type = (value[1] & 0x1C0) >> 6;
	for (i = 1; i < argc - 1; i++) {
		if (sscanf(argv[i], "--l3Type=%i", &tmp) == 1)
			l3Type = tmp & 0x7;
	}
	/* unsupport two/three/four mpls labels */
	if (l3Type == TUNNEL_L3_TYPE_IPV4) {
		iptype = SWITCH_FIELD_TYPE_IPV4;
	} else if (l3Type == TUNNEL_L3_TYPE_IPV6) {
		iptype = SWITCH_FIELD_TYPE_IPV6;
	}

	switch_tunnel_exit_lookup_tcam[TUNNEL_EXIT_LOOKUP_TCAM_ARG_IPSA_MASK].flag = iptype;
	switch_tunnel_exit_lookup_tcam[TUNNEL_EXIT_LOOKUP_TCAM_ARG_IPSA].flag = iptype;
	switch_tunnel_exit_lookup_tcam[TUNNEL_EXIT_LOOKUP_TCAM_ARG_IPDA_MASK].flag = iptype;
	switch_tunnel_exit_lookup_tcam[TUNNEL_EXIT_LOOKUP_TCAM_ARG_IPDA].flag = iptype;

	//update
	if (!switch_field_setup(argc, argv, switch_tunnel_exit_lookup_tcam, ARRAY_SIZE(switch_tunnel_exit_lookup_tcam), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < TUNNEL_EXIT_LOOKUP_TCAM_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x %08x -- %08x %08x %08x]\n", __func__, index, address,
			value[TUNNEL_EXIT_LOOKUP_TCAM_ADDR_PER_ENTRY - 1],
			value[TUNNEL_EXIT_LOOKUP_TCAM_ADDR_PER_ENTRY - 2],
			value[TUNNEL_EXIT_LOOKUP_TCAM_ADDR_PER_ENTRY - 3],
			value[2], value[1], value[0]);

	return 1;

ERR:
	switch_tunnel_exit_lookup_tcam_usage();
	return 0;
}

struct switch_field_def switch_tunnel_exit_lookup_answer[] = {
	{TUNNEL_EXIT_LOOKUP_ANSWER_ARG_SRCPORTMASK, "srcPortMask", 6, 0},
	{TUNNEL_EXIT_LOOKUP_ANSWER_ARG_SENDTOCPU, "sendToCpu", 1, 6},
	{TUNNEL_EXIT_LOOKUP_ANSWER_ARG_SECONDSHIFT, "secondShift", 8, 7},
	{TUNNEL_EXIT_LOOKUP_ANSWER_ARG_SECONDINCLUDEVLAN, "secondIncludeVlan", 1, 15},
	{TUNNEL_EXIT_LOOKUP_ANSWER_ARG_DIRECT, "direct", 1, 16},
	/*over 64 bits*/
	//{TUNNEL_EXIT_LOOKUP_ANSWER_ARG_KEY, "key", 80, 17},
	{TUNNEL_EXIT_LOOKUP_ANSWER_ARG_KEY0, "key0", 64, 17},
	{TUNNEL_EXIT_LOOKUP_ANSWER_ARG_KEY1, "key1", 16, 81},
	/*over 64 bits*/
	//{TUNNEL_EXIT_LOOKUP_ANSWER_ARG_LOOKUPMASK, "lookupMask", 80, 97},
	{TUNNEL_EXIT_LOOKUP_ANSWER_ARG_LOOKUPMASK0, "lookupMask0", 64, 97},
	{TUNNEL_EXIT_LOOKUP_ANSWER_ARG_LOOKUPMASK1, "lookupMask1", 16, 161},
	{TUNNEL_EXIT_LOOKUP_ANSWER_ARG_TABINDEX, "tabIndex", 2, 177},
};

void switch_tunnel_exit_lookup_answer_usage(void)
{
	printf("Usage: npecmd switch tunnel exit_answer <index> <--arg=value>\n");
	printf("maximum index: %d\n", TUNNEL_EXIT_LOOKUP_TCAM_ANSWER_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_tunnel_exit_lookup_answer); i++)
		printf(" %s", switch_tunnel_exit_lookup_answer[i].name);

	printf("\n");
}

int switch_tunnel_exit_lookup_answer_config(int argc, char **argv)
{
	u64 address;
	u32 value[TUNNEL_EXIT_LOOKUP_TCAM_ANSWER_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= TUNNEL_EXIT_LOOKUP_TCAM_ANSWER_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = TUNNEL_EXIT_LOOKUP_TCAM_ANSWER + index *  TUNNEL_EXIT_LOOKUP_TCAM_ANSWER_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < TUNNEL_EXIT_LOOKUP_TCAM_ANSWER_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_tunnel_exit_lookup_answer, ARRAY_SIZE(switch_tunnel_exit_lookup_answer), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < TUNNEL_EXIT_LOOKUP_TCAM_ANSWER_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x %08x %08x %08x %08x]\n", __func__, index, address,
				value[5], value[4], value[3], value[2], value[1], value[0]);

	return 1;

ERR:
	switch_tunnel_exit_lookup_answer_usage();
	return 0;
}

struct switch_field_def switch_second_tunnel_exit_lookup_tcam[] = {
	{SECOND_TUNNEL_EXIT_LOOKUP_TCAM_ARG_VALID, "valid", 1, 0},
	/*over 64 bits*/
	//{SECOND_TUNNEL_EXIT_LOOKUP_TCAM_ARG_PKTKEY_MASK, "pktKey_mask", 80, 1},
	{SECOND_TUNNEL_EXIT_LOOKUP_TCAM_ARG_PKTKEY_MASK0, "pktKey_mask0", 64, 1},
	{SECOND_TUNNEL_EXIT_LOOKUP_TCAM_ARG_PKTKEY_MASK1, "pktKey_mask1", 16, 65},
	/*over 64 bits*/
	//{SECOND_TUNNEL_EXIT_LOOKUP_TCAM_ARG_PKTKEY, "pktKey", 80, 81},
	{SECOND_TUNNEL_EXIT_LOOKUP_TCAM_ARG_PKTKEY0, "pktKey0", 64, 81},
	{SECOND_TUNNEL_EXIT_LOOKUP_TCAM_ARG_PKTKEY1, "pktKey1", 16, 145},
	{SECOND_TUNNEL_EXIT_LOOKUP_TCAM_ARG_TABKEY_MASK, "tabKey_mask", 2, 161},
	{SECOND_TUNNEL_EXIT_LOOKUP_TCAM_ARG_TABKEY, "tabKey", 2, 163},
};

void switch_second_tunnel_exit_lookup_tcam_usage(void)
{
	printf("Usage: npecmd switch tunnel second_exit_tcam <index> <--arg=value>\n");
	printf("maximum index: %d\n", SECOND_TUNNEL_EXIT_LOOKUP_TCAM_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_second_tunnel_exit_lookup_tcam); i++)
		printf(" %s", switch_second_tunnel_exit_lookup_tcam[i].name);

	printf("\n");
}

int switch_second_tunnel_exit_lookup_tcam_config(int argc, char **argv)
{
	u64 address;
	u32 value[SECOND_TUNNEL_EXIT_LOOKUP_TCAM_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= SECOND_TUNNEL_EXIT_LOOKUP_TCAM_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = SECOND_TUNNEL_EXIT_LOOKUP_TCAM + index *  SECOND_TUNNEL_EXIT_LOOKUP_TCAM_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < SECOND_TUNNEL_EXIT_LOOKUP_TCAM_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_second_tunnel_exit_lookup_tcam, ARRAY_SIZE(switch_second_tunnel_exit_lookup_tcam), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < SECOND_TUNNEL_EXIT_LOOKUP_TCAM_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x %08x %08x %08x %08x]\n", __func__, index, address,
				value[5], value[4], value[3], value[2], value[1], value[0]);

	return 1;

ERR:
	switch_second_tunnel_exit_lookup_tcam_usage();
	return 0;
}

struct switch_field_def switch_second_tunnel_exit_lookup_answer[] = {
	{SECOND_TUNNEL_EXIT_LOOKUP_ANSWER_ARG_HOWMANYBYTESTOREMOVE, "howManyBytesToRemove", 8, 0},
	{SECOND_TUNNEL_EXIT_LOOKUP_ANSWER_ARG_UPDATEETHTYPE, "updateEthType", 1, 8},
	{SECOND_TUNNEL_EXIT_LOOKUP_ANSWER_ARG_ETHTYPE, "ethType", 16, 9},
	{SECOND_TUNNEL_EXIT_LOOKUP_ANSWER_ARG_REMOVEVLAN, "removeVlan", 1, 25},
	{SECOND_TUNNEL_EXIT_LOOKUP_ANSWER_ARG_UPDATEL4PROTOCOL, "updateL4Protocol", 1, 26},
	{SECOND_TUNNEL_EXIT_LOOKUP_ANSWER_ARG_L4PROTOCOL, "l4Protocol", 8, 27},
	{SECOND_TUNNEL_EXIT_LOOKUP_ANSWER_ARG_WHERETOREMOVE, "whereToRemove", 2, 35},
	{SECOND_TUNNEL_EXIT_LOOKUP_ANSWER_ARG_DROPPKT, "dropPkt", 1, 37},
	{SECOND_TUNNEL_EXIT_LOOKUP_ANSWER_ARG_DONTEXIT, "dontExit", 1, 38},
	{SECOND_TUNNEL_EXIT_LOOKUP_ANSWER_ARG_REPLACEVID, "replaceVid", 1, 39},
	{SECOND_TUNNEL_EXIT_LOOKUP_ANSWER_ARG_NEWVID, "newVid", 12, 40},
	{SECOND_TUNNEL_EXIT_LOOKUP_ANSWER_ARG_TUNNELEXITEGRESSPTR, "tunnelExitEgressPtr", 5, 52},
};

void switch_second_tunnel_exit_lookup_answer_usage(void)
{
	printf("Usage: npecmd switch tunnel second_exit_answer <index> <--arg=value>\n");
	printf("maximum index: %d\n", SECOND_TUNNEL_EXIT_LOOKUP_TCAM_ANSWER_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_second_tunnel_exit_lookup_answer); i++)
		printf(" %s", switch_second_tunnel_exit_lookup_answer[i].name);

	printf("\n");
}

int switch_second_tunnel_exit_lookup_answer_config(int argc, char **argv)
{
	u64 address;
	u32 value[SECOND_TUNNEL_EXIT_LOOKUP_TCAM_ANSWER_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= SECOND_TUNNEL_EXIT_LOOKUP_TCAM_ANSWER_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = SECOND_TUNNEL_EXIT_LOOKUP_TCAM_ANSWER + index *  SECOND_TUNNEL_EXIT_LOOKUP_TCAM_ANSWER_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < SECOND_TUNNEL_EXIT_LOOKUP_TCAM_ANSWER_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_second_tunnel_exit_lookup_answer, ARRAY_SIZE(switch_second_tunnel_exit_lookup_answer), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < SECOND_TUNNEL_EXIT_LOOKUP_TCAM_ANSWER_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x]\n", __func__, index, address, value[1], value[0]);

	return 1;

ERR:
	switch_second_tunnel_exit_lookup_answer_usage();
	return 0;
}

struct switch_field_def switch_egress_tunnel_exit_table[] = {
	{EGRESS_TUNNEL_EXIT_ARG_HOWMANYBYTESTOREMOVE, "howManyBytesToRemove", 8, 0},
	{EGRESS_TUNNEL_EXIT_ARG_UPDATEETHTYPE, "updateEthType", 1, 8},
	{EGRESS_TUNNEL_EXIT_ARG_ETHTYPE, "ethType", 16, 9},
	{EGRESS_TUNNEL_EXIT_ARG_REMOVEVLAN, "removeVlan", 1, 25},
	{EGRESS_TUNNEL_EXIT_ARG_UPDATEL4PROTOCOL, "updateL4Protocol", 1, 26},
	{EGRESS_TUNNEL_EXIT_ARG_L4PROTOCOL, "l4Protocol", 8, 27},
	{EGRESS_TUNNEL_EXIT_ARG_WHERETOREMOVE, "whereToRemove", 2, 35},
};

void switch_egress_tunnel_exit_table_usage(void)
{
	printf("Usage: npecmd switch tunnel egress_exit <index> <--arg=value>\n");
	printf("maximum index: %d\n", EGRESS_TUNNEL_EXIT_TABLE_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_egress_tunnel_exit_table); i++)
		printf(" %s", switch_egress_tunnel_exit_table[i].name);

	printf("\n");
}

int switch_egress_tunnel_exit_table_config(int argc, char **argv)
{
	u64 address;
	u32 value[EGRESS_TUNNEL_EXIT_TABLE_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= EGRESS_TUNNEL_EXIT_TABLE_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = EGRESS_TUNNEL_EXIT_TABLE + index *EGRESS_TUNNEL_EXIT_TABLE_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < EGRESS_TUNNEL_EXIT_TABLE_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_egress_tunnel_exit_table, ARRAY_SIZE(switch_egress_tunnel_exit_table), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < EGRESS_TUNNEL_EXIT_TABLE_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x]\n", __func__, index, address, value[1], value[0]);

	return 1;

ERR:
	switch_egress_tunnel_exit_table_usage();
	return 0;
}

struct switch_field_def switch_tunnel_entry_header_data[] = {
	/*over 64 bits*/
	//{TUNNEL_ENTRY_HEADER_ARG_DATA, "data", 640, 0},
	{TUNNEL_ENTRY_HEADER_ARG_DATA0, "data0", 64, 0},
	{TUNNEL_ENTRY_HEADER_ARG_DATA1, "data1", 64, 64},
	{TUNNEL_ENTRY_HEADER_ARG_DATA2, "data2", 64, 128},
	{TUNNEL_ENTRY_HEADER_ARG_DATA3, "data3", 64, 192},
	{TUNNEL_ENTRY_HEADER_ARG_DATA4, "data4", 64, 256},
	{TUNNEL_ENTRY_HEADER_ARG_DATA5, "data5", 64, 320},
	{TUNNEL_ENTRY_HEADER_ARG_DATA6, "data6", 64, 384},
	{TUNNEL_ENTRY_HEADER_ARG_DATA7, "data7", 64, 448},
	{TUNNEL_ENTRY_HEADER_ARG_DATA8, "data8", 64, 512},
	{TUNNEL_ENTRY_HEADER_ARG_DATA9, "data9", 64, 576},
};

void switch_tunnel_entry_header_data_usage(void)
{
	printf("Usage: npecmd switch tunnel entry_header <index> <--arg=value>\n");
	printf("maximum index: %d\n", TUNNEL_ENTRY_HEADER_DATA_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_tunnel_entry_header_data); i++)
		printf(" %s", switch_tunnel_entry_header_data[i].name);

	printf("\n");
}


#define SWITCH_TUNNEL_ENTRY_HEADER_DATA_MAX_ENTRY	20
int switch_tunnel_entry_header_data_config(int argc, char **argv)
{
	u64 address;
	u32 value[TUNNEL_ENTRY_HEADER_DATA_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= TUNNEL_ENTRY_HEADER_DATA_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = TUNNEL_ENTRY_HEADER_DATA + index * TUNNEL_ENTRY_HEADER_DATA_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < TUNNEL_ENTRY_HEADER_DATA_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_tunnel_entry_header_data, ARRAY_SIZE(switch_tunnel_entry_header_data), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < TUNNEL_ENTRY_HEADER_DATA_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x %08x -- %08x %08x %08x]\n", __func__, index, address,
			value[SWITCH_TUNNEL_ENTRY_HEADER_DATA_MAX_ENTRY - 1],
			value[SWITCH_TUNNEL_ENTRY_HEADER_DATA_MAX_ENTRY - 2],
			value[SWITCH_TUNNEL_ENTRY_HEADER_DATA_MAX_ENTRY - 3],
			value[2], value[1], value[0]);

	return 1;
ERR:
	switch_tunnel_entry_header_data_usage();
	return 0;
}


struct switch_field_def switch_l2_tunnel_entry_instr_table[] = {
	{L2_TUNNEL_ENTRY_INSTR_ARG_L3TYPE, "l3Type", 2, 0},
	{L2_TUNNEL_ENTRY_INSTR_ARG_HASUDP, "hasUdp", 1, 2},
	{L2_TUNNEL_ENTRY_INSTR_ARG_UPDATEETHERTYPE, "updateEtherType", 1, 3},
	{L2_TUNNEL_ENTRY_INSTR_ARG_OUTERETHERTYPE, "outerEtherType", 16, 4},
};

void switch_l2_tunnel_entry_instr_table_usage(void)
{
	printf("Usage: npecmd switch tunnel l2_entry_instr <index> <--arg=value>\n");
	printf("maximum index: %d\n", L2_TUNNEL_ENTRY_INSTRUCTION_TABLE_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_l2_tunnel_entry_instr_table); i++)
		printf(" %s", switch_l2_tunnel_entry_instr_table[i].name);

	printf("\n");
}

int switch_l2_tunnel_entry_instr_table_config(int argc, char **argv)
{
	u64 address;
	u32 value[L2_TUNNEL_ENTRY_INSTRUCTION_TABLE_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= L2_TUNNEL_ENTRY_INSTRUCTION_TABLE_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = L2_TUNNEL_ENTRY_INSTRUCTION_TABLE + index * L2_TUNNEL_ENTRY_INSTRUCTION_TABLE_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < L2_TUNNEL_ENTRY_INSTRUCTION_TABLE_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_l2_tunnel_entry_instr_table, ARRAY_SIZE(switch_l2_tunnel_entry_instr_table), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < L2_TUNNEL_ENTRY_INSTRUCTION_TABLE_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x%08x\n", __func__, index, address, value[0]);

	return 1;

ERR:
	switch_l2_tunnel_entry_instr_table_usage();
	return 0;
}

struct switch_field_def switch_tunnel_entry_instr_table[] = {
	{TUNNEL_ENTRY_INSTR_ARG_TUNNELENTRYTYPE, "tunnelEntryType", 2, 0},
	{TUNNEL_ENTRY_INSTR_ARG_INSERTLENGTH, "insertLength", 1, 2},
	{TUNNEL_ENTRY_INSTR_ARG_LENGTHPOS, "lengthPos", 7, 3},
	{TUNNEL_ENTRY_INSTR_ARG_LENGTHNEGOFFSET, "lengthNegOffset", 14, 10},
	{TUNNEL_ENTRY_INSTR_ARG_LENGTHPOSOFFSET, "lengthPosOffset", 14, 24},
	{TUNNEL_ENTRY_INSTR_ARG_INCVLANSINLENGTH, "incVlansInLength", 1, 38},
	{TUNNEL_ENTRY_INSTR_ARG_TUNNELHEADERPTR, "tunnelHeaderPtr", 4, 39},
	{TUNNEL_ENTRY_INSTR_ARG_TUNNELHEADERLEN, "tunnelHeaderLen", 7, 43},
};

void switch_tunnel_entry_instr_table_usage(void)
{
	printf("Usage: npecmd switch tunnel entry_instr <index> <--arg=value>\n");
	printf("maximum index: %d\n", TUNNEL_ENTRY_INSTRUCTION_TABLE_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_tunnel_entry_instr_table); i++)
		printf(" %s", switch_tunnel_entry_instr_table[i].name);

	printf("\n");
}

int switch_tunnel_entry_instr_table_config(int argc, char **argv)
{
	u64 address;
	u32 value[TUNNEL_ENTRY_INSTRUCTION_TABLE_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= TUNNEL_ENTRY_INSTRUCTION_TABLE_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = TUNNEL_ENTRY_INSTRUCTION_TABLE + index * TUNNEL_ENTRY_INSTRUCTION_TABLE_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < TUNNEL_ENTRY_INSTRUCTION_TABLE_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_tunnel_entry_instr_table, ARRAY_SIZE(switch_tunnel_entry_instr_table), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < TUNNEL_ENTRY_INSTRUCTION_TABLE_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x]\n", __func__, index, address, value[1], value[0]);

	return 1;

ERR:
	switch_tunnel_entry_instr_table_usage();
	return 0;
}

struct cmd_module switch_tunnel_cmd[] = {
	{"l2_decoder", switch_l2_tunnel_decoder_config, switch_l2_tunnel_decoder_usage},
	{"exit_tcam", switch_tunnel_exit_lookup_tcam_config, switch_tunnel_exit_lookup_tcam_usage},
	{"exit_answer", switch_tunnel_exit_lookup_answer_config, switch_tunnel_exit_lookup_answer_usage},
	{"second_exit_tcam", switch_second_tunnel_exit_lookup_tcam_config, switch_second_tunnel_exit_lookup_tcam_usage},
	{"second_exit_answer", switch_second_tunnel_exit_lookup_answer_config, switch_second_tunnel_exit_lookup_answer_usage},
	{"egress_exit", switch_egress_tunnel_exit_table_config, switch_egress_tunnel_exit_table_usage},
	{"entry_header", switch_tunnel_entry_header_data_config, switch_tunnel_entry_header_data_usage},
	{"l2_entry_instr", switch_l2_tunnel_entry_instr_table_config, switch_l2_tunnel_entry_instr_table_usage},
	{"entry_instr", switch_tunnel_entry_instr_table_config, switch_tunnel_entry_instr_table_usage},
};

void switch_tunnel_usage(void)
{
	printf("Usage: npecmd switch tunnel SUB-FUNCTION {COMMAND}\n");
	printf("where  SUB-FUNCTION :=");
	for (int i = 0; i < ARRAY_SIZE(switch_tunnel_cmd); i++)
		printf(" %s", switch_tunnel_cmd[i].name);

	printf("\n");
}

int switch_tunnel_config(int argc, char **argv)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(switch_tunnel_cmd); i++) {
		if (!strcasecmp(argv[0], switch_tunnel_cmd[i].name)) {
			if (argc <= 1) {
				if (switch_tunnel_cmd[i].usage)
					switch_tunnel_cmd[i].usage();

				return 0;
			}

			if (switch_tunnel_cmd[i].func)
				return switch_tunnel_cmd[i].func(argc - 1, argv + 1);
		}
	}

	switch_tunnel_usage();
	return 0;
}
