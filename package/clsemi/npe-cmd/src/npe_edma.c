// SPDX-License-Identifier: GPL-2.0
// Copyright(c) 2023 Clourneysemi Corporation.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "npe.h"

int edma_enable_config(int argc, char **argv)
{
	printf("[%s] UNSUPPORT\n", __func__);
}

int edma_bus_config(int argc, char **argv)
{
	printf("[%s] UNSUPPORT\n", __func__);
}

int edma_alarm_intr_config(int argc, char **argv)
{
	printf("[%s] UNSUPPORT\n", __func__);
}

int edma_token_intr_config(int argc, char **argv)
{
	printf("[%s] UNSUPPORT\n", __func__);
}

int edma_tx_timeout_config(int argc, char **argv)
{
	printf("[%s] UNSUPPORT\n", __func__);
}

int edma_rx_timeout_config(int argc, char **argv)
{
	printf("[%s] UNSUPPORT\n", __func__);
}

struct cmd_module edma_cmd[] = {
	{"enable", edma_enable_config, NULL},
	{"bus", edma_bus_config, NULL},
	{"alarm_intr", edma_alarm_intr_config, NULL},
	{"token_intr", edma_token_intr_config, NULL},
	{"tx_timeout", edma_tx_timeout_config, NULL},
	{"rx_timeout", edma_rx_timeout_config, NULL}
};

void npe_edma_usage(void)
{
	printf("Usage: npecmd edma FUNCTION COMMAND\n");
	printf("where  FUNCTION := { enable | bus | alarm_intr | token_intr | tx_timeout | rx_timeout}\n");
}

int npe_edma_config(int argc, char **argv)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(edma_cmd); i++) {
		if (!strcasecmp(argv[0], edma_cmd[i].name)) {
			if (argc <= 1) {
				if (edma_cmd[i].usage)
					edma_cmd[i].usage();

				return 0;
			}

			if (edma_cmd[i].func)
				return edma_cmd[i].func(argc - 1, argv + 1);
		}
	}

	npe_edma_usage();
	return 0;
}
