#include <linux/kernel.h>
#include <linux/kmod.h>
#include <linux/debugfs.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include "cls_wifi_cali_debugfs.h"
#include "cls_wifi_msg_tx.h"
#include "cls_wifi_irf.h"
#include "cls_wifi_dif_sm.h"
#include "cls_wifi_main.h"
#include <linux/kernel.h>

static char *sm_str[] = {
	"IDLE",
	"FINISH",
	"ONLINE",
	"PAUSE",
	"DBG",
	"EQ",
	"PWR",
	"ZIF",
	"ACI_DET",
	"INVALID"
};
static char *cause_str[] = {
	"done",
	"mode",
	"throughput",
	"update",
	"ant_mask",
	"table",
	"alarm",
	"timeout",
	"unknow"
};

static struct  dif_cali_evt_str evt_cmd[] = {
	{"boot", 	DIF_EVT_START_BOOT_CALI},
	{"online",	DIF_EVT_START_ONLINE_CALI},
	{"pause",	DIF_EVT_CALI_PAUSE},
	{"resume",	DIF_EVT_CALI_RESUME},
	{"reset",	DIF_EVT_CALI_RESET},
	{"stop",	DIF_EVT_CALI_STOP},
	{"zif",		DIF_EVT_START_ZIF_CALI},
};

static char *dif_sm2str(u8 sm_state)
{
	if(sm_state < DIF_SM_INVALID){
		return sm_str[sm_state];
	} else {
		return sm_str[DIF_SM_INVALID];
	}
}

static char *dif_cause2str(u8 cause)
{
	if(cause < DIF_CALI_STOP_CAUSE_MAX){
		return cause_str[cause];
	} else {
		return cause_str[DIF_CALI_STOP_CAUSE_MAX];
	}
}

int dif_evt_str2cmd(char *cmd_str, enum dif_cali_evt_cmd *cmd)
{
	u8 i;

	for(i = 0; i < NELEMENTS(evt_cmd); i++){
		if(!strcasecmp(cmd_str,evt_cmd[i].evt_str)){
			*cmd = evt_cmd[i].evt_cmd;
			return 0;
		}
	}
	return -1;
}


int cls_wifi_dif_sm_set_event(struct cls_wifi_hw *cls_wifi_hw, enum dif_cali_evt_cmd dif_cali_cmd)
{
	struct irf_set_cali_evt_req req;
	int ret;
	struct irf_set_cali_evt_cfm set_cali_cfm;
	u8 dif_drv_state;
	u8 dif_fw_state = cls_wifi_hw->dif_sm.dif_fw_state;
	struct cls_wifi_dif_sch *dif_sch;

	if (!cls_wifi_hw->plat || !cls_wifi_hw->plat->dif_sch)
		return -1;

	dif_sch = cls_wifi_hw->plat->dif_sch;

	dif_drv_state  = dif_sch->dif_drv_state;
	switch(dif_cali_cmd){
	case DIF_EVT_CALI_PAUSE:
		if((dif_drv_state != DIF_SM_ONLINE)||(dif_fw_state != DIF_SM_ONLINE)){
			pr_err("%s pause fail, sm state: %s-%s\n", RADIO2STR(cls_wifi_hw->radio_idx),
				dif_sm2str(dif_drv_state), dif_sm2str(dif_fw_state));
			return -1;
		}
		break;
	case DIF_EVT_CALI_RESUME:
		if((dif_drv_state != DIF_EVT_CALI_PAUSE)||(dif_fw_state != DIF_EVT_CALI_PAUSE)){
			pr_err("%s resume fail, sm state: %s-%s\n", RADIO2STR(cls_wifi_hw->radio_idx),
				dif_sm2str(dif_drv_state), dif_sm2str(dif_fw_state));
			return -1;
		}
		break;
	case DIF_EVT_START_BOOT_CALI:
	case DIF_EVT_START_ONLINE_CALI:
	case DIF_EVT_CALI_RESET:
	case DIF_EVT_CALI_STOP:
	case DIF_EVT_START_PWR_CALI:
	case DIF_EVT_START_ZIF_CALI:
	case DIF_EVT_RELOAD_TABLE:
	case DIF_EVT_ACI_DET:
		break;
	default:
		pr_err("%s dif sm command not support: %d\n",RADIO2STR(cls_wifi_hw->radio_idx),dif_cali_cmd);
		return -1;
	}

	req.radio_id = cls_wifi_hw->radio_idx;
	req.evt_cmd = dif_cali_cmd;
	req.task_type = cls_wifi_hw->dif_sm.dif_online_dpd_task_type;
	ret = cls_wifi_send_irf_calib_evt_req(cls_wifi_hw,&req,&set_cali_cfm);
	if(!ret){
		cls_wifi_hw->dif_sm.dif_fw_state = set_cali_cfm.dif_sm_state;
		if(!set_cali_cfm.status){
			switch(dif_cali_cmd){
			case DIF_EVT_START_BOOT_CALI:
				if(cls_wifi_hw->dif_sm.pppc_status){
					/*cls_wifi_send_pppc_resume(cls_wifi_hw);*/
					cls_wifi_hw->dif_sm.pppc_status = 0;
				}
				dif_sch->dif_drv_state = DIF_SM_IDLE;
				if (!cls_wifi_is_repeater_mode(cls_wifi_hw))
					cls_wifi_hw->dif_sm.boot_cali_status = 1;
				pr_info("%s dif boot_cali ret:%d\n", RADIO2STR(req.radio_id), ret);
				break;
			case DIF_EVT_START_ZIF_CALI:
				dif_sch->dif_drv_state = DIF_SM_ZIF;
				break;
			case DIF_EVT_START_ONLINE_CALI:
			case DIF_EVT_CALI_RESUME:
				dif_sch->dif_drv_state = DIF_SM_ONLINE;
				break;
			case DIF_EVT_CALI_PAUSE:
				dif_sch->dif_drv_state = DIF_SM_PAUSE;
				break;
			case DIF_EVT_START_PWR_CALI:
				dif_sch->dif_drv_state = DIF_SM_PWR;
				break;
			case DIF_EVT_CALI_RESET:
			case DIF_EVT_CALI_STOP:
				dif_sch->dif_drv_state = DIF_SM_IDLE;
				break;
			case DIF_EVT_RELOAD_TABLE:
				dif_sch->dif_drv_state = DIF_SM_IDLE;
				dif_sch->reload_table_flag = 0;
				cls_wifi_dif_mutex_unlock(cls_wifi_hw->plat);
				break;
			default:
				break;
			}
		}else {
			pr_warn("%s fail: %s cmd[%d], ret: %d\n", __func__, RADIO2STR(req.radio_id),
				dif_cali_cmd, set_cali_cfm.status);
			if (dif_cali_cmd == DIF_EVT_START_BOOT_CALI)
				cls_wifi_hw->dif_sm.boot_cali_status = 0;
		}
	} else {
		if (dif_cali_cmd == DIF_EVT_START_ZIF_CALI)
			cls_wifi_dif_mutex_unlock(cls_wifi_hw->plat);

		dif_sch->dif_drv_state = DIF_SM_INVALID;
	}
	return ret;
}

