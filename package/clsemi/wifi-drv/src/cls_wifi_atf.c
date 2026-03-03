
#include "cls_wifi_utils.h"
#include "cls_wifi_defs.h"
#include "cls_wifi_debugfs.h"
#include "ipc_host.h"
#include "cls_wifi_atf.h"
#include "cls_wifi_msg_tx.h"
#include "vendor.h"

uint16_t cls_wifi_airtime_stats_radio_acc_offset(void)
{
	return (sizeof(struct atf_airtime_info));
}

uint16_t cls_wifi_airtime_stats_radio_offset(void)
{
	uint16_t radio_acc_sz = sizeof(struct atf_per_radio_airtime_stats);

	return (cls_wifi_airtime_stats_radio_acc_offset() + radio_acc_sz);
}

uint16_t cls_wifi_airtime_stats_bss_offset(void)
{
	uint16_t radio_sz = sizeof(struct atf_per_radio_airtime_stats);

	return (cls_wifi_airtime_stats_radio_offset() + radio_sz);
}

uint16_t cls_wifi_airtime_stats_sta_offset(struct cls_wifi_hw *wifi_hw)
{
	uint16_t bss_sz =  hw_vdev_max(wifi_hw) * sizeof(struct atf_per_bss_airtime_stats);

	return (cls_wifi_airtime_stats_bss_offset() + bss_sz);
}

uint16_t cls_wifi_airtime_stats_size(struct cls_wifi_hw *wifi_hw)
{
	uint16_t sta_sz =  (hw_remote_sta_max(wifi_hw) + hw_vdev_max(wifi_hw)) * sizeof(struct atf_per_sta_airtime_stats);

	return (cls_wifi_airtime_stats_sta_offset(wifi_hw) + sta_sz);
}

void cls_wifi_atf_stats_addr_update(struct cls_wifi_hw *wifi_hw)
{
	char *p = (char *)wifi_hw->atf_stats_buf.addr;

	wifi_hw->atf.atf_stats.info = (struct atf_airtime_info *)p;
	wifi_hw->atf.atf_stats.radio_acc =
		(struct atf_per_radio_airtime_stats *)(p + cls_wifi_airtime_stats_radio_acc_offset());
	wifi_hw->atf.atf_stats.radio =
		(struct atf_per_radio_airtime_stats *)(p + cls_wifi_airtime_stats_radio_offset());
	wifi_hw->atf.atf_stats.bss =
		(struct atf_per_bss_airtime_stats *)(p + cls_wifi_airtime_stats_bss_offset());
	wifi_hw->atf.atf_stats.sta =
		(struct atf_per_sta_airtime_stats *)(p + cls_wifi_airtime_stats_sta_offset(wifi_hw));
}

uint16_t cls_wifi_quota_table_size(struct cls_wifi_hw *wifi_hw)
{
	return (hw_remote_sta_max(wifi_hw) * sizeof(uint32_t));
}

static int cls_wifi_is_atf_enabled(struct cls_wifi_hw *wifi_hw)
{
	return (!!wifi_hw->atf.enabled);
}

void cls_wifi_atf_stats_exchange_survey(struct cls_wifi_hw *wifi_hw, struct atf_airtime_stats *stats)
{
	struct cls_wifi_survey_info *cls_wifi_survey;
	struct atf_airtime_info *info = stats->info;
	int idx = cls_wifi_freq_to_idx(wifi_hw, info->freq);

	if (idx > ARRAY_SIZE(wifi_hw->survey))
		return;

	cls_wifi_survey = &wifi_hw->survey[idx];

	// Store the received parameters
	if (info->mode != ATF_CHAN_TYPE_TRAFFIC) {
		cls_wifi_survey->chan_time_ms = info->period / 1000;
		cls_wifi_survey->chan_time_busy_ms = stats->radio_acc->air_cca_busy / 1000;
		cls_wifi_survey->noise_dbm = info->noise_dbm;
	} else {
		cls_wifi_survey->chan_time_ms = (info->end_time - info->start_time) / 1000;
		cls_wifi_survey->chan_time_busy_ms = stats->radio->air_cca_busy / 1000;
	}

	cls_wifi_survey->noise_dbm = info->noise_dbm;
	cls_wifi_survey->filled = (SURVEY_INFO_TIME | SURVEY_INFO_TIME_BUSY);

	if (info->noise_dbm != 0)
		cls_wifi_survey->filled |= SURVEY_INFO_NOISE_DBM;
}

