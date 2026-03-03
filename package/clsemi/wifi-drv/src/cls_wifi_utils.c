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

#include "cls_wifi_utils.h"
#include "cls_wifi_defs.h"
#include "cls_wifi_rx.h"
#include "cls_wifi_tx.h"
#include "cls_wifi_msg_rx.h"
#include "cls_wifi_debugfs.h"
#include "cls_wifi_prof.h"
#include "cls_wifi_csi.h"
#include "ipc_host.h"
#include "cls_wifi_atf.h"

#ifdef CLS_WIFI_DUBHE_ETH
#include "asm/cacheflush.h"
#endif
#define FW_STR  "fmac"

/**
 * cls_wifi_ipc_buf_pool_alloc() - Allocate and push to fw a pool of IPC buffer.
 *
 * @cls_wifi_hw: Main driver structure
 * @pool: Pool to allocate
 * @nb: Size of the pool to allocate
 * @buf_size: Size of one pool element
 * @pool_name: Name of the pool
 * @push: Function to push one pool buffer to fw
 *
 * This function will allocate an array to store the list of IPC buffers,
 * a dma pool and @nb element in the dma pool.
 * Each buffer is initialized with '0' and then pushed to fw using the @push function.
 *
 * Return: 0 on success and <0 upon error. If error is returned any allocated
 * memory is NOT freed and cls_wifi_ipc_buf_pool_dealloc() must be called.
 */
static int cls_wifi_ipc_buf_pool_alloc(struct cls_wifi_hw *cls_wifi_hw,
								   struct cls_wifi_ipc_buf_pool *pool,
								   int nb, size_t buf_size, char *pool_name,
								   int (*push)(struct ipc_host_env_tag *,
											   struct cls_wifi_ipc_buf *))
{
	struct cls_wifi_ipc_buf *buf;
	int i;

	pool->nb = 0;

	/* allocate buf array */
	pool->buffers = kmalloc(nb * sizeof(struct cls_wifi_ipc_buf), GFP_KERNEL);
	if (!pool->buffers) {
		dev_err(cls_wifi_hw->dev, "Allocation of buffer array for %s failed\n",
				pool_name);
		return -ENOMEM;
	}

	/* allocate dma pool */
	pool->pool = dma_pool_create(pool_name, cls_wifi_hw->dev, buf_size,
								 cache_line_size(), 0);
	if (!pool->pool) {
		dev_err(cls_wifi_hw->dev, "Allocation of dma pool %s failed\n",
				pool_name);
		return -ENOMEM;
	}

	for (i = 0, buf = pool->buffers; i < nb; buf++, i++) {
		/* allocate a buffer */
		buf->size = buf_size;
		buf->addr = dma_pool_alloc(pool->pool, GFP_KERNEL, &buf->dma_addr);
		if (!buf->addr) {
			dev_err(cls_wifi_hw->dev, "Allocation of block %d/%d in %s failed\n",
					(i + 1), nb, pool_name);
			return -ENOMEM;
		}
		pool->nb++;

		/* reset the buffer */
		memset(buf->addr, 0, buf_size);

		/* push it to FW */
		push(cls_wifi_hw->ipc_env, buf);
	}

	return 0;
}

/**
 * cls_wifi_ipc_buf_pool_alloc() - Allocate and push to fw a pool of IPC buffer.
 *
 * @cls_wifi_hw: Main driver structure
 * @pool: Pool to allocate
 * @nb: Size of the pool to allocate
 * @buf_size: Size of one pool element
 * @pool_name: Name of the pool
 * @push: Function to push one pool buffer to fw
 *
 * This function will allocate an array to store the list of IPC buffers,
 * a dma pool and @nb element in the dma pool.
 * Each buffer is initialized with '0' and then pushed to fw using the @push function.
 *
 * Return: 0 on success and <0 upon error. If error is returned any allocated
 * memory is NOT freed and cls_wifi_ipc_buf_pool_dealloc() must be called.
 */
static int cls_wifi_cmn_ipc_buf_pool_alloc(struct ipc_host_cmn_env_tag *ipc_host_cmn_env,
								   struct cls_wifi_ipc_buf_pool *pool,
								   int nb, size_t buf_size, char *pool_name,
								   int (*push)(struct ipc_host_cmn_env_tag *,
											   struct cls_wifi_ipc_buf *))
{
	struct cls_wifi_ipc_buf *buf;
	int i;

	pool->nb = 0;

	/* allocate buf array */
	pool->buffers = kmalloc(nb * sizeof(struct cls_wifi_ipc_buf), GFP_KERNEL);
	if (!pool->buffers) {
		dev_err(ipc_host_cmn_env->dev, "Allocation of buffer array for %s failed\n",
				pool_name);
		return -ENOMEM;
	}

	/* allocate dma pool */
	pool->pool = dma_pool_create(pool_name, ipc_host_cmn_env->dev, buf_size,
								 cache_line_size(), 0);
	if (!pool->pool) {
		dev_err(ipc_host_cmn_env->dev, "Allocation of dma pool %s failed\n",
				pool_name);
		return -ENOMEM;
	}

	for (i = 0, buf = pool->buffers; i < nb; buf++, i++) {
		/* allocate a buffer */
		buf->size = buf_size;
		buf->addr = dma_pool_alloc(pool->pool, GFP_KERNEL, &buf->dma_addr);
		if (!buf->addr) {
			dev_err(ipc_host_cmn_env->dev, "Allocation of block %d/%d in %s failed\n",
					(i + 1), nb, pool_name);
			return -ENOMEM;
		}
		pool->nb++;

		/* reset the buffer */
		memset(buf->addr, 0, buf_size);

		/* push it to FW */
		push(ipc_host_cmn_env, buf);
	}

	return 0;
}



/**
 * cls_wifi_ipc_buf_pool_dealloc() - Free all memory allocated for a pool
 *
 * @pool: Pool to free
 *
 * Must be call once after cls_wifi_ipc_buf_pool_alloc(), even if it returned
 * an error
 */
static void cls_wifi_ipc_buf_pool_dealloc(struct cls_wifi_ipc_buf_pool *pool)
{
	struct cls_wifi_ipc_buf *buf;
	int i;

	for (i = 0, buf = pool->buffers; i < pool->nb ; buf++, i++) {
		dma_pool_free(pool->pool, buf->addr, buf->dma_addr);
	}
	pool->nb = 0;

	if (pool->pool)
		dma_pool_destroy(pool->pool);
	pool->pool = NULL;

	if (pool->buffers)
		kfree(pool->buffers);
	pool->buffers = NULL;
}

/**
 * cls_wifi_ipc_buf_alloc - Alloc a single ipc buffer and MAP it for DMA access
 *
 * @cls_wifi_hw: Main driver structure
 * @buf: IPC buffer to allocate
 * @buf_size: Size of the buffer to allocate
 * @dir: DMA direction
 * @init: Pointer to initial data to write in buffer before DMA sync. Used
 * only if direction is DMA_TO_DEVICE and it must be at least @buf_size long
 *
 * It allocates a buffer, initializes it if @init is set, and map it for DMA
 * Use @cls_wifi_ipc_buf_dealloc when this buffer is no longer needed.
 *
 * @return: 0 on success and <0 upon error. If error is returned any allocated
 * memory has been freed.
 */
int cls_wifi_ipc_buf_alloc(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_ipc_buf *buf,
					   size_t buf_size, enum dma_data_direction dir, const void *init)
{
	buf->addr = kmalloc(buf_size, GFP_KERNEL);
	if (!buf->addr)
		return -ENOMEM;

	buf->size = buf_size;

	if ((dir == DMA_TO_DEVICE) && init) {
		memcpy(buf->addr, init, buf_size);
	}

	buf->dma_addr = dma_map_single(cls_wifi_hw->dev, buf->addr, buf_size, dir);
	if (dma_mapping_error(cls_wifi_hw->dev, buf->dma_addr)) {
		pr_warn("%s %d %zx buf->addr %px buf->dma_addr %pad\n",
			__func__, __LINE__, buf_size, buf->addr, &buf->dma_addr);
		kfree(buf->addr);
		buf->addr = NULL;
		return -EIO;
	}

	return 0;
}

/**
 * cls_wifi_ipc_buf_dealloc() - Free memory allocated for a single ipc buffer
 *
 * @cls_wifi_hw: Main driver structure
 * @buf: IPC buffer to free
 *
 * IPC buffer must have been allocated by @cls_wifi_ipc_buf_alloc() or initialized
 * by @cls_wifi_ipc_buf_init() and pointing to a buffer allocated by kmalloc.
 */
void cls_wifi_ipc_buf_dealloc(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_ipc_buf *buf)
{
	if (!buf->addr)
		return;
	dma_unmap_single(cls_wifi_hw->dev, buf->dma_addr, buf->size, DMA_TO_DEVICE);
	kfree(buf->addr);
	buf->addr = NULL;
}

/**
 * cls_wifi_ipc_buf_a2e_init - Initialize an Application to Embedded IPC buffer
 * with a pre-allocated buffer
 *
 * @cls_wifi_hw: Main driver structure
 * @buf: IPC buffer to initialize
 * @data: Data buffer to use for the IPC buffer.
 * @buf_size: Size of the buffer the @data buffer
 *
 * Initialize the IPC buffer with the provided buffer and map it for DMA transfer.
 * The mapping direction is always DMA_TO_DEVICE as this an "a2e" buffer.
 * Use @cls_wifi_ipc_buf_dealloc() when this buffer is no longer needed.
 *
 * @return: 0 on success and <0 upon error. If error is returned the @data buffer
 * is freed.
 */
