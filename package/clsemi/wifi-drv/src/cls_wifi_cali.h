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

#ifndef _CLS_WIFI_CALI_H_
#define _CLS_WIFI_CALI_H_
#ifdef __KERNEL__
#include <linux/workqueue.h>
#include <linux/if_ether.h>
#endif

#include "cali_struct.h"

#define DEV_NAME_2P4G	   "wifi0"
#define DEV_NAME_5G		 "wifi1"
#define RADIO_NAME_2P4G	 "2G"
#define RADIO_NAME_5G	   "5G"
#define RADIO_2P4G_INDEX	0
#define RADIO_5G_INDEX	  1
#define MAX_RADIO_NUM	   2

#define WIFI_MEM_SIZE			0x02000000
#define WIFI40_BASE_ADDR		0x48000000
#define WIFI160_BASE_ADDR		0x4A000000

#define DIF_MEM_SIZE			0x00400000
#define DIF40_BASE_ADDR			0x4C400000
#define DIF160_BASE_ADDR		0x4C800000

#define CAL_DBG			 1
#define RX_LAST_MPDU_FILE	"/tmp/rx_last_mpdu.bin"
#define RX_LAST_CSI_FILE	"/tmp/rx_last_csi.bin"

struct wpu_regs_map {
	uint32_t reg_base_addr;
	uint32_t reg_size;
	void __iomem *io_base_addr;
};

#ifdef __KERNEL__
///////////////////////////////////////////////////////////////////////////////
/////////// For Calibration messages
///////////////////////////////////////////////////////////////////////////////
enum cal_msg_tag {
	/// Calibration parameter configuration Request.
	CAL_PARAM_CONFIG_REQ = LMAC_FIRST_MSG(TASK_CAL),

	/// Calibration parameter configuration Confirmation.
	CAL_PARAM_CONFIG_CFM,

	/// Calibration parameter update Request
	CAL_PARAM_UPDATE_ONLY_REQ,
	/// Calibration parameter update Confirmation
	CAL_PARAM_UPDATE_ONLY_CFM,

	/// Calibration rx statistics Request
	CAL_RX_STATS_REQ,
	/// Calibration rx statistics Confirmation
	CAL_RX_STATS_CFM,
	/// Calibration rx status Request
	CAL_RX_STATUS_REQ,
	/// Calibration rx status Confirmation
	CAL_RX_STATUS_CFM,

	/// Calibration TX SU Request
	CAL_TX_SU_REQ,
	/// Calibration TX SU Confirmation
	CAL_TX_SU_CFM,
	/// Calibration TX SU result indicate
	CAL_TX_SU_RES_IND,
	/// Calibration TX Statistics Request
	CAL_TX_STATS_REQ,
	/// Calibration TX Statistics Confirmation
	CAL_TX_STATS_CFM,
	/// Calibration TX MU Request
	CAL_TX_MU_REQ,
	/// Calibration TX MU Confirmation
	CAL_TX_MU_CFM,
	/// Calibration SOUNDING Request
	CAL_SOUNDING_REQ,
	/// Calibration SOUNDING Confirmation
	CAL_SOUNDING_CFM,
	/// Calibration memory operation request
	CAL_MEM_OP_REQ,
	/// Calibration memory operation confirmation
	CAL_MEM_OP_CFM,
	/// Calibration initialization request
	CAL_FW_INIT_REQ,
	/// Calibration initialization confirmation
	CAL_FW_INIT_CFM,
	/// Calibration DIF sample request
	CAL_DIF_SMP_REQ,
	/// Calibration DIF sample confirmation
	CAL_DIF_SMP_CFM,
	/// Calibration radar detect request
	CAL_RADAR_DETECT_REQ,
	/// Calibration radar detect confirmation
	CAL_RADAR_DETECT_CFM,
	/// Calibration interference detect request
	CAL_INTERFERENCE_DETECT_REQ,
	/// Calibration interference detect confirmation
	CAL_INTERFERENCE_DETECT_CFM,

	/// Calibration log set request
	CAL_LOG_SET_REQ,
	/// Calibration log set confirmation
	CAL_LOG_SET_CFM,
	/// Calibration log get request
	CAL_LOG_GET_REQ,
	/// Calibration log get confirmation
	CAL_LOG_GET_CFM,