void cls_wifi_dif_table_reload(struct cls_wifi_plat *cls_wifi_plat)
{
	u32 irf_tbl_addr;
	struct cls_wifi_plat *plat;

	plat = (struct cls_wifi_plat *)cls_wifi_plat->dif_sch->plat[RADIO_2P4G_INDEX];
	if (plat && is_band_enabled(plat, RADIO_2P4G_INDEX)) {
		irf_tbl_addr = plat->if_ops->get_phy_address(plat,
				RADIO_2P4G_INDEX, CLS_WIFI_ADDR_IRF_TBL_PHY, 0);
		irf_load_table(RADIO_2P4G_INDEX, irf_tbl_addr, plat, 0);
	}

	plat = (struct cls_wifi_plat *)cls_wifi_plat->dif_sch->plat[RADIO_5G_INDEX];
	if (plat && is_band_enabled(plat, RADIO_5G_INDEX)) {
		irf_tbl_addr = plat->if_ops->get_phy_address(plat,
				RADIO_5G_INDEX, CLS_WIFI_ADDR_IRF_TBL_PHY, 0);
		irf_load_table(RADIO_5G_INDEX, irf_tbl_addr, plat, 0);
	}

}
void cls_wifi_dif_sm_update(struct cls_wifi_hw *cls_wifi_hw, struct irf_set_cali_evt_ind *cali_evt)
{
	struct cls_wifi_dif_sch *dif_sch = cls_wifi_hw->plat->dif_sch;
	u32 zif_retry_time;

	switch(cali_evt->evt_cmd){
	case DIF_EVT_START_EQ_CALI:
	case DIF_EVT_START_DBG:
		if(CO_BUSY != cali_evt->status){
			cls_wifi_hw->dif_sm.dif_fw_state = DIF_SM_IDLE;
			dif_sch->dif_drv_state = DIF_SM_IDLE;
		}
		break;
	case DIF_EVT_SM_REPORT:
		cls_wifi_hw->dif_sm.dif_fw_state = cali_evt->dif_sm_state;
		if (cali_evt->cause != DIF_CALI_STOP_BY_DONE) {
			pr_warn("%s DIF cali finish, sm:%s-%s, cause: %s\n", RADIO2STR(cls_wifi_hw->radio_idx),
				dif_sm2str(dif_sch->dif_drv_state), dif_sm2str(cali_evt->dif_sm_state),
				dif_cause2str(cali_evt->cause));
			if ((dif_sch->function == DIF_SCH_PWR_CTRL) && (cali_evt->cause == DIF_CALI_STOP_BY_TABLE))
				cls_wifi_hw->dif_sm.pwr_ctrl_en = 0;
		}

		if (dif_sch->function == DIF_SCH_ZIF_CALI) {
			if (dif_sch->reload_table_flag)
				cls_wifi_dif_schedule(cls_wifi_hw->plat, dif_sch->radio,
					DIF_RELOAD_TABLE, DIF_EVT_RELOAD_TABLE);
			else
				dif_sch->dif_drv_state = DIF_SM_IDLE;

			dif_sch->zif_temp_record[dif_sch->radio] = dif_sch->tsensor_temp[dif_sch->radio];
			dif_sch->time[dif_sch->radio] = 0;

			if (cali_evt->cause == DIF_CALI_STOP_BY_THROUGHPUT) {
				pr_info("%s fw busy, online ZIF will try later.\n", RADIO2STR(dif_sch->radio));
				if (dif_sch->online_zif_cycle[dif_sch->radio] > DIF_SCH_RETRY_SEC) {
					zif_retry_time = dif_sch->online_zif_cycle[dif_sch->radio] - DIF_SCH_RETRY_SEC;
					dif_sch->time[dif_sch->radio] = DIF_SCH_SEC_2_TIME(zif_retry_time);
				}
			} else
				pr_info("%s ZIF online end.\n", RADIO2STR(dif_sch->radio));
		} else
			dif_sch->dif_drv_state = DIF_SM_IDLE;
		break;
	default:
		break;
	}

	return;
}

