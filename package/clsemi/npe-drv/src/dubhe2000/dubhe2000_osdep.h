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

/* glue for the OS independent part of dubhe1000
 * includes register access macros
 */

#ifndef _DUBHE1000_OSDEP_H_
#define _DUBHE1000_OSDEP_H_

#include <asm/io.h>

#define er64(reg)		(readq(conf_base_addr + (DUBHE1000_##reg)))
#define ew64(reg, value)	(writeq((value), (conf_base_addr + (DUBHE1000_##reg))))
#define er32(reg)		(readl(conf_base_addr + (DUBHE1000_##reg)))
#define ew32(reg, value)	(writel((value), (conf_base_addr + (DUBHE1000_##reg))))

#define er32_x(cpu_id, reg)		(readl(conf_base_addr + (DUBHE1000_##reg##_X(cpu_id))))
#define ew32_x(cpu_id, reg, value)	(writel((value), (conf_base_addr + (DUBHE1000_##reg##_X(cpu_id)))))
#define er64_x(cpu_id, reg)		(readq(conf_base_addr + (DUBHE1000_##reg##_X(cpu_id))))
#define ew64_x(cpu_id, reg, value)	(writeq((value), (conf_base_addr + (DUBHE1000_##reg##_X(cpu_id)))))

#define edma_debug_r32(reg)		(readl(debug_base_addr + (DUBHE1000_##reg)))
#define edma_debug_w32(reg, value)	(writel((value), (debug_base_addr + (DUBHE1000_##reg))))

#define DUBHE1000_WRITE_REG_ARRAY(reg, offset, value)                                                                 \
	(writel((value), (conf_base_addr + (DUBHE1000_##reg) + (offset))))

#define DUBHE1000_READ_REG_ARRAY(reg, offset) (readl(conf_base_addr + (DUBHE1000_##reg) + (offset)))

#define DUBHE1000_READ_REG_ARRAY_DWORD	DUBHE1000_READ_REG_ARRAY
#define DUBHE1000_WRITE_REG_ARRAY_DWORD DUBHE1000_WRITE_REG_ARRAY

#define DUBHE1000_WRITE_REG_ARRAY_WORD(reg, offset, value)                                                             \
	(writew((value), (conf_base_addr + (DUBHE1000_##reg) + (offset))))

#define DUBHE1000_READ_REG_ARRAY_WORD(reg, offset) (readw(conf_base_addr + (DUBHE1000_##reg) + (offset)))

#define DUBHE1000_WRITE_REG_ARRAY_BYTE(reg, offset, value)                                                             \
	(writeb((value), (conf_base_addr + (DUBHE1000_##reg) + (offset))))

#define DUBHE1000_READ_REG_ARRAY_BYTE(reg, offset) (readb(conf_base_addr + (DUBHE1000_##reg) + (offset)))

#define DUBHE1000_WRITE_FLUSH() er32(STATUS)

#define aging_r32(reg) (readl(aging_base_addr + reg))

#define aging_w32(reg, value) (writel((value), (aging_base_addr + reg)))

extern void __iomem *conf_base_addr;
extern void __iomem *debug_base_addr;

extern void __iomem *switch_base_addr;

extern void __iomem *xpcs0_base_addr;
extern void __iomem *xpcs1_base_addr;

extern void __iomem *aging_base_addr;

#endif /* _DUBHE1000_OSDEP_H_ */
