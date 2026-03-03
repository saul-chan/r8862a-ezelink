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

#ifndef _IPC_HOST_H_
#define _IPC_HOST_H_

/*
 * INCLUDE FILES
 ******************************************************************************
 */
#include "ipc_shared.h"
#include "cls_wifi_platform.h"

/*
 * ENUMERATION
 ******************************************************************************
 */

enum ipc_host_desc_status {
	/// Descriptor is IDLE
	IPC_HOST_DESC_IDLE	  = 0,
	/// Data can be forwarded
	IPC_HOST_DESC_FORWARD,
	/// Data has to be kept in UMAC memory
	IPC_HOST_DESC_KEEP,
	/// Delete stored packet
	IPC_HOST_DESC_DELETE,
	/// Update Frame Length status
	IPC_HOST_DESC_LEN_UPDATE,
};

/**
 ******************************************************************************
 * @brief This structure is used to initialize the MAC SW
 *
 * The WLAN device driver provides functions call-back with this structure
 ******************************************************************************
 */
struct ipc_host_cb_tag {
	/// WLAN driver call-back function: send_data_cfm
	int (*send_data_cfm)(void *pthis, void *host_id);

	/// WLAN driver call-back function: recv_data_ind
	uint8_t (*recv_data_ind)(void *pthis, void *host_id, u8 rx_from_cmn);

	/// WLAN driver call-back function: recv_radar_ind
	uint8_t (*recv_radar_ind)(void *pthis, void *host_id);

	/// WLAN driver call-back function: recv_he_mu_map_ind
	uint8_t (*recv_he_mu_map_ind)(void *pthis, void *host_id);

	/// WLAN driver call-back function: recv_unsup_rx_vec_ind
	uint8_t (*recv_unsup_rx_vec_ind)(void *pthis, void *host_id);

	/// WLAN driver call-back function: recv_msg_ind
	uint8_t (*recv_msg_ind)(void *pthis, void *host_id);

	/// WLAN driver call-back function: recv_msgack_ind
	uint8_t (*recv_msgack_ind)(void *pthis, void *host_id);

	/// WLAN driver call-back function: recv_dbg_ind
	uint8_t (*recv_dbg_ind)(void *pthis, void *host_id);

	/// WLAN driver call-back function: recv_csi_ind
	uint8_t (*recv_csi_ind)(void *pthis, void *host_id);

	/* WLAN driver recv_atf_stats_ind */
	uint8_t (*recv_atf_stats_ind)(void *pthis, void *host_id);
};

/// Struct used to associate a local pointer with a shared 32bits value
struct ipc_hostid {
	struct list_head list;
	void *hostptr;   ///< local pointer
	uint32_t hostid; ///< associated value shared over IPC
};

/// Definition of the IPC Host environment structure.
struct ipc_host_cmn_env_tag {
	/// Structure containing the callback pointers
	struct ipc_host_cb_tag cb;
	const struct cls_wifi_ep_ops *ops;
	struct cls_wifi_plat *plat;
	struct device *dev;
	u8 radio_idx;

	struct cmn_ipc_shared_env_tag *shared;

	/// Fields for Debug MSGs handling
	// Array of debug buffer allocated for the firmware
	struct cls_wifi_ipc_buf *dbgbuf[CMN_DBG_CNT];
	// Index of the next Debug message to read
	uint32_t dbgbuf_idx;

	struct cls_wifi_ipc_buf *rxdesc_2g[CMN_RXDESC_CNT_MAX_2G];
	uint16_t rxdesc_idx_2g;
	struct cls_wifi_ipc_buf *rxdesc_5g[CMN_RXDESC_CNT_MAX_5G];
	uint16_t rxdesc_idx_5g;

	uint16_t rxbuf_idx_2g;
	uint16_t rxbuf_idx_5g;
	uint32_t rxbuf_sz;
};