void cls_wifi_atf_stats_process(struct cls_wifi_hw *wifi_hw)
{
	struct atf_airtime_stats *stats;
	struct cls_wifi_vif *vif;
	struct cls_wifi_sta *sta;
	u8 vid;
	u8 sid;

	if (wifi_hw->atf.atf_log)
		pr_info("%s\n", __func__);

	stats = &wifi_hw->atf.atf_stats;

	//invalid cache
	cls_wifi_ipc_buf_e2a_sync(wifi_hw->dev, &wifi_hw->atf_stats_buf, 0);

	// update channel_survey
	cls_wifi_atf_stats_exchange_survey(wifi_hw, stats);

	if (stats->info->mode != ATF_CHAN_TYPE_TRAFFIC) {
		/* off-chan atf ignore stats of per_bss/per_sta */
		goto DUMP_STATS;
	}
	/* count all STAs */
	list_for_each_entry(vif, &wifi_hw->vifs, list) {
		vid = vif->vif_index;
		if ((CLS_WIFI_VIF_TYPE(vif) != NL80211_IFTYPE_AP))
			continue;

		 stats->bss[vid].ul_data_airtime_stats = 0;
		 stats->bss[vid].dl_data_airtime_stats = 0;

		/* loop the STAs */
		list_for_each_entry(sta, &vif->ap.sta_list, list) {
			sid = sta->sta_idx;
			stats->bss[vid].ul_data_airtime_stats += stats->sta[sid].ul_data_airtime_stats;
			stats->bss[vid].dl_data_airtime_stats += stats->sta[sid].dl_data_airtime_stats;
		}
	}

DUMP_STATS:
	if (wifi_hw->atf.atf_log)
		cls_wifi_atf_dump_history_stats(wifi_hw);
}

void cls_wifi_atf_set_bss_quota(struct cls_wifi_vif *vif, uint32_t quota)
{
	pr_info("%s bss quota %u -> %u\n", __func__, vif->atf.quota, quota);
	vif->atf.quota = quota;
}

void cls_wifi_atf_set_sta_quota(struct cls_wifi_sta *sta, uint32_t quota)
{
	pr_info("%s sta quota %u -> %u\n", __func__, sta->atf.quota, quota);
	sta->atf.quota = quota;
}

/* calculate the BSS quota (us) */
void cls_wifi_atf_update_vif_quota_us(struct cls_wifi_hw *wifi_hw)
{
	struct cls_wifi_vif *vif;
	uint32_t total_quota = 0;
	uint32_t vif_num = 0;

//	if (wifi_hw->atf.granularity != CLS_ATF_GRAN_RADIO && wifi_hw->atf.granularity != CLS_ATF_GRAN_BSS)
//		return;

//	if (wifi_hw->atf.granularity == CLS_ATF_GRAN_RADIO) {
//		list_for_each_entry(vif, &wifi_hw->vifs, list) {
//			if ((CLS_WIFI_VIF_TYPE(vif) != NL80211_IFTYPE_AP))
//				continue;
//
//			vif_num++;
//		}
//
//		if (vif_num == 0)
//			return;
//
//		list_for_each_entry(vif, &wifi_hw->vifs, list) {
//			if ((CLS_WIFI_VIF_TYPE(vif) != NL80211_IFTYPE_AP))
//				continue;
//
//			vif->atf.quota_us = wifi_hw->atf.sched_period_us / vif_num;
//		}
//
//		return;
//	}

	list_for_each_entry(vif, &wifi_hw->vifs, list) {
		if ((CLS_WIFI_VIF_TYPE(vif) != NL80211_IFTYPE_AP))
			continue;

		total_quota += vif->atf.quota;
		vif_num++;
	}

	if (vif_num == 0)
		return;

	list_for_each_entry(vif, &wifi_hw->vifs, list) {
		if ((CLS_WIFI_VIF_TYPE(vif) != NL80211_IFTYPE_AP))
			continue;

		if (wifi_hw->atf.granularity < CLS_ATF_GRAN_BSS)
			vif->atf.quota_us = wifi_hw->atf.sched_period_us / vif_num;
		else
			vif->atf.quota_us = (vif->atf.quota * wifi_hw->atf.sched_period_us) /
						total_quota;
		pr_info("quota %u total %u quota_us %u sched_period_us %u\n",
			vif->atf.quota, total_quota, vif->atf.quota_us,
			wifi_hw->atf.sched_period_us);
	}
}

/* calculate the STA quota (us) */
void cls_wifi_atf_update_sta_quota_us(struct cls_wifi_hw *wifi_hw)
{
	struct cls_wifi_vif *vif;
	struct cls_wifi_sta *sta;
	uint32_t total_quota = 0;
	uint32_t sta_num = 0; 

	if (wifi_hw->atf.granularity >= CLS_ATF_GRAN_MAX)
		return;

	list_for_each_entry(vif, &wifi_hw->vifs, list) {
		if ((CLS_WIFI_VIF_TYPE(vif) != NL80211_IFTYPE_AP))
			continue;

		sta_num = 0;
		total_quota = 0;
		list_for_each_entry(sta, &vif->ap.sta_list, list) {
			sta_num++;
			total_quota += sta->atf.quota;
		}

		if (sta_num == 0)
			continue;

		list_for_each_entry(sta, &vif->ap.sta_list, list) {
			if (wifi_hw->atf.granularity < CLS_ATF_GRAN_STA)
				sta->atf.quota_us = vif->atf.quota_us / sta_num;
			else
				sta->atf.quota_us = sta->atf.quota * vif->atf.quota_us / total_quota;
			wifi_hw->atf.quota_table.quota_us[sta->sta_idx] = sta->atf.quota_us;
			pr_info("quota %u total %u quota_us %u\n",
				sta->atf.quota, total_quota, sta->atf.quota_us);
		}
	}
}

