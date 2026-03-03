// SPDX-License-Identifier: GPL-2.0
// Copyright(c) 2023 Clourneysemi Corporation.

#include "npe_switch.h"
#include "switch_conf.h"
#include "switch_vlan.h"
#include "switch_generic.h"
#include <arpa/inet.h>

struct switch_field_def switch_source_port_table[] = {
	{SOURCE_PORT_TABLE_ARG_LEARNINGEN, "learningEn", 1, 0},
	{SOURCE_PORT_TABLE_ARG_DROPUNKNOWNDA, "dropUnknownDa", 1, 1},
	{SOURCE_PORT_TABLE_ARG_PRIOFROML3, "prioFromL3", 1, 2},
	{SOURCE_PORT_TABLE_ARG_COLORFROML3, "colorFromL3", 1, 3},
	{SOURCE_PORT_TABLE_ARG_USEACL0, "useAcl0", 1, 4},
	{SOURCE_PORT_TABLE_ARG_ACLRULE0, "aclRule0", 3, 5},
	{SOURCE_PORT_TABLE_ARG_USEACL1, "useAcl1", 1, 8},
	{SOURCE_PORT_TABLE_ARG_ACLRULE1, "aclRule1", 3, 9},
	{SOURCE_PORT_TABLE_ARG_USEACL2, "useAcl2", 1, 12},
	{SOURCE_PORT_TABLE_ARG_ACLRULE2, "aclRule2", 3, 13},
	{SOURCE_PORT_TABLE_ARG_USEACL3, "useAcl3", 1, 16},
	{SOURCE_PORT_TABLE_ARG_ACLRULE3, "aclRule3", 3, 17},
	{SOURCE_PORT_TABLE_ARG_VLANSINGLEOP, "vlanSingleOp", 3, 20},
	{SOURCE_PORT_TABLE_ARG_VIDSEL, "vidSel", 2, 23},
	{SOURCE_PORT_TABLE_ARG_CFIDEISEL, "cfiDeiSel", 2, 25},
	{SOURCE_PORT_TABLE_ARG_PCPSEL, "pcpSel", 2, 27},
	{SOURCE_PORT_TABLE_ARG_NRVLANSVIDOPERATIONIF, "nrVlansVidOperationIf", 2, 29},
	{SOURCE_PORT_TABLE_ARG_VLANSINGLEOPIF, "vlanSingleOpIf", 3, 31},
	{SOURCE_PORT_TABLE_ARG_VIDSELIF, "vidSelIf", 2, 34},
	{SOURCE_PORT_TABLE_ARG_CFIDEISELIF, "cfiDeiSelIf", 2, 36},
	{SOURCE_PORT_TABLE_ARG_PCPSELIF, "pcpSelIf", 2, 38},
	{SOURCE_PORT_TABLE_ARG_TYPESELIF, "typeSelIf", 2, 40},
	{SOURCE_PORT_TABLE_ARG_DEFAULTVIDIF, "defaultVidIf", 12, 42},
	{SOURCE_PORT_TABLE_ARG_DEFAULTCFIDEIIF, "defaultCfiDeiIf", 1, 54},
	{SOURCE_PORT_TABLE_ARG_DEFAULTPCPIF, "defaultPcpIf", 3, 55},
	{SOURCE_PORT_TABLE_ARG_TYPESEL, "typeSel", 2, 58},
	{SOURCE_PORT_TABLE_ARG_VLANASSIGNMENT, "vlanAssignment", 2, 60},
	{SOURCE_PORT_TABLE_ARG_DEFAULTVID, "defaultVid", 12, 62},
	{SOURCE_PORT_TABLE_ARG_DEFAULTCFIDEI, "defaultCfiDei", 1, 74},
	{SOURCE_PORT_TABLE_ARG_DEFAULTPCP, "defaultPcp", 3, 75},
	{SOURCE_PORT_TABLE_ARG_DEFAULTVIDORDER, "defaultVidOrder", 2, 78},
	{SOURCE_PORT_TABLE_ARG_MINALLOWEDVLANS, "minAllowedVlans", 2, 80},
	{SOURCE_PORT_TABLE_ARG_MAXALLOWEDVLANS, "maxAllowedVlans", 2, 82},
	{SOURCE_PORT_TABLE_ARG_IGNOREVLANMEMBERSHIP, "ignoreVlanMembership", 1, 84},
	{SOURCE_PORT_TABLE_ARG_LEARNMULTICASTSAMAC, "learnMulticastSaMac", 1, 85},
	{SOURCE_PORT_TABLE_ARG_INPUTMIRRORENABLED, "inputMirrorEnabled", 1, 86},
	{SOURCE_PORT_TABLE_ARG_IMUNDERVLANMEMBERSHIP, "imUnderVlanMembership", 1, 87},
	{SOURCE_PORT_TABLE_ARG_IMUNDERPORTISOLATION, "imUnderPortIsolation", 1, 88},
	{SOURCE_PORT_TABLE_ARG_DESTINPUTMIRROR, "destInputMirror", 3, 89},
	{SOURCE_PORT_TABLE_ARG_SPT, "spt", 3, 92},
	{SOURCE_PORT_TABLE_ARG_ENABLEPRIORITYTAG, "enablePriorityTag", 1, 95},
	{SOURCE_PORT_TABLE_ARG_PRIORITYVID, "priorityVid", 12, 96},
	{SOURCE_PORT_TABLE_ARG_ENABLEFROMCPUTAG, "enableFromCpuTag", 1, 108},
	{SOURCE_PORT_TABLE_ARG_DISABLETUNNELEXIT, "disableTunnelExit", 1, 109},
	{SOURCE_PORT_TABLE_ARG_FIRSTHITSECONDMISSSENDTOCPU, "firstHitSecondMissSendToCpu", 1, 110},
	{SOURCE_PORT_TABLE_ARG_DISABLEROUTING, "disableRouting", 1, 111},
	{SOURCE_PORT_TABLE_ARG_NATACTIONTABLEENABLE, "natActionTableEnable", 1, 112},
	{SOURCE_PORT_TABLE_ARG_NATPORTSTATE, "natPortState", 1, 113},
	{SOURCE_PORT_TABLE_ARG_ENABLEL2ACTIONTABLE, "enableL2ActionTable", 1, 114},
	{SOURCE_PORT_TABLE_ARG_L2ACTIONTABLEPORTSTATE, "l2ActionTablePortState", 1, 115},
	{SOURCE_PORT_TABLE_ARG_ENABLEDEFAULTPORTACL, "enableDefaultPortAcl", 1, 116},
	{SOURCE_PORT_TABLE_ARG_FORCEPORTACLACTION, "forcePortAclAction", 1, 117},
	{SOURCE_PORT_TABLE_ARG_PRELOOKUPACLBITS, "preLookupAclBits", 2, 118},
};

