// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 1999 - 2006 Intel Corporation. */

#include <malloc.h>
#include <linux/dma-mapping.h>
#include "cls_npe.h"

void cls_unmap_tx_resource(struct udevice *dev,
				 struct cls_tx_buffer *buffer_info)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);

	if (buffer_info->dma) {
		dma_unmap_single(buffer_info->dma,
				 buffer_info->length,
				 DMA_TO_DEVICE);
	}

	buffer_info->dma = 0;
	buffer_info->length = 0;
	buffer_info->rx_description1 = 0;
	priv_data->total_tx_unmap_cnt += 1;
}

/**
 * cls_reset_tx_ring - Reset Tx Buffers
 * @tx_ring: ring to be reset
 **/
static void cls_reset_tx_ring(struct udevice *dev,
				struct cls_tx_ring *tx_ring)
{
	struct cls_tx_buffer *buffer_info;
	unsigned long size;
	unsigned int i;

	/* Free all the Tx ring sk_buffs */
	for (i = 0; i < tx_ring->count; i++) {
		buffer_info = &tx_ring->buffer_info[i];
		cls_unmap_tx_resource(dev, buffer_info);
	}

	size = sizeof(struct cls_tx_buffer) * tx_ring->count;
	memset(tx_ring->buffer_info, 0, size);

	tx_ring->next_to_use = 0;
	tx_ring->next_to_clean = 0;
}

/**
 * cls_free_tx_resources - Free all Tx Resources per Queue
 * Free all transmit software resources
 **/
static void cls_free_tx_resources(struct udevice *dev,
				    struct cls_tx_ring *tx_ring)
{
	cls_reset_tx_ring(dev, tx_ring);

	free(tx_ring->buffer_info);
	tx_ring->buffer_info = NULL;
}

/**
 * cls_free_all_tx_resources - Free all Tx Resources for All Queues
 * Free all transmit software resources
 **/
void cls_free_all_tx_resources(struct udevice *dev)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	struct cls_tx_ring *txdr = &priv_data->tx_ring;

	cls_free_tx_resources(dev, txdr);
}

/**
 * cls_eth_setup_all_tx_resources
 *		- allocate Tx resources for all queues(without ring_buffer)
 * Return 0 on success, negative on failure
 **/
static int cls_eth_setup_all_tx_resources(struct udevice *dev)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	struct cls_tx_ring *txdr = &priv_data->tx_ring;
	int size;

	size = sizeof(struct cls_tx_buffer) * (txdr->count);

	txdr->buffer_info = memalign(64, size);
	if (!txdr->buffer_info) {
		cls_eth_print("[%s] alloc tx_buffer failed!\n", __func__);
		return -ENOMEM;
	}

	memset(txdr->buffer_info, 0, size);
	txdr->next_to_use = 0;
	txdr->next_to_clean = 0;

	return 0;
}

/*RX buffer: CPU_TAG + eth_packer*/
static void *cls_alloc_frag(struct udevice *dev)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	unsigned int len = ALIGN(priv_data->rx_buffer_len + CLS_NPE_TO_CPU_TAG_HLEN, 64);
	u8 *data = memalign(64, len);

	if (data)
		memset((void *)data, 0, len);

	return data;
}

/**
 * cls_sw_bmu_alloc_rx_buffers - alloc rx ring buffers from next_to_use
 * @rx_ring: pointer to ring struct
 * @cleaned_count: number of new Rx buffers to try to allocate
 **/
static int cls_sw_bmu_alloc_rx_buffers(struct udevice *dev,
				   struct cls_rx_ring *rx_ring,
				   int cleaned_count)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	struct cls_rx_buffer *buffer_info;
	unsigned int i;
	static u16 packet_id;

	i = rx_ring->next_to_use;
	buffer_info = &rx_ring->buffer_info[i];

	while (cleaned_count--) {
		void *data;

		priv_data->total_alloc_map_rx_buff_cnt += 1;

		if (buffer_info->data) {
			priv_data->total_alloc_rx_buff_skip += 1;
			cls_eth_print("[%s] skip alloc rx_buff:index[%d]! data=0x%p\n", __func__, i, buffer_info->data);
			return -1;
		}

		data = cls_alloc_frag(dev);
		if (!data) {
			priv_data->total_alloc_rx_buff_failed += 1;
			cls_eth_print("[%s] alloc rxbuf failed!\n", __func__);
			break;
		}

		buffer_info->dma = dma_map_single(data,
						  ALIGN(priv_data->rx_buffer_len + CLS_NPE_TO_CPU_TAG_HLEN, 64),
						  DMA_FROM_DEVICE);
		if (dma_mapping_error(dev, buffer_info->dma)) {
			free(data);
			buffer_info->dma = 0;
			priv_data->total_map_rx_buff_failed += 1;
			cls_eth_print("[%s] dma_map_single failed!\n", __func__);
			break;
		}

		buffer_info->data = data;
		priv_data->total_alloc_map_rx_buff_succ += 1;

		if (!priv_data->split_mode) {
#ifdef CLS_NPE_DEBUG
		if (g_debug_cls)
			cls_eth_print("[%s] pktid=%d data=0x%p \n", __func__, packet_id, buffer_info->data);
#endif
			cls_edma_bmu_config(dev, packet_id, 0, buffer_info->dma);
			packet_id++;
		}
//skip:
		if (unlikely(++i == rx_ring->count))
			i = 0;
		buffer_info = &rx_ring->buffer_info[i];
	}

	// check if any rx_buff have been alloced
	if (likely(rx_ring->next_to_use != i)) {
		rx_ring->next_to_use = i;
		dmb();
	}

	return 0;
}

