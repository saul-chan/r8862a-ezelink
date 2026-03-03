#include <linux/kernel.h>
#include <linux/kmod.h>
#include <linux/debugfs.h>
#include <linux/string.h>
#include <net/netlink.h>
#include <net/mac80211.h>
#include <net/cfg80211.h>
#include "cls_wifi_cali_debugfs.h"
#include "cls_wifi_msg_tx.h"
#include "cls_wifi_cfgfile.h"
#include "cls_wifi_irf.h"
#include "cls_wifi_afe.h"
#include "cls_wifi_dif_sm.h"
#include "cls_wifi_main.h"
#include "vendor.h"

#define EQ_DATA_TX          0
#define EQ_DATA_RX          1
#define EQ_DATA_TX_RX       2

#define TEMP_CTRIM_TABLE_DELTA 25
#define IRF_TABLE_LIST_NUM	14
#define IRF_RX_DCOC_TABLE_LIST_NUM	2

uint8_t dcoc_status[2] = {0};
uint8_t dif_cali_status = 0;
uint8_t fb_err_cali_status = 0;
uint8_t rx_cali_status = 0;
uint8_t tx_err_cali_status = 0;
u32 *cfr_data_buf = NULL;

static struct irf_ram_block smp_send_ram[][IRF_MAX_NODE] =
{
	{
		{IRF_RAM_BASE_ADDR_D2K,                              IRF_RAM_BLOCK_SIZE_D2K, IRF_RAM_IDLE, IRF_DEF_SND_SMP_MOD},
		{IRF_RAM_BASE_ADDR_D2K + IRF_RAM_BLOCK_SIZE_D2K,     IRF_RAM_BLOCK_SIZE_D2K, IRF_RAM_IDLE, IRF_DEF_SND_SMP_MOD},
		{IRF_RAM_BASE_ADDR_D2K + IRF_RAM_BLOCK_SIZE_D2K * 2, IRF_RAM_BLOCK_SIZE_D2K, IRF_RAM_IDLE, IRF_DEF_SND_SMP_MOD},
		{IRF_RAM_BASE_ADDR_D2K + IRF_RAM_BLOCK_SIZE_D2K * 3, IRF_RAM_BLOCK_SIZE_D2K, IRF_RAM_IDLE, IRF_DEF_SND_SMP_MOD}
	},
	{
		{IRF_RAM_BASE_ADDR_M2K,                              IRF_RAM_BLOCK_SIZE_M2K, IRF_RAM_IDLE, IRF_DEF_SND_SMP_MOD},
		{IRF_RAM_BASE_ADDR_M2K + IRF_RAM_BLOCK_SIZE_M2K,     IRF_RAM_BLOCK_SIZE_M2K, IRF_RAM_IDLE, IRF_DEF_SND_SMP_MOD},
		{IRF_RAM_BASE_ADDR_M2K + IRF_RAM_BLOCK_SIZE_M2K * 2, IRF_RAM_BLOCK_SIZE_M2K, IRF_RAM_IDLE, IRF_DEF_SND_SMP_MOD},
		{IRF_RAM_BASE_ADDR_M2K + IRF_RAM_BLOCK_SIZE_M2K * 3, IRF_RAM_BLOCK_SIZE_M2K, IRF_RAM_IDLE, IRF_DEF_SND_SMP_MOD}
	},
	{
		{IRF_RAM_BASE_ADDR_M3K,                              IRF_RAM_BLOCK_SIZE_M3K,
			IRF_RAM_IDLE, IRF_DEF_SND_SMP_MOD},
		{IRF_RAM_BASE_ADDR_M3K + IRF_RAM_BLOCK_SIZE_M3K,     IRF_RAM_BLOCK_SIZE_M3K,
			IRF_RAM_IDLE, IRF_DEF_SND_SMP_MOD},
		{IRF_RAM_BASE_ADDR_M3K + IRF_RAM_BLOCK_SIZE_M3K * 2, IRF_RAM_BLOCK_SIZE_M3K,
			IRF_RAM_IDLE, IRF_DEF_SND_SMP_MOD},
		{IRF_RAM_BASE_ADDR_M3K + IRF_RAM_BLOCK_SIZE_M3K * 3, IRF_RAM_BLOCK_SIZE_M3K,
			IRF_RAM_IDLE, IRF_DEF_SND_SMP_MOD}
	},
};

static struct irf_node_ram_map node_ram_map[IRF_MAX_NODE] =
{
	{0, IRF_DEF_SND_SMP_MOD},
	{0, IRF_DEF_SND_SMP_MOD},
	{0, IRF_DEF_SND_SMP_MOD},
	{0, IRF_DEF_SND_SMP_MOD}
};


#define RADAR_DETECT_CFG		0x40140000
#define RADAR_DETECT_RESULT		0x40144000
#define FFT_SMTH_MEM			0x40609000
#define FFT_SMTH_MEM_LEN		8192

static uint32_t radar_det_buf[256] = {0};
static uint32_t inf_det_buf[FFT_SMTH_MEM_LEN / 4] = {0};

extern int irf_help(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw);


int irf_spilt_param(char *cmd_str, char *argv[CAL_CMD_MAX_PARAM])
{
	char *ppos = cmd_str;
	char *pend;
	int argc = 0;

	pend = cmd_str + strlen(cmd_str);

	while(ppos < pend){
		while((*ppos == ' ')&&(ppos < pend))
			ppos++;

		if(ppos == pend)
			break;

		argv[argc++] = ppos;

		if(argc >= CAL_CMD_MAX_PARAM)
			break;

		while((*ppos != ' ')&&(ppos < pend))
			ppos++;

		if(*ppos == ' ')
			*ppos++ = '\0';
	}

	return argc;
}

int irf_str2val(char *str)
{
	if(('0' == *str)&&(('x' == *(str+1))||('X' == *(str+1)))){
		return simple_strtoul(str,NULL,16);
	}

	return simple_strtol(str,NULL,10);
}

int irf_hw_cfg(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_hw_cfg_req req;

	if(argc<6){
		pr_err("parameter error, format: hwcfg bw ant_mask freq_MHz tx_power mode_11b\n");
		return -1;
	}
	req.bw = irf_str2val(argv[1]);
	req.ant_mask = irf_str2val(argv[2]);
	req.prim20_freq = irf_str2val(argv[3]);
	req.tx_power_level = irf_str2val(argv[4]);
	req.mode_11b = irf_str2val(argv[5]);

	return cls_wifi_send_irf_hw_cfg_req(cls_wifi_hw,&req);
}



int irf_set_mode(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_set_mode_req req;
	int ret;

	if(argc<2){
		pr_err("parameter error, format: mode 0/1\n");
		return -1;
	}
	req.mode = irf_str2val(argv[1]);
	ret = cls_wifi_send_irf_set_mode_req(cls_wifi_hw,&req);

	if(ret != 0)
		return ret;

	if(req.mode){
		cls_wifi_hw->plat->dif_sch->mode = 1;
	} else {
		cls_wifi_hw->plat->dif_sch->mode = 0;
	}

	return ret;
}

int irf_show_table(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_show_tbl_req req;

	if(argc<6){
		pr_err("parameter error, format: table type ANT0/1 freq BW stage\n");
		pr_err("type: 0#dif, 1#tx-level, 2#rx-level...4#freq-plan, 5#tx-fcomp...8#tx-tcomp...\n");
		return -1;
	}

	req.radio_id = cls_wifi_hw->radio_idx;
	req.table_type = irf_str2val(argv[1]);
	req.channel = irf_str2val(argv[2]);
	req.freq = irf_str2val(argv[3]);
	req.bw = irf_str2val(argv[4]);
	req.power_stage = irf_str2val(argv[5]);

	return cls_wifi_send_irf_show_tbl_req(cls_wifi_hw,&req);
}


int irf_show_status(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_show_req req;

	if(argc<4){
		pr_err("parameter error, format: show ANT0/1 type param\n");
		pr_err("type: 0-pwr, 1-alarm, 2-set irf-log switch, 3-rx power, 4-fem switch\n");
		return -1;
	}
	req.radio_id = cls_wifi_hw->radio_idx;
	req.channel = irf_str2val(argv[1]);
	req.type = irf_str2val(argv[2]);
	req.param = irf_str2val(argv[3]);

	return cls_wifi_send_irf_show_status_req(cls_wifi_hw,&req);
}



int irf_run_task(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_run_task_req req;
	u8 dif_drv_state  = cls_wifi_hw->plat->dif_sch->dif_drv_state;
	u8 dif_fw_state = cls_wifi_hw->dif_sm.dif_fw_state;
	if(argc<4){
		pr_err("parameter error, format: task ANT0/1 times task1,task2\n");
		return -1;
	}

	if((dif_drv_state != DIF_SM_IDLE)|| ((dif_fw_state != DIF_SM_IDLE)&&(dif_fw_state != DIF_SM_FINISH))){
		pr_err("%s drv/fw SM(%d-%d) not idle\n",RADIO2STR(cls_wifi_hw->radio_idx),
			dif_drv_state,cls_wifi_hw->dif_sm.dif_fw_state);
		return -1;
	}

	req.radio_id = cls_wifi_hw->radio_idx;
	req.channel = irf_str2val(argv[1]);
	req.run_time = irf_str2val(argv[2]);
	snprintf(req.task_list,IRF_TASK_LEN_MAX,"%s",argv[3]);

	cls_wifi_hw->plat->dif_sch->dif_drv_state = DIF_SM_DBG;
	cls_wifi_hw->dif_sm.dif_fw_state = DIF_SM_DBG;

	return  cls_wifi_send_irf_run_task_req(cls_wifi_hw,&req);
}

int irf_dif_eq(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_dif_eq_req req;
	u8 dif_drv_state  = cls_wifi_hw->plat->dif_sch->dif_drv_state;
	u8 dif_fw_state = cls_wifi_hw->dif_sm.dif_fw_state;

	if(argc<5){
		pr_err("parameter error, format: dif_equip ANT0/1 freq BW stage cali_type\n");
		pr_err("cali_type: 1-tx_fb, 2-tx_rx, 3-all(tx_fb+tx_rx)\n");
		return -1;
	}

	if((dif_drv_state != DIF_SM_IDLE)|| ((dif_fw_state != DIF_SM_IDLE)&&(dif_fw_state != DIF_SM_FINISH))){
		pr_err("%s drv/fw SM(%d-%d) not idle\n",RADIO2STR(cls_wifi_hw->radio_idx),
			dif_drv_state,cls_wifi_hw->dif_sm.dif_fw_state);
		return -1;
	}

	req.radio_id = cls_wifi_hw->radio_idx;
	req.channel = irf_str2val(argv[1]);
	req.freq = irf_str2val(argv[2]);
	req.bw = irf_str2val(argv[3]);
	req.stage = irf_str2val(argv[4]);
	if(argc == 5){
		req.cali_mask = 0x3;
	} else {
		req.cali_mask = irf_str2val(argv[5]);
	}

	cls_wifi_hw->plat->dif_sch->dif_drv_state = DIF_SM_EQ;
	cls_wifi_hw->dif_sm.dif_fw_state = DIF_SM_EQ;
	dif_cali_status = IRF_CALI_STATUS_BUSY;

	return cls_wifi_send_irf_dif_eq_req(cls_wifi_hw,&req);
}

int irf_dif_eq_status(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	switch(dif_cali_status){
	case IRF_CALI_STATUS_BUSY:
		pr_err("DIF_EQ: Busy.\n");
		break;
	case IRF_CALI_STATUS_DONE:
		pr_err("DIF_EQ: Done.\n");
		break;
	case IRF_CALI_STATUS_FAIL:
		pr_err("DIF_EQ: Fail.\n");
		break;
	default:
		pr_err("DIF_EQ: Idle.\n");
		break;
	}
	return 0;
}

int irf_dif_eq_save(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_dif_eq_save_req req;

	req.radio_id = cls_wifi_hw->radio_idx;

	return cls_wifi_send_irf_dif_eq_save_req(cls_wifi_hw,&req);

}

int irf_show_chip_id(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	uint32_t *chip_id;

	if (cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK2000 && cls_wifi_hw->plat->chip_id) {
		chip_id = cls_wifi_hw->plat->chip_id;
		pr_err("chip_id: %X-%X-%X-%X-%X\n", chip_id[0], chip_id[1], chip_id[2], chip_id[3], chip_id[4]);
	}
	return 0;
}

bool irf_check_chip_id(struct cls_wifi_plat *cls_wifi_plat)
{
	uint32_t id[5] = {0};
	char full_name[FS_PATH_MAX_LEN];

	if (cls_wifi_plat->hw_rev != CLS_WIFI_HW_MERAK2000 || !cls_wifi_plat->chip_id)
		return true;

	if (!cls_wifi_plat->path_info.cal_path)
		return true;

	strcpy(full_name, cls_wifi_plat->path_info.cal_path);
	strcat(full_name, CAL_INFO);
	pr_err("Get chip id from file %s.\n", full_name);
	if (!cls_wifi_get_cal_chip_id(full_name, id)) {
		if (memcmp(id, cls_wifi_plat->chip_id, sizeof(id))) {
			pr_err("chip_id not match: %X-%X-%X-%X-%X\n", id[0], id[1], id[2], id[3], id[4]);
			return false;
		}
	}

	return true;
}

void irf_write_table_info(struct cls_wifi_plat *plat, uint32_t radio_id, uint32_t irf_mem_addr,
		struct irf_data *radio_data, uint32_t data_type)
{
	if (radio_id == RADIO_2P4G_INDEX)
		plat->ep_ops->irf_table_writen(plat, radio_id,
				irf_mem_addr + offsetof(struct irf_share_data,
				irf_data_2G[data_type]),
				radio_data, sizeof(struct irf_data));
	else
		plat->ep_ops->irf_table_writen(plat, radio_id,
				irf_mem_addr + offsetof(struct irf_share_data,
				irf_data_5G[data_type]),
				radio_data, sizeof(struct irf_data));
}


struct irf_file_list equipment_data_list[] =
{
	{TX_ZIF_DATA, IRF_DATA_TX_ZIF, TX_ZIF_ADDR, TX_ZIF_SIZE},
#if defined(CFG_MERAK3000)
	{TX_ZIF_1_DATA, IRF_DATA_TX_1_ZIF, TX_ZIF_1_ADDR, TX_ZIF_1_SIZE},
#endif
};

struct irf_file_list table_2g_list[][IRF_TABLE_LIST_NUM] =
{
	{
		{DIF_EQ_2G_DATA, IRF_DATA_DIF_EQ, DIF_EQ_2G_ADDR_D2K, DIF_EQ_2G_SIZE_D2K},
		{TX_LEVEL_2G_DATA, IRF_DATA_TX_LEVEL, TX_LEVEL_2G_ADDR_D2K, TX_LEVEL_2G_SIZE_D2K},
		{FB_LEVEL_2G_DATA, IRF_DATA_FB_LEVEL, FB_LEVEL_2G_ADDR_D2K, FB_LEVEL_2G_SIZE_D2K},
		/*
		{RX_LEVEL_2G_DATA, IRF_DATA_RX_LEVEL, RX_LEVEL_2G_ADDR_D2K, RX_LEVEL_2G_SIZE_D2K},
		*/
		{TX_FCOMP_2G_DATA, IRF_DATA_TX_FCOMP, TX_FCOMP_2G_ADDR_D2K, TX_FCOMP_2G_SIZE_D2K},
		{FB_FCOMP_2G_DATA, IRF_DATA_FB_FCOMP, FB_FCOMP_2G_ADDR_D2K, FB_FCOMP_2G_SIZE_D2K},
		{RX_FCOMP_2G_DATA, IRF_DATA_RX_FCOMP, RX_FCOMP_2G_ADDR_D2K, RX_FCOMP_2G_SIZE_D2K},
		{TX_TCOMP_2G_DATA, IRF_DATA_TX_TCOMP, TX_TCOMP_2G_ADDR_D2K, TX_TCOMP_2G_SIZE_D2K},
		{FB_TCOMP_2G_DATA, IRF_DATA_FB_TCOMP, FB_TCOMP_2G_ADDR_D2K, FB_TCOMP_2G_SIZE_D2K},
		{RX_TCOMP_2G_DATA, IRF_DATA_RX_TCOMP, RX_TCOMP_2G_ADDR_D2K, RX_TCOMP_2G_SIZE_D2K},
		{PLL_PLAN_2G_DATA, IRF_DATA_PLL_PLAN, PLL_PLAN_2G_ADDR_D2K, PLL_PLAN_2G_SIZE_D2K},
		/*
		{RX_DC_OFFSET_2G_LOW_DATA, IRF_DATA_RX_DC_OFFSET_2G_LOW,
			RX_DC_OFFSET_2G_LOW_ADDR_D2K, RX_DC_OFFSET_2G_LOW_SIZE_D2K},
		{RX_DC_OFFSET_2G_HIGH_DATA, IRF_DATA_RX_DC_OFFSET_2G_HIGH,
			RX_DC_OFFSET_2G_HIGH_ADDR_D2K, RX_DC_OFFSET_2G_HIGH_SIZE_D2K},
		*/
		{EQ_CALI_XTAL_CTRIM_DATA, IRF_DATA_EQ_CALI_XTAL_CTRIM,
			EQ_CALI_XTAL_CTRIM_ADDR_D2K, EQ_CALI_XTAL_CTRIM_SIZE_D2K},
		{EQ_CALI_XTAL_TEMP_DATA, IRF_DATA_EQ_CALI_XTAL_TEMP,
			EQ_CALI_XTAL_TEMP_ADDR_D2K, EQ_CALI_XTAL_TEMP_SIZE_D2K},
		{FB_GAIN_ERR_2G_DATA, IRF_DATA_FB_GAIN_ERR, FB_GAIN_ERR_2G_ADDR_D2K,
			FB_GAIN_ERR_2G_SIZE_D2K},
		/*
		{RX_LEVEL_COMP_2G_DATA, IRF_DATA_RX_LEVEL_COMP,
			RX_LEVEL_COMP_2G_ADDR_D2K, RX_LEVEL_COMP_2G_SIZE_D2K},
		{RX_FEM_COMP_2G_DATA, IRF_DATA_RX_FEM_COMP,
			RX_FEM_COMP_2G_ADDR_D2K, RX_FEM_COMP_2G_SIZE_D2K},
		*/
		{TX_GAIN_ERR_2G_DATA, IRF_DATA_TX_GAIN_ERR, TX_GAIN_ERR_2G_ADDR_D2K,
			TX_GAIN_ERR_2G_SIZE_D2K},
	},
	{
		{DIF_EQ_2G_DATA, IRF_DATA_DIF_EQ, DIF_EQ_2G_ADDR_M2K, DIF_EQ_2G_SIZE_M2K},
		{TX_LEVEL_2G_DATA, IRF_DATA_TX_LEVEL, TX_LEVEL_2G_ADDR_M2K, TX_LEVEL_2G_SIZE_M2K},
		{FB_LEVEL_2G_DATA, IRF_DATA_FB_LEVEL, FB_LEVEL_2G_ADDR_M2K, FB_LEVEL_2G_SIZE_M2K},
		/*
		{RX_LEVEL_2G_DATA, IRF_DATA_RX_LEVEL, RX_LEVEL_2G_ADDR_M2K, RX_LEVEL_2G_SIZE_M2K},
		*/
		{TX_FCOMP_2G_DATA, IRF_DATA_TX_FCOMP, TX_FCOMP_2G_ADDR_M2K, TX_FCOMP_2G_SIZE_M2K},
		{FB_FCOMP_2G_DATA, IRF_DATA_FB_FCOMP, FB_FCOMP_2G_ADDR_M2K, FB_FCOMP_2G_SIZE_M2K},
		{RX_FCOMP_2G_DATA, IRF_DATA_RX_FCOMP, RX_FCOMP_2G_ADDR_M2K, RX_FCOMP_2G_SIZE_M2K},
		{TX_TCOMP_2G_DATA, IRF_DATA_TX_TCOMP, TX_TCOMP_2G_ADDR_M2K, TX_TCOMP_2G_SIZE_M2K},
		{FB_TCOMP_2G_DATA, IRF_DATA_FB_TCOMP, FB_TCOMP_2G_ADDR_M2K, FB_TCOMP_2G_SIZE_M2K},
		{RX_TCOMP_2G_DATA, IRF_DATA_RX_TCOMP, RX_TCOMP_2G_ADDR_M2K, RX_TCOMP_2G_SIZE_M2K},
		{PLL_PLAN_2G_DATA, IRF_DATA_PLL_PLAN, PLL_PLAN_2G_ADDR_M2K, PLL_PLAN_2G_SIZE_M2K},
		/*
		{RX_DC_OFFSET_2G_LOW_DATA, IRF_DATA_RX_DC_OFFSET_2G_LOW,
			RX_DC_OFFSET_2G_LOW_ADDR_M2K, RX_DC_OFFSET_2G_LOW_SIZE_M2K},
		{RX_DC_OFFSET_2G_HIGH_DATA, IRF_DATA_RX_DC_OFFSET_2G_HIGH,
			RX_DC_OFFSET_2G_HIGH_ADDR_M2K, RX_DC_OFFSET_2G_HIGH_SIZE_M2K},
		*/
		{EQ_CALI_XTAL_CTRIM_DATA, IRF_DATA_EQ_CALI_XTAL_CTRIM,
			EQ_CALI_XTAL_CTRIM_ADDR_M2K, EQ_CALI_XTAL_CTRIM_SIZE_M2K},
		{EQ_CALI_XTAL_TEMP_DATA, IRF_DATA_EQ_CALI_XTAL_TEMP,
			EQ_CALI_XTAL_TEMP_ADDR_M2K, EQ_CALI_XTAL_TEMP_SIZE_M2K},
		{FB_GAIN_ERR_2G_DATA, IRF_DATA_FB_GAIN_ERR, FB_GAIN_ERR_2G_ADDR_M2K,
			FB_GAIN_ERR_2G_SIZE_M2K},
		/*
		{RX_LEVEL_COMP_2G_DATA, IRF_DATA_RX_LEVEL_COMP,
			RX_LEVEL_COMP_2G_ADDR_M2K, RX_LEVEL_COMP_2G_SIZE_M2K},
		{RX_FEM_COMP_2G_DATA, IRF_DATA_RX_FEM_COMP,
			RX_FEM_COMP_2G_ADDR_M2K, RX_FEM_COMP_2G_SIZE_M2K},
		*/
		{TX_GAIN_ERR_2G_DATA, IRF_DATA_TX_GAIN_ERR, TX_GAIN_ERR_2G_ADDR_M2K,
			TX_GAIN_ERR_2G_SIZE_M2K},
	},
	{
		{TX_LEVEL_2G_DATA, IRF_DATA_TX_LEVEL, TX_LEVEL_2G_ADDR_M3K,
			TX_LEVEL_2G_SIZE_M3K},
		{FB_LEVEL_2G_DATA, IRF_DATA_FB_LEVEL, FB_LEVEL_2G_ADDR_M3K,
			FB_LEVEL_2G_SIZE_M3K},
		/*
		{RX_LEVEL_2G_DATA, IRF_DATA_RX_LEVEL, RX_LEVEL_2G_ADDR_M3K,
			RX_LEVEL_2G_SIZE_M3K},
		*/
		{TX_FCOMP_2G_DATA, IRF_DATA_TX_FCOMP, TX_FCOMP_2G_ADDR_M3K,
			TX_FCOMP_2G_SIZE_M3K},
		{FB_FCOMP_2G_DATA, IRF_DATA_FB_FCOMP, FB_FCOMP_2G_ADDR_M3K,
			FB_FCOMP_2G_SIZE_M3K},
		{RX_FCOMP_2G_DATA, IRF_DATA_RX_FCOMP, RX_FCOMP_2G_ADDR_M3K,
			RX_FCOMP_2G_SIZE_M3K},
		{TX_TCOMP_2G_DATA, IRF_DATA_TX_TCOMP, TX_TCOMP_2G_ADDR_M3K,
			TX_TCOMP_2G_SIZE_M3K},
		{FB_TCOMP_2G_DATA, IRF_DATA_FB_TCOMP, FB_TCOMP_2G_ADDR_M3K,
			FB_TCOMP_2G_SIZE_M3K},
		{RX_TCOMP_2G_DATA, IRF_DATA_RX_TCOMP, RX_TCOMP_2G_ADDR_M3K,
			RX_TCOMP_2G_SIZE_M3K},
		{PLL_PLAN_2G_DATA, IRF_DATA_PLL_PLAN, PLL_PLAN_2G_ADDR_M3K,
			PLL_PLAN_2G_SIZE_M3K},
		/*
		{RX_DC_OFFSET_2G_LOW_DATA, IRF_DATA_RX_DC_OFFSET_2G_LOW,
			RX_DC_OFFSET_2G_LOW_ADDR_M3K, RX_DC_OFFSET_2G_LOW_SIZE_M3K},
		{RX_DC_OFFSET_2G_HIGH_DATA, IRF_DATA_RX_DC_OFFSET_2G_HIGH,
			RX_DC_OFFSET_2G_HIGH_ADDR_M3K, RX_DC_OFFSET_2G_HIGH_SIZE_M3K},
		*/
		{EQ_CALI_XTAL_CTRIM_DATA, IRF_DATA_EQ_CALI_XTAL_CTRIM,
			EQ_CALI_XTAL_CTRIM_ADDR_M3K, EQ_CALI_XTAL_CTRIM_SIZE_M3K},
		{EQ_CALI_XTAL_TEMP_DATA, IRF_DATA_EQ_CALI_XTAL_TEMP,
			EQ_CALI_XTAL_TEMP_ADDR_M3K, EQ_CALI_XTAL_TEMP_SIZE_M3K},
		{FB_GAIN_ERR_2G_DATA, IRF_DATA_FB_GAIN_ERR, FB_GAIN_ERR_2G_ADDR_M3K,
			FB_GAIN_ERR_2G_SIZE_M3K},
		/*
		{RX_LEVEL_COMP_2G_DATA, IRF_DATA_RX_LEVEL_COMP,
			RX_LEVEL_COMP_2G_ADDR_M3K, RX_LEVEL_COMP_2G_SIZE_M3K},
		{RX_FEM_COMP_2G_DATA, IRF_DATA_RX_FEM_COMP,
			RX_FEM_COMP_2G_ADDR_M3K, RX_FEM_COMP_2G_SIZE_M3K},
		*/
		{TX_GAIN_ERR_2G_DATA, IRF_DATA_TX_GAIN_ERR, TX_GAIN_ERR_2G_ADDR_M3K,
			TX_GAIN_ERR_2G_SIZE_M3K},
	}
};

