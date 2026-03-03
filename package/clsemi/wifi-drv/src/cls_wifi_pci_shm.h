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
#ifndef __CLS_WIFI_PCI_SHM_H__
#define __CLS_WIFI_PCI_SHM_H__

#define PCIE_SHM_POOLS_TABLE  						\
	[SHM_POOL0_IDX] =  {   						\
		"POOL0", IRF_TBL_IDX_MAX, 512 * 1024 			\
	},

typedef enum PCIE_SHM_POOL_INDEX {
	SHM_POOL0_IDX,
	SHM_POOL_IDX_MAX,
} pcie_shm_pool_idx_em;

enum SHM_POOL0_TABLE {
	SHM_POOL0_IDX_IRF,
	SHM_POOL0_IDX_LOG_TRACE,
	SHM_POOL0_IDX_CSI,
	SHM_POOL0_IDX_TXBUFF,
	SHM_POOL0_IDX_RXBUFF,
	IRF_TBL_IDX_MAX,
};

struct pcie_shm_msg_create {
	uint32_t type;
	uint32_t size;
	uint64_t host_addr;
};

typedef struct pcie_shm_pool pcie_shm_pool_st;
struct cls_wifi_plat;


pcie_shm_pool_st *pcie_shm_pool_create(struct cls_wifi_plat *cls_wifi_plat,
		pcie_shm_pool_idx_em type);
uint64_t pcie_shm_get_pool_phyinfos(pcie_shm_pool_st *shm_obj, uint32_t *size);
void *pcie_shm_get_pool_addrinfos(pcie_shm_pool_st *shm_obj, uint32_t *size);
void pcie_shm_pool_dump(pcie_shm_pool_st *shm_obj);
void *pcie_shm_alloc(pcie_shm_pool_st *shm_obj, int tbl_idx, size_t size);
void pcie_shm_free(pcie_shm_pool_st *shm_obj, void *ptr, size_t size);
unsigned long pcie_shm_virt_to_phys(void *ptr);
void pcie_shmem_write_sync(pcie_shm_pool_st *shm_obj, void *ptr, size_t size);
void pcie_shmem_read_sync(pcie_shm_pool_st *shm_obj, void *ptr, size_t size);
int pcie_shm_get_tbl_offset(pcie_shm_pool_st *shm_obj, int tbl_idx, uint32_t *offset);

#endif /* __CLS_WIFI_PCI_SHM_H__*/
