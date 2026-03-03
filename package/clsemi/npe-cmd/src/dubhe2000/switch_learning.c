// SPDX-License-Identifier: GPL-2.0
// Copyright(c) 2023 Clourneysemi Corporation.

#include "npe_switch.h"
#include "switch_conf.h"
#include "switch_learning.h"

struct switch_field_def switch_learnig_and_aging_enable[] = {
	{LEARNING_AND_AGING_ARG_LEARNINGENABLE, "learningEnable", 1, 0},
	{LEARNING_AND_AGING_ARG_AGINGENABLE, "agingEnable", 1, 1},
	{LEARNING_AND_AGING_ARG_DAHITENABLE, "daHitEnable", 1, 2},
	{LEARNING_AND_AGING_ARG_LRU, "lru", 1, 3},
};

void switch_learning_and_aging_enable_usage(void)
{
	printf("Usage: npecmd switch learning enable <index> <--arg=value>\n");
	printf("maximum index: %d\n", LEARNING_AND_AGING_ENABLE_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_learnig_and_aging_enable); i++)
		printf(" %s", switch_learnig_and_aging_enable[i].name);

	printf("\n");
}

int switch_learning_and_aging_enable_config(int argc, char **argv)
{
	u64 address;
	u32 value[LEARNING_AND_AGING_ENABLE_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= LEARNING_AND_AGING_ENABLE_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = LEARNING_AND_AGING_ENABLE + index * LEARNING_AND_AGING_ENABLE_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < LEARNING_AND_AGING_ENABLE_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_learnig_and_aging_enable, ARRAY_SIZE(switch_learnig_and_aging_enable), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < LEARNING_AND_AGING_ENABLE_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] addr 0x%lx value 0x%08x\n", __func__, address, value[0]);

	return 1;

ERR:
	switch_learning_and_aging_enable_usage();
	return 0;
}

struct switch_field_def switch_software_aging_enable[] = {
	{SOFTWARE_AGING_ENABLE_ARG_ENABLE, "enable", 1, 0},
};

void switch_software_aging_enable_usage(void)
{
	printf("Usage: npecmd switch learning sw_enable <index> <--arg=value>\n");
	printf("maximum index: %d\n", SOFTWARE_AGING_ENABLE_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_software_aging_enable); i++)
		printf(" %s", switch_software_aging_enable[i].name);

	printf("\n");
}

int switch_software_aging_enable_config(int argc, char **argv)
{
	u64 address;
	u32 value[SOFTWARE_AGING_ENABLE_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= SOFTWARE_AGING_ENABLE_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = SOFTWARE_AGING_ENABLE + index * SOFTWARE_AGING_ENABLE_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < SOFTWARE_AGING_ENABLE_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_software_aging_enable, ARRAY_SIZE(switch_software_aging_enable), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < SOFTWARE_AGING_ENABLE_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] addr 0x%lx value 0x%08x\n", __func__, address, value[0]);

	return 1;

ERR:
	switch_software_aging_enable_usage();
	return 0;
}

struct switch_field_def switch_learning_packet_control[] = {
	{LEARNING_PACKET_CONTROL_ARG_HWLEARNINGWRITEBACK, "hwLearningWriteBack", 1, 0},
	{LEARNING_PACKET_CONTROL_ARG_HWAGINGWRITEBACK, "hwAgingWriteBack", 1, 1},
	{LEARNING_PACKET_CONTROL_ARG_HWHITWRITEBACK, "hwHitWriteBack", 1, 2},
};

void switch_learning_packet_control_usage(void)
{
	printf("Usage: npecmd switch learning packet_control <index> <--arg=value>\n");
	printf("maximum index: %d\n", LEARNING_PACKET_CONTROL_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_learning_packet_control); i++)
		printf(" %s", switch_learning_packet_control[i].name);

	printf("\n");
}

int switch_learning_packet_control_config(int argc, char **argv)
{
	u64 address;
	u32 value[LEARNING_PACKET_CONTROL_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= LEARNING_PACKET_CONTROL_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = LEARNING_AND_AGING_ENABLE + index * LEARNING_PACKET_CONTROL_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < LEARNING_PACKET_CONTROL_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_learning_packet_control, ARRAY_SIZE(switch_learning_packet_control), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < LEARNING_PACKET_CONTROL_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] addr 0x%lx value 0x%08x\n", __func__, address, value[0]);

	return 1;

ERR:
	switch_learning_packet_control_usage();
	return 0;
}

