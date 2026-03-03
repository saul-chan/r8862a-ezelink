/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023 Clourneysemi Co., LTD.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,
 * as published by the Free Software Foundation - version 2.
 */

#ifndef _CLSEMI_HW_FLOW_H
#define _CLSEMI_HW_FLOW_H

#include <linux/types.h>

#define HW_FT_CAP_MAGIC		0x20250120

struct hw_flowtable_cap {
	u32 magic; /* TLV is bloated, use magic to make sure the data is valid */
	u32 hw_ft_size;
};

#endif
