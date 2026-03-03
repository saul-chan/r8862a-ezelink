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

/*
 * INCLUDE FILES
 ******************************************************************************
 */
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include "cls_wifi_defs.h"
#include "cls_wifi_prof.h"
#include "ipc_host.h"
#include "cls_wifi_msgq.h"
#include "cls_wifi_main.h"
#include "cls_wifi_core.h"

/*
 * TYPES DEFINITION
 ******************************************************************************
 */
extern uint32_t use_msgq;

uint32_t ipc_host_get_txdesc_nb(struct cls_wifi_hw *cls_wifi_hw)
{
	return (cls_wifi_hw->plat->hw_params.txdesc_cnt0[cls_wifi_hw->radio_idx] +
			cls_wifi_hw->plat->hw_params.txdesc_cnt1[cls_wifi_hw->radio_idx] +
			cls_wifi_hw->plat->hw_params.txdesc_cnt2[cls_wifi_hw->radio_idx] +
			cls_wifi_hw->plat->hw_params.txdesc_cnt3[cls_wifi_hw->radio_idx] +
			cls_wifi_hw->plat->hw_params.txdesc_cnt4[cls_wifi_hw->radio_idx] +
			cls_wifi_hw->plat->hw_params.txdesc_mu_cnt[cls_wifi_hw->radio_idx] + 1);
}

uint32_t ipc_host_get_txcfm_nb(struct cls_wifi_hw *cls_wifi_hw)
{
	return (cls_wifi_hw->plat->hw_params.txdesc_cnt0[cls_wifi_hw->radio_idx] +
			cls_wifi_hw->plat->hw_params.txdesc_cnt1[cls_wifi_hw->radio_idx] +
			cls_wifi_hw->plat->hw_params.txdesc_cnt2[cls_wifi_hw->radio_idx] +
			cls_wifi_hw->plat->hw_params.txdesc_cnt3[cls_wifi_hw->radio_idx] +
			cls_wifi_hw->plat->hw_params.txdesc_cnt4[cls_wifi_hw->radio_idx] +
			cls_wifi_hw->plat->hw_params.txdesc_mu_cnt[cls_wifi_hw->radio_idx]);
}

uint32_t ipc_host_get_rxdesc_nb(struct cls_wifi_hw *cls_wifi_hw)
{
	return IPC_RXDESC_CNT_NR(cls_wifi_hw->plat->hw_params.mu_user[cls_wifi_hw->radio_idx]);
}

uint32_t ipc_host_get_rxbuf_nb(struct cls_wifi_hw *cls_wifi_hw)
{
	return IPC_RXBUF_CNT_NR(cls_wifi_hw->plat->hw_params.mu_user[cls_wifi_hw->radio_idx]);
}

struct ipc_hostid *ipc_host_get_tx_hostid_from_env(struct ipc_host_env_tag *env)
{
	return (struct ipc_hostid *)((void *)env + sizeof(struct ipc_host_env_tag));
}

struct cls_wifi_ipc_buf **ipc_host_get_txcfm_from_env(struct ipc_host_env_tag *env)
{
	return (struct cls_wifi_ipc_buf **)((void *)env + sizeof(struct ipc_host_env_tag) +
			env->txcfm_nb * sizeof(struct ipc_hostid));
}

struct cls_wifi_ipc_buf **ipc_host_get_rxdesc_from_env(struct ipc_host_env_tag *env)
{
	return (struct cls_wifi_ipc_buf **)((void *)env + sizeof(struct ipc_host_env_tag) +
			env->txcfm_nb * sizeof(struct ipc_hostid) +
			env->txcfm_nb * sizeof(struct cls_wifi_ipc_buf *));
}

/*
 * FUNCTIONS DEFINITIONS
 ******************************************************************************
 */
/**
 * ipc_host_rxdesc_handler() - Handle the reception of a Rx Descriptor
 *
 * @env: pointer to the IPC Host environment
 *
 * Called from general IRQ handler when status %IPC_IRQ_E2A_RXDESC is set
 */
void ipc_host_rxdesc_handler(struct cls_wifi_hw *cls_wifi_hw)
{
	struct ipc_host_env_tag *env = cls_wifi_hw->ipc_env;
	struct cls_wifi_ipc_buf **rxdesc = ipc_host_get_rxdesc_from_env(env);
	unsigned long exit_time = jiffies + msecs_to_jiffies(cls_wifi_mod_params.irq_force_exit_time);

	// For profiling
	REG_SW_SET_PROFILING(cls_wifi_hw, SW_PROF_IRQ_E2A_RXDESC);

	// LMAC has triggered an IT saying that a reception has occurred.
	// Then we first need to check the validity of the current hostbuf, and the validity
	// of the next hostbufs too, because it is likely that several hostbufs have been
	// filled within the time needed for this irq handling
	while (1) {
		// call the external function to indicate that a RX descriptor is received
		if (env->cb.recv_data_ind(cls_wifi_hw, rxdesc[env->rxdesc_idx], 0) != 0)
			break;

		if (time_after(jiffies, exit_time))
			break;
	}

	// For profiling
	REG_SW_CLEAR_PROFILING(cls_wifi_hw, SW_PROF_IRQ_E2A_RXDESC);
}
EXPORT_SYMBOL(ipc_host_rxdesc_handler);

void ipc_host_cmn_rxdesc_handler(struct cls_wifi_hw *cls_wifi_hw)
{
	struct ipc_host_cmn_env_tag *env = cls_wifi_hw->ipc_cmn_env;
	struct cls_wifi_ipc_buf **rxdesc;
	uint16_t *rxdesc_idx;

	if (cls_wifi_hw->radio_idx == RADIO_2P4G_INDEX) {
		rxdesc = &env->rxdesc_2g[0];
		rxdesc_idx = &env->rxdesc_idx_2g;
	} else {
		rxdesc = &env->rxdesc_5g[0];
		rxdesc_idx = &env->rxdesc_idx_5g;
	}

	// LMAC has triggered an IT saying that a reception has occurred.
	// Then we first need to check the validity of the current hostbuf, and the validity
	// of the next hostbufs too, because it is likely that several hostbufs have been
	// filled within the time needed for this irq handling
	while (1) {
		// call the external function to indicate that a RX descriptor is received
		if (env->cb.recv_data_ind(cls_wifi_hw, rxdesc[*rxdesc_idx], 1) != 0)
			break;
	}
}
EXPORT_SYMBOL(ipc_host_cmn_rxdesc_handler);

/**
 * ipc_host_radar_handler() - Handle the reception of radar events
 *
 * @env: pointer to the IPC Host environment
 *
 * Called from general IRQ handler when status %IPC_IRQ_E2A_RADAR is set
 */
void ipc_host_radar_handler(struct cls_wifi_hw *cls_wifi_hw)
{
#ifdef CONFIG_CLS_WIFI_RADAR
	struct ipc_host_env_tag *env = cls_wifi_hw->ipc_env;

	spin_lock(&cls_wifi_hw->radar.lock);
	env->cb.recv_radar_ind(cls_wifi_hw, env->radar[env->radar_idx]);
	spin_unlock(&cls_wifi_hw->radar.lock);
#endif /* CONFIG_CLS_WIFI_RADAR */
}
EXPORT_SYMBOL(ipc_host_radar_handler);

/**
 * ipc_host_he_mu_map_handler() - Handle the reception of HE MU DL Map events
 *
 * @env: pointer to the IPC Host environment
 *
 * Called from general IRQ handler when status %IPC_IRQ_E2A_HE_MU_DESC is set
 */
void ipc_host_he_mu_map_handler(struct cls_wifi_hw *cls_wifi_hw)
{
	struct ipc_host_env_tag *env = cls_wifi_hw->ipc_env;

	env->cb.recv_he_mu_map_ind(cls_wifi_hw, env->hemumap[env->hemumap_idx]);
}
EXPORT_SYMBOL(ipc_host_he_mu_map_handler);