	/// Calibration rssi status Request
	CAL_RSSI_STATUS_REQ,
	/// Calibration rssi status confirmation
	CAL_RSSI_STATUS_CFM,
	/// Calibration rssi status Request
	CAL_RSSI_START_REQ,
	/// Calibration rssi status confirmation
	CAL_RSSI_START_CFM,

	/// Calibration get temperature Request
	CAL_GET_TEMP_REQ,
	/// Calibration get temperature confirmation
	CAL_GET_TEMP_CFM,

	/// Calibration get CSI Request
	CAL_GET_CSI_REQ,
	/// Calibration get CSI confirmation
	CAL_GET_CSI_CFM,

	CAL_LEAF_TIMER_GET_REQ,
	CAL_LEAF_TIMER_GET_CFM,

	/// MAX number of messages
	CAL_MSG_MAX
};
#endif

enum cal_param_type {
	CAL_PARAM_BASIC,
	CAL_PARAM_TYPE_MAX,
};

#ifdef __KERNEL__
/// Structure containing the parameters of the @ref CAL_PARAM_CONFIG_REQ message
struct cal_param_config_req {
	u16 radio;
	u16 param_type;
	u16 param_len;
	u16 use_dma;
	/* This parameter are effective when the parameter is transfered via DMA */
	dma_addr_t param_dma_addr;
};
/// Commont structure containing the parameters of the below message
/// @ref CAL_FW_INIT_REQ
struct cal_ipc_comm_req {
	u16 radio;
	u16 param_len;
};

/// Commont structure containing the parameters of the below message
/// @ref CAL_FW_INIT_CFM
/// @ref CAL_PARAM_CONFIG_CFM
struct cal_ipc_comm_cfm {
	/// radio from which the confirmation
	u16 radio;
	/// Status of the request
	int16_t status;
};

/// Structure containing the parameters of the @ref CAL_MEM_OP_REQ message
#define CAL_MEM_OP_RD		1
#define CAL_MEM_OP_WR		2
#define CAL_MEM_OP_WR_LEN_MAX	32
#define CAL_MEM_OP_RD_LEN_MAX	256
struct cal_mem_op_req {
	u16 radio;
	u8 op_type;
	u8 mem_op_len; /* unit: 4 bytes */
	u32 mem_op_addr;
	u32 buf[CAL_MEM_OP_WR_LEN_MAX];
};

/// Structure containing the parameters of the @ref CAL_MEM_OP_CFM message
struct cal_mem_op_cfm {
	/// radio from which the confirmation
	u16 radio;
	/// Status of the request
	int16_t status;
};

struct cal_leaf_timer_req {
	u16 radio;
	u8 op_type;
	u8 pad;
};

/// Structure containing the parameters of the @ref CAL_MEM_OP_CFM message
struct cal_leaf_timer_cfm {
	/// radio from which the confirmation
	u16 radio;
	/// Status of the request
	int16_t status;
	u32 us_low;
	u32 us_hi;
	u32 ns;
};

struct cal_rx_stats_req {
	uint16_t radio_id;
#define CAL_RX_STATS_FLAG_RD_CLR	0x0001
	uint16_t flags;
};

struct cal_rx_stats_cfm {
	uint16_t radio_id;
	uint16_t length;
	int32_t status;
};

struct cal_rx_status_req {
	uint16_t radio_id;
	uint16_t flags;
};

struct cal_rx_status_cfm {
	uint16_t radio_id;
	uint16_t length;
	int32_t status;
};

struct tx_mld_params {
	/// str / nstr / emlsr / emlmr
	/// See "enum CALI_MLO_MODE"
	/// For MU-test, use per-user's tx-mode.
	uint32_t	mlo_mode;

	uint32_t	ppdu_count;

	/// Push PPDU to WMAC @tx_tick.  0 : Push to WMAC immediately.
	uint32_t	tx_tick;

	/// The end of prev-PPDU to start of next-PPDU.
	/// 0		: No delay, Push to WMAC one by one.
	/// Others	: After Tx-Done, delay a given time.
	uint32_t	ppdu_delay;

