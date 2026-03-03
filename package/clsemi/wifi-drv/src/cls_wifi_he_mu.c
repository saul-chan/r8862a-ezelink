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

#include "cls_wifi_defs.h"
#include "cls_wifi_he_mu.h"
#include "cls_wifi_events.h"

/**
 * cls_wifi_he_mu_init - Initialize HE MU structure
 * @cls_wifi_hw: Main driver data
 */
void cls_wifi_he_mu_init(struct cls_wifi_hw *cls_wifi_hw)
{
	struct cls_wifi_he_mu *he_mu = &cls_wifi_hw->he_mu;
	spin_lock_init(&he_mu->lock);
	INIT_LIST_HEAD(&he_mu->active_sta);
}

/**
 * cls_wifi_he_mu_enable - Enable HE MU transmissions
 * @cls_wifi_hw: Main driver data
 * @vif: AP interface for whic HE MU is enabled
 *
 * HE MU transmissions are enabled when a new (non-empty) DL map is received after
 * they have been disabled.
 * This function stop TXQs for all HE stations (with reason CLS_WIFI_TXQ_STOP_MU), and
 * only TXQs present in the DL map will be re-enabled
 */
static void cls_wifi_he_mu_enable(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *vif)
{
	struct cls_wifi_he_mu *he_mu = &cls_wifi_hw->he_mu;
	struct cls_wifi_sta *sta;
	struct list_head *sta_list;

	trace_he_mu_disable(he_mu->map.idx);
	if (!list_empty(&he_mu->active_sta)) {
		dev_err(cls_wifi_hw->dev, "Enable HE-MU but active station list is not empty\n");
		INIT_LIST_HEAD(&he_mu->active_sta);
	}

	sta_list = &vif->ap.sta_list;
	list_for_each_entry(sta, sta_list, list) {
		if (cls_wifi_sta_is_he(sta)) {
			cls_wifi_txq_sta_stop_he_mu(cls_wifi_hw, sta);
		}
	}
}

/**
 * cls_wifi_he_mu_disable - Disable HE MU transmissions
 * @cls_wifi_hw: Main driver data
 * @vif: AP interface for whic HE MU is enabled
 *
 * When HE MU transmissions are disabled by firmware, all HE stations's TXQs are
 * re-associated with ACx HWQ and their TXQs are restarted for CLS_WIFI_TXQ_STOP_MU
 */
static void cls_wifi_he_mu_disable(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *vif)
{
	struct cls_wifi_he_mu *he_mu = &cls_wifi_hw->he_mu;
	struct cls_wifi_sta *sta, *sta_next;
	struct list_head *sta_list;

	list_for_each_entry_safe(sta, sta_next, &he_mu->active_sta, he_mu) {
		list_del(&sta->he_mu);
	}
	he_mu->active_user = 0;

	sta_list = &vif->ap.sta_list;
	list_for_each_entry(sta, sta_list, list) {
		if (cls_wifi_sta_is_he(sta)) {
			cls_wifi_txq_sta_disable_he_mu(cls_wifi_hw, sta);
		}
	}
}

/**
 * cls_wifi_he_mu_apply_dl_map - Apply a new DL map
 * @cls_wifi_hw: Main driver data
 *
 * Apply the DL map copied locally.
 * If DL map is empty it means that HE-MU has been disabled.
 * Otherwise the list of active is reset (active stations have already been stopped)
 * and stations included in the new DL map are started for HE-MU transmissions
 * and added to the list of active stations.
 *
 * // call this function with HE MU lock taken
 */