int cls_wifi_dif_sm_init(struct cls_wifi_plat *cls_wifi_plat,u8 radio_idx)
{
	struct cls_wifi_hw *cls_wifi_hw = cls_wifi_plat->cls_wifi_hw[radio_idx];

	if(radio_idx > RADIO_5G_INDEX){
		return 0;
	}

	cls_wifi_hw->dif_sm.dif_fw_state = DIF_SM_IDLE;
	cls_wifi_hw->dif_sm.online_zif_en = 1;
	cls_wifi_hw->dif_sm.pwr_ctrl_en = 1;
	cls_wifi_hw->dif_sm.pppc_status = 0;
	cls_wifi_hw->dif_sm.boot_cali_status = 0;
	cls_wifi_hw->dif_sm.boot_cali_enable = cls_wifi_hw->radio_params->boot_cali_enable;
	return 0;
}
EXPORT_SYMBOL(cls_wifi_dif_sm_init);

int cls_wifi_check_table(struct cls_wifi_hw *cls_wifi_hw,EN_IRF_DATA table_type)
{
	struct irf_data table_data = {0};
	uint32_t irf_mem_addr;

	if(table_type >= IRF_DATA_RESERVED){
		return -1;
	}

	if (cls_wifi_hw->plat && cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_DUBHE2000)
		irf_mem_addr =  IRF_SHARE_MEM_ADDR_D2K;
	else if (cls_wifi_hw->plat && cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK2000)
		irf_mem_addr =  IRF_SHARE_MEM_ADDR_M2K;
	else if (cls_wifi_hw->plat && cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK3000)
		irf_mem_addr =  IRF_SHARE_MEM_ADDR_M3K;

	if(NULL != cls_wifi_hw->plat->ep_ops->irf_table_readn){
		if(RADIO_2P4G_INDEX == cls_wifi_hw->radio_idx){
			cls_wifi_hw->plat->ep_ops->irf_table_readn(cls_wifi_hw->plat, cls_wifi_hw->radio_idx,
					irf_mem_addr + offsetof(struct irf_share_data,irf_data_2G[table_type]),
					&table_data,sizeof(struct irf_data));

		} else {
			cls_wifi_hw->plat->ep_ops->irf_table_readn(cls_wifi_hw->plat, cls_wifi_hw->radio_idx,
					irf_mem_addr + offsetof(struct irf_share_data,irf_data_5G[table_type]),
					&table_data,sizeof(struct irf_data));
		}
	} else {
		pr_err("irf_table_readn is NULL.\n");
		return -1;
	}
	pr_err("data addr=0x%x, size=%d, flag=%d.\n",table_data.data_addr,table_data.data_size,table_data.load_flag);

	if(table_data.load_flag != DATA_OK){
		return -1;
	}
	return 0;
}

void cls_wifi_dif_sm_pause(struct cls_wifi_hw *cls_wifi_hw)
{
	struct cls_wifi_plat *cls_wifi_plat = cls_wifi_hw->plat;

	cls_wifi_plat->dif_sch->dif_sm_pause_cnt++;
}

void cls_wifi_dif_sm_resume(struct cls_wifi_hw *cls_wifi_hw)
{
	struct cls_wifi_plat *cls_wifi_plat = cls_wifi_hw->plat;

	if (cls_wifi_plat->dif_sch->dif_sm_pause_cnt > 0)
		cls_wifi_plat->dif_sch->dif_sm_pause_cnt--;
}
#ifndef CFG_PCIE_SHM
#if defined(MERAK2000)
#define DATA_TYPE_FOR_TX_ZIF    0
#define CH_FOR_BOOT_CALI        1

static int cls_wifi_dif_send_cfr_data(struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_send_eq_data_req req;

	req.radio_id = cls_wifi_hw->radio_idx;
	req.channel = CH_FOR_BOOT_CALI;
	req.data_type = DATA_TYPE_FOR_TX_ZIF;
	req.stop = 0;

	return cls_wifi_send_irf_eq_data_req(cls_wifi_hw, &req);
}
#endif
#endif

void cls_wifi_dif_boot_cali_clear(struct cls_wifi_hw *cls_wifi_hw)
{
	cls_wifi_hw->dif_sm.boot_cali_status = 0;

	if (cls_wifi_hw->plat && cls_wifi_hw->plat->dif_sch) {
		cls_wifi_hw->plat->dif_sch->time[RADIO_2P4G_INDEX] = 0;
		cls_wifi_hw->plat->dif_sch->time[RADIO_5G_INDEX] = 0;
	}
}

