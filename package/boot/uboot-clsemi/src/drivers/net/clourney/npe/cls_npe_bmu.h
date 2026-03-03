/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 1999 - 2006 Clourneysemi Corporation. */

#ifndef _CLS_NPE_BMU_H_
#define _CLS_NPE_BMU_H_
#include "cls_npe_tag.h"
#include "cls_npe.h"

/* wrapper around a pointer to a socket buffer,
 * so a DMA handle can be stored along with the buffer
 */
struct cls_tx_buffer {
	dma_addr_t dma;
	u16 length;
	unsigned int rx_description1;
};

struct cls_rx_buffer {
	u8 *data; /* unsupport jumbo */
	dma_addr_t dma;
};

struct cls_tx_ring {
	/* number of descriptors in the ring */
	unsigned int count;
	/* next descriptor to associate a buffer with */
	unsigned int next_to_use;
	/* next descriptor to check for DD status bit */
	unsigned int next_to_clean;
	/* array of buffer information structs */
	struct cls_tx_buffer *buffer_info;
};

struct cls_rx_ring {
	/* number of descriptors in the ring */
	unsigned int count;
	/* next descriptor to associate a buffer with */
	unsigned int next_to_use;
	/* next descriptor to check for DD status bit */
	unsigned int next_to_clean;
	/* array of buffer information structs */
	struct cls_rx_buffer *buffer_info;
};

#define CLS_NPE_DESC_UNUSED(R)				\
({								\
	unsigned int clean = (R)->next_to_clean;		\
	unsigned int use = (R)->next_to_use;			\
	(clean > use ? 0 : (R)->count) + clean - use - 1;	\
})

/* this is the size past which hardware will drop packets when setting LPE=0 */
#define MAXIMUM_ETHERNET_VLAN_SIZE      1522

unsigned int cls_eth_soft_bmu_init(struct udevice *dev);
void cls_eth_free_bmu(struct udevice *dev);
void cls_unmap_tx_resource(struct udevice *dev,
				 struct cls_tx_buffer *buffer_info);
void cls_free_all_tx_resources(struct udevice *dev);
void cls_free_all_rx_resources(struct udevice *dev);
#endif
