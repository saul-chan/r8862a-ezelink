/*
 * Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */


#include "clsapi_base.h"
#include "clsapi_cli.h"
#include "autogened_cli_base.h"


static struct clsapi_cli_entry clsapi_cli_entry_base_manual[] = {
	{}
};


int clsapi_cli_base_init(struct clsapi_cli_entry *cli_entry_tbl[])
{
	cli_entry_tbl[CLSAPI_CLI_ENTRY_TABLE_BASE_AUTO] = clsapi_cli_entry_base_auto;
	cli_entry_tbl[CLSAPI_CLI_ENTRY_TABLE_BASE_MANUAL] = clsapi_cli_entry_base_manual;

	return CLSAPI_OK;
}


struct clsapi_cli_plugin clsapi_cli_plugin = {
	.init = clsapi_cli_base_init
};