static void cls_wifi_he_mu_apply_dl_map(struct cls_wifi_hw *cls_wifi_hw)
{
	struct cls_wifi_he_mu *he_mu = &cls_wifi_hw->he_mu;
	struct cls_wifi_vif *vif;
	struct cls_wifi_sta *sta;
	struct he_statid_desc *desc;
	int i;

	if (!he_mu->map_pending)
		return;

	he_mu->map_pending = false;

	vif = cls_wifi_get_vif(cls_wifi_hw, he_mu->map.vif_index);
	if (!vif || CLS_WIFI_VIF_TYPE(vif) != NL80211_IFTYPE_AP) {
		dev_err(cls_wifi_hw->dev, "Received DL map for non AP interface");
		return;
	}

	if (he_mu->map.cnt_sta == 0) {
		cls_wifi_he_mu_disable(cls_wifi_hw, vif);
		return;
	}

	if (list_empty(&he_mu->active_sta)) {
		// HE-MU is (re-)activated
		cls_wifi_he_mu_enable(cls_wifi_hw, vif);
	} else {
		// HE-MU already active, clear list of active sta
		struct cls_wifi_sta *sta_next;
		list_for_each_entry_safe(sta, sta_next, &he_mu->active_sta, he_mu) {
			list_del(&sta->he_mu);
		}
	}

	he_mu->active_user = 0;
	he_mu->map_hwq = CLS_WIFI_HWQ_VO;
	for (i = 0, desc = he_mu->map.sta_alloc; i < he_mu->map.cnt_sta; i++, desc++)
	{
		struct cls_wifi_hwq *mu_hwq;
		sta = cls_wifi_get_sta(cls_wifi_hw, desc->sta_idx, vif, false);
		if (!sta || !cls_wifi_sta_is_he(sta)) {
			dev_err(cls_wifi_hw->dev, "Invalid Sta index %d in DL map\n", desc->sta_idx);
			continue;
		}

		// station may enter in PS between the moment the we received the DL map
		// and when we apply it
		if (sta->ps.active)
			continue;

		mu_hwq = cls_wifi_hwq_mu_reset(cls_wifi_hw, CLS_WIFI_HWQ_USER_BASE + desc->userid,
								   desc->credit, desc->psdu_len_max);
		if (!mu_hwq) {
			dev_err(cls_wifi_hw->dev, "Cannot find MU HWQ %d\n", desc->userid);
			continue;
		}
		cls_wifi_txq_sta_start_he_mu(cls_wifi_hw, sta, mu_hwq, desc->tidmap);
		list_add_tail(&sta->he_mu, &he_mu->active_sta);
		he_mu->active_user |= BIT(desc->userid);

		if (desc->tidmap & 0x6) // special test for tid 1 and tid 2
			he_mu->map_hwq = CLS_WIFI_HWQ_BK;
		else if (desc->tidmap) {
			int hwq = cls_wifi_tid2hwq[ffs(desc->tidmap) - 1];
			if (hwq < he_mu->map_hwq)
				he_mu->map_hwq = hwq;
		}
	}

	trace_he_mu_apply_dl_map(&he_mu->map, he_mu->map_hwq);
}

/**
 * cls_wifi_he_mu_process_dl_map - Process a new DL map reported by firmware
 * @cls_wifi_hw: Main driver data
 * @map: New HE MU DL map to process
 *
 * The map is first copied locally, and then all HE stations currently scheduled for
 * HE MU are stopped (i.e. their TXQs are stopped so that no new buffers can be sent
 * to the firmware for those stations).
 * If all MU HWQ are flushed (i.e. there are no more buffer pending at firmware level)
 * then the new DL map is applied immediately. Otherwise application of this new DL
 * map is postponed until all MU HWQ are flushed (cf cls_wifi_he_mu_cfm_hwq)
 */
void cls_wifi_he_mu_process_dl_map(struct cls_wifi_hw *cls_wifi_hw,
							   struct he_mu_map_array_desc *map)
{
	struct cls_wifi_he_mu *he_mu = &cls_wifi_hw->he_mu;
	struct cls_wifi_hwq *hwq;
	struct cls_wifi_sta *sta;
	int i;

	trace_he_mu_dl_map(map->idx, map->cnt_sta);
	spin_lock(&he_mu->lock);

	// Copy map locally
	memcpy(&he_mu->map, map, sizeof(he_mu->map));
	he_mu->map_pending = true;

	if (!list_empty(&he_mu->active_sta)) {
		// A DL map is already active, stop all TXQs linked to MU HWQ to flush them
		spin_lock(&cls_wifi_hw->tx_lock);
		list_for_each_entry(sta, &he_mu->active_sta, he_mu) {
			cls_wifi_txq_sta_stop_he_mu(cls_wifi_hw, sta);
		}
		spin_unlock(&cls_wifi_hw->tx_lock);
	}

	// Don't use active_sta list as station may have been removed from the active list
	for (hwq = &cls_wifi_hw->hwq[CLS_WIFI_HWQ_USER_BASE], i=0;
		 i < CLS_WIFI_HWQ_NB - CLS_WIFI_HWQ_USER_BASE; i++, hwq++) {
		if ((he_mu->active_user & BIT(i)) && cls_wifi_hwq_is_flushed(hwq))
			he_mu->active_user &= ~BIT(i);
	}

	if (!he_mu->active_user) {
		// If all MU HWQ are already flushed we can apply the new map now
		spin_lock(&cls_wifi_hw->tx_lock);
		cls_wifi_he_mu_apply_dl_map(cls_wifi_hw);
		spin_unlock(&cls_wifi_hw->tx_lock);
	}