	/// MU_RTS / BSRP / QoS_NULL
	/// See "enum CALI_EML_INIT_CTRL_TYPE"
	uint32_t	eml_init_frame_type;

	uint32_t	eml_padding_delay;

	uint32_t	force_send_init_frame;

	uint32_t	replace_mld_addr;

	uint32_t	new_txop;

	uint32_t	eml_ul_mcs;
	uint32_t	eml_ul_duration;
};

enum {
	HW_AGG_MODE_DISABLE = 0,
	HW_AGG_MODE_SINGLE_LINK,
	HW_AGG_MODE_MULTI_LINK,
	HW_AGG_MODE_MAX
};

struct tx_hw_agg_params {
	// hw_agg_mode=0/1/2 : disabled(0) / single-link(1) or multi-link(2) mode.
	uint32_t	hw_agg_mode;
	// master=0/1 : For slaver-radio, it's tx-time be controller by master-radio.
	uint32_t	master_radio;

	// retry_bmp 0x1 0x2 0x3 0x4 : 128 bits / 4 words.
	uint32_t	retry_bitmap[4];

	// succ_bmp 0x1 0x2 0x3 0x4 : 128 bits / 4 words.
	uint32_t	succ_bitmap[4];

	// hw_agg_len=number
	uint32_t	hw_agg_len;

	// hw_agg_mpdu=number
	uint32_t	hw_agg_mpdu_num;

	// hw_agg_tx_delay=signed number
	// On slave side, needn't config it.
	// On master side, this param to control when to tx on slave side.
	int32_t		hw_agg_slave_tx_diff;

	// hw_agg_dup=0/1
	uint32_t	hw_agg_dup;

	// fast-retry=0/1
	uint32_t	fast_retry;

	// dyn-fetch=number // 0 : disable
	uint32_t	dyn_fetch;
};


struct cal_tx_su_req {
	uint32_t payload_len;
	uint32_t en_sample;
	uint32_t num_sample;
	uint32_t en_dif_send;
	uint32_t eof_padding;
	uint32_t terminate;
#ifdef CFG_MERAK3000
	struct tx_mld_params	mld_params;
	struct tx_hw_agg_params	hw_agg_params;
	uint32_t		one_off_test;
	uint32_t		wmci_tx_stop_delay;
	uint32_t		wmci_tx_stop_queue_bmp;
#endif
};

struct cal_tx_su_cfm {
	int32_t status;
};

struct cal_tx_su_res_ind {
	uint32_t txstatus;
};

struct cal_tx_stats_req {
#define CAL_TX_STATS_FLAG_RD_CLR	0x0001
	uint32_t flags;
};
#endif

#ifdef __KERNEL__
struct cal_tx_stats_cfm {
	uint32_t length;
	int32_t status;
};

struct cal_tx_mu_req {
	uint32_t en_sample;
	uint32_t num_sample;

#ifdef CFG_MERAK3000
	struct tx_mld_params	mld_params;
	struct tx_hw_agg_params hw_agg_params;
	uint8_t			trig_ppdu_id;
#endif
};

struct cal_tx_mu_cfm {
	int32_t status;
};

struct cal_sounding_req {
	uint32_t en_sample;
	uint32_t num_sample;
	void *host_report_addr[CLS_MU_USER_MAX];
	uint32_t dma_addr[CLS_MU_USER_MAX];
};

struct cal_sounding_cfm {
	int32_t status;
};
#endif
struct cal_bf_req {
	void *host_report_addr[CLS_MU_USER_MAX];
	uint32_t dma_addr[CLS_MU_USER_MAX];
	uint32_t host_report_len[CLS_MU_USER_MAX];
	uint32_t host_report_max[CLS_MU_USER_MAX];
};

/* FIXME: maybe need move to other file */
struct cal_tx_stats {
	uint32_t ctrl_frame;
	uint32_t ctrl_frame_succ;
	uint32_t mgmt_frame;
	uint32_t mgmt_frame_succ;
	uint32_t data_frame;
	uint32_t null_data_frame;
	uint32_t qos_data_frame;
	uint32_t qos_null_data_frame;
	uint32_t data_frame_succ;
	uint32_t null_data_frame_succ;
	uint32_t qos_data_frame_succ;
	uint32_t qos_null_data_frame_succ;
	uint32_t ampdu;
	uint32_t ampdu_succ;
	uint32_t mpdu;
	uint32_t mpdu_succ;
	uint32_t retries;
	uint32_t sr_frame;
	uint32_t sr_succ;
	uint32_t sr_skip;

#if defined(CFG_MERAK3000)
	uint32_t tx_mld_push;
	uint32_t tx_mld_push_delay;
	uint32_t tx_mld_done;
	uint32_t tx_mld_succ;
	uint32_t tx_mld_fail;