int cls_wifi_ipc_buf_a2e_init(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_ipc_buf *buf,
						  void *data, size_t buf_size)
{
#ifdef CLS_WIFI_DUBHE_ETH
	struct cls_wifi_plat *cls_wifi_plat = cls_wifi_hw->plat;
	struct cls_wifi_eth_info *eth_info = &cls_wifi_plat->eth_info;
#endif

	buf->addr = data;
	buf->size = buf_size;

#ifdef CLS_WIFI_DUBHE_ETH
	//pr_warn("[%s]dev:%px,addr:%px,soft_bmu_en %d, tx_addr_phy %pad, tx_size_virt %x, tx_addr_virt %px\r\n", __FUNCTION__,cls_wifi_hw->dev, buf->addr,
	//	eth_info->soft_bmu_en, &eth_info->tx_addr_phy, eth_info->tx_size_virt, eth_info->tx_addr_virt);

	if((!eth_info->soft_bmu_en) &&
		(buf->addr >= eth_info->tx_addr_virt) &&
		(buf->addr <= (eth_info->tx_addr_virt + eth_info->tx_size_virt))){
		buf->dma_addr = eth_info->tx_addr_phy + (buf->addr - eth_info->tx_addr_virt);
		__flush_dcache_area(buf->addr, buf_size);
	}else {
#endif
		buf->dma_addr = dma_map_single(cls_wifi_hw->dev, buf->addr, buf_size,
									   DMA_TO_DEVICE);
		if (dma_mapping_error(cls_wifi_hw->dev, buf->dma_addr)) {
			buf->addr = NULL;
			return -EIO;
		}
#ifdef CLS_WIFI_DUBHE_ETH
	}
#endif

	return 0;
}

/**
 * cls_wifi_ipc_buf_release() - Release DMA mapping for an IPC buffer
 *
 * @cls_wifi_hw: Main driver structure
 * @buf: IPC buffer to release
 * @dir: DMA direction.
 *
 * This also "release" the IPC buffer structure (i.e. its addr field is reset)
 * so that it cannot be re-used except to map another buffer.
 */
void cls_wifi_ipc_buf_release(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_ipc_buf *buf,
						  enum dma_data_direction dir)
{
#ifdef CLS_WIFI_DUBHE_ETH
	struct cls_wifi_plat *cls_wifi_plat = cls_wifi_hw->plat;
	struct cls_wifi_eth_info *eth_info = &cls_wifi_plat->eth_info;
#endif

	if (!buf->addr)
		return;

#ifdef CLS_WIFI_DUBHE_ETH
	if((buf->addr < eth_info->tx_addr_virt) ||
			(buf->addr > (eth_info->tx_addr_virt + eth_info->tx_size_virt))) {
		dma_unmap_single(cls_wifi_hw->dev, buf->dma_addr, buf->size, dir);
	}
#else
	dma_unmap_single(cls_wifi_hw->dev, buf->dma_addr, buf->size, dir);
#endif

	buf->addr = NULL;
}

/**
 * cls_wifi_ipc_buf_e2a_sync() - Synchronize all (or part) of an IPC buffer before
 * reading content written by the embedded
 *
 * @cls_wifi_hw: Main driver structure
 * @buf: IPC buffer to sync
 * @len: Length to read, 0 means the whole buffer
 */
void cls_wifi_ipc_buf_e2a_sync(struct device *dev, struct cls_wifi_ipc_buf *buf,
						   size_t len)
{
	if (!len)
		len = buf->size;

	dma_sync_single_for_cpu(dev, buf->dma_addr, len, DMA_FROM_DEVICE);
}

/**
 * cls_wifi_ipc_buf_e2a_sync_back() - Synchronize back all (or part) of an IPC buffer
 * to allow embedded updating its content.
 *
 * @cls_wifi_hw: Main driver structure
 * @buf: IPC buffer to sync
 * @len: Length to sync back, 0 means the whole buffer
 *
 * Must be called after each call to cls_wifi_ipc_buf_e2a_sync() even if host didn't
 * modified the content of the buffer.
 */
void cls_wifi_ipc_buf_e2a_sync_back(struct device *dev, struct cls_wifi_ipc_buf *buf,
							size_t len)
{
	if (!len)
		len = buf->size;

	dma_sync_single_for_device(dev, buf->dma_addr, len, DMA_FROM_DEVICE);
}

/**
 * cls_wifi_ipc_rxskb_alloc() - Allocate a skb for RX path
 *
 * @cls_wifi_hw: Main driver data
 * @buf: cls_wifi_ipc_buf structure to store skb address
 * @skb_size: Size of the buffer to allocate
 *
 * Allocate a skb for RX path, meaning that the data buffer is written by the firmware
 * and needs then to be DMA mapped.
 *
 * Note that even though the result is stored in a struct cls_wifi_ipc_buf, in this case the
 * cls_wifi_ipc_buf.addr points to skb structure whereas the cls_wifi_ipc_buf.dma_addr is the
 * DMA address of the skb data buffer (i.e. skb->data)
 */
static int cls_wifi_ipc_rxskb_alloc(struct device *dev,
								struct cls_wifi_ipc_buf *buf, size_t skb_size)
{
	struct sk_buff *skb = dev_alloc_skb(skb_size);

	if (unlikely(!skb)) {
		dev_err(dev, "Allocation of RX skb failed\n");
		buf->addr = NULL;
		return -ENOMEM;
	}

	buf->dma_addr = dma_map_single(dev, skb->data, skb_size,
								   DMA_FROM_DEVICE);
	if (unlikely(dma_mapping_error(dev, buf->dma_addr))) {
		dev_err(dev, "DMA mapping of RX skb failed\n");
		dev_kfree_skb(skb);
		buf->addr = NULL;
		return -EIO;
	}

	buf->addr = skb;
	buf->size = skb_size;

	return 0;
}

/**
 * cls_wifi_ipc_rxskb_reset_pattern() - Reset pattern in a RX skb or unsupported
 * RX vector buffer
 *
 * @cls_wifi_hw: Main driver data
 * @buf: RX skb to reset
 * @pattern_offset: Pattern location, in byte from the start of the buffer
 *
 * Reset the pattern in a RX/unsupported RX vector skb buffer to inform embedded
 * that it has been processed by the host.
 * Pattern in a 32bit value.
 */
static void cls_wifi_ipc_rxskb_reset_pattern(struct device *dev,
										 struct cls_wifi_ipc_buf *buf,
										 size_t pattern_offset)
{
	struct sk_buff *skb = buf->addr;
	u32 *pattern = (u32 *)(skb->data + pattern_offset);

	*pattern = 0;
	dma_sync_single_for_device(dev, buf->dma_addr + pattern_offset,
							   sizeof(u32), DMA_FROM_DEVICE);
}

/**
 * cls_wifi_ipc_rxskb_dealloc() - Free a skb allocated for the RX path
 *
 * @cls_wifi_hw: Main driver data
 * @buf: Rx skb to free
 *
 * Free a RX skb allocated by @cls_wifi_ipc_rxskb_alloc
 */
static void cls_wifi_ipc_rxskb_dealloc(struct device *dev,
								   struct cls_wifi_ipc_buf *buf)
{
	if (!buf->addr)
		return;

	dma_unmap_single(dev, buf->dma_addr, buf->size, DMA_TO_DEVICE);
	dev_kfree_skb((struct sk_buff *)buf->addr);
	buf->addr = NULL;
}


/**
 * cls_wifi_ipc_unsup_rx_vec_elem_allocs() - Allocate and push an unsupported
 *									   RX vector buffer for the FW
 *
 * @cls_wifi_hw: Main driver data
 * @elem: Pointer to the skb elem that will contain the address of the buffer
 */
int cls_wifi_ipc_unsuprxvec_alloc(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_ipc_buf *buf)
{
	int err;

	err = cls_wifi_ipc_rxskb_alloc(cls_wifi_hw->dev, buf, cls_wifi_hw->ipc_env->unsuprxvec_sz);
	if (err)
		return err;

	cls_wifi_ipc_rxskb_reset_pattern(cls_wifi_hw->dev, buf,
								 offsetof(struct rx_vector_desc, pattern));
	ipc_host_unsuprxvec_push(cls_wifi_hw->ipc_env, buf);
	return 0;
}

/**
 * cls_wifi_ipc_unsuprxvec_repush() - Reset and repush an already allocated buffer
 * for unsupported RX vector
 *
 * @cls_wifi_hw: Main driver data
 * @buf: Buf to repush
 */
void cls_wifi_ipc_unsuprxvec_repush(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_ipc_buf *buf)
{
	cls_wifi_ipc_rxskb_reset_pattern(cls_wifi_hw->dev, buf,
								 offsetof(struct rx_vector_desc, pattern));
	ipc_host_unsuprxvec_push(cls_wifi_hw->ipc_env, buf);
}

/**
* cls_wifi_ipc_unsuprxvecs_alloc() - Allocate and push all unsupported RX
* vector buffers for the FW
*
* @cls_wifi_hw: Main driver data
*/
static int cls_wifi_ipc_unsuprxvecs_alloc(struct cls_wifi_hw *cls_wifi_hw)
{
   struct cls_wifi_ipc_buf *buf;
   int i;

   memset(cls_wifi_hw->unsuprxvecs, 0, sizeof(cls_wifi_hw->unsuprxvecs));

   for (i = 0, buf = cls_wifi_hw->unsuprxvecs; i < ARRAY_SIZE(cls_wifi_hw->unsuprxvecs); i++, buf++)
   {
	   if (cls_wifi_ipc_unsuprxvec_alloc(cls_wifi_hw, buf)) {
		   dev_err(cls_wifi_hw->dev, "Failed to allocate unsuprxvec buf %d\n", i + 1);
		   return -ENOMEM;
	   }
   }

   return 0;
}

