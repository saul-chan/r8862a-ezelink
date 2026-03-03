/****************************************************************************
*
* Copyright (c) 2023  Clourney Semiconductor Co.,Ltd.
*
* Permission to use, copy, modify, and/or distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
* WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
* SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
* RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
* NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
* USE OR PERFORMANCE OF THIS SOFTWARE.
*
*****************************************************************************/

#ifndef CLS_DUT_COMMON_H_
#define CLS_DUT_COMMON_H_

#include "csigma_common.h"
#include "cls/clsapi.h"
#include "cls_cmd_parser.h"
#include "common/list.h"
#include <libubus.h>

#define	SM(_v, _f)	(((_v) << _f##_S) & _f)
#define	MS(_v, _f)	(((_v) & _f) >> _f##_S)

#define CLS_MAP_MAX_BUF	(3*1024)
#define CLS_MAX_TLV_BUF	(2*1024+512)
#define CLS_MAX_CMD_BUF	(1024)

#define CLS_MAX_BSS 8
#define CLS_MAX_ETHER_STRING sizeof("xxx.xxx.xxx.xxx")
#define CLS_MAX_PKHASH_LEN 65
#define CLS_NUMS_NEIGH_REPORT 32

/* The HE PHY provides support for 3.2 μs (1x), 6.4 μs (2x), and 12.8 μs (4x) HE-LTF durations. */
#define CLS_HE_LTF_1 1
#define CLS_HE_LTF_2 2
#define CLS_HE_LTF_4 4

#define PHY_TYPE_HT 7
#define PHY_TYPE_VHT 9

#define DEAFULT_BEACON_INTERVAL 100

#ifdef APPEND_CMD
#undef APPEND_CMD
#endif
#define APPEND_CMD(_cmd, _len, _format, ...) do { \
size_t len = strlen(_cmd); \
char *pos = (char *)(_cmd) + len; \
if (len + 1 < _len) \
	snprintf(pos, _len - len, _format, ##__VA_ARGS__); \
} while (0)

#define BUILD_MBO_CMD_HEAD(method) do { \
APPEND_CMD(cmd, sizeof(cmd), "%s", method); \
} while (0)

#define IS_EMPTY_PARAM(_param) (memcmp(_param, "\x00", 1) == 0)

#define MACCPY(dst,src)  memcpy((dst), (src), MACLEN)
#define MACFMT  "%02x:%02x:%02x:%02x:%02x:%02x"
#define MACARG(src) (src)[0], (src)[1], (src)[2], (src)[3], (src)[4], (src)[5]

struct cls_npu_config {
	char br_ipaddr[CLS_MAX_ETHER_STRING];
	unsigned char al_macaddr[6];
	char ssh_cli[48];
	int npu_topology;
};

enum cls_dut_band_index {
	CLS_DUT_2P4G_BAND = 0,	/*2.4G band*/
	CLS_DUT_5G_BAND = 1,		/*5G band*/
	CLS_DUT_6G_BAND = 2,		/*6G band*/
	CLS_DUT_BANDS_NUM
};

#define CLS_DUT_TAG_NUM	4
enum cls_dut_tag_index {
	CLS_DUT_FIRST_TAG = 1,
	CLS_DUT_SECOND_TAG = 2,
	CLS_DUT_THIRD_TAG = 3,
	CLS_DUT_FOURTH_TAG = 4,
	CLS_DUT_UNKNOWN_TAG
};

#ifndef	IFNAMSIZ
#define	IFNAMSIZ	16
#endif
struct radio_band {
	unsigned int radio;
	unsigned int band;
	char ifname[IFNAMSIZ];
};

#define CLS_MAX_BUF_LEN		512

struct cls_dut_dpp_config {
	unsigned short bs_method;
	unsigned short config_id;
	unsigned short role;
	unsigned short peer_bootstrap;
	unsigned short local_bootstrap;
	//struct clsapi_dpp_bss_config *bss_conf;
	char peer_uri[CLS_MAX_BUF_LEN];
	char local_uri[CLS_MAX_BUF_LEN];
	char tcp_addr[CLS_MAX_ETHER_STRING];
};