void switch_generic_source_port_usage(void)
{
	printf("Usage: npecmd switch generic source_port <index> <--arg=value>\n");
	printf("index range: [0 - %d]\n", SOURCE_PORT_TABLE_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_source_port_table); i++)
		printf(" %s", switch_source_port_table[i].name);

	printf("\n");
}

int switch_generic_source_port_config(int argc, char **argv)
{
	u64 address;
	u32 value[SOURCE_PORT_TABLE_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= SOURCE_PORT_TABLE_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = SOURCE_PORT_TABLE + index * SOURCE_PORT_TABLE_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < SOURCE_PORT_TABLE_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_source_port_table, ARRAY_SIZE(switch_source_port_table), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < SOURCE_PORT_TABLE_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x %08x %08x]\n", __func__, index, address, value[3], value[2], value[1], value[0]);

	return 1;
ERR:
	switch_generic_source_port_usage();
	return 0;
}

struct switch_field_def switch_egress_port_configuration[] = {
	{EGRESS_PORT_CONFIGURATION_ARG_COLORREMAP, "colorRemap", 1, 0},
	{EGRESS_PORT_CONFIGURATION_ARG_VLANSINGLEOP, "vlanSingleOp", 3, 1},
	{EGRESS_PORT_CONFIGURATION_ARG_REMOVESNAP, "removeSNAP", 1, 4},
	{EGRESS_PORT_CONFIGURATION_ARG_TYPESEL, "typeSel", 2, 5},
	{EGRESS_PORT_CONFIGURATION_ARG_VIDSEL, "vidSel", 2, 7},
	{EGRESS_PORT_CONFIGURATION_ARG_CFIDEISEL, "cfiDeiSel", 2, 9},
	{EGRESS_PORT_CONFIGURATION_ARG_PCPSEL, "pcpSel", 2, 11},
	{EGRESS_PORT_CONFIGURATION_ARG_VID, "vid", 12, 13},
	{EGRESS_PORT_CONFIGURATION_ARG_CFIDEI, "cfiDei", 1, 25},
	{EGRESS_PORT_CONFIGURATION_ARG_PCP, "pcp", 3, 26},
	{EGRESS_PORT_CONFIGURATION_ARG_DISABLED, "disabled", 1, 29},
	{EGRESS_PORT_CONFIGURATION_ARG_DROPCTAGGEDVLANS, "dropCtaggedVlans", 1, 30},
	{EGRESS_PORT_CONFIGURATION_ARG_DROPSTAGGEDVLANS, "dropStaggedVlans", 1, 31},
	{EGRESS_PORT_CONFIGURATION_ARG_MORETHANONEVLANS, "moreThanOneVlans", 1, 32},
	{EGRESS_PORT_CONFIGURATION_ARG_DROPUNTAGGEDVLANS, "dropUntaggedVlans", 1, 33},
	{EGRESS_PORT_CONFIGURATION_ARG_DROPSINGLETAGGEDVLANS, "dropSingleTaggedVlans", 1, 34},
	{EGRESS_PORT_CONFIGURATION_ARG_DROPDUALTAGGEDVLANS, "dropDualTaggedVlans", 1, 35},
	{EGRESS_PORT_CONFIGURATION_ARG_DROPCSTAGGEDVLANS, "dropCStaggedVlans", 1, 36},
	{EGRESS_PORT_CONFIGURATION_ARG_DROPSCTAGGEDVLANS, "dropSCtaggedVlans", 1, 37},
	{EGRESS_PORT_CONFIGURATION_ARG_DROPCCTAGGEDVLANS, "dropCCtaggedVlans", 1, 38},
	{EGRESS_PORT_CONFIGURATION_ARG_DROPSSTAGGEDVLANS, "dropSStaggedVlans", 1, 39},
	{EGRESS_PORT_CONFIGURATION_ARG_USEEGRESSQUEUEREMAPPING, "useEgressQueueRemapping", 1, 40},
};