struct irf_file_list table_5g_list[][IRF_TABLE_LIST_NUM] =
{
	{
		{DIF_EQ_5G_DATA, IRF_DATA_DIF_EQ, DIF_EQ_5G_ADDR_D2K, DIF_EQ_5G_SIZE_D2K  },
		{TX_LEVEL_5G_DATA, IRF_DATA_TX_LEVEL, TX_LEVEL_5G_ADDR_D2K, TX_LEVEL_5G_SIZE_D2K},
		{FB_LEVEL_5G_DATA, IRF_DATA_FB_LEVEL, FB_LEVEL_5G_ADDR_D2K, FB_LEVEL_5G_SIZE_D2K},
		/*
		{RX_LEVEL_5G_DATA, IRF_DATA_RX_LEVEL, RX_LEVEL_5G_ADDR_D2K, RX_LEVEL_5G_SIZE_D2K},
		*/
		{TX_FCOMP_5G_DATA, IRF_DATA_TX_FCOMP, TX_FCOMP_5G_ADDR_D2K, TX_FCOMP_5G_SIZE_D2K},
		{FB_FCOMP_5G_DATA, IRF_DATA_FB_FCOMP, FB_FCOMP_5G_ADDR_D2K, FB_FCOMP_5G_SIZE_D2K},
		{RX_FCOMP_5G_DATA, IRF_DATA_RX_FCOMP, RX_FCOMP_5G_ADDR_D2K, RX_FCOMP_5G_SIZE_D2K},
		{TX_TCOMP_5G_DATA, IRF_DATA_TX_TCOMP, TX_TCOMP_5G_ADDR_D2K, TX_TCOMP_5G_SIZE_D2K},
		{FB_TCOMP_5G_DATA, IRF_DATA_FB_TCOMP, FB_TCOMP_5G_ADDR_D2K, FB_TCOMP_5G_SIZE_D2K},
		{RX_TCOMP_5G_DATA, IRF_DATA_RX_TCOMP, RX_TCOMP_5G_ADDR_D2K, RX_TCOMP_5G_SIZE_D2K},
		{PLL_PLAN_5G_DATA, IRF_DATA_PLL_PLAN, PLL_PLAN_5G_ADDR_D2K, PLL_PLAN_5G_SIZE_D2K},
		/*
		{RX_DC_OFFSET_5G_LOW_DATA, IRF_DATA_RX_DC_OFFSET_5G_LOW,
			RX_DC_OFFSET_5G_LOW_ADDR_D2K, RX_DC_OFFSET_5G_LOW_SIZE_D2K},
		{RX_DC_OFFSET_5G_HIGH_DATA, IRF_DATA_RX_DC_OFFSET_5G_HIGH,
			RX_DC_OFFSET_5G_HIGH_ADDR_D2K, RX_DC_OFFSET_5G_HIGH_SIZE_D2K},
		*/
		{EQ_CALI_XTAL_CTRIM_DATA, IRF_DATA_EQ_CALI_XTAL_CTRIM,
			EQ_CALI_XTAL_CTRIM_ADDR_D2K, EQ_CALI_XTAL_CTRIM_SIZE_D2K},
		{EQ_CALI_XTAL_TEMP_DATA, IRF_DATA_EQ_CALI_XTAL_TEMP,
			EQ_CALI_XTAL_TEMP_ADDR_D2K, EQ_CALI_XTAL_TEMP_SIZE_D2K},
		{FB_GAIN_ERR_5G_DATA, IRF_DATA_FB_GAIN_ERR, FB_GAIN_ERR_5G_ADDR_D2K,
			FB_GAIN_ERR_5G_SIZE_D2K},
		/*
		{RX_LEVEL_COMP_5G_DATA, IRF_DATA_RX_LEVEL_COMP,
			RX_LEVEL_COMP_5G_ADDR_D2K, RX_LEVEL_COMP_5G_SIZE_D2K},
		{RX_FEM_COMP_5G_DATA, IRF_DATA_RX_FEM_COMP,
			RX_FEM_COMP_5G_ADDR_D2K, RX_FEM_COMP_5G_SIZE_D2K},
		*/
		{TX_GAIN_ERR_5G_DATA, IRF_DATA_TX_GAIN_ERR, TX_GAIN_ERR_5G_ADDR_D2K,
			TX_GAIN_ERR_5G_SIZE_D2K},
	},
	{
		{DIF_EQ_5G_DATA, IRF_DATA_DIF_EQ, DIF_EQ_5G_ADDR_M2K, DIF_EQ_5G_SIZE_M2K},
		{TX_LEVEL_5G_DATA, IRF_DATA_TX_LEVEL, TX_LEVEL_5G_ADDR_M2K, TX_LEVEL_5G_SIZE_M2K},
		{FB_LEVEL_5G_DATA, IRF_DATA_FB_LEVEL, FB_LEVEL_5G_ADDR_M2K, FB_LEVEL_5G_SIZE_M2K},
		/*
		{RX_LEVEL_5G_DATA, IRF_DATA_RX_LEVEL, RX_LEVEL_5G_ADDR_M2K, RX_LEVEL_5G_SIZE_M2K},
		*/
		{TX_FCOMP_5G_DATA, IRF_DATA_TX_FCOMP, TX_FCOMP_5G_ADDR_M2K, TX_FCOMP_5G_SIZE_M2K},
		{FB_FCOMP_5G_DATA, IRF_DATA_FB_FCOMP, FB_FCOMP_5G_ADDR_M2K, FB_FCOMP_5G_SIZE_M2K},
		{RX_FCOMP_5G_DATA, IRF_DATA_RX_FCOMP, RX_FCOMP_5G_ADDR_M2K, RX_FCOMP_5G_SIZE_M2K},
		{TX_TCOMP_5G_DATA, IRF_DATA_TX_TCOMP, TX_TCOMP_5G_ADDR_M2K, TX_TCOMP_5G_SIZE_M2K},
		{FB_TCOMP_5G_DATA, IRF_DATA_FB_TCOMP, FB_TCOMP_5G_ADDR_M2K, FB_TCOMP_5G_SIZE_M2K},
		{RX_TCOMP_5G_DATA, IRF_DATA_RX_TCOMP, RX_TCOMP_5G_ADDR_M2K, RX_TCOMP_5G_SIZE_M2K},
		{PLL_PLAN_5G_DATA, IRF_DATA_PLL_PLAN, PLL_PLAN_5G_ADDR_M2K, PLL_PLAN_5G_SIZE_M2K},
		/*
		{RX_DC_OFFSET_5G_LOW_DATA, IRF_DATA_RX_DC_OFFSET_5G_LOW,
			RX_DC_OFFSET_5G_LOW_ADDR_M2K, RX_DC_OFFSET_5G_LOW_SIZE_M2K},
		{RX_DC_OFFSET_5G_HIGH_DATA, IRF_DATA_RX_DC_OFFSET_5G_HIGH,
			RX_DC_OFFSET_5G_HIGH_ADDR_M2K, RX_DC_OFFSET_5G_HIGH_SIZE_M2K},
		*/
		{EQ_CALI_XTAL_CTRIM_DATA, IRF_DATA_EQ_CALI_XTAL_CTRIM,
			EQ_CALI_XTAL_CTRIM_ADDR_M2K, EQ_CALI_XTAL_CTRIM_SIZE_M2K},
		{EQ_CALI_XTAL_TEMP_DATA, IRF_DATA_EQ_CALI_XTAL_TEMP,
			EQ_CALI_XTAL_TEMP_ADDR_M2K, EQ_CALI_XTAL_TEMP_SIZE_M2K},
		{FB_GAIN_ERR_5G_DATA, IRF_DATA_FB_GAIN_ERR, FB_GAIN_ERR_5G_ADDR_M2K,
			FB_GAIN_ERR_5G_SIZE_M2K},
		/*
		{RX_LEVEL_COMP_5G_DATA, IRF_DATA_RX_LEVEL_COMP,
			RX_LEVEL_COMP_5G_ADDR_M2K, RX_LEVEL_COMP_5G_SIZE_M2K},
		{RX_FEM_COMP_5G_DATA, IRF_DATA_RX_FEM_COMP,
			RX_FEM_COMP_5G_ADDR_M2K, RX_FEM_COMP_5G_SIZE_M2K},
		*/
		{TX_GAIN_ERR_5G_DATA, IRF_DATA_TX_GAIN_ERR, TX_GAIN_ERR_5G_ADDR_M2K,
			TX_GAIN_ERR_5G_SIZE_M2K},
	},
	{
		{TX_LEVEL_5G_DATA, IRF_DATA_TX_LEVEL, TX_LEVEL_5G_ADDR_M3K,
			TX_LEVEL_5G_SIZE_M3K},
		{FB_LEVEL_5G_DATA, IRF_DATA_FB_LEVEL, FB_LEVEL_5G_ADDR_M3K,
			FB_LEVEL_5G_SIZE_M3K},
		/*
		{RX_LEVEL_5G_DATA, IRF_DATA_RX_LEVEL, RX_LEVEL_5G_ADDR_M3K,
			RX_LEVEL_5G_SIZE_M3K},
		*/
		{TX_FCOMP_5G_DATA, IRF_DATA_TX_FCOMP, TX_FCOMP_5G_ADDR_M3K,
			TX_FCOMP_5G_SIZE_M3K},
		{FB_FCOMP_5G_DATA, IRF_DATA_FB_FCOMP, FB_FCOMP_5G_ADDR_M3K,
			FB_FCOMP_5G_SIZE_M3K},
		{RX_FCOMP_5G_DATA, IRF_DATA_RX_FCOMP, RX_FCOMP_5G_ADDR_M3K,
			RX_FCOMP_5G_SIZE_M3K},
		{TX_TCOMP_5G_DATA, IRF_DATA_TX_TCOMP, TX_TCOMP_5G_ADDR_M3K,
			TX_TCOMP_5G_SIZE_M3K},
		{FB_TCOMP_5G_DATA, IRF_DATA_FB_TCOMP, FB_TCOMP_5G_ADDR_M3K,
			FB_TCOMP_5G_SIZE_M3K},
		{RX_TCOMP_5G_DATA, IRF_DATA_RX_TCOMP, RX_TCOMP_5G_ADDR_M3K,
			RX_TCOMP_5G_SIZE_M3K},
		{PLL_PLAN_5G_DATA, IRF_DATA_PLL_PLAN, PLL_PLAN_5G_ADDR_M3K,
			PLL_PLAN_5G_SIZE_M3K},
		/*
		{RX_DC_OFFSET_5G_LOW_DATA, IRF_DATA_RX_DC_OFFSET_5G_LOW,
			RX_DC_OFFSET_5G_LOW_ADDR_M3K, RX_DC_OFFSET_5G_LOW_SIZE_M3K},
		{RX_DC_OFFSET_5G_HIGH_DATA, IRF_DATA_RX_DC_OFFSET_5G_HIGH,
			RX_DC_OFFSET_5G_HIGH_ADDR_M3K, RX_DC_OFFSET_5G_HIGH_SIZE_M3K},
		*/
		{EQ_CALI_XTAL_CTRIM_DATA, IRF_DATA_EQ_CALI_XTAL_CTRIM,
			EQ_CALI_XTAL_CTRIM_ADDR_M3K, EQ_CALI_XTAL_CTRIM_SIZE_M3K},
		{EQ_CALI_XTAL_TEMP_DATA, IRF_DATA_EQ_CALI_XTAL_TEMP,
			EQ_CALI_XTAL_TEMP_ADDR_M3K, EQ_CALI_XTAL_TEMP_SIZE_M3K},
		{FB_GAIN_ERR_5G_DATA, IRF_DATA_FB_GAIN_ERR, FB_GAIN_ERR_5G_ADDR_M3K,
			FB_GAIN_ERR_5G_SIZE_M3K},
		/*
		{RX_LEVEL_COMP_5G_DATA, IRF_DATA_RX_LEVEL_COMP,
			RX_LEVEL_COMP_5G_ADDR_M3K, RX_LEVEL_COMP_5G_SIZE_M3K},
		{RX_FEM_COMP_5G_DATA, IRF_DATA_RX_FEM_COMP,
			RX_FEM_COMP_5G_ADDR_M3K, RX_FEM_COMP_5G_SIZE_M3K},
		*/
		{TX_GAIN_ERR_5G_DATA, IRF_DATA_TX_GAIN_ERR, TX_GAIN_ERR_5G_ADDR_M3K,
			TX_GAIN_ERR_5G_SIZE_M3K},
	}
};

struct irf_file_list rx_dcoc_table_2g_list[][IRF_RX_DCOC_TABLE_LIST_NUM] = {
	{
		{RX_DC_OFFSET_2G_LOW_DATA, IRF_DATA_RX_DC_OFFSET_2G_LOW,
			RX_DC_OFFSET_2G_LOW_ADDR_D2K, RX_DC_OFFSET_2G_LOW_SIZE_D2K},
		{RX_DC_OFFSET_2G_HIGH_DATA, IRF_DATA_RX_DC_OFFSET_2G_HIGH,
			RX_DC_OFFSET_2G_HIGH_ADDR_D2K, RX_DC_OFFSET_2G_HIGH_SIZE_D2K},
	},
	{
		{RX_DC_OFFSET_2G_LOW_DATA, IRF_DATA_RX_DC_OFFSET_2G_LOW,
			RX_DC_OFFSET_2G_LOW_ADDR_M2K, RX_DC_OFFSET_2G_LOW_SIZE_M2K},
		{RX_DC_OFFSET_2G_HIGH_DATA, IRF_DATA_RX_DC_OFFSET_2G_HIGH,
			RX_DC_OFFSET_2G_HIGH_ADDR_M2K, RX_DC_OFFSET_2G_HIGH_SIZE_M2K},
	},
	{
		{RX_DC_OFFSET_2G_LOW_DATA, IRF_DATA_RX_DC_OFFSET_2G_LOW,
			RX_DC_OFFSET_2G_LOW_ADDR_M3K, RX_DC_OFFSET_2G_LOW_SIZE_M3K},
		{RX_DC_OFFSET_2G_HIGH_DATA, IRF_DATA_RX_DC_OFFSET_2G_HIGH,
			RX_DC_OFFSET_2G_HIGH_ADDR_M3K, RX_DC_OFFSET_2G_HIGH_SIZE_M3K},
	}
};

struct irf_file_list rx_dcoc_table_5g_list[][IRF_RX_DCOC_TABLE_LIST_NUM] = {
	{
		{RX_DC_OFFSET_5G_LOW_DATA, IRF_DATA_RX_DC_OFFSET_5G_LOW,
			RX_DC_OFFSET_5G_LOW_ADDR_D2K, RX_DC_OFFSET_5G_LOW_SIZE_D2K},
		{RX_DC_OFFSET_5G_HIGH_DATA, IRF_DATA_RX_DC_OFFSET_5G_HIGH,
			RX_DC_OFFSET_5G_HIGH_ADDR_D2K, RX_DC_OFFSET_5G_HIGH_SIZE_D2K},
	},
	{
		{RX_DC_OFFSET_5G_LOW_DATA, IRF_DATA_RX_DC_OFFSET_5G_LOW,
			RX_DC_OFFSET_5G_LOW_ADDR_M2K, RX_DC_OFFSET_5G_LOW_SIZE_M2K},
		{RX_DC_OFFSET_5G_HIGH_DATA, IRF_DATA_RX_DC_OFFSET_5G_HIGH,
			RX_DC_OFFSET_5G_HIGH_ADDR_M2K, RX_DC_OFFSET_5G_HIGH_SIZE_M2K},
	},
	{
		{RX_DC_OFFSET_5G_LOW_DATA, IRF_DATA_RX_DC_OFFSET_5G_LOW,
			RX_DC_OFFSET_5G_LOW_ADDR_M3K, RX_DC_OFFSET_5G_LOW_SIZE_M3K},
		{RX_DC_OFFSET_5G_HIGH_DATA, IRF_DATA_RX_DC_OFFSET_5G_HIGH,
			RX_DC_OFFSET_5G_HIGH_ADDR_M3K, RX_DC_OFFSET_5G_HIGH_SIZE_M3K},
	}
};

enum XTAL_CAL_STAUS g_xtal_cali_status = IRF_XTAL_CAL_STATUS_MAX;

int irf_get_fullname(struct cls_wifi_plat *plat, char *full_name,
		enum IRF_FILE_PATH_TYPE path_type, char *file_name)
{
	if (!full_name)
		return -1;

	switch (path_type) {
	case PATH_TYPE_CAL:
		strcpy(full_name, plat->path_info.cal_path);
		break;
	case PATH_TYPE_IRF:
		strcpy(full_name, plat->path_info.irf_path);
		break;
	default:
		pr_err("Unknown path type %d.\n", path_type);
		return -1;
	}

	strcat(full_name, file_name);
	return 0;
}

int irf_load_equipment_data(uint32_t radio_id,uint32_t addr,uint32_t load_for,struct cls_wifi_hw *cls_wifi_hw)
{
	uint32_t offset;
	int buf_len;
	struct irf_file_list *plist;
	uint32_t item;
	uint32_t i;
	uint32_t file_size;
	struct irf_data radio_data;
	uint32_t irf_mem_addr;

#if defined(CFG_MERAK3000)
	(void)load_for;
#endif

	plist = equipment_data_list;
	item = NELEMENTS(equipment_data_list);

	if (cls_wifi_hw->plat && cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_DUBHE2000)
		irf_mem_addr =  IRF_SHARE_MEM_ADDR_D2K;
	else if (cls_wifi_hw->plat && cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK2000)
		irf_mem_addr =  IRF_SHARE_MEM_ADDR_M2K;
	else if (cls_wifi_hw->plat && cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK3000)
		irf_mem_addr =  IRF_SHARE_MEM_ADDR_M3K;

	for(i = 0;i < item;i++){
		offset = plist->load_addr;
		buf_len = plist->load_size;
		if (cls_wifi_hw->radio_params->debug_mode)
			file_size = (u32)cls_wifi_load_irf_configfile(cls_wifi_hw, false, plist->file_name, offset,
					buf_len/sizeof(uint32_t), IRF_RESV_MEM, false);
		else
			file_size = (u32)cls_wifi_load_irf_configfile(cls_wifi_hw, false, plist->file_name, offset,
					buf_len/sizeof(uint32_t), IRF_RESV_MEM, true);

		if(file_size > 0){
			radio_data.data_addr = addr + offset;
			radio_data.data_size = buf_len;
			radio_data.load_flag = DATA_OK;
			if (RADIO_2P4G_INDEX == radio_id) {
				cls_wifi_hw->ipc_env->ops->irf_table_writen(cls_wifi_hw->plat,
						cls_wifi_hw->radio_idx,
						irf_mem_addr + offsetof(struct irf_share_data,
						irf_data_2G[plist->data_type]),
						&radio_data, sizeof(radio_data));
			} else {
				cls_wifi_hw->ipc_env->ops->irf_table_writen(cls_wifi_hw->plat,
						cls_wifi_hw->radio_idx,
						irf_mem_addr + offsetof(struct irf_share_data,
						irf_data_5G[plist->data_type]),
						&radio_data, sizeof(radio_data));
			}
		} else {
			pr_err("%s load fail.\n",plist->file_name);
		}

		plist++;
	}
	return 0;
}

int irf_load_cfr_data(uint32_t radio_id, uint32_t addr, struct cls_wifi_hw *cls_wifi_hw)
{
	uint32_t offset;
	int buf_len;
	uint32_t file_size;
	struct irf_data radio_data;
	uint32_t irf_mem_addr;

	if (cls_wifi_hw->plat && cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_DUBHE2000)
		irf_mem_addr =  IRF_SHARE_MEM_ADDR_D2K;
	else if (cls_wifi_hw->plat && cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK2000)
		irf_mem_addr =  IRF_SHARE_MEM_ADDR_M2K;

	offset = TX_ZIF_ADDR;
	buf_len = TX_ZIF_SIZE;
	if (cls_wifi_hw->radio_params->debug_mode)
		file_size = (u32)cls_wifi_load_irf_cfr_data(cls_wifi_hw, false, TX_ZIF_DATA, offset,
				buf_len/sizeof(uint32_t), IRF_RESV_MEM, false);
	else
		file_size = (u32)cls_wifi_load_irf_cfr_data(cls_wifi_hw, false, TX_ZIF_DATA, offset,
				buf_len/sizeof(uint32_t), IRF_RESV_MEM, true);

	if (file_size > 0) {
		radio_data.data_addr = addr + offset;
		radio_data.data_size = buf_len;
		radio_data.load_flag = DATA_OK;
		if (radio_id == RADIO_2P4G_INDEX) {
			cls_wifi_hw->ipc_env->ops->irf_table_writen(cls_wifi_hw->plat,
					cls_wifi_hw->radio_idx,
					irf_mem_addr + offsetof(struct irf_share_data,
					irf_data_2G[IRF_DATA_TX_ZIF]),
					&radio_data, sizeof(radio_data));
		} else {
			cls_wifi_hw->ipc_env->ops->irf_table_writen(cls_wifi_hw->plat,
					cls_wifi_hw->radio_idx,
					irf_mem_addr + offsetof(struct irf_share_data,
					irf_data_5G[IRF_DATA_TX_ZIF]),
					&radio_data, sizeof(radio_data));
		}
	} else {
		pr_err("Path type %d, file %s load fail.\n", TX_ZIF_DATA);
		return -1;
	}

	return 0;
}

int irf_load_table(uint32_t radio_id, uint32_t addr, struct cls_wifi_plat *cls_wifi_plat, int msg_flag)
{
	uint32_t offset;
	int buf_len;
	struct irf_file_list *plist;
	uint32_t item;
	uint32_t i;
	char full_name[FS_PATH_MAX_LEN];
	uint32_t file_size;
	struct irf_data radio_data;
	int8_t cos_sin_table_init = 0;
	uint8_t rx_gain_tbl_ver = 0;
	uint8_t rx_dcoc_tbl_ver = 0;
	uint8_t version = 0;
	struct irf_tbl_head table_head = {0};
	uint32_t irf_mem_addr;
	bool chip_id_match = true;

	if (radio_id == RADIO_2P4G_INDEX) {
		plist = table_2g_list[cls_wifi_plat->hw_rev];
		item = NELEMENTS(table_2g_list[cls_wifi_plat->hw_rev]);
	} else {
		plist = table_5g_list[cls_wifi_plat->hw_rev];
		item = NELEMENTS(table_5g_list[cls_wifi_plat->hw_rev]);
	}

	if (cls_wifi_plat && cls_wifi_plat->hw_rev == CLS_WIFI_HW_DUBHE2000)
		irf_mem_addr =  IRF_SHARE_MEM_ADDR_D2K;
	else if (cls_wifi_plat && cls_wifi_plat->hw_rev == CLS_WIFI_HW_MERAK2000)
		irf_mem_addr =  IRF_SHARE_MEM_ADDR_M2K;
	else if (cls_wifi_plat && cls_wifi_plat->hw_rev == CLS_WIFI_HW_MERAK3000)
		irf_mem_addr =  IRF_SHARE_MEM_ADDR_M3K;

	chip_id_match = irf_check_chip_id(cls_wifi_plat);

	for (i = 0; i < item; i++) {
		irf_get_fullname(cls_wifi_plat, full_name, plist->path_type, plist->file_name);
		offset = plist->load_addr;
		buf_len = plist->load_size;
		radio_data.data_addr = addr + offset;
		radio_data.data_size = buf_len;

		if (cls_wifi_plat->hw_rev == CLS_WIFI_HW_MERAK2000 ||
		    cls_wifi_plat->hw_rev == CLS_WIFI_HW_MERAK3000)
		{
			if (plist->load_addr + plist->load_size > irf_mem_addr) {
				pr_err("Error: %s %d file %s addr 0x%x size %x max 0x%x\n",
						__func__, __LINE__, full_name, plist->load_addr,
						plist->load_size, irf_mem_addr);
				radio_data.load_flag = DATA_NOK;
				irf_write_table_info(cls_wifi_plat, radio_id, irf_mem_addr,
					&radio_data, plist->data_type);
				plist++;
				continue;
			}

			if (!chip_id_match) {
				if (plist->path_type == PATH_TYPE_CAL) {
					radio_data.load_flag = DATA_NOK;
					irf_write_table_info(cls_wifi_plat, radio_id, irf_mem_addr,
						&radio_data, plist->data_type);
					plist++;
					continue;
				}
			}
		}

		file_size = cls_wifi_load_irf_binfile(cls_wifi_plat, radio_id, full_name,
				offset, buf_len, &version, msg_flag);
		if (file_size > 0) {
			radio_data.load_flag = DATA_OK;
			if (plist->data_type == IRF_DATA_RX_LEVEL)
				rx_gain_tbl_ver = version;

			if (plist->data_type >= IRF_DATA_RX_DC_OFFSET_2G_LOW &&
				plist->data_type <= IRF_DATA_RX_DC_OFFSET_5G_HIGH) {
				rx_dcoc_tbl_ver = version;
				if (rx_dcoc_tbl_ver != rx_gain_tbl_ver)
					pr_err("!!! Radio%d table version not match: rx dcoc: %u rx gain: %u !!!\n",
							radio_id, rx_dcoc_tbl_ver, rx_gain_tbl_ver);
			}
		}  else {
			cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_id, offset, &table_head,
				sizeof(struct irf_tbl_head));

			radio_data.data_addr = addr + offset;
			radio_data.data_size = buf_len;
			//pr_err("%s load fail, file not exist.\n", full_name);
			radio_data.load_flag = DATA_NOK;
		}
		irf_write_table_info(cls_wifi_plat, radio_id, irf_mem_addr, &radio_data, plist->data_type);

		plist++;
	}

	irf_load_rx_gain_dcoc_tbl(radio_id, addr, cls_wifi_plat, msg_flag);

	/* reserved memory */
	if (cls_wifi_plat->hw_rev == CLS_WIFI_HW_MERAK3000) {
		radio_data.data_addr = addr + IRF_RESERVED_ADDR_M3K;
		radio_data.data_size = IRF_RESERVED_SIZE_M3K;
	} else if (cls_wifi_plat->hw_rev == CLS_WIFI_HW_MERAK2000) {
		radio_data.data_addr = addr + IRF_RESERVED_ADDR_M2K;
		radio_data.data_size = IRF_RESERVED_SIZE_M2K;
	} else if (cls_wifi_plat->hw_rev == CLS_WIFI_HW_DUBHE2000) {
		radio_data.data_addr = addr + IRF_RESERVED_ADDR_D2K;
		radio_data.data_size = IRF_RESERVED_SIZE_D2K;
	}
	radio_data.load_flag = DATA_OK;
	irf_write_table_info(cls_wifi_plat, radio_id, irf_mem_addr, &radio_data, IRF_DATA_RESERVED);

	cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_id,
			irf_mem_addr + offsetof(struct irf_share_data,
			cos_sin_table_init),
			&cos_sin_table_init, sizeof(cos_sin_table_init));

	return 0;
}
int irf_load_boot_cali_data(struct cls_wifi_hw *cls_wifi_hw)
{
	uint32_t addr;

	if (cls_wifi_hw->plat &&
	    (cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK2000 ||
	     cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK3000))
	{
		if (cls_wifi_hw->radio_params->debug_mode)
			addr = cls_wifi_hw->plat->if_ops->get_phy_address(cls_wifi_hw->plat,
					cls_wifi_hw->radio_idx, CLS_WIFI_ADDR_IRF_PHY, 0);
		else
			addr = cls_wifi_hw->plat->if_ops->get_phy_address(cls_wifi_hw->plat,
					cls_wifi_hw->radio_idx, CLS_WIFI_ADDR_IRF_TBL_PHY, 0);
	} else {

		addr = cls_wifi_hw->plat->if_ops->get_phy_address(cls_wifi_hw->plat,
				cls_wifi_hw->radio_idx, CLS_WIFI_ADDR_IRF_PHY, 0);
	}

	return irf_load_cfr_data(cls_wifi_hw->radio_idx, addr, cls_wifi_hw);
}