/* when new STA associated, assign a default quota */
void cls_wifi_atf_sta_assoc(struct cls_wifi_hw *wifi_hw, struct cls_wifi_sta *sta)
{
	if (!cls_wifi_is_atf_enabled(wifi_hw))
		return;

	cls_wifi_atf_set_sta_quota(sta, CLS_ATF_QUOTA_DEFAULT);
	cls_wifi_atf_update_quota_table(wifi_hw);
}

void cls_wifi_atf_bss_added(struct cls_wifi_hw *wifi_hw, struct cls_wifi_vif *vif)
{
	if (!cls_wifi_is_atf_enabled(wifi_hw))
		return;

	cls_wifi_atf_set_bss_quota(vif, CLS_ATF_QUOTA_DEFAULT);
	cls_wifi_atf_update_quota_table(wifi_hw);
}

/* When STA disassociated */
void cls_wifi_atf_sta_disassoc(struct cls_wifi_hw *wifi_hw, uint8_t sta_index)
{
	if (!cls_wifi_is_atf_enabled(wifi_hw))
		return;

	memset(&wifi_hw->atf.atf_stats.sta[sta_index], 0, sizeof(wifi_hw->atf.atf_stats.sta[sta_index]));
	cls_wifi_atf_update_quota_table(wifi_hw);
}

/* When VIF removed */
void cls_wifi_atf_bss_removed(struct cls_wifi_hw *wifi_hw, uint8_t vif_index)
{
	if (!cls_wifi_is_atf_enabled(wifi_hw))
		return;

	memset(&wifi_hw->atf.atf_stats.bss[vif_index], 0, sizeof(wifi_hw->atf.atf_stats.bss[vif_index]));
	cls_wifi_atf_update_quota_table(wifi_hw);
}

/* Loop VIF and STA, clean quota */
static void cls_wifi_atf_clean_quota(struct cls_wifi_hw *wifi_hw)
{
	struct cls_wifi_vif *vif;
	struct cls_wifi_sta *sta;

	pr_info("%s\n", __func__);
	list_for_each_entry(vif, &wifi_hw->vifs, list) {
		if ((CLS_WIFI_VIF_TYPE(vif) != NL80211_IFTYPE_AP))
			continue;

		vif->atf.quota = CLS_ATF_QUOTA_DEFAULT;
		/* loop the STAs */
		list_for_each_entry(sta, &vif->ap.sta_list, list) {
			sta->atf.quota = CLS_ATF_QUOTA_DEFAULT;
		}
	}
}

static void cls_wifi_atf_clean_airtime_stats(struct cls_wifi_hw *wifi_hw)
{
	memset(wifi_hw->atf.atf_stats.radio, 0, sizeof(struct atf_per_radio_airtime_stats));
	memset(wifi_hw->atf.atf_stats.bss, 0, sizeof(struct atf_per_bss_airtime_stats) * hw_vdev_max(wifi_hw));
	memset(wifi_hw->atf.atf_stats.sta, 0, sizeof(struct atf_per_sta_airtime_stats) * (hw_remote_sta_max(wifi_hw) + hw_vdev_max(wifi_hw)));
}

void cls_wifi_atf_set_enable(struct cls_wifi_hw *wifi_hw, uint8_t enable)
{
	struct mm_atf_params_req req;

	if (wifi_hw->atf.enabled == enable)
		return;

	pr_info("%s enabled %u -> %u\n", __func__, wifi_hw->atf.enabled, enable);
	wifi_hw->atf.enabled = enable;
	cls_wifi_atf_clean_quota(wifi_hw);
	cls_wifi_atf_clean_airtime_stats(wifi_hw);

	req.command = ATF_CMD_ENABLE;
	req.enable = enable;
	req.mode = wifi_hw->atf.mode;
	req.sched_period_us = wifi_hw->atf.sched_period_us;
	req.stats_period_us = wifi_hw->atf.stats_period_us;
	req.stats_busy_thres_us = wifi_hw->atf.stats_busy_thres_us;
	req.stats_clear_thres_us = wifi_hw->atf.stats_clear_thres_us;
	req.stats_clear_duration_us = wifi_hw->atf.stats_clear_duration_us;
	req.srrc_enable = wifi_hw->atf.srrc_enable;

	pr_info("%s sched_period_us %u, stats_period_us %u [busy_thres_us %u, clear_thres_us %u, clear_duration_us %u]\n", __func__,
			wifi_hw->atf.sched_period_us, wifi_hw->atf.stats_period_us,
			wifi_hw->atf.stats_busy_thres_us,
			wifi_hw->atf.stats_clear_thres_us,
			wifi_hw->atf.stats_clear_duration_us);

	cls_wifi_send_atf_update_req(wifi_hw, &req);
}