int cls_wifi_dif_boot_cali(struct cls_wifi_hw *cls_wifi_hw)
{
	u8 radio = cls_wifi_hw->radio_idx;
	u8 other_radio;
	struct cls_wifi_hw *other_radio_hw;
	int ret;
	struct cls_wifi_dif_sch *dif_sch;
	struct cls_wifi_plat *other_plat;

	if (!cls_wifi_hw->plat || !cls_wifi_hw->plat->dif_sch)
		return -1;
	dif_sch = cls_wifi_hw->plat->dif_sch;

	if(cls_wifi_hw->radio_params->boot_cali_enable){

		if (cls_wifi_hw->plat->dif_sch->mode) {
			pr_err("%s EQ mode, skip DIF boot cali.\n", __func__);
			return -1;
		}

		if(!cls_wifi_hw->dif_sm.boot_cali_enable){
			pr_err("%s boot_cali_enable false, skip DIF boot cali.\n",__func__);
			return -1;
		}
/*
		if(cls_wifi_check_table(cls_wifi_hw, IRF_DATA_DIF_EQ)){
			pr_err("%s dif eq table not OK, skip DIF boot cali.\n",__func__);
			return -1;
		}
*/
		cls_wifi_dif_sm_pause(cls_wifi_hw);

		/* force other radio to idle status */
		if(RADIO_2P4G_INDEX == radio){
			other_radio = RADIO_5G_INDEX;
		} else {
			other_radio = RADIO_2P4G_INDEX;
		}

		cls_wifi_dif_sm_set_event(cls_wifi_hw, DIF_EVT_CALI_STOP);

		other_plat = (struct cls_wifi_plat *)dif_sch->plat[other_radio];
		if (other_plat)
			other_radio_hw = other_plat->cls_wifi_hw[other_radio];

		if (other_plat && is_band_enabled(other_plat, other_radio))
			cls_wifi_dif_sm_set_event(other_radio_hw, DIF_EVT_CALI_STOP);

#ifndef CFG_PCIE_SHM
		if (irf_load_boot_cali_data(cls_wifi_hw)) {
			cls_wifi_dif_sm_resume(cls_wifi_hw);
			return -1;
		}
#if defined(MERAK2000)
		if (!cls_wifi_hw->radio_params->debug_mode) {
			cls_wifi_dif_send_cfr_data(cls_wifi_hw);
			cls_wifi_dif_table_reload(cls_wifi_hw->plat);
		}
#endif
#endif

		if (cls_wifi_dif_get_pppc_switch(cls_wifi_hw)) {
			/*cls_wifi_send_pppc_pause(cls_wifi_hw,PPPC_INVALID_POWER);*/
			cls_wifi_hw->dif_sm.pppc_status = 1;
		}

		cls_wifi_hw->plat->dif_sch->time[radio] = 0;
		ret = cls_wifi_dif_sm_set_event(cls_wifi_hw,DIF_EVT_START_BOOT_CALI);
		cls_wifi_dif_sm_resume(cls_wifi_hw);
		return ret;
	}
	return 0;
}

int cls_wifi_dif_trigger_online_cali_once(struct cls_wifi_hw *cls_wifi_hw)
{
	/* debug mode not support online zif */
	return 0;
}

int cls_wifi_dif_trigger_online_dpd_task_once(struct cls_wifi_hw *cls_wifi_hw)
{
	if (!cls_wifi_hw->plat || !cls_wifi_hw->plat->dif_sch)
		return -1;

	cls_wifi_hw->plat->dif_sch->function = DIF_SCH_DPD_CALI;

	return cls_wifi_dif_sm_set_event(cls_wifi_hw,DIF_EVT_START_ONLINE_CALI);
}

static void cls_wifi_dif_schedule_work(struct work_struct *dif_work)
{
	struct cls_wifi_dif_sch *dif_sch = container_of(dif_work, struct cls_wifi_dif_sch, dif_work);
	u8 radio = dif_sch->radio;
	struct cls_wifi_plat *cls_wifi_plat = (struct cls_wifi_plat *)dif_sch->plat[radio];

	if (!test_bit(CLS_WIFI_DEV_STARTED, &cls_wifi_plat->cls_wifi_hw[radio]->flags))
		return;

	if (dif_sch->dif_evt_cmd == DIF_EVT_START_ZIF_CALI) {

		pr_info("%s ZIF online start, temperture = %d:%d, time = %d\n", RADIO2STR(radio),
			dif_sch->zif_temp_record[dif_sch->radio], dif_sch->tsensor_temp[dif_sch->radio],
			DIF_SCH_TIME_2_SEC(dif_sch->time[radio]));

		cls_wifi_dif_mutex_lock(cls_wifi_plat);
		if (irf_load_boot_cali_data(cls_wifi_plat->cls_wifi_hw[radio])) {
			cls_wifi_dif_mutex_unlock(cls_wifi_plat);
			return;
		}
		if (cls_wifi_plat->hw_rev == CLS_WIFI_HW_MERAK2000)
			dif_sch->reload_table_flag = 1;
	} else if (dif_sch->dif_evt_cmd == DIF_EVT_RELOAD_TABLE) {
		cls_wifi_dif_mutex_trylock(cls_wifi_plat);
		cls_wifi_dif_table_reload(cls_wifi_plat);
	} else if (dif_sch->dif_evt_cmd == DIF_EVT_DPD_FBDELAY_CALI) {
		cls_wifi_start_dpd_fbdelay_cali(cls_wifi_plat->cls_wifi_hw[radio]);
		return;
	} else if (dif_sch->dif_evt_cmd == DIF_EVT_DPD_CALI) {
		cls_wifi_start_dpd_cali(cls_wifi_plat->cls_wifi_hw[radio]);
		return;
	}

	cls_wifi_dif_sm_set_event(cls_wifi_plat->cls_wifi_hw[radio], dif_sch->dif_evt_cmd);
}

void cls_wifi_dif_schedule(struct cls_wifi_plat *cls_wifi_plat, u8 radio, u8 function, u8 cmd)
{
	cls_wifi_plat->dif_sch->radio = radio;
	cls_wifi_plat->dif_sch->function = function;
	cls_wifi_plat->dif_sch->dif_evt_cmd = cmd;

	schedule_work(&cls_wifi_plat->dif_sch->dif_work);
}