/// Definition of the IPC Host environment structure.
struct ipc_host_env_tag {
	/// Structure containing the callback pointers
	struct ipc_host_cb_tag cb;
	struct cls_wifi_plat *plat;
	const struct cls_wifi_ep_ops *ops;
	u8 radio_idx;

	// Index of the next RX descriptor to be filled by the firmware
	uint16_t rxdesc_idx;
	/// Number of RX Descriptors
	uint16_t rxdesc_nb;

	// Index of the next RX data buffer to write (and read for softmac)
	uint16_t rxbuf_idx;
	// Number of RX data buffers
	uint32_t rxbuf_nb;
	// Size, in bytes, of a Rx Data buffer
	uint32_t rxbuf_sz;

	/// Fields for Radar events handling
	// Array of Radar data buffers pushed to firmware
	struct cls_wifi_ipc_buf *radar[IPC_RADARBUF_CNT];
	// Index of the next radr buffer to read
	uint8_t radar_idx;

	/// Fields for HE MU DL Map events handling
	// Array of HE MU DL Map buffers pushed to firmware
	struct cls_wifi_ipc_buf *hemumap[IPC_HEMUBUF_CNT];
	// Index of the next HE MU map buffer to read
	uint8_t hemumap_idx;

	/// Fields for Unsupported frame handling
	// Array of Buffer pushed to firmware to upload unsupported frames
	struct cls_wifi_ipc_buf *unsuprxvec[IPC_UNSUPRXVECBUF_CNT];
	// Index used for ipc_host_unsuprxvecbuf_array to point to current buffer
	uint8_t unsuprxvec_idx;
	// Store the size of unsupported rx vector buffers
	uint32_t unsuprxvec_sz;

	/// Fields for Data Tx handling
	// Index to the first free TX descriptor address element
	uint32_t txdesc_addr_idx;
	// Pointer to the TX descriptor address array
	volatile uint32_t *txdesc_addr;

	// List of available tx_hostid
	struct list_head tx_hostid_available;
	// List of tx_hostid currently pushed to the firmware
	struct list_head tx_hostid_pushed;
	// Index of next TX confirmation to process
	uint32_t txcfm_idx;

	/// Fields for Emb->App Messages
	// Array of MSG buffer allocated for the firmware
	struct cls_wifi_ipc_buf *msgbuf[IPC_MSGE2A_BUF_CNT];
	// Index of the next MSG E2A buffers to read
	uint16_t msgbuf_idx;

	/// Fields for App->Emb Messages
	/// E2A ACKs of A2E MSGs
	uint8_t msga2e_cnt;
	void *msga2e_hostid;

	/// Fields for Debug MSGs handling
	// Array of debug buffer allocated for the firmware
	struct cls_wifi_ipc_buf *dbgbuf[IPC_DBGBUF_CNT];
	// Index of the next Debug message to read
	uint32_t dbgbuf_idx;

	struct cls_wifi_ipc_buf *atf_stats_buf;
	struct cls_wifi_ipc_buf *atf_quota_buf;
	struct cls_wifi_ipc_buf *dbg_cnt_buf;

	bool pp_valid;
	uint32_t txdesc_nb;
	uint32_t pp_offset[CLS_TXQ_CNT];
	uint32_t pp_cnt[CLS_TXQ_CNT];
	uint32_t pp_idx[CLS_TXQ_CNT];
	uint32_t txdesc_offset;
	uint32_t txdesc_size;
	uint32_t txcfm_nb;
	uint32_t txcfm_offset;
	uint32_t txcfm_size;
	uint32_t rxdesc_offset;
	uint32_t rxdesc_size;
	uint32_t rxbuf_offset;
	uint32_t rxbuf_size;
	uint32_t e2amsg_nb;
	uint32_t e2amsg_offset;
	uint32_t e2amsg_size;
	uint32_t dbgbuf_nb;
	uint32_t dbgbuf_offset;
	uint32_t dbgbuf_size;
	uint32_t dbgcnt_nb;
	uint32_t dbgcnt_offset;
	uint32_t dbgcnt_size;
	uint32_t buffered_nb;
	uint32_t buffered_offset;
	uint32_t buffered_size;
	uint32_t ipc_shared_size;