int irf_load_data(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	uint32_t data_type;
	uint32_t addr;
	uint32_t irf_mem_addr;

	if(argc<2){
		pr_err("parameter error, format: load data_type bw\n");
		pr_err("data_type : 0 - equipment data, 2 - table data\n");
		return -1;
	}
	data_type = irf_str2val(argv[1]);

	if (cls_wifi_hw->plat && cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_DUBHE2000)
		irf_mem_addr =  IRF_SHARE_MEM_ADDR_D2K;
	else if (cls_wifi_hw->plat && cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK2000)
		irf_mem_addr =  IRF_SHARE_MEM_ADDR_M2K;
	else if (cls_wifi_hw->plat && cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK3000)
		irf_mem_addr =  IRF_SHARE_MEM_ADDR_M3K;

	if (data_type == 0) {
		addr = cls_wifi_hw->plat->if_ops->get_phy_address(cls_wifi_hw->plat,
				cls_wifi_hw->radio_idx, CLS_WIFI_ADDR_IRF_PHY, 0);
		irf_load_equipment_data(cls_wifi_hw->radio_idx, addr,  EQ_DATA_TX_RX, cls_wifi_hw);

	} else if (data_type == 2) {
		addr = cls_wifi_hw->plat->if_ops->get_phy_address(cls_wifi_hw->plat,
				cls_wifi_hw->radio_idx, CLS_WIFI_ADDR_IRF_TBL_PHY, 0);
		irf_load_table(cls_wifi_hw->radio_idx, addr, cls_wifi_hw->plat, 1);
	} else {
		irf_load_boot_cali_data(cls_wifi_hw);
	}

	return 0;
}

int irf_set_calc_step(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_set_calc_step_req req;

	if(argc<3){
		pr_err("parameter error, format: calc_step task_name step\n");
		pr_err("step: unit 0.1\n");
		return -1;
	}

	req.radio_id = cls_wifi_hw->radio_idx;
	snprintf(req.name,sizeof(req.name),"%s",argv[1]);
	req.step = irf_str2val(argv[2]);

	return cls_wifi_send_irf_set_calc_step_req(cls_wifi_hw,&req);
}

int irf_set_calib_evt(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	enum dif_cali_evt_cmd dif_cali_cmd;
	int ret;

	if(argc<2){
		pr_err("parameter error, format: cali_evt evt_cmd\n");
		pr_err("evt_cmd: boot/online/pause/resume/reset/stop/zif\n");
		return -1;
	}
	if(dif_evt_str2cmd(argv[1],&dif_cali_cmd))
	{
		pr_err("evt_cmd: boot/online/pause/resume/reset/stop/zif\n");
		return -1;
	}

	if (dif_cali_cmd == DIF_EVT_START_ZIF_CALI) {
		if (cls_wifi_hw->plat->dif_sch->g_online_zif_en || cls_wifi_hw->plat->dif_sch->g_pwr_ctrl_en) {
			pr_err("please close pwr_ctrl and online_zif global switch\n");
			return -1;
		}
		if (irf_load_boot_cali_data(cls_wifi_hw))
			return -1;
		cls_wifi_hw->plat->dif_sch->function = DIF_SCH_ZIF_CALI;
	}

	ret = cls_wifi_dif_sm_set_event(cls_wifi_hw, dif_cali_cmd);

	if (ret && (dif_cali_cmd == DIF_EVT_START_ZIF_CALI) && (cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK2000))
		cls_wifi_dif_table_reload(cls_wifi_hw->plat);

	return ret;
}

int irf_send_eq_data(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_send_eq_data_req req;

	if(argc<3){
		pr_err("parameter error, format: send ANT0/1 data_type\n");
		pr_err("data_type: 0-TX ZIF, 1-TX EQ, 2-RX EQ, 3-RX ZIF\n");
		return -1;
	}

	req.radio_id = cls_wifi_hw->radio_idx;
	req.channel = irf_str2val(argv[1]);
	req.data_type = irf_str2val(argv[2]);
	req.stop = 0;
	if(argc > 3){
		if(!strcasecmp(argv[3], "stop")){
			req.stop = 1;
		}
	}

	return cls_wifi_send_irf_eq_data_req(cls_wifi_hw,&req);
}

int irf_dcoc_calc(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_fb_dcoc_req req;
	lmac_msg_id_t dcoc_type;

	if (argc < 3) {
		pr_err("parameter error, format: dcoc_calc type high_temp\n");
		pr_err("type: fb or rx\n");
		pr_err("high_temp: 0 - low temp, 1 - high temp\n");
		return -1;
	}

	req.radio_id = cls_wifi_hw->radio_idx;
	if (!strcasecmp(argv[1],"fb")){
		dcoc_type = IRF_FB_DCOC_REQ;
		dcoc_status[0] = IRF_CALI_STATUS_BUSY;
	} else {
		dcoc_type = IRF_RX_DCOC_REQ;
		dcoc_status[1] = IRF_CALI_STATUS_BUSY;
	}

	req.high_temp = irf_str2val(argv[2]);

	return cls_wifi_send_irf_dcoc_calc_req(cls_wifi_hw, &req, dcoc_type);
}

int irf_dcoc_status(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	uint8_t status;

	if (argc < 2) {
		pr_err("parameter error, format: dcoc_status type\n");
		pr_err("type: fb or rx\n");
		return -1;
	}

	if (!strcasecmp(argv[1],"fb"))
		status = dcoc_status[0];
	else
		status = dcoc_status[1];

	if(status == IRF_CALI_STATUS_BUSY){
		pr_err("DCOC: Busy.\n");
	} else if(status == IRF_CALI_STATUS_DONE){
		pr_err("DCOC: Done.\n");
	} else {
		pr_err("DCOC: Idle.\n");
	}
	return 0;
}

int irf_get_temp(void)
{
	int (*extern_cls_tsens_get_value)(int *data);
	int tmp_rdata[3];
	extern_cls_tsens_get_value = symbol_get(cls_tsens_get_value);
	if (extern_cls_tsens_get_value) {
		extern_cls_tsens_get_value(tmp_rdata);
		symbol_put(cls_tsens_get_value);
	}

	pr_info("*tmp_rdata[0] = %d\n", tmp_rdata[0]);
	pr_info("*tmp_rdata[1] = %d\n", tmp_rdata[1]);
	pr_info("*tmp_rdata[2] = %d\n", tmp_rdata[2]);

	return tmp_rdata[0] / 1000;
}

int irf_get_temp_ctrim_offset(struct cls_wifi_hw *cls_wifi_hw, int temp, int8_t *ctrim_offset)
{
	struct irf_comm_tbl *table_temp = NULL;
	struct temp_ctrim *temp_start = NULL;
	struct temp_ctrim *temp_end = NULL;
	struct temp_ctrim *compensate = NULL;
	struct irf_data radio_data;
	uint8_t delta = TEMP_CTRIM_TABLE_DELTA;
	int8_t temp_in_table;
	uint8_t temp_num = 0;
	uint8_t *buf;
	uint32_t irf_mem_addr;

	pr_info("current temp is %d\n", temp);

	if (cls_wifi_hw->plat && cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_DUBHE2000)
		irf_mem_addr =  IRF_SHARE_MEM_ADDR_D2K;
	else if (cls_wifi_hw->plat && cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK2000)
		irf_mem_addr =  IRF_SHARE_MEM_ADDR_M2K;
	else if (cls_wifi_hw->plat && cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK3000)
		irf_mem_addr =  IRF_SHARE_MEM_ADDR_M3K;

	if (RADIO_2P4G_INDEX == cls_wifi_hw->radio_idx) {
		cls_wifi_hw->plat->ep_ops->irf_table_readn(cls_wifi_hw->plat, cls_wifi_hw->radio_idx,
				irf_mem_addr + offsetof(struct irf_share_data,
				irf_data_2G[IRF_DATA_EQ_CALI_XTAL_TEMP]),
				&radio_data, sizeof(radio_data));
	} else {
		cls_wifi_hw->plat->ep_ops->irf_table_readn(cls_wifi_hw->plat, cls_wifi_hw->radio_idx,
				irf_mem_addr + offsetof(struct irf_share_data,
				irf_data_5G[IRF_DATA_EQ_CALI_XTAL_TEMP]),
				&radio_data, sizeof(radio_data));
	}

	if (radio_data.load_flag != DATA_OK)
		return -1;

	buf = kzalloc(radio_data.data_size, GFP_KERNEL);

	if (!buf)
		return -1;

	if (cls_wifi_hw->plat && cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_DUBHE2000)
		cls_wifi_hw->plat->ep_ops->irf_table_readn(cls_wifi_hw->plat,
							   cls_wifi_hw->radio_idx,
							   EQ_CALI_XTAL_TEMP_ADDR_D2K, buf,
							   radio_data.data_size);
	else if (cls_wifi_hw->plat && cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK2000)
		cls_wifi_hw->plat->ep_ops->irf_table_readn(cls_wifi_hw->plat,
							   cls_wifi_hw->radio_idx,
							   EQ_CALI_XTAL_TEMP_ADDR_M2K, buf,
							   radio_data.data_size);
	else if (cls_wifi_hw->plat && cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK3000)
		cls_wifi_hw->plat->ep_ops->irf_table_readn(cls_wifi_hw->plat,
							   cls_wifi_hw->radio_idx,
							   EQ_CALI_XTAL_TEMP_ADDR_M3K, buf,
							   radio_data.data_size);

	table_temp = (struct irf_comm_tbl *)buf;
	temp_num = table_temp->head.len / sizeof(struct temp_ctrim);
	temp_start = (struct temp_ctrim *)table_temp->data;
	temp_end = temp_start + temp_num;

	for (compensate = temp_start; compensate < temp_end; compensate++) {
		temp_in_table = compensate->temp;
		if (temp_in_table-delta/2 <= temp && temp_in_table+delta/2 >= temp) {
			*ctrim_offset = compensate->ctrim_offset;
			pr_info("get ctrim offset, temp: %d, offset: %d\n", compensate->temp, compensate->ctrim_offset);
			break;
		}
	}

	if (compensate == temp_end) {
		compensate--;
		if (temp < temp_start->temp) {
			*ctrim_offset = temp_start->ctrim_offset;
			pr_err("current temp is lower than lowest temp in xtal_temp table\n");
		}
		else if (temp > compensate->temp) {
			*ctrim_offset = compensate->ctrim_offset;
			pr_err("current temp is higher than highest temp in xtal_temp table\n");
		}
		else {
			pr_err("can not get the temp in xtal_temp table.\n");
			kfree(buf);
			return -1;
		}
	}

	kfree(buf);
	return 0;
}

int irf_get_ctrim(struct cls_wifi_hw *cls_wifi_hw, uint8_t *ctrim)
{
	uint8_t *buf;
	struct irf_data radio_data;
	struct irf_comm_tbl *table_ctrim = NULL;
	uint32_t irf_mem_addr;

	if (cls_wifi_hw->plat && cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_DUBHE2000)
		irf_mem_addr =  IRF_SHARE_MEM_ADDR_D2K;
	else if (cls_wifi_hw->plat && cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK2000)
		irf_mem_addr =  IRF_SHARE_MEM_ADDR_M2K;
	else if (cls_wifi_hw->plat && cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK3000)
		irf_mem_addr =  IRF_SHARE_MEM_ADDR_M3K;
	/*获取flash中的（50度的）ctrim值*/
	if (RADIO_2P4G_INDEX == cls_wifi_hw->radio_idx) {
		cls_wifi_hw->plat->ep_ops->irf_table_readn(cls_wifi_hw->plat, cls_wifi_hw->radio_idx,
				irf_mem_addr + offsetof(struct irf_share_data,
				irf_data_2G[IRF_DATA_EQ_CALI_XTAL_CTRIM]),
				&radio_data, sizeof(radio_data));
	} else {
		cls_wifi_hw->plat->ep_ops->irf_table_readn(cls_wifi_hw->plat, cls_wifi_hw->radio_idx,
				irf_mem_addr + offsetof(struct irf_share_data,
				irf_data_5G[IRF_DATA_EQ_CALI_XTAL_CTRIM]),
				&radio_data, sizeof(radio_data));
	}

	if (radio_data.load_flag != DATA_OK)
		return -1;

	buf = kzalloc(radio_data.data_size, GFP_KERNEL);

	if (!buf)
		return -1;

	if (cls_wifi_hw->plat && cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_DUBHE2000)
		cls_wifi_hw->plat->ep_ops->irf_table_readn(cls_wifi_hw->plat,
							   cls_wifi_hw->radio_idx,
							   EQ_CALI_XTAL_CTRIM_ADDR_D2K, buf,
							   radio_data.data_size);
	else if (cls_wifi_hw->plat && cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK2000)
		cls_wifi_hw->plat->ep_ops->irf_table_readn(cls_wifi_hw->plat,
							   cls_wifi_hw->radio_idx,
							   EQ_CALI_XTAL_CTRIM_ADDR_M2K, buf,
							   radio_data.data_size);
	else if (cls_wifi_hw->plat && cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK3000)
		cls_wifi_hw->plat->ep_ops->irf_table_readn(cls_wifi_hw->plat,
							   cls_wifi_hw->radio_idx,
							   EQ_CALI_XTAL_CTRIM_ADDR_M3K, buf,
							   radio_data.data_size);

	table_ctrim = (struct irf_comm_tbl *)buf;
	*ctrim = table_ctrim->data[0];

	kfree(buf);
	return 0;
}

static void cls_wifi_xtal_set_schedule_work(struct work_struct *xtal_set_work)
{
	struct cls_wifi_dif_sch *dif_sch = container_of(xtal_set_work, struct cls_wifi_dif_sch, xtal_set_work);
	struct cls_wifi_plat *plat;
	int temp;
	int8_t ctrim_offset = 0;
	uint8_t ctrim = 0;
	struct cls_wifi_hw *cls_wifi_hw;
	struct irf_xtal_ctrim_config_req xtal_ctrim_config = {0};
	struct irf_xtal_ctrim_config_cfm cfm = {0};

	plat = (struct cls_wifi_plat *)dif_sch->plat[dif_sch->radio];

	/* if band disable, skip current radio */
	if ((plat == NULL) || !is_band_enabled(plat, dif_sch->radio)) {
		dif_sch->radio = cls_wifi_dif_next_radio(dif_sch->radio);
		plat = (struct cls_wifi_plat *)dif_sch->plat[dif_sch->radio];

		if ((plat == NULL) || !is_band_enabled(plat, dif_sch->radio))
			return;
	}
	cls_wifi_hw = plat->cls_wifi_hw[dif_sch->radio];

	/*获取温度*/
	temp = irf_get_temp();

	/*获取温补表， 确定当前温度对应温补表哪一行, 得到对应的ctrim_offset*/
	if (irf_get_temp_ctrim_offset(cls_wifi_hw, temp, &ctrim_offset) != 0) {
		ctrim_offset = 0;
		pr_err("ctrim_offset get error when set xtal ctrim");
	}

	/*获取flash中的（50度的）ctrim值*/
	if (irf_get_ctrim(cls_wifi_hw, &ctrim) != 0) {
		ctrim = 128;
		pr_err("ctrim get error when set xtal ctrim");
	}
	pr_info("xtal_ctrim_config ctrim: %u", ctrim + ctrim_offset);

	xtal_ctrim_config.ctrim = ctrim + ctrim_offset;

	cls_wifi_send_xtal_ctrim_config_req(cls_wifi_hw, &xtal_ctrim_config, &cfm);
}

void irf_set_ctrim_handler(struct timer_list *timer)
{
	struct cls_wifi_dif_sch *dif_sch = container_of(timer, struct cls_wifi_dif_sch, xtal_set_timer);

	schedule_work(&dif_sch->xtal_set_work);

	if (cls_wifi_mod_params.xtal_ctrim_set && !cls_wifi_mod_params.debug_mode)
		mod_timer(&dif_sch->xtal_set_timer, jiffies + DIF_SET_CTRIM_SCH_TIMER_INTERVAL_JIFFIES);
}

int cls_wifi_afe_xtal_ctrim_set(struct cls_wifi_plat *cls_wifi_plat)
{
	if (cls_wifi_plat->dif_sch == NULL)
		return -1;

	if (cls_wifi_mod_params.xtal_ctrim_set && !cls_wifi_mod_params.debug_mode)
		timer_setup(&cls_wifi_plat->dif_sch->xtal_set_timer, irf_set_ctrim_handler, 0);

	INIT_WORK(&cls_wifi_plat->dif_sch->xtal_set_work, cls_wifi_xtal_set_schedule_work);

	irf_set_ctrim_handler(&cls_wifi_plat->dif_sch->xtal_set_timer);

	return 0;
}
EXPORT_SYMBOL(cls_wifi_afe_xtal_ctrim_set);

void cls_wifi_afe_xtal_ctrim_unset(struct cls_wifi_plat *cls_wifi_plat)
{
	if (cls_wifi_plat->dif_sch == NULL)
		return;

	if (cls_wifi_mod_params.xtal_ctrim_set && !cls_wifi_mod_params.debug_mode)
		del_timer_sync(&cls_wifi_plat->dif_sch->xtal_set_timer);

	cancel_work_sync(&cls_wifi_plat->dif_sch->xtal_set_work);
}
EXPORT_SYMBOL(cls_wifi_afe_xtal_ctrim_unset);

int irf_xtal_cal(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	int ret;
	char name[64];
	int32_t freq_err;
	int32_t ppm_err;
	uint8_t thd;
	uint8_t init_step;
	int8_t ctrim_offset = 0;
	int temp;

	struct irf_xtal_cali_req req = {0};
	struct irf_xtal_cali_cfm cfm = {0};

	if (argc < 2) {
		pr_err("parameter error, format: xtal_cal start/xtal_cal freq [freq error in KHZ]/xtal_cal ppm [freq error in ppm]\n");
		return -1;
	}

	if (!strcasecmp(argv[1], "start")) {
		req.mode = IRF_XTAL_CAL_START;
		req.init_step = IRF_XTAL_CALI_CTRIM;
		if (argc == 3) {
			sscanf(argv[2], "%hhd", &thd);
			req.thd = thd;
		}
		if (argc == 4) {
			sscanf(argv[3], "%hhd", &init_step);
			req.init_step = init_step;
		}
	} else if (!strcasecmp(argv[1], "freq")) {
		req.mode = IRF_XTAL_CAL_FREQ_MODE;
		sscanf(argv[2], "%d", &freq_err);
		req.freq_err = freq_err;
	} else if (!strcasecmp(argv[1], "ppm")) {
		req.mode = IRF_XTAL_CAL_PPM_MODE;
		sscanf(argv[2], "%d", &ppm_err);
		req.ppm_err = ppm_err;
	} else {
		pr_err("parameter error");
		return -1;
	}

	ret = cls_wifi_send_irf_xtal_cal_req(cls_wifi_hw, &req, &cfm);
	if (cfm.status != CO_OK) {
		pr_err("status: %d\n", cfm.status);
	}

	g_xtal_cali_status = cfm.xtal_cal_status;

	if (cfm.xtal_cal_status == IRF_XTAL_CAL_STATUS_DONE) {
		/*获取温度*/
		temp = irf_get_temp();

		/*获取温补表， 确定当前温度对应温补表哪一行, 得到对应的ctrim_offset*/
		if (irf_get_temp_ctrim_offset(cls_wifi_hw, temp, &ctrim_offset) != 0) {
			ctrim_offset = 0;
			pr_err("ctrim_offset get error after eq xtal cali");
		}

		pr_info("get ctrim_offset after eq xtal cali = %d\n", ctrim_offset);

		cfm.ctrim = cfm.ctrim - ctrim_offset;

		pr_info("save ctrim in bin file after eq xtal cali = %u\n", cfm.ctrim);
		irf_get_fullname(cls_wifi_hw->plat, name, EQ_CALI_XTAL_CTRIM_DATA);
		cls_wifi_save_irf_binfile(name, (u32 *)&cfm.head, sizeof(cfm.head) + sizeof(cfm.ctrim));
	}

	return ret;
}

int irf_init_tx_cali(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_set_cali_param_req req;

	if (argc < 3) {
		pr_err("parameter error, format: init_tx_cali ANT0/1 offset \n");
		pr_err("ANT0/1: 0 - ANT0, 1 - ANT1\n");
		pr_err("offset: gain offset, usually is 0, unit 0.1db\n");
		return -1;
	}
	req.radio_id = cls_wifi_hw->radio_idx;
	req.channel = irf_str2val(argv[1]);
	req.gain_comp = irf_str2val(argv[2]);

	return cls_wifi_send_irf_init_txcali_req(cls_wifi_hw,&req);
}

int irf_tx_power_cali(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_set_cali_param_req req;

	if (argc < 3) {
		pr_err("parameter error, format: tx_power_cali ANT0/1 offset \n");
		pr_err("ANT0/1: 0 - ANT0, 1 - ANT1\n");
		pr_err("offset: power offset,unit 0.1db\n");
		return -1;
	}
	req.radio_id = cls_wifi_hw->radio_idx;
	req.channel = irf_str2val(argv[1]);
	req.gain_comp = irf_str2val(argv[2]);

	return cls_wifi_send_irf_txcali_pwr_req(cls_wifi_hw,&req);
}


int irf_fb_power_cali(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_set_cali_param_req req;

	if (argc < 3) {
		pr_err("parameter error, format: fb_power_cali ANT0/1 offset tx_act_pwr\n");
		pr_err("ANT0/1: 0 - ANT0, 1 - ANT1\n");
		pr_err("offset: power offset,unit 0.1db\n");
		pr_err("tx_act_pwr: tx actual power,unit 0.1db\n");
		return -1;
	}
	req.radio_id = cls_wifi_hw->radio_idx;
	req.channel = irf_str2val(argv[1]);
	req.gain_comp = irf_str2val(argv[2]);
	if(argc > 3){
		req.tx_act_pwr = irf_str2val(argv[3]);
	} else {
		req.tx_act_pwr = INVALID_POWER;
	}

	return cls_wifi_send_irf_fbcali_pwr_req(cls_wifi_hw,&req);
}



int irf_set_tx_power(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_set_tx_power_req req;

	if (argc < 3) {
		pr_err("parameter error, format: tx_power ANT0/1 power \n");
		pr_err("ANT0/1: 0 - ANT0, 1 - ANT1\n");
		pr_err("tx_power: tx power level, unit in dBm\n");
		return -1;
	}
	req.radio_id = cls_wifi_hw->radio_idx;
	req.channel = irf_str2val(argv[1]);
	req.tx_power = irf_str2val(argv[2]);

	return cls_wifi_send_irf_tx_pwr_req(cls_wifi_hw,&req);
}