/**
 * cls_wifi_ipc_unsuprxvecs_dealloc() - Free all unsupported RX vector buffers
 * allocated for the FW
 *
 * @cls_wifi_hw: Main driver data
 */
static void cls_wifi_ipc_unsuprxvecs_dealloc(struct cls_wifi_hw *cls_wifi_hw)
{
	struct cls_wifi_ipc_buf *buf;
	int i = 0;

	for (buf = cls_wifi_hw->unsuprxvecs; i < ARRAY_SIZE(cls_wifi_hw->unsuprxvecs); i++, buf++)
	{
		cls_wifi_ipc_rxskb_dealloc(cls_wifi_hw->dev, buf);
	}
}

/**
 * cls_wifi_ipc_rxbuf_init_alloc() - Allocate and push a rx buffer for the FW
 *
 * @cls_wifi_hw: Main driver data
 * @buf: IPC buffer where to store address of the skb. In fullmac this
 * parameter is not available so look for the first free IPC buffer
 */
int cls_wifi_ipc_rxbuf_init_alloc(struct cls_wifi_hw *cls_wifi_hw)
{
	int err;
	int max_rxbuf_cnt;
	struct cls_wifi_ipc_buf *buf;
	int nb = 0, idx = cls_wifi_hw->rxbuf_idx;

	max_rxbuf_cnt = cls_wifi_hw->rxbufs_nb;

	while (cls_wifi_hw->rxbufs[idx].addr && (nb < max_rxbuf_cnt)) {
		idx = ( idx + 1 ) % max_rxbuf_cnt;
		nb++;
	}
	if (nb == max_rxbuf_cnt) {
		dev_err(cls_wifi_hw->dev, "No more free space for rxbuff");
		return -ENOMEM;
	}

	buf = &cls_wifi_hw->rxbufs[idx];

	/* Save idx so that on next push the free slot will be found quicker */
	cls_wifi_hw->rxbuf_idx = ( idx + 1 ) % max_rxbuf_cnt;

	err = cls_wifi_ipc_rxskb_alloc(cls_wifi_hw->dev, buf, cls_wifi_hw->ipc_env->rxbuf_sz);
	if (err)
		return err;

	cls_wifi_ipc_rxskb_reset_pattern(cls_wifi_hw->dev, buf, offsetof(struct hw_rxhdr, pattern));
	CLS_WIFI_RXBUFF_HOSTID_SET(buf, CLS_WIFI_RXBUFF_IDX_TO_HOSTID(idx));
	ipc_host_rxbuf_init_push(cls_wifi_hw->ipc_env, buf);

	return 0;
}

int cls_wifi_cmn_ipc_rxbuf_init_alloc(struct cls_wifi_cmn_hw *cmn_hw, int radio)
{
	int err;
	int max_rxbuf_cnt;
	struct cls_wifi_ipc_buf *buf;
	struct cls_wifi_ipc_buf *bufs;
	int nb = 0;
	int idx, *next_idx;

	if (radio == RADIO_2P4G_INDEX) {
		idx = cmn_hw->rxbuf_idx_2g;
		max_rxbuf_cnt = CLS_WIFI_RXBUFF_MAX_2G4_CMN;
		bufs = (struct cls_wifi_ipc_buf *)cmn_hw->rxbufs_2g;
		next_idx = &cmn_hw->rxbuf_idx_2g;
	} else {
		idx = cmn_hw->rxbuf_idx_5g;
		max_rxbuf_cnt = CLS_WIFI_RXBUFF_MAX_5G_CMN;
		bufs = (struct cls_wifi_ipc_buf *)&cmn_hw->rxbufs_5g;
		next_idx = &cmn_hw->rxbuf_idx_5g;
	}

	while (bufs[idx].addr && (nb < max_rxbuf_cnt)) {
		idx = ( idx + 1 ) % max_rxbuf_cnt;
		nb++;
	}
	if (nb == max_rxbuf_cnt) {
		dev_err(cmn_hw->dev, "No more free space for cmn rxbuff %s\n",
			radio == RADIO_2P4G_INDEX ? "2g" : "5g");
		return -ENOMEM;
	}

	buf = &bufs[idx];

	/* Save idx so that on next push the free slot will be found quicker */
	*next_idx = ( idx + 1 ) % max_rxbuf_cnt;

	err = cls_wifi_ipc_rxskb_alloc(cmn_hw->dev, buf, cmn_hw->ipc_host_cmn_env->rxbuf_sz);
	if (err)
		return err;

	cls_wifi_ipc_rxskb_reset_pattern(cmn_hw->dev, buf, offsetof(struct hw_rxhdr, pattern));
	CLS_WIFI_RXBUFF_HOSTID_SET(buf, CLS_WIFI_RXBUFF_IDX_TO_HOSTID(idx));
	if (radio == RADIO_2P4G_INDEX)
		ipc_host_cmn_rxbuf_init_push_2g(cmn_hw->ipc_host_cmn_env, buf);
	else
		ipc_host_cmn_rxbuf_init_push_5g(cmn_hw->ipc_host_cmn_env, buf);

	return 0;
}

/**
 * cls_wifi_ipc_rxbuf_alloc() - Allocate and push a rx buffer for the FW
 *
 * @cls_wifi_hw: Main driver data
 * @buf: IPC buffer where to store address of the skb. In fullmac this
 * parameter is not available so look for the first free IPC buffer
 */
int cls_wifi_ipc_rxbuf_alloc(struct cls_wifi_hw *cls_wifi_hw)
{
	int err;
	int max_rxbuf_cnt;
	struct cls_wifi_ipc_buf *buf;
	int nb = 0, idx = cls_wifi_hw->rxbuf_idx;

	max_rxbuf_cnt = cls_wifi_hw->rxbufs_nb;

	while (cls_wifi_hw->rxbufs[idx].addr && (nb < max_rxbuf_cnt)) {
		idx = ( idx + 1 ) % max_rxbuf_cnt;
		nb++;
	}
	if (nb == max_rxbuf_cnt) {
		dev_err(cls_wifi_hw->dev, "No more free space for rxbuff");
		return -ENOMEM;
	}

	buf = &cls_wifi_hw->rxbufs[idx];

	/* Save idx so that on next push the free slot will be found quicker */
	cls_wifi_hw->rxbuf_idx = ( idx + 1 ) % max_rxbuf_cnt;

	err = cls_wifi_ipc_rxskb_alloc(cls_wifi_hw->dev, buf, cls_wifi_hw->ipc_env->rxbuf_sz);
	if (err)
		return err;

	cls_wifi_ipc_rxskb_reset_pattern(cls_wifi_hw->dev, buf, offsetof(struct hw_rxhdr, pattern));
	CLS_WIFI_RXBUFF_HOSTID_SET(buf, CLS_WIFI_RXBUFF_IDX_TO_HOSTID(idx));
	ipc_host_rxbuf_push(cls_wifi_hw->ipc_env, buf);

	return 0;
}

int cls_wifi_cmn_ipc_rxbuf_alloc(struct cls_wifi_cmn_hw *cmn_hw, int radio)
{
	int err;
	int max_rxbuf_cnt;
	struct cls_wifi_ipc_buf *buf;
	struct cls_wifi_ipc_buf *bufs;
	int nb = 0;
	int idx, *next_idx;

	if (radio == RADIO_2P4G_INDEX) {
		idx = cmn_hw->rxbuf_idx_2g;
		max_rxbuf_cnt = CLS_WIFI_RXBUFF_MAX_2G4_CMN;
		bufs = (struct cls_wifi_ipc_buf *)cmn_hw->rxbufs_2g;
		next_idx = &cmn_hw->rxbuf_idx_2g;
	} else {
		idx = cmn_hw->rxbuf_idx_5g;
		max_rxbuf_cnt = CLS_WIFI_RXBUFF_MAX_5G_CMN;
		bufs = (struct cls_wifi_ipc_buf *)&cmn_hw->rxbufs_5g;
		next_idx = &cmn_hw->rxbuf_idx_5g;
	}

	while (bufs[idx].addr && (nb < max_rxbuf_cnt)) {
		idx = ( idx + 1 ) % max_rxbuf_cnt;
		nb++;
	}
	if (nb == max_rxbuf_cnt) {
		dev_err(cmn_hw->dev, "No more free space for cmn rxbuff %s\n",
			radio == RADIO_2P4G_INDEX ? "2g" : "5g");
		return -ENOMEM;
	}

	buf = &bufs[idx];

	/* Save idx so that on next push the free slot will be found quicker */
	*next_idx = ( idx + 1 ) % max_rxbuf_cnt;

	err = cls_wifi_ipc_rxskb_alloc(cmn_hw->dev, buf, cmn_hw->ipc_host_cmn_env->rxbuf_sz);
	if (err)
		return err;

	cls_wifi_ipc_rxskb_reset_pattern(cmn_hw->dev, buf, offsetof(struct hw_rxhdr, pattern));
	CLS_WIFI_RXBUFF_HOSTID_SET(buf, CLS_WIFI_RXBUFF_IDX_TO_HOSTID(idx));
	if (radio == RADIO_2P4G_INDEX)
		ipc_host_cmn_rxbuf_push_2g(cmn_hw->ipc_host_cmn_env, buf);
	else
		ipc_host_cmn_rxbuf_push_5g(cmn_hw->ipc_host_cmn_env, buf);

	return 0;
}
/**
 * cls_wifi_ipc_rxbuf_dealloc() - Free a RX buffer for the FW
 *
 * @cls_wifi_hw: Main driver data
 * @buf: IPC buffer associated to the RX buffer to free
 */