/**
 * ipc_host_unsup_rx_vec_handler() - Handle the reception of unsupported rx vector
 *
 * @env: pointer to the IPC Host environment
 *
 * Called from general IRQ handler when status %IPC_IRQ_E2A_UNSUP_RX_VEC is set
 */
void ipc_host_unsup_rx_vec_handler(struct cls_wifi_hw *cls_wifi_hw)
{
	struct ipc_host_env_tag *env = cls_wifi_hw->ipc_env;

	while (env->cb.recv_unsup_rx_vec_ind(cls_wifi_hw, env->unsuprxvec[env->unsuprxvec_idx]) == 0)
		;
}
EXPORT_SYMBOL(ipc_host_unsup_rx_vec_handler);

/**
 * ipc_host_msg_handler() - Handler for firmware message
 *
 * @env: pointer to the IPC Host environment
 *
 * Called from general IRQ handler when status %IPC_IRQ_E2A_MSG is set
 */
void ipc_host_msg_handler(struct cls_wifi_hw *cls_wifi_hw)
{
	struct ipc_host_env_tag *env;

	if (!cls_wifi_hw)
		return;

	env = cls_wifi_hw->ipc_env;

	while (env->cb.recv_msg_ind(cls_wifi_hw, env->msgbuf[env->msgbuf_idx]) == 0)
		;
}
EXPORT_SYMBOL(ipc_host_msg_handler);

/**
 * ipc_host_msgack_handler() - Handle the reception of message acknowledgement
 *
 * @env: pointer to the IPC Host environment
 *
 * Called from general IRQ handler when status %IPC_IRQ_E2A_MSG_ACK is set
 */
void ipc_host_msgack_handler(struct cls_wifi_hw *cls_wifi_hw)
{
	struct ipc_host_env_tag *env = cls_wifi_hw->ipc_env;
	void *hostid = env->msga2e_hostid;
	struct lmac_msg lmac_msg;

	env->ops->readn(env->plat, env->radio_idx,
		offsetof(struct ipc_shared_env_tag, msg_a2e_buf.msg), &lmac_msg, sizeof(lmac_msg));

	if (unlikely(!hostid))
		pr_err("%s %d hostid %p\n", __func__, __LINE__, hostid);
	if (unlikely(env->msga2e_cnt != (lmac_msg.src_id & 0xFF)))
		pr_err("%s %d msga2e_cnt 0x%x, src_id 0x%x\n", __func__, __LINE__,
				env->msga2e_cnt, lmac_msg.src_id);

	env->msga2e_hostid = NULL;
	env->msga2e_cnt++;
	env->cb.recv_msgack_ind(cls_wifi_hw, hostid);
}
EXPORT_SYMBOL(ipc_host_msgack_handler);

void cls_wifi_dbg_work(struct work_struct *work)
{
	struct cls_wifi_hw *cls_wifi_hw;
	struct ipc_host_env_tag *env;

	if (work == NULL)
		return;

	cls_wifi_hw = container_of(work, struct cls_wifi_hw, dbg_work);
	if (cls_wifi_hw == NULL) {
		pr_err("%s cls_wifi_hw: %p\n", __func__, cls_wifi_hw);
		return;
	}
	env = cls_wifi_hw->ipc_env;
	if (env == NULL) {
		pr_err("%s ipc_env: %p\n", __func__, env);
		return;
	}

	if (work == NULL || env == NULL)
		return;

	while (env->cb.recv_dbg_ind(cls_wifi_hw, env->dbgbuf[env->dbgbuf_idx]) == 0)
		;
}

void cls_wifi_dbg_cmn_work(struct work_struct *work)
{
	struct cls_wifi_hw *cls_wifi_hw ;
	struct ipc_host_cmn_env_tag *env;

	if (work == NULL)
		return;

	cls_wifi_hw = container_of(work, struct cls_wifi_hw, dbg_cmn_work);
	if (cls_wifi_hw == NULL) {
		pr_err("%s cls_wifi_hw: %p\n", __func__, cls_wifi_hw);
		return;
	}
	env = cls_wifi_hw->ipc_cmn_env;
	if (env == NULL) {
		pr_err("%s ipc_cmn_env: %p\n", __func__, env);
		return;
	}

	while (env->cb.recv_dbg_ind(cls_wifi_hw, env->dbgbuf[env->dbgbuf_idx]) == 0)
		;
}


void cls_wifi_csi_work(struct work_struct *work)
{
	struct cls_wifi_hw *cls_wifi_hw;
	struct ipc_host_env_tag *env;

	if (work == NULL)
		return;

	cls_wifi_hw = container_of(work, struct cls_wifi_hw, csi_work);
	if (cls_wifi_hw == NULL) {
		pr_err("%s cls_wifi_hw: %p\n", __func__, cls_wifi_hw);
		return;
	}
	env = cls_wifi_hw->ipc_env;
	if (env == NULL) {
		pr_err("%s ipc_env: %p\n", __func__, env);
		return;
	}

	env->cb.recv_csi_ind(cls_wifi_hw, env->csibuf[env->csibuf_idx]);
}

void cls_wifi_trig_dump_wpu(struct cls_wifi_hw *cls_wifi_hw, uint32_t val)
{
	struct ipc_host_env_tag *ipc_env;

	ipc_env = cls_wifi_hw->ipc_env;
	if (ipc_env && ipc_env->plat->hw_rev == CLS_WIFI_HW_MERAK2000) {
		ipc_env->ops->write32(ipc_env->plat, ipc_env->radio_idx,
				offsetof(struct ipc_shared_env_tag, dump_wmac_info_flag), val);
		ipc_env->ops->irq_trigger(ipc_env->plat,
				ipc_env->radio_idx, IPC_IRQ_A2E_DBG);
	}
}

/**
 * ipc_host_dbg_handler() - Handle the reception of Debug event
 *
 * @env: pointer to the IPC Host environment
 *
 * Called from general IRQ handler when status %IPC_IRQ_E2A_DBG is set
 */
void ipc_host_dbg_handler(struct cls_wifi_hw *cls_wifi_hw)
{
	schedule_work(&cls_wifi_hw->dbg_work);
}
EXPORT_SYMBOL(ipc_host_dbg_handler);

void ipc_host_cmn_dbg_handler(struct cls_wifi_hw *cls_wifi_hw)
{
	schedule_work(&cls_wifi_hw->dbg_cmn_work);
}
EXPORT_SYMBOL(ipc_host_cmn_dbg_handler);


void ipc_host_csi_handler(struct cls_wifi_hw *cls_wifi_hw)
{
	///cls_wifi_csi_work
	queue_work(cls_wifi_hw->snr_workqueue, &cls_wifi_hw->csi_work);
	///schedule_work(&cls_wifi_hw->csi_work);
}
EXPORT_SYMBOL(ipc_host_csi_handler);

void ipc_host_atf_stats_handler(struct cls_wifi_hw *cls_wifi_hw)
{
	struct ipc_host_env_tag *env = cls_wifi_hw->ipc_env;

	env->cb.recv_atf_stats_ind(cls_wifi_hw, env->atf_stats_buf);
}
EXPORT_SYMBOL(ipc_host_atf_stats_handler);


#if WIFI_DRV_OPTI_SPIN_LOCK_CFG
#define SKB_FREE_TEMP_SIZE   128
int wifi_drv_opti_spin_lock_lvl(void *pthis)
{
////struct cls_wifi_hw *cls_wifi_hw = pthis;
    int spin_lock_lvl = txfree_wq & 0xFU;

    return spin_lock_lvl;
}

void cls_wifi_tx_free_skb_work_handler(struct work_struct *work)
{
    struct cls_wifi_hw *cls_wifi_hw = container_of(work, struct cls_wifi_hw, tx_free_skb_work);
    struct sk_buff *skb;
    //if(txfwq_hdr_on_cpu == 0xFF)
    //     txfwq_hdr_on_cpu = raw_smp_processor_id();

    skb = skb_dequeue(&cls_wifi_hw->tx_free_skb_queue);
    while (skb != NULL) {
        consume_skb(skb);
        skb = skb_dequeue(&cls_wifi_hw->tx_free_skb_queue);
    }
}