	uint32_t tx_mld_nstr;
	uint32_t tx_mld_str;
	uint32_t tx_mld_eml;

	uint32_t tx_mld_eml_conflict;
	uint32_t tx_mld_nstr_conflict;

	uint32_t tx_hw_agg_total;
	uint32_t tx_hw_agg_no_mpdu;
	uint32_t tx_hw_agg_canceled;	// HW canceled this aggregation.
	uint32_t tx_hw_agg_tx_succ;
	uint32_t tx_hw_agg_tx_fail;	// Tx-ed, but failed.

	uint32_t tx_hw_agg_dup_count;	// per-PPDU or MPDU??

	uint32_t tx_mld_eml_data_frm_done;
	uint32_t tx_mld_eml_data_frm_fail;
	uint32_t tx_mld_eml_init_frm_done;
	uint32_t tx_mld_eml_init_frm_fail;
#endif

};

#ifdef __KERNEL__
struct cal_dif_sample_req {
	uint32_t num_sample;
	uint32_t chan;
};

struct cal_dif_sample_cfm {
	int32_t status;
};

struct cal_radar_detect_req {
	uint32_t enable;
};

struct cal_radar_detect_cfm {
	int32_t status;
};

struct cal_interference_detect_req {
	uint32_t enable;
};

struct cal_interference_detect_cfm {
	int32_t status;
};

struct cal_rssi_start_req {
	uint8_t enable;
	uint8_t debug;
	uint32_t max_num;
};

struct cal_rssi_start_cfm {
	int32_t status;
};

struct cal_rssi_status_req {
	uint8_t rssi_mode;
	uint8_t legacy;
};

struct cal_rssi_status_cfm {
	int32_t status;
	int8_t rssi;
};

struct cal_get_temp_req {
	uint32_t ts_id;
};

struct cal_get_temp_cfm {
	int32_t status;
	int8_t temp;
};

struct cal_log_set_req {
	uint32_t log_module;
	uint32_t log_level;
};

struct cal_log_set_cfm {
	int32_t status;
};

struct cal_log_get_cfm {
	int32_t status;
};

struct cal_get_csi_req {
	uint16_t radio_id;
	uint16_t flags;
};
#endif

struct cal_test_uncache_rw_req {
	uint32_t len;	/* Buffer length */
};

#ifdef __KERNEL__
struct cal_get_csi_cfm {
	uint16_t radio_id;
	uint16_t length;
	int32_t status;
	uint32_t buff_len;
};


/// Definition of the Calibration environment structure.
struct cls_wifi_ipc_buf;
#endif
struct cls_wifi_cal_env_tag {
	uint32_t radio_idx;
	struct cls_wifi_ipc_buf cal_param_ipc_buf;
	struct cal_ipc_comm_cfm cal_param_cfm;
	struct cal_rx_stats_cfm rx_stats_cfm;
	struct cal_rx_status_cfm rx_status_cfm;
	struct cal_tx_su_cfm tx_su_cfm;
	struct cal_tx_stats_cfm tx_stats_cfm;
	struct cal_tx_mu_cfm tx_mu_cfm;
	struct cal_sounding_req tx_sounding_req;
	struct cal_bf_req tx_bf_req;
	struct cal_sounding_cfm tx_sounding_cfm;
	struct cal_dif_sample_cfm dif_smp_cfm;
	struct cal_radar_detect_cfm radar_detect_cfm;
	struct cal_interference_detect_cfm interference_detect_cfm;
	struct cal_log_set_cfm log_set_cfm;
	struct cal_log_get_cfm log_get_cfm;
	struct cal_rssi_start_cfm rssi_start_cfm;
	struct cal_rssi_status_cfm rssi_status_cfm;
	struct cal_get_temp_cfm get_temp_cfm;
	struct cal_get_csi_cfm get_csi_cfm;
};

