/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2023 Clourneysemi Corporation. */

/* glue for the OS independent part of cls
 * includes register access macros
 */

#ifndef _CLS_NPE_OSDEP_H_
#define _CLS_NPE_OSDEP_H_

#include <asm/io.h>

#define EDMA_CONFIG_BASE         0x4C200000
#define EDMA_DEBUG_BASE          0x4C208000

#define XPCS0_REG_BASE           0x52000000
#define XPCS1_REG_BASE           0x52800000
#define SWITCH_REG_BASE          0x53000000
#define GMAC1_REG_BASE           0x53900000
#define GMAC2_REG_BASE           0x53A00000
#define GMAC3_REG_BASE           0x53B00000

#define XGMAC0_REG_BASE           0x53C00000
#define XGMAC1_REG_BASE           0x53D00000
#define TOP_REG_BASE              0x53E00000

#define EDMA_REG_OFFSET       0x0


#define EDMA_CONFIG_REG_BASE \
	((unsigned int)(EDMA_CONFIG_BASE + EDMA_REG_OFFSET))

#define EDMA_DEBUG_REG_BASE \
		((unsigned int)(EDMA_DEBUG_BASE + EDMA_REG_OFFSET))

#define er64(reg)							\
	(readq(conf_base_addr + (CLS_NPE_##reg)))
#define ew64(reg, value)						\
	(writeq((value), (conf_base_addr + (CLS_NPE_##reg))))

#define er32(reg)							\
	(readl(conf_base_addr + (CLS_NPE_##reg)))

#define ew32(reg, value)						\
	(writel((value), (conf_base_addr + (CLS_NPE_##reg))))

#define edma_debug_r32(reg)							\
	(readl(debug_base_addr + (CLS_NPE_##reg)))

#define edma_debug_w32(reg, value)						\
	(writel((value), (debug_base_addr + (CLS_NPE_##reg))))

#define CLS_NPE_WRITE_REG_ARRAY(reg, offset, value) ( \
    writel((value), (conf_base_addr + (CLS_NPE_##reg) + (offset))))

#define CLS_NPE_READ_REG_ARRAY(reg, offset) ( \
    readl(conf_base_addr + (CLS_NPE_##reg) + (offset)))

#define CLS_NPE_READ_REG_ARRAY_DWORD CLS_NPE_READ_REG_ARRAY
#define CLS_NPE_WRITE_REG_ARRAY_DWORD CLS_NPE_WRITE_REG_ARRAY

#define CLS_NPE_WRITE_REG_ARRAY_WORD(reg, offset, value) ( \
    writew((value), (conf_base_addr + (CLS_NPE_##reg) + (offset))))

#define CLS_NPE_READ_REG_ARRAY_WORD(reg, offset) ( \
    readw(conf_base_addr + (CLS_NPE_##reg) + (offset)))

#define CLS_NPE_WRITE_REG_ARRAY_BYTE(reg, offset, value) ( \
    writeb((value), (conf_base_addr + (CLS_NPE_##reg) + (offset))))

#define CLS_NPE_READ_REG_ARRAY_BYTE(reg, offset) ( \
    readb(conf_base_addr + (CLS_NPE_##reg) + (offset)))

#define CLS_NPE_WRITE_FLUSH() er32(STATUS)

extern void __iomem *conf_base_addr;
extern void __iomem *debug_base_addr;

extern void __iomem *switch_base_addr;

extern void __iomem *xpcs0_base_addr;
extern void __iomem *xpcs1_base_addr;

#endif /* _CLS_NPE_OSDEP_H_ */
