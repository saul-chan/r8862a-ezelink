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

#ifndef _CLS_WIFI_UTILS_H_
#define _CLS_WIFI_UTILS_H_
#ifdef __KERNEL__
#include <linux/dma-mapping.h>
#include <linux/dmapool.h>
#include <linux/skbuff.h>

#include "ipc_host.h"
#include "lmac_msg.h"

#ifdef CONFIG_CLS_WIFI_DBG
#define CLS_WIFI_DBG(format, arg...) pr_debug(format, ## arg)
#define CLS_WIFI_INFO(format, arg...) pr_info(format, ## arg)
#else
#define CLS_WIFI_DBG(a...) do {} while (0)
#define CLS_WIFI_INFO(a...) do {} while (0)
#endif

#define CLS_WIFI_FN_ENTRY_STR ">>> %s()\n", __func__

enum cls_wifi_dev_flag {
	CLS_WIFI_DEV_RESTARTING,
	CLS_WIFI_DEV_STACK_RESTARTING,
	CLS_WIFI_DEV_STARTED,
	CLS_WIFI_DEV_ADDING_STA,
};

struct cls_wifi_hw;
struct cls_wifi_sta;
struct cls_wifi_sw_txhdr;
#endif
/**
 * struct cls_wifi_ipc_buf - Generic IPC buffer
 * An IPC buffer is a buffer allocated in host memory and "DMA mapped" to be
 * accessible by the firmware.
 *
 * @addr: Host address of the buffer. If NULL other field are invalid
 * @dma_addr: DMA address of the buffer.
 * @size: Size, in bytes, of the buffer
 */
struct cls_wifi_ipc_buf
{
	void *addr;
	dma_addr_t dma_addr;
	size_t size;
};

#ifdef __KERNEL__
/**
 * struct cls_wifi_ipc_buf_pool - Generic pool of IPC buffers
 *
 * @nb: Number of buffers currently allocated in the pool
 * @buffers: Array of buffers (size of array is @nb)
 * @pool: DMA pool in which buffers have been allocated
 */
struct cls_wifi_ipc_buf_pool {
	int nb;
	struct cls_wifi_ipc_buf *buffers;
	struct dma_pool *pool;
};

/**
 * struct cls_wifi_ipc_dbgdump - IPC buffer for debug dump
 *
 * @mutex: Mutex to protect access to debug dump
 * @buf: IPC buffer
 */
struct cls_wifi_ipc_dbgdump {
	struct mutex mutex;
	struct cls_wifi_ipc_buf buf;
};

static const u32 cls_wifi_thd_pattern = 0xDABEAD02;
static const u32 cls_wifi_tbd_pattern = 0xAD03;

/*
 * Maximum Length of Radiotap header vendor specific data(in bytes)
 */
#define RADIOTAP_HDR_VEND_MAX_LEN   16

/*
 * Maximum Radiotap Header Length without vendor specific data (in bytes)
 */
#define RADIOTAP_HDR_MAX_LEN		80

/*
 * Unsupported HT Frame data length (in bytes)
 */
#define UNSUP_RX_VEC_DATA_LEN	   2

#define MSDU_LENGTH	2304


/**
 * IPC environment control
 */
int cls_wifi_ipc_init(struct cls_wifi_hw *cls_wifi_hw, int stage);
int cls_wifi_cmn_ipc_init(struct cls_wifi_plat *cls_wifi_plat,
		struct ipc_host_cmn_env_tag *ipc_host_cmn_env);
void cls_wifi_cmn_ipc_deinit(struct cls_wifi_plat *cls_wifi_plat);
int cls_wifi_cmn_platform_on(struct cls_wifi_plat *cls_wifi_plat, void *config);
void cls_wifi_cmn_platform_off(struct cls_wifi_plat *cls_wifi_plat);
void cls_wifi_ipc_deinit(struct cls_wifi_hw *cls_wifi_hw);
void cls_wifi_ipc_start(struct cls_wifi_hw *cls_wifi_hw);
void cls_wifi_ipc_stop(struct cls_wifi_hw *cls_wifi_hw);
void cls_wifi_ipc_msg_push(struct cls_wifi_hw *cls_wifi_hw, void *msg_buf, uint16_t len);

/**
 * IPC buffer management
 */
int cls_wifi_ipc_buf_alloc(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_ipc_buf *buf,
					   size_t buf_size, enum dma_data_direction dir, const void *init);
/**
 * cls_wifi_ipc_buf_e2a_alloc() - Allocate an Embedded To Application Input IPC buffer
 *
 * @cls_wifi_hw: Main driver data
 * @buf: IPC buffer structure to store I{PC buffer information
 * @buf_size: Size of the Buffer to allocate
 * @return: 0 on success and != 0 otherwise
 */
static inline int cls_wifi_ipc_buf_e2a_alloc(struct cls_wifi_hw *cls_wifi_hw,
										 struct cls_wifi_ipc_buf *buf,
										 size_t buf_size)
{
	return cls_wifi_ipc_buf_alloc(cls_wifi_hw, buf, buf_size, DMA_FROM_DEVICE, NULL);
}

/**
 * cls_wifi_ipc_buf_a2e_alloc() - Allocate an Application to Embedded Output IPC buffer
 *
 * @cls_wifi_hw: Main driver data
 * @buf: IPC buffer structure to store I{PC buffer information
 * @buf_size: Size of the Buffer to allocate
 * @buf_data: Initialization data for the buffer. Must be at least
 * @buf_size long
 * @return: 0 on success and != 0 otherwise
 */
