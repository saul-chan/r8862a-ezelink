#ifndef _CLS_WIFI_BW_MONITOR_H_
#define _CLS_WIFI_BW_MONITOR_H_

#if defined(CONFIG_DEBUG_FS) && defined(CONFIG_WIFI_TRAFFIC_MONITOR)
#define WBM_STATS_NUM	8
struct wtm_stats {
	struct {
		u32 rx_pkts;
		u32 tx_pkts;
		u32 rx_bytes;
		u32 tx_bytes;
		s32 rssi_sum;
		unsigned long ts;
	} stats[WBM_STATS_NUM];
	u32 cur_idx;
	u32 peak_rx_pkts;
	u32 peak_tx_pkts;
	u32 peak_rx_bytes;
	u32 peak_tx_bytes;
	s16 peak_rssi;
};

void cls_wtm_init_debugfs(void);
void cls_wtm_deinit_debugfs(void);
void cls_wtm_add_phy(struct cls_wifi_hw *cls_wifi_hw);
void cls_wtm_del_phy(struct cls_wifi_hw *cls_wifi_hw);
void cls_wtm_update_stats(struct cls_wifi_hw *phy, struct cls_wifi_vif *vif, struct cls_wifi_sta *sta,
			  u32 bytes, s16 rssi, bool is_rx);
void cls_wtm_reset_sta_peak_data(struct cls_wifi_sta *sta);
#else
static inline void cls_wtm_init_debugfs(void) {}
static inline void cls_wtm_deinit_debugfs(void) {}
static inline void cls_wtm_add_phy(struct cls_wifi_hw *cls_wifi_hw) {}
static inline void cls_wtm_del_phy(struct cls_wifi_hw *cls_wifi_hw) {}
static inline void cls_wtm_update_stats(struct cls_wifi_hw *phy, struct cls_wifi_vif *vif, struct cls_wifi_sta *sta,
					u32 bytes, s16 rssi, bool is_rx) {}
static inline void cls_wtm_reset_sta_peak_data(struct cls_wifi_sta *sta) {}
#endif

#endif