int irf_tx_cali_save(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_set_cali_param_req req;

	req.radio_id = cls_wifi_hw->radio_idx;

	return cls_wifi_send_irf_tx_cali_save_req(cls_wifi_hw,&req);
}

int irf_init_rssi_cali(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_set_cali_param_req req;

	if (argc < 3) {
		pr_err("parameter error, format: init_rssi_cali ANT0/1 offset \n");
		pr_err("ANT0/1: 0 - ANT0, 1 - ANT1\n");
		pr_err("offset: gain offset, usually is 0, unit 0.1db\n");
		return -1;
	}
	req.radio_id = cls_wifi_hw->radio_idx;
	req.channel = irf_str2val(argv[1]);
	req.gain_comp = irf_str2val(argv[2]);

	return cls_wifi_send_irf_init_rxcali_req(cls_wifi_hw,&req);
}
int irf_rssi_cali(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_set_cali_param_req req;

	if (argc < 3) {
		pr_err("parameter error, format: rssi_cali ANT0/1 offset \n");
		pr_err("ANT0/1: 0 - ANT0, 1 - ANT1\n");
		pr_err("offset: power offset,unit 0.1db\n");
		return -1;
	}
	req.radio_id = cls_wifi_hw->radio_idx;
	req.channel = irf_str2val(argv[1]);
	req.gain_comp = irf_str2val(argv[2]);

	return cls_wifi_send_irf_rxcali_rssi_req(cls_wifi_hw,&req);
}

int irf_rssi_cali_save(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_set_cali_param_req req;

	req.radio_id = cls_wifi_hw->radio_idx;

	return cls_wifi_send_irf_rssi_cali_save_req(cls_wifi_hw,&req);
}

int irf_set_rx_gain_lvl(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_set_rx_gain_lvl_req req;

	if (argc < 1) {
		pr_err("parameter error, format: set_rx_gain_lvl gain\n");
		return -1;
	}

	req.radio_id = cls_wifi_hw->radio_idx;
	req.gain = irf_str2val(argv[1]);

	return cls_wifi_send_irf_set_rx_gain_lvl_req(cls_wifi_hw, &req);
}

/*!
 * @brief afe big step debug command parse
 * argv:
 * 1: big module or step, like abb
 * 2. submode or setp, like tx
 *    dig_ctrl++
 * 3. ch c0/c1
 * 4. bw
 * 5. freq
 * 7. value
 * @param argc
 * @param argv
 * @param cls_wifi_hw
 * @return
 */
int irf_afe_cfg(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_afe_cfg_req req;
	int radio;

	if (argc < 2) {
		pr_err("please input command [afe_cfg help]\n");
		return -1;
	}
	radio = cls_wifi_hw->radio_idx;

	if (!strcasecmp(argv[1], "init")) {
		req.mod_id = IRF_AFE_INIT;
		if (!strcasecmp(argv[2], "main")) {
			req.sub_mod_id = IRF_AFE_INIT_MAIN_PROC;
		} else if (!strcasecmp(argv[2], "reset")) {
			req.sub_mod_id = IRF_AFE_INIT_RESET;
		} else if (!strcasecmp(argv[2], "clk2dif")) {
			req.sub_mod_id = IRF_AFE_INIT_CLK_2_DIF;
		} else if (!strcasecmp(argv[2], "colse")) {
			req.sub_mod_id = IRF_AFE_INIT_CLOSE;
		} else if (!strcasecmp(argv[2], "resume")) {
			req.sub_mod_id = IRF_AFE_INIT_RESUME;
		} else {
			pr_err("invalid AFE sub-module %s\n", argv[2]);
			return -1;
		}
		req.radio_id = radio;
		req.chan = irf_str2val(argv[3]);
		req.bw = irf_str2val(argv[4]);
		req.freq = irf_str2val(argv[5]);
		req.val = irf_str2val(argv[6]);
	} else if (!strcasecmp(argv[1], "com_top")) {
		req.mod_id = IRF_AFE_COM_TOP;
		if (!strcasecmp(argv[2], "bandgap")) {
			req.sub_mod_id = IRF_AFE_COM_TOP_BANDGAP;
		} else if (!strcasecmp(argv[2], "rctune")) {
			req.sub_mod_id = IRF_AFE_COM_TOP_RCTUNE;
		} else if (!strcasecmp(argv[2], "rtune")) {
			req.sub_mod_id = IRF_AFE_COM_TOP_RTUNE;
		} else if (!strcasecmp(argv[2], "psensor")) {
			req.sub_mod_id = IRF_AFE_COM_TOP_PSENSOR;
		} else if (!strcasecmp(argv[2], "iptat")) {
			req.sub_mod_id = IRF_AFE_COM_TOP_IPTAT;
		} else {
			pr_err("invalid AFE sub-module %s\n", argv[2]);
			return -1;
		}
		req.radio_id = radio;
		req.chan = irf_str2val(argv[3]);
		req.bw = irf_str2val(argv[4]);
		req.freq = irf_str2val(argv[5]);
		req.val = irf_str2val(argv[6]);
	} else if (!strcasecmp(argv[1], "pll")) {
		req.mod_id = IRF_AFE_PLL;
		if (!strcasecmp(argv[2], "sa")) {
			req.sub_mod_id = IRF_AFE_PLL_SA;
		} else if (!strcasecmp(argv[2], "2g")) {
			req.sub_mod_id = IRF_AFE_PLL_2G;
		} else if (!strcasecmp(argv[2], "5g")) {
			req.sub_mod_id = IRF_AFE_PLL_5G;
		} else {
			pr_err("invalid AFE sub-module %s\n", argv[2]);
			return -1;
		}
		req.radio_id = radio;
		if (req.sub_mod_id != IRF_AFE_PLL_SA) {
			req.chan = irf_str2val(argv[3]);
			req.bw = irf_str2val(argv[4]);
			req.freq = irf_str2val(argv[5]);
			req.val = irf_str2val(argv[6]);
		}
	} else if (!strcasecmp(argv[1], "lo_gen")) {
		req.mod_id = IRF_AFE_LO_GEN;
		if (!strcasecmp(argv[2], "init")) {
			req.sub_mod_id = IRF_AFE_LO_GEN_INIT;
		} else if (!strcasecmp(argv[2], "cali")) {
			req.sub_mod_id = IRF_AFE_LO_GEN_CALI;
		} else {
			pr_err("invalid AFE sub-module %s\n", argv[2]);
			return -1;
		}
		req.radio_id = radio;
		req.chan = irf_str2val(argv[3]);
		req.bw = irf_str2val(argv[4]);
		req.freq = irf_str2val(argv[5]);
		req.val = irf_str2val(argv[6]);
	} else if (!strcasecmp(argv[1], "abb_clk")) {
		req.mod_id = IRF_AFE_ABB_CLK;
		if (!strcasecmp(argv[2], "rx")) {
			req.sub_mod_id = IRF_AFE_ABB_CLK_RX;
		} else if (!strcasecmp(argv[2], "tx")) {
			req.sub_mod_id = IRF_AFE_ABB_CLK_TX;
		} else if (!strcasecmp(argv[2], "fb")) {
			req.sub_mod_id = IRF_AFE_ABB_CLK_FB;
		} else {
			pr_err("invalid AFE sub-module %s\n", argv[2]);
			return -1;
		}
		req.radio_id = radio;
		req.chan = irf_str2val(argv[3]);
		req.bw = irf_str2val(argv[4]);
		req.freq = irf_str2val(argv[5]);
		req.val = irf_str2val(argv[6]);
	} else if (!strcasecmp(argv[1], "iq_gen")) {
		req.mod_id = IRF_AFE_IQ_GEN;
		if (!strcasecmp(argv[2], "cali")) {
			req.sub_mod_id = IRF_AFE_IQ_GEN_CALI;
		} else {
			pr_err("invalid AFE sub-module %s\n", argv[2]);
			return -1;
		}
		req.radio_id = radio;
		req.chan = irf_str2val(argv[3]);
		req.bw = irf_str2val(argv[4]);
		req.freq = irf_str2val(argv[5]);
		req.val = irf_str2val(argv[6]);
	} else if (!strcasecmp(argv[1], "rf")) {
		req.mod_id = IRF_AFE_RF;
		if (!strcasecmp(argv[2], "rx")) {
			req.sub_mod_id = IRF_AFE_RF_RX;
		} else if (!strcasecmp(argv[2], "tx")) {
			req.sub_mod_id = IRF_AFE_RF_TX;
		} else if (!strcasecmp(argv[2], "fb")) {
			req.sub_mod_id = IRF_AFE_RF_FB;
		} else if (!strcasecmp(argv[2], "iqcal")) {
			req.sub_mod_id = IRF_AFE_RF_IQCAL;
		} else if (!strcasecmp(argv[2], "dpd")) {
			req.sub_mod_id = IRF_AFE_RF_DPD;
		} else {
			pr_err("invalid AFE sub-module %s\n", argv[2]);
			return -1;
		}
		req.radio_id = radio;
		req.chan = irf_str2val(argv[3]);
		req.bw = irf_str2val(argv[4]);
		req.freq = irf_str2val(argv[5]);
		req.val = irf_str2val(argv[6]);

	} else if (!strcasecmp(argv[1], "abb")) {
		req.mod_id = IRF_AFE_ABB;
		if (!strcasecmp(argv[2], "rx")) {
			req.sub_mod_id = IRF_AFE_ABB_RX;
		} else if (!strcasecmp(argv[2], "rx_sft")) {
			req.sub_mod_id = IRF_AFE_ABB_RX_SFT;
		} else if (!strcasecmp(argv[2], "tx")) {
			req.sub_mod_id = IRF_AFE_ABB_TX;
		} else if (!strcasecmp(argv[2], "tx_sft")) {
			req.sub_mod_id = IRF_AFE_ABB_TX_SFT;
		} else if (!strcasecmp(argv[2], "fb")) {
			req.sub_mod_id = IRF_AFE_ABB_FB;
		} else if (!strcasecmp(argv[2], "fb_sft")) {
			req.sub_mod_id = IRF_AFE_ABB_FB_SFT;
		} else {
			pr_err("invalid AFE sub-module %s\n", argv[2]);
			return -1;
		}
		req.radio_id = radio;
		req.chan = irf_str2val(argv[3]);
		req.bw = irf_str2val(argv[4]);
		req.freq = irf_str2val(argv[5]);
		req.val = irf_str2val(argv[6]);
	} else if (!strcasecmp(argv[1], "cali")) {
		req.mod_id = IRF_AFE_ALL_CALI;
		req.radio_id = radio;
		req.chan = irf_str2val(argv[3]);
		req.bw = irf_str2val(argv[4]);
		req.freq = irf_str2val(argv[5]);
		req.val = irf_str2val(argv[6]);
	} else if (!strcasecmp(argv[1], "chbw")) {
		req.mod_id = IRF_AFE_CH_BW_SWITCH;
		req.radio_id = radio;
		req.chan = irf_str2val(argv[3]);
		req.bw = irf_str2val(argv[4]);
		req.freq = irf_str2val(argv[5]);
		req.val = irf_str2val(argv[6]);
	} else if (!strcasecmp(argv[1], "dig_ctrl")) {
		req.mod_id = IRF_AFE_DIG_CTRL;
		if (!strcasecmp(argv[2], "abb")) {
			if (!strcasecmp(argv[3], "rx")) {
				req.sub_mod_id = IRF_AFE_ABB_RX;
			} else if (!strcasecmp(argv[3], "tx")) {
				req.sub_mod_id = IRF_AFE_ABB_TX;
			} else if (!strcasecmp(argv[3], "fb")) {
				req.sub_mod_id = IRF_AFE_ABB_FB;
			} else {
				pr_err("invalid AFE ABB sub-module %s\n", argv[3]);
				return -1;
			}
		} else if (!strcasecmp(argv[2], "rf")) {
			if (!strcasecmp(argv[3], "rx")) {
				req.sub_mod_id = IRF_AFE_RF_RX;
			} else if (!strcasecmp(argv[3], "tx")) {
				req.sub_mod_id = IRF_AFE_RF_TX;
			} else if (!strcasecmp(argv[3], "fb")) {
				req.sub_mod_id = IRF_AFE_RF_FB;
			} else {
				pr_err("invalid AFE RF sub-module %s\n", argv[3]);
				return -1;
			}
		} else {
			pr_err("invalid AFE DIG_CTRL sub-module %s\n", argv[2]);
			return -1;
		}
		req.radio_id = radio;
		req.chan = irf_str2val(argv[4]);
		req.bw = irf_str2val(argv[5]);
		req.freq = irf_str2val(argv[6]);
		req.val = irf_str2val(argv[7]);
	} else if (!strcasecmp(argv[1], "help")) {
		pr_err("help for afe_cfg, below is command format:\n");
		pr_err("afe_cfg [mod], [submod], (dig++), [ch], [bw], [center_freq], [value]\n");
		pr_err("afe_cfg init main\n");
		pr_err("afe_cfg init reset\n");
		pr_err("afe_cfg init clk2dif\n");
		pr_err("afe_cfg init close\n");
		pr_err("afe_cfg init resume\n");
		pr_err("afe_cfg com_top bandgap\n");
		pr_err("afe_cfg com_top rctune\n");
		pr_err("afe_cfg com_top rtune\n");
		pr_err("afe_cfg com_top psensor\n");
		pr_err("afe_cfg com_top iptat\n");
		pr_err("afe_cfg pll sa\n");
		pr_err("afe_cfg pll 2g chain bw freq\n");
		pr_err("afe_cfg pll 5g chain bw freq\n");
		pr_err("afe_cfg lo_gen init chain bw freq\n");
		pr_err("afe_cfg lo_gen cali chain bw freq\n");
		pr_err("afe_cfg abb_clk rx chain bw freq\n");
		pr_err("afe_cfg abb_clk tx chain bw freq\n");
		pr_err("afe_cfg abb_clk fb chain bw freq\n");
		pr_err("afe_cfg iq_gen cali chain bw freq\n");
		pr_err("afe_cfg rf rx chain bw freq\n");
		pr_err("afe_cfg rf tx chain bw freq\n");
		pr_err("afe_cfg rf fb chain bw freq\n");
		pr_err("afe_cfg rf iqcal chain bw freq\n");
		pr_err("afe_cfg abb rx chain bw freq\n");
		pr_err("afe_cfg abb tx chain bw freq\n");
		pr_err("afe_cfg abb fb chain bw freq\n");
		pr_err("afe_cfg abb rx_sft chain bw freq val\n");
		pr_err("afe_cfg abb tx_sft chain bw freq val\n");
		pr_err("afe_cfg abb fb_sft chain bw freq val\n");
		pr_err("afe_cfg cali any chain bw freq\n");
		pr_err("afe_cfg chbw any chain bw freq\n");
		pr_err("afe_cfg dig_ctrl abb rx chain bw freq 1\n");
		pr_err("afe_cfg dig_ctrl abb tx chain bw freq 1\n");
		pr_err("afe_cfg dig_ctrl abb fb chain bw freq 1\n");
		pr_err("afe_cfg dig_ctrl rf rx chain bw freq 1\n");
		pr_err("afe_cfg dig_ctrl rf tx chain bw freq 1\n");
		pr_err("afe_cfg dig_ctrl rf fb chain bw freq 1\n");
		pr_err("afe_cfg help\n");
		return 0;
	} else {
		pr_err("invalid AFE module %s\n", argv[1]);
		return -1;
	}

	return cls_wifi_send_irf_afe_cfg_req(cls_wifi_hw, &req);
}

int irf_set_task_param(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_task_param_req req;

	if(argc < 3){
		pr_err("example: task_param fbcgl type 1000 80000\n");
		pr_err("#fbcgl is task name,type is parameter type, 1000 is gateAve, 80000 is train length\n");
		return -1;
	}

	snprintf(req.name,32,"%s",argv[1]);
		req.para_type = irf_str2val(argv[2]);
	if (!strcasecmp(argv[1], "fbeq") || !strcasecmp(argv[1], "ffeq") || !strcasecmp(argv[1], "rxeq")) {
		req.eq_coef_fact = irf_str2val(argv[3]);
	} else {
		switch (req.para_type) {
		case 0:
		case 3:
			req.gate = irf_str2val(argv[3]);
			req.train_len = irf_str2val(argv[4]);
		break;
		case 1:
			if (!strcasecmp(argv[1], "qmc") || !strcasecmp(argv[1], "rxqmc")) {
				req.min_d_thrshld = irf_str2val(argv[3]);
				req.min_d_coef_adj = irf_str2val(argv[4]);
				req.max_d_thrshld = irf_str2val(argv[5]);
				req.max_d_coef_adj = irf_str2val(argv[6]);
				req.c_fact = irf_str2val(argv[7]);
			} else if (!strcasecmp(argv[1], "fbcgl") || !strcasecmp(argv[1], "rxcgl")) {
				req.cgl_calc_len = irf_str2val(argv[3]);
			}
		break;
		case 2:
			req.ff_noise_coef = irf_str2val(argv[3]);
			req.fb_noise_coef = irf_str2val(argv[4]);
		default:
		break;
		}
	}

	return cls_wifi_send_irf_set_task_param_req(cls_wifi_hw,&req);
}

int irf_get_task_smp_data(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	pr_err("Function Not Supported!\n");
	return 0;
}
int irf_capacity(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_show_capacity_req req;

	req.radio_id = cls_wifi_hw->radio_idx;
	return cls_wifi_send_irf_show_capacity_req(cls_wifi_hw,&req);
}

int irf_set_subband_idx(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_set_subband_idx_req req;

	if (argc < 2) {
		pr_err("parameter error, format: subband_idx band_id \n");
		pr_err("band_id: 0/1 for 2.4G, 0~13 for 5G\n");
		return -1;
	}

	req.radio_id = cls_wifi_hw->radio_idx;
	req.subband_idx = irf_str2val(argv[1]);

	return cls_wifi_send_irf_set_subband_idx_req(cls_wifi_hw,&req);
}

int irf_afe_cmd(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	uint32_t i = 0;
	uint8_t str_len = 0;
	struct irf_afe_cmd_req req;

	req.radio_id = current_radio;
	req.argc = argc;

	memset(req.cmd_lines_str, ' ', sizeof(req.cmd_lines_str));

	for (i = 0; i < argc; i++) {
		memcpy(req.cmd_lines_str + str_len, argv[i], strlen(argv[i]));
		str_len += strlen(argv[i]) + 1;
		if (str_len > sizeof(req.cmd_lines_str)) {
			pr_err("string len exceed limitation!\n");
			return -ENOMEM;
		}
	}

	return cls_wifi_send_irf_afe_cmd_req(cls_wifi_hw, &req);
}

int irf_get_radar_detect_result(struct cls_wifi_hw *cls_wifi_hw)
{
	unsigned int det_pulse_num = cls_wifi_hw->radar.detected.pulses_cnt;
	char name[64];
	void __iomem *radar_detect_cfg;
	void __iomem *radar_detect_result;
	int ret;

	radar_detect_cfg = ioremap(RADAR_DETECT_CFG, 0x100);
	radar_detect_result = ioremap(RADAR_DETECT_RESULT, 0x400);
	pr_info("%s: detected pulse num: %u\n", __func__, det_pulse_num);

	irf_reg_write_bits(radar_detect_cfg, DETMEM_OPT_1CFG, 1 << RESULT_MEM_ACCESS_MODE_LSB, RESULT_MEM_ACCESS_MODE_MASK);
	memcpy(&radar_det_buf, radar_detect_result, 8 * det_pulse_num);
	irf_reg_write_bits(radar_detect_cfg, DETMEM_OPT_1CFG, 0 << RESULT_MEM_ACCESS_MODE_LSB, RESULT_MEM_ACCESS_MODE_MASK);

	sprintf(name, "/tmp/irf_radar_det_result.dat");
	ret = cls_wifi_save_radar_int_detect_result_file(name, radar_det_buf, 2 * det_pulse_num);
	if (ret < 0)
		pr_err("%s: fail to save radar detect result as %s\n", __func__, name);

	iounmap(radar_detect_cfg);
	iounmap(radar_detect_result);
	return ret;
}

int irf_radar_detect(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	uint32_t phymode;
	uint32_t trgt_pulse_num;
	uint32_t cac_mode;

	struct irf_radar_detect_req req = {0};

	if (argc < 2) {
		pr_err("parameter error, format: radar_detect get_det_result/radar_detect disable/radar_detect enable [phymode] [trgt_pulse_num] [cac]\n");
		return -1;
	}

	if (!strcasecmp(argv[1], "disable")) {
		req.enable = 0;
	} else if (!strcasecmp(argv[1], "enable")) {
		if (argc < 5) {
			pr_err("parameter error");
			return -1;
		}
		req.enable = 1;
		sscanf(argv[2], "%u", &phymode);
		sscanf(argv[3], "%u", &trgt_pulse_num);
		sscanf(argv[4], "%u", &cac_mode);
		req.phymode = phymode;
		req.trgt_pulse_num = trgt_pulse_num;
		req.cac_mode = cac_mode;
	} else if (!strcasecmp(argv[1], "get_det_result")) {
		return irf_get_radar_detect_result(cls_wifi_hw);
	} else {
		pr_err("parameter error");
		return -1;
	}

	return cls_wifi_send_irf_radar_detect_req(cls_wifi_hw, &req);
}

int irf_get_interference_detect_result(void)
{
	char name[64];
	void __iomem *inf_detect_result;
	int ret;

	inf_detect_result = ioremap(FFT_SMTH_MEM, 0x2000);

	memcpy(&inf_det_buf, inf_detect_result, FFT_SMTH_MEM_LEN);

	sprintf(name, "/tmp/dif_inf_det_result.dat");

	ret = cls_wifi_save_radar_int_detect_result_file(name, inf_det_buf, FFT_SMTH_MEM_LEN / 4);
	if (ret < 0)
		pr_err("%s: fail to save interference detect result as %s\n", __func__, name);

	iounmap(inf_detect_result);
	return ret;
}

int irf_interference_detect(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	uint32_t phymode;
	uint32_t hw_det;
	int ret;

	struct irf_interference_detect_req req = {0};
	struct irf_interference_detect_cfm cfm = {0};

	if (argc < 2) {
		pr_err("parameter error, format: interference_detect get_det_result/interference_detect enable [phymode] [hw_det]\n");
		return -1;
	}

	if (!strcasecmp(argv[1], "enable")) {
		if (argc < 4) {
			pr_err("parameter error");
			return -1;
		}
		sscanf(argv[2], "%u", &phymode);
		sscanf(argv[3], "%u", &hw_det);
		req.phymode = phymode;
		req.hw_det = hw_det;
	} else if (!strcasecmp(argv[1], "get_det_result")) {
		req.get_result = 1;
	} else {
		pr_err("parameter error");
		return -1;
	}

	ret = cls_wifi_send_irf_interference_detect_req(cls_wifi_hw, &req, &cfm);

	if (req.get_result == 1) {
		if (cfm.status == CO_OK) {
			irf_get_interference_detect_result();
		}
	}

	return ret;
}

int clsemi_vndr_cmds_set_cca_cs_thr(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len)
{
	struct nlattr *tb[CLS_NL80211_ATTR_MAX + 1];
	struct cls_wifi_hw *cls_wifi_hw = NULL;
	int rc;
	struct irf_cca_cs_config_req cca_cs_config = {0};
	struct irf_cca_cs_config_cfm cfm = {0};

	cls_wifi_hw = wiphy_priv(wiphy);
	rc = nla_parse(tb, CLS_NL80211_ATTR_MAX, data,
			len, cls_vnd_cmd_policy, NULL);

	if (rc) {
		pr_warn("%s Invalid NL80211 ATTR\n", __func__);
		return rc;
	}

	if (!tb[CLS_NL80211_ATTR_CS_INBDPOW1PUPTHR]) {
		pr_warn("%s Invalid INBDPOW1PUPTHR ATTR\n", __func__);
		return -EINVAL;
	}

	if (!tb[CLS_NL80211_ATTR_CS_CCADELTA]) {
		pr_warn("%s Invalid CCADELTA ATTR\n", __func__);
		return -EINVAL;
	}

	cca_cs_config.inbdpow1_pupthr = nla_get_s8(tb[CLS_NL80211_ATTR_CS_INBDPOW1PUPTHR]);
	cca_cs_config.cca_delta = nla_get_s8(tb[CLS_NL80211_ATTR_CS_CCADELTA]);

	cls_wifi_send_irf_cca_cs_config_req(cls_wifi_hw, &cca_cs_config, &cfm);

	return rc;
}

int clsemi_vndr_cmds_get_cca_cs_thr(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len)
{
	struct cls_wifi_hw *cls_wifi_hw = NULL;
	struct sk_buff *msg;
	struct irf_cca_cs_config_req cca_cs_config = {0};
	struct irf_cca_cs_config_cfm cfm = {0};
	int rc;

	cls_wifi_hw = wiphy_priv(wiphy);

	msg = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, 1);
	if (!msg)
		return -ENOMEM;

	cca_cs_config.get_config = 1;

	cls_wifi_send_irf_cca_cs_config_req(cls_wifi_hw, &cca_cs_config, &cfm);

	nla_put_s8(msg, CLS_NL80211_ATTR_CS_INBDPOW1PUPTHR, cfm.inbdpow1_pupthr);

	rc = cfg80211_vendor_cmd_reply(msg);

	return rc;
}

