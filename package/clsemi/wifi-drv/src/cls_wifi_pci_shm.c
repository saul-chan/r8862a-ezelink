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

#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/genalloc.h>
#include <asm/cacheflush.h>
#include <asm/cacheflush.h>
#include <linux/dma-mapping.h>
#include <linux/device.h>
#include "cls_wifi_pci_shm.h"
#include "cls_wifi_platform.h"

typedef struct pcie_shm_header {
	uint32_t offset;
	uint32_t size;
} pcie_shm_header_st;

typedef struct pcie_shm_pool_info {
	char *name;
	uint32_t num;
	uint32_t size;
} pcie_shm_pool_info_st;

typedef struct pcie_shm_pool {
	struct device         *dev;
	void                  *virt_base;
	struct gen_pool       *pool;
	pcie_shm_header_st    *header;
	dma_addr_t            dma_addr;
	uint32_t              type;
	uint32_t              size;
	uint16_t              h_size;
	uint16_t              tbl_max;
} pcie_shm_pool_st;

static pcie_shm_pool_info_st pcie_shm_pool_infos[SHM_POOL_IDX_MAX] = {
	PCIE_SHM_POOLS_TABLE
};

void pcie_shm_free(pcie_shm_pool_st *shm_obj, void *ptr, size_t size)
{
	if (!shm_obj || !shm_obj->pool || !ptr)
		return;

	gen_pool_free(shm_obj->pool, (unsigned long)ptr, size);
}
EXPORT_SYMBOL(pcie_shm_free);

unsigned long pcie_shm_virt_to_phys(void *ptr)
{
	return virt_to_phys(ptr);
}
EXPORT_SYMBOL(pcie_shm_virt_to_phys);

void pcie_shmem_write_sync(pcie_shm_pool_st *shm_obj, void *ptr, size_t size)
{
	dma_sync_single_for_device(shm_obj->dev, virt_to_phys(ptr), size, DMA_TO_DEVICE);
}
EXPORT_SYMBOL(pcie_shmem_write_sync);

void pcie_shmem_read_sync(pcie_shm_pool_st *shm_obj, void *ptr, size_t size)
{
	dma_sync_single_for_cpu(shm_obj->dev, virt_to_phys(ptr), size, DMA_FROM_DEVICE);
}
EXPORT_SYMBOL(pcie_shmem_read_sync);

void *pcie_shm_alloc(pcie_shm_pool_st *shm_obj, int tbl_idx, size_t size)
{
	unsigned long addr = 0;

	if (!shm_obj || !shm_obj->pool)
		return NULL;

	size = ALIGN(size, cache_line_size());
	addr = gen_pool_alloc(shm_obj->pool, size);

	if (!addr || !shm_obj->header
			|| shm_obj->tbl_max <= tbl_idx)
		return NULL;

	shm_obj->header[tbl_idx] = (pcie_shm_header_st) {
		.offset = addr - (unsigned long)shm_obj->virt_base,
		.size = size,
	};

	pcie_shmem_write_sync(shm_obj, (void *)addr, size);

	return (void *)addr;
}
EXPORT_SYMBOL(pcie_shm_alloc);

static void devm_shm_pool_release(struct device *dev, void *res)
{
	pcie_shm_pool_st *shm_obj = res;

	if (shm_obj->pool)
		gen_pool_destroy(shm_obj->pool);

	if (shm_obj->virt_base) {
		dma_unmap_single(dev, shm_obj->dma_addr,
				 shm_obj->size, DMA_BIDIRECTIONAL);
		free_pages((unsigned long)shm_obj->virt_base, get_order(shm_obj->size));
	}
}

void pcie_shm_pool_dump(pcie_shm_pool_st *shm_obj)
{
	int idx;
	pcie_shm_header_st *header;

	if (!shm_obj || !shm_obj->header)
		return;

	header = shm_obj->header;
	for (idx = 0; idx < shm_obj->tbl_max; idx++)
		pr_err("idx(%d): offset(%d) size %d\n",
				idx, header[idx].offset, header[idx].size);
}
EXPORT_SYMBOL(pcie_shm_pool_dump);