struct switch_field_def switch_learnig_data_fifo[] = {
	{LEARNING_DATA_FIFO_ARG_MAC, "mac", 48, 0, SWITCH_FIELD_TYPE_MACADDR},
	{LEARNING_DATA_FIFO_ARG_GID, "gid", 16, 48},
	{LEARNING_DATA_FIFO_ARG_DESTADDRESS, "destAddress", 13, 64},
	{LEARNING_DATA_FIFO_ARG_UC, "uc", 1, 77},
	{LEARNING_DATA_FIFO_ARG_PORT_OR_PTR, "port_or_ptr", 6, 78},
	{LEARNING_DATA_FIFO_ARG_DROP, "drop", 1, 84},
	{LEARNING_DATA_FIFO_ARG_L2ACTIONTABLEDASTATUS, "l2ActionTableDaStatus", 1, 85},
	{LEARNING_DATA_FIFO_ARG_L2ACTIONTABLESASTATUS, "l2ActionTableSaStatus", 1, 86},
	{LEARNING_DATA_FIFO_ARG_METADATA, "metaData", 16, 87},
	{LEARNING_DATA_FIFO_ARG_STATUS, "status", 3, 103},
	{LEARNING_DATA_FIFO_ARG_TYPE, "type", 1, 106},
	{LEARNING_DATA_FIFO_ARG_VALID, "valid", 1, 107},
};

void switch_learning_data_fifo_usage(void)
{
	printf("Usage: npecmd switch learning data_fifo <index> <--arg=value>\n");
	printf("maximum index: %d\n", LEARNING_DATA_FIFO_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_learnig_data_fifo); i++)
		printf(" %s", switch_learnig_data_fifo[i].name);

	printf("\n");
}

int switch_learning_data_fifo_config(int argc, char **argv)
{
	u64 address;
	u32 value[LEARNING_DATA_FIFO_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= LEARNING_DATA_FIFO_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = LEARNING_DATA_FIFO + index * LEARNING_DATA_FIFO_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < LEARNING_DATA_FIFO_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_learnig_data_fifo, ARRAY_SIZE(switch_learnig_data_fifo), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < LEARNING_DATA_FIFO_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] addr 0x%lx value 0x[%08x %08x %08x]\n", __func__, address, value[2], value[1], value[0]);

	return 1;

ERR:
	switch_learning_data_fifo_usage();
	return 0;
}


struct switch_field_def switch_learnig_conflict[] = {
	{LEARNING_CONFLICT_ARG_VALID, "valid", 1, 0},
	{LEARNING_CONFLICT_ARG_MACADDR, "macAddr", 48, 1, SWITCH_FIELD_TYPE_MACADDR},
	{LEARNING_CONFLICT_ARG_GID, "gid", 12, 49},
	{LEARNING_CONFLICT_ARG_PORT, "port", 3, 61},
};

void switch_learning_conflict_usage(void)
{
	printf("Usage: npecmd switch learning conflict <index> <--arg=value>\n");
	printf("maximum index: %d\n", LEARNING_CONFLICT_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_learnig_conflict); i++)
		printf(" %s", switch_learnig_conflict[i].name);

	printf("\n");
}

int switch_learning_conflict_config(int argc, char **argv)
{
	u64 address;
	u32 value[LEARNING_CONFLICT_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= LEARNING_CONFLICT_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = LEARNING_CONFLICT + index * LEARNING_CONFLICT_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < LEARNING_CONFLICT_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_learnig_conflict, ARRAY_SIZE(switch_learnig_conflict), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < LEARNING_CONFLICT_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] addr 0x%lx value 0x[%08x %08x]\n", __func__, address, value[1], value[0]);

	return 1;

ERR:
	switch_learning_conflict_usage();
	return 0;
}

struct switch_field_def switch_learnig_overflow[] = {
	{LEARNING_OVERFLOW_ARG_VALID, "valid", 1, 0},
	{LEARNING_OVERFLOW_ARG_MACADDR, "macAddr", 48, 1, SWITCH_FIELD_TYPE_MACADDR},
	{LEARNING_OVERFLOW_ARG_GID, "gid", 12, 49},
	{LEARNING_OVERFLOW_ARG_PORT, "port", 3, 61},
};

void switch_learning_overflow_usage(void)
{
	printf("Usage: npecmd switch learning overflow <index> <--arg=value>\n");
	printf("maximum index: %d\n", LEARNING_OVERFLOW_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_learnig_overflow); i++)
		printf(" %s", switch_learnig_overflow[i].name);

	printf("\n");
}

