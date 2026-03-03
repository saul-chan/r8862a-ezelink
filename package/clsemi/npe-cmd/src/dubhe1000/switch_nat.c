// SPDX-License-Identifier: GPL-2.0
// Copyright(c) 2023 Clourneysemi Corporation.

#include "npe_switch.h"
#include "switch_conf.h"
#include "switch_nat.h"
#include <arpa/inet.h>

struct switch_field_def switch_nat_ingress_op[] = {
	{INGRESS_NAT_ACTION_ARG_REPLACESRC, "replaceSrc", 1, 0},
	{INGRESS_NAT_ACTION_ARG_REPLACEIP, "replaceIP", 1, 1},
	{INGRESS_NAT_ACTION_ARG_REPLACEL4PORT, "replaceL4Port", 1, 2},
	{INGRESS_NAT_ACTION_ARG_IPADDRESS, "ipAddress", 32, 3, SWITCH_FIELD_TYPE_IPV4},
	{INGRESS_NAT_ACTION_ARG_PORT, "port", 16, 35},
};

void switch_nat_ingress_op_usage(void)
{
	printf("Usage: npecmd switch nat ingress_op <index> <--arg=value>\n");
	printf("maximum index: %d\n", INGRESS_NAT_OPERATION_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_nat_ingress_op); i++)
		printf(" %s", switch_nat_ingress_op[i].name);

	printf("\n");
}

int switch_nat_ingress_op_config(int argc, char **argv)
{
	u64 address;
	u32 value[INGRESS_NAT_OPERATION_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= INGRESS_NAT_OPERATION_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = INGRESS_NAT_OPERATION + index * INGRESS_NAT_OPERATION_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < INGRESS_NAT_OPERATION_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_nat_ingress_op, ARRAY_SIZE(switch_nat_ingress_op), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < INGRESS_NAT_OPERATION_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x]\n", __func__, index, address, value[1], value[0]);

	return 1;

ERR:
	switch_nat_ingress_op_usage();
	return 0;
}

struct switch_field_def switch_nat_egress_op[] = {
	{EGRESS_NAT_ACTION_ARG_REPLACESRC, "replaceSrc", 1, 0},
	{EGRESS_NAT_ACTION_ARG_REPLACEIP, "replaceIP", 1, 1},
	{EGRESS_NAT_ACTION_ARG_REPLACEL4PORT, "replaceL4Port", 1, 2},
	{EGRESS_NAT_ACTION_ARG_IPADDRESS, "ipAddress", 32, 3, SWITCH_FIELD_TYPE_IPV4},
	{EGRESS_NAT_ACTION_ARG_PORT, "port", 16, 35},
};

void switch_nat_egress_op_usage(void)
{
	printf("Usage: npecmd switch nat egress_op <index> <--arg=value>\n");
	printf("maximum index: %d\n", EGRESS_NAT_OPERATION_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_nat_egress_op); i++)
		printf(" %s", switch_nat_egress_op[i].name);

	printf("\n");
}

int switch_nat_egress_op_config(int argc, char **argv)
{
	u64 address;
	u32 value[EGRESS_NAT_OPERATION_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= EGRESS_NAT_OPERATION_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = EGRESS_NAT_OPERATION + index * EGRESS_NAT_OPERATION_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < EGRESS_NAT_OPERATION_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_nat_egress_op, ARRAY_SIZE(switch_nat_egress_op), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < EGRESS_NAT_OPERATION_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x[%08x %08x]\n", __func__, index, address, value[1], value[0]);

	return 1;

ERR:
	switch_nat_egress_op_usage();
	return 0;
}

struct switch_field_def switch_nat_eport_state[] = {
	{EGRESS_PORT_NAT_STATE_ARG_PORTSTATE, "portState", 6, 0},
};

void switch_nat_eport_state_usage(void)
{
	printf("Usage: npecmd switch nat eport_state <index> <--arg=value>\n");
	printf("Usage: npecmd switch nat eport_state <index> show\n");
	printf("maximum index: %d\n", EGRESS_PORT_NAT_STATE_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_nat_eport_state); i++)
		printf(" %s", switch_nat_eport_state[i].name);

	printf("\n");
}

