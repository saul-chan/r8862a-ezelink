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

#ifndef _CLS_WIFI_CFGFILE_H_
#define _CLS_WIFI_CFGFILE_H_

/*
 * Structure used to retrieve information from the Config file used at Initialization time
 */
struct cls_wifi_conf_file {
	u8 mac_addr[ETH_ALEN];
};

int cls_wifi_parse_configfile(struct cls_wifi_hw *cls_wifi_hw, const char *filename,
		struct cls_wifi_conf_file *config);
int cls_wifi_load_irf_configfile(struct cls_wifi_hw *cls_wifi_hw, bool full_path, char *src_fname, u32 offset,
						u32 len, u32 mem_typ, bool cfr_data_flag);
void cls_wifi_save_irf_configfile(struct cls_wifi_hw *cls_wifi_hw, u32 offset, u32 len,
		u32 data_mux, u32 mem_typ);
void cls_wifi_save_irf_multi_phase_configfile(struct cls_wifi_hw *cls_wifi_hw,
		struct irf_smp_start_ind *ind);

int cls_wifi_save_irf_binfile(char *name,u32 *buf, u32 len);
int cls_wifi_save_irf_binfile_mem(struct cls_wifi_hw *cls_wifi_hw, char *name, u32 offset, u32 len);
int cls_wifi_load_irf_binfile(struct cls_wifi_plat *cls_wifi_plat, u8 radio_index,
		char *name, u32 offset, u32 len, u8 *version, int msg_flag);

int cls_wifi_load_irf_cfr_data(struct cls_wifi_hw *cls_wifi_hw, bool full_path, int path_type,
		char *src_fname, u32 offset, u32 len, u32 mem_typ, bool cfr_data_flag);
int cls_wifi_get_cal_chip_id(const char *name, u32 id[5]);

#endif /* _CLS_WIFI_CFGFILE_H_ */
