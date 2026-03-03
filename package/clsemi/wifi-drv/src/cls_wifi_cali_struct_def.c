/*
 * Copyright (c) 2021-2024, Clourney Semiconductor. All rights reserved.
 *
 * This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
 * Reproduction and redistribution in binary or source form, with or without modification,
 * for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
 * must retain the above copyright notice.
 *
 * By using this software and/or documentation, you agree to the limited terms and conditions.
 */

#include <linux/kernel.h>
#include <linux/kmod.h>
#include <linux/debugfs.h>
#include <linux/string.h>


#define CALI_DEF_CONFIG_IMPL
#include "cali_struct.h"

void cali_set_default_config(struct cali_config_tag *cali_cfg_ptr, enum cali_phy_band phy_band)
{
	if (!cali_cfg_ptr)
		return;

	cali_set_default_config_internal(cali_cfg_ptr, phy_band);
}