void switch_generic_egress_port_usage(void)
{
	printf("Usage: npecmd switch generic egress_port <index> <--arg=value>\n");
	printf("index range: [0 - %d]\n", EGRESS_PORT_CONFIGURATION_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_egress_port_configuration); i++)
		printf(" %s", switch_egress_port_configuration[i].name);

	printf("\n");
}

int switch_generic_egress_port_config(int argc, char **argv)
{
	u64 address;
	u32 value[EGRESS_PORT_CONFIGURATION_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= EGRESS_PORT_CONFIGURATION_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = EGRESS_PORT_CONFIGURATION + index * EGRESS_PORT_CONFIGURATION_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < EGRESS_PORT_CONFIGURATION_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_egress_port_configuration, ARRAY_SIZE(switch_egress_port_configuration), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < EGRESS_PORT_CONFIGURATION_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x]\n", __func__, index, address, value[1], value[0]);

	return 1;
ERR:
	switch_generic_egress_port_usage();
	return 0;
}

void switch_generic_version_usage(void)
{
	printf("Usage: npecmd switch generic version <command>\n");
	printf("command list := %s %s\n", SWITCH_GENERIC_VERSION_CMD_SHOW, SWITCH_GENERIC_VERSION_CMD_CHECK);
}

int switch_generic_version_config(int argc, char **argv)
{
	u64 address;
	u32 value[CORE_VERSION_ADDR_PER_ENTRY] = {0};

	if (argc != 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	address = CORE_VERSION;
	switch_read(address, &value[0]);

	if (!strcasecmp(argv[0], SWITCH_GENERIC_VERSION_CMD_SHOW)) {
		printf("[%s] version = 0x%x\n", __func__, value[0]);
	} else if (!strcasecmp(argv[0], SWITCH_GENERIC_VERSION_CMD_CHECK)) {
		if (value[0] == SWITCH_CORE_VERSION) {
			printf("[%s] Version Match!\n", __func__);
		} else {
			printf("[%s] Version Mismatch(0x%x)!\n", __func__, SWITCH_CORE_VERSION);
		}
	} else {
		printf("[%s] Failed! Invalid command\n", __func__);
		goto ERR;
	}

	return 1;
ERR:
	switch_generic_version_usage();
	return 0;
}

/*********************SWITCH GENERIC**********************/

struct cmd_module switch_generic_cmd[] = {
	{"source_port", switch_generic_source_port_config, switch_generic_source_port_usage},
	{"egress_port", switch_generic_egress_port_config, switch_generic_egress_port_usage},
	{"version", switch_generic_version_config, switch_generic_version_usage},
};


void switch_generic_usage(void)
{
	printf("Usage: npecmd switch generic SUB-FUNCTION {COMMAND}\n");
	printf("where  SUB-FUNCTION :=");
	for (int i = 0; i < ARRAY_SIZE(switch_generic_cmd); i++)
		printf(" %s", switch_generic_cmd[i].name);

	printf("\n");
}

int switch_generic_config(int argc, char **argv)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(switch_generic_cmd); i++) {
		if (!strcasecmp(argv[0], switch_generic_cmd[i].name)) {
			if (argc <= 1) {
				if (switch_generic_cmd[i].usage)
					switch_generic_cmd[i].usage();

				return 0;
			}

			if (switch_generic_cmd[i].func)
				return switch_generic_cmd[i].func(argc - 1, argv + 1);
		}
	}

	switch_generic_usage();
	return 0;

}