extern struct cls_wifi_hw *g_radio_cls_wifi_hw[MAX_RADIO_NUM];
extern msg_cb_fct cal_hdlrs[MSG_I(CAL_MSG_MAX)];

#ifdef __KERNEL__
int cls_wifi_cal_init(struct cls_wifi_hw *cls_wifi_hw);
void cls_wifi_cal_deinit(struct cls_wifi_hw *cls_wifi_hw);
int cali_debugfs_init(void);
void cali_debugfs_deinit(void);
#endif
int cls_wifi_cal_fw_init_req(struct cls_wifi_hw *cls_wifi_hw, int radio_idx);
int cls_wifi_send_cal_param_config_req(struct cls_wifi_hw *cls_wifi_hw, int radio,
	int type, int param_len, void *param, int dma);
int cls_wifi_send_cal_param_update_only_req(struct cls_wifi_hw *cls_wifi_hw, int radio_idx,
	int type, int param_len, void *param, int dma);
int cls_wifi_send_cal_tx_su_req(struct cls_wifi_hw *cls_wifi_hw, struct cal_tx_su_req *tx_param);
int cls_wifi_send_cal_tx_stats_req(struct cls_wifi_hw *cls_wifi_hw, bool clear);
int cls_wifi_send_cal_rx_status_req(struct cls_wifi_hw *cls_wifi_hw, uint16_t radio_id);
int cls_wifi_send_cal_rx_stats_req(struct cls_wifi_hw *cls_wifi_hw, uint16_t radio_id, uint32_t clear);
int cls_wifi_send_cal_tx_mu_req(struct cls_wifi_hw *cls_wifi_hw, struct cal_tx_mu_req *req_param);
int cls_wifi_send_cal_sounding_req(struct cls_wifi_hw *cls_wifi_hw, struct cal_sounding_req *req_param);
struct wpu_regs_map *cls_wifi_cal_get_wpu_regs_map(int radio_idx, uint32_t reg_addr);
int cls_wifi_cal_mem_read(int radio_idx, uint32_t addr, int req_words, void *buf);
int cls_wifi_cal_mem_write(int radio_idx, uint32_t addr, int req_words, void *buf);
int cls_wifi_send_cal_dif_smp_req(struct cls_wifi_hw *cls_wifi_hw, struct cal_dif_sample_req *dif_param);
int cls_wifi_send_radar_detect_req(struct cls_wifi_hw *cls_wifi_hw,
		struct cal_radar_detect_req *radar_detect);
int cls_wifi_send_interference_detect_req(struct cls_wifi_hw *cls_wifi_hw,
		struct cal_interference_detect_req *interference_detect);
int cls_wifi_send_rssi_start_req(struct cls_wifi_hw *cls_wifi_hw,
		struct cal_rssi_start_req *rssi_start);
int cls_wifi_send_rssi_stats_req(struct cls_wifi_hw *cls_wifi_hw,
		uint8_t rssi_mode, uint8_t legacy);
int cls_wifi_send_get_temp_req(struct cls_wifi_hw *cls_wifi_hw);
int cls_wifi_save_buf_to_file(const char *filename, const uint8_t *buf, uint32_t buf_len);
int cls_wifi_save_radar_int_detect_result_file(const char *filename, const uint32_t *buf, uint32_t buf_len);
/// Functions for log
int cls_wifi_send_cal_log_set_req(struct cls_wifi_hw *cls_wifi_hw, struct cal_log_set_req *set_req);

#ifdef __KERNEL__
int cls_wifi_test_uncache_rw(struct cls_wifi_hw *cls_wifi_hw,
		struct cal_test_uncache_rw_req *req);
#endif

void cls_wifi_cal_save_work_init(struct cls_wifi_hw *cls_wifi_hw);
void cls_wifi_cal_save_work_deinit(struct cls_wifi_hw *cls_wifi_hw);

int cls_wifi_cal_leaf_timer_read(int radio_idx, uint32_t *us_lo_ptr, uint32_t *us_hi_ptr, uint32_t *ns_ptr);


#endif /* _CLS_WIFI_CALI_H_ */
