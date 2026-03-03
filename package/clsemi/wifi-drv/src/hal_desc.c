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

#include <linux/string.h>
#include "hal_desc.h"

const struct cls_wifi_legrate legrates_lut[] = {
	[0]  = { .idx = 0,  .rate = 10 },
	[1]  = { .idx = 1,  .rate = 20 },
	[2]  = { .idx = 2,  .rate = 55 },
	[3]  = { .idx = 3,  .rate = 110 },
	[4]  = { .idx = -1, .rate = 0 },
	[5]  = { .idx = -1, .rate = 0 },
	[6]  = { .idx = -1, .rate = 0 },
	[7]  = { .idx = -1, .rate = 0 },
	[8]  = { .idx = 10, .rate = 480 },
	[9]  = { .idx = 8,  .rate = 240 },
	[10] = { .idx = 6,  .rate = 120 },
	[11] = { .idx = 4,  .rate = 60 },
	[12] = { .idx = 11, .rate = 540 },
	[13] = { .idx = 9,  .rate = 360 },
	[14] = { .idx = 7,  .rate = 180 },
	[15] = { .idx = 5,  .rate = 90 },
};

