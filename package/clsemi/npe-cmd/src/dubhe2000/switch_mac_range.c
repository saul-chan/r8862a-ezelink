// SPDX-License-Identifier: GPL-2.0
// Copyright(c) 2023 Clourneysemi Corporation.

#include "npe_switch.h"
#include "switch_conf.h"
#include "switch_mac_range.h"

struct switch_field_def switch_reserved_dmac_addr_range[] = {
	{RESERVED_DMAC_ADDR_RANGE_ARG_STARTADDR, "startAddr", 48, 0, SWITCH_FIELD_TYPE_MACADDR},
	{RESERVED_DMAC_ADDR_RANGE_ARG_STOPADDR, "stopAddr", 48, 48, SWITCH_FIELD_TYPE_MACADDR},
	{RESERVED_DMAC_ADDR_RANGE_ARG_DROPENABLE, "dropEnable", 1, 96},
	{RESERVED_DMAC_ADDR_RANGE_ARG_SENDTOCPU, "sendToCpu", 1, 97},
	{RESERVED_DMAC_ADDR_RANGE_ARG_FORCEQUEUE, "forceQueue", 1, 98},
	{RESERVED_DMAC_ADDR_RANGE_ARG_EQUEUE, "eQueue", 3, 99},
	{RESERVED_DMAC_ADDR_RANGE_ARG_COLOR, "color", 2, 102},
	{RESERVED_DMAC_ADDR_RANGE_ARG_FORCECOLOR, "forceColor", 1, 104},
	{RESERVED_DMAC_ADDR_RANGE_ARG_MMPVALID, "mmpValid", 1, 105},
	{RESERVED_DMAC_ADDR_RANGE_ARG_MMPPTR, "mmpPtr", 5, 106},
	{RESERVED_DMAC_ADDR_RANGE_ARG_MMPORDER, "mmpOrder", 2, 111},
	{RESERVED_DMAC_ADDR_RANGE_ARG_ENABLE, "enable", 6, 113},
};

void switch_reserved_dmac_addr_range_usage(void)
{
	printf("Usage: npecmd switch mac_range dest <index> <--arg=value>\n");
	printf("maximum index: %d\n", RESERVED_DESTINATION_MAC_ADDRESS_RANGE_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_reserved_dmac_addr_range); i++)
		printf(" %s", switch_reserved_dmac_addr_range[i].name);

	printf("\n");
}

int switch_reserved_dmac_addr_range_config(int argc, char **argv)
{
	u64 address;
	u32 value[RESERVED_DESTINATION_MAC_ADDRESS_RANGE_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= RESERVED_DESTINATION_MAC_ADDRESS_RANGE_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = RESERVED_DESTINATION_MAC_ADDRESS_RANGE + index * RESERVED_DESTINATION_MAC_ADDRESS_RANGE_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < RESERVED_DESTINATION_MAC_ADDRESS_RANGE_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_reserved_dmac_addr_range, ARRAY_SIZE(switch_reserved_dmac_addr_range), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < RESERVED_DESTINATION_MAC_ADDRESS_RANGE_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] addr 0x%lx value 0x%08x\n", __func__, address, value[0]);

	return 1;

ERR:
	switch_reserved_dmac_addr_range_usage();
	return 0;
}

struct switch_field_def switch_reserved_smac_addr_range[] = {
	{RESERVED_SMAC_ADDR_RANGE_ARG_STARTADDR, "startAddr", 48, 0, SWITCH_FIELD_TYPE_MACADDR},
	{RESERVED_SMAC_ADDR_RANGE_ARG_STOPADDR, "stopAddr", 48, 48, SWITCH_FIELD_TYPE_MACADDR},
	{RESERVED_SMAC_ADDR_RANGE_ARG_DROPENABLE, "dropEnable", 1, 96},
	{RESERVED_SMAC_ADDR_RANGE_ARG_SENDTOCPU, "sendToCpu", 1, 97},
	{RESERVED_SMAC_ADDR_RANGE_ARG_FORCEQUEUE, "forceQueue", 1, 98},
	{RESERVED_SMAC_ADDR_RANGE_ARG_EQUEUE, "eQueue", 3, 99},
	{RESERVED_SMAC_ADDR_RANGE_ARG_COLOR, "color", 2, 102},
	{RESERVED_SMAC_ADDR_RANGE_ARG_FORCECOLOR, "forceColor", 1, 104},
	{RESERVED_SMAC_ADDR_RANGE_ARG_MMPVALID, "mmpValid", 1, 105},
	{RESERVED_SMAC_ADDR_RANGE_ARG_MMPPTR, "mmpPtr", 5, 106},
	{RESERVED_SMAC_ADDR_RANGE_ARG_MMPORDER, "mmpOrder", 2, 111},
	{RESERVED_SMAC_ADDR_RANGE_ARG_ENABLE, "enable", 6, 113},
};

void switch_reserved_smac_addr_range_usage(void)
{
	printf("Usage: npecmd switch mac_range source <index> <--arg=value>\n");
	printf("maximum index: %d\n", RESERVED_SOURCE_MAC_ADDRESS_RANGE_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_reserved_smac_addr_range); i++)
		printf(" %s", switch_reserved_smac_addr_range[i].name);

	printf("\n");
}