	/* CSI buffer */
	struct cls_wifi_ipc_buf *csibuf[IPC_CSIBUF_CNT];
	uint32_t csibuf_sz;
	uint8_t csibuf_idx;

	/// Pointer to the attached object (used in callbacks and register accesses)
	void *pthis;

	// Table of element to store tx_hostid
	//struct ipc_hostid tx_hostid[IPC_TXCFM_CNT];
	// Array of Buffer pushed to firmware to upload TX confirmation
	//struct cls_wifi_ipc_buf *txcfm[IPC_TXCFM_CNT];
	/// Fields for Data Rx handling
	/// Array of RX descriptor pushed to firmware
	//struct cls_wifi_ipc_buf *rxdesc[IPC_RXDESC_CNT];
};

extern void ipc_host_ipc_pattern_set(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index, u32 value);
extern void ipc_host_wpu_ipc_pattern_set(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index, u32 value);
extern u32 ipc_host_wpu_ipc_pattern_get(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index);
extern void ipc_host_trace_pattern_set(struct ipc_host_env_tag *env, u32 value);
extern u32 ipc_host_trace_pattern_get(struct ipc_host_env_tag *env);
extern u32 ipc_host_trace_offset_get(struct ipc_host_env_tag *env);
extern u32 ipc_host_trace_data_get(struct ipc_host_env_tag *env, u32 offset);
extern u32 ipc_host_trace_size_get(struct ipc_host_env_tag *env);
extern u32 ipc_host_trace_nb_compo_get(struct ipc_host_env_tag *env);
extern u32 ipc_host_trace_offset_compo_get(struct ipc_host_env_tag *env);
extern void ipc_host_trace_compo_set(struct ipc_host_env_tag *env, u32 compo_id, u32 value);
extern u32 ipc_host_trace_compo_get(struct ipc_host_env_tag *env, u32 compo_id);
extern void ipc_host_trace_start_set(struct ipc_host_env_tag *env, u32 value);
extern u32 ipc_host_trace_start_get(struct ipc_host_env_tag *env);
extern void ipc_host_trace_end_set(struct ipc_host_env_tag *env, u32 value);
extern u32 ipc_host_trace_end_get(struct ipc_host_env_tag *env);
extern void ipc_host_dbg_cnt_get(struct ipc_host_env_tag *env, struct ipc_shared_dbg_cnt *dbg_cnt);
extern u32 ipc_host_dbg_cnt_addr_get(struct ipc_host_env_tag *env);
extern void ipc_host_dbg_cnt_addr_push(struct ipc_host_env_tag *env, struct cls_wifi_ipc_buf *buf);

/**
 ******************************************************************************
 * @brief Initialize the IPC running on the Application CPU.
 *
 * This function:
 *   - initializes the IPC software environments
 *   - enables the interrupts in the IPC block
 *
 * @param[in]   env   Pointer to the IPC host environment
 *
 * @warning Since this function resets the IPC Shared memory, it must be called
 * before the LMAC FW is launched because LMAC sets some init values in IPC
 * Shared memory at boot.
 *
 ******************************************************************************
 */
void ipc_host_init(struct ipc_host_env_tag *env,
				  struct ipc_host_cb_tag *cb,
				  void *pthis);

/**
 ******************************************************************************
 * ipc_host_txdesc_push() - Push an IPC buffer containing a Tx descriptor to the FW.
 *
 * @env: IPC host environment
 * @buf: IPC buffer that contains the TX descriptor
 ******************************************************************************
 */
void ipc_host_txdesc_push(struct ipc_host_env_tag *env, struct cls_wifi_ipc_buf *buf, int hw_queue);

