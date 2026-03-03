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
#include "cls_wifi_msg_tx.h"
#ifdef CONFIG_CLS_WIFI_BFMER
#include "cls_wifi_bfmer.h"
#endif //(CONFIG_CLS_WIFI_BFMER)
#include "cls_wifi_compat.h"
#include "cls_wifi_cali_debugfs.h"
#include "cls_wifi_power_tbl.h"
#include "lmac_msg.h"
#endif
#ifdef __KERNEL__
extern uint32_t use_msgq;
const struct mac_addr mac_addr_bcst = {{0xFFFF, 0xFFFF, 0xFFFF}};

/* Default MAC Rx filters that can be changed by mac80211
 * (via the configure_filter() callback) */
#define CLS_WIFI_MAC80211_CHANGEABLE		(									   \
										 CLS_MAC_ACCEPT_BA_BIT				  | \
										 CLS_MAC_ACCEPT_BAR_BIT				 | \
										 CLS_MAC_ACCEPT_OTHER_DATA_FRAMES_BIT   | \
										 CLS_MAC_ACCEPT_PROBE_REQ_BIT		   | \
										 CLS_MAC_ACCEPT_PS_POLL_BIT			   \
										)

/* Default MAC Rx filters that cannot be changed by mac80211 */
#define CLS_WIFI_MAC80211_NOT_CHANGEABLE	(									   \
										 CLS_MAC_ACCEPT_QO_S_NULL_BIT		   | \
										 CLS_MAC_ACCEPT_Q_DATA_BIT			  | \
										 CLS_MAC_ACCEPT_DATA_BIT				| \
										 CLS_MAC_ACCEPT_OTHER_MGMT_FRAMES_BIT   | \
										 CLS_MAC_ACCEPT_MY_UNICAST_BIT		  | \
										 CLS_MAC_ACCEPT_BROADCAST_BIT		   | \
										 CLS_MAC_ACCEPT_BEACON_BIT			  | \
										 CLS_MAC_ACCEPT_PROBE_RESP_BIT			\
										)

/* Default MAC Rx filter */
#define CLS_WIFI_DEFAULT_RX_FILTER  (CLS_WIFI_MAC80211_CHANGEABLE | CLS_WIFI_MAC80211_NOT_CHANGEABLE)

const int bw2chnl[] = {
	[NL80211_CHAN_WIDTH_20_NOHT] = PHY_CHNL_BW_20,
	[NL80211_CHAN_WIDTH_20]	  = PHY_CHNL_BW_20,
	[NL80211_CHAN_WIDTH_40]	  = PHY_CHNL_BW_40,
	[NL80211_CHAN_WIDTH_80]	  = PHY_CHNL_BW_80,
	[NL80211_CHAN_WIDTH_160]	 = PHY_CHNL_BW_160,
	[NL80211_CHAN_WIDTH_80P80]   = PHY_CHNL_BW_80P80,
};

const int chnl2bw[] = {
	[PHY_CHNL_BW_20]	  = NL80211_CHAN_WIDTH_20,
	[PHY_CHNL_BW_40]	  = NL80211_CHAN_WIDTH_40,
	[PHY_CHNL_BW_80]	  = NL80211_CHAN_WIDTH_80,
	[PHY_CHNL_BW_160]	 = NL80211_CHAN_WIDTH_160,
	[PHY_CHNL_BW_80P80]   = NL80211_CHAN_WIDTH_80P80,
};
#endif
#ifdef __KERNEL__
/*****************************************************************************/
/*
 * Parse the ampdu density to retrieve the value in usec, according to the
 * values defined in ieee80211.h
 */
static inline u8 cls_wifi_ampdudensity2usec(u8 ampdudensity)
{
	switch (ampdudensity) {
	case IEEE80211_HT_MPDU_DENSITY_NONE:
		return 0;
		/* 1 microsecond is our granularity */
	case IEEE80211_HT_MPDU_DENSITY_0_25:
	case IEEE80211_HT_MPDU_DENSITY_0_5:
	case IEEE80211_HT_MPDU_DENSITY_1:
		return 1;
	case IEEE80211_HT_MPDU_DENSITY_2:
		return 2;
	case IEEE80211_HT_MPDU_DENSITY_4:
		return 4;
	case IEEE80211_HT_MPDU_DENSITY_8:
		return 8;
	case IEEE80211_HT_MPDU_DENSITY_16:
		return 16;
	default:
		return 0;
	}
}

static inline bool use_pairwise_key(struct cfg80211_crypto_settings *crypto)
{
	if ((crypto->cipher_group ==  WLAN_CIPHER_SUITE_WEP40) ||
		(crypto->cipher_group ==  WLAN_CIPHER_SUITE_WEP104))
		return false;

	return true;
}

static inline bool use_privacy(struct cfg80211_crypto_settings *crypto)
{
	return (crypto->cipher_group != 0);
}

static inline bool is_non_blocking_msg(int id)
{
	return ((id == MM_TIM_UPDATE_REQ) || (id == MM_RC_SET_RATE_REQ) ||
			(id == MM_BFMER_ENABLE_REQ) || (id == ME_TRAFFIC_IND_REQ) ||
			(id == TDLS_PEER_TRAFFIC_IND_REQ) ||
			(id == MESH_PATH_CREATE_REQ) || (id == MESH_PROXY_ADD_REQ) ||
			(id == SM_EXTERNAL_AUTH_REQUIRED_RSP || (id == MM_STA_RC_UPDATE_REQ)));
}

/**
 * copy_connect_ies -- Copy Association Elements in the the request buffer
 * send to the firmware
 *
 * @vif: Vif that received the connection request
 * @req: Connection request to send to the firmware
 * @sme: Connection info
 *
 * For driver that do not use userspace SME (like this one) the host connection
 * request doesn't explicitly mentions that the connection can use FT over the
 * air. if FT is possible, send the FT elements (as received in update_ft_ies callback)
 * to the firmware
 *
 * In all other cases simply copy the list povided by the user space in the
 * request buffer
 */
static void copy_connect_ies(struct cls_wifi_vif *vif, struct sm_connect_req *req,
							struct cfg80211_connect_params *sme)
{
	if ((sme->auth_type == NL80211_AUTHTYPE_FT) && !(vif->sta.flags & CLS_WIFI_STA_FT_OVER_DS))
	{
		const struct element *rsne, *fte, *mde;
		uint8_t *pos;
		rsne = cfg80211_find_elem(WLAN_EID_RSN, vif->sta.ft_assoc_ies,
									vif->sta.ft_assoc_ies_len);
		fte = cfg80211_find_elem(WLAN_EID_FAST_BSS_TRANSITION, vif->sta.ft_assoc_ies,
									vif->sta.ft_assoc_ies_len);
		mde = cfg80211_find_elem(WLAN_EID_MOBILITY_DOMAIN,
										 vif->sta.ft_assoc_ies, vif->sta.ft_assoc_ies_len);
		pos = (uint8_t *)req->ie_buf;

		// We can use FT over the air
		memcpy(&vif->sta.ft_target_ap, sme->bssid, ETH_ALEN);

		if (rsne) {
			memcpy(pos, rsne, sizeof(struct element) + rsne->datalen);
			pos += sizeof(struct element) + rsne->datalen;
		}
		memcpy(pos, mde, sizeof(struct element) + mde->datalen);
		pos += sizeof(struct element) + mde->datalen;
		if (fte) {
			memcpy(pos, fte, sizeof(struct element) + fte->datalen);
			pos += sizeof(struct element) + fte->datalen;
		}

		req->ie_len = pos - (uint8_t *)req->ie_buf;
	}
	else
	{
		memcpy(req->ie_buf, sme->ie, sme->ie_len);
		req->ie_len = sme->ie_len;
	}
}

/**
 * update_connect_req -- Return the length of the association request IEs
 *
 * @vif: Vif that received the connection request
 * @sme: Connection info
 *
 * Return the ft_ie_len in case of FT.
 * FT over the air is possible if:
 * - auth_type = AUTOMATIC (if already set to FT then it means FT over DS)
 * - already associated to a FT BSS
 * - Target Mobility domain is the same as the curent one
 *
 * If FT is not possible return ie length of the connection info
 */
static int update_connect_req(struct cls_wifi_vif *vif, struct cfg80211_connect_params *sme)
{
	if ((vif->sta.ap) &&
		(vif->sta.ft_assoc_ies) &&
		(sme->auth_type == NL80211_AUTHTYPE_AUTOMATIC))
	{
		const struct element *rsne, *fte, *mde, *mde_req;
		int ft_ie_len = 0;

		mde_req = cfg80211_find_elem(WLAN_EID_MOBILITY_DOMAIN,
									 sme->ie, sme->ie_len);
		mde = cfg80211_find_elem(WLAN_EID_MOBILITY_DOMAIN,
								 vif->sta.ft_assoc_ies, vif->sta.ft_assoc_ies_len);
		if (!mde || !mde_req ||
			memcmp(mde, mde_req, sizeof(struct element) + mde->datalen))
		{
			return sme->ie_len;
		}

		ft_ie_len += sizeof(struct element) + mde->datalen;

		rsne = cfg80211_find_elem(WLAN_EID_RSN, vif->sta.ft_assoc_ies,
									vif->sta.ft_assoc_ies_len);
		fte = cfg80211_find_elem(WLAN_EID_FAST_BSS_TRANSITION, vif->sta.ft_assoc_ies,
									vif->sta.ft_assoc_ies_len);

		if (rsne && fte)
		{
			ft_ie_len += 2 * sizeof(struct element) + rsne->datalen + fte->datalen;
			sme->auth_type = NL80211_AUTHTYPE_FT;
			return ft_ie_len;
		}
		else if (rsne || fte)
		{
			netdev_warn(vif->ndev, "Missing RSNE or FTE element, skip FT over air");
		}
		else
		{
			sme->auth_type = NL80211_AUTHTYPE_FT;
			return ft_ie_len;
		}
	}
	return sme->ie_len;
}

static inline u16_l get_chan_flags(uint32_t flags)
{
	u16_l chan_flags = 0;
	if (flags & IEEE80211_CHAN_NO_IR)
		chan_flags |= CHAN_NO_IR;
	if (flags & IEEE80211_CHAN_RADAR)
		chan_flags |= CHAN_RADAR;
	return chan_flags;
}

static inline s8_l chan_to_fw_pwr(int power)
{
	return power > 127 ? 127 : (s8_l)power;
}

static void cfg80211_to_cls_wifi_chan(const struct cfg80211_chan_def *chandef,
								  struct mac_chan_op *chan)
{
	chan->band = chandef->chan->band;
	chan->type = bw2chnl[chandef->width];
	chan->prim20_freq = chandef->chan->center_freq;
	chan->center1_freq = chandef->center_freq1;
	chan->center2_freq = chandef->center_freq2;
	chan->flags = get_chan_flags(chandef->chan->flags);
	chan->tx_power = chan_to_fw_pwr(chandef->chan->max_power);
}

static inline void limit_chan_bw(u8_l *bw, u16_l primary, u16_l *center1)
{
	int oft, new_oft = 10;

	if (*bw <= PHY_CHNL_BW_40)
		return;

	oft = *center1 - primary;
	*bw = PHY_CHNL_BW_40;

	if (oft < 0)
		new_oft = new_oft * -1;
	if (abs(oft) == 10 || abs(oft) == 50)
		new_oft = new_oft * -1;

	*center1 = primary + new_oft;
}

/**
 ******************************************************************************
 * @brief Allocate memory for a message
 *
 * This primitive allocates memory for a message that has to be sent. The memory
 * is allocated dynamically on the heap and the length of the variable parameter
 * structure has to be provided in order to allocate the correct size.
 *
 * Several additional parameters are provided which will be preset in the message
 * and which may be used internally to choose the kind of memory to allocate.
 *
 * The memory allocated will be automatically freed by the kernel, after the
 * pointer has been sent to ke_msg_send(). If the message is not sent, it must
 * be freed explicitly with ke_msg_free().
 *
 * Allocation failure is considered critical and should not happen.
 *
 * @param[in] id		Message identifier
 * @param[in] dest_id   Destination Task Identifier
 * @param[in] src_id	Source Task Identifier
 * @param[in] param_len Size of the message parameters to be allocated
 *
 * @return Pointer to the parameter member of the ke_msg. If the parameter
 *		 structure is empty, the pointer will point to the end of the message
 *		 and should not be used (except to retrieve the message pointer or to
 *		 send the message)
 ******************************************************************************
 */
static inline void *cls_wifi_msg_zalloc(lmac_msg_id_t const id,
									lmac_task_id_t const dest_id,
									lmac_task_id_t const src_id,
									uint16_t const param_len)
{
	struct lmac_msg *msg;
	gfp_t flags;

	if (is_non_blocking_msg(id) && in_softirq())
		flags = GFP_ATOMIC;
	else
		flags = GFP_KERNEL;

	msg = (struct lmac_msg *)kzalloc(sizeof(struct lmac_msg) + param_len,
									 flags);
	if (msg == NULL) {
		printk(KERN_CRIT "%s: msg allocation failed\n", __func__);
		return NULL;
	}

	msg->id = id;
	msg->dest_id = dest_id;
	msg->src_id = src_id;
	msg->param_len = param_len;

	return msg->param;
}

static void cls_wifi_msg_free(struct cls_wifi_hw *cls_wifi_hw, const void *msg_params)
{
	struct lmac_msg *msg = container_of((void *)msg_params,
										struct lmac_msg, param);

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Free the message */
	kfree(msg);
}

static void cls_wifi_send_msg_err_code(struct cls_wifi_hw *cls_wifi_hw,
				int reqcfm, struct cls_wifi_cmd *cmd)
{
	if (cmd->result) {
		printk(KERN_ERR "%s: radio %u, vif_started %d, flag 0x%lx send msg fail\n",
			__func__, cls_wifi_hw->radio_idx, cls_wifi_hw->vif_started, cls_wifi_hw->flags);
	}

	if (cmd->result == -ENOMEM) {
		printk(KERN_ERR "%s: reqid=%d, cfmid=%d, reqcfm=%d has too many cmds (%d) already queued\n",
			__func__, cmd->id, cmd->reqid, reqcfm, cls_wifi_hw->cmd_mgr.max_queue_sz);
	}

	if (cmd->result == -ETIMEDOUT) {
		printk(KERN_ERR "%s: reqid=%d, cfmid=%d, reqcfm=%d cmd timed-out, cmd queue crashed\n",
			__func__, cmd->id, cmd->reqid, reqcfm);
		cls_wifi_hw->cmd_mgr.crashdump(cmd);
	}
}

static int cls_wifi_send_msg(struct cls_wifi_hw *cls_wifi_hw, const void *msg_params,
						 int reqcfm, lmac_msg_id_t reqid, void *cfm)
{
	struct lmac_msg *msg;
	struct cls_wifi_cmd *cmd;
	bool nonblock;
	int ret;
	gfp_t flag = GFP_KERNEL;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	msg = container_of((void *)msg_params, struct lmac_msg, param);

	if ((cls_wifi_hw->cmd_mgr.state == CLS_WIFI_CMD_MGR_STATE_CRASHED) ||
		(cls_wifi_hw->heartbeat_uevent)) {
		kfree(msg);
		return -EPIPE;
	}

	if (!test_bit(CLS_WIFI_DEV_STARTED, &cls_wifi_hw->flags) && reqid != MM_PLAT_PARAM_CFM &&
		reqid != MM_RESET_CFM && reqid != MM_VERSION_CFM &&
		reqid != MM_START_CFM && reqid != MM_SET_IDLE_CFM &&
		reqid != ME_CONFIG_CFM && reqid != MM_SET_PS_MODE_CFM &&
		reqid != ME_CHAN_CONFIG_CFM && reqid != MM_UL_PARAMETERS_CFM &&
		reqid != MM_DL_PARAMETERS_CFM) {
		kfree(msg);
		return -EBUSY;
	} else if (!cls_wifi_hw->ipc_env) {
		printk(KERN_ERR "%s: radio %u ipc_env is NULL, reqid %u, vif_started %d, flag 0x%lx\n",
					__func__, cls_wifi_hw->radio_idx, reqid,
					cls_wifi_hw->vif_started, cls_wifi_hw->flags);
		kfree(msg);
		return -EBUSY;
	}

	nonblock = is_non_blocking_msg(msg->id);

#if CAL_DBG
	pr_debug("dev=%s, reqid=%d, cfmid=%d, reqcfm=%d\n", dev_name(cls_wifi_hw->dev),
			msg->id, reqid, reqcfm);
#endif

	if (nonblock || in_softirq() || in_irq())
		flag = GFP_ATOMIC;

	cmd = kzalloc(sizeof(struct cls_wifi_cmd), flag);
	cmd->result  = -EINTR;
	cmd->id	  = msg->id;
	cmd->reqid   = reqid;
	cmd->a2e_msg = msg;
	cmd->e2a_msg = cfm;
	if (nonblock)
		cmd->flags = CLS_WIFI_CMD_FLAG_NONBLOCK;
	if (reqcfm)
		cmd->flags |= CLS_WIFI_CMD_FLAG_REQ_CFM;
	ret = cls_wifi_hw->cmd_mgr.queue(&cls_wifi_hw->cmd_mgr, cmd);

	if (!ret)
		ret = cmd->result;

	// decode error code
	if (ret)
		cls_wifi_send_msg_err_code(cls_wifi_hw, reqcfm, cmd);

	if (!nonblock)
		kfree(cmd);

	return ret;
}
int cls_wifi_send_plat_param(struct cls_wifi_hw *cls_wifi_hw)
{
	struct mm_plat_param_req *param;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	param = cls_wifi_msg_zalloc(MM_PLAT_PARAM_REQ, TASK_MM, DRV_TASK_ID,
			sizeof(struct mm_plat_param_req));
	if (!param)
		return -ENOMEM;

	param->use_msgq = use_msgq;

	return cls_wifi_send_msg(cls_wifi_hw, param, 1, MM_PLAT_PARAM_CFM, NULL);
}
#endif
int cls_wifi_send_reset(struct cls_wifi_hw *cls_wifi_hw)
{
	void *void_param;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* RESET REQ has no parameter */
	void_param = cls_wifi_msg_zalloc(MM_RESET_REQ, TASK_MM, DRV_TASK_ID, 0);
	if (!void_param)
		return -ENOMEM;

	return cls_wifi_send_msg(cls_wifi_hw, void_param, 1, MM_RESET_CFM, NULL);
}

#ifdef __KERNEL__
int cls_wifi_send_wmm_lock(struct cls_wifi_hw *cls_wifi_hw, uint32_t lock_edca)
{
	struct dht_wmm_lock_req *param;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* RESET REQ has no parameter */
	param = cls_wifi_msg_zalloc(DHT_WMM_LOCK_REQ, TASK_DHT, DRV_TASK_ID,
					sizeof(struct dht_wmm_lock_req));
	if (!param)
		return -ENOMEM;

	param->enable = lock_edca;

	return cls_wifi_send_msg(cls_wifi_hw, param, 0, 0, NULL);
}

int cls_wifi_send_start(struct cls_wifi_hw *cls_wifi_hw)
{
	struct mm_start_req *start_req_param;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the START REQ message */
	start_req_param = cls_wifi_msg_zalloc(MM_START_REQ, TASK_MM, DRV_TASK_ID,
									  sizeof(struct mm_start_req));
	if (!start_req_param)
		return -ENOMEM;

	/* Set parameters for the START message */
	memcpy(&start_req_param->phy_cfg, &cls_wifi_hw->phy.cfg, sizeof(cls_wifi_hw->phy.cfg));
	start_req_param->uapsd_timeout = (u32_l)cls_wifi_hw->radio_params->uapsd_timeout;
	start_req_param->lp_clk_accuracy = (u16_l)cls_wifi_hw->radio_params->lp_clk_ppm;
	start_req_param->tx_timeout[AC_BK] = (u16_l)cls_wifi_hw->radio_params->tx_to_bk;
	start_req_param->tx_timeout[AC_BE] = (u16_l)cls_wifi_hw->radio_params->tx_to_be;
	start_req_param->tx_timeout[AC_VI] = (u16_l)cls_wifi_hw->radio_params->tx_to_vi;
	start_req_param->tx_timeout[AC_VO] = (u16_l)cls_wifi_hw->radio_params->tx_to_vo;
	start_req_param->rx_hostbuf_size = (u16_l)cls_wifi_hw->ipc_env->rxbuf_sz;

	/* Send the START REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, start_req_param, 1, MM_START_CFM, NULL);
}

int cls_wifi_send_version_req(struct cls_wifi_hw *cls_wifi_hw, struct mm_version_cfm *cfm)
{
	void *void_param;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* VERSION REQ has no parameter */
	void_param = cls_wifi_msg_zalloc(MM_VERSION_REQ, TASK_MM, DRV_TASK_ID, 0);
	if (!void_param)
		return -ENOMEM;

	return cls_wifi_send_msg(cls_wifi_hw, void_param, 1, MM_VERSION_CFM, cfm);
}

int cls_wifi_send_add_if(struct cls_wifi_hw *cls_wifi_hw, const unsigned char *mac,
					 enum nl80211_iftype iftype, bool p2p, struct mm_add_if_cfm *cfm)
{
	struct mm_add_if_req *add_if_req_param;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the ADD_IF_REQ message */
	add_if_req_param = cls_wifi_msg_zalloc(MM_ADD_IF_REQ, TASK_MM, DRV_TASK_ID,
									   sizeof(struct mm_add_if_req));
	if (!add_if_req_param)
		return -ENOMEM;

	/* Set parameters for the ADD_IF_REQ message */
	memcpy(&(add_if_req_param->addr.array[0]), mac, ETH_ALEN);
	switch (iftype) {
	case NL80211_IFTYPE_P2P_CLIENT:
		add_if_req_param->p2p = true;
		fallthrough;
	case NL80211_IFTYPE_STATION:
		add_if_req_param->type = MM_STA;
		break;

	case NL80211_IFTYPE_ADHOC:
		add_if_req_param->type = MM_IBSS;
		break;

	case NL80211_IFTYPE_P2P_GO:
		add_if_req_param->p2p = true;
		fallthrough;
	case NL80211_IFTYPE_AP:
		add_if_req_param->type = MM_AP;
		break;
	case NL80211_IFTYPE_MESH_POINT:
		add_if_req_param->type = MM_MESH_POINT;
		break;
	case NL80211_IFTYPE_AP_VLAN:
		return -1;
	case NL80211_IFTYPE_MONITOR:
		add_if_req_param->type = MM_MONITOR;
		break;
	default:
		add_if_req_param->type = MM_STA;
		break;
	}

	add_if_req_param->ba_buf_size = cls_wifi_hw->radio_params->ba_buffer_size;

