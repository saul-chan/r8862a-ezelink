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
#ifdef __KERNEL__
#include "cls_wifi_defs.h"
#include "cls_wifi_prof.h"
#include "cls_wifi_tx.h"
#ifdef CONFIG_CLS_WIFI_BFMER
#include "cls_wifi_bfmer.h"
#endif //(CONFIG_CLS_WIFI_BFMER)
#include "cls_wifi_debugfs.h"
#include "cls_wifi_msg_tx.h"
#include "cls_wifi_tdls.h"
#include "cls_wifi_events.h"
#include "cls_wifi_compat.h"
#include "cls_wifi_cali.h"
#include "cls_wifi_cfgfile.h"
#include "cls_wifi_irf.h"
#include "cls_wifi_dif_sm.h"

/***************************************************************************
 * Messages from MM task
 **************************************************************************/
static inline int cls_wifi_rx_chan_pre_switch_ind(struct cls_wifi_hw *cls_wifi_hw,
											  struct cls_wifi_cmd *cmd,
											  struct ipc_e2a_msg *msg)
{
	struct cls_wifi_vif *vif;
	int chan_idx = ((struct mm_channel_pre_switch_ind *)msg->param)->chan_index;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	REG_SW_SET_PROFILING_CHAN(cls_wifi_hw, SW_PROF_CHAN_CTXT_PSWTCH_BIT);

	list_for_each_entry(vif, &cls_wifi_hw->vifs, list) {
		if (vif->up && vif->ch_index == chan_idx) {
			cls_wifi_txq_vif_stop(vif, CLS_WIFI_TXQ_STOP_CHAN, cls_wifi_hw);
		}
	}

	REG_SW_CLEAR_PROFILING_CHAN(cls_wifi_hw, SW_PROF_CHAN_CTXT_PSWTCH_BIT);

	return 0;
}

static inline int cls_wifi_rx_chan_switch_ind(struct cls_wifi_hw *cls_wifi_hw,
										  struct cls_wifi_cmd *cmd,
										  struct ipc_e2a_msg *msg)
{
	struct cls_wifi_vif *vif, *curr_vif;
	int chan_idx = ((struct mm_channel_switch_ind *)msg->param)->chan_index;
	bool roc_req = ((struct mm_channel_switch_ind *)msg->param)->roc;
	bool roc_tdls = ((struct mm_channel_switch_ind *)msg->param)->roc_tdls;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	REG_SW_SET_PROFILING_CHAN(cls_wifi_hw, SW_PROF_CHAN_CTXT_SWTCH_BIT);

	if (roc_tdls) {
		u8 vif_index = ((struct mm_channel_switch_ind *)msg->param)->vif_index;
		list_for_each_entry(vif, &cls_wifi_hw->vifs, list) {
			if (vif->vif_index == vif_index) {
				vif->roc_tdls = true;
				curr_vif = vif;
				cls_wifi_txq_tdls_sta_start(vif, CLS_WIFI_TXQ_STOP_CHAN, cls_wifi_hw);
			}
		}
	} else if (!roc_req) {
		if (cls_wifi_hw->csa_fin_flag) {
			cls_wifi_hw->csa_fin_flag = false;
			schedule_work(&cls_wifi_hw->csa_delay_cali_work);
		} else {
			list_for_each_entry(vif, &cls_wifi_hw->vifs, list) {
				if (vif->up && vif->ch_index == chan_idx) {
					curr_vif = vif;
					cls_wifi_txq_vif_start(vif, CLS_WIFI_TXQ_STOP_CHAN, cls_wifi_hw);
				}
			}
		}
	} else {
		struct cls_wifi_roc *roc = cls_wifi_hw->roc;
		vif = roc->vif;
		curr_vif = vif;
		trace_switch_roc(vif->vif_index);

		if (!roc->internal) {
			// If RoC has been started by the user space, inform it that we have
			// switched on the requested off-channel
			cfg80211_ready_on_channel(&vif->wdev, (u64)(uintptr_t)(roc),
									  roc->chan, roc->duration, GFP_ATOMIC);
		}

		// Keep in mind that we have switched on the channel
		roc->on_chan = true;
		// Enable traffic on OFF channel queue
		cls_wifi_txq_offchan_start(cls_wifi_hw);
	}

	cls_wifi_hw->cur_chanctx = chan_idx;
	cls_wifi_radar_detection_enable_on_cur_channel(cls_wifi_hw, curr_vif);

	REG_SW_CLEAR_PROFILING_CHAN(cls_wifi_hw, SW_PROF_CHAN_CTXT_SWTCH_BIT);

	return 0;
}

static inline int cls_wifi_rx_tdls_chan_switch_cfm(struct cls_wifi_hw *cls_wifi_hw,
												struct cls_wifi_cmd *cmd,
												struct ipc_e2a_msg *msg)
{
	return 0;
}

static inline int cls_wifi_rx_tdls_chan_switch_ind(struct cls_wifi_hw *cls_wifi_hw,
											   struct cls_wifi_cmd *cmd,
											   struct ipc_e2a_msg *msg)
{
	// Enable traffic on OFF channel queue
	cls_wifi_txq_offchan_start(cls_wifi_hw);

	return 0;
}

static inline int cls_wifi_rx_tdls_chan_switch_base_ind(struct cls_wifi_hw *cls_wifi_hw,
													struct cls_wifi_cmd *cmd,
													struct ipc_e2a_msg *msg)
{
	struct cls_wifi_vif *vif;
	u8 vif_index = ((struct tdls_chan_switch_base_ind *)msg->param)->vif_index;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	list_for_each_entry(vif, &cls_wifi_hw->vifs, list) {
		if (vif->vif_index == vif_index) {
			vif->roc_tdls = false;
			cls_wifi_txq_tdls_sta_stop(vif, CLS_WIFI_TXQ_STOP_CHAN, cls_wifi_hw);
		}
	}
	return 0;
}

static inline int cls_wifi_rx_tdls_peer_ps_ind(struct cls_wifi_hw *cls_wifi_hw,
										   struct cls_wifi_cmd *cmd,
										   struct ipc_e2a_msg *msg)
{
	struct cls_wifi_vif *vif;
	u8 vif_index = ((struct tdls_peer_ps_ind *)msg->param)->vif_index;
	bool ps_on = ((struct tdls_peer_ps_ind *)msg->param)->ps_on;

	list_for_each_entry(vif, &cls_wifi_hw->vifs, list) {
		if (vif->vif_index == vif_index) {
			vif->sta.tdls_sta->tdls.ps_on = ps_on;
			// Update PS status for the TDLS station
			cls_wifi_ps_bh_enable(cls_wifi_hw, vif->sta.tdls_sta, ps_on);
		}
	}

	return 0;
}

static inline int cls_wifi_rx_remain_on_channel_exp_ind(struct cls_wifi_hw *cls_wifi_hw,
													struct cls_wifi_cmd *cmd,
													struct ipc_e2a_msg *msg)
{
	struct cls_wifi_roc *roc = cls_wifi_hw->roc;
	struct cls_wifi_vif *vif = roc->vif;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	trace_roc_exp(vif->vif_index);

	if (!roc->internal && roc->on_chan) {
		// If RoC has been started by the user space and hasn't been cancelled,
		// inform it that off-channel period has expired
		cfg80211_remain_on_channel_expired(&vif->wdev, (u64)(uintptr_t)(roc),
										   roc->chan, GFP_ATOMIC);
	}

	cls_wifi_txq_offchan_deinit(vif);

	kfree(roc);
	cls_wifi_hw->roc = NULL;

	return 0;
}

static inline int cls_wifi_rx_p2p_vif_ps_change_ind(struct cls_wifi_hw *cls_wifi_hw,
												struct cls_wifi_cmd *cmd,
												struct ipc_e2a_msg *msg)
{
	struct mm_p2p_vif_ps_change_ind *ind = (struct mm_p2p_vif_ps_change_ind *)msg->param;
	struct cls_wifi_vif *vif = cls_wifi_get_vif(cls_wifi_hw, ind->vif_index);

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (!vif)
		return 1;

	if (ind->ps_state == MM_PS_MODE_OFF) {
		cls_wifi_txq_vif_start(vif, CLS_WIFI_TXQ_STOP_VIF_PS, cls_wifi_hw);
	}
	else {
		cls_wifi_txq_vif_stop(vif, CLS_WIFI_TXQ_STOP_VIF_PS, cls_wifi_hw);
	}

	return 0;
}

static inline int cls_wifi_rx_channel_survey_ind(struct cls_wifi_hw *cls_wifi_hw,
											 struct cls_wifi_cmd *cmd,
											 struct ipc_e2a_msg *msg)
{
	struct mm_channel_survey_ind *ind = (struct mm_channel_survey_ind *)msg->param;
	struct cls_wifi_survey_info *cls_wifi_survey;
	int idx = cls_wifi_freq_to_idx(cls_wifi_hw, ind->freq);

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (idx >  ARRAY_SIZE(cls_wifi_hw->survey))
		return 0;

	cls_wifi_survey = &cls_wifi_hw->survey[idx];

	// Store the received parameters
	cls_wifi_survey->chan_time_ms = ind->chan_time_ms;
	cls_wifi_survey->chan_time_busy_ms = ind->chan_time_busy_ms;
	cls_wifi_survey->noise_dbm = ind->noise_dbm;
	cls_wifi_survey->filled = (SURVEY_INFO_TIME |
						   SURVEY_INFO_TIME_BUSY);

	if (ind->noise_dbm != 0) {
		cls_wifi_survey->filled |= SURVEY_INFO_NOISE_DBM;
	}

	return 0;
}

static inline int cls_wifi_rx_p2p_noa_upd_ind(struct cls_wifi_hw *cls_wifi_hw,
										  struct cls_wifi_cmd *cmd,
										  struct ipc_e2a_msg *msg)
{
	return 0;
}