/**
 * cls_reset_rx_ring - Reset Rx Buffers per Queue
 * @rx_ring: ring to be reset
 **/
static void cls_reset_rx_ring(struct udevice *dev,
				struct cls_rx_ring *rx_ring)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	struct cls_rx_buffer *buffer_info;
	unsigned long size;
	unsigned int i;

	/* Free all the Rx netfrags */
	for (i = 0; i < rx_ring->count; i++) {
		buffer_info = &rx_ring->buffer_info[i];
		if (buffer_info->dma)
			dma_unmap_single(buffer_info->dma,
					 ALIGN(priv_data->rx_buffer_len + CLS_NPE_TO_CPU_TAG_HLEN, 64),
					 DMA_FROM_DEVICE);

		if (buffer_info->data)
			free(buffer_info->data);

		buffer_info->data = NULL;
		buffer_info->dma = 0;
	}

	size = sizeof(struct cls_rx_buffer) * rx_ring->count;
	memset(rx_ring->buffer_info, 0, size);

	rx_ring->next_to_clean = 0;
	rx_ring->next_to_use = 0;
}

/**
 * cls_free_rx_resources - Free Rx Resources per Queue
 * @rx_ring: ring to free the resources from
 *
 * Free all receive software resources
 **/
static void cls_free_rx_resources(struct udevice *dev,
				    struct cls_rx_ring *rx_ring)
{
	cls_reset_rx_ring(dev, rx_ring);

	free(rx_ring->buffer_info);
	rx_ring->buffer_info = NULL;
}

/**
 * cls_free_all_rx_resources - Free All Rx Resources for All Queues
 * Free all receive software resources
 **/
void cls_free_all_rx_resources(struct udevice *dev)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	struct cls_rx_ring *rxdr = &priv_data->rx_ring;

	cls_free_rx_resources(dev, rxdr);
}

/**
 * cls_eth_setup_all_rx_resources
 *		- allocate Rx resources for all queues(without ring_buffer)
 * Return 0 on success, negative on failure
 **/
static int cls_eth_setup_all_rx_resources(struct udevice *dev)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	struct cls_rx_ring *rxdr = &priv_data->rx_ring;
	int size;

	size = sizeof(struct cls_rx_buffer) * (rxdr->count);

	rxdr->buffer_info = memalign(64, size);
	if (!rxdr->buffer_info) {
		cls_eth_print("[%s] alloc rx_buffer failed!\n", __func__);
		return -ENOMEM;
	}

	memset(rxdr->buffer_info, 0, size);
	rxdr->next_to_clean = 0;
	rxdr->next_to_use = 0;

	return 0;
}

static void cls_eth_sw_init(struct udevice *dev)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);

	priv_data->rx_buffer_len = MAXIMUM_ETHERNET_VLAN_SIZE;
	priv_data->alloc_rx_buf = cls_sw_bmu_alloc_rx_buffers;

	// Only support num_tx_queues=1/num_rx_queue=1
	priv_data->tx_ring.count = priv_data->lhost_fifo_depth * CLS_NPE_FIFO_DEPTH_LEVEL;
	priv_data->rx_ring.count = priv_data->lhost_fifo_depth * CLS_NPE_FIFO_DEPTH_LEVEL;
}

static int cls_eth_configure_rx_buffer(struct udevice *dev)
{
	struct cls_eth_priv *priv_data = dev_get_priv(dev);
	struct cls_rx_ring *rxdr = &priv_data->rx_ring;

	return priv_data->alloc_rx_buf(dev, rxdr, CLS_NPE_DESC_UNUSED(rxdr));
}

void cls_eth_free_bmu(struct udevice *dev)
{
	cls_free_all_tx_resources(dev);
	cls_free_all_rx_resources(dev);
}

unsigned int cls_eth_soft_bmu_init(struct udevice *dev)
{
	int err;

	/* configure buffer param */
	cls_eth_sw_init(dev);

	/* alloc tx buffer_info: fifo_depth(dts)*level(16) */
	err = cls_eth_setup_all_tx_resources(dev);
	if (err)
		return err;

	/* alloc rx buffer_info: fifo_depth(dts)*level(16) */
	err = cls_eth_setup_all_rx_resources(dev);
	if (err) {
		cls_free_all_tx_resources(dev);
		return err;
	}

	/* alloc and map rx buffer_info->data: CPU_TAG(24) + rx_buffer_len(1152) */
	err = cls_eth_configure_rx_buffer(dev);
	if (err) {
		cls_eth_free_bmu(dev);
		return err;
	}

#ifdef CLS_NPE_DEBUG
	if (g_debug_cls)
		cls_eth_print("[%s] done!\n", __func__);
#endif
	return 0;
}