int cls_wifi_tx_free_skb_work_push(struct cls_wifi_hw *cls_wifi_hw, struct sk_buff *skb,int cpu_idx)
{
    //if(txfwq_on_cpu == 0xFF)
    //    txfwq_on_cpu = raw_smp_processor_id();
    if(skb != NULL)
        skb_queue_tail(&cls_wifi_hw->tx_free_skb_queue, skb);

    if ((cpu_idx >= 0) && (cpu_idx <= 3)) {
        queue_work_on(cpu_idx, cls_wifi_hw->tx_free_skb_workqueue, &cls_wifi_hw->tx_free_skb_work);
    }
    else {
        queue_work(cls_wifi_hw->tx_free_skb_workqueue, &cls_wifi_hw->tx_free_skb_work);
    }

    return 0;
}

///on_cpu: core0 = 1, core1 = 2, core2 = 3, core3 = 4
void ipc_host_txcfm_free_skb(struct cls_wifi_hw *cls_wifi_hw,
                int on_cpu,struct sk_buff * skb, int work)
{
    if (on_cpu <= 0) {
        consume_skb(skb);
    }
    else {
        if (work == 0) {
            skb_queue_tail(&cls_wifi_hw->tx_free_skb_queue, skb);
        } else {
            cls_wifi_tx_free_skb_work_push(cls_wifi_hw, skb, on_cpu - 1);
        }
    }
}



#endif

/**
 * ipc_host_tx_cfm_handler() - Handle the reception of TX confirmation
 *
 * @env: pointer to the IPC Host environment
 * @queue_idx: index of the hardware on which the confirmation has been received
 * @user_pos: index of the user position
 *
 * Called from general IRQ handler when status %IPC_IRQ_E2A_TXCFM is set.
 * Process confirmations in order until:
 * - There is no more buffer pushed (no need to check confirmation in this case)
 * - The confirmation has not been updated by firmware
 */
void ipc_host_tx_cfm_handler(struct cls_wifi_hw *cls_wifi_hw)
{
	struct ipc_host_env_tag *env = cls_wifi_hw->ipc_env;
	struct cls_wifi_ipc_buf **txcfm = ipc_host_get_txcfm_from_env(env);
#if (WIFI_DRV_OPTI_SPIN_LOCK_CFG)
	int txfree_wq = wifi_drv_opti_spin_lock_lvl(env->pthis);
	void* temp_free_skb[SKB_FREE_TEMP_SIZE];
	struct {
		unsigned int skb_array_size;
		void* temp_free_skb;
	}freeskb_tab;
	void *arg[2];
	unsigned int total_size = 0, i;

	if(txfree_wq) {
		arg[1] = &freeskb_tab;
	}else{
		arg[1] = NULL;
	}
#endif

	while (true) {
		spin_lock(&cls_wifi_hw->tx_lock);
		if (list_empty(&env->tx_hostid_pushed)) {
			spin_unlock(&cls_wifi_hw->tx_lock);
			break;
		}
		spin_unlock(&cls_wifi_hw->tx_lock);

#if (WIFI_DRV_OPTI_SPIN_LOCK_CFG)
		if(txfree_wq) {
			freeskb_tab.skb_array_size = SKB_FREE_TEMP_SIZE - total_size;
			freeskb_tab.temp_free_skb = &temp_free_skb[total_size];
		}
		arg[0] = txcfm[env->txcfm_idx];

		if (env->cb.send_data_cfm(cls_wifi_hw, arg))
			break;

		if(txfree_wq) {
			total_size += freeskb_tab.skb_array_size;
			if (total_size >= (SKB_FREE_TEMP_SIZE - 32)) {
				for(i = 0; i < total_size; i++) {
					if(temp_free_skb[i] != NULL) {
						ipc_host_txcfm_free_skb((struct cls_wifi_hw *)env->pthis, txfree_wq,
														(struct sk_buff *)temp_free_skb[i], 0);
						temp_free_skb[i] = NULL;
					}
				}
				total_size = 0;
				if(txfree_wq)
					ipc_host_txcfm_free_skb((struct cls_wifi_hw *)env->pthis,
												txfree_wq, NULL, 1);
			}
		}

		env->txcfm_idx++;
		if (env->txcfm_idx == env->txcfm_nb)
			env->txcfm_idx = 0;
#else
		if (env->cb.send_data_cfm(cls_wifi_hw, txcfm[env->txcfm_idx]))
			break;

		env->txcfm_idx++;
		if (env->txcfm_idx == env->txcfm_nb)
			env->txcfm_idx = 0;
#endif
	}

#if (WIFI_DRV_OPTI_SPIN_LOCK_CFG)
	if(txfree_wq) {
		i = 0;
		for (i = 0; i < total_size; i++) {
			if(temp_free_skb[i] != NULL) {
				ipc_host_txcfm_free_skb((struct cls_wifi_hw *)env->pthis,txfree_wq,(struct sk_buff *)temp_free_skb[i],0);
				temp_free_skb[i] = NULL;
			}
		}

		if ( total_size > 0 ) {
			cls_wifi_tx_free_skb_work_push((struct cls_wifi_hw *)env->pthis, NULL, txfree_wq - 1);
		}
	}
#endif
}
EXPORT_SYMBOL(ipc_host_tx_cfm_handler);

/**
 ******************************************************************************
 */
bool ipc_host_tx_frames_pending(struct ipc_host_env_tag *env)
{
	return !list_empty(&env->tx_hostid_pushed);
}

/**
 ******************************************************************************
 */
void *ipc_host_tx_flush(struct ipc_host_env_tag *env)
{
	struct ipc_hostid *tx_hostid;
	void *hostptr;

	tx_hostid = list_first_entry_or_null(&env->tx_hostid_pushed, struct ipc_hostid, list);

	if (!tx_hostid)
		return NULL;
	hostptr = tx_hostid->hostptr;
	list_del(&tx_hostid->list);
	tx_hostid->hostptr = NULL;
	list_add_tail(&tx_hostid->list, &env->tx_hostid_available);
	return hostptr;
}

void ipc_host_ipc_pattern_set(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index, u32 value)
{
	cls_wifi_plat->ep_ops->write32(cls_wifi_plat, radio_index, offsetof(struct ipc_shared_env_tag, ipc_pattern), value);
}
EXPORT_SYMBOL(ipc_host_ipc_pattern_set);

void ipc_host_ipc_cal_set(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index, u32 value)
{
	cls_wifi_plat->ep_ops->write32(cls_wifi_plat, radio_index,
			offsetof(struct ipc_shared_env_tag, cal_enabled), value);
}

void ipc_host_wpu_ipc_pattern_set(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index, u32 value)
{
	cls_wifi_plat->ep_ops->write32(cls_wifi_plat, radio_index, offsetof(struct ipc_shared_env_tag, wpu_ipc_pattern), value);
}
EXPORT_SYMBOL(ipc_host_wpu_ipc_pattern_set);
u32 ipc_host_wpu_ipc_pattern_get(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index)
{
	return cls_wifi_plat->ep_ops->read32(cls_wifi_plat, radio_index, offsetof(struct ipc_shared_env_tag, wpu_ipc_pattern));
}

int ipc_host_log_write(struct ipc_host_env_tag *env, uint32_t offset, void *dst, uint32_t len)
{
	env->ops->cpu_writen(env->plat, env->radio_idx,
			env->plat->hw_params.log_offset[env->radio_idx] + offset, dst, len);
	return 0;
}

void ipc_host_trace_pattern_set(struct ipc_host_env_tag *env, u32 value)
{
	env->ops->write32(env->plat, env->radio_idx, offsetof(struct ipc_shared_env_tag, trace_pattern), value);
}