static inline int cls_wifi_rx_rssi_status_ind(struct cls_wifi_hw *cls_wifi_hw,
										  struct cls_wifi_cmd *cmd,
										  struct ipc_e2a_msg *msg)
{
	struct mm_rssi_status_ind *ind = (struct mm_rssi_status_ind *)msg->param;
	bool rssi_status = ind->rssi_status;
	struct cls_wifi_vif *vif = cls_wifi_get_vif(cls_wifi_hw, ind->vif_index);

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (!vif)
		return 1;

	cfg80211_cqm_rssi_notify(vif->ndev,
							 rssi_status ? NL80211_CQM_RSSI_THRESHOLD_EVENT_LOW :
							 NL80211_CQM_RSSI_THRESHOLD_EVENT_HIGH,
							 ind->rssi, GFP_ATOMIC);

	return 0;
}

static inline int cls_wifi_rx_pktloss_notify_ind(struct cls_wifi_hw *cls_wifi_hw,
											 struct cls_wifi_cmd *cmd,
											 struct ipc_e2a_msg *msg)
{
	struct mm_pktloss_ind *ind = (struct mm_pktloss_ind *)msg->param;
	struct cls_wifi_vif *vif = cls_wifi_get_vif(cls_wifi_hw, ind->vif_index);

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (!vif)
		return 1;

	cfg80211_cqm_pktloss_notify(vif->ndev, (const u8 *)ind->mac_addr.array,
								ind->num_packets, GFP_ATOMIC);

	return 0;
}

static inline int cls_wifi_rx_csa_counter_ind(struct cls_wifi_hw *cls_wifi_hw,
										  struct cls_wifi_cmd *cmd,
										  struct ipc_e2a_msg *msg)
{
	struct mm_csa_counter_ind *ind = (struct mm_csa_counter_ind *)msg->param;
	struct cls_wifi_vif *vif = cls_wifi_get_vif(cls_wifi_hw, ind->vif_index);

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (!vif)
		return 1;

	if (vif->ap.csa)
		vif->ap.csa->count = ind->csa_count;
	else
		netdev_err(vif->ndev, "CSA counter update but no active CSA");

	return 0;
}

static inline int cls_wifi_rx_csa_finish_ind(struct cls_wifi_hw *cls_wifi_hw,
										 struct cls_wifi_cmd *cmd,
										 struct ipc_e2a_msg *msg)
{
	struct mm_csa_finish_ind *ind = (struct mm_csa_finish_ind *)msg->param;
	struct cls_wifi_vif *vif = cls_wifi_get_vif(cls_wifi_hw, ind->vif_index);

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (!vif)
		return 1;

	if (CLS_WIFI_VIF_TYPE(vif) == NL80211_IFTYPE_AP ||
		CLS_WIFI_VIF_TYPE(vif) == NL80211_IFTYPE_P2P_GO) {
		if (vif->ap.csa) {
			vif->ap.csa->status = ind->status;
			vif->ap.csa->ch_idx = ind->chan_idx;
			schedule_work(&vif->ap.csa->work);
		} else
			netdev_err(vif->ndev, "CSA finish indication but no active CSA");
	} else {
		if (ind->status == 0) {
			cls_wifi_chanctx_unlink(vif);
			cls_wifi_chanctx_link(vif, ind->chan_idx, NULL);
			if (cls_wifi_hw->cur_chanctx == ind->chan_idx) {
				cls_wifi_radar_detection_enable_on_cur_channel(cls_wifi_hw, vif);
				cls_wifi_txq_vif_start(vif, CLS_WIFI_TXQ_STOP_CHAN, cls_wifi_hw);
			} else
				cls_wifi_txq_vif_stop(vif, CLS_WIFI_TXQ_STOP_CHAN, cls_wifi_hw);
		}
	}

	return 0;
}

static inline int cls_wifi_rx_csa_traffic_ind(struct cls_wifi_hw *cls_wifi_hw,
										  struct cls_wifi_cmd *cmd,
										  struct ipc_e2a_msg *msg)
{
	struct mm_csa_traffic_ind *ind = (struct mm_csa_traffic_ind *)msg->param;
	struct cls_wifi_vif *vif = cls_wifi_get_vif(cls_wifi_hw, ind->vif_index);

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (!vif)
		return 1;

	if (ind->enable)
		cls_wifi_txq_vif_start(vif, CLS_WIFI_TXQ_STOP_CSA, cls_wifi_hw);
	else
		cls_wifi_txq_vif_stop(vif, CLS_WIFI_TXQ_STOP_CSA, cls_wifi_hw);

	return 0;
}

static inline int cls_wifi_rx_repeater_csa_ind(struct cls_wifi_hw *cls_wifi_hw,
										  struct cls_wifi_cmd *cmd,
										  struct ipc_e2a_msg *msg)
{
	struct mm_repeater_csa_ind *ind = (struct mm_repeater_csa_ind *)msg->param;
	struct cls_wifi_vif *vif = cls_wifi_get_vif(cls_wifi_hw, ind->vif_index);
	struct cls_wifi_vif *ap_vif;
	struct cls_wifi_uevent_cs uevent_cs;
	int i;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	pr_info("[%s] receive REPEATER_CSA_IND: %d %d %d %d %d %d\n", __func__, ind->vif_index, ind->csa_count, ind->bw, ind->freq, ind->center1_freq, ind->center2_freq);

	if (!vif || (CLS_WIFI_VIF_TYPE(vif) != NL80211_IFTYPE_STATION)) {
		netdev_err(vif->ndev, "REPEATER_CSA_IND should not be triggered in non-sta vif");

		return 1;
	}

	uevent_cs.csa_count =ind->csa_count;
	uevent_cs.blocktx =ind->blocktx;
	uevent_cs.bw =ind->bw;
	uevent_cs.freq =ind->freq;
	uevent_cs.center1_freq =ind->center1_freq;
	uevent_cs.center2_freq =ind->center2_freq;

	pr_info("[%s] vif %s\n", __func__, vif->ndev->name);

	// trigger csa on all repeater-ap
	for (i = 0; i < CLS_ITF_MAX; i++) {
		ap_vif = cls_wifi_hw->vif_table[i];
		if (ap_vif && (CLS_WIFI_VIF_TYPE(ap_vif) == NL80211_IFTYPE_AP)) {
			pr_info("[%s] vif %s should trigger CSA\n", __func__, ap_vif->ndev->name);

			if (uevent_cs.blocktx)
				cls_wifi_txq_vif_stop(ap_vif, CLS_WIFI_TXQ_STOP_CHAN, cls_wifi_hw);

			cls_wifi_uevent_channel_switch(ap_vif, &uevent_cs);
		}
	}

	return 0;
}

static inline int cls_wifi_rx_ps_change_ind(struct cls_wifi_hw *cls_wifi_hw,
										struct cls_wifi_cmd *cmd,
										struct ipc_e2a_msg *msg)
{
	struct mm_ps_change_ind *ind = (struct mm_ps_change_ind *)msg->param;
	struct cls_wifi_sta *sta = cls_wifi_get_sta(cls_wifi_hw, ind->sta_idx, NULL,
										test_bit(CLS_WIFI_DEV_ADDING_STA, &cls_wifi_hw->flags));

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (!sta) {
		wiphy_err(cls_wifi_hw->wiphy, "Invalid sta index reported by fw %d\n",
				  ind->sta_idx);
		return 1;
	}

	if (sta->valid) {
		cls_wifi_ps_bh_enable(cls_wifi_hw, sta, ind->ps_state);
	} else {
		sta->ps.active = ind->ps_state ? true : false;
	}

	return 0;
}


static inline int cls_wifi_rx_traffic_req_ind(struct cls_wifi_hw *cls_wifi_hw,
										  struct cls_wifi_cmd *cmd,
										  struct ipc_e2a_msg *msg)
{
	struct mm_traffic_req_ind *ind = (struct mm_traffic_req_ind *)msg->param;
	struct cls_wifi_sta *sta = cls_wifi_get_sta(cls_wifi_hw, ind->sta_idx, NULL, false);

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (!sta)
		return 1;

	if (is_multicast_sta(cls_wifi_hw, sta->sta_idx)) {
		uint32_t cur_value;

		cur_value = cls_wifi_hw->ipc_env->ops->read32(cls_wifi_hw->ipc_env->plat, cls_wifi_hw->ipc_env->radio_idx,
			offsetof(struct ipc_shared_env_tag, bcmc_traffic_msg_cnt_get));
		cur_value += 1;
		cls_wifi_hw->ipc_env->ops->write32(cls_wifi_hw->ipc_env->plat,
					cls_wifi_hw->ipc_env->radio_idx,
					offsetof(struct ipc_shared_env_tag, bcmc_traffic_msg_cnt_get), cur_value);
	}

	if (sta->valid == false)
		return 0;

	cls_wifi_ps_bh_traffic_req(cls_wifi_hw, sta, ind->pkt_cnt,
						   ind->uapsd ? UAPSD_ID : LEGACY_PS_ID);

	return 0;
}

static inline int cls_wifi_rx_cca_counter_ind(struct cls_wifi_hw *cls_wifi_hw,
		struct cls_wifi_cmd *cmd,
		struct ipc_e2a_msg *msg)
{
	struct mm_cca_counter_ind *ind = (struct mm_cca_counter_ind *)msg->param;
	struct cls_wifi_vif *vif = cls_wifi_get_vif(cls_wifi_hw, ind->vif_index);

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (!vif)
		return 1;

	if (vif->ap.cca)
		vif->ap.cca->count = ind->cca_count;
	else
		netdev_err(vif->ndev, "CSA counter update but no active CSA");

	return 0;
}

static inline int cls_wifi_rx_cca_finish_ind(struct cls_wifi_hw *cls_wifi_hw,
		struct cls_wifi_cmd *cmd,
		struct ipc_e2a_msg *msg)
{
	struct mm_cca_finish_ind *ind = (struct mm_cca_finish_ind *)msg->param;
	struct cls_wifi_vif *vif = cls_wifi_get_vif(cls_wifi_hw, ind->vif_index);

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (!vif || CLS_WIFI_VIF_TYPE(vif) != NL80211_IFTYPE_AP)
		return 1;