struct cls_dut_config {
	char ifname[8];
	unsigned char testbed_enable;
	unsigned char dut_enable;
	unsigned char bws_enable;
	unsigned char bws_dynamic;
	unsigned char force_rts;
	unsigned char update_settings;
	unsigned char use_ul_mumimo;
	char sta_ip[CLS_MAX_ETHER_STRING];
	char sta_mask[CLS_MAX_ETHER_STRING];
	char sta_dhcp_enabled;
	char cert_prog[16];
	struct cls_dut_dpp_config *dpp_config;
};

struct cls_non_pref_entry {
	uint8_t valid;
	char sta_mac[MAC_ADDR_STR_LEN];
	uint8_t opclass;
	uint8_t channel;
	uint8_t pref;
	uint8_t reason;
};

struct cls_neigh_report_entry {
	struct list_head l;
	struct cls_neigh_report_info neigh;
};

struct cls_ap_btm_req {
	char sta_mac[MAC_ADDR_STR_LEN];
	uint16_t disassoc_timer;
	uint8_t disassoc_imnt;
	uint8_t validity_interval;
	uint8_t mbo_reason;
	uint8_t cell_pref;
	uint8_t nums_neigh;
	struct cls_neigh_report_info neighs[CLS_NUMS_NEIGH_REPORT];
	uint8_t subel_len;
	uint8_t subels[0];
};

struct ubus_subscriber_ctx {
	struct ubus_subscriber suscriber;
	uint32_t id;
};

void cls_dut_backup_num_snd_dim(int num_snd_dim, enum cls_dut_band_index band_idx);
int cls_dut_restore_num_snd_dim(enum cls_dut_band_index band_idx);
void cls_dut_set_reg_domain_global_flag(int enable);
int cls_dut_get_reg_domain_global_flag(void);
void cls_dut_set_configure_ie(enum cls_dut_band_index band_idx);
int cls_dut_get_configure_ie(enum cls_dut_band_index band_idx);
void cls_dut_clear_configure_ie(enum cls_dut_band_index band_idx);
void cls_dut_set_mbo_auto_candidate_flag(int enable);
int cls_dut_get_mbo_auto_candidate_flag(void);

void cls_dut_reset_config(struct cls_dut_config *conf);
struct cls_dut_config * cls_dut_get_config(const char* ifname);

void cls_dut_make_response_none(int tag, int status, int err_code, int *out_len,
	unsigned char *out_buf);

void cls_dut_make_response_macaddr(int tag, int status, int err_code, const unsigned char *macaddr,
	int *out_len, unsigned char *out_buf);

void cls_dut_make_response_pmk(int tag, int status, int err_code,
	const unsigned char *pmk, int *out_len, unsigned char *out_buf);

void cls_dut_make_response_vendor_info(int tag, int status, int err_code,
	const char *vendor_info, int *out_len, unsigned char *out_buf);
void cls_dut_make_response_str(int tag, int status, int err_code, char *res,
					int res_len, int *out_len, unsigned char *out_buf);
void cls_dut_make_response_mid(int tag, int status, int err_code, char *mid,
	int *out_len, unsigned char *out_buf);
int cls_parse_mac(const char *mac_str, unsigned char *mac);