	spin_unlock(&he_mu->lock);
}

/**
 * cls_wifi_he_mu_cfm_hwq - Update traffic statistic and apply new DL map if needed
 * after transmission confirmation.
 * @cls_wifi_hw: Main driver data
 * @hwq: HWQ for which firmware confirmed transmission of a buffer
 * @sw_txhdr: Software TX descriptor of the frame confirmed
 *
 * This function apply a new HE MU DL map if:
 * - The buffer confirmed has been pushed on a MU HWQ
 * - There is a pending DL map to apply
 * - and now all MU HWQ are flushed
 */
void cls_wifi_he_mu_cfm_hwq(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_hwq *hwq,
						struct cls_wifi_sw_txhdr *sw_txhdr)
{
	struct cls_wifi_he_mu *he_mu = &cls_wifi_hw->he_mu;

	if (hwq->id < CLS_WIFI_HWQ_USER_BASE)
		// do nothing for non MU HWQ
		return;

	if (sw_txhdr->frame_len > hwq->size_pushed)
		hwq->size_pushed = 0;
	else
		hwq->size_pushed -= sw_txhdr->frame_len;

	spin_lock(&he_mu->lock);

	if (!he_mu->map_pending)
		goto release_lock;

	if (!cls_wifi_hwq_is_flushed(hwq))
		goto release_lock;

	he_mu->active_user &= ~BIT(hwq->id - CLS_WIFI_HWQ_USER_BASE);
	if (!he_mu->active_user)
		 cls_wifi_he_mu_apply_dl_map(cls_wifi_hw);

release_lock:
	spin_unlock(&he_mu->lock);
}

/**
 * cls_wifi_he_mu_sta_ps_update - Handle HE station that change their PS status
 *
 * @cls_wifi_hw: Main driver data
 * @sta: Station to process
 * @ps_active: Whether sta enter (true) or exit (false) PS mode.
 *
 * Does nothing if sta is not a HE station or HE MU is not active.
 * When a HE station enter station enter PS it is necessary to treat it as
 * non-HE station (i.e. its TXQ but be linked to ACx HWQ to handle traffic
 * request).
 * When a HE station exist PS it should be considered as a non scheduled
 * station.
 */
void cls_wifi_he_mu_sta_ps_update(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_sta *sta,
							  bool ps_active)
{
	struct cls_wifi_he_mu *he_mu = &cls_wifi_hw->he_mu;
	if (list_empty(&he_mu->active_sta) || !cls_wifi_sta_is_he(sta))
		return;

	lockdep_assert_held(&cls_wifi_hw->tx_lock);

	if (ps_active) {
		struct cls_wifi_sta *sta_iter, *sta_next;
		list_for_each_entry_safe(sta_iter, sta_next, &he_mu->active_sta, he_mu) {
			if (sta_iter == sta) {
				struct cls_wifi_hwq *hwq = cls_wifi_hwq_mu_sta_get(cls_wifi_hw, sta);
				if (!hwq)
					dev_err(cls_wifi_hw->dev, "Active HE-MU station %d not linked to a MU HWQ\n",
							sta->sta_idx);
				list_del(&sta->he_mu);
				break;
			}
		}
		cls_wifi_txq_sta_disable_he_mu(cls_wifi_hw, sta);
	} else
		// If station was part of the current active DL map, maybe we should restart it ?
		cls_wifi_txq_sta_stop_he_mu(cls_wifi_hw, sta);
}


/**
 * cls_wifi_he_mu_set_user - Set MAP/User info in TX descriptor
 *
 * @cls_wifi_hw: Main driver data
 * @sw_txhdr: Tx descriptor to update
 * @return HW queue on which the TX descriptor should be sent
 *
 * Initialize HE-MU information in a TX descriptor if scheduled for HE-MU transmission
 */
u16 cls_wifi_he_mu_set_user(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_sw_txhdr *sw_txhdr)
{
	struct cls_wifi_hwq *hwq = sw_txhdr->txq->hwq;

	if (hwq->id < CLS_WIFI_HWQ_USER_BASE)
		return hwq->id;

	hwq->size_pushed += sw_txhdr->frame_len;

	sw_txhdr->desc.api.host.user_idx = hwq->id - CLS_WIFI_HWQ_USER_BASE;
	sw_txhdr->desc.api.host.rua_map_idx = cls_wifi_hw->he_mu.map.idx ;
	return cls_wifi_hw->he_mu.map_hwq;
}