u32 ipc_host_trace_pattern_get(struct ipc_host_env_tag *env)
{
	return env->ops->read32(env->plat, env->radio_idx, (offsetof(struct ipc_shared_env_tag, trace_pattern)));
}

u32 ipc_host_trace_offset_get(struct ipc_host_env_tag *env)
{
	return env->ops->read32(env->plat, env->radio_idx, (offsetof(struct ipc_shared_env_tag, trace_offset)));
}

u32 ipc_host_trace_data_get(struct ipc_host_env_tag *env, u32 offset)
{
	u32 data;

	ipc_host_log_read(env, offset, (void *)&data, sizeof(u32));
	return data;
}

u32 ipc_host_trace_size_get(struct ipc_host_env_tag *env)
{
	return env->ops->read32(env->plat, env->radio_idx, (offsetof(struct ipc_shared_env_tag, trace_size)));
}

u32 ipc_host_trace_nb_compo_get(struct ipc_host_env_tag *env)
{
	return env->ops->read32(env->plat, env->radio_idx, (offsetof(struct ipc_shared_env_tag, trace_nb_compo)));
}

u32 ipc_host_trace_offset_compo_get(struct ipc_host_env_tag *env)
{
	return env->ops->read32(env->plat, env->radio_idx, (offsetof(struct ipc_shared_env_tag, trace_offset_compo)));
}

void ipc_host_trace_compo_set(struct ipc_host_env_tag *env, u32 compo_id, u32 value)
{
	env->ops->write32(env->plat, env->radio_idx, (offsetof(struct ipc_shared_env_tag, trace_offset_compo) +
			env->ops->read32(env->plat, env->radio_idx,
			(offsetof(struct ipc_shared_env_tag, trace_offset_compo))) + 4 * compo_id), value);
}

u32 ipc_host_trace_compo_get(struct ipc_host_env_tag *env, u32 compo_id)
{
	return env->ops->read32(env->plat, env->radio_idx, (offsetof(struct ipc_shared_env_tag, trace_offset_compo) +
		env->ops->read32(env->plat, env->radio_idx, (offsetof(struct ipc_shared_env_tag, trace_offset_compo))) +
		4 * compo_id));
}

void ipc_host_trace_start_set(struct ipc_host_env_tag *env, u32 value)
{
	env->ops->write32(env->plat, env->radio_idx, (offsetof(struct ipc_shared_env_tag, trace_start)), value);
}

u32 ipc_host_trace_start_get(struct ipc_host_env_tag *env)
{
	return env->ops->read32(env->plat, env->radio_idx, (offsetof(struct ipc_shared_env_tag, trace_start)));
}

void ipc_host_trace_end_set(struct ipc_host_env_tag *env, u32 value)
{
	env->ops->write32(env->plat, env->radio_idx, (offsetof(struct ipc_shared_env_tag, trace_end)), value);
}

u32 ipc_host_trace_end_get(struct ipc_host_env_tag *env)
{
	return env->ops->read32(env->plat, env->radio_idx, (offsetof(struct ipc_shared_env_tag, trace_end)));
}

void ipc_host_dbg_cnt_get(struct ipc_host_env_tag *env, struct ipc_shared_dbg_cnt *dbg_cnt)
{
 	env->ops->readn(env->plat, env->radio_idx, env->dbgcnt_offset,
 			dbg_cnt, sizeof(struct ipc_shared_dbg_cnt));
}

u32 ipc_host_dbg_cnt_addr_get(struct ipc_host_env_tag *env)
{
	return env->ops->read32(env->plat, env->radio_idx, (offsetof(struct ipc_shared_env_tag, dbg_cnt_addr)));
}

void ipc_host_dbg_cnt_addr_push(struct ipc_host_env_tag *env, struct cls_wifi_ipc_buf *buf)
{
	env->dbg_cnt_buf = buf;
	return env->ops->write32(env->plat, env->radio_idx,
		offsetof(struct ipc_shared_env_tag, dbg_cnt_addr),
		buf->dma_addr);
}


/**
 ******************************************************************************
 */
void ipc_host_init(struct ipc_host_env_tag *env,
				  struct ipc_host_cb_tag *cb,
				  void *pthis)
{
	struct cls_wifi_hw *cls_wifi_hw = pthis;
	unsigned int i;
	struct ipc_hostid *tx_hostid;
	uint32_t txcfm_nb;
	uint32_t rxdesc_nb;
	uint32_t ipc_env_size;

	txcfm_nb = ipc_host_get_txcfm_nb(cls_wifi_hw);
	rxdesc_nb = ipc_host_get_rxdesc_nb(cls_wifi_hw);
	ipc_env_size = sizeof(struct ipc_host_env_tag) + txcfm_nb * sizeof(struct ipc_hostid) +
			txcfm_nb * sizeof(struct cls_wifi_ipc_buf *) +
			rxdesc_nb * sizeof(struct cls_wifi_ipc_buf *);

	// Reset the IPC Host environment
	memset(env, 0, ipc_env_size);

	env->radio_idx = cls_wifi_hw->radio_idx;
	env->ops = cls_wifi_hw->plat->ep_ops;
	env->plat = cls_wifi_hw->plat;

	env->e2amsg_nb =  IPC_MSGE2A_BUF_CNT;
	env->dbgbuf_nb =  IPC_DBGBUF_CNT;

#if defined(MERAK2000)
		env->dbgcnt_nb =  0;
#else
		env->dbgcnt_nb =  1;
#endif
	env->buffered_nb = hw_remote_sta_max(cls_wifi_hw) * TID_MAX;
	// Save the callbacks in our own environment
	env->cb = *cb;

	// Save the pointer to the register base
	env->pthis = pthis;

	env->buffered_offset = sizeof(struct ipc_shared_env_tag);
	env->buffered_size =  sizeof(u32_l) * env->buffered_nb;

	// Initialize buffers numbers and buffers sizes needed for DMA Receptions
	env->txdesc_nb = ipc_host_get_txdesc_nb(cls_wifi_hw);
	env->pp_cnt[0] = cls_wifi_hw->plat->hw_params.txdesc_cnt0[cls_wifi_hw->radio_idx];
	env->pp_cnt[1] = cls_wifi_hw->plat->hw_params.txdesc_cnt1[cls_wifi_hw->radio_idx];
	env->pp_cnt[2] = cls_wifi_hw->plat->hw_params.txdesc_cnt2[cls_wifi_hw->radio_idx];
	env->pp_cnt[3] = cls_wifi_hw->plat->hw_params.txdesc_cnt3[cls_wifi_hw->radio_idx];
	env->pp_cnt[4] = cls_wifi_hw->plat->hw_params.txdesc_cnt4[cls_wifi_hw->radio_idx]
		+ cls_wifi_hw->plat->hw_params.txdesc_mu_cnt[cls_wifi_hw->radio_idx] + 1;
	env->pp_offset[0] = 0;
	env->pp_offset[1] = env->pp_offset[0] + env->pp_cnt[0];
	env->pp_offset[2] = env->pp_offset[1] + env->pp_cnt[1];
	env->pp_offset[3] = env->pp_offset[2] + env->pp_cnt[2];
	env->pp_offset[4] = env->pp_offset[3] + env->pp_cnt[3];
	env->txdesc_offset = env->buffered_offset + env->buffered_size ;
	env->txdesc_size = sizeof(u32_l) * env->txdesc_nb;
	env->pp_valid = false;

	env->txcfm_nb = ipc_host_get_txcfm_nb(cls_wifi_hw);
	env->txcfm_offset = env->txdesc_offset + env->txdesc_size;
	env->txcfm_size = sizeof(u32_l) * env->txcfm_nb;

	env->rxdesc_nb = ipc_host_get_rxdesc_nb(cls_wifi_hw);
	env->rxdesc_offset = env->txcfm_offset + env->txcfm_size;
	env->rxdesc_size = sizeof(struct ipc_shared_rx_desc) * env->rxdesc_nb;

	env->rxbuf_nb = ipc_host_get_rxbuf_nb(cls_wifi_hw);
	env->rxbuf_offset = env->rxdesc_offset + env->rxdesc_size;
	env->rxbuf_size = sizeof(struct ipc_shared_rx_buf) * env->rxbuf_nb;

	env->e2amsg_offset = env->rxbuf_offset + env->rxbuf_size;
	env->e2amsg_size = sizeof(u32_l) * env->e2amsg_nb;
	env->dbgbuf_offset = env->e2amsg_offset + env->e2amsg_size;
	env->dbgbuf_size = sizeof(u32_l) * env->dbgbuf_nb;
	if(env->dbgcnt_nb)
	{
		env->dbgcnt_offset = env->dbgbuf_offset + env->dbgbuf_size;
		env->dbgcnt_size = sizeof(struct ipc_shared_dbg_cnt);
	}
	if(env->dbgcnt_nb)
		env->ipc_shared_size = env->dbgcnt_offset + env->dbgcnt_size;
	else
		env->ipc_shared_size = env->dbgbuf_offset + env->dbgbuf_size;

	if ((env->plat->hw_params.hw_rev == CLS_WIFI_HW_MERAK2000 &&
	     ((env->radio_idx == 0 &&
	       env->ipc_shared_size > CLS_WIFI_MERAK2000_MEM_SIZE_0) ||
	     (env->radio_idx == 1 &&
	      env->ipc_shared_size > CLS_WIFI_MERAK2000_MEM_SIZE_1))) ||
	    (env->plat->hw_params.hw_rev == CLS_WIFI_HW_MERAK3000 &&
	     ((env->radio_idx == 0 &&
	       env->ipc_shared_size > CLS_WIFI_MERAK3000_MEM_SIZE_0) ||
	      (env->radio_idx == 1 &&
	       env->ipc_shared_size > CLS_WIFI_MERAK3000_MEM_SIZE_1))))
		pr_err("Fatal: %s %d 0x%x too large.\n", __func__, __LINE__, env->ipc_shared_size);

	env->unsuprxvec_sz = max_t(size_t, sizeof(struct rx_vector_desc), RADIOTAP_HDR_MAX_LEN) +
			RADIOTAP_HDR_VEND_MAX_LEN + UNSUP_RX_VEC_DATA_LEN;

	// Reset shared memory
	env->ops->writen(env->plat, env->radio_idx, 0, 0, env->ipc_shared_size);

	// Set calibration flag before ipc pattern
	ipc_host_ipc_cal_set(cls_wifi_hw->plat, cls_wifi_hw->radio_idx,
			cls_wifi_hw->radio_params->debug_mode);

	ipc_host_ipc_pattern_set(cls_wifi_hw->plat, cls_wifi_hw->radio_idx, IPC_PATTERN_MAGIC);

	INIT_LIST_HEAD(&env->tx_hostid_available);
	INIT_LIST_HEAD(&env->tx_hostid_pushed);
	tx_hostid = ipc_host_get_tx_hostid_from_env(env);
	for (i = 0; i < env->txcfm_nb; i++, tx_hostid++) {
		tx_hostid->hostid = i + 1; // +1 so that 0 is not a valid value
		list_add_tail(&tx_hostid->list, &env->tx_hostid_available);
	}
}