void cls_wifi_ipc_rxbuf_dealloc(struct device *dev, struct cls_wifi_ipc_buf *buf)
{
	cls_wifi_ipc_rxskb_dealloc(dev, buf);
}

/**
 * cls_wifi_ipc_rxbuf_repush() - Reset and repush an already allocated RX buffer
 *
 * @cls_wifi_hw: Main driver data
 * @buf: Buf to repush
 *
 * In case a skb is not forwarded to upper layer it can be re-used.
 */
void cls_wifi_ipc_rxbuf_repush(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_ipc_buf *buf)
{
	cls_wifi_ipc_rxskb_reset_pattern(cls_wifi_hw->dev, buf, offsetof(struct hw_rxhdr, pattern));
	ipc_host_rxbuf_push(cls_wifi_hw->ipc_env, buf);
}

void cls_wifi_cmn_ipc_rxbuf_repush(struct cls_wifi_cmn_hw *cmn_hw, struct cls_wifi_ipc_buf *buf, u8 radio_idx)
{
	cls_wifi_ipc_rxskb_reset_pattern(cmn_hw->dev, buf, offsetof(struct hw_rxhdr, pattern));
	if (radio_idx == RADIO_2P4G_INDEX)
		ipc_host_cmn_rxbuf_push_2g(cmn_hw->ipc_host_cmn_env, buf);
	else
		ipc_host_cmn_rxbuf_push_5g(cmn_hw->ipc_host_cmn_env, buf);
}


/**
 * cls_wifi_ipc_rxbufs_alloc() - Allocate and push all RX buffer for the FW
 *
 * @cls_wifi_hw: Main driver data
 */
static int cls_wifi_ipc_rxbufs_alloc(struct cls_wifi_hw *cls_wifi_hw)
{
	int i, nb = cls_wifi_hw->ipc_env->rxbuf_nb;

	memset(cls_wifi_hw->rxbufs, 0, cls_wifi_hw->rxbufs_nb*sizeof(struct cls_wifi_ipc_buf));

	for (i = 0; i < nb; i++) {
		if (cls_wifi_ipc_rxbuf_init_alloc(cls_wifi_hw)) {
			dev_err(cls_wifi_hw->dev, "Failed to allocate rx buf %d/%d\n",
					i + 1, nb);
			return -ENOMEM;
		}
	}

	return 0;
}

static int cls_wifi_cmn_ipc_rxbufs_alloc(struct cls_wifi_cmn_hw *cmn_hw)
{
	int i;

	memset(cmn_hw->rxbufs_2g, 0, sizeof(cmn_hw->rxbufs_2g));

	for (i = 0; i < CMN_RXBUF_CNT_2G; i++) {
		if (cls_wifi_cmn_ipc_rxbuf_init_alloc(cmn_hw, RADIO_2P4G_INDEX)) {
			dev_err(cmn_hw->dev, "Failed to allocate 2g cmn rx buf %d/%d\n",
					i + 1, CMN_RXBUF_CNT_2G);
			return -ENOMEM;
		}
	}

	for (i = 0; i < CMN_RXBUF_CNT_5G; i++) {
		if (cls_wifi_cmn_ipc_rxbuf_init_alloc(cmn_hw, RADIO_5G_INDEX)) {
			dev_err(cmn_hw->dev, "Failed to allocate 5g cmn rx buf %d/%d\n",
					i + 1, CMN_RXBUF_CNT_5G);
			return -ENOMEM;
		}
	}
	return 0;
}
/**
 * cls_wifi_ipc_rxbufs_dealloc() - Free all RX buffer allocated for the FW
 *
 * @cls_wifi_hw: Main driver data
 */
static void cls_wifi_ipc_rxbufs_dealloc(struct cls_wifi_hw *cls_wifi_hw)
{
	struct cls_wifi_ipc_buf *buf;
	int i;

	for (i = 0, buf = cls_wifi_hw->rxbufs; i < cls_wifi_hw->rxbufs_nb; i++, buf++) {
		cls_wifi_ipc_rxskb_dealloc(cls_wifi_hw->dev, buf);
	}
}

static void cls_wifi_cmn_ipc_rxbufs_dealloc(struct cls_wifi_cmn_hw *cmn_hw)
{
	struct cls_wifi_ipc_buf *buf;
	int i;

	for (i = 0, buf = cmn_hw->rxbufs_2g; i < ARRAY_SIZE(cmn_hw->rxbufs_2g); i++, buf++) {
		cls_wifi_ipc_rxskb_dealloc(cmn_hw->dev, buf);
	}

	for (i = 0, buf = cmn_hw->rxbufs_5g; i < ARRAY_SIZE(cmn_hw->rxbufs_5g); i++, buf++) {
		cls_wifi_ipc_rxskb_dealloc(cmn_hw->dev, buf);
	}
}

/**
 * cls_wifi_ipc_rxdesc_repush() - Repush a RX descriptor to FW
 *
 * @cls_wifi_hw: Main driver data
 * @buf: RX descriptor to repush
 *
 * Once RX buffer has been received, the RX descriptor used by FW to upload this
 * buffer can be re-used for another RX buffer.
 */
void cls_wifi_ipc_rxdesc_repush(struct cls_wifi_hw *cls_wifi_hw,
							struct cls_wifi_ipc_buf *buf)
{
	struct rxdesc_tag *rxdesc = buf->addr;
	rxdesc->status = 0;
	dma_sync_single_for_device(cls_wifi_hw->dev, buf->dma_addr,
							   sizeof(struct rxdesc_tag), DMA_BIDIRECTIONAL);
	ipc_host_rxdesc_push(cls_wifi_hw->ipc_env, buf);
}

void cls_wifi_cmn_ipc_rxdesc_repush(struct cls_wifi_cmn_hw *cmn_hw,
							struct cls_wifi_ipc_buf *buf, u8 radio_idx)
{
	struct rxdesc_tag *rxdesc = buf->addr;
	rxdesc->status = 0;
	dma_sync_single_for_device(cmn_hw->dev, buf->dma_addr,
							   sizeof(struct rxdesc_tag), DMA_BIDIRECTIONAL);
	if (radio_idx == RADIO_2P4G_INDEX)
		ipc_host_cmn_rxdesc_push_2g(cmn_hw->ipc_host_cmn_env, buf);
	else
		ipc_host_cmn_rxdesc_push_5g(cmn_hw->ipc_host_cmn_env, buf);
}
/**
 * cls_wifi_ipc_rxbuf_from_hostid() - Return IPC buffer of a RX buffer from a hostid
 *
 * @cls_wifi_hw: Main driver data
 * @hostid: Hostid of the RX buffer
 * @return: Pointer to the RX buffer with the provided hostid and NULL if the
 * hostid is invalid or no buffer is associated.
 */
struct cls_wifi_ipc_buf *cls_wifi_ipc_rxbuf_from_hostid(struct cls_wifi_hw *cls_wifi_hw, u32 hostid)
{
	unsigned int rxbuf_idx = CLS_WIFI_RXBUFF_HOSTID_TO_IDX(hostid);

	if (rxbuf_idx < cls_wifi_hw->rxbufs_nb) {
		struct cls_wifi_ipc_buf *buf = &cls_wifi_hw->rxbufs[rxbuf_idx];
		if (buf->addr && (CLS_WIFI_RXBUFF_HOSTID_GET(buf) == hostid))
			return buf;

		dev_err(cls_wifi_hw->dev, "Invalid Rx buff: hostid=%d addr=%p hostid_in_buff=%d\n",
				hostid, buf->addr, (buf->addr) ? CLS_WIFI_RXBUFF_HOSTID_GET(buf): -1);

		if (buf->addr)
			cls_wifi_ipc_rxbuf_dealloc(cls_wifi_hw->dev, buf);
	}

	dev_err(cls_wifi_hw->dev, "RX Buff invalid hostid [%d]\n", hostid);
	return NULL;
}

struct cls_wifi_ipc_buf *cls_wifi_cmn_ipc_rxbuf_from_hostid(struct cls_wifi_cmn_hw *cmn_hw, u32 hostid, u8 radio_idx, u16 status)
{
	unsigned int rxbuf_idx = CLS_WIFI_RXBUFF_HOSTID_TO_IDX(hostid);
	uint32_t rxbuf_max;

	if(radio_idx == RADIO_2P4G_INDEX)
		rxbuf_max = CLS_WIFI_RXBUFF_MAX_2G4_CMN;
	else
		rxbuf_max = CLS_WIFI_RXBUFF_MAX_5G_CMN;

	if (rxbuf_idx < rxbuf_max) {
		struct cls_wifi_ipc_buf *buf;
		if (radio_idx == RADIO_2P4G_INDEX)
			buf = &cmn_hw->rxbufs_2g[rxbuf_idx];
		else
			buf = &cmn_hw->rxbufs_5g[rxbuf_idx];
		if (buf->addr && (CLS_WIFI_RXBUFF_HOSTID_GET(buf) == hostid))
			return buf;

		if (RX_STAT_ALLOC & status)
			return NULL;

		dev_err(cmn_hw->dev, "Invalid Rx buff cmn: radio=%d hostid=%d addr=%p hostid_in_buff=%d\n",
				radio_idx, hostid, buf->addr, (buf->addr) ? CLS_WIFI_RXBUFF_HOSTID_GET(buf): -1);

		if (buf->addr)
			cls_wifi_ipc_rxbuf_dealloc(cmn_hw->dev, buf);
	}