u8 cls_wifi_dif_get_pppc_switch(struct cls_wifi_hw *cls_wifi_hw)
{
	if(!cls_wifi_mod_params.debug_mode){
		return cls_wifi_hw->pppc_enabled;
	} else {
		//return g_cal_config[cls_wifi_hw->radio_idx].pppc_info.pppc_en;
		return 0;
	}
}

u8 cls_wifi_dif_next_radio(u8 cur_radio)
{
	if (cur_radio == RADIO_2P4G_INDEX)
		return RADIO_5G_INDEX;
	else
		return RADIO_2P4G_INDEX;
}

int cls_wifi_online_zif_trigger(struct cls_wifi_dif_sch *dif_sch, u8 radio)
{
	s8 cur_temp = dif_sch->tsensor_temp[radio];
	s8 last_temp = dif_sch->zif_temp_record[radio];
	u8 temp_dlt;
	u8 temp_thres;

	if (cur_temp >= DIF_ONLINE_ZIF_TEMP_H_L_BOUND)
		temp_thres = dif_sch->zif_trig_h_thres[radio];
	else
		temp_thres = dif_sch->zif_trig_l_thres[radio];

	temp_dlt = abs(last_temp - cur_temp);

	if ((DIF_SCH_TIME_2_SEC(dif_sch->time[radio]) >= dif_sch->online_zif_cycle[radio])
		|| ((last_temp != DIF_INVALID_TEMP) && (temp_dlt >= temp_thres)))
		return true;
	else
		return false;
}

int cls_wifi_online_dpd_trigger(struct cls_wifi_hw *cls_wifi_hw, struct cls_wifi_dif_sch *dif_sch, u8 radio)
{
	int ret = false;

	if (!dif_sch->pd_sch_times[radio]++)
		ret = true;

	if (dif_sch->pd_sch_times[radio] == cls_wifi_hw->dpd_timer_interval)
		dif_sch->pd_sch_times[radio] = 0;

	return ret;
}

int cls_wifi_online_close_loop_cali_trigger(struct cls_wifi_plat *plat, u8 radio)
{
	struct cls_wifi_close_loop_cali_sch *ctx = &plat->dif_sch->sch_close_loop_cali;
	u8 next_radio;

	if (ctx->cycle == 0)
		return true;
	if (radio != ctx->exp_radio)
		return false;
	ctx->cnt++;
	if (ctx->cnt < ctx->cycle)
		return false;
	next_radio = (ctx->exp_radio + 1) % MAX_RADIO_NUM;
	if (is_band_enabled(plat, next_radio))
		ctx->exp_radio = next_radio;
	ctx->cnt = 0;
	return true;
}

int cls_wifi_dif_sm_search_next(struct cls_wifi_plat *cls_wifi_plat, u8 *next_radio, u8 *next_function)
{
	struct cls_wifi_dif_sch *dif_sch = cls_wifi_plat->dif_sch;
	u8 radio = dif_sch->radio;
	u8 function = dif_sch->function;
	struct cls_wifi_plat *plat;
	struct cls_wifi_hw *cls_wifi_hw;
	int i;

	if (dif_sch->reload_table_flag) {
		*next_radio = radio;
		*next_function = DIF_RELOAD_TABLE;
		return 0;
	}

	/* 2G ZIF & RELOAD_TBL & PWR & DPD + 5G ZIF & RELOAD_TBL & PWR & DPD & ACI_DET */
	for (i = 0; i < (DIF_SCH_MAX * 2); i++) {

		function++;

		if (function >= DIF_SCH_MAX) {
			function = DIF_SCH_ZIF_CALI;
			radio++;
		}
		if (radio > RADIO_5G_INDEX) {
			function = DIF_SCH_ZIF_CALI;
			radio = RADIO_2P4G_INDEX;
		}

		plat = (struct cls_wifi_plat *)dif_sch->plat[radio];
		if (!plat || !is_band_enabled(plat, radio))
			continue;

		cls_wifi_hw = plat->cls_wifi_hw[radio];

		if (function == DIF_SCH_ZIF_CALI) {
			if (cls_wifi_hw->dif_sm.online_zif_en && dif_sch->g_online_zif_en
				&& cls_wifi_hw->dif_sm.boot_cali_status
				&& cls_wifi_mod_params.zif_online_en
				&& cls_wifi_online_zif_trigger(dif_sch, radio)) {
				*next_radio = radio;
				*next_function = function;
				return 0;
			}
		} else if (function == DIF_SCH_PWR_CTRL) {
			if (cls_wifi_hw->dif_sm.pwr_ctrl_en && dif_sch->g_pwr_ctrl_en) {
				if (cls_wifi_online_close_loop_cali_trigger(plat, radio)) {
					*next_radio = radio;
					*next_function = function;
					return 0;
				}
			}
		} else if (function == DIF_SCH_DPD_CALI) {
			if (cls_wifi_mod_params.dpd_online_en && cls_wifi_hw->dif_sm.boot_cali_status &&
				cls_wifi_hw->dpd_timer_enabled
				&& cls_wifi_online_dpd_trigger(cls_wifi_hw, dif_sch, radio)) {
				*next_radio = radio;
				*next_function = function;
				return 0;
			}
		} else if (function == DIF_SCH_ACI_DET) {
#if defined(MERAK2000)
			if (cls_wifi_mod_params.debug_mode && dif_sch->g_aci_det_en &&
					(DIF_SCH_TIME_2_SEC(dif_sch->time[radio]) >= dif_sch->dif_aci_det_cycle)) {
				*next_radio = radio;
				*next_function = function;

				return 0;
			}
#endif
		}
	}

	return -1;
}