void cls_set_rts_settings(const char *ifname, struct cls_dut_config *conf);
int cls_set_mu_enable(const char *ifname, int enable);
void cls_set_sigma_interface(const char *interface, enum cls_dut_band_index band_index);
int cls_get_sigma_band_info_from_interface(const char *ifname);
void cls_set_sigma_first_active_band_idx(enum cls_dut_band_index band_index);
int cls_get_sigma_first_active_band_idx(void);
void cls_set_sigma_tag_interface_map(const char *interface, enum cls_dut_tag_index tag_index);
void cls_clear_sigma_tag_interface_map(void);
//int cls_get_sigma_interface_band_idx(enum clsapi_freq_band band);
//char *cls_get_sigma_interface_name(enum clsapi_freq_band band);
int cls_get_sigma_interface_band_idx_from_cmd(const char *cmd);
void cls_clear_sigma_radio_band_info(void);
//const int cls_get_sigma_verify_channel_for_band(int band_idx, clsapi_unsigned_int channel_value);
const int cls_is_dcdc_configuration(void);
const int cls_get_sigma_default_channel(const char *ifname);
const char* cls_get_sigma_interface();
const char *cls_get_sigma_sta_interface(void);
const char* cls_get_sigma_interface_for_band(enum cls_dut_band_index band_index);
const char *cls_get_sigma_tag_interface_map(enum cls_dut_tag_index tag_index);
void cls_set_sigma_vap_interface(const char *ifname,
	unsigned int vap_index, enum cls_dut_band_index band_index);
const char* cls_get_sigma_vap_interface(unsigned vap_index, enum cls_dut_band_index band_index);
const char *cls_get_sigma_interface_name_from_cmd(const char *cmd, unsigned int vap_index);
void cls_set_sigma_interface_radio_band_info(enum cls_dut_band_index band_index, int radio_id, enum clsapi_freq_band band, const char *ifname);
int cls_get_sigma_interface_band_idx(enum clsapi_freq_band band);
char *cls_get_sigma_interface_name(enum clsapi_freq_band band);
const int cls_get_sigma_verify_channel_for_band(int band_idx, clsapi_unsigned_int channel_value);
void cls_init_sigma_interface(void);
void cls_clear_sigma_interface(enum cls_dut_band_index band_index);
void cls_clear_sigma_interfaces(void);
void cls_init_sigma_interfaces(void);
void cls_clear_neigh_list();
int cls_set_rf_enable(int enable);
int cls_set_rf_enable_timeout(int enable, int timeout_secs);
int cls_dut_sigma_single_band(void);
int cls_dut_sigma_dual_band(void);
int cls_dut_sigma_tri_band(void);
int cls_wifi_get_rf_chipid(unsigned int radio_id, int *chipid);
int set_tx_bandwidth(const char *ifname, unsigned bandwidth, const char *phy_mode);
void cls_set_ampdu(const char* ifname, int enable);
int cls_set_amsdu(const char *ifname, int enable);
int cls_set_fixed_bw(const char *ifname, int bw);
int cls_set_phy_mode(const char *ifname, const char *phy_mode);
int cls_set_nss_cap(const char *ifname, int mimo_type, unsigned int nss);
int cls_set_mcs_cap(const char *ifname, unsigned int mcs_set);
int cls_set_fixed_mcs_rate(const char *ifname, int tx_rate, const char *phy_mode);
int cls_set_fixed_tx_mcs_nss(const char *ifname, int num_ss, int mcs, const char *phy_mode);
int cls_set_nss_mcs_opt(const char *ifname, const char *nss_mcs_opt, const char *phy_mode);
int cls_is_2p4_interface(const char *ifname);
int cls_is_11ax_mode(const char *ifname);
int cls_set_nss_mcs_cap(const char *ifname, int nss_cap, int mcs_low, int mcs_high);
int cls_set_he_ltf_gi(const char *ifname, char *he_ltf_str, char *he_gi_str, int ofdma_val);
int cls_dut_disable_mu_bf(const char *ifname);
#ifdef QDOCK2
int cls_mbo_set_assoc_disallow(char *mac_str, int disable);
int cls_mbo_set_disassoc_imm(int disassoc_imm);
int cls_mbo_get_disassoc_imm(void);
#endif
void cls_add_neighbor(struct list_head *list, char *neigh_bssid, struct cls_neigh_report_entry **item);
uint8_t cls_get_candidates(struct cls_neigh_report_info * neigh, struct list_head * candidates, uint8_t check_nonpref);
void cls_clear_mbo_listen();
int cls_add_mbo_listen(char *ifname);
void cls_handle_dev_send_frame(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out);
#endif				/* CLS_DUT_COMMON_H_ */