	if (vif->ap.cca) {
		vif->ap.cca->status = ind->status;
		schedule_work(&vif->ap.cca->work);
	} else {
		netdev_err(vif->ndev, "CCA finish indication but no active CCA");
	}

	return 0;
}

/***************************************************************************
 * Messages from SCANU task
 **************************************************************************/
static inline int cls_wifi_rx_scanu_start_cfm(struct cls_wifi_hw *cls_wifi_hw,
										  struct cls_wifi_cmd *cmd,
										  struct ipc_e2a_msg *msg)
{
	struct scanu_start_cfm *cfm = (struct scanu_start_cfm *)msg->param;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	cls_wifi_ipc_buf_dealloc(cls_wifi_hw, &cls_wifi_hw->scan_ie);
	cls_wifi_scan_done(cls_wifi_hw, (cfm->status != SCANU_DONE));

	return 0;
}

#if defined(CFG_M3K_FPGA)
struct last_scanu_result_ind {
	int used;
	struct scanu_result_ind ind;
	uint32_t frame[1024];
};
static struct last_scanu_result_ind last_scan_ind;

static void cls_wifi_save_last_scan_ind(struct cls_wifi_hw *cls_wifi_hw, struct scanu_result_ind *ind)
{
	last_scan_ind.used = 1;
	memcpy(&last_scan_ind.ind, ind, sizeof(last_scan_ind.ind));
	memcpy(last_scan_ind.frame, ind->payload, ind->length);
}

static void cls_wifi_reinform_bss_frame(struct cls_wifi_hw *cls_wifi_hw)
{
	struct cfg80211_bss *bss = NULL;
	struct ieee80211_channel *chan;
	struct scanu_result_ind *ind = &last_scan_ind.ind;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);
	if (last_scan_ind.used == 0)
		return;

	chan = ieee80211_get_channel(cls_wifi_hw->wiphy, ind->center_freq);
	if (chan != NULL)
		bss = cfg80211_inform_bss_frame(cls_wifi_hw->wiphy, chan,
				(struct ieee80211_mgmt *)ind->payload,
				ind->length, ind->rssi * 100, GFP_ATOMIC);

	if (bss != NULL)
		cfg80211_put_bss(cls_wifi_hw->wiphy, bss);
}
#endif

static inline int cls_wifi_rx_scanu_result_ind(struct cls_wifi_hw *cls_wifi_hw,
										   struct cls_wifi_cmd *cmd,
										   struct ipc_e2a_msg *msg)
{
	struct cfg80211_bss *bss = NULL;
	struct ieee80211_channel *chan;
	struct scanu_result_ind *ind = (struct scanu_result_ind *)msg->param;
	int i;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (!cls_wifi_hw->scan_request)
		return 0;

	for (i = 0; i < cls_wifi_hw->scan_request->n_channels; i++) {
		chan = cls_wifi_hw->scan_request->channels[i];
		if (chan->center_freq == ind->center_freq)
			break;
	}

	if (i == cls_wifi_hw->scan_request->n_channels)
		return 0;

	chan = ieee80211_get_channel(cls_wifi_hw->wiphy, ind->center_freq);
#if defined(CFG_M3K_FPGA)
	cls_wifi_save_last_scan_ind(cls_wifi_hw, ind);
#endif
	if (chan != NULL)
		bss = cfg80211_inform_bss_frame(cls_wifi_hw->wiphy, chan,
										(struct ieee80211_mgmt *)ind->payload,
										ind->length, ind->rssi * 100, GFP_ATOMIC);

	if (bss != NULL)
		cfg80211_put_bss(cls_wifi_hw->wiphy, bss);

	return 0;
}

/***************************************************************************
 * Messages from ME task
 **************************************************************************/
static inline int cls_wifi_rx_me_tkip_mic_failure_ind(struct cls_wifi_hw *cls_wifi_hw,
												  struct cls_wifi_cmd *cmd,
												  struct ipc_e2a_msg *msg)
{
	struct me_tkip_mic_failure_ind *ind = (struct me_tkip_mic_failure_ind *)msg->param;
	struct cls_wifi_vif *vif = cls_wifi_get_vif(cls_wifi_hw, ind->vif_idx);

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (!vif)
		return 1;

	cfg80211_michael_mic_failure(vif->ndev, (u8 *)&ind->addr,
								 (ind->ga ?
								  NL80211_KEYTYPE_GROUP :
								  NL80211_KEYTYPE_PAIRWISE),
								 ind->keyid, (u8 *)&ind->tsc, GFP_ATOMIC);

	return 0;
}

static inline int cls_wifi_rx_me_tx_credits_update_ind(struct cls_wifi_hw *cls_wifi_hw,
												   struct cls_wifi_cmd *cmd,
												   struct ipc_e2a_msg *msg)
{
	struct me_tx_credits_update_ind *ind = (struct me_tx_credits_update_ind *)msg->param;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	cls_wifi_txq_credit_update(cls_wifi_hw, ind->sta_idx, ind->tid, ind->credits);

	return 0;
}

/***************************************************************************
 * Messages from SM task
 **************************************************************************/
static inline int cls_wifi_rx_sm_connect_ind(struct cls_wifi_hw *cls_wifi_hw,
										 struct cls_wifi_cmd *cmd,
										 struct ipc_e2a_msg *msg)
{
	struct sm_connect_ind *ind = (struct sm_connect_ind *)msg->param;
	struct cls_wifi_vif *vif = cls_wifi_get_vif(cls_wifi_hw, ind->vif_idx);
	struct cls_wifi_sta *sta = &cls_wifi_hw->sta_table[ind->ap_sta_idx];
	struct net_device *ndev = vif->ndev;
	const u8 *req_ie, *rsp_ie;
	const u8 *extcap_ie;
	u8 txq_status;
	struct ieee80211_channel *chan;
	struct cfg80211_chan_def chandef;
	const struct ieee_types_extcap *extcap;
	struct station_parameters params;
#ifdef CONFIG_CLS_FWT
	u32 sub_port;
	u16 node_idx, vif_idx;
#endif

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (!vif)
		return 1;

	/* Retrieve IE addresses and lengths */
	req_ie = (const u8 *)ind->assoc_ie_buf;
	rsp_ie = req_ie + ind->assoc_req_ie_len;

	// Fill-in the AP information
	if (ind->status_code == 0)
	{
		sta->valid = true;
		sta->sta_idx = ind->ap_sta_idx;
		sta->ch_idx = ind->ch_idx;
		sta->vif_idx = ind->vif_idx;
		sta->vlan_idx = sta->vif_idx;
		sta->qos = ind->qos;
		sta->acm = ind->acm;
		sta->ps.active = false;
		sta->aid = ind->aid;
		sta->band = ind->chan.band;
		sta->width = ind->chan.type;
		sta->center_freq = ind->chan.prim20_freq;
		sta->center_freq1 = ind->chan.center1_freq;
		sta->center_freq2 = ind->chan.center2_freq;
		vif->sta.ap = sta;
		vif->generation++;
		chan = ieee80211_get_channel(cls_wifi_hw->wiphy, ind->chan.prim20_freq);
		cfg80211_chandef_create(&chandef, chan, NL80211_CHAN_NO_HT);
		if (!cls_wifi_hw->radio_params->ht_on)
			chandef.width = NL80211_CHAN_WIDTH_20_NOHT;
		else
			chandef.width = chnl2bw[ind->chan.type];
		chandef.center_freq1 = ind->chan.center1_freq;
		chandef.center_freq2 = ind->chan.center2_freq;
		cls_wifi_chanctx_link(vif, ind->ch_idx, &chandef);
		memcpy(sta->mac_addr, ind->bssid.array, ETH_ALEN);
		if (ind->ch_idx == cls_wifi_hw->cur_chanctx) {
			txq_status = 0;
		} else {
			txq_status = CLS_WIFI_TXQ_STOP_CHAN;
		}
		memcpy(sta->ac_param, ind->ac_param, sizeof(sta->ac_param));
		cls_wifi_txq_sta_init(cls_wifi_hw, sta, txq_status);
		cls_wifi_dbgfs_register_sta(cls_wifi_hw, sta);
		cls_wtm_reset_sta_peak_data(sta);
		cls_wifi_txq_tdls_vif_init(vif);
		/* Look for TDLS Channel Switch Prohibited flag in the Extended Capability
		 * Information Element*/
		extcap_ie = cfg80211_find_ie(WLAN_EID_EXT_CAPABILITY, rsp_ie, ind->assoc_rsp_ie_len);
		if (extcap_ie && extcap_ie[1] >= 5) {
			extcap = (void *)(extcap_ie);
			vif->tdls_chsw_prohibited = extcap->ext_capab[4] & WLAN_EXT_CAPA5_TDLS_CH_SW_PROHIBITED;
		}

#ifdef CONFIG_CLS_WIFI_BFMER
		/* If Beamformer feature is activated, check if features can be used
		 * with the new peer device
		 */
		if (cls_wifi_hw->radio_params->bfmer) {
			const u8 *vht_capa_ie;
			const struct ieee80211_vht_cap *vht_cap;
			const u8 *he_capa_ie;
			const struct ieee80211_he_cap_elem *he_cap;

			/* Look for VHT Capability Information Element */
			vht_capa_ie = cfg80211_find_ie(WLAN_EID_VHT_CAPABILITY, rsp_ie,
										   ind->assoc_rsp_ie_len);

			/* Look for HE Capability Information Element */
			he_capa_ie = cfg80211_find_ext_ie(WLAN_EID_EXT_HE_CAPABILITY, rsp_ie,
											  ind->assoc_rsp_ie_len);

			/* Stop here if peer device neither supports VHT not HE */
			if (vht_capa_ie || he_capa_ie)
			{
				params.vht_capa = NULL;
				params.he_capa = NULL;

				if (he_capa_ie)
				{
					// Offset 3 is for Element ID, Length and Element ID Extension
					// 802.11ax-2021 section 9.4.2.248
					he_cap = (const struct ieee80211_he_cap_elem *)(he_capa_ie + 3);
					params.he_capa = he_cap;
				}
				else
				{
					// Offset 2 is for Element ID and Length
					// 802.11ac-2013 section 8.4.2.160.1
					vht_cap = (const struct ieee80211_vht_cap *)(vht_capa_ie + 2);
					params.vht_capa = vht_cap;
				}

				// Send MM_BFMER_ENABLE_REQ message if needed.
				// Only the fields vht_capa and
				// he_capa of the params parameter are read by this function.
				// The other field do not need to be updated.
				cls_wifi_send_bfmer_enable(cls_wifi_hw, sta, &params);
			}
		}
#endif //(CONFIG_CLS_WIFI_BFMER)

#ifdef CONFIG_CLS_WIFI_MON_DATA
		// If there are 1 sta and 1 monitor interface active at the same time then
		// monitor interface channel context is always the same as the STA interface.
		// This doesn't work with 2 STA interfaces but we don't want to support it.
		if (cls_wifi_hw->monitor_vif != CLS_WIFI_INVALID_VIF) {
			struct cls_wifi_vif *mon_vif = cls_wifi_hw->vif_table[cls_wifi_hw->monitor_vif];
			cls_wifi_chanctx_unlink(mon_vif);
			cls_wifi_chanctx_link(mon_vif, ind->ch_idx, NULL);
		}
#endif
	}

