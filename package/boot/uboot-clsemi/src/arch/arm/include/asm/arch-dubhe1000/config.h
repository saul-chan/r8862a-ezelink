/*
 * Copyright (C) 2022 Clourney Semiconductor. All rights reserved.
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef _ASM_ARMV8_CLS_DUBHE1000_CONFIG_H_
#define _ASM_ARMV8_CLS_DUBHE1000_CONFIG_H_

#include <stdint.h>
#include <linux/kconfig.h>

#define CORE_0	0
#define CORE_1  1
#define CORE_2  2
#define CORE_3  3
#define CORE_OFFSET  0

#define CPU_LSYS_ADDR                  0x90400000
#define CPU_UNLOCK_VAL                 0x5A5A5A5A
#define CPU_TOP_SYS_BASE               0x90000000

#define COREn_RST_REG                  (CPU_LSYS_ADDR + 0x14000)
#define COREn_RELEASE_ADDR_REG         (CPU_LSYS_ADDR + 0x38)

/* Generic Interrupt Controller Definitions */
#define GICD_BASE                      0x62800000
#define GICR_BASE                      0x62880000

#endif /* _ASM_ARMV8_FSL_LAYERSCAPE_CONFIG_H_ */
