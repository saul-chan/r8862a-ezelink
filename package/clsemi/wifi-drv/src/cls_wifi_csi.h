#ifndef _CLS_WIFI_CSI_H_
#define _CLS_WIFI_CSI_H_

#define CLS_CSI_MAX_STA_NUM 10
#define CSI_MAX_SUBCARRIER_NUM 2048
#define CSI_HEAD_INFO_LEN 16
#define CSI_EXTRA_REPORT_INFO_LEN 128
#define CSI_REPORT_INFO_MAX_SIZE (CSI_EXTRA_REPORT_INFO_LEN - CSI_HEAD_INFO_LEN)

enum {
	CSI_CMD_ENABLE,
	CSI_CMD_ADD_STA,
	CSI_CMD_REMOVE_STA,
	CSI_CMD_DUMP_INFO,
	CSI_CMD_LOG_LEVEL,
	CSI_CMD_SET_PERIOD,
	CSI_CMD_SET_FORMAT,
	CSI_CMD_SET_SMOOTH,

	/* Host only */
	CSI_CMD_SAVE_FILE,
	CSI_CMD_MAX,
};

/* The struct requested by Customer */
struct cls_csi_report_info {
	uint32_t timestamp_hi;
	uint32_t timestamp_lo;
	uint8_t dest_mac_addr[6];
	uint8_t sta_mac_addr[6];
	int8_t rssi;
	uint8_t rx_nss;
	uint8_t tx_nss;
	uint8_t channel;
	uint8_t bw;
	uint16_t subcarrier_num;
	/* enum hw_format_mod */
	uint8_t ppdu_format;
	/* enum csi_src_ltf_type */
	uint8_t ltf_type;
	uint8_t xx_ltf_type;
	uint8_t agc[8];
#ifdef __KERNEL__
} __packed;
#else
} __PACKED;
#endif

union cls_csi_report_info_aligned {
	struct cls_csi_report_info info;
	uint8_t csi_inf_reserved[CSI_REPORT_INFO_MAX_SIZE];
};

/* Head of CSI info */
struct cls_csi_info_head {
	uint16_t ppdu_id;
	uint16_t csi_length;
#if defined(CFG_MERAK3000)
	uint8_t csi_format : 4;
	uint8_t reserved_1 : 4;
	uint8_t reserved_2[11];
#else
	uint32_t reserved[3];
#endif
};

/* The information in 1 subcarrier */
#if defined(CFG_MERAK3000)
struct cls_csi_info_per_subcarrier {
	uint16_t h00_q;
	uint16_t h00_i;
	uint16_t h10_q;
	uint16_t h10_i;
	uint16_t h20_q;
	uint16_t h20_i;
	uint16_t h01_q;
	uint16_t h01_i;
	uint16_t h11_q;
	uint16_t h11_i;
	uint16_t h21_q;
	uint16_t h21_i;
};
#else
struct cls_csi_info_per_subcarrier {
	uint16_t h00_q;
	uint16_t h00_i;
	uint16_t h01_q;
	uint16_t h01_i;
	uint16_t h10_q;
	uint16_t h10_i;
	uint16_t h11_q;
	uint16_t h11_i;
};
#endif

struct cls_csi_report_info_iq {
	struct cls_csi_info_per_subcarrier subc[CSI_MAX_SUBCARRIER_NUM];
};

/* CSI report to Host side */
struct cls_csi_report {
	union cls_csi_report_info_aligned info_aligned;
	struct cls_csi_info_head hdr;
	struct cls_csi_report_info_iq csi_iq;
};

struct cls_wifi_csi_sta {
	/* if associated, point to the STA. If not, NULL */
	struct cls_wifi_sta *sta;
	uint8_t mac_addr[6];
#define CLS_CSI_STA_ENABLED 0x1
#define CLS_CSI_STA_ASSOCIATED 0x2
	uint16_t flags;
	uint16_t sta_idx;
	uint16_t hw_key_idx;
};


struct cls_wifi_csi_params {
	/* disabled = 0, enabled = 1*/
	uint8_t enabled;
	uint8_t enable_non_assoc;
	uint8_t csi_smooth;
	uint8_t csi_buffer_num;
	uint8_t log_level;
	uint8_t save_csi_to_file;
	uint16_t csi_report_period;
	uint16_t format_mask;
	/* monitored STAs */
	struct cls_wifi_csi_sta sta[CLS_CSI_MAX_STA_NUM];
	uint8_t csi_sta_num;
};

#ifdef __KERNEL__
void cls_wifi_process_csi_report(struct cls_wifi_hw *cls_wifi_hw,
		struct cls_csi_report *report);
int cls_wifi_enable_csi(struct cls_wifi_hw *wifi_hw, int enable);
int cls_wifi_add_csi_sta(struct cls_wifi_hw *wifi_hw, uint8_t *mac_addr);
void cls_wifi_add_associated_csi_sta(struct cls_wifi_hw *wifi_hw, struct cls_wifi_sta *sta);
int cls_wifi_remove_csi_sta(struct cls_wifi_hw *wifi_hw, uint8_t *mac_addr);
int cls_wifi_set_csi_period(struct cls_wifi_hw *wifi_hw, int period);
int cls_wifi_set_csi_format(struct cls_wifi_hw *wifi_hw, uint32_t format);
int cls_wifi_dump_csi_info(struct cls_wifi_hw *wifi_hw);
int cls_wifi_set_csi_log_level(struct cls_wifi_hw *wifi_hw, int log_level);
int cls_wifi_set_he_ltf_smooth(struct cls_wifi_hw *wifi_hw, uint8_t smooth);
int cls_wifi_set_save_file(struct cls_wifi_hw *wifi_hw, uint8_t save_csi_to_file);
int cls_wifi_csi_init(struct cls_wifi_hw *wifi_hw);
int clsemi_vndr_cmds_csi_set(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len);
int clsemi_vndr_cmds_csi_get(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len);
int clsemi_vndr_cmds_add_csi_sta(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len);
int clsemi_vndr_cmds_del_csi_sta(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len);
int clsemi_vndr_cmds_get_csi_sta(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len);
#endif
#endif // _CLS_WIFI_CSI_H_