int switch_nat_eport_state_config(int argc, char **argv)
{
	u64 address;
	u32 value[EGRESS_PORT_NAT_STATE_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= EGRESS_PORT_NAT_STATE_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = EGRESS_PORT_NAT_STATE + index * EGRESS_PORT_NAT_STATE_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < EGRESS_PORT_NAT_STATE_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	if (strcasecmp(argv[1], "show")) {
		//update
		if (!switch_field_setup(argc, argv, switch_nat_eport_state, ARRAY_SIZE(switch_nat_eport_state), value)) {
			printf("[%s] Failed! Invalid arg list\n", __func__);
			goto ERR;
		}

		//config
		for (i = 0; i < EGRESS_PORT_NAT_STATE_ADDR_PER_ENTRY; i++)
			switch_write(address + i, value[i]);
	}

	printf("[%s] index %d addr 0x%lx value 0x%08x\n", __func__, index, address, value[0]);

	return 1;

ERR:
	switch_nat_eport_state_usage();
	return 0;
}

struct switch_field_def switch_nat_action_table[] = {
	{NAT_ACTION_TABLE_ARG_ACTION, "action", 2, 0},
};

void switch_nat_action_table_usage(void)
{
	printf("Usage: npecmd switch nat action_table <index> <--arg=value>\n");
	printf("maximum index: %d\n", NAT_ACTION_TABLE_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_nat_action_table); i++)
		printf(" %s", switch_nat_action_table[i].name);

	printf("\n");
}

int switch_nat_action_table_config(int argc, char **argv)
{
	u64 address;
	u32 value[NAT_ACTION_TABLE_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= NAT_ACTION_TABLE_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = NAT_ACTION_TABLE + index * NAT_ACTION_TABLE_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < NAT_ACTION_TABLE_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_nat_action_table, ARRAY_SIZE(switch_nat_action_table), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < NAT_ACTION_TABLE_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x%08x\n", __func__, index, address, value[0]);

	return 1;

ERR:
	switch_nat_action_table_usage();
	return 0;
}

struct switch_field_def switch_nat_add_eport_cal[] = {
	{NAT_ADD_EGRESS_PORT_FOR_NAT_CAL_ARG_DONTADDINGRESS, "dontAddIngress", 1, 0},
	{NAT_ADD_EGRESS_PORT_FOR_NAT_CAL_ARG_DONTADDEGRESS, "dontAddEgress", 1, 1},
};

void switch_nat_add_eport_cal_usage(void)
{
	printf("Usage: npecmd switch nat add_eport_cal <index> <--arg=value>\n");
	printf("maximum index: %d\n", NAT_ADD_EGRESS_PORT_FOR_NAT_CALCULATION_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_nat_add_eport_cal); i++)
		printf(" %s", switch_nat_add_eport_cal[i].name);

	printf("\n");
}

int switch_nat_add_eport_cal_config(int argc, char **argv)
{
	u64 address;
	u32 value[NAT_ADD_EGRESS_PORT_FOR_NAT_CALCULATION_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= NAT_ADD_EGRESS_PORT_FOR_NAT_CALCULATION_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = NAT_ADD_EGRESS_PORT_FOR_NAT_CALCULATION + index * NAT_ADD_EGRESS_PORT_FOR_NAT_CALCULATION_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < NAT_ADD_EGRESS_PORT_FOR_NAT_CALCULATION_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_nat_add_eport_cal, ARRAY_SIZE(switch_nat_add_eport_cal), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < NAT_ADD_EGRESS_PORT_FOR_NAT_CALCULATION_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] index %d addr 0x%lx value 0x%08x\n", __func__, index, address, value[0]);

	return 1;

ERR:
	switch_nat_add_eport_cal_usage();
	return 0;
}


struct cmd_module switch_nat_cmd[] = {
	{"ingress_op", switch_nat_ingress_op_config, switch_nat_ingress_op_usage},
	{"egress_op", switch_nat_egress_op_config, switch_nat_egress_op_usage},
	{"eport_state", switch_nat_eport_state_config, switch_nat_eport_state_usage},
	{"action_table", switch_nat_action_table_config, switch_nat_action_table_usage},
	{"add_eport_cal", switch_nat_add_eport_cal_config, switch_nat_add_eport_cal_usage},
};

void switch_nat_usage(void)
{
	printf("Usage: npecmd switch nat SUB-FUNCTION {COMMAND}\n");
	printf("where  SUB-FUNCTION :=");
	for (int i = 0; i < ARRAY_SIZE(switch_nat_cmd); i++)
		printf(" %s", switch_nat_cmd[i].name);

	printf("\n");
}

int switch_nat_config(int argc, char **argv)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(switch_nat_cmd); i++) {
		if (!strcasecmp(argv[0], switch_nat_cmd[i].name)) {
			if (argc <= 1) {
				if (switch_nat_cmd[i].usage)
					switch_nat_cmd[i].usage();

				return 0;
			}

			if (switch_nat_cmd[i].func)
				return switch_nat_cmd[i].func(argc - 1, argv + 1);
		}
	}

	switch_nat_usage();
	return 0;
}