void ipc_host_atf_stats_push(struct ipc_host_env_tag *env, struct cls_wifi_ipc_buf *buf);
void ipc_host_atf_quota_push(struct ipc_host_env_tag *env, struct cls_wifi_ipc_buf *buf);

/**
 ******************************************************************************
 * @brief Check if there are TX frames pending in the TX queues.
 *
 * @param[in]   env   Pointer to the IPC host environment
 *
 * @return true if there are frames pending, false otherwise.
 *
 ******************************************************************************
 */
bool ipc_host_tx_frames_pending(struct ipc_host_env_tag *env);

/**
 ******************************************************************************
 * @brief Get and flush a packet from the IPC queue passed as parameter.
 *
 * @param[in]   env		Pointer to the IPC host environment
 *
 * @return The flushed hostid if there is one, 0 otherwise.
 ******************************************************************************
 */
void *ipc_host_tx_flush(struct ipc_host_env_tag *env);

/**
 ******************************************************************************
 * ipc_host_pattern_push() - Push address on the IPC buffer from where FW can
 * download TX pattern.
 *
 * @env: IPC host environment
 * @buf: IPC buffer that contains the pattern downloaded by fw after each TX
 ******************************************************************************
 */
void ipc_host_thd_pattern_push(struct ipc_host_env_tag *env, struct cls_wifi_ipc_buf *buf);
void ipc_host_tbd_pattern_push(struct ipc_host_env_tag *env, struct cls_wifi_ipc_buf *buf);

int ipc_host_rxbuf_init_push(struct ipc_host_env_tag *env, struct cls_wifi_ipc_buf *buf);
int ipc_host_cmn_rxbuf_init_push_2g(struct ipc_host_cmn_env_tag *env, struct cls_wifi_ipc_buf *buf);
int ipc_host_cmn_rxbuf_init_push_5g(struct ipc_host_cmn_env_tag *env, struct cls_wifi_ipc_buf *buf);
/**
 ******************************************************************************
 * ipc_host_rxbuf_push() - Push a pre-allocated buffer for a Rx packet
 *
 * @env: IPC host environment
 * @buf: IPC buffer to use for the RX packet
 *
 * Push a new buffer for the firmware to upload an RX packet.
 ******************************************************************************
 */
int ipc_host_rxbuf_push(struct ipc_host_env_tag *env, struct cls_wifi_ipc_buf *buf);
int ipc_host_cmn_rxbuf_push_2g(struct ipc_host_cmn_env_tag *env, struct cls_wifi_ipc_buf *buf);
int ipc_host_cmn_rxbuf_push_5g(struct ipc_host_cmn_env_tag *env, struct cls_wifi_ipc_buf *buf);

int ipc_host_rxdesc_init_push(struct ipc_host_env_tag *env, struct cls_wifi_ipc_buf *buf);
int ipc_host_cmn_rxdesc_init_push_2g(struct ipc_host_cmn_env_tag *env, struct cls_wifi_ipc_buf *buf);
int ipc_host_cmn_rxdesc_init_push_5g(struct ipc_host_cmn_env_tag *env, struct cls_wifi_ipc_buf *buf);
/**
 ******************************************************************************
 * ipc_host_rxdesc_push() - Push a pre-allocated buffer for a Rx Descriptor
 *
 * @env: IPC host environment
 * @buf: IPC buffer to use for the RX descriptor
 *
 * Push a new buffer for the firmware to upload an RX descriptor.
 ******************************************************************************
 */
int ipc_host_rxdesc_push(struct ipc_host_env_tag *env, struct cls_wifi_ipc_buf *buf);
int ipc_host_cmn_rxdesc_push_2g(struct ipc_host_cmn_env_tag *env, struct cls_wifi_ipc_buf *buf);
int ipc_host_cmn_rxdesc_push_5g(struct ipc_host_cmn_env_tag *env, struct cls_wifi_ipc_buf *buf);