/**
 ******************************************************************************
 */
void ipc_host_thd_pattern_push(struct ipc_host_env_tag *env, struct cls_wifi_ipc_buf *buf)
{
	env->ops->write32(env->plat, env->radio_idx, offsetof(struct ipc_shared_env_tag, thd_pattern_addr), buf->dma_addr);
}

void ipc_host_tbd_pattern_push(struct ipc_host_env_tag *env, struct cls_wifi_ipc_buf *buf)
{
	env->ops->write32(env->plat, env->radio_idx, offsetof(struct ipc_shared_env_tag, tbd_pattern_addr), buf->dma_addr);
}

/**
 ******************************************************************************
 */
int ipc_host_rxbuf_init_push(struct ipc_host_env_tag *env, struct cls_wifi_ipc_buf *buf)
{
	env->ops->write32(env->plat, env->radio_idx, (env->rxbuf_offset + env->rxbuf_idx *
			sizeof(struct ipc_shared_rx_buf)), CLS_WIFI_RXBUFF_HOSTID_GET(buf));
	env->ops->write32(env->plat, env->radio_idx, (env->rxbuf_offset + env->rxbuf_idx *
			sizeof(struct ipc_shared_rx_buf) + sizeof(u32_l)), buf->dma_addr);

	// Increment the array index
	env->rxbuf_idx = (env->rxbuf_idx + 1) % env->rxbuf_nb;

	return 0;
}

int ipc_host_cmn_rxbuf_init_push_2g(struct ipc_host_cmn_env_tag *env, struct cls_wifi_ipc_buf *buf)
{
	env->ops->write32(env->plat, env->radio_idx,
		offsetof(struct cmn_ipc_shared_env_tag, host_rxbuf_2g[env->rxbuf_idx_2g]) +
		offsetof(struct ipc_shared_rx_buf, hostid),
		CLS_WIFI_RXBUFF_HOSTID_GET(buf));

	env->ops->write32(env->plat, env->radio_idx,
		offsetof(struct cmn_ipc_shared_env_tag, host_rxbuf_2g[env->rxbuf_idx_2g]) +
		offsetof(struct ipc_shared_rx_buf, dma_addr),
		buf->dma_addr);

	// Increment the array index
	env->rxbuf_idx_2g = (env->rxbuf_idx_2g + 1) % CMN_RXBUF_CNT_2G;

	return 0;
}

int ipc_host_cmn_rxbuf_init_push_5g(struct ipc_host_cmn_env_tag *env, struct cls_wifi_ipc_buf *buf)
{
	env->ops->write32(env->plat, env->radio_idx,
		offsetof(struct cmn_ipc_shared_env_tag, host_rxbuf_5g[env->rxbuf_idx_5g]) +
		offsetof(struct ipc_shared_rx_buf, hostid),
		CLS_WIFI_RXBUFF_HOSTID_GET(buf));

	env->ops->write32(env->plat, env->radio_idx,
		offsetof(struct cmn_ipc_shared_env_tag, host_rxbuf_5g[env->rxbuf_idx_5g]) +
		offsetof(struct ipc_shared_rx_buf, dma_addr),
		buf->dma_addr);

	// Increment the array index
	env->rxbuf_idx_5g = (env->rxbuf_idx_5g + 1) % CMN_RXBUF_CNT_5G;

	return 0;
}
/**
 ******************************************************************************
 */
int ipc_host_rxbuf_push(struct ipc_host_env_tag *env, struct cls_wifi_ipc_buf *buf)
{
	env->ops->write32(env->plat, env->radio_idx, (env->rxbuf_offset + env->rxbuf_idx *
			sizeof(struct ipc_shared_rx_buf)), CLS_WIFI_RXBUFF_HOSTID_GET(buf));
	env->ops->write32(env->plat, env->radio_idx, (env->rxbuf_offset + env->rxbuf_idx *
			sizeof(struct ipc_shared_rx_buf) + sizeof(u32_l)), buf->dma_addr);
	if (!use_msgq)
		env->ops->irq_trigger(env->plat, env->radio_idx, IPC_IRQ_A2E_RXBUF_BACK);
	// Increment the array index
	env->rxbuf_idx = (env->rxbuf_idx + 1) % env->rxbuf_nb;

	return 0;
}