	if (ind->roamed) {
		struct cfg80211_roam_info info;
		memset(&info, 0, sizeof(info));

		if (vif->ch_index < CLS_CHAN_CTXT_CNT)
			info.channel = cls_wifi_hw->chanctx_table[vif->ch_index].chan_def.chan;
		info.bssid = (const u8 *)ind->bssid.array;
		info.req_ie = req_ie;
		info.req_ie_len = ind->assoc_req_ie_len;
		info.resp_ie = rsp_ie;
		info.resp_ie_len = ind->assoc_rsp_ie_len;
		cfg80211_roamed(ndev, &info, GFP_ATOMIC);
	} else {
#if defined(CFG_M3K_FPGA)
		cls_wifi_reinform_bss_frame(cls_wifi_hw);
#endif
		cfg80211_connect_result(ndev, (const u8 *)ind->bssid.array, req_ie,
								ind->assoc_req_ie_len, rsp_ie,
								ind->assoc_rsp_ie_len, ind->status_code,
								GFP_ATOMIC);
	}

	if (ind->status_code == 0) {
#ifdef CONFIG_CLS_FWT
		/* sub_port */
		if (cls_fwt_g_enable && sta->valid) {
			// TODO: cls_wifi_hw->radio_idx ??
			node_idx = CLS_IEEE80211_NODE_TO_IDXS(cls_wifi_hw->radio_idx, sta->sta_idx);
			node_idx = CLS_IEEE80211_NODE_IDX_MAP(node_idx);

			vif_idx = CLS_IEEE80211_VIF_IDX_MAP(vif->vif_index);

			sub_port = CLS_IEEE80211_NODE_TO_SUBPORT(node_idx, vif_idx,
				CLS_FWT_ENTRY_TYPE_AP_IN_STA_MODE, 0);

			br_fdb_update_const_hook(ndev, sta->mac_addr, br_port_get_pvid(ndev), sub_port);
		}
#endif
		netif_tx_start_all_queues(ndev);
		netif_carrier_on(ndev);
	}
	return 0;
}

static inline int cls_wifi_rx_sm_disconnect_ind(struct cls_wifi_hw *cls_wifi_hw,
											struct cls_wifi_cmd *cmd,
											struct ipc_e2a_msg *msg)
{
	struct sm_disconnect_ind *ind = (struct sm_disconnect_ind *)msg->param;
	struct cls_wifi_vif *vif;
#ifdef CONFIG_CLS_FWT
	u32 sub_port;
	u16 node_idx, vif_idx;
#endif

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if ((ind->vif_idx >= hw_vdev_max(cls_wifi_hw)) ||
		! cls_wifi_hw->vif_table[ind->vif_idx])
		return 1;

	vif = cls_wifi_hw->vif_table[ind->vif_idx];

	/* if vif is not up, cls_wifi_close has already been called */
	if (vif->up) {
		struct net_device *ndev = vif->ndev;
		if (!ind->reassoc) {
			cfg80211_disconnected(ndev, ind->reason_code, NULL, 0,
								  (ind->reason_code <= 1), GFP_ATOMIC);

#ifdef CONFIG_CLS_FWT
			if (cls_fwt_g_enable) {
				// TODO: cls_wifi_hw->radio_idx ??
				node_idx = CLS_IEEE80211_NODE_TO_IDXS(cls_wifi_hw->radio_idx, vif->sta.ap->sta_idx);
				node_idx = CLS_IEEE80211_NODE_IDX_MAP(node_idx);

				vif_idx = CLS_IEEE80211_VIF_IDX_MAP(vif->vif_index);

				sub_port = CLS_IEEE80211_NODE_TO_SUBPORT(
					node_idx, vif_idx, CLS_FWT_ENTRY_TYPE_STA_IN_AP_MODE, 0);

				// vlan.0
				br_fdb_delete_by_subport_hook(vif->ndev, sub_port);
			}
#endif

			if (vif->sta.ft_assoc_ies) {
				kfree(vif->sta.ft_assoc_ies);
				vif->sta.ft_assoc_ies = NULL;
				vif->sta.ft_assoc_ies_len = 0;
			}
		}
		netif_tx_stop_all_queues(ndev);
		netif_carrier_off(ndev);
	}

#ifdef CONFIG_CLS_WIFI_BFMER
	/* Disable Beamformer if supported */
	cls_wifi_bfmer_report_del(cls_wifi_hw, vif->sta.ap);
#endif //(CONFIG_CLS_WIFI_BFMER)

	cls_wifi_txq_sta_deinit(cls_wifi_hw, vif->sta.ap);
	cls_wifi_txq_tdls_vif_deinit(vif);
	cls_wifi_dbgfs_unregister_sta(cls_wifi_hw, vif->sta.ap);
	vif->sta.ap->valid = false;
	vif->sta.ap = NULL;
	vif->generation++;
	cls_wifi_external_auth_disable(vif);
	cls_wifi_chanctx_unlink(vif);

	return 0;
}

static inline int cls_wifi_rx_sm_external_auth_required_ind(struct cls_wifi_hw *cls_wifi_hw,
														struct cls_wifi_cmd *cmd,
														struct ipc_e2a_msg *msg)
{
	struct sm_external_auth_required_ind *ind =
		(struct sm_external_auth_required_ind *)msg->param;
	struct cls_wifi_vif *vif = cls_wifi_get_vif(cls_wifi_hw, ind->vif_idx);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
	struct cfg80211_external_auth_params params;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (!vif)
		return 1;

	params.action = NL80211_EXTERNAL_AUTH_START;
	memcpy(params.bssid, ind->bssid.array, ETH_ALEN);
	params.ssid.ssid_len = ind->ssid.length;
	memcpy(params.ssid.ssid, ind->ssid.array,
		   min_t(size_t, ind->ssid.length, sizeof(params.ssid.ssid)));
	params.key_mgmt_suite = ind->akm;

	if ((CLS_WIFI_VIF_TYPE(vif) != NL80211_IFTYPE_STATION) ||
		cfg80211_external_auth_request(vif->ndev, &params, GFP_ATOMIC)) {
		wiphy_err(cls_wifi_hw->wiphy, "Failed to start external auth on vif %d",
				  ind->vif_idx);
		cls_wifi_send_sm_external_auth_required_rsp(cls_wifi_hw, vif,
												WLAN_STATUS_UNSPECIFIED_FAILURE);
		return 0;
	}

	cls_wifi_external_auth_enable(vif);
#else
	cls_wifi_send_sm_external_auth_required_rsp(cls_wifi_hw, vif,
											WLAN_STATUS_UNSPECIFIED_FAILURE);
#endif
	return 0;
}

static inline int cls_wifi_rx_sm_ft_auth_ind(struct cls_wifi_hw *cls_wifi_hw,
										 struct cls_wifi_cmd *cmd,
										 struct ipc_e2a_msg *msg)
{
	struct sm_ft_auth_ind *ind = (struct sm_ft_auth_ind *)msg->param;
	struct cls_wifi_vif *vif = cls_wifi_get_vif(cls_wifi_hw, ind->vif_idx);
	struct sk_buff *skb;
	size_t data_len = (offsetof(struct ieee80211_mgmt, u.auth.variable) +
					   ind->ft_ie_len);

	if (!vif)
		return 1;

	skb = dev_alloc_skb(data_len);
	if (skb) {
		struct ieee80211_mgmt *mgmt = skb_put(skb, data_len);
		mgmt->frame_control = cpu_to_le16(IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_AUTH);
		memcpy(mgmt->u.auth.variable, ind->ft_ie_buf, ind->ft_ie_len);
		cls_wifi_rx_defer_skb(cls_wifi_hw, vif, skb);
		dev_kfree_skb(skb);
	} else {
		netdev_warn(vif->ndev, "Allocation failed for FT auth ind\n");
	}

	return 0;
}

/***************************************************************************
 * Messages from TWT task
 **************************************************************************/
static inline int cls_wifi_rx_twt_setup_ind(struct cls_wifi_hw *cls_wifi_hw,
										struct cls_wifi_cmd *cmd,
										struct ipc_e2a_msg *msg)
{
	struct twt_setup_ind *ind = (struct twt_setup_ind *)msg->param;
	struct cls_wifi_sta *sta = cls_wifi_get_sta(cls_wifi_hw, ind->sta_idx, NULL, false);

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (!sta)
		return 1;

	memcpy(&sta->twt_ind, ind, sizeof(struct twt_setup_ind));
	return 0;
}

static inline int cls_wifi_rx_twt_sp_ind(struct cls_wifi_hw *cls_wifi_hw,
									 struct cls_wifi_cmd *cmd,
									 struct ipc_e2a_msg *msg)
{
	struct twt_sp_ind *ind = (struct twt_sp_ind *)msg->param;
	struct cls_wifi_sta *sta = cls_wifi_get_sta(cls_wifi_hw, ind->sta_idx, NULL, false);

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (!sta)
		return 1;