void cls_wifi_atf_set_mode(struct cls_wifi_hw *wifi_hw, uint8_t mode)
{
	struct mm_atf_params_req req;

	if (wifi_hw->atf.mode == mode)
		return;

	pr_info("%s mode %u -> %u\n", __func__, wifi_hw->atf.mode, mode);
	wifi_hw->atf.mode = mode;

	req.command = ATF_CMD_MODE;
	req.mode = wifi_hw->atf.mode;
	cls_wifi_send_atf_update_req(wifi_hw, &req);
}

void cls_wifi_atf_set_granularity(struct cls_wifi_hw *wifi_hw, uint8_t gran)
{
	pr_info("%s granularity %d -> %u\n", __func__, wifi_hw->atf.granularity, gran);
	wifi_hw->atf.granularity = gran;

	cls_wifi_atf_clean_quota(wifi_hw);

	cls_wifi_atf_update_quota_table(wifi_hw);
}

void cls_wifi_atf_set_bss_quota_from_mac(struct cls_wifi_hw *wifi_hw, uint8_t *bss_mac_addr, uint32_t quota)
{
	struct cls_wifi_vif *vif;

	vif = cls_wifi_get_vif_from_mac(wifi_hw, bss_mac_addr);

	if (!vif) {
		pr_info("%s bss(%pM) not exist\n", __func__, bss_mac_addr);
		return;
	}

	cls_wifi_atf_set_bss_quota(vif, quota);
}

void cls_wifi_atf_set_sta_quota_from_mac(struct cls_wifi_hw *wifi_hw, uint8_t *sta_mac_addr, uint32_t quota)
{
	struct cls_wifi_sta *sta;

	sta = cls_wifi_get_sta_from_mac(wifi_hw, sta_mac_addr);

	if (!sta) {
		pr_info("%s sta(%pM) not exist\n", __func__, sta_mac_addr);
		return;
	}

	cls_wifi_atf_set_sta_quota(sta, quota);
}

/* Loop all associated STAs and update quota table */
void cls_wifi_atf_update_quota_table(struct cls_wifi_hw *wifi_hw)
{
	struct mm_atf_params_req req;

	if (!cls_wifi_is_atf_enabled(wifi_hw))
		return;

	pr_info("%s\n", __func__);
	cls_wifi_atf_update_vif_quota_us(wifi_hw);
	cls_wifi_atf_update_sta_quota_us(wifi_hw);

	/* TODO: update quota to WPU */
	req.command = ATF_CMD_UPDATE_QUOTA_TABLE;
	cls_wifi_send_atf_update_req(wifi_hw, &req);
}

void cls_wifi_atf_set_sched_period(struct cls_wifi_hw *wifi_hw, uint32_t sched_period)
{
	struct mm_atf_params_req req;

	if (wifi_hw->atf.sched_period_us == sched_period)
		return;

	pr_info("%s sched_period %u -> %u\n", __func__, wifi_hw->atf.sched_period_us, sched_period);
	wifi_hw->atf.sched_period_us = sched_period;

	req.command = ATF_CMD_SCHED_PERIOD;
	req.sched_period_us = wifi_hw->atf.sched_period_us;
	cls_wifi_send_atf_update_req(wifi_hw, &req);
}

void cls_wifi_atf_set_stats_period(struct cls_wifi_hw *wifi_hw, uint32_t stats_period)
{
	struct mm_atf_params_req req;

	if (wifi_hw->atf.stats_period_us == stats_period)
		return;

	pr_info("%s stats_period %u -> %u\n", __func__, wifi_hw->atf.stats_period_us, stats_period);
	wifi_hw->atf.stats_period_us = stats_period;

	req.command = ATF_CMD_STATS_PERIOD;
	req.stats_period_us = wifi_hw->atf.stats_period_us;
	cls_wifi_send_atf_update_req(wifi_hw, &req);
}

void cls_wifi_atf_set_stats_busy_thres(struct cls_wifi_hw *wifi_hw, uint32_t stats_thres)
{
	struct mm_atf_params_req req;

	if (wifi_hw->atf.stats_busy_thres_us == stats_thres)
		return;

	pr_info("%s stats_busy_thres %u -> %u\n", __func__, wifi_hw->atf.stats_busy_thres_us, stats_thres);
	wifi_hw->atf.stats_busy_thres_us = stats_thres;

	req.command = ATF_CMD_STATS_BUSY_THRES;
	req.stats_busy_thres_us = wifi_hw->atf.stats_busy_thres_us;
	cls_wifi_send_atf_update_req(wifi_hw, &req);
}

void cls_wifi_atf_set_stats_clear_thres(struct cls_wifi_hw *wifi_hw, uint32_t stats_thres)
{
	struct mm_atf_params_req req;

	if (wifi_hw->atf.stats_clear_thres_us == stats_thres)
		return;

	pr_info("%s stats_clear_thres %u -> %u\n", __func__, wifi_hw->atf.stats_clear_thres_us, stats_thres);
	wifi_hw->atf.stats_clear_thres_us = stats_thres;

	req.command = ATF_CMD_STATS_CLEAR_THRES;
	req.stats_clear_thres_us = wifi_hw->atf.stats_clear_thres_us;
	cls_wifi_send_atf_update_req(wifi_hw, &req);
}

