// SPDX-License-Identifier: GPL-2.0
// Copyright(c) 2023 Clourneysemi Corporation.

#include "npe.h"
#include "switch_cmd.h"

struct cmd_module npe_cmd[] = {
	{NPE_MODULE_EDMA, npe_edma_config, npe_edma_usage},
//	{NPE_MODULE_EDMA_DBG, npe_edma_dbg_config},
	{NPE_MODULE_SWITCH, npe_switch_config, npe_switch_usage},
//	{NPE_MODULE_MAC, npe_mac_config}
};

void npe_cmd_usgae(void)
{
	printf("Usage: npecmd MODULE FUNCTION [COMMAND]\n");
	printf("where  MODULE := { edma | edma_dbg | switch | mac }\n");
}

int npecmd_set(char *module, u64 address, u32 value, bool is_set)
{
	char cmd[128] = {0};
	FILE *fp = NULL;
	int ret = -1;

	fp = fopen(NPE_DEBUGFS_PATH, "w");
	if (fp == NULL) {
		printf("[%s] Fail to open %s\n", __func__, NPE_DEBUGFS_PATH);
		goto ERR;
	}

	if (is_set)
		snprintf(cmd, sizeof(cmd) - 1, "npecmd_set %s %llu %lu", module, address, value);
	else
		snprintf(cmd, sizeof(cmd) - 1, "npecmd_get %s %llu", module, address);

	if (fwrite(cmd, sizeof(char), strlen(cmd), fp) != strlen(cmd)) {
		printf("[%s] Fail to write command = [%s]\n", __func__, cmd);
		goto ERR;
	}

	ret = 0;
ERR:
	if (fp)
		fclose(fp);

	if (ret)
		exit(0);

	return 0;
}

int npe_write(char *module, u64 address, u32 value)
{
#ifndef LOCAL_TEST
	return npecmd_set(module, address, value, 1);
#else
	return 0;
#endif
}

int npe_read(char *module, u64 address, u32 *value)
{
	char cmd[128] = {0};
	FILE *fp = NULL;
	char buffer[64];
	int ret = -1;

#ifndef LOCAL_TEST
	npecmd_set(module, address, 0, 0);

	fp = fopen(NPE_DEBUGFS_PATH, "r");
	if (fp == NULL) {
		printf("[%s] Fail to open %s\n", __func__, NPE_DEBUGFS_PATH);
		goto ERR;
	}

	if (fgets(buffer, sizeof(buffer), fp) == NULL) {
		printf("[%s] Fail to get Result: address 0x%llx\n", __func__, address);
		goto ERR;
	}

	if ((sscanf(buffer, "Result: %i", value) != 1)) {
		printf("[%s] Invalid address 0x%llx\n", __func__, address);
		goto ERR;
	}

	ret = 0;
ERR:
	if (fp)
		fclose(fp);

	if (ret)
		exit(0);

	return 0;
#else
	*value = 0x0;
	return 0;
#endif
}

int main(int argc, char **argv)
{
	int i, ret;

	if (argc == 1)
		goto CMD_ERROR;

	for (i = 0; i < ARRAY_SIZE(npe_cmd); i++) {
		if (!strcasecmp(argv[1], npe_cmd[i].name)) {
			if (argc == 2) {// invlaid Format - Without FUNCTION: npecmd + MODULE
				if (npe_cmd[i].usage)
					npe_cmd[i].usage();

				return 0;
			}

			if (npe_cmd[i].func) {
				ret = npe_cmd[i].func(argc - 2, argv + 2);

				if (ret)
					printf("====NPECMD RUN SUCCESS====\n");
				else
					printf("====NPECMD RUN UNSUCCESS===\n");

				return ret;
			}

			break;
		}
	}

CMD_ERROR:
	npe_cmd_usgae();
	return 0;
}