	dev_err(cmn_hw->dev, "RX Buff cmn invalid hostid [%d], radio:%d\n", hostid, radio_idx);
	return NULL;
}
/**
 * cls_wifi_elems_deallocs() - Deallocate IPC storage elements.
 * @cls_wifi_hw: Main driver data
 *
 * This function deallocates all the elements required for communications with
 * LMAC, such as Rx Data elements, MSGs elements, ...
 * This function should be called in correspondence with the allocation function.
 */
static void cls_wifi_elems_deallocs(struct cls_wifi_hw *cls_wifi_hw)
{
	cls_wifi_ipc_rxbufs_dealloc(cls_wifi_hw);
	cls_wifi_ipc_unsuprxvecs_dealloc(cls_wifi_hw);
	cls_wifi_ipc_buf_pool_dealloc(&cls_wifi_hw->msgbuf_pool);
	cls_wifi_ipc_buf_pool_dealloc(&cls_wifi_hw->dbgbuf_pool);
	cls_wifi_ipc_buf_pool_dealloc(&cls_wifi_hw->radar_pool);
	cls_wifi_ipc_buf_pool_dealloc(&cls_wifi_hw->he_mu_map_pool);
	cls_wifi_ipc_buf_pool_dealloc(&cls_wifi_hw->txcfm_pool);
	cls_wifi_ipc_buf_dealloc(cls_wifi_hw, &cls_wifi_hw->thd_pattern);
	cls_wifi_ipc_buf_dealloc(cls_wifi_hw, &cls_wifi_hw->tbd_pattern);
	cls_wifi_ipc_buf_dealloc(cls_wifi_hw, &cls_wifi_hw->dbgdump.buf);
	cls_wifi_ipc_buf_pool_dealloc(&cls_wifi_hw->csibuf_pool);
	cls_wifi_ipc_buf_dealloc(cls_wifi_hw, &cls_wifi_hw->atf_quota_buf);
	cls_wifi_ipc_buf_dealloc(cls_wifi_hw, &cls_wifi_hw->atf_stats_buf);
	cls_wifi_ipc_buf_dealloc(cls_wifi_hw, &cls_wifi_hw->dbg_cnt_buf);
}

static void cls_wifi_cmn_elems_deallocs(struct cls_wifi_cmn_hw *cmn_hw)
{
	cls_wifi_cmn_ipc_rxbufs_dealloc(cmn_hw);
	cls_wifi_ipc_buf_pool_dealloc(&cmn_hw->dbgbuf_pool);
	cls_wifi_ipc_buf_pool_dealloc(&cmn_hw->rxdesc_pool_2g);
	cls_wifi_ipc_buf_pool_dealloc(&cmn_hw->rxdesc_pool_5g);
}

/**
 * cls_wifi_elems_allocs() - Allocate IPC storage elements.
 * @cls_wifi_hw: Main driver data
 *
 * This function allocates all the elements required for communications with
 * LMAC, such as Rx Data elements, MSGs elements, ...
 * This function should be called in correspondence with the deallocation function.
 */
static int cls_wifi_elems_allocs(struct cls_wifi_hw *cls_wifi_hw)
{
	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (dma_set_mask_and_coherent(cls_wifi_hw->dev, DMA_BIT_MASK(32)))
		goto err_alloc;

	if (cls_wifi_ipc_buf_pool_alloc(cls_wifi_hw, &cls_wifi_hw->msgbuf_pool,
								cls_wifi_hw->ipc_env->e2amsg_nb,
								sizeof(struct ipc_e2a_msg),
								"cls_wifi_ipc_msgbuf_pool",
								ipc_host_msgbuf_push))
		goto err_alloc;

	if (cls_wifi_ipc_buf_pool_alloc(cls_wifi_hw, &cls_wifi_hw->dbgbuf_pool,
								cls_wifi_hw->ipc_env->dbgbuf_nb,
								sizeof(struct ipc_dbg_msg),
								"cls_wifi_ipc_dbgbuf_pool",
								ipc_host_dbgbuf_push))
		goto err_alloc;

	if (cls_wifi_ipc_buf_pool_alloc(cls_wifi_hw, &cls_wifi_hw->radar_pool,
								IPC_RADARBUF_CNT,
								sizeof(struct radar_pulse_array_desc),
								"cls_wifi_ipc_radar_pool",
								ipc_host_radar_push))
		goto err_alloc;

	if (cls_wifi_ipc_buf_pool_alloc(cls_wifi_hw, &cls_wifi_hw->he_mu_map_pool,
								IPC_HEMUBUF_CNT,
								sizeof(struct he_mu_map_array_desc),
								"cls_wifi_ipc_he_mu_map_pool",
								ipc_host_he_mu_map_push))
		goto err_alloc;

	if (cls_wifi_ipc_buf_pool_alloc(cls_wifi_hw, &cls_wifi_hw->txcfm_pool,
								cls_wifi_hw->ipc_env->txcfm_nb,
								sizeof(struct tx_cfm_tag),
								"cls_wifi_ipc_txcfm_pool",
								ipc_host_txcfm_push))
		goto err_alloc;

	if (cls_wifi_ipc_buf_pool_alloc(cls_wifi_hw, &cls_wifi_hw->csibuf_pool,
								IPC_CSIBUF_CNT,
								sizeof(struct cls_csi_report),
								"cls_wifi_ipc_csibuf_pool",
								ipc_host_csi_buf_push))
		goto err_alloc;

	if (cls_wifi_ipc_unsuprxvecs_alloc(cls_wifi_hw))
		goto err_alloc;

	if (cls_wifi_ipc_buf_a2e_alloc(cls_wifi_hw, &cls_wifi_hw->thd_pattern, sizeof(u32),
							   &cls_wifi_thd_pattern))
		goto err_alloc;
	ipc_host_thd_pattern_push(cls_wifi_hw->ipc_env, &cls_wifi_hw->thd_pattern);

	if (cls_wifi_ipc_buf_a2e_alloc(cls_wifi_hw, &cls_wifi_hw->tbd_pattern, sizeof(u32),
							   &cls_wifi_tbd_pattern))
		goto err_alloc;
	ipc_host_tbd_pattern_push(cls_wifi_hw->ipc_env, &cls_wifi_hw->tbd_pattern);

	if (cls_wifi_ipc_buf_a2e_alloc(cls_wifi_hw, &cls_wifi_hw->atf_quota_buf,
		cls_wifi_quota_table_size(cls_wifi_hw), NULL))
		goto err_alloc;

	ipc_host_atf_quota_push(cls_wifi_hw->ipc_env, &cls_wifi_hw->atf_quota_buf);

	if (cls_wifi_ipc_buf_e2a_alloc(cls_wifi_hw, &cls_wifi_hw->atf_stats_buf, cls_wifi_airtime_stats_size(cls_wifi_hw)))
		goto err_alloc;

	ipc_host_atf_stats_push(cls_wifi_hw->ipc_env, &cls_wifi_hw->atf_stats_buf);
	//todo: comment to debug
	//if (cls_wifi_ipc_buf_e2a_alloc(cls_wifi_hw, &cls_wifi_hw->dbgdump.buf,
	//						   sizeof(struct dbg_debug_dump_tag)))
	//	goto err_alloc;
	//ipc_host_dbginfo_push(cls_wifi_hw->ipc_env, &cls_wifi_hw->dbgdump.buf);

	if (cls_wifi_ipc_buf_e2a_alloc(cls_wifi_hw, &cls_wifi_hw->dbg_cnt_buf,
		sizeof(struct ipc_shared_dbg_cnt))) {
		pr_warn("---alloc dbg_cnt_buf failed\n");
		goto err_alloc;
	}
	ipc_host_dbg_cnt_addr_push(cls_wifi_hw->ipc_env, &cls_wifi_hw->dbg_cnt_buf);

	return 0;

err_alloc:
	dev_err(cls_wifi_hw->dev, "Error while allocating IPC buffers\n");
	cls_wifi_elems_deallocs(cls_wifi_hw);
	return -ENOMEM;
}

/**
 * cls_wifi_cmn_elems_allocs() - Allocate IPC storage elements.
 * @cls_wifi_hw: Main driver data
 *
 * This function allocates all the elements required for communications with
 * LMAC, such as Rx Data elements, MSGs elements, ...
 * This function should be called in correspondence with the deallocation function.
 */
static int cls_wifi_cmn_elems_allocs(struct cls_wifi_cmn_hw *cmn_hw,
			struct ipc_host_cmn_env_tag *ipc_host_cmn_env)
{
	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (cls_wifi_cmn_ipc_buf_pool_alloc(ipc_host_cmn_env, &cmn_hw->dbgbuf_pool,
								CMN_DBG_CNT,
								sizeof(struct ipc_dbg_msg),
								"cls_wifi_cmn_ipc_dbgbuf_pool",
								ipc_host_cmn_dbgbuf_push))
		goto err_alloc;

	if (cls_wifi_cmn_ipc_buf_pool_alloc(ipc_host_cmn_env, &cmn_hw->rxdesc_pool_2g,
								 CMN_RXDESC_CNT_MAX_2G,
								 sizeof(struct rxdesc_tag),
								 "cls_wifi_cmn_ipc_rxdesc_pool_2g",
								 ipc_host_cmn_rxdesc_init_push_2g))
		goto err_alloc;

