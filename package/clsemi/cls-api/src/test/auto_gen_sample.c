/*
 * Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */

#include "clsapi_net.h"

int clsapi_get_net_client_name(const char *mac, char *name, int name_len)
{
	int ret = CLSAPI_OK;
	char *device = "iPhone-13";

	strcpy(name, device);

	return ret;
}