	spin_lock_bh(&cls_wifi_hw->tx_lock);

	if (ind->active)
		cls_wifi_txq_sta_start(sta, CLS_WIFI_TXQ_STOP_TWT, cls_wifi_hw);
	else
		cls_wifi_txq_sta_stop(sta, CLS_WIFI_TXQ_STOP_TWT, cls_wifi_hw);

	spin_unlock_bh(&cls_wifi_hw->tx_lock);
	return 0;
}

/***************************************************************************
 * Messages from MESH task
 **************************************************************************/
static inline int cls_wifi_rx_mesh_path_create_cfm(struct cls_wifi_hw *cls_wifi_hw,
											   struct cls_wifi_cmd *cmd,
											   struct ipc_e2a_msg *msg)
{
	struct mesh_path_create_cfm *cfm = (struct mesh_path_create_cfm *)msg->param;
	struct cls_wifi_vif *vif = cls_wifi_get_vif(cls_wifi_hw, cfm->vif_idx);

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (!vif && (CLS_WIFI_VIF_TYPE(vif) != NL80211_IFTYPE_MESH_POINT))
		return 1;

	vif->ap.flags &= ~CLS_WIFI_AP_CREATE_MESH_PATH;
	return 0;
}

static inline int cls_wifi_rx_mesh_peer_update_ind(struct cls_wifi_hw *cls_wifi_hw,
											   struct cls_wifi_cmd *cmd,
											   struct ipc_e2a_msg *msg)
{
	struct mesh_peer_update_ind *ind = (struct mesh_peer_update_ind *)msg->param;
	struct cls_wifi_vif *vif = cls_wifi_get_vif(cls_wifi_hw, ind->vif_idx);
	struct cls_wifi_sta *sta;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (!vif || (CLS_WIFI_VIF_TYPE(vif) != NL80211_IFTYPE_MESH_POINT))
		return 1;

	if (vif->ap.flags & CLS_WIFI_AP_USER_MESH_PM)
	{
		sta = cls_wifi_get_sta(cls_wifi_hw, ind->sta_idx, vif, false);
		if (!ind->estab && sta) {
			/* There is no way to inform upper layer for lost of peer, still
			   clean everything in the driver */
			sta->ps.active = false;
			sta->valid = false;

			/* Remove the station from the list of VIF's station */
			list_del_init(&sta->list);

			cls_wifi_txq_sta_deinit(cls_wifi_hw, sta);
			cls_wifi_dbgfs_unregister_sta(cls_wifi_hw, sta);
		} else {
			netdev_err(vif->ndev, "Unexpected Mesh peer update: estab=%d sta_idx=%d\n",
					   ind->estab, ind->sta_idx);
		}
	} else {
		/* Check if peer link has been established or lost */
		sta = cls_wifi_get_sta(cls_wifi_hw, ind->sta_idx, vif, true);
		if (!sta)
			return 1;

		if (ind->estab) {
			if (!sta->valid) {
				u8 txq_status;

				sta->valid = true;
				sta->sta_idx = ind->sta_idx;
				sta->ch_idx = vif->ch_index;
				sta->vif_idx = ind->vif_idx;
				sta->vlan_idx = sta->vif_idx;
				sta->ps.active = false;
				sta->qos = true;
				sta->aid = ind->sta_idx + 1;
				//sta->acm = ind->acm;
				memcpy(sta->mac_addr, ind->peer_addr.array, ETH_ALEN);

				cls_wifi_chanctx_link(vif, sta->ch_idx, NULL);

				/* Add the station in the list of VIF's stations */
				INIT_LIST_HEAD(&sta->list);
				list_add_tail(&sta->list, &vif->ap.sta_list);

				/* Initialize the TX queues */
				if (sta->ch_idx == cls_wifi_hw->cur_chanctx) {
					txq_status = 0;
				} else {
					txq_status = CLS_WIFI_TXQ_STOP_CHAN;
				}

				cls_wifi_txq_sta_init(cls_wifi_hw, sta, txq_status);
				cls_wifi_dbgfs_register_sta(cls_wifi_hw, sta);
				cls_wtm_reset_sta_peak_data(sta);

#ifdef CONFIG_CLS_WIFI_BFMER
				// TODO: update indication to contains vht capabilties
				if (cls_wifi_hw->radio_params->bfmer)
					cls_wifi_send_bfmer_enable(cls_wifi_hw, sta, NULL);
#endif /* CONFIG_CLS_WIFI_BFMER */

			} else {
				netdev_err(vif->ndev, "Unexpected Mesh peer update: estab=%d sta_idx=%d\n",
						   ind->estab, ind->sta_idx);
			}
		} else {
			if (sta->valid) {
				sta->ps.active = false;
				sta->valid = false;

				/* Remove the station from the list of VIF's station */
				list_del_init(&sta->list);

				cls_wifi_txq_sta_deinit(cls_wifi_hw, sta);
				cls_wifi_dbgfs_unregister_sta(cls_wifi_hw, sta);
			} else {
				netdev_err(vif->ndev, "Unexpected Mesh peer update: estab=%d sta_idx=%d\n",
						   ind->estab, ind->sta_idx);
			}
		}
	}

	return 0;
}

static inline int cls_wifi_rx_mesh_path_update_ind(struct cls_wifi_hw *cls_wifi_hw,
											   struct cls_wifi_cmd *cmd,
											   struct ipc_e2a_msg *msg)
{
	struct mesh_path_update_ind *ind = (struct mesh_path_update_ind *)msg->param;
	struct cls_wifi_vif *vif = cls_wifi_get_vif(cls_wifi_hw, ind->vif_idx);
	struct cls_wifi_mesh_path *mesh_path;
	bool found = false;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (!vif || (CLS_WIFI_VIF_TYPE(vif) != NL80211_IFTYPE_MESH_POINT))
		return 1;

	/* Look for path with provided target address */
	list_for_each_entry(mesh_path, &vif->ap.mpath_list, list) {
		if (mesh_path->path_idx == ind->path_idx) {
			found = true;
			break;
		}
	}

	/* Check if element has been deleted */
	if (ind->delete) {
		if (found) {
			trace_mesh_delete_path(mesh_path);
			/* Remove element from list */
			list_del_init(&mesh_path->list);
			/* Free the element */
			kfree(mesh_path);
		}
	}
	else {
		if (found) {
			// Update the Next Hop STA
			mesh_path->nhop_sta = &cls_wifi_hw->sta_table[ind->nhop_sta_idx];
			trace_mesh_update_path(mesh_path);
		} else {
			// Allocate a Mesh Path structure
			mesh_path = kmalloc(sizeof(struct cls_wifi_mesh_path), GFP_ATOMIC);

			if (mesh_path) {
				INIT_LIST_HEAD(&mesh_path->list);

				mesh_path->path_idx = ind->path_idx;
				mesh_path->nhop_sta = &cls_wifi_hw->sta_table[ind->nhop_sta_idx];
				memcpy(&mesh_path->tgt_mac_addr, &ind->tgt_mac_addr, MAC_ADDR_LEN);

				// Insert the path in the list of path
				list_add_tail(&mesh_path->list, &vif->ap.mpath_list);
				trace_mesh_create_path(mesh_path);
			}
		}
	}

	return 0;
}

static inline int cls_wifi_rx_mesh_proxy_update_ind(struct cls_wifi_hw *cls_wifi_hw,
											   struct cls_wifi_cmd *cmd,
											   struct ipc_e2a_msg *msg)
{
	struct mesh_proxy_update_ind *ind = (struct mesh_proxy_update_ind *)msg->param;
	struct cls_wifi_vif *vif = cls_wifi_get_vif(cls_wifi_hw, ind->vif_idx);
	struct cls_wifi_mesh_proxy *mesh_proxy;
	bool found = false;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (!vif || (CLS_WIFI_VIF_TYPE(vif) != NL80211_IFTYPE_MESH_POINT))
		return 1;

	/* Look for path with provided external STA address */
	list_for_each_entry(mesh_proxy, &vif->ap.proxy_list, list) {
		if (!memcmp(&ind->ext_sta_addr, &mesh_proxy->ext_sta_addr, ETH_ALEN)) {
			found = true;
			break;
		}
	}

	if (ind->delete && found) {
		/* Delete mesh path */
		list_del_init(&mesh_proxy->list);
		kfree(mesh_proxy);
	} else if (!ind->delete && !found) {
		/* Allocate a Mesh Path structure */
		mesh_proxy = (struct cls_wifi_mesh_proxy *)kmalloc(sizeof(*mesh_proxy),
													   GFP_ATOMIC);

		if (mesh_proxy) {
			INIT_LIST_HEAD(&mesh_proxy->list);

			memcpy(&mesh_proxy->ext_sta_addr, &ind->ext_sta_addr, MAC_ADDR_LEN);
			mesh_proxy->local = ind->local;

			if (!ind->local) {
				memcpy(&mesh_proxy->proxy_addr, &ind->proxy_mac_addr, MAC_ADDR_LEN);
			}

			/* Insert the path in the list of path */
			list_add_tail(&mesh_proxy->list, &vif->ap.proxy_list);
		}
	}

	return 0;
}

/***************************************************************************
 * Messages from APM task
 **************************************************************************/
static inline int cls_wifi_rx_apm_probe_client_ind(struct cls_wifi_hw *cls_wifi_hw,
											   struct cls_wifi_cmd *cmd,
											   struct ipc_e2a_msg *msg)
{
	struct apm_probe_client_ind *ind = (struct apm_probe_client_ind *)msg->param;
	struct cls_wifi_vif *vif = cls_wifi_get_vif(cls_wifi_hw, ind->vif_idx);
	struct cls_wifi_sta *sta = cls_wifi_get_sta(cls_wifi_hw, ind->sta_idx, vif, false);

	if (!vif || !sta)
		return 1;

	if (ind->client_present)
		sta->stats.last_act = jiffies;
	cfg80211_probe_status(vif->ndev, sta->mac_addr, (u64)ind->probe_id,
						  ind->client_present, 0, false, GFP_ATOMIC);