	/* Send the ADD_IF_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, add_if_req_param, 1, MM_ADD_IF_CFM, cfm);
}

int cls_wifi_send_remove_if(struct cls_wifi_hw *cls_wifi_hw, u8 vif_index)
{
	struct mm_remove_if_req *remove_if_req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the MM_REMOVE_IF_REQ message */
	remove_if_req = cls_wifi_msg_zalloc(MM_REMOVE_IF_REQ, TASK_MM, DRV_TASK_ID,
									sizeof(struct mm_remove_if_req));
	if (!remove_if_req)
		return -ENOMEM;

	/* Set parameters for the MM_REMOVE_IF_REQ message */
	remove_if_req->inst_nbr = vif_index;

	/* Send the MM_REMOVE_IF_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, remove_if_req, 1, MM_REMOVE_IF_CFM, NULL);
}

int cls_wifi_send_set_channel(struct cls_wifi_hw *cls_wifi_hw, int phy_idx,
						  struct mm_set_channel_cfm *cfm)
{
	struct mm_set_channel_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (phy_idx >= cls_wifi_hw->phy.cnt)
		return -ENOTSUPP;

	req = cls_wifi_msg_zalloc(MM_SET_CHANNEL_REQ, TASK_MM, DRV_TASK_ID,
						  sizeof(struct mm_set_channel_req));
	if (!req)
		return -ENOMEM;

	if (phy_idx == 0) {
		/* On FULLMAC only setting channel of secondary chain */
		wiphy_err(cls_wifi_hw->wiphy, "Trying to set channel of primary chain");
		return 0;
	} else {
		req->chan = cls_wifi_hw->phy.sec_chan;
	}

	req->index = phy_idx;

	if (cls_wifi_hw->phy.limit_bw)
		limit_chan_bw(&req->chan.type, req->chan.prim20_freq, &req->chan.center1_freq);

	CLS_WIFI_DBG("mac80211:   freq=(c1:%d - c2:%d)/width=%d - band=%d\n"
			 "   hw(%d): prim20=%d(c1:%d - c2:%d)/ type=%d - band=%d\n",
			 req->chan.center1_freq, req->chan.center2_freq,
			 cls_wifi_hw->phy.limit_bw, req->chan.band,
			 phy_idx, req->chan.prim20_freq, req->chan.center1_freq,
			 req->chan.center2_freq, req->chan.type, req->chan.band);

	/* Send the MM_SET_CHANNEL_REQ REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, MM_SET_CHANNEL_CFM, cfm);
}


int cls_wifi_send_key_add(struct cls_wifi_hw *cls_wifi_hw, u8 vif_idx, u16 sta_idx, bool pairwise,
					  u8 *key, u8 key_len, u8 key_idx, u8 cipher_suite,
					  struct mm_key_add_cfm *cfm)
{
	struct mm_key_add_req *key_add_req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the MM_KEY_ADD_REQ message */
	key_add_req = cls_wifi_msg_zalloc(MM_KEY_ADD_REQ, TASK_MM, DRV_TASK_ID,
								  sizeof(struct mm_key_add_req));
	if (!key_add_req)
		return -ENOMEM;

	/* Set parameters for the MM_KEY_ADD_REQ message */
	key_add_req->sta_idx = sta_idx;
	key_add_req->key_idx = key_idx;
	key_add_req->pairwise = pairwise;
	key_add_req->inst_nbr = vif_idx;
	key_add_req->key.length = key_len;
	memcpy(&(key_add_req->key.array[0]), key, key_len);

	key_add_req->cipher_suite = cipher_suite;

	CLS_WIFI_DBG("%s: sta_idx:%d key_idx:%d inst_nbr:%d cipher:%d key_len:%d\n", __func__,
			 key_add_req->sta_idx, key_add_req->key_idx, key_add_req->inst_nbr,
			 key_add_req->cipher_suite, key_add_req->key.length);
#if defined(CONFIG_CLS_WIFI_DBG) || defined(CONFIG_DYNAMIC_DEBUG)
	print_hex_dump_bytes("key: ", DUMP_PREFIX_OFFSET, key_add_req->key.array, key_add_req->key.length);