int clsemi_vndr_cmds_set_cca_ed_thr(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len)
{
	struct nlattr *tb[CLS_NL80211_ATTR_MAX + 1];
	struct cls_wifi_hw *cls_wifi_hw = NULL;
	int rc;
	struct irf_cca_ed_config_req cca_ed_config = {0};
	struct irf_cca_ed_config_cfm cfm = {0};

	cls_wifi_hw = wiphy_priv(wiphy);
	rc = nla_parse(tb, CLS_NL80211_ATTR_MAX, data,
			len, cls_vnd_cmd_policy, NULL);

	if (rc) {
		pr_warn("%s Invalid NL80211 ATTR\n", __func__);
		return rc;
	}

	if (!tb[CLS_NL80211_ATTR_ED_CCA20PRISETHR]) {
		pr_warn("%s Invalid CCA20PRISETHR ATTR\n", __func__);
		return -EINVAL;
	}

	if (!tb[CLS_NL80211_ATTR_ED_CCA20PFALLTHR]) {
		pr_warn("%s Invalid CCA20PFALLTHR ATTR\n", __func__);
		return -EINVAL;
	}

	if (!tb[CLS_NL80211_ATTR_ED_CCA20SRISETHR]) {
			pr_warn("%s Invalid CCA20SRISETHR ATTR\n", __func__);
			return -EINVAL;
	}

	if (!tb[CLS_NL80211_ATTR_ED_CCA20SFALLTHR]) {
		pr_warn("%s Invalid CCA20SFALLTHR ATTR\n", __func__);
		return -EINVAL;
	}

	cca_ed_config.cca20p_risethr = nla_get_s8(tb[CLS_NL80211_ATTR_ED_CCA20PRISETHR]);
	cca_ed_config.cca20p_fallthr = nla_get_s8(tb[CLS_NL80211_ATTR_ED_CCA20PFALLTHR]);
	cca_ed_config.cca20s_risethr = nla_get_s8(tb[CLS_NL80211_ATTR_ED_CCA20SRISETHR]);
	cca_ed_config.cca20s_fallthr = nla_get_s8(tb[CLS_NL80211_ATTR_ED_CCA20SFALLTHR]);

	cls_wifi_send_irf_cca_ed_config_req(cls_wifi_hw, &cca_ed_config, &cfm);

	return rc;
}

int clsemi_vndr_cmds_get_cca_ed_thr(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len)
{
	struct cls_wifi_hw *cls_wifi_hw = NULL;
	struct sk_buff *msg;
	struct irf_cca_ed_config_req cca_ed_config = {0};
	struct irf_cca_ed_config_cfm cfm = {0};
	int rc;

	cls_wifi_hw = wiphy_priv(wiphy);

	msg = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, 1);
	if (!msg)
		return -ENOMEM;

	cca_ed_config.get_config = 1;

	cls_wifi_send_irf_cca_ed_config_req(cls_wifi_hw, &cca_ed_config, &cfm);

	nla_put_s8(msg, CLS_NL80211_ATTR_ED_CCA20PRISETHR, cfm.cca20p_risethr);
	nla_put_s8(msg, CLS_NL80211_ATTR_ED_CCA20PFALLTHR, cfm.cca20p_fallthr);
	nla_put_s8(msg, CLS_NL80211_ATTR_ED_CCA20SRISETHR, cfm.cca20s_risethr);
	nla_put_s8(msg, CLS_NL80211_ATTR_ED_CCA20SFALLTHR, cfm.cca20s_fallthr);

	rc = cfg80211_vendor_cmd_reply(msg);

	return rc;
}

int irf_dcoc_soft_dbg(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_fb_dcoc_req req;
	lmac_msg_id_t dcoc_type;
	uint32_t radio_id;
	uint32_t channel;

	if (argc < 4) {
		pr_err("parameter error, format: dcoc_soft_dbg type ANT0/1 high_temp\n");
		pr_err("type: fb or rx\n");
		pr_err("channel: 0 - ch0, 1 - ch1\n");
		pr_err("high_temp: 0 - low temp, 1 - high temp\n");
		return -1;
	}

	if (strcasecmp(argv[1],"rx")){
		pr_err("just support rx dcoc debug only\n");
		return -1;
	}

	dcoc_type = IRF_RX_DCOC_SOFT_DBG_REQ;
	dcoc_status[1] = IRF_CALI_STATUS_BUSY;
	radio_id = cls_wifi_hw->radio_idx;
	channel = irf_str2val(argv[2]);
	/* bit[4:7] for radio id, bit[0:3] for channel. */
	req.radio_id = (radio_id << 4) | channel;
	req.high_temp = irf_str2val(argv[3]);

	return cls_wifi_send_irf_dcoc_soft_dbg_req(cls_wifi_hw, &req, dcoc_type);
}

int irf_fb_gain_err_cali(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_set_cali_param_req req;

	if (argc < 2) {
		pr_err("parameter error, format: fb_gain_err_cali ANT0/1 \n");
		pr_err("ANT0/1: 0 - ANT0, 1 - ANT1\n");
		return -1;
	}

	if(0 != irf_load_boot_cali_data(cls_wifi_hw)){
		return -1;
	}

	req.radio_id = cls_wifi_hw->radio_idx;
	req.channel = irf_str2val(argv[1]);
	fb_err_cali_status = IRF_CALI_STATUS_BUSY;

	return cls_wifi_send_irf_fb_gain_err_req(cls_wifi_hw,&req);
}

int irf_fb_gain_err_cali_all(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_set_cali_param_req req;

	if (argc < 1) {
		pr_err("parameter error, format: fb_gain_err_cali_all\n");
		return -1;
	}

	if (irf_load_boot_cali_data(cls_wifi_hw) != 0)
		return -1;


	req.radio_id = cls_wifi_hw->radio_idx;
	req.channel = 0xffffffff;
	fb_err_cali_status = IRF_CALI_STATUS_BUSY;
	return cls_wifi_send_irf_fb_gain_err_req(cls_wifi_hw, &req);
}


int irf_fb_gain_err_cali_status(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	switch(fb_err_cali_status){
	case IRF_CALI_STATUS_BUSY:
		pr_err("FB_ERR_CALI: Busy.\n");
		break;
	case IRF_CALI_STATUS_DONE:
		pr_err("FB_ERR_CALI: Done.\n");
		break;
	case IRF_CALI_STATUS_FAIL:
		pr_err("FB_ERR_CALI: Fail.\n");
		break;
	default:
		pr_err("FB_ERR_CALI: Idle.\n");
		break;
	}
	return 0;
}
int irf_tx_fcomp_cali(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_tx_fcomp_cali_req req;
	if (argc < 3) {
		pr_err("parameter error, format: tx_fcomp_cali ANT0/1 fb_level band_idx \n");
		pr_err("ANT0/1: 0 - ANT0, 1 - ANT1\n");
		pr_err("fb_level: fb gain level, defalut 27\n");
		pr_err("band_idx: band index\n");
		return -1;
	}

	req.radio_id = cls_wifi_hw->radio_idx;
	req.channel = irf_str2val(argv[1]);
	req.fb_gain_level = irf_str2val(argv[2]);
	req.band_idx = irf_str2val(argv[3]);

	return cls_wifi_send_irf_tx_fcomp_cali_req(cls_wifi_hw,&req);
}

int irf_tx_cur_power(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_tx_cur_pwr_req req;
	if (argc < 3) {
		pr_err("parameter error, format: tx_cur_power ANT0/1 cur_pwr \n");
		pr_err("ANT0/1: 0 - ANT0, 1 - ANT1\n");
		pr_err("cur_pwr: tx power, unit:0.1dbm\n");
		return -1;
	}

	req.radio_id = cls_wifi_hw->radio_idx;
	req.channel = irf_str2val(argv[1]);
	req.tx_act_pwr = irf_str2val(argv[2]);

	return cls_wifi_send_irf_tx_act_pwr_req(cls_wifi_hw,&req);
}

int irf_tx_loop_power(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_tx_cur_pwr_req req;
	if (argc < 2) {
		pr_err("parameter error, format: tx_loop_power ANT0/1 \n");
		pr_err("ANT0/1: 0 - ANT0, 1 - ANT1\n");
		return -1;
	}

	req.radio_id = cls_wifi_hw->radio_idx;
	req.channel = irf_str2val(argv[1]);

	return cls_wifi_send_irf_tx_loop_power_req(cls_wifi_hw,&req);
}

int irf_rx_gain_lvl_cali(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_rx_gain_lvl_cali_req req;
	int chan;

	if (argc < 5) {
		pr_err("parameter error, format: start_rx_gain_level_cali ant_cali_mod "
				"gain_intv_idx ANT0/1 in_pwr0 in_pwr1\n");
		pr_err("ant_cali_mod: antenna cali mode\n");
		pr_err("gain_intv_idx: rx gain interval index\n");
		pr_err("ANT0/1: 0 - ANT0, 1 - ANT1\n");
		pr_err("in_pwr0: antenna 0 input power\n");
		pr_err("in_pwr1: antenna 1 input power\n");
		return -1;
	}

	req.radio_id = cls_wifi_hw->radio_idx;
	req.chan_cali_mod = irf_str2val(argv[1]);
	req.gain_intv_idx = irf_str2val(argv[2]);
	req.channel = irf_str2val(argv[3]);
	if (req.chan_cali_mod == RX_GAIN_SIG_ANT_CALI) {
		req.in_pwr[0] = irf_str2val(argv[4]);
	} else {
		for (chan = 0; chan < IRF_MAX_ANT_NUM; chan++)
			req.in_pwr[chan] = irf_str2val(argv[4 + chan]);
	}
	rx_cali_status = IRF_CALI_STATUS_BUSY;

	return cls_wifi_send_irf_rx_gain_lvl_cali_req(cls_wifi_hw, &req);
}

int irf_rx_gain_cali_status(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	switch (rx_cali_status) {
	case IRF_CALI_STATUS_BUSY:
		pr_err("RX_GAIN_CALI: Busy.\n");
		break;
	case IRF_CALI_STATUS_DONE:
		pr_err("RX_GAIN_CALI: Done.\n");
		break;
	case IRF_CALI_STATUS_FAIL:
		pr_err("RX_GAIN_CALI: Fail.\n");
		break;
	default:
		pr_err("RX_GAIN_CALI: Idle.\n");
		break;
	}
	return 0;
}

int irf_rx_gain_freq_cali(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_rx_gain_freq_cali_req req;
	if (argc < 4) {
		pr_err("parameter error, format: start_rx_gain_freq_cali ANT0/1 gain band_idx\n");
		pr_err("ANT0/1: 0 - ANT0, 1 - ANT1\n");
		pr_err("gain: base gain\n");
		pr_err("band_idx: band index\n");
		return -1;
	}

	req.radio_id = cls_wifi_hw->radio_idx;
	req.channel = irf_str2val(argv[1]);
	req.gain = irf_str2val(argv[2]);
	req.band_idx = irf_str2val(argv[3]);
	rx_cali_status = IRF_CALI_STATUS_BUSY;

	return cls_wifi_send_irf_rx_gain_freq_cali_req(cls_wifi_hw, &req);
}

int irf_save_rx_gain_freq_ofst(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_set_cali_param_req req;

	req.radio_id = cls_wifi_hw->radio_idx;
	rx_cali_status = IRF_CALI_STATUS_BUSY;

	return cls_wifi_send_irf_save_rx_gain_freq_ofst_req(cls_wifi_hw, &req);
}

int irf_init_rx_gain_cali(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_set_cali_param_req req;

	if (argc < 3) {
		pr_err("parameter error, format: init_rx_gain_cali ANT0/1 offset \n");
		pr_err("ANT0/1: 0 - ANT0, 1 - ANT1\n");
		pr_err("offset: gain offset, usually is 0, unit 0.1db\n");
		return -1;
	}
	req.radio_id = cls_wifi_hw->radio_idx;
	req.channel = irf_str2val(argv[1]);
	req.gain_comp = irf_str2val(argv[2]);

	return cls_wifi_send_irf_init_rxcali_req(cls_wifi_hw,&req);
}

int irf_gain_dbg_lvl(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_gain_dbg_lvl_req req;

	if (argc < 2) {
		pr_err("parameter error, format: gain_dbg_level dbg_level\n");
		pr_err("dbg_level: debug level\n");
		return -1;
	}
	req.dbg_lvl = irf_str2val(argv[1]);

	return cls_wifi_send_irf_gain_dbg_lvl_req(cls_wifi_hw, &req);
}


int irf_tx_gain_err_cali(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_set_cali_param_req req;

	if (argc < 2) {
		pr_err("parameter error, format: tx_gain_err_cali ANT0/1 \n");
		pr_err("ANT0/1: 0 - ANT0, 1 - ANT1\n");
		return -1;
	}

	if(0 != irf_load_boot_cali_data(cls_wifi_hw)){
		return -1;
	}

	req.radio_id = cls_wifi_hw->radio_idx;
	req.channel = irf_str2val(argv[1]);
	tx_err_cali_status = IRF_CALI_STATUS_BUSY;

	return cls_wifi_send_irf_tx_gain_err_req(cls_wifi_hw,&req);
}

int irf_tx_gain_err_cali_all(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_set_cali_param_req req;

	if (argc < 1) {
		pr_err("parameter error, format: tx_gain_err_cali_all\n");
		return -1;
	}

	if (irf_load_boot_cali_data(cls_wifi_hw) != 0)
		return -1;

	req.radio_id = cls_wifi_hw->radio_idx;
	req.channel = 0xffffffff;
	tx_err_cali_status = IRF_CALI_STATUS_BUSY;
	return cls_wifi_send_irf_tx_gain_err_req(cls_wifi_hw, &req);
}


int irf_tx_gain_err_cali_status(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	switch(tx_err_cali_status){
	case IRF_CALI_STATUS_BUSY:
		pr_err("TX_ERR_CALI: Busy.\n");
		break;
	case IRF_CALI_STATUS_DONE:
		pr_err("TX_ERR_CALI: Done.\n");
		break;
	case IRF_CALI_STATUS_FAIL:
		pr_err("TX_ERR_CALI: Fail.\n");
		break;
	default:
		pr_err("TX_ERR_CALI: Idle.\n");
		break;
	}
	return 0;
}

int irf_set_boot_cali(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	if (argc < 1) {
		pr_err("parameter error, format: set_boot_cali 0/1 \n");
		pr_err("0 - disable, 1 - enable\n");
		return -1;
	}

	cls_wifi_hw->dif_sm.boot_cali_enable = irf_str2val(argv[1]);
	return 0;
}
int irf_tx_loop_power_init(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_set_cali_param_req req;

	if (argc < 2) {
		pr_err("parameter error, format: loop_power_init ANT0/1 \n");
		pr_err("ANT0/1: 0 - ANT0, 1 - ANT1\n");
		return -1;
	}

	req.radio_id = cls_wifi_hw->radio_idx;
	req.channel = irf_str2val(argv[1]);

	return cls_wifi_send_irf_tx_loop_pwr_init_req(cls_wifi_hw,&req);
}

int irf_check_alarm(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_check_alm_req req;

	if (argc < 2) {
		pr_err("parameter error, format: check_alm ANT0/1 \n");
		pr_err("ANT0/1: 0 - ANT0, 1 - ANT1\n");
		return -1;
	}

	req.radio_id = cls_wifi_hw->radio_idx;
	req.channel = irf_str2val(argv[1]);

	return cls_wifi_send_check_alarm_req(cls_wifi_hw, &req);
}

int irf_set_tcomp_en(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	uint32_t irf_mem_addr;
	int8_t tcomp_en;
	struct cls_wifi_plat *cls_wifi_plat = cls_wifi_hw->plat;

	if (argc < 1) {
		pr_err("parameter error, format: set_tcomp_en 0/1\n");
		pr_err("0 - disable, 1 - enable\n");
		return -1;
	}

	if (cls_wifi_mod_params.gain_tcomp_en) {

		if (cls_wifi_plat && cls_wifi_plat->hw_rev == CLS_WIFI_HW_DUBHE2000)
			irf_mem_addr =	IRF_SHARE_MEM_ADDR_D2K;
		else
			irf_mem_addr =	IRF_SHARE_MEM_ADDR_M2K;

		tcomp_en = irf_str2val(argv[1]);

		cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, 0,
				irf_mem_addr + offsetof(struct irf_share_data, tcomp_en),
				&tcomp_en, sizeof(int8_t));
	}
	return 0;
}

int irf_set_th_wall_mode(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_th_wall_req req;
	struct irf_th_wall_cfm cfm;

	if (argc < 2) {
		pr_err("parameter error, format: set_th_wall_mode 0/1\n");
		pr_err("0 - disable, 1 - enable\n");
		return -1;
	}

	req.radio_id = cls_wifi_hw->radio_idx;
	req.enable = irf_str2val(argv[1]);
	req.option = IRF_SET_TH_WALL;

	return cls_wifi_send_th_wall_req(cls_wifi_hw, &req, &cfm);
}

int irf_get_th_wall_mode(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_th_wall_req req;
	struct irf_th_wall_cfm cfm;

	if (argc > 1) {
		pr_err("parameter error, format: get_th_wall_mode\n");
		return -1;
	}

	req.radio_id = cls_wifi_hw->radio_idx;
	req.option = IRF_GET_TH_WALL;

	cls_wifi_send_th_wall_req(cls_wifi_hw, &req, &cfm);

	if (cfm.status == CO_OK)
		pr_err("TH wall mode: %u\n", cfm.enable);
	else
		pr_err("get TH wall fail: %u\n", cfm.status);

	return 0;
}

int irf_set_aci_det_para(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_aci_det_para_req req;

	if (argc < 9) {
		pr_err("parameter error, format: set_aci_det_para rssi_thrs aci_pwr_thrs \
				pos_freq_beg_idx pos_freq_end_idx neg_freq_beg_idx \
				neg_freq_end_idx intf_freq_beg_idx intf_freq_end_idx\n");

		return -1;
	}

	if (!cls_wifi_hw->radio_params->debug_mode) {
		pr_err("only support at debug mode\n");

		return -1;
	}

	req.radio_id = cls_wifi_hw->radio_idx;
	req.rssi_thrs = irf_str2val(argv[1]);
	req.aci_pwr_thrs = irf_str2val(argv[2]);
	req.pos_freq_beg_idx = irf_str2val(argv[3]);
	req.pos_freq_end_idx = irf_str2val(argv[4]);
	req.neg_freq_beg_idx = irf_str2val(argv[5]);
	req.neg_freq_end_idx = irf_str2val(argv[6]);
	req.intf_freq_beg_idx = irf_str2val(argv[7]);
	req.intf_freq_end_idx = irf_str2val(argv[8]);

	return cls_wifi_irf_set_aci_det_para_req(cls_wifi_hw, &req);
}

int irf_get_aci_det_para(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_aci_det_para_req req;

	if (argc < 1) {
		pr_err("parameter error, format: get_aci_det_para\n");

		return -1;
	}

	if (!cls_wifi_hw->radio_params->debug_mode) {
		pr_err("only support at debug mode\n");

		return -1;
	}

	req.radio_id = cls_wifi_hw->radio_idx;

	return cls_wifi_irf_get_aci_det_para_req(cls_wifi_hw, &req);
}

int irf_set_close_loop_cali_cycle(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	if (argc < 1) {
		pr_err("parameter error, format: set_close_loop_cali_cycle 0/1~255\n");
		return -1;
	}
	cls_wifi_hw->plat->dif_sch->sch_close_loop_cali.cycle = irf_str2val(argv[1]);
	return 0;
}

int irf_get_rx_gain_cali_para(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_rx_gain_cali_para_req req;

	if (argc < 2) {
		pr_err("example: get_rx_gain_cali_para imd3_thrs\n");
		return -1;
	}

	req.radio_id = cls_wifi_hw->radio_idx;
	if (!strcasecmp(argv[1], "ilna_out_pwr_thrs")) {
		req.para_type = RX_GAIN_CALI_ILNA_OUT_PWR;
		req.ilna_out_pwr.gain_lvl = irf_str2val(argv[2]);
	} else if (!strcasecmp(argv[1], "tx_rx_pwr_calc_max_err")) {
		req.para_type = RX_GAIN_CALI_TX_RX_PWR_ERR;
	} else if (!strcasecmp(argv[1], "imd3_thrs")) {
		req.para_type = RX_GAIN_CALI_IMD3_THRS;
	} else if (!strcasecmp(argv[1], "unused_ilna_lst")) {
		req.para_type = RX_GAIN_CALI_UNUSED_ILNA_LST;
	} else if (!strcasecmp(argv[1], "imd3_calc_ilna_lst")) {
		req.para_type = RX_GAIN_CALI_IMD3_CALC_ILNA_LST;
	} else if (!strcasecmp(argv[1], "input_pwr_lst")) {
		req.para_type = RX_GAIN_CALI_INPUT_PWR_LST;
	} else {
		pr_err("unsupported parameter [%s]\n", argv[1]);
		return -1;
	}

	return cls_wifi_send_irf_get_rx_gain_cali_para_req(cls_wifi_hw, &req);
}

int irf_set_rx_gain_cali_para(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_rx_gain_cali_para_req req;

	if (argc < 3) {
		pr_err("example: set_rx_gain_cali_para imd3_thrs 330\n");
		return -1;
	}

	req.radio_id = cls_wifi_hw->radio_idx;
	if (!strcasecmp(argv[1], "ilna_out_pwr_thrs")) {
		req.para_type = RX_GAIN_CALI_ILNA_OUT_PWR;
		req.ilna_out_pwr.gain_lvl = irf_str2val(argv[2]);
		req.ilna_out_pwr.out_pwr = irf_str2val(argv[3]);
	} else if (!strcasecmp(argv[1], "tx_rx_pwr_calc_max_err")) {
		req.para_type = RX_GAIN_CALI_TX_RX_PWR_ERR;
		req.tx_rx_pwr_err.pwr_err = irf_str2val(argv[2]);
	} else if (!strcasecmp(argv[1], "imd3_thrs")) {
		req.para_type = RX_GAIN_CALI_IMD3_THRS;
		req.imd3_thrs.thrs_val = irf_str2val(argv[2]);
	} else if (!strcasecmp(argv[1], "unused_ilna_lst")) {
		req.para_type = RX_GAIN_CALI_UNUSED_ILNA_LST;
		snprintf(req.unused_ilna_lst.ilna_lst, sizeof(req.unused_ilna_lst.ilna_lst),
				"%s", argv[2]);
	} else if (!strcasecmp(argv[1], "imd3_calc_ilna_lst")) {
		req.para_type = RX_GAIN_CALI_IMD3_CALC_ILNA_LST;
		snprintf(req.imd3_calc_ilna_lst.ilna_lst, sizeof(req.imd3_calc_ilna_lst.ilna_lst),
				"%s", argv[2]);
	} else {
		pr_err("unsupported parameter [%s]\n", argv[1]);
		return -1;
	}

	return cls_wifi_send_irf_set_rx_gain_cali_para_req(cls_wifi_hw, &req);
}

int irf_rx_gain_cali_prep(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_rx_gain_cali_prep_req req;

	if (argc < 1) {
		pr_err("parameter error, format: rx_gain_cali_prep\n");
		return -1;
	}

	req.radio_id = cls_wifi_hw->radio_idx;
	rx_cali_status = IRF_CALI_STATUS_BUSY;

	return cls_wifi_send_irf_rx_gain_cali_prep_req(cls_wifi_hw, &req);
}

int irf_rx_gain_lvl_bist_cali(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_rx_gain_lvl_bist_cali_req req;

	if (argc < 2) {
		pr_err("parameter error, format: start_rx_gain_lvl_bist_cali ant_cali_mod ANT0/1\n");
		pr_err("ant_cali_mod: antenna cali mode\n");
		pr_err("ANT0/1: 0 - ANT0, 1 - ANT1\n");
		return -1;
	}

	req.radio_id = cls_wifi_hw->radio_idx;
	req.chan_cali_mod = irf_str2val(argv[1]);
	if (req.chan_cali_mod == RX_GAIN_SIG_ANT_CALI)
		req.channel = irf_str2val(argv[2]);
	else
		req.channel = 0;
	rx_cali_status = IRF_CALI_STATUS_BUSY;

	return cls_wifi_send_irf_rx_gain_lvl_bist_cali_req(cls_wifi_hw, &req);
}

int irf_rx_gain_imd3_test(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_rx_gain_imd3_test_req req;

	if (argc < 3) {
		pr_err("parameter error, format: rx_gain_imd3_test ANT0/1 gain in_pwr\n");
		pr_err("ANT0/1: 0 - ANT0, 1 - ANT1\n");
		pr_err("gain: rx gain\n");
		pr_err("in_pwr: rx input power\n");
		return -1;
	}

	req.radio_id = cls_wifi_hw->radio_idx;
	req.channel = irf_str2val(argv[1]);
	req.gain = irf_str2val(argv[2]);
	req.in_pwr = irf_str2val(argv[3]);

	return cls_wifi_send_irf_rx_gain_imd3_test_req(cls_wifi_hw, &req);
}

int irf_set_pwr_ctrl_thre(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_pwr_ctrl_thre_req req;

	if (argc < 3) {
		pr_err("parameter error, format: set_pwr_ctrl_thre <lower_thre> <upper_thre>\n");
		pr_err("<threshold> unit 0.1dBm\n");
		return -1;
	}
	req.radio_id = cls_wifi_hw->radio_idx;
	req.lower_thre = irf_str2val(argv[1]);
	req.upper_thre = irf_str2val(argv[2]);
	return cls_wifi_send_set_irf_pwr_ctrl_thre_req(cls_wifi_hw, &req);
}

int irf_get_pwr_ctrl_thre(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_pwr_ctrl_thre_req req;

	if (argc < 1) {
		pr_err("parameter error, format: show_pwr_ctrl_thre\n");
		pr_err("<threshold> unit 0.1dBm\n");
		return -1;
	}
	req.radio_id = cls_wifi_hw->radio_idx;
	return cls_wifi_send_get_irf_pwr_ctrl_thre_req(cls_wifi_hw, &req);
}