	return 0;
}

/***************************************************************************
 * Messages from DEBUG task
 **************************************************************************/
static inline int cls_wifi_rx_dbg_error_ind(struct cls_wifi_hw *cls_wifi_hw,
										struct cls_wifi_cmd *cmd,
										struct ipc_e2a_msg *msg)
{
	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	cls_wifi_error_ind(cls_wifi_hw);

	return 0;
}
#endif

/***************************************************************************
 * Messages from IRF task
 **************************************************************************/
#ifdef __KERNEL__
static inline int cls_wifi_irf_sample_start_ind(struct cls_wifi_hw *cls_wifi_hw,
		struct cls_wifi_cmd *cmd,
		struct ipc_e2a_msg *msg)
{
	struct irf_smp_start_ind *ind = (struct irf_smp_start_ind *)msg->param;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (ind->status != CO_OK) {
		printk("%s failure: %d\n", __func__, ind->status);
		return CO_FAIL;
	}

	memcpy(&cls_wifi_hw->irf_file.irf_smp_ind, ind, sizeof(*ind));
	schedule_work(&cls_wifi_hw->irf_file.smp_save_dat_work);

	return CO_OK;
}
#else
static inline int cls_wifi_irf_sample_start_ind(struct cls_wifi_hw *cls_wifi_hw,
		struct cls_wifi_cmd *cmd,
		struct ipc_e2a_msg *msg)
{
	uint8_t i;
	uint32_t offset;
	struct irf_smp_start_ind *ind = (struct irf_smp_start_ind *)msg->param;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (ind->status != CO_OK) {
		printk("IRF Sample indicates failure: %d\n", ind->status);
		return CO_FAIL;
	}

	for(i = 0; i < IRF_MAX_NODE; i++) {
		if (ind->sel_bitmap & CO_BIT(i)) {
			offset = ind->smp_addr_desc[i].irf_smp_buf_addr - IRF_DATA_BASE;
			/* Only for debug usage */
			yc_printf("ind node: 0x%d, addr: 0x%08x, offset: 0x%08x, len:0x%x\n",
					ind->smp_addr_desc[i].node,
					ind->smp_addr_desc[i].irf_smp_buf_addr,
					offset,ind->smp_addr_desc[i].len);
		}
	}

	return CO_OK;
}
#endif

#ifdef __KERNEL__
static inline int cls_wifi_irf_sample_lms_ind(struct cls_wifi_hw *cls_wifi_hw,
										struct cls_wifi_cmd *cmd,
										struct ipc_e2a_msg *msg)
{
	struct irf_smp_lms_ind *ind = (struct irf_smp_lms_ind *)msg->param;
	struct cls_wifi_plat *cls_wifi_plat = cls_wifi_hw->plat;
	struct irf_lms_desc lms_desc;
	u32 offset;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (ind->status != CO_OK) {
		printk("IRF Sample lms indicates failure: %d\n", ind->status);
		return CO_FAIL;
	}

	offset = ind->irf_smp_buf_addr - cls_wifi_plat->if_ops->get_phy_address(cls_wifi_hw->plat,
			cls_wifi_hw->radio_idx, CLS_WIFI_ADDR_IRF_PHY, 0);

	cls_wifi_hw->ipc_env->ops->irf_readn(cls_wifi_hw->ipc_env->plat,
			cls_wifi_hw->ipc_env->radio_idx, offset, &lms_desc, sizeof(lms_desc));

	/* Only for debug usage */
	printk("ind addr: 0x%08x, offset: 0x%08x\n", ind->irf_smp_buf_addr, offset);

	cls_wifi_save_irf_configfile(cls_wifi_hw, offset + offsetof(struct irf_lms_desc, lms_data),
			lms_desc.len, lms_desc.lms_node, IRF_RESV_MEM);

	if (lms_desc.update_node)
		cls_wifi_save_irf_configfile(cls_wifi_hw, offset + offsetof(struct irf_lms_desc,
			update_data), lms_desc.len, lms_desc.update_node, IRF_RESV_MEM);

	return 0;
}
#endif
static inline int cls_wifi_irf_start_dif_eq_ind(struct cls_wifi_hw *cls_wifi_hw,
										struct cls_wifi_cmd *cmd,
										struct ipc_e2a_msg *msg)
{
	struct irf_dif_eq_ind *ind = (struct irf_dif_eq_ind *)msg->param;
#ifdef __KERNEL__
	struct irf_set_cali_evt_ind dif_eq_ind;
#endif

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

#ifdef __KERNEL__

	dif_eq_ind.status = ind->status;
	dif_eq_ind.radio_id = ind->radio_id;
	dif_eq_ind.evt_cmd = DIF_EVT_START_EQ_CALI;
	dif_eq_ind.cause = DIF_CALI_STOP_BY_DONE;

	if(CO_BUSY == ind->status){
		printk("%s DIF fw sm busy.\n",RADIO2STR(ind->radio_id));
		dif_eq_ind.dif_sm_state = cls_wifi_hw->dif_sm.dif_fw_state;
		cls_wifi_dif_sm_update(cls_wifi_hw,&dif_eq_ind);
		dif_cali_status = IRF_CALI_STATUS_FAIL;
		return CO_FAIL;
	} else if (ind->status != CO_OK) {
		printk("%s DIF EQ test indicates failure: %d\n",RADIO2STR(ind->radio_id),ind->status);
		dif_eq_ind.dif_sm_state = DIF_SM_IDLE;
		cls_wifi_dif_sm_update(cls_wifi_hw,&dif_eq_ind);
		dif_cali_status = IRF_CALI_STATUS_FAIL;
		return CO_FAIL;
	}
	dif_eq_ind.dif_sm_state = DIF_SM_IDLE;

	cls_wifi_dif_sm_update(cls_wifi_hw,&dif_eq_ind);
	dif_cali_status = IRF_CALI_STATUS_DONE;

	return CO_OK;
#else

	if (ind->status != CO_OK) {
		printk("DIF EQ test indicates failure: %d\n", ind->status);
		return CO_FAIL;
	}

	yc_printf("ind: radio %d dif equipment Done\n", ind->radio_id);
	return CO_OK;
#endif
}

static inline int cls_wifi_irf_dif_cali_save_ind(struct cls_wifi_hw *cls_wifi_hw,
										struct cls_wifi_cmd *cmd,
										struct ipc_e2a_msg *msg)
{
	struct irf_dif_eq_ind *ind = (struct irf_dif_eq_ind *)msg->param;
#ifndef __KERNEL__
	uint32_t addr;
#endif

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

#ifdef __KERNEL__


if (ind->status != CO_OK) {
		printk("%s DIF EQ table save indicates failure: %d\n",RADIO2STR(ind->radio_id),ind->status);
		return CO_FAIL;
	}

	cls_wifi_hw->irf_file.dif_eq_size = ind->data_size;
	schedule_work(&cls_wifi_hw->irf_file.dif_cali_save_bin_work);

	return CO_OK;
#else

	if (ind->status != CO_OK) {
		printk("DIF EQ test indicates failure: %d\n", ind->status);
		return CO_FAIL;
	}

	addr = (0 == ind->radio_id)?DIF_EQ_2G_ADDR:DIF_EQ_5G_ADDR;
	addr += IRF_TABLE_BASE;
	yc_printf("ind: radio %d dif equipment data at 0x%x, size:0x%x\n", ind->radio_id, addr, ind->data_size);
	return CO_OK;
#endif
}

#ifdef __KERNEL__

static inline int cls_wifi_irf_calib_evt_ind(struct cls_wifi_hw *cls_wifi_hw,
										struct cls_wifi_cmd *cmd,
										struct ipc_e2a_msg *msg)
{
	struct irf_set_cali_evt_ind *ind = (struct irf_set_cali_evt_ind *)msg->param;

	//CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	cls_wifi_dif_sm_update(cls_wifi_hw,ind);

	if (ind->status != CO_OK) {
		printk("%s DIF evt_cmd(%d) failure: %d\n",RADIO2STR(ind->radio_id),ind->evt_cmd,ind->status);
		return CO_FAIL;
	}

	if (ind->cause == DIF_CALI_STOP_BY_DONE) {
		if (cls_wifi_hw->dif_sm.dif_online_dpd_task_type == DIF_FBDELAY_TASK)
			cls_wifi_hw->dif_sm.fbdelay_success_flag = !ind->fbdelay_drop_flag;

		if (cls_wifi_hw->dif_sm.dif_online_dpd_task_type == DIF_PD_TASK)
			cls_wifi_hw->dif_sm.pd_task_success_flag = ind->pd_task_success_flag;
	}

	return CO_OK;
}

#ifdef CONFIG_CLS_VBSS
static int cls_wifi_rx_vbss_seq_num_cfm(struct cls_wifi_hw *cls_wifi_hw,
		struct cls_wifi_cmd *cmd, struct ipc_e2a_msg *msg)
{
	struct vbss_sta_seq_num *cfm = (struct vbss_sta_seq_num *)msg->param;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);
	pr_info("%s cfm %p sta %u tx pn %u rx pn %u",
			__func__, cfm, cfm->sta_idx, cfm->tx_pn, cfm->rx_pn);

	return 0;
}
#endif

static int cls_wifi_rx_atf_cfm(struct cls_wifi_hw *cls_wifi_hw,
		struct cls_wifi_cmd *cmd, struct ipc_e2a_msg *msg)
{
//	struct mm_atf_params_cfm *cfm = (struct mm_atf_params_cfm *)msg->param;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	return 0;
}

static int cls_wifi_dpd_wmac_tx_cfm(struct cls_wifi_hw *wifi_hw,
		struct cls_wifi_cmd *cmd, struct ipc_e2a_msg *msg)
{
	struct mm_dpd_wmac_tx_ind *ind;

	ind = (struct mm_dpd_wmac_tx_ind *)msg->param;
	cls_wifi_dpd_wmac_tx_post_process(wifi_hw, ind);