void cls_wifi_atf_set_stats_clear_duration(struct cls_wifi_hw *wifi_hw, uint32_t stats_duration)
{
	struct mm_atf_params_req req;

	if (wifi_hw->atf.stats_clear_duration_us == stats_duration)
		return;

	pr_info("%s stats_clear_duration %u -> %u\n", __func__, wifi_hw->atf.stats_clear_duration_us, stats_duration);
	wifi_hw->atf.stats_clear_duration_us = stats_duration;

	req.command = ATF_CMD_STATS_CLEAR_DURATION;
	req.stats_clear_duration_us = wifi_hw->atf.stats_clear_duration_us;
	cls_wifi_send_atf_update_req(wifi_hw, &req);
}


void cls_wifi_atf_set_srrc_enable(struct cls_wifi_hw *wifi_hw, uint8_t srrc_enable)
{
	struct mm_atf_params_req req;

	if (wifi_hw->atf.srrc_enable == srrc_enable)
		return;

	pr_info("%s srrc_enable %u -> %u\n", __func__, wifi_hw->atf.srrc_enable, srrc_enable);
	wifi_hw->atf.srrc_enable = srrc_enable;

	req.command = ATF_CMD_SRRC_ENABLE;
	req.srrc_enable = wifi_hw->atf.srrc_enable;
	cls_wifi_send_atf_update_req(wifi_hw, &req);
}

void cls_wifi_atf_set_log_enable(struct cls_wifi_hw *wifi_hw, uint8_t atf_log)
{
	if (wifi_hw->atf.atf_log == atf_log)
		return;

	pr_info("%s drv atf_log %u -> %u\n", __func__, wifi_hw->atf.atf_log, atf_log);
	wifi_hw->atf.atf_log = atf_log;
}

void cls_wifi_atf_get_stats(struct cls_wifi_hw *wifi_hw)
{
	struct mm_atf_params_req req;

	pr_info("%s\n", __func__);

	req.command = ATF_CMD_GET_STATS;
	cls_wifi_send_atf_update_req(wifi_hw, &req);
}

/* dump quota table */
void cls_wifi_atf_dump_quota_table(struct cls_wifi_hw *wifi_hw)
{
	struct cls_wifi_vif *vif;
	struct cls_wifi_sta *sta;

	pr_info("%s\n", __func__);
	list_for_each_entry(vif, &wifi_hw->vifs, list) {
		if ((CLS_WIFI_VIF_TYPE(vif) != NL80211_IFTYPE_AP))
			continue;

		pr_info("VIF %pM quota %u quota_us %u\n",
			vif->ndev->dev_addr, vif->atf.quota, vif->atf.quota_us);

		/* loop the STAs */
		list_for_each_entry(sta, &vif->ap.sta_list, list)
			pr_info("\tSTA %pM quota %u quota_us %u\n",
				sta->mac_addr, sta->atf.quota, sta->atf.quota_us);
	}
}