int irf_set_pwr_prec_offset(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_pwr_prec_offset_req req;

	if (argc < 2) {
		pr_err("parameter error, format: set_pwr_prec_offset <prec_offset>\n");
		pr_err("<prec_offset> unit 0.1dB\n");
		return -1;
	}

	req.radio_id = cls_wifi_hw->radio_idx;
	req.pwr_prec_offset = irf_str2val(argv[1]);

	return cls_wifi_send_set_irf_pwr_prec_offset_req(cls_wifi_hw, &req);
}

int irf_get_pwr_prec_offset(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_pwr_prec_offset_req req;

	if (argc < 1) {
		pr_err("parameter error, format: get_pwr_prec_offset\n");
		pr_err("<prec_offset> unit 0.1dB\n");
		return -1;
	}

	req.radio_id = cls_wifi_hw->radio_idx;

	return cls_wifi_send_get_irf_pwr_prec_offset_req(cls_wifi_hw, &req);
}

int irf_set_comp_stub_bitmap(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_comp_stub_bitmap_req req;

	if (argc < 2) {
		pr_err("parameter error, format: set_comp_stub_bitmap <bitmap>\n");
		pr_err("| 7| 6| 5| 4| 3| 2| 1| 0|\n");
		pr_err("|freq. comp.|temp. comp.|\n");
		pr_err("|--|FB|RX|TX|--|FB|RX|TX|\n");
		return -1;
	}

	req.radio_id = cls_wifi_hw->radio_idx;
	req.bitmap = irf_str2val(argv[1]);

	return cls_wifi_send_set_irf_comp_stub_bitmap_req(cls_wifi_hw, &req);
}

int irf_get_comp_stub_bitmap(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_comp_stub_bitmap_req req;

	if (argc < 1) {
		pr_err("parameter error, format: get_comp_stub_bitmap\n");
		return -1;
	}

	req.radio_id = cls_wifi_hw->radio_idx;

	return cls_wifi_send_get_irf_comp_stub_bitmap_req(cls_wifi_hw, &req);
}

int irf_set_digital_tx_gain(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	struct irf_set_digital_tx_gain_req req;

	if (argc < 3) {
		pr_err("parameter error, format: set_digital_tx_gain <channel> <gain>\n");
		return -1;
	}

	req.radio_id = cls_wifi_hw->radio_idx;
	req.channel = irf_str2val(argv[1]);
	req.gain = irf_str2val(argv[2]);
	return cls_wifi_send_irf_set_digital_tx_gain_req(cls_wifi_hw, &req);
}


struct irf_cmd irf_cmd_tbl[] = {
	IRF_CMD_DEF("help",	 irf_help),
	IRF_CMD_DEF("hwcfg",	irf_hw_cfg),
	IRF_CMD_DEF("mode",	 irf_set_mode),
	IRF_CMD_DEF("table",	irf_show_table),
	IRF_CMD_DEF("status",   irf_show_status),
	IRF_CMD_DEF("task",	 irf_run_task),
	IRF_CMD_DEF("dif_equip",irf_dif_eq),
	IRF_CMD_DEF("dif_eq_status",irf_dif_eq_status),
	IRF_CMD_DEF("dif_eq_save",irf_dif_eq_save),
	IRF_CMD_DEF("load",	 irf_load_data),
	IRF_CMD_DEF("calc_step",irf_set_calc_step),
	IRF_CMD_DEF("cali_evt",  irf_set_calib_evt),
	IRF_CMD_DEF("send",	 irf_send_eq_data),
	IRF_CMD_DEF("dcoc_calc",  irf_dcoc_calc),
	IRF_CMD_DEF("dcoc_status",irf_dcoc_status),
	IRF_CMD_DEF("xtal_cal",  irf_xtal_cal),
	IRF_CMD_DEF("init_tx_cali", irf_init_tx_cali),
	IRF_CMD_DEF("tx_power_cali",irf_tx_power_cali),
	IRF_CMD_DEF("fb_power_cali",irf_fb_power_cali),
	IRF_CMD_DEF("tx_cali_save", irf_tx_cali_save),
	IRF_CMD_DEF("init_rssi_cali", irf_init_rssi_cali),
	IRF_CMD_DEF("rssi_cali",	  irf_rssi_cali),
	IRF_CMD_DEF("rssi_cali_save", irf_rssi_cali_save),
	IRF_CMD_DEF("set_rx_gain_lvl", irf_set_rx_gain_lvl),
	IRF_CMD_DEF("afe_cfg", irf_afe_cfg),
	IRF_CMD_DEF("task_param", irf_set_task_param),
	IRF_CMD_DEF("get_task_smp_data", irf_get_task_smp_data),
	IRF_CMD_DEF("tx_power", irf_set_tx_power),
	IRF_CMD_DEF("capacity", irf_capacity),
	IRF_CMD_DEF("subband_idx", irf_set_subband_idx),
	IRF_CMD_DEF("afe_cmd", irf_afe_cmd),
	IRF_CMD_DEF("radar_detect", irf_radar_detect),
	IRF_CMD_DEF("interference_detect", irf_interference_detect),
	IRF_CMD_DEF("dcoc_soft_dbg", irf_dcoc_soft_dbg),
	IRF_CMD_DEF("fb_gain_err_cali", irf_fb_gain_err_cali),
	IRF_CMD_DEF("fb_gain_err_cali_all", irf_fb_gain_err_cali_all),
	IRF_CMD_DEF("fb_gain_err_status",irf_fb_gain_err_cali_status),
	IRF_CMD_DEF("tx_fcomp_cali", irf_tx_fcomp_cali),
	IRF_CMD_DEF("tx_cur_power", irf_tx_cur_power),
	IRF_CMD_DEF("tx_loop_power",irf_tx_loop_power),
	IRF_CMD_DEF("start_rx_gain_level_cali", irf_rx_gain_lvl_cali),
	IRF_CMD_DEF("rx_gain_cali_status", irf_rx_gain_cali_status),
	IRF_CMD_DEF("start_rx_gain_freq_cali", irf_rx_gain_freq_cali),
	IRF_CMD_DEF("save_rx_gain_freq_offset", irf_save_rx_gain_freq_ofst),
	IRF_CMD_DEF("init_rx_gain_cali", irf_init_rx_gain_cali),
	IRF_CMD_DEF("gain_dbg_level", irf_gain_dbg_lvl),
	IRF_CMD_DEF("tx_gain_err_cali", irf_tx_gain_err_cali),
	IRF_CMD_DEF("tx_gain_err_cali_all", irf_tx_gain_err_cali_all),
	IRF_CMD_DEF("tx_gain_err_status",irf_tx_gain_err_cali_status),
	IRF_CMD_DEF("set_boot_cali",irf_set_boot_cali),
	IRF_CMD_DEF("loop_power_init",irf_tx_loop_power_init),
	IRF_CMD_DEF("check_alm",irf_check_alarm),
	IRF_CMD_DEF("set_tcomp_en", irf_set_tcomp_en),
	IRF_CMD_DEF("chip_id", irf_show_chip_id),
	IRF_CMD_DEF("set_th_wall_mode", irf_set_th_wall_mode),
	IRF_CMD_DEF("get_th_wall_mode", irf_get_th_wall_mode),
	IRF_CMD_DEF("set_aci_det_para", irf_set_aci_det_para),
	IRF_CMD_DEF("get_aci_det_para", irf_get_aci_det_para),
	IRF_CMD_DEF("set_close_loop_cali_cycle", irf_set_close_loop_cali_cycle),
	IRF_CMD_DEF("get_rx_gain_cali_para", irf_get_rx_gain_cali_para),
	IRF_CMD_DEF("set_rx_gain_cali_para", irf_set_rx_gain_cali_para),
	IRF_CMD_DEF("rx_gain_cali_prep", irf_rx_gain_cali_prep),
	IRF_CMD_DEF("start_rx_gain_lvl_bist_cali", irf_rx_gain_lvl_bist_cali),
	IRF_CMD_DEF("rx_gain_imd3_test", irf_rx_gain_imd3_test),
	IRF_CMD_DEF("set_pwr_ctrl_thre", irf_set_pwr_ctrl_thre),
	IRF_CMD_DEF("get_pwr_ctrl_thre", irf_get_pwr_ctrl_thre),
	IRF_CMD_DEF("set_pwr_prec_offset", irf_set_pwr_prec_offset),
	IRF_CMD_DEF("get_pwr_prec_offset", irf_get_pwr_prec_offset),
	IRF_CMD_DEF("set_comp_stub_bitmap", irf_set_comp_stub_bitmap),
	IRF_CMD_DEF("get_comp_stub_bitmap", irf_get_comp_stub_bitmap),
	IRF_CMD_DEF("set_digital_tx_gain", irf_set_digital_tx_gain),
};

int irf_help(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw)
{
	int i;

	pr_err("irf cmd list below:\n");
	for(i = 1; i < sizeof(irf_cmd_tbl)/sizeof(struct irf_cmd);i++){
		pr_err("%s\n",irf_cmd_tbl[i].name);
	}
	return 0;
}

int irf_cmd_distribute(struct cls_wifi_hw *cls_wifi_hw,char *cmd_str)
{
	char *argv[CAL_CMD_MAX_PARAM];
	int argc;
	int i;

	argc = irf_spilt_param(cmd_str,argv);
	if(argc <= 0){
		return -1;
	}

	for(i = 0; i < sizeof(irf_cmd_tbl)/sizeof(struct irf_cmd);i++){
		if(!strcasecmp(argv[0],irf_cmd_tbl[i].name)){
			return irf_cmd_tbl[i].pfunc_exec(argc,argv,cls_wifi_hw);
		}
	}

	pr_err("NO such IRF command:%s\n",argv[0]);

	return -1;
}

void cls_wifi_start_dpd_fbdelay_cali(struct cls_wifi_hw *cls_wifi_hw)
{
	struct cls_wifi_vif *vif;
	struct mm_get_power_cfm cfm;
	struct irf_set_tx_power_req pw_req;
	struct mm_dpd_wmac_tx_params_req tx_req;
	struct irf_set_pppc_switch_req pppc_req;
	u8 bw;
	u8 other_radio;
	struct cls_wifi_hw *other_radio_hw;
	struct cls_wifi_plat *other_plat;

	bw = irf_get_curr_bw(cls_wifi_hw);
	//save current tx power
	vif = cls_wifi_get_vif(cls_wifi_hw, 0);
	if (vif)
		cls_wifi_send_get_power(cls_wifi_hw, vif->vif_index, &cfm);
	else
		return;

	cls_wifi_hw->current_power = cfm.power;

	//Disable pwr ctrl before dpd starts
	cls_wifi_dif_sm_pause(cls_wifi_hw);
	cls_wifi_dif_sm_set_event(cls_wifi_hw, DIF_EVT_CALI_STOP);
	if (cls_wifi_hw->radio_idx == RADIO_2P4G_INDEX)
		other_radio = RADIO_5G_INDEX;
	else
		other_radio = RADIO_2P4G_INDEX;

	other_plat = cls_wifi_hw->plat->dif_sch->plat[other_radio];
	if (other_plat && is_band_enabled(other_plat, other_radio)) {
		other_radio_hw = other_plat->cls_wifi_hw[other_radio];
		cls_wifi_dif_sm_set_event(other_radio_hw, DIF_EVT_CALI_STOP);
	}

	//Disable pppc before dpd starts
	pppc_req.pppc_switch = 0;
	pppc_req.radio_id = cls_wifi_hw->radio_idx;
	cls_wifi_set_irf_pppc_switch(cls_wifi_hw, &pppc_req);

	//set tx power from below table
	//5G:19; 2.4G-20M:22, 2.4G-40M:21
	pw_req.radio_id = cls_wifi_hw->radio_idx;
	pw_req.channel = 0;
	if (cls_wifi_hw->radio_idx == RADIO_5G_INDEX)
		pw_req.tx_power = cls_wifi_mod_params.dpd_tx_power_5g;
	else {
		if (bw == PHY_CHNL_BW_20)
			pw_req.tx_power = cls_wifi_mod_params.dpd_tx_power_2g_20;
		else if (bw == PHY_CHNL_BW_40)
			pw_req.tx_power = cls_wifi_mod_params.dpd_tx_power_2g_40;
	}

	cls_wifi_send_irf_tx_pwr_req(cls_wifi_hw,&pw_req);
	pw_req.channel = 1;
	cls_wifi_send_irf_tx_pwr_req(cls_wifi_hw,&pw_req);

	if (!test_bit(CLS_WIFI_DEV_STARTED, &cls_wifi_hw->flags)) {
		schedule_work(&cls_wifi_hw->dpd_restore_work);
		return;
	}

	//start wmac sending API, sending short packets 0.1ms interval, 2 agg, payload 60 bytes，sending 1000 packets
	tx_req.ppdu_interval = 100;
	tx_req.mpdu_num = 2;
	cls_wifi_hw->dpd_wmac_tx_params.req.mpdu_num = tx_req.mpdu_num;
	tx_req.mpdu_payload_size = 1000;
	tx_req.ppdu_num = 1000;
	tx_req.bw = bw;
	tx_req.source = 0;
	tx_req.nss_mcs = 0xb;
	tx_req.tx_power = pw_req.tx_power;

	cls_wifi_hw->dif_sm.dif_online_dpd_task_type = DIF_FBDELAY_TASK;
	cls_wifi_dpd_wmac_tx_handler(cls_wifi_hw, &tx_req);

	cls_wifi_dif_trigger_online_dpd_task_once(cls_wifi_hw);
}

void cls_wifi_start_dpd_cali(struct cls_wifi_hw *cls_wifi_hw)
{
	struct mm_dpd_wmac_tx_params_req tx_req;
	u8 bw;

	bw = irf_get_curr_bw(cls_wifi_hw);

	if (!test_bit(CLS_WIFI_DEV_STARTED, &cls_wifi_hw->flags)) {
		schedule_work(&cls_wifi_hw->dpd_restore_work);
		return;
	}

	//start wmac sending API, sending short packets 10ms interval, 48 agg, payload 11k bytes, sending 20 packets
	tx_req.ppdu_interval = 10000;

	if (bw == PHY_CHNL_BW_160)
		tx_req.mpdu_num = 48;
	else if (bw == PHY_CHNL_BW_80)
		tx_req.mpdu_num = 24;
	else if (bw == PHY_CHNL_BW_40)
		tx_req.mpdu_num = 12;
	else if (bw == PHY_CHNL_BW_20)
		tx_req.mpdu_num = 6;

	cls_wifi_hw->dpd_wmac_tx_params.req.mpdu_num = tx_req.mpdu_num;
	tx_req.mpdu_payload_size = 11000;
	tx_req.ppdu_num = 80;
	tx_req.bw = bw;
	if (cls_wifi_mod_params.dpd_tx_nss == 1)
		tx_req.nss_mcs = 0xb;
	else
		tx_req.nss_mcs = 0x1b;

	tx_req.source = 0;

	if (cls_wifi_hw->radio_idx == 1)
		tx_req.tx_power = cls_wifi_mod_params.dpd_tx_power_5g;
	else {
		if (bw == PHY_CHNL_BW_20)
			tx_req.tx_power = cls_wifi_mod_params.dpd_tx_power_2g_20;
		else if (bw == PHY_CHNL_BW_40)
			tx_req.tx_power = cls_wifi_mod_params.dpd_tx_power_2g_40;
	}

	cls_wifi_hw->dif_sm.dif_online_dpd_task_type = DIF_PD_TASK;
	cls_wifi_dpd_wmac_tx_handler(cls_wifi_hw, &tx_req);
	cls_wifi_dif_trigger_online_dpd_task_once(cls_wifi_hw);
}

static void cls_wifi_dpd_restore_work(struct work_struct *ws)
{
	struct cls_wifi_hw *cls_wifi_hw = container_of(ws, struct cls_wifi_hw, dpd_restore_work);
	struct irf_set_pppc_switch_req pppc_req;
	struct irf_set_tx_power_req pw_req;

	cls_wifi_hw->dif_sm.dif_online_dpd_task_type = DIF_ZIF_TASK;

	cls_wifi_hw->dif_sm.fbdelay_task_times = 0;
	cls_wifi_hw->dif_sm.fbdelay_success_flag = 0;
	cls_wifi_hw->dif_sm.pd_task_success_flag = 0;

	//reset power
	pw_req.radio_id = cls_wifi_hw->radio_idx;
	pw_req.channel = 0;
	pw_req.tx_power = cls_wifi_hw->current_power;
	cls_wifi_send_irf_tx_pwr_req(cls_wifi_hw,&pw_req);
	pw_req.channel = 1;
	cls_wifi_send_irf_tx_pwr_req(cls_wifi_hw,&pw_req);

	//open pwr cali and pppc
	cls_wifi_dif_sm_resume(cls_wifi_hw);
	pppc_req.pppc_switch = 1;
	pppc_req.radio_id = cls_wifi_hw->radio_idx;
	cls_wifi_set_irf_pppc_switch(cls_wifi_hw, &pppc_req);
}

int cls_wifi_dpd_online_schedule_init(struct cls_wifi_hw *cls_wifi_hw)
{
	INIT_WORK(&cls_wifi_hw->dpd_restore_work, cls_wifi_dpd_restore_work);

	return 0;
}
EXPORT_SYMBOL(cls_wifi_dpd_online_schedule_init);

void cls_wifi_dpd_online_schedule_deinit(struct cls_wifi_hw *cls_wifi_hw)
{
	if (cls_wifi_hw->dpd_restore_work.func)
		cancel_work_sync(&cls_wifi_hw->dpd_restore_work);
}
EXPORT_SYMBOL(cls_wifi_dpd_online_schedule_deinit);

int cls_wifi_irf_init(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx)
{
#ifndef CFG_PCIE_SHM
	uint32_t irf_tbl_addr;
	uint32_t irf_mem_addr;

	if (cls_wifi_plat && cls_wifi_plat->hw_rev == CLS_WIFI_HW_DUBHE2000)
		irf_mem_addr =  IRF_SHARE_MEM_ADDR_D2K;
	else if (cls_wifi_plat->hw_rev == CLS_WIFI_HW_MERAK2000)
		irf_mem_addr =  IRF_SHARE_MEM_ADDR_M2K;
	else if (cls_wifi_plat->hw_rev == CLS_WIFI_HW_MERAK3000)
		irf_mem_addr =  IRF_SHARE_MEM_ADDR_M3K;
	if (cls_wifi_plat->ep_ops->irf_table_writen) {
		if (RADIO_2P4G_INDEX == radio_idx) {
			cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_idx,
					irf_mem_addr + offsetof(struct irf_share_data,
					afe_2g_enable),
					&cls_wifi_mod_params.afe_enable, sizeof(uint8_t));
			cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_idx,
					irf_mem_addr + offsetof(struct irf_share_data,
					bw_2g),
					&cls_wifi_mod_params.bw_2g, sizeof(int));
			cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_idx,
					irf_mem_addr + offsetof(struct irf_share_data,
					chan_ieee_2g),
					&cls_wifi_mod_params.chan_ieee_2g, sizeof(int));
			cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_idx,
					irf_mem_addr + offsetof(struct irf_share_data,
					ant_num_2g),
					&cls_wifi_mod_params.ant_num_2g, sizeof(uint8_t));
		} else if (RADIO_5G_INDEX == radio_idx) {
			cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_idx,
					irf_mem_addr + offsetof(struct irf_share_data,
					afe_5g_enable),
					&cls_wifi_mod_params.afe_enable, sizeof(uint8_t));
			cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_idx,
					irf_mem_addr + offsetof(struct irf_share_data,
					bw_5g),
					&cls_wifi_mod_params.bw_5g, sizeof(int));
			cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_idx,
					irf_mem_addr + offsetof(struct irf_share_data,
					chan_ieee_5g),
					&cls_wifi_mod_params.chan_ieee_5g, sizeof(int));
			cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_idx,
					irf_mem_addr + offsetof(struct irf_share_data,
					ant_num_5g),
					&cls_wifi_mod_params.ant_num_5g, sizeof(uint8_t));
		}
		cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_idx,
					irf_mem_addr + offsetof(struct irf_share_data,
					ant_mask),
					&cls_wifi_mod_params.ant_mask, sizeof(int));
		cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_idx,
					irf_mem_addr + offsetof(struct irf_share_data,
					afe_dig_ctrl),
					&cls_wifi_mod_params.afe_dig_ctrl, sizeof(int));
		cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_idx,
					irf_mem_addr + offsetof(struct irf_share_data,
					afe_fem_en),
					&cls_wifi_mod_params.afe_fem_en, sizeof(int));
		cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_idx,
					irf_mem_addr + offsetof(struct irf_share_data,
					afe_feat_mask),
					&cls_wifi_mod_params.afe_feat_mask, sizeof(int));
		cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_idx,
					irf_mem_addr + offsetof(struct irf_share_data,
					dpd_cca_disable),
					&cls_wifi_mod_params.dpd_disable_cca, sizeof(bool));

		cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_idx,
					irf_mem_addr + offsetof(struct irf_share_data,
					dpd_tx_power_5g),
					&cls_wifi_mod_params.dpd_tx_power_5g, sizeof(int));
		cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_idx,
					irf_mem_addr + offsetof(struct irf_share_data,
					dpd_tx_power_2g_20),
					&cls_wifi_mod_params.dpd_tx_power_2g_20, sizeof(int));
		cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_idx,
					irf_mem_addr + offsetof(struct irf_share_data,
					dpd_tx_power_2g_40),
					&cls_wifi_mod_params.dpd_tx_power_2g_40, sizeof(int));
		cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_idx,
					irf_mem_addr + offsetof(struct irf_share_data,
					low_pwr_en),
					&cls_wifi_mod_params.low_pwr_en, sizeof(bool));
		cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_idx,
					irf_mem_addr + offsetof(struct irf_share_data,
					ibex_en),
					&cls_wifi_mod_params.ibex_en, sizeof(bool));
	}

	irf_tbl_addr = cls_wifi_plat->if_ops->get_phy_address(cls_wifi_plat,
			radio_idx, CLS_WIFI_ADDR_IRF_TBL_PHY, 0);
	irf_load_table(radio_idx, irf_tbl_addr, cls_wifi_plat, 1);
#endif

	return 0;
}
EXPORT_SYMBOL(cls_wifi_irf_init);

int cls_wifi_irf_smp_send_ram_init(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx)
{
	if (is_band_enabled(cls_wifi_plat, radio_idx))
		irf_smp_send_ram_init(cls_wifi_plat, radio_idx);

	return 0;
}
EXPORT_SYMBOL(cls_wifi_irf_smp_send_ram_init);

void cls_wifi_irf_deinit(struct cls_wifi_hw *cls_wifi_hw)
{
	kfree(cfr_data_buf);
	cfr_data_buf = NULL;
}
EXPORT_SYMBOL(cls_wifi_irf_deinit);

#ifdef CFG_PCIE_SHM
static void cls_wifi_irf_init_shared_data(struct irf_share_data *irf_shared_addr)
{
	((struct irf_share_data *)irf_shared_addr)->afe_2g_enable =
		cls_wifi_mod_params.afe_enable;
	((struct irf_share_data *)irf_shared_addr)->afe_5g_enable =
		cls_wifi_mod_params.afe_enable;
	((struct irf_share_data *)irf_shared_addr)->bw_2g =
		cls_wifi_mod_params.bw_2g;
	((struct irf_share_data *)irf_shared_addr)->bw_5g =
		cls_wifi_mod_params.bw_5g;
	((struct irf_share_data *)irf_shared_addr)->chan_ieee_2g =
		cls_wifi_mod_params.chan_ieee_2g;
	((struct irf_share_data *)irf_shared_addr)->chan_ieee_5g =
		cls_wifi_mod_params.chan_ieee_5g;
	((struct irf_share_data *)irf_shared_addr)->ant_num_2g =
		cls_wifi_mod_params.ant_num_2g;
	((struct irf_share_data *)irf_shared_addr)->ant_num_5g =
		cls_wifi_mod_params.ant_num_5g;
	((struct irf_share_data *)irf_shared_addr)->ant_mask =
		cls_wifi_mod_params.ant_mask;
	((struct irf_share_data *)irf_shared_addr)->afe_dig_ctrl =
		cls_wifi_mod_params.afe_dig_ctrl;
	((struct irf_share_data *)irf_shared_addr)->afe_fem_en =
		cls_wifi_mod_params.afe_fem_en;
	((struct irf_share_data *)irf_shared_addr)->afe_feat_mask =
		cls_wifi_mod_params.afe_feat_mask;
}

static int cls_wifi_irf_outbound_load_cfgfile(struct cls_wifi_plat *plat, char *src_fname, u32 offset,
						u32 len)
{
	struct file *filp = NULL;
	char tmp_str[9] = {0};
	u32 i;
	char delimiter;
	unsigned long buffer;
	u32 *buf;
	ssize_t read_size;
	char full_name[FS_PATH_MAX_LEN];

	if ((strlen(plat->path_info.irf_path) + strlen(src_fname)) > FS_PATH_MAX_LEN)
	{
		pr_warn("file name or file path overlong\n");
		return -1;
	}
	strcpy(full_name, plat->path_info.irf_path);
	strcat(full_name, src_fname);
	pr_warn("open target file : %s\n", full_name);
	filp = filp_open(full_name, O_RDONLY, 0);

	if (IS_ERR(filp)) {
		pr_warn("open target file fail: %s\n", src_fname);
		return -1;
	}

	buf = kzalloc(len * sizeof(u32), GFP_KERNEL);
	if (!buf) {
		filp_close(filp, current->files);
		return -1;
	}

	for (i = 0; i < len; i++) {
		read_size = kernel_read(filp, tmp_str, sizeof(tmp_str) - 1, &filp->f_pos);
		if(read_size <= 0)
			break;
		if (kstrtoul(tmp_str, 16, &buffer))
			break;
		buf[i] = buffer;
		kernel_read(filp, &delimiter, 1, &filp->f_pos);
		if (delimiter == '\r')
			filp->f_pos += 1;
	}

	pr_warn("*** loading file %s size: %lu/%u\n", src_fname, i*sizeof(u32), len);
	memcpy(plat->irf_tbl_virt_addr + IRF_TBL_SIZE + offset, buf, i * sizeof(u32));

	kfree(buf);
	filp_close(filp, current->files);
	return (i * sizeof(u32));
}