	if (!test_bit(CLS_WIFI_DEV_STARTED, &wifi_hw->flags))
		schedule_work(&wifi_hw->dpd_restore_work);
	else {
		if (ind->status == 0) {
			if (wifi_hw->dif_sm.dif_online_dpd_task_type == DIF_FBDELAY_TASK) {
				wifi_hw->dif_sm.fbdelay_task_times++;
				if (wifi_hw->dif_sm.fbdelay_success_flag) {
					//fbdelay success, start dpd task
					cls_wifi_dif_schedule(wifi_hw->plat, wifi_hw->radio_idx,
					DIF_SCH_DPD_CALI, DIF_EVT_DPD_CALI);
				} else {
					//fbdelay fail
					pr_info("%s: radio[%u] fbdelay fail, times: %u\n",
					__func__, wifi_hw->radio_idx, wifi_hw->dif_sm.fbdelay_task_times);
					if (wifi_hw->dif_sm.fbdelay_task_times < 5) {
						pr_info("%s: fbdelay fail, schedule again\n", __func__);
						wifi_hw->plat->dif_sch->sm_cnt = 0;
						cls_wifi_dif_sm_resume(wifi_hw);
						cls_wifi_dif_schedule(wifi_hw->plat, wifi_hw->radio_idx,
							DIF_SCH_DPD_CALI, DIF_EVT_DPD_FBDELAY_CALI);
					} else {
						//abort dpd restore tx power
						schedule_work(&wifi_hw->dpd_restore_work);
						pr_info("%s: fbdelay exceed retry times abort dpd\n", __func__);
					}
				}
			} else if (wifi_hw->dif_sm.dif_online_dpd_task_type == DIF_PD_TASK) {
				if (wifi_hw->dif_sm.pd_task_success_flag == false)
					pr_info("%s radio[%u] pd task run fail\n", __func__, wifi_hw->radio_idx);
				schedule_work(&wifi_hw->dpd_restore_work);
			}
		} else {
			//abort dpd
			pr_info("%s status %u abort dpd\n", __func__, ind->status);
			schedule_work(&wifi_hw->dpd_restore_work);
		}
	}

	return 0;
}

static int cls_wifi_vip_node_ind(struct cls_wifi_hw *wifi_hw,
		struct cls_wifi_cmd *cmd, struct ipc_e2a_msg *msg)
{
	struct mm_vip_node_ind *ind = (struct mm_vip_node_ind *)msg->param;

	if (wifi_hw->vip_node.enable != ind->enable) {
		wifi_hw->vip_node.enable = ind->enable;
		if (ind->enable)
			cls_wifi_start_pps_cal_timer(wifi_hw, 1);
		else
			cls_wifi_start_pps_cal_timer(wifi_hw, 0);
	}
	wifi_hw->vip_node.node_idx = ind->vip_node_idx;
	wifi_hw->vip_node.traffic_ratio = ind->traffic_ratio;
	wifi_hw->vip_node.pps_thresh = ind->pps_thresh;

	pr_err("[%s][%d]: enable:%d, node_idx:%d, traffic_ratio:%d, pps_thresh:%d\n",
			__func__, __LINE__, ind->enable, ind->vip_node_idx, ind->traffic_ratio, ind->pps_thresh);

	return 0;
}

#endif
static inline int cls_wifi_irf_fb_dcoc_done_ind(struct cls_wifi_hw *cls_wifi_hw,
										struct cls_wifi_cmd *cmd,
										struct ipc_e2a_msg *msg)
{
	struct irf_fb_dcoc_done_ind *ind = (struct irf_fb_dcoc_done_ind *)msg->param;
#ifdef __KERNEL__
	u32 offset;
	char *name;
#endif

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (ind->status != CO_OK) {
		printk("IRF FB DCOC done failed: %d %d %d\n", ind->status, ind->radio_id,
				ind->high_temp);
		dcoc_status[0] = IRF_CALI_STATUS_IDLE;
		return CO_FAIL;
	}
#ifdef __KERNEL__
	offset = ind->fb_dcoc_addr - cls_wifi_hw->plat->if_ops->get_phy_address(cls_wifi_hw->plat,
			cls_wifi_hw->radio_idx, CLS_WIFI_ADDR_IRF_TBL_PHY, 0);
	dcoc_status[0] = IRF_CALI_STATUS_DONE;

	return cls_wifi_save_irf_binfile_mem(cls_wifi_hw, name, offset, ind->fb_dcoc_size);
#else
	yc_printf("radio %d channel %d FB DCOC done, data addr = 0x%x, size = 0x%x\n",ind->fb_dcoc_addr,ind->fb_dcoc_size);
	return 0;
#endif
}

static inline int cls_wifi_irf_tx_cali_save_ind(struct cls_wifi_hw *cls_wifi_hw,
										struct cls_wifi_cmd *cmd,
										struct ipc_e2a_msg *msg)
{
	struct irf_tx_cali_save_ind *ind = (struct irf_tx_cali_save_ind *)msg->param;
	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (ind->status != CO_OK) {
		printk("tx power cali indicates failure: %d\n", ind->status);
		return CO_FAIL;
	}

#ifdef __KERNEL__
	cls_wifi_hw->irf_file.tx_data_size = ind->tx_data_size;
	cls_wifi_hw->irf_file.fb_data_size = ind->fb_data_size;
	schedule_work(&cls_wifi_hw->irf_file.tx_cali_save_bin_work);
	return CO_OK;
#else
  yc_printf("radio %d TX-Power calibration done, data addr = 0x%x, size = 0x%x\n",ind->tx_data_addr,ind->tx_data_size);
  yc_printf("radio %d FB-Power calibration done, data addr = 0x%x, size = 0x%x\n",ind->fb_data_addr,ind->fb_data_size);
  return CO_OK;
#endif
}

static inline int cls_wifi_irf_rx_cali_save_ind(struct cls_wifi_hw *cls_wifi_hw,
										struct cls_wifi_cmd *cmd,
										struct ipc_e2a_msg *msg)
{
	struct irf_rx_cali_save_ind *ind = (struct irf_rx_cali_save_ind *)msg->param;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (ind->status != CO_OK) {
		printk("rssi cali indicates failure: %d\n", ind->status);
		rx_cali_status = IRF_CALI_STATUS_FAIL;
		return CO_FAIL;
	}

#ifdef __KERNEL__
	switch (ind->rx_cali_type) {
	case IRF_RX_CALI_GAIN_LEVEL:
		cls_wifi_hw->irf_file.rx_data_gain_lvl_size = ind->rx_data_size;
		schedule_work(&cls_wifi_hw->irf_file.rx_cali_gain_lvl_save_bin_work);
		break;
	case IRF_RX_CALI_GAIN_FREQ_COMP:
		cls_wifi_hw->irf_file.rx_data_freq_comp_size = ind->rx_data_size;
		schedule_work(&cls_wifi_hw->irf_file.rx_cali_fcomp_save_bin_work);
		break;
	default:
		printk("invalid rx_cali_type %u\n", ind->rx_cali_type);
		return CO_FAIL;
	}

	return CO_OK;
#else
  yc_printf("radio %d RSSI calibration done, data addr = 0x%x, size = 0x%x\n",ind->rx_data_addr,ind->rx_data_size);
  return CO_OK;
#endif
}
#ifdef __KERNEL__
static inline int cls_wifi_irf_rx_dcoc_done_ind(struct cls_wifi_hw *cls_wifi_hw,
										struct cls_wifi_cmd *cmd,
										struct ipc_e2a_msg *msg)
{
	struct irf_fb_dcoc_done_ind *ind = (struct irf_fb_dcoc_done_ind *)msg->param;
	u32 offset;
#ifdef CFG_PCIE_SHM
	u32 pool_offset;
	pcie_shm_pool_st *shm_obj = cls_wifi_hw->plat->pcie_shm_pools[0];
#endif
	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (ind->status != CO_OK) {
		printk("IRF RX DCOC done failed: %d %d %d\n", ind->status, ind->radio_id,
				ind->high_temp);
		dcoc_status[1] = IRF_CALI_STATUS_IDLE;
		return CO_FAIL;
	}

#ifdef CFG_PCIE_SHM
	if (pcie_shm_get_tbl_offset(shm_obj, IRF_TBL_DFT_IDX, &pool_offset)) {
		pr_err("pcie_shm_get_tbl_offset failed!\n");
		return -1;
	}
	offset = ind->fb_dcoc_addr - IRF_TBL_FW_BASE_ADDR - pool_offset;
#else
	offset = ind->fb_dcoc_addr - cls_wifi_hw->plat->if_ops->get_phy_address(cls_wifi_hw->plat,
			cls_wifi_hw->radio_idx, CLS_WIFI_ADDR_IRF_TBL_PHY, 0);
#endif

	cls_wifi_hw->irf_file.rx_dcoc_offset = offset;
	cls_wifi_hw->irf_file.rx_dcoc_size = ind->fb_dcoc_size;
	cls_wifi_hw->irf_file.rx_dcoc_high_temp = ind->high_temp;
	schedule_work(&cls_wifi_hw->irf_file.rx_dcoc_save_bin_work);
	return CO_OK;
}

static inline int cls_wifi_irf_fb_err_cali_done_ind(struct cls_wifi_hw *cls_wifi_hw,
										struct cls_wifi_cmd *cmd,
										struct ipc_e2a_msg *msg)
{
	struct irf_gain_err_cali_ind *ind = (struct irf_gain_err_cali_ind *)msg->param;
	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (ind->status != CO_OK) {
		printk("fb gain offset cali indicates failure: %d\n", ind->status);
		fb_err_cali_status = IRF_CALI_STATUS_FAIL;
		return CO_FAIL;
	}

	cls_wifi_hw->irf_file.fb_gain_err_data_size = ind->gain_err_data_size;
	schedule_work(&cls_wifi_hw->irf_file.fb_err_cali_save_bin_work);

	return CO_OK;
}

static inline int cls_wifi_irf_rx_cali_done_ind(struct cls_wifi_hw *cls_wifi_hw,
								struct cls_wifi_cmd *cmd,
								struct ipc_e2a_msg *msg)
{
	struct irf_rx_cali_done_ind *ind = (struct irf_rx_cali_done_ind *)msg->param;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (ind->status != CO_OK) {
		printk("rx cali indicates failure: %d\n", ind->status);
		rx_cali_status = IRF_CALI_STATUS_FAIL;
		return CO_FAIL;
	}