void cls_wifi_atf_dump_history_stats(struct cls_wifi_hw *wifi_hw)
{
	struct cls_wifi_vif *vif;
	struct cls_wifi_sta *sta;
	struct cfg80211_chan_def *chandef;
	struct atf_airtime_stats *stats = &wifi_hw->atf.atf_stats;
	struct atf_airtime_info *info = stats->info;
	u8 vid;
	u8 sid;
	u8 bw;
	u8 width[] = {20, 20, 40, 80, 80, 160};

	pr_info("%s\n", __func__);

	/* TODO: dump quota(us) and compare with history stats */
	chandef = &wifi_hw->chanctx_table[wifi_hw->cur_chanctx].chan_def;
	bw = irf_get_curr_bw(wifi_hw);
	if (bw <= PHY_CHNL_BW_160) {
		pr_info("Radio %u BW %uMHz\n""Channel %d, prim20 freq %uMHz, central freq %uMHz\n",
			wifi_hw->radio_idx, width[chandef->width],
			wifi_hw->radio_idx ? cls_wifi_mod_params.chan_ieee_5g :
				cls_wifi_mod_params.chan_ieee_2g,
			chandef->center_freq1 - width[chandef->width] / 2 + 10, chandef->center_freq1);
	} else if (bw == PHY_CHNL_BW_80P80) {
		pr_info("Radio %u BW 160MHz(80P80)\n""central freq 1 %uMHz, central freq 2 %uMHz\n",
			wifi_hw->radio_idx,
			chandef->center_freq1, chandef->center_freq2);
	} else {
		pr_info("Radio %u\n""central freq %uMHz\n",
			wifi_hw->radio_idx,
			chandef->center_freq1);
	}

	if (info->mode != ATF_CHAN_TYPE_TRAFFIC) {
		pr_info("Scan(%d) ATF status: period: %u, freq: %u, noise_dbm: %d\n",
			info->mode, info->period, info->freq, info->noise_dbm);

		pr_info("cca_busy_prim20: %u, cca_busy_sec20: %u, cca_busy_sec40: %u, cca_busy_sec80: %u,\n"
			"air_cca_busy: %u, unknown_ul_data_airtime: %u, ul_data_airtime: %u, ul_mgmt_ctrl_aritime: %u\n",
			stats->radio_acc->cca_busy_prim20, stats->radio_acc->cca_busy_sec20,
			stats->radio_acc->cca_busy_sec40, stats->radio_acc->cca_busy_sec80,
			stats->radio_acc->air_cca_busy, stats->radio_acc->unknown_ul_data_airtime,
			stats->radio_acc->ul_data_airtime, stats->radio_acc->mgmt_ctrl_aritime);
		return;
	} else {
		// Normal atf
		pr_info("Normal ATF status: period: %u(%u - %u), freq: %u, noise_dbm: %d\n",
			info->period, info->start_time, info->end_time, info->freq, info->noise_dbm);

		pr_info("cca_busy_prim20: %u, cca_busy_sec20: %u, cca_busy_sec40: %u, cca_busy_sec80: %u,\n"
			"air_cca_busy: %u, unknown_ul_data_airtime: %u, ul_data_airtime: %u, ul_mgmt_ctrl_aritime: %u\n",
			stats->radio->cca_busy_prim20, stats->radio->cca_busy_sec20,
			stats->radio->cca_busy_sec40, stats->radio->cca_busy_sec80,
			stats->radio->air_cca_busy, stats->radio->unknown_ul_data_airtime,
			stats->radio->ul_data_airtime, stats->radio->mgmt_ctrl_aritime);
	}

	list_for_each_entry(vif, &wifi_hw->vifs, list) {
		vid = vif->vif_index;
		if ((CLS_WIFI_VIF_TYPE(vif) != NL80211_IFTYPE_AP))
			continue;

		pr_info("VIF %pM vid %u\n"
			"ul_data_airtime_stats: %u, dl_data_airtime_stats: %u, dl_mgmt_ctrl_aritime_stats: %u\n",
				vif->ndev->dev_addr, vid,
				stats->bss[vid].ul_data_airtime_stats,
				stats->bss[vid].dl_data_airtime_stats,
				stats->bss[vid].dl_mgmt_ctrl_aritime_stats);

		/* loop the STAs */
		list_for_each_entry(sta, &vif->ap.sta_list, list) {
			sid = sta->sta_idx;
			pr_info("\tSTA %pM sid %u\n"
				"\tul_data_airtime_stats: %u, dl_data_airtime_stats: %u\n",
					sta->mac_addr, sid,
					stats->sta[sid].ul_data_airtime_stats,
					stats->sta[sid].dl_data_airtime_stats);
		}
	}
}

void cls_wifi_atf_init(struct cls_wifi_hw *wifi_hw)
{
	pr_info("%s\n", __func__);
	wifi_hw->atf.enabled = 0;
	wifi_hw->atf.mode = CLS_ATF_MODE_MONITOR;
	wifi_hw->atf.stats_period_us = CLS_ATF_STATS_PERIOD_DEFAULT;
	wifi_hw->atf.stats_busy_thres_us = CLS_ATF_STATS_BUSY_THRES_DEFAULT;
	wifi_hw->atf.stats_clear_thres_us = CLS_ATF_STATS_CLEAR_THRES_DEFAULT;
	wifi_hw->atf.stats_clear_duration_us = CLS_ATF_STATS_CLEAR_DURATION_DEFAULT;
	wifi_hw->atf.atf_log = 0;
	wifi_hw->atf.srrc_enable = 0;
	wifi_hw->atf.sched_period_us = CLS_ATF_SCHED_PERIOD_DEFAULT;
	wifi_hw->atf.quota_table.quota_us = (uint32_t *)wifi_hw->atf_quota_buf.addr;
	cls_wifi_atf_stats_addr_update(wifi_hw);
}

void cls_wifi_vndr_event_report_atf_stats(struct cls_wifi_hw *wifi_hw)
{
	struct sk_buff *skb;

	/* send event, event index 2 is CLS_NL80211_CMD_REPORT_CSI */
	skb = cfg80211_vendor_event_alloc(wifi_hw->wiphy, NULL,
			(cls_wifi_airtime_stats_size(wifi_hw) + 100),
			CLS_NL80211_CMD_REPORT_ATF_STATS, GFP_KERNEL);
	if (skb) {
		pr_info("%s\n", __func__);
		nla_put(skb, CLS_NL80211_ATTR_ATF_STATS, cls_wifi_airtime_stats_size(wifi_hw),
				&wifi_hw->atf_stats_buf);
		cfg80211_vendor_event(skb, GFP_KERNEL);
	}
}