	if (cls_wifi_cmn_ipc_buf_pool_alloc(ipc_host_cmn_env, &cmn_hw->rxdesc_pool_5g,
								 CMN_RXDESC_CNT_MAX_5G,
								 sizeof(struct rxdesc_tag),
								 "cls_wifi_cmn_ipc_rxdesc_pool_5g",
								 ipc_host_cmn_rxdesc_init_push_5g))
		goto err_alloc;

	if (cls_wifi_cmn_ipc_rxbuf_init(cmn_hw, (sizeof(struct hw_rxhdr) + MSDU_LENGTH))) {
		dev_err(cmn_hw->dev, "Cannot allocate the RX buffers for common wpu\n");
		goto err_alloc;
	}

	return 0;

err_alloc:
	dev_err(cmn_hw->dev, "Error while allocating IPC buffers\n");
	cls_wifi_cmn_elems_deallocs(cmn_hw);
	return -ENOMEM;
}

/**
 * cls_wifi_ipc_msg_push() - Push a msg to IPC queue
 *
 * @cls_wifi_hw: Main driver data
 * @msg_buf: Pointer to message
 * @len: Size, in bytes, of message
 */
void cls_wifi_ipc_msg_push(struct cls_wifi_hw *cls_wifi_hw, void *msg_buf, uint16_t len)
{
	ipc_host_msg_push(cls_wifi_hw->ipc_env, msg_buf, len);
}

/**
 * cls_wifi_ipc_txdesc_push() - Push a txdesc to FW
 *
 * @cls_wifi_hw: Main driver data
 * @sw_txhdr: Pointer to the SW TX header associated to the descriptor to push
 * @skb: TX Buffer associated. Pointer saved in ipc env to retrieve it upon confirmation.
 * @hw_queue: Hw queue to push txdesc to
 */
void cls_wifi_ipc_txdesc_push(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_sw_txhdr *sw_txhdr,
						  struct sk_buff *skb, int hw_queue,
						  u16 txq_credits, u16 hwq_credits, u16 host_pushed)
{
	struct txdesc_host *txdesc_host = &sw_txhdr->desc;
	struct cls_wifi_ipc_buf *ipc_desc = &sw_txhdr->ipc_desc;

	txdesc_host->ctrl.hwq = hw_queue;
	txdesc_host->api.host.hostid = ipc_host_tx_host_ptr_to_id(cls_wifi_hw->ipc_env, skb);
	txdesc_host->ready = 0xFFFFFFFF;
	if (!txdesc_host->api.host.hostid) {
		dev_err(cls_wifi_hw->dev, "No more tx_hostid available \n");
		return;
	}

	if (cls_wifi_ipc_buf_a2e_init(cls_wifi_hw, ipc_desc, txdesc_host, sizeof(*txdesc_host)))
		return ;

	cls_wifi_hw->ipc_stats.ipc_tx_push++;

	ipc_host_txdesc_push(cls_wifi_hw->ipc_env, ipc_desc, hw_queue);
}

/**
 * cls_wifi_ipc_get_skb_from_cfm() - Retrieve the TX buffer associated to a confirmation buffer
 *
 * @cls_wifi_hw: Main driver data
 * @buf: IPC buffer for the confirmation buffer
 * @return: Pointer to TX buffer associated to this confirmation and NULL if confirmation
 * has not yet been updated by firmware
 *
 * To ensure that a confirmation has been processed by firmware check if the hostid field
 * has been updated. If this is the case retrieve TX buffer from it and reset it, otherwise
 * simply return NULL.
 */
struct sk_buff *cls_wifi_ipc_get_skb_from_cfm(struct cls_wifi_hw *cls_wifi_hw,
										  struct cls_wifi_ipc_buf *buf)
{
	struct sk_buff *skb = NULL;
	struct tx_cfm_tag *cfm = buf->addr;

	/* get ownership of confirmation */
	cls_wifi_ipc_buf_e2a_sync(cls_wifi_hw->dev, buf, 0);

	/* Check host id in the confirmation. */
	/* If 0 it means that this confirmation has not yet been updated by firmware */
	if (cfm->hostid) {
		spin_lock(&cls_wifi_hw->tx_lock);
		skb = ipc_host_tx_host_id_to_ptr(cls_wifi_hw->ipc_env, cfm->hostid);
		spin_unlock(&cls_wifi_hw->tx_lock);
		if (unlikely(!skb)) {
			dev_err(cls_wifi_hw->dev, "Cannot retrieve skb from cfm=%p/0x%pad, hostid %d in confirmation\n",
					buf->addr, &buf->dma_addr, cfm->hostid);
		} else {
			/* Unmap TX descriptor */
			struct cls_wifi_txhdr * txhdr = (struct cls_wifi_txhdr *)skb->data;
			struct cls_wifi_sw_txhdr *sw_hdr;
			struct cls_wifi_ipc_buf *ipc_desc;

			sw_hdr = txhdr->sw_hdr;
			ipc_desc = &sw_hdr->ipc_desc;
			cls_wifi_ipc_buf_a2e_release(cls_wifi_hw, ipc_desc);
		}

		cls_wifi_hw->ipc_stats.ipc_tx_cfm++;
		cfm->hostid = 0;
	}

	/* always re-give ownership to firmware. */
	cls_wifi_ipc_buf_e2a_sync_back(cls_wifi_hw->dev, buf, 0);

	return skb;
}

uint32_t cls_wifi_ipc_buffered_offset_get( u16 sta_idx, uint8_t tid)
{
	return ((sta_idx * TID_MAX + tid) * sizeof(u32_l));

}

/**
 * cls_wifi_ipc_sta_buffer_init - Initialize counter of buffered data for a given sta
 *
 * @cls_wifi_hw: Main driver data
 * @sta_idx: Index of the station to initialize
 */
void cls_wifi_ipc_sta_buffer_init(struct cls_wifi_hw *cls_wifi_hw, u16 sta_idx)
{
	if (sta_idx >= hw_remote_sta_max(cls_wifi_hw))
		return;
	cls_wifi_hw->ipc_env->ops->writen(cls_wifi_hw->plat, cls_wifi_hw->radio_idx,
		cls_wifi_hw->ipc_env->buffered_offset + cls_wifi_ipc_buffered_offset_get(sta_idx, 0), NULL, TID_MAX);
}

/**
 * cls_wifi_ipc_sta_buffer - Update counter of buffered data for a given sta
 *
 * @cls_wifi_hw: Main driver data
 * @sta: Managed station
 * @tid: TID on which data has been added or removed
 * @size: Size of data to add (or remove if < 0) to STA buffer.
 */
void cls_wifi_ipc_sta_buffer(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_sta *sta, int tid, int size)
{
#if 0
	if (!sta)
		return;

	if ((sta->sta_idx >= hw_remote_sta_max(cls_wifi_hw)) || (tid >= TID_MAX))
		return;

	if (size < 0) {
		size = -size;
		if (cls_wifi_hw->ipc_env->ops->read32(cls_wifi_hw->plat, cls_wifi_hw->radio_idx,
				cls_wifi_ipc_buffered_offset_get(sta->sta_idx, tid)) < size)
			cls_wifi_hw->ipc_env->ops->write32(cls_wifi_hw->plat, cls_wifi_hw->radio_idx,
					cls_wifi_ipc_buffered_offset_get(sta->sta_idx, tid), 0);
		else
			cls_wifi_hw->ipc_env->ops->write32(cls_wifi_hw->plat, cls_wifi_hw->radio_idx,
					cls_wifi_ipc_buffered_offset_get(sta->sta_idx, tid),
					cls_wifi_hw->ipc_env->ops->read32(cls_wifi_hw->plat, cls_wifi_hw->radio_idx,
					cls_wifi_ipc_buffered_offset_get(sta->sta_idx, tid)) - size);
	} else {
		// no test on overflow
		cls_wifi_hw->ipc_env->ops->write32(cls_wifi_hw->plat, cls_wifi_hw->radio_idx,
				cls_wifi_ipc_buffered_offset_get(sta->sta_idx, tid),
				cls_wifi_hw->ipc_env->ops->read32(cls_wifi_hw->plat, cls_wifi_hw->radio_idx,
				cls_wifi_ipc_buffered_offset_get(sta->sta_idx, tid)) + size);
	}
#endif
}

/**
 * cls_wifi_msgind() - IRQ handler callback for %IPC_IRQ_E2A_MSG
 *
 * @pthis: Pointer to main driver data
 * @arg: Pointer to IPC buffer from msgbuf_pool
 */
static u8 cls_wifi_msgind(void *pthis, void *arg)
{
	struct cls_wifi_hw *cls_wifi_hw = pthis;
	struct cls_wifi_ipc_buf *buf = arg;
	struct ipc_e2a_msg *msg = buf->addr;
	u8 ret = 0;

	REG_SW_SET_PROFILING(cls_wifi_hw, SW_PROF_MSGIND);

	if (!cls_wifi_hw) {
		ret = -1;
		goto msg_no_push;
	}

	/* Look for pattern which means that this hostbuf has been used for a MSG */
	if (msg->pattern != IPC_MSGE2A_VALID_PATTERN) {
		ret = -1;
		goto msg_no_push;
	}
	/* Relay further actions to the msg parser */
	cls_wifi_rx_handle_msg(cls_wifi_hw, msg);

	/* Reset the msg buffer and re-use it */
	msg->pattern = 0;
	wmb();

	/* Push back the buffer to the LMAC */
	ipc_host_msgbuf_push(cls_wifi_hw->ipc_env, buf);

msg_no_push:
	REG_SW_CLEAR_PROFILING(cls_wifi_hw, SW_PROF_MSGIND);
	return ret;
}

/**
 * cls_wifi_msgackind() - IRQ handler callback for %IPC_IRQ_E2A_MSG_ACK
 *
 * @pthis: Pointer to main driver data
 * @hostid: Pointer to command acknowledged
 */
