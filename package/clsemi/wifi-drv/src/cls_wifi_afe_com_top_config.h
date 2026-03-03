#ifndef __COM_TOP_CFG_H__
#define __COM_TOP_CFG_H__

#include "cls_wifi_platform.h"

/*
2'b00:  svt sensor array
2'b01:  lvt sensor array
2'b10: ulvt sensor array
2'b11: IO18 sensor array
*/
enum psensorE {
	PSENSOR_SVT,
	PSENSOR_LVT,
	PSENSOR_ULVT,
	PSENSOR_IO18,
	PS_ARRAY_TYPE_NUM
};

#define PS_SEN_SEL_NUM 8

typedef struct {
	uint32_t psensor_sen_sel_idx[PS_SEN_SEL_NUM];
	uint32_t N_mode_val[PS_SEN_SEL_NUM];
	uint32_t P_mode_val[PS_SEN_SEL_NUM];
	uint32_t N_value;
	uint32_t P_value;
} psensor_info_T;

extern int afe_com_top_bandgap_config(void);
extern int afe_check_done_flag(void __iomem *base, uint32_t reg_addr, uint32_t lsb, uint32_t mask, uint32_t delay);
extern int afe_com_top_rctune_config(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index);
extern int afe_com_top_rtune_ibias_config(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index);
extern int afe_com_top_psensor_config(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index);
#endif