int ipc_host_cmn_rxbuf_push_2g(struct ipc_host_cmn_env_tag *env, struct cls_wifi_ipc_buf *buf)
{
	env->ops->write32(env->plat, env->radio_idx,
		offsetof(struct cmn_ipc_shared_env_tag, host_rxbuf_2g[env->rxbuf_idx_2g]) +
		offsetof(struct ipc_shared_rx_buf, hostid),
		CLS_WIFI_RXBUFF_HOSTID_GET(buf));

	env->ops->write32(env->plat, env->radio_idx,
		offsetof(struct cmn_ipc_shared_env_tag, host_rxbuf_2g[env->rxbuf_idx_2g]) +
		offsetof(struct ipc_shared_rx_buf, dma_addr),
		buf->dma_addr);

//	if (!use_msgq)
//		env->ops->irq_trigger(env->plat, env->radio_idx, IPC_IRQ_A2E_RXBUF_BACK);

	// Increment the array index
	env->rxbuf_idx_2g = (env->rxbuf_idx_2g + 1) % CMN_RXBUF_CNT_2G;

	return 0;
}

int ipc_host_cmn_rxbuf_push_5g(struct ipc_host_cmn_env_tag *env, struct cls_wifi_ipc_buf *buf)
{
	env->ops->write32(env->plat, env->radio_idx,
		offsetof(struct cmn_ipc_shared_env_tag, host_rxbuf_5g[env->rxbuf_idx_5g]) +
		offsetof(struct ipc_shared_rx_buf, hostid),
		CLS_WIFI_RXBUFF_HOSTID_GET(buf));

	env->ops->write32(env->plat, env->radio_idx,
		offsetof(struct cmn_ipc_shared_env_tag, host_rxbuf_5g[env->rxbuf_idx_5g]) +
		offsetof(struct ipc_shared_rx_buf, dma_addr),
		buf->dma_addr);

//	if (!use_msgq)
//		env->ops->irq_trigger(env->plat, env->radio_idx, IPC_IRQ_A2E_RXBUF_BACK);

	// Increment the array index
	env->rxbuf_idx_5g = (env->rxbuf_idx_5g + 1) % CMN_RXBUF_CNT_5G;

	return 0;
}
/**
 ******************************************************************************
 */
int ipc_host_rxdesc_init_push(struct ipc_host_env_tag *env, struct cls_wifi_ipc_buf *buf)
{
	struct cls_wifi_ipc_buf **rxdesc = ipc_host_get_rxdesc_from_env(env);

	rxdesc[env->rxdesc_idx] = buf;
	if (use_msgq) {
		cls_wifi_msgq_push((struct cls_wifi_hw *)env->pthis,
				env->plat->hw_params.msgq_rxdesc[env->radio_idx], buf->dma_addr);
	} else {
		env->ops->write32(env->plat, env->radio_idx, (env->rxdesc_offset + env->rxdesc_idx *
			sizeof(struct ipc_shared_rx_desc)), buf->dma_addr);
	}
	env->rxdesc_idx = (env->rxdesc_idx + 1) % env->rxdesc_nb;
	return 0;
}

int ipc_host_cmn_rxdesc_init_push_2g(struct ipc_host_cmn_env_tag *env, struct cls_wifi_ipc_buf *buf)
{
	env->rxdesc_2g[env->rxdesc_idx_2g]= buf;
	env->ops->write32(env->plat, env->radio_idx,
		offsetof(struct cmn_ipc_shared_env_tag, host_rxdesc_2g[env->rxdesc_idx_2g]),
		buf->dma_addr);
	env->rxdesc_idx_2g = (env->rxdesc_idx_2g + 1) % CMN_RXDESC_CNT_MAX_2G;
	return 0;
}

int ipc_host_cmn_rxdesc_init_push_5g(struct ipc_host_cmn_env_tag *env, struct cls_wifi_ipc_buf *buf)
{
	env->rxdesc_5g[env->rxdesc_idx_5g]= buf;
	env->ops->write32(env->plat, env->radio_idx,
		offsetof(struct cmn_ipc_shared_env_tag, host_rxdesc_5g[env->rxdesc_idx_5g]),
		buf->dma_addr);
	env->rxdesc_idx_5g = (env->rxdesc_idx_5g + 1) % CMN_RXDESC_CNT_MAX_5G;
	return 0;
}
/**
 ******************************************************************************
 */
int ipc_host_rxdesc_push(struct ipc_host_env_tag *env, struct cls_wifi_ipc_buf *buf)
{
	struct cls_wifi_ipc_buf **rxdesc = ipc_host_get_rxdesc_from_env(env);

	if (rxdesc[env->rxdesc_idx] != buf)
		pr_err("WARNING: [%s][%d] env->rxdesc_idx(%d) rxdesc != buf\n",
				__func__, __LINE__, env->rxdesc_idx);

	rxdesc[env->rxdesc_idx] = buf;
	if (use_msgq) {
		cls_wifi_msgq_push((struct cls_wifi_hw *)env->pthis,
				env->plat->hw_params.msgq_rxdesc[env->radio_idx], buf->dma_addr);
	} else {
		env->ops->write32(env->plat, env->radio_idx, (env->rxdesc_offset + env->rxdesc_idx *
				sizeof(struct ipc_shared_rx_desc)), buf->dma_addr);
		env->ops->irq_trigger(env->plat, env->radio_idx, IPC_IRQ_A2E_RXDESC_BACK);
	}
	env->rxdesc_idx = (env->rxdesc_idx + 1) % env->rxdesc_nb;
	return 0;
}

int ipc_host_cmn_rxdesc_push_2g(struct ipc_host_cmn_env_tag *env, struct cls_wifi_ipc_buf *buf)
{
	env->rxdesc_2g[env->rxdesc_idx_2g]= buf;
	env->ops->write32(env->plat, env->radio_idx,
		offsetof(struct cmn_ipc_shared_env_tag, host_rxdesc_2g[env->rxdesc_idx_2g]),
		buf->dma_addr);
//	if (!use_msgq)
//		env->ops->irq_trigger(env->plat, env->radio_idx, IPC_IRQ_A2E_RXDESC_BACK);
	env->rxdesc_idx_2g = (env->rxdesc_idx_2g + 1) % CMN_RXDESC_CNT_MAX_2G;
	return 0;
}

int ipc_host_cmn_rxdesc_push_5g(struct ipc_host_cmn_env_tag *env, struct cls_wifi_ipc_buf *buf)
{
	env->rxdesc_5g[env->rxdesc_idx_5g]= buf;
	env->ops->write32(env->plat, env->radio_idx,
		offsetof(struct cmn_ipc_shared_env_tag, host_rxdesc_5g[env->rxdesc_idx_5g]),
		buf->dma_addr);
//	if (!use_msgq)
//		env->ops->irq_trigger(env->plat, env->radio_idx, IPC_IRQ_A2E_RXDESC_BACK);
	env->rxdesc_idx_5g = (env->rxdesc_idx_5g + 1) % CMN_RXDESC_CNT_MAX_5G;
	return 0;
}
/**
 ******************************************************************************
 */
int ipc_host_radar_push(struct ipc_host_env_tag *env, struct cls_wifi_ipc_buf *buf)
{
	// Save Ipc buffer in host env
	env->radar[env->radar_idx] = buf;

	// Copy the DMA address in the ipc shared memory
	env->ops->write32(env->plat, env->radio_idx, offsetof(struct ipc_shared_env_tag, radarbuf_hostbuf[env->radar_idx]), buf->dma_addr);

	// Increment the array index
	env->radar_idx = (env->radar_idx + 1) % IPC_RADARBUF_CNT;

	return 0;
}

/**
 ******************************************************************************
 */
int ipc_host_he_mu_map_push(struct ipc_host_env_tag *env, struct cls_wifi_ipc_buf *buf)
{
	// Save Ipc buffer in host env
	env->hemumap[env->hemumap_idx] = buf;

	// Copy the DMA address in the ipc shared memory
	env->ops->write32(env->plat, env->radio_idx, offsetof(struct ipc_shared_env_tag, hemubuf_hostbuf[env->hemumap_idx]), buf->dma_addr);

	// Increment the array index
	env->hemumap_idx = (env->hemumap_idx + 1) % IPC_HEMUBUF_CNT;

	return 0;
}