int switch_reserved_smac_addr_range_config(int argc, char **argv)
{
	u64 address;
	u32 value[RESERVED_SOURCE_MAC_ADDRESS_RANGE_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= RESERVED_SOURCE_MAC_ADDRESS_RANGE_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = RESERVED_SOURCE_MAC_ADDRESS_RANGE + index * RESERVED_SOURCE_MAC_ADDRESS_RANGE_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < RESERVED_SOURCE_MAC_ADDRESS_RANGE_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_reserved_smac_addr_range, ARRAY_SIZE(switch_reserved_smac_addr_range), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < RESERVED_SOURCE_MAC_ADDRESS_RANGE_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] addr 0x%lx value 0x%08x\n", __func__, address, value[0]);

	return 1;

ERR:
	switch_reserved_smac_addr_range_usage();
	return 0;
}

struct switch_field_def switch_l2_reserved_multi_addr_base[] = {
	{L2_RESERVED_MULTI_ADDR_ARG_MACBASE, "macBase", 40, 0},
	{L2_RESERVED_MULTI_ADDR_ARG_MASK, "mask", 16, 40},

};

struct switch_field_def switch_l2_reserved_multi_addr_action[] = {
	{L2_RESERVED_MULTI_ADDR_ARG_DROPMASK, "dropMask", 6, 0},
	{L2_RESERVED_MULTI_ADDR_ARG_SENDTOCPUMASK, "sendToCpuMask", 6, 6},
};

void switch_l2_reserved_multi_addr_usage(void)
{
	printf("Usage: npecmd switch mac_range l2_multi MACADDR <mask> <--arg=value>\n");
	printf("maximum mask: 0x%x\n", get_one_bits(0, switch_l2_reserved_multi_addr_base[L2_RESERVED_MULTI_ADDR_ARG_MASK].width - 1));
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_l2_reserved_multi_addr_action); i++)
		printf(" %s", switch_l2_reserved_multi_addr_action[i].name);

	printf("\n");
}

int switch_l2_reserved_multi_addr_config(int argc, char **argv)
{
	u64 address, macaddr;
	u32 value[L2_RESERVED_MULTICAST_ADDRESS_BASE_ADDR_PER_ENTRY] = {0}, mask;
	u32 action_value[L2_RESERVED_MULTICAST_ADDRESS_ACTION_ADDR_PER_ENTRY];
	u16 index;
	int i;

	if (argc <= 2) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	/* base */
	// 1. check macaddr
	if (!switch_mac_to_u64(argv[0], &macaddr)) {
		printf("[%s] Add Failed! Invalid MACADDR\n", __func__);
		goto ERR;
	}

	// 2. check mask
	if ((sscanf(argv[1], "%i", &mask) != 1)
			|| mask > get_one_bits(0, switch_l2_reserved_multi_addr_base[L2_RESERVED_MULTI_ADDR_ARG_MASK].width - 1)) {
		printf("[%s] Failed! Invalid mask\n", __func__);
		goto ERR;
	}

	value[0] = (macaddr >> 8) & get_one_bits(0, 31);
	value[1] = (macaddr >> 40) & get_one_bits(0, 7);
	value[1] += mask << 8;

	address = L2_RESERVED_MULTICAST_ADDRESS_BASE;

	//3. config
	for (i = 0; i < L2_RESERVED_MULTICAST_ADDRESS_BASE_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] base addr 0x%lx value 0x[%08x %08x]\n", __func__, address, value[1], value[0]);

	/* action */
	index = macaddr & 0xFF;

	address = L2_RESERVED_MULTICAST_ADDRESS_ACTION + index * L2_RESERVED_MULTICAST_ADDRESS_ACTION_ADDR_PER_ENTRY;
	// 1. get old value
	for (i = 0; i < L2_RESERVED_MULTICAST_ADDRESS_ACTION_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &action_value[i]);

	// 2. update
	if (!switch_field_setup(argc - 1, argv + 1, switch_l2_reserved_multi_addr_action, ARRAY_SIZE(switch_l2_reserved_multi_addr_action), action_value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//3. config
	for (i = 0; i < L2_RESERVED_MULTICAST_ADDRESS_ACTION_ADDR_PER_ENTRY; i++)
		switch_write(address + i, action_value[i]);

	printf("[%s] action addr 0x%lx value 0x%08x\n", __func__, address, action_value[0]);

	return 1;

ERR:
	switch_l2_reserved_multi_addr_usage();
	return 0;
}


struct cmd_module switch_mac_range_cmd[] = {
	{"dest", switch_reserved_dmac_addr_range_config, switch_reserved_dmac_addr_range_usage},
	{"source", switch_reserved_smac_addr_range_config, switch_reserved_smac_addr_range_usage},
	{"l2_multi", switch_l2_reserved_multi_addr_config, switch_l2_reserved_multi_addr_usage},
};

void switch_mac_range_usage(void)
{
	printf("Usage: npecmd switch mac_range SUB-FUNCTION {COMMAND}\n");
	printf("where  SUB-FUNCTION :=");
	for (int i = 0; i < ARRAY_SIZE(switch_mac_range_cmd); i++)
		printf(" %s", switch_mac_range_cmd[i].name);

	printf("\n");
}

int switch_mac_range_config(int argc, char **argv)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(switch_mac_range_cmd); i++) {
		if (!strcasecmp(argv[0], switch_mac_range_cmd[i].name)) {
			if (argc <= 1) {
				if (switch_mac_range_cmd[i].usage)
					switch_mac_range_cmd[i].usage();

				return 0;
			}

			if (switch_mac_range_cmd[i].func)
				return switch_mac_range_cmd[i].func(argc - 1, argv + 1);
		}
	}

	switch_mac_range_usage();
	return 0;
}