static u8 cls_wifi_msgackind(void *pthis, void *hostid)
{
	struct cls_wifi_hw *cls_wifi_hw = (struct cls_wifi_hw *)pthis;
	cls_wifi_hw->cmd_mgr.llind(&cls_wifi_hw->cmd_mgr, (struct cls_wifi_cmd *)hostid);
	return -1;
}

/**
 * cls_wifi_radarind() - IRQ handler callback for %IPC_IRQ_E2A_RADAR
 *
 * @pthis: Pointer to main driver data
 * @arg: Pointer to IPC buffer from radar_pool
 */
static u8 cls_wifi_radarind(void *pthis, void *arg)
{
#ifdef CONFIG_CLS_WIFI_RADAR
	struct cls_wifi_hw *cls_wifi_hw = pthis;
	struct cls_wifi_ipc_buf *buf = arg;
	struct radar_pulse_array_desc *pulses = buf->addr;
	u8 ret = 0;

	if (cls_wifi_hw == NULL) {
		ret = -1;
		goto radar_no_push;
	}

	/* Look for pulse count meaning that this hostbuf contains RADAR pulses */
	if (pulses->cnt == 0) {
		ret = -1;
		goto radar_no_push;
	}

	//TODO: repeater will support radar_detected in next phase
	if (pulses->cnt <= RADAR_PULSE_MAX && !cls_wifi_is_repeater_mode(cls_wifi_hw)) {
		if (cls_wifi_hw->radio_params->debug_mode || cls_wifi_radar_detection_is_enable(&cls_wifi_hw->radar)) {
			/* Save the received pulses only if radar detection is enabled */
			struct cls_wifi_radar_pulses *p = &cls_wifi_hw->radar.pulses;

			memcpy(&p->buffer[p->index], buf->addr, sizeof(struct radar_pulse_array_desc));
			p->index = (p->index + 1) % CLS_WIFI_RADAR_PULSE_MAX;
			if (p->count < CLS_WIFI_RADAR_PULSE_MAX)
				p->count++;

			/* Defer pulse processing in separate work */
			if (! work_pending(&cls_wifi_hw->radar.detection_work))
				schedule_work(&cls_wifi_hw->radar.detection_work);
		}
	}

	/* Reset the radar bufent and re-use it */
	pulses->cnt = 0;
	wmb();

	/* Push back the buffer to the LMAC */
	ipc_host_radar_push(cls_wifi_hw->ipc_env, buf);

radar_no_push:
	return ret;
#else
	return -1;
#endif
}

/**
 * cls_wifi_hemumapind() - IRQ handler callback for %IPC_IRQ_E2A_HE_MU_DESC
 *
 * @pthis: Pointer to main driver data
 * @arg: Pointer to IPC buffer from hemu_pool
 */
static u8 cls_wifi_hemumapind(void *pthis, void *arg)
{
	struct cls_wifi_hw *cls_wifi_hw = pthis;
	struct cls_wifi_ipc_buf *buf = arg;
	struct he_mu_map_array_desc *map = buf->addr;

	cls_wifi_he_mu_process_dl_map(cls_wifi_hw, map);

	/* Reset the he mu buffer content and re-use it */
	map->cnt_sta = 0;
	wmb();

	/* Push back the buffer to the LMAC */
	ipc_host_he_mu_map_push(cls_wifi_hw->ipc_env, buf);
	return 0;
}

/**
 * cls_wifi_dbgind() - IRQ handler callback for %IPC_IRQ_E2A_DBG
 *
 * @pthis: Pointer to main driver data
 * @hostid: Pointer to IPC buffer from dbgbuf_pool
 */
static u8 cls_wifi_dbgind(void *pthis, void *arg)
{
	struct cls_wifi_hw *cls_wifi_hw = (struct cls_wifi_hw *)pthis;
	struct cls_wifi_ipc_buf *buf = arg;
	struct ipc_dbg_msg *dbg_msg = buf->addr;
	u8 ret = 0;

	REG_SW_SET_PROFILING(cls_wifi_hw, SW_PROF_DBGIND);
	barrier();
	/* Look for pattern which means that this hostbuf has been used for a MSG */
	if (dbg_msg->pattern != IPC_DBG_VALID_PATTERN) {
		ret = -1;
		goto dbg_no_push;
	}

	printk("%s%d %s", (char *)FW_STR, cls_wifi_hw->radio_idx, (char *)dbg_msg->string);

	/* Reset the msg buffer and re-use it */
	dbg_msg->pattern = 0;
	wmb();

	/* Push back the buffer to the LMAC */
	ipc_host_dbgbuf_push(cls_wifi_hw->ipc_env, buf);

dbg_no_push:
	REG_SW_CLEAR_PROFILING(cls_wifi_hw, SW_PROF_DBGIND);

	return ret;
}

/**
 * cls_wifi_cmn_dbgind() - IRQ handler callback for %IPC_IRQ_E2A_DBG
 *
 * @pthis: Pointer to main driver data
 * @hostid: Pointer to IPC buffer from dbgbuf_pool
 */
u8 cls_wifi_cmn_dbgind(void *pthis, void *arg)
{
	struct cls_wifi_hw *cls_wifi_hw = (struct cls_wifi_hw *)pthis;
	struct cls_wifi_ipc_buf *buf = arg;
	struct ipc_dbg_msg *dbg_msg = buf->addr;
	u8 ret = 0;
	struct ipc_host_cmn_env_tag *env = cls_wifi_hw->ipc_cmn_env;

	///pr_warn("%s dbgbuf_idx: %u,dbg_msg->pattern: 0x%x\n", __func__, env->dbgbuf_idx, dbg_msg->pattern);

	barrier();
	/* Look for pattern which means that this hostbuf has been used for a MSG */
	if (dbg_msg->pattern != IPC_DBG_CMN_VALID_PATTERN) {
		ret = -1;
		goto dbg_no_push;
	}

	printk("%s%d(%d) %s", (char *)FW_STR, env->radio_idx, cls_wifi_hw->radio_idx, (char *)dbg_msg->string);

	/* Reset the msg buffer and re-use it */
	dbg_msg->pattern = 0;
	wmb();

	/* Push back the buffer to the LMAC */
	ipc_host_cmn_dbgbuf_push(cls_wifi_hw->ipc_cmn_env, buf);

dbg_no_push:

	return ret;
}

static uint8_t cls_wifi_csi_ind(void *pthis, void *arg)
{
	struct cls_wifi_hw *cls_wifi_hw = pthis;
	struct cls_wifi_ipc_buf *buf = arg;
	struct cls_csi_report *report;

	report = (struct cls_csi_report *)buf->addr;
	cls_wifi_process_csi_report(cls_wifi_hw, report);
	wmb();

	/* Push back the buffer to the LMAC */
	ipc_host_csi_buf_push(cls_wifi_hw->ipc_env, buf);

	return 0;
}

static uint8_t cls_wifi_atf_stats_ind(void *pthis, void *arg)
{
	struct cls_wifi_hw *cls_wifi_hw = pthis;
	struct cls_wifi_ipc_buf *buf = arg;

	if (buf == NULL) {
		pr_info("%s buf is NULL\n", __func__);

		return -1;
	}

	cls_wifi_atf_stats_process(cls_wifi_hw);

	return 0;
}

/**
 * cls_wifi_ipc_rxbuf_init() - Allocate and initialize RX buffers.
 *
 * @cls_wifi_hw: Main driver data
 * @rxbuf_sz: Size of the buffer to be allocated
 *
 * This function updates the RX buffer size according to the parameter and allocates the
 * RX buffers
 */
int cls_wifi_ipc_rxbuf_init(struct cls_wifi_hw *cls_wifi_hw, uint32_t rxbuf_sz)
{
	cls_wifi_hw->ipc_env->rxbuf_sz = rxbuf_sz;
	return cls_wifi_ipc_rxbufs_alloc(cls_wifi_hw);
}

int cls_wifi_cmn_ipc_rxbuf_init(struct cls_wifi_cmn_hw *cmn_hw, uint32_t rxbuf_sz)

{
	cmn_hw->ipc_host_cmn_env->rxbuf_sz = rxbuf_sz;
	return cls_wifi_cmn_ipc_rxbufs_alloc(cmn_hw);
}

/**
 * cls_wifi_ipc_init() - Initialize IPC interface.
 *
 * @cls_wifi_hw: Main driver data
 * @shared_ram: Pointer to shared memory that contains IPC shared struct
 *
 * This function initializes IPC interface by registering callbacks, setting
 * shared memory area and calling IPC Init function.
 * It should be called only once during driver's lifetime.
 */