static int cls_wifi_irf_outbound_load_table(uint32_t radio_id, struct cls_wifi_plat *cls_wifi_plat)
{
	uint32_t offset;
	int buf_len;
	struct irf_file_list *plist;
	uint32_t item;
	uint32_t i;
	uint32_t file_size;
	struct irf_data radio_data;
	uint8_t rx_gain_tbl_ver = 0;
	uint8_t rx_dcoc_tbl_ver = 0;
	uint8_t version = 0;
	uint32_t pool_offset;
	char full_name[FS_PATH_MAX_LEN];
	struct irf_tbl_head table_head = {0};
	pcie_shm_pool_st *shm_obj = cls_wifi_plat->pcie_shm_pools[0];
	struct irf_share_data *irf_shared_addr = (struct irf_share_data *)(
		cls_wifi_plat->irf_tbl_virt_addr + IRF_SHARE_MEM_ADDR_M3K);

	if (pcie_shm_get_tbl_offset(shm_obj, IRF_TBL_DFT_IDX, &pool_offset)) {
		pr_err("pcie_shm_get_tbl_offset failed!\n");
		return -1;
	}

	if (RADIO_2P4G_INDEX == radio_id) {
		plist = table_2g_list[cls_wifi_plat->hw_rev];
		item = NELEMENTS(table_2g_list[cls_wifi_plat->hw_rev]);
	} else {
		plist = table_5g_list[cls_wifi_plat->hw_rev];
		item = NELEMENTS(table_5g_list[cls_wifi_plat->hw_rev]);
	}

	for (i = 0; i < item - 1; i++) {
		irf_get_fullname(cls_wifi_plat, full_name, plist->path_type, plist->file_name);

		if (plist->load_addr + plist->load_size > IRF_TBL_DATA_TOTAL_SIZE) {
			pr_err("Error: %s %d file %s addr 0x%x size %x\n",
					__func__, __LINE__, plist->file_name, plist->load_addr,
					plist->load_size);
			continue;
		}

		offset = plist->load_addr;
		buf_len = plist->load_size;
		file_size = cls_wifi_load_irf_binfile(cls_wifi_plat, radio_id, full_name,
				offset, buf_len, &version, 1);

		if (file_size > 0) {
			radio_data.data_addr = IRF_TBL_FW_BASE_ADDR + pool_offset + offset;
			radio_data.data_size = buf_len;
			radio_data.load_flag = DATA_OK;
			if (plist->data_type == IRF_DATA_RX_LEVEL)
				rx_gain_tbl_ver = version;

			if (plist->data_type >= IRF_DATA_RX_DC_OFFSET_2G_LOW &&
				plist->data_type <= IRF_DATA_RX_DC_OFFSET_5G_HIGH) {
				rx_dcoc_tbl_ver = version;
				if (rx_dcoc_tbl_ver != rx_gain_tbl_ver)
					pr_err("!!! table version not match: rx dcoc: %u rx gain: %u !!!\n",
							rx_dcoc_tbl_ver, rx_gain_tbl_ver);
			}
		} else {
			cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_id, offset, &table_head, sizeof(struct irf_tbl_head));

			radio_data.data_addr = IRF_TBL_FW_BASE_ADDR + pool_offset + offset;
			radio_data.data_size = buf_len;
			pr_err("%s load fail, file not exist.\n",plist->file_name);
			radio_data.load_flag = DATA_NOK;
		}

		if (RADIO_2P4G_INDEX == radio_id)
			memcpy(&irf_shared_addr->irf_data_2G[plist->data_type], &radio_data,
					sizeof(radio_data));
		else
			memcpy(&irf_shared_addr->irf_data_5G[plist->data_type], &radio_data,
					sizeof(radio_data));

		plist++;
	}

	/* reserved memory */
	radio_data.data_addr = IRF_TBL_FW_BASE_ADDR + pool_offset + IRF_RESERVED_ADDR_M3K;
	radio_data.data_size = IRF_RESERVED_SIZE_M3K;
	radio_data.load_flag = DATA_OK;

	if (RADIO_2P4G_INDEX == radio_id)
		memcpy(&irf_shared_addr->irf_data_2G[IRF_DATA_RESERVED], &radio_data,
					sizeof(radio_data));
	else
		memcpy(&irf_shared_addr->irf_data_5G[IRF_DATA_RESERVED], &radio_data,
					sizeof(radio_data));

	return 0;
}

static int cls_wifi_irf_outbound_load_data(struct cls_wifi_plat *cls_wifi_plat)
{
	uint32_t offset;
	int buf_len;
	struct irf_file_list *plist;
	uint32_t item;
	uint32_t i;
	uint32_t file_size;
	struct irf_data radio_data;
	pcie_shm_pool_st *shm_obj = cls_wifi_plat->pcie_shm_pools[0];
	uint32_t pool_offset;
	struct irf_share_data *irf_shared_addr = (struct irf_share_data *)(
		cls_wifi_plat->irf_tbl_virt_addr + IRF_SHARE_MEM_ADDR_M3K);

	if (pcie_shm_get_tbl_offset(shm_obj, IRF_TBL_DFT_IDX, &pool_offset)) {
		pr_err("pcie_shm_get_tbl_offset failed!\n");
		return -1;
	}

	plist = equipment_data_list;
	item = NELEMENTS(equipment_data_list);

	for(i = 0; i < item; i++) {
		offset = plist->load_addr;
		buf_len = plist->load_size;
		file_size = (u32)cls_wifi_irf_outbound_load_cfgfile(cls_wifi_plat, plist->file_name,
				offset, buf_len/sizeof(uint32_t));

		if (file_size > 0) {
			radio_data.data_addr = IRF_TBL_FW_BASE_ADDR + IRF_DATA_OFFSET +
				pool_offset + offset;
			radio_data.data_size = buf_len;
			radio_data.load_flag = DATA_OK;
			memcpy(&irf_shared_addr->irf_data_2G[plist->data_type], &radio_data,
					sizeof(radio_data));
			memcpy(&irf_shared_addr->irf_data_5G[plist->data_type], &radio_data,
					sizeof(radio_data));
		} else {
			pr_err("%s load fail.\n",plist->file_name);
		}

		plist++;
	}
	return 0;
}

int cls_wifi_irf_outbound_init(struct cls_wifi_plat *cls_wifi_plat)
{
	pcie_shm_pool_st *shm_obj = cls_wifi_plat->pcie_shm_pools[0];
	void *addr = NULL;
	int tbl_idx = IRF_TBL_DFT_IDX;
	size_t size = IRF_TBL_DATA_TOTAL_SIZE;

	addr = pcie_shm_alloc(shm_obj, tbl_idx, size);
	if (addr) {
		pr_info("irf shm_alloc: index(%d), addr(0x%px), size(%ld)\n",
				tbl_idx, addr, size);
		cls_wifi_plat->irf_tbl_virt_addr = addr;
	} else {
		pr_err("irf alloc shm failed!\n");
		return -1;
	}

	cls_wifi_irf_init_shared_data(addr + IRF_SHARE_MEM_ADDR_M3K);
	cls_wifi_irf_outbound_load_table(RADIO_2P4G_INDEX, cls_wifi_plat);
	cls_wifi_irf_outbound_load_table(RADIO_5G_INDEX, cls_wifi_plat);
	cls_wifi_irf_outbound_load_data(cls_wifi_plat);
	pcie_shmem_write_sync(shm_obj, addr, IRF_TBL_DATA_TOTAL_SIZE);

	return 0;
}
EXPORT_SYMBOL(cls_wifi_irf_outbound_init);
#endif
int32_t irf_smp_send_ram_init(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx)
{
	uint32_t irf_snd_smp_addr;
	uint32_t i;

	if (!cls_wifi_plat) {
		pr_err("%s : cls_wifi_plat is NULL\n", __func__);
		return -1;
	}

	irf_snd_smp_addr = cls_wifi_plat->if_ops->get_phy_address(cls_wifi_plat, radio_idx,
			CLS_WIFI_ADDR_IRF_SND_SMP_PHY, 0);

	if (cls_wifi_plat->hw_rev == CLS_WIFI_HW_DUBHE2000) {
		for (i = 0; i < IRF_MAX_NODE; i++)
			smp_send_ram[cls_wifi_plat->hw_rev][i].ram_base =
				irf_snd_smp_addr + IRF_RAM_BLOCK_SIZE_D2K * i;
	} else if (cls_wifi_plat->hw_rev == CLS_WIFI_HW_MERAK2000) {
		for (i = 0; i < IRF_MAX_NODE; i++)
			smp_send_ram[cls_wifi_plat->hw_rev][i].ram_base =
				irf_snd_smp_addr + IRF_RAM_BLOCK_SIZE_M2K * i;
	} else if (cls_wifi_plat->hw_rev == CLS_WIFI_HW_MERAK3000) {
#if defined(CFG_M3K_FPGA)
		for (i = 0; i < IRF_MAX_NODE; i++)
			smp_send_ram[cls_wifi_plat->hw_rev][i].ram_base =
				IRF_RAM_BASE_ADDR_M3K + IRF_RAM_BLOCK_SIZE_M3K * i;

#else
		for (i = 0; i < IRF_MAX_NODE; i++)
			smp_send_ram[cls_wifi_plat->hw_rev][i].ram_base =
				irf_snd_smp_addr + IRF_RAM_BLOCK_SIZE_M3K * i;
#endif
	}
	return 0;
}

int32_t irf_smp_send_ram_malloc(struct cls_wifi_plat *cls_wifi_plat,
		uint32_t node, uint32_t snd_smp_mod, uint32_t length, uint32_t *phy_addr)
{
	int i;
	int j;
	int k;
	uint32_t free_ram_len;

	if (!cls_wifi_plat) {
		pr_err("%s : cls_wifi_plat is NULL\n", __func__);
		return -1;
	}

	if (!phy_addr || (node >= IRF_MAX_NODE) || ((snd_smp_mod != IRF_DDR_SND_MOD) && (snd_smp_mod != IRF_DDR_SMP_MOD)
#if defined(CFG_M3K_FPGA)
		&& (snd_smp_mod != IRF_COM_SND_MOD)
#endif
	)) {
		pr_err("%s : invalid parameter, node [%u] snd_smp_mod [%u]\n", __func__, node, snd_smp_mod);
		return -1;
	}

	for (i = 0; i < IRF_MAX_NODE; i++) {
		if (smp_send_ram[cls_wifi_plat->hw_rev][i].status == IRF_RAM_BUSY)
			continue;

		free_ram_len = 0;
		j = i;
		while (free_ram_len < length) {
			free_ram_len += smp_send_ram[cls_wifi_plat->hw_rev][j].ram_size;
			if (free_ram_len >= length) {
				node_ram_map[node].ram_bitmap = 0;
				node_ram_map[node].snd_smp_mod = snd_smp_mod;
				for (k = i; k <= j; k++) {
					node_ram_map[node].ram_bitmap |= CO_BIT(k);
					smp_send_ram[cls_wifi_plat->hw_rev][k].snd_smp_mod = snd_smp_mod;
					smp_send_ram[cls_wifi_plat->hw_rev][k].status = IRF_RAM_BUSY;
				}

				pr_info("%s : node [%u] ram_bitmap [%08x] ram_base [%08x]\n", __func__, node,
						node_ram_map[node].ram_bitmap, smp_send_ram[cls_wifi_plat->hw_rev][i].ram_base);
				*phy_addr = smp_send_ram[cls_wifi_plat->hw_rev][i].ram_base;

				return 0;
			}

			if (++j >= IRF_MAX_NODE)
				return -1;

			if (smp_send_ram[cls_wifi_plat->hw_rev][j].status == IRF_RAM_BUSY)
				break;
		}
	}

	return 0;
}

int32_t irf_smp_send_ram_free(struct cls_wifi_plat *cls_wifi_plat, uint32_t node_msk)
{
	uint32_t i;
	uint32_t j;
	uint32_t node_mask_val;

	node_mask_val =  0xf & node_msk;
	if (!cls_wifi_plat) {
		pr_err("%s : cls_wifi_plat is NULL\n", __func__);
		return -1;
	}

	if (!node_mask_val)
		return -1;

	for (i = 0; i < IRF_MAX_NODE; i++) {
		if (node_mask_val & CO_BIT(i)) {
			pr_info("%s : node [%u] ram_bitmap [%08x]\n", __func__, i, node_ram_map[i].ram_bitmap);
			if ((node_ram_map[i].snd_smp_mod != IRF_DDR_SND_MOD) && (node_ram_map[i].snd_smp_mod != IRF_DDR_SMP_MOD)
#if defined(CFG_M3K_FPGA)
			&& (node_ram_map[i].snd_smp_mod != IRF_COM_SND_MOD)
#endif
			)
				continue;

			for (j = 0; j < IRF_MAX_NODE; j++) {
				if (node_ram_map[i].ram_bitmap & CO_BIT(j)) {
					smp_send_ram[cls_wifi_plat->hw_rev][j].status = IRF_RAM_IDLE;
					smp_send_ram[cls_wifi_plat->hw_rev][j].snd_smp_mod = IRF_DEF_SND_SMP_MOD;
				}
			}
			node_ram_map[i].ram_bitmap = 0;
		}
	}

	return 0;
}

int32_t irf_set_snd_smp_mod(uint32_t node, uint32_t snd_smp_mod)
{
	if (node >= IRF_MAX_NODE) {
		pr_err("%s : invalid parameter, node [%u] snd_smp_mod [%u]\n", __func__, node, snd_smp_mod);
		return -1;
	}

	node_ram_map[node].snd_smp_mod = snd_smp_mod;

	return 0;
}

int32_t irf_get_snd_smp_mod(uint32_t node, uint32_t *snd_smp_mod)
{
	if ((node >= IRF_MAX_NODE) || !snd_smp_mod) {
		pr_err("%s : invalid parameter, node [%u]\n", __func__, node);
		return -1;
	}

	*snd_smp_mod = node_ram_map[node].snd_smp_mod;

	return 0;
}

static void cls_wifi_dif_cali_save_bin_file(struct work_struct *ws)
{
	char name[FS_PATH_MAX_LEN];
	u32 offset;
	u32 len;
	struct cls_wifi_irf_file *param = container_of(ws, struct cls_wifi_irf_file,
						       dif_cali_save_bin_work);
	struct cls_wifi_hw *cls_wifi_hw = container_of(param, struct cls_wifi_hw,
						       irf_file);

	if (RADIO_2P4G_INDEX == cls_wifi_hw->radio_idx) {
		irf_get_fullname(cls_wifi_hw->plat, name, DIF_EQ_2G_DATA);
		if (cls_wifi_hw->plat && cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_DUBHE2000) {
			offset = DIF_EQ_2G_ADDR_D2K;
		} else if (cls_wifi_hw->plat &&
			   cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK2000) {
			offset = DIF_EQ_2G_ADDR_M2K;
		} else if (cls_wifi_hw->plat &&
			   cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK3000) {
			pr_err("dif cali not support for m3k!\n");
			return;
		}
	} else if (RADIO_5G_INDEX == cls_wifi_hw->radio_idx) {
		irf_get_fullname(cls_wifi_hw->plat, name, DIF_EQ_5G_DATA);
		if (cls_wifi_hw->plat && cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_DUBHE2000) {
			offset = DIF_EQ_5G_ADDR_D2K;
		} else if (cls_wifi_hw->plat &&
			   cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK2000) {
			offset = DIF_EQ_5G_ADDR_M2K;
		} else if (cls_wifi_hw->plat &&
			   cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK3000) {
			pr_err("dif cali not support for m3k!\n");
			return;
		}
	} else {
		pr_err("radio id: %d not support!\n", cls_wifi_hw->radio_idx);
		return;
	}

	spin_lock_bh(&param->dif_eq_lock);
	len = param->dif_eq_size;
	spin_unlock_bh(&param->dif_eq_lock);

	cls_wifi_save_irf_binfile_mem(cls_wifi_hw, name, offset, len);
}

static void cls_wifi_rx_dcoc_save_bin_file(struct work_struct *ws)
{
	char name[FS_PATH_MAX_LEN];
	u32 offset;
	u32 len;
	struct cls_wifi_irf_file *param = container_of(ws, struct cls_wifi_irf_file,
						       rx_dcoc_save_bin_work);
	struct cls_wifi_hw *cls_wifi_hw = container_of(param, struct cls_wifi_hw,
						       irf_file);
	int ret = -1;

	spin_lock_bh(&param->rx_dcoc_lock);
	if (RADIO_2P4G_INDEX == cls_wifi_hw->radio_idx) {
		if (param->rx_dcoc_high_temp)
			irf_get_fullname(cls_wifi_hw->plat, name, RX_DC_OFFSET_2G_HIGH_DATA);
		else
			irf_get_fullname(cls_wifi_hw->plat, name, RX_DC_OFFSET_2G_LOW_DATA);
	} else if (RADIO_5G_INDEX == cls_wifi_hw->radio_idx) {
		if (param->rx_dcoc_high_temp)
			irf_get_fullname(cls_wifi_hw->plat, name, RX_DC_OFFSET_5G_HIGH_DATA);
		else
			irf_get_fullname(cls_wifi_hw->plat, name, RX_DC_OFFSET_5G_LOW_DATA);
	} else {
		pr_err("radio id: %d not support!\n", cls_wifi_hw->radio_idx);
		return;
	}

	offset = param->rx_dcoc_offset;
	len = param->rx_dcoc_size;
	spin_unlock_bh(&param->rx_dcoc_lock);

	ret = cls_wifi_save_irf_binfile_mem(cls_wifi_hw, name, offset, len);
	if (0 == ret) {
		spin_lock_bh(&param->rx_dcoc_lock);
		dcoc_status[1] = IRF_CALI_STATUS_DONE;
		spin_unlock_bh(&param->rx_dcoc_lock);
	}
}

static void cls_wifi_tx_cali_save_bin_file(struct work_struct *ws)
{
	char name[FS_PATH_MAX_LEN];
	u32 offset;
	u32 tx_len;
	u32 fb_len;
	struct cls_wifi_irf_file *param = container_of(ws, struct cls_wifi_irf_file,
						       tx_cali_save_bin_work);
	struct cls_wifi_hw *cls_wifi_hw = container_of(param, struct cls_wifi_hw,
						       irf_file);

	spin_lock_bh(&param->tx_cali_lock);
	tx_len = param->tx_data_size;
	fb_len = param->fb_data_size;
	spin_unlock_bh(&param->tx_cali_lock);

	/* save tx fcomp table */
	if (RADIO_2P4G_INDEX == cls_wifi_hw->radio_idx) {
		irf_get_fullname(cls_wifi_hw->plat, name, TX_FCOMP_2G_DATA);
		if (cls_wifi_hw->plat && cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_DUBHE2000) {
			offset = TX_FCOMP_2G_ADDR_D2K;
		} else if (cls_wifi_hw->plat &&
			   cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK2000) {
			offset = TX_FCOMP_2G_ADDR_M2K;
		} else if (cls_wifi_hw->plat &&
			   cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK3000) {
			offset = TX_FCOMP_2G_ADDR_M3K;
		}
	} else if (RADIO_5G_INDEX == cls_wifi_hw->radio_idx) {
		irf_get_fullname(cls_wifi_hw->plat, name, TX_FCOMP_5G_DATA);
		if (cls_wifi_hw->plat && cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_DUBHE2000) {
			offset = TX_FCOMP_5G_ADDR_D2K;
		} else if (cls_wifi_hw->plat &&
			   cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK2000) {
			offset = TX_FCOMP_5G_ADDR_M2K;
		} else if (cls_wifi_hw->plat &&
			   cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK3000) {
			offset = TX_FCOMP_5G_ADDR_M3K;
		}
	} else {
		pr_err("radio id: %d not support!\n", cls_wifi_hw->radio_idx);
		return;
	}

	cls_wifi_save_irf_binfile_mem(cls_wifi_hw, name, offset, tx_len);

	/* save fb fcomp table */
	if (RADIO_2P4G_INDEX == cls_wifi_hw->radio_idx) {
		irf_get_fullname(cls_wifi_hw->plat, name, FB_FCOMP_2G_DATA);
		if (cls_wifi_hw->plat && cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_DUBHE2000) {
			offset = FB_FCOMP_2G_ADDR_D2K;
		} else if (cls_wifi_hw->plat &&
			   cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK2000) {
			offset = FB_FCOMP_2G_ADDR_M2K;
		} else if (cls_wifi_hw->plat &&
			   cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK3000) {
			offset = FB_FCOMP_2G_ADDR_M3K;
		}
	} else if (RADIO_5G_INDEX == cls_wifi_hw->radio_idx) {
		irf_get_fullname(cls_wifi_hw->plat, name, FB_FCOMP_5G_DATA);
		if (cls_wifi_hw->plat && cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_DUBHE2000) {
			offset = FB_FCOMP_5G_ADDR_D2K;
		} else if (cls_wifi_hw->plat &&
			   cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK2000) {
			offset = FB_FCOMP_5G_ADDR_M2K;
		} else if (cls_wifi_hw->plat &&
			   cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK3000) {
			offset = FB_FCOMP_5G_ADDR_M3K;
		}
	} else {
		pr_err("radio id: %d not support!\n", cls_wifi_hw->radio_idx);
		return;
	}

	cls_wifi_save_irf_binfile_mem(cls_wifi_hw, name, offset, fb_len);
}

static void cls_wifi_rx_cali_gain_lvl_save_bin_file(struct work_struct *ws)
{
	char name[FS_PATH_MAX_LEN];
	u32 offset;
	u32 len;
	struct cls_wifi_irf_file *param = container_of(ws, struct cls_wifi_irf_file,
						       rx_cali_gain_lvl_save_bin_work);
	struct cls_wifi_hw *cls_wifi_hw = container_of(param, struct cls_wifi_hw,
						       irf_file);
	int ret = -1;

	if (RADIO_2P4G_INDEX == cls_wifi_hw->radio_idx) {
		irf_get_fullname(cls_wifi_hw->plat, name, RX_LEVEL_2G_CALI_DATA);
		if (cls_wifi_hw->plat && cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_DUBHE2000) {
			offset = RX_LEVEL_2G_ADDR_D2K;
		} else if (cls_wifi_hw->plat &&
			   cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK2000) {
			offset = RX_LEVEL_2G_ADDR_M2K;
		} else if (cls_wifi_hw->plat &&
			   cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK3000) {
			offset = RX_LEVEL_2G_ADDR_M3K;
		}
	} else if (RADIO_5G_INDEX == cls_wifi_hw->radio_idx) {
		irf_get_fullname(cls_wifi_hw->plat, name, RX_LEVEL_5G_CALI_DATA);
		if (cls_wifi_hw->plat && cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_DUBHE2000) {
			offset = RX_LEVEL_5G_ADDR_D2K;
		} else if (cls_wifi_hw->plat &&
			   cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK2000) {
			offset = RX_LEVEL_5G_ADDR_M2K;
		} else if (cls_wifi_hw->plat &&
			   cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK3000) {
			offset = RX_LEVEL_5G_ADDR_M3K;
		}
	} else {
		pr_err("radio id: %d not support!\n", cls_wifi_hw->radio_idx);
		return;
	}

	spin_lock_bh(&param->rx_cali_gain_lvl_lock);
	len = param->rx_data_gain_lvl_size;
	spin_unlock_bh(&param->rx_cali_gain_lvl_lock);

	ret = cls_wifi_save_irf_binfile_mem(cls_wifi_hw, name, offset, len);
	if (0 == ret) {
		spin_lock_bh(&param->rx_cali_gain_lvl_lock);
		rx_cali_status = IRF_CALI_STATUS_DONE;
		spin_unlock_bh(&param->rx_cali_gain_lvl_lock);
	}
}

static void cls_wifi_rx_cali_fcomp_save_bin_file(struct work_struct *ws)
{
	char name[FS_PATH_MAX_LEN];
	u32 offset;
	u32 len;
	struct cls_wifi_irf_file *param = container_of(ws, struct cls_wifi_irf_file,
						       rx_cali_fcomp_save_bin_work);
	struct cls_wifi_hw *cls_wifi_hw = container_of(param, struct cls_wifi_hw,
						       irf_file);
	int ret = -1;

	if (RADIO_2P4G_INDEX == cls_wifi_hw->radio_idx) {
		irf_get_fullname(cls_wifi_hw->plat, name, RX_FCOMP_2G_DATA);
		if (cls_wifi_hw->plat && cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_DUBHE2000) {
			offset = RX_FCOMP_2G_ADDR_D2K;
		} else if (cls_wifi_hw->plat &&
			   cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK2000) {
			offset = RX_FCOMP_2G_ADDR_M2K;
		} else if (cls_wifi_hw->plat &&
			   cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK3000) {
			offset = RX_FCOMP_2G_ADDR_M3K;
		}
	} else if (RADIO_5G_INDEX == cls_wifi_hw->radio_idx) {
		irf_get_fullname(cls_wifi_hw->plat, name, RX_FCOMP_5G_DATA);
		if (cls_wifi_hw->plat && cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_DUBHE2000) {
			offset = RX_FCOMP_5G_ADDR_D2K;
		} else if (cls_wifi_hw->plat &&
			   cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK2000) {
			offset = RX_FCOMP_5G_ADDR_M2K;
		} else if (cls_wifi_hw->plat &&
			   cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK3000) {
			offset = RX_FCOMP_5G_ADDR_M3K;
		}
	} else {
		pr_err("radio id: %d not support!\n", cls_wifi_hw->radio_idx);
		return;
	}

	spin_lock_bh(&param->rx_cali_fcomp_lock);
	len = param->rx_data_freq_comp_size;
	spin_unlock_bh(&param->rx_cali_fcomp_lock);

	ret = cls_wifi_save_irf_binfile_mem(cls_wifi_hw, name, offset, len);
	if (0 == ret) {
		spin_lock_bh(&param->rx_cali_fcomp_lock);
		rx_cali_status = IRF_CALI_STATUS_DONE;
		spin_unlock_bh(&param->rx_cali_fcomp_lock);
	}
}