void cls_wifi_dif_online_schedule(struct timer_list *timer)
{
	struct cls_wifi_dif_sch *dif_sch = container_of(timer, struct cls_wifi_dif_sch, sch_timer);
	u8 radio;
	u8 function;
	struct cls_wifi_hw *cls_wifi_hw;
	int dif_drv_state  = dif_sch->dif_drv_state;
	struct cls_wifi_dif_fw_sm *sm;
	struct cls_wifi_plat *plat;
	u32 multiple;

	for (radio = 0; radio < CLS_WIFI_MAX_RADIOS; radio++)
		dif_sch->time[radio]++;

	if ((dif_sch->mode) || (!dif_sch->g_online_zif_en && !dif_sch->g_pwr_ctrl_en &&
		!cls_wifi_mod_params.dpd_online_en)
		|| dif_sch->dif_sm_pause_cnt) {
		mod_timer(&dif_sch->sch_timer, jiffies + DIF_SCH_TIMER_INTERVAL_JIFFIES);
		return;
	}

	function = dif_sch->function;
	plat = (struct cls_wifi_plat *)dif_sch->plat[dif_sch->radio];

	/* if band disable, skip current radio */
	if ((plat == NULL) || !is_band_enabled(plat, dif_sch->radio)) {
		dif_sch->radio = cls_wifi_dif_next_radio(dif_sch->radio);
		plat = (struct cls_wifi_plat *)dif_sch->plat[dif_sch->radio];
		if ((plat == NULL) || !is_band_enabled(plat, dif_sch->radio)) {
			mod_timer(&dif_sch->sch_timer, jiffies + DIF_SCH_TIMER_INTERVAL_JIFFIES);
			return;
		}
	}

	cls_wifi_hw = plat->cls_wifi_hw[dif_sch->radio];
	sm = &cls_wifi_hw->dif_sm;
	if ((dif_drv_state == DIF_SM_IDLE)
		&& ((sm->dif_fw_state == DIF_SM_FINISH) || (sm->dif_fw_state == DIF_SM_IDLE))) {
		sm->dif_fw_state = DIF_SM_IDLE;
		dif_sch->sm_cnt = 0;
		if (cls_wifi_dif_sm_search_next(plat, &radio, &function)) {
			mod_timer(&dif_sch->sch_timer, jiffies + DIF_SCH_TIMER_INTERVAL_JIFFIES);
			return;
		}
		plat = (struct cls_wifi_plat *)dif_sch->plat[radio];
		if (function == DIF_SCH_ZIF_CALI)
			cls_wifi_dif_schedule(plat, radio, DIF_SCH_ZIF_CALI, DIF_EVT_START_ZIF_CALI);
		else if (function == DIF_RELOAD_TABLE)
			cls_wifi_dif_schedule(plat, radio, DIF_RELOAD_TABLE, DIF_EVT_RELOAD_TABLE);
		else if (function == DIF_SCH_PWR_CTRL)
			cls_wifi_dif_schedule(plat, radio, DIF_SCH_PWR_CTRL, DIF_EVT_START_PWR_CALI);
		else if (function == DIF_SCH_DPD_CALI)
#if defined(CFG_MERAK3000)
			cls_wifi_dif_schedule(plat, radio, DIF_SCH_DPD_CALI, DIF_EVT_START_ONLINE_CALI);
#else
			cls_wifi_dif_schedule(plat, radio, DIF_SCH_DPD_CALI, DIF_EVT_DPD_FBDELAY_CALI);
#endif
		else if (function == DIF_SCH_ACI_DET)
			cls_wifi_dif_schedule(plat, radio, DIF_SCH_ACI_DET, DIF_EVT_ACI_DET);
	} else {
		dif_sch->sm_cnt++;
		if (dif_drv_state == DIF_SM_INVALID)
			multiple = 10;
		else
			multiple = 1;

		if (DIF_SCH_TIME_2_SEC(dif_sch->sm_cnt) > (multiple * DIF_SCH_TIMEOUT_SEC)) {
			dif_sch->sm_cnt = 0;
			pr_warn("dif cali sm reset, radio%d state:%s-%s, function:%d\n", dif_sch->radio,
				dif_sm2str(sm->dif_fw_state), dif_sm2str(dif_drv_state), dif_sch->function);
			if (dif_sch->reload_table_flag)
				cls_wifi_dif_schedule(plat, dif_sch->radio,
					DIF_RELOAD_TABLE, DIF_EVT_RELOAD_TABLE);
			else
				cls_wifi_dif_schedule(plat, dif_sch->radio,
					DIF_SCH_ZIF_CALI, DIF_EVT_CALI_RESET);
		}
	}

	mod_timer(&dif_sch->sch_timer, jiffies + DIF_SCH_TIMER_INTERVAL_JIFFIES);
	return;
}

void cls_wifi_dif_mutex_lock(struct cls_wifi_plat *cls_wifi_plat)
{
#if defined(MERAK2000)
	if (!cls_wifi_plat->dif_sch)
		return;

	mutex_lock(&cls_wifi_plat->dif_sch->mutex);
	cls_wifi_plat->dif_sch->sch_is_lock = 1;
#endif
}