/**
 ******************************************************************************
 * ipc_host_radar_push() - Push a pre-allocated buffer for a Rx Descriptor
 *
 * @env: IPC host environment
 * @buf: IPC buffer to use for the Radar event
 *
 * This function is called at Init time to initialize all radar event buffers.
 * Then each time embedded send a radar event, this function is used to push
 * back the same buffer once it has been handled.
 ******************************************************************************
 */
int ipc_host_radar_push(struct ipc_host_env_tag *env, struct cls_wifi_ipc_buf *buf);

/**
 ******************************************************************************
 * ipc_host_he_mu_map_push() - Push a pre-allocated buffer for a DL Map
 *
 * @env: IPC host environment
 * @buf: IPC buffer to use for the DL Map event
 *
 * This function is called at Init time to initialize all HE MU DL Map event buffers.
 * Then each time embedded send a DL Map event, this function is used to push
 * back the same buffer once it has been handled.
 ******************************************************************************
 */
int ipc_host_he_mu_map_push(struct ipc_host_env_tag *env, struct cls_wifi_ipc_buf *buf);

/**
 ******************************************************************************
 * ipc_host_unsuprxvec_push() - Push a pre-allocated buffer for the firmware
 * to upload unsupported RX vector descriptor.
 *
 * @env: IPC host environment
 * @buf: IPC buffer to use for the Unsupported RX vector.
 *
 * This function is called at Init time to initialize all unsupported rx vector
 * buffers. Then each time the embedded sends a unsupported rx vector, this
 * function is used to push a new unsupported rx vector buffer.
 ******************************************************************************
 */
int ipc_host_unsuprxvec_push(struct ipc_host_env_tag *env, struct cls_wifi_ipc_buf *buf);

/**
 ******************************************************************************
 * ipc_host_msgbuf_push() - Push the pre-allocated buffer for IPC MSGs from
 * the firmware
 *
 * @env: IPC host environment
 * @buf: IPC buffer to use for the IPC messages
 *
 * This function is called at Init time to initialize all Emb2App messages
 * buffers. Then each time embedded send a IPC message, this function is used
 * to push back the same buffer once it has been handled.
 ******************************************************************************
 */
int ipc_host_msgbuf_push(struct ipc_host_env_tag *env, struct cls_wifi_ipc_buf *buf);

/**
 ******************************************************************************
 * ipc_host_dbgbuf_push() - Push the pre-allocated buffer for Debug MSGs from
 * the firmware
 *
 * @env: IPC host environment
 * @buf: IPC buffer to use for the debug messages
 *
 * This function is called at Init time to initialize all debug messages.
 * Then each time embedded send a debug message, this function is used to push
 * back the same buffer once it has been handled.
 ******************************************************************************
 */
int ipc_host_dbgbuf_push(struct ipc_host_env_tag *env, struct cls_wifi_ipc_buf *buf);
int ipc_host_cmn_dbgbuf_push(struct ipc_host_cmn_env_tag *env, struct cls_wifi_ipc_buf *buf);

/**
 ******************************************************************************
 * ipc_host_txcfm_push() - Push a pre-allocated buffer descriptor for Tx
 * confirmation
 *
 * @env: IPC host environment
 * @buf: IPC buffer to use for the TX confirmation
 *
 * This function is only called IPC_TXCFM_CNT times at Init to update
 * confirmation address in ipc shared structure.
 ******************************************************************************
 */
int ipc_host_txcfm_push(struct ipc_host_env_tag *env, struct cls_wifi_ipc_buf *buf);

/**
 ******************************************************************************
 * ipc_host_dbginfo_push() - Push the pre-allocated logic analyzer and debug
 * information buffer
 *
 * @env: IPC host environment
 * @buf: IPC buffer to use for the FW dump
 ******************************************************************************
 */
void ipc_host_dbginfo_push(struct ipc_host_env_tag *env, struct cls_wifi_ipc_buf *buf);