#endif

	/* Send the MM_KEY_ADD_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, key_add_req, 1, MM_KEY_ADD_CFM, cfm);
}

int cls_wifi_send_key_del(struct cls_wifi_hw *cls_wifi_hw, uint8_t hw_key_idx)
{
	struct mm_key_del_req *key_del_req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the MM_KEY_DEL_REQ message */
	key_del_req = cls_wifi_msg_zalloc(MM_KEY_DEL_REQ, TASK_MM, DRV_TASK_ID,
								  sizeof(struct mm_key_del_req));
	if (!key_del_req)
		return -ENOMEM;

	/* Set parameters for the MM_KEY_DEL_REQ message */
	key_del_req->hw_key_idx = hw_key_idx;

	/* Send the MM_KEY_DEL_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, key_del_req, 1, MM_KEY_DEL_CFM, NULL);
}

int cls_wifi_send_bcn_change(struct cls_wifi_hw *cls_wifi_hw, u8 vif_idx, u32 bcn_addr,
						 u16 bcn_len, u16 tim_oft, u16 tim_len, u16 *csa_oft, u16 cca_oft)
{
	struct mm_bcn_change_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the MM_BCN_CHANGE_REQ message */
	req = cls_wifi_msg_zalloc(MM_BCN_CHANGE_REQ, TASK_MM, DRV_TASK_ID,
						  sizeof(struct mm_bcn_change_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the MM_BCN_CHANGE_REQ message */
	req->bcn_ptr = bcn_addr;
	req->bcn_len = bcn_len;
	req->tim_oft = tim_oft;
	req->tim_len = tim_len;
	req->inst_nbr = vif_idx;
	req->cca_oft = cca_oft;

	if (csa_oft) {
		int i;
		for (i = 0; i < BCN_MAX_CSA_CPT; i++) {
			req->csa_oft[i] = csa_oft[i];
		}
	}

	/* Send the MM_BCN_CHANGE_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, MM_BCN_CHANGE_CFM, NULL);
}

int cls_wifi_send_roc(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *vif,
				  struct ieee80211_channel *chan, unsigned  int duration)
{
	struct mm_remain_on_channel_req *req;
	struct cfg80211_chan_def chandef;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Create channel definition structure */
	cfg80211_chandef_create(&chandef, chan, NL80211_CHAN_NO_HT);

	/* Build the MM_REMAIN_ON_CHANNEL_REQ message */
	req = cls_wifi_msg_zalloc(MM_REMAIN_ON_CHANNEL_REQ, TASK_MM, DRV_TASK_ID,
						  sizeof(struct mm_remain_on_channel_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the MM_REMAIN_ON_CHANNEL_REQ message */
	req->op_code	  = MM_ROC_OP_START;
	req->vif_index	= vif->vif_index;
	req->duration_ms  = duration;
	cfg80211_to_cls_wifi_chan(&chandef, &req->chan);

	/* Send the MM_REMAIN_ON_CHANNEL_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, MM_REMAIN_ON_CHANNEL_CFM, NULL);
}

int cls_wifi_send_cancel_roc(struct cls_wifi_hw *cls_wifi_hw)
{
	struct mm_remain_on_channel_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the MM_REMAIN_ON_CHANNEL_REQ message */
	req = cls_wifi_msg_zalloc(MM_REMAIN_ON_CHANNEL_REQ, TASK_MM, DRV_TASK_ID,
						  sizeof(struct mm_remain_on_channel_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the MM_REMAIN_ON_CHANNEL_REQ message */
	req->op_code = MM_ROC_OP_CANCEL;

	/* Send the MM_REMAIN_ON_CHANNEL_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_set_power(struct cls_wifi_hw *cls_wifi_hw, u8 vif_idx, s8 pwr,
						struct mm_set_power_cfm *cfm)
{
	struct mm_set_power_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the MM_SET_POWER_REQ message */
	req = cls_wifi_msg_zalloc(MM_SET_POWER_REQ, TASK_MM, DRV_TASK_ID,
						  sizeof(struct mm_set_power_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the MM_SET_POWER_REQ message */
	req->inst_nbr = vif_idx;
	req->power = pwr;

	/* Send the MM_SET_POWER_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, MM_SET_POWER_CFM, cfm);
}

int cls_wifi_send_get_power(struct cls_wifi_hw *cls_wifi_hw, u8 vif_idx,
				 struct mm_get_power_cfm *cfm)
{
	struct mm_get_power_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the MM_GET_POWER_REQ message */
	req = cls_wifi_msg_zalloc(MM_GET_POWER_REQ, TASK_MM, DRV_TASK_ID,
						  sizeof(struct mm_get_power_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the MM_GET_POWER_REQ message */
	req->inst_nbr = vif_idx;

	/* Send the MM_GET_POWER_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, MM_GET_POWER_CFM, cfm);
}

int cls_wifi_send_get_bctx_pn(struct cls_wifi_hw *cls_wifi_hw, u8 vif_idx,
				struct mm_bctx_pn_cfm *cfm)
{
	struct mm_bctx_pn_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);
	/* Build the MM_GET_BCTX_PN_REQ message */
	req = cls_wifi_msg_zalloc(MM_GET_BCTX_PN_REQ, TASK_MM, DRV_TASK_ID,
						  sizeof(struct mm_bctx_pn_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the MM_GET_BCTX_PN_REQ message */
	req->inst_nbr = vif_idx;

	/* Send the MM_GET_BCTX_PN_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, MM_GET_BCTX_PN_CFM, cfm);
}

int cls_wifi_send_set_edca(struct cls_wifi_hw *cls_wifi_hw, u8 hw_queue, u32 param,
					   bool uapsd, u8 inst_nbr)
{
	struct mm_set_edca_req *set_edca_req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the MM_SET_EDCA_REQ message */
	set_edca_req = cls_wifi_msg_zalloc(MM_SET_EDCA_REQ, TASK_MM, DRV_TASK_ID,
								   sizeof(struct mm_set_edca_req));
	if (!set_edca_req)
		return -ENOMEM;

	/* Set parameters for the MM_SET_EDCA_REQ message */
	set_edca_req->ac_param = param;
	set_edca_req->uapsd = uapsd;
	set_edca_req->hw_queue = hw_queue;
	set_edca_req->inst_nbr = inst_nbr;

	/* Send the MM_SET_EDCA_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, set_edca_req, 1, MM_SET_EDCA_CFM, NULL);
}

int cls_wifi_send_set_puncture_info(struct cls_wifi_hw *cls_wifi_hw, u8 vif_idx, u8 inact_bitmap)
{
	struct mm_puncture_params_req *punc_req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the MM_SET_EDCA_REQ message */
	punc_req = cls_wifi_msg_zalloc(MM_SET_PUNCTURE_REQ, TASK_MM, DRV_TASK_ID,
			sizeof(struct mm_puncture_params_req));
	if (!punc_req)
		return -ENOMEM;

	/* Set parameters for the MM_SET_EDCA_REQ message */
	punc_req->vif_idx = vif_idx;
	punc_req->inact_bitmap = inact_bitmap;

	/* Send the MM_SET_EDCA_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, punc_req, 0, MM_SET_PUNCTURE_CFM, NULL);
}

#if defined(MERAK2000) && MERAK2000
int cls_wifi_send_set_trace_loglevel(struct cls_wifi_hw *cls_wifi_hw, u8 radio, int mod, int val,
				struct dbg_trace_loglevel_set_cfm *cfm)
{
	struct dbg_trace_loglevel_set_req *set_loglev_req;

	if (cls_wifi_hw->plat->hw_rev != CLS_WIFI_HW_MERAK2000)
		return 0;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);
	set_loglev_req = cls_wifi_msg_zalloc(DBG_SET_TRACE_LOG_LEVEL_REQ, TASK_DBG, DRV_TASK_ID,
								sizeof(struct dbg_trace_loglevel_set_req));
	set_loglev_req->mod_index = mod;
	set_loglev_req->value = val;
	return  cls_wifi_send_msg(cls_wifi_hw, set_loglev_req, 1, DBG_SET_TRACE_LOG_LEVEL_CFM, cfm);
}

int cls_wifi_send_get_trace_loglevel(struct cls_wifi_hw *cls_wifi_hw, u8 radio,
				struct dbg_trace_loglevel_get_cfm *cfm)
{
	struct dbg_trace_loglevel_get_req *get_loglev_req;

	if (cls_wifi_hw->plat->hw_rev != CLS_WIFI_HW_MERAK2000)
		return 0;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);
	get_loglev_req = cls_wifi_msg_zalloc(DBG_GET_TRACE_LOG_LEVEL_REQ, TASK_DBG, DRV_TASK_ID,
								sizeof(struct dbg_trace_loglevel_get_req));
	get_loglev_req->radio = radio;
	return cls_wifi_send_msg(cls_wifi_hw, get_loglev_req, 1, DBG_GET_TRACE_LOG_LEVEL_CFM, cfm);
}
#endif

#ifdef CONFIG_CLS_WIFI_P2P_DEBUGFS
int cls_wifi_send_p2p_oppps_req(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *cls_wifi_vif,
							u8 ctw, struct mm_set_p2p_oppps_cfm *cfm)
{
	struct mm_set_p2p_oppps_req *p2p_oppps_req;
	int error;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the MM_SET_P2P_OPPPS_REQ message */
	p2p_oppps_req = cls_wifi_msg_zalloc(MM_SET_P2P_OPPPS_REQ, TASK_MM, DRV_TASK_ID,
									sizeof(struct mm_set_p2p_oppps_req));

	if (!p2p_oppps_req) {
		return -ENOMEM;
	}

	/* Fill the message parameters */
	p2p_oppps_req->vif_index = cls_wifi_vif->vif_index;
	p2p_oppps_req->ctwindow = ctw;

	/* Send the MM_P2P_OPPPS_REQ message to LMAC FW */
	error = cls_wifi_send_msg(cls_wifi_hw, p2p_oppps_req, 1, MM_SET_P2P_OPPPS_CFM, cfm);

	return (error);
}

int cls_wifi_send_p2p_noa_req(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *cls_wifi_vif,
						  int count, int interval, int duration, bool dyn_noa,
						  struct mm_set_p2p_noa_cfm *cfm)
{
	struct mm_set_p2p_noa_req *p2p_noa_req;
	int error;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Param check */
	if (count > 255)
		count = 255;

	if (duration >= interval) {
		dev_err(cls_wifi_hw->dev, "Invalid p2p NOA config: interval=%d <= duration=%d\n",
				interval, duration);
		return -EINVAL;
	}

	/* Build the MM_SET_P2P_NOA_REQ message */
	p2p_noa_req = cls_wifi_msg_zalloc(MM_SET_P2P_NOA_REQ, TASK_MM, DRV_TASK_ID,
								  sizeof(struct mm_set_p2p_noa_req));

	if (!p2p_noa_req) {
		return -ENOMEM;
	}

	/* Fill the message parameters */
	p2p_noa_req->vif_index = cls_wifi_vif->vif_index;
	p2p_noa_req->noa_inst_nb = 0;
	p2p_noa_req->count = count;

	if (count) {
		p2p_noa_req->duration_us = duration * 1024;
		p2p_noa_req->interval_us = interval * 1024;
		p2p_noa_req->start_offset = (interval - duration - 10) * 1024;
		p2p_noa_req->dyn_noa = dyn_noa;
	}

	/* Send the MM_SET_2P_NOA_REQ message to LMAC FW */
	error = cls_wifi_send_msg(cls_wifi_hw, p2p_noa_req, 1, MM_SET_P2P_NOA_CFM, cfm);

	return (error);
}
#endif /* CONFIG_CLS_WIFI_P2P_DEBUGFS */

#if defined(CFG_MERAK3000)
static void cls_wifi_eth_capa_fake_func(struct me_config_req *me_req,
	struct me_sta_add_req *sta_req)
{
	struct mac_eht_capability *eht_cap;

	if (me_req) {
		me_req->eht_supp = true;
		eht_cap = &me_req->eht_cap;
	} else if (sta_req) {
		sta_req->flags |= STA_EHT_CAPA;
		eht_cap = &sta_req->eht_cap;
	} else {
		pr_info("both me/sta req are NULL\n");

		return;
	}

	eht_cap->mac_cap_info[0] = 0x80;
	eht_cap->mac_cap_info[1] = 0;

	eht_cap->phy_cap_info[0] = 0x20;
	eht_cap->phy_cap_info[1] = 0;
	eht_cap->phy_cap_info[2] = 0;
	eht_cap->phy_cap_info[3] = 0;
	eht_cap->phy_cap_info[4] = 0;
	eht_cap->phy_cap_info[5] = 0x76;
	eht_cap->phy_cap_info[6] = 0x18;
	eht_cap->phy_cap_info[7] = 0x20;
	eht_cap->phy_cap_info[8] = 0;

	eht_cap->mcs_supp.mcs_le_80m = 0x222222;
	eht_cap->mcs_supp.mcs_160m = 0x222222;
}
#endif

/******************************************************************************
 *	Control messages handling functions (FULLMAC only)
 *****************************************************************************/
int cls_wifi_send_me_config_req(struct cls_wifi_hw *cls_wifi_hw)
{
	struct me_config_req *req;
	struct wiphy *wiphy = cls_wifi_hw->wiphy;
	struct ieee80211_sta_ht_cap ht_capability;
	struct ieee80211_sta_ht_cap *ht_cap = &ht_capability;
	struct ieee80211_sta_vht_cap vht_capability;
	struct ieee80211_sta_vht_cap *vht_cap = &vht_capability;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 20, 0)
	struct ieee80211_sta_he_cap he_capability;
	struct ieee80211_sta_he_cap *he_cap = &he_capability;
#endif
	uint8_t *ht_mcs = (uint8_t *)&ht_cap->mcs;
	int i;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	memset(ht_cap, 0, sizeof(*ht_cap));
	memset(vht_cap, 0, sizeof(*vht_cap));
	memset(he_cap, 0, sizeof(*he_cap));
	if(cls_wifi_hw->band_cap & CLS_WIFI_BAND_CAP_5G)
	{
		memcpy(ht_cap, &cls_wifi_hw->if_cfg80211.sbands[NL80211_BAND_5GHZ].ht_cap, sizeof(*ht_cap));
		memcpy(vht_cap, &cls_wifi_hw->if_cfg80211.sbands[NL80211_BAND_5GHZ].vht_cap, sizeof(*vht_cap));
		memcpy(he_cap, &cls_wifi_hw->if_cfg80211.sbands[NL80211_BAND_5GHZ].iftype_data->he_cap, sizeof(*he_cap));
	}
	else if(cls_wifi_hw->band_cap & CLS_WIFI_BAND_CAP_2G)
	{
		memcpy(ht_cap, &cls_wifi_hw->if_cfg80211.sbands[NL80211_BAND_2GHZ].ht_cap, sizeof(*ht_cap));
		memcpy(he_cap, &cls_wifi_hw->if_cfg80211.sbands[NL80211_BAND_2GHZ].iftype_data->he_cap, sizeof(*he_cap));
	}
	else
	{
		return -ENOTSUPP;
	}

	/* Build the ME_CONFIG_REQ message */
	req = cls_wifi_msg_zalloc(ME_CONFIG_REQ, TASK_ME, DRV_TASK_ID,
						  sizeof(struct me_config_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the ME_CONFIG_REQ message */
	req->ht_supp = ht_cap->ht_supported;
	req->vht_supp = vht_cap->vht_supported;
	req->ht_cap.ht_capa_info = cpu_to_le16(ht_cap->cap);
	req->ht_cap.a_mpdu_param = ht_cap->ampdu_factor |
							   (ht_cap->ampdu_density <<
								IEEE80211_HT_AMPDU_PARM_DENSITY_SHIFT);
	for (i = 0; i < sizeof(ht_cap->mcs); i++)
		req->ht_cap.mcs_rate[i] = ht_mcs[i];
	req->ht_cap.ht_extended_capa = 0;
	req->ht_cap.tx_beamforming_capa = 0;
	req->ht_cap.asel_capa = 0;

	req->vht_cap.vht_capa_info = cpu_to_le32(vht_cap->cap);
	req->vht_cap.rx_highest = cpu_to_le16(vht_cap->vht_mcs.rx_highest);
	req->vht_cap.rx_mcs_map = cpu_to_le16(vht_cap->vht_mcs.rx_mcs_map);
	req->vht_cap.tx_highest = cpu_to_le16(vht_cap->vht_mcs.tx_highest);
	req->vht_cap.tx_mcs_map = cpu_to_le16(vht_cap->vht_mcs.tx_mcs_map);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 20, 0)
	req->he_supp = he_cap->has_he;
	for (i = 0; i < ARRAY_SIZE(he_cap->he_cap_elem.mac_cap_info); i++) {
		req->he_cap.mac_cap_info[i] = he_cap->he_cap_elem.mac_cap_info[i];
	}
	for (i = 0; i < ARRAY_SIZE(he_cap->he_cap_elem.phy_cap_info); i++) {
		req->he_cap.phy_cap_info[i] = he_cap->he_cap_elem.phy_cap_info[i];
	}
	req->he_cap.mcs_supp.rx_mcs_80 = cpu_to_le16(he_cap->he_mcs_nss_supp.rx_mcs_80);
	req->he_cap.mcs_supp.tx_mcs_80 = cpu_to_le16(he_cap->he_mcs_nss_supp.tx_mcs_80);
	req->he_cap.mcs_supp.rx_mcs_160 = cpu_to_le16(he_cap->he_mcs_nss_supp.rx_mcs_160);
	req->he_cap.mcs_supp.tx_mcs_160 = cpu_to_le16(he_cap->he_mcs_nss_supp.tx_mcs_160);
	req->he_cap.mcs_supp.rx_mcs_80p80 = cpu_to_le16(he_cap->he_mcs_nss_supp.rx_mcs_80p80);
	req->he_cap.mcs_supp.tx_mcs_80p80 = cpu_to_le16(he_cap->he_mcs_nss_supp.tx_mcs_80p80);
	for (i = 0; i < MAC_HE_PPE_THRES_MAX_LEN; i++) {
		req->he_cap.ppe_thres[i] = he_cap->ppe_thres[i];
	}
#else
	req->he_supp = false;
#endif

#if defined(CFG_MERAK3000)
	cls_wifi_eth_capa_fake_func(req, NULL);
#endif
	req->ps_on = cls_wifi_hw->radio_params->ps_on;
	req->dpsm = cls_wifi_hw->radio_params->dpsm;
	req->tx_lft = cls_wifi_hw->radio_params->tx_lft;
	req->amsdu_tx = cls_wifi_hw->radio_params->amsdu_tx;
	req->ant_div_on = cls_wifi_hw->radio_params->ant_div;
	if (cls_wifi_hw->radio_params->use_160)
		req->phy_bw_max = PHY_CHNL_BW_160;
	else if (cls_wifi_hw->radio_params->use_80)
		req->phy_bw_max = PHY_CHNL_BW_80;
	else if (cls_wifi_hw->radio_params->use_2040)
		req->phy_bw_max = PHY_CHNL_BW_40;
	else
		req->phy_bw_max = PHY_CHNL_BW_20;

	wiphy_info(wiphy, "HT supp %d, VHT supp %d, HE supp %d\n", req->ht_supp,
															   req->vht_supp,
															   req->he_supp);

	/* Send the ME_CONFIG_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, ME_CONFIG_CFM, NULL);
}

int cls_wifi_send_me_chan_config_req(struct cls_wifi_hw *cls_wifi_hw)
{
	struct me_chan_config_req *req;
	int i;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the ME_CHAN_CONFIG_REQ message */
	req = cls_wifi_msg_zalloc(ME_CHAN_CONFIG_REQ, TASK_ME, DRV_TASK_ID,
											sizeof(struct me_chan_config_req));
	if (!req)
		return -ENOMEM;

	req->chan2G4_cnt=  0;

	if(cls_wifi_hw->band_cap & CLS_WIFI_BAND_CAP_2G) {
		struct ieee80211_supported_band *b = &cls_wifi_hw->if_cfg80211.sbands[NL80211_BAND_2GHZ];

		for (i = 0; i < b->n_channels; i++) {
			req->chan2G4[req->chan2G4_cnt].flags = 0;
			if (b->channels[i].flags & IEEE80211_CHAN_DISABLED)
				req->chan2G4[req->chan2G4_cnt].flags |= CHAN_DISABLED;
			req->chan2G4[req->chan2G4_cnt].flags |= get_chan_flags(b->channels[i].flags);
			req->chan2G4[req->chan2G4_cnt].band = NL80211_BAND_2GHZ;
			req->chan2G4[req->chan2G4_cnt].freq = b->channels[i].center_freq;
			req->chan2G4[req->chan2G4_cnt].tx_power = chan_to_fw_pwr(b->channels[i].max_power);
			req->chan2G4_cnt++;
			if (req->chan2G4_cnt == MAC_DOMAINCHANNEL_24G_MAX)
				break;
		}
	}

	req->chan5G_cnt = 0;

	if(cls_wifi_hw->band_cap & CLS_WIFI_BAND_CAP_5G) {
		struct ieee80211_supported_band *b = &cls_wifi_hw->if_cfg80211.sbands[NL80211_BAND_5GHZ];

		for (i = 0; i < b->n_channels; i++) {
			req->chan5G[req->chan5G_cnt].flags = 0;
			if (b->channels[i].flags & IEEE80211_CHAN_DISABLED)
				req->chan5G[req->chan5G_cnt].flags |= CHAN_DISABLED;
			req->chan5G[req->chan5G_cnt].flags |= get_chan_flags(b->channels[i].flags);
			req->chan5G[req->chan5G_cnt].band = NL80211_BAND_5GHZ;
			req->chan5G[req->chan5G_cnt].freq = b->channels[i].center_freq;
			req->chan5G[req->chan5G_cnt].tx_power = chan_to_fw_pwr(b->channels[i].max_power);
			req->chan5G_cnt++;
			if (req->chan5G_cnt == MAC_DOMAINCHANNEL_5G_MAX)
				break;
		}
	}

	/* Send the ME_CHAN_CONFIG_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, ME_CHAN_CONFIG_CFM, NULL);
}

int cls_wifi_send_me_set_control_port_req(struct cls_wifi_hw *cls_wifi_hw, bool opened, u16 sta_idx)
{
	struct me_set_control_port_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the ME_SET_CONTROL_PORT_REQ message */
	req = cls_wifi_msg_zalloc(ME_SET_CONTROL_PORT_REQ, TASK_ME, DRV_TASK_ID,
								   sizeof(struct me_set_control_port_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the ME_SET_CONTROL_PORT_REQ message */
	req->sta_idx = sta_idx;
	req->control_port_open = opened;

	/* Send the ME_SET_CONTROL_PORT_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, ME_SET_CONTROL_PORT_CFM, NULL);
}

int cls_wifi_send_me_sta_add(struct cls_wifi_hw *cls_wifi_hw, struct station_parameters *params,
						 const u8 *mac, u8 inst_nbr, struct me_sta_add_cfm *cfm)
{
	struct me_sta_add_req *req;
	u8 *ht_mcs = (u8 *)&params->ht_capa->mcs;
	int i;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the MM_STA_ADD_REQ message */
	req = cls_wifi_msg_zalloc(ME_STA_ADD_REQ, TASK_ME, DRV_TASK_ID,
								  sizeof(struct me_sta_add_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the MM_STA_ADD_REQ message */
	memcpy(&(req->mac_addr.array[0]), mac, ETH_ALEN);

	req->rate_set.length = params->supported_rates_len;
	for (i = 0; i < params->supported_rates_len; i++)
		req->rate_set.array[i] = params->supported_rates[i];

	req->flags = 0;

	if (params->capability & WLAN_CAPABILITY_SHORT_PREAMBLE)
		req->flags |= STA_SHORT_PREAMBLE_CAPA;

	if (params->ht_capa) {
		const struct ieee80211_ht_cap *ht_capa = params->ht_capa;

		req->flags |= STA_HT_CAPA;
		req->ht_cap.ht_capa_info = cpu_to_le16(ht_capa->cap_info);
		req->ht_cap.a_mpdu_param = ht_capa->ampdu_params_info;
		for (i = 0; i < sizeof(ht_capa->mcs); i++)
			req->ht_cap.mcs_rate[i] = ht_mcs[i];
		req->ht_cap.ht_extended_capa = cpu_to_le16(ht_capa->extended_ht_cap_info);
		req->ht_cap.tx_beamforming_capa = cpu_to_le32(ht_capa->tx_BF_cap_info);
		req->ht_cap.asel_capa = ht_capa->antenna_selection_info;
	}

	if (params->vht_capa) {
		const struct ieee80211_vht_cap *vht_capa = params->vht_capa;

		req->flags |= STA_VHT_CAPA;
		req->vht_cap.vht_capa_info = cpu_to_le32(vht_capa->vht_cap_info);
		req->vht_cap.rx_highest = cpu_to_le16(vht_capa->supp_mcs.rx_highest);
		req->vht_cap.rx_mcs_map = cpu_to_le16(vht_capa->supp_mcs.rx_mcs_map);
		req->vht_cap.tx_highest = cpu_to_le16(vht_capa->supp_mcs.tx_highest);
		req->vht_cap.tx_mcs_map = cpu_to_le16(vht_capa->supp_mcs.tx_mcs_map);
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 20, 0)
	if (params->he_capa) {
		const struct ieee80211_he_cap_elem *he_capa = params->he_capa;
		struct ieee80211_he_mcs_nss_supp *mcs_nss_supp =
								(struct ieee80211_he_mcs_nss_supp *)(he_capa + 1);

		req->flags |= STA_HE_CAPA;
		for (i = 0; i < ARRAY_SIZE(he_capa->mac_cap_info); i++) {
			req->he_cap.mac_cap_info[i] = he_capa->mac_cap_info[i];
		}
		for (i = 0; i < ARRAY_SIZE(he_capa->phy_cap_info); i++) {
			req->he_cap.phy_cap_info[i] = he_capa->phy_cap_info[i];
		}
		req->he_cap.mcs_supp.rx_mcs_80 = mcs_nss_supp->rx_mcs_80;
		req->he_cap.mcs_supp.tx_mcs_80 = mcs_nss_supp->tx_mcs_80;
		req->he_cap.mcs_supp.rx_mcs_160 = mcs_nss_supp->rx_mcs_160;
		req->he_cap.mcs_supp.tx_mcs_160 = mcs_nss_supp->tx_mcs_160;
		req->he_cap.mcs_supp.rx_mcs_80p80 = mcs_nss_supp->rx_mcs_80p80;
		req->he_cap.mcs_supp.tx_mcs_80p80 = mcs_nss_supp->tx_mcs_80p80;
	}
#endif

#if defined(CFG_MERAK3000)
	cls_wifi_eth_capa_fake_func(NULL, req);
#endif
	if (params->sta_flags_set & BIT(NL80211_STA_FLAG_WME))
		req->flags |= STA_QOS_CAPA;

	if (params->sta_flags_set & BIT(NL80211_STA_FLAG_MFP))
		req->flags |= STA_MFP_CAPA;

	if (params->opmode_notif_used) {
		req->flags |= STA_OPMOD_NOTIF;
		req->opmode = params->opmode_notif;
	}

	req->aid = cpu_to_le16(params->aid);
	req->uapsd_queues = params->uapsd_queues;
	req->max_sp_len = params->max_sp * 2;
	req->vif_idx = inst_nbr;

	if (params->sta_flags_set & BIT(NL80211_STA_FLAG_TDLS_PEER)) {
		struct cls_wifi_vif *cls_wifi_vif = cls_wifi_hw->vif_table[inst_nbr];
		req->tdls_sta = true;
		if ((params->ext_capab[3] & WLAN_EXT_CAPA4_TDLS_CHAN_SWITCH) &&
			!cls_wifi_vif->tdls_chsw_prohibited)
			req->tdls_chsw_allowed = true;
		if (cls_wifi_vif->tdls_status == TDLS_SETUP_RSP_TX)
			req->tdls_sta_initiator = true;
	}

	/* Send the ME_STA_ADD_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, ME_STA_ADD_CFM, cfm);
}

int cls_wifi_send_me_sta_del(struct cls_wifi_hw *cls_wifi_hw, u16 sta_idx, bool tdls_sta)
{
	struct me_sta_del_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (!test_bit(CLS_WIFI_DEV_STARTED, &cls_wifi_hw->flags))
		return 0;

	/* Build the MM_STA_DEL_REQ message */
	req = cls_wifi_msg_zalloc(ME_STA_DEL_REQ, TASK_ME, DRV_TASK_ID,
						  sizeof(struct me_sta_del_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the MM_STA_DEL_REQ message */
	req->sta_idx = sta_idx;
	req->tdls_sta = tdls_sta;

	/* Send the ME_STA_DEL_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, ME_STA_DEL_CFM, NULL);
}

int cls_wifi_send_me_traffic_ind(struct cls_wifi_hw *cls_wifi_hw, u16 sta_idx, bool uapsd, u8 tx_status)
{
	struct me_traffic_ind_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the ME_UTRAFFIC_IND_REQ message */
	req = cls_wifi_msg_zalloc(ME_TRAFFIC_IND_REQ, TASK_ME, DRV_TASK_ID,
						  sizeof(struct me_traffic_ind_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the ME_TRAFFIC_IND_REQ message */
	req->sta_idx = sta_idx;
	req->tx_avail = tx_status;
	req->uapsd = uapsd;

	/* Send the ME_TRAFFIC_IND_REQ to UMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, ME_TRAFFIC_IND_CFM, NULL);
}

int cls_wifi_send_twt_request(struct cls_wifi_hw *cls_wifi_hw,
						  u8 setup_type, u8 vif_idx,
						  struct twt_conf_tag *conf,
						  struct twt_setup_cfm *cfm)
{
	struct twt_setup_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the TWT_SETUP_REQ message */
	req = cls_wifi_msg_zalloc(TWT_SETUP_REQ, TASK_TWT, DRV_TASK_ID,
						  sizeof(struct twt_setup_req));
	if (!req)
		return -ENOMEM;

	memcpy(&req->conf, conf, sizeof(req->conf));
	req->setup_type = setup_type;
	req->vif_idx = vif_idx;

	/* Send the TWT_SETUP_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, TWT_SETUP_CFM, cfm);
}

int cls_wifi_send_twt_teardown(struct cls_wifi_hw *cls_wifi_hw,
						   struct twt_teardown_req *twt_teardown,
						   struct twt_teardown_cfm *cfm)
{
	struct twt_teardown_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the TWT_TEARDOWN_REQ message */
	req = cls_wifi_msg_zalloc(TWT_TEARDOWN_REQ, TASK_TWT, DRV_TASK_ID,
						  sizeof(struct twt_teardown_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, twt_teardown, sizeof(struct twt_teardown_req));

	/* Send the TWT_TEARDOWN_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, TWT_TEARDOWN_CFM, cfm);
}

int cls_wifi_send_me_set_ps_mode(struct cls_wifi_hw *cls_wifi_hw, u8 ps_mode)
{
	struct me_set_ps_mode_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the ME_SET_PS_MODE_REQ message */
	req = cls_wifi_msg_zalloc(ME_SET_PS_MODE_REQ, TASK_ME, DRV_TASK_ID,
						  sizeof(struct me_set_ps_mode_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the ME_SET_PS_MODE_REQ message */
	req->ps_state = ps_mode;

	/* Send the ME_SET_PS_MODE_REQ message to FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, ME_SET_PS_MODE_CFM, NULL);
}

int cls_wifi_send_sm_connect_req(struct cls_wifi_hw *cls_wifi_hw,
							 struct cls_wifi_vif *cls_wifi_vif,
							 struct cfg80211_connect_params *sme,
							 struct sm_connect_cfm *cfm)
{
	struct sm_connect_req *req;
	int i, ie_len;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	ie_len = update_connect_req(cls_wifi_vif, sme);

	/* Build the SM_CONNECT_REQ message */
	req = cls_wifi_msg_zalloc(SM_CONNECT_REQ, TASK_SM, DRV_TASK_ID,
					 (sizeof(struct sm_connect_req) + ie_len));

	if (!req)
		return -ENOMEM;

	/* Set parameters for the SM_CONNECT_REQ message */
	if (sme->crypto.n_ciphers_pairwise &&
		((sme->crypto.ciphers_pairwise[0] == WLAN_CIPHER_SUITE_WEP40) ||
		 (sme->crypto.ciphers_pairwise[0] == WLAN_CIPHER_SUITE_TKIP) ||
		 (sme->crypto.ciphers_pairwise[0] == WLAN_CIPHER_SUITE_WEP104)))
		req->flags |= DISABLE_HT;

	if (use_privacy(&sme->crypto))
		req->flags |= USE_PRIVACY;

	if (sme->crypto.control_port)
		req->flags |= CONTROL_PORT_HOST;

	if (sme->crypto.control_port_no_encrypt)
		req->flags |= CONTROL_PORT_NO_ENC;

	if (use_pairwise_key(&sme->crypto))
		req->flags |= USE_PAIRWISE_KEY;

	if (sme->mfp == NL80211_MFP_REQUIRED)
		req->flags |= MFP_IN_USE;

	if (cls_wifi_hw->radio_params->amsdu_require_spp)
		req->flags |= REQUIRE_SPP_AMSDU;

	req->ctrl_port_ethertype = sme->crypto.control_port_ethertype;

	if (sme->bssid)
		memcpy(&req->bssid, sme->bssid, ETH_ALEN);
	else
		req->bssid = mac_addr_bcst;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0)
	if (sme->prev_bssid)
		req->flags |= REASSOCIATION;
#else
	if (cls_wifi_vif->sta.ap)
		req->flags |= REASSOCIATION;
#endif

	if ((sme->auth_type == NL80211_AUTHTYPE_FT) && (cls_wifi_vif->sta.flags & CLS_WIFI_STA_FT_OVER_DS))
		req->flags |= (REASSOCIATION | FT_OVER_DS);

	req->vif_idx = cls_wifi_vif->vif_index;
	if (sme->channel) {
		req->chan.band = sme->channel->band;
		req->chan.freq = sme->channel->center_freq;
		req->chan.flags = get_chan_flags(sme->channel->flags);
	} else {
		req->chan.freq = (u16_l)-1;
	}
	for (i = 0; i < sme->ssid_len; i++)
		req->ssid.array[i] = sme->ssid[i];
	req->ssid.length = sme->ssid_len;

	req->listen_interval = cls_wifi_hw->radio_params->listen_itv;
	req->dont_wait_bcmc = !cls_wifi_hw->radio_params->listen_bcmc;

	/* Set auth_type */
	if (sme->auth_type == NL80211_AUTHTYPE_AUTOMATIC)
		req->auth_type = WLAN_AUTH_OPEN;
	else if (sme->auth_type == NL80211_AUTHTYPE_OPEN_SYSTEM)
		req->auth_type = WLAN_AUTH_OPEN;
	else if (sme->auth_type == NL80211_AUTHTYPE_SHARED_KEY)
		req->auth_type = WLAN_AUTH_SHARED_KEY;
	else if (sme->auth_type == NL80211_AUTHTYPE_FT)
		req->auth_type = WLAN_AUTH_FT;
	else if (sme->auth_type == NL80211_AUTHTYPE_SAE)
		req->auth_type = WLAN_AUTH_SAE;
	else
		goto invalid_param;

	copy_connect_ies(cls_wifi_vif, req, sme);

	/* Set UAPSD queues */
	req->uapsd_queues = cls_wifi_hw->radio_params->uapsd_queues;

	/* Send the SM_CONNECT_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, SM_CONNECT_CFM, cfm);

invalid_param:
	cls_wifi_msg_free(cls_wifi_hw, req);
	return -EINVAL;
}

int cls_wifi_send_sm_disconnect_req(struct cls_wifi_hw *cls_wifi_hw,
								struct cls_wifi_vif *cls_wifi_vif,
								u16 reason)
{
	struct sm_disconnect_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the SM_DISCONNECT_REQ message */
	req = cls_wifi_msg_zalloc(SM_DISCONNECT_REQ, TASK_SM, DRV_TASK_ID,
								   sizeof(struct sm_disconnect_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the SM_DISCONNECT_REQ message */
	req->reason_code = reason;
	req->vif_idx = cls_wifi_vif->vif_index;

	/* Send the SM_DISCONNECT_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, SM_DISCONNECT_CFM, NULL);
}

int cls_wifi_send_sm_external_auth_required_rsp(struct cls_wifi_hw *cls_wifi_hw,
											struct cls_wifi_vif *cls_wifi_vif,
											u16 status)
{
	struct sm_external_auth_required_rsp *rsp;

	/* Build the SM_EXTERNAL_AUTH_CFM message */
	rsp = cls_wifi_msg_zalloc(SM_EXTERNAL_AUTH_REQUIRED_RSP, TASK_SM, DRV_TASK_ID,
						  sizeof(struct sm_external_auth_required_rsp));
	if (!rsp)
		return -ENOMEM;

	rsp->status = status;
	rsp->vif_idx = cls_wifi_vif->vif_index;

	/* send the SM_EXTERNAL_AUTH_REQUIRED_RSP message UMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, rsp, 0, 0, NULL);
}

int cls_wifi_send_sm_ft_auth_rsp(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *cls_wifi_vif,
							 uint8_t *ie, int ie_len)
{
	struct sm_connect_req *rsp;

	rsp = cls_wifi_msg_zalloc(SM_FT_AUTH_RSP, TASK_SM, DRV_TASK_ID,
						 (sizeof(struct sm_connect_req) + ie_len));
	if (!rsp)
		return -ENOMEM;

	rsp->vif_idx = cls_wifi_vif->vif_index;
	rsp->ie_len = ie_len;
	memcpy(rsp->ie_buf, ie, rsp->ie_len);

	return cls_wifi_send_msg(cls_wifi_hw, rsp, 0, 0, NULL);
}

int cls_wifi_send_apm_start_req(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *vif,
							struct cfg80211_ap_settings *settings,
							struct apm_start_cfm *cfm)
{
	struct apm_start_req *req;
	struct cls_wifi_bcn *bcn = &vif->ap.bcn;
	struct cls_wifi_ipc_buf buf;
	u8 *bcn_buf;
	u32 flags = 0;
	const u8 *rate_ie;
	u8 rate_len = 0;
	int var_offset = offsetof(struct ieee80211_mgmt, u.beacon.variable);
	const u8 *var_pos;
	int len, i, error;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the APM_START_REQ message */
	req = cls_wifi_msg_zalloc(APM_START_REQ, TASK_APM, DRV_TASK_ID,
								   sizeof(struct apm_start_req));
	if (!req)
		return -ENOMEM;

	// Build the beacon
	bcn->dtim = (u8)settings->dtim_period;
	bcn_buf = cls_wifi_build_bcn(bcn, &settings->beacon);
	if (!bcn_buf) {
		cls_wifi_msg_free(cls_wifi_hw, req);
		return -ENOMEM;
	}

	// Retrieve the basic rate set from the beacon buffer
	len = bcn->len - var_offset;
	var_pos = bcn_buf + var_offset;

// Assume that rate higher that 54 Mbps are BSS membership
#define IS_BASIC_RATE(r) (r & 0x80) && ((r & ~0x80) <= (54 * 2))

	rate_ie = cfg80211_find_ie(WLAN_EID_SUPP_RATES, var_pos, len);
	if (rate_ie) {
		const u8 *rates = rate_ie + 2;
		for (i = 0; (i < rate_ie[1]) && (rate_len < MAC_RATESET_LEN); i++) {
			if (IS_BASIC_RATE(rates[i]))
				req->basic_rates.array[rate_len++] = rates[i];
		}
	}
	rate_ie = cfg80211_find_ie(WLAN_EID_EXT_SUPP_RATES, var_pos, len);
	if (rate_ie) {
		const u8 *rates = rate_ie + 2;
		for (i = 0; (i < rate_ie[1]) && (rate_len < MAC_RATESET_LEN); i++) {
			if (IS_BASIC_RATE(rates[i]))
				req->basic_rates.array[rate_len++] = rates[i];
		}
	}
	req->basic_rates.length = rate_len;
#undef IS_BASIC_RATE

	// Sync buffer for FW
	if ((error = cls_wifi_ipc_buf_a2e_init(cls_wifi_hw, &buf, bcn_buf, bcn->len))) {
		netdev_err(vif->ndev, "Failed to allocate IPC buf for AP Beacon\n");
		kfree(bcn_buf);
		return -EIO;
	}

	/* Set parameters for the APM_START_REQ message */
	req->vif_idx = vif->vif_index;
	req->bcn_addr = buf.dma_addr;
	req->bcn_len = bcn->len;
	req->tim_oft = bcn->head_len;
	req->tim_len = bcn->tim_len;
	cfg80211_to_cls_wifi_chan(&settings->chandef, &req->chan);
	req->bcn_int = settings->beacon_interval;

	if (use_privacy(&settings->crypto))
		flags |= USE_PRIVACY;

	if (settings->crypto.control_port)
		flags |= CONTROL_PORT_HOST;

	if (settings->crypto.control_port_no_encrypt)
		flags |= CONTROL_PORT_NO_ENC;

	if (use_pairwise_key(&settings->crypto))
		flags |= USE_PAIRWISE_KEY;

	if (settings->crypto.control_port_ethertype) {
		req->ctrl_port_ethertype = settings->crypto.control_port_ethertype;
		pr_warn(" %s: set ethertype[0x%04x]\n", __func__,
				settings->crypto.control_port_ethertype);
	} else
		req->ctrl_port_ethertype = ETH_P_PAE;
	req->flags = flags;
	pr_warn(" %s: req_flags= 0x%08x, ethtype=0x%04x\n", __func__, req->flags,
		req->ctrl_port_ethertype);

	/* Send the APM_START_REQ message to LMAC FW */
	error = cls_wifi_send_msg(cls_wifi_hw, req, 1, APM_START_CFM, cfm);

	cls_wifi_ipc_buf_dealloc(cls_wifi_hw, &buf);

	return error;
}

int cls_wifi_send_apm_stop_req(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *vif)
{
	struct apm_stop_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the APM_STOP_REQ message */
	req = cls_wifi_msg_zalloc(APM_STOP_REQ, TASK_APM, DRV_TASK_ID,
						  sizeof(struct apm_stop_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the APM_STOP_REQ message */
	req->vif_idx = vif->vif_index;

	/* Send the APM_STOP_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, APM_STOP_CFM, NULL);
}

int cls_wifi_send_apm_probe_req(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *vif,
							struct cls_wifi_sta *sta, struct apm_probe_client_cfm *cfm)
{
	struct apm_probe_client_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	req = cls_wifi_msg_zalloc(APM_PROBE_CLIENT_REQ, TASK_APM, DRV_TASK_ID,
						  sizeof(struct apm_probe_client_req));
	if (!req)
		return -ENOMEM;

	req->vif_idx = vif->vif_index;
	req->sta_idx = sta->sta_idx;

	/* Send the APM_PROBE_CLIENT_REQ message to UMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, APM_PROBE_CLIENT_CFM, cfm);
}

int cls_wifi_send_scanu_req(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *cls_wifi_vif,
						struct cfg80211_scan_request *param)
{
	struct scanu_start_req *req;
	int i;
	uint16_t chan_flags = 0;
#if defined(CONFIG_CLS_EMU_ADAPTER) || defined(CFG_M3K_FPGA)
	uint32_t chan_cnt_nb = 0;
#endif

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the SCANU_START_REQ message */
	req = cls_wifi_msg_zalloc(SCANU_START_REQ, TASK_SCANU, DRV_TASK_ID,
						  sizeof(struct scanu_start_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters */
	req->vif_idx = cls_wifi_vif->vif_index;
	req->chan_cnt = (u8)min_t(int, SCAN_CHANNEL_MAX, param->n_channels);
	req->ssid_cnt = (u8)min_t(int, SCAN_SSID_MAX, param->n_ssids);
	req->bssid = mac_addr_bcst;
	req->no_cck = param->no_cck;
	req->ext_enabled = cls_wifi_hw->scan_ext.ext_enabled;
	req->rx_filter = cls_wifi_hw->scan_ext.rx_filter;
	req->work_duration = cls_wifi_hw->scan_ext.work_duration;
	req->scan_interval = cls_wifi_hw->scan_ext.scan_interval;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)
	if (param->duration_mandatory)
		req->duration = ieee80211_tu_to_usec(param->duration);
#endif

	if (req->ssid_cnt == 0)
		chan_flags |= CHAN_NO_IR;
	for (i = 0; i < req->ssid_cnt; i++) {
		int j;
		for (j = 0; j < param->ssids[i].ssid_len; j++)
			req->ssid[i].array[j] = param->ssids[i].ssid[j];
		req->ssid[i].length = param->ssids[i].ssid_len;
	}

	if (param->ie) {
		if (cls_wifi_ipc_buf_a2e_alloc(cls_wifi_hw, &cls_wifi_hw->scan_ie,
								   param->ie_len, param->ie)) {
			netdev_err(cls_wifi_vif->ndev, "Failed to allocate IPC buf for SCAN IEs\n");
			goto error;
		}

		req->add_ie_len = param->ie_len;
		req->add_ies = cls_wifi_hw->scan_ie.dma_addr;
	} else {
		req->add_ie_len = 0;
		req->add_ies = 0;
	}

#if defined(CONFIG_CLS_EMU_ADAPTER) || defined(CFG_M3K_FPGA)
	pr_warn("[scan]chan_cnt:%d\n", req->chan_cnt);
	for (i = 0; i < req->chan_cnt; i++) {
		struct ieee80211_channel *chan = param->channels[i];
		if (req->chan_cnt < 2) {
			req->chan[i].band = chan->band;
			req->chan[i].freq = chan->center_freq;
			req->chan[i].flags = chan_flags | get_chan_flags(chan->flags);
			req->chan[i].tx_power = chan_to_fw_pwr(chan->max_reg_power);
			chan_cnt_nb++;
		} else {
			if((chan->band == 1)
				&& (chan->center_freq == 5180)) {
				req->chan[chan_cnt_nb].band = chan->band;
				req->chan[chan_cnt_nb].freq = chan->center_freq;
				req->chan[chan_cnt_nb].flags = chan_flags | get_chan_flags(chan->flags);
				req->chan[chan_cnt_nb].tx_power = chan_to_fw_pwr(chan->max_reg_power);
				chan_cnt_nb++;
			}
		}
		pr_warn("[scan]band:%d, freq:%d\r\n", chan->band, chan->center_freq);
	}
	req->chan_cnt = chan_cnt_nb;
	req->duration = 30 * 1000; //30ms
#else
	for (i = 0; i < req->chan_cnt; i++) {
		struct ieee80211_channel *chan = param->channels[i];

		req->chan[i].band = chan->band;
		req->chan[i].freq = chan->center_freq;
		req->chan[i].flags = chan_flags | get_chan_flags(chan->flags);
		req->chan[i].tx_power = chan_to_fw_pwr(chan->max_reg_power);
	}
#endif

	req->off_channel = !!(param->flags & NL80211_SCAN_FLAG_OFF_CHANNEL);

	/* Send the SCANU_START_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
error:
	if (req != NULL)
		cls_wifi_msg_free(cls_wifi_hw, req);
	return -ENOMEM;
}

int cls_wifi_send_scanu_abort_req(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *cls_wifi_vif)
{
	struct scanu_abort_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);
	req = cls_wifi_msg_zalloc(SCANU_ABORT_REQ, TASK_SCANU, DRV_TASK_ID,
						  sizeof(struct scanu_abort_req));
	if (!req)
		return -ENOMEM;

	req->vif_idx = cls_wifi_vif->vif_index;
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_apm_start_cac_req(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *vif,
								struct cfg80211_chan_def *chandef,
								struct apm_start_cac_cfm *cfm, bool rd_enable)
{
	struct apm_start_cac_req *req;

	CLS_WIFI_INFO(CLS_WIFI_FN_ENTRY_STR);

	/* Build the APM_START_CAC_REQ message */
	req = cls_wifi_msg_zalloc(APM_START_CAC_REQ, TASK_APM, DRV_TASK_ID,
						  sizeof(struct apm_start_cac_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the APM_START_CAC_REQ message */
	req->vif_idx = vif->vif_index;
	cfg80211_to_cls_wifi_chan(chandef, &req->chan);

	req->rd_enable = rd_enable;

	/* Send the APM_START_CAC_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, APM_START_CAC_CFM, cfm);
}

int cls_wifi_send_apm_stop_cac_req(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *vif)
{
	struct apm_stop_cac_req *req;

	CLS_WIFI_INFO(CLS_WIFI_FN_ENTRY_STR);

	/* Build the APM_STOP_CAC_REQ message */
	req = cls_wifi_msg_zalloc(APM_STOP_CAC_REQ, TASK_APM, DRV_TASK_ID,
						  sizeof(struct apm_stop_cac_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the APM_STOP_CAC_REQ message */
	req->vif_idx = vif->vif_index;

	/* Send the APM_STOP_CAC_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, APM_STOP_CAC_CFM, NULL);
}

int cls_wifi_send_mesh_start_req(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *vif,
							 const struct mesh_config *conf, const struct mesh_setup *setup,
							 struct mesh_start_cfm *cfm)
{
	// Message to send
	struct mesh_start_req *req;
	// Supported basic rates
	struct ieee80211_supported_band *band = &cls_wifi_hw->if_cfg80211.sbands[setup->chandef.chan->band];
	/* Counter */
	int i;
	/* Return status */
	int status;
	/* DMA Address to be unmapped after confirmation reception */
	u32 dma_addr = 0;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the MESH_START_REQ message */
	req = cls_wifi_msg_zalloc(MESH_START_REQ, TASK_MESH, DRV_TASK_ID,
						  sizeof(struct mesh_start_req));
	if (!req) {
		return -ENOMEM;
	}

	req->vif_index = vif->vif_index;
	req->bcn_int = setup->beacon_interval;
	req->dtim_period = setup->dtim_period;
	req->mesh_id_len = setup->mesh_id_len;

	for (i = 0; i < setup->mesh_id_len; i++) {
		req->mesh_id[i] = *(setup->mesh_id + i);
	}

	req->user_mpm = setup->user_mpm;
	req->is_auth = setup->is_authenticated;
	req->auth_id = setup->auth_id;
	req->ie_len = setup->ie_len;

	if (setup->ie_len) {
		/*
		 * Need to provide a Virtual Address to the MAC so that it can download the
		 * additional information elements.
		 */
		req->ie_addr = dma_map_single(cls_wifi_hw->dev, (void *)setup->ie,
									  setup->ie_len, DMA_FROM_DEVICE);

		/* Check DMA mapping result */
		if (dma_mapping_error(cls_wifi_hw->dev, req->ie_addr)) {
			printk(KERN_CRIT "%s - DMA Mapping error on additional IEs\n", __func__);

			/* Consider there is no Additional IEs */
			req->ie_len = 0;
		} else {
			/* Store DMA Address so that we can unmap the memory section once MESH_START_CFM is received */
			dma_addr = req->ie_addr;
		}
	}

	/* Provide rate information */
	req->basic_rates.length = 0;
	for (i = 0; i < band->n_bitrates; i++) {
		u16 rate = band->bitrates[i].bitrate;

		/* Read value is in in units of 100 Kbps, provided value is in units
		 * of 1Mbps, and multiplied by 2 so that 5.5 becomes 11 */
		rate = (rate << 1) / 10;

		if (setup->basic_rates & CO_BIT(i)) {
			rate |= 0x80;
		}

		req->basic_rates.array[i] = (u8)rate;
		req->basic_rates.length++;
	}

	/* Provide channel information */
	cfg80211_to_cls_wifi_chan(&setup->chandef, &req->chan);

	/* Send the MESH_START_REQ message to UMAC FW */
	status = cls_wifi_send_msg(cls_wifi_hw, req, 1, MESH_START_CFM, cfm);

	/* Unmap DMA area */
	if (setup->ie_len) {
		dma_unmap_single(cls_wifi_hw->dev, dma_addr, setup->ie_len, DMA_TO_DEVICE);
	}

	/* Return the status */
	return (status);
}

int cls_wifi_send_mesh_stop_req(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *vif,
							struct mesh_stop_cfm *cfm)
{
	// Message to send
	struct mesh_stop_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the MESH_STOP_REQ message */
	req = cls_wifi_msg_zalloc(MESH_STOP_REQ, TASK_MESH, DRV_TASK_ID,
						  sizeof(struct mesh_stop_req));
	if (!req) {
		return -ENOMEM;
	}

	req->vif_idx = vif->vif_index;

	/* Send the MESH_STOP_REQ message to UMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, MESH_STOP_CFM, cfm);
}

int cls_wifi_send_mesh_update_req(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *vif,
							  u32 mask, const struct mesh_config *p_mconf, struct mesh_update_cfm *cfm)
{
	// Message to send
	struct mesh_update_req *req;
	// Keep only bit for fields which can be updated
	u32 supp_mask = (mask << 1) & (CO_BIT(NL80211_MESHCONF_GATE_ANNOUNCEMENTS)
								   | CO_BIT(NL80211_MESHCONF_HWMP_ROOTMODE)
								   | CO_BIT(NL80211_MESHCONF_FORWARDING)
								   | CO_BIT(NL80211_MESHCONF_POWER_MODE));


	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (!supp_mask) {
		return -ENOENT;
	}

	/* Build the MESH_UPDATE_REQ message */
	req = cls_wifi_msg_zalloc(MESH_UPDATE_REQ, TASK_MESH, DRV_TASK_ID,
						  sizeof(struct mesh_update_req));

	if (!req) {
		return -ENOMEM;
	}

	req->vif_idx = vif->vif_index;

	if (supp_mask & CO_BIT(NL80211_MESHCONF_GATE_ANNOUNCEMENTS))
	{
		req->flags |= CO_BIT(MESH_UPDATE_FLAGS_GATE_MODE_BIT);
		req->gate_announ = p_mconf->dot11MeshGateAnnouncementProtocol;
	}

	if (supp_mask & CO_BIT(NL80211_MESHCONF_HWMP_ROOTMODE))
	{
		req->flags |= CO_BIT(MESH_UPDATE_FLAGS_ROOT_MODE_BIT);
		req->root_mode = p_mconf->dot11MeshHWMPRootMode;
	}

	if (supp_mask & CO_BIT(NL80211_MESHCONF_FORWARDING))
	{
		req->flags |= CO_BIT(MESH_UPDATE_FLAGS_MESH_FWD_BIT);
		req->mesh_forward = p_mconf->dot11MeshForwarding;
	}

	if (supp_mask & CO_BIT(NL80211_MESHCONF_POWER_MODE))
	{
		req->flags |= CO_BIT(MESH_UPDATE_FLAGS_LOCAL_PSM_BIT);
		req->local_ps_mode = p_mconf->power_mode;
	}

	/* Send the MESH_UPDATE_REQ message to UMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, MESH_UPDATE_CFM, cfm);
}

int cls_wifi_send_mesh_peer_info_req(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *vif,
								 u16 sta_idx, struct mesh_peer_info_cfm *cfm)
{
	// Message to send
	struct mesh_peer_info_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the MESH_PEER_INFO_REQ message */
	req = cls_wifi_msg_zalloc(MESH_PEER_INFO_REQ, TASK_MESH, DRV_TASK_ID,
						  sizeof(struct mesh_peer_info_req));
	if (!req) {
		return -ENOMEM;
	}

	req->sta_idx = sta_idx;

	/* Send the MESH_PEER_INFO_REQ message to UMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, MESH_PEER_INFO_CFM, cfm);
}

void cls_wifi_send_mesh_peer_update_ntf(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *vif,
									u16 sta_idx, u8 mlink_state)
{
	// Message to send
	struct mesh_peer_update_ntf *ntf;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the MESH_PEER_UPDATE_NTF message */
	ntf = cls_wifi_msg_zalloc(MESH_PEER_UPDATE_NTF, TASK_MESH, DRV_TASK_ID,
						  sizeof(struct mesh_peer_update_ntf));

	if (ntf) {
		ntf->vif_idx = vif->vif_index;
		ntf->sta_idx = sta_idx;
		ntf->state = mlink_state;

		/* Send the MESH_PEER_INFO_REQ message to UMAC FW */
		cls_wifi_send_msg(cls_wifi_hw, ntf, 0, 0, NULL);
	}
}

void cls_wifi_send_mesh_path_create_req(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *vif, u8 *tgt_addr)
{
	struct mesh_path_create_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Check if we are already waiting for a confirmation */
	if (vif->ap.flags & CLS_WIFI_AP_CREATE_MESH_PATH)
		return;

	/* Build the MESH_PATH_CREATE_REQ message */
	req = cls_wifi_msg_zalloc(MESH_PATH_CREATE_REQ, TASK_MESH, DRV_TASK_ID,
						  sizeof(struct mesh_path_create_req));
	if (!req)
		return;

	req->vif_idx = vif->vif_index;
	memcpy(&req->tgt_mac_addr, tgt_addr, ETH_ALEN);

	vif->ap.flags |= CLS_WIFI_AP_CREATE_MESH_PATH;

	/* Send the MESH_PATH_CREATE_REQ message to UMAC FW */
	cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_mesh_path_update_req(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *vif, const u8 *tgt_addr,
								   const u8 *p_nhop_addr, struct mesh_path_update_cfm *cfm)
{
	// Message to send
	struct mesh_path_update_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the MESH_PATH_UPDATE_REQ message */
	req = cls_wifi_msg_zalloc(MESH_PATH_UPDATE_REQ, TASK_MESH, DRV_TASK_ID,
						  sizeof(struct mesh_path_update_req));
	if (!req) {
		return -ENOMEM;
	}

	req->delete = (p_nhop_addr == NULL);
	req->vif_idx = vif->vif_index;
	memcpy(&req->tgt_mac_addr, tgt_addr, ETH_ALEN);

	if (p_nhop_addr) {
		memcpy(&req->nhop_mac_addr, p_nhop_addr, ETH_ALEN);
	}

	/* Send the MESH_PATH_UPDATE_REQ message to UMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, MESH_PATH_UPDATE_CFM, cfm);
}

void cls_wifi_send_mesh_proxy_add_req(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *vif, u8 *ext_addr)
{
	// Message to send
	struct mesh_proxy_add_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the MESH_PROXY_ADD_REQ message */
	req = cls_wifi_msg_zalloc(MESH_PROXY_ADD_REQ, TASK_MESH, DRV_TASK_ID,
						  sizeof(struct mesh_proxy_add_req));

	if (req) {
		req->vif_idx = vif->vif_index;
		memcpy(&req->ext_sta_addr, ext_addr, ETH_ALEN);

		/* Send the MESH_PROXY_ADD_REQ message to UMAC FW */
		cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
	}
}

int cls_wifi_send_tdls_peer_traffic_ind_req(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *cls_wifi_vif)
{
	struct tdls_peer_traffic_ind_req *tdls_peer_traffic_ind_req;

	if (!cls_wifi_vif->sta.tdls_sta)
		return -ENOLINK;

	/* Build the TDLS_PEER_TRAFFIC_IND_REQ message */
	tdls_peer_traffic_ind_req = cls_wifi_msg_zalloc(TDLS_PEER_TRAFFIC_IND_REQ, TASK_TDLS, DRV_TASK_ID,
										   sizeof(struct tdls_peer_traffic_ind_req));

	if (!tdls_peer_traffic_ind_req)
		return -ENOMEM;

	/* Set parameters for the TDLS_PEER_TRAFFIC_IND_REQ message */
	tdls_peer_traffic_ind_req->vif_index = cls_wifi_vif->vif_index;
	tdls_peer_traffic_ind_req->sta_idx = cls_wifi_vif->sta.tdls_sta->sta_idx;
	memcpy(&(tdls_peer_traffic_ind_req->peer_mac_addr.array[0]),
		   cls_wifi_vif->sta.tdls_sta->mac_addr, ETH_ALEN);
	tdls_peer_traffic_ind_req->dialog_token = 0; // check dialog token value
	tdls_peer_traffic_ind_req->last_tid = cls_wifi_vif->sta.tdls_sta->tdls.last_tid;
	tdls_peer_traffic_ind_req->last_sn = cls_wifi_vif->sta.tdls_sta->tdls.last_sn;

	/* Send the TDLS_PEER_TRAFFIC_IND_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, tdls_peer_traffic_ind_req, 0, 0, NULL);
}

int cls_wifi_send_config_monitor_req(struct cls_wifi_hw *cls_wifi_hw,
								 struct cfg80211_chan_def *chandef,
								 struct me_config_monitor_cfm *cfm)
{
	struct me_config_monitor_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the ME_CONFIG_MONITOR_REQ message */
	req = cls_wifi_msg_zalloc(ME_CONFIG_MONITOR_REQ, TASK_ME, DRV_TASK_ID,
								   sizeof(struct me_config_monitor_req));
	if (!req)
		return -ENOMEM;

	if (chandef) {
		req->chan_set = true;
		cfg80211_to_cls_wifi_chan(chandef, &req->chan);

		if (cls_wifi_hw->phy.limit_bw)
			limit_chan_bw(&req->chan.type, req->chan.prim20_freq, &req->chan.center1_freq);
	} else {
		 req->chan_set = false;
	}

	req->uf = cls_wifi_hw->radio_params->uf;

	/* Send the ME_CONFIG_MONITOR_REQ message to FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, ME_CONFIG_MONITOR_CFM, cfm);
}

int cls_wifi_send_tdls_chan_switch_req(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_vif *cls_wifi_vif,
								   struct cls_wifi_sta *cls_wifi_sta, bool sta_initiator,
								   u8 oper_class, struct cfg80211_chan_def *chandef,
								   struct tdls_chan_switch_cfm *cfm)
{
	struct tdls_chan_switch_req *tdls_chan_switch_req;

	/* Build the TDLS_CHAN_SWITCH_REQ message */
	tdls_chan_switch_req = cls_wifi_msg_zalloc(TDLS_CHAN_SWITCH_REQ, TASK_TDLS, DRV_TASK_ID,
										   sizeof(struct tdls_chan_switch_req));

	if (!tdls_chan_switch_req)
		return -ENOMEM;

	/* Set parameters for the TDLS_CHAN_SWITCH_REQ message */
	tdls_chan_switch_req->vif_index = cls_wifi_vif->vif_index;
	tdls_chan_switch_req->sta_idx = cls_wifi_sta->sta_idx;
	memcpy(&(tdls_chan_switch_req->peer_mac_addr.array[0]),
		   cls_wifi_sta_addr(cls_wifi_sta), ETH_ALEN);
	tdls_chan_switch_req->initiator = sta_initiator;
	cfg80211_to_cls_wifi_chan(chandef, &tdls_chan_switch_req->chan);
	tdls_chan_switch_req->op_class = oper_class;

	/* Send the TDLS_CHAN_SWITCH_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, tdls_chan_switch_req, 1, TDLS_CHAN_SWITCH_CFM, cfm);
}

int cls_wifi_send_tdls_cancel_chan_switch_req(struct cls_wifi_hw *cls_wifi_hw,
										  struct cls_wifi_vif *cls_wifi_vif,
										  struct cls_wifi_sta *cls_wifi_sta,
										  struct tdls_cancel_chan_switch_cfm *cfm)
{
	struct tdls_cancel_chan_switch_req *tdls_cancel_chan_switch_req;

	/* Build the TDLS_CHAN_SWITCH_REQ message */
	tdls_cancel_chan_switch_req = cls_wifi_msg_zalloc(TDLS_CANCEL_CHAN_SWITCH_REQ, TASK_TDLS, DRV_TASK_ID,
										   sizeof(struct tdls_cancel_chan_switch_req));
	if (!tdls_cancel_chan_switch_req)
		return -ENOMEM;

	/* Set parameters for the TDLS_CHAN_SWITCH_REQ message */
	tdls_cancel_chan_switch_req->vif_index = cls_wifi_vif->vif_index;
	tdls_cancel_chan_switch_req->sta_idx = cls_wifi_sta->sta_idx;
	memcpy(&(tdls_cancel_chan_switch_req->peer_mac_addr.array[0]),
		   cls_wifi_sta_addr(cls_wifi_sta), ETH_ALEN);

	/* Send the TDLS_CHAN_SWITCH_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, tdls_cancel_chan_switch_req, 1, TDLS_CANCEL_CHAN_SWITCH_CFM, cfm);
}

int cls_wifi_send_mm_rc_stats(struct cls_wifi_hw *cls_wifi_hw, u16 sta_idx,
						  struct mm_rc_stats_cfm *cfm)
{
	struct mm_rc_stats_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the MM_RC_STATS_REQ message */
	req = cls_wifi_msg_zalloc(MM_RC_STATS_REQ, TASK_MM, DRV_TASK_ID,
						  sizeof(struct mm_rc_stats_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the MM_RC_STATS_REQ message */
	req->sta_idx = sta_idx;

	/* Send the MM_RC_STATS_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, MM_RC_STATS_CFM, cfm);
}

int cls_wifi_send_mm_rc_set_rate(struct cls_wifi_hw *cls_wifi_hw, u16 sta_idx,
							 u32 rate_config, bool fix_rate)
{
	struct mm_rc_set_rate_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the MM_RC_SET_RATE_REQ message */
	req = cls_wifi_msg_zalloc(MM_RC_SET_RATE_REQ, TASK_MM, DRV_TASK_ID,
						  sizeof(struct mm_rc_set_rate_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the MM_RC_SET_RATE_REQ message */
	req->sta_idx = sta_idx;
	req->rate_config = rate_config;
	req->fix_rate = fix_rate;

	/* Send the MM_RC_SET_RATE_REQ message to FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

#ifdef CONFIG_CLS_WIFI_HEMU_TX
int cls_wifi_send_mm_ul_parameters(struct cls_wifi_hw *cls_wifi_hw)
{
	struct mm_ul_parameters_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the MM_UL_PARAMETERS_REQ message */
	req = cls_wifi_msg_zalloc(MM_UL_PARAMETERS_REQ, TASK_MM, DRV_TASK_ID,
						  sizeof(struct mm_ul_parameters_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the MM_UL_PARAMETERS_REQ message */
	*req = cls_wifi_hw->ul_params;

	/* Send the MM_UL_PARAMETERS_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, MM_UL_PARAMETERS_CFM, NULL);
}

int cls_wifi_send_mm_dl_parameters(struct cls_wifi_hw *cls_wifi_hw)
{
	struct mm_dl_parameters_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the MM_DL_PARAMETERS_REQ message */
	req = cls_wifi_msg_zalloc(MM_DL_PARAMETERS_REQ, TASK_MM, DRV_TASK_ID,
						  sizeof(struct mm_dl_parameters_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the MM_DL_PARAMETERS_REQ message */
	*req = cls_wifi_hw->dl_params;

	/* Send the MM_DL_PARAMETERS_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, MM_DL_PARAMETERS_CFM, NULL);
}
#endif /* CONFIG_CLS_WIFI_HEMU_TX */

#ifdef CONFIG_CLS_WIFI_BFMER
void cls_wifi_send_bfmer_enable(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_sta *cls_wifi_sta,
							struct station_parameters *params)
{
	struct mm_bfmer_enable_req *bfmer_en_req;
	u32 bfmee_su_capability_flag;
	u32 bfmee_mu_capability_flag;
	void *mcs_nss_supp;
	__le32 capability;
	u16 rx_mcs_map;
	u8 rx_nss = 0;

	void *capa;
	if (cls_wifi_sta->he)
	{
		capa = (struct ieee80211_he_cap_elem *)params->he_capa;
		mcs_nss_supp = (struct ieee80211_he_mcs_nss_supp *)(capa + 1);
		bfmee_su_capability_flag = IEEE80211_HE_PHY_CAP4_SU_BEAMFORMEE;

		// In any case, the capability has to be at least equal to 4 STS hence MU
		// beamformee is always supported by HE STA.
		// However, in this SW version, the HE MU beamforming is not implemented on the AP
		bfmee_mu_capability_flag =
			IEEE80211_HE_PHY_CAP4_BEAMFORMEE_MAX_STS_UNDER_80MHZ_MASK;
	}
	else
	{
		capa = (struct ieee80211_vht_cap *)params->vht_capa;
		mcs_nss_supp = (struct ieee80211_vht_mcs_info *)(capa + 1);
		bfmee_su_capability_flag = IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE;
		bfmee_mu_capability_flag = IEEE80211_VHT_CAP_MU_BEAMFORMEE_CAPABLE;
	}

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (!capa) {
		goto end;
	}

	if (cls_wifi_sta->he)
	{
		capability = ((struct ieee80211_he_cap_elem *)capa)->phy_cap_info[4];
		// In this SW version, the HE MU beamforming is not implemented on the AP =>
		// clear the MU Beamformee field.
		capability &= ~IEEE80211_HE_PHY_CAP4_BEAMFORMEE_MAX_STS_UNDER_80MHZ_MASK;
	}
	else
	{
		capability = ((struct ieee80211_vht_cap *)capa)->vht_cap_info;
	}

	if (!(capability & bfmee_su_capability_flag)) {
		goto end;
	}

	if (cls_wifi_sta->he)
	{
		rx_mcs_map = le16_to_cpu(((struct ieee80211_he_mcs_nss_supp *)mcs_nss_supp)->rx_mcs_80);
		rx_nss = cls_wifi_bfmer_get_rx_nss(rx_mcs_map);
	}
	else
	{
		rx_mcs_map = le16_to_cpu(((struct ieee80211_vht_mcs_info *)mcs_nss_supp)->rx_mcs_map);
		rx_nss = cls_wifi_bfmer_get_rx_nss(rx_mcs_map);
	}

	/* Allocate a structure that will contain the beamforming report */
	if (cls_wifi_bfmer_report_add(cls_wifi_hw, cls_wifi_sta, CLS_WIFI_BFMER_REPORT_SPACE_SIZE))
	{
		goto end;
	}

	/* Build the MM_BFMER_ENABLE_REQ message */
	bfmer_en_req = cls_wifi_msg_zalloc(MM_BFMER_ENABLE_REQ, TASK_MM, DRV_TASK_ID,
								   sizeof(struct mm_bfmer_enable_req));

	/* Check message allocation */
	if (!bfmer_en_req) {
		/* Free memory allocated for the report */
		cls_wifi_bfmer_report_del(cls_wifi_hw, cls_wifi_sta);

		/* Do not use beamforming */
		goto end;
	}

	/* Provide DMA address to the MAC */
	bfmer_en_req->host_bfr_addr = cls_wifi_sta->bfm_report->dma_addr;
	bfmer_en_req->host_bfr_size = CLS_WIFI_BFMER_REPORT_SPACE_SIZE;
	bfmer_en_req->sta_idx = cls_wifi_sta->sta_idx;
	bfmer_en_req->aid = cls_wifi_sta->aid;
	bfmer_en_req->rx_nss = rx_nss;

	// In this sw version, allow only MU beamforming for VHT mode.
	// => HE beamforming is always SU.
	if (capability & bfmee_mu_capability_flag) {
		bfmer_en_req->mu_bfmee = true;
	} else {
		bfmer_en_req->mu_bfmee = false;
	}
	/* Send the MM_BFMER_EN_REQ message to LMAC FW */
	cls_wifi_send_msg(cls_wifi_hw, bfmer_en_req, 0, 0, NULL);

end:
	return;
}
#endif /* CONFIG_CLS_WIFI_BFMER */

/**********************************************************************
 *	Debug Messages
 *********************************************************************/
int cls_wifi_send_dbg_trigger_req(struct cls_wifi_hw *cls_wifi_hw, char *msg)
{
	struct mm_dbg_trigger_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the MM_DBG_TRIGGER_REQ message */
	req = cls_wifi_msg_zalloc(MM_DBG_TRIGGER_REQ, TASK_MM, DRV_TASK_ID,
						  sizeof(struct mm_dbg_trigger_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the MM_DBG_TRIGGER_REQ message */
	strncpy(req->error, msg, sizeof(req->error));

	/* Send the MM_DBG_TRIGGER_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, -1, NULL);
}

int cls_wifi_send_dbg_mem_read_req(struct cls_wifi_hw *cls_wifi_hw, u32 mem_addr,
							   struct dbg_mem_read_cfm *cfm)
{
	struct dbg_mem_read_req *mem_read_req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the DBG_MEM_READ_REQ message */
	mem_read_req = cls_wifi_msg_zalloc(DBG_MEM_READ_REQ, TASK_DBG, DRV_TASK_ID,
								   sizeof(struct dbg_mem_read_req));
	if (!mem_read_req)
		return -ENOMEM;

	/* Set parameters for the DBG_MEM_READ_REQ message */
	mem_read_req->memaddr = mem_addr;

	/* Send the DBG_MEM_READ_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, mem_read_req, 1, DBG_MEM_READ_CFM, cfm);
}

int cls_wifi_send_dbg_mem_write_req(struct cls_wifi_hw *cls_wifi_hw, u32 mem_addr,
								u32 mem_data)
{
	struct dbg_mem_write_req *mem_write_req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the DBG_MEM_WRITE_REQ message */
	mem_write_req = cls_wifi_msg_zalloc(DBG_MEM_WRITE_REQ, TASK_DBG, DRV_TASK_ID,
									sizeof(struct dbg_mem_write_req));
	if (!mem_write_req)
		return -ENOMEM;

	/* Set parameters for the DBG_MEM_WRITE_REQ message */
	mem_write_req->memaddr = mem_addr;
	mem_write_req->memdata = mem_data;

	/* Send the DBG_MEM_WRITE_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, mem_write_req, 1, DBG_MEM_WRITE_CFM, NULL);
}

int cls_wifi_send_dbg_set_mod_filter_req(struct cls_wifi_hw *cls_wifi_hw, u32 filter)
{
	struct dbg_set_mod_filter_req *set_mod_filter_req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the DBG_SET_MOD_FILTER_REQ message */
	set_mod_filter_req =
		cls_wifi_msg_zalloc(DBG_SET_MOD_FILTER_REQ, TASK_DBG, DRV_TASK_ID,
						sizeof(struct dbg_set_mod_filter_req));
	if (!set_mod_filter_req)
		return -ENOMEM;

	/* Set parameters for the DBG_SET_MOD_FILTER_REQ message */
	set_mod_filter_req->mod_filter = filter;

	/* Send the DBG_SET_MOD_FILTER_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, set_mod_filter_req, 1, DBG_SET_MOD_FILTER_CFM, NULL);
}

int cls_wifi_send_dbg_set_sev_filter_req(struct cls_wifi_hw *cls_wifi_hw, u32 filter)
{
	struct dbg_set_sev_filter_req *set_sev_filter_req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the DBG_SET_SEV_FILTER_REQ message */
	set_sev_filter_req =
		cls_wifi_msg_zalloc(DBG_SET_SEV_FILTER_REQ, TASK_DBG, DRV_TASK_ID,
						sizeof(struct dbg_set_sev_filter_req));
	if (!set_sev_filter_req)
		return -ENOMEM;

	/* Set parameters for the DBG_SET_SEV_FILTER_REQ message */
	set_sev_filter_req->sev_filter = filter;

	/* Send the DBG_SET_SEV_FILTER_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, set_sev_filter_req, 1, DBG_SET_SEV_FILTER_CFM, NULL);
}

int cls_wifi_send_dbg_get_sys_stat_req(struct cls_wifi_hw *cls_wifi_hw,
								   struct dbg_get_sys_stat_cfm *cfm)
{
	void *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Allocate the message */
	req = cls_wifi_msg_zalloc(DBG_GET_SYS_STAT_REQ, TASK_DBG, DRV_TASK_ID, 0);
	if (!req)
		return -ENOMEM;

	/* Send the DBG_MEM_READ_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, DBG_GET_SYS_STAT_CFM, cfm);
}

int cls_wifi_send_dbg_get_mib_req(struct cls_wifi_hw *cls_wifi_hw,
								   struct dbg_get_mib_cfm *cfm)
{
	struct dbg_get_mib_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Allocate the message */
	req = (struct dbg_get_mib_req*)cls_wifi_msg_zalloc(DBG_GET_SYS_MIB_REG, TASK_DBG, DRV_TASK_ID,
														sizeof(struct dbg_get_mib_req));
	if (!req)
		return -ENOMEM;

	req->get_mib_flag = MIB_UPDATE_ALL;
	req->not_ignore_cfm = 1;

	/* Send the DBG_MEM_READ_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, DBG_GET_SYS_MIB_CFM, cfm);
}

int cls_wifi_send_dbg_get_phy_dfx_req(struct cls_wifi_hw *cls_wifi_hw,
								   struct dbg_get_phy_dfx_cfm *cfm)
{
	struct dbg_get_phy_dfx_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Allocate the message */
	req = (struct dbg_get_phy_dfx_req*)cls_wifi_msg_zalloc(DBG_GET_SYS_PHY_DFX_REG, TASK_DBG, DRV_TASK_ID,
														sizeof(struct dbg_get_phy_dfx_req));
	if (!req)
		return -ENOMEM;

	req->get_mib_flag = PHYDFX_UPDATE_ALL;
	req->not_ignore_cfm = 1;

	/* Send the DBG_MEM_READ_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, DBG_GET_SYS_PHY_DFX_CFM, cfm);
}

int cls_wifi_send_dbg_get_dbgcnt_req(struct cls_wifi_hw *cls_wifi_hw)
{
	struct dbg_get_dbgcnt_req *req;
	struct dbg_get_dbgcnt_cfm cfm;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Allocate the message */
	req = (struct dbg_get_dbgcnt_req*)cls_wifi_msg_zalloc(DBG_GET_DBGCNT_REG,
			TASK_DBG, DRV_TASK_ID, sizeof(struct dbg_get_dbgcnt_req));
	if (!req)
		return -ENOMEM;

	req->reset = 0;

	cls_wifi_send_msg(cls_wifi_hw, req, 1, DBG_GET_DBGCNT_CFM, &cfm);

	return cfm.success ? 0 : -1;
}

int cls_wifi_send_dbg_pct_stat_request(struct cls_wifi_hw *cls_wifi_hw,
					struct dbg_pct_stat_req *conf,
					struct dbg_pct_stat_cfm *cfm)
{
	struct dbg_pct_stat_req *req;

	/* Build the DBG_PCT_STATS_REQ message */
	req = cls_wifi_msg_zalloc(DBG_PCT_STATS_REQ, TASK_DBG, DRV_TASK_ID,
				sizeof(struct dbg_pct_stat_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, conf, sizeof(struct dbg_pct_stat_req));
	/* Send the DBG_PCT_STATS_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, DBG_PCT_STATS_CFM, cfm);
}

int cls_wifi_send_dbg_profile_stat_request(struct cls_wifi_hw *cls_wifi_hw,
						struct dbg_profile_stat_req *conf,
						struct dbg_profile_stat_cfm *cfm)
{
	struct dbg_profile_stat_req *req;

	/* Build the DBG_PROFILE_STATS_REQ message */
	req = cls_wifi_msg_zalloc(DBG_PROFILE_STATS_REQ, TASK_DBG, DRV_TASK_ID,
			sizeof(struct dbg_profile_stat_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, conf, sizeof(struct dbg_profile_stat_req));
	/* Send the DBG_PROFILE_STATS_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, DBG_PROFILE_STATS_CFM, cfm);
}

int cls_wifi_send_dbg_get_mgmt_req(struct cls_wifi_hw *cls_wifi_hw, u8 vif_index,
		 struct dbg_get_mgmt_stats_cfm *cfm)
{
	struct dbg_get_mgmt_stats_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Allocate the message */
	req = (struct dbg_get_mgmt_stats_req *)cls_wifi_msg_zalloc(DBG_GET_MGMT_STATS_REQ,
			TASK_DBG, DRV_TASK_ID, sizeof(struct dbg_get_mgmt_stats_req));
	if (!req)
		return -ENOMEM;

	req->vif_index = vif_index;

	/* Send the DBG_GET_MGMT_STATS_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, DBG_GET_MGMT_STATS_CFM, cfm);
}

int cls_wifi_send_dbg_reset_mgmt_req(struct cls_wifi_hw *cls_wifi_hw, u8 vif_index)
{
	struct dbg_reset_mgmt_stats_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Allocate the message */
	req = (struct dbg_reset_mgmt_stats_req *)cls_wifi_msg_zalloc(DBG_RESET_MGMT_STATS_REQ,
			TASK_DBG, DRV_TASK_ID, sizeof(struct dbg_reset_mgmt_stats_req));
	if (!req)
		return -ENOMEM;

	req->vif_index = vif_index;

	return cls_wifi_send_msg(cls_wifi_hw, req, 1, DBG_RESET_MGMT_STATS_CFM, NULL);
}

int cls_wifi_send_cfg_rssi_req(struct cls_wifi_hw *cls_wifi_hw, u8 vif_index, int rssi_thold, u32 rssi_hyst)
{
	struct mm_cfg_rssi_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the MM_CFG_RSSI_REQ message */
	req = cls_wifi_msg_zalloc(MM_CFG_RSSI_REQ, TASK_MM, DRV_TASK_ID,
						  sizeof(struct mm_cfg_rssi_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the MM_CFG_RSSI_REQ message */
	req->vif_index = vif_index;
	req->rssi_thold = (s8)rssi_thold;
	req->rssi_hyst = (u8)rssi_hyst;

	/* Send the MM_CFG_RSSI_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_ampdu_max_size_req(struct cls_wifi_hw *cls_wifi_hw, int ampdu_max)
{
	struct dht_set_ampdu_max_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the DHT_SET_AMPDU_MAX_REQ message */
	req = cls_wifi_msg_zalloc(DHT_SET_AMPDU_MAX_REQ, TASK_DHT, DRV_TASK_ID,
						  sizeof(struct dht_set_ampdu_max_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the MM_CFG_RSSI_REQ message */
	req->ampdu_max_size = ampdu_max;

	/* Send the MM_CFG_RSSI_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_smm_idx_req(struct cls_wifi_hw *cls_wifi_hw, int smm_indx)
{
	struct dht_set_smm_idx_req *req;

	req = cls_wifi_msg_zalloc(DHT_SET_SMM_IDX_DBG_REQ, TASK_DHT, DRV_TASK_ID,
						  sizeof(struct dht_set_smm_idx_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the MM_CFG_RSSI_REQ message */
	req->smm_idx = smm_indx;

	/* Send the MM_CFG_RSSI_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_ampdu_prot_req(struct cls_wifi_hw *cls_wifi_hw, int prot_type)
{
	struct dht_set_ampdu_prt_req *req;

	req = cls_wifi_msg_zalloc(DHT_D2K_SET_AMPDU_PRT_REQ, TASK_DHT, DRV_TASK_ID,
			sizeof(struct dht_set_ampdu_prt_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the MM_CFG_RSSI_REQ message */
	req->type = prot_type;

	/* Send the MM_CFG_RSSI_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_txop_en_req(struct cls_wifi_hw *cls_wifi_hw, int txop_en)
{
	struct dht_set_txop_en_req *req;

	req = cls_wifi_msg_zalloc(DHT_D2K_SET_TXOP_EN_REQ, TASK_DHT, DRV_TASK_ID,
			sizeof(struct dht_set_ampdu_prt_req));
	if (!req)
		return -ENOMEM;

	req->enable = txop_en;

	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_set_rd_max_num_thrd_req(struct cls_wifi_hw *cls_wifi_hw, u8 thrd)
{
	struct mm_rd_max_num_thrd_req *req;

	req = cls_wifi_msg_zalloc(MM_RD_MAX_NUM_THRD_REQ, TASK_MM, DRV_TASK_ID,
			sizeof(struct mm_rd_max_num_thrd_req));
	if (!req)
		return -ENOMEM;

	req->option = RD_MAX_NUM_THRD_OPT_SET;
	req->rd_max_num_thrd = thrd;

	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_get_rd_max_num_thrd_req(struct cls_wifi_hw *cls_wifi_hw,
				struct mm_rd_max_num_thrd_cfm *cfm)
{
	struct mm_rd_max_num_thrd_req *req;

	req = cls_wifi_msg_zalloc(MM_RD_MAX_NUM_THRD_REQ, TASK_MM, DRV_TASK_ID,
			sizeof(struct mm_rd_max_num_thrd_req));
	if (!req)
		return -ENOMEM;

	req->option = RD_MAX_NUM_THRD_OPT_GET;

	return cls_wifi_send_msg(cls_wifi_hw, req, 1, MM_RD_MAX_NUM_THRD_CFM, cfm);
}

int cls_wifi_send_set_rd_channel_req(struct cls_wifi_hw *cls_wifi_hw, u8 channel)
{
	struct mm_rd_chan_req *req;

	req = cls_wifi_msg_zalloc(MM_RD_CHAN_REQ, TASK_MM, DRV_TASK_ID,
			sizeof(struct mm_rd_chan_req));
	if (!req)
		return -ENOMEM;

	req->option = RD_CHAN_OPT_SET;
	req->channel = channel;

	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_get_rd_channel_req(struct cls_wifi_hw *cls_wifi_hw,
				struct mm_rd_chan_cfm *cfm)
{
	struct mm_rd_chan_req *req;

	req = cls_wifi_msg_zalloc(MM_RD_CHAN_REQ, TASK_MM, DRV_TASK_ID,
			sizeof(struct mm_rd_chan_req));
	if (!req)
		return -ENOMEM;

	req->option = RD_CHAN_OPT_GET;

	return cls_wifi_send_msg(cls_wifi_hw, req, 1, MM_RD_CHAN_CFM, cfm);
}

int cls_wifi_send_set_rd_dbg_req(struct cls_wifi_hw *cls_wifi_hw, bool lvl)
{
	struct mm_rd_dbg_req *req;

	req = cls_wifi_msg_zalloc(MM_RD_DBG_REQ, TASK_MM, DRV_TASK_ID,
			sizeof(struct mm_rd_dbg_req));
	if (!req)
		return -ENOMEM;

	req->rd_dbg_level = lvl;

	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_set_rd_agc_war_req(struct cls_wifi_hw *cls_wifi_hw, u8 enable)
{
	struct mm_rd_agc_war_req *req;

	req = cls_wifi_msg_zalloc(MM_RD_AGC_WAR_REQ, TASK_MM, DRV_TASK_ID,
			sizeof(struct mm_rd_agc_war_req));
	if (!req)
		return -ENOMEM;

	req->option = RD_AGC_WAR_OPT_SET;

	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_log_to_uart_req(struct cls_wifi_hw *cls_wifi_hw, int enable)
{
	struct dht_log_to_uart_req *req;

	req = cls_wifi_msg_zalloc(DHT_LOG_TO_UART_REQ, TASK_DHT, DRV_TASK_ID,
			sizeof(struct dht_log_to_uart_req));
	if (!req)
		return -ENOMEM;

	req->enable = enable;

	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_get_rd_agc_war_req(struct cls_wifi_hw *cls_wifi_hw,
				struct mm_rd_agc_war_cfm *cfm)
{
	struct mm_rd_agc_war_req *req;

	req = cls_wifi_msg_zalloc(MM_RD_AGC_WAR_REQ, TASK_MM, DRV_TASK_ID,
			sizeof(struct mm_rd_agc_war_req));
	if (!req)
		return -ENOMEM;

	req->option = RD_AGC_WAR_OPT_GET;

	return cls_wifi_send_msg(cls_wifi_hw, req, 1, MM_RD_AGC_WAR_CFM, cfm);
}

int cls_wifi_rts_cts_dbg_req(struct cls_wifi_hw *cls_wifi_hw, int enable)
{
	struct dht_rts_cts_dbg_req *req;

	req = cls_wifi_msg_zalloc(DHT_RTS_CTS_DBG_REQ, TASK_DHT, DRV_TASK_ID,
			sizeof(struct dht_rts_cts_dbg_req));
	if (!req)
		return -ENOMEM;

	req->enable_rts_cts_dump = enable;

	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_dump_edma_info_req(struct cls_wifi_hw *cls_wifi_hw)
{
	struct dht_dump_edma_info_req *req;

	req = cls_wifi_msg_zalloc(DHT_DUMP_EDMA_INFO_REQ, TASK_DHT, DRV_TASK_ID,
			sizeof(struct dht_dump_edma_info_req));
	if (!req)
		return -ENOMEM;

	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_m3k_boc_reg_req(struct cls_wifi_hw *cls_wifi_hw, int enable)
{
	struct dht_m3k_boc_reg_req *req;

	req = cls_wifi_msg_zalloc(DHT_SET_M3K_BOC_REG_REQ, TASK_DHT, DRV_TASK_ID,
			sizeof(struct dht_m3k_boc_reg_req));
	if (!req)
		return -ENOMEM;

	req->enable = enable;

	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_set_tx_rx_loop_online_en_req(struct cls_wifi_hw *cls_wifi_hw,
						struct tx_rx_loop_online_en_req *tx_rx_loop_online_en_config)
{
	struct tx_rx_loop_online_en_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the TX_RX_LOOP_ONLINE_EN_REQ message */
	req = cls_wifi_msg_zalloc(DHT_TX_RX_LOOP_ONLINE_REQ, TASK_DHT, DRV_TASK_ID,
			sizeof(struct tx_rx_loop_online_en_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the IRF_TX_RX_LOOP_ONLINE_REQ message */
	memcpy(req, tx_rx_loop_online_en_config, sizeof(struct tx_rx_loop_online_en_req));

	/* Send the MM_TX_RX_LOOP_ONLINE_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_pppc_txpower_manually_req(struct cls_wifi_hw *cls_wifi_hw, struct dht_set_pppc_req *tmp)
{
	struct dht_set_pppc_req *req;

	if (!tmp)
		return -EINVAL;

	req = cls_wifi_msg_zalloc(DHT_SET_CMCC_PPPC_REQ, TASK_DHT, DRV_TASK_ID,
			sizeof(struct dht_set_pppc_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, tmp, sizeof(struct dht_set_pppc_req));

	/* Send the DHT_SET_CMCC_PPPC_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_pppc_txpower_show_record_req(struct cls_wifi_hw *cls_wifi_hw)
{
	void *req;

	/* Allocate the message */
	req = cls_wifi_msg_zalloc(DHT_DUMP_CMCC_PPPC_TXPWR_RECORD_REQ, TASK_DHT, DRV_TASK_ID, 0);
	if (!req)
		return -ENOMEM;

	/* Send the DHT_DUMP_CMCC_PPPC_TXPWR_RECORD_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_sync_pppc_txpower_req(struct cls_wifi_hw *cls_wifi_hw, struct cfg80211_chan_def *chandef)
{
	union dht_sync_pppc_txpower_req *req;
	u8 chan_idx;
	int i;
	bool found = false;

	chan_idx = ieee80211_frequency_to_channel(chandef->chan->center_freq);

	req = cls_wifi_msg_zalloc(DHT_SYNC_CMCC_PPPC_TXPWR_REQ, TASK_DHT, DRV_TASK_ID,
			sizeof(union dht_sync_pppc_txpower_req));
	if (!req)
		return -ENOMEM;

	/* update txpower list for the DHT_SYNC_CMCC_PPPC_TXPWR_REQ message */
	if (RADIO_5G_INDEX == cls_wifi_hw->radio_idx) {
		memset(req, CLS_CMCC_PPPC_DEFAULT_TXPOWER, sizeof(struct pppc_5g_power_list));

		for (i = 0; i < PPPC_5G_TX_POWER_SIZE; i++) {
			if (chan_idx == cls_pppc_5g_tx_power_list[i].chan) {
				memcpy(req, &(cls_pppc_5g_tx_power_list[i].power_list),
							sizeof(struct pppc_5g_power_list));

				found = true;
				break;
			}
		}
	} else {
		memset(req, CLS_CMCC_PPPC_DEFAULT_TXPOWER, sizeof(struct pppc_2g_power_list));

		for (i = 0; i < PPPC_2G_TX_POWER_SIZE; i++) {
			if (chan_idx == cls_pppc_2g_tx_power_list[i].chan) {
				memcpy(req, &(cls_pppc_2g_tx_power_list[i].power_list),
							sizeof(struct pppc_2g_power_list));

				found = true;
				break;
			}
		}
	}

	if (!found) {
		pr_err("[%s] invald param: radio %d chan_idx %d\n",
					__func__, cls_wifi_hw->radio_idx, chan_idx);

		return -EINVAL;
	}

	pr_info("[%s] radio %d chan_idx %d\n", __func__, cls_wifi_hw->radio_idx, chan_idx);

	/* Send the DHT_SYNC_CMCC_PPPC_TXPWR_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_dyn_pwr_offset_req(struct cls_wifi_hw *cls_wifi_hw, int8_t offset)
{
	struct dht_dyn_pwr_offset_req *req;

	req = cls_wifi_msg_zalloc(DHT_SET_DYN_PWR_OFFSET_REQ, TASK_DHT, DRV_TASK_ID,
			sizeof(struct dht_dyn_pwr_offset_req));
	if (!req)
		return -ENOMEM;

	req->offset = offset;

	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}
#endif

#if CONFIG_CLS_SMTANT
int cls_wifi_send_mm_set_smant(struct cls_wifi_hw *cls_wifi_hw, struct mm_smart_antenna_req *smant_cfg)
{
	struct mm_smart_antenna_req *req;

	if (!smant_cfg)
		return -EINVAL;

	req = cls_wifi_msg_zalloc(MM_SET_SMANT_CFG_REQ, TASK_MM, DRV_TASK_ID,
			sizeof(struct mm_smart_antenna_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, smant_cfg, sizeof(struct mm_smart_antenna_req));

	/* Send the DHT_SET_CMCC_PPPC_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}
#endif

int cls_wifi_send_message(struct cls_wifi_hw *cls_wifi_hw, u8 *buf)
{
	struct cls_wifi_msg *msg = (struct cls_wifi_msg *)buf;
	u8 *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);
	/* Build the message */
	req = cls_wifi_msg_zalloc((lmac_msg_id_t)msg->id, (lmac_task_id_t)msg->dest_id,
                          (lmac_task_id_t)msg->src_id, msg->param_len);
	if (!req)
		return -ENOMEM;

	memcpy(req, msg->param, msg->param_len);
	/* Send the message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

#ifdef __KERNEL__
int cls_wifi_send_clicmd_cdf_req(struct cls_wifi_hw *cls_wifi_hw, void *para)
{
	struct cdf_stats *req;

	/* Build the CAL_TX_SU_REQ message */
	req = cls_wifi_msg_zalloc(DHT_CDF_STATS_REQ, TASK_DHT, TASK_API,
		sizeof(struct cal_tx_su_req));
	if (req == NULL)
		return -ENOMEM;

	/* Set parameters for the DHT_CDF_STATS_REQ message */
	memcpy(req, para, sizeof(struct cdf_stats));
	/* Send the DHT_CDF_STATS_REQ message to FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}
#endif

/**********************************************************************
 *	Calibration Messages
 *********************************************************************/
/**
 * cls_wifi_hw: cls_wifi environment per radio
 * msg_id: request ID
 * msg_len: request message length
 * msg: request message body
 * cfm: confimation buffer to hold the confirmation message from WPU
 *	  NULL: need to confirmation
 */
int cls_wifi_send_cal_msg_req(struct cls_wifi_hw *cls_wifi_hw,
						lmac_msg_id_t msg_id, int msg_len, void *msg,
						lmac_msg_id_t cfm_id, void *cfm)
{
#ifdef __KERNEL__
	uint32_t *cal_msg;
#else
	void *cal_msg;
	struct ipc_shared_env_tag *ipc_shared_env;
#endif
	int is_cfm = 0;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (!cls_wifi_hw || !msg || (msg_len <= 0) || (msg_id > CAL_MSG_MAX))
		return -EINVAL;
#ifndef __KERNEL__
	if (!cls_wifi_hw->ipc_env)
		return -EINVAL;

	ipc_shared_env = cls_wifi_hw->ipc_env->shared;
	if (!ipc_shared_env)
		return -EINVAL;
#endif
	/* Build calibration message */
	cal_msg = cls_wifi_msg_zalloc(msg_id, TASK_CAL, DRV_TASK_ID, msg_len);
	if (!cal_msg)
		return -ENOMEM;

	memcpy(cal_msg, msg, msg_len);
	if (cfm)
		is_cfm = 1;

	return cls_wifi_send_msg(cls_wifi_hw, cal_msg, is_cfm, cfm_id, cfm);
}


int cls_wifi_send_cal_rx_stats_req(struct cls_wifi_hw *cls_wifi_hw, uint16_t radio_id, uint32_t clear)
{
	struct cal_rx_stats_req *req;

	pr_info("%s: radio_id %u\n", __func__, radio_id);

	/* Build the CAL_RX_STATS_REQ message */
	req = cls_wifi_msg_zalloc(CAL_RX_STATS_REQ, TASK_CAL, DRV_TASK_ID,
		sizeof(struct cal_rx_stats_req));
	if (req == NULL)
		return -ENOMEM;

	/* Set parameters for the CAL_RX_STATS_REQ message */
	req->radio_id = radio_id;
	req->flags = 0;
	if (clear)
		req->flags |= CAL_RX_STATS_FLAG_RD_CLR;

	/* Send the CAL_RX_STATS_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, CAL_RX_STATS_CFM, &cls_wifi_hw->cal_env->rx_stats_cfm);
}

int cls_wifi_send_cal_rx_status_req(struct cls_wifi_hw *cls_wifi_hw, uint16_t radio_id)
{
	struct cal_rx_status_req *req;

	pr_info("%s: radio_id %u\n", __func__, radio_id);

	/* Build the CAL_RX_STATS_REQ message */
	req = cls_wifi_msg_zalloc(CAL_RX_STATUS_REQ, TASK_CAL, DRV_TASK_ID,
		sizeof(struct cal_rx_status_req));
	if (req == NULL)
		return -ENOMEM;

	/* Set parameters for the CAL_RX_STATUS_REQ message */
	req->radio_id = radio_id;
	req->flags = 0;

	/* Send the CAL_RX_STATUS_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, CAL_RX_STATUS_CFM, &cls_wifi_hw->cal_env->rx_status_cfm);
}

int cls_wifi_send_cal_tx_su_req(struct cls_wifi_hw *cls_wifi_hw, struct cal_tx_su_req *tx_param)
{
	struct cal_tx_su_req *req;

	pr_info("%s: payload length %u, en_sample %u, num_sample %u\n", __func__,
			tx_param->payload_len, tx_param->en_sample, tx_param->num_sample);

	/* Build the CAL_TX_SU_REQ message */
	req = cls_wifi_msg_zalloc(CAL_TX_SU_REQ, TASK_CAL, DRV_TASK_ID,
		sizeof(struct cal_tx_su_req));
	if (req == NULL)
		return -ENOMEM;

	/* Set parameters for the CAL_TX_SU_REQ message */
	memcpy(req, tx_param, sizeof(struct cal_tx_su_req));
	/* Send the CAL_TX_SU_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, CAL_TX_SU_CFM, &cls_wifi_hw->cal_env->tx_su_cfm);
}

int cls_wifi_send_cal_dif_smp_req(struct cls_wifi_hw *cls_wifi_hw, struct cal_dif_sample_req *dif_param)
{
	struct cal_dif_sample_req *req;

	pr_info("%s: sample num %u, channel num %u\n", __func__,
			dif_param->num_sample, dif_param->chan);

	/* Build the CAL_TX_SU_REQ message */
	req = cls_wifi_msg_zalloc(CAL_DIF_SMP_REQ, TASK_CAL, DRV_TASK_ID,
		sizeof(struct cal_dif_sample_req));
	if (req == NULL)
		return -ENOMEM;

	/* Set parameters for the CAL_DIF_SMP_REQ message */
	memcpy(req, dif_param, sizeof(struct cal_dif_sample_req));
	/* Send the CAL_DIF_SMP_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, CAL_DIF_SMP_CFM, &cls_wifi_hw->cal_env->dif_smp_cfm);
}

int cls_wifi_send_radar_detect_req(struct cls_wifi_hw *cls_wifi_hw,
							struct cal_radar_detect_req *radar_detect)
{
	struct cal_radar_detect_req *req;

	pr_info("%s: enable %u\n", __func__, radar_detect->enable);

	/* Build the CAL_RADAR_DETECT_REQ message */
	req = cls_wifi_msg_zalloc(CAL_RADAR_DETECT_REQ, TASK_CAL, DRV_TASK_ID,
		sizeof(struct cal_radar_detect_req));
	if (req == NULL)
		return -ENOMEM;

	/* Set parameters for the CAL_RADAR_DETECT_REQ message */
	memcpy(req, radar_detect, sizeof(struct cal_radar_detect_req));
	/* Send the CAL_RADAR_DETECT_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, CAL_RADAR_DETECT_CFM,
					&cls_wifi_hw->cal_env->radar_detect_cfm);
}

int cls_wifi_send_interference_detect_req(struct cls_wifi_hw *cls_wifi_hw,
							struct cal_interference_detect_req *interference_detect)
{
	struct cal_interference_detect_req *req;

	pr_info("%s: enable %u\n", __func__, interference_detect->enable);

	/* Build the CAL_INTERFERENCE_DETECT_REQ message */
	req = cls_wifi_msg_zalloc(CAL_INTERFERENCE_DETECT_REQ, TASK_CAL, DRV_TASK_ID,
		sizeof(struct cal_interference_detect_req));
	if (req == NULL)
		return -ENOMEM;

	/* Set parameters for the CAL_INTERFERENCE_DETECT_REQ message */
	memcpy(req, interference_detect, sizeof(struct cal_interference_detect_req));
	/* Send the CAL_INTERFERENCE_DETECT_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, CAL_INTERFERENCE_DETECT_CFM,
					&cls_wifi_hw->cal_env->interference_detect_cfm);
}

int cls_wifi_send_rssi_start_req(struct cls_wifi_hw *cls_wifi_hw,
							struct cal_rssi_start_req *rssi_start)
{
	struct cal_rssi_start_req *req;

	pr_info("%s: enable %hhu debug %hhu max_num %u\n", __func__, rssi_start->enable,
			rssi_start->debug, rssi_start->max_num);

	/* Build the CAL_RSSI_START_REQ message */
	req = cls_wifi_msg_zalloc(CAL_RSSI_START_REQ, TASK_CAL, DRV_TASK_ID,
		sizeof(struct cal_rssi_start_req));
	if (req == NULL)
		return -ENOMEM;

	/* Set parameters for the CAL_RSSI_START_REQ message */
	memcpy(req, rssi_start, sizeof(struct cal_rssi_start_req));
	/* Send the CAL_RSSI_START_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, CAL_RSSI_START_CFM,
					&cls_wifi_hw->cal_env->rssi_start_cfm);
}

int cls_wifi_send_rssi_stats_req(struct cls_wifi_hw *cls_wifi_hw,
							uint8_t rssi_mode, uint8_t legacy)
{
	struct cal_rssi_status_req *req;

	pr_info("%s: mode %hhu leg %hhu\n", __func__, rssi_mode, legacy);

	/* Build the CAL_RSSI_STATUS_REQ message */
	req = cls_wifi_msg_zalloc(CAL_RSSI_STATUS_REQ, TASK_CAL, DRV_TASK_ID,
		sizeof(struct cal_rssi_status_req));
	if (req == NULL)
		return -ENOMEM;

	/* Set parameters for the CAL_RSSI_STATUS_REQ message */
	req->rssi_mode = rssi_mode;
	req->legacy = legacy;
	/* Send the CAL_RSSI_STATUS_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, CAL_RSSI_STATUS_CFM,
					&cls_wifi_hw->cal_env->rssi_status_cfm);
}

int cls_wifi_send_get_temp_req(struct cls_wifi_hw *cls_wifi_hw)
{
	struct cal_get_temp_req *req = NULL;

	/* Build the IRF_GET_TEMP_REQ message */
	req = cls_wifi_msg_zalloc(CAL_GET_TEMP_REQ, TASK_CAL, DRV_TASK_ID,
		sizeof(struct cal_get_temp_req));
	if (req == NULL)
		return -ENOMEM;

	/* Set parameters for the CAL_GET_TEMP_REQ message */
	req->ts_id = current_radio;

	/* Send the CAL_GET_TEMP_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, CAL_GET_TEMP_CFM,
					&cls_wifi_hw->cal_env->get_temp_cfm);
}

int cls_wifi_send_cal_tx_stats_req(struct cls_wifi_hw *cls_wifi_hw, bool clear)
{
	struct cal_tx_stats_req *req;

	pr_info("%s\n", __func__);

	/* Build the CAL_TX_STATS_REQ message */
	req = cls_wifi_msg_zalloc(CAL_TX_STATS_REQ, TASK_CAL, DRV_TASK_ID,
		sizeof(struct cal_tx_stats_req));
	if (req == NULL)
		return -ENOMEM;

	/* Set parameters for the CAL_TX_STATS_REQ message */
	req->flags = 0;
	if (clear)
		req->flags |= CAL_TX_STATS_FLAG_RD_CLR;

	/* Send the CAL_TX_STATS_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, CAL_TX_STATS_CFM, &cls_wifi_hw->cal_env->tx_stats_cfm);
}

int cls_wifi_send_cal_tx_mu_req(struct cls_wifi_hw *cls_wifi_hw, struct cal_tx_mu_req *req_param)
{
	struct cal_tx_mu_req *req;

	/* Build the CAL_TX_MU_REQ message */
	req = cls_wifi_msg_zalloc(CAL_TX_MU_REQ, TASK_CAL, DRV_TASK_ID,
		sizeof(struct cal_tx_mu_req));
	if (req == NULL)
		return -ENOMEM;

	/* Set parameters for the CAL_TX_MU_REQ message */
	*req = *req_param;

	/* Send the CAL_TX_MU_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, CAL_TX_MU_CFM, &cls_wifi_hw->cal_env->tx_mu_cfm);
}

int cls_wifi_send_cal_sounding_req(struct cls_wifi_hw *cls_wifi_hw, struct cal_sounding_req *req_param)
{
	struct cal_sounding_req *src;
	struct cal_sounding_req *req;
	int i;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);
	if (!cls_wifi_hw || !cls_wifi_hw->cal_env)
		return -EINVAL;

	src = &cls_wifi_hw->cal_env->tx_sounding_req;
	req = cls_wifi_msg_zalloc(CAL_SOUNDING_REQ, TASK_CAL, DRV_TASK_ID,
		sizeof(struct cal_sounding_req));
	if (req == NULL)
		return -ENOMEM;

#ifndef CFG_CHIP_SHELL
	/* Acknowlege the CBF report Host address */
	for (i = 0; i < 4; i++)
		memset(src->host_report_addr[i], 0, 2048);
#endif

	memcpy(req, src, sizeof(struct cal_sounding_req));
	req->en_sample = req_param->en_sample;
	req->num_sample = req_param->num_sample;

	return cls_wifi_send_msg(cls_wifi_hw, req, 1, CAL_SOUNDING_CFM, &cls_wifi_hw->cal_env->tx_sounding_cfm);

	return 0;
}

int cls_wifi_send_cal_log_set_req(struct cls_wifi_hw *cls_wifi_hw, struct cal_log_set_req *set_req)
{
	struct cal_log_set_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	if (!cls_wifi_hw || !cls_wifi_hw->cal_env)
		return -EINVAL;

	/* Build the CAL_LOG_SET_REQ message */
	req = cls_wifi_msg_zalloc(CAL_LOG_SET_REQ, TASK_CAL, DRV_TASK_ID,
		sizeof(struct cal_log_set_req));
	if (req == NULL)
		return -ENOMEM;

	/* Set parameters for the CAL_LOG_SET_REQ message */
	memcpy(req, set_req, sizeof(*req));

	/* Send the CAL_LOG_SET_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, CAL_LOG_SET_CFM, &cls_wifi_hw->cal_env->log_set_cfm);
}

int cls_wifi_send_irf_hw_init_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_hw_init_req *hw_init,
								struct irf_hw_init_cfm *cfm)
{
	struct irf_hw_init_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_REG_INIT_REQ message */
	req = cls_wifi_msg_zalloc(IRF_REG_INIT_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_hw_init_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the IRF_REG_INIT_REQ message */
	memcpy(req, hw_init, sizeof(struct irf_hw_init_req));

	/* Send the IRF_REG_INIT_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, IRF_REG_INIT_CFM, cfm);
}

int cls_wifi_send_irf_smp_config_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_smp_config_req *smp_config,
								struct irf_smp_config_cfm *cfm)
{
	struct irf_smp_config_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_SAMPLE_CONFIG_REQ message */
	req = cls_wifi_msg_zalloc(IRF_SAMPLE_CONFIG_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_smp_config_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, smp_config, sizeof(struct irf_smp_config_req));

	/* Send the IRF_SAMPLE_CONFIG_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, IRF_SAMPLE_CONFIG_CFM, cfm);
}

int cls_wifi_send_irf_smp_start_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_smp_start_req *smp_start)
{
	struct irf_smp_start_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_SAMPLE_START_REQ message */
	req = cls_wifi_msg_zalloc(IRF_SAMPLE_START_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_smp_start_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, smp_start, sizeof(struct irf_smp_start_req));

	/* Send the IRF_SAMPLE_START_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_irf_send_config_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_send_config_req *send_config,
									struct irf_send_config_cfm *cfm)
{
	struct irf_send_config_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_SEND_CONFIG_REQ message */
	req = cls_wifi_msg_zalloc(IRF_SEND_CONFIG_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_send_config_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, send_config, sizeof(struct irf_send_config_req));

	/* Send the IRF_SEND_CONFIG_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, IRF_SEND_CONFIG_CFM, cfm);
}

int cls_wifi_send_irf_send_start_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_send_start_req *send_start,
									struct irf_send_start_cfm *cfm)
{
	struct irf_send_start_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_SEND_START_REQ message */
	req = cls_wifi_msg_zalloc(IRF_SEND_START_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_send_start_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, send_start, sizeof(struct irf_send_start_req));

	/* Send the IRF_SEND_START_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, IRF_SEND_START_CFM, cfm);
}

int cls_wifi_send_irf_send_stop_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_send_stop_req *send_start,
									struct irf_send_stop_cfm *cfm)
{
	struct irf_send_stop_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_SEND_STOP_REQ message */
	req = cls_wifi_msg_zalloc(IRF_SEND_STOP_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_send_stop_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, send_start, sizeof(struct irf_send_stop_req));

	/* Send the IRF_SEND_STOP_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, IRF_SEND_STOP_CFM, cfm);
}

int cls_wifi_send_xtal_ctrim_config_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_xtal_ctrim_config_req *xtal_ctrim_config,
									struct irf_xtal_ctrim_config_cfm *cfm)
{
	struct irf_xtal_ctrim_config_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_XTAL_CTRIM_CONFIG_REQ message */
	req = cls_wifi_msg_zalloc(IRF_XTAL_CTRIM_CONFIG_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_xtal_ctrim_config_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, xtal_ctrim_config, sizeof(struct irf_xtal_ctrim_config_req));

	/* Send the IRF_SEND_START_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, IRF_XTAL_CTRIM_CONFIG_CFM, cfm);
}

int cls_wifi_send_irf_cca_cs_config_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_cca_cs_config_req *cca_cs_config,
									struct irf_cca_cs_config_cfm *cfm)
{
	struct irf_cca_cs_config_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_CCA_CONFIG_REQ message */
	req = cls_wifi_msg_zalloc(IRF_CCA_CS_CONFIG_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_cca_cs_config_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, cca_cs_config, sizeof(struct irf_cca_cs_config_req));

	/* Send the IRF_SEND_START_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, IRF_CCA_CS_CONFIG_CFM, cfm);
}

int cls_wifi_send_irf_cca_ed_config_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_cca_ed_config_req *cca_ed_config,
									struct irf_cca_ed_config_cfm *cfm)
{
	struct irf_cca_ed_config_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_CCA_CONFIG_REQ message */
	req = cls_wifi_msg_zalloc(IRF_CCA_ED_CONFIG_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_cca_ed_config_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, cca_ed_config, sizeof(struct irf_cca_ed_config_req));

	/* Send the IRF_SEND_START_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, IRF_CCA_ED_CONFIG_CFM, cfm);
}

#ifdef __KERNEL__
int cls_wifi_send_irf_smp_lms_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_smp_lms_req *smp_lms)
{
	struct irf_smp_lms_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_SAMPLE_LMS_REQ message */
	req = cls_wifi_msg_zalloc(IRF_SAMPLE_LMS_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_smp_lms_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, smp_lms, sizeof(struct irf_smp_lms_req));

	/* Send the IRF_SAMPLE_LMS_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_irf_write_lms_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_write_lms_req *lms_config,
									struct irf_write_lms_cfm *cfm)
{
	struct irf_write_lms_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_WRITE_LMS_REQ message */
	req = cls_wifi_msg_zalloc(IRF_WRITE_LMS_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_write_lms_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, lms_config, sizeof(struct irf_write_lms_req));

	/* Send the IRF_WRITE_LMS_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, IRF_WRITE_LMS_CFM, cfm);
}
#endif

int cls_wifi_send_irf_hw_cfg_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_hw_cfg_req *hw_cfg_req)
{
	struct irf_hw_cfg_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_HW_CFG_REQ message */
	req = cls_wifi_msg_zalloc(IRF_HW_CFG_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_hw_cfg_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, hw_cfg_req, sizeof(struct irf_hw_cfg_req));

	/* Send the IRF_HW_CFG_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}


int cls_wifi_send_irf_set_mode_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_set_mode_req *set_mode_req)
{
	struct irf_set_mode_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_SET_MODE_REQ message */
	req = cls_wifi_msg_zalloc(IRF_SET_MODE_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_set_mode_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, set_mode_req, sizeof(struct irf_set_mode_req));

	/* Send the IRF_SET_MODE_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_irf_show_tbl_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_show_tbl_req *show_tbl_req)
{
	struct irf_show_tbl_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_SHOW_TBL_REQ message */
	req = cls_wifi_msg_zalloc(IRF_SHOW_TBL_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_show_tbl_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, show_tbl_req, sizeof(struct irf_show_tbl_req));

	/* Send the IRF_SHOW_TBL_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_irf_show_status_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_show_req *show_status_req)
{
	struct irf_show_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_SHOW_STATUS_REQ message */
	req = cls_wifi_msg_zalloc(IRF_SHOW_STATUS_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_show_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, show_status_req, sizeof(struct irf_show_req));

	/* Send the IRF_SHOW_STATUS_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}


int cls_wifi_send_irf_run_task_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_run_task_req *run_task_req)
{
	struct irf_run_task_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_RUN_TASK_REQ message */
	req = cls_wifi_msg_zalloc(IRF_RUN_TASK_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_run_task_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, run_task_req, sizeof(struct irf_run_task_req));

	/* Send the IRF_RUN_TASK_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_irf_dif_eq_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_dif_eq_req *dif_eq_req)
{
	struct irf_dif_eq_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_RUN_TASK_REQ message */
	req = cls_wifi_msg_zalloc(IRF_START_EQ_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_dif_eq_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, dif_eq_req, sizeof(struct irf_dif_eq_req));

	/* Send the IRF_START_EQ_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_irf_dif_eq_save_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_dif_eq_save_req *dif_eq_req)
{
	struct irf_dif_eq_save_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_DIF_CALI_SAVE_REQ message */
	req = cls_wifi_msg_zalloc(IRF_DIF_CALI_SAVE_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_dif_eq_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, dif_eq_req, sizeof(struct irf_dif_eq_save_req));

	/* Send the IRF_DIF_CALI_SAVE_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}


int cls_wifi_send_irf_set_calc_step_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_set_calc_step_req *calc_setp_req)
{
	struct irf_set_calc_step_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_SET_CALC_STEP_REQ message */
	req = cls_wifi_msg_zalloc(IRF_SET_CALC_STEP_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_set_calc_step_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, calc_setp_req, sizeof(struct irf_set_calc_step_req));

	/* Send the IRF_SET_CALC_STEP_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}


int cls_wifi_send_irf_calib_evt_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_set_cali_evt_req *calib_evt_req,
								struct irf_set_cali_evt_cfm *cfm)
{
	struct irf_set_cali_evt_req *req;

	//CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_SET_CALIB_EVT_REQ message */
	req = cls_wifi_msg_zalloc(IRF_SET_CALIB_EVT_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_set_cali_evt_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, calib_evt_req, sizeof(struct irf_set_cali_evt_req));

	/* Send the IRF_SET_CALIB_EVT_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, IRF_SET_CALIB_EVT_CFM, cfm);
}


int cls_wifi_send_irf_start_sched_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_start_schedule_req *start_sched_req)
{
	struct irf_start_schedule_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_START_SCHEDULE_REQ message */
	req = cls_wifi_msg_zalloc(IRF_START_SCHEDULE_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_start_schedule_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, start_sched_req, sizeof(struct irf_start_schedule_req));

	/* Send the IRF_START_SCHEDULE_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_irf_eq_data_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_send_eq_data_req *send_data_req)
{
	struct irf_send_eq_data_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_START_SCHEDULE_REQ message */
	req = cls_wifi_msg_zalloc(IRF_SEND_EQ_DATA_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_send_eq_data_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, send_data_req, sizeof(struct irf_send_eq_data_req));

	/* Send the IRF_SEND_EQ_DATA_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, IRF_SEND_EQ_DATA_CFM, NULL);
}

int cls_wifi_send_irf_agc_reload_req(struct cls_wifi_hw *cls_wifi_hw,
		uint8_t radio_idx, uint32_t len)
{
	struct irf_agc_reload_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_AGC_RELOAD_REQ message */
	req = cls_wifi_msg_zalloc(IRF_AGC_RELOAD_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_agc_reload_req));
	if (!req)
		return -ENOMEM;

	memset(req, 0, sizeof(*req));
	req->radio_id = radio_idx;
	req->agc_len = len;

	/* Send the IRF_AGC_RELOAD_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

#ifdef __KERNEL__
#ifdef CONFIG_CLS_VBSS
/* Set the VBSS related per STA sequence number info into FW */
int clsemi_set_sta_seq_num_req(struct cls_wifi_hw *cls_wifi_hw, struct vbss_set_sta_seq_num_req *seq_req)
{
	struct irf_send_eq_data_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_START_SCHEDULE_REQ message */
	req = cls_wifi_msg_zalloc(MM_SET_SEQ_REQ, TASK_MM, DRV_TASK_ID,
			sizeof(struct vbss_set_sta_seq_num_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, seq_req, sizeof(struct vbss_sta_seq_num));

	/* Send the MM_SET_SEQ_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}


/* Get the VBSS related per STA sequence number info from FW */
int clsemi_get_sta_seq_num_req(struct cls_wifi_hw *cls_wifi_hw, struct vbss_get_sta_seq_num_cfm *cfm)
{
	void *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Allocate the message */
	req = cls_wifi_msg_zalloc(MM_GET_SEQ_REQ, TASK_MM, DRV_TASK_ID,
			sizeof(struct vbss_get_sta_seq_num_req));
	if (!req)
		return -ENOMEM;

	/* Send the DBG_MEM_READ_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, MM_GET_SEQ_CFM, cfm);
}
#endif

/* allow un-encrypted, TA != BSSID packets to be forwarded to Host side */
int clsemi_send_rx_filter(struct cls_wifi_hw *cls_wifi_hw, uint8_t rx_filter)
{

	struct mm_set_filter_req *set_filter_req_param;

	set_filter_req_param = cls_wifi_msg_zalloc(MM_SET_FILTER_REQ, TASK_MM, DRV_TASK_ID,
			sizeof(struct mm_set_filter_req));
	if (!set_filter_req_param)
		return -ENOMEM;

	/* Add the filter flags that are set by default and cannot be changed here */
	rx_filter |= CLS_WIFI_MAC80211_NOT_CHANGEABLE;
	CLS_WIFI_DBG("rx filter set to  0x%08x\n", rx_filter);

	set_filter_req_param->filter = rx_filter;
	/* Send the MM_SET_FILTER_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, set_filter_req_param, 1, MM_SET_FILTER_CFM, NULL);
}
#endif

int cls_wifi_send_irf_dcoc_calc_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_fb_dcoc_req *fb_dcoc_req, lmac_msg_id_t dcoc_type)
{
	struct irf_fb_dcoc_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_FB_DCOC_REQ/IRF_RX_DCOC_REQ message */
	req = cls_wifi_msg_zalloc(dcoc_type, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_fb_dcoc_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, fb_dcoc_req, sizeof(struct irf_fb_dcoc_req));

	/* Send the IRF_FB_DCOC_REQ/IRF_RX_DCOC_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_irf_xtal_cal_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_xtal_cali_req *xtal_cali_req,
							   struct irf_xtal_cali_cfm *xtal_cali_cfm)
{
	struct irf_xtal_cali_req *req = NULL;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_XTAL_CALI_REQ message */
	req = cls_wifi_msg_zalloc(IRF_XTAL_CALI_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_xtal_cali_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, xtal_cali_req, sizeof(struct irf_xtal_cali_req));

	/* Send the IRF_XTAL_CALI_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, IRF_XTAL_CALI_CFM, xtal_cali_cfm);
}

int cls_wifi_send_irf_init_txcali_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_set_cali_param_req *init_txcali_req)
{
	struct irf_set_cali_param_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_INIT_TX_CALI_REQ message */
	req = cls_wifi_msg_zalloc(IRF_TX_CALI_INIT_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_set_cali_param_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, init_txcali_req, sizeof(struct irf_set_cali_param_req));

	/* Send the IRF_INIT_TX_CALI_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_irf_txcali_pwr_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_set_cali_param_req *txcali_pwr_req)
{
	struct irf_set_cali_param_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);
	/* Build the IRF_TX_CALI_OFFSET_REQ message */
	req = cls_wifi_msg_zalloc(IRF_TX_CALI_OFFSET_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_set_cali_param_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, txcali_pwr_req, sizeof(struct irf_set_cali_param_req));

	/* Send the IRF_TX_CALI_OFFSET_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_irf_fbcali_pwr_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_set_cali_param_req *fbcali_pwr_req)
{
	struct irf_set_cali_param_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);
	/* Build the IRF_FB_CALI_OFFSET_REQ message */
	req = cls_wifi_msg_zalloc(IRF_FB_CALI_OFFSET_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_set_cali_param_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, fbcali_pwr_req, sizeof(struct irf_set_cali_param_req));

	/* Send the IRF_FB_CALI_OFFSET_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}


int cls_wifi_send_irf_tx_pwr_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_set_tx_power_req *tx_power)
{
	struct irf_set_tx_power_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);
	/* Build the IRF_TX_POWER_REQ message */
	req = cls_wifi_msg_zalloc(IRF_TX_POWER_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_set_tx_power_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, tx_power, sizeof(struct irf_set_tx_power_req));

	/* Send the IRF_TX_POWER_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_set_irf_pppc_switch(struct cls_wifi_hw *cls_wifi_hw, struct irf_set_pppc_switch_req *pppc_req)
{
	struct irf_set_pppc_switch_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_PPPC_SWITCH_REQ message */
	req = cls_wifi_msg_zalloc(IRF_PPPC_SWITCH_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_set_pppc_switch_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, pppc_req, sizeof(struct irf_set_pppc_switch_req));

	/* Send the MM_GET_POWER_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_irf_tx_cali_save_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_set_cali_param_req *tx_cali_save_req)
{
	struct irf_set_cali_param_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_TX_CALI_SAVE_REQ message */
	req = cls_wifi_msg_zalloc(IRF_TX_CALI_SAVE_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_set_cali_param_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, tx_cali_save_req, sizeof(struct irf_set_cali_param_req));

	/* Send the IRF_TX_CALI_SAVE_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}


int cls_wifi_send_irf_init_rxcali_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_set_cali_param_req *init_rxcali_req)
{
	struct irf_set_cali_param_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_INIT_RX_CALI_REQ message */
	req = cls_wifi_msg_zalloc(IRF_RX_CALI_INIT_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_set_cali_param_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, init_rxcali_req, sizeof(struct irf_set_cali_param_req));

	/* Send the IRF_INIT_RX_CALI_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_irf_rxcali_rssi_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_set_cali_param_req *rxcali_pwr_req)
{
	struct irf_set_cali_param_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_RX_CALI_OFFSET_REQ message */
	req = cls_wifi_msg_zalloc(IRF_RX_CALI_OFFSET_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_set_cali_param_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, rxcali_pwr_req, sizeof(struct irf_set_cali_param_req));

	/* Send the IRF_RX_CALI_OFFSET_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_irf_rssi_cali_save_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_set_cali_param_req *rssi_cali_save_req)
{
	struct irf_set_cali_param_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_RX_CALI_SAVE_REQ message */
	req = cls_wifi_msg_zalloc(IRF_RX_CALI_SAVE_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_set_cali_param_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, rssi_cali_save_req, sizeof(struct irf_set_cali_param_req));

	/* Send the IRF_RX_CALI_SAVE_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_irf_set_rx_gain_lvl_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_set_rx_gain_lvl_req *rx_gain_lvl_req)
{
	struct irf_set_rx_gain_lvl_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_RX_LEVEL_SET_REQ message */
	req = cls_wifi_msg_zalloc(IRF_RX_LEVEL_SET_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_set_rx_gain_lvl_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, rx_gain_lvl_req, sizeof(struct irf_set_rx_gain_lvl_req));

	/* Send the IRF_RX_LEVEL_SET_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_cal_get_csi_req(struct cls_wifi_hw *cls_wifi_hw, uint16_t radio_id)
{
	struct cal_get_csi_req *req;
	int ret;

	pr_info("%s: radio_id %u\n", __func__, radio_id);

	/* Build the CAL_GET_CSI_REQ message */
	req = cls_wifi_msg_zalloc(CAL_GET_CSI_REQ, TASK_CAL, DRV_TASK_ID,
		sizeof(struct cal_rx_status_req));
	if (req == NULL)
		return -ENOMEM;

	/* Set parameters for the CAL_GET_CSI_REQ message */
	req->radio_id = radio_id;
	req->flags = 0;

	/* Send the CAL_GET_CSI_REQ message to LMAC FW */
	ret = cls_wifi_send_msg(cls_wifi_hw, req, 1, CAL_GET_CSI_CFM,
		&cls_wifi_hw->cal_env->get_csi_cfm);

	return ret;
}


int cls_wifi_send_irf_afe_cfg_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_afe_cfg_req *afe_cfg_req)
{
	struct irf_afe_cfg_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_AFE_CFG_REQ message */
	req = cls_wifi_msg_zalloc(IRF_AFE_CFG_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_afe_cfg_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, afe_cfg_req, sizeof(struct irf_afe_cfg_req));

	/* Send the IRF_AFE_CFG_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_irf_set_task_param_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_task_param_req *task_thres_req)
{
	struct irf_task_param_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_SET_TASK_PARAM_REQ message */
	req = cls_wifi_msg_zalloc(IRF_SET_TASK_PARAM_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_task_param_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, task_thres_req, sizeof(struct irf_task_param_req));

	/* Send the IRF_SET_TASK_PARAM_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_irf_show_capacity_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_show_capacity_req *capacity_req)
{
	struct irf_show_capacity_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_SHOW_CAPACITY_REQ message */
	req = cls_wifi_msg_zalloc(IRF_SHOW_CAPACITY_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_show_capacity_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, capacity_req, sizeof(struct irf_show_capacity_req));

	/* Send the IRF_SHOW_CAPACITY_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_irf_set_subband_idx_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_set_subband_idx_req *set_subband_req)
{
	struct irf_set_subband_idx_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_SET_CALI_BAND_IDX_REQ message */
	req = cls_wifi_msg_zalloc(IRF_SET_CALI_BAND_IDX_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_set_subband_idx_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, set_subband_req, sizeof(struct irf_set_subband_idx_req));

	/* Send the IRF_SET_CALI_BAND_IDX_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_irf_afe_cmd_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_afe_cmd_req *afe_cmd_req)
{
    struct irf_afe_cmd_req *req;

    CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

    /* Build the IRF_AFE_CMD_REQ message */
    req = cls_wifi_msg_zalloc(IRF_AFE_CMD_REQ, TASK_IRF, DRV_TASK_ID,
                          sizeof(struct irf_afe_cmd_req));
    if (!req)
        return -ENOMEM;

    memcpy(req, afe_cmd_req, sizeof(struct irf_afe_cmd_req));

    /* Send the IRF_AFE_CMD_REQ message to LMAC FW */
    return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_irf_radar_detect_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_radar_detect_req *radar_detect_req)
{
	struct irf_radar_detect_req *req = NULL;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_RADAR_DETECT_REQ message */
	req = cls_wifi_msg_zalloc(IRF_RADAR_DETECT_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_radar_detect_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, radar_detect_req, sizeof(struct irf_radar_detect_req));

	/* Send the IRF_RADAR_DETECT_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_irf_interference_detect_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_interference_detect_req *interference_detect_req,
							   struct irf_interference_detect_cfm *interference_detect_cfm)
{
	struct irf_interference_detect_req *req = NULL;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_interference_DETECT_REQ message */
	req = cls_wifi_msg_zalloc(IRF_INTERFERENCE_DETECT_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_interference_detect_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, interference_detect_req, sizeof(struct irf_interference_detect_req));

	/* Send the IRF_RADAR_DETECT_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, IRF_INTERFERENCE_DETECT_CFM, interference_detect_cfm);
}

int cls_wifi_send_irf_dcoc_soft_dbg_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_fb_dcoc_req *fb_dcoc_req, lmac_msg_id_t dcoc_type)
{
	struct irf_fb_dcoc_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_FB_DCOC_REQ/IRF_RX_DCOC_REQ message */
	req = cls_wifi_msg_zalloc(dcoc_type, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_fb_dcoc_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, fb_dcoc_req, sizeof(struct irf_fb_dcoc_req));

	/* Send the IRF_FB_DCOC_REQ/IRF_RX_DCOC_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_irf_fb_gain_err_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_set_cali_param_req *fb_err_req)
{
	struct irf_set_cali_param_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_FB_ERR_CALI_REQ message */
	req = cls_wifi_msg_zalloc(IRF_FB_ERR_CALI_REQ, TASK_IRF, DRV_TASK_ID,
			sizeof(struct irf_set_cali_param_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the IRF_FB_ERR_CALI_REQ message */
	memcpy(req, fb_err_req, sizeof(struct irf_set_cali_param_req));

	/* Send the IRF_FB_ERR_CALI_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, IRF_FB_ERR_CALI_CFM, NULL);
}

int cls_wifi_send_irf_tx_fcomp_cali_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_tx_fcomp_cali_req *tx_fcomp_cali_req)
{
	struct irf_tx_fcomp_cali_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_TX_FCOMP_CALI_REQ message */
	req = cls_wifi_msg_zalloc(IRF_TX_FCOMP_CALI_REQ, TASK_IRF, DRV_TASK_ID,
			sizeof(struct irf_tx_fcomp_cali_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the IRF_FB_ERR_CALI_REQ message */
	memcpy(req, tx_fcomp_cali_req, sizeof(struct irf_tx_fcomp_cali_req));

	/* Send the IRF_TX_FCOMP_CALI_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, IRF_TX_FCOMP_CALI_REQ, NULL);
}

int cls_wifi_send_irf_tx_act_pwr_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_tx_cur_pwr_req *tx_act_pwr_req)
{
	struct irf_tx_cur_pwr_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_TX_CUR_PWR_REQ message */
	req = cls_wifi_msg_zalloc(IRF_TX_CUR_PWR_REQ, TASK_IRF, DRV_TASK_ID,
			sizeof(struct irf_tx_cur_pwr_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the IRF_TX_CUR_PWR_REQ message */
	memcpy(req, tx_act_pwr_req, sizeof(struct irf_tx_cur_pwr_req));

	/* Send the IRF_TX_CUR_PWR_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, IRF_TX_CUR_PWR_REQ, NULL);
}
int cls_wifi_send_irf_tx_loop_power_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_tx_cur_pwr_req *tx_loop_pwr_req)
{
	struct irf_tx_cur_pwr_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_TX_LOOP_PWR_REQ message */
	req = cls_wifi_msg_zalloc(IRF_TX_LOOP_PWR_REQ, TASK_IRF, DRV_TASK_ID,
			sizeof(struct irf_tx_cur_pwr_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the IRF_TX_LOOP_PWR_REQ message */
	memcpy(req, tx_loop_pwr_req, sizeof(struct irf_tx_cur_pwr_req));

	/* Send the IRF_TX_LOOP_PWR_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, IRF_TX_LOOP_PWR_REQ, NULL);
}

int cls_wifi_send_irf_tx_gain_err_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_set_cali_param_req *tx_err_req)
{
	struct irf_set_cali_param_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_TX_ERR_CALI_REQ message */
	req = cls_wifi_msg_zalloc(IRF_TX_ERR_CALI_REQ, TASK_IRF, DRV_TASK_ID,
			sizeof(struct irf_set_cali_param_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the IRF_TX_ERR_CALI_REQ message */
	memcpy(req, tx_err_req, sizeof(struct irf_set_cali_param_req));

	/* Send the IRF_TX_ERR_CALI_CFM message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, IRF_TX_ERR_CALI_CFM, NULL);
}

int cls_wifi_send_irf_tx_loop_pwr_init_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_set_cali_param_req *loop_init_req)
{
	struct irf_set_cali_param_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_TX_LOOP_PWR_INIT_REQ message */
	req = cls_wifi_msg_zalloc(IRF_TX_LOOP_PWR_INIT_REQ, TASK_IRF, DRV_TASK_ID,
			sizeof(struct irf_set_cali_param_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the IRF_TX_LOOP_PWR_INIT_REQ message */
	memcpy(req, loop_init_req, sizeof(struct irf_set_cali_param_req));

	/* Send the IRF_TX_LOOP_PWR_INIT_CFM message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_check_alarm_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_check_alm_req *check_alm_req)
{
	struct irf_check_alm_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_CHECK_ALM_REQ message */
	req = cls_wifi_msg_zalloc(IRF_CHECK_ALM_REQ, TASK_IRF, DRV_TASK_ID,
				   sizeof(struct irf_check_alm_req));
	if (!req)
		   return -ENOMEM;

	/* Set parameters for the IRF_CHECK_ALM_REQ message */
	memcpy(req, check_alm_req, sizeof(struct irf_check_alm_req));

	/* Send the IRF_CHECK_ALM_CFM message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, IRF_CHECK_ALM_CFM, NULL);
}

int cls_wifi_send_th_wall_req(struct cls_wifi_hw *cls_wifi_hw,
			struct irf_th_wall_req *th_wall_req, struct irf_th_wall_cfm *cfm)
{
	struct irf_th_wall_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_TH_WALL_REQ message */
	req = cls_wifi_msg_zalloc(IRF_TH_WALL_REQ, TASK_IRF, DRV_TASK_ID,
				   sizeof(struct irf_th_wall_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the IRF_TH_WALL_REQ message */
	memcpy(req, th_wall_req, sizeof(struct irf_th_wall_req));

	/* Send the IRF_TH_WALL_CFM message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, IRF_TH_WALL_CFM, cfm);
}

int cls_wifi_send_csi_cmd_req(struct cls_wifi_hw *cls_wifi_hw, struct mm_csi_params_req *csi)
{
	struct mm_csi_params_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);
	req = cls_wifi_msg_zalloc(MM_CSI_CMD_REQ, TASK_MM, DRV_TASK_ID,
			sizeof(struct mm_csi_params_req));
	if (!req)
		return -ENOMEM;
	memcpy(req, csi, sizeof(struct mm_csi_params_req));

	/* Send the MM_DL_PARAMETERS_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, MM_CSI_CMD_CFM, NULL);
}

int cls_wifi_send_irf_rx_gain_lvl_cali_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_rx_gain_lvl_cali_req *rx_gain_lvl_cali_req)
{
	struct irf_rx_gain_lvl_cali_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_RX_GAIN_LVL_CALI_REQ message */
	req = cls_wifi_msg_zalloc(IRF_RX_GAIN_LVL_CALI_REQ, TASK_IRF, DRV_TASK_ID,
			sizeof(struct irf_rx_gain_lvl_cali_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the IRF_RX_GAIN_LVL_CALI_REQ message */
	memcpy(req, rx_gain_lvl_cali_req, sizeof(struct irf_rx_gain_lvl_cali_req));

	/* Send the IRF_RX_GAIN_LVL_CALI_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_irf_rx_gain_freq_cali_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_rx_gain_freq_cali_req *rx_gain_freq_cali_req)
{
	struct irf_rx_gain_freq_cali_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_RX_GAIN_FREQ_CALI_REQ message */
	req = cls_wifi_msg_zalloc(IRF_RX_GAIN_FREQ_CALI_REQ, TASK_IRF, DRV_TASK_ID,
			sizeof(struct irf_rx_gain_freq_cali_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the IRF_RX_GAIN_FREQ_CALI_REQ message */
	memcpy(req, rx_gain_freq_cali_req, sizeof(struct irf_rx_gain_freq_cali_req));

	/* Send the IRF_RX_GAIN_FREQ_CALI_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_irf_save_rx_gain_freq_ofst_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_set_cali_param_req *save_rx_gain_freq_ofst_req)
{
	struct irf_set_cali_param_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_RX_CALI_SAVE_REQ message */
	req = cls_wifi_msg_zalloc(IRF_RX_CALI_SAVE_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_set_cali_param_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, save_rx_gain_freq_ofst_req, sizeof(struct irf_set_cali_param_req));

	/* Send the IRF_RX_CALI_SAVE_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_irf_gain_dbg_lvl_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_gain_dbg_lvl_req *dbg_lvl_req)
{
	struct irf_gain_dbg_lvl_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_CALI_DBG_LVL_REQ message */
	req = cls_wifi_msg_zalloc(IRF_CALI_DBG_LVL_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_gain_dbg_lvl_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, dbg_lvl_req, sizeof(struct irf_gain_dbg_lvl_req));

	/* Send the IRF_CALI_DBG_LVL_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);

}

int cls_wifi_send_atf_update_req(struct cls_wifi_hw *cls_wifi_hw, struct mm_atf_params_req *atf)
{
	struct mm_atf_params_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the MM_DL_PARAMETERS_REQ message */
	req = cls_wifi_msg_zalloc(MM_SET_ATF_REQ, TASK_MM, DRV_TASK_ID,
			sizeof(struct mm_atf_params_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, atf, sizeof(struct mm_atf_params_req));

	/* Send the MM_DL_PARAMETERS_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, MM_SET_ATF_CFM, NULL);
}

int cls_wifi_send_mem_cmd_req(struct cls_wifi_hw *cls_wifi_hw, struct mm_mem_params_req *op,
		struct mm_mem_params_cfm *cfm)
{
	struct mm_mem_params_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the MM_MEM_CMD_REQ message */
	req = cls_wifi_msg_zalloc(MM_MEM_CMD_REQ, TASK_MM, DRV_TASK_ID,
			sizeof(struct mm_mem_params_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the MM_DL_PARAMETERS_REQ message */
	memcpy(req, op, sizeof(struct mm_mem_params_req));

	/* Send the MM_DL_PARAMETERS_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 1, MM_MEM_CMD_CFM, cfm);
}

int cls_wifi_send_dpd_wmac_tx_cmd_req(struct cls_wifi_hw *cls_wifi_hw,
		struct mm_dpd_wmac_tx_params_req *tx_req)
{
	struct mm_dpd_wmac_tx_params_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the MM_DPD_WMAC_TX_CMD_REQ message */
	req = cls_wifi_msg_zalloc(MM_DPD_WMAC_TX_CMD_REQ, TASK_MM, DRV_TASK_ID,
			sizeof(struct mm_dpd_wmac_tx_params_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the MM_DL_PARAMETERS_REQ message */
	memcpy(req, tx_req, sizeof(struct mm_dpd_wmac_tx_params_req));

	/* Send the MM_DL_PARAMETERS_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, MM_DPD_WMAC_TX_CMD_IND, NULL);
}

#ifdef __KERNEL__
#define MEM_OP_CRC
//#define MEM_OP_DEBUG

#ifdef MEM_OP_CRC
#define CLS_WIFI_CRC32C_INIT		0
#define CLS_WIFI_CRC_TABLE_SIZE		256
#define CLS_WIFI_CRC_POLY_REV		0xEDB88320

static uint32_t crc32c_table[CLS_WIFI_CRC_TABLE_SIZE];

static void cls_wifi_crc32_table_gen(uint32_t crc_table[], uint32_t poly)
{
	uint32_t x;
	size_t i;
	size_t j;

	for (i = 0; i < CLS_WIFI_CRC_TABLE_SIZE; i++) {
		x = i;

		for (j = 0; j < 8; j++)
			x = (x >> 1) ^ (poly & (-(int32_t)(x & 1)));

		crc_table[i] = x;
	}
}

uint32_t cls_wifi_crc32c(uint32_t crc, const uint8_t *buf, size_t len)
{
	const uint32_t crc32c_poly_rev = CLS_WIFI_CRC_POLY_REV;

	if (unlikely(!crc32c_table[1]))
		cls_wifi_crc32_table_gen(crc32c_table, crc32c_poly_rev);

	while (len--)
		crc = (crc << 8) ^ crc32c_table[(crc >> 24) ^ *buf++];

	return crc;
}
#endif

int cls_wifi_mem_ops(struct cls_wifi_hw *cls_wifi_hw, uint8_t op, uint8_t region, u32 offset,
		void *addr, u32 len)
{
	struct mm_mem_params_req req;
	struct mm_mem_params_cfm cfm;
	uint32_t offset_cur;
	uint32_t len_left;
	uint32_t len_max;
	int ret = CO_OK;
#ifdef MEM_OP_CRC
	uint32_t crc;
#endif

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

#ifdef MEM_OP_DEBUG
	pr_warn("%s %d radio %d op %d region %d offset 0x%x len 0x%x\n", __func__, __LINE__,
			cls_wifi_hw->radio_idx, op, region, offset, len);
	pr_warn("%s %d reqmax %ld cfmmax %d\n", __func__, __LINE__,
			MM_MEM_REQ_LEN_MAX, MM_MEM_CFM_LEN_MAX);
#endif
#ifdef MEM_OP_CRC
	if (op == MM_MEM_OP_WRITE) {
		crc = cls_wifi_crc32c(CLS_WIFI_CRC32C_INIT, (uint8_t *)addr, len);
		pr_warn("%s %d write crc 0x%x\n", __func__, __LINE__, crc);
	}
#endif
	req.radio_id = cls_wifi_hw->radio_idx;
	req.op = op;
	req.region = region;
	offset_cur = 0;
	len_left = len;

	switch (op) {
	case MM_MEM_OP_WRITE:
		len_max = MM_MEM_REQ_LEN_MAX * sizeof(uint32_t);
		break;
	case MM_MEM_OP_SET:
		len_max = MM_MEM_SET_LEN_MAX;
		break;
	case MM_MEM_OP_READ:
		len_max = MM_MEM_CFM_LEN_MAX * sizeof(uint32_t);
		break;
	default:
		pr_warn("%s %d invalid op %d\n", __func__, __LINE__, op);
		return -1;
	}

	while (len_left) {
		req.offset = offset + offset_cur;
		if (len_left > len_max) {
			req.len = len_max;
			req.isend = 0;
		} else {
			req.len = len_left;
			req.isend = 1;
		}
		if (op == MM_MEM_OP_WRITE)
			memcpy(req.payload, addr + offset_cur, req.len);
		else if (op == MM_MEM_OP_SET)
			req.payload[0] = *(uint32_t *)addr;
#ifdef MEM_OP_DEBUG
		pr_warn("%s %d offset_cur 0x%x len_left 0x%x isend %d\n", __func__, __LINE__,
				offset_cur, len_left, req.isend);
		pr_warn("%s %d radio %d op %d region %d offset 0x%x len 0x%x\n", __func__, __LINE__,
				req.radio_id, req.op, req.region, req.offset, req.len);
#endif
		ret = cls_wifi_send_mem_cmd_req(cls_wifi_hw, &req, &cfm);
		if (ret != CO_OK) {
			pr_warn("%s %d ret %d\n", __func__, __LINE__, ret);
			return ret;
		}
		if (op == MM_MEM_OP_READ)
			memcpy(addr + offset_cur, cfm.payload, req.len);
		offset_cur += req.len;
		len_left -= req.len;
	}

#ifdef MEM_OP_CRC
	if (op == MM_MEM_OP_READ) {
		crc = cls_wifi_crc32c(CLS_WIFI_CRC32C_INIT, (uint8_t *)addr, len);
		pr_warn("%s %d read crc 0x%x\n", __func__, __LINE__, crc);
	}
#endif
	return ret;
}

int cls_wifi_send_bf_parameters_req(struct cls_wifi_hw *cls_wifi_hw,
	struct bf_parameters_req *cbf_req)
{
	struct bf_parameters_req *req;

	/* Build the BF_PARAMETERS_REQ message */
	req = cls_wifi_msg_zalloc(MM_BF_PARAMETERS_REQ, TASK_MM, DRV_TASK_ID,
			sizeof(struct bf_parameters_req));
	if (!req)
		return -ENOMEM;

	/* Set parameters for the BF_PARAMETERS_REQ message */
	memcpy(req, cbf_req, sizeof(struct bf_parameters_req));

	/* Send the BF_PARAMETERS_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, MM_BF_PARAMETERS_CFM, NULL);
}

int cls_wifi_send_force_soft_reset_hw_req(struct cls_wifi_hw *cls_wifi_hw,
	struct dht_force_reset_hw_req_op *reset_req)
{
	struct dht_force_reset_hw_req_op *req;

	/* Build the DHT_D2K_SOFT_RESET_REQ message */
	req = cls_wifi_msg_zalloc(DHT_D2K_SOFT_RESET_REQ, TASK_DHT, DRV_TASK_ID,
			sizeof(struct dht_force_reset_hw_req_op));
	if (!req)
		return -ENOMEM;

	memcpy(req, reset_req, sizeof(struct dht_force_reset_hw_req_op));
	/* Send the DHT_D2K_SOFT_RESET_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_irf_set_aci_det_para_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_aci_det_para_req *aci_det_para_req)
{
	struct irf_aci_det_para_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_SET_ACI_DET_PARA_REQ message */
	req = cls_wifi_msg_zalloc(IRF_SET_ACI_DET_PARA_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_aci_det_para_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, aci_det_para_req, sizeof(struct irf_aci_det_para_req));

	/* Send the IRF_SET_ACI_DET_PARA_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_irf_get_aci_det_para_req(struct cls_wifi_hw *cls_wifi_hw, struct irf_aci_det_para_req *aci_det_para_req)
{
	struct irf_aci_det_para_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_GET_ACI_DET_PARA_REQ message */
	req = cls_wifi_msg_zalloc(IRF_GET_ACI_DET_PARA_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_aci_det_para_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, aci_det_para_req, sizeof(struct irf_aci_det_para_req));

	/* Send the IRF_GET_ACI_DET_PARA_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_irf_set_rx_gain_cali_para_req(struct cls_wifi_hw *cls_wifi_hw,
		struct irf_rx_gain_cali_para_req *cali_para_req)
{
	struct irf_rx_gain_cali_para_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_SET_RX_GAIN_CALI_PARA_REQ message */
	req = cls_wifi_msg_zalloc(IRF_SET_RX_GAIN_CALI_PARA_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_rx_gain_cali_para_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, cali_para_req, sizeof(struct irf_rx_gain_cali_para_req));

	/* Send the IRF_SET_RX_GAIN_CALI_PARA_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_irf_get_rx_gain_cali_para_req(struct cls_wifi_hw *cls_wifi_hw,
		struct irf_rx_gain_cali_para_req *cali_para_req)
{
	struct irf_rx_gain_cali_para_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_GET_RX_GAIN_CALI_PARA_REQ message */
	req = cls_wifi_msg_zalloc(IRF_GET_RX_GAIN_CALI_PARA_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_rx_gain_cali_para_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, cali_para_req, sizeof(struct irf_rx_gain_cali_para_req));

	/* Send the IRF_GET_RX_GAIN_CALI_PARA_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_irf_rx_gain_cali_prep_req(struct cls_wifi_hw *cls_wifi_hw,
		struct irf_rx_gain_cali_prep_req *cali_prep_req)
{
	struct irf_rx_gain_cali_prep_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_RX_GAIN_CALI_PREP_REQ message */
	req = cls_wifi_msg_zalloc(IRF_RX_GAIN_CALI_PREP_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_rx_gain_cali_prep_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, cali_prep_req, sizeof(struct irf_rx_gain_cali_prep_req));

	/* Send the IRF_RX_GAIN_CALI_PREP_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_irf_rx_gain_lvl_bist_cali_req(struct cls_wifi_hw *cls_wifi_hw,
		struct irf_rx_gain_lvl_bist_cali_req *bist_cali_req)
{
	struct irf_rx_gain_lvl_bist_cali_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_RX_GAIN_BIST_CALI_REQ message */
	req = cls_wifi_msg_zalloc(IRF_RX_GAIN_BIST_CALI_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_rx_gain_lvl_bist_cali_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, bist_cali_req, sizeof(struct irf_rx_gain_lvl_bist_cali_req));

	/* Send the IRF_RX_GAIN_BIST_CALI_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_irf_rx_gain_imd3_test_req(struct cls_wifi_hw *cls_wifi_hw,
		struct irf_rx_gain_imd3_test_req *imd3_test_req)
{
	struct irf_rx_gain_imd3_test_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_RX_GAIN_IMD3_TEST_REQ message */
	req = cls_wifi_msg_zalloc(IRF_RX_GAIN_IMD3_TEST_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_rx_gain_imd3_test_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, imd3_test_req, sizeof(struct irf_rx_gain_imd3_test_req));

	/* Send the IRF_RX_GAIN_IMD3_TEST_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_set_irf_pwr_ctrl_thre_req(struct cls_wifi_hw *cls_wifi_hw,
		struct irf_pwr_ctrl_thre_req *data)
{
	struct irf_pwr_ctrl_thre_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_SET_PWR_CTRL_THRE_REQ message */
	req = cls_wifi_msg_zalloc(IRF_SET_PWR_CTRL_THRE_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_pwr_ctrl_thre_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, data, sizeof(struct irf_pwr_ctrl_thre_req));

	/* Send the IRF_SET_PWR_CTRL_THRE_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_get_irf_pwr_ctrl_thre_req(struct cls_wifi_hw *cls_wifi_hw,
		struct irf_pwr_ctrl_thre_req *data)
{
	struct irf_pwr_ctrl_thre_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_GET_PWR_CTRL_THRE_REQ message */
	req = cls_wifi_msg_zalloc(IRF_GET_PWR_CTRL_THRE_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_pwr_ctrl_thre_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, data, sizeof(struct irf_pwr_ctrl_thre_req));

	/* Send the IRF_GET_PWR_CTRL_THRE_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_set_irf_pwr_prec_offset_req(struct cls_wifi_hw *cls_wifi_hw,
		struct irf_pwr_prec_offset_req *data)
{
	struct irf_pwr_prec_offset_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_SET_PWR_PREC_OFFSET_REQ message */
	req = cls_wifi_msg_zalloc(IRF_SET_PWR_PREC_OFFSET_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_pwr_prec_offset_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, data, sizeof(struct irf_pwr_prec_offset_req));

	/* Send the IRF_SET_PWR_PREC_OFFSET_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_get_irf_pwr_prec_offset_req(struct cls_wifi_hw *cls_wifi_hw,
		struct irf_pwr_prec_offset_req *data)
{
	struct irf_pwr_prec_offset_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_GET_PWR_PREC_OFFSET_REQ message */
	req = cls_wifi_msg_zalloc(IRF_GET_PWR_PREC_OFFSET_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_pwr_prec_offset_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, data, sizeof(struct irf_pwr_prec_offset_req));

	/* Send the IRF_GET_PWR_PREC_OFFSET_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_set_irf_comp_stub_bitmap_req(struct cls_wifi_hw *cls_wifi_hw,
		struct irf_comp_stub_bitmap_req *data)
{
	struct irf_comp_stub_bitmap_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_SET_COMP_STUB_BITMAP_REQ message */
	req = cls_wifi_msg_zalloc(IRF_SET_COMP_STUB_BITMAP_REQ, TASK_IRF, DRV_TASK_ID,
						sizeof(struct irf_comp_stub_bitmap_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, data, sizeof(struct irf_comp_stub_bitmap_req));

	/* Send the IRF_SET_COMP_STUB_BITMAP_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_get_irf_comp_stub_bitmap_req(struct cls_wifi_hw *cls_wifi_hw,
		struct irf_comp_stub_bitmap_req *data)
{
	struct irf_comp_stub_bitmap_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_GET_COMP_STUB_BITMAP_REQ message */
	req = cls_wifi_msg_zalloc(IRF_GET_COMP_STUB_BITMAP_REQ, TASK_IRF, DRV_TASK_ID,
						sizeof(struct irf_comp_stub_bitmap_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, data, sizeof(struct irf_comp_stub_bitmap_req));

	/* Send the IRF_GET_COMP_STUB_BITMAP_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

int cls_wifi_send_irf_set_digital_tx_gain_req(struct cls_wifi_hw *cls_wifi_hw,
		struct irf_set_digital_tx_gain_req *imd3_test_req)
{
	struct irf_set_digital_tx_gain_req *req;

	CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

	/* Build the IRF_SET_DIGITAL_TX_GAIN_REQ message */
	req = cls_wifi_msg_zalloc(IRF_SET_DIGITAL_TX_GAIN_REQ, TASK_IRF, DRV_TASK_ID,
						  sizeof(struct irf_set_digital_tx_gain_req));
	if (!req)
		return -ENOMEM;

	memcpy(req, imd3_test_req, sizeof(struct irf_set_digital_tx_gain_req));

	/* Send the IRF_SET_DIGITAL_TX_GAIN_REQ message to LMAC FW */
	return cls_wifi_send_msg(cls_wifi_hw, req, 0, 0, NULL);
}

#endif