	rx_cali_status = IRF_CALI_STATUS_DONE;

	return CO_OK;
}

static inline int cls_wifi_irf_tx_err_cali_done_ind(struct cls_wifi_hw *cls_wifi_hw,
										struct cls_wifi_cmd *cmd,
										struct ipc_e2a_msg *msg)
{
	struct irf_gain_err_cali_ind *ind = (struct irf_gain_err_cali_ind *)msg->param;
	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (ind->status != CO_OK) {
		printk("tx gain offset cali indicates failure: %d\n", ind->status);
		tx_err_cali_status = IRF_CALI_STATUS_FAIL;
		return CO_FAIL;
	}

	cls_wifi_hw->irf_file.tx_gain_err_data_size = ind->gain_err_data_size;
	schedule_work(&cls_wifi_hw->irf_file.tx_err_cali_save_bin_work);

	return CO_OK;
}
#endif

static msg_cb_fct mm_hdlrs[MSG_I(MM_MAX)] = {
	[MSG_I(MM_CHANNEL_SWITCH_IND)]	 = cls_wifi_rx_chan_switch_ind,
	[MSG_I(MM_CHANNEL_PRE_SWITCH_IND)] = cls_wifi_rx_chan_pre_switch_ind,
	[MSG_I(MM_REMAIN_ON_CHANNEL_EXP_IND)] = cls_wifi_rx_remain_on_channel_exp_ind,
	[MSG_I(MM_PS_CHANGE_IND)]		  = cls_wifi_rx_ps_change_ind,
	[MSG_I(MM_TRAFFIC_REQ_IND)]		= cls_wifi_rx_traffic_req_ind,
	[MSG_I(MM_P2P_VIF_PS_CHANGE_IND)]  = cls_wifi_rx_p2p_vif_ps_change_ind,
	[MSG_I(MM_CSA_COUNTER_IND)]		= cls_wifi_rx_csa_counter_ind,
	[MSG_I(MM_CSA_FINISH_IND)]		 = cls_wifi_rx_csa_finish_ind,
	[MSG_I(MM_CSA_TRAFFIC_IND)]		= cls_wifi_rx_csa_traffic_ind,
	[MSG_I(MM_REPEATER_CSA_IND)]		= cls_wifi_rx_repeater_csa_ind,
	[MSG_I(MM_CHANNEL_SURVEY_IND)]	 = cls_wifi_rx_channel_survey_ind,
	[MSG_I(MM_P2P_NOA_UPD_IND)]		= cls_wifi_rx_p2p_noa_upd_ind,
	[MSG_I(MM_RSSI_STATUS_IND)]		= cls_wifi_rx_rssi_status_ind,
	[MSG_I(MM_PKTLOSS_IND)]			= cls_wifi_rx_pktloss_notify_ind,
	[MSG_I(MM_CCA_COUNTER_IND)]		= cls_wifi_rx_cca_counter_ind,
	[MSG_I(MM_CCA_FINISH_IND)]		= cls_wifi_rx_cca_finish_ind,
#ifdef __KERNEL__
#ifdef CONFIG_CLS_VBSS
	[MSG_I(MM_GET_SEQ_CFM)]		   = cls_wifi_rx_vbss_seq_num_cfm,
#endif
	[MSG_I(MM_SET_ATF_CFM)]			= cls_wifi_rx_atf_cfm,
	[MSG_I(MM_DPD_WMAC_TX_CMD_IND)]		= cls_wifi_dpd_wmac_tx_cfm,
	[MSG_I(MM_VIP_NODE_IND)]		= cls_wifi_vip_node_ind,
#endif
};

static msg_cb_fct scan_hdlrs[MSG_I(SCANU_MAX)] = {
	[MSG_I(SCANU_START_CFM)]		   = cls_wifi_rx_scanu_start_cfm,
	[MSG_I(SCANU_RESULT_IND)]		  = cls_wifi_rx_scanu_result_ind,
};

static msg_cb_fct me_hdlrs[MSG_I(ME_MAX)] = {
	[MSG_I(ME_TKIP_MIC_FAILURE_IND)] = cls_wifi_rx_me_tkip_mic_failure_ind,
	[MSG_I(ME_TX_CREDITS_UPDATE_IND)] = cls_wifi_rx_me_tx_credits_update_ind,
};

static msg_cb_fct sm_hdlrs[MSG_I(SM_MAX)] = {
	[MSG_I(SM_CONNECT_IND)]	= cls_wifi_rx_sm_connect_ind,
	[MSG_I(SM_DISCONNECT_IND)] = cls_wifi_rx_sm_disconnect_ind,
	[MSG_I(SM_EXTERNAL_AUTH_REQUIRED_IND)] = cls_wifi_rx_sm_external_auth_required_ind,
	[MSG_I(SM_FT_AUTH_IND)] = cls_wifi_rx_sm_ft_auth_ind,
};

static msg_cb_fct apm_hdlrs[MSG_I(APM_MAX)] = {
	[MSG_I(APM_PROBE_CLIENT_IND)] = cls_wifi_rx_apm_probe_client_ind,
};

static msg_cb_fct twt_hdlrs[MSG_I(TWT_MAX)] = {
	[MSG_I(TWT_SETUP_IND)]	= cls_wifi_rx_twt_setup_ind,
	[MSG_I(TWT_SP_IND)]	   = cls_wifi_rx_twt_sp_ind,
};

#if  defined(__KERNEL__)  || CLS_MESH
static msg_cb_fct mesh_hdlrs[MSG_I(MESH_MAX)] = {
	[MSG_I(MESH_PATH_CREATE_CFM)]  = cls_wifi_rx_mesh_path_create_cfm,
	[MSG_I(MESH_PEER_UPDATE_IND)]  = cls_wifi_rx_mesh_peer_update_ind,
	[MSG_I(MESH_PATH_UPDATE_IND)]  = cls_wifi_rx_mesh_path_update_ind,
	[MSG_I(MESH_PROXY_UPDATE_IND)] = cls_wifi_rx_mesh_proxy_update_ind,
};
#endif

static msg_cb_fct dbg_hdlrs[MSG_I(DBG_MAX)] = {
	[MSG_I(DBG_ERROR_IND)]		= cls_wifi_rx_dbg_error_ind,
};

#if  defined(__KERNEL__)  || CLS_TDLS
static msg_cb_fct tdls_hdlrs[MSG_I(TDLS_MAX)] = {
	[MSG_I(TDLS_CHAN_SWITCH_CFM)] = cls_wifi_rx_tdls_chan_switch_cfm,
	[MSG_I(TDLS_CHAN_SWITCH_IND)] = cls_wifi_rx_tdls_chan_switch_ind,
	[MSG_I(TDLS_CHAN_SWITCH_BASE_IND)] = cls_wifi_rx_tdls_chan_switch_base_ind,
	[MSG_I(TDLS_PEER_PS_IND)] = cls_wifi_rx_tdls_peer_ps_ind,
};
#endif

static msg_cb_fct irf_hdlrs[MSG_I(IRF_MAX)] = {
#if defined(__KERNEL__) || IRF_SUPPORT
	[MSG_I(IRF_SAMPLE_START_IND)] = cls_wifi_irf_sample_start_ind,
	[MSG_I(IRF_FB_DCOC_DONE_IND)] = cls_wifi_irf_fb_dcoc_done_ind,
	[MSG_I(IRF_TX_CALI_SAVE_IND)] = cls_wifi_irf_tx_cali_save_ind,
	[MSG_I(IRF_RX_CALI_SAVE_IND)] = cls_wifi_irf_rx_cali_save_ind,
#endif
#ifdef __KERNEL__
	[MSG_I(IRF_SAMPLE_LMS_IND)] = cls_wifi_irf_sample_lms_ind,
	[MSG_I(IRF_START_EQ_IND)] = cls_wifi_irf_start_dif_eq_ind,
	[MSG_I(IRF_DIF_CALI_SAVE_IND)] = cls_wifi_irf_dif_cali_save_ind,
	[MSG_I(IRF_SET_CALIB_EVT_IND)] = cls_wifi_irf_calib_evt_ind,
	[MSG_I(IRF_RX_DCOC_DONE_IND)] = cls_wifi_irf_rx_dcoc_done_ind,
	[MSG_I(IRF_FB_ERR_CALI_IND)] = cls_wifi_irf_fb_err_cali_done_ind,
	[MSG_I(IRF_RX_CALI_DONE_IND)] = cls_wifi_irf_rx_cali_done_ind,
	[MSG_I(IRF_TX_ERR_CALI_IND)] = cls_wifi_irf_tx_err_cali_done_ind,
#endif
};

static msg_cb_fct *msg_hdlrs[] = {
	[TASK_MM]	= mm_hdlrs,
	[TASK_DBG]   = dbg_hdlrs,
	[TASK_CAL]   = cal_hdlrs,
#if  defined(__KERNEL__) || CLS_TDLS
	[TASK_TDLS]  = tdls_hdlrs,
#endif
	[TASK_SCANU] = scan_hdlrs,
	[TASK_ME]	= me_hdlrs,
	[TASK_SM]	= sm_hdlrs,
	[TASK_APM]   = apm_hdlrs,
#if  defined(__KERNEL__) || CLS_MESH
	[TASK_MESH]  = mesh_hdlrs,
#endif
	[TASK_TWT]   = twt_hdlrs,
	[TASK_IRF]	 = irf_hdlrs,
};

#ifdef __KERNEL__
/**
 *
 */
void cls_wifi_rx_handle_msg(struct cls_wifi_hw *cls_wifi_hw, struct ipc_e2a_msg *msg)
{
	cls_wifi_hw->cmd_mgr.msgind(&cls_wifi_hw->cmd_mgr, msg,
							msg_hdlrs[MSG_T(msg->id)][MSG_I(msg->id)]);
}
#endif