int switch_learning_overflow_config(int argc, char **argv)
{
	u64 address;
	u32 value[LEARNING_OVERFLOW_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= LEARNING_OVERFLOW_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = LEARNING_OVERFLOW + index * LEARNING_OVERFLOW_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < LEARNING_OVERFLOW_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_learnig_overflow, ARRAY_SIZE(switch_learnig_overflow), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < LEARNING_OVERFLOW_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] addr 0x%lx value 0x[%08x %08x]\n", __func__, address, value[1], value[0]);

	return 1;

ERR:
	switch_learning_overflow_usage();
	return 0;
}

struct switch_field_def switch_learnig_da_mac[] = {
	{LEARNING_DA_MAC_ARG_MAC, "mac", 48, 0, SWITCH_FIELD_TYPE_MACADDR},
	{LEARNING_DA_MAC_ARG_ENABLE, "enable", 1, 48},
};

void switch_learning_da_mac_usage(void)
{
	printf("Usage: npecmd switch learning da_mac <index> <--arg=value>\n");
	printf("maximum index: %d\n", LEARNING_DA_MAC_MAX - 1);
	printf("arg :=");
	for (int i = 0; i < ARRAY_SIZE(switch_learnig_da_mac); i++)
		printf(" %s", switch_learnig_da_mac[i].name);

	printf("\n");
}

int switch_learning_da_mac_config(int argc, char **argv)
{
	u64 address;
	u32 value[LEARNING_DA_MAC_ADDR_PER_ENTRY] = {0}, index;
	int i;

	if (argc <= 1) {
		printf("[%s] Failed! Invalid argc\n", __func__);
		goto ERR;
	}

	//check index
	if ((sscanf(argv[0], "%i", &index) != 1) || (index >= LEARNING_DA_MAC_MAX)) {
		printf("[%s] Failed! Invalid index\n", __func__);
		goto ERR;
	}

	address = LEARNING_DA_MAC + index * LEARNING_DA_MAC_ADDR_PER_ENTRY;

	//get old value
	for (i = 0; i < LEARNING_DA_MAC_ADDR_PER_ENTRY; i++)
		switch_read(address + i, &value[i]);

	//update
	if (!switch_field_setup(argc, argv, switch_learnig_da_mac, ARRAY_SIZE(switch_learnig_da_mac), value)) {
		printf("[%s] Failed! Invalid arg list\n", __func__);
		goto ERR;
	}

	//config
	for (i = 0; i < LEARNING_DA_MAC_ADDR_PER_ENTRY; i++)
		switch_write(address + i, value[i]);

	printf("[%s] addr 0x%lx value 0x[%08x %08x]\n", __func__, address, value[1], value[0]);

	return 1;

ERR:
	switch_learning_da_mac_usage();
	return 0;
}

struct cmd_module switch_learning_cmd[] = {
	{"enable", switch_learning_and_aging_enable_config, switch_learning_and_aging_enable_usage},
	{"sw_aging", switch_software_aging_enable_config, switch_software_aging_enable_usage},
	{"packet_control", switch_learning_packet_control_config, switch_learning_packet_control_usage},
	{"data_fifo", switch_learning_data_fifo_config, switch_learning_data_fifo_usage},
	{"conflict", switch_learning_conflict_config, switch_learning_conflict_usage},
	{"overflow", switch_learning_overflow_config, switch_learning_overflow_usage},
	{"da_mac", switch_learning_da_mac_config, switch_learning_da_mac_usage},
};

void switch_learning_usage(void)
{
	printf("Usage: npecmd switch learning SUB-FUNCTION {COMMAND}\n");
	printf("where  SUB-FUNCTION :=");
	for (int i = 0; i < ARRAY_SIZE(switch_learning_cmd); i++)
		printf(" %s", switch_learning_cmd[i].name);

	printf("\n");
}

int switch_learning_config(int argc, char **argv)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(switch_learning_cmd); i++) {
		if (!strcasecmp(argv[0], switch_learning_cmd[i].name)) {
			if (argc <= 1) {
				if (switch_learning_cmd[i].usage)
					switch_learning_cmd[i].usage();

				return 0;
			}

			if (switch_learning_cmd[i].func)
				return switch_learning_cmd[i].func(argc - 1, argv + 1);
		}
	}

	switch_learning_usage();
	return 0;
}