/**
 ******************************************************************************
 */
int ipc_host_csi_buf_push(struct ipc_host_env_tag *env, struct cls_wifi_ipc_buf *buf)
{
	// Save Ipc buffer in host env
	env->csibuf[env->csibuf_idx] = buf;

	// Copy the DMA address in the ipc shared memory
	env->ops->write32(env->plat, env->radio_idx,
		offsetof(struct ipc_shared_env_tag, csibuf_hostbuf[env->csibuf_idx]),
		buf->dma_addr);

	// Increment the array index
	env->csibuf_idx = (env->csibuf_idx + 1) % IPC_CSIBUF_CNT;

	return 0;
}

/**
 ******************************************************************************
 */
int ipc_host_unsuprxvec_push(struct ipc_host_env_tag *env, struct cls_wifi_ipc_buf *buf)
{
	env->unsuprxvec[env->unsuprxvec_idx] = buf;
	env->ops->write32(env->plat, env->radio_idx, offsetof(struct ipc_shared_env_tag, unsuprxvecbuf_hostbuf[env->unsuprxvec_idx]), buf->dma_addr);

	env->unsuprxvec_idx = (env->unsuprxvec_idx + 1) % IPC_UNSUPRXVECBUF_CNT;

	return 0;
}

/**
 ******************************************************************************
 */
int ipc_host_msgbuf_push(struct ipc_host_env_tag *env, struct cls_wifi_ipc_buf *buf)
{
	env->msgbuf[env->msgbuf_idx] = buf;
	env->ops->write32(env->plat, env->radio_idx,
			(env->e2amsg_offset + env->msgbuf_idx * sizeof(u32_l)),
			buf->dma_addr);
	env->msgbuf_idx = (env->msgbuf_idx + 1) % env->e2amsg_nb;

	return 0;
}

/**
 ******************************************************************************
 */
int ipc_host_dbgbuf_push(struct ipc_host_env_tag *env, struct cls_wifi_ipc_buf *buf)
{
	if ((env == NULL) || (buf == NULL))
		return -1;

	env->dbgbuf[env->dbgbuf_idx] = buf;
	env->ops->write32(env->plat, env->radio_idx,
			(env->dbgbuf_offset + env->dbgbuf_idx * sizeof(u32_l)),
			buf->dma_addr);

	env->dbgbuf_idx = (env->dbgbuf_idx + 1) % env->dbgbuf_nb;
	return 0;
}

int ipc_host_cmn_dbgbuf_push(struct ipc_host_cmn_env_tag *env, struct cls_wifi_ipc_buf *buf)
{
	///pr_warn("%s dbgbuf_idx: %u\n", __func__, env->dbgbuf_idx);
	env->dbgbuf[env->dbgbuf_idx] = buf;
	env->ops->write32(env->plat, env->radio_idx,
		offsetof(struct cmn_ipc_shared_env_tag, dbg_hostbuf_addr[env->dbgbuf_idx]),
		buf->dma_addr);

	env->dbgbuf_idx = (env->dbgbuf_idx + 1) % CMN_DBG_CNT;

	return 0;
}

/**
 ******************************************************************************
 */
int ipc_host_txcfm_push(struct ipc_host_env_tag *env, struct cls_wifi_ipc_buf *buf)
{
	struct cls_wifi_ipc_buf **txcfm = ipc_host_get_txcfm_from_env(env);

	txcfm[env->txcfm_idx] = buf;
	env->ops->write32(env->plat, env->radio_idx, (env->txcfm_offset + env->txcfm_idx *
			sizeof(u32_l)), buf->dma_addr);

	env->txcfm_idx++;
	if (env->txcfm_idx == env->txcfm_nb)
		env->txcfm_idx = 0;

	return 0;
}

/**
 ******************************************************************************
 */
void ipc_host_dbginfo_push(struct ipc_host_env_tag *env, struct cls_wifi_ipc_buf *buf)
{
	env->ops->write32(env->plat, env->radio_idx, offsetof(struct ipc_shared_env_tag, la_dbginfo_addr), buf->dma_addr);
}

/**
 ******************************************************************************
 */
void ipc_host_txdesc_push(struct ipc_host_env_tag *env, struct cls_wifi_ipc_buf *buf, int hw_queue)
{
	uint32_t idx;
	uint32_t idx_queue;

	if (env->pp_valid) {
		if (hw_queue <= 4)
			idx_queue = hw_queue;
		else
			idx_queue = 4;
		idx = env->pp_idx[idx_queue] + env->pp_offset[idx_queue];
	} else {
		idx = env->txdesc_addr_idx;
	}

	if (use_msgq) {
		cls_wifi_msgq_push((struct cls_wifi_hw *)env->pthis, env->plat->hw_params.msgq_txdesc[env->radio_idx], buf->dma_addr);
	} else {
		// Write DMA address to the descriptor
		env->ops->write32(env->plat, env->radio_idx,
				(env->txdesc_offset + idx * sizeof(u32_l)), buf->dma_addr);
		//memory barrier for write32
		wmb();
		env->ops->irq_trigger(env->plat, env->radio_idx, IPC_IRQ_A2E_TXDESC);
	}

	if (env->pp_valid) {
		if ((env->pp_idx[idx_queue] + 1) == env->pp_cnt[idx_queue])
			env->pp_idx[idx_queue] = 0;
		else
			env->pp_idx[idx_queue]++;
	} else {
		idx++;
		if (idx == env->txdesc_nb)
			env->txdesc_addr_idx = 0;
		else
			env->txdesc_addr_idx = idx;
	}
}

void ipc_host_atf_stats_push(struct ipc_host_env_tag *env, struct cls_wifi_ipc_buf *buf)
{
	env->atf_stats_buf = buf;
	env->ops->write32(env->plat, env->radio_idx,
		offsetof(struct ipc_shared_env_tag, atf_stats_hostbuf),
		buf->dma_addr);
}

void ipc_host_atf_quota_push(struct ipc_host_env_tag *env, struct cls_wifi_ipc_buf *buf)
{
	env->atf_quota_buf = buf;
	env->ops->write32(env->plat, env->radio_idx,
		offsetof(struct ipc_shared_env_tag, atf_quota_hostbuf),
		buf->dma_addr);
}

/**
 * ipc_host_tx_host_ptr_to_id() - Save and convert host pointer to host id
 *
 * @env: pointer to the IPC Host environment
 * @host_ptr: host pointer to save in the ipc_hostid element ()
 * @return: uint32_t value associated to this host buffer.
 *
 * Move a free ipc_hostid from the tx_hostid_available list to the tx_hostid_pushed list.
 * The element is initialized with the host pointer and the associated 32bits value is
 * returned.
 * It is expected that list tx_hostid_available contains at least one element.
 */
uint32_t ipc_host_tx_host_ptr_to_id(struct ipc_host_env_tag *env, void *host_ptr)
{
	struct ipc_hostid *tx_hostid;

	tx_hostid = list_first_entry_or_null(&env->tx_hostid_available, struct ipc_hostid, list);
	if (!tx_hostid)
		return 0;

	list_del(&tx_hostid->list);
	list_add_tail(&tx_hostid->list, &env->tx_hostid_pushed);

	tx_hostid->hostptr = host_ptr;
	return tx_hostid->hostid;
}

/**
 * ipc_host_tx_host_id_to_ptr() - Retrieve host ptr from host id
 *
 * @env: pointer to the IPC Host environment
 * @hostid: hostid present in the confirmation
 * @return: pointer saved via ipc_host_tx_host_ptr_to_id()
 *
 * Allow to retrieve the host ptr (to the tx buffer) form the host id found in
 * the confirmation.
 * Move back the tx_hostid element from the tx_hostid_pushed list to the
 * tx_hostid_available list.
 */