int ipc_host_csi_buf_push(struct ipc_host_env_tag *env, struct cls_wifi_ipc_buf *buf);

/**
 ******************************************************************************
 * @brief Handle all IPC interrupts on the host side.
 *
 * The following interrupts should be handled:
 * Tx confirmation, Rx buffer requests, Rx packet ready and kernel messages
 *
 * @param[in]   env   Pointer to the IPC host environment
 *
 ******************************************************************************
 */
void ipc_host_irq(struct cls_wifi_hw *cls_wifi_hw, uint32_t status);

void ipc_host_dbg_handler(struct cls_wifi_hw *cls_wifi_hw);
void ipc_host_msg_handler(struct cls_wifi_hw *cls_wifi_hw);
void ipc_host_msgack_handler(struct cls_wifi_hw *cls_wifi_hw);
void ipc_host_rxdesc_handler(struct cls_wifi_hw *cls_wifi_hw);
void ipc_host_radar_handler(struct cls_wifi_hw *cls_wifi_hw);
void ipc_host_unsup_rx_vec_handler(struct cls_wifi_hw *cls_wifi_hw);
void ipc_host_he_mu_map_handler(struct cls_wifi_hw *cls_wifi_hw);
void ipc_host_tx_cfm_handler(struct cls_wifi_hw *cls_wifi_hw);
void ipc_host_csi_handler(struct cls_wifi_hw *cls_wifi_hw);
void ipc_host_atf_stats_handler(struct cls_wifi_hw *cls_wifi_hw);

/**
 ******************************************************************************
 * @brief Send a message to the embedded side
 *
 * @param[in]   env	  Pointer to the IPC host environment
 * @param[in]   msg_buf  Pointer to the message buffer
 * @param[in]   msg_len  Length of the message to be transmitted
 *
 * @return	  Non-null value on failure
 *
 ******************************************************************************
 */
int ipc_host_msg_push(struct ipc_host_env_tag *env, void *msg_buf, uint16_t len);

/**
 ******************************************************************************
 * @brief Enable IPC interrupts
 *
 * @param[in]   env  Global ipc_host environment pointer
 * @param[in]   value  Bitfield of the interrupts to enable
 *
 * @warning After calling this function, IPC interrupts can be triggered at any
 * time. Potentially, an interrupt could happen even before returning from the
 * function if there is a request pending from the embedded side.
 *
 ******************************************************************************
 */
void ipc_host_enable_irq(struct ipc_host_env_tag *env, uint32_t value);
void ipc_host_disable_irq(struct ipc_host_env_tag *env, uint32_t value);

uint32_t ipc_host_get_status(struct ipc_host_env_tag *env);
uint32_t ipc_host_get_rawstatus(struct ipc_host_env_tag *env);

uint32_t ipc_host_tx_host_ptr_to_id(struct ipc_host_env_tag *env, void *host_ptr);
void *ipc_host_tx_host_id_to_ptr(struct ipc_host_env_tag *env, uint32_t hostid);

void cls_wifi_tx_free_skb_work_handler(struct work_struct *work);
void cls_wifi_dbg_work(struct work_struct *work);
void cls_wifi_dbg_cmn_work(struct work_struct *work);
void cls_wifi_csi_work(struct work_struct *work);
void cls_wifi_trig_dump_wpu(struct cls_wifi_hw *cls_wifi_hw, uint32_t val);

uint32_t ipc_host_get_txdesc_nb(struct cls_wifi_hw *cls_wifi_hw);
uint32_t ipc_host_get_txcfm_nb(struct cls_wifi_hw *cls_wifi_hw);
uint32_t ipc_host_get_rxdesc_nb(struct cls_wifi_hw *cls_wifi_hw);
uint32_t ipc_host_get_rxbuf_nb(struct cls_wifi_hw *cls_wifi_hw);

int ipc_host_log_read(struct ipc_host_env_tag *env, uint32_t offset, void *dst, uint32_t len);
#endif // _IPC_HOST_H_
