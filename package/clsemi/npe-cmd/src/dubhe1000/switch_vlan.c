// SPDX-License-Identifier: GPL-2.0
// Copyright(c) 2023 Clourneysemi Corporation.

#include "npe_switch.h"
#include "switch_conf.h"
#include "switch_vlan.h"
#include <arpa/inet.h>

struct switch_field_def switch_vlan_table[] = {
	{VLAN_TABLE_ARG_VLANPORTMASK, "vlanPortMask", 6, 0},
	{VLAN_TABLE_ARG_GID, "gid", 12, 6},
	{VLAN_TABLE_ARG_MMPVALID, "mmpValid", 1, 18},
	{VLAN_TABLE_ARG_MMPPTR, "mmpPtr", 5, 19},
	{VLAN_TABLE_ARG_MMPORDER, "mmpOrder", 2, 24},
	{VLAN_TABLE_ARG_MSPTPTR, "msptPtr", 4, 26},
	{VLAN_TABLE_ARG_VLANSINGLEOP, "vlanSingleOp", 3, 30},
	{VLAN_TABLE_ARG_VIDSEL, "vidSel", 2, 33},
	{VLAN_TABLE_ARG_CFIDEISEL, "cfiDeiSel", 2, 35},
	{VLAN_TABLE_ARG_PCPSEL, "pcpSel", 2, 37},
	{VLAN_TABLE_ARG_NRVLANSVIDOPERATIONIF, "nrVlansVidOperationIf", 12, 39},
	{VLAN_TABLE_ARG_VLANSINGLEOPIF, "vlanSingleOpIf", 3, 51},
	{VLAN_TABLE_ARG_VIDSELIF, "vidSelIf", 2, 54},
	{VLAN_TABLE_ARG_CFIDEISELIF, "cfiDeiSelIf", 2, 56},
	{VLAN_TABLE_ARG_PCPSELIF, "pcpSelIf", 2, 58},
	{VLAN_TABLE_ARG_TYPESELIF, "typeSelIf", 2, 60},
	{VLAN_TABLE_ARG_VIDIF, "vidIf", 12, 62},
	{VLAN_TABLE_ARG_PCPIF, "pcpIf", 3, 74},
	{VLAN_TABLE_ARG_CFIDEIIF, "cfiDeiIf", 1, 77},
	{VLAN_TABLE_ARG_TYPESEL, "typeSel", 2, 78},
	{VLAN_TABLE_ARG_VID, "vid", 12, 80},
	{VLAN_TABLE_ARG_PCP, "pcp", 3, 92},
	{VLAN_TABLE_ARG_CFIDEI, "cfiDei", 1, 95},
	{VLAN_TABLE_ARG_ALLOWROUTING, "allowRouting", 1, 96},
	{VLAN_TABLE_ARG_SENDIPMCTOCPU, "sendIpMcToCpu", 1, 97},
	{VLAN_TABLE_ARG_TUNNELENTRY, "tunnelEntry", 1, 98},
	{VLAN_TABLE_ARG_TUNNELENTRYPTR, "tunnelEntryPtr", 6, 99},
	{VLAN_TABLE_ARG_TUNNELEXIT, "tunnelExit", 1, 105},
	{VLAN_TABLE_ARG_TUNNELEXITPTR, "tunnelExitPtr", 7, 106},
};

void switch_vlan_table_usage(void)
{
	printf("Usage: npecmd switch vlan table <index> <--arg=value>\n");
	printf("index range: [0 - %d]\n", VLAN_TABLE_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_vlan_table); i++)
		printf(" %s", switch_vlan_table[i].name);

	printf("\n");
}

int switch_vlan_table_config(int argc, char **argv)
{
	u64 address;
	u32 value[VLAN_TABLE_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= VLAN_TABLE_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = VLAN_TABLE + index * VLAN_TABLE_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < VLAN_TABLE_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_vlan_table, ARRAY_SIZE(switch_vlan_table), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < VLAN_TABLE_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x %08x %08x]\n", __func__, index, address, value[3], value[2], value[1], value[0]);

	return 1;

ERR:
	switch_vlan_table_usage();
	return 0;
}

void switch_vlan_dump_table_usage(void)
{
	printf("Usage: npecmd switch vlan dump_table <vlan_range>\n");
	printf("vlan_range := [start_vlanid] [end_vlanid]\n");
	printf("vlanid range: [0 - %d]\n", VLAN_TABLE_MAX - 1);
}

int switch_vlan_dump_table_config(int argc, char **argv)
{
	int k = 0, k2 = 0;
	u32 val[VLAN_TABLE_ADDR_PER_ENTRY] = {0};
	u64 address;
	u32 index, start = 0, end = VLAN_TABLE_MAX - 1;
	u8 valid = 0;

	if (argc == 1) {
		if ((sscanf(argv[0], "%i", &index) != 1) || (index >= VLAN_TABLE_MAX)) {
			goto ERR;
		} else {
			start = index;
			end = index;
		}
	} else if (argc == 2) {
		if ((sscanf(argv[0], "%i", &start) != 1) || (start >= VLAN_TABLE_MAX)
				|| (sscanf(argv[1], "%i", &end) != 1) || (end >= VLAN_TABLE_MAX)
				|| (end < start)) {
			goto ERR;
		}
	} else {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	printf("[%s] start=%d end=%d argc=%d\n", __func__, start, end, argc);

	for (k = start; k <= end; k++) {
		valid = 0;
		address = VLAN_TABLE + k * VLAN_TABLE_ADDR_PER_ENTRY;
		for (k2 = 0; k2 < VLAN_TABLE_ADDR_PER_ENTRY; k2++) {
			switch_read(address + k2, &val[k2]);
			if (val[k2] != 0)
				valid = 1;
		}
		if (valid)
			printf("[DUMP] VLAN_TABLE[%d] 0x[%08x %08x %08x %08x]\n",
						k, val[3], val[2], val[1], val[0]);
	}

	return 1;

ERR:
	switch_vlan_dump_table_usage();

	return 0;
}

struct cmd_module switch_vlan_cmd[] = {
	{"table", switch_vlan_table_config, switch_vlan_table_usage},
	{"dump_table", switch_vlan_dump_table_config, switch_vlan_dump_table_usage},
};

void switch_vlan_usage(void)
{
	printf("Usage: npecmd switch vlan SUB-FUNCTION {COMMAND}\n");
	printf("where  SUB-FUNCTION :=");
	for (int i = 0; i < ARRAY_SIZE(switch_vlan_cmd); i++)
		printf(" %s", switch_vlan_cmd[i].name);

	printf("\n");
}

int switch_vlan_config(int argc, char **argv)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(switch_vlan_cmd); i++) {
		if (!strcasecmp(argv[0], switch_vlan_cmd[i].name)) {
			if (argc <= 1) {
				if (switch_vlan_cmd[i].usage)
					switch_vlan_cmd[i].usage();

				return 0;
			}

			if (switch_vlan_cmd[i].func)
				return switch_vlan_cmd[i].func(argc - 1, argv + 1);
		}
	}

	switch_vlan_usage();
	return 0;
}
