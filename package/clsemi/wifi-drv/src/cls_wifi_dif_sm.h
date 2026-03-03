#ifndef __CLS_WIFI_DIF_SM_H__
#define __CLS_WIFI_DIF_SM_H__
#include <linux/timer.h>
#include <linux/jiffies.h>

#include "cls_wifi_debugfs.h"
#include "lmac_msg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#if defined(CFG_MERAK3000)
#define DIF_SCH_TIMER_INTERVAL_JIFFIES			(HZ/4)
#define DIF_SCH_TIME_2_SEC(TIME)				(TIME/4)
#define DIF_SCH_SEC_2_TIME(SEC)					(SEC*4)
#else
#define DIF_SCH_TIMER_INTERVAL_JIFFIES			(HZ/2)
#define DIF_SCH_TIME_2_SEC(TIME)				(TIME/2)
#define DIF_SCH_SEC_2_TIME(SEC)					(SEC*2)
#endif

#if defined(CFG_M3K_FPGA)
#define DIF_SCH_TIMEOUT_SEC						(5*160)
#else
#define DIF_SCH_TIMEOUT_SEC						5
#endif

#define DIF_SCH_RETRY_SEC						60

#if defined(CFG_MERAK3000)
#define DIF_ONLINE_ZIF_CYCLE_SEC				(30*60)
#else
#define DIF_ONLINE_ZIF_CYCLE_SEC				(30*24*60*60)
#endif
#define PPPC_INVALID_POWER						0x7F
#define DIF_ONLINE_ZIF_TEMP_H_L_BOUND           50
#define DIF_ONLINE_ZIF_TEMP_TRIG_HTHRES         20
#define DIF_ONLINE_ZIF_TEMP_TRIG_LTHRES         10

#define DIF_INVALID_TEMP						(-128)

/* */
#define TSENSOR_TIMER_INTERVAL_JIFFIES			(HZ*2)
#define DIF_ACI_DET_CYCLE_SEC				(2)

enum dif_cali_evt_cmd{
	DIF_EVT_START_BOOT_CALI = 0,
	DIF_EVT_START_ONLINE_CALI,
	DIF_EVT_CALI_PAUSE,
	DIF_EVT_CALI_RESUME,
	DIF_EVT_CALI_RESET,
	DIF_EVT_CALI_STOP = 5,
	DIF_EVT_START_DBG,
	DIF_EVT_START_EQ_CALI,
	DIF_EVT_START_PWR_CALI,
	DIF_EVT_START_ZIF_CALI,
	DIF_EVT_RELOAD_TABLE,
	DIF_EVT_DPD_FBDELAY_CALI,
	DIF_EVT_DPD_CALI,
	DIF_EVT_ACI_DET,
	DIF_EVT_SM_REPORT,
	DIF_EVT_MAX
};

enum dif_sm_state{
	DIF_SM_IDLE = 0,
	DIF_SM_FINISH,
	DIF_SM_ONLINE,
	DIF_SM_PAUSE,
	DIF_SM_DBG,
	DIF_SM_EQ = 5,
	DIF_SM_PWR,
	DIF_SM_ZIF,
	DIF_SM_ACI_DET,
	DIF_SM_INVALID
};

enum dif_cali_stop_cause{
	DIF_CALI_STOP_BY_DONE = 0,
	DIF_CALI_STOP_BY_MODE,
	DIF_CALI_STOP_BY_THROUGHPUT,
	DIF_CALI_STOP_BY_UPDATE,
	DIF_CALI_STOP_BY_CONFIG,
	DIF_CALI_STOP_BY_TABLE = 5,
	DIF_CALI_STOP_BY_ALARM,
	DIF_CALI_STOP_BY_TIME_OUT,
	DIF_CALI_STOP_CAUSE_MAX
};


enum dif_sta_ampdus{
	AMPDUS_STA_INIT=0,
	AMPDUS_STA_LT_START_TH,
	AMPDUS_STA_HT_STOP_TH
};

enum dif_sch_function{
	DIF_SCH_ZIF_CALI,
	DIF_RELOAD_TABLE,
	DIF_SCH_PWR_CTRL,
	DIF_SCH_DPD_CALI,
	DIF_SCH_ACI_DET,
	DIF_SCH_MAX
};

struct cls_wifi_dif_fw_sm{
	bool fbdelay_success_flag;
	bool pd_task_success_flag;
	u8 fbdelay_task_times;
	u8 dif_online_dpd_task_type;
	u8 dif_fw_state;
	u8 online_zif_en;
	u8 pwr_ctrl_en;
	u8 pppc_status;
	u8 boot_cali_status;
	u8 boot_cali_enable;
};

struct dif_cali_evt_str{
	char *evt_str;
	u8 evt_cmd;
};

int cls_wifi_dif_sm_init(struct cls_wifi_plat *cls_wifi_plat,u8 radio_idx);
int cls_wifi_dif_sm_set_event(struct cls_wifi_hw *cls_wifi_hw, enum dif_cali_evt_cmd dif_cali_cmd);
void cls_wifi_dif_sm_pause(struct cls_wifi_hw *cls_wifi_hw);
void cls_wifi_dif_sm_resume(struct cls_wifi_hw *cls_wifi_hw);
void cls_wifi_dif_sm_update(struct cls_wifi_hw *cls_wifi_hw, struct irf_set_cali_evt_ind *cali_evt);
int cls_wifi_dif_boot_cali(struct cls_wifi_hw *cls_wifi_hw);
int cls_wifi_dif_trigger_online_cali_once(struct cls_wifi_hw *cls_wifi_hw);
int cls_wifi_dif_trigger_online_dpd_task_once(struct cls_wifi_hw *cls_wifi_hw);
int cls_wifi_dif_online_schedule_init(struct cls_wifi_plat *cls_wifi_plat);
void cls_wifi_dif_online_schedule_deinit(struct cls_wifi_plat *cls_wifi_plat);
u8 cls_wifi_dif_get_pppc_switch(struct cls_wifi_hw *cls_wifi_hw);
int dif_evt_str2cmd(char *cmd_str, enum dif_cali_evt_cmd *cmd);
int cls_wifi_tsensor_timer_init(struct cls_wifi_plat *cls_wifi_plat);
u8 cls_wifi_dif_next_radio(u8 cur_radio);
void cls_wifi_tsensor_timer_deinit(struct cls_wifi_plat *cls_wifi_plat);
void cls_wifi_dif_table_reload(struct cls_wifi_plat *cls_wifi_plat);
void cls_wifi_dif_boot_cali_clear(struct cls_wifi_hw *cls_wifi_hw);
void cls_wifi_dif_mutex_lock(struct cls_wifi_plat *cls_wifi_plat);
void cls_wifi_dif_mutex_trylock(struct cls_wifi_plat *cls_wifi_plat);
void cls_wifi_dif_mutex_unlock(struct cls_wifi_plat *cls_wifi_plat);
void cls_wifi_dif_schedule(struct cls_wifi_plat *cls_wifi_plat, u8 radio, u8 function, u8 cmd);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __CLS_WIFI_DIF_SM_H__ */