/* Set the ATF enable 0/1 */
int clsemi_vndr_cmds_set_atf_enable(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len)
{
	struct cls_wifi_hw *cls_wifi_hw = NULL;
	uint8_t atf_enable;
	int rc;
	struct nlattr *tb[CLS_NL80211_ATTR_MAX + 1];

	cls_wifi_hw = wiphy_priv(wiphy);

	rc = nla_parse(tb, CLS_NL80211_ATTR_MAX, data,
			len, cls_vnd_cmd_policy, NULL);
	if (rc) {
		pr_warn("%s Invalid ATF ATTR\n", __func__);

		return rc;
	}

	if (!tb[CLS_NL80211_ATTR_ATF_ENABLE]) {
		pr_warn("%s Invalid ATF enable ATTR\n", __func__);

		return -EINVAL;
	}

	atf_enable = nla_get_u8(tb[CLS_NL80211_ATTR_ATF_ENABLE]);
	pr_warn("%s atf enable: %u -> %u\n", __func__, cls_wifi_hw->atf.enabled, atf_enable);
	cls_wifi_atf_set_enable(cls_wifi_hw, atf_enable);

	return 0;
}

/* Set the ATF mode enum cls_atf_mode */
int clsemi_vndr_cmds_set_atf_mode(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len)
{
	struct cls_wifi_hw *cls_wifi_hw = NULL;
	uint8_t atf_mode;
	int rc;
	struct nlattr *tb[CLS_NL80211_ATTR_MAX + 1];

	cls_wifi_hw = wiphy_priv(wiphy);

	rc = nla_parse(tb, CLS_NL80211_ATTR_MAX, data,
			len, cls_vnd_cmd_policy, NULL);
	if (rc) {
		pr_warn("%s Invalid ATF ATTR\n", __func__);
		return rc;
	}

	if (!tb[CLS_NL80211_ATTR_ATF_MODE]) {
		pr_warn("%s Invalid ATF mode ATTR\n", __func__);

		return -EINVAL;
	}

	atf_mode = nla_get_u8(tb[CLS_NL80211_ATTR_ATF_MODE]);

	if(atf_mode >= CLS_ATF_MODE_MAX) {
		pr_warn("%s Invalid ATF mode\n", __func__);
		return -EINVAL;
	}

	pr_warn("%s atf mode: %u -> %u\n", __func__, cls_wifi_hw->atf.mode, atf_mode);
	cls_wifi_atf_set_mode(cls_wifi_hw, atf_mode);

	return 0;
}

/* Set atf gran */
int clsemi_vndr_cmds_set_atf_granularity(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len)
{
	struct cls_wifi_hw *cls_wifi_hw = NULL;
	uint8_t atf_gran;
	int rc;
	struct nlattr *tb[CLS_NL80211_ATTR_MAX + 1];

	cls_wifi_hw = wiphy_priv(wiphy);

	rc = nla_parse(tb, CLS_NL80211_ATTR_MAX, data,
			len, cls_vnd_cmd_policy, NULL);
	if (rc) {
		pr_warn("%s Invalid ATF ATTR\n", __func__);
		return rc;
	}

	if (!tb[CLS_NL80211_ATTR_ATF_GRAN]) {
		pr_warn("%s Invalid ATF gran ATTR\n", __func__);

		return -EINVAL;
	}

	atf_gran = nla_get_u8(tb[CLS_NL80211_ATTR_ATF_GRAN]);

	if(atf_gran >= CLS_ATF_GRAN_MAX) {
		pr_warn("%s Invalid ATF granularity\n", __func__);
		return -EINVAL;
	}

	pr_warn("%s atf granularity: %u -> %u\n", __func__, cls_wifi_hw->atf.granularity, atf_gran);
	cls_wifi_atf_set_granularity(cls_wifi_hw, atf_gran);

	return 0;
}

/* Set atf bss quota */
int clsemi_vndr_cmds_set_atf_bss_quota(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len)
{
	struct cls_wifi_hw *cls_wifi_hw = NULL;
	uint32_t atf_bss_quota;
	uint8_t bss_mac_addr[6];
	int rc;
	struct nlattr *tb[CLS_NL80211_ATTR_MAX + 1];

	cls_wifi_hw = wiphy_priv(wiphy);

	rc = nla_parse(tb, CLS_NL80211_ATTR_MAX, data,
			len, cls_vnd_cmd_policy, NULL);
	if (rc) {
		pr_warn("%s Invalid ATF ATTR\n", __func__);
		return rc;
	}

	if (!tb[CLS_NL80211_ATTR_ATF_BSS_QUOTA] || !tb[CLS_NL80211_ATTR_MAC_ADDR]) {
		pr_warn("%s Invalid ATF bss quota ATTR\n", __func__);

		return -EINVAL;
	}

	atf_bss_quota = nla_get_u32(tb[CLS_NL80211_ATTR_ATF_BSS_QUOTA]);
	ether_addr_copy(bss_mac_addr, nla_data(tb[CLS_NL80211_ATTR_MAC_ADDR]));

	pr_warn("%s atf bss(%pM) quota: %u\n", __func__, bss_mac_addr, atf_bss_quota);
	cls_wifi_atf_set_bss_quota_from_mac(cls_wifi_hw, bss_mac_addr, atf_bss_quota);

	return 0;
}

