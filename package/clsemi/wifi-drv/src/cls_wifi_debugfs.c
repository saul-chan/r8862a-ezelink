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

#include <linux/kernel.h>
#include <linux/kmod.h>
#include <linux/debugfs.h>
#include <linux/string.h>
#include <linux/sort.h>
#include <linux/firmware.h>
#include <linux/math64.h>
#include <linux/version.h>

#include "cls_wifi_debugfs.h"
#include "cls_wifi_msg_tx.h"
#include "cls_wifi_radar.h"
#include "cls_wifi_tx.h"
#include "cls_wifi_version.h"
#include "cls_wifi_cfgfile.h"
#include "cls_wifi_msgq.h"
#include "ipc_shared.h"
#include "cls_wifi_irf.h"
#include "cls_wifi_csi.h"
#include "cls_wifi_atf.h"
#include "cls_wifi_pci.h"
#include "cls_wifi_power_tbl.h"
#ifdef CFG_PCIE_SHM
#include "cls_wifi_pci_shm.h"
#endif

static ssize_t cls_wifi_dbgfs_stats_read(struct file *file,
									 char __user *user_buf,
									 size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char *buf;
	int ret;
	int i, skipped;
#ifdef CONFIG_CLS_WIFI_SPLIT_TX_BUF
	int per;
#endif
	ssize_t read;
	int bufsz = (CLS_TXQ_CNT) * 20 + (ARRAY_SIZE(priv->stats.amsdus_rx) + 1) * 40
		+ (ARRAY_SIZE(priv->stats.ampdus_tx) * 30);

	if (*ppos)
		return 0;

	buf = kmalloc(bufsz, GFP_ATOMIC);
	if (buf == NULL)
		return 0;

	ret = scnprintf(buf, bufsz, "TXQs CFM balances ");
	for (i = 0; i < CLS_TXQ_CNT; i++)
		ret += scnprintf(&buf[ret], bufsz - ret,
						 "  [%1d]:%3d", i,
						 priv->stats.cfm_balance[i]);

	ret += scnprintf(&buf[ret], bufsz - ret, "\n");

#ifdef CONFIG_CLS_WIFI_SPLIT_TX_BUF
	ret += scnprintf(&buf[ret], bufsz - ret,
					 "\nAMSDU[len]	   done		 failed   received\n");
	for (i = skipped = 0; i < CLS_TX_PAYLOAD_MAX; i++) {
		if (priv->stats.amsdus[i].done) {
			per = DIV_ROUND_UP((priv->stats.amsdus[i].failed) *
							   100, priv->stats.amsdus[i].done);
		} else if (priv->stats.amsdus_rx[i]) {
			per = 0;
		} else {
			per = 0;
			skipped = 1;
			continue;
		}
		if (skipped) {
			ret += scnprintf(&buf[ret], bufsz - ret, "   ...\n");
			skipped = 0;
		}

		ret += scnprintf(&buf[ret], bufsz - ret,
						 "   [%2d]	%10d %8d(%3d%%) %10d\n",  i ? i + 1 : i,
						 priv->stats.amsdus[i].done,
						 priv->stats.amsdus[i].failed, per,
						 priv->stats.amsdus_rx[i]);
	}

	for (; i < ARRAY_SIZE(priv->stats.amsdus_rx); i++) {
		if (!priv->stats.amsdus_rx[i]) {
			skipped = 1;
			continue;
		}
		if (skipped) {
			ret += scnprintf(&buf[ret], bufsz - ret, "   ...\n");
			skipped = 0;
		}

		ret += scnprintf(&buf[ret], bufsz - ret,
						 "   [%2d]							  %10d\n",
						 i + 1, priv->stats.amsdus_rx[i]);
	}
#else
	ret += scnprintf(&buf[ret], bufsz - ret,
					 "\nAMSDU[len]   received\n");
	for (i = skipped = 0; i < ARRAY_SIZE(priv->stats.amsdus_rx); i++) {
		if (!priv->stats.amsdus_rx[i]) {
			skipped = 1;
			continue;
		}
		if (skipped) {
			ret += scnprintf(&buf[ret], bufsz - ret,
							 "   ...\n");
			skipped = 0;
		}

		ret += scnprintf(&buf[ret], bufsz - ret,
						 "   [%2d]	%10d\n",
						 i + 1, priv->stats.amsdus_rx[i]);
	}

#endif /* CONFIG_CLS_WIFI_SPLIT_TX_BUF */

	ret += scnprintf(&buf[ret], bufsz - ret,
					 "\nAMPDU[len]	 done  received\n");
	for (i = skipped = 0; i < ARRAY_SIZE(priv->stats.ampdus_tx); i++) {
		if (!priv->stats.ampdus_tx[i] && !priv->stats.ampdus_rx[i]) {
			skipped = 1;
			continue;
		}
		if (skipped) {
			ret += scnprintf(&buf[ret], bufsz - ret,
							 "	...\n");
			skipped = 0;
		}

		ret += scnprintf(&buf[ret], bufsz - ret,
						 "   [%2d]   %9d %9d\n", i ? i + 1 : i,
						 priv->stats.ampdus_tx[i], priv->stats.ampdus_rx[i]);
	}

	ret += scnprintf(&buf[ret], bufsz - ret,
					 "#mpdu missed		%9d\n",
					 priv->stats.ampdus_rx_miss);
	ret += scnprintf(&buf[ret], bufsz - ret,
					"#rx_msdu_truncated  %9d\n#rx_mmpdu_truncated  %9d\n",
					priv->stats.rx_msdu_truncated, priv->stats.rx_mmpdu_truncated);
	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	kfree(buf);

	return read;
}

static ssize_t cls_wifi_dbgfs_stats_write(struct file *file,
									  const char __user *user_buf,
									  size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;

	/* Prevent from interrupt preemption as these statistics are updated under
	 * interrupt */
	spin_lock_bh(&priv->tx_lock);

	memset(&priv->stats, 0, sizeof(priv->stats));

	spin_unlock_bh(&priv->tx_lock);

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(stats);

#define TXQ_STA_PREF "tid|"
#define TXQ_STA_PREF_FMT "%3d|"

#define TXQ_VIF_PREF "type|"
#define TXQ_VIF_PREF_FMT "%4s|"

#define TXQ_HDR "idx|   status|credit|ready|retry|pushed"
#define TXQ_HDR_FMT "%3d|%s%s%s%s%s%s%s%s%s|%6d|%5d|%5d|%6d"

#ifdef CONFIG_CLS_WIFI_AMSDUS_TX
#define TXQ_HDR_SUFF "|amsdu|fc_drop"
#define TXQ_HDR_SUFF_FMT "|%5d|%5d"
#else
#define TXQ_HDR_SUFF ""
#define TXQ_HDR_SUF_FMT ""
#endif /* CONFIG_CLS_WIFI_AMSDUS_TX */

#define TXQ_HDR_MAX_LEN (sizeof(TXQ_STA_PREF) + sizeof(TXQ_HDR) + sizeof(TXQ_HDR_SUFF) + 1)

#define PS_HDR  "Legacy PS: ready=%d, sp=%d / UAPSD: ready=%d, sp=%d"
#define PS_HDR_LEGACY "Legacy PS: ready=%d, sp=%d"
#define PS_HDR_UAPSD  "UAPSD: ready=%d, sp=%d"
#define PS_HDR_MAX_LEN  sizeof("Legacy PS: ready=xxx, sp=xxx / UAPSD: ready=xxx, sp=xxx\n")

#define STA_HDR "** STA %d (%pM)\n"
#define STA_HDR_MAX_LEN sizeof("- STA xx (xx:xx:xx:xx:xx:xx)\n") + PS_HDR_MAX_LEN

#define VIF_HDR "* VIF [%d] %s\n"
#define VIF_HDR_MAX_LEN sizeof(VIF_HDR) + IFNAMSIZ


#ifdef CONFIG_CLS_WIFI_AMSDUS_TX

#define VIF_SEP "---------------------------------------\n"

#else /* ! CONFIG_CLS_WIFI_AMSDUS_TX */
#define VIF_SEP "---------------------------------\n"
#endif /* CONFIG_CLS_WIFI_AMSDUS_TX*/

#define VIF_SEP_LEN sizeof(VIF_SEP)

#define CAPTION "status: L=in hwq list, F=stop full, P=stop sta PS, V=stop vif PS,\
 C=stop channel, S=stop CSA, M=stop MU, T=TWT, N=Ndev queue stopped"
#define CAPTION_LEN sizeof(CAPTION)

#define STA_TXQ 0
#define VIF_TXQ 1

static int cls_wifi_dbgfs_txq(char *buf, size_t size, struct cls_wifi_txq *txq, int type, int tid, char *name)
{
	int res, idx = 0;

	if (type == STA_TXQ) {
		res = scnprintf(&buf[idx], size, TXQ_STA_PREF_FMT, tid);
		idx += res;
		size -= res;
	} else {
		res = scnprintf(&buf[idx], size, TXQ_VIF_PREF_FMT, name);
		idx += res;
		size -= res;
	}

	res = scnprintf(&buf[idx], size, TXQ_HDR_FMT, txq->idx,
					(txq->status & CLS_WIFI_TXQ_IN_HWQ_LIST) ? "L" : " ",
					(txq->status & CLS_WIFI_TXQ_STOP_FULL) ? "F" : " ",
					(txq->status & CLS_WIFI_TXQ_STOP_STA_PS) ? "P" : " ",
					(txq->status & CLS_WIFI_TXQ_STOP_VIF_PS) ? "V" : " ",
					(txq->status & CLS_WIFI_TXQ_STOP_CHAN) ? "C" : " ",
					(txq->status & CLS_WIFI_TXQ_STOP_CSA) ? "S" : " ",
					(txq->status & CLS_WIFI_TXQ_STOP_MU) ? "M" : " ",
					(txq->status & CLS_WIFI_TXQ_STOP_TWT) ? "T" : " ",
					(txq->status & CLS_WIFI_TXQ_NDEV_FLOW_CTRL) ? "N" : " ",
					txq->credits, skb_queue_len(&txq->sk_list),
					txq->nb_retry, txq->pkt_pushed);
	idx += res;
	size -= res;

#ifdef CONFIG_CLS_WIFI_AMSDUS_TX
	if (type == STA_TXQ) {
		res = scnprintf(&buf[idx], size, TXQ_HDR_SUFF_FMT,
						txq->amsdu_len, txq->pkt_fc_drop);
		idx += res;
		size -= res;
	}
#endif

	res = scnprintf(&buf[idx], size, "\n");
	idx += res;
	size -= res;

	return idx;
}

static int cls_wifi_dbgfs_txq_sta(char *buf, size_t size, struct cls_wifi_sta *cls_wifi_sta,
							  struct cls_wifi_hw *cls_wifi_hw)
{
	int tid, res, idx = 0;
	struct cls_wifi_txq *txq;

	res = scnprintf(&buf[idx], size, "\n" STA_HDR,
					cls_wifi_sta->sta_idx,
					cls_wifi_sta_addr(cls_wifi_sta)
					);
	idx += res;
	size -= res;

	if (cls_wifi_sta->ps.active) {
		if (cls_wifi_sta->uapsd_tids &&
			(cls_wifi_sta->uapsd_tids == ((1 << CLS_NB_TXQ_PER_STA) - 1)))
			res = scnprintf(&buf[idx], size, PS_HDR_UAPSD "\n",
							cls_wifi_sta->ps.pkt_ready[UAPSD_ID],
							cls_wifi_sta->ps.sp_cnt[UAPSD_ID]);
		else if (cls_wifi_sta->uapsd_tids)
			res = scnprintf(&buf[idx], size, PS_HDR "\n",
							cls_wifi_sta->ps.pkt_ready[LEGACY_PS_ID],
							cls_wifi_sta->ps.sp_cnt[LEGACY_PS_ID],
							cls_wifi_sta->ps.pkt_ready[UAPSD_ID],
							cls_wifi_sta->ps.sp_cnt[UAPSD_ID]);
		else
			res = scnprintf(&buf[idx], size, PS_HDR_LEGACY "\n",
							cls_wifi_sta->ps.pkt_ready[LEGACY_PS_ID],
							cls_wifi_sta->ps.sp_cnt[LEGACY_PS_ID]);
		idx += res;
		size -= res;
	} else {
		res = scnprintf(&buf[idx], size, "\n");
		idx += res;
		size -= res;
	}


	res = scnprintf(&buf[idx], size, TXQ_STA_PREF TXQ_HDR TXQ_HDR_SUFF "\n");
	idx += res;
	size -= res;


	foreach_sta_txq(cls_wifi_sta, txq, tid, cls_wifi_hw) {
		res = cls_wifi_dbgfs_txq(&buf[idx], size, txq, STA_TXQ, tid, NULL);
		idx += res;
		size -= res;
	}

	return idx;
}

static int cls_wifi_dbgfs_txq_vif(char *buf, size_t size, struct cls_wifi_vif *cls_wifi_vif,
							  struct cls_wifi_hw *cls_wifi_hw)
{
	int res, idx = 0;
	struct cls_wifi_txq *txq;
	struct cls_wifi_sta *cls_wifi_sta;

	res = scnprintf(&buf[idx], size, VIF_HDR, cls_wifi_vif->vif_index, cls_wifi_vif->ndev->name);
	idx += res;
	size -= res;
	if (!cls_wifi_vif->up || cls_wifi_vif->ndev == NULL)
		return idx;


	if (CLS_WIFI_VIF_TYPE(cls_wifi_vif) ==  NL80211_IFTYPE_AP ||
		CLS_WIFI_VIF_TYPE(cls_wifi_vif) ==  NL80211_IFTYPE_P2P_GO ||
		CLS_WIFI_VIF_TYPE(cls_wifi_vif) ==  NL80211_IFTYPE_MESH_POINT) {
		res = scnprintf(&buf[idx], size, TXQ_VIF_PREF TXQ_HDR "\n");
		idx += res;
		size -= res;
		txq = cls_wifi_txq_vif_get(cls_wifi_vif, CLS_UNK_TXQ_TYPE);
		res = cls_wifi_dbgfs_txq(&buf[idx], size, txq, VIF_TXQ, 0, "UNK");
		idx += res;
		size -= res;
		txq = cls_wifi_txq_vif_get(cls_wifi_vif, CLS_BCMC_TXQ_TYPE);
		res = cls_wifi_dbgfs_txq(&buf[idx], size, txq, VIF_TXQ, 0, "BCMC");
		idx += res;
		size -= res;
		cls_wifi_sta = &cls_wifi_hw->sta_table[cls_wifi_vif->ap.bcmc_index];
		if (cls_wifi_sta->ps.active) {
			res = scnprintf(&buf[idx], size, PS_HDR_LEGACY "\n",
							cls_wifi_sta->ps.sp_cnt[LEGACY_PS_ID],
							cls_wifi_sta->ps.sp_cnt[LEGACY_PS_ID]);
			idx += res;
			size -= res;
		} else {
			res = scnprintf(&buf[idx], size, "\n");
			idx += res;
			size -= res;
		}

		list_for_each_entry(cls_wifi_sta, &cls_wifi_vif->ap.sta_list, list) {
			res = cls_wifi_dbgfs_txq_sta(&buf[idx], size, cls_wifi_sta, cls_wifi_hw);
			idx += res;
			size -= res;
		}
	} else if (CLS_WIFI_VIF_TYPE(cls_wifi_vif) ==  NL80211_IFTYPE_STATION ||
			   CLS_WIFI_VIF_TYPE(cls_wifi_vif) ==  NL80211_IFTYPE_P2P_CLIENT) {
		if (cls_wifi_vif->sta.ap) {
			res = cls_wifi_dbgfs_txq_sta(&buf[idx], size, cls_wifi_vif->sta.ap, cls_wifi_hw);
			idx += res;
			size -= res;
		}
	}

	return idx;
}

static ssize_t cls_wifi_dbgfs_txq_read(struct file *file ,
								   char __user *user_buf,
								   size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *cls_wifi_hw = file->private_data;
	struct cls_wifi_vif *vif;
	char *buf;
	int idx, res;
	ssize_t read;
	uint16_t nb_txq = CLS_NB_TXQ(hw_vdev_max(cls_wifi_hw), hw_remote_sta_max(cls_wifi_hw));
	size_t bufsz = ((hw_remote_sta_max(cls_wifi_hw) * (VIF_HDR_MAX_LEN + 2 * VIF_SEP_LEN)) +
					(hw_remote_sta_max(cls_wifi_hw) * STA_HDR_MAX_LEN) +
					((hw_remote_sta_max(cls_wifi_hw) + hw_vdev_max(cls_wifi_hw) + nb_txq) *
					 TXQ_HDR_MAX_LEN) + CAPTION_LEN);

	/* everything is read in one go */
	if (*ppos)
		return 0;

	bufsz = min_t(size_t, bufsz, count);
	buf = kmalloc(bufsz, GFP_ATOMIC);
	if (buf == NULL)
		return 0;

	bufsz--;
	idx = 0;

	res = scnprintf(&buf[idx], bufsz, CAPTION);
	idx += res;
	bufsz -= res;

	//spin_lock_bh(&cls_wifi_hw->tx_lock);
	list_for_each_entry(vif, &cls_wifi_hw->vifs, list) {
		res = scnprintf(&buf[idx], bufsz, "\n"VIF_SEP);
		idx += res;
		bufsz -= res;
		res = cls_wifi_dbgfs_txq_vif(&buf[idx], bufsz, vif, cls_wifi_hw);
		idx += res;
		bufsz -= res;
		res = scnprintf(&buf[idx], bufsz, VIF_SEP);
		idx += res;
		bufsz -= res;
	}
	//spin_unlock_bh(&cls_wifi_hw->tx_lock);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, idx);
	kfree(buf);

	return read;
}
DEBUGFS_READ_FILE_OPS(txq);

static ssize_t cls_wifi_dbgfs_acsinfo_read(struct file *file,
										   char __user *user_buf,
										   size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[(SCAN_CHANNEL_MAX + 1) * 43];
	int survey_cnt = 0;
	int len = 0;
	int band, chan_cnt;

	mutex_lock(&priv->dbgdump.mutex);

	len += scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					 "FREQ	TIME(ms)	BUSY(ms)	NOISE(dBm)\n");

	for (band = NL80211_BAND_2GHZ; band <= NL80211_BAND_5GHZ; band++) {
		if (!(priv->band_cap & (1 << band)))
			continue;
		for (chan_cnt = 0; chan_cnt < priv->if_cfg80211.sbands[band].n_channels; chan_cnt++) {
			struct cls_wifi_survey_info *p_survey_info = &priv->survey[survey_cnt];
			struct ieee80211_channel *p_chan = &priv->if_cfg80211.sbands[band].channels[chan_cnt];

			if (p_survey_info->filled) {
				len += scnprintf(&buf[len], min_t(size_t, sizeof(buf) - len - 1, count),
								 "%d	%03d		 %03d		 %d\n",
								 p_chan->center_freq,
								 p_survey_info->chan_time_ms,
								 p_survey_info->chan_time_busy_ms,
								 p_survey_info->noise_dbm);
			} else {
				len += scnprintf(&buf[len], min_t(size_t, sizeof(buf) -len -1, count),
								 "%d	NOT AVAILABLE\n",
								 p_chan->center_freq);
			}

			survey_cnt++;
		}
	}

	mutex_unlock(&priv->dbgdump.mutex);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

DEBUGFS_READ_FILE_OPS(acsinfo);

static ssize_t cls_wifi_dbgfs_fw_dbg_read(struct file *file,
										   char __user *user_buf,
										   size_t count, loff_t *ppos)
{
	char help[]="usage: [MOD:<ALL|KE|DBG|IPC|DMA|MM|TX|RX|PHY>]* "
		"[DBG:<NONE|CRT|ERR|WRN|INF|VRB>]\n";

	return simple_read_from_buffer(user_buf, count, ppos, help, sizeof(help));
}


static ssize_t cls_wifi_dbgfs_fw_dbg_write(struct file *file,
											const char __user *user_buf,
											size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int idx = 0;
	u32 mod = 0;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;
	buf[len] = '\0';

#define CLS_WIFI_MOD_TOKEN(str, val)										\
	if (strncmp(&buf[idx], str, sizeof(str) - 1 ) == 0) {			   \
		idx += sizeof(str) - 1;										 \
		mod |= val;													 \
		continue;													   \
	}

#define CLS_WIFI_DBG_TOKEN(str, val)								\
	if (strncmp(&buf[idx], str, sizeof(str) - 1) == 0) {		\
		idx += sizeof(str) - 1;								 \
		dbg = val;											  \
		goto dbg_done;										  \
	}

	while ((idx + 4) < len) {
		if (strncmp(&buf[idx], "MOD:", 4) == 0) {
			idx += 4;
			CLS_WIFI_MOD_TOKEN("ALL", 0xffffffff);
			CLS_WIFI_MOD_TOKEN("KE",  BIT(0));
			CLS_WIFI_MOD_TOKEN("DBG", BIT(1));
			CLS_WIFI_MOD_TOKEN("IPC", BIT(2));
			CLS_WIFI_MOD_TOKEN("DMA", BIT(3));
			CLS_WIFI_MOD_TOKEN("MM",  BIT(4));
			CLS_WIFI_MOD_TOKEN("TX",  BIT(5));
			CLS_WIFI_MOD_TOKEN("RX",  BIT(6));
			CLS_WIFI_MOD_TOKEN("PHY", BIT(7));
			idx++;
		} else if (strncmp(&buf[idx], "DBG:", 4) == 0) {
			u32 dbg = 0;
			idx += 4;
			CLS_WIFI_DBG_TOKEN("NONE", 0);
			CLS_WIFI_DBG_TOKEN("CRT",  1);
			CLS_WIFI_DBG_TOKEN("ERR",  2);
			CLS_WIFI_DBG_TOKEN("WRN",  3);
			CLS_WIFI_DBG_TOKEN("INF",  4);
			CLS_WIFI_DBG_TOKEN("VRB",  5);
			idx++;
			continue;
		  dbg_done:
			cls_wifi_send_dbg_set_sev_filter_req(priv, dbg);
		} else {
			idx++;
		}
	}

	if (mod) {
		cls_wifi_send_dbg_set_mod_filter_req(priv, mod);
	}

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(fw_dbg);

static ssize_t cls_wifi_dbgfs_sys_stats_read(struct file *file,
										 char __user *user_buf,
										 size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[3*64];
	int len = 0;
	ssize_t read;
	int error = 0;
	struct dbg_get_sys_stat_cfm cfm;
	u32 sleep_int, sleep_frac, doze_int, doze_frac;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Get the information from the FW */
	if ((error = cls_wifi_send_dbg_get_sys_stat_req(priv, &cfm)))
		return error;

	if (cfm.stats_time == 0)
		return 0;

	sleep_int = ((cfm.cpu_sleep_time * 100) / cfm.stats_time);
	sleep_frac = (((cfm.cpu_sleep_time * 100) % cfm.stats_time) * 10) / cfm.stats_time;
	doze_int = ((cfm.doze_time * 100) / cfm.stats_time);
	doze_frac = (((cfm.doze_time * 100) % cfm.stats_time) * 10) / cfm.stats_time;

	len += scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					 "\nSystem statistics:\n");
	len += scnprintf(&buf[len], min_t(size_t, sizeof(buf) - 1, count),
					 "  CPU sleep [%%]: %d.%d\n", sleep_int, sleep_frac);
	len += scnprintf(&buf[len], min_t(size_t, sizeof(buf) - 1, count),
					 "  Doze	  [%%]: %d.%d\n", doze_int, doze_frac);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, len);

	return read;
}

DEBUGFS_READ_FILE_OPS(sys_stats);

