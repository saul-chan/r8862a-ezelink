/*
 *  Copyright (c) 2021-2024, Clourney Semiconductor. All rights reserved.
 *
 *  This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
 *  Reproduction and redistribution in binary or source form, with or without modification,
 *  for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
 *  must retain the above copyright notice.
 *
 *  By using this software and/or documentation, you agree to the limited terms and conditions.
 */

#include <common.h>
#include <asm/global_data.h>
#include <fdtdec.h>
#include <asm/armv8/mmu.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

static struct mm_region dubhe2000_rdb_mem_map[] = {
	{
		.virt = 0x00000000UL,
		.phys = 0x00000000UL,
		.size = 0x40000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			PTE_BLOCK_INNER_SHARE
	},
	{
		.virt = 0x40000000UL,
		.phys = 0x40000000UL,
		.size = 0x20000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	{
		.virt = 0x60000000UL,
		.phys = 0x60000000UL,
		.size = 0x2000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			PTE_BLOCK_INNER_SHARE
	},
	{
		.virt = 0x62000000UL,
		.phys = 0x62000000UL,
		.size = 0x8000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	{
		.virt = 0x80000000UL,
		.phys = 0x80000000UL,
		.size = 0xe000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	{
		.virt = 0x90000000UL,
		.phys = 0x90000000UL,
		.size = 0x800000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	{
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = dubhe2000_rdb_mem_map;

int timer_init(void)
{
	writel(1, (u32 *)COUNTER_DUBHE_TIMER_ENABLE_ADDR);
	return 0;
}

void reset_cpu(void)
{
	//reset_cpu need to confirm with SoC
	writel(DUBHE2000_GLOBAL_SWRST_EN    ,(u32 *)DUBHE2000_GLOBAL_SWRST_ADDR);
	writel(DUBHE2000_GLOBAL_SWRST_START ,(u32 *)DUBHE2000_GLOBAL_SWRST_ADDR);
	while (1) {
		/*
		 * spin for .5 seconds before reset
		 */
	}
}

int dram_init(void)
{
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;
	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = CONFIG_SYS_SDRAM_SIZE;

	return 0;
}

int board_init(void)
{
	return 0;
}

#if defined(CONFIG_CLS_WATCHDOG)
void hw_watchdog_reset(void)
{
	writel(DUBHE2000_HW_WATCHDOG_VAL, (u32 *)DUBHE2000_HW_WATCHDOG_ADDR);
}

void hw_watchdog_init(void)
{
	writel(0x1, (u32 *)DUBHE2000_HW_WDT_STAR_ADDR);
}
#endif