/* Set atf sta quota */
int clsemi_vndr_cmds_set_atf_sta_quota(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len)
{
	struct cls_wifi_hw *cls_wifi_hw = NULL;
	uint32_t atf_sta_quota;
	uint8_t sta_mac_addr[6];
	int rc;
	struct nlattr *tb[CLS_NL80211_ATTR_MAX + 1];

	cls_wifi_hw = wiphy_priv(wiphy);

	rc = nla_parse(tb, CLS_NL80211_ATTR_MAX, data,
			len, cls_vnd_cmd_policy, NULL);
	if (rc) {
		pr_warn("%s Invalid ATF ATTR\n", __func__);
		return rc;
	}

	if (!tb[CLS_NL80211_ATTR_ATF_STA_QUOTA] || !tb[CLS_NL80211_ATTR_MAC_ADDR]) {
		pr_warn("%s Invalid ATF sta quota ATTR\n", __func__);

		return -EINVAL;
	}

	atf_sta_quota = nla_get_u32(tb[CLS_NL80211_ATTR_ATF_STA_QUOTA]);
	ether_addr_copy(sta_mac_addr, nla_data(tb[CLS_NL80211_ATTR_MAC_ADDR]));

	pr_warn("%s atf sta(%pM) quota: %u\n", __func__, sta_mac_addr, atf_sta_quota);
	cls_wifi_atf_set_sta_quota_from_mac(cls_wifi_hw, sta_mac_addr, atf_sta_quota);

	return 0;
}

/* Update atf quota table to FW */
int clsemi_vndr_cmds_update_atf_quota_table(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len)
{
	struct cls_wifi_hw *cls_wifi_hw = NULL;

	cls_wifi_hw = wiphy_priv(wiphy);

	if (!cls_wifi_hw) {
		pr_warn("%s Invalid wiphy\n", __func__);
		return -EINVAL;
	}

	pr_warn("%s\n", __func__);
	cls_wifi_atf_update_quota_table(cls_wifi_hw);

	return 0;
}

/* Set the ATF sched period */
int clsemi_vndr_cmds_set_atf_sched_period(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len)
{
	struct cls_wifi_hw *cls_wifi_hw = NULL;
	uint32_t sched_period;
	int rc;
	struct nlattr *tb[CLS_NL80211_ATTR_MAX + 1];

	cls_wifi_hw = wiphy_priv(wiphy);

	rc = nla_parse(tb, CLS_NL80211_ATTR_MAX, data,
			len, cls_vnd_cmd_policy, NULL);
	if (rc) {
		pr_warn("%s Invalid ATF ATTR\n", __func__);
		return rc;
	}

	if (!tb[CLS_NL80211_ATTR_ATF_SCHED_PERIOD]) {
		pr_warn("%s Invalid ATF sched period ATTR\n", __func__);
		return -EINVAL;
	}

	sched_period = nla_get_u32(tb[CLS_NL80211_ATTR_ATF_SCHED_PERIOD]);
	pr_warn("%s atf sched period: %u -> %u\n", __func__, cls_wifi_hw->atf.sched_period_us, sched_period);
	cls_wifi_atf_set_sched_period(cls_wifi_hw, sched_period);

	return 0;
}

/* Set the ATF stats period */
int clsemi_vndr_cmds_set_atf_stats_period(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len)
{
	struct cls_wifi_hw *cls_wifi_hw = NULL;
	uint32_t stats_period;
	int rc;
	struct nlattr *tb[CLS_NL80211_ATTR_MAX + 1];

	cls_wifi_hw = wiphy_priv(wiphy);

	rc = nla_parse(tb, CLS_NL80211_ATTR_MAX, data,
			len, cls_vnd_cmd_policy, NULL);
	if (rc) {
		pr_warn("%s Invalid ATF ATTR\n", __func__);
		return rc;
	}

	if (!tb[CLS_NL80211_ATTR_ATF_STATS_PERIOD]) {
		pr_warn("%s Invalid ATF sched period ATTR\n", __func__);
		return -EINVAL;
	}

	stats_period = nla_get_u32(tb[CLS_NL80211_ATTR_ATF_STATS_PERIOD]);
	pr_warn("%s atf stats period: %u -> %u\n", __func__, cls_wifi_hw->atf.stats_period_us, stats_period);
	cls_wifi_atf_set_stats_period(cls_wifi_hw, stats_period);

	return 0;
}

/* Get atf stats from FW */
int clsemi_vndr_cmds_get_atf_stats(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len)
{
	struct cls_wifi_hw *cls_wifi_hw = NULL;

	cls_wifi_hw = wiphy_priv(wiphy);

	if (!cls_wifi_hw) {
		pr_warn("%s Invalid wiphy\n", __func__);
		return -EINVAL;
	}

	pr_warn("%s\n", __func__);
	cls_wifi_atf_get_stats(cls_wifi_hw);

	return 0;
}