uint64_t pcie_shm_get_pool_phyinfos(pcie_shm_pool_st *shm_obj, uint32_t *size)
{
	if (!shm_obj)
		return 0;

	if (size)
		*size = shm_obj->size;

	return shm_obj->dma_addr;
}
EXPORT_SYMBOL(pcie_shm_get_pool_phyinfos);

void *pcie_shm_get_pool_addrinfos(pcie_shm_pool_st *shm_obj, uint32_t *size)
{
	if (!shm_obj)
		return 0;

	if (size)
		*size = shm_obj->size;

	return shm_obj->virt_base;
}
EXPORT_SYMBOL(pcie_shm_get_pool_addrinfos);

pcie_shm_pool_st* pcie_shm_pool_create(
		struct cls_wifi_plat *cls_wifi_plat,
		pcie_shm_pool_idx_em type)
{
	void *base = NULL;
	struct gen_pool *pool;
	dma_addr_t dma_addr;
	uint32_t shm_header_sz = 0;
	struct device *dev = cls_wifi_plat->dev;
	pcie_shm_header_st *shm_header = NULL;
	pcie_shm_pool_st *shm_obj = NULL;
	uint32_t pool_sz = PAGE_ALIGN(pcie_shm_pool_infos[type].size);
	int page_n = get_order(pool_sz);
	long res = 0;

	shm_obj = devres_alloc(devm_shm_pool_release,
			sizeof(pcie_shm_pool_st), GFP_KERNEL);
	if (!shm_obj) {
		res = -1;
		goto ERR_DEVRES;
	}

	/* DMA USE GFP_ATOMIC | GFP_DMA | __GFP_ZERO */
	base = (void *)__get_free_pages(GFP_ATOMIC | GFP_DMA | __GFP_ZERO, page_n);
	if (!base) {
		res = -2;
		goto ERR_PAGES;
	}

	dma_addr = dma_map_single(cls_wifi_plat->dev, base, pool_sz, DMA_BIDIRECTIONAL);

	pool = gen_pool_create(ilog2(SMP_CACHE_BYTES), -1);
	if (!pool) {
		res = -3;
		goto ERR_POOL_CREATE;
	}

	if (gen_pool_add(pool, (unsigned long)base, pool_sz, -1)) {
		res = -4;
		goto ERR_POOL_ADD;
	}

	shm_header_sz = pcie_shm_pool_infos[type].num * sizeof(pcie_shm_header_st);
	if (!shm_header_sz)
		shm_header_sz = sizeof(pcie_shm_header_st);

	shm_header_sz = ALIGN(shm_header_sz, cache_line_size());
	shm_header = (pcie_shm_header_st *)gen_pool_alloc(pool, shm_header_sz);
	if (!shm_header) {
		res = -5;
		goto ERR_POOL_ADD;
	}

	*shm_obj = (pcie_shm_pool_st) {
		.virt_base = base,
		.dma_addr = dma_addr,
		.pool = pool,
		.header = shm_header,
		.size = pool_sz,
		.h_size = shm_header_sz,
		.type = type,
		.dev = dev,
		.tbl_max = pcie_shm_pool_infos[type].num
	};

	cls_wifi_plat->pcie_shm_pools[type] = shm_obj;

	devres_add(dev, shm_obj);

	pr_info("shm_pool_create created: virt=0x%px phys=0x%llx size=%uKB\n",
			base, (u64)virt_to_phys(base), pool_sz / 1024);

	return shm_obj;

ERR_POOL_ADD:
	gen_pool_destroy(pool);
ERR_POOL_CREATE:
	free_pages(*((unsigned int *)&base), page_n);
ERR_PAGES:
	devres_free(shm_obj);
ERR_DEVRES:
	pr_err("shm_pool_create Failed(%ld): pool(%d) size(%dKB) pages(%d)",
			res, type, pool_sz / 1024, page_n);

	return ERR_PTR(res);
}
EXPORT_SYMBOL(pcie_shm_pool_create);

int pcie_shm_get_tbl_offset(pcie_shm_pool_st *shm_obj, int tbl_idx, uint32_t *offset)
{
	if (!shm_obj || !shm_obj->header)
		return -1;

	*offset = shm_obj->header[tbl_idx].offset;
	return 0;
}
EXPORT_SYMBOL(pcie_shm_get_tbl_offset);
