// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <common.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <asm/spin_table.h>
#include <asm/global_data.h>
#include <asm/io.h>
DECLARE_GLOBAL_DATA_PTR;
#include <asm/arch/config.h>

int spin_table_update_dt(void *fdt)
{
	int cpus_offset, offset, off_prev = -1;
	const char *prop;
	fdt32_t *reg;
	u64 core_id;
	int ret, addr_cells, unlock_done=0;
	unsigned long rsv_addr = (unsigned long)&spin_table_reserve_begin;
	unsigned long rsv_size = &spin_table_reserve_end -
						&spin_table_reserve_begin;
	unsigned int core_rst_val = 0;

	cpus_offset = fdt_path_offset(fdt, "/cpus");
	if (cpus_offset < 0)
		return -ENODEV;

	for (offset = fdt_first_subnode(fdt, cpus_offset);
	     offset >= 0;
	     offset = fdt_next_subnode(fdt, offset)) {
		prop = fdt_getprop(fdt, offset, "device_type", NULL);
		if (!prop || strcmp(prop, "cpu"))
			continue;

		/*
		 * In the first loop, we check if every CPU node specifies
		 * spin-table.  Otherwise, just return successfully to not
		 * disturb other methods, like psci.
		 */
		prop = fdt_getprop(fdt, offset, "enable-method", NULL);
		if (!prop || strcmp(prop, "spin-table"))
			return 0;
	}

	fdt_support_default_count_cells(fdt, cpus_offset, &addr_cells, NULL);

        offset = fdt_node_offset_by_prop_value(fdt, off_prev, "device_type",
                                            "cpu", 4);
        while (offset != -FDT_ERR_NOTFOUND) {
                reg = (fdt32_t *)fdt_getprop(fdt, offset, "reg", 0);
                if (reg) {
                        core_id = fdt_read_number(reg, addr_cells);

			if ((core_id != CORE_0) && (unlock_done == 0)) {
				writel(CPU_UNLOCK_VAL,CPU_LSYS_ADDR);
				unlock_done = 1;
			}
			//power on cores and set release address @ gd->relocaddr
			if (core_id > CORE_0) {
				printf("Waking secondary core%lld to start from %lx\n", core_id, gd->relocaddr);
				writel(gd->relocaddr, COREn_RELEASE_ADDR_REG + core_id * 4);
				core_rst_val = readl(COREn_RST_REG);
				writel( (core_rst_val | 1 << (core_id + CORE_OFFSET)), COREn_RST_REG );
			}
                }
                off_prev = offset;
                offset = fdt_node_offset_by_prop_value(fdt, off_prev,
                                                    "device_type", "cpu", 4);
        }

	for (offset = fdt_first_subnode(fdt, cpus_offset);
	     offset >= 0;
	     offset = fdt_next_subnode(fdt, offset)) {
		prop = fdt_getprop(fdt, offset, "device_type", NULL);
		if (!prop || strcmp(prop, "cpu"))
			continue;

		ret = fdt_setprop_u64(fdt, offset, "cpu-release-addr",
				(unsigned long)&spin_table_cpu_release_addr);
		if (ret)
			return -ENOSPC;
	}

	ret = fdt_add_mem_rsv(fdt, rsv_addr, rsv_size);
	if (ret)
		return -ENOSPC;

	printf("   Reserved memory region for spin-table: addr=%lx size=%lx\n",
	       rsv_addr, rsv_size);

	return 0;
}