#define DBG_CNT_VAL(name)                   (dbg_cnt_env->name)
#define PRINT_DBG_CNT(name)                 pr_warn("%s = %d\n", #name, DBG_CNT_VAL(name))
#define PRINT_DBG_CNT0(name)                if(DBG_CNT_VAL(name))  \
                                               pr_warn("%s = %d\n", #name, DBG_CNT_VAL(name))
#define PRINT_DBG2_CNT(name, name1)        pr_warn("%s = %d, %s = %d\n", #name, \
                                                DBG_CNT_VAL(name), #name1, DBG_CNT_VAL(name1))

#define PRINT_DBG3_CNT(name, name1, name2)        pr_warn("%s = %d, %s = %d\n", #name, \
                                                    DBG_CNT_VAL(name), #name1, DBG_CNT_VAL(name1),  \
                                                    #name2, DBG_CNT_VAL(name2))


#define PRINT_DBG_ARRAY_CNT(name,i)         pr_warn("%s[%d] = %d\n", #name, i, DBG_CNT_VAL(name[i]))
#define PRINT_DBG_ARRAY_CNT0(name,i)        if(DBG_CNT_VAL(name[i])) \
                                                 pr_warn("%s[%d] = %d\n", #name, i, DBG_CNT_VAL(name[i]))
#define PRINT_DBG_ARRAY2_CNT(name, i, j)    pr_warn("%s[%d][%d] = %d\n", #name, i, j, DBG_CNT_VAL(name[i][j]))
#define PRINT_DBG_ARRAY2_CNT0(name, i, j)   if(DBG_CNT_VAL(name[i][j]))  \
                                                pr_warn("%s[%d][%d] = %d\n", #name, i, j, DBG_CNT_VAL(name[i][j]))

#define PRINT_DBG_AMPDU_CNT0(i, j)   do{  \
        if ((DBG_CNT_VAL(tx_ampdu_cnt[i][j]))  \
            || (DBG_CNT_VAL(tx_ampdu_ba_succ[i][j]))) { \
            pr_warn("TX AMPDU[%d][%d] cnt %d; ba_succ pkt: %d; edca_faild: %d; ba_loss: %d; partial_ba: %d;\n", i, j + 1,   \
                DBG_CNT_VAL(tx_ampdu_cnt[i][j]),DBG_CNT_VAL(tx_ampdu_ba_succ[i][j]),   \
                DBG_CNT_VAL(tx_ampdu_edca_faild[i][j]),  \
                DBG_CNT_VAL(tx_ampdu_ba_loss_faild[i][j]), DBG_CNT_VAL(tx_ampdu_partial_ba_faild[i][j]));   \
         }   \
    }while(0)

static ssize_t cls_wifi_dbgfs_dbgcnt_read(struct file *file,
				char __user *user_buf,
				size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *cls_wifi_hw = file->private_data;
	struct ipc_shared_dbg_cnt *dbg_cnt_env;
	ssize_t read = 0;
	int i, j;

	struct cls_wifi_plat *plat;

	plat = cls_wifi_hw->plat;

	if (!plat) {
		pr_warn("%s %d can't find plat \n", __func__, __LINE__);
		return 0;
	}

	if (plat->hw_rev == CLS_WIFI_HW_DUBHE2000) {
		dbg_cnt_env = kmalloc(sizeof(struct ipc_shared_dbg_cnt), GFP_KERNEL);
		if (!dbg_cnt_env)
			return 0;
	}

	/* Get the information from the FW */
	if (cls_wifi_send_dbg_get_dbgcnt_req(cls_wifi_hw)) {
		pr_warn("cls_wifi_send_dbg_get_dbg_cnt failed\n");
		if (plat->hw_rev == CLS_WIFI_HW_DUBHE2000) {
			if (dbg_cnt_env)
				kfree(dbg_cnt_env);
		}
		return 0;
	}

	if (plat->hw_rev == CLS_WIFI_HW_DUBHE2000)
		ipc_host_dbg_cnt_get(cls_wifi_hw->ipc_env, dbg_cnt_env);
	else if (plat->hw_rev == CLS_WIFI_HW_MERAK2000)
		dbg_cnt_env = (struct ipc_shared_dbg_cnt *)cls_wifi_hw->dbg_cnt_buf.addr;

	PRINT_DBG_CNT(main_loop);
	PRINT_DBG_CNT0(ipc_hostbuf_get_cnt);
	for (i = 0; i < CLS_TXQ_CNT; i++)
		PRINT_DBG_ARRAY_CNT0(hostbuf_ac_cnt, i);

	PRINT_DBG2_CNT(tx_ampdu_last_mpdu_cnt, tx_ampdu_done_cnt);
	PRINT_DBG2_CNT(tx_cfm_cnt, tx_cfm_total_cnt);
	for (j = 0; j < AC_MAX; j++) {
		PRINT_DBG_ARRAY_CNT0(rx_ba_cnt, j);
		for (i = 0; i < (MAX_TXDESC_CNT + 1); i++)
			PRINT_DBG_AMPDU_CNT0(j, i);
	}

	PRINT_DBG_CNT(tx_mpdu_hwretry_cnt);
	for (j = 0; j < AC_MAX; j++)
		PRINT_DBG_ARRAY_CNT(tx_ampdu_hwretry_cnt, j);
	PRINT_DBG_CNT0(tx_repush_mpdu_cnt);

	for (j = 0; j < AC_MAX; j++)
		PRINT_DBG_ARRAY_CNT(tx_ampdu_rtscts_retry_limit, j);
	for (j = 0; j < AC_MAX; j++)
		PRINT_DBG_ARRAY_CNT(tx_ampdu_rtscts_retry_limit_ignore, j);
	for (j = 0; j < AC_MAX; j++)
		PRINT_DBG_ARRAY_CNT(tx_ampdu_rtscts_retry_limit_expire, j);
	for (j = 0; j < AC_MAX; j++)
		for (i = 0; i < 2; i++)
			PRINT_DBG_ARRAY2_CNT0(tx_ampdu_split_time, j, i);

	///PRINT_DBG2_CNT(rx_ampdu_hw_cnt, rx_ampdu_hw_cnt_last);
	pr_warn("rx_ampdu_hw_cnt = 0x%x, rx_ampdu_hw_cnt_last = 0x%x\n",
			DBG_CNT_VAL(rx_ampdu_hw_cnt), DBG_CNT_VAL(rx_ampdu_hw_cnt_last));
	for (i = 0; i < (MAX_RXDESC_CNT + 1); i++)
		PRINT_DBG_ARRAY_CNT0(rx_ampdu_cnt, i);

	PRINT_DBG2_CNT(tx_isr, tx_done_isr);
	PRINT_DBG2_CNT(rx_done_isr, rx_evt);
	PRINT_DBG2_CNT(rx_msdu_cnt, rx_mpdu_cnt);
	PRINT_DBG_CNT0(rx_data_mpdu_cnt);
	PRINT_DBG_CNT0(tx_trigger_frame_cnt);
	PRINT_DBG_CNT0(rx_tb_mpdu_cnt);
	PRINT_DBG_CNT0(rx_tb_mpdu_err_cnt);
	PRINT_DBG_CNT0(rx_tb_qos_data_cnt);
	PRINT_DBG_CNT0(rx_tb_qos_null_cnt);
	PRINT_DBG_CNT0(rx_mismatch_amsdu_disagg);

	PRINT_DBG_CNT(bitmap);
	PRINT_DBG2_CNT(rxbuf1_overflow, rxbuf2_overflow);
	PRINT_DBG_CNT0(rxbuf1_hw_read_delay_update);
	PRINT_DBG_CNT0(rxbuf2_hw_read_delay_update);
	PRINT_DBG_CNT0(phyif_overflow);
	PRINT_DBG_CNT0(rx_fcs_error_cnt);
	PRINT_DBG_CNT0(rx_undef_error_cnt);
	PRINT_DBG_CNT0(rx_decry_error_cnt);
	PRINT_DBG_CNT(tx_phy_err_data_len);
	PRINT_DBG_CNT(tx_ampdu_bw_drop);
	PRINT_DBG_CNT(tx_ppdu_drop_twt_ps);
	PRINT_DBG_CNT(tx_ppdu_drop_omi);
	PRINT_DBG_CNT(tx_ampdu_rts_cts_war);

	PRINT_DBG_CNT(reset_ppdu_cnt);
	PRINT_DBG_CNT(reset_mpdu_cnt);
	PRINT_DBG_CNT(reset_count);
	PRINT_DBG_CNT(reset_count_with_mpdu);
	PRINT_DBG_CNT(reset_with_frequent_int);
	PRINT_DBG_CNT0(dpd_wmac_tx_ampdu);
	PRINT_DBG_CNT0(dpd_wmac_tx_smpdu);

	PRINT_DBG_CNT0(sound_cnt);
	PRINT_DBG_CNT0(rx_cbf_cnt);
	PRINT_DBG_CNT0(cbf_download_cnt);
	PRINT_DBG_CNT0(cbf_timeout);
	PRINT_DBG_CNT0(tx_bf_used);

	PRINT_DBG_CNT0(try_mu_group);
	PRINT_DBG_CNT0(mu_select_buddy);
	PRINT_DBG_CNT0(mu_lack_sta_num);
	PRINT_DBG_CNT0(mu_airtime_overlength);
	PRINT_DBG_CNT0(mu_buddy_select_mu);
	PRINT_DBG_CNT0(mu_buddy_select_su);
	PRINT_DBG_CNT0(mu_buddy_fail_1st);
	PRINT_DBG_CNT0(mu_buddy_fail_ps);
	PRINT_DBG_CNT0(mu_buddy_fail_he);
	PRINT_DBG_CNT0(mu_buddy_fail_mpdu);
	PRINT_DBG_CNT0(mu_buddy_fail_bytes);
	PRINT_DBG_CNT0(mu_buddy_fail_bw);
	PRINT_DBG_CNT0(mu_buddy_fail_rc_format);
	PRINT_DBG_CNT0(mu_buddy_fail_mcs);
	PRINT_DBG_CNT0(mu_format_ppdu);
	PRINT_DBG_CNT0(mu_format_with_qos_null);
	PRINT_DBG_CNT0(mu_chain_mu_bar);
	PRINT_DBG_CNT0(mu_tx_mpdu_cnt);
	PRINT_DBG_CNT0(mu_tx_ampdu_cnt);
	PRINT_DBG_CNT0(mu_tx_mpdu_failed_cnt);
	PRINT_DBG_CNT0(mu_release_pending);
	PRINT_DBG_CNT0(mu_abnormal_no_desc);
	PRINT_DBG_CNT0(mu_done_ppdu_failed);
	PRINT_DBG_CNT0(mu_done_trig_done);
	PRINT_DBG_CNT0(mu_done_trig_hang);
	PRINT_DBG_CNT0(mu_done_trig_success);
	PRINT_DBG_CNT0(mu_done_trig_failed);
	PRINT_DBG_CNT0(mu_format_mu_bar_ba_cnt);
	PRINT_DBG_CNT0(mu_format_mu_bar_no_ba);
	for(i = 0; i < 16; i++)
		PRINT_DBG_ARRAY_CNT0(mu_user_num, i);

	PRINT_DBG_CNT(txop_enabled);
	PRINT_DBG_CNT(txop_disabled);

	if (plat->hw_rev == CLS_WIFI_HW_MERAK2000) {

		PRINT_DBG_CNT0(ac0_tx_dma_dead_cnt);
		PRINT_DBG_CNT0(ac1_tx_dma_dead_cnt);
		PRINT_DBG_CNT0(ac2_tx_dma_dead_cnt);
		PRINT_DBG_CNT0(ac3_tx_dma_dead_cnt);
		PRINT_DBG_CNT0(bcn_tx_dma_dead_cnt);

		PRINT_DBG_CNT(mu_rua);
		PRINT_DBG_CNT(mu_rua_failed);
		PRINT_DBG_CNT(mu_format_seq_bar);
		PRINT_DBG_CNT(mu_format_mu_bar);
		PRINT_DBG_CNT(mu_format_unknown_bar);
		PRINT_DBG_CNT(mu_chain_seq_bar);
		PRINT_DBG_CNT(mu_chain_ppdu);
		PRINT_DBG_CNT(mu_done_ppdu_retran);
		PRINT_DBG_CNT(mu_done_ppdu_all_done);
		PRINT_DBG_CNT(mu_done_bar_no_next);

		PRINT_DBG_CNT(reset_ppdu_cnt);
		PRINT_DBG_CNT(reset_mpdu_cnt);
		PRINT_DBG_CNT(reset_count);
		PRINT_DBG_CNT(reset_count_with_mpdu);
		PRINT_DBG_CNT(reset_with_frequent_int);
		PRINT_DBG_CNT(dpd_wmac_tx_ampdu);
		PRINT_DBG_CNT(dpd_wmac_tx_smpdu);

		PRINT_DBG_CNT(sound_cnt);
		PRINT_DBG_CNT(sound_push_cnt);
		PRINT_DBG_CNT(rx_cbf_cnt);
		PRINT_DBG_CNT(cbf_upload_cnt);
		PRINT_DBG_CNT(cbf_download_cnt);
		PRINT_DBG_CNT(cbf_timeout);
		PRINT_DBG_CNT(tx_bf_used);

		PRINT_DBG_CNT(msgq_txdesc);
		PRINT_DBG_CNT(msgq_rxdesc);
	}

	if (plat->hw_rev == CLS_WIFI_HW_DUBHE2000) {
		if (dbg_cnt_env)
			kfree(dbg_cnt_env);
	}
	return read;
}

DEBUGFS_READ_FILE_OPS(dbgcnt);


static ssize_t cls_wifi_dbgfs_mib_read(struct file *file,
										 char __user *user_buf,
										 size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	int buf_len = (MIB_ITEM_NB + 16) * 64;
	char *buf = kzalloc(buf_len, GFP_ATOMIC);
	int len = 0, i, j, max_item, qid = 0, fid = 0, n, offset;
	ssize_t read = 0;
	int error = 0;
	char** str = NULL;
	u32_l *info = NULL;
	struct dbg_mac_reg_info *mac_reg;
	struct dbg_get_mib_cfm *cfm = kzalloc(sizeof(struct dbg_get_mib_cfm), GFP_ATOMIC);
	static const char* basic_str[BASIC_MIB_SET_NUM] = {
		"TKIP decrypt err:     %u\n",
		"CCMP128 decrypt err:  %u\n",
		"CCMP256 decrypt err:  %u\n",
		"GCMP128 decrypt err:  %u\n",
		"GCMP256 decrypt err:  %u\n",
		"WAPI decrypt err:     %u\n",
		"RX FCS err pkts:  %u\n",
		"RX phyerr pkts:   %u\n",
		"RX rxerr pkts:    %u\n",
		"RX rxbuf overflow:%u\n",
		"DMA pattern err:       %u\n",
		"DMA pointer err:       %u\n",
		"RX incorrect Delimeter:%u\n",
		"TX underrun:          %u\n",
		"MPIF RX_FIFO overflow:%u\n",
		"MPIF TX_END miss:     %u\n",
		"MPIF RX_END miss:     %u\n",
		"RX protocol err:      %u\n"
	};
	static const char* edca_str[EDCA_MIB_SET_NUM_ITEM] = {
		"TX Unicast MPDU stats(AC%d): %u\n",
		"TX MC/BC MPDU stats(AC%d):   %u\n",
		"QOS FAILED stats(AC%d):      %u\n",
		"QOS RETRY stats(AC%d):       %u\n",
		"QOS RTS Succ stats(AC%d):    %u\n",
		"QOS RTS Fail stats(AC%d):    %u\n",
		"QOS RTS ACK Fail stats(AC%d):%u\n",
		"RX Unicast MPDU stats(AC%d): %u\n",
		"RX MC/BC MPDU stats(AC%d):   %u\n",
		"RX Unicast other MPDU(AC%d): %u\n",
		"RX Retry MPDU(AC%d):         %u\n"
	};
	static const char* tb_str[TRIGGER_MIB_SET_NUM] = {
			"TB type0: %u\n",
			"TB type1: %u\n",
			"TB type2: %u\n",
			"TB type3: %u\n",
			"TB type4: %u\n",
			"TB type5: %u\n",
			"TB type6: %u\n",
			"TB type7: %u\n",
			"TB rx packed:     %u\n",
			"TB rx mpdu packed:%u\n",
			"TB rx bytes:      %u\n"
		};
	static const char* ampdu_str[AMPDU_MIB_SET_NUM] = {
		"TX AMPDU frame:   %u\n",
		"TX MPDU  frame:   %u\n",
		"TX AMPDU bytes:   %u\n",
		"RX AMPDU frame:   %u\n",
		"RX MPDU  frame:   %u\n",
		"RX AMSDU frame:   %u\n",
		"RX AMPDU bytes:   %u\n",
		"RX BA fail(implicit BAR): %u\n",
		"RX BA fail(explicit BAR): %u\n",
		"TX Beacon:    %u\n",
		"TX Trigger:   %u\n",
		"RX ACK:       %u\n",
		"RX BA:        %u\n",
		"RX Beacon:    %u\n",
		"TX MU OFDMA:  %u\n",
		"TX MU MIMO:   %u\n"
	};
	static const char* bw_str[BW_TXRX_MIB_SET_NUM] = {
		"BW20 TX frame:    %u\n",
		"BW40 TX frame:    %u\n",
		"BW80 TX frame:    %u\n",
		"BW160 TX frame:   %u\n",
		"BW20 RX frame:    %u\n",
		"BW40 RX frame:    %u\n",
		"BW80 RX frame:    %u\n",
		"BW160 RX frame:   %u\n",
		"BW20 TXOP Fail:   %u\n",
		"BW20 TXOP Succ:   %u\n",
		"BW40 TXOP Fail:   %u\n",
		"BW40 TXOP Succ:   %u\n",
		"BW80 TXOP Fail:   %u\n",
		"BW80 TXOP Succ:   %u\n",
		"BW160 TXOP Fail:  %u\n",
		"BW160 TXOP Succ:  %u\n",
		"Dynamic BW Drop:  %u\n",
		"Static BW TX fail:%u\n",
		"Spatial Reuse MCS drop:   %u\n"
	};
	static const char* bf_str[BF_MIB_SET_NUM] = {
		"TX Beamformed Frame:  %u\n",
		"CBF Report Error: %u\n",
		"RX Beamformed Frame:  %u\n",
		"BRFPT Frame:  %u\n"
	};

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if ((buf == NULL) || (cfm == NULL)){
		if (cfm) {
			kfree(cfm);
		}
		if (buf) {
			kfree(buf);
		}
		return 0;
	}

	/* Get the information from the FW */
	if ((error = cls_wifi_send_dbg_get_mib_req(priv, cfm))){
		len += scnprintf(&buf[len], min_t(size_t, buf_len - len - 1, count),
					 "cls_wifi_send_dbg_get_mib_req error: 0x%x\n",error);
		read = simple_read_from_buffer(user_buf, count, ppos, buf, len);
		if (cfm) {
			kfree(cfm);
		}
		if (buf) {
			kfree(buf);
		}
		pr_warn("cls_wifi_send_dbg_get_mib_req error:%d\n",error);
		return read;
	}

	len += scnprintf(&buf[len], min_t(size_t, buf_len - len - 1, count),
						"get MIB time:%llu,mib_flag:0x%x\n", cfm->cur_wpu_time, cfm->mib_flag);

	for(i = 0; i < MIB_MAX_ITEM; i++) {
		if(!(cfm->mib_flag & (1U << i)))
			continue;
		qid = 0;
		fid = 0;
		offset = 0;

		if (i == 0) {
			max_item = BASIC_MIB_SET_NUM;
			str = (char**)basic_str;
			info = cfm->mib.basic.info;
		}
		else if(i == 1) {
			max_item = EDCA_MIB_SET_NUM_ITEM;  ///EDCA type
			str = (char**)edca_str;
			qid = 8;
			fid = 8;
			info = cfm->mib.edca.info;
				len += scnprintf(&buf[len], min_t(size_t, buf_len - len - 1, count),
						"AC0~AC4 is AC_BK...; ACAC5~AC8 is timer0~3;AC6 is beacon; AC7 is reserved.\n");
			for(j = 0; j < max_item; j++){
				for(n = 0; j < 7 && n < qid && str[j]; n++){
					len += scnprintf(&buf[len], min_t(size_t, buf_len - len - 1, count),
								str[j], n, info[offset]);
					offset++;
				}
				for(n = 0; j >= 7 && n < fid && str[j]; n++){
					len += scnprintf(&buf[len], min_t(size_t, buf_len - len - 1, count),
								str[j], n, info[offset]);
					offset++;
				}
			}
			max_item = 0;
		}
		else if(i == 2) {
			max_item = TRIGGER_MIB_SET_NUM;
			str = (char**)tb_str;
			info = cfm->mib.tb.info;
		}
		else if(i == 3) {
			max_item = AMPDU_MIB_SET_NUM;
			str = (char**)ampdu_str;
			info = cfm->mib.ampdu.info;
		}
		else if(i == 4) {
			max_item = BW_TXRX_MIB_SET_NUM;
			str = (char**)bw_str;
			info = cfm->mib.bw.info;
		}
		else if(i == 5) {
			max_item = BF_MIB_SET_NUM;
			str = (char**)bf_str;
			info = cfm->mib.bfr.info;
		}
		else {
			max_item = 0;
		}
		for(j = 0; j < max_item && str[j]; j++){
			len += scnprintf(&buf[len], min_t(size_t, buf_len - len - 1, count),
				str[j], info[j]);
		}
	}

	len += scnprintf(&buf[len], min_t(size_t, buf_len - len - 1, count),
						"=== MIB END ===\n");
	read = simple_read_from_buffer(user_buf, count, ppos, buf, len);

	if(cfm->mib_flag & MIB_UPDATE_STATIC_REG)
	{
#define MACREG_DUMP(name)  pr_warn("%s: 0x%x\n", #name, mac_reg->name)

		mac_reg = &cfm->mac_reg;
		MACREG_DUMP(mpif_underflow_dbg);
		MACREG_DUMP(mpif_tb_rx_err_dbg);
		MACREG_DUMP(mac_rx_hang_ctrl);
		MACREG_DUMP(mac_rx_hang_dbg0);
		MACREG_DUMP(mac_rx_hang_dbg1);
		MACREG_DUMP(mpif_underflow_dbg2);
		MACREG_DUMP(mpif_underflow_dbg3);
		MACREG_DUMP(rx_vector1_dbg[0]);
		MACREG_DUMP(rx_vector1_dbg[1]);
		MACREG_DUMP(rx_vector1_dbg[2]);
		MACREG_DUMP(rx_vector1_dbg[3]);
		MACREG_DUMP(rx_vector1_dbg[4]);
	}

	if (buf) {
		kfree(buf);
		buf = NULL;
	}

	if (cfm) {
		kfree(cfm);
		cfm = NULL;
	}

	return read;
}

DEBUGFS_READ_FILE_OPS(mib);

static ssize_t cls_wifi_dbgfs_dfx_read(struct file *file, char __user *user_buf, size_t count,
		loff_t *ppos)
{
	struct cls_wifi_hw *hw = file->private_data;
	struct cls_wifi_vif *vif;
	struct cls_wifi_sta *sta;

	list_for_each_entry(vif, &hw->vifs, list) {
		pr_warn("vif[%d] txb %llu txm %llu txu %llu rxb %llu rxm %llu rxu %llu\n",
				vif->drv_vif_index,
				vif->dfx_stats.tx_broadcast, vif->dfx_stats.tx_multicast,
				vif->dfx_stats.tx_unicast, vif->dfx_stats.rx_broadcast,
				vif->dfx_stats.rx_multicast, vif->dfx_stats.rx_unicast);
		cls_wifi_send_dbg_get_mgmt_req(hw, vif->vif_index,
				&vif->dfx_stats.mgmt_stats);
		pr_warn("tx proberesp %llu auth %llu deauth %llu assocreq %llu assocresp %llu "
				"reassocreq %llu reassocresp %llu disassoc %llu beacon %llu\n",
				vif->dfx_stats.mgmt_stats.tx_stats.tx_probersppkts,
				vif->dfx_stats.mgmt_stats.tx_stats.tx_authpkts,
				vif->dfx_stats.mgmt_stats.tx_stats.tx_deauthpkts,
				vif->dfx_stats.mgmt_stats.tx_stats.tx_assocreqpkts,
				vif->dfx_stats.mgmt_stats.tx_stats.tx_assocrsppkts,
				vif->dfx_stats.mgmt_stats.tx_stats.tx_reascreqpkts,
				vif->dfx_stats.mgmt_stats.tx_stats.tx_reascrsppkts,
				vif->dfx_stats.mgmt_stats.tx_stats.tx_disassocpkts,
				vif->dfx_stats.mgmt_stats.tx_stats.tx_bcnpkts);
		pr_warn("rx proberesp %llu auth %llu deauth %llu assocreq %llu assocresp %llu "
				"reassocreq %llu reassocresp %llu disassoc %llu\n",
				vif->dfx_stats.mgmt_stats.rx_stats.rx_probersppkts,
				vif->dfx_stats.mgmt_stats.rx_stats.rx_authpkts,
				vif->dfx_stats.mgmt_stats.rx_stats.rx_deauthpkts,
				vif->dfx_stats.mgmt_stats.rx_stats.rx_assocreqpkts,
				vif->dfx_stats.mgmt_stats.rx_stats.rx_assocrsppkts,
				vif->dfx_stats.mgmt_stats.rx_stats.rx_reascreqpkts,
				vif->dfx_stats.mgmt_stats.rx_stats.rx_reascrsppkts,
				vif->dfx_stats.mgmt_stats.rx_stats.rx_disassocpkts);
		list_for_each_entry(sta, &vif->ap.sta_list, list) {
			pr_warn("sta[%pM] txb %llu txm %llu txu %llu rxb %llu rxm %llu rxu %llu\n",
					sta->mac_addr,
					sta->dfx_stats.tx_broadcast, sta->dfx_stats.tx_multicast,
					sta->dfx_stats.tx_unicast, sta->dfx_stats.rx_broadcast,
					sta->dfx_stats.rx_multicast, sta->dfx_stats.rx_unicast);
		}
	}
	return 0;
}

static ssize_t cls_wifi_dbgfs_dfx_write(struct file *file, const char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *hw = file->private_data;
	struct cls_wifi_vif *vif;
	struct cls_wifi_sta *sta;

	list_for_each_entry(vif, &hw->vifs, list) {
		memset(&vif->dfx_stats, 0, sizeof(vif->dfx_stats));
		cls_wifi_send_dbg_reset_mgmt_req(hw, vif->vif_index);
		list_for_each_entry(sta, &vif->ap.sta_list, list) {
			memset(&sta->dfx_stats, 0, sizeof(sta->dfx_stats));
		}
	}
	return count;
}
DEBUGFS_READ_WRITE_FILE_OPS(dfx);

static ssize_t cls_wifi_dbgfs_phydfx_read(struct file *file,
										 char __user *user_buf,
										 size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	int buf_len = (PHYDFX_ITEM_NB + 16) * 64;
	char *buf = kzalloc(buf_len, GFP_ATOMIC);
	int len = 0;
	int i;
	ssize_t read = 0;
	int error = 0;
	u16_l *ptr16;
	u32_l *ptr32;
	struct phydfx_abn_rpt *abn_rpt;
	struct phydfx_ppdu_txv *txv;
	struct phy_dfx_comm_info *comm;
	struct phydfx_ppdu_static *ppdu_tx_rx;
	struct dbg_get_phy_dfx_cfm *cfm = kzalloc(sizeof(struct dbg_get_phy_dfx_cfm), GFP_ATOMIC);
	static const char* ppdu_tx_rx_str[PHYDFX_PPDU_TRX] = {
		"tx_ofdm_start_cnt:     %d\n",
		"tx_dsss_start_cnt:     %d\n",
		"tx_ppdu_end_cnt:       %d\n",
		"rx_ofdm_start_cnt:     %d\n",
		"rx_dsss_start_cnt:     %d\n",
		"rx_ppdu_timingend_cnt: %d\n",
		"rx_ppdu_end_cnt:       %d\n",
		"rx_ppdu_fd_done_cnt:   %d\n",
		"rx_ppdu_sfo_done_cnt:  %d\n",
		"rx_ppdu_td_done_cnt:   %d\n",
		"tx_err_ppdu_cnt:   %d\n",
		"rx_err_ppdu_cnt:   %d\n"
	};

	static const char* abn_str[PHYDFX_PPDU_TRX] = {
		"rx_abn_rpt_aa70:   0x%x\n",
		"rx_abn_rpt_aa74:   0x%x\n",
		"rx_time_rpt_aa78:  0x%x\n",
		"rx_time_rpt_aa7c:  0x%x\n",
		"rx_ppdu_idx_aa80:  0x%x\n"
	};

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if ((buf == NULL) || (cfm == NULL)) {
		if (cfm) {
			kfree(cfm);
		}
		if (buf) {
			kfree(buf);
		}
		return 0;
	}

	/* Get the information from the FW */
	if ((error = cls_wifi_send_dbg_get_phy_dfx_req(priv, cfm))){
		len += scnprintf(&buf[len], min_t(size_t, buf_len - len - 1, count),
					 "cls_wifi_send_dbg_get_phy_dfx_req error: 0x%x\n", error);
		read = simple_read_from_buffer(user_buf, count, ppos, buf, len);
		if (cfm) {
			kfree(cfm);
		}
		if (buf) {
			kfree(buf);
		}
		pr_warn("cls_wifi_send_dbg_get_phy_dfx_req error:%d\n",error);
		return read;
	}

	len += scnprintf(&buf[len], min_t(size_t, buf_len - len - 1, count),
						"get PHY-DFX time:%u,mib_flag:0x%x\n", cfm->cur_wpu_time, cfm->mib_flag);

	comm = &cfm->dfx.comm;
	len += scnprintf(&buf[len], min_t(size_t, buf_len - len - 1, count),
					"mfsm_tx_state: %d, mfsm_rx_state: %d\n"
					"mdmb_txfifo_count: %d, mdmb_rxfifo_count: %d\n",
					comm->comm_info_rpt[0]&0xF,(comm->comm_info_rpt[0] >> 4) & 0xF,
					(comm->comm_info_rpt[0] >> 8)&0xF,(comm->comm_info_rpt[0] >> 13)&0xF);
	len += scnprintf(&buf[len], min_t(size_t, buf_len - len - 1, count),
				"smp_errterm_datalen:0x%x\n", comm->comm_info_rpt[1]);
	len += scnprintf(&buf[len], min_t(size_t, buf_len - len - 1, count),
				"smp_errterm_ppduidx:0x%x\n", comm->comm_info_rpt[2]);

	ppdu_tx_rx = &cfm->dfx.ppdu_tx_rx;
	ptr16 = &ppdu_tx_rx->tx_ofdm_start_cnt;
	for(i = 0; i < PHYDFX_PPDU_TRX; i++) {
		len += scnprintf(&buf[len], min_t(size_t, buf_len - len - 1, count),
				ppdu_tx_rx_str[i], ptr16[i]);
	}

	abn_rpt = &cfm->dfx.abn_rpt;
	ptr32 = &abn_rpt->rx_abn_rpt_aa70;
	for(i = 0; i < PHYDFX_ABN_RPT; i++) {
		len += scnprintf(&buf[len], min_t(size_t, buf_len - len - 1, count),
				abn_str[i], ptr32[i]);
	}

	txv = &cfm->dfx.txv;
	for(i = 0; i < 9; i++) {
		len += scnprintf(&buf[len], min_t(size_t, buf_len - len - 1, count),
				"txv[%d]: 0x%x\n", i,txv->txv[i]);
	}

	len += scnprintf(&buf[len], min_t(size_t, buf_len - len - 1, count),
				"txv_abn: 0x%x\n", txv->txv_abn);

	len += scnprintf(&buf[len], min_t(size_t, buf_len - len - 1, count),
				"txbf_abn: 0x%x\n", txv->txbf_abn);

	len += scnprintf(&buf[len], min_t(size_t, buf_len - len - 1, count),
				"txv_mask: 0x%x\n", txv->txv_mask);

	len += scnprintf(&buf[len], min_t(size_t, buf_len - len - 1, count),
						"=== PHY-DFX END ===\n");
	read = simple_read_from_buffer(user_buf, count, ppos, buf, len);

	if (buf) {
		kfree(buf);
		buf = NULL;
	}

	if (cfm) {
		kfree(cfm);
		cfm = NULL;
	}

	return read;
}

DEBUGFS_READ_FILE_OPS(phydfx);

#if !defined(DUBHE2000)
/**
 * cls_wifi_irf_agc_reload() - Reload AGC ucode
 *
 * @cls_wifi_plat: platform data
 * c.f Modem UM (AGC/CCA initialization)
 */
static int cls_wifi_irf_agc_reload(struct cls_wifi_hw *hw, u8 radio_index)
{
	int ret = 0;
	const char *agcbin;
	const struct firmware *fw;
	struct cls_wifi_plat *cls_wifi_plat;
	struct device *dev;

	if (!hw) {
		pr_err("hw is null\n");
		return -1;
	}
	cls_wifi_plat = hw->plat;
	if (!cls_wifi_plat) {
		pr_err("cls_wifi_plat is null\n");
		return -1;
	}
	dev = cls_wifi_plat->dev;
	if (!dev) {
		pr_err("dev is null\n");
		return -1;
	}
	agcbin = cls_wifi_plat->hw_params.agc_file_name[radio_index];
	dev_err(dev, "AGC load:%s\n", agcbin);

	ret = request_firmware(&fw, agcbin, dev);
	if (ret) {
		dev_err(dev, "Requesting agc bin failed:%s for radio%d\n", agcbin,
				radio_index);
		return ret;
	}
	cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_index,
			CLS_WIFI_AGCBIN_OFFSET, (void *)fw->data, (u32)fw->size);
	release_firmware(fw);

	ret = cls_wifi_send_irf_agc_reload_req(hw, hw->radio_idx, fw->size);
	if (ret) {
		dev_err(dev, "%s-%d: failed %d\n", __func__, __LINE__, ret);
		return ret;
	}

	return ret;
}
#endif

static ssize_t cls_wifi_dbgfs_reload_agcram_write(struct file *file,
									const char __user *user_buf,
									size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;

	if(priv->radio_idx)
		pr_warn("load /lib/firmware/agcram.bin to agcram\n");
	else
		pr_warn("load /lib/firmware/agcram_2g.bin to agcram\n");

#if defined(DUBHE2000) && DUBHE2000
	priv->plat->ep_ops->reload_agc(priv->plat, priv->radio_idx);
#else
	if (cls_wifi_irf_agc_reload(priv, priv->radio_idx))
		return -1;
#endif

	return count;
}

DEBUGFS_WRITE_FILE_OPS(reload_agcram);

static ssize_t cls_wifi_dbgfs_wfa_wmm_write(struct file *file,
									const char __user *user_buf,
									size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[1024];
	size_t len = min_t(size_t, count, sizeof(buf) - 1);
	int lock_edca = 0;

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	if (sscanf(buf, "wmm_lock=%d", &lock_edca) > 0) {
		pr_warn("radio%u wmm_lock: %s\n",
				priv->radio_idx, (lock_edca >= 1) ? "enable" : "disable");
		lock_edca = (lock_edca >= 1) ? 0x01 : 0x00;
		cls_wifi_send_wmm_lock(priv, lock_edca);
	} else if (sscanf(buf, "overl_bss=%d", &lock_edca) > 0) {
		pr_warn("radio%u overl_bss: %s\n",
				priv->radio_idx, (lock_edca >= 1) ? "enable" : "disable");
		lock_edca = (lock_edca >= 1) ? 0xA1 : 0xA0;
		cls_wifi_send_wmm_lock(priv, lock_edca);
	} else {
		pr_warn("radio%u wmm_lock set failed\n",
				priv->radio_idx);
	}

	return count;
}

DEBUGFS_WRITE_FILE_OPS(wfa_wmm);
extern ktime_t uptime_ktime;
static ssize_t cls_wifi_dbgfs_rxdesc_read(struct file *file,
										 char __user *user_buf,
										 size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	struct ipc_host_env_tag *env = priv->ipc_env;
	char buf[128];
	int len = 0;
	uint16_t i;
	s64 uptime_ns;
	unsigned long uptime_sec;
	uint32_t value_dma_addr = 0;
	uint16_t used = 0;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	for (i = 0; i < env->rxbuf_nb; i++) {
		value_dma_addr = env->ops->read32(env->plat, env->radio_idx,
				(env->rxbuf_offset + i * sizeof(struct ipc_shared_rx_buf)
				 + sizeof(u32_l)));
		if (value_dma_addr == 0)
			used++;
	}
	uptime_ns = ktime_to_ns(uptime_ktime);
	uptime_sec = (unsigned long)div_s64(uptime_ns, NSEC_PER_SEC);

	len += scnprintf(&buf[len], min_t(size_t, sizeof(buf) - len - 1, count),
			"rxbuf cnt %d  used %d lastrxdata %lu\n", env->rxbuf_nb,
			used, uptime_sec);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

DEBUGFS_READ_FILE_OPS(rxdesc);

static ssize_t cls_wifi_dbgfs_ipc_stats_read(struct file *file,
										 char __user *user_buf,
										 size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	struct ipc_host_env_tag *env = priv->ipc_env;
	char buf[128];
	int len = 0;
	uint16_t i;
	uint32_t value_dma_addr = 0;
	uint16_t used = 0;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	for (i = 0; i < env->txdesc_nb; i++) {
		value_dma_addr = env->ops->read32(env->plat, env->radio_idx,
				(env->txdesc_offset + i * sizeof(u32_l)));
		if (value_dma_addr != 0)
			used++;
	}

	len += scnprintf(&buf[len], min_t(size_t, sizeof(buf) - len - 1, count),
			"tx_pushed %d tx_cdm %d txdesc_nb %d used %d tx_idx %d\n",
			priv->ipc_stats.ipc_tx_push, priv->ipc_stats.ipc_tx_cfm,
			env->txdesc_nb, used, env->txdesc_addr_idx);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

DEBUGFS_READ_FILE_OPS(ipc_stats);

static ssize_t cls_wifi_dbgfs_hw_plat_read(struct file *file,
										 char __user *user_buf,
										 size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[256];
	int len = 0;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (priv->plat && priv->plat->hw_rev == CLS_WIFI_HW_MERAK2000) {
		u32 *chip_id = priv->plat->chip_id;

		len = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
				"m2k chipid %X-%X-%X-%X-%X\n", chip_id[0],
				chip_id[1], chip_id[2], chip_id[3], chip_id[4]);
	} else
		len = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count), "d2k\n");

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

DEBUGFS_READ_FILE_OPS(hw_plat);

static ssize_t cls_wifi_dbgfs_version_read(struct file *file,
										 char __user *user_buf,
										 size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[256];
	int len = 0, remain = sizeof(buf), res;
	u32 fw_ver = priv->version_cfm.version_lmac;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	res = scnprintf(buf, remain, "%s v%d.%d.%d.%d - build: %s\n",
					"fmac",
					(fw_ver & (0xff << 24)) >> 24, (fw_ver & (0xff << 16)) >> 16,
					(fw_ver & (0xff <<  8)) >>  8, (fw_ver & (0xff <<  0)) >>  0,
					priv->version_cfm.build_info);

	if (res >= remain) {
		len += remain;
		goto end;
	}
	len += res;
	remain -= res;

	res = scnprintf(&buf[len], remain, "cls_wifi %s - build: %s - %s\n",
					cls_wifi_mod_version, cls_wifi_build_date, cls_wifi_build_version);
	if (res >= remain)
		len += remain;
	else
		len += res + 1;

end:
	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

DEBUGFS_READ_FILE_OPS(version);

static ssize_t cls_wifi_dbgfs_command_read(struct file *file, char __user *user_buf, size_t count,
		loff_t *ppos)
{
	struct cls_wifi_hw *cls_wifi_hw = file->private_data;
	static char buf[256*8];
	int vif_idx, len = 0;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	for(vif_idx = 0; vif_idx < CLS_ITF_MAX; vif_idx++) {
		if (!cls_wifi_hw->vif_table[vif_idx] || !cls_wifi_hw->vif_table[vif_idx]->ndev)
			continue;

		len += scnprintf(buf + len, min_t(size_t, sizeof(buf) - 1, count) - len,
				"vif_id:%d name:%s status:\n", vif_idx, cls_wifi_hw->vif_table[vif_idx]->ndev->name);
		len += scnprintf(buf + len, min_t(size_t, sizeof(buf) - 1, count) - len,
				"use_4addr %d\nauto_4addr %d\nforce_4addr %d\n"
				"m2u_3addr_resend %d\nlog_enable %d\ndump_pkt %d\n",
				cls_wifi_hw->vif_table[vif_idx]->use_4addr,
				cls_wifi_hw->vif_table[vif_idx]->auto_4addr,
				cls_wifi_hw->vif_table[vif_idx]->force_4addr,
				cls_wifi_hw->vif_table[vif_idx]->m2u_3addr_resend,
				cls_wifi_hw->vif_table[vif_idx]->log_enable,
				cls_wifi_hw->vif_table[vif_idx]->dump_pkt);

	}

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t cls_wifi_dbgfs_command_write(struct file *file, const char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *cls_wifi_hw = file->private_data;
	char string[64], command[64], ifname[64];
	int vif_idx, value = 0;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (count > 64) {
		pr_warn("%s %d, string too long[%d].\n", __func__, __LINE__, (int)(count - 1));
		return -EFAULT;
	}

	memset(string, 0, sizeof(string));
	memset(command, 0, sizeof(command));

	/* Get the content of the file */
	if (copy_from_user(string, user_buf, count))
		return -EFAULT;

	string[count - 1] = '\0';
	if (sscanf(string, "%s %s %d", ifname, command, &value) == 3) {
		pr_warn("Set ifname:%s %s to %d\n", ifname, command, value);
		for(vif_idx = 0; vif_idx < CLS_ITF_MAX; vif_idx++)
		{
			if (!cls_wifi_hw->vif_table[vif_idx] ||
					!cls_wifi_hw->vif_table[vif_idx]->ndev ||
					strncmp(cls_wifi_hw->vif_table[vif_idx]->ndev->name, ifname, 63))
				continue;

			if (!strcasecmp(command, "use_4addr"))
				cls_wifi_hw->vif_table[vif_idx]->use_4addr = value;
			else if (!strcasecmp(command, "auto_4addr"))
				cls_wifi_hw->vif_table[vif_idx]->auto_4addr = value;
			else if (!strcasecmp(command, "force_4addr"))
				cls_wifi_hw->vif_table[vif_idx]->force_4addr = value;
			else if (!strcasecmp(command, "m2u_3addr_resend"))
				cls_wifi_hw->vif_table[vif_idx]->m2u_3addr_resend = value;
			else if (!strcasecmp(command, "log_enable"))
				cls_wifi_hw->vif_table[vif_idx]->log_enable = value;
			else if (!strcasecmp(command, "dump_pkt"))
				cls_wifi_hw->vif_table[vif_idx]->dump_pkt = value;
		}
	} else {
		pr_warn("Supported command: ifname + ");
		pr_warn("\tuse_4addr 0/1\n");
		pr_warn("\tauto_4addr 0/1\n");
		pr_warn("\tforce_4addr 0/1\n");
		pr_warn("\tm2u_3addr_resend 0/1\n");
		pr_warn("\tlog_enable 0/1\n");
		pr_warn("\tdump_pkt 0/1\n");
	}

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(command);

#ifdef CONFIG_CLS_WIFI_P2P_DEBUGFS
static ssize_t cls_wifi_dbgfs_oppps_write(struct file *file,
									  const char __user *user_buf,
									  size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *rw_hw = file->private_data;
	struct cls_wifi_vif *rw_vif;
	char buf[32];
	size_t len = min_t(size_t, count, sizeof(buf) - 1);
	int ctw;

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;
	buf[len] = '\0';

	/* Read the written CT Window (provided in ms) value */
	if (sscanf(buf, "ctw=%d", &ctw) > 0) {
		/* Check if at least one VIF is configured as P2P GO */
		list_for_each_entry(rw_vif, &rw_hw->vifs, list) {
			if (CLS_WIFI_VIF_TYPE(rw_vif) == NL80211_IFTYPE_P2P_GO) {
				struct mm_set_p2p_oppps_cfm cfm;

				/* Forward request to the embedded and wait for confirmation */
				cls_wifi_send_p2p_oppps_req(rw_hw, rw_vif, (u8)ctw, &cfm);

				break;
			}
		}
	}

	return count;
}

DEBUGFS_WRITE_FILE_OPS(oppps);

static ssize_t cls_wifi_dbgfs_noa_write(struct file *file,
									const char __user *user_buf,
									size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *rw_hw = file->private_data;
	struct cls_wifi_vif *rw_vif;
	char buf[64];
	size_t len = min_t(size_t, count, sizeof(buf) - 1);
	int noa_count, interval, duration, dyn_noa;

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;
	buf[len] = '\0';

	/* Read the written NOA information */
	if (sscanf(buf, "count=%d interval=%d duration=%d dyn=%d",
			   &noa_count, &interval, &duration, &dyn_noa) > 0) {
		/* Check if at least one VIF is configured as P2P GO */
		list_for_each_entry(rw_vif, &rw_hw->vifs, list) {
			if (CLS_WIFI_VIF_TYPE(rw_vif) == NL80211_IFTYPE_P2P_GO) {
				struct mm_set_p2p_noa_cfm cfm;

				/* Forward request to the embedded and wait for confirmation */
				cls_wifi_send_p2p_noa_req(rw_hw, rw_vif, noa_count, interval,
									  duration, (dyn_noa > 0),  &cfm);

				break;
			}
		}
	}

	return count;
}

DEBUGFS_WRITE_FILE_OPS(noa);
#endif /* CONFIG_CLS_WIFI_P2P_DEBUGFS */

struct cls_wifi_dbgfs_fw_trace {
	struct cls_wifi_fw_trace_local_buf lbuf;
	struct cls_wifi_fw_trace *trace;
	struct cls_wifi_hw *cls_wifi_hw;
};

static int cls_wifi_dbgfs_fw_trace_open(struct inode *inode, struct file *file)
{
	struct cls_wifi_dbgfs_fw_trace *ltrace = kmalloc(sizeof(*ltrace), GFP_KERNEL);
	struct cls_wifi_hw *priv = inode->i_private;

	if (!ltrace)
		return -ENOMEM;

	if (cls_wifi_fw_trace_alloc_local(&ltrace->lbuf, 5120)) {
		kfree(ltrace);
	}

	ltrace->trace = &priv->debugfs.fw_trace;
	ltrace->cls_wifi_hw = priv;
	file->private_data = ltrace;
	return 0;
}

static int cls_wifi_dbgfs_fw_trace_release(struct inode *inode, struct file *file)
{
	struct cls_wifi_dbgfs_fw_trace *ltrace = file->private_data;

	if (ltrace) {
		cls_wifi_fw_trace_free_local(&ltrace->lbuf);
		kfree(ltrace);
	}

	return 0;
}

static ssize_t cls_wifi_dbgfs_fw_trace_read(struct file *file,
										char __user *user_buf,
										size_t count, loff_t *ppos)
{
	struct cls_wifi_dbgfs_fw_trace *ltrace = file->private_data;
	bool dont_wait = ((file->f_flags & O_NONBLOCK) ||
					  ltrace->cls_wifi_hw->debugfs.unregistering);

	return cls_wifi_fw_trace_read(ltrace->trace, &ltrace->lbuf,
							  dont_wait, user_buf, count);
}

static ssize_t cls_wifi_dbgfs_fw_trace_write(struct file *file,
										 const char __user *user_buf,
										 size_t count, loff_t *ppos)
{
	struct cls_wifi_dbgfs_fw_trace *ltrace = file->private_data;
	int ret;

	ret = _cls_wifi_fw_trace_reset(ltrace->trace, true);
	if (ret)
		return ret;

	return count;
}

DEBUGFS_READ_WRITE_OPEN_RELEASE_FILE_OPS(fw_trace);

static ssize_t cls_wifi_dbgfs_fw_trace_level_read(struct file *file,
											  char __user *user_buf,
											  size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	return cls_wifi_fw_trace_level_read(&priv->debugfs.fw_trace, user_buf,
									count, ppos);
}

static ssize_t cls_wifi_dbgfs_fw_trace_level_write(struct file *file,
											   const char __user *user_buf,
											   size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	return cls_wifi_fw_trace_level_write(&priv->debugfs.fw_trace, user_buf, count);
}
DEBUGFS_READ_WRITE_FILE_OPS(fw_trace_level);


#ifdef CONFIG_CLS_WIFI_RADAR
static ssize_t cls_wifi_dbgfs_pulses_read(struct file *file,
									  char __user *user_buf,
									  size_t count, loff_t *ppos,
									  int rd_idx)
{
	struct cls_wifi_hw *priv = file->private_data;
	char *buf;
	int len = 0;
	int bufsz;
	ssize_t read;

	if (*ppos != 0)
		return 0;

	/* Prevent from interrupt preemption */
	spin_lock_bh(&priv->radar.lock);
	bufsz = 51;
	bufsz += cls_wifi_radar_dump_pattern_detector(NULL, 0, &priv->radar, rd_idx);
	buf = kmalloc(bufsz, GFP_ATOMIC);
	if (buf == NULL) {
		spin_unlock_bh(&priv->radar.lock);
		return 0;
	}

	len += cls_wifi_radar_dump_pattern_detector(&buf[len], bufsz - len,
											&priv->radar, rd_idx);

	spin_unlock_bh(&priv->radar.lock);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, len);

	kfree(buf);

	return read;
}

static ssize_t cls_wifi_dbgfs_pulses_prim_read(struct file *file,
										   char __user *user_buf,
										   size_t count, loff_t *ppos)
{
	return cls_wifi_dbgfs_pulses_read(file, user_buf, count, ppos, 0);
}

DEBUGFS_READ_FILE_OPS(pulses_prim);

static ssize_t cls_wifi_dbgfs_pulses_sec_read(struct file *file,
										  char __user *user_buf,
										  size_t count, loff_t *ppos)
{
	return cls_wifi_dbgfs_pulses_read(file, user_buf, count, ppos, 1);
}

DEBUGFS_READ_FILE_OPS(pulses_sec);

static ssize_t cls_wifi_dbgfs_detected_read(struct file *file,
										char __user *user_buf,
										size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char *buf;
	int bufsz,len = 0;
	ssize_t read;

	if (*ppos != 0)
		return 0;

	bufsz = 5; // RIU:\n
	bufsz += cls_wifi_radar_dump_radar_detected(NULL, 0, &priv->radar,
											CLS_WIFI_RADAR_RIU);

	if (priv->phy.cnt > 1) {
		bufsz += 5; // FCU:\n
		bufsz += cls_wifi_radar_dump_radar_detected(NULL, 0, &priv->radar,
												CLS_WIFI_RADAR_FCU);
	}

	buf = kmalloc(bufsz, GFP_KERNEL);
	if (buf == NULL) {
		return 0;
	}

	len = scnprintf(&buf[len], bufsz, "RIU:\n");
	len += cls_wifi_radar_dump_radar_detected(&buf[len], bufsz - len, &priv->radar,
											CLS_WIFI_RADAR_RIU);

	if (priv->phy.cnt > 1) {
		len += scnprintf(&buf[len], bufsz - len, "FCU:\n");
		len += cls_wifi_radar_dump_radar_detected(&buf[len], bufsz - len,
											  &priv->radar, CLS_WIFI_RADAR_FCU);
	}

	read = simple_read_from_buffer(user_buf, count, ppos, buf, len);

	kfree(buf);

	return read;
}

DEBUGFS_READ_FILE_OPS(detected);

static ssize_t cls_wifi_dbgfs_enable_read(struct file *file,
									char __user *user_buf,
									size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"%d\n", priv->radar.dpd->enabled);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_enable_write(struct file *file,
									 const char __user *user_buf,
									 size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		cls_wifi_radar_detection_enable(&priv->radar, val, CLS_WIFI_RADAR_RIU);

	priv->radio_params->dfs_enable = val ? true : false;

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(enable);

static ssize_t cls_wifi_dbgfs_band_read(struct file *file,
									char __user *user_buf,
									size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"BAND=%d\n", priv->phy.sec_chan.band);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_band_write(struct file *file,
									 const char __user *user_buf,
									 size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if ((sscanf(buf, "%d", &val) > 0) && (val >= 0) && (val <= NL80211_BAND_5GHZ))
		priv->phy.sec_chan.band = val;

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(band);

static ssize_t cls_wifi_dbgfs_type_read(struct file *file,
									char __user *user_buf,
									size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"TYPE=%d\n", priv->phy.sec_chan.type);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_type_write(struct file *file,
									 const char __user *user_buf,
									 size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if ((sscanf(buf, "%d", &val) > 0) && (val >= PHY_CHNL_BW_20) &&
		(val <= PHY_CHNL_BW_80P80))
		priv->phy.sec_chan.type = val;

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(type);

static ssize_t cls_wifi_dbgfs_prim20_read(struct file *file,
									  char __user *user_buf,
									  size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"PRIM20=%dMHz\n", priv->phy.sec_chan.prim20_freq);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_prim20_write(struct file *file,
									   const char __user *user_buf,
									   size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		priv->phy.sec_chan.prim20_freq = val;

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(prim20);

static ssize_t cls_wifi_dbgfs_center1_read(struct file *file,
									   char __user *user_buf,
									   size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"CENTER1=%dMHz\n", priv->phy.sec_chan.center1_freq);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_center1_write(struct file *file,
										const char __user *user_buf,
										size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		priv->phy.sec_chan.center1_freq = val;

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(center1);

static ssize_t cls_wifi_dbgfs_center2_read(struct file *file,
									   char __user *user_buf,
									   size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"CENTER2=%dMHz\n", priv->phy.sec_chan.center2_freq);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_center2_write(struct file *file,
										const char __user *user_buf,
										size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		priv->phy.sec_chan.center2_freq = val;

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(center2);


static ssize_t cls_wifi_dbgfs_set_read(struct file *file,
								   char __user *user_buf,
								   size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t cls_wifi_dbgfs_set_write(struct file *file,
									const char __user *user_buf,
									size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;

	cls_wifi_send_set_channel(priv, 1, NULL);
	cls_wifi_radar_detection_enable(&priv->radar, CLS_WIFI_RADAR_DETECT_ENABLE,
								CLS_WIFI_RADAR_FCU);

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(set);

static ssize_t cls_wifi_dbgfs_rd_rebuild_thres_read(struct file *file,
								   char __user *user_buf,
								   size_t count, loff_t *ppos)
{
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"%u\n", g_radar_rebuild_thresh);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_rd_rebuild_thres_write(struct file *file,
									const char __user *user_buf,
									size_t count, loff_t *ppos)
{
	char buf[32];
	u32 val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%u", &val) > 0)
		g_radar_rebuild_thresh = val;

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(rd_rebuild_thres);

static ssize_t cls_wifi_dbgfs_rd_det_times_read(struct file *file,
								   char __user *user_buf,
								   size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"%d\n", priv->radar.detected.count);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_rd_det_times_write(struct file *file,
									const char __user *user_buf,
									size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		priv->radar.detected.count = val;

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(rd_det_times);

static ssize_t cls_wifi_dbgfs_rd_reset_write(struct file *file,
									const char __user *user_buf,
									size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0) {
		if (val > 0)
			dfs_pattern_detector_reset(priv->radar.dpd);
	}

	return count;
}

DEBUGFS_WRITE_FILE_OPS(rd_reset);

static ssize_t cls_wifi_dbgfs_rd_force_cac_read(struct file *file,
								   char __user *user_buf,
								   size_t count, loff_t *ppos)
{
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"%hhu\n", g_radar_force_cac);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_rd_force_cac_write(struct file *file,
									const char __user *user_buf,
									size_t count, loff_t *ppos)
{
	char buf[32];
	u32 val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%u", &val) > 0)
		g_radar_force_cac = val;

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(rd_force_cac);

static ssize_t cls_wifi_dbgfs_rd_sw_det_write(struct file *file,
									const char __user *user_buf,
									size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	//TODO: repeater will support radar detected in next phase
	if (cls_wifi_is_repeater_mode(priv))
		return -EFAULT;

	if (sscanf(buf, "%d", &val) > 0) {
		if (val > 0)
			cls_wifi_radar_detected(priv);
	}

	return count;
}

DEBUGFS_WRITE_FILE_OPS(rd_sw_det);

static ssize_t cls_wifi_dbgfs_rd_debug_read(struct file *file,
								   char __user *user_buf,
								   size_t count, loff_t *ppos)
{
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"%hhu\n", g_radar_debug);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_rd_debug_write(struct file *file,
									const char __user *user_buf,
									size_t count, loff_t *ppos)
{
	char buf[32];
	u32 val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);
	struct cls_wifi_hw *priv = file->private_data;

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (kstrtouint(buf, 0, &val) == 0) {
		cls_wifi_send_set_rd_dbg_req(priv, val);
		g_radar_debug = val;
	}

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(rd_debug);

static ssize_t cls_wifi_dbgfs_rd_ratio_read(struct file *file,
								   char __user *user_buf,
								   size_t count, loff_t *ppos)
{
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"%hhu\n", g_radar_ratio);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_rd_ratio_write(struct file *file,
									const char __user *user_buf,
									size_t count, loff_t *ppos)
{
	char buf[32];
	u32 val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (kstrtouint(buf, 0, &val) == 0)
		g_radar_ratio = val;

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(rd_ratio);

static ssize_t cls_wifi_dbgfs_rd_max_num_thrd_read(struct file *file,
								   char __user *user_buf,
								   size_t count, loff_t *ppos)
{
	char buf[32];
	int ret = -1;
	ssize_t read = 0;
	struct cls_wifi_hw *priv = file->private_data;
	struct mm_rd_max_num_thrd_cfm cfm;

	ret = cls_wifi_send_get_rd_max_num_thrd_req(priv, &cfm);
	if (ret == CO_OK && cfm.status == CO_OK) {
		ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
						"%hhu\n", cfm.rd_max_num_thrd);

		read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);
	}

	return read;
}

static ssize_t cls_wifi_dbgfs_rd_max_num_thrd_write(struct file *file,
									const char __user *user_buf,
									size_t count, loff_t *ppos)
{
	char buf[32];
	u32 val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);
	struct cls_wifi_hw *priv = file->private_data;

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (kstrtouint(buf, 0, &val) == 0)
		cls_wifi_send_set_rd_max_num_thrd_req(priv, val);

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(rd_max_num_thrd);

static ssize_t cls_wifi_dbgfs_rd_chan_read(struct file *file,
								   char __user *user_buf,
								   size_t count, loff_t *ppos)
{
	char buf[32];
	int ret = -1;
	ssize_t read = 0;
	struct cls_wifi_hw *priv = file->private_data;
	struct mm_rd_chan_cfm cfm;

	ret = cls_wifi_send_get_rd_channel_req(priv, &cfm);
	if (ret == CO_OK && cfm.status == CO_OK) {
		ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
						"%hhu\n", cfm.channel);

		read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);
	}

	return read;
}

static ssize_t cls_wifi_dbgfs_rd_chan_write(struct file *file,
									const char __user *user_buf,
									size_t count, loff_t *ppos)
{
	char buf[32];
	u32 val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);
	struct cls_wifi_hw *priv = file->private_data;

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (kstrtouint(buf, 0, &val) == 0)
		cls_wifi_send_set_rd_channel_req(priv, val);

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(rd_chan);

static ssize_t cls_wifi_dbgfs_rd_rebuild_num_thres_read(struct file *file,
								   char __user *user_buf,
								   size_t count, loff_t *ppos)
{
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"%hhu\n", g_radar_rebuild_num_thresh);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_rd_rebuild_num_thres_write(struct file *file,
									const char __user *user_buf,
									size_t count, loff_t *ppos)
{
	char buf[32];
	u32 val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (kstrtouint(buf, 0, &val) == 0)
		g_radar_rebuild_num_thresh = val;

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(rd_rebuild_num_thres);

static ssize_t cls_wifi_dbgfs_rd_min_num_det_read(struct file *file,
								   char __user *user_buf,
								   size_t count, loff_t *ppos)
{
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"%hhu\n", g_radar_min_num_det);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_rd_min_num_det_write(struct file *file,
									const char __user *user_buf,
									size_t count, loff_t *ppos)
{
	char buf[32];
	u32 val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (kstrtouint(buf, 0, &val) == 0)
		g_radar_min_num_det = val;

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(rd_min_num_det);

static ssize_t cls_wifi_dbgfs_pulses_hist_read(struct file *file,
										   char __user *user_buf,
										   size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char *buf;
	int len = 0;
	int bufsz;
	ssize_t read;
	struct cls_wifi_radar_pulses *p = &priv->radar.pulses;
	int i,j;
	int index;

	if (*ppos != 0)
		return 0;

	/* Prevent from interrupt preemption */
	spin_lock_bh(&priv->radar.lock);
	bufsz = 60 + 180 * CLS_WIFI_RADAR_PULSE_MAX;
	buf = kmalloc(bufsz, GFP_ATOMIC);
	if (buf == NULL) {
		spin_unlock_bh(&priv->radar.lock);
		return 0;
	}

	len += scnprintf(&buf[len], bufsz - len,
									"PRI	WIDTH	 FREQ_S    FREQ_E   TIMEOUT_FLAG\n");
	if (p->index > 0)
		index = p->index - 1;
	else
		index = CLS_WIFI_RADAR_PULSE_MAX - 1;

	for (i = 0; i < CLS_WIFI_RADAR_PULSE_MAX; i++) {
		struct radar_pulse_array_desc *pulses = &p->buffer[index];

		if (!pulses->cnt)
			break;

		len += scnprintf(&buf[len], bufsz - len, "------------------------------------------\n");
		if (index > 0)
			index--;
		else
			index = CLS_WIFI_RADAR_PULSE_MAX - 1;

		for (j = 0; j < pulses->cnt; j++)
			len += scnprintf(&buf[len], bufsz - len,
										"%-7d  %-8d  %-8d  %-6d  %d\n",
										PRI_ROUND(pulses->pulse[j].pri),
										pulses->pulse[j].width,
										FREQ_ROUND(pulses->pulse[j].freq_start),
										FREQ_ROUND(pulses->pulse[j].freq_end),
										pulses->pulse[j].timeout);
	}

	spin_unlock_bh(&priv->radar.lock);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, len);

	kfree(buf);

	return read;
}

DEBUGFS_READ_FILE_OPS(pulses_hist);

static ssize_t cls_wifi_dbgfs_rd_agc_war_read(struct file *file,
								   char __user *user_buf,
								   size_t count, loff_t *ppos)
{
	char buf[32];
	int ret = -1;
	ssize_t read = 0;
	struct cls_wifi_hw *priv = file->private_data;
	struct mm_rd_agc_war_cfm cfm;

	ret = cls_wifi_send_get_rd_agc_war_req(priv, &cfm);
	if (ret == CO_OK && cfm.status == CO_OK) {
		ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
						"%hhu\n", cfm.enable);

		read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);
	}

	return read;
}

static ssize_t cls_wifi_dbgfs_rd_agc_war_write(struct file *file,
									const char __user *user_buf,
									size_t count, loff_t *ppos)
{
	char buf[32];
	u32 val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);
	struct cls_wifi_hw *priv = file->private_data;

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (kstrtouint(buf, 0, &val) == 0)
		cls_wifi_send_set_rd_agc_war_req(priv, val);

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(rd_agc_war);

#endif /* CONFIG_CLS_WIFI_RADAR */

#ifdef CONFIG_CLS_WIFI_HEMU_TX
static ssize_t cls_wifi_dbgfs_ul_duration_max_read(struct file *file,
											   char __user *user_buf,
											   size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"UL_DURATION_MAX=%dus\n", priv->ul_params.ul_duration_max);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_ul_duration_max_write(struct file *file,
												const char __user *user_buf,
												size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		priv->ul_params.ul_duration_max = val;

	cls_wifi_send_mm_ul_parameters(priv);

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(ul_duration_max);

static ssize_t cls_wifi_dbgfs_nss_max_read(struct file *file,
									   char __user *user_buf,
									   size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"NSS_MAX=%d\n", priv->ul_params.nss_max);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_nss_max_write(struct file *file,
										const char __user *user_buf,
										size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		priv->ul_params.nss_max = val;

	cls_wifi_send_mm_ul_parameters(priv);

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(nss_max);

static ssize_t cls_wifi_dbgfs_mcs_read(struct file *file,
								   char __user *user_buf,
								   size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"MCS=%d\n", priv->ul_params.mcs);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_mcs_write(struct file *file,
									const char __user *user_buf,
									size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		priv->ul_params.mcs = val;

	cls_wifi_send_mm_ul_parameters(priv);

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(mcs);

static ssize_t cls_wifi_dbgfs_fec_allowed_read(struct file *file,
										   char __user *user_buf,
										   size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"FEC_ALLOWED=%d\n", priv->ul_params.fec_allowed);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_fec_allowed_write(struct file *file,
											const char __user *user_buf,
											size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		priv->ul_params.fec_allowed = val;

	cls_wifi_send_mm_ul_parameters(priv);

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(fec_allowed);

static ssize_t cls_wifi_dbgfs_gi_ltf_mode_read(struct file *file,
								   char __user *user_buf,
								   size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"GI_LTF=%d\n", priv->ul_params.gi_ltf_mode);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_gi_ltf_mode_write(struct file *file,
									const char __user *user_buf,
									size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		priv->ul_params.gi_ltf_mode = val;

	cls_wifi_send_mm_ul_parameters(priv);

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(gi_ltf_mode);


#define FORMAT_HEX	0
#define FORMAT_BIN 	1
#define FORMAT_HEX16	2
#define ENDIAN_LITTLE	0
#define ENDIAN_BIG	1

int read_memdata_from_file(struct cls_wifi_hw *cls_wifi_hw, char *filename,
				 int format, int endian, uint32_t addr)
{
	struct file *fp = NULL;
	int len = 0;
	int i = 0;
	int j = 0;
	char tmp_str[10] = { 0 };
	loff_t pos = 0;
	uint8_t *buf;
	ssize_t bytes_read;
	uint8_t tmp;

	fp = filp_open(filename, O_RDWR, 0644);

	if (IS_ERR(fp)) {
		pr_err("%s: open file failed\n", __func__);
		return 0;
	}

	len = fp->f_inode->i_size;
	buf = kzalloc(len, GFP_KERNEL);
	if (!buf) {
		filp_close(fp, NULL);
		return 0;
	}
	do {
		bytes_read = kernel_read(fp, tmp_str, sizeof(tmp_str), &pos);
		for (i = 0; i < bytes_read; i++) {
			if (tmp_str[i] == '\n' || tmp_str[i] == '\r')
				 continue;
			buf[j++] = tmp_str[i];

		}
	} while (bytes_read > 0);

	if (format == FORMAT_HEX) {//hex

		for (i = 0; i < j/2; i++) {
			memset(tmp_str, 0, sizeof(tmp_str));
			memcpy(tmp_str, buf + 2 * i, 2);
			buf[i] = simple_strtoul(tmp_str, NULL, 16) & 0xff;
		}
		if (endian == ENDIAN_LITTLE) {
			for (j = 0; j < i/4; j++) {
				tmp = buf[4 * j];
				buf[4 * j] = buf[4 * j + 3];
				buf[4 * j + 3] = tmp;
				tmp = buf[4 * j + 1];
				buf[4 * j + 1] = buf[4 * j + 2];
				buf[4 * j + 2] = tmp;
			}
		}
	} else {//bin
		if (endian == ENDIAN_LITTLE) {
			i = j;
		} else if (endian == ENDIAN_BIG) {
			i = j;
			for (j = 0; j < i/4; j++) {
				tmp = buf[4 * j];
				buf[4 * j] = buf[4 * j + 3];
				buf[4 * j + 3] = tmp;
				tmp = buf[4 * j + 1];
				buf[4 * j + 1] = buf[4 * j + 2];
				buf[4 * j + 2] = tmp;
			}
		}
	}
	i -= i % 4;
	pr_err("%s, i= %d\n", __func__, i);

	cls_wifi_mem_ops(cls_wifi_hw, MM_MEM_OP_WRITE, 1, addr, (void *)buf, i);
	kfree(buf);
	filp_close(fp, NULL);
	return i;
}

int write_memdata_to_file(const char *filename, int format, int endian, int len,
				uint32_t *data, uint32_t addr, int print_addr)
{
	struct file *fp = NULL;
	int i = 0;
	char tmp_str[32] = { 0 };
	uint32_t tmp;

	pr_err("%s,file_name =%s\n", __func__, filename);

	fp = filp_open(filename, O_RDWR | O_CREAT | O_TRUNC, 0666);
	if (IS_ERR(fp))
		return -EPERM;

	for (i = 0; i < len; i++) {
		if (format == FORMAT_HEX) {
			if (endian == ENDIAN_LITTLE) {
				if (print_addr)
					sprintf(tmp_str, "%08x %08x\n", addr + i * 4, *(data + i));
				else
					sprintf(tmp_str, "%08x\n", *(data + i));
			} else {
				if (print_addr)
					sprintf(tmp_str, "%08x %08x\n", addr + i * 4,
							__cpu_to_be32(*(data + i)));
				else
					sprintf(tmp_str, "%08x\n", __cpu_to_be32(*(data + i)));
			}
			__kernel_write(fp, tmp_str, strlen(tmp_str), &fp->f_pos);
		} else if (format == FORMAT_BIN) {
			if (endian == ENDIAN_LITTLE)
				tmp = *(data + i);
			else
				tmp = __cpu_to_be32(*(data + i));
			__kernel_write(fp, (uint8_t *)&tmp, sizeof(uint32_t), &fp->f_pos);
		} else {
			if (endian == ENDIAN_LITTLE)
				sprintf(tmp_str, "%04x\n%04x\n", *(data + i) & 0xffff,
						(*(data + i) & 0xffff0000) >> 16);
			else
				sprintf(tmp_str, "%04x\n%04x\n",
						(__cpu_to_be32(*(data + i)) & 0xffff0000) >> 16,
						__cpu_to_be32(*(data + i)) & 0xffff);
			__kernel_write(fp, tmp_str, strlen(tmp_str), &fp->f_pos);
		}
	}

	filp_close(fp, NULL);

	return 0;
}

void print_memdata(int format, int endian, int len, uint32_t *data, uint32_t addr, int print_addr)
{
	int i;

	for (i = 0; i < len; i++) {
		if (format == FORMAT_HEX || format == FORMAT_BIN) {
			if (endian == ENDIAN_LITTLE) {
				if (print_addr)
					pr_err("%08x %08x\n", addr + i * 4, *(data + i));
				else
					pr_err("%08x\n", *(data + i));
			} else {
				if (print_addr)
					pr_err("%08x %08x\n", addr + i * 4,
							__cpu_to_be32(*(data + i)));
				else
					pr_err("%08x\n", __cpu_to_be32(*(data + i)));
			}

		} else {
			if (endian == ENDIAN_LITTLE) {
				pr_err("%04x\n", *(data + i) & 0xffff);
				pr_err("%04x\n", (*(data + i) & 0xffff0000) >> 16);
			} else {
				pr_err("%04x\n", (__cpu_to_be32(*(data + i)) & 0xffff0000) >> 16);
				pr_err("%04x\n", __cpu_to_be32(*(data + i)) & 0xffff);
			}
		}
	}
}

int cls_wifi_load_mem_data(struct cls_wifi_hw * cls_wifi_hw, uint32_t addr,
				char *file_name, int format, int endian)
{
	int len = 0;

	len = read_memdata_from_file(cls_wifi_hw, file_name, format, endian, addr);
	return len;
}

int cls_wifi_dump_mem_data(struct cls_wifi_hw * cls_wifi_hw, uint32_t addr, uint32_t len,
					uint32_t format, uint32_t endian,
					const char *filename, int print_addr)
{
	uint32_t *mem_data;

	mem_data = (uint32_t *)kzalloc(len * sizeof(uint32_t), GFP_KERNEL);
	if (!mem_data)
		return 0;

	cls_wifi_mem_ops(cls_wifi_hw, MM_MEM_OP_READ, 1, addr,
				(void *)mem_data, len * sizeof(uint32_t));

	if (!filename[0])
		print_memdata(format, endian, len, mem_data, addr, print_addr);
	else
		write_memdata_to_file(filename, format, endian, len, mem_data, addr, print_addr);
	kfree(mem_data);
	return 0;
}

static ssize_t cls_wifi_dbgfs_mem_read(struct file *file,
						char __user *user_buf,
						size_t count, loff_t *ppos)
{
	char buf[32];
	ssize_t read;
	int ret = 0;

	memset(buf, 0, sizeof(buf));

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_mem_write(struct file *file,
							const char __user *user_buf,
							size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[128];
	char file_name[64] = {0};
	int addr;
	int format = FORMAT_HEX;
	int endian = 0;
	char format_str[8];
	char cmd[8];
	int num = 0;
	char *pend = NULL;
	int pr_addr = 0;

	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	pend = buf + len - 1;
	while ((*pend == ' ') || (*pend == '\n')) {
		*pend = 0;
		len--;
		pend--;
	}

	memset(file_name, 0, sizeof(file_name));
	memset(format_str, 0, sizeof(format_str));

	if (!strncmp(buf, "load", strlen("load"))) {
		if (sscanf(buf, "%s %x %s %s %d", cmd, &addr, format_str, file_name, &endian) > 0) {

			if (!strcmp(format_str, "hex"))
				format = FORMAT_HEX;
			else if (!strcmp(format_str, "bin"))
				format = FORMAT_BIN;
			else
				pr_err("mem load invalid file format");

			pr_err("%s : load addr=0x%8x, format =%d, endian=%d, filename=%s",
				__func__, addr, format, endian, file_name);

			if (format == FORMAT_HEX || format == FORMAT_BIN)
				cls_wifi_load_mem_data(priv, addr, file_name, format, endian);
		}

	} else if (!strncmp(buf, "dump", strlen("dump"))) {
		if (sscanf(buf, "%s %x %d %s %d %d %s", cmd, &addr, &num, format_str,
			&endian, &pr_addr, file_name) > 0) {

			if (!strcmp(format_str, "hex"))
				format = FORMAT_HEX;
			else if (!strcmp(format_str, "bin"))
				format = FORMAT_BIN;
			else if (!strcmp(format_str, "hex16"))
				format = FORMAT_HEX16;
			else
				pr_err("mem dump invalid file format");

			pr_err("%s : dump addr = 0x%8x, mem_data_len = %d,"
				" format = %d, endian = %d, filename = %s\n",
				__func__, addr, num, format, endian, file_name);

			cls_wifi_dump_mem_data(priv, addr, num,
						format, endian, file_name, pr_addr);
		}
	} else {
		;
	}

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(mem);

static ssize_t cls_wifi_dbgfs_tsensor_read(struct file *file,
						char __user *user_buf,
						size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int temp, ret;

	if (priv->plat->ep_ops->tsensor_get == NULL) {
		ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count), "Not supported.\n");
		return simple_read_from_buffer(user_buf, count, ppos, buf, ret);
	}

	temp = priv->plat->ep_ops->tsensor_get(priv->plat);
	if (temp < 0)
		ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
				"TS0 Temperature[-%d.%d C]\n",
				(temp * -1) / 1000, (temp * -1) % 1000);
	else
		ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
				"TS0 Temperature[%d.%d C]\n",
				temp / 1000, temp % 1000);

	return simple_read_from_buffer(user_buf, count, ppos, buf, ret);
}
DEBUGFS_READ_FILE_OPS(tsensor);

static ssize_t cls_wifi_dbgfs_ul_duration_force_read(struct file *file,
												 char __user *user_buf,
												 size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"UL_DURATION_FORCE=%d\n", priv->ul_params.ul_duration_force);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_ul_duration_force_write(struct file *file,
												  const char __user *user_buf,
												  size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		priv->ul_params.ul_duration_force = val;

	cls_wifi_send_mm_ul_parameters(priv);

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(ul_duration_force);


static ssize_t cls_wifi_dbgfs_ul_duration_read(struct file *file,
						char __user *user_buf,
						size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
		"UL_duration=%d\n", priv->ul_params.ul_duration);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_ul_duration_write(struct file *file,
						const char __user *user_buf,
						size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		priv->ul_params.ul_duration = val;

	cls_wifi_send_mm_ul_parameters(priv);

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(ul_duration);

static ssize_t cls_wifi_dbgfs_tf_period_read(struct file *file,
			char __user *user_buf,
			size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
		"UL-tf_period=%d\n", priv->ul_params.tf_period);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_tf_period_write(struct file *file,
			const char __user *user_buf,
			size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		priv->ul_params.tf_period = val;

	cls_wifi_send_mm_ul_parameters(priv);

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(tf_period);

static ssize_t cls_wifi_dbgfs_ul_user_num_read(struct file *file,
			char __user *user_buf,
			size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
		"UL_user_num=%d\n", priv->ul_params.user_num);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_ul_user_num_write(struct file *file,
			const char __user *user_buf,
			size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		priv->ul_params.user_num = val;

	cls_wifi_send_mm_ul_parameters(priv);

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(ul_user_num);

static ssize_t cls_wifi_dbgfs_ul_on_read(struct file *file,
									 char __user *user_buf,
									 size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"UL_ON=%d\n", priv->ul_params.ul_on);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_ul_on_write(struct file *file,
									  const char __user *user_buf,
									  size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		priv->ul_params.ul_on = val;

	cls_wifi_send_mm_ul_parameters(priv);

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(ul_on);

static ssize_t cls_wifi_dbgfs_ul_dbg_level_read(struct file *file,
						char __user *user_buf,
						size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
		"UL_dbg_level=0x%08x", priv->ul_params.ul_dbg_level);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_ul_dbg_level_write(struct file *file,
							const char __user *user_buf,
							size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		priv->ul_params.ul_dbg_level = val;

	cls_wifi_send_mm_ul_parameters(priv);

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(ul_dbg_level);

static ssize_t cls_wifi_dbgfs_ul_bw_read(struct file *file,
						char __user *user_buf,
						size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
			"UL_bw=%d\n", priv->ul_params.ul_bw);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_ul_bw_write(struct file *file,
						const char __user *user_buf,
						size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		priv->ul_params.ul_bw = val;

	cls_wifi_send_mm_ul_parameters(priv);

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(ul_bw);

static ssize_t cls_wifi_dbgfs_ul_param_read(struct file *file,
						char __user *user_buf,
						size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[256];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
			"ul-on %u ul-duration %u tf-period %u user-num %u ul-bw %u "
			"nss-max %u mcs %u sched-mode %u work-mode %u gi-ltf-mode %u\n",
			priv->ul_params.ul_on,
			priv->ul_params.ul_duration,
			priv->ul_params.tf_period,
			priv->ul_params.user_num,
			priv->ul_params.ul_bw,
			priv->ul_params.nss_max,
			priv->ul_params.mcs,
			priv->ul_params.sched_mode,
			priv->ul_params.work_mode,
			priv->ul_params.gi_ltf_mode);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_ul_param_write(struct file *file,
						const char __user *user_buf,
						size_t count, loff_t *ppos)
{
	char *tb_cmd_name[] = {"ul-on", "ul-duration", "tf-period", "user-num",
			"ul-bw", "nss-max","mcs", "sched-mode", "work-mode"};
	struct cls_wifi_hw *priv = file->private_data;
	char *tb_cmd = NULL;
	char buf[256];
	size_t len;
	int value;
	int cmd;
	enum {
		UL_CMD_ON,
		UL_CMD_DURATION,
		UL_CMD_TF_PERIOD,
		UL_CMD_USER_NUM,
		UL_CMD_BW,
		UL_CMD_NSS_MAX,
		UL_CMD_MCS,
		UL_CMD_SCHED_MODE,
		UL_CMD_WORK_MODE,
		UL_CMD_MAX,
	};

	len = min_t(size_t, count, sizeof(buf) - 1);
	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';
	for (cmd = 0 ; cmd < ARRAY_SIZE(tb_cmd_name); cmd++) {
		if (!strncmp(buf, tb_cmd_name[cmd], strlen(tb_cmd_name[cmd]))) {
			tb_cmd = tb_cmd_name[cmd];
			break;
		}
	}

	if (tb_cmd == NULL)
		return count;

	if (cmd == UL_CMD_ON) {
		if (sscanf(buf, "ul-on %d", &value) > 0)
			priv->ul_params.ul_on = value;
	} else if (cmd == UL_CMD_DURATION) {
		if (sscanf(buf, "ul-duration %d", &value) > 0)
			priv->ul_params.ul_duration = value;
	} else if (cmd == UL_CMD_TF_PERIOD) {
		if (sscanf(buf, "tf-period %d", &value) > 0)
			priv->ul_params.ul_duration = value;
	} else if (cmd == UL_CMD_USER_NUM) {
		if (sscanf(buf, "user-num %d", &value) > 0)
			priv->ul_params.user_num = value;
	} else if (cmd == UL_CMD_BW) {
		if (sscanf(buf, "ul-bw %d", &value) > 0)
			priv->ul_params.ul_bw = value;
	} else if (cmd == UL_CMD_MCS) {
		if (sscanf(buf, "mcs %d", &value) > 0)
			priv->ul_params.mcs = value;
	} else if (cmd == UL_CMD_NSS_MAX) {
		if (sscanf(buf, "nss-max %d", &value) > 0)
			priv->ul_params.nss_max = value;
	} else if (cmd == UL_CMD_SCHED_MODE) {
		if (sscanf(buf, "sched-mode %d", &value) > 0)
			priv->ul_params.sched_mode = value;
	}  else if (cmd == UL_CMD_WORK_MODE) {
		if (sscanf(buf, "work-mode %d", &value) > 0)
			priv->ul_params.work_mode = value;
	}

	cls_wifi_send_mm_ul_parameters(priv);

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(ul_param);


static ssize_t cls_wifi_dbgfs_dl_nss_max_read(struct file *file,
										  char __user *user_buf,
										  size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"NSS_MAX=%d\n", priv->dl_params.nss_max);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_dl_nss_max_write(struct file *file,
										   const char __user *user_buf,
										   size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		priv->dl_params.nss_max = val;

	cls_wifi_send_mm_dl_parameters(priv);

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(dl_nss_max);

static ssize_t cls_wifi_dbgfs_dl_mcs_read(struct file *file,
									  char __user *user_buf,
									  size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"MCS=%d\n", priv->dl_params.mcs);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_dl_mcs_write(struct file *file,
									   const char __user *user_buf,
									   size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		priv->dl_params.mcs = val;

	cls_wifi_send_mm_dl_parameters(priv);

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(dl_mcs);

static ssize_t cls_wifi_dbgfs_dl_fec_allowed_read(struct file *file,
										   char __user *user_buf,
										   size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"FEC_ALLOWED=%d\n", priv->dl_params.fec_allowed);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_dl_fec_allowed_write(struct file *file,
											   const char __user *user_buf,
											   size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		priv->dl_params.fec_allowed = val;

	cls_wifi_send_mm_dl_parameters(priv);

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(dl_fec_allowed);

static ssize_t cls_wifi_dbgfs_dl_on_read(struct file *file,
									 char __user *user_buf,
									 size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"DL_ON=%d\n", priv->dl_params.dl_on);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_dl_on_write(struct file *file,
									  const char __user *user_buf,
									  size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		priv->dl_params.dl_on = val;

	cls_wifi_send_mm_dl_parameters(priv);

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(dl_on);

static ssize_t cls_wifi_dbgfs_work_mode_read(struct file *file,
		char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
			"WorkMode=%d\n", priv->dl_params.work_mode);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_work_mode_write(struct file *file,
		const char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		priv->dl_params.work_mode = val;

	cls_wifi_send_mm_dl_parameters(priv);

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(work_mode);

static ssize_t cls_wifi_dbgfs_mu_type_read(struct file *file,
		char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
			"MU type=%d\n", priv->dl_params.mu_type);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_mu_type_write(struct file *file,
		const char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		priv->dl_params.mu_type = val;

	cls_wifi_send_mm_dl_parameters(priv);

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(mu_type);

static ssize_t cls_wifi_dbgfs_ppdu_bw_read(struct file *file,
		char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
			"PPDU bw=%d\n", priv->dl_params.ppdu_bw);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_ppdu_bw_write(struct file *file,
		const char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		priv->dl_params.ppdu_bw = val;

	cls_wifi_send_mm_dl_parameters(priv);

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(ppdu_bw);

static ssize_t cls_wifi_dbgfs_gi_read(struct file *file,
		char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
			"gi=%d\n", priv->dl_params.gi);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_gi_write(struct file *file,
		const char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		priv->dl_params.gi = val;

	cls_wifi_send_mm_dl_parameters(priv);

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(gi);

static ssize_t cls_wifi_dbgfs_ltf_type_read(struct file *file,
		char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
			"ltf type=%d\n", priv->dl_params.ltf_type);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}


static ssize_t cls_wifi_dbgfs_ltf_type_write(struct file *file,
		const char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		priv->dl_params.ltf_type = val;

	cls_wifi_send_mm_dl_parameters(priv);

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(ltf_type);


static ssize_t cls_wifi_dbgfs_num_users_read(struct file *file,
		char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
			"user num=%d\n", priv->dl_params.user_num);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_num_users_write(struct file *file,
		const char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		priv->dl_params.user_num = val;

	cls_wifi_send_mm_dl_parameters(priv);

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(num_users);

static ssize_t cls_wifi_dbgfs_max_ampdu_subfrm_read(struct file *file,
		char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
			"user num=%d\n", priv->dl_params.max_ampdu_subfrm);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_max_ampdu_subfrm_write(struct file *file,
		const char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		priv->dl_params.max_ampdu_subfrm = val;

	cls_wifi_send_mm_dl_parameters(priv);

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(max_ampdu_subfrm);

static ssize_t cls_wifi_dbgfs_mu_log_level_read(struct file *file,
		char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
			"log level=%d\n", priv->dl_params.log_level);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_mu_log_level_write(struct file *file,
		const char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		priv->dl_params.log_level = val;

	cls_wifi_send_mm_dl_parameters(priv);

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(mu_log_level);

static ssize_t cls_wifi_dbgfs_mu_pkt_len_read(struct file *file,
		char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
			"pkt len threshold=%d\n", priv->dl_params.pkt_len_threshold);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_mu_pkt_len_write(struct file *file,
		const char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		priv->dl_params.pkt_len_threshold = val;

	cls_wifi_send_mm_dl_parameters(priv);

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(mu_pkt_len);


#endif /* CONFIG_CLS_WIFI_HEMU_TX */

static int cls_wifi_dbgfs_irf_str2val(char *str, char **end_ptr)
{
	while (*str <= ' ' && *str > 0)
		++str;

	if (*str == 0)
		return 0;

	if (('0' == *str) && (('x' == *(str + 1)) || ('X' == *(str + 1)))) {
		return simple_strtoul(str, end_ptr, 16);
	}

	return simple_strtol(str, end_ptr, 10);
}

static ssize_t cls_wifi_dbgfs_irf_get_reg_write(struct file *file,
									   const char __user *user_buf,
									   size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	struct dbg_mem_read_cfm mem_read_cfm;
	char buf[32];
	char *end_buf = NULL;
	u32 mem_addr;
	int status;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	mem_addr = cls_wifi_dbgfs_irf_str2val(buf, &end_buf);

	if ((status = cls_wifi_send_dbg_mem_read_req(priv, mem_addr, &mem_read_cfm)))
		printk("Send cmd ERROR status: %d\n", status);
	else
		printk("Get REG ADDR: 0x%08x VALUE: 0x%08x\n", mem_read_cfm.memaddr, mem_read_cfm.memdata);

	return count;
}

DEBUGFS_WRITE_FILE_OPS(irf_get_reg);

static ssize_t cls_wifi_dbgfs_irf_set_reg_write(struct file *file,
									   const char __user *user_buf,
									   size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	char *end_buf = NULL;
	u32 mem_addr;
	u32 mem_data;
	int status;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	mem_addr = cls_wifi_dbgfs_irf_str2val(buf, &end_buf);
	mem_data = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);

	if ((status = cls_wifi_send_dbg_mem_write_req(priv, mem_addr, mem_data)))
		printk("Send cmd ERROR status: %d\n", status);
	else
		printk("Set REG ADDR: 0x%08x VALUE: 0x%08x\n", mem_addr, mem_data);

	return count;
}

DEBUGFS_WRITE_FILE_OPS(irf_set_reg);

static ssize_t cls_wifi_dbgfs_irf_hw_init_write(struct file *file,
									   const char __user *user_buf,
									   size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	char *end_buf = NULL;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);
	struct irf_hw_init_req hw_init;
	struct irf_hw_init_cfm cfm;

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	hw_init.bw = cls_wifi_dbgfs_irf_str2val(buf, &end_buf);
	hw_init.bitmap = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);

	cls_wifi_send_irf_hw_init_req(priv, &hw_init, &cfm);
	if (cfm.status != CO_OK)
		pr_err("status: %d\n", cfm.status);

	return count;
}

DEBUGFS_WRITE_FILE_OPS(irf_hw_init);

static ssize_t cls_wifi_dbgfs_irf_smp_config_write(struct file *file,
									   const char __user *user_buf,
									   size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[64];
	char *end_buf = NULL;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);
	struct irf_smp_config_req smp_cfg;
	struct irf_smp_config_cfm cfm;
	u32 snd_smp_mod;
	u32 ddr_res_mem = 0;
	u32 data_len;

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	smp_cfg.node = cls_wifi_dbgfs_irf_str2val(buf, &end_buf);
	smp_cfg.mux_data = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	smp_cfg.mux_trg = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	smp_cfg.len = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	smp_cfg.width = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	smp_cfg.smp_trg = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	smp_cfg.interval = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	smp_cfg.trg_sel = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	smp_cfg.delay = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	snd_smp_mod = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	smp_cfg.update = 0;

	if (!snd_smp_mod)
		smp_cfg.smp_mod = IRF_COM_SMP_MOD;
	else
		smp_cfg.smp_mod = IRF_DDR_SMP_MOD;

	irf_set_snd_smp_mod(smp_cfg.node, smp_cfg.smp_mod);
	if (smp_cfg.smp_mod != IRF_DDR_SMP_MOD) {
		smp_cfg.irf_smp_buf_addr = priv->plat->if_ops->get_phy_address(priv->plat,
				priv->radio_idx, CLS_WIFI_ADDR_IRF_PHY, 0);
		/* Only for debug usage */
		printk("irf_smp_buf_addr: 0x%08x\n", smp_cfg.irf_smp_buf_addr);

		cls_wifi_send_irf_smp_config_req(priv, &smp_cfg, &cfm);
		if (cfm.status != CO_OK) {
			pr_err("status: %d\n", cfm.status);
			irf_set_snd_smp_mod(smp_cfg.node, IRF_DEF_SND_SMP_MOD);
		}
	} else {
		data_len = (1 << smp_cfg.width) * sizeof(u32);
		if (irf_smp_send_ram_malloc(priv->plat, smp_cfg.node, smp_cfg.smp_mod, smp_cfg.len * data_len, &ddr_res_mem)) {
			pr_err("%s : fail to get DDR memory for node %u\n", __func__, smp_cfg.node);
			return count;
		}
		smp_cfg.irf_smp_buf_addr = ddr_res_mem;
		/* Only for debug usage */
		printk("irf_smp_buf_addr: 0x%08x\n", smp_cfg.irf_smp_buf_addr);

		cls_wifi_send_irf_smp_config_req(priv, &smp_cfg, &cfm);
		if (cfm.status != CO_OK) {
			pr_err("status: %d\n", cfm.status);
			irf_smp_send_ram_free(priv->plat, CO_BIT(smp_cfg.node));
			irf_set_snd_smp_mod(smp_cfg.node, IRF_DEF_SND_SMP_MOD);
		}
	}

	return count;
}

DEBUGFS_WRITE_FILE_OPS(irf_smp_config);

static ssize_t cls_wifi_dbgfs_irf_smp_start_write(struct file *file,
									   const char __user *user_buf,
									   size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[64];
	char *end_buf = NULL;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);
	struct irf_smp_start_req smp_start;

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	smp_start.node_mask = cls_wifi_dbgfs_irf_str2val(buf, &end_buf);
	smp_start.timeout = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);

	cls_wifi_send_irf_smp_start_req(priv, &smp_start);

	return count;
}

DEBUGFS_WRITE_FILE_OPS(irf_smp_start);

static ssize_t cls_wifi_dbgfs_irf_send_config_write(struct file *file,
									   const char __user *user_buf,
									   size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	struct cls_wifi_plat *plat = priv->plat;
	char buf[192];
	char *end_buf = NULL;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);
	struct irf_send_config_req send_cfg;
	struct irf_send_config_cfm cfm;
	int ret;
	char src_file[128] = {0};
	char *token = NULL;
	u32 snd_smp_mod;
	u32 ddr_res_mem = 0;
	u32 res_mem_offset = 0;

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	if (buf[len - 1] == '\n')
		buf[len - 1] = '\0';
	else
		buf[len] = '\0';

	send_cfg.node = cls_wifi_dbgfs_irf_str2val(buf, &end_buf);
	send_cfg.interval = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	send_cfg.trg_sel = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	send_cfg.delay = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	send_cfg.len = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	send_cfg.loop = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	send_cfg.ppdu_front_len = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	send_cfg.ppdu_back_len = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	send_cfg.ppdu_gap_len = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	send_cfg.width = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);

	token = strchr(end_buf + 1, ' ');
	if (token) {
		*token = '\0';
		snprintf(src_file, sizeof(src_file), "%s", end_buf + 1);
		end_buf = token + 1;
		send_cfg.dat_vld = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
		snd_smp_mod = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	} else {
		snprintf(src_file, sizeof(src_file), "%s", end_buf + 1);
		send_cfg.dat_vld = 0;
		snd_smp_mod = 0;
	}

	if (!snd_smp_mod) {
		send_cfg.snd_mod = IRF_COM_SND_MOD;
		if (send_cfg.len > IRF_SEND_CFG_MAX_LEN / (1 << send_cfg.width))
			send_cfg.len = IRF_SEND_CFG_MAX_LEN / (1 << send_cfg.width);

	} else {
		send_cfg.snd_mod = IRF_DDR_SND_MOD;
		if (send_cfg.len > IRF_DDR_SEND_CFG_MAX_LEN / (1 << send_cfg.width))
			send_cfg.len = IRF_DDR_SEND_CFG_MAX_LEN / (1 << send_cfg.width);
	}

	irf_set_snd_smp_mod(send_cfg.node, send_cfg.snd_mod);
	if (send_cfg.snd_mod != IRF_DDR_SND_MOD) {
#if defined(CFG_M3K_FPGA)
		if (irf_smp_send_ram_malloc(priv->plat, send_cfg.node, send_cfg.snd_mod, send_cfg.len * (1 << send_cfg.width) * sizeof(u32), &ddr_res_mem)) {
				pr_err("%s : fail to get DDR memory for node %u\n", __func__, send_cfg.node);
				return count;
		}
		send_cfg.irf_send_buf_addr = ddr_res_mem;
		ret = cls_wifi_load_irf_configfile(priv, true, src_file, send_cfg.irf_send_buf_addr, send_cfg.len * (1 << send_cfg.width), IRF_SND_SMP_MEM_FPGA, false);
		send_cfg.irf_send_buf_addr = 0;
#else
		send_cfg.irf_send_buf_addr = plat->if_ops->get_phy_address(priv->plat,
				priv->radio_idx, CLS_WIFI_ADDR_IRF_PHY, 0);
		ret = cls_wifi_load_irf_configfile(priv, true, src_file, 0, send_cfg.len * (1 << send_cfg.width), IRF_RESV_MEM, false);
		if (ret < 0)
			send_cfg.irf_send_buf_addr = 0;
#endif
		cls_wifi_send_irf_send_config_req(priv, &send_cfg, &cfm);
		if (cfm.status != CO_OK) {
			pr_err("status: %d\n", cfm.status);
			irf_set_snd_smp_mod(send_cfg.node, IRF_DEF_SND_SMP_MOD);
		}
	} else {
		if (irf_smp_send_ram_malloc(priv->plat, send_cfg.node, send_cfg.snd_mod, send_cfg.len * (1 << send_cfg.width) * sizeof(u32), &ddr_res_mem)) {
			pr_err("%s : fail to get DDR memory for node %u\n", __func__, send_cfg.node);
			return count;
		}
		send_cfg.irf_send_buf_addr = ddr_res_mem;
		res_mem_offset = ddr_res_mem - plat->if_ops->get_phy_address(priv->plat, priv->radio_idx, CLS_WIFI_ADDR_IRF_SND_SMP_PHY, 0);
		pr_info("%s : res_mem_offset [%08x]\n", __func__, res_mem_offset);
		ret = cls_wifi_load_irf_configfile(priv, true, src_file, res_mem_offset, send_cfg.len * (1 << send_cfg.width), IRF_SND_SMP_MEM, false);
		if (ret < 0) {
			pr_err("%s : fail to load config file\n", __func__);
			irf_smp_send_ram_free(priv->plat, CO_BIT(send_cfg.node));
			irf_set_snd_smp_mod(send_cfg.node, IRF_DEF_SND_SMP_MOD);
			return count;
		}
		cls_wifi_send_irf_send_config_req(priv, &send_cfg, &cfm);
		if (cfm.status != CO_OK) {
			pr_err("status: %d\n", cfm.status);
			irf_smp_send_ram_free(priv->plat, CO_BIT(send_cfg.node));
			irf_set_snd_smp_mod(send_cfg.node, IRF_DEF_SND_SMP_MOD);
		}
	}

	return count;
}

DEBUGFS_WRITE_FILE_OPS(irf_send_config);

static ssize_t cls_wifi_dbgfs_irf_send_start_write(struct file *file,
									   const char __user *user_buf,
									   size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	char *end_buf = NULL;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);
	struct irf_send_start_req send_start;
	struct irf_send_start_cfm cfm;
	u32 i = 0;

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	send_start.node_mask = cls_wifi_dbgfs_irf_str2val(buf, &end_buf);
	send_start.band = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	send_start.channel = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	send_start.src_node = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);

	cls_wifi_send_irf_send_start_req(priv, &send_start, &cfm);
	if (cfm.status != CO_OK) {
		pr_err("status: %d\n", cfm.status);
		irf_smp_send_ram_free(priv->plat, send_start.node_mask);
		for (i = 0; i < IRF_MAX_NODE; i++) {
			if (send_start.node_mask & CO_BIT(i))
				irf_set_snd_smp_mod(i, IRF_DEF_SND_SMP_MOD);
		}
	}

	return count;
}

DEBUGFS_WRITE_FILE_OPS(irf_send_start);

static ssize_t cls_wifi_dbgfs_irf_send_stop_write(struct file *file,
									   const char __user *user_buf,
									   size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	char *end_buf = NULL;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);
	struct irf_send_stop_req send_stop;
	struct irf_send_stop_cfm cfm;
	u32 i = 0;

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	send_stop.send_stop_req.node_mask = cls_wifi_dbgfs_irf_str2val(buf, &end_buf);
	send_stop.send_stop_req.band = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	send_stop.send_stop_req.channel = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	send_stop.send_stop_req.src_node = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	send_stop.wmac_handle_flag = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);

	cls_wifi_send_irf_send_stop_req(priv, &send_stop, &cfm);
	if (cfm.status != CO_OK)
		pr_err("status: %d\n", cfm.status);

	irf_smp_send_ram_free(priv->plat, send_stop.send_stop_req.node_mask);

	for (i = 0; i < IRF_MAX_NODE; i++) {
		if (send_stop.send_stop_req.node_mask & CO_BIT(i))
			irf_set_snd_smp_mod(i, IRF_DEF_SND_SMP_MOD);
	}

	return count;
}

DEBUGFS_WRITE_FILE_OPS(irf_send_stop);

static ssize_t cls_wifi_dbgfs_irf_smp_lms_write(struct file *file,
									   const char __user *user_buf,
									   size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[64];
	char *end_buf = NULL;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);
	struct irf_smp_lms_req smp_lms;

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	smp_lms.node = cls_wifi_dbgfs_irf_str2val(buf, &end_buf);
	smp_lms.mux_data = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	smp_lms.mux_trg = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	smp_lms.len = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	smp_lms.width = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	smp_lms.sync = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	smp_lms.interval = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	smp_lms.trg_sel = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	smp_lms.delay = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	smp_lms.update = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	smp_lms.irf_smp_buf_addr = priv->plat->if_ops->get_phy_address(priv->plat,
			priv->radio_idx, CLS_WIFI_ADDR_IRF_PHY, 0);
	smp_lms.band = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	smp_lms.channel = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);

	/* Only for debug usage */
	printk("irf_smp_buf_addr: 0x%08x\n", smp_lms.irf_smp_buf_addr);

	cls_wifi_send_irf_smp_lms_req(priv, &smp_lms);

	return count;
}

DEBUGFS_WRITE_FILE_OPS(irf_smp_lms);


static ssize_t cls_wifi_dbgfs_irf_write_lms_write(struct file *file,
									   const char __user *user_buf,
									   size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	struct cls_wifi_plat *plat = priv->plat;
	char buf[192];
	char *end_buf = NULL;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);
	struct irf_write_lms_req lms_cfg;
	struct irf_write_lms_cfm cfm;
	int ret;

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	if (buf[len - 1] == '\n')
		buf[len - 1] = '\0';
	else
		buf[len] = '\0';

	lms_cfg.channel = cls_wifi_dbgfs_irf_str2val(buf, &end_buf);
	lms_cfg.len = LMS_MAX_LEN;
	lms_cfg.irf_lms_data_addr = plat->if_ops->get_phy_address(plat,
			priv->radio_idx, CLS_WIFI_ADDR_IRF_PHY, 0);

	ret = cls_wifi_load_irf_configfile(priv, true, end_buf + 1, 0, lms_cfg.len, IRF_RESV_MEM, false);
	if (ret < 0)
		lms_cfg.irf_lms_data_addr = 0;

	cls_wifi_send_irf_write_lms_req(priv, &lms_cfg, &cfm);
	if (cfm.status != CO_OK)
		pr_err("status: %d\n", cfm.status);

	return count;
}

DEBUGFS_WRITE_FILE_OPS(irf_write_lms);

static ssize_t cls_wifi_dbgfs_irf_cmd_write(struct file *file,
										const char __user *user_buf,
										size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[256];
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	if (buf[len - 1] == '\n')
		buf[len - 1] = '\0';
	else
			buf[len] = '\0';

	irf_cmd_distribute(priv,buf);
	return count;
}

DEBUGFS_WRITE_FILE_OPS(irf_cmd);

static ssize_t cls_wifi_dbgfs_irf_cca_cs_config_read(struct file *file,
									   char __user *user_buf,
									   size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;
	struct irf_cca_cs_config_req cca_cs_config = {0};
	struct irf_cca_cs_config_cfm cfm = {0};

	if (*ppos)
		return 0;

	cca_cs_config.get_config = 1;

	cls_wifi_send_irf_cca_cs_config_req(priv, &cca_cs_config, &cfm);

	if (cfm.status != CO_OK)
		pr_err("status: %d\n", cfm.status);

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"INBDPOW1PUPTHR=%d\n", cfm.inbdpow1_pupthr);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_irf_cca_cs_config_write(struct file *file,
									   const char __user *user_buf,
									   size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	char *end_buf = NULL;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);
	struct irf_cca_cs_config_req cca_cs_config = {0};
	struct irf_cca_cs_config_cfm cfm = {0};

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	cca_cs_config.inbdpow1_pupthr = cls_wifi_dbgfs_irf_str2val(buf, &end_buf);
	cca_cs_config.cca_delta = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);

	cls_wifi_send_irf_cca_cs_config_req(priv, &cca_cs_config, &cfm);

	if (cfm.status != CO_OK)
		pr_err("status: %d\n", cfm.status);

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(irf_cca_cs_config);

static ssize_t cls_wifi_dbgfs_irf_cca_ed_config_read(struct file *file,
									   char __user *user_buf,
									   size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[200];
	int ret;
	ssize_t read;
	struct irf_cca_ed_config_req cca_ed_config = {0};
	struct irf_cca_ed_config_cfm cfm = {0};

	if (*ppos)
		return 0;

	cca_ed_config.get_config = 1;

	cls_wifi_send_irf_cca_ed_config_req(priv, &cca_ed_config, &cfm);

	if (cfm.status != CO_OK)
		pr_err("status: %d\n", cfm.status);

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
			"CCA20PRISETHR=%d CCA20PFALLTHR=%d CCA20SRISETHR=%d CCA20SFALLTHR=%d "
			"CCA40SRISETHR=%d CCA40SFALLTHR=%d CCA80SRISETHR=%d CCA80SFALLTHR=%d\n",
					cfm.cca20p_risethr, cfm.cca20p_fallthr,
					cfm.cca20s_risethr, cfm.cca20s_fallthr,
					cfm.cca40s_risethr, cfm.cca40s_fallthr,
					cfm.cca80s_risethr, cfm.cca80s_fallthr);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_irf_cca_ed_config_write(struct file *file,
									   const char __user *user_buf,
									   size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	char *end_buf = NULL;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);
	struct irf_cca_ed_config_req cca_ed_config = {0};
	struct irf_cca_ed_config_cfm cfm = {0};

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	cca_ed_config.cca20p_risethr = cls_wifi_dbgfs_irf_str2val(buf, &end_buf);
	cca_ed_config.cca20p_fallthr = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	cca_ed_config.cca20s_risethr = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	cca_ed_config.cca20s_fallthr = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	cca_ed_config.cca40s_risethr = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	cca_ed_config.cca40s_fallthr = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	cca_ed_config.cca80s_risethr = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	cca_ed_config.cca80s_fallthr = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);

	cls_wifi_send_irf_cca_ed_config_req(priv, &cca_ed_config, &cfm);

	if (cfm.status != CO_OK)
		pr_err("status: %d\n", cfm.status);

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(irf_cca_ed_config);

static ssize_t cls_wifi_dbgfs_dif_drv_state_read(struct file *file,
										char __user *user_buf,
										size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[64];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"dif_drv_state=%d, pause=%d, boot_cali=%d\n",
					priv->plat->dif_sch->dif_drv_state,
					priv->plat->dif_sch->dif_sm_pause_cnt, priv->dif_sm.boot_cali_status);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_dif_drv_state_write(struct file *file,
										const char __user *user_buf,
										size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		priv->plat->dif_sch->dif_drv_state = val;

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(dif_drv_state);

static ssize_t cls_wifi_dbgfs_dif_fw_state_read(struct file *file,
										char __user *user_buf,
										size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"dif_fw_state=%d\n", priv->dif_sm.dif_fw_state);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

DEBUGFS_READ_FILE_OPS(dif_fw_state);

static ssize_t cls_wifi_dbgfs_online_zif_en_read(struct file *file,
										char __user *user_buf,
										size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[64];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"online_zif_en=%d, reload=%d, boot:%d\n", priv->dif_sm.online_zif_en,
					priv->plat->dif_sch->reload_table_flag, priv->dif_sm.boot_cali_status);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_online_zif_en_write(struct file *file,
										const char __user *user_buf,
										size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		priv->dif_sm.online_zif_en = val;


	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(online_zif_en);

static ssize_t cls_wifi_dbgfs_g_online_zif_en_read(struct file *file,
										char __user *user_buf,
										size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"g_online_zif_en=%d\n", priv->plat->dif_sch->g_online_zif_en);
	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_g_online_zif_en_write(struct file *file,
										const char __user *user_buf,
										size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		priv->plat->dif_sch->g_online_zif_en = val;

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(g_online_zif_en);

static ssize_t cls_wifi_dbgfs_online_zif_cycle_read(struct file *file,
										char __user *user_buf,
										size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"cycle=%d, time=%d\n", priv->plat->dif_sch->online_zif_cycle[priv->radio_idx],
					DIF_SCH_TIME_2_SEC(priv->plat->dif_sch->time[priv->radio_idx]));

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_online_zif_cycle_write(struct file *file,
										const char __user *user_buf,
										size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	long val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (!kstrtol(buf, 0, &val))
		priv->plat->dif_sch->online_zif_cycle[priv->radio_idx] = (uint32_t)val;

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(online_zif_cycle);

static ssize_t cls_wifi_dbgfs_zif_temp_thres_read(struct file *file,
										char __user *user_buf,
										size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	struct cls_wifi_dif_sch *dif_sch;
	char buf[32];
	int ret;
	ssize_t read;

	dif_sch = priv->plat->dif_sch;
	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"thres=%d,%d, temp=%d,%d\n", dif_sch->zif_trig_h_thres[priv->radio_idx],
					dif_sch->zif_trig_l_thres[priv->radio_idx],
					dif_sch->zif_temp_record[priv->radio_idx],
					dif_sch->tsensor_temp[priv->radio_idx]);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_zif_temp_thres_write(struct file *file,
										const char __user *user_buf,
										size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int h_thres;
	int l_thres;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d,%d", &h_thres, &l_thres) > 0) {
		priv->plat->dif_sch->zif_trig_h_thres[priv->radio_idx] = (uint32_t)h_thres;
		priv->plat->dif_sch->zif_trig_l_thres[priv->radio_idx] = (uint32_t)l_thres;
	}

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(zif_temp_thres);



static ssize_t cls_wifi_dbgfs_pwr_ctrl_en_read(struct file *file,
										char __user *user_buf,
										size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"pwr_ctrl_en=%d\n", priv->dif_sm.pwr_ctrl_en);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_pwr_ctrl_en_write(struct file *file,
										const char __user *user_buf,
										size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		priv->dif_sm.pwr_ctrl_en = val;

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(pwr_ctrl_en);

static ssize_t cls_wifi_dbgfs_g_pwr_ctrl_en_read(struct file *file,
										char __user *user_buf,
										size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"g_pwr_ctrl_en=%d\n", priv->plat->dif_sch->g_pwr_ctrl_en);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_g_pwr_ctrl_en_write(struct file *file,
										const char __user *user_buf,
										size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		priv->plat->dif_sch->g_pwr_ctrl_en = val;

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(g_pwr_ctrl_en);

static ssize_t cls_wifi_dbgfs_tx_rx_loop_online_en_write(struct file *file,
										const char __user *user_buf,
										size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);
	struct tx_rx_loop_online_en_req tx_rx_loop_online_en_config = {0};

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		tx_rx_loop_online_en_config.tx_rx_loop_online_en = val;

	cls_wifi_set_tx_rx_loop_online_en_req(priv, &tx_rx_loop_online_en_config);

	return count;
}

DEBUGFS_WRITE_FILE_OPS(tx_rx_loop_online_en);

static ssize_t cls_wifi_dbgfs_dpd_timer_interval_read(struct file *file,
										char __user *user_buf,
										size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"pd_timer_interval=%d\n", priv->dpd_timer_interval);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_dpd_timer_interval_write(struct file *file,
										const char __user *user_buf,
										size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		//配置n即为默认周期的n倍
		priv->dpd_timer_interval = val;

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(dpd_timer_interval);

static ssize_t cls_wifi_dbgfs_enable_dpd_timer_read(struct file *file,
										char __user *user_buf,
										size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"enable_dpd_timer=%d\n", priv->dpd_timer_enabled);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_enable_dpd_timer_write(struct file *file,
										const char __user *user_buf,
										size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val = 0;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0) {
		if (val)
			priv->dpd_timer_enabled = 1;
		else
			priv->dpd_timer_enabled = 0;
	}

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(enable_dpd_timer);

static ssize_t cls_wifi_dbgfs_heartbeat_enable_read(struct file *file,
							char __user *user_buf,
							size_t count, loff_t *ppos)
{
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"heartbeat_enable=%d\n", cls_wifi_mod_params.heartbeat_en);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_heartbeat_enable_write(struct file *file,
							const char __user *user_buf,
							size_t count, loff_t *ppos)
{
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (kstrtoint(buf, 0, &val) == 0)
		cls_wifi_mod_params.heartbeat_en = val;
	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(heartbeat_enable);

static ssize_t cls_wifi_dbgfs_g_aci_det_en_read(struct file *file,
						char __user *user_buf,
						size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
			"g_aci_det_en=%u\n", priv->plat->dif_sch->g_aci_det_en);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_g_aci_det_en_write(struct file *file,
						const char __user *user_buf,
						size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		priv->plat->dif_sch->g_aci_det_en = (uint8_t)val;

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(g_aci_det_en);

static ssize_t cls_wifi_dbgfs_dif_aci_det_cycle_read(struct file *file,
						char __user *user_buf,
						size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"cycle=%u\n", priv->plat->dif_sch->dif_aci_det_cycle);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_dif_aci_det_cycle_write(struct file *file,
						const char __user *user_buf,
						size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	long val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (!kstrtol(buf, 0, &val))
		priv->plat->dif_sch->dif_aci_det_cycle = (uint32_t)val;

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(dif_aci_det_cycle);

#define LINE_MAX_SZ 150

struct st {
	char line[LINE_MAX_SZ + 1];
	unsigned int r_idx;
};

static int compare_idx(const void *st1, const void *st2)
{
	int index1 = ((struct st *)st1)->r_idx;
	int index2 = ((struct st *)st2)->r_idx;

	if (index1 > index2) return 1;
	if (index1 < index2) return -1;

	return 0;
}

static const int bitrates_cck[4] = { 10, 20, 55, 110 };
static const int bitrates_ofdm[8] = { 6, 9, 12, 18, 24, 36, 48, 54};
static const int ru_size_he_er[] =
{
	242,
	106
};

static const int ru_size_he_mu[] =
{
	26,
	52,
	106,
	242,
	484,
	996
};

static const char he_gi[3][4] = {"0.8", "1.6", "3.2"};

static int print_rate(char *buf, int size, u32 rate_config, int *r_idx)
{
	int res = 0;
	u8 format_mod = RC_RATE_GET(FORMAT_MOD, rate_config);
	u8 mcs = RC_RATE_GET(MCS, rate_config);
	u8 bw = RC_RATE_GET(BW, rate_config);
	u8 gi = RC_RATE_GET(GI, rate_config);
	u8 nss = RC_RATE_GET(NSS, rate_config);
	u8 ru = RC_RATE_GET(RU_SIZE, rate_config);

	if (format_mod < FORMATMOD_HT_MF) {
		if (mcs < 4) {
			u8 long_preamble = RC_RATE_GET(LONG_PREAMBLE, rate_config);
			if (r_idx) {
				*r_idx = (mcs * 2) + long_preamble;
				res = scnprintf(buf, size - res, "%4d ", *r_idx);
			}
			res += scnprintf(&buf[res], size - res, "L-CCK/%cP%11c%2u.%1uM   ",
							 long_preamble > 0 ? 'L' : 'S', ' ',
							 bitrates_cck[mcs] / 10,
							 bitrates_cck[mcs] % 10);
		} else {
			mcs -= 4;
			if (r_idx) {
				*r_idx = N_CCK + mcs;
				res = scnprintf(buf, size - res, "%4d ", *r_idx);
			}
			res += scnprintf(&buf[res], size - res, "L-OFDM%13c%2u.0M   ",
							 ' ', bitrates_ofdm[mcs]);
		}
	} else if (format_mod < FORMATMOD_VHT) {
		if (r_idx) {
			*r_idx = N_CCK + N_OFDM +
				nss * (8 * 2 * 2) + mcs * (2 * 2) + bw * 2 + gi;
			res = scnprintf(buf, size - res, "%4d ", *r_idx);
		}
		mcs += nss * 8; // For HT, nss is "included" in mcs index
		res += scnprintf(&buf[res], size - res, "HT%d/%cGI%11cMCS%-2d   ",
						 20 * (1 << bw), gi ? 'S' : 'L', ' ', mcs);
	} else if (format_mod == FORMATMOD_VHT){
		if (r_idx) {
			*r_idx = N_CCK + N_OFDM + N_HT +
				nss * (10 * 4 * 2) + mcs * (4 * 2) + bw * 2 + gi;
			res = scnprintf(buf, size - res, "%4d ", *r_idx);
		}
		res += scnprintf(&buf[res], size - res, "VHT%d/%cGI%*cMCS%d/%1d  ",
						 20 * (1 << bw), gi ? 'S' : 'L',
						 bw > 2 ? 9 : 10, ' ', mcs, nss + 1);
	} else if (format_mod == FORMATMOD_HE_SU){
		if (r_idx) {
			*r_idx = N_CCK + N_OFDM + N_HT + N_VHT +
				nss * (12 * 4 * 3) + mcs * (4 * 3) + bw * 3 + gi;
			res = scnprintf(buf, size - res, "%4d ", *r_idx);
		}
		res += scnprintf(&buf[res], size - res, "HE%d/GI%s%4s%*cMCS%d/%1d%*c",
						 20 * (1 << bw), he_gi[gi],
						 RC_RATE_GET(DCM, rate_config) ? "/DCM" : "",
						 bw > 2 ? 4 : 5, ' ', mcs, nss + 1,
						 mcs > 9 ? 1 : 2, ' ');
	} else if (format_mod == FORMATMOD_HE_MU){
		if (r_idx) {
			*r_idx = N_CCK + N_OFDM + N_HT + N_VHT + N_HE_SU +
				nss * (12 * 6 * 3) + mcs * (6 * 3) + ru * 3 + gi;
			res = scnprintf(buf, size - res, "%4d ", *r_idx);
		}
		res += scnprintf(&buf[res], size - res, "HEMU-%d/GI%s%*cMCS%d/%1d%*c",
						 ru_size_he_mu[ru], he_gi[gi], ru > 1 ? 5 : 6, ' ',
						 mcs, nss + 1, mcs > 9 ? 1 : 2, ' ');

	}
	else if (format_mod == FORMATMOD_HE_ER) {
		if (r_idx) {
			*r_idx = N_CCK + N_OFDM + N_HT + N_VHT + N_HE_SU + N_HE_MU +
				bw * (3 * 3) + mcs * 3 + gi;
			res = scnprintf(buf, size - res, "%4d ", *r_idx);
		}
		res += scnprintf(&buf[res], size - res, "HEER-%d/GI%s%4s%1cMCS%d/%1d%2c",
						 ru_size_he_er[bw], he_gi[gi],
						 RC_RATE_GET(DCM, rate_config) ? "/DCM" : "",
						 ' ', mcs, nss + 1, ' ');
	}
	else { // HE TB
		if (r_idx) {
			*r_idx = N_CCK + N_OFDM + N_HT + N_VHT + N_HE_SU + N_HE_MU + N_HE_ER +
				nss * (12 * 6 * 3) + mcs * (6 * 3) + ru * 3 + gi;
			res = scnprintf(buf, size - res, "%4d ", *r_idx);
		}
		res += scnprintf(&buf[res], size - res, "HETB-%d/GI%s%*cMCS%d/%1d%*c",
						 ru_size_he_mu[ru], he_gi[gi], ru > 1 ? 5 : 6, ' ',
						 mcs, nss + 1, mcs > 9 ? 1 : 2, ' ');
	}

	return res;
}

int cls_wifi_dbgfs_rate_idx(int format_mod, int bw, int mcs, int gi, int nss, int preamble)
{
	 switch (format_mod) {
		 case FORMATMOD_NON_HT:
		 case FORMATMOD_NON_HT_DUP_OFDM:
			 if (mcs < 4)
				 return mcs * 2 + preamble;
			 else
				 return N_CCK + mcs - 4;
		 case FORMATMOD_HT_MF:
		 case FORMATMOD_HT_GF:
			 return N_CCK + N_OFDM + nss * 32 + mcs * 4 +  bw * 2 + gi;
		 case FORMATMOD_VHT:
			 return N_CCK + N_OFDM + N_HT + nss * 80 + mcs * 8 + bw * 2 + gi;
		 case FORMATMOD_HE_SU:
			 return N_CCK + N_OFDM + N_HT + N_VHT + nss * 144 + mcs * 12 + bw * 3 + gi;
			 break;
		 case FORMATMOD_HE_MU:
			 return (N_CCK + N_OFDM + N_HT + N_VHT + N_HE_SU +
					 nss * 216 + mcs * 18 + bw * 3 + gi);
		 case FORMATMOD_HE_ER:
			 return (N_CCK + N_OFDM + N_HT + N_VHT + N_HE_SU + N_HE_MU +
					 bw * 9 + mcs * 3 + gi);
			 break;
		 case FORMATMOD_HE_TB:
			 return (N_CCK + N_OFDM + N_HT + N_VHT + N_HE_SU + N_HE_MU + N_HE_ER +
					 nss * 216 + mcs * 18 + bw * 3 + gi);
		case FORMATMOD_EHT_MU_SU:
			return (N_CCK + N_OFDM + N_HT + N_VHT + N_HE_SU + N_HE_MU + N_HE_ER +
				nss * 192 + mcs * 12 + bw * 3 + gi);
		case FORMATMOD_EHT_TB:
			return (N_CCK + N_OFDM + N_HT + N_VHT + N_HE_SU + N_HE_MU + N_HE_ER +
				N_EHT_SU + nss * 384 + mcs * 36 + bw * 3 + gi);
		 default:
			 return -1;
	 }
}

static void idx_to_rate_cfg(int idx, u32 *rate_config)
{
	*rate_config = 0;
	if (idx < N_CCK)
	{
		RC_RATE_SET(FORMAT_MOD, *rate_config, FORMATMOD_NON_HT);
		RC_RATE_SET(LONG_PREAMBLE, *rate_config, idx & 1);
		RC_RATE_SET(MCS, *rate_config, idx / 2);
	}
	else if (idx < (N_CCK + N_OFDM))
	{
		RC_RATE_SET(FORMAT_MOD, *rate_config, FORMATMOD_NON_HT);
		RC_RATE_SET(MCS, *rate_config,  idx - N_CCK + 4);
	}
	else if (idx < (N_CCK + N_OFDM + N_HT))
	{
		idx -= (N_CCK + N_OFDM);
		RC_RATE_SET(FORMAT_MOD, *rate_config, FORMATMOD_HT_MF);
		RC_RATE_SET(NSS, *rate_config, idx / (8*2*2));
		RC_RATE_SET(MCS, *rate_config, (idx % (8*2*2)) / (2*2));
		RC_RATE_SET(BW, *rate_config, ((idx % (8*2*2)) % (2*2)) / 2);
		RC_RATE_SET(GI, *rate_config, idx & 1);
	}
	else if (idx < (N_CCK + N_OFDM + N_HT + N_VHT))
	{
		idx -= (N_CCK + N_OFDM + N_HT);
		RC_RATE_SET(FORMAT_MOD, *rate_config, FORMATMOD_VHT);
		RC_RATE_SET(NSS, *rate_config, idx / (10*4*2));
		RC_RATE_SET(MCS, *rate_config, (idx % (10*4*2)) / (4*2));
		RC_RATE_SET(BW, *rate_config, ((idx % (10*4*2)) % (4*2)) / 2);
		RC_RATE_SET(GI, *rate_config, idx & 1);
	}
	else if (idx < (N_CCK + N_OFDM + N_HT + N_VHT + N_HE_SU))
	{
		idx -= (N_CCK + N_OFDM + N_HT + N_VHT);
		RC_RATE_SET(FORMAT_MOD, *rate_config, FORMATMOD_HE_SU);
		RC_RATE_SET(NSS, *rate_config, idx / (12*4*3));
		RC_RATE_SET(MCS, *rate_config, (idx % (12*4*3)) / (4*3));
		RC_RATE_SET(BW, *rate_config, ((idx % (12*4*3)) % (4*3)) / 3);
		RC_RATE_SET(GI, *rate_config, idx % 3);
	}
	else if (idx < (N_CCK + N_OFDM + N_HT + N_VHT + N_HE_SU + N_HE_MU))
	{
		idx -= (N_CCK + N_OFDM + N_HT + N_VHT + N_HE_SU);
		RC_RATE_SET(FORMAT_MOD, *rate_config, FORMATMOD_HE_MU);
		RC_RATE_SET(NSS, *rate_config, idx / (12*6*3));
		RC_RATE_SET(MCS, *rate_config, (idx % (12*6*3)) / (6*3));
		RC_RATE_SET(RU_SIZE, *rate_config, ((idx % (12*6*3)) % (6*3)) / 3);
		RC_RATE_SET(GI, *rate_config, idx % 3);
	}
	else if (idx < (N_CCK + N_OFDM + N_HT + N_VHT + N_HE_SU + N_HE_MU + N_HE_ER))
	{
		idx -= (N_CCK + N_OFDM + N_HT + N_VHT + N_HE_SU + N_HE_MU);
		RC_RATE_SET(FORMAT_MOD, *rate_config, FORMATMOD_HE_ER);
		RC_RATE_SET(NSS, *rate_config, 0);
		RC_RATE_SET(MCS, *rate_config, (idx % 9) / 3);
		RC_RATE_SET(BW, *rate_config, idx / 9); // actually RU size
		RC_RATE_SET(GI, *rate_config, idx % 3);
	}
	else
	{
		idx -= (N_CCK + N_OFDM + N_HT + N_VHT + N_HE_SU + + N_HE_MU + N_HE_ER);
		RC_RATE_SET(FORMAT_MOD, *rate_config, FORMATMOD_HE_TB);
		RC_RATE_SET(NSS, *rate_config, idx / (12*6*3));
		RC_RATE_SET(MCS, *rate_config, (idx % (12*6*3)) / (6*3));
		RC_RATE_SET(RU_SIZE, *rate_config, ((idx % (12*6*3)) % (6*3)) / 3);
		RC_RATE_SET(GI, *rate_config, idx % 3);
	}
}

static struct cls_wifi_sta* cls_wifi_dbgfs_get_sta(struct cls_wifi_hw *cls_wifi_hw,
										   char* mac_addr)
{
	u8 mac[6];

	if (sscanf(mac_addr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
		&mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) != 6)
		return NULL;
	return cls_wifi_get_sta_from_mac(cls_wifi_hw, mac);
}

static ssize_t cls_wifi_dbgfs_twt_request_read(struct file *file,
										   char __user *user_buf,
										   size_t count,
										   loff_t *ppos)
{
	char buf[750];
	ssize_t read;
	struct cls_wifi_hw *priv = file->private_data;
	struct cls_wifi_sta *sta = NULL;
	int len;

	/* Get the station index from MAC address */
	sta = cls_wifi_dbgfs_get_sta(priv, file->f_path.dentry->d_parent->d_parent->d_iname);
	if (sta == NULL)
		return -EINVAL;
	if (sta->twt_ind.sta_idx != CLS_WIFI_INVALID_STA)
	{
		struct twt_conf_tag *conf = &sta->twt_ind.conf;
		if (sta->twt_ind.resp_type == MAC_TWT_SETUP_ACCEPT)
			len = scnprintf(buf, sizeof(buf) - 1, "Accepted configuration");
		else if (sta->twt_ind.resp_type == MAC_TWT_SETUP_ALTERNATE)
			len = scnprintf(buf, sizeof(buf) - 1, "Alternate configuration proposed by AP");
		else if (sta->twt_ind.resp_type == MAC_TWT_SETUP_DICTATE)
			len = scnprintf(buf, sizeof(buf) - 1, "AP dictates the following configuration");
		else if (sta->twt_ind.resp_type == MAC_TWT_SETUP_REJECT)
			len = scnprintf(buf, sizeof(buf) - 1, "AP rejects the following configuration");
		else
		{
			len = scnprintf(buf, sizeof(buf) - 1, "Invalid response from the peer");
			goto end;
		}
		len += scnprintf(&buf[len], sizeof(buf) - 1 - len,":\n"
						 "flow_type = %d\n"
						 "wake interval mantissa = %d\n"
						 "wake interval exponent = %d\n"
						 "wake interval = %d us\n"
						 "nominal minimum wake duration = %d us\n",
						 conf->flow_type, conf->wake_int_mantissa,
						 conf->wake_int_exp,
						 conf->wake_int_mantissa << conf->wake_int_exp,
						 conf->wake_dur_unit ?
						 conf->min_twt_wake_dur * 1024:
						 conf->min_twt_wake_dur * 256);
	}
	else
	{
		len = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
						"setup_command = <0: request, 1: suggest, 2: demand>,"
						"flow_type = <0: announced, 1: unannounced>,"
						"wake_interval_mantissa = <0 if setup request and no constraints>,"
						"wake_interval_exp = <0 if setup request and no constraints>,"
						"nominal_min_wake_dur = <0 if setup request and no constraints>,"
						"wake_dur_unit = <0: 256us, 1: tu>");
	}
  end:
	read = simple_read_from_buffer(user_buf, count, ppos, buf, len);
	return read;
}

static ssize_t cls_wifi_dbgfs_twt_request_write(struct file *file,
											const char __user *user_buf,
											size_t count,
											loff_t *ppos)
{
	char *accepted_params[] = {"setup_command",
							   "flow_type",
							   "wake_interval_mantissa",
							   "wake_interval_exp",
							   "nominal_min_wake_dur",
							   "wake_dur_unit",
							   0
							   };
	struct twt_conf_tag twt_conf;
	struct twt_setup_cfm twt_setup_cfm;
	struct cls_wifi_sta *sta = NULL;
	struct cls_wifi_hw *priv = file->private_data;
	char buf[1024], param[30];
	char *line;
	int error = 1, i, val, setup_command = -1;
	bool found;
	size_t len = sizeof(buf) - 1;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);
	/* Get the station index from MAC address */
	sta = cls_wifi_dbgfs_get_sta(priv, file->f_path.dentry->d_parent->d_parent->d_iname);
	if (sta == NULL)
		return -EINVAL;

	/* Get the content of the file */
	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';
	memset(&twt_conf, 0, sizeof(twt_conf));

	line = buf;
	/* Get the content of the file */
	while (line != NULL)
	{
		if (sscanf(line, "%s = %d", param, &val) == 2)
		{
			i = 0;
			found = false;
			// Check if parameter is valid
			while(accepted_params[i])
			{
				if (strcmp(accepted_params[i], param) == 0)
				{
					found = true;
					break;
				}
				i++;
			}

			if (!found)
			{
				dev_err(priv->dev, "%s: parameter %s is not valid\n", __func__, param);
				return -EINVAL;
			}

			if (!strcmp(param, "setup_command"))
			{
				setup_command = val;
			}
			else if (!strcmp(param, "flow_type"))
			{
				twt_conf.flow_type = val;
			}
			else if (!strcmp(param, "wake_interval_mantissa"))
			{
				twt_conf.wake_int_mantissa = val;
			}
			else if (!strcmp(param, "wake_interval_exp"))
			{
				twt_conf.wake_int_exp = val;
			}
			else if (!strcmp(param, "nominal_min_wake_dur"))
			{
				twt_conf.min_twt_wake_dur = val;
			}
			else if (!strcmp(param, "wake_dur_unit"))
			{
				twt_conf.wake_dur_unit = val;
			}
		}
		else
		{
			dev_err(priv->dev, "%s: Impossible to read TWT configuration option\n", __func__);
			return -EFAULT;
		}
		line = strchr(line, ',');
		if(line == NULL)
			break;
		line++;
	}

	if (setup_command == -1)
	{
		dev_err(priv->dev, "%s: TWT missing setup command\n", __func__);
		return -EFAULT;
	}

	// Forward the request to the LMAC
	if ((error = cls_wifi_send_twt_request(priv, setup_command, sta->vif_idx,
									   &twt_conf, &twt_setup_cfm)) != 0)
		return error;

	// Check the status
	if (twt_setup_cfm.status != CO_OK)
		return -EIO;

	return count;
}
DEBUGFS_READ_WRITE_FILE_OPS(twt_request);

static ssize_t cls_wifi_dbgfs_twt_teardown_read(struct file *file,
											char __user *user_buf,
											size_t count,
											loff_t *ppos)
{
	char buf[512];
	int ret;
	ssize_t read;


	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
					"TWT teardown format:\n\n"
					"flow_id = <ID>\n");
	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_twt_teardown_write(struct file *file,
											 const char __user *user_buf,
											 size_t count,
											 loff_t *ppos)
{
	struct twt_teardown_req twt_teardown;
	struct twt_teardown_cfm twt_teardown_cfm;
	struct cls_wifi_sta *sta = NULL;
	struct cls_wifi_hw *priv = file->private_data;
	char buf[256];
	char *line;
	int error = 1;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);
	/* Get the station index from MAC address */
	sta = cls_wifi_dbgfs_get_sta(priv, file->f_path.dentry->d_parent->d_parent->d_iname);
	if (sta == NULL)
		return -EINVAL;

	/* Get the content of the file */
	if (copy_from_user(buf, user_buf, len))
		return -EINVAL;

	buf[len] = '\0';
	memset(&twt_teardown, 0, sizeof(twt_teardown));

	/* Get the content of the file */
	line = buf;

	if (sscanf(line, "flow_id = %d", (int *) &twt_teardown.id) != 1)
	{
		dev_err(priv->dev, "%s: Invalid TWT configuration\n", __func__);
		return -EINVAL;
	}

	twt_teardown.neg_type = 0;
	twt_teardown.all_twt = 0;
	twt_teardown.vif_idx = sta->vif_idx;

	// Forward the request to the LMAC
	if ((error = cls_wifi_send_twt_teardown(priv, &twt_teardown, &twt_teardown_cfm)) != 0)
		return error;

	// Check the status
	if (twt_teardown_cfm.status != CO_OK)
		return -EIO;

	return count;
}
DEBUGFS_READ_WRITE_FILE_OPS(twt_teardown);

static ssize_t cls_wifi_dbgfs_rc_stats_read(struct file *file,
										char __user *user_buf,
										size_t count, loff_t *ppos)
{
	struct cls_wifi_sta *sta = NULL;
	struct cls_wifi_hw *priv = file->private_data;
	char *buf;
	int bufsz, len = 0;
	ssize_t read;
	int i = 0;
	int error = 0;
	struct mm_rc_stats_cfm mm_rc_stats_cfm;
	unsigned int no_samples;
	struct st *st;
	const u8 *mac_addr;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* everything should fit in one call */
	if (*ppos)
		return 0;

	/* Get the station index from MAC address */
	sta = cls_wifi_dbgfs_get_sta(priv, file->f_path.dentry->d_parent->d_parent->d_iname);
	if (sta == NULL)
		return -EINVAL;

	/* Forward the information to the LMAC */
	if ((error = cls_wifi_send_mm_rc_stats(priv, sta->sta_idx, &mm_rc_stats_cfm)))
		return error;

	no_samples = mm_rc_stats_cfm.no_samples;
	if (no_samples == 0)
		return 0;

	bufsz = no_samples * LINE_MAX_SZ + 500;

	buf = kmalloc(bufsz + 1, GFP_ATOMIC);
	if (buf == NULL)
		return 0;

	st = kmalloc(sizeof(struct st) * no_samples, GFP_ATOMIC);
	if (st == NULL)
	{
		kfree(buf);
		return 0;
	}

	for (i = 0; i < no_samples; i++)
	{
		unsigned int tp, eprob;
		len = print_rate(st[i].line, LINE_MAX_SZ,
						 mm_rc_stats_cfm.rate_stats[i].rate_config, &st[i].r_idx);

		if (mm_rc_stats_cfm.sw_retry_step != 0)
		{
			len += scnprintf(&st[i].line[len], LINE_MAX_SZ - len,  "%c",
					mm_rc_stats_cfm.retry_step_idx[mm_rc_stats_cfm.sw_retry_step] == i ? '*' : ' ');
		}
		else
		{
			len += scnprintf(&st[i].line[len], LINE_MAX_SZ - len, " ");
		}
		len += scnprintf(&st[i].line[len], LINE_MAX_SZ - len, "%c",
				mm_rc_stats_cfm.retry_step_idx[0] == i ? 'T' : ' ');
		len += scnprintf(&st[i].line[len], LINE_MAX_SZ - len, "%c",
				mm_rc_stats_cfm.retry_step_idx[1] == i ? 't' : ' ');
		len += scnprintf(&st[i].line[len], LINE_MAX_SZ - len, "%c ",
				mm_rc_stats_cfm.retry_step_idx[2] == i ? 'P' : ' ');

		tp = mm_rc_stats_cfm.tp[i] / 10;
		len += scnprintf(&st[i].line[len], LINE_MAX_SZ - len, " %4u.%1u",
						 tp / 10, tp % 10);

		eprob = ((mm_rc_stats_cfm.rate_stats[i].probability * 1000) >> 16) + 1;
		len += scnprintf(&st[i].line[len],LINE_MAX_SZ - len,
						 "  %4u.%1u %5u(%6u)  %6u",
						 eprob / 10, eprob % 10,
						 mm_rc_stats_cfm.rate_stats[i].success,
						 mm_rc_stats_cfm.rate_stats[i].attempts,
						 mm_rc_stats_cfm.rate_stats[i].sample_skipped);
	}

	mac_addr = cls_wifi_sta_addr(sta);
	len = scnprintf(buf, bufsz ,
					 "\nTX rate info for %02X:%02X:%02X:%02X:%02X:%02X:\n",
					 mac_addr[0], mac_addr[1], mac_addr[2],
					 mac_addr[3], mac_addr[4], mac_addr[5]);

	len += scnprintf(&buf[len], bufsz - len,
			"   # type			   rate			 tpt   eprob	ok(   tot)   skipped\n");

	// add sorted statistics to the buffer
	sort(st, no_samples, sizeof(st[0]), compare_idx, NULL);
	for (i = 0; i < no_samples; i++)
	{
		len += scnprintf(&buf[len], bufsz - len, "%s\n", st[i].line);
	}

	// display HE TB statistics if any
	if (mm_rc_stats_cfm.rate_stats[RC_HE_STATS_IDX].rate_config) {
		unsigned int tp, eprob;
		struct rc_rate_stats *rate_stats = &mm_rc_stats_cfm.rate_stats[RC_HE_STATS_IDX];
		int ul_length = rate_stats->ul_length;

		len += scnprintf(&buf[len], bufsz - len,
						 "\nHE TB rate info:\n");

		len += scnprintf(&buf[len], bufsz - len,
				"	 type			   rate			 tpt   eprob	ok(   tot)   ul_length\n	 ");

		len += print_rate(&buf[len], bufsz - len, rate_stats->rate_config, NULL);

		tp = mm_rc_stats_cfm.tp[RC_HE_STATS_IDX] / 10;
		len += scnprintf(&buf[len], bufsz - len, "	  %4u.%1u",
						 tp / 10, tp % 10);

		eprob = ((rate_stats->probability * 1000) >> 16) + 1;
		len += scnprintf(&buf[len],bufsz - len,
						 "  %4u.%1u %5u(%6u)  %6u\n",
						 eprob / 10, eprob % 10,
						 rate_stats->success,
						 rate_stats->attempts,
						 ul_length);
	}

	len += scnprintf(&buf[len], bufsz - len, "\n MPDUs AMPDUs AvLen trialP");
	len += scnprintf(&buf[len], bufsz - len, "\n%6u %6u %3d.%1d %6u\n",
					 mm_rc_stats_cfm.ampdu_len,
					 mm_rc_stats_cfm.ampdu_packets,
					 mm_rc_stats_cfm.avg_ampdu_len >> 16,
					 ((mm_rc_stats_cfm.avg_ampdu_len * 10) >> 16) % 10,
					 mm_rc_stats_cfm.sample_wait);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, len);

	kfree(buf);
	kfree(st);

	return read;
}

DEBUGFS_READ_FILE_OPS(rc_stats);

static ssize_t cls_wifi_dbgfs_rc_fixed_rate_idx_write(struct file *file,
												  const char __user *user_buf,
												  size_t count, loff_t *ppos)
{
	struct cls_wifi_sta *sta = NULL;
	struct cls_wifi_hw *priv = file->private_data;
	char buf[10];
	int fixed_rate_idx = -1;
	u32 rate_config;
	bool fix_rate;
	int error = 0;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Get the station index from MAC address */
	sta = cls_wifi_dbgfs_get_sta(priv, file->f_path.dentry->d_parent->d_parent->d_iname);
	if (sta == NULL)
		return -EINVAL;

	/* Get the content of the file */
	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;
	buf[len] = '\0';
	sscanf(buf, "%i\n", &fixed_rate_idx);

	/* Convert rate index into rate configuration */
	if ((fixed_rate_idx < 0) ||
		(fixed_rate_idx >= (N_CCK + N_OFDM + N_HT + N_VHT + N_HE_SU + N_HE_MU + N_HE_ER)))
	{
		// disable fixed rate
		rate_config = 0;
		fix_rate = false;
	}
	else
	{
		idx_to_rate_cfg(fixed_rate_idx, &rate_config);
		fix_rate = true;
	}

	// Forward the request to the LMAC
	if ((error = cls_wifi_send_mm_rc_set_rate(priv, sta->sta_idx, rate_config, fix_rate)))
	{
		return error;
	}

	priv->debugfs.rc_config[sta->sta_idx] = rate_config;
	return len;
}

DEBUGFS_WRITE_FILE_OPS(rc_fixed_rate_idx);

static ssize_t cls_wifi_dbgfs_direct_fixed_rate_read(struct file *file,
		char __user *user_buf, size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	struct cls_wifi_sta *sta = NULL;
	char buf[512];
	int ret;
	ssize_t read;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);
	sta = cls_wifi_dbgfs_get_sta(priv, file->f_path.dentry->d_parent->d_parent->d_iname);
	if (sta == NULL)
		return -EINVAL;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
		"help:"
		"\tset HE SU, GI0.8, BW160MHz NSS2, MCS11 to RC\n"
		"\techo format 5 gi 0 bw 3 nss 2 mcs 11 > direct_fixed_rate\n\n"
		"\tset EHT SU, GI0.8, BW160MHz NSS2, MCS13 to RC\n"
		"\techo format 9 gi 0 bw 3 nss 2 mcs 13 > direct_fixed_rate\n\n"
		"\tset RC auto rate\n"
		"\techo > direct_fixed_rate\n\n"
		"\tformat = 0:non-HT; 1:non-HT dup; 2:HT-MF; 3: HT-GF; 4: VHT; 5: HS-SU"
		"6: HE-MU 7: HE-ER; 8: HE-TB; 9: EHT-MU-SU; 10: EHT-TB; 11: EHT-ER\n"
		"\tgi = 0: 0.8; 1: 1.6; 2: 3.2\n"
		"\tbw = 0: 20MHz; 1: 40MHz; 2: 80MHz; 3: 160MHz\n"
		"\tnss = 1: 1SS; 2: 2SS\n"
		"\tmcs = 0~15: MCS 0~15\n\n"
		"Current fixed rate: 0x%08x\n",
		priv->debugfs.rc_config[sta->sta_idx]);
	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_direct_fixed_rate_write(struct file *file,
		const char __user *user_buf, size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	struct cls_wifi_sta *sta = NULL;
	u32 rate_config = 0;
	int format;
	int error = 0;
	char buf[512];
	int nss;
	int mcs;
	int gi;
	int bw;
	size_t len;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);
	sta = cls_wifi_dbgfs_get_sta(priv, file->f_path.dentry->d_parent->d_parent->d_iname);
	if (sta == NULL)
		return -EINVAL;

	len = min_t(size_t, count, sizeof(buf) - 1);
	/* Get the content of the file */
	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';
	if (sscanf(buf, "format %d gi %d bw %d nss %d mcs %d\n",
		&format, &gi, &bw, &nss, &mcs) > 0) {
		RC_RATE_SET(FORMAT_MOD, rate_config, format);
		RC_RATE_SET(GI, rate_config, gi);
		RC_RATE_SET(BW, rate_config, bw);
		RC_RATE_SET(NSS, rate_config, (nss - 1));
		RC_RATE_SET(MCS, rate_config, mcs);

		// Forward the request to the LMAC
		if ((error = cls_wifi_send_mm_rc_set_rate(priv, sta->sta_idx, rate_config, 1)))
			return error;

		priv->debugfs.rc_config[sta->sta_idx] = rate_config;
	} else {
		if ((error = cls_wifi_send_mm_rc_set_rate(priv, sta->sta_idx, rate_config, 0)))
			return error;
	}

	return len;
}
DEBUGFS_READ_WRITE_FILE_OPS(direct_fixed_rate);

static ssize_t cls_wifi_dbgfs_snr_ppdu_write(struct file *file,
												  const char __user *user_buf,
												  size_t count, loff_t *ppos)
{
	struct cls_wifi_sta *sta = NULL;
	struct cls_wifi_hw *priv = file->private_data;
	char buf[10];
	int snr_ppdu = 0;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Get the station index from MAC address */
	sta = cls_wifi_dbgfs_get_sta(priv, file->f_path.dentry->d_parent->d_parent->d_iname);
	if (sta == NULL)
		return -EINVAL;

	/* Get the content of the file */
	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;
	buf[len] = '\0';
	sscanf(buf, "%i\n", &snr_ppdu);

	sta->snr_format_type = snr_ppdu;

	pr_warn("-1: disable,0: NON-HT,1: NON-HT DUP OFDM,2: HT-MF,3: HT-GF,4: VHT,5: HE-SU,6: HE-MU,7: HE-ER,8: HE-TB\n");

	return len;
}

DEBUGFS_WRITE_FILE_OPS(snr_ppdu);


#define RATE_HIST "++++++++++++++++++++++++++++++++++++++++++++++++++"
#define RATE_HIST_LEN sizeof(RATE_HIST)
#define RATE_LEN (50 + RATE_HIST_LEN)

static int cls_wifi_dbgfs_print_rates_histogram(char *buf, int bufsz,
											struct cls_wifi_sta *sta, bool rx)
{
	char hist[] = "++++++++++++++++++++++++++++++++++++++++++++++++++";
	int hist_len = sizeof(hist) - 1;
	int i, len = 0;
	struct cls_wifi_rate_stats *rate_stats;
	const u8 *mac_addr;

	mac_addr = cls_wifi_sta_addr(sta);
	if (rx) {
		rate_stats = &sta->stats.rx_rate;
		len = scnprintf(buf, bufsz, "\nRX ");
	} else {
		rate_stats = &sta->stats.tx_rate;
		len = scnprintf(buf, bufsz, "\nTX ");
	}
	len += scnprintf(&buf[len], bufsz - len,
					 "rate info for %02X:%02X:%02X:%02X:%02X:%02X:\n",
					 mac_addr[0], mac_addr[1], mac_addr[2],
					 mac_addr[3], mac_addr[4], mac_addr[5]);

	// Display Statistics
	for (i = 0 ; i < rate_stats->size ; i++ )
	{
		if (rate_stats->table[i]) {
			int percent = (int)div_u64(((u64)rate_stats->table[i]) * 1000, rate_stats->cpt);
			int p, idx;
			u32 rate_config;

			if (percent == 0)
				continue;

			idx_to_rate_cfg(i, &rate_config);
			len += print_rate(&buf[len], bufsz - len, rate_config, &idx);
			p = (percent * hist_len) / 1000;
			len += scnprintf(&buf[len], bufsz - len, ": %9d(%2d.%1d%%)%.*s\n",
							 rate_stats->table[i],
							 percent / 10, percent % 10, p, hist);
		}
	}

	return len;
}

static ssize_t cls_wifi_dbgfs_rx_rate_read(struct file *file,
									   char __user *user_buf,
									   size_t count, loff_t *ppos)
{
	struct cls_wifi_sta *sta = NULL;
	struct cls_wifi_hw *priv = file->private_data;
	u32 rate_config;
	char *buf;
	int bufsz, len = 0;
	ssize_t read;
	struct rx_vector_1 *last_rx;
	u8 nrx;
	char snr_avg[16][2];
	int i;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* everything should fit in one call */
	if (*ppos)
		return 0;

	/* Get the station index from MAC address */
	sta = cls_wifi_dbgfs_get_sta(priv, file->f_path.dentry->d_parent->d_parent->d_iname);
	if (sta == NULL)
		return -EINVAL;

	bufsz = (sta->stats.rx_rate.rate_cnt * RATE_LEN + 200);
	bufsz += 200;  ///for SINR
	buf = kmalloc(bufsz + 1, GFP_ATOMIC);
	if (buf == NULL)
		return 0;

	len = cls_wifi_dbgfs_print_rates_histogram(buf, bufsz, sta, true);

	// Get number of RX paths
	nrx = (priv->version_cfm.version_phy_1 & MDM_NRX_MASK) >> MDM_NRX_LSB;

	// Display detailed info of the last received rate
	last_rx = &sta->stats.last_rx.rx_vect1;
	len += scnprintf(&buf[len], bufsz - len,"\nLast received rate\n"
					 "type			   rate	 LDPC STBC BEAMFM DCM DOPPLER %s\n",
					 (nrx > 1) ? "rssi1(dBm) rssi2(dBm)" : "rssi(dBm)");

	rate_config = 0;
	RC_RATE_SET(FORMAT_MOD, rate_config, last_rx->format_mod);
	RC_RATE_SET(BW, rate_config, last_rx->ch_bw);
	RC_RATE_SET(LONG_PREAMBLE, rate_config, last_rx->pre_type);
#ifdef CONFIG_CLS_WIFI_MACHW_HE_AP
	if (last_rx->format_mod == FORMATMOD_HE_TB) {
		RC_RATE_SET(MCS, rate_config, last_rx->he_tb.mcs);
		RC_RATE_SET(NSS, rate_config, last_rx->he_tb.nss);
		RC_RATE_SET(GI, rate_config, last_rx->he_tb.gi_type);
		RC_RATE_SET(BW, rate_config, last_rx->he_tb.ru_size);
		RC_RATE_SET(DCM, rate_config, last_rx->he_tb.dcm);
	} else
#endif
	if (last_rx->format_mod >= FORMATMOD_HE_SU) {
		RC_RATE_SET(MCS, rate_config, last_rx->he.mcs);
		RC_RATE_SET(NSS, rate_config, last_rx->he.nss);
		RC_RATE_SET(GI, rate_config, last_rx->he.gi_type);
		if ((last_rx->format_mod == FORMATMOD_HE_MU) ||
			(last_rx->format_mod == FORMATMOD_HE_ER))
			RC_RATE_SET(BW, rate_config, last_rx->he.ru_size);
		RC_RATE_SET(DCM, rate_config, last_rx->he.dcm);
	} else if (last_rx->format_mod == FORMATMOD_VHT) {
		RC_RATE_SET(MCS, rate_config, last_rx->vht.mcs);
		RC_RATE_SET(NSS, rate_config, last_rx->vht.nss);
		RC_RATE_SET(GI, rate_config, last_rx->vht.short_gi);
	} else if (last_rx->format_mod >= FORMATMOD_HT_MF) {
		RC_RATE_SET(MCS, rate_config, last_rx->ht.mcs % 8);
		RC_RATE_SET(NSS, rate_config, last_rx->ht.mcs / 8);
		RC_RATE_SET(GI, rate_config, last_rx->ht.short_gi);
	} else {
		if (legrates_lut[last_rx->leg_rate].idx == -1) {
			dev_err(priv->dev, "Invalid lecacy rate index %d\n", last_rx->leg_rate);
		} else {
			RC_RATE_SET(MCS, rate_config, legrates_lut[last_rx->leg_rate].idx);
		}
		RC_RATE_SET(NSS, rate_config, 0);
		RC_RATE_SET(GI, rate_config, 0);
	}
	if (last_rx->format_mod == FORMATMOD_OFFLOAD)
		rate_config = last_rx->offload_vector1.rate_config;

	len += print_rate(&buf[len], bufsz - len, rate_config, NULL);

	/* flags for HT/VHT/HE */
#ifdef CONFIG_CLS_WIFI_MACHW_HE_AP
	if (last_rx->format_mod == FORMATMOD_HE_TB) {
		len += scnprintf(&buf[len], bufsz - len, "  %c	%c	 %c	%c	 %c",
						 last_rx->he_tb.fec ? 'L' : ' ',
						 last_rx->he_tb.stbc ? 'S' : ' ',
						 last_rx->he_tb.beamformed ? 'B' : ' ',
						 last_rx->he_tb.dcm ? 'D' : ' ',
						 last_rx->he_tb.doppler ? 'D' : ' ');
	} else
#endif
	if (last_rx->format_mod >= FORMATMOD_HE_SU) {
		len += scnprintf(&buf[len], bufsz - len, "  %c	%c	 %c	%c	 %c",
						 last_rx->he.fec ? 'L' : ' ',
						 last_rx->he.stbc ? 'S' : ' ',
						 last_rx->he.beamformed ? 'B' : ' ',
						 last_rx->he.dcm ? 'D' : ' ',
						 last_rx->he.doppler ? 'D' : ' ');
	} else if (last_rx->format_mod == FORMATMOD_VHT) {
		len += scnprintf(&buf[len], bufsz - len, "  %c	%c	 %c		   ",
						 last_rx->vht.fec ? 'L' : ' ',
						 last_rx->vht.stbc ? 'S' : ' ',
						 last_rx->vht.beamformed ? 'B' : ' ');
	} else if (last_rx->format_mod >= FORMATMOD_HT_MF) {
		len += scnprintf(&buf[len], bufsz - len, "  %c	%c				  ",
						 last_rx->ht.fec ? 'L' : ' ',
						 last_rx->ht.stbc ? 'S' : ' ');
	} else {
		len += scnprintf(&buf[len], bufsz - len, "						 ");
	}
	if (nrx > 1) {
		len += scnprintf(&buf[len], bufsz - len, "	   %-4d	   %d\n",
						 last_rx->rssi1, last_rx->rssi1);
	} else {
		len += scnprintf(&buf[len], bufsz - len, "	  %d\n", last_rx->rssi1);
	}

	len += scnprintf(&buf[len], bufsz - len, "%s PPDU\n",sta->stats.hw_rx_vect2_valid ? "cur" : "prev");
	len += scnprintf(&buf[len], bufsz - len, "SINR nss0: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
					 sta->stats.last_hw_rx_vect2.sinr[0] & 0xFF,
					 sta->stats.last_hw_rx_vect2.sinr[1] & 0xFF,
					 sta->stats.last_hw_rx_vect2.sinr[2] & 0xFF,
					 sta->stats.last_hw_rx_vect2.sinr[3] & 0xFF,
					 sta->stats.last_hw_rx_vect2.sinr[4] & 0xFF,
					 sta->stats.last_hw_rx_vect2.sinr[5] & 0xFF,
					 sta->stats.last_hw_rx_vect2.sinr[6] & 0xFF,
					 sta->stats.last_hw_rx_vect2.sinr[7] & 0xFF,
					 sta->stats.last_hw_rx_vect2.sinr[8] & 0xFF,
					 sta->stats.last_hw_rx_vect2.sinr[9] & 0xFF,
					 sta->stats.last_hw_rx_vect2.sinr[10] & 0xFF,
					 sta->stats.last_hw_rx_vect2.sinr[11] & 0xFF,
					 sta->stats.last_hw_rx_vect2.sinr[12] & 0xFF,
					 sta->stats.last_hw_rx_vect2.sinr[13] & 0xFF,
					 sta->stats.last_hw_rx_vect2.sinr[14] & 0xFF,
					 sta->stats.last_hw_rx_vect2.sinr[15] & 0xFF);
	len += scnprintf(&buf[len], bufsz - len, "SINR nss1: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
					 (sta->stats.last_hw_rx_vect2.sinr[0] >> 8) & 0xFF,
					 (sta->stats.last_hw_rx_vect2.sinr[1] >> 8) & 0xFF,
					 (sta->stats.last_hw_rx_vect2.sinr[2] >> 8) & 0xFF,
					 (sta->stats.last_hw_rx_vect2.sinr[3] >> 8) & 0xFF,
					 (sta->stats.last_hw_rx_vect2.sinr[4] >> 8) & 0xFF,
					 (sta->stats.last_hw_rx_vect2.sinr[5] >> 8) & 0xFF,
					 (sta->stats.last_hw_rx_vect2.sinr[6] >> 8) & 0xFF,
					 (sta->stats.last_hw_rx_vect2.sinr[7] >> 8) & 0xFF,
					 (sta->stats.last_hw_rx_vect2.sinr[8] >> 8) & 0xFF,
					 (sta->stats.last_hw_rx_vect2.sinr[9] >> 8) & 0xFF,
					 (sta->stats.last_hw_rx_vect2.sinr[10] >> 8) & 0xFF,
					 (sta->stats.last_hw_rx_vect2.sinr[11] >> 8) & 0xFF,
					 (sta->stats.last_hw_rx_vect2.sinr[12] >> 8) & 0xFF,
					 (sta->stats.last_hw_rx_vect2.sinr[13] >> 8) & 0xFF,
					 (sta->stats.last_hw_rx_vect2.sinr[14] >> 8) & 0xFF,
					 (sta->stats.last_hw_rx_vect2.sinr[15] >> 8) & 0xFF);

	if(sta->stats.sinr_cnt) {
		for (i = 0; i < 16; i++) {
			snr_avg[i][0] = (sta->stats.sinr_total[i][0] / sta->stats.sinr_cnt) & 0xFF;
			snr_avg[i][1] = (sta->stats.sinr_total[i][1] / sta->stats.sinr_cnt) & 0xFF;
		}
		len += scnprintf(&buf[len], bufsz - len, "Agv cnt:%d\n", sta->stats.sinr_cnt);
		len += scnprintf(&buf[len], bufsz - len, "SINR nss0: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
					snr_avg[0][0], snr_avg[1][0], snr_avg[2][0], snr_avg[3][0],
					snr_avg[4][0], snr_avg[5][0], snr_avg[6][0], snr_avg[7][0],
					snr_avg[8][0], snr_avg[9][0], snr_avg[10][0], snr_avg[11][0],
					snr_avg[12][0], snr_avg[13][0], snr_avg[14][0], snr_avg[15][0]);
		len += scnprintf(&buf[len], bufsz - len, "SINR nss1: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
					snr_avg[0][1], snr_avg[1][1], snr_avg[2][1], snr_avg[3][1],
					snr_avg[4][1], snr_avg[5][1], snr_avg[6][1], snr_avg[7][1],
					snr_avg[8][1], snr_avg[9][1], snr_avg[10][1], snr_avg[11][1],
					snr_avg[12][1], snr_avg[13][1], snr_avg[14][1], snr_avg[15][1]);
	}

	read = simple_read_from_buffer(user_buf, count, ppos, buf, len);

	kfree(buf);
	return read;
}

static ssize_t cls_wifi_dbgfs_rx_rate_write(struct file *file,
										const char __user *user_buf,
										size_t count, loff_t *ppos)
{
	struct cls_wifi_sta *sta = NULL;
	struct cls_wifi_hw *priv = file->private_data;

	/* Get the station index from MAC address */
	sta = cls_wifi_dbgfs_get_sta(priv, file->f_path.dentry->d_parent->d_parent->d_iname);
	if (sta == NULL)
		return -EINVAL;

	/* Prevent from interrupt preemption as these statistics are updated under
	 * interrupt */
	spin_lock_bh(&priv->tx_lock);
	memset(sta->stats.rx_rate.table, 0,
		   sta->stats.rx_rate.size * sizeof(sta->stats.rx_rate.table[0]));
	sta->stats.rx_rate.cpt = 0;
	sta->stats.rx_rate.rate_cnt = 0;
	memset(sta->stats.sinr_total, 0, sizeof(sta->stats.sinr_total));
	sta->stats.sinr_cnt = 0;
	spin_unlock_bh(&priv->tx_lock);

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(rx_rate);

static ssize_t cls_wifi_dbgfs_tx_rate_read(struct file *file,
									   char __user *user_buf,
									   size_t count, loff_t *ppos)
{
	struct cls_wifi_sta *sta = NULL;
	struct cls_wifi_hw *priv = file->private_data;
	char *buf;
	int bufsz, len;
	ssize_t read;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* everything should fit in one call */
	if (*ppos)
		return 0;

	/* Get the station index from MAC address */
	sta = cls_wifi_dbgfs_get_sta(priv, file->f_path.dentry->d_parent->d_parent->d_iname);
	if (sta == NULL)
		return -EINVAL;

	bufsz = (sta->stats.tx_rate.rate_cnt * RATE_LEN + 200);
	buf = kmalloc(bufsz + 1, GFP_ATOMIC);
	if (buf == NULL)
		return 0;

	len = cls_wifi_dbgfs_print_rates_histogram(buf, bufsz, sta, false);
	len += scnprintf(&buf[len], bufsz - len,"tx mgmt pkts: %d\n",sta->stats.tx_mgmt_pkts);
	read = simple_read_from_buffer(user_buf, count, ppos, buf, len);

	kfree(buf);
	return read;
}

static ssize_t cls_wifi_dbgfs_tx_rate_write(struct file *file,
										const char __user *user_buf,
										size_t count, loff_t *ppos)
{
	struct cls_wifi_sta *sta = NULL;
	struct cls_wifi_hw *priv = file->private_data;

	/* Get the station index from MAC address */
	sta = cls_wifi_dbgfs_get_sta(priv, file->f_path.dentry->d_parent->d_parent->d_iname);
	if (sta == NULL)
		return -EINVAL;

	/* Prevent from interrupt preemption as these statistics are updated under
	 * interrupt */
	spin_lock_bh(&priv->tx_lock);
	memset(sta->stats.tx_rate.table, 0,
		   sta->stats.tx_rate.size * sizeof(sta->stats.tx_rate.table[0]));
	sta->stats.tx_rate.cpt = 0;
	sta->stats.tx_rate.rate_cnt = 0;
	sta->stats.tx_mgmt_pkts = 0;
	spin_unlock_bh(&priv->tx_lock);

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(tx_rate);

static ssize_t cls_wifi_dbgfs_csi_read(struct file *file,
		char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;

	/* dump CSI parameters */
	cls_wifi_dump_csi_info(priv);

	return 0;
}

static ssize_t cls_wifi_dbgfs_csi_write(struct file *file,
		const char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[256];
	size_t len = min_t(size_t, count, sizeof(buf) - 1);
	char *csi_cmd_name[] = {"enable", "add", "remove", "dump", "log_level", "period",
			"format", "smooth", "savefile"};
	char *csi_cmd = NULL;
	int cmd;
	int enable;
	int period;
	int log_level;
	uint32_t format;
	int smooth;
	int save_file;
	uint8_t macaddr[6] = {0};

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';
	/*
	* usage example:
	* enable 1
	* add aa:bb:cc:dd:ee:ff
	* remove aa:bb:cc:dd:ee:ff
	* period 100
	* format 0x1FF
	*/
	for (cmd = 0 ; cmd < ARRAY_SIZE(csi_cmd_name); cmd++) {
		if (!strncmp(buf, csi_cmd_name[cmd], strlen(csi_cmd_name[cmd]))) {
			csi_cmd = csi_cmd_name[cmd];
			break;
		}
	}

	if (csi_cmd == NULL)
		return count;

	if (cmd == CSI_CMD_ENABLE) {
		if (sscanf(buf, "enable %d", &enable) > 0)
			cls_wifi_enable_csi(priv, enable);
	} else if (cmd == CSI_CMD_ADD_STA) {
		if (sscanf(buf, "add %hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
			&macaddr[0], &macaddr[1], &macaddr[2], &macaddr[3], &macaddr[4], &macaddr[5]) > 0)
			cls_wifi_add_csi_sta(priv, macaddr);
	} else if (cmd == CSI_CMD_REMOVE_STA) {
		if (sscanf(buf, "remove %hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
			&macaddr[0], &macaddr[1], &macaddr[2], &macaddr[3], &macaddr[4], &macaddr[5]) > 0)
			cls_wifi_remove_csi_sta(priv, macaddr);
	} else if (cmd == CSI_CMD_DUMP_INFO) {
		cls_wifi_dump_csi_info(priv);
	} else if (cmd == CSI_CMD_LOG_LEVEL) {
		if (sscanf(buf, "log_level %d", &log_level) > 0)
			cls_wifi_set_csi_log_level(priv, log_level);
	} else if (cmd == CSI_CMD_SET_PERIOD) {
		if (sscanf(buf, "period %x", &period) > 0)
			cls_wifi_set_csi_period(priv, period);
	} else if (cmd == CSI_CMD_SET_FORMAT) {
		if (sscanf(buf, "format %x", &format) > 0)
			cls_wifi_set_csi_format(priv, format);
	} else if (cmd == CSI_CMD_SET_SMOOTH) {
		if (sscanf(buf, "smooth %d", &smooth) > 0)
			cls_wifi_set_he_ltf_smooth(priv, smooth);
	} else if (cmd == CSI_CMD_SAVE_FILE) {
		if (sscanf(buf, "savefile %d", &save_file) > 0)
			cls_wifi_set_save_file(priv, save_file);
	}

	return count;
}
DEBUGFS_READ_WRITE_FILE_OPS(csi);

static ssize_t cls_wifi_dbgfs_atf_read(struct file *file,
		char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *wifi_hw = file->private_data;

//	cls_wifi_atf_dump_quota_table(wifi_hw);
	cls_wifi_atf_get_stats(wifi_hw);
//	cls_wifi_atf_dump_history_stats(wifi_hw);

	return 0;
}

static ssize_t cls_wifi_dbgfs_atf_write(struct file *file,
		const char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[256];
	size_t len = min_t(size_t, count, sizeof(buf) - 1);
	char *atf_cmd_name[] = {"enable", "mode", "gran", "bss_quota","sta_quota", "update",
					"sched_period", "stats_period", "get_stats",
					"stats_busy_thres", "stats_clear_thres","stats_clear_duration",
					"srrc_enable", "log_enable"};
	char *atf_cmd = NULL;
	int cmd;
	uint8_t enable;
	uint8_t mode;
	uint8_t gran;
	uint32_t bss_quota;
	uint32_t sta_quota;
	uint8_t mac_addr[MAC_ADDR_LEN];
	uint32_t sched_period;
	uint32_t stats_period;
	uint32_t stats_thres;
	uint8_t srrc_enable;
	uint8_t log_enable;

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';
	/*
	* usage example:
	* enable 1
	* add aa:bb:cc:dd:ee:ff
	* remove aa:bb:cc:dd:ee:ff
	* period 100
	* format 0x1FF
	*/
	for (cmd = 0 ; cmd < ARRAY_SIZE(atf_cmd_name); cmd++) {
		if (!strncmp(buf, atf_cmd_name[cmd], strlen(atf_cmd_name[cmd]))) {
			atf_cmd = atf_cmd_name[cmd];
			break;
		}
	}

	if (atf_cmd == NULL) {
		pr_info("%s invalid cmd format", __func__);
		return count;
	}

	pr_info("atf_cmd %s cmd %u\n", atf_cmd, cmd);
	if (cmd == ATF_CMD_ENABLE) {
		if (sscanf(buf, "enable %hhu", &enable) > 0)
			cls_wifi_atf_set_enable(priv, enable);
	} else if (cmd == ATF_CMD_MODE) {
		if (sscanf(buf, "mode %hhu", &mode) > 0)
			cls_wifi_atf_set_mode(priv, mode);
	} else if (cmd == ATF_CMD_GRAN) {
		if (sscanf(buf, "gran %hhu", &gran) > 0)
			cls_wifi_atf_set_granularity(priv, gran);
	} else if (cmd == ATF_CMD_BSS_QUOTA) {
		if (sscanf(buf, "bss_quota %hhx:%hhx:%hhx:%hhx:%hhx:%hhx %u",
				&mac_addr[0], &mac_addr[1], &mac_addr[2],
				&mac_addr[3], &mac_addr[4], &mac_addr[5], &bss_quota) > 0)
			cls_wifi_atf_set_bss_quota_from_mac(priv, mac_addr, bss_quota);
	} else if (cmd == ATF_CMD_STA_QUOTA) {
		if (sscanf(buf, "sta_quota %hhx:%hhx:%hhx:%hhx:%hhx:%hhx %u",
				&mac_addr[0], &mac_addr[1], &mac_addr[2],
				&mac_addr[3], &mac_addr[4], &mac_addr[5], &sta_quota) > 0)
			cls_wifi_atf_set_sta_quota_from_mac(priv, mac_addr, sta_quota);
	} else if (cmd == ATF_CMD_UPDATE_QUOTA_TABLE) {
		cls_wifi_atf_update_quota_table(priv);
	} else if (cmd == ATF_CMD_SCHED_PERIOD) {
		if (sscanf(buf, "sched_period %u", &sched_period) > 0)
			cls_wifi_atf_set_sched_period(priv, sched_period);
	} else if (cmd == ATF_CMD_STATS_PERIOD) {
		if (sscanf(buf, "stats_period %u", &stats_period) > 0)
			cls_wifi_atf_set_stats_period(priv, stats_period);
	} else if (cmd == ATF_CMD_GET_STATS) {
		cls_wifi_atf_get_stats(priv);
	} else if (cmd == ATF_CMD_STATS_BUSY_THRES) {
		if (sscanf(buf, "stats_busy_thres %u", &stats_thres) > 0)
			cls_wifi_atf_set_stats_busy_thres(priv, stats_thres);
	} else if (cmd == ATF_CMD_STATS_CLEAR_THRES) {
		if (sscanf(buf, "stats_clear_thres %u", &stats_thres) > 0)
			cls_wifi_atf_set_stats_clear_thres(priv, stats_thres);
	} else if (cmd == ATF_CMD_STATS_CLEAR_DURATION) {
		if (sscanf(buf, "stats_clear_duration %u", &stats_thres) > 0)
			cls_wifi_atf_set_stats_clear_duration(priv, stats_thres);
	} else if (cmd == ATF_CMD_SRRC_ENABLE) {
		if (sscanf(buf, "srrc_enable %hhu", &srrc_enable) > 0)
			cls_wifi_atf_set_srrc_enable(priv, srrc_enable);
	} else if (cmd == ATF_CMD_LOG_ENABLE) {
		if (sscanf(buf, "log_enable %hhu", &log_enable) > 0)
			cls_wifi_atf_set_log_enable(priv, log_enable);
	} else {
		pr_info("%s invalid cmd format", __func__);
	}

	return count;
}
DEBUGFS_READ_WRITE_FILE_OPS(atf);

enum {
	CU_CMD_ENABLE,
	CU_CMD_STATS_PERIOD,
	CU_CMD_GET_STATS,
	CU_CMD_MAX,
};

static ssize_t cls_wifi_dbgfs_cu_read(struct file *file,
		char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *wifi_hw = file->private_data;

	cls_wifi_atf_get_stats(wifi_hw);

	return 0;
}

static ssize_t cls_wifi_dbgfs_cu_write(struct file *file,
		const char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[256];
	size_t len = min_t(size_t, count, sizeof(buf) - 1);
	char *cu_cmd_name[] = {"enable", "stats_period", "get_stats"};
	char *cu_cmd = NULL;
	int cmd;
	uint8_t enable;
	uint32_t stats_period;

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';
	/*
	* usage example:
	* enable 1
	* stats_period 100
	* get_stats
	*/
	for (cmd = 0 ; cmd < ARRAY_SIZE(cu_cmd_name); cmd++) {
		if (!strncmp(buf, cu_cmd_name[cmd], strlen(cu_cmd_name[cmd]))) {
			cu_cmd = cu_cmd_name[cmd];
			break;
		}
	}

	if (cu_cmd == NULL) {
		pr_info("%s invalid cmd format", __func__);
		return count;
	}

	pr_info("cu_cmd %s cmd %u\n", cu_cmd, cmd);
	if (cmd == CU_CMD_ENABLE) {
		if (sscanf(buf, "enable %hhu", &enable) > 0)
			cls_wifi_atf_set_enable(priv, enable);
	} else if (cmd == CU_CMD_STATS_PERIOD) {
		if (sscanf(buf, "stats_period %u", &stats_period) > 0)
			cls_wifi_atf_set_stats_period(priv, stats_period);
	} else if (cmd == CU_CMD_GET_STATS) {
		cls_wifi_atf_get_stats(priv);
	} else {
		pr_info("%s invalid cmd format", __func__);
	}

	return count;
}
DEBUGFS_READ_WRITE_FILE_OPS(cu);

static ssize_t cls_wifi_dbgfs_dpd_wmac_tx_read(struct file *file,
		char __user *user_buf,
		size_t count, loff_t *ppos)
{
	char buf[512];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count),
		"usage: \n"
		"echo \"8 10000 17 0x0b 3 48 11000 0\" > "
		"/sys/kernel/debug/ieee80211/phy1/cls_wifi/dpd_wmac_tx\n"
		"ppdu num: 8\n"
		"ppdu interval: 10000(us)\n"
		"tx power: 17(dBm)\n"
		"nss/mcs: 0x0b, nss=1, mcs=11\n"
		"bw: 3(0=20MHz, 1=40MHz, 2=80MHz, 3=160MHz)\n"
		"mpdu num: 48\n"
		"mpdu length: 11000(bytes)\n"
		"source: 0-random, 1-fixed 0xA5, 2-from file /tmp/wmactx.dat "
		"(mpdu length would be replaced by wmactx.dat file length)\n");

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_dpd_wmac_tx_write(struct file *file,
		const char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[128];
	size_t len = min_t(size_t, count, sizeof(buf) - 1);
	struct mm_dpd_wmac_tx_params_req *req = &priv->dpd_wmac_tx_params.req;
	char *end_buf = NULL;

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';
	req->ppdu_num = cls_wifi_dbgfs_irf_str2val(buf, &end_buf);
	req->ppdu_interval = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	req->tx_power = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	req->nss_mcs = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	req->bw = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	req->mpdu_num = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	req->mpdu_payload_size = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);
	req->source = cls_wifi_dbgfs_irf_str2val(end_buf, &end_buf);

	cls_wifi_dpd_wmac_tx_handler(priv, req);

	return count;
}
DEBUGFS_READ_WRITE_FILE_OPS(dpd_wmac_tx);

static ssize_t cls_wifi_dbgfs_smm_idx_write(struct file *file,
			const char __user *user_buf,
			size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		cls_wifi_send_smm_idx_req(priv, val);

	return count;
}

DEBUGFS_WRITE_FILE_OPS(smm_idx);

static ssize_t cls_wifi_dbgfs_ampdu_prot_write(struct file *file,
			const char __user *user_buf,
			size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		cls_wifi_send_ampdu_prot_req(priv, val);

	return count;
}

DEBUGFS_WRITE_FILE_OPS(ampdu_prot);

static ssize_t cls_wifi_dbgfs_txop_en_write(struct file *file,
			const char __user *user_buf,
			size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		cls_wifi_send_txop_en_req(priv, val);

	return count;
}
DEBUGFS_WRITE_FILE_OPS(txop_en);

static ssize_t cls_wifi_dbgfs_bfparam_read(struct file *file,
		char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	int bufsz;
	ssize_t read;
	int offset;
	char buf[2000];
	int total_len = 0;

	/* everything should fit in one call */
	if (*ppos)
		return 0;

	bufsz = 2000;
	offset = scnprintf(&buf[0], min_t(size_t, bufsz - 1, count),
			"enabled=%u mode=%u grp_user=%u max_snd_sta=%u support_2ss=%u\n"
			"period=%u cbf_lifetime=%u snd log level%u bf log level %u\n"
			"NDP power %u GI %u smooth %u filter %u alpha %u snapshot %u\n",
			priv->bf_param.bf_req.enabled, priv->bf_param.bf_req.snd_mode,
			priv->bf_param.bf_req.max_grp_user, priv->bf_param.bf_req.max_snd_sta,
			priv->bf_param.bf_req.support_2ss, priv->bf_param.bf_req.snd_period,
			priv->bf_param.bf_req.cbf_lifetime, priv->bf_param.bf_req.snd_log_level,
			priv->bf_param.bf_req.bf_log_level,
			priv->bf_param.bf_req.ndp_power,
			priv->bf_param.bf_req.ndp_gi,
			priv->bf_param.bf_req.enable_smooth,
			priv->bf_param.bf_req.enable_fiter,
			priv->bf_param.bf_req.alpha,
			priv->bf_param.bf_req.snapshot);

	offset += scnprintf(&buf[offset], bufsz - offset,"help:\n"
			"\tenable 0/1/2 -- 0: All disabled; 1: SND enabled, BF disabled; 2: SND+BF enabled\n"
			"\tsnd_mode 0/1 -- 0: disabled; 1: oneshot; 2: periodical\n"
			"\tmax_grp_user 1~16 -- (Not support)sound in 1 frame exchange sequence\n"
			"\tmax_snd_sta 1~16 -- max supported sounding STA number\n"
			"\tsnd_period 200,000 -- sounding period(us), typical 200,000us\n"
			"\tfeedback_type 0/1-- CBF feedback type: 0-SU; 1-MU\n"
			"\tcbf_lifetime -- lifetime of received CBF frame\n"
			"\tsnd_log_level 2 -- snd log level\n"
			"\tbf_log_level 2 -- bf log level\n"
			"\tdump_cbf 1 -- dump all STA's current 2 CBFs into file cbf_sta_xx_xx_xx_xx_xx_xx\n"
			"\tndp_power 17 -- direct change the NDP's power\n"
			"\tndp_gi 1 -- Set NDP GI 0=0.8/2x; 1=1.6/2x; 2=3.2/4x\n"
			"\tndp_bw -- set NDP/NDPA BW value(0~3), 256 == invalid\n"
			"\tndp_time_csd 0-- set NDP/BFed PPDU time CSD value\n"
			"\tndp_smm_idx -- set NDP SMM index value\n"
			"\tsupport_2ss -- Support beamforming for 2ss PPDU\n"
			"\tsmooth 0 -- enable(1)/disable(0) BF smooth\n"
			"\tfiter 0 -- enable(1)/disable(0) BF filter\n"
			"\talpha 2048 -- set BF filter alpha value\n"
			"\tsnapshot 1 -- 0:disable snapshot; 1:dump NDP; 2: dump BFed frame\n\n");

	total_len += offset;
	read = simple_read_from_buffer(user_buf, count, ppos, buf, total_len);

	return read;
}

static ssize_t cls_wifi_dbgfs_bfparam_write(struct file *file,
		const char __user *user_buf,
		size_t count, loff_t *ppos)
{
	char *bf_cmd_name[] = {"enable", "snd_mode", "max_grp_user", "max_snd_sta",
				"snd_period", "feedback_type", "cbf_lifetime",
				"snd_log_level", "bf_log_level", "dump_cbf", "ndp_power",
				"ndp_gi", "ndp_bw", "ndp_time_csd", "ndp_smm_idx",
				"support_2ss", "smooth", "filter", "alpha",
				"snapshot"};
	struct cls_wifi_hw *priv = file->private_data;
	char *bf_cmd = NULL;
	char buf[256];
	char *pbuf = &buf[0];
	int offset;
	size_t len;
	int value;
	int cmd;

	len = min_t(size_t, count, sizeof(buf) - 1);
	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';
	for (cmd = 0 ; cmd < ARRAY_SIZE(bf_cmd_name); cmd++) {
		if (!strncmp(buf, bf_cmd_name[cmd], strlen(bf_cmd_name[cmd]))) {
			bf_cmd = bf_cmd_name[cmd];

			break;
		}
	}

	if (bf_cmd == NULL)
		return count;

	offset = strlen(bf_cmd_name[cmd]) + 1;
	pbuf += offset;
	if (sscanf(pbuf, "%d", &value) <= 0)
		return count;

	cls_wifi_bfmer_cmd_handler(priv, cmd, value);

	return count;
}
DEBUGFS_READ_WRITE_FILE_OPS(bfparam);

static ssize_t cls_wifi_dbgfs_power_table_write(struct file *file,
		const char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[128];
	size_t len = min_t(size_t, count, sizeof(buf) - 1);
	//int chan, bw, phymod. power_list[12];

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	cls_wifi_tx_power_table_config(priv, buf);

	return count;

}
DEBUGFS_WRITE_FILE_OPS(power_table);

static ssize_t cls_wifi_dbgfs_pppc_read(struct file *file,
				  char __user *user_buf,
				  size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;

	return cls_wifi_pppc_txpower_show_record_req(priv);
}

static ssize_t cls_wifi_dbgfs_pppc_write(struct file *file,
		const char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[128];
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	cls_wifi_pppc_tx_power_set(priv, buf);

	return count;
}
DEBUGFS_READ_WRITE_FILE_OPS(pppc);

static ssize_t cls_wifi_dbgfs_log_to_uart_write(struct file *file,
			const char __user *user_buf,
			size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		cls_wifi_log_to_uart_req(priv, val);

	return count;
}
DEBUGFS_WRITE_FILE_OPS(log_to_uart);

static ssize_t cls_wifi_dbgfs_rts_cts_dbg_write(struct file *file,
			const char __user *user_buf,
			size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		cls_wifi_rts_cts_dbg_req(priv, val);

	return count;
}
DEBUGFS_WRITE_FILE_OPS(rts_cts_dbg);

static ssize_t cls_wifi_dbgfs_m3k_boc_reg_write(struct file *file,
			const char __user *user_buf,
			size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		cls_wifi_m3k_boc_reg_req(priv, val);

	return count;
}
DEBUGFS_WRITE_FILE_OPS(m3k_boc_reg);

static ssize_t cls_wifi_dbgfs_puncture_write(struct file *file,
			const char __user *user_buf,
			size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[64];
	int vif_idx;
	int bitmap;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';
	pr_info("%s buf %s\n", __func__, buf);
	if (sscanf(buf, "vif %d bitmap 0x%02x", &vif_idx, &bitmap) > 0) {
		pr_info("vif %u bitmap 0x%02x\n", vif_idx, bitmap);
		cls_wifi_send_set_puncture_info(priv, vif_idx, bitmap);
	} else
		cls_wifi_send_set_puncture_info(priv, 0, 0xc0);

	return count;
}
DEBUGFS_WRITE_FILE_OPS(puncture);

static ssize_t cls_wifi_dbgfs_edma_info_read(struct file *file,
		char __user *user_buf,
		size_t count, loff_t *ppos)

{
	struct cls_wifi_hw *wifi_hw = file->private_data;

	cls_wifi_dump_edma_info_req(wifi_hw);

	return 0;
}
DEBUGFS_READ_FILE_OPS(edma_info);

static ssize_t cls_wifi_dbgfs_amsdu_maxnb_write(struct file *file,
			const char __user *user_buf,
			size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int val;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%d", &val) > 0)
		cls_wifi_limit_amsdu_maxnb(priv, val);

	return count;
}
DEBUGFS_WRITE_FILE_OPS(amsdu_maxnb);

static ssize_t cls_wifi_dbgfs_scan_ext_read(struct file *file,
		char __user *user_buf,
		size_t count, loff_t *ppos)

{
	struct cls_wifi_hw *hw = file->private_data;
	char buf[128];
	int len = 0;

	/* everything should fit in one call */
	if (*ppos)
		return 0;

	len = scnprintf(&buf[0], min_t(size_t, sizeof(buf) - 1, count),
			"ext_enabled=%d rx_filter=0x%x work_duration=%d scan_interval=%d\n",
			hw->scan_ext.ext_enabled, hw->scan_ext.rx_filter,
			hw->scan_ext.work_duration, hw->scan_ext.scan_interval);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t cls_wifi_dbgfs_scan_ext_write(struct file *file,
			const char __user *user_buf,
			size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *hw = file->private_data;
	char buf[128];
	struct cls_wifi_scan_ext_info scan_ext;
	size_t len = min_t(size_t, count, sizeof(buf) - 1);
	u32 argc;

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	argc = sscanf(buf, "%u 0x%x %u %u",
			&scan_ext.ext_enabled, &scan_ext.rx_filter, &scan_ext.work_duration,
			&scan_ext.scan_interval);

	if (argc == 4)
		memcpy(&hw->scan_ext, &scan_ext, sizeof(hw->scan_ext));
	else
		pr_warn("[enabled] [rx_filter] [work_duration] [scan_interval]\n");

	return count;
}
DEBUGFS_READ_WRITE_FILE_OPS(scan_ext);

#ifdef CFG_PCIE_SHM
static int __parse_args(char *cmd_buf, char**argv, int max)
{
	int argc = 0;
	char *str_ptr = cmd_buf;
	bool is_para_left = true;

	while ( '\0' != *str_ptr && argc < max) {
		if ('\t' == *str_ptr || ' ' == *str_ptr || '\n' == *str_ptr) {
			*str_ptr = '\0';
			is_para_left = true;
		} else if (is_para_left) {
			is_para_left = false;
			argv[argc++] = str_ptr;
		}
		str_ptr++;
	}

	return argc;
}

static ssize_t cls_wifi_dbgfs_pcie_shm_write(struct file *file,
			const char __user *user_buf,
			size_t count, loff_t *ppos)
{
	void *(*shm_alloc)(pcie_shm_pool_st *shm_obj, int tbl_idx, size_t size);
	void (*shmem_write_sync)(pcie_shm_pool_st *shm_obj, void *ptr, size_t size);
	void (*shmem_read_sync)(pcie_shm_pool_st *shm_obj, void *ptr, size_t size);
	void (*shm_pool_dump)(pcie_shm_pool_st *shm_obj);

	struct cls_wifi_hw *cls_wifi_hw = file->private_data;
	struct cls_wifi_plat *cls_wifi_plat = cls_wifi_hw->plat;
	char *_argv[10] = {0};
	int argc = 0;
	char ** argv =_argv;
	char buf[128];
	char *name = NULL;
	pcie_shm_pool_st *shm_obj = cls_wifi_plat->pcie_shm_pools[0];

	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	argc = __parse_args(buf, argv, 10);
	argc--;
	argv++;

	name = argv[0];

	shm_alloc = symbol_get(pcie_shm_alloc);
	if (!shm_alloc) {
		pr_err("%s symbol_get %s error ", __FUNCTION__, "pcie_shm_alloc");
		goto END;
	}

	shmem_write_sync = symbol_get(pcie_shmem_write_sync);
	if (!shmem_write_sync) {
		pr_err("%s symbol_get %s error ", __FUNCTION__, "pcie_shmem_write_sync");
		goto END;
	}

	shmem_read_sync = symbol_get(pcie_shmem_read_sync);
	if (!shmem_read_sync) {
		pr_err("%s symbol_get %s error ", __FUNCTION__, "pcie_shmem_read_sync");
		goto END;
	}

	shm_pool_dump = symbol_get(pcie_shm_pool_dump);
	if (!shm_pool_dump) {
		pr_err("%s symbol_get %s error ", __FUNCTION__, "pcie_shm_pool_dump");
		goto END;
	}

	if (0 == strcasecmp("alloc", name)) {
		void * addr = NULL;
		int tbl_idx;
		size_t size;

		if (argc < 3) {
			pr_err("alloc parameters too many(%d)!!\n", argc);
			pr_err("alloc <tbl_idx> <size>\n");
			goto END;
		}

		tbl_idx = simple_strtol(argv[1], NULL, 0);
		size = simple_strtol(argv[2], NULL, 0);

		addr = shm_alloc(shm_obj, tbl_idx, size);

		pr_err("shm_alloc: index(%d), addr(0x%px), size(%ld)\n",
				tbl_idx, addr, size);

	} else if (0 == strcasecmp("write", name)) {
		uint32_t width = 1;
		uint32_t data, size;
		uint8_t *addr;
#define MEMSET_UNIT(type, ptr, value, size)                          \
		do {                                                         \
			size_t i = 0; 											 \
			type *p = (type *)ptr;                                   \
			type v = (type)(value & ((1ULL << (sizeof(type) * 8)) - 1));    \
			for (i = 0; 											 \
					i < (size + sizeof(v) - 1) / sizeof(v);          \
					i++) p[i] = v;     								 \
		} while (0)

		if (argc < 5) {
			pr_err("write parameters too many(%d)!!\n", argc);
			pr_err("write <addr> <uint(1,2,4)> <data> <size>\n");
			goto END;
		}

		addr = (uint8_t *)simple_strtoull(argv[1], NULL, 0);
		width = simple_strtol(argv[2], NULL, 0);
		data = simple_strtol(argv[3], NULL, 0);
		size = simple_strtol(argv[4], NULL, 0);

		switch (width) {
		case 1: MEMSET_UNIT(uint8_t,  addr, data, size);  break;
		case 2: MEMSET_UNIT(uint16_t, addr, data, size);  break;
		case 4: MEMSET_UNIT(uint32_t, addr, data, size);  break;
		default: break;
		}

		shmem_write_sync(shm_obj, addr, size);
		pr_err("shm_write: uint(%d) addr(0x%px) data(0x%x) size(%d)\n",
				width, addr, data, size);

	} else if (0 == strcasecmp("read", name)) {
		uint8_t *addr;
		uint32_t size;
		if (argc < 3) {
			pr_err("read parameters too many(%d)!!\n", argc);
			pr_err("read <size>\n");
			goto END;
		}

		addr = (uint8_t *)simple_strtoull(argv[1], NULL, 0);
		size = simple_strtol(argv[2], NULL, 0);

		shmem_read_sync(shm_obj, addr, size);

		print_hex_dump_bytes("SHM: ", DUMP_PREFIX_OFFSET, addr, size);
	} else if (0 == strcasecmp("header", name)) {
		shm_pool_dump(shm_obj);
	} else {
		pr_err("Err commod:%s\n", name);
	}
END:
	return count;
}
DEBUGFS_WRITE_FILE_OPS(pcie_shm);
#endif

static int hex2num(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}


int hex2byte(const char *hex)
{
	int a, b;
	a = hex2num(*hex++);
	if (a < 0)
		return -1;
	b = hex2num(*hex++);
	if (b < 0)
		return -1;
	return (a << 4) | b;
}

int hexstr2bin(const char *hex, u8 *buf, size_t len)
{
	size_t i;
	int a;
	const char *ipos = hex;
	u8 *opos = buf;

	for (i = 0; i < len; i++) {
		a = hex2byte(ipos);
		if (a < 0)
			return -1;
		*opos++ = a;
		ipos += 2;
	}
	return 0;
}

static ssize_t cls_wifi_dbgfs_msg_write(struct file *file,
												  const char __user *user_buf,
												  size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char *string;
	u8 *buf;
	size_t len;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	pr_warn("%s %d, count [%zu].\n", __func__, __LINE__, count);

	if(count & 1)
	{
		pr_warn("%s %d, count [%zu] is not even, count--.\n", __func__, __LINE__, count);
		count--;
		if (!count)
			return 1;
	}

	len = count / 2;
	string = kmalloc(count, GFP_KERNEL);
	buf = kmalloc(len, GFP_KERNEL);

	/* Get the content of the file */
	if (copy_from_user(string, user_buf, count))
		return -EFAULT;

	pr_warn("%s %d, string [%c %c %c %c %c %c %c %c]\n", __func__, __LINE__,
		string[0], string[1], string[2], string[3], string[4], string[5], string[6], string[7]);

	hexstr2bin(string, buf, len);

	pr_warn("%s %d, buf [%02x %02x %02x %02x %02x %02x %02x %02x]\n", __func__, __LINE__,
		buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);

	cls_wifi_send_message(priv, buf);

	kfree(buf);
	kfree(string);

	return count;
}

DEBUGFS_WRITE_FILE_OPS(msg);

static ssize_t cls_wifi_dbgfs_ampdu_write(struct file *file,
												  const char __user *user_buf,
												  size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char *string;
	int ampdu_size = 0;
	int cnt;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	pr_warn("%s %d, count [%zu].\n", __func__, __LINE__, count);

	string = kmalloc(count + 1, GFP_KERNEL);
	string[count] = '\0';

	/* Get the content of the file */
	if (copy_from_user(string, user_buf, count))
		return -EFAULT;

	cnt = sscanf(&string[0], "%i", &ampdu_size);

	pr_warn("%s %d, cnt:%d, ampdu_size:%d\n", __func__, __LINE__,
		cnt, ampdu_size);

	if(cnt == 1 && ampdu_size > 0)
		cls_wifi_send_ampdu_max_size_req(priv, ampdu_size);

	kfree(string);

	return count;
}

DEBUGFS_WRITE_FILE_OPS(ampdu);


static ssize_t cls_wifi_dbgfs_msgq_write(struct file *file,
												  const char __user *user_buf,
												  size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *cls_wifi_hw = file->private_data;
	char buf[32];
	char cmd[32];
	u32 queue_index;
	u32 value, value2, value3;
	u32 argc;
	u32 ret;
	u32 i;
	size_t len = min_t(size_t, count, sizeof(buf));

	/* Get the content of the file */
	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len - 1] = '\0';

	argc = sscanf(buf, "%s %u %u %u %u", cmd, &queue_index, &value, &value2, &value3);

	if (!strcmp(cmd, "init")) {
		if (argc < 5) {
			pr_warn("init [queue_index] [int_en] [int_th] [que_depth]\n");
			return count;
		}
		cls_wifi_msgq_init(cls_wifi_hw, queue_index, value, value2, value3);
	} else if (!strcmp(cmd, "dump")) {
		cls_wifi_msgq_dump(cls_wifi_hw);
	} else if (!strcmp(cmd, "peek")) {
		if (argc < 3) {
			pr_warn("peek [queue_index] [item_index]\n");
			return count;
		}
		ret = cls_wifi_msgq_peek(cls_wifi_hw, queue_index, value);
		pr_warn("Peeked queue %d index %d value [0x%08x]\n", queue_index, value, ret);
	} else if (!strcmp(cmd, "peekinv")) {
		if (argc < 3) {
			pr_warn("peekinv [queue_index] [item_num]\n");
			return count;
		}
		cls_wifi_msgq_peek_invalid(cls_wifi_hw, queue_index, value);
	} else if (!strcmp(cmd, "pop")) {
		if (argc < 3) {
			pr_warn("pop [queue_index] [item_number]\n");
			return count;
		}
		for (i = 0; i < value; i++) {
			ret = cls_wifi_msgq_pop(cls_wifi_hw, queue_index);
			pr_warn("Poped queue %d index %d value [0x%08x]\n",
					queue_index, i, ret);
		}
	} else if (!strcmp(cmd, "push")) {
		if (argc < 4) {
			pr_warn("push [queue_index] [value] [item_number]\n");
			return count;
		}
		for (i = 0; i < value2; i++)
			cls_wifi_msgq_push(cls_wifi_hw, queue_index, value + i);
	} else if (!strcmp(cmd, "push2")) {
		if (argc < 4) {
			pr_warn("push2 [queue_index] [value1] [value2]\n");
			return count;
		}
#if defined(__aarch64__) || defined(__x86_64__)
		cls_wifi_msgq_push2(cls_wifi_hw, queue_index, value, value2);
#else
		pr_warn("push2 is not supported in 32bit platform\n");
#endif

	} else if (!strcmp(cmd, "sgi")) {
		if (argc < 2) {
			pr_warn("sgi [bit]\n");
			return count;
		}
		cls_wifi_hw->plat->ep_ops->irq_trigger(cls_wifi_hw->plat, cls_wifi_hw->radio_idx,
				1 << queue_index);
	} else if (!strcmp(cmd, "memread")) {
		if (argc < 3) {
			pr_warn("memread [offset] [length]\n");
			return count;
		}
		switch (value) {
		case 1:
			ret = ioread8(cls_wifi_hw->plat->if_ops->get_address(cls_wifi_hw->plat,
					cls_wifi_hw->radio_idx, CLS_WIFI_ADDR_SHARED, queue_index));
			break;
		case 2:
			ret = ioread16(cls_wifi_hw->plat->if_ops->get_address(cls_wifi_hw->plat,
					cls_wifi_hw->radio_idx, CLS_WIFI_ADDR_SHARED, queue_index));
			break;
		case 4:
			ret = ioread32(cls_wifi_hw->plat->if_ops->get_address(cls_wifi_hw->plat,
					cls_wifi_hw->radio_idx, CLS_WIFI_ADDR_SHARED, queue_index));
			break;
		default:
			pr_warn("valid length is 1/2/4\n");
			return count;
		}
		pr_warn("memread offset %d length %d value [0x%08x]\n", queue_index, value, ret);
	} else if (!strcmp(cmd, "memwrite")) {
		if (argc < 4) {
			pr_warn("memwrite [offset] [length] [value]\n");
			return count;
		}
		switch (value) {
		case 1:
			iowrite8(value2, cls_wifi_hw->plat->if_ops->get_address(cls_wifi_hw->plat,
					cls_wifi_hw->radio_idx, CLS_WIFI_ADDR_SHARED, queue_index));
			break;
		case 2:
			iowrite16(value2, cls_wifi_hw->plat->if_ops->get_address(cls_wifi_hw->plat,
					cls_wifi_hw->radio_idx, CLS_WIFI_ADDR_SHARED, queue_index));
			break;
		case 4:
			iowrite32(value2, cls_wifi_hw->plat->if_ops->get_address(cls_wifi_hw->plat,
					cls_wifi_hw->radio_idx, CLS_WIFI_ADDR_SHARED, queue_index));
			break;
		default:
			pr_warn("valid length is 1/2/4\n");
			return count;
		}
	} else if (!strcmp(cmd, "bmuinit")) {
		if (argc < 5) {
			pr_warn("bmuinit [pool_index] [size] [nr_per_pool] [p_pool_mask]\n");
			return count;
		}
		cls_wifi_bmu_init(cls_wifi_hw, queue_index, value, value2, value3, 0x10000000);
	} else if (!strcmp(cmd, "bmuput")) {
		if (argc < 4) {
			pr_warn("bmuput [pool_index] [address] [item_number]\n");
			return count;
		}
		for (i = 0; i < value2; i++)
			cls_wifi_bmu_put(cls_wifi_hw, queue_index, value + i * 4);
	} else if (!strcmp(cmd, "bmuget")) {
		if (argc < 3) {
			pr_warn("bmuget [pool_index] [item_number]\n");
			return count;
		}
		for (i = 0; i < value; i++) {
			ret = cls_wifi_bmu_get(cls_wifi_hw, queue_index);
			pr_warn("BMU queue %d index %d value [0x%08x]\n", queue_index, i, ret);
		}
	} else {
		pr_warn("Supported commands:\n");
		pr_warn("\tinit [queue_index] [int_en] [int_th] [que_depth]\n");
		pr_warn("\tdump\n");
		pr_warn("\tpeek [queue_index] [item_index]\n");
		pr_warn("\tpeekinv [queue_index] [item_number]\n");
		pr_warn("\tpop [queue_index] [item_number]\n");
		pr_warn("\tpush [queue_index] [value] [item_number]\n");
		pr_warn("\tpush2 [queue_index] [value1] [value2]\n");
		pr_warn("\tsgi [bit]\n");
		pr_warn("\tmemread [offset] [length]\n");
		pr_warn("\tmemwrite [offset] [length] [value]\n");
		pr_warn("\tbmuinit [pool_index] [size] [nr_per_pool] [p_pool_mask]\n");
		pr_warn("\tbmuput [pool_index] [address] [item_number]\n");
		pr_warn("\tbmuget [pool_index] [item_number]\n");
	}

	return count;
}

DEBUGFS_WRITE_FILE_OPS(msgq);

static ssize_t cls_wifi_dbgfs_fileop_write(struct file *file,
		const char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *cls_wifi_hw = file->private_data;
	char buf[64];
	char filename[32];
	u32 op, region, offset;
	size_t len = min_t(size_t, count, sizeof(buf));
	const struct firmware *fw;
	int ret = 0, argc;

	/* Get the content of the file */
	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len - 1] = '\0';

	argc = sscanf(buf, "%s %u %u 0x%x", filename, &op, &region, &offset);
	if (argc < 4) {
		pr_warn("\t[file_name] [op(1-write/2-set/3-read)] [region(1-wifi/2-iram)] [offset]\n");
		return count;
	}

	pr_warn("file %s op %d region %d offset 0x%x\n", filename, op, region, offset);
	ret = request_firmware(&fw, filename, cls_wifi_hw->dev);
	if (ret) {
		pr_warn("%s: Failed to get %s (%d)\n", __func__, filename, ret);
		return count;
	}

	ret = cls_wifi_mem_ops(cls_wifi_hw, op, region, offset, (void *)fw->data, fw->size);
	if (ret) {
		pr_warn("%s: Failed to op file %s (%d)\n", __func__, filename, ret);
		release_firmware(fw);
		return count;
	}

	release_firmware(fw);
	return count;
}

DEBUGFS_WRITE_FILE_OPS(fileop);

#define CDF_STATS_NUM   4
static struct cdf_stats cdf_stats_env[CDF_STATS_NUM] = {0};
static ssize_t cls_wifi_dbgfs_clicmd_write(struct file *file,
                                                  const char __user *user_buf,
                                                  size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char *string = NULL;
	uint32_t idx = CDF_STATS_NUM;
	uint32_t reg_addr = 0;
	uint32_t offset = 0;
	uint32_t mask = 0;
	uint32_t used = 0;
	uint32_t dump = 0;
	uint32_t ops = 0;
	bool update = false;
	struct cdf_stats *env;

	string = kmalloc(count + 3, GFP_KERNEL);

	if (string == NULL) {
		pr_warn("%s %d string == NULL\n",__FUNCTION__,__LINE__);
		return 0;
	}

	memset(string, 0, count + 3);
	/* Get the content of the file */
	if (copy_from_user(string, user_buf, count))
		return -EFAULT;

	if (sscanf(string, "index :%d offset %d", &idx, &offset) > 1) {
		if(idx < CDF_STATS_NUM) {
			cdf_stats_env[idx].offset = offset;
			cdf_stats_env[idx].index = idx;
			update = true;
		}
	}

	if (sscanf(string, "index :%d used %d", &idx, &used) > 1){
		if(idx < CDF_STATS_NUM) {
			cdf_stats_env[idx].used = used;
			cdf_stats_env[idx].index = idx;
			update = true;
		}
	}

	if (sscanf(string, "index :%d reg_addr 0x%x", &idx, &reg_addr) > 1){
		if(idx < CDF_STATS_NUM) {
			cdf_stats_env[idx].reg_addr = reg_addr;
			cdf_stats_env[idx].index = idx;
			update = true;
		}
	}

	if (sscanf(string, "index :%d ops %d", &idx, &ops) > 1){
		if(idx < CDF_STATS_NUM) {
			cdf_stats_env[idx].ops = ops;
			cdf_stats_env[idx].index = idx;
			update = true;
		}
	}

	if (sscanf(string, "index :%d mask 0x%x", &idx, &mask) > 1){
		if(idx < CDF_STATS_NUM) {
			cdf_stats_env[idx].mask = mask;
			cdf_stats_env[idx].index = idx;
			update = true;
		}
	}

	if (sscanf(string, "index :%d dump %d", &idx, &dump) > 1){
		if(idx < CDF_STATS_NUM) {
			cdf_stats_env[idx].dump = dump;
			cdf_stats_env[idx].index = idx;
			update = true;
		}
	}

	if (update != false) {
		env = &cdf_stats_env[idx];
		pr_warn("reg_addr: 0x%x,used:%d,offset:%d,mask:0x%x,index:%d,ops:%d\n",
						env->reg_addr,env->used,env->offset,env->mask,env->index,env->ops);

		#ifdef __KERNEL__
		cls_wifi_send_clicmd_cdf_req(priv, &cdf_stats_env[idx]);
		#endif
	}

	if(string)
		kfree(string);

    return count;
}

static ssize_t cls_wifi_dbgfs_clicmd_read(struct file *file,
                                       char __user *user_buf,
                                       size_t count, loff_t *ppos)
{
	//struct cls_wifi_hw *priv = file->private_data;
	u8 buf[1024];
	int len = 0;
	ssize_t read;
	struct cdf_stats *env;
	int i;

	for( i = 0; i < CDF_STATS_NUM; i++) {
		env = &cdf_stats_env[i];
		if(env->used)
			len = sprintf(buf, "reg_addr: 0x%x,used:%d,offset:%d,mask:0x%x,index:%d\n",
				env->reg_addr,env->used,env->offset,env->mask,env->index);
	}
	read = simple_read_from_buffer(user_buf, count, ppos, buf, len);

    return read;
}

DEBUGFS_READ_WRITE_FILE_OPS(clicmd);

static ssize_t cls_wifi_dbgfs_pct_stat_write(struct file *file,
						const char __user *user_buf,
						size_t count, loff_t *ppos)
{
	static const char * const accepted_params[] = {"cnt0_cc_idx",
						"cnt1_cc_idx",
						"ku_mode",
						"exception",
						"interrupt",
						"uflag_op",
						"uflag0",
						"uflag1",
						"run",
						0};
	struct dbg_pct_stat_req pct_req = {
				.cnt0_cc_idx = 55,
				.cnt1_cc_idx = 2,
				.ku_mode = 0,
				.exception = 0,
				.interrupt = 0,
				.uflag0 = 0,
				.uflag1 = 0,
				.runflag = 0,};
	struct dbg_pct_stat_cfm pct_cfm;
	struct cls_wifi_hw *priv = file->private_data;
	char buf[1024], param[30];
	char *line;
	int error = 1, i, val;
	bool found;
	size_t len = sizeof(buf) - 1;
	u8 radio_idx = priv->radio_idx;

	/* Get the content of the file */
	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;
	buf[len] = '\0';
	line = buf;
	/* Get the content of the file */
	while (line != NULL) {
		if (sscanf(line, "%s = %d", param, &val) == 2) {
			i = 0;
			found = false;
			// Check if parameter is valid
			while (accepted_params[i]) {
				if (strcmp(accepted_params[i], param) == 0) {
					found = true;
					break;
				}
				i++;
			}
			if (!found) {
				dev_err(priv->dev, "%s: parameter %s is not valid\n", __func__, param);
				return -EINVAL;
			}
			if (!strcmp(param, "cnt0_cc_idx"))
				pct_req.cnt0_cc_idx = val;
			else if (!strcmp(param, "cnt1_cc_idx"))
				pct_req.cnt1_cc_idx = val;
			else if (!strcmp(param, "ku_mode"))
				pct_req.ku_mode = val;
			else if (!strcmp(param, "exception"))
				pct_req.exception = val;
			else if (!strcmp(param, "interrupt"))
				pct_req.interrupt = val;
			else if (!strcmp(param, "uflag_op"))
				pct_req.uflag_op = val;
			else if (!strcmp(param, "uflag0"))
				pct_req.uflag0 = val;
			else if (!strcmp(param, "uflag1"))
				pct_req.uflag1 = val;
			else if (!strcmp(param, "run"))
				pct_req.runflag = val;
		} else {
			dev_err(priv->dev, "%s: Impossible to read PCT Statistics option\n", __func__);
			return -EFAULT;
		}
		line = strchr(line, ',');
		if (line == NULL)
			break;
		line++;
	}
	// Forward the request to the LMAC
	error = cls_wifi_send_dbg_pct_stat_request(priv, &pct_req, &pct_cfm);
	if (error != 0)
		return error;

	// Check the status
	if ((pct_cfm.status != CO_OK) && (pct_cfm.status != CO_OP_IN_PROGRESS))
		return -EIO;

	if (pct_cfm.status == CO_OK) {
		pr_info("##### Performance Counter for wpu radio %d Done #####\n", radio_idx);
		pr_info("seconds:%d", pct_cfm.run_sec);
		pr_info("%s: %lld\n", pct_cfm.cnt0_name, *((uint64_t *)&pct_cfm.cnt0_l));
		pr_info("%s: %lld\n", pct_cfm.cnt1_name, *((uint64_t *)&pct_cfm.cnt1_l));
		pr_info("#####################################################\n");
	}

	if (pct_cfm.status == CO_OP_IN_PROGRESS)
		pr_info("##### Performance Counter for wpu radio %d Start #####\n", radio_idx);

	return count;
}

DEBUGFS_WRITE_FILE_OPS(pct_stat);

enum {
	PR_CMD_ENABLE,
	PR_CMD_BUF_MAX,
	PR_CMD_RUN,
	PR_CMD_DUMP,
	PR_CMD_GET_STATUS,
	PR_CMD_MAX,
};

int safe_kernel_write(struct file *filp, const char *buf, size_t len)
{
	int ret;
	loff_t pos = filp->f_pos;

#if KERNEL_VERSION(5, 10, 0) > LINUX_VERSION_CODE
	mm_segment_t old_fs = get_fs();

	set_fs(KERNEL_DS);
	ret = kernel_write(filp, buf, len, &pos);
	set_fs(old_fs);
#else
	ret = kernel_write(filp, buf, len, &pos);
#endif
	if (ret > 0)
		filp->f_pos = pos;

	return ret;
}

static ssize_t cls_wifi_dbgfs_profile_stat_write(struct file *file,
					const char __user *user_buf,
					size_t count, loff_t *ppos)
{
	static const char * const profile_params[] = {"enable", "buf_size",
						"run", "dump", "get_status", 0};
	static struct dbg_profile_info pr_info;
	struct dbg_profile_stat_req pr_req = {0};
	struct dbg_profile_stat_cfm pr_cfm = {0};
	struct cls_wifi_hw *priv = file->private_data;
	char buf[1024], param[30], prlog[64], prfile_name[128];
	char *line;
	uint32_t *profile_dump = NULL;
	int error = 1, i, val, cnt, dump_no, dump_max = 0;
	bool found;
	size_t len = sizeof(buf) - 1;
	struct file *filp;
#if KERNEL_VERSION(5, 10, 0) > LINUX_VERSION_CODE
	mm_segment_t old_fs;

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	/* Get the content of the file */
	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;
	set_fs(old_fs);
#else
	if (copy_from_user_nofault(buf, user_buf, len))
		return -EFAULT;
#endif
	buf[len] = '\0';
	line = buf;

	/* Get the content of the file */
	if (line != NULL) {
		if (sscanf(line, "%s = %d", param, &val) == 2) {
			i = 0;
			found = false;
			// Check if parameter is valid
			while (profile_params[i]) {
				if (strcmp(profile_params[i], param) == 0) {
					found = true;
					break;
				}
				i++;
			}
			if (!found) {
				dev_err(priv->dev, "%s: parameter %s is not valid\n", __func__, param);
				return -EINVAL;
			}
			if (!strcmp(param, "enable")) {
				pr_req.cmd = PR_CMD_ENABLE;
				pr_req.cmd_val = val;
				pr_info.enable = val;
				goto profile_handling;
			} else if (!strcmp(param, "buf_size")) {
				pr_req.cmd = PR_CMD_BUF_MAX;
				pr_req.cmd_val = val;
				pr_info.buf_size = val;
				if (pr_req.cmd_val % PROFILE_BUF_MAX) {
					dev_err(priv->dev, "%s: parameter %s is not valid, must align with %d\n",
							__func__, param, 128);
					return -EINVAL;
				}
				goto profile_handling;
			} else if (!strcmp(param, "run")) {
				pr_req.cmd = PR_CMD_RUN;
				pr_req.cmd_val = val;
				pr_info.run = val;
				goto profile_handling;
			} else if (!strcmp(param, "dump")) {
				pr_req.cmd = PR_CMD_DUMP;
				pr_info.dump = val;
				goto profile_handling;
			} else if  (!strcmp(param, "get_status")) {
				pr_req.cmd = PR_CMD_GET_STATUS;
				pr_req.cmd_val = val;
				goto profile_handling;
			}
		} else {
			dev_err(priv->dev, "%s: Impossible to read Profiling Statistics option\n", __func__);
			return -EFAULT;
		}
	}

profile_handling:
	if ((pr_req.cmd == PR_CMD_ENABLE) || (pr_req.cmd == PR_CMD_BUF_MAX) ||
			(pr_req.cmd == PR_CMD_RUN) || (pr_req.cmd == PR_CMD_GET_STATUS))
		cls_wifi_send_dbg_profile_stat_request(priv, &pr_req, &pr_cfm);

	if (pr_req.cmd == PR_CMD_DUMP) {
		if (pr_info.buf_size) {
			dump_max = (pr_info.buf_size % PROFILE_BUF_MAX) ?
				(1 + pr_info.buf_size / PROFILE_BUF_MAX) : (pr_info.buf_size / PROFILE_BUF_MAX);
			profile_dump = kmalloc((sizeof(uint32_t) * pr_info.buf_size), GFP_KERNEL);
			for (dump_no = 0; dump_no < dump_max; dump_no++) {
				pr_req.cmd_val = dump_no;
				error = cls_wifi_send_dbg_profile_stat_request(priv, &pr_req, &pr_cfm);
				if (error != 0)
					goto err_exit;
				// Check the status
				if (pr_cfm.status != CO_OK) {
					error = -EIO;
					goto err_exit;
				}

				for (cnt = 0; cnt < PROFILE_BUF_MAX; cnt++)
					profile_dump[dump_no * PROFILE_BUF_MAX + cnt] = pr_cfm.buf[cnt];
			}

			sprintf(prfile_name, "/tmp/Profiling_Counter_WPU%d.txt", priv->radio_idx);
			filp = filp_open(prfile_name, O_CREAT | O_RDWR | O_TRUNC, 0644);
			if (IS_ERR(filp)) {
				pr_err("Failed to open file: %ld\n", PTR_ERR(filp));
				goto err_exit;
			}

			pr_info("### WPU%d Profileing Dump Start###\r\n", priv->radio_idx);
			sprintf(prlog, "### WPU%d Profileing Dump Start###\r\n", priv->radio_idx);
			safe_kernel_write(filp, prlog, strlen(prlog));

			sprintf(prlog, "### WPU%d Start: %d End:%d ###\r\n",
					priv->radio_idx, pr_cfm.buf_start, pr_cfm.buf_end);
			safe_kernel_write(filp, prlog, strlen(prlog));

			if (pr_cfm.buf_count <= pr_info.buf_size)
				for (cnt = pr_cfm.buf_start; cnt < pr_cfm.buf_end; cnt++) {
					sprintf(prlog, "ilink: 0x%08x\r\n", profile_dump[cnt]);
					safe_kernel_write(filp, prlog, strlen(prlog));
				}
			else {
				for (cnt = pr_cfm.buf_start; cnt < pr_info.buf_size; cnt++) {
					sprintf(prlog, "ilink: 0x%08x\r\n", profile_dump[cnt]);
					safe_kernel_write(filp, prlog, strlen(prlog));
				}

				for (cnt = 0; cnt < pr_cfm.buf_end; cnt++) {
					sprintf(prlog, "ilink: 0x%08x\r\n", profile_dump[cnt]);
					safe_kernel_write(filp, prlog, strlen(prlog));
				}
			}

			sprintf(prlog, "### WPU%d Profileing Dump End###\r\n", priv->radio_idx);
			safe_kernel_write(filp, prlog, strlen(prlog));

			pr_info.dump = 0;
			error = 0;
			filp_close(filp, NULL);
			pr_info("### WPU%d Profileing Dump Done###\r\n", priv->radio_idx);
			goto err_exit;
		}
	}

	pr_info("### cmd:%s buf:0x%08x buf_size:%d enable:%d run:%d start:%d end:%d count:%d###\r\n",
			param, (uint32_t)pr_cfm.buf_point, pr_cfm.buf_size, pr_cfm.enable,
			pr_cfm.run, pr_cfm.buf_start, pr_cfm.buf_end, pr_cfm.buf_count);
err_exit:
	kfree(profile_dump);
	return count;
}

DEBUGFS_WRITE_FILE_OPS(profile_stat);


#ifdef CLS_WIFI_DBGCNT_HOST
struct reg_wr_record_item {
    uint32_t reg_addr;
    uint32_t reg_value;
};

struct reg_wr_record_env {
    volatile uint32_t upattern;
    uint32_t mem_size;
    uint32_t mem_addr;
    volatile uint32_t read_ptr;
    volatile uint32_t write_ptr;
    ///struct reg_wr_record_item *item;
    uint32_t item;
    uint32_t item_size;
    volatile uint32_t skip_it;
    volatile uint32_t overwrite_dis;
};

#define RWR_DATAITME(rd_index)  item[rd_index].reg_addr, item[rd_index].reg_value
static void reg_wr_record_dump(volatile struct reg_wr_record_env *rwr_env,int unprotect,struct file *filp)
{
    uint32_t wr_index;
    uint32_t rd_index;
    uint32_t rd_index2;
    uint32_t i;
    uint32_t start_rd_index;
    int delt;
    uint32_t dump_size = 0;
    struct reg_wr_record_item *item;
    uint32_t item_size;
    loff_t pos = 0;
    ssize_t ret;
    char regmsgbuf[48];

    if(rwr_env == NULL) {
        pr_warn("[warn]rwr_env = 0x%px\n", rwr_env);
        return;
    }

    item = (struct reg_wr_record_item *)(((void*)rwr_env) + 512);

    if(rwr_env->upattern != 0x7E7E5555) {
        pr_warn("rwr_env(0x%px)->upattern(0x%x) != 0x7E7E5555\n", rwr_env, rwr_env->upattern);
        return;
    }

    if(!unprotect)
        rwr_env->skip_it |= 1;
    pr_warn("reg_wr_reord dump start!!!rwr_env 0x%px wr_index: %d, rd_index:%d,mem_size:%d,"
        "mem_addr:0x%x,item_size:%d,item: 0x%x(0x%px),hold_flag:0x%x,overwrite_dis:0x%x\n",
        rwr_env,rwr_env->write_ptr, rwr_env->read_ptr,rwr_env->mem_size,
        rwr_env->mem_addr,rwr_env->item_size,rwr_env->item,item,rwr_env->skip_it,rwr_env->overwrite_dis);
    item_size = rwr_env->item_size;

    if(rwr_env->write_ptr > rwr_env->item_size) {
        pr_err("[WARN]write_ptr(%d) > item_size(%d)\n",rwr_env->write_ptr,rwr_env->item_size);
        rwr_env->write_ptr = rwr_env->item_size - 1;
        return;
    }
    if(rwr_env->read_ptr > rwr_env->item_size) {
        pr_err("[WARN]read_ptr(%d) > item_size(%d)\n",rwr_env->read_ptr,rwr_env->item_size);
        rwr_env->read_ptr = 0;
        return;
    }

    do
    {
        wr_index = rwr_env->write_ptr;
        rd_index = rwr_env->read_ptr;
        start_rd_index = rd_index;

        if(rd_index < wr_index) {
            delt = wr_index - rd_index;
        }else if(rd_index > wr_index) {
            delt = item_size - rd_index;
        }else {
            break;
        }
        if(delt <= 0) {
            break;
        }

        delt = (delt > 256) ? 256 : delt;
        for(i = 0; i < delt; i+=1) {
            if(filp) {
                ret = sprintf(regmsgbuf, "0x%08x: 0x%08x\n", RWR_DATAITME(rd_index));
                if(ret > 0) {
                    ret = kernel_write(filp, regmsgbuf, ret, &pos);
                    if(ret < 0) {
                        pr_warn("[REG_Record]write file failed\n");
                        break;
                    }
                }else if( ret < 0){
                    pr_warn("[REG_Record]sprintf failed\n");
                    break;
                }
            }else {
                pr_warn("0x%08x: 0x%08x\n", RWR_DATAITME(rd_index));
            }
            rd_index += 1;
            dump_size += 1;
        }

        rd_index2 = rwr_env->read_ptr;
        if((rd_index2 <= rd_index) && (start_rd_index <= rd_index2)) {
            if(rd_index >= item_size) {
                rwr_env->read_ptr = 0;
            }else {
                rwr_env->read_ptr = rd_index;
            }
        }

        if(dump_size > (item_size * 2)) {
            pr_warn("dump size(%d item) break\n",dump_size);
            break;
        }
    } while(1);

    if(!unprotect)
        rwr_env->skip_it &= (~1U);

    pr_warn("reg_wr_reord dump done!!! wr_index: %d, rd_index:%d, dump %d item\n",
        rwr_env->write_ptr,rwr_env->read_ptr,dump_size);
}

static int cls_wifi_dbgfs_reg_record_read_op(uint32_t input_addr,uint32_t men_size,uint32_t offset)
{
    struct reg_wr_record_env *env;
    void __iomem *remap_addr;

    remap_addr = ioremap(input_addr, men_size);
    if(remap_addr == NULL)
        return -1;

    pr_warn("input_addr:0x%x,men_size:%d,offset:%x,remap_addr: 0x%px\n",input_addr,men_size,offset,remap_addr);

    env = (struct reg_wr_record_env *)(remap_addr + offset);
    reg_wr_record_dump(env, 0, NULL);

    iounmap(remap_addr);

    return 0;
}

static int cls_wifi_dbgfs_reg_record_hold_op(uint32_t input_addr,uint32_t men_size,uint32_t offset,int hold)
{
    struct reg_wr_record_env *env;
    void __iomem *remap_addr;

    remap_addr = ioremap(input_addr, men_size);
    if(remap_addr == NULL)
        return -1;
    pr_warn("input_addr:0x%x,men_size:%d,offset:%x,remap_addr: 0x%px\n",input_addr,men_size,offset,remap_addr);

    env = (struct reg_wr_record_env *)(remap_addr + offset);
    if(env->upattern == 0x7E7E5555) {
        if(hold)
            env->skip_it |= (1 << 3);
        else
            env->skip_it &= (~(1 << 3));
        pr_warn("rwr_env(%p)->upattern(0x%x) env->skip_it:%x\n", env, env->upattern,env->skip_it);
    }else {
        pr_warn("rwr_env(%p)->upattern(0x%x) != 0x7E7E5555\n", env, env->upattern);
    }

    iounmap(remap_addr);

    return 0;
}


static ssize_t cls_wifi_dbgfs_reg_record_write(struct file *file,
                                                  const char __user *user_buf,
                                                  size_t count, loff_t *ppos)
{
    struct cls_wifi_hw *priv = file->private_data;
    char *string = NULL;
    uint32_t radio_5g_addr = 2 * 1024 * 1024;  ///memory start addr
    uint32_t radio_2g4_addr = (64 * 1024);  ///memory start addr
    uint32_t input_addr;
    uint32_t men_size = 2 * 1024 * 1024;
    uint32_t offset = 0;
    struct reg_wr_record_env *env = NULL;
    void __iomem *remap_addr = NULL;
    struct file *filp = NULL;
    char file_path[48];
    static uint16_t index[2] = {0};

    pr_warn("%s %d, count [%zu].\n", __func__, __LINE__, count);
    string = kmalloc(count + 3, GFP_KERNEL);

    if(string == NULL){
        pr_warn("%s %d string == NULL\n",__FUNCTION__,__LINE__);
        return 0;
    }

    memset(string, 0, count + 3);
    /* Get the content of the file */
    if (copy_from_user(string, user_buf, count))
        return -EFAULT;

    pr_warn("%s %d count: %zu string: %s\n", __FUNCTION__, __LINE__, count, string);
    if(priv->radio_idx == 0) {
        input_addr = radio_2g4_addr;
        men_size = 2 * 1024 * 1024 - (64 * 1024);
    }else{
        input_addr = radio_5g_addr;
        men_size = 2 * 1024 * 1024;
    }

    remap_addr = ioremap(input_addr, men_size);
    if(remap_addr)
        env = (struct reg_wr_record_env *)(remap_addr + offset);

    pr_warn("input_addr:0x%x,men_size:%d,offset:%x,remap_addr: 0x%px\n",input_addr,men_size,offset,remap_addr);

    if (strncmp(string, "clear", 5) == 0) {
        if(env) {
            if(env->upattern == 0x7E7E5555) {
                env->skip_it |= (1 << 5);
                barrier();
                env->read_ptr = env->write_ptr;
                barrier();
                env->skip_it &= (~(1 << 5));
            }else {
                pr_warn("rwr_env(%p)->upattern(0x%x) != 0x7E7E5555\n", env, env->upattern);
            }
        }
    }else if (strncmp(string, "hold", 4) == 0) {
        cls_wifi_dbgfs_reg_record_hold_op(input_addr, men_size, offset, 1);
    }else if (strncmp(string, "unhold", 6) == 0) {
        cls_wifi_dbgfs_reg_record_hold_op(input_addr, men_size, offset, 0);
    }else if (strncmp(string, "overwrite", 9) == 0) {
        if(env) {
            if(env->upattern == 0x7E7E5555) {
                env->overwrite_dis = 0;
            }else{
                pr_warn("rwr_env(%p)->upattern(0x%x) != 0x7E7E5555\n", env, env->upattern);
            }
        }
    }else if (strncmp(string, "un-overwrite", 12) == 0) {
        if(env) {
            if(env->upattern == 0x7E7E5555) {
                env->overwrite_dis = 1;
            }else{
                pr_warn("rwr_env(%p)->upattern(0x%x) != 0x7E7E5555\n", env, env->upattern);
            }
        }
    }else if (strncmp(string, "info", 4) == 0) {
        if(env){
            pr_warn("rwr_env 0x%px upattern:0x%x,wr_index: %d, rd_index:%d,"
                "mem_size:%d,mem_addr:0x%x,item_size:%d,item: 0x%x,hold_flag:0x%x,overwrite_dis:0x%x\n",
                env,env->upattern,env->write_ptr, env->read_ptr,env->mem_size,
                env->mem_addr,env->item_size,env->item,env->skip_it,env->overwrite_dis);
        }
    }else if (strncmp(string, "dump", 4) == 0) {
        reg_wr_record_dump(env, 0, NULL);
    }else if (strncmp(string, "file_dump", 9) == 0) {
        if(priv->radio_idx == 0) {
            sprintf(file_path, "/tmp/reg_rocord_2g4_%d", index[0]);
            index[0]++;
        }else{
            sprintf(file_path, "/tmp/reg_rocord_5g_%d", index[1]);
            index[0]++;
        }
        filp = filp_open(file_path, O_RDWR | O_CREAT | O_TRUNC, 0666);
        if(filp) {
            reg_wr_record_dump(env, 0, filp);
            filp_close(filp, NULL);
        }
    }else if (strncmp(string, "unprotect_dump", 14) == 0) {
        reg_wr_record_dump(env, 1, NULL);
    }else {
        pr_warn("support command: clear,hold,unhold,info,dump\n");
    }

    if(string)
        kfree(string);

    if(remap_addr)
        iounmap(remap_addr);

    return count;
}

static ssize_t cls_wifi_dbgfs_reg_record_read(struct file *file,
                                       char __user *user_buf,
                                       size_t count, loff_t *ppos)
{
    struct cls_wifi_hw *priv = file->private_data;
    uint32_t radio_5g_addr = 2 * 1024 * 1024;
    uint32_t radio_2g4_addr = (64 * 1024);
    uint32_t input_addr = radio_5g_addr;
    uint32_t men_size = 2 * 1024 * 1024;
    uint32_t offset = 0;

    if(priv->radio_idx == 0) {
        input_addr = radio_2g4_addr;
        men_size = 2 * 1024 * 1024 - (64 * 1024);
    }else {
        input_addr = radio_5g_addr;
    }

    cls_wifi_dbgfs_reg_record_read_op(input_addr, men_size, offset);

    return 0;
}

DEBUGFS_READ_WRITE_FILE_OPS(reg_record);
#define RC_STATS_UPATTERN   0x0867890AU

struct rc_stats_record_item {
    volatile uint16_t sta_idx;
    /// Number of attempts (per sampling interval)
    volatile uint16_t attempts;
    /// Number of success (per sampling interval)
    volatile uint16_t success;
    /// Estimated probability of success (EWMA)
    volatile uint16_t probability;
    /// Rate configuration of the sample (@ref rc_rate_bf)
    volatile uint32_t rate_config;

    volatile uint8_t rc_step;
    volatile uint8_t sample_idx;
    volatile uint8_t sample_no;
    volatile uint8_t seq_num;
    volatile int8_t rssi;
    volatile uint16_t skip;
    volatile uint16_t edca_failed;
    volatile uint16_t trial_cnt;
    volatile uint16_t ampdu_cnt;
    volatile uint16_t submpdu_cnt;
    volatile uint16_t ampdu_fail_ba_notrecv;
    volatile uint16_t ampdu_fail_ba_invalid;
    volatile uint16_t ampdu_fail_ba_allzero;
    volatile uint16_t collision_punish;
    volatile uint16_t collision_continuous_max;
    volatile uint32_t tp;
};

struct rc_stats_record_env {
    volatile uint32_t upattern;
    volatile uint32_t mem_size;
    volatile uint32_t mem_addr;
    volatile uint32_t rd_idx;
    volatile uint32_t write_idx;
    volatile uint32_t item;
    volatile uint32_t item_size;
    volatile uint32_t skip_it;
    volatile uint32_t overwrite_dis;
    volatile uint32_t seq_num;
};

static void reg_rc_record_dump(volatile struct rc_stats_record_env *rc_env,int unprotect,struct file *filp)
{
    uint32_t wr_index;
    uint32_t rd_index;
    uint32_t rd_index2;
    uint32_t i;
    uint32_t start_rd_index;
    int delt;
    uint32_t dump_size = 0;
    volatile struct rc_stats_record_item *item, *item_array;
    uint32_t item_size;
    loff_t pos = 0;
    ssize_t ret;
    char regmsgbuf[512];
    uint8_t format;
    uint8_t gi;
    uint8_t mcs;
    uint8_t nss;
    uint8_t bw;
    bool dcm;

    if(rc_env == NULL) {
        pr_warn("[warn]rwr_env = 0x%px\n", rc_env);
        return;
    }

    item_array = (struct rc_stats_record_item *)(((void*)rc_env) + 256);

    if(rc_env->upattern != RC_STATS_UPATTERN) {
        pr_warn("rwr_env(0x%px)->upattern(0x%x) != 0x7E7E5555\n", rc_env, rc_env->upattern);
        return;
    }

    if(!unprotect)
        rc_env->skip_it |= 2;
    pr_warn("rc_record dump start!!!rc_env 0x%px wr_index: %d, rd_index:%d,mem_size:%d,"
        "mem_addr:0x%x,item_size:%d,item: 0x%x(0x%px),skip_it:0x%x,overwrite_dis:0x%x\n",
        rc_env,rc_env->write_idx, rc_env->rd_idx,rc_env->mem_size,
        rc_env->mem_addr,rc_env->item_size,rc_env->item,item_array,rc_env->skip_it,rc_env->overwrite_dis);
    item_size = rc_env->item_size;

    if(rc_env->write_idx > rc_env->item_size) {
        pr_err("[WARN]write_ptr(%d) > item_size(%d)\n",rc_env->write_idx,rc_env->item_size);
        rc_env->write_idx = rc_env->item_size - 1;
        return;
    }
    if(rc_env->rd_idx > rc_env->item_size) {
        pr_err("[WARN]read_ptr(%d) > item_size(%d)\n",rc_env->rd_idx,rc_env->item_size);
        rc_env->rd_idx = rc_env->write_idx;
        return;
    }

    do
    {
        wr_index = rc_env->write_idx;
        rd_index = rc_env->rd_idx;
        start_rd_index = rd_index;

        if(rd_index < wr_index) {
            delt = wr_index - rd_index;
        }else if(rd_index > wr_index) {
            delt = item_size - rd_index;
        }else {
            break;
        }
        if(delt <= 0) {
            break;
        }

        delt = (delt > 256) ? 256 : delt;
        for(i = 0; i < delt; i+=1) {
            item = &item_array[rd_index];
            format = RC_RATE_GET(FORMAT_MOD, item->rate_config);
            if(format <= 1)
                gi = RC_RATE_GET(LONG_PREAMBLE, item->rate_config);
            else
                gi = RC_RATE_GET(GI, item->rate_config);
            mcs = RC_RATE_GET(MCS, item->rate_config);
            nss = RC_RATE_GET(NSS, item->rate_config);
            bw = RC_RATE_GET(BW, item->rate_config);
            dcm = RC_RATE_GET(DCM, item->rate_config);
            if(filp) {
                ///HE TB
                if(item->sample_idx == 10) {
                    ret = sprintf(regmsgbuf, " , \n");
                    ret = kernel_write(filp, regmsgbuf, ret, &pos);
                    if(ret < 0) {
                        pr_warn("[RC_Record]write file failed\n");
                        break;
                    }
                }
                ret = sprintf(regmsgbuf,
                    "sta_idx,%d,sample_idx,%d,rate_config,0x%08x,seq_num,%d,format,%d,mcs,%d,nss,%d,bw,%d,dcm,%d,gi,%d,"
                    "success_mdpu,%d,attempts_ampdu,%d,probability,%d,skip,%d,edca_failed,%d,rc_step,%d,sampel_no,%d,tp,%d,"
					"trial_cnt,%d,ampdu_cnt,%d,submpdu_cnt,%d,ampdu_fail_ba_notrecv,%d,ampdu_fail_ba_invalid,%d,"
					"ampdu_fail_ba_allzero,%d,collision_punish,%d,collision_continuous_max,%d,rssi,%d\n",
                    item->sta_idx, item->sample_idx, item->rate_config,item->seq_num, format, mcs, nss, bw, dcm,gi,
                    item->success, item->attempts, item->probability,item->skip,item->edca_failed,
                    ((item->rc_step & 0xFF) >= 0) ? (item->rc_step & 0xFF) : -1, item->sample_no,item->tp,
                    item->trial_cnt, item->ampdu_cnt, item->submpdu_cnt, item->ampdu_fail_ba_notrecv,
                    item->ampdu_fail_ba_invalid, item->ampdu_fail_ba_allzero, item->collision_punish,
                    item->collision_continuous_max, item->rssi);
                if (ret > 0) {
                    ret = kernel_write(filp, regmsgbuf, ret, &pos);
                    if(ret < 0) {
                        pr_warn("[RC_Record]write file failed\n");
                        break;
                    }
                }else if( ret < 0){
                    pr_warn("[RC_Record]sprintf failed\n");
                    break;
                }
            }
            else {
                pr_warn("sta_idx:%04x,format:%02d,mcs:%02d,nss :%02d,bw:%02d,dcm:%d,"
                    "success_mdpu:0x%04x,attempts_ampdu:0x%04x,probability:0x%04d\n",
                    item->sta_idx, format, mcs, nss, bw, dcm,
                    item->success,item->attempts,item->probability);
            }
            rd_index += 1;
            dump_size += 1;
        }

        rd_index2 = rc_env->rd_idx;
        if((rd_index2 <= rd_index) && (start_rd_index <= rd_index2)) {
            if(rd_index >= item_size) {
                rc_env->rd_idx = 0;
            }else {
                rc_env->rd_idx = rd_index;
            }
        }

        if(dump_size > (item_size * 2)) {
            pr_warn("dump size(%d item) break\n",dump_size);
            break;
        }
    } while(1);

    if(!unprotect)
        rc_env->skip_it &= (~2U);

    pr_warn("reg_wr_reord dump done!!! wr_index: %d, rd_index:%d, dump %d item\n",
        rc_env->write_idx, rc_env->rd_idx, dump_size);
}


static int cls_wifi_dbgfs_rc_record_read_op(uint32_t input_addr,uint32_t men_size,uint32_t offset)
{
    struct rc_stats_record_env *env;
    void __iomem *remap_addr;
    struct file *filp;
    u8 file_path[64] = {0};

    remap_addr = ioremap(input_addr, men_size);
    if(remap_addr == NULL)
        return -1;

    snprintf(file_path, 64,"/tmp/rate_ctrl_0x%08x.csv",input_addr);
    filp = filp_open(file_path, O_RDWR | O_CREAT | O_TRUNC, 0666);

    pr_warn("input_addr:0x%x,men_size:%d,offset:%x,remap_addr: 0x%px\n",
            input_addr,men_size,offset,remap_addr);

    env = (struct rc_stats_record_env *)(remap_addr + offset);
    reg_rc_record_dump(env, 0, filp);

    iounmap(remap_addr);
    if (filp != NULL) {
        filp_close(filp, NULL);
    }

    return 0;
}

static ssize_t cls_wifi_dbgfs_rc_record_read(struct file *file,
                                       char __user *user_buf,
                                       size_t count, loff_t *ppos)
{
    struct cls_wifi_hw *priv = file->private_data;
    uint32_t radio_5g_addr = 6 * 1024 * 1024;
    uint32_t radio_2g4_addr = 4 * 1024 * 1024;
    uint32_t input_addr = radio_5g_addr;
    uint32_t men_size = 2 * 1024 * 1024;
    uint32_t offset = 0;

    if(priv->radio_idx == 0) {
        input_addr = radio_2g4_addr;
    }else {
        input_addr = radio_5g_addr;
    }

    cls_wifi_dbgfs_rc_record_read_op(input_addr, men_size, offset);

    return 0;
}

static ssize_t cls_wifi_dbgfs_rc_record_write(struct file *file,
                                                  const char __user *user_buf,
                                                  size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	struct rc_stats_record_env *env;
	void __iomem *remap_addr;
	uint32_t radio_5g_addr = 6 * 1024 * 1024;
	uint32_t radio_2g4_addr = 4 * 1024 * 1024;
	uint32_t input_addr = radio_5g_addr;
	uint32_t men_size = 2 * 1024 * 1024;
	uint32_t offset = 0;
	char string[64];
	char cmd_string[64];
	uint32_t ops;
	int ret;

	if(priv->radio_idx == 0) {
		input_addr = radio_2g4_addr;
	}else {
		input_addr = radio_5g_addr;
	}

	/* Get the content of the file */
	if (copy_from_user(string, user_buf, count))
		return -EFAULT;

	remap_addr = ioremap(input_addr, men_size);
	if(remap_addr == NULL)
		return 0;
	env = (struct rc_stats_record_env *)(remap_addr + offset);

	ret = sscanf(string, "%s %d", cmd_string, &ops);
	if(ret > 1){
		if(!strcmp(cmd_string,"enable")) {
			if(!ops)
				env->skip_it |= 1;
			else
				env->skip_it &= (~1);
		}
		if(!strcmp(cmd_string,"overwrite")) {
			if(!ops)
				env->overwrite_dis |= 1;
			else
				env->overwrite_dis &= (~1);
		}
	}
	pr_warn("skip_it:%x\n",env->skip_it);
	iounmap(remap_addr);

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(rc_record);
#endif

static ssize_t cls_wifi_dbgfs_fp_write(struct file *file,
												  const char __user *user_buf,
												  size_t count, loff_t *ppos)
{
    struct cls_wifi_hw *cls_wifi_hw = file->private_data;
    char string[64], dev_src[64], dev_des[64];
    int vif_idx;
    bool found_src = false;
    bool bypass_qdisc = true;

    CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

    if(count > 64)
    {
        pr_warn("%s %d, string too long[%d].\n", __func__, __LINE__, (int)(count - 1));
        return -EFAULT;
    }

    memset(string, 0, sizeof(string));
    memset(dev_src, 0, sizeof(dev_src));
    memset(dev_des, 0, sizeof(dev_des));

    /* Get the content of the file */
    if (copy_from_user(string, user_buf, count))
        return -EFAULT;

    string[count - 1] = '\0';
    sscanf(string, "%s %s", dev_src, dev_des);
    if (strstr(string, "bypass_qdisc_en"))
        bypass_qdisc = true;
    else if (strstr(string, "bypass_qdisc_dis"))
        bypass_qdisc = false;


    for(vif_idx = 0; vif_idx < CLS_ITF_MAX; vif_idx++)
    {
        if (!cls_wifi_hw->vif_table[vif_idx] || !cls_wifi_hw->vif_table[vif_idx]->ndev)
        {
            continue;
        }
        if (!strcmp(dev_src, cls_wifi_hw->vif_table[vif_idx]->ndev->name))
        {
            if((cls_wifi_hw->vif_table[vif_idx]->ndev_fp = dev_get_by_name(&init_net, dev_des)) != NULL)
            {
                cls_wifi_hw->vif_table[vif_idx]->fp_bypass_qdisc = bypass_qdisc;
                dev_put(cls_wifi_hw->vif_table[vif_idx]->ndev_fp);
                pr_warn("%s %d, fp from [%s] to [%s], bypass_qdisc %d.\n",
                        __func__, __LINE__, dev_src, dev_des, bypass_qdisc);
            }
            else
            {
                pr_warn("%s %d, des dev [%s] not found.\n", __func__, __LINE__, dev_des);
            }
            found_src = true;
            break;
        }
    }

    if(!found_src)
    {
        pr_warn("%s %d, src dev [%s] not found.\n", __func__, __LINE__, dev_src);
    }

    return count;
}

DEBUGFS_WRITE_FILE_OPS(fp);

static ssize_t cls_wifi_dbgfs_trig_dump_wmac_write(struct file *file,
				const char __user *user_buf, size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *cls_wifi_hw = file->private_data;
	char string[64], type[64];
	struct cls_wifi_plat *cls_wifi_plat;
	struct ipc_host_env_tag *ipc_env;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);
	cls_wifi_plat = cls_wifi_hw->plat;
	ipc_env = cls_wifi_hw->ipc_env;
	if (!cls_wifi_plat || !ipc_env) {
		pr_warn("%s %d can't find plat \n", __func__, __LINE__);
		return -EFAULT;
	}
	if (cls_wifi_plat->hw_rev == CLS_WIFI_HW_DUBHE2000)
		return count;

	if (count > 64) {
		pr_warn("%s %d, string too long[%d].\n", __func__, __LINE__, (int)(count - 1));
		return -EFAULT;
	}

	memset(string, 0, sizeof(string));
	memset(type, 0, sizeof(string));

	/* Get the content of the file */
	if (copy_from_user(string, user_buf, count))
	return -EFAULT;

	string[count - 1] = '\0';

	sscanf(string, "%s", type);
	if (!strcmp(type, "wmac"))
		cls_wifi_trig_dump_wpu(cls_wifi_hw, IPC_DUMP_FLAG_WMAC);
	else if (!strcmp(type, "dbgcnt"))
		cls_wifi_trig_dump_wpu(cls_wifi_hw, IPC_DUMP_FLAG_DBGCNT);
	return count;
}

DEBUGFS_WRITE_FILE_OPS(trig_dump_wmac);

static ssize_t cls_wifi_dbgfs_op_mode_read(struct file *file,
					char __user *user_buf,
					size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[32];
	int ret;
	ssize_t read;

	ret = scnprintf(buf, min_t(size_t, sizeof(buf) - 1, count), "%s\n", cls_wifi_op_mode_name[priv->op_mode]);

	read = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	return read;
}

static ssize_t cls_wifi_dbgfs_op_mode_write(struct file *file,
					const char __user *user_buf,
					size_t count, loff_t *ppos)
{
	struct cls_wifi_hw *priv = file->private_data;
	char buf[16];
	size_t len = min_t(size_t, count, sizeof(buf) - 1);

	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (!strncasecmp(buf, "sta", 3))
		priv->op_mode = CLS_WIFI_STA_MODE;
	else if (!strncasecmp(buf, "rpt", 3) || !strncasecmp(buf, "repeater", 8))
		priv->op_mode = CLS_WIFI_REPEATER_MODE;
	else
		priv->op_mode = CLS_WIFI_AP_MODE;

	pr_info("[%s] radio %d op_mode %s\n", __func__, priv->radio_idx, cls_wifi_op_mode_name[priv->op_mode]);

	return count;
}

DEBUGFS_READ_WRITE_FILE_OPS(op_mode);

/*
 * trace helper
 */
void cls_wifi_fw_trace_dump(struct cls_wifi_hw *cls_wifi_hw)
{
	/* may be called before cls_wifi_dbgfs_register */
	if (cls_wifi_hw->enabled && !cls_wifi_hw->debugfs.fw_trace.buf.data) {
		cls_wifi_fw_trace_buf_init(cls_wifi_hw, &cls_wifi_hw->debugfs.fw_trace.buf);
	}

	if (!cls_wifi_hw->debugfs.fw_trace.buf.data)
		return;

	_cls_wifi_fw_trace_dump(&cls_wifi_hw->debugfs.fw_trace.buf);
}

void cls_wifi_fw_trace_reset(struct cls_wifi_hw *cls_wifi_hw)
{
	_cls_wifi_fw_trace_reset(&cls_wifi_hw->debugfs.fw_trace, true);
}

void cls_wifi_dbgfs_trigger_fw_dump(struct cls_wifi_hw *cls_wifi_hw, char *reason)
{
	cls_wifi_send_dbg_trigger_req(cls_wifi_hw, reason);
}

static void _cls_wifi_dbgfs_register_sta(struct cls_wifi_debugfs *cls_wifi_debugfs, struct cls_wifi_sta *sta)
{
	struct cls_wifi_hw *cls_wifi_hw = container_of(cls_wifi_debugfs, struct cls_wifi_hw, debugfs);
	struct dentry *dir_sta;
	char sta_name[18];
	struct dentry *dir_rc;
	struct dentry *file;
	struct cls_wifi_rate_stats *rx_rate_stats = &sta->stats.rx_rate;
	struct cls_wifi_rate_stats *tx_rate_stats = &sta->stats.tx_rate;
	int nb_rate = N_CCK + N_OFDM;
	struct cls_wifi_rc_config_save *rc_cfg, *next;

	if (sta->sta_idx >= hw_remote_sta_max(cls_wifi_hw)) {
		scnprintf(sta_name, sizeof(sta_name), "bc_mc");
	} else {
		scnprintf(sta_name, sizeof(sta_name), "%pM", cls_wifi_sta_addr(sta));
	}

	if (!(dir_sta = debugfs_create_dir(sta_name, cls_wifi_debugfs->dir_stas)))
		goto error;
	cls_wifi_debugfs->dir_sta[sta->sta_idx] = dir_sta;

	if (!(dir_rc = debugfs_create_dir("rc", cls_wifi_debugfs->dir_sta[sta->sta_idx])))
		goto error_after_dir;

	cls_wifi_debugfs->dir_rc_sta[sta->sta_idx] = dir_rc;

	file = debugfs_create_file("stats", DBGFS_RO, dir_rc, cls_wifi_hw,
							   &cls_wifi_dbgfs_rc_stats_ops);
	if (IS_ERR_OR_NULL(file))
		goto error_after_dir;

	file = debugfs_create_file("direct_fixed_rate", DBGFS_RW, dir_rc, cls_wifi_hw,
			&cls_wifi_dbgfs_direct_fixed_rate_ops);
	if (IS_ERR_OR_NULL(file))
		goto error_after_dir;

	file = debugfs_create_file("fixed_rate_idx", DBGFS_WR , dir_rc, cls_wifi_hw,
							   &cls_wifi_dbgfs_rc_fixed_rate_idx_ops);
	if (IS_ERR_OR_NULL(file))
		goto error_after_dir;

	file = debugfs_create_file("snr_ppdu", DBGFS_WR , dir_rc, cls_wifi_hw,
						   &cls_wifi_dbgfs_snr_ppdu_ops);
	if (IS_ERR_OR_NULL(file))
		goto error_after_dir;

	file = debugfs_create_file("rx_rate", DBGFS_RW, dir_rc, cls_wifi_hw,
							   &cls_wifi_dbgfs_rx_rate_ops);
	if (IS_ERR_OR_NULL(file))
		goto error_after_dir;

	file = debugfs_create_file("tx_rate", DBGFS_RW, dir_rc, cls_wifi_hw,
							   &cls_wifi_dbgfs_tx_rate_ops);
	if (IS_ERR_OR_NULL(file))
		goto error_after_dir;

	if (cls_wifi_hw->radio_params->ht_on)
		nb_rate += N_HT;

	if (cls_wifi_hw->radio_params->vht_on)
		nb_rate += N_VHT;

	if (cls_wifi_hw->radio_params->he_on)
		nb_rate += N_HE_SU + N_HE_MU + N_HE_ER + N_HE_TB;

	rx_rate_stats->table = kzalloc(nb_rate * sizeof(rx_rate_stats->table[0]),
								   GFP_KERNEL);
	if (!rx_rate_stats->table)
		goto error_after_dir;

	rx_rate_stats->size = nb_rate;
	rx_rate_stats->cpt = 0;
	rx_rate_stats->rate_cnt = 0;

	tx_rate_stats->table = kzalloc(nb_rate * sizeof(tx_rate_stats->table[0]),
								   GFP_KERNEL);
	if (!tx_rate_stats->table)
		goto error_after_dir;

	tx_rate_stats->size = nb_rate;
	tx_rate_stats->cpt = 0;
	tx_rate_stats->rate_cnt = 0;

	/* By default enable rate contoller */
	cls_wifi_debugfs->rc_config[sta->sta_idx] = -1;

	/* Unless we already fix the rate for this station */
	list_for_each_entry_safe(rc_cfg, next, &cls_wifi_debugfs->rc_config_save, list) {
		if (jiffies_to_msecs(jiffies - rc_cfg->timestamp) > RC_CONFIG_DUR) {
			list_del(&rc_cfg->list);
			kfree(rc_cfg);
		} else if (!memcmp(rc_cfg->mac_addr, cls_wifi_sta_addr(sta), ETH_ALEN)) {
			u32 rate_config;
			cls_wifi_debugfs->rc_config[sta->sta_idx] = rc_cfg->rate;
			rate_config = rc_cfg->rate;
			if (rate_config != -1) {
				if (cls_wifi_send_mm_rc_set_rate(cls_wifi_hw, sta->sta_idx, rate_config, true))
					cls_wifi_debugfs->rc_config[sta->sta_idx] = -1;
			}
			list_del(&rc_cfg->list);
			kfree(rc_cfg);
			break;
		}
	}

	if (CLS_WIFI_VIF_TYPE(cls_wifi_hw->vif_table[sta->vif_idx]) == NL80211_IFTYPE_STATION)
	{
		/* register the sta */
		struct dentry *dir_twt;
		struct dentry *file;

		if (!(dir_twt = debugfs_create_dir("twt", cls_wifi_debugfs->dir_sta[sta->sta_idx])))
			goto error_after_dir;

		cls_wifi_debugfs->dir_twt_sta[sta->sta_idx] = dir_twt;

		file = debugfs_create_file("request", DBGFS_RW, dir_twt, cls_wifi_hw,
								   &cls_wifi_dbgfs_twt_request_ops);
		if (IS_ERR_OR_NULL(file))
			goto error_after_dir;

		file = debugfs_create_file("teardown", DBGFS_RW, dir_twt, cls_wifi_hw,
								   &cls_wifi_dbgfs_twt_teardown_ops);
		if (IS_ERR_OR_NULL(file))
			goto error_after_dir;

		sta->twt_ind.sta_idx = CLS_WIFI_INVALID_STA;
	}
	return;

error_after_dir:
	if (sta->stats.rx_rate.table) {
		kfree(sta->stats.rx_rate.table);
		sta->stats.rx_rate.table = NULL;
		sta->stats.rx_rate.size = 0;
	}
	if (sta->stats.tx_rate.table) {
		kfree(sta->stats.tx_rate.table);
		sta->stats.tx_rate.table = NULL;
		sta->stats.tx_rate.size = 0;
	}
	debugfs_remove_recursive(cls_wifi_debugfs->dir_sta[sta->sta_idx]);
	cls_wifi_debugfs->dir_sta[sta->sta_idx] = NULL;
	cls_wifi_debugfs->dir_rc_sta[sta->sta_idx] = NULL;
	cls_wifi_debugfs->dir_twt_sta[sta->sta_idx] = NULL;
error:
	dev_err(cls_wifi_hw->dev,
			"Error while registering debug entry for sta %d\n", sta->sta_idx);
}

static void _cls_wifi_dbgfs_unregister_sta(struct cls_wifi_debugfs *cls_wifi_debugfs, struct cls_wifi_sta *sta,
									   unsigned int sta_idx)
{
	debugfs_remove_recursive(cls_wifi_debugfs->dir_sta[sta_idx]);
	cls_wifi_debugfs->dir_sta[sta_idx] = NULL;
	cls_wifi_debugfs->dir_rc_sta[sta_idx] = NULL;
	cls_wifi_debugfs->dir_twt_sta[sta_idx] = NULL;

	/* Clear STA's statistics */
	if (sta->stats.rx_rate.table) {
		kfree(sta->stats.rx_rate.table);
		sta->stats.rx_rate.table = NULL;
	}
	sta->stats.rx_rate.size = 0;
	sta->stats.rx_rate.cpt  = 0;
	sta->stats.rx_rate.rate_cnt = 0;

	if (sta->stats.tx_rate.table) {
		kfree(sta->stats.tx_rate.table);
		sta->stats.tx_rate.table = NULL;
	}
	sta->stats.tx_rate.size = 0;
	sta->stats.tx_rate.cpt  = 0;
	sta->stats.tx_rate.rate_cnt = 0;
	sta->stats.tx_pkts = 0;
	sta->stats.tx_mgmt_pkts = 0;
	sta->stats.tx_bytes = 0;
	sta->stats.tx_pkts_old = 0;
	sta->stats.tx_pps = 0;

	/* If fix rate was set for this station, save the configuration in case
	   we reconnect to this station within RC_CONFIG_DUR msec */
	if (cls_wifi_debugfs->rc_config[sta_idx] >= 0) {
		struct cls_wifi_rc_config_save *rc_cfg;
		rc_cfg = kmalloc(sizeof(*rc_cfg), GFP_KERNEL);
		if (rc_cfg) {
			rc_cfg->rate = cls_wifi_debugfs->rc_config[sta_idx];
			rc_cfg->timestamp = jiffies;
			memcpy(rc_cfg->mac_addr, cls_wifi_sta_addr(sta), ETH_ALEN);
			list_add_tail(&rc_cfg->list, &cls_wifi_debugfs->rc_config_save);
		}
	}

	sta->twt_ind.sta_idx = CLS_WIFI_INVALID_STA;
}

static bool _cls_wifi_sta_work(struct work_struct *ws)
{
	struct cls_wifi_debugfs *cls_wifi_debugfs = container_of(ws, struct cls_wifi_debugfs, sta_work);
	struct cls_wifi_hw *cls_wifi_hw = container_of(cls_wifi_debugfs, struct cls_wifi_hw, debugfs);
	struct cls_wifi_sta *sta;
	uint16_t sta_idx;

	sta_idx = cls_wifi_debugfs->sta_idx[cls_wifi_debugfs->sta_idx_rd];
	if (sta_idx == CLS_WIFI_INVALID_STA)
		return false;
	cls_wifi_debugfs->sta_idx[cls_wifi_debugfs->sta_idx_rd] = CLS_WIFI_INVALID_STA;
	cls_wifi_debugfs->sta_idx_rd++;
	if (cls_wifi_debugfs->sta_idx_rd >= STA_DEBUGFS_MAX)
		cls_wifi_debugfs->sta_idx_rd = 0;

	if (cls_wifi_debugfs->dir_sta == NULL)
		return true;

	if (cls_wifi_debugfs->dir_sta[sta_idx] == NULL) {
		sta = cls_wifi_get_sta(cls_wifi_hw, sta_idx, NULL, false);
		if (!sta)
			dev_err(cls_wifi_hw->dev,
					"Cannot get sta %d when registering debugfs\n", sta_idx);
		else
			_cls_wifi_dbgfs_register_sta(cls_wifi_debugfs, sta);
	} else {
		sta = cls_wifi_get_sta(cls_wifi_hw, sta_idx, NULL, true);
		_cls_wifi_dbgfs_unregister_sta(cls_wifi_debugfs, sta, sta_idx);
	}

	return true;
}

static void cls_wifi_sta_work(struct work_struct *ws)
{
	while (_cls_wifi_sta_work(ws))
		;
}

void cls_wifi_sta_deinit(struct cls_wifi_sta *sta)
{
	struct cls_sta_snr_info *_snr_list;
	struct cls_sta_snr_info *free_snr_list;
	struct cls_sta_snr_info *node;

	cancel_work_sync(&sta->sta_snr_work);
	spin_lock_bh(&sta->snr_lock);
	_snr_list = sta->snr_list;
	sta->snr_list = NULL;
	sta->snr_list_last = NULL;
	free_snr_list = sta->free_list;
	sta->free_list = NULL;
	sta->free_cnt = 0;
	spin_unlock_bh(&sta->snr_lock);

	while (free_snr_list != NULL) {
		node = free_snr_list;
		free_snr_list = node->next;
		if(node)
			kfree(node);
	}

	while (_snr_list != NULL) {
		node = _snr_list;
		_snr_list = node->next;
		if(node)
			kfree(node);
	}

	if (sta->snrlog != NULL) {
		filp_close(sta->snrlog, NULL);
		sta->snrlog = NULL;
	}

	if (sta->csilog != NULL) {
		filp_close(sta->csilog, NULL);
		sta->csilog = NULL;
	}
}

#define SNR_WR_BUF_SIZE  512U
static void cls_wifi_sta_snr_work(struct work_struct *wss)
{
	struct cls_wifi_sta *sta = container_of(wss, struct cls_wifi_sta, sta_snr_work);
	struct cls_sta_snr_info *_snr_list;
	struct cls_sta_snr_info *free_snr_list = NULL,*free_snr_last = NULL;
	unsigned int free_cnt = 0, list_free_cnt = 0;
	struct cls_sta_snr_info *node;
	int buf_size = 0;
	u8 buf[SNR_WR_BUF_SIZE];
	int i;

	if (sta->snrlog == NULL)
	{
		snprintf(buf, SNR_WR_BUF_SIZE,"/tmp/sta_snr_%04x_%04x_%04x",sta->mac_addr[0],sta->mac_addr[1],sta->mac_addr[2]);
		sta->snrlog = filp_open(buf, O_RDWR | O_CREAT | O_TRUNC, 0666);
		if (IS_ERR(sta->snrlog)) {
			pr_warn("Open target file fail: %s\n", buf);
			return;
		}
		pr_warn("Open target file succeeds: %s\n", buf);
	}

	spin_lock_bh(&sta->snr_lock);
	_snr_list = sta->snr_list;
	sta->snr_list = NULL;
	sta->snr_list_last = NULL;
	list_free_cnt = sta->free_cnt;
	spin_unlock_bh(&sta->snr_lock);

	if(_snr_list == NULL)
		return;

	while(1)
	{
		if (_snr_list == NULL) {
			spin_lock_bh(&sta->snr_lock);
			_snr_list = sta->snr_list;
			sta->snr_list = NULL;
			spin_unlock_bh(&sta->snr_lock);
		}

		if (_snr_list == NULL)
			break;

		node = _snr_list;
		_snr_list = node->next;

		buf_size = snprintf(&buf[buf_size], SNR_WR_BUF_SIZE - buf_size,"format: %04x,nss:%d,mcs:%02d,gi:%d,bw:%d:",
				node->format, node->nss, node->mcs, node->gi, node->bw);
		for(i = 0; i < 16; i++)
			buf_size += snprintf(&buf[buf_size],SNR_WR_BUF_SIZE - buf_size, " 0x%02x 0x%02x",
					node->snr[i] & 0xFF,(node->snr[i] >> 8)&0xFF);
		buf_size += snprintf(&buf[buf_size], SNR_WR_BUF_SIZE - buf_size, "\n");

		///pr_warn("%s\n", buf);

		kernel_write(sta->snrlog, buf, buf_size, &sta->snrlog->f_pos);
		buf_size = 0;

		if ((list_free_cnt + free_cnt) > 64) {
			kfree(node);
			node = NULL;
		} else {
			free_cnt++;
			node->next = NULL;
			if(free_snr_last == NULL) {
				free_snr_last = node;
				free_snr_list = node;
			} else {
				free_snr_last->next = node;
				free_snr_last = node;
			}
		}
	}

	if (free_snr_list) {
		spin_lock_bh(&sta->snr_lock);
		free_snr_last->next = sta->free_list;
		sta->free_list = free_snr_list;
		sta->free_cnt += free_cnt;
		spin_unlock_bh(&sta->snr_lock);
	}
}

int cls_wifi_sta_snr_record(struct cls_wifi_hw *cls_wifi_hw,struct cls_wifi_sta *sta,u16 *snr,u8 format,u8 nss,u8 mcs,u8 gi,u8 bw)
{
	struct cls_sta_snr_info *node = NULL;

	if (sta == NULL)
		return -1;

	if (!(sta->snr_format_type >= 0))
		return 0;

	if ((sta->snr_format_type <= 0xF) && (sta->snr_format_type != format))
		return 0;

	if (sta->free_cnt) {
		spin_lock(&sta->snr_lock);
		sta->free_cnt--;
		node = sta->free_list;
		sta->free_list = node->next;
		node->next = NULL;
		spin_unlock(&sta->snr_lock);
	}

	if(node == NULL) {
		node = kzalloc(sizeof(struct cls_sta_snr_info), GFP_KERNEL);
	}

	if (node == NULL) {
		return -1;
	}

	node->format = format;
	node->nss = nss;
	node->mcs = mcs;
	node->gi = gi;
	node->bw = bw;

	memcpy(node->snr, snr, 32);
	node->next = NULL;

	spin_lock(&sta->snr_lock);
	if(sta->snr_list == NULL) {
		sta->snr_list = node;
		sta->snr_list_last = node;
	} else {
		sta->snr_list_last->next = node;
		sta->snr_list_last = node;
	}
	spin_unlock(&sta->snr_lock);

	///schedule_work(&sta->sta_snr_work);
	queue_work(cls_wifi_hw->snr_workqueue, &sta->sta_snr_work);

	return 0;
}

void _cls_wifi_dbgfs_sta_write(struct cls_wifi_debugfs *cls_wifi_debugfs, uint8_t sta_idx)
{
	struct cls_wifi_hw *cls_wifi_hw = container_of(cls_wifi_debugfs, struct cls_wifi_hw, debugfs);
	if (cls_wifi_debugfs->unregistering)
		return;

	if (sta_idx >= hw_remote_sta_max(cls_wifi_hw)) {
		pr_err("Invalid sta index while (un)registering debugfs: %d\n",
			   sta_idx);
		return;
	}

	if (CLS_WIFI_INVALID_STA == cls_wifi_debugfs->sta_idx[cls_wifi_debugfs->sta_idx_wr]) {
		cls_wifi_debugfs->sta_idx[cls_wifi_debugfs->sta_idx_wr] = sta_idx;
		cls_wifi_debugfs->sta_idx_wr++;
		if (cls_wifi_debugfs->sta_idx_wr >= STA_DEBUGFS_MAX)
			cls_wifi_debugfs->sta_idx_wr = 0;
	} else
		pr_err("(un)registering debugfs: %d failed\n", sta_idx);

	schedule_work(&cls_wifi_debugfs->sta_work);
}

void cls_wifi_dbgfs_unregister_sta(struct cls_wifi_hw *cls_wifi_hw,
							   struct cls_wifi_sta *sta)
{
	_cls_wifi_dbgfs_sta_write(&cls_wifi_hw->debugfs, sta->sta_idx);
	cls_wifi_sta_deinit(sta);
}

void cls_wifi_dbgfs_register_sta(struct cls_wifi_hw *cls_wifi_hw,
							 struct cls_wifi_sta *sta)
{
	if (sta != NULL) {
		sta->snr_format_type = -1;
		sta->snr_list = NULL;
		sta->snr_list_last = NULL;
		sta->free_list = NULL;
		sta->free_cnt = 0;
		sta->snrlog = NULL;
		sta->csilog = NULL;
		spin_lock_init(&sta->snr_lock);
		INIT_WORK(&sta->sta_snr_work, cls_wifi_sta_snr_work);
	}
	_cls_wifi_dbgfs_sta_write(&cls_wifi_hw->debugfs, sta->sta_idx);
}

#ifdef CONFIG_CLS_MSGQ_TEST
static struct dentry *cls_wifi_msgq_dir;
void cls_wifi_msgq_dbgfs_register(struct cls_wifi_hw *cls_wifi_hw)
{
	cls_wifi_msgq_dir = debugfs_create_dir("msgq", NULL);
	if (!cls_wifi_msgq_dir)
		pr_warn("-----create cls_wifi_msgq_dir failed-----\n");
	DEBUGFS_ADD_FILE(msgq, cls_wifi_msgq_dir, DBGFS_WR);
	pr_warn("-----create cls_wifi_msgq_dir OK-----\n");
	return;
err:
	cls_wifi_msgq_dbgfs_unregister();
}
EXPORT_SYMBOL(cls_wifi_msgq_dbgfs_register);

void  cls_wifi_msgq_dbgfs_unregister(void)
{
	debugfs_remove_recursive(cls_wifi_msgq_dir);
	cls_wifi_msgq_dir = NULL;
}
EXPORT_SYMBOL(cls_wifi_msgq_dbgfs_unregister);
#endif

int cls_wifi_dbgfs_prepare(struct cls_wifi_hw *cls_wifi_hw)
{
	uint32_t sta_max = hw_remote_sta_max(cls_wifi_hw);
	uint16_t i;

	cls_wifi_hw->debugfs.dir_sta = kmalloc(sta_max * sizeof(struct dentry *), GFP_KERNEL);
	if (!cls_wifi_hw->debugfs.dir_sta)
		return -1;
	for (i=0; i < sta_max; i++)
	{
		cls_wifi_hw->debugfs.dir_sta[i] = NULL;
	}


	cls_wifi_hw->debugfs.dir_rc_sta = kmalloc(sta_max * sizeof(struct dentry *), GFP_KERNEL);
	if (!cls_wifi_hw->debugfs.dir_rc_sta)
		return -1;
	for (i=0; i < sta_max; i++)
	{
		cls_wifi_hw->debugfs.dir_rc_sta[i] = NULL;
	}

	cls_wifi_hw->debugfs.rc_config = kmalloc(sta_max * sizeof(u32), GFP_KERNEL);
	if (!cls_wifi_hw->debugfs.rc_config)
		return -1;

	cls_wifi_hw->debugfs.dir_twt_sta = kmalloc(sta_max * sizeof(struct dentry *), GFP_KERNEL);
	if (!cls_wifi_hw->debugfs.dir_twt_sta)
		return -1;
	for (i=0; i < sta_max; i++)
	{
		cls_wifi_hw->debugfs.dir_twt_sta[i] = NULL;
	}

	return 0;
}

void cls_wifi_dbgfs_free(struct cls_wifi_hw *cls_wifi_hw)
{
	if (cls_wifi_hw->debugfs.dir_sta)
		kfree(cls_wifi_hw->debugfs.dir_sta);

	if (cls_wifi_hw->debugfs.dir_rc_sta)
		kfree(cls_wifi_hw->debugfs.dir_rc_sta);

	if (cls_wifi_hw->debugfs.rc_config)
		kfree(cls_wifi_hw->debugfs.rc_config);

	if (cls_wifi_hw->debugfs.dir_twt_sta)
		kfree(cls_wifi_hw->debugfs.dir_twt_sta);
}

int cls_wifi_dbgfs_symlink_create(struct cls_wifi_hw *cls_wifi_hw)
{
	struct dentry *dir_link;
	struct dentry *phyd = cls_wifi_hw->wiphy->debugfsdir;
	struct cls_wifi_debugfs *cls_wifi_debugfs = &cls_wifi_hw->debugfs;

	if (cls_wifi_hw->radio_idx == 0) {
		dir_link = debugfs_lookup("phy0", phyd->d_parent);
		if (!dir_link) {
			dir_link = debugfs_create_symlink("phy0", phyd->d_parent , wiphy_name(cls_wifi_hw->wiphy));
			if (!dir_link)
				return -ENOMEM;
			cls_wifi_debugfs->dir_link = dir_link;
		}
	} else if (cls_wifi_hw->radio_idx == 1) {
		dir_link = debugfs_lookup("phy1", phyd->d_parent);
		if (!dir_link)
		{
			dir_link = debugfs_create_symlink("phy1", phyd->d_parent , wiphy_name(cls_wifi_hw->wiphy));
			if (!dir_link)
				return -ENOMEM;
			cls_wifi_debugfs->dir_link = dir_link;
		}
	}

	return 0;
}

int cls_wifi_dbgfs_register(struct cls_wifi_hw *cls_wifi_hw, const char *name)
{
	struct dentry *phyd = cls_wifi_hw->wiphy->debugfsdir;
	struct cls_wifi_debugfs *cls_wifi_debugfs = &cls_wifi_hw->debugfs;
	struct dentry *dir_drv, *dir_diags, *dir_stas;
	unsigned long i;

	if (cls_wifi_dbgfs_symlink_create(cls_wifi_hw))
		return -ENOMEM;

	if (!(dir_drv = debugfs_create_dir(name, phyd)))
		return -ENOMEM;

	cls_wifi_debugfs->dir = dir_drv;

	if (!(dir_stas = debugfs_create_dir("stations", dir_drv)))
		return -ENOMEM;

	cls_wifi_debugfs->dir_stas = dir_stas;
	cls_wifi_debugfs->unregistering = false;

	if (!(dir_diags = debugfs_create_dir("diags", dir_drv)))
		goto err;

	INIT_WORK(&cls_wifi_debugfs->sta_work, cls_wifi_sta_work);
	INIT_LIST_HEAD(&cls_wifi_debugfs->rc_config_save);
	for (i = 0; i < STA_DEBUGFS_MAX; i++)
		cls_wifi_debugfs->sta_idx[i] = CLS_WIFI_INVALID_STA;
	cls_wifi_debugfs->sta_idx_rd = 0;
	cls_wifi_debugfs->sta_idx_wr = 0;

	DEBUGFS_ADD_U32(tcp_pacing_shift, dir_drv, &cls_wifi_hw->tcp_pacing_shift,
					DBGFS_RW);
	DEBUGFS_ADD_FILE(stats, dir_drv, DBGFS_RW);
	DEBUGFS_ADD_FILE(sys_stats, dir_drv, DBGFS_RO);
	DEBUGFS_ADD_FILE(version, dir_drv, DBGFS_RO);
	DEBUGFS_ADD_FILE(hw_plat, dir_drv, DBGFS_RO);
	DEBUGFS_ADD_FILE(rxdesc, dir_drv, DBGFS_RO);
	DEBUGFS_ADD_FILE(ipc_stats, dir_drv, DBGFS_RO);
	DEBUGFS_ADD_FILE(txq, dir_drv, DBGFS_RO);
	DEBUGFS_ADD_FILE(acsinfo, dir_drv, DBGFS_RO);
	DEBUGFS_ADD_FILE(msg, dir_drv, DBGFS_WR);
	DEBUGFS_ADD_FILE(ampdu, dir_drv, DBGFS_WR);
	DEBUGFS_ADD_FILE(smm_idx, dir_drv, DBGFS_WR);
	DEBUGFS_ADD_FILE(clicmd, dir_drv, DBGFS_RW);
	DEBUGFS_ADD_FILE(pct_stat, dir_drv, DBGFS_WR);
	DEBUGFS_ADD_FILE(profile_stat, dir_drv, DBGFS_WR);
	DEBUGFS_ADD_FILE(ampdu_prot, dir_drv, DBGFS_WR);
	DEBUGFS_ADD_FILE(txop_en, dir_drv, DBGFS_WR);
	DEBUGFS_ADD_FILE(log_to_uart, dir_drv, DBGFS_WR);
#ifdef CLS_WIFI_DBGCNT_HOST
	DEBUGFS_ADD_FILE(reg_record, dir_drv, DBGFS_RW);
	DEBUGFS_ADD_FILE(rc_record, dir_drv, DBGFS_RW);
#endif
	DEBUGFS_ADD_FILE(fp, dir_drv, DBGFS_WR);
	DEBUGFS_ADD_FILE(msgq, dir_drv, DBGFS_WR);
	DEBUGFS_ADD_FILE(dbgcnt, dir_drv, DBGFS_RO);
	DEBUGFS_ADD_FILE(command, dir_drv, DBGFS_WR);
	DEBUGFS_ADD_FILE(mib, dir_drv, DBGFS_RO);
	DEBUGFS_ADD_FILE(dfx, dir_drv, DBGFS_RW);
	DEBUGFS_ADD_FILE(phydfx, dir_drv, DBGFS_RO);
	DEBUGFS_ADD_FILE(dpd_timer_interval, dir_drv, DBGFS_RW);
	DEBUGFS_ADD_FILE(enable_dpd_timer, dir_drv, DBGFS_RW);
	DEBUGFS_ADD_FILE(heartbeat_enable, dir_drv, DBGFS_RW);
	DEBUGFS_ADD_FILE(reload_agcram, dir_drv, DBGFS_WR);
	DEBUGFS_ADD_FILE(wfa_wmm, dir_drv, DBGFS_WR);
	DEBUGFS_ADD_FILE(csi, dir_drv, DBGFS_RW);
	DEBUGFS_ADD_FILE(fileop, dir_drv, DBGFS_RW);
	DEBUGFS_ADD_FILE(atf, dir_drv, DBGFS_RW);
	DEBUGFS_ADD_FILE(cu, dir_drv, DBGFS_RW);
	DEBUGFS_ADD_FILE(mem, dir_drv, DBGFS_RW);
	DEBUGFS_ADD_FILE(dpd_wmac_tx, dir_drv, DBGFS_RW);
	DEBUGFS_ADD_FILE(bfparam, dir_drv, DBGFS_RW);
	DEBUGFS_ADD_FILE(tsensor, dir_drv, DBGFS_RO);
	DEBUGFS_ADD_FILE(power_table, dir_drv, DBGFS_RW);
	DEBUGFS_ADD_FILE(pppc, dir_drv, DBGFS_RW);
	DEBUGFS_ADD_FILE(trig_dump_wmac, dir_drv, DBGFS_RW);
	DEBUGFS_ADD_FILE(op_mode, dir_drv, DBGFS_RW);
	DEBUGFS_ADD_FILE(rts_cts_dbg, dir_drv, DBGFS_RW);
	DEBUGFS_ADD_FILE(tx_rx_loop_online_en, dir_drv, DBGFS_WR);
	DEBUGFS_ADD_FILE(m3k_boc_reg, dir_drv, DBGFS_RW);
	DEBUGFS_ADD_FILE(puncture, dir_drv, DBGFS_RW);
	DEBUGFS_ADD_FILE(edma_info, dir_drv, DBGFS_RO);
	DEBUGFS_ADD_FILE(amsdu_maxnb, dir_drv, DBGFS_WR);
	DEBUGFS_ADD_FILE(scan_ext, dir_drv, DBGFS_WR);
#ifdef CFG_PCIE_SHM
	DEBUGFS_ADD_FILE(pcie_shm, dir_drv, DBGFS_WR);
#endif

#ifdef CONFIG_CLS_WIFI_P2P_DEBUGFS
	{
		/* Create a p2p directory */
		struct dentry *dir_p2p;
		if (!(dir_p2p = debugfs_create_dir("p2p", dir_drv)))
			goto err;

		/* Add file allowing to control Opportunistic PS */
		DEBUGFS_ADD_FILE(oppps, dir_p2p, DBGFS_RO);
		/* Add file allowing to control Notice of Absence */
		DEBUGFS_ADD_FILE(noa, dir_p2p, DBGFS_RO);
	}
#endif /* CONFIG_CLS_WIFI_P2P_DEBUGFS */

	if (cls_wifi_dbgfs_register_fw_dump(cls_wifi_hw, dir_drv, dir_diags))
		goto err;
	DEBUGFS_ADD_FILE(fw_dbg, dir_diags, DBGFS_RW);

#if defined(MERAK2000) && MERAK2000
	if (!cls_wifi_fw_trace_init(cls_wifi_hw, &cls_wifi_hw->debugfs.fw_trace)) {
		DEBUGFS_ADD_FILE(fw_trace, dir_diags, DBGFS_RW);
		if (cls_wifi_hw->debugfs.fw_trace.buf.nb_compo)
			DEBUGFS_ADD_FILE(fw_trace_level, dir_diags, DBGFS_RW);

	} else {
		cls_wifi_debugfs->fw_trace.buf.data = NULL;
	}
#endif

#ifdef CONFIG_CLS_WIFI_RADAR
	{
		struct dentry *dir_radar, *dir_sec;
		if (!(dir_radar = debugfs_create_dir("radar", dir_drv)))
			goto err;

		DEBUGFS_ADD_FILE(pulses_prim, dir_radar, DBGFS_RO);
		DEBUGFS_ADD_FILE(detected,	dir_radar, DBGFS_RO);
		DEBUGFS_ADD_FILE(enable,	  dir_radar, DBGFS_RO);
		DEBUGFS_ADD_FILE(rd_det_times,	dir_radar, DBGFS_RW);
		DEBUGFS_ADD_FILE(rd_rebuild_thres,	dir_radar, DBGFS_RW);
		DEBUGFS_ADD_FILE(rd_rebuild_num_thres,	dir_radar, DBGFS_RW);
		DEBUGFS_ADD_FILE(rd_min_num_det,	dir_radar, DBGFS_RW);
#if defined(MERAK2000) && MERAK2000
		DEBUGFS_ADD_FILE(rd_agc_war,	dir_radar, DBGFS_RW);
#endif
		DEBUGFS_ADD_FILE(rd_reset,	dir_radar, DBGFS_WR);
		DEBUGFS_ADD_FILE(rd_force_cac,	dir_radar, DBGFS_RW);
		DEBUGFS_ADD_FILE(rd_sw_det,	dir_radar, DBGFS_WR);
		DEBUGFS_ADD_FILE(rd_debug,	dir_radar, DBGFS_RW);
		DEBUGFS_ADD_FILE(rd_max_num_thrd,	dir_radar, DBGFS_RW);
		DEBUGFS_ADD_FILE(rd_chan,	dir_radar, DBGFS_RW);
		DEBUGFS_ADD_FILE(rd_ratio,	dir_radar, DBGFS_RW);
		DEBUGFS_ADD_FILE(pulses_hist, dir_radar, DBGFS_RO);

		if (cls_wifi_hw->phy.cnt == 2) {
			DEBUGFS_ADD_FILE(pulses_sec, dir_radar, DBGFS_RO);

			if (!(dir_sec = debugfs_create_dir("sec", dir_radar)))
				goto err;

			DEBUGFS_ADD_FILE(band,	dir_sec, DBGFS_RW);
			DEBUGFS_ADD_FILE(type,	dir_sec, DBGFS_RW);
			DEBUGFS_ADD_FILE(prim20,  dir_sec, DBGFS_RW);
			DEBUGFS_ADD_FILE(center1, dir_sec, DBGFS_RW);
			DEBUGFS_ADD_FILE(center2, dir_sec, DBGFS_RW);
			DEBUGFS_ADD_FILE(set,	 dir_sec, DBGFS_RW);
		}
	}
#endif /* CONFIG_CLS_WIFI_RADAR */

#ifdef CONFIG_CLS_WIFI_HEMU_TX
	{
		struct dentry *dir_ul, *dir_dl;

		if (!(dir_ul = debugfs_create_dir("ul_ofdma", dir_drv)))
			goto err;

		DEBUGFS_ADD_FILE(ul_on, dir_ul, DBGFS_RW);
		DEBUGFS_ADD_FILE(ul_duration_max, dir_ul, DBGFS_RW);
		DEBUGFS_ADD_FILE(nss_max, dir_ul, DBGFS_RW);
		DEBUGFS_ADD_FILE(mcs, dir_ul, DBGFS_RW);
		DEBUGFS_ADD_FILE(fec_allowed, dir_ul, DBGFS_RW);
		DEBUGFS_ADD_FILE(ul_duration_force, dir_ul, DBGFS_RW);
		DEBUGFS_ADD_FILE(ul_duration, dir_ul, DBGFS_RW);
		DEBUGFS_ADD_FILE(tf_period, dir_ul, DBGFS_RW);
		DEBUGFS_ADD_FILE(gi_ltf_mode, dir_ul, DBGFS_RW);
		DEBUGFS_ADD_FILE(ul_user_num, dir_ul, DBGFS_RW);
		DEBUGFS_ADD_FILE(ul_bw, dir_ul, DBGFS_RW);
		DEBUGFS_ADD_FILE(ul_dbg_level, dir_ul, DBGFS_RW);
		DEBUGFS_ADD_FILE(ul_param, dir_ul, DBGFS_RW);

		if (!(dir_dl = debugfs_create_dir("dl_ofdma", dir_drv)))
			goto err;

		DEBUGFS_ADD_FILE(dl_on, dir_dl, DBGFS_RW);
		DEBUGFS_ADD_FILE(work_mode, dir_dl, DBGFS_RW);
		DEBUGFS_ADD_FILE_WITH_OPS(dl_nss_max, nss_max, dir_dl, DBGFS_RW);
		DEBUGFS_ADD_FILE_WITH_OPS(dl_mcs, mcs, dir_dl, DBGFS_RW);
		DEBUGFS_ADD_FILE_WITH_OPS(dl_fec_allowed, fec_allowed, dir_dl, DBGFS_RW);
		DEBUGFS_ADD_FILE(mu_type, dir_dl, DBGFS_RW);
		DEBUGFS_ADD_FILE(max_ampdu_subfrm, dir_dl, DBGFS_RW);
		DEBUGFS_ADD_FILE(ppdu_bw, dir_dl, DBGFS_RW);
		DEBUGFS_ADD_FILE(gi, dir_dl, DBGFS_RW);
		DEBUGFS_ADD_FILE(ltf_type, dir_dl, DBGFS_RW);
		DEBUGFS_ADD_FILE(mu_log_level, dir_dl, DBGFS_RW);
		DEBUGFS_ADD_FILE(mu_pkt_len, dir_dl, DBGFS_RW);
	}
#endif /* CONFIG_CLS_WIFI_HEMU_TX */

#ifdef CONFIG_DEBUG_IRF
	{
		struct dentry *dir_irf;

		if (!(dir_irf = debugfs_create_dir("irf", dir_drv)))
			goto err;

		DEBUGFS_ADD_FILE(irf_get_reg, dir_irf, DBGFS_WR);
		DEBUGFS_ADD_FILE(irf_set_reg, dir_irf, DBGFS_WR);
		DEBUGFS_ADD_FILE(irf_hw_init, dir_irf, DBGFS_WR);
		DEBUGFS_ADD_FILE(irf_smp_config, dir_irf, DBGFS_WR);
		DEBUGFS_ADD_FILE(irf_smp_start, dir_irf, DBGFS_WR);
		DEBUGFS_ADD_FILE(irf_send_config, dir_irf, DBGFS_WR);
		DEBUGFS_ADD_FILE(irf_send_start, dir_irf, DBGFS_WR);
		DEBUGFS_ADD_FILE(irf_send_stop, dir_irf, DBGFS_WR);
		DEBUGFS_ADD_FILE(irf_smp_lms, dir_irf, DBGFS_WR);
		DEBUGFS_ADD_FILE(irf_write_lms, dir_irf, DBGFS_WR);
		DEBUGFS_ADD_FILE(irf_cmd, dir_irf, DBGFS_WR);
		DEBUGFS_ADD_FILE(dif_drv_state, dir_irf, DBGFS_WR);
		DEBUGFS_ADD_FILE(dif_fw_state, dir_irf, DBGFS_WR);
		DEBUGFS_ADD_FILE(online_zif_en, dir_irf, DBGFS_WR);
		DEBUGFS_ADD_FILE(online_zif_cycle, dir_irf, DBGFS_WR);
		DEBUGFS_ADD_FILE(zif_temp_thres, dir_irf, DBGFS_WR);
		DEBUGFS_ADD_FILE(g_online_zif_en, dir_irf, DBGFS_WR);
		DEBUGFS_ADD_FILE(pwr_ctrl_en, dir_irf, DBGFS_WR);
		DEBUGFS_ADD_FILE(g_pwr_ctrl_en, dir_irf, DBGFS_WR);
		DEBUGFS_ADD_FILE(irf_cca_cs_config, dir_irf, DBGFS_RW);
		DEBUGFS_ADD_FILE(irf_cca_ed_config, dir_irf, DBGFS_RW);
		DEBUGFS_ADD_FILE(g_aci_det_en, dir_irf, DBGFS_RW);
		DEBUGFS_ADD_FILE(dif_aci_det_cycle, dir_irf, DBGFS_RW);
	}
#endif

	return 0;

err:
	cls_wifi_dbgfs_unregister(cls_wifi_hw);
	return -ENOMEM;
}

void cls_wifi_dbgfs_unregister(struct cls_wifi_hw *cls_wifi_hw)
{
	struct cls_wifi_debugfs *cls_wifi_debugfs = &cls_wifi_hw->debugfs;
	struct cls_wifi_rc_config_save *cfg, *next;

	if (!cls_wifi_hw->debugfs.dir)
		return;

	list_for_each_entry_safe(cfg, next, &cls_wifi_debugfs->rc_config_save, list) {
		list_del(&cfg->list);
		kfree(cfg);
	}

	spin_lock_bh(&cls_wifi_debugfs->umh_lock);
	cls_wifi_debugfs->unregistering = true;
	spin_unlock_bh(&cls_wifi_debugfs->umh_lock);
	cls_wifi_wait_um_helper(cls_wifi_hw);
#if defined(MERAK2000) && MERAK2000
	cls_wifi_fw_trace_deinit(&cls_wifi_hw->debugfs.fw_trace);
#endif
	cancel_work_sync(&cls_wifi_debugfs->sta_work);
	debugfs_remove_recursive(cls_wifi_hw->debugfs.dir);
	cls_wifi_hw->debugfs.dir = NULL;

	if (!cls_wifi_hw->debugfs.dir_link)
		return;
	debugfs_remove_recursive(cls_wifi_hw->debugfs.dir_link);
	cls_wifi_hw->debugfs.dir_link = NULL;

}