static void cls_wifi_tx_err_cali_save_bin_file(struct work_struct *ws)
{
	char name[FS_PATH_MAX_LEN];
	u32 offset;
	u32 len;
	struct cls_wifi_irf_file *param = container_of(ws, struct cls_wifi_irf_file,
						       tx_err_cali_save_bin_work);
	struct cls_wifi_hw *cls_wifi_hw = container_of(param, struct cls_wifi_hw,
						       irf_file);
	int ret = -1;

	/* save TX gain err table */
	if (RADIO_2P4G_INDEX == cls_wifi_hw->radio_idx) {
		irf_get_fullname(cls_wifi_hw->plat, name, TX_GAIN_ERR_2G_DATA);
		if (cls_wifi_hw->plat && cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_DUBHE2000) {
			offset = TX_GAIN_ERR_2G_ADDR_D2K;
		} else if (cls_wifi_hw->plat &&
			   cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK2000) {
			offset = TX_GAIN_ERR_2G_ADDR_M2K;
		} else if (cls_wifi_hw->plat &&
			   cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK3000) {
			offset = TX_GAIN_ERR_2G_ADDR_M3K;
		}
	} else if (RADIO_5G_INDEX == cls_wifi_hw->radio_idx) {
		irf_get_fullname(cls_wifi_hw->plat, name, TX_GAIN_ERR_5G_DATA);
		if (cls_wifi_hw->plat && cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_DUBHE2000) {
			offset = TX_GAIN_ERR_5G_ADDR_D2K;
		} else if (cls_wifi_hw->plat &&
			   cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK2000) {
			offset = TX_GAIN_ERR_5G_ADDR_M2K;
		} else if (cls_wifi_hw->plat &&
			   cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK3000) {
			offset = TX_GAIN_ERR_5G_ADDR_M3K;
		}
	} else {
		pr_err("radio id: %d not support!\n", cls_wifi_hw->radio_idx);
		return;
	}

	spin_lock_bh(&param->tx_gain_err_lock);
	len = param->tx_gain_err_data_size;
	spin_unlock_bh(&param->tx_gain_err_lock);

	ret = cls_wifi_save_irf_binfile_mem(cls_wifi_hw, name, offset, len);
	if (0 == ret) {
		spin_lock_bh(&param->tx_gain_err_lock);
		tx_err_cali_status = IRF_CALI_STATUS_DONE;
		spin_unlock_bh(&param->tx_gain_err_lock);
	}
}

static void cls_wifi_fb_err_cali_save_bin_file(struct work_struct *ws)
{
	char name[FS_PATH_MAX_LEN];
	u32 offset;
	u32 len;

	struct cls_wifi_irf_file *param = container_of(ws, struct cls_wifi_irf_file,
						       fb_err_cali_save_bin_work);
	struct cls_wifi_hw *cls_wifi_hw = container_of(param, struct cls_wifi_hw,
						       irf_file);
	int ret = -1;

	/* save FB gain err table */
	if (RADIO_2P4G_INDEX == cls_wifi_hw->radio_idx) {
		irf_get_fullname(cls_wifi_hw->plat, name, FB_GAIN_ERR_2G_DATA);
		if (cls_wifi_hw->plat && cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_DUBHE2000) {
			offset = FB_GAIN_ERR_2G_ADDR_D2K;
		} else if (cls_wifi_hw->plat &&
			   cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK2000) {
			offset = FB_GAIN_ERR_2G_ADDR_M2K;
		} else if (cls_wifi_hw->plat &&
			   cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK3000) {
			offset = FB_GAIN_ERR_2G_ADDR_M3K;
		}
	} else if (RADIO_5G_INDEX == cls_wifi_hw->radio_idx) {
		irf_get_fullname(cls_wifi_hw->plat, name, FB_GAIN_ERR_5G_DATA);
		if (cls_wifi_hw->plat && cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_DUBHE2000) {
			offset = FB_GAIN_ERR_5G_ADDR_D2K;
		} else if (cls_wifi_hw->plat &&
			   cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK2000) {
			offset = FB_GAIN_ERR_5G_ADDR_M2K;
		} else if (cls_wifi_hw->plat &&
			   cls_wifi_hw->plat->hw_rev == CLS_WIFI_HW_MERAK3000) {
			offset = FB_GAIN_ERR_5G_ADDR_M3K;
		}
	} else {
		pr_err("radio id: %d not support!\n", cls_wifi_hw->radio_idx);
		return;
	}

	spin_lock_bh(&param->fb_gain_err_lock);
	len = param->fb_gain_err_data_size;
	spin_unlock_bh(&param->fb_gain_err_lock);

	ret = cls_wifi_save_irf_binfile_mem(cls_wifi_hw, name, offset, len);
	if (0 == ret) {
		spin_lock_bh(&param->fb_gain_err_lock);
		fb_err_cali_status = IRF_CALI_STATUS_DONE;
		spin_unlock_bh(&param->fb_gain_err_lock);
	}
}

static void cls_wifi_irf_smp_save_dat_file(struct work_struct *ws)
{
	struct cls_wifi_irf_file *param = container_of(ws, struct cls_wifi_irf_file,
						       smp_save_dat_work);
	struct cls_wifi_hw *cls_wifi_hw = container_of(param, struct cls_wifi_hw,
						       irf_file);
	struct irf_smp_start_ind smp_ind;
	struct irf_smp_start_ind *ind = NULL;
#ifdef __KERNEL__
	struct cls_wifi_plat *cls_wifi_plat = cls_wifi_hw->plat;
	u32 offset;
	u32 snd_smp_mod;
	u32 i;
#else
	u8 i;
	u32 offset;
#endif

	spin_lock_bh(&param->smp_lock);
	memcpy(&smp_ind, &param->irf_smp_ind, sizeof(struct irf_smp_start_ind));
	ind = &smp_ind;
	spin_unlock_bh(&param->smp_lock);

	if (ind->status != CO_OK) {
		pr_err("IRF Sample indicates failure: %d\n", ind->status);
#ifdef __KERNEL__
		irf_smp_send_ram_free(cls_wifi_plat, ind->sel_bitmap);
		for (i = 0; i < IRF_MAX_NODE; i++) {
			if (ind->sel_bitmap & CO_BIT(i))
				irf_set_snd_smp_mod(i, IRF_DEF_SND_SMP_MOD);
		}
#endif
		return;
	}

#ifdef __KERNEL__
	if (ind->multi_phase) {
		cls_wifi_save_irf_multi_phase_configfile(cls_wifi_hw, ind);
	} else {
		for(i = 0; i < IRF_MAX_SMP_DESC_NUM; i++) {
			if (ind->sel_bitmap & CO_BIT(i)) {
				irf_get_snd_smp_mod(i, &snd_smp_mod);
				if (snd_smp_mod == IRF_COM_SMP_MOD) {
					offset = ind->smp_addr_desc[i].irf_smp_buf_addr -
							cls_wifi_plat->if_ops->get_phy_address(cls_wifi_hw->plat,
							cls_wifi_hw->radio_idx, CLS_WIFI_ADDR_IRF_PHY, 0);
				} else if (snd_smp_mod == IRF_DDR_SMP_MOD) {
					offset = ind->smp_addr_desc[i].irf_smp_buf_addr -
							cls_wifi_plat->if_ops->get_phy_address(cls_wifi_hw->plat,
							cls_wifi_hw->radio_idx, CLS_WIFI_ADDR_IRF_SND_SMP_PHY, 0);
				} else {
					continue;
				}
				/* Only for debug usage */
				printk("node : %u, ind addr: 0x%08x, offset: 0x%08x\n",
						i, ind->smp_addr_desc[i].irf_smp_buf_addr, offset);

				if (snd_smp_mod == IRF_COM_SMP_MOD) {
#if defined(CFG_M3K_FPGA)
					cls_wifi_save_irf_configfile(cls_wifi_hw,
										ind->smp_addr_desc[i].irf_smp_buf_addr,
										ind->smp_addr_desc[i].len,
										ind->smp_addr_desc[i].node,
										IRF_SND_SMP_MEM_FPGA);
#else
					if (!ind->extend_ram_flag)
						cls_wifi_save_irf_configfile(cls_wifi_hw, offset,
								ind->smp_addr_desc[i].len, ind->smp_addr_desc[i].node, IRF_RESV_MEM);
					else
						pr_warn("Extended RAM mode only for EMU test!\n");
#endif
				} else if (snd_smp_mod == IRF_DDR_SMP_MOD) {
					cls_wifi_save_irf_configfile(cls_wifi_hw, offset,
							ind->smp_addr_desc[i].len, ind->smp_addr_desc[i].node, IRF_SND_SMP_MEM);
				}
			}
		}

		irf_smp_send_ram_free(cls_wifi_plat, ind->sel_bitmap);
		for (i = 0; i < IRF_MAX_NODE; i++) {
			if (ind->sel_bitmap & CO_BIT(i))
				irf_set_snd_smp_mod(i, IRF_DEF_SND_SMP_MOD);
		}
	}
#else
	for(i = 0; i < IRF_MAX_NODE; i++) {
		if (ind->sel_bitmap & CO_BIT(i)) {
			offset = ind->smp_addr_desc[i].irf_smp_buf_addr - IRF_DATA_BASE;
			/* Only for debug usage */
			pr_warn("ind node: 0x%d, addr: 0x%08x, offset: 0x%08x, len:0x%x\n",ind->smp_addr_desc[i].node,
					ind->smp_addr_desc[i].irf_smp_buf_addr, offset,ind->smp_addr_desc[i].len);
		}
	}
#endif
}

void cls_wifi_irf_save_work_init(struct cls_wifi_hw *cls_wifi_hw)
{
	struct cls_wifi_irf_file *irf_file = &cls_wifi_hw->irf_file;

	spin_lock_init(&irf_file->smp_lock);
	spin_lock_init(&irf_file->dif_eq_lock);
	spin_lock_init(&irf_file->rx_dcoc_lock);
	spin_lock_init(&irf_file->tx_cali_lock);
	spin_lock_init(&irf_file->rx_cali_gain_lvl_lock);
	spin_lock_init(&irf_file->rx_cali_fcomp_lock);
	spin_lock_init(&irf_file->tx_gain_err_lock);
	spin_lock_init(&irf_file->fb_gain_err_lock);

	INIT_WORK(&irf_file->smp_save_dat_work, cls_wifi_irf_smp_save_dat_file);
	INIT_WORK(&irf_file->dif_cali_save_bin_work, cls_wifi_dif_cali_save_bin_file);
	INIT_WORK(&irf_file->rx_dcoc_save_bin_work, cls_wifi_rx_dcoc_save_bin_file);
	INIT_WORK(&irf_file->tx_cali_save_bin_work, cls_wifi_tx_cali_save_bin_file);
	INIT_WORK(&irf_file->rx_cali_gain_lvl_save_bin_work, cls_wifi_rx_cali_gain_lvl_save_bin_file);
	INIT_WORK(&irf_file->rx_cali_fcomp_save_bin_work, cls_wifi_rx_cali_fcomp_save_bin_file);
	INIT_WORK(&irf_file->tx_err_cali_save_bin_work, cls_wifi_tx_err_cali_save_bin_file);
	INIT_WORK(&irf_file->fb_err_cali_save_bin_work, cls_wifi_fb_err_cali_save_bin_file);
}
EXPORT_SYMBOL(cls_wifi_irf_save_work_init);

void cls_wifi_irf_save_work_deinit(struct cls_wifi_hw *cls_wifi_hw)
{
	struct cls_wifi_irf_file *irf_file = &cls_wifi_hw->irf_file;

	cancel_work_sync(&irf_file->smp_save_dat_work);
	cancel_work_sync(&irf_file->dif_cali_save_bin_work);
	cancel_work_sync(&irf_file->rx_dcoc_save_bin_work);
	cancel_work_sync(&irf_file->tx_cali_save_bin_work);
	cancel_work_sync(&irf_file->rx_cali_gain_lvl_save_bin_work);
	cancel_work_sync(&irf_file->rx_cali_fcomp_save_bin_work);
	cancel_work_sync(&irf_file->tx_err_cali_save_bin_work);
	cancel_work_sync(&irf_file->fb_err_cali_save_bin_work);
}
EXPORT_SYMBOL(cls_wifi_irf_save_work_deinit);

u8 irf_get_curr_bw(struct cls_wifi_hw *cls_wifi_hw)
{
	struct cls_wifi_chanctx *ctxt;
	u8 bw;

	/* If no information on current channel do nothing */
	if (!cls_wifi_chanctx_valid(cls_wifi_hw, cls_wifi_hw->cur_chanctx))
		return PHY_CHNL_BW_OTHER;

	ctxt = &cls_wifi_hw->chanctx_table[cls_wifi_hw->cur_chanctx];

	switch (ctxt->chan_def.width) {
	case NL80211_CHAN_WIDTH_20_NOHT:
	case NL80211_CHAN_WIDTH_20:
		bw = PHY_CHNL_BW_20;
		break;
	case NL80211_CHAN_WIDTH_40:
		bw = PHY_CHNL_BW_40;
		break;
	case NL80211_CHAN_WIDTH_80:
		bw = PHY_CHNL_BW_80;
		break;
	case NL80211_CHAN_WIDTH_160:
		bw = PHY_CHNL_BW_160;
		break;
	case NL80211_CHAN_WIDTH_80P80:
		bw = PHY_CHNL_BW_80P80;
		break;
	default:
		bw = PHY_CHNL_BW_OTHER;
		break;
	}

	return bw;
}

static void cls_wifi_csa_delay_cali_work(struct work_struct *ws)
{
	struct cls_wifi_hw *cls_wifi_hw = container_of(ws, struct cls_wifi_hw, csa_delay_cali_work);
	struct cls_wifi_vif *vif, *curr_vif;

	cls_wifi_dif_boot_cali(cls_wifi_hw);
	list_for_each_entry(vif, &cls_wifi_hw->vifs, list) {
		if (vif->up) {
			curr_vif = vif;
			cls_wifi_txq_vif_start(vif, CLS_WIFI_TXQ_STOP_CHAN, cls_wifi_hw);
		}
	}
}

void cls_wifi_csa_delay_cali_init(struct cls_wifi_hw *cls_wifi_hw)
{
	INIT_WORK(&cls_wifi_hw->csa_delay_cali_work, cls_wifi_csa_delay_cali_work);
}
EXPORT_SYMBOL(cls_wifi_csa_delay_cali_init);

void cls_wifi_csa_delay_cali_deinit(struct cls_wifi_hw *cls_wifi_hw)
{
	if (cls_wifi_hw->csa_delay_cali_work.func)
		cancel_work_sync(&cls_wifi_hw->csa_delay_cali_work);
}
EXPORT_SYMBOL(cls_wifi_csa_delay_cali_deinit);

int irf_load_rx_gain_dcoc_tbl(uint32_t radio_id, uint32_t addr, struct cls_wifi_plat *cls_wifi_plat, int msg_flag)
{
	uint32_t offset;
	int buf_len;
	struct irf_file_list *plist;
	struct irf_file_list rx_gain_lvl_tbl_info;
	uint32_t item;
	uint32_t i;
	uint32_t file_size;
	struct irf_data radio_data;
	uint8_t rx_gain_tbl_ver = 0;
	uint8_t rx_dcoc_tbl_ver = 0;
	uint8_t version = 0;
	struct irf_tbl_head table_head = {0};
	uint32_t irf_mem_addr;
	char full_name[FS_PATH_MAX_LEN];

	rx_gain_lvl_tbl_info.path_type = PATH_TYPE_CAL;
	rx_gain_lvl_tbl_info.data_type = IRF_DATA_RX_LEVEL;
	if (cls_wifi_plat && cls_wifi_plat->hw_rev == CLS_WIFI_HW_DUBHE2000) {
		irf_mem_addr =  IRF_SHARE_MEM_ADDR_D2K;
		if (radio_id == RADIO_2P4G_INDEX) {
			rx_gain_lvl_tbl_info.file_name = RX_LEVEL_2G_DATA_FIL_NAM;
			rx_gain_lvl_tbl_info.load_addr = RX_LEVEL_2G_ADDR_D2K;
			rx_gain_lvl_tbl_info.load_size = RX_LEVEL_2G_SIZE_D2K;
			plist = rx_dcoc_table_2g_list[cls_wifi_plat->hw_rev];
			item = NELEMENTS(rx_dcoc_table_2g_list[cls_wifi_plat->hw_rev]);
		} else {
			rx_gain_lvl_tbl_info.file_name = RX_LEVEL_5G_DATA_FIL_NAM;
			rx_gain_lvl_tbl_info.load_addr = RX_LEVEL_5G_ADDR_D2K;
			rx_gain_lvl_tbl_info.load_size = RX_LEVEL_5G_SIZE_D2K;
			plist = rx_dcoc_table_5g_list[cls_wifi_plat->hw_rev];
			item = NELEMENTS(rx_dcoc_table_5g_list[cls_wifi_plat->hw_rev]);
		}
	} else if (cls_wifi_plat && cls_wifi_plat->hw_rev == CLS_WIFI_HW_MERAK2000) {
		irf_mem_addr =  IRF_SHARE_MEM_ADDR_M2K;
		if (radio_id == RADIO_2P4G_INDEX) {
			rx_gain_lvl_tbl_info.file_name = RX_LEVEL_2G_DATA_FIL_NAM;
			rx_gain_lvl_tbl_info.load_addr = RX_LEVEL_2G_ADDR_M2K;
			rx_gain_lvl_tbl_info.load_size = RX_LEVEL_2G_SIZE_M2K;
			plist = rx_dcoc_table_2g_list[cls_wifi_plat->hw_rev];
			item = NELEMENTS(rx_dcoc_table_2g_list[cls_wifi_plat->hw_rev]);
		} else {
			rx_gain_lvl_tbl_info.file_name = RX_LEVEL_5G_DATA_FIL_NAM;
			rx_gain_lvl_tbl_info.load_addr = RX_LEVEL_5G_ADDR_M2K;
			rx_gain_lvl_tbl_info.load_size = RX_LEVEL_5G_SIZE_M2K;
			plist = rx_dcoc_table_5g_list[cls_wifi_plat->hw_rev];
			item = NELEMENTS(rx_dcoc_table_5g_list[cls_wifi_plat->hw_rev]);
		}
	} else if (cls_wifi_plat && cls_wifi_plat->hw_rev == CLS_WIFI_HW_MERAK3000) {
		irf_mem_addr =  IRF_SHARE_MEM_ADDR_M3K;
		if (RADIO_2P4G_INDEX == radio_id) {
			rx_gain_lvl_tbl_info.file_name = RX_LEVEL_2G_DATA_FIL_NAM;
			rx_gain_lvl_tbl_info.data_type = IRF_DATA_RX_LEVEL;
			rx_gain_lvl_tbl_info.load_addr = RX_LEVEL_2G_ADDR_M3K;
			rx_gain_lvl_tbl_info.load_size = RX_LEVEL_2G_SIZE_M3K;
			plist = rx_dcoc_table_2g_list[cls_wifi_plat->hw_rev];
			item = NELEMENTS(rx_dcoc_table_2g_list[cls_wifi_plat->hw_rev]);
		} else {
			rx_gain_lvl_tbl_info.file_name = RX_LEVEL_5G_DATA_FIL_NAM;
			rx_gain_lvl_tbl_info.data_type = IRF_DATA_RX_LEVEL;
			rx_gain_lvl_tbl_info.load_addr = RX_LEVEL_5G_ADDR_M3K;
			rx_gain_lvl_tbl_info.load_size = RX_LEVEL_5G_SIZE_M3K;
			plist = rx_dcoc_table_5g_list[cls_wifi_plat->hw_rev];
			item = NELEMENTS(rx_dcoc_table_5g_list[cls_wifi_plat->hw_rev]);
		}
	}

	/* load rx gain table first, then rxdcoc cali table. */
	if ((cls_wifi_plat->hw_rev == CLS_WIFI_HW_MERAK2000) ||
			(cls_wifi_plat->hw_rev == CLS_WIFI_HW_MERAK3000)) {
		if (rx_gain_lvl_tbl_info.load_addr + rx_gain_lvl_tbl_info.load_size
				> irf_mem_addr) {
			pr_err("Error: %s %d file %s addr 0x%x size %x max 0x%x\n",
				__func__, __LINE__, rx_gain_lvl_tbl_info.file_name,
				rx_gain_lvl_tbl_info.load_addr, rx_gain_lvl_tbl_info.load_size,
				irf_mem_addr);
			return -1;
		}
	}

	offset = rx_gain_lvl_tbl_info.load_addr;
	buf_len = rx_gain_lvl_tbl_info.load_size;
	radio_data.data_addr = addr + offset;
	radio_data.data_size = buf_len;
	irf_get_fullname(cls_wifi_plat, full_name, rx_gain_lvl_tbl_info.path_type,
			rx_gain_lvl_tbl_info.file_name);
	if (cls_wifi_mod_params.load_rx_cali_tbl &&
			(cls_wifi_load_irf_binfile(cls_wifi_plat, radio_id,
			full_name, offset, buf_len, &version, msg_flag) > 0)) {
		radio_data.load_flag = DATA_OK;
		rx_gain_tbl_ver = version;
	} else {
		if (cls_wifi_mod_params.load_rx_cali_tbl)
			pr_err("%s load fail, load original rx gain level table.\n", full_name);

		rx_gain_lvl_tbl_info.path_type = PATH_TYPE_IRF;
		irf_get_fullname(cls_wifi_plat, full_name, rx_gain_lvl_tbl_info.path_type,
				rx_gain_lvl_tbl_info.file_name);
		file_size = cls_wifi_load_irf_binfile(cls_wifi_plat, radio_id, full_name,
				offset, buf_len, &version, msg_flag);
		if (file_size > 0) {
			radio_data.load_flag = DATA_OK;
			rx_gain_tbl_ver = version;
		} else {
			pr_err("%s load fail, file not exist.\n", full_name);
			cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_id, offset,
					&table_head, sizeof(struct irf_tbl_head));
			radio_data.load_flag = DATA_NOK;
		}
	}

	if (radio_id == RADIO_2P4G_INDEX) {
		cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_id,
				irf_mem_addr + offsetof(struct irf_share_data,
				irf_data_2G[rx_gain_lvl_tbl_info.data_type]),
				&radio_data, sizeof(radio_data));
	} else {
		cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_id,
				irf_mem_addr + offsetof(struct irf_share_data,
				irf_data_5G[rx_gain_lvl_tbl_info.data_type]),
				&radio_data, sizeof(radio_data));
	}

	for (i = 0; i < item; i++) {
		if ((cls_wifi_plat->hw_rev == CLS_WIFI_HW_MERAK2000) ||
				(cls_wifi_plat->hw_rev == CLS_WIFI_HW_MERAK3000)) {
			if (plist->load_addr + plist->load_size > irf_mem_addr) {
				pr_err("Error: %s %d file %s addr 0x%x size %x max 0x%x\n",
						__func__, __LINE__, plist->file_name,
						plist->load_addr, plist->load_size, irf_mem_addr);
				continue;
			}
		}

		irf_get_fullname(cls_wifi_plat, full_name, plist->path_type, plist->file_name);
		offset = plist->load_addr;
		buf_len = plist->load_size;
		radio_data.data_addr = addr + offset;
		radio_data.data_size = buf_len;
		file_size = cls_wifi_load_irf_binfile(cls_wifi_plat, radio_id, full_name,
				offset, buf_len, &version, msg_flag);
		if (file_size > 0) {
			radio_data.load_flag = DATA_OK;
			rx_dcoc_tbl_ver = version;
			if (rx_dcoc_tbl_ver != rx_gain_tbl_ver)
				pr_err("!!! table version not match: rx dcoc: %u rx gain: %u !!!\n",
						rx_dcoc_tbl_ver, rx_gain_tbl_ver);
		} else {
			cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_id, offset,
					&table_head, sizeof(struct irf_tbl_head));

			pr_err("%s load fail, file not exist.\n", full_name);
			radio_data.load_flag = DATA_NOK;
		}
		if (radio_id == RADIO_2P4G_INDEX) {
			cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_id,
					irf_mem_addr + offsetof(struct irf_share_data,
					irf_data_2G[plist->data_type]),
					&radio_data, sizeof(radio_data));
		} else {
			cls_wifi_plat->ep_ops->irf_table_writen(cls_wifi_plat, radio_id,
					irf_mem_addr + offsetof(struct irf_share_data,
					irf_data_5G[plist->data_type]),
					&radio_data, sizeof(radio_data));
		}
		plist++;
	}

	return 0;
}
