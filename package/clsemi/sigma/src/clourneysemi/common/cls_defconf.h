#ifndef CLS_DEFCONF_H_
#define CLS_DEFCONF_H_

#define CLS_DEFCONF_CMDBUF_LEN          512

enum {
	DEFAULT_HT_CHANNEL  = 11,
	DEFAULT_VHT_CHANNEL = 36,
	DEFAULT_VHT_CHANNEL_DCDC_0 = 100,
	DEFAULT_VHT_CHANNEL_DCDC_1 = 36,
	DEFAULT_MAP_HT_CHANNEL = 6,
	DEFAULT_MAP_VHT_CHANNEL = 36,
	DEFAULT_MAP_6G_CHANNEL = 37,
	DEFAULT_HT_DPP_PKEX_CHANNEL = 6,
	DEFAULT_VHT_DPP_PKEX_CHANNEL = 44,
	DEFAULT_VHT_DPP_PKEX_CHANNEL_DCDC_0 = 149,
	DEFAULT_VHT_DPP_PKEX_CHANNEL_DCDC_1 = 44
};

int cls_defconf_mbo_dut_ap_all(void);
int cls_defconf_he_testbed_sta(const char* ifname);
int cls_defconf_he_testbed_ap(const char* ifname);
int cls_defconf_he_dut_sta(const char* ifname);
int cls_defconf_he_dut_ap_all(void);
int cls_defconf_he_dut_ap_radio(int radio_id);
int cls_defconf_vht_testbed_sta(const char* ifname);
int cls_defconf_vht_testbed_ap(const char* ifname);
int cls_defconf_vht_dut_sta(const char* ifname);
int cls_defconf_vht_dut_ap(const char* ifname);
int cls_defconf_vht_dut_ap_all(void);
int cls_defconf_pmf_dut(const char* ifname);
int cls_defconf_hs2_dut(const char* ifname);
int cls_defconf_hs2_dut_all(void);
int cls_defconf_11n_dut(const char* ifname, const char *phy_mode);
int cls_defconf_11n_testbed(const char* ifname);
int cls_defconf_wpa3_dut_ap(const char *ifname);
int cls_defconf_wpa3_dut_sta(const char *ifname);
int cls_defconf_dpp(const char *ifname);
int cls_defconf_ffd(const char *ifname);
int cls_check_radio_available(uint8_t radio_idx, char *ifname, size_t ifname_len, uint32_t *p_band);
int cls_defconf_easymesh(const char *ifname, int channel);

#endif /* CLS_DEFCONF_H_ */