void cls_wifi_dif_mutex_trylock(struct cls_wifi_plat *cls_wifi_plat)
{
#if defined(MERAK2000)
	if (!cls_wifi_plat->dif_sch)
		return;

	if (mutex_trylock(&cls_wifi_plat->dif_sch->mutex))
		cls_wifi_plat->dif_sch->sch_is_lock = 1;
#endif
}

void cls_wifi_dif_mutex_unlock(struct cls_wifi_plat *cls_wifi_plat)
{
#if defined(MERAK2000)
	if (!cls_wifi_plat->dif_sch)
		return;

	if (cls_wifi_plat->dif_sch->sch_is_lock)
		mutex_unlock(&cls_wifi_plat->dif_sch->mutex);
#endif
}

int cls_wifi_dif_online_schedule_init(struct cls_wifi_plat *cls_wifi_plat)
{
	u8 radio;
	static struct cls_wifi_dif_sch *dif_sch = NULL;

	pr_warn("%s %d enabled %d\n", __func__, __LINE__, cls_wifi_plat->bands_enabled);
	if (!cls_wifi_plat->bands_enabled)
		return 0;

	if (dif_sch == NULL) {
		dif_sch = kzalloc(sizeof(struct cls_wifi_dif_sch), GFP_KERNEL);
		if (dif_sch == NULL) {
			pr_err("%s %d kzalloc fail\n", __func__, __LINE__);
			return -1;
		}
#if defined(MERAK2000)
		mutex_init(&dif_sch->mutex);
		dif_sch->sch_is_lock = 0;
		if (cls_wifi_mod_params.debug_mode) {
#if !defined(CFG_MERAK3000)
			dif_sch->g_aci_det_en = 1;
#else
			dif_sch->g_aci_det_en = 0;
#endif
			dif_sch->dif_aci_det_cycle = DIF_ACI_DET_CYCLE_SEC;
		}
#endif
		dif_sch->dif_drv_state = DIF_SM_IDLE;
		if (cls_wifi_plat->hw_rev == CLS_WIFI_HW_MERAK3000)
			dif_sch->g_pwr_ctrl_en = 0;
		else
			dif_sch->g_pwr_ctrl_en = 1;

		if (!cls_wifi_mod_params.debug_mode && cls_wifi_mod_params.zif_online_en)
			dif_sch->g_online_zif_en = 1;
		else
			dif_sch->g_online_zif_en = 0;

		dif_sch->mode = 0;
		dif_sch->sm_cnt = 0;
		dif_sch->radio = cls_wifi_plat->hw_params.radio_base;

		INIT_WORK(&dif_sch->dif_work, cls_wifi_dif_schedule_work);
	}

	for (radio = cls_wifi_plat->hw_params.radio_base; radio < cls_wifi_plat->hw_params.radio_max; radio++) {
		if (is_band_enabled(cls_wifi_plat, radio)) {
			cls_wifi_plat->cls_wifi_hw[radio]->dif_sm.fbdelay_success_flag = 0;
			cls_wifi_plat->cls_wifi_hw[radio]->dif_sm.pd_task_success_flag = 0;
			cls_wifi_plat->cls_wifi_hw[radio]->dif_sm.fbdelay_task_times = 0;
			dif_sch->time[radio] = 0;
			dif_sch->online_zif_cycle[radio] = DIF_ONLINE_ZIF_CYCLE_SEC;
			dif_sch->plat[radio] = cls_wifi_plat;
			dif_sch->tsensor_temp[radio] = DIF_INVALID_TEMP;
			dif_sch->zif_temp_record[radio] = DIF_INVALID_TEMP;
			dif_sch->zif_trig_h_thres[radio] = DIF_ONLINE_ZIF_TEMP_TRIG_HTHRES;
			dif_sch->zif_trig_l_thres[radio] = DIF_ONLINE_ZIF_TEMP_TRIG_LTHRES;
			dif_sch->pd_sch_times[radio] = 0;
#if defined(CFG_MERAK3000)
			dif_sch->sch_close_loop_cali.cycle = 2;
#else
			dif_sch->sch_close_loop_cali.cycle = 0;
#endif
			dif_sch->sch_close_loop_cali.cnt = 0;
			dif_sch->sch_close_loop_cali.exp_radio = radio;
		}
	}

	cls_wifi_plat->dif_sch = dif_sch;
	del_timer_sync(&dif_sch->sch_timer);
	timer_setup(&dif_sch->sch_timer, cls_wifi_dif_online_schedule, 0);
	mod_timer(&dif_sch->sch_timer, jiffies + DIF_SCH_TIMER_INTERVAL_JIFFIES);

	return 0;
}
EXPORT_SYMBOL(cls_wifi_dif_online_schedule_init);

void cls_wifi_dif_online_schedule_deinit(struct cls_wifi_plat *cls_wifi_plat)
{
	if (cls_wifi_plat->dif_sch == NULL)
		return;

	del_timer_sync(&cls_wifi_plat->dif_sch->sch_timer);
	cancel_work_sync(&cls_wifi_plat->dif_sch->dif_work);
}
EXPORT_SYMBOL(cls_wifi_dif_online_schedule_deinit);

int cls_wifi_get_temp_dubhe2000(int *tsensor_data)
{
	int (*extern_cls_tsens_get_value)(int *data);

	extern_cls_tsens_get_value = symbol_get(cls_tsens_get_value);
	if (extern_cls_tsens_get_value) {
		extern_cls_tsens_get_value(tsensor_data);
		symbol_put(cls_tsens_get_value);
		return 0;
	}
	return -1;
}

