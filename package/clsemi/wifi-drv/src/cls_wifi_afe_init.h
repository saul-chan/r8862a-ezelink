#ifndef __AFE_INIT_H__
#define __AFE_INIT_H__

#include "cls_wifi_platform.h"
#include "cls_wifi_soc.h"

//###################SRC and PLL TOP REG START#######
#define IO_LEFT_SRC_PARA_BASE		   0x90422000
#define IO_LEFT_SRC_2G_PLL_BASE		 0x90424000
#define IO_LEFT_SRC_5G_PLL_BASE		 0x90424800
#define IO_LEFT_SRC_SA_PLL_BASE		 0x90425000
#define IO_LEFT_SRC_PARA_SIZE		   0x2000
#define IO_LEFT_SRC_2G_PLL_SIZE		 0x800
#define IO_LEFT_SRC_5G_PLL_SIZE		 0x800
#define IO_LEFT_SRC_SA_PLL_SIZE		 0x1000

#define AFE_MAX_RUN_COUNT	10
#define AFE_TUNE_RUN_TIME	10
struct irf_reg_cfg {
	uint32_t addr;
	uint32_t value;
};

static inline void cls_wifi_irf_reg_write(void __iomem *base, uint32_t reg_addr, uint32_t val)
{
	CLS_REG_RAW_WRITE32(base, reg_addr, val);
}

static inline uint32_t cls_wifi_irf_reg_read(void __iomem *base, uint32_t reg_addr)
{
	return CLS_REG_RAW_READ32(base, reg_addr);
}

extern int irf_init_afe(void);
extern int irf_start_afe(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index);
#endif