static inline int cls_wifi_ipc_buf_a2e_alloc(struct cls_wifi_hw *cls_wifi_hw,
										 struct cls_wifi_ipc_buf *buf,
										 size_t buf_size, const void *buf_data)
{
	return cls_wifi_ipc_buf_alloc(cls_wifi_hw, buf, buf_size, DMA_TO_DEVICE, buf_data);
}
void cls_wifi_ipc_buf_dealloc(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_ipc_buf *buf);
int cls_wifi_ipc_buf_a2e_init(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_ipc_buf *buf,
						  void *data, size_t buf_size);

void cls_wifi_ipc_buf_release(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_ipc_buf *buf,
						  enum dma_data_direction dir);

/**
 * cls_wifi_ipc_buf_e2a_release() - Release DMA mapping for an Application to Embedded IPC buffer
 *
 * @cls_wifi_hw: Main driver structure
 * @buf: IPC buffer to release
 *
 * An A2E buffer is realeased when it has been read by the embbeded side. This is
 * used before giving back a buffer to upper layer, or before deleting a buffer
 * when cls_wifi_ipc_buf_dealloc() cannot be used.
 */
static inline void cls_wifi_ipc_buf_a2e_release(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_ipc_buf *buf)
{
	cls_wifi_ipc_buf_release(cls_wifi_hw, buf, DMA_TO_DEVICE);
}

/**
 * cls_wifi_ipc_buf_e2a_release() - Release DMA mapping for an Embedded to Application IPC buffer
 *
 * @cls_wifi_hw: Main driver structure
 * @buf: IPC buffer to release
 *
 * An E2A buffer is released when it has been updated by the embedded and it's ready
 * to be forwarded to upper layer (i.e. out of the driver) or to be deleted and
 * cls_wifi_ipc_buf_dealloc() cannot be used.
 *
 * Note: This function has the side effect to synchronize the buffer for the host so no need to
 * call cls_wifi_ipc_buf_e2a_sync().
 */
static inline void cls_wifi_ipc_buf_e2a_release(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_ipc_buf *buf)
{
	cls_wifi_ipc_buf_release(cls_wifi_hw, buf, DMA_FROM_DEVICE);
}

void cls_wifi_ipc_buf_e2a_sync(struct device *dev, struct cls_wifi_ipc_buf *buf, size_t len);
void cls_wifi_ipc_buf_e2a_sync_back(struct device *dev, struct cls_wifi_ipc_buf *buf, size_t len);

/**
 * IPC rx buffer management
 */
int cls_wifi_ipc_rxbuf_init(struct cls_wifi_hw *cls_wifi_hw, uint32_t rx_bufsz);
int cls_wifi_cmn_ipc_rxbuf_init(struct cls_wifi_cmn_hw *cmn_hw, uint32_t rxbuf_sz);
int cls_wifi_ipc_rxbuf_alloc(struct cls_wifi_hw *cls_wifi_hw);
int cls_wifi_cmn_ipc_rxbuf_alloc(struct cls_wifi_cmn_hw *cmn_hw, int radio);
void cls_wifi_ipc_rxbuf_dealloc(struct device *dev, struct cls_wifi_ipc_buf *buf);
void cls_wifi_ipc_rxbuf_repush(struct cls_wifi_hw *cls_wifi_hw,
							struct cls_wifi_ipc_buf *buf);
void cls_wifi_cmn_ipc_rxbuf_repush(struct cls_wifi_cmn_hw *cmn_hw,
							struct cls_wifi_ipc_buf *buf, u8 radio_idx);
void cls_wifi_ipc_rxdesc_repush(struct cls_wifi_hw *cls_wifi_hw,
							struct cls_wifi_ipc_buf *buf);
void cls_wifi_cmn_ipc_rxdesc_repush(struct cls_wifi_cmn_hw *cmn_hw,
							struct cls_wifi_ipc_buf *buf, u8 radio_idx);
struct cls_wifi_ipc_buf *cls_wifi_ipc_rxbuf_from_hostid(struct cls_wifi_hw *cls_wifi_hw, u32 hostid);
struct cls_wifi_ipc_buf *cls_wifi_cmn_ipc_rxbuf_from_hostid(struct cls_wifi_cmn_hw *cmn_hw, u32 hostid, u8 radio_idx, u16 status);

int cls_wifi_ipc_unsuprxvec_alloc(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_ipc_buf *buf);
void cls_wifi_ipc_unsuprxvec_repush(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_ipc_buf *buf);

/**
 * IPC TX specific functions
 */
void cls_wifi_ipc_txdesc_push(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_sw_txhdr *sw_txhdr,
						  struct sk_buff *hostid, int hw_queue,
						  u16 txq_credits, u16 hwq_credits, u16 host_pushed);
struct sk_buff *cls_wifi_ipc_get_skb_from_cfm(struct cls_wifi_hw *cls_wifi_hw,
										  struct cls_wifi_ipc_buf *buf);
void cls_wifi_ipc_sta_buffer_init(struct cls_wifi_hw *cls_wifi_hw, u16 sta_idx);
void cls_wifi_ipc_sta_buffer(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_sta *sta, int tid, int size);
void cls_wifi_ipc_tx_drain(struct cls_wifi_hw *cls_wifi_hw);
bool cls_wifi_ipc_tx_pending(struct cls_wifi_hw *cls_wifi_hw);

/**
 * FW dump handler / trace
 */
void cls_wifi_error_ind(struct cls_wifi_hw *cls_wifi_hw);
void cls_wifi_umh_done(struct cls_wifi_hw *cls_wifi_hw);
#endif /*__KERNEL__*/
#endif /* _CLS_WIFI_UTILS_H_ */