int cls_wifi_ipc_init(struct cls_wifi_hw *cls_wifi_hw, int stage)
{
	struct ipc_host_cb_tag cb;
	uint32_t txcfm_nb;
	uint32_t rxdesc_nb;
	uint32_t ipc_env_size;
	int res = 0;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (stage == 1) {
		if (cls_wifi_ipc_buf_pool_alloc(cls_wifi_hw, &cls_wifi_hw->rxdesc_pool,
				cls_wifi_hw->ipc_env->rxdesc_nb,
				sizeof(struct rxdesc_tag),
				"cls_wifi_ipc_rxdesc_pool",
				ipc_host_rxdesc_init_push)) {
			cls_wifi_ipc_buf_pool_dealloc(&cls_wifi_hw->rxdesc_pool);
			return -ENOMEM;
		}
		return 0;
	}

	/* initialize the API interface */
	cb.recv_data_ind		 = cls_wifi_rxdataind;
	cb.recv_radar_ind		= cls_wifi_radarind;
	cb.recv_he_mu_map_ind	= cls_wifi_hemumapind;
	cb.recv_msg_ind		  = cls_wifi_msgind;
	cb.recv_msgack_ind	   = cls_wifi_msgackind;
	cb.recv_dbg_ind		  = cls_wifi_dbgind;
	cb.send_data_cfm		 = cls_wifi_txdatacfm;
	cb.recv_unsup_rx_vec_ind = cls_wifi_unsup_rx_vec_ind;
	cb.recv_csi_ind		= cls_wifi_csi_ind;
	cb.recv_atf_stats_ind = cls_wifi_atf_stats_ind;

	txcfm_nb = ipc_host_get_txcfm_nb(cls_wifi_hw);
	rxdesc_nb = ipc_host_get_rxdesc_nb(cls_wifi_hw);
	ipc_env_size = sizeof(struct ipc_host_env_tag) + txcfm_nb * sizeof(struct ipc_hostid) +
			txcfm_nb * sizeof(struct cls_wifi_ipc_buf *) +
			rxdesc_nb * sizeof(struct cls_wifi_ipc_buf *);
	/* set the IPC environment */
	cls_wifi_hw->ipc_env = (struct ipc_host_env_tag *)kzalloc(ipc_env_size, GFP_KERNEL);

	if (!cls_wifi_hw->ipc_env)
		return -ENOMEM;

	/* call the initialization of the IPC */
	ipc_host_init(cls_wifi_hw->ipc_env, &cb, cls_wifi_hw);

	cls_wifi_cmd_mgr_init(&cls_wifi_hw->cmd_mgr);

	res = cls_wifi_elems_allocs(cls_wifi_hw);
	if (res) {
		kfree(cls_wifi_hw->ipc_env);
		cls_wifi_hw->ipc_env = NULL;
	}

	return res;
}

/**
 * cls_wifi_cmn_ipc_init() - Initialize IPC interface.
 *
 * @cls_wifi_hw: Main driver data
 * @shared_ram: Pointer to shared memory that contains IPC shared struct
 *
 * This function initializes IPC interface by registering callbacks, setting
 * shared memory area and calling IPC Init function.
 * It should be called only once during driver's lifetime.
 */
int cls_wifi_cmn_ipc_init(struct cls_wifi_plat *cls_wifi_plat,
		struct ipc_host_cmn_env_tag *ipc_host_cmn_env)
{
	struct ipc_host_cb_tag cb;
	int res = 0;
	struct cls_wifi_cmn_hw *cmn_hw = cls_wifi_plat->cmn_hw;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* initialize the API interface */
	cb.recv_data_ind		 = cls_wifi_rxdataind;
	cb.recv_radar_ind		= NULL;
	cb.recv_he_mu_map_ind	= NULL;
	cb.recv_msg_ind		  = NULL;
	cb.recv_msgack_ind	   = NULL;
	cb.recv_dbg_ind		  = cls_wifi_cmn_dbgind;
	cb.send_data_cfm		 = NULL;
	cb.recv_unsup_rx_vec_ind = NULL;
	cb.recv_csi_ind		= NULL;
	cb.recv_atf_stats_ind = NULL;

	/* set the IPC environment */
	///cls_wifi_hw->ipc_cmn_env = ipc_host_cmn_env;

	if (!ipc_host_cmn_env)
		return -ENOMEM;
	// Save the callbacks in our own environment
	ipc_host_cmn_env->cb = cb;

	res = cls_wifi_cmn_elems_allocs(cmn_hw, ipc_host_cmn_env);
	if (res) {
		///TODO:
		pr_err("cls_wifi_cmn_elems_allocs error: %d\n",res);
	}

	return res;
}

/**
 * cls_wifi_cmn_ipc_deinit() - Deinitialize IPC interface.
 *
 * This function deinitializes buffers of cmn hw
 */
void cls_wifi_cmn_ipc_deinit(struct cls_wifi_plat *cls_wifi_plat)
{
	struct cls_wifi_cmn_hw *cmn_hw = cls_wifi_plat->cmn_hw;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	cls_wifi_cmn_elems_deallocs(cmn_hw);
}

/**
 * cls_wifi_ipc_deinit() - Release IPC interface
 *
 * @cls_wifi_hw: Main driver data
 */
void cls_wifi_ipc_deinit(struct cls_wifi_hw *cls_wifi_hw)
{
	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	cls_wifi_ipc_tx_drain(cls_wifi_hw);
	cls_wifi_cmd_mgr_deinit(&cls_wifi_hw->cmd_mgr);
	cls_wifi_ipc_buf_pool_dealloc(&cls_wifi_hw->rxdesc_pool);
	cls_wifi_elems_deallocs(cls_wifi_hw);
	if (cls_wifi_hw->ipc_env) {
		kfree(cls_wifi_hw->ipc_env);
		cls_wifi_hw->ipc_env = NULL;
	}
}

/**
 * cls_wifi_ipc_start() - Start IPC interface
 *
 * @cls_wifi_hw: Main driver data
 */
void cls_wifi_ipc_start(struct cls_wifi_hw *cls_wifi_hw)
{
	ipc_host_enable_irq(cls_wifi_hw->ipc_env, IPC_IRQ_E2A_ALL);
}

/**
 * cls_wifi_ipc_stop() - Stop IPC interface
 *
 * @cls_wifi_hw: Main driver data
 */
void cls_wifi_ipc_stop(struct cls_wifi_hw *cls_wifi_hw)
{
	ipc_host_disable_irq(cls_wifi_hw->ipc_env, IPC_IRQ_E2A_ALL);
}

/**
 * cls_wifi_ipc_tx_drain() - Flush IPC TX buffers
 *
 * @cls_wifi_hw: Main driver data
 *
 * This assumes LMAC is still (tx wise) and there's no TX race until LMAC is up
 * tx wise.
 * This also lets both IPC sides remain in sync before resetting the LMAC,
 * e.g with cls_wifi_send_reset.
 */
void cls_wifi_ipc_tx_drain(struct cls_wifi_hw *cls_wifi_hw)
{
	struct sk_buff *skb;
	int count = 0;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (!cls_wifi_hw->ipc_env) {
		printk(KERN_CRIT "%s: bypassing (restart must have failed)\n", __func__);
		return;
	}

	while ((skb = ipc_host_tx_flush(cls_wifi_hw->ipc_env))) {
		struct cls_wifi_sw_txhdr *sw_txhdr = ((struct cls_wifi_txhdr *)skb->data)->sw_hdr;

#ifdef CONFIG_CLS_WIFI_AMSDUS_TX
		if (sw_txhdr->desc.api.host.packet_cnt > 1) {
			struct cls_wifi_amsdu_txhdr *amsdu_txhdr , *next;
			list_for_each_entry_safe(amsdu_txhdr, next, &sw_txhdr->amsdu.hdrs, list) {
				cls_wifi_ipc_buf_a2e_release(cls_wifi_hw, &amsdu_txhdr->ipc_data);
				dev_kfree_skb_any(amsdu_txhdr->skb);
			}
		}
#endif
		count++;
		cls_wifi_ipc_buf_a2e_release(cls_wifi_hw, &sw_txhdr->ipc_data);
		kmem_cache_free(cls_wifi_hw->sw_txhdr_cache, sw_txhdr);
		skb_pull(skb, CLS_WIFI_TX_HEADROOM);
		dev_kfree_skb_any(skb);
	}
	if (count > 0)
		printk(KERN_CRIT "%s: flush skb %d\n", __func__, count);
}

/**
 * cls_wifi_ipc_tx_pending() - Check if TX frames are pending at FW level
 *
 * @cls_wifi_hw: Main driver data
 */
bool cls_wifi_ipc_tx_pending(struct cls_wifi_hw *cls_wifi_hw)
{
	return ipc_host_tx_frames_pending(cls_wifi_hw->ipc_env);
}

/**
 * cls_wifi_error_ind() - %DBG_ERROR_IND message callback
 *
 * @cls_wifi_hw: Main driver data
 *
 * This function triggers the UMH script call that will indicate to the user
 * space the error that occurred and stored the debug dump. Once the UMH script
 * is executed, the cls_wifi_umh_done() function has to be called.
 */
void cls_wifi_error_ind(struct cls_wifi_hw *cls_wifi_hw)
{
	struct cls_wifi_ipc_buf *buf = &cls_wifi_hw->dbgdump.buf;
	struct dbg_debug_dump_tag *dump = buf->addr;

	cls_wifi_ipc_buf_e2a_sync(cls_wifi_hw->dev, buf, 0);
	dev_err(cls_wifi_hw->dev, "(type %d): dump received\n",
			dump->dbg_info.error_type);
	cls_wifi_hw->debugfs.trace_prst = true;
	cls_wifi_trigger_um_helper(&cls_wifi_hw->debugfs);
}

/**
 * cls_wifi_umh_done() - Indicate User Mode helper finished
 *
 * @cls_wifi_hw: Main driver data
 *
 */
void cls_wifi_umh_done(struct cls_wifi_hw *cls_wifi_hw)
{
	if (!test_bit(CLS_WIFI_DEV_STARTED, &cls_wifi_hw->flags))
		return;

	/* this assumes error_ind won't trigger before ipc_host_dbginfo_push
	   is called and so does not irq protect (TODO) against error_ind */
	cls_wifi_hw->debugfs.trace_prst = false;
	ipc_host_dbginfo_push(cls_wifi_hw->ipc_env, &cls_wifi_hw->dbgdump.buf);
}