int cls_wifi_get_temp_merak2000(struct cls_wifi_plat *cls_wifi_plat, int *tsensor_data)
{
	if (cls_wifi_plat->ep_ops->tsensor_get != NULL) {
		tsensor_data[0] = cls_wifi_plat->ep_ops->tsensor_get(cls_wifi_plat);
		tsensor_data[1] = tsensor_data[0];
		tsensor_data[2] = tsensor_data[0];
		return 0;
	}
	return -1;
}


static void cls_wifi_tsensor_work(struct work_struct *tsensor_work)
{
	struct cls_wifi_dif_sch *dif_sch = container_of(tsensor_work, struct cls_wifi_dif_sch, tsensor_work);
	struct cls_wifi_plat *plat;
	int tsensor_data[TS_NUM];
	s8 temperature[TS_NUM];
	u32 irf_mem_addr;
	u8 radio;
	int ret;
	int temp;

	plat = (struct cls_wifi_plat *)dif_sch->plat[dif_sch->radio];
	if (plat == NULL)
		return;

	if (plat->hw_rev == CLS_WIFI_HW_DUBHE2000) {
		irf_mem_addr =  IRF_SHARE_MEM_ADDR_D2K;
		ret = cls_wifi_get_temp_dubhe2000(tsensor_data);
	} else {
		irf_mem_addr =  IRF_SHARE_MEM_ADDR_M2K;
		ret = cls_wifi_get_temp_merak2000(plat, tsensor_data);
	}

	if (!ret && plat->ep_ops->irf_table_writen) {
		temp = tsensor_data[0] / 1000;
		temp = min(temp, 127);
		temp = max(temp, -128);
		temperature[0] = (int8_t)(temp);

		temp = tsensor_data[1] / 1000;
		temp = min(temp, 127);
		temp = max(temp, -128);
		temperature[1] = (int8_t)(temp);

		temp = tsensor_data[2] / 1000;
		temp = min(temp, 127);
		temp = max(temp, -128);
		temperature[2] = (int8_t)(temp);
		dif_sch->tsensor_temp[RADIO_2P4G_INDEX] = temperature[RADIO_2P4G_INDEX];
		dif_sch->tsensor_temp[RADIO_5G_INDEX] = temperature[RADIO_5G_INDEX];

		plat->ep_ops->irf_table_writen(plat, 0,
				irf_mem_addr + offsetof(struct irf_share_data, temperature),
				temperature, sizeof(int8_t)*TS_NUM);

		for (radio = 0; radio < CLS_WIFI_MAX_RADIOS; radio++) {
			/* use the temperature 5s after boot cali as first zif_temp_record */
			plat = (struct cls_wifi_plat *)dif_sch->plat[radio];
			if (plat && (is_band_enabled(plat, radio))
					&& (dif_sch->zif_temp_record[radio] == DIF_INVALID_TEMP)
					&& plat->cls_wifi_hw[radio]->dif_sm.boot_cali_status
					&& (DIF_SCH_TIME_2_SEC(dif_sch->time[radio]) > 5))
				dif_sch->zif_temp_record[radio] = dif_sch->tsensor_temp[radio];
		}
	}
}

void cls_wifi_tsensor_timer_cb(struct timer_list *timer)
{
	struct cls_wifi_dif_sch *dif_sch = container_of(timer, struct cls_wifi_dif_sch, tsensor_timer);

	schedule_work(&dif_sch->tsensor_work);

	mod_timer(&dif_sch->tsensor_timer, jiffies + TSENSOR_TIMER_INTERVAL_JIFFIES);
}

int cls_wifi_tsensor_timer_init(struct cls_wifi_plat *cls_wifi_plat)
{
	uint32_t irf_mem_addr;
	int8_t tcomp_en = 0;

	if (cls_wifi_plat->dif_sch == NULL)
		return -1;

	if (cls_wifi_plat->hw_rev == CLS_WIFI_HW_DUBHE2000)
		irf_mem_addr =  IRF_SHARE_MEM_ADDR_D2K;
	else
		irf_mem_addr =  IRF_SHARE_MEM_ADDR_M2K;

	INIT_WORK(&cls_wifi_plat->dif_sch->tsensor_work, cls_wifi_tsensor_work);

	if (cls_wifi_mod_params.gain_tcomp_en) {
		del_timer_sync(&cls_wifi_plat->dif_sch->tsensor_timer);
		timer_setup(&cls_wifi_plat->dif_sch->tsensor_timer, cls_wifi_tsensor_timer_cb, 0);
		mod_timer(&cls_wifi_plat->dif_sch->tsensor_timer, jiffies + TSENSOR_TIMER_INTERVAL_JIFFIES);
		tcomp_en = 1;
	}

	if (cls_wifi_plat->ep_ops->irf_table_writen)
		cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, 0,
				irf_mem_addr + offsetof(struct irf_share_data, tcomp_en),
				&tcomp_en, sizeof(int8_t));

	return 0;
}
EXPORT_SYMBOL(cls_wifi_tsensor_timer_init);

void cls_wifi_tsensor_timer_deinit(struct cls_wifi_plat *cls_wifi_plat)
{
	if (cls_wifi_plat->dif_sch == NULL)
		return;

	cancel_work_sync(&cls_wifi_plat->dif_sch->tsensor_work);
	if (cls_wifi_mod_params.gain_tcomp_en)
		del_timer_sync(&cls_wifi_plat->dif_sch->tsensor_timer);
}
EXPORT_SYMBOL(cls_wifi_tsensor_timer_deinit);