void *ipc_host_tx_host_id_to_ptr(struct ipc_host_env_tag *env, uint32_t hostid)
{
	struct ipc_hostid *tx_hostid = ipc_host_get_tx_hostid_from_env(env);
	void *hostptr;

	if (unlikely(!hostid || (hostid > env->txcfm_nb)))
		return NULL;

	tx_hostid = &tx_hostid[hostid - 1];
	hostptr = tx_hostid->hostptr;
	if (unlikely(!hostptr))
		pr_err("%s %d hostid %d\n", __func__, __LINE__, hostid);

	tx_hostid->hostptr = NULL;
	list_del(&tx_hostid->list);
	list_add_tail(&tx_hostid->list, &env->tx_hostid_available);
	return hostptr;
}

/**
 ******************************************************************************
 */
uint32_t host_irq_status_restore;

void ipc_host_irq(struct cls_wifi_hw *cls_wifi_hw, uint32_t status)
{
	struct ipc_host_env_tag *env = cls_wifi_hw->ipc_env;
	unsigned long force_time;

	env->ops->irq_ack(env->plat, env->radio_idx, status);
	// And re-read the status, just to be sure that the acknowledgment is
	// effective when we start the interrupt handling
	env->ops->irq_get_status(env->plat, env->radio_idx);

	status |= host_irq_status_restore;
	host_irq_status_restore = 0;

	// Optimized for only one IRQ at a time
	// handle the HE MU DL MAP event reception
	if (status & IPC_IRQ_E2A_HE_MU_DESC)
		ipc_host_he_mu_map_handler(cls_wifi_hw);
	// handle the RX descriptor reception
	if (status & IPC_IRQ_E2A_RXDESC) {
		force_time = jiffies + msecs_to_jiffies(cls_wifi_mod_params.irq_force_exit_time);
		ipc_host_rxdesc_handler(cls_wifi_hw);

		if (time_after_eq(jiffies, force_time)) {
			host_irq_status_restore |= IPC_IRQ_E2A_RXDESC;
			pr_info("[%s] IPC_IRQ_E2A_RXDESC continue %d mseconds\n",
					__func__, cls_wifi_mod_params.irq_force_exit_time);
		}
	}
	if (status & IPC_IRQ_E2A_CMN_RXDESC)
		ipc_host_cmn_rxdesc_handler(cls_wifi_hw);
	if (status & IPC_IRQ_E2A_CMN_DBG)
		ipc_host_cmn_dbg_handler(cls_wifi_hw);
	if (status & IPC_IRQ_E2A_MSG_ACK)
		ipc_host_msgack_handler(cls_wifi_hw);
	if (status & IPC_IRQ_E2A_MSG)
		ipc_host_msg_handler(cls_wifi_hw);
	if (status & IPC_IRQ_E2A_TXCFM) {
		int i;

		// handle the TX confirmation reception
		for (i = 0; i < IPC_TXQUEUE_CNT; i++) {
			uint32_t q_bit = CO_BIT(i + IPC_IRQ_E2A_TXCFM_POS);

			// handle the confirmation
			if (status & q_bit)
				ipc_host_tx_cfm_handler(cls_wifi_hw);
		}
	}
	// handle the radar event reception
	if (status & IPC_IRQ_E2A_RADAR)
		ipc_host_radar_handler(cls_wifi_hw);

	// handle the unsupported rx vector reception
	if (status & IPC_IRQ_E2A_UNSUP_RX_VEC)
		ipc_host_unsup_rx_vec_handler(cls_wifi_hw);

	if (status & IPC_IRQ_E2A_DBG)
		ipc_host_dbg_handler(cls_wifi_hw);

	if (status & IPC_IRQ_E2A_CSI)
		ipc_host_csi_handler(cls_wifi_hw);

	if (status & IPC_IRQ_E2A_ATF_STATS)
		ipc_host_atf_stats_handler(cls_wifi_hw);
}
EXPORT_SYMBOL(ipc_host_irq);

/**
 ******************************************************************************
 */
int ipc_host_msg_push(struct ipc_host_env_tag *env, void *msg_buf, uint16_t len)
{
	uint32_t *src = (uint32_t *)((struct cls_wifi_cmd *)msg_buf)->a2e_msg;

	REG_SW_SET_PROFILING(env->pthis, SW_PROF_IPC_MSGPUSH);

	if (unlikely(env->msga2e_hostid))
		pr_err("%s %d hostid %p\n", __func__, __LINE__, env->msga2e_hostid);
	if (unlikely(round_up(len, 4) >= sizeof(struct ipc_a2e_msg)))
		pr_err("%s %d len %d, size %zu\n", __func__, __LINE__,
				len, sizeof(struct ipc_a2e_msg));

	env->ops->writen(env->plat, env->radio_idx, offsetof(struct ipc_shared_env_tag, msg_a2e_buf.msg), src, len);
	env->msga2e_hostid = msg_buf;

	// Trigger the irq to send the message to EMB
	env->ops->irq_trigger(env->plat, env->radio_idx, IPC_IRQ_A2E_MSG);

	REG_SW_CLEAR_PROFILING(env->pthis, SW_PROF_IPC_MSGPUSH);
	return 0;
}

int ipc_host_log_read(struct ipc_host_env_tag *env, uint32_t offset, void *dst, uint32_t len)
{
	env->ops->cpu_readn(env->plat, env->radio_idx,
			env->plat->hw_params.log_offset[env->radio_idx] + offset, dst, len);
	return 0;
}

/**
 ******************************************************************************
 */
void ipc_host_enable_irq(struct ipc_host_env_tag *env, uint32_t value)
{
	env->ops->irq_enable(env->plat, env->radio_idx, value);
}

/**
 ******************************************************************************
 */
void ipc_host_disable_irq(struct ipc_host_env_tag *env, uint32_t value)
{
	env->ops->irq_disable(env->plat, env->radio_idx, value);
}

/**
 ******************************************************************************
 */
uint32_t ipc_host_get_status(struct ipc_host_env_tag *env)
{
	volatile uint32_t status;

	status = env->ops->irq_get_status(env->plat, env->radio_idx);

	return status;
}
EXPORT_SYMBOL(ipc_host_get_status);

/**
 ******************************************************************************
 */
uint32_t ipc_host_get_rawstatus(struct ipc_host_env_tag *env)
{
	volatile uint32_t rawstatus;

	rawstatus = env->ops->irq_get_raw_status(env->plat, env->radio_idx);

	return rawstatus;
}

/**
 ******************************************************************************
 */
u32 ipc_host_heart_ack(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index, u32 value)
{
	if (value == 0)
		return cls_wifi_plat->ep_ops->read32(cls_wifi_plat, radio_index,
							offsetof(struct ipc_shared_env_tag, heart_ack));
	else
		cls_wifi_plat->ep_ops->write32(cls_wifi_plat, radio_index,
						offsetof(struct ipc_shared_env_tag, heart_ack), value);
	return 0;
}
EXPORT_SYMBOL(ipc_host_heart_ack);


/**
 ******************************************************************************
 */
u32 ipc_host_heart_rdy(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index, u32 value)
{
	if (value == 0)
		return cls_wifi_plat->ep_ops->read32(cls_wifi_plat, radio_index,
							offsetof(struct ipc_shared_env_tag, heart_rdy));
	else
		cls_wifi_plat->ep_ops->write32(cls_wifi_plat, radio_index,
						offsetof(struct ipc_shared_env_tag, heart_rdy), value);
	return 0;
}
EXPORT_SYMBOL(ipc_host_heart_rdy);

/**
 ******************************************************************************
 */
u32 ipc_host_uart_info(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index, u32 value)
{
	if (value == 0)
		return cls_wifi_plat->ep_ops->read32(cls_wifi_plat, radio_index,
							offsetof(struct ipc_shared_env_tag, uart_info));
	else
		cls_wifi_plat->ep_ops->write32(cls_wifi_plat, radio_index,
						offsetof(struct ipc_shared_env_tag, uart_info), value);
	return 0;
}
EXPORT_SYMBOL(ipc_host_uart_info);
