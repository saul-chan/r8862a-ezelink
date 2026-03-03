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

#include <linux/kernel.h>
#include <linux/kmod.h>
#include <linux/debugfs.h>
#include <linux/string.h>


#include "cls_wifi_debugfs.h"
#include "cls_wifi_msg_tx.h"
#include "cls_wifi_tx.h"
#include "cls_wifi_version.h"
#include "cali_struct.h"
#include "cali_rxstats.h"
#include "cls_wifi_cali.h"
#include "cls_wifi_cali_debugfs.h"
#include "cls_wifi_soc.h"
#include "cls_wifi_irf.h"
#include "cls_wifi_dif_sm.h"
#include "cls_wifi_csi.h"
#endif
#define CALI_CONFIG_FW_NAME			 "/tmp/cali_def_config.ini"

//0: 2.4G, 1: 5G
char radio_list[][8] = {"2G", "5G"};

// 0: "11b" / 1: "11a" / 2: "11g" / 3: "11n_2g" / 4: "11an_5g" / 5: "11ac" / 6: "11be"
#if defined(CFG_MERAK3000)
char phymode_list[][16] = {"11b", "11a", "11g", "11n_2g", "11n_5g", "11ac", "11be"};
#else
char phymode_list[][16] = {"11b", "11a", "11g", "11n_2g", "11n_5g", "11ac"};
#endif


//0: none / 1: WEP / 2: TKIP / 3: CCMP / 4: WAPI / 5: GCMP
char ciper_list[][16] = {"none", "wep", "tkip", "ccmp", "wapi", "gcmp",
			"bip-cmac-128", "bip-cmac-256", "wapi-gcm", "sft-gcmp",
			"bip-gmac-128", "bip-gmac-256"};

//0: "AP" / 1:"STA"
char mode_list[][8] = {"AP", "STA"};

//0: NON-HT / 1: NON-HT-DUP-OFDM / 2: HT-MF / 3: HT-GF
//4: VHT / 5: HE-SU / 6: HE-MU / 7: HE-ER-SU / 8: HE-TB
char ppdu_type_list[][16] = {"NON-HT", "NON-HT-DUP-OFDM", "HT-MF", "HT-GF", "VHT", "HE-SU", "HE-MU",
#if defined(CFG_MERAK3000)
					"HE-ER-SU", "HE-TB", "EHT-MU", "EHT-TB"};
#else
					"HE-ER-SU", "HE-TB"};
#endif

#if defined(CFG_MERAK3000)
//0: EHT-MU-SU / 1: EHT-MU-OFDMA / 2: EHT-MU-MIMO
char mac_ppdu_type_list[][16] = {"EHT-MU-OFDMA", "EHT-MU-SU", "EHT-MU-MIMO"};
#endif

//0: "long" / 1: "short" / 2: "0.8" / 3: "1.6" / 4: "3.2"
char gi_list[][8] = {"long", "short", "0.8", "1.6", "3.2"};

// "prot_nav_frm_ex=string". Valid string: 0: "none" / 1: "self_cts" / 2: "rts_cts" / 3: "rts_cts_qap" / 4: "stbc"
char nav_frm_list[][16] = {"none", "self_cts", "rts_cts", "rts_cts_qap", "stbc"};

// Valid string: 0:"0"/1: "0.25" / 2: "0.5" /3: "1"/ 4: "2" / 5: "4" / 6: "8" / 7: "16"
char mpdu_space_list[][8] = {"0", "0.25", "0.5", "1", "2", "4", "8", "16"};

//Valid string: 0: no_ack / 1: normal / 2: implicit
char ba_ack_list[][16] = {"no_ack", "normal", "implicit"};

//Valid string: 0: round down / 1: round up
char nsymb_choose_list[][16] = {"down", "up", "0", "1"};

char cali_cmd_name[][16] = {"set", "show", "update", "read-mem", "write-mem", "sounding", "tx-su",
							"tx-mu", "reset", "reload", "dif-sample"};
char cmd_type[][16] = {"radio", "bss", "ptk", "gtk", "su", "mu", "mac-phy", "mu-info", "sounding",
			"wmm", "xtal_cal_status", "rx-stats", "igtk", "csi", "pppc",
			"mld", "ap-mld", "sta-mld", "tdma"};

char *radio_name_list[] = {"radio", NULL};

char *bss_name_list[] = {"mode", "bw", "channel", "mu-user", "current-ac", "local-addr", "vap", "vap-num", NULL};

char *ptk_name_list[] = {"key-addr", "cipher", "key-len", "key", "hw-index", "wep-index", NULL};

char *macphy_name_list[] = {"ppdu-type", "stbc", "gi", "gid", "bw-ppdu", "spatial-reuse", "n-tx-prot",
							"midable", "doppler", "num-ext-nss", "bss-color", "antenna-set", "tx-power-level",
							"he-ltf", "preamble-type", "prot-nav-frm-ex", "prot-tx-power", "prot-format-mod",
							"prot-preamble-type", "prot-bw", "prot-mcs", "dyn-pre-punc-type", "num-he-ltf",
							"center-26tone-ru", "sigb-comp", "inact-subchan-bitmap", NULL};

char *su_name_list[] = {"peer-addr", "mpdu-type", "mpdu-subtype", "mpdu-protect", "mpdu-htc", "mpdu-ackpolicy",
	"mpdu-body", "mpdu-ssn", "mpdu-subframe", "mpdu-body-len", "ppdu-protection",
	"mpdu-duration", "mpdu-seq-ctrl", "mpdu-qos-ctrl", "mpdu-amsdu-num", "ppdu-bf",
	"mpdu-ignore-cca", "aid", "paid", "sta-index", "tx-id", "ba-rx-size", "ba-tx-size",
	"ba-ack-policy", "ba-ssn", "fec", "mcs", "mcs-legacy", "packet-ext", "dcm",
	"min-mpdu-space", "cbf-host-addr", "cbf-report-len", "user-pos", "start-sts", "ss-num",
	"ru-index", "ru-size", "ru-offset", "ru-pwr-factor", "is-eht", NULL};

char *muinfo_name_list[] = {"mu-type", "mu-ack", "mu-prot", NULL};

char *wmm_name_list[] = {"aifsn", "cw-min", "cw-max", NULL};

char *sounding_name_list[] = {"type", "nb-sta", "feedback-type", "nc-idx", "ng", NULL};

char *rx_stats_list[] = {"rx-stats", NULL};
char *csi_name_list[] = {"csi_on", "format-on", "base-addr", "bank-size", "bank-num",
		"smp-mode-sel", "smp-cont-sel0", "smp-cont-sel1", "smpout-send-wait",
		"rx-ppdu-symb-thresh", "rx-time-thresh", "l-ltf-gran", "non-he-ltf-gran",
		"he-ltf-gran", "csi_flag", NULL};


#if defined(CFG_MERAK3000)
//
// Keep same sequence with "enum CALI_MLO_MODE"
//
const char *sta_mlo_mode_list[] = {
	"none", "str", "nstr", "mlsr", "emlsr", "emlmr"
};

uint32_t string_to_mlo_mode(const char *mlo_mod)
{
	int i;

	for (i = 0; ARRAY_SIZE(sta_mlo_mode_list); i++) {
		if (!strcasecmp(mlo_mod, sta_mlo_mode_list[i]))
			return i;
	}
	return 0;
}

//
// Keep same sequence with "enum CALI_EML_INIT_CTRL_TYPE"
//
const char *eml_init_frame_type_list[] = {
	"mu-rts",	"bsrp",		"qos-null"
};

uint32_t string_to_init_frame_type(const char *frm_type)
{
	int i;

	for (i = 0; ARRAY_SIZE(eml_init_frame_type_list); i++) {
		if (!strcasecmp(frm_type, eml_init_frame_type_list[i]))
			return i;
	}
	return 0;
}

char *tdma_name_list[] = {"tdma-en", "mode", "ack-duration", NULL};
#endif // CFG_MERAK3000


#ifndef __KERNEL__
struct cls_csi_report g_cali_csi_report = {0};
int current_radio = -1;
char out_buf[5*1024] = "";
#else
char out_buf[5*1024] = "";
#endif
int out_buf_len = 0;
#define MAX_CMD_SIZE 1024
#ifdef __KERNEL__
static struct dentry *wifi_debug_dir;
#endif
static char calcmd[MAX_CMD_SIZE] = "get radio";

uint8_t zero_mac[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#ifndef __KERNEL__
struct cali_config_tag g_cal_config[MAX_RADIO_NUM];
struct cls_wifi_hw *g_radio_cls_wifi_hw[MAX_RADIO_NUM] =
#if MAX_RADIO_NUM == 2
{0, 0};
#endif
#endif

struct type_name_list type_name_config[] = {
	{TYPE_RADIO, 0, radio_name_list},
	{TYPE_BSS, 0, bss_name_list},
	{TYPE_PTK, 1, ptk_name_list},
	{TYPE_GTK, 0, ptk_name_list},
	{TYPE_SU, 0, su_name_list},
	{TYPE_MU, 1, su_name_list},
	{TYPE_MACPHY, 0, macphy_name_list},
	{TYPE_MUINFO, 0, muinfo_name_list},
	{TYPE_WMM, 1, wmm_name_list},
	{TYPE_SOUNDING, 0, sounding_name_list},
	{TYPE_RXSTATS, 0, rx_stats_list},
	{TYPE_IGTK, 0, ptk_name_list},
	{TYPE_CSI, 1, csi_name_list},
#if defined(CFG_MERAK3000)
	{TYPE_TDMA, 0, tdma_name_list},
#endif
};

int mac_str_to_bin(char *str, unsigned char *mac)
{
	int i;
	char *s, *e;

	if ((mac == NULL) || (str == NULL))
		return -1;

	s = (char *) str;
	for (i = 0; i < 6; ++i) {
		mac[i] = s ? simple_strtol(s, &e, 16) : 0;
		if (s)
			s = (*e) ? e + 1 : e;
	}
	return 0;
}

int bitmap_str_to_bin(char *str, uint32_t *bitmap, int word_num)
{
	int i;
	char *s, *e;

	if ((bitmap == NULL) || (str == NULL))
		return -1;

	s = (char *) str;
	for (i = 0; i < word_num; ++i) {
		bitmap[i] = s ? simple_strtol(s, &e, 16) : 0;
		if (s)
			s = (*e) ? e + 1 : e;
	}
	return 0;
}

int str2hex(char *str, char split, int key_len, int str_len, unsigned char *dst)
{
	char *end = str;
	char *p_shift = str;
	int i = 0;

	while (p_shift - str < str_len) {
		dst[i] = simple_strtoul(p_shift, &end, 16);
		i++;
		if (i == key_len)
			break;

		if (*end == split)
			end += 1;
		p_shift = end;
	}

	return i;
}

void hex_to_string(unsigned char *hex, char *str, int len)
{
	int i = 0;

	for (; i < len; i++) {
		char tmp[3];

		memset(tmp, 0, sizeof(tmp));
		sprintf(tmp, "%02x", hex[i]);
		strcat(str, tmp);
	}
}

int get_name_value_index(const char *buff, int len, char *varname, char *varvalue, int *idx)
{
	int ret = 0;
	char *pval = NULL;
	char *pindex = NULL;

	pval = strchr(buff, '=');
	if (!pval)
		return ret;
	pval++;
	strncpy(varvalue, pval, buff + len - pval);
	pindex = strchr(buff, '[');
	if (!pindex) {
		strncpy(varname, buff, pval - buff - 1);
		ret = 1;
	} else {
		*idx = *(pindex + 1) - '0';
		strncpy(varname, buff, pindex - buff);
		ret = 1;
	}

	return ret;
}

enum cali_cmd_type get_cmdtype_from_name_index(char *name, int index)
{
	enum cali_cmd_type type = TYPE_MAX;
	int  i = 0;
	int j = 0;

	for (; i < ARRAY_SIZE(type_name_config); i++) {
		if (index == -1) {
			if (type_name_config[i].is_multi == 1)
				continue;
		} else {
			if (type_name_config[i].is_multi == 0)
				continue;
		}
		j = 0;
		while (type_name_config[i].name_list[j] != NULL) {
			if (!strcasecmp(name, type_name_config[i].name_list[j])) {
				type = type_name_config[i].type;
				return type;
			}
			j++;
		}
	}
	return  type;
}

/*
 * parse name=value and set the corresponding variable in cali_config_tag
 */
void parse_name_value(const char *buff, int len, struct cali_config_tag *ptag)
{
	int ret = 0;
	char name[32];
	char value[128];
	int idx = -1;
	enum cali_cmd_type type = TYPE_MAX;

	memset(name, 0, sizeof(name));
	memset(value, 0, sizeof(value));

	ret = get_name_value_index(buff, len, name, value, &idx);

#if CAL_DBG
	pr_err("%s:get name[%s], value[%s], idx[%d]\n", __func__, name, value, idx);
#endif

	if (ret) {
		type = get_cmdtype_from_name_index(name, idx);
		set_single_param(type, idx, name, value, ptag);
	}
}

#ifdef __KERNEL__
int set_def_cal_config(struct cali_config_tag *pconf, char *filename, struct device *device)
{
	const struct firmware *config_fw;
	unsigned int curr, line_start = 0, line_size;
	int file_size = 0;
	const u8 *file_data = NULL;
	int ret = 0;

	ret = request_firmware(&config_fw, filename, device);
	if (ret) {
		pr_warn("%s: Failed to get %s (%d)\n", __func__, filename, ret);
		return ret;
	}

	file_data = config_fw->data;
	file_size = config_fw->size;
	while (line_start < file_size) {
		/* Search the end of the current line (or the end of the file) */
		for (curr = line_start; curr < file_size; curr++)
			if (file_data[curr] == '\n')
				break;

		/* Compute the line size */
		line_size = curr - line_start;

		/*Parse the line to get name and value */
		parse_name_value(file_data + line_start, line_size, pconf);

		/* Move to next line */
		line_start = curr + 1;
	}
	release_firmware(config_fw);
	return ret;
}
#endif

#if defined(__KERNEL__)
int read_payload_from_file(struct cls_wifi_hw *cls_wifi_hw, int type, u32 offset)
{
	struct file *fp = NULL;
	int len = 0;
	int i = 0;
	int j = 0;
	char tmp_str[3] = { 0 };
	loff_t pos = 0;
	uint8_t *buf;

	if (type == 0)
		fp = filp_open("/tmp/manage.raw", O_RDWR, 0644);
	else if (type == 1)
		fp = filp_open("/tmp/control.raw", O_RDWR, 0644);
	else if (type == 2)
		fp = filp_open("/tmp/data.raw", O_RDWR, 0644);

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
	for (; i < len - 1; i++) {
		if (kernel_read(fp, tmp_str, sizeof(tmp_str), &pos) < 2)
			break;
		buf[j++] = simple_strtoul(tmp_str, NULL, 16);
	}

	cls_wifi_hw->ipc_env->ops->writen(cls_wifi_hw->ipc_env->plat,
			cls_wifi_hw->ipc_env->radio_idx, offset, buf, j);
	kfree(buf);
	filp_close(fp, NULL);
	return j;
}
#else
int read_payload_from_file(struct cls_wifi_hw *cls_wifi_hw, int type, u32 offset)
{
	return 1;
}

#endif
int set_radio(char *name, char *pval, struct cali_config_tag *ptag)
{
	int ret = 0;

	if (!strcasecmp(name, "radio")) {
		ret = 1;
		if (!strcasecmp(pval, "2G"))
			current_radio = RADIO_2P4G_INDEX;
		else
			current_radio = RADIO_5G_INDEX;
	}
	return ret;
}

int set_bss_param(char *name, char *pval, struct cali_config_tag *ptag)
{
	int i = 0;
	int ret = 0;
	struct cali_config_tag *pcurrent = ptag;

	if (!strcasecmp(name, "mode")) {
		ret = 1;
		if (!strcasecmp(pval, "ap"))
			pcurrent->bss_info.mac_mode = 0;
		else
			pcurrent->bss_info.mac_mode = 1;
	} else if (!strcasecmp(name, "channel")) {
		ret = 1;
		pcurrent->bss_info.chan_ieee = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "bw")) {
		ret = 1;
		pcurrent->bss_info.bw = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "mu-user")) {
		ret = 1;
		pcurrent->mu_users = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "phy-mode")) {
		ret = 1;
		for (; ARRAY_SIZE(phymode_list); i++) {
			if (!strcasecmp(pval, phymode_list[i])) {
				pcurrent->bss_info.phy_mode = i;
				break;
			}
		}
	} else if (!strcasecmp(name, "vap-num")) {
		ret = 1;
		pcurrent->bss_info.vap_num = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "bw_change")) {
		ret = 1;
		pcurrent->bss_info.bw_change = simple_strtol(pval, NULL, 10);
#ifndef __KERNEL__
		yc_printf("bw_change =%d\n", pcurrent->bss_info.bw_change);
#endif
	} else if (!strcasecmp(name, "current-ac")) {
		ret = 1;
		pcurrent->current_ac = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "local-addr")) {
		ret = 1;
		mac_str_to_bin(pval, pcurrent->bss_info.local_addr);
	} else if (!strcasecmp(name, "tx-ppdu-cnt")) {
		ret = 1;
		pcurrent->tx_ppdu_cnt = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "tx-interval")) {
		ret = 1;
		pcurrent->tx_interval = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "vap")) {
		ret = 0;
	} else if (!strcasecmp(name, "mfp")) {
		ret = 1;
		pcurrent->bss_info.mfp = simple_strtol(pval, NULL, 0);
	}
#if defined(CFG_MERAK3000)
	else if (!strcasecmp(name, "ap-mld-id")) {
		ret = 1;
		pcurrent->bss_info.ap_mld_id = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "link-id")) {
		ret = 1;
		pcurrent->bss_info.link_id = simple_strtol(pval, NULL, 0);
	}
#endif //CFG_MERAK3000

	return ret;
}

void set_mbss_single_param(int idx, char *name, char *pval, struct cali_config_tag *ptag)
{
	struct cali_config_tag *pcurrent = ptag;

	if (!strcasecmp(name, "mode")) {
		if (!strcasecmp(pval, "ap"))
			pcurrent->bss_info.mbss_info[idx].mac_mode = 0;
		else
			pcurrent->bss_info.mbss_info[idx].mac_mode = 1;
	} else if (!strcasecmp(name, "local-addr")) {
		mac_str_to_bin(pval, pcurrent->bss_info.mbss_info[idx].local_addr);
	} else {
#if CAL_DBG
		pr_err("invalid name[%s]\n", name);
#endif
	}
	return;
}

int set_mbss_all_param(char *vap_buf, char *pend, struct cali_config_tag *ptag)
{
	char name[32];
	char value[128];
	char *pspace = NULL;
	char *pchar = vap_buf;
	int vap_idx = -1;

	while (*pchar == ' ')
		pchar++;
	sscanf(pchar, "%d ", &vap_idx);
	if (vap_idx == 0) {
		return vap_idx; // vap 0 is the base bss, args "vap 0" is optional in cal cmd
	} else if (vap_idx > 0 && vap_idx < 32) {
		pchar = strchr(pchar, ' ');
		if (!pchar)
			return vap_idx;
		pchar += 1;
		// len -= pch - calcmd;
		// set_all_param(set_type, idx_multi, pch, len, &g_cal_config[current_radio]);
	} else {
#if CAL_DBG
		pr_err("invalid vap id: %d \n", vap_idx);
#endif
		return vap_idx;
	}

	while (pchar <= pend)
	{
		memset(name, 0, sizeof(name));
		memset(value, 0, sizeof(value));
		while (*pchar == ' ')
			pchar++;
		if (pchar == pend)
			break;

		pspace = strchr(pchar, ' ');
		if (!pspace)
			break;
		strncpy(name, pchar, pspace - pchar);
		pchar = pspace + 1;
		while (*pchar == ' ')
			pchar++;
		pspace = strchr(pchar, ' ');
		if (!pspace)
			strncpy(value, pchar, pend - pchar + 1);
		else
			strncpy(value, pchar, pspace - pchar);

#if CAL_DBG
		pr_err("type bss-name[%s]-value[%s]\n", name, value);
#endif
		set_mbss_single_param(vap_idx - 1, name, value, ptag);

		if (pspace)
			pchar = pspace + 1;
	}
	return vap_idx;
}

void string_to_hex(char *str, unsigned char *hex, int len)
{
	int i;
	uint32_t val;

	for (i = 0; i < len; i++) {
		sscanf(str + 2 * i, "%2x", &val);
		hex[i] = val;
	}
}

int set_pgtk_param(char *name, char *pval, struct cali_key_info_tag *ptag)
{
	struct cali_key_info_tag *pcurrent = ptag;
	int val = 0;
	int ret = 0;

	if (!strcasecmp(name, "cipher")) {
		int idx = 0;

		ret = 1;
		for (; idx < ARRAY_SIZE(ciper_list); idx++) {
			if (!strcasecmp(pval, ciper_list[idx]))
				pcurrent->cipher = idx & 0xffff;
		}
	} else if (!strcasecmp(name, "wep-index")) {
		ret = 1;
		val = simple_strtol(pval, NULL, 10);
		if (val >= 0  && val <= 5)
			pcurrent->key_index = val & 0xffff;
	} else if (!strcasecmp(name, "key-len")) {
		ret = 1;
		val = simple_strtol(pval, NULL, 0);
		if (val <= 32)
			pcurrent->key_len = val & 0xffff;
	} else if (!strcasecmp(name, "key")) {
		if (strlen(pval) % 2 == 0) {
			int len = strlen(pval) / 2;

			ret = 1;
			if (pcurrent->key_len != 0) {
				if (pcurrent->key_len < len)
					len = pcurrent->key_len;
			}
			if (len >= 32)
				len = 32;

			string_to_hex(pval, pcurrent->key, len);
			pcurrent->key_len = len;

		}
	} else if (!strcasecmp(name, "hw-index")) {
		ret = 1;
		val = simple_strtol(pval, NULL, 10);
		pcurrent->key_hw_index = val & 0xffff;
	} else if (!strcasecmp(name, "mac-addr") || !strcasecmp(name, "key-addr")) {
		ret = 1;
		mac_str_to_bin(pval, pcurrent->key_addr);
	} else if (!strcasecmp(name, "bw-sig-force")) {
		ret = 1;
		pcurrent->bw_signaling_force = simple_strtol(pval, NULL, 10) & 0xff;
	}
	return ret;
}

int set_csi_param(char *name, char *pval, struct cali_csi_info_tag *ptag)
{
	struct cali_csi_info_tag *pcurrent = ptag;
	int val = 0;
	int ret = 0;
	int i;

	if (!strcasecmp(name, "csi-on")) {
		ret = 1;
		val = simple_strtol(pval, NULL, 10);
		pcurrent->csi_on = !!val;
	} else if (!strcasecmp(name, "format-on")) {
		ret = 1;
		val = simple_strtol(pval, NULL, 0);
		pcurrent->format_on = val & 0xffff;
	} else if (!strcasecmp(name, "base-addr")) {
		ret = 1;
		val = simple_strtol(pval, NULL, 0);
		pcurrent->base_addr = val;
	} else if (!strcasecmp(name, "bank-size")) {
		ret = 1;
		val = simple_strtol(pval, NULL, 10);
		pcurrent->bank_size = val & 0xffff;
	} else if (!strcasecmp(name, "bank-num")) {
		ret = 1;
		val = simple_strtol(pval, NULL, 10);
		pcurrent->bank_num = val & 0xffff;
	} else if (!strcasecmp(name, "smp-mode-sel")) {
		ret = 1;
		val = simple_strtol(pval, NULL, 10);
		pcurrent->smp_mode_sel = val & 0xffff;
	} else if (!strcasecmp(name, "smp-cont-sel0")) {
		ret = 1;
		val = simple_strtol(pval, NULL, 10);
		pcurrent->smp_cont_sel0 = val & 0xffff;
	} else if (!strcasecmp(name, "smp-cont-sel1")) {
		ret = 1;
		val = simple_strtol(pval, NULL, 10);
		pcurrent->smp_cont_sel1 = val & 0xffff;
	} else if (!strcasecmp(name, "smpout-send-wait")) {
		ret = 1;
		val = simple_strtol(pval, NULL, 10);
		pcurrent->csi_smpout_send_wait = val & 0xffff;
	} else if (!strcasecmp(name, "rx-ppdu-symb-thresh")) {
		ret = 1;
		val = simple_strtol(pval, NULL, 10);
		pcurrent->rx_ppdu_symb_thresh = val & 0xffff;
	} else if (!strcasecmp(name, "rx-time-thresh")) {
		ret = 1;
		val = simple_strtol(pval, NULL, 10);
		pcurrent->rx_time_thresh = val & 0xffff;
	} else if (!strcasecmp(name, "l-ltf-gran")) {
		ret = 1;
		val = simple_strtol(pval, NULL, 10);
		pcurrent->l_ltf_gran = val & 0xffff;
	} else if (!strcasecmp(name, "non-he-ltf-gran")) {
		ret = 1;
		val = simple_strtol(pval, NULL, 16);
		for (i = 0; i < CALI_PHY_BW_MAX; i++)
			pcurrent->non_he_ltf_gran[i] = (val >> (i * 8)) & 0xFF;
	} else if (!strcasecmp(name, "he-ltf-gran")) {
		ret = 1;
		val = simple_strtol(pval, NULL, 16);
		for (i = 0; i < CALI_PHY_BW_MAX; i++)
			pcurrent->he_ltf_gran[i] = (val >> (i * 8)) & 0xFF;
	} else if (!strcasecmp(name, "csi-flag")) {
		ret = 1;
		val = simple_strtol(pval, NULL, 10);
		pcurrent->csi_flag = !!val;
	}

	return ret;
}


int set_macphy_param(char *name, char *pval, struct cali_config_tag *ptag)
{
	int ret = 0;
	struct cali_config_tag *pcurrent = ptag;

	if (!strcasecmp(name, "ppdu-type")) {
		int idx = 0;

		ret = 1;
		for (; idx < ARRAY_SIZE(ppdu_type_list); idx++) {
			if (!strcasecmp(pval, ppdu_type_list[idx]))
				pcurrent->mac_phy_params.format_mod = idx;
		}
#if defined(CFG_MERAK3000)
	} else if (!strcasecmp(name, "mac-ppdu-type")) {
		int idx = 0;

		ret = 1;
		for (; idx < ARRAY_SIZE(mac_ppdu_type_list); idx++) {
			if (!strcasecmp(pval, mac_ppdu_type_list[idx]))
				pcurrent->mac_phy_params.mac_ppdu_type = idx;
		}
	} else if (!strcasecmp(name, "non-standard-ta-func")) {
		ret = 1;
		pcurrent->mac_phy_params.non_standard_ta_func = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "non-standard-ta")) {
		ret = 1;
		pcurrent->mac_phy_params.non_standard_ta = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "dyn-bw")) {
		ret = 1;
		pcurrent->mac_phy_params.dyn_bw = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "txbf-filter-en")) {
		ret = 1;
		pcurrent->mac_phy_params.txbf_filter_en = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "txbf-alpha")) {
		ret = 1;
		pcurrent->mac_phy_params.txbf_alpha = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "txbf-smooth")) {
		ret = 1;
		pcurrent->mac_phy_params.txbf_smooth = simple_strtol(pval, NULL, 10);
#endif
	} else if (!strcasecmp(name, "stbc")) {
		ret = 1;
		pcurrent->mac_phy_params.stbc = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "gi")) {
		int idx = 0;

		ret = 1;
		for (; idx < ARRAY_SIZE(gi_list); idx++) {
			if (!strcasecmp(pval, gi_list[idx]))
				pcurrent->mac_phy_params.gi = idx;
		}
	} else if (!strcasecmp(name, "gid")) {
		ret = 1;
		pcurrent->mac_phy_params.group_id = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "bw-ppdu")) {
		ret = 1;
		pcurrent->mac_phy_params.bw_ppdu = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "spatial-reuse")) {
		ret = 1;
		pcurrent->mac_phy_params.spatial_reuse = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "n-tx-prot")) {
		ret = 1;
		pcurrent->mac_phy_params.n_tx_prot = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "n-tx")) {
		ret = 1;
		pcurrent->mac_phy_params.n_tx = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "midable")) {
		ret = 1;
		pcurrent->mac_phy_params.midable = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "smm-index")) {
		ret = 1;
		pcurrent->mac_phy_params.smm_index = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "doppler")) {
		ret = 1;
		pcurrent->mac_phy_params.doppler = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "num-ext-nss")) {
		ret = 1;
		pcurrent->mac_phy_params.num_ext_nss = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "bss-color")) {
		ret = 1;
		pcurrent->mac_phy_params.bss_color = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "antenna-set")) {
		ret = 1;
		pcurrent->mac_phy_params.antenna_set = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "tx-power-level")) {
		ret = 1;
		pcurrent->mac_phy_params.tx_power_level = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "he-ltf")) {
		ret = 1;
		pcurrent->mac_phy_params.he_ltf = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "preamble-type")) {
		ret = 1;
		pcurrent->mac_phy_params.preamble_type = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "prot-nav-frm-ex")) {
		int idx = 0;

		ret = 1;
		for (; idx < ARRAY_SIZE(nav_frm_list); idx++) {
			if (!strcasecmp(pval, nav_frm_list[idx]))
				pcurrent->mac_phy_params.prot_nav_frm_ex = idx;
		}
	} else if (!strcasecmp(name, "prot-tx-power")) {
		ret = 1;
		pcurrent->mac_phy_params.prot_tx_power = simple_strtol(pval, NULL, 16);
	} else if (!strcasecmp(name, "prot-format-mod")) {
		int idx = 0;

		ret = 1;
		for (; idx < ARRAY_SIZE(ppdu_type_list); idx++) {
			if (!strcasecmp(pval, ppdu_type_list[idx]))
				pcurrent->mac_phy_params.prot_format_mod = idx;
		}
	} else if (!strcasecmp(name, "prot-preamble-type")) {
		ret = 1;
		pcurrent->mac_phy_params.prot_preamble_type = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "prot-bw")) {
		ret = 1;
		pcurrent->mac_phy_params.prot_bw = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "prot-mcs")) {
		ret = 1;
		pcurrent->mac_phy_params.prot_mcs = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "dyn-pre-punc-type")) {
		ret = 1;
		pcurrent->mac_phy_params.dyn_pre_punc_type = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "num-he-ltf")) {
		ret = 1;
		pcurrent->mac_phy_params.num_he_ltf = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "num-eht-ltf")) {
		ret = 1;
		pcurrent->mac_phy_params.num_eht_ltf = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "center-26tone-ru")) {
		ret = 1;
		pcurrent->mac_phy_params.center_26tone_ru = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "sigb-comp")) {
		ret = 1;
		pcurrent->mac_phy_params.sigB_comp = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "sigb-dcm")) {
		ret = 1;
		pcurrent->mac_phy_params.sigB_dcm = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "sigb-mcs")) {
		ret = 1;
		pcurrent->mac_phy_params.sigB_mcs = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "sigb-ru-alloc1")) {
		ret = 1;
		pcurrent->mac_phy_params.sigB_ru_alloc_1 = simple_strtol(pval, NULL, 16);
	} else if (!strcasecmp(name, "sigb-ru-alloc2")) {
		ret = 1;
		pcurrent->mac_phy_params.sigB_ru_alloc_2 = simple_strtol(pval, NULL, 16);
	} else if (!strcasecmp(name, "beamformed")) {
		ret = 1;
		pcurrent->mac_phy_params.beamformed = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "inact-subchan-bitmap")) {
		ret = 1;
		pcurrent->mac_phy_params.inact_subchan_bitmap = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "dropbwentx")) {
		ret = 1;
		pcurrent->mac_phy_params.dropbwentx = simple_strtol(pval, NULL, 10);
	}
	 else if (!strcasecmp(name, "txopdurationen")) {
		ret = 1;
		pcurrent->mac_phy_params.txopdurationen = simple_strtol(pval, NULL, 10);
#ifndef __KERNEL__
		yc_printf("txopdurationen =%d\n", pcurrent->mac_phy_params.txopdurationen);
#endif
	}  else if (!strcasecmp(name, "txopduration")) {
		ret = 1;
		pcurrent->mac_phy_params.txopduration = simple_strtol(pval, NULL, 0);
#ifndef __KERNEL__
		yc_printf("txopduration =%d\n", pcurrent->mac_phy_params.txopduration);
#endif
	} else if (!strcasecmp(name, "hw-retry-en")) {
		ret = 1;
#if defined(CFG_MERAK3000)
		pcurrent->mac_phy_params.txop_txtime_overcut_en = simple_strtol(pval, NULL, 10);
#else
		pcurrent->mac_phy_params.hw_retry_en = simple_strtol(pval, NULL, 10);
#endif
	} else if (!strcasecmp(name, "smart-ant-en")) {
		ret = 1;
		pcurrent->mac_phy_params.smart_ant_en = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "smart-ant-en")) {
		ret = 1;
		pcurrent->mac_phy_params.smart_ant_en = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "smart-ant-param0")) {
		ret = 1;
		pcurrent->mac_phy_params.smart_ant_param0 = simple_strtol(pval, NULL, 16);
	} else if (!strcasecmp(name, "smart-ant-param1")) {
		ret = 1;
		pcurrent->mac_phy_params.smart_ant_param1 = simple_strtol(pval, NULL, 16);
	} else if (!strcasecmp(name, "sr-ppdu-min-mcs")) {
		ret = 1;
		pcurrent->mac_phy_params.sr_ppdu_min_mcs = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "mpdu-retry-limit")) {
		ret = 1;
		pcurrent->mac_phy_params.mpdu_retry_limit = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "no-just-hard-retry-limit")) {
		ret = 1;
		pcurrent->mac_phy_params.no_just_hard_retry_limit = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "prot-retry-limit")) {
		ret = 1;
		pcurrent->mac_phy_params.prot_retry_limit = simple_strtol(pval, NULL, 10);
	}

	return ret;
}

int set_user_ppdu_param(char *name, char *pval, struct cali_ppdu_info_tag *ptag)
{
	int ret = 0;
	struct cali_ppdu_info_tag *pcurrent = ptag;

	if (!strcasecmp(name, "mpdu-type")) {
		int val = simple_strtol(pval, NULL, 10);

		ret = 1;
		if (val == 0)
			ptag->frame_ctrl &= 0xfff3;
		else if (val == 1) {
			ptag->frame_ctrl &= ~(1 << 3);
			ptag->frame_ctrl |= 1 << 2;
		} else if (val == 2) {
			ptag->frame_ctrl &= ~(1 << 2);
			ptag->frame_ctrl |= (1 << 3);
		}
	} else if (!strcasecmp(name, "mpdu-subtype")) {
		int val = simple_strtol(pval, NULL, 0);

		ptag->frame_ctrl = (val << 4) | (ptag->frame_ctrl & 0xffffff0f);
		ret = 1;
	} else if (!strcasecmp(name, "mpdu-fc-flags")) {
		int val = simple_strtol(pval, NULL, 0);

		ptag->frame_ctrl = (val << 8) | (ptag->frame_ctrl & 0xffff00ff);
		ret = 1;
	} else if (!strcasecmp(name, "mpdu-protect")) {
		ret = 1;
		pcurrent->ppdu_encry = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "mpdu-htc")) {
		ret = 1;
		pcurrent->ht_ctrl = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "mpdu-ackpolicy")) {
		ret = 1;
		pcurrent->ack_policy = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "mpdu-body")) {
		ret = 1;
		pcurrent->payload_pattern = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "mpdu-ssn")) {
		ret = 1;
		pcurrent->ssn = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "mpdu-subframe")) {
		ret = 1;
		pcurrent->ampdu_num = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "mpdu-body-len")) {
		ret = 1;
		pcurrent->msdu_len = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "apep-len")) {
		ret = 1;
		pcurrent->apep_len = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "mpdu-ignore-cca")) {
		ret = 1;
		pcurrent->ignore_cca = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "ppdu-bf")) {
		ret = 1;
		pcurrent->ppdu_bf = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "mpdu-amsdu-num")) {
		ret = 1;
		pcurrent->amsdu_num = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "mpdu-qos-ctrl")) {
		ret = 1;
		pcurrent->qos_ctrl = simple_strtol(pval, NULL, 16);
	} else if (!strcasecmp(name, "mpdu-seq-ctrl")) {
		ret = 1;
		pcurrent->seq_ctrl = simple_strtol(pval, NULL, 16);
	} else if (!strcasecmp(name, "mpdu-duration")) {
		ret = 1;
		pcurrent->duration = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "ppdu-protection")) {
		ret = 1;
		pcurrent->protection = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "ssn-reset")) {
		ret = 1;
		pcurrent->ssn_reset = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "non-qos-ack")) {
		ret = 1;
		pcurrent->non_qos_ack = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "bw-signal-en")) {
		ret = 1;
		pcurrent->bw_signal_en = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "sr-disallow")) {
		ret = 1;
		pcurrent->sr_disallow = simple_strtol(pval, NULL, 10);
#if defined(CFG_MERAK3000)
	} else if (!strcasecmp(name, "hw-llc-encap-en")) {
		ret = 1;
		pcurrent->hw_llc_encap_en = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "sr_drop_pwr_en")) {
		ret = 1;
		pcurrent->sr_drop_pwr_en = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "sr_adj_mcs_delta")) {
		ret = 1;
		pcurrent->sr_adj_mcs_delta = simple_strtol(pval, NULL, 0);
#endif
	}

	return ret;
}

int set_user_info_param(char *name, char *pval, struct cali_user_info_tag *ptag)
{
	int ret = 0;
	struct cali_user_info_tag *pcurrent = ptag;

	if (!strcasecmp(name, "aid")) {
		ret = 1;
		pcurrent->aid = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "paid")) {
		ret = 1;
		pcurrent->paid = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "mac-addr") || !strcasecmp(name, "peer-addr")) {
		ret = 1;
		mac_str_to_bin(pval, pcurrent->peer_addr);
	} else if (!strcasecmp(name, "tx-tid")) {
		ret = 1;
		pcurrent->tx_tid = simple_strtol(pval, NULL, 0) & 0xff;
		pcurrent->ba_info.ba_tid = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "sta-index")) {
		ret = 1;
		pcurrent->sta_index = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "ba-tx-size")) {
		ret = 1;
		pcurrent->ba_info.ba_tx_size = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "ba-rx-size")) {
		ret = 1;
		pcurrent->ba_info.ba_rx_size = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "ba-ack-policy")) {
		int idx = 0;

		ret = 1;
		for (; idx < ARRAY_SIZE(ba_ack_list); idx++) {
			if (!strcasecmp(pval, ba_ack_list[idx]))
				pcurrent->ba_info.ba_ack_policy = idx;
		}
	} else if (!strcasecmp(name, "ba-ssn")) {
		ret = 1;
		pcurrent->ba_info.ba_ssn = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "lock-csi")) {
		ret = 1;
		pcurrent->lock_csi = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "spp")) {
		ret = 1;
		pcurrent->spp = simple_strtol(pval, NULL, 0);
	}
#if defined(CFG_MERAK3000)
	if (!strcasecmp(name, "ap-mld-id")) {
		ret = 1;
		pcurrent->ap_mld_id = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "link-id")) {
		ret = 1;
		pcurrent->link_id = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "is-eht")) {
		ret = 1;
		pcurrent->is_eht = simple_strtol(pval, NULL, 0);
	}
#endif // CFG_MERAK3000

	return ret;
}

int set_user_mac_phy_param(char *name, char *pval, struct cali_per_user_mac_phy_params_tag *ptag)
{
	int  ret = 0;
	struct cali_per_user_mac_phy_params_tag *pcurrent = ptag;

	if (!strcasecmp(name, "fec")) {
		ret = 1;
		pcurrent->fec = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "mcs")) {
		ret = 1;
		pcurrent->mcs = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "mcs-legacy")) {
		ret = 1;
		pcurrent->mcs_legacy = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "packet-ext")) {
		ret = 1;
		pcurrent->packet_ext = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "min-mpdu-space")) {
		int idx = 0;

		ret = 1;
		for (; idx < ARRAY_SIZE(mpdu_space_list); idx++) {
			if (!strcasecmp(pval, mpdu_space_list[idx]))
				pcurrent->min_mpdu_space = idx;
		}
	} else if (!strcasecmp(name, "dcm")) {
		ret = 1;
		pcurrent->dcm = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "cbf-host-addr")) {
		ret = 1;
		pcurrent->cbf_host_addr = simple_strtol(pval, NULL, 16);
	} else if (!strcasecmp(name, "cbf-report-len")) {
		ret = 1;
		pcurrent->cbf_report_len = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "user-pos")) {
		ret = 1;
		pcurrent->user_pos = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "start-sts")) {
		ret = 1;
		pcurrent->start_sts = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "ss-num")) {
		ret = 1;
		pcurrent->ss_num = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "ru-index")) {
		ret = 1;
		pcurrent->ru_index = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "ru-size")) {
		ret = 1;
		pcurrent->ru_size = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "ru-offset")) {
		ret = 1;
		pcurrent->ru_offset = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "eof-padding")) {
		ret = 1;
		pcurrent->eof_padding = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "retry-rate-dec")) {
		ret = 1;
		pcurrent->retry_rate_dec = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "user-bf")) {
		ret = 1;
		pcurrent->user_bf = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "ru-pwr-factor")) {
		ret = 1;
		pcurrent->ru_power_factor = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "dropMCSEnTx")) {
		ret = 1;
		pcurrent->dropMCSEnTx = simple_strtol(pval, NULL, 10);
#ifndef __KERNEL__
		yc_printf("dropMCSEnTx =%d\n", pcurrent->dropMCSEnTx );
#endif
	} else if (!strcasecmp(name, "pow_split_en")) {
		ret = 1;
		pcurrent->pow_split_en = simple_strtol(pval, NULL, 10);
#ifndef __KERNEL__
		yc_printf("pow_split_en =%d\n", pcurrent->pow_split_en);
#endif
	}
	return ret;
}

int set_ptk_macaddr(char *name, char *pval, struct cali_config_tag *ptag)
{
	int i = 0;
	uint8_t tmp_mac[6];

	mac_str_to_bin(pval, tmp_mac);
	for (; i < CALI_MU_USER_NUM; i++) {
		if (!memcmp(ptag->key_ptk[i].key_addr, tmp_mac, 6))
			break;
	}
	if (i < CALI_MU_USER_NUM)
		return i;

	if (ptag->mu_users) {
		// For 5G + 2.4G, because we have init-ed all key_ptk[x].key_addr,
		// If mac-addr in 2.4G is not in the key_ptk[] list(default-value: For 5G or single 5G/2.4G test),
		// Cant find a empty one, and can't set the PTK.
		// So, use the peer_addr to avoid this problem.
		for (i = 0; i < CALI_MU_USER_NUM; i++) {
			if (!memcmp(ptag->mu_user_info[i].peer_addr, tmp_mac, 6)) {
				memcpy(ptag->key_ptk[i].key_addr, tmp_mac, 6);
				return i;
			}
		}
	} else {
		if (!memcmp(ptag->su_user_info.peer_addr, tmp_mac, 6)) {
			memcpy(ptag->key_ptk[0].key_addr, tmp_mac, 6);
			return 0;
		}
	}

	for (i = 0; i < CALI_MU_USER_NUM; i++) {
		if (!memcmp(ptag->key_ptk[i].key_addr, zero_mac, 6))
			break;
	}
	if (i == CALI_MU_USER_NUM)
		i = 0;
	memcpy(ptag->key_ptk[i].key_addr, tmp_mac, 6);

	return i;

}

int set_muinfo_param(char *name, char *pval, struct cali_config_tag *ptag)
{
	int ret = 0;
	struct cali_mu_info_tag *pcurrent = &ptag->mu_info;

	if (*name == 0 || *pval == 0)
		return ret;

	if (!strcasecmp(name, "mu-type")) {
		ret = 1;
		pcurrent->mu_type = simple_strtol(pval, NULL, 10) & 0xff;
	} else if (!strcasecmp(name, "mu-ack")) {
		ret = 1;
		pcurrent->mu_ack = simple_strtol(pval, NULL, 10) & 0xff;
	} else if (!strcasecmp(name, "mu-prot")) {
		ret = 1;
		pcurrent->mu_prot = simple_strtol(pval, NULL, 10) & 0xff;
	} else if (!strcasecmp(name, "tb-hw")) {
		ret = 1;
		pcurrent->tb_hw = simple_strtol(pval, NULL, 10) & 0xff;
	} else if (!strcasecmp(name, "ul-duration")) {
		ret = 1;
		pcurrent->ul_duration = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "en-sample")) {
		ret = 1;
		pcurrent->en_sample = simple_strtol(pval, NULL, 0) & 0xff;
	} else if (!strcasecmp(name, "pre-fec-pad-factor")) {
		ret = 1;
		pcurrent->pre_fec_pad_factor = simple_strtol(pval, NULL, 0) & 0xff;
	} else if (!strcasecmp(name, "ldpc-extra-sym")) {
		ret = 1;
		pcurrent->ldpc_extra_sym = simple_strtol(pval, NULL, 0) & 0xff;
	} else if (!strcasecmp(name, "pe-disambiguity")) {
		ret = 1;
		pcurrent->pe_disambiguity = simple_strtol(pval, NULL, 0) & 0xff;
	} else if (!strcasecmp(name, "ul-mu-start-num")) {
		ret = 1;
		pcurrent->ul_mu_start_num = simple_strtol(pval, NULL, 0) & 0xff;
	} else if (!strcasecmp(name, "nsymb-choose")) {
		int idx = 0;

		ret = 1;
		for (; idx < ARRAY_SIZE(nsymb_choose_list); idx++) {
			if (!strcasecmp(pval, nsymb_choose_list[idx]))
				pcurrent->nsymb_choose = idx % 2;
		}
	}
	return ret;
}

int set_sounding_param(char *name, char *pval, struct cali_config_tag *ptag)
{
	int ret = 0;
	struct cali_sounding_info_tag *pcurrent = &ptag->sounding_info;

	if (*name == 0 || *pval == 0)
		return ret;
	if (!strcasecmp(name, "type")) {
		ret = 1;
		pcurrent->type = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "nb-sta")) {
		ret = 1;
		pcurrent->nb_sta = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "feedback-type")) {
		ret = 1;
		pcurrent->feedback_type = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "nc-idx")) {
		ret = 1;
		pcurrent->nc_idx = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "ng")) {
		ret = 1;
		pcurrent->ng = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "codebook-size")) {
		ret = 1;
		pcurrent->codebook_size = simple_strtol(pval, NULL, 10);
	}  else if (!strcasecmp(name, "sequence")) {
		ret = 1;
		pcurrent->sequence = simple_strtol(pval, NULL, 0);
	}
	return ret;
}

int set_wmm_param(char *name, char *pval, struct cali_wmm_params_tag *ptag)
{
	int ret = 0;
	struct cali_wmm_params_tag *pcurrent = ptag;

	if (*name == 0 || *pval == 0)
		return ret;
	if (!strcasecmp(name, "aifsn")) {
		ret = 1;
		pcurrent->aifsn = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "cw-min")) {
		ret = 1;
		pcurrent->cw_min = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "cw-max")) {
		ret = 1;
		pcurrent->cw_max = simple_strtol(pval, NULL, 0);
	}
	return ret;
}

int set_rxstats_param(char *name, char *pval, struct cali_config_tag *ptag)
{
	int  ret = 0;

	if (*name == 0 || *pval == 0)
		return ret;
	if (!strcasecmp(name, "rx-stats")) {
		if (!strcasecmp(pval, "on"))
			ptag->en_rxstats = 1;
		else
			ptag->en_rxstats = 0;
	}
	return ret;
}

int set_pppc_param(char *name, char *pval, struct cali_pppc_info_tag *ptag)
{
	int ret = 0;
	int i;
	char *s, *e;
	int32_t template_power_ppducnt[10][2] = {0};

	if (!strcasecmp(name, "enable")) {
		ptag->pppc_en = simple_strtol(pval, NULL, 10);
		ret = 1;
	} else if (!strcasecmp(name, "template")) {
		s = pval;
		for (i = 0; i < 10; i++) {
			template_power_ppducnt[i][0] = simple_strtol(s, &e, 10);
			if (*e != ',') {
				pr_err("%s(%d) invalid power template\n", __func__, __LINE__);
				return ret;
			}
			s = e + 1;
			template_power_ppducnt[i][1] = simple_strtol(s, &e, 10);
			if (*e == 0) {
				i = i + 1;
				break;
			} else {
				s = e + 1;
			}
		}
		ptag->template_num = i;
		memcpy(ptag->template_power_ppducnt, template_power_ppducnt, sizeof(ptag->template_power_ppducnt));
		ret = 1;

		pr_err("template_num = %d\n", ptag->template_num);
		for (i = 0; i < ptag->template_num; i++) {
			pr_err("template[%d] = (%d, %d)\n", i, ptag->template_power_ppducnt[i][0], ptag->template_power_ppducnt[i][1]);
		}
	}

	return ret;
}

/*
 *This function parses the long string to serval name-value pairs
 *and call set_single_param to save the value according to the type.
 *
 */
void set_all_param(enum cali_cmd_type type, int idx, char *buf, int len,
		struct cali_config_tag *ptag)
{
	char name[32];
	char value[128];
	char *pspace = NULL;
	char *pchar = buf;
	char *pend = buf + len - 1;
	int ptk_idx  = -1;

	while (pchar < buf + len) {
		memset(name, 0, sizeof(name));
		memset(value, 0, sizeof(value));
		while (*pchar == ' ')
			pchar++;
		if (pchar == pend)
			break;

		pspace = strchr(pchar, ' ');
		if (!pspace)
			break;
		strncpy(name, pchar, pspace - pchar);
		pchar = pspace + 1;
		while (*pchar == ' ')
			pchar++;
		pspace = strchr(pchar, ' ');
		if (!pspace)
			strncpy(value, pchar, buf + len - pchar);
		else
			strncpy(value, pchar, pspace - pchar);

		if (type == TYPE_PTK) {
			if (!strcasecmp(name, "mac-addr"))
				ptk_idx = set_ptk_macaddr(name, value, ptag);
			else
				set_pgtk_param(name, value, &ptag->key_ptk[ptk_idx]);
		} else if (type == TYPE_BSS && !strcasecmp(name, "vap")) {
			if (set_mbss_all_param(pchar, pend, ptag))
				break;
		} else {
#if CAL_DBG
			pr_err("type[%d]-name[%s]-value[%s]\n", type, name, value);
#endif
			set_single_param(type, idx, name, value, ptag);
		}
		if (pspace)
			pchar = pspace + 1;
	}
}

void parse_tx_su_param(char *name, char *value, struct cal_tx_su_req *txsu_param)
{
	if (!strcasecmp(name, "en-sample"))
		txsu_param->en_sample = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "sample-num"))
		txsu_param->num_sample = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "eof-padding"))
		txsu_param->eof_padding = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "stop"))
		txsu_param->terminate = simple_strtol(value, NULL, 0);
	if (!strcasecmp(name, "en-dif-send"))
		txsu_param->en_dif_send = simple_strtol(value, NULL, 0);
#if defined(CFG_MERAK3000)
	if (!strcasecmp(name, "mode"))
		txsu_param->mld_params.mlo_mode = string_to_mlo_mode(value);
	else if (!strcasecmp(name, "tx-tick"))
		txsu_param->mld_params.tx_tick = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "ppdu-cnt"))
		txsu_param->mld_params.ppdu_count = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "ppdu-delay"))
		txsu_param->mld_params.ppdu_delay = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "eml-init-frame-type"))
		txsu_param->mld_params.eml_init_frame_type = string_to_init_frame_type(value);
	else if (!strcasecmp(name, "eml-padding-delay"))
		txsu_param->mld_params.eml_padding_delay = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "eml-ul-duration"))
		txsu_param->mld_params.eml_ul_duration = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "eml-ul-mcs"))
		txsu_param->mld_params.eml_ul_mcs = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "force-send-init-frame"))
		txsu_param->mld_params.force_send_init_frame = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "replace-mld-addr"))
		txsu_param->mld_params.replace_mld_addr = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "new-txop"))
		txsu_param->mld_params.new_txop = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "hw-agg-mode"))
		txsu_param->hw_agg_params.hw_agg_mode = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "master"))
		txsu_param->hw_agg_params.master_radio = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "retry-bmp"))
		bitmap_str_to_bin(value, txsu_param->hw_agg_params.retry_bitmap, 4);
	else if (!strcasecmp(name, "succ-bmp"))
		bitmap_str_to_bin(value, txsu_param->hw_agg_params.succ_bitmap, 4);
	else if (!strcasecmp(name, "hw-agg-len"))
		txsu_param->hw_agg_params.hw_agg_len = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "hw-agg-mpdu"))
		txsu_param->hw_agg_params.hw_agg_mpdu_num = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "tx-diff"))
		txsu_param->hw_agg_params.hw_agg_slave_tx_diff = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "hw-agg-dup"))
		txsu_param->hw_agg_params.hw_agg_dup = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "fast-retry"))
		txsu_param->hw_agg_params.fast_retry = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "dyn-fetch"))
		txsu_param->hw_agg_params.dyn_fetch = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "one-off"))
		txsu_param->one_off_test = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "wmci-stop"))
		txsu_param->wmci_tx_stop_delay = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "wmci-tid-bmp"))
		txsu_param->wmci_tx_stop_queue_bmp = simple_strtol(value, NULL, 0);
#endif // CFG_MERAK3000
}

void parse_dif_sample_param(char *name, char *value, struct cal_dif_sample_req *dif_param)
{
	if (!strcasecmp(name, "sample-num"))
		dif_param->num_sample = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "chan"))
		dif_param->chan = simple_strtol(value, NULL, 10);
}

void parse_log_set_param(char *name, char *value, struct cal_log_set_req *set_req)
{
	if (!strcasecmp(name, "module"))
		set_req->log_module = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "level"))
		set_req->log_level = simple_strtol(value, NULL, 0);
}

void parse_tx_mu_param(char *name, char *value, struct cal_tx_mu_req *txmu_param)
{
	if (!strcasecmp(name, "en-sample"))
		txmu_param->en_sample = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "sample-num"))
		txmu_param->num_sample = simple_strtol(value, NULL, 0);
#ifdef CFG_MERAK3000
	else if (!strcasecmp(name, "trig-ppdu-id"))
		txmu_param->trig_ppdu_id = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "mode"))
		txmu_param->mld_params.mlo_mode = string_to_mlo_mode(value);
	else if (!strcasecmp(name, "tx-tick"))
		txmu_param->mld_params.tx_tick = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "ppdu-cnt"))
		txmu_param->mld_params.ppdu_count = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "ppdu-delay"))
		txmu_param->mld_params.ppdu_delay = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "eml-init-frame-type"))
		txmu_param->mld_params.eml_init_frame_type = string_to_init_frame_type(value);
	else if (!strcasecmp(name, "eml-padding-delay"))
		txmu_param->mld_params.eml_padding_delay = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "eml-ul-duration"))
		txmu_param->mld_params.eml_ul_duration = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "eml-ul-mcs"))
		txmu_param->mld_params.eml_ul_mcs = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "force-send-init-frame"))
		txmu_param->mld_params.force_send_init_frame = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "replace-mld-addr"))
		txmu_param->mld_params.replace_mld_addr = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "new-txop"))
		txmu_param->mld_params.new_txop = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "hw-agg-mode"))
		txmu_param->hw_agg_params.hw_agg_mode = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "master"))
		txmu_param->hw_agg_params.master_radio = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "retry-bmp"))
		bitmap_str_to_bin(value, txmu_param->hw_agg_params.retry_bitmap, 4);
	else if (!strcasecmp(name, "succ-bmp"))
		bitmap_str_to_bin(value, txmu_param->hw_agg_params.succ_bitmap, 4);
	else if (!strcasecmp(name, "hw-agg-len"))
		txmu_param->hw_agg_params.hw_agg_len = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "hw-agg-mpdu"))
		txmu_param->hw_agg_params.hw_agg_mpdu_num = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "tx-diff"))
		txmu_param->hw_agg_params.hw_agg_slave_tx_diff = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "hw-agg-dup"))
		txmu_param->hw_agg_params.hw_agg_dup = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "fast-retry"))
		txmu_param->hw_agg_params.fast_retry = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "dyn-fetch"))
		txmu_param->hw_agg_params.dyn_fetch = simple_strtol(value, NULL, 0);
#endif
}

void parse_radar_detect_param(char *name, char *value,
					struct cal_radar_detect_req *radar_detect_param)
{
	if (!strcasecmp(name, "enable"))
		radar_detect_param->enable = simple_strtol(value, NULL, 10);

	return;
}

void parse_interference_detect_param(char *name, char *value,
					struct cal_interference_detect_req *interference_detect_param)
{
	if (!strcasecmp(name, "enable"))
		interference_detect_param->enable = simple_strtol(value, NULL, 10);

	return;
}

void parse_start_rssi_param(char *name, char *value,
					struct cal_rssi_start_req *rssi_start_req)
{
	if (!strcasecmp(name, "enable"))
		rssi_start_req->enable = simple_strtol(value, NULL, 10);
	else if (!strcasecmp(name, "debug"))
		rssi_start_req->debug = simple_strtol(value, NULL, 10);
	else if (!strcasecmp(name, "max-num"))
		rssi_start_req->max_num = simple_strtol(value, NULL, 10);
}

void parse_tx_snd_param(char *name, char *value, struct cal_sounding_req *txsnd_param)
{
	if (!strcasecmp(name, "en-sample"))
		txsnd_param->en_sample = simple_strtol(value, NULL, 0);
	else if (!strcasecmp(name, "sample-num"))
		txsnd_param->num_sample = simple_strtol(value, NULL, 0);
}

void parse_test_uncache_rw_param(char *name, char *value, struct cal_test_uncache_rw_req *param)
{
	if (!strcasecmp(name, "len"))
		param->len = simple_strtol(value, NULL, 0);
}

void parse_action_param(enum cali_action_type action_type, char *buf, int len, void *param)
{
	char name[32];
	char value[128];
	char *pspace = NULL;
	char *pchar = buf;
	char *pend = buf + len - 1;

	while (pchar < buf + len) {
		memset(name, 0, sizeof(name));
		memset(value, 0, sizeof(value));
		while (*pchar == ' ')
			pchar++;
		if (pchar == pend)
			break;

		pspace = strchr(pchar, ' ');
		if (!pspace)
			break;
		strncpy(name, pchar, pspace - pchar);
		pchar = pspace + 1;
		while (*pchar == ' ')
			pchar++;
		pspace = strchr(pchar, ' ');
		if (!pspace)
			strncpy(value, pchar, buf + len - pchar);
		else
			strncpy(value, pchar, pspace - pchar);

		if (action_type == TYPE_ACTION_TXSU)
			parse_tx_su_param(name, value, (struct cal_tx_su_req *)param);
		else if (action_type == TYPE_ACTION_DIFSAMPLE)
			parse_dif_sample_param(name, value, (struct cal_dif_sample_req *)param);
		else if (action_type == TYPE_ACTION_LOG_SET)
			parse_log_set_param(name, value, (struct cal_log_set_req *)param);
		else if (action_type == TYPE_ACTION_TXMU)
			parse_tx_mu_param(name, value, (struct cal_tx_mu_req *)param);
		else if (action_type == TYPE_ACTION_TXSND)
			parse_tx_snd_param(name, value, (struct cal_sounding_req *)param);
		else if (action_type == TYPE_ACTION_RADAR_DETECT)
			parse_radar_detect_param(name, value, (struct cal_radar_detect_req *)param);
		else if (action_type == TYPE_ACTION_INTERFERENCE_DETECT)
			parse_interference_detect_param(name, value, (struct cal_interference_detect_req *)param);
		else if (action_type == TYPE_ACTION_RSSI)
			parse_start_rssi_param(name, value, (struct cal_rssi_start_req *)param);
		else if (action_type == TYPE_ACTION_TEST_UNCACHE_RW)
			parse_test_uncache_rw_param(name, value,
				(struct cal_test_uncache_rw_req *)param);

		if (pspace)
			pchar = pspace + 1;
	}

}

#if defined(CFG_MERAK3000)
int set_global_mld_param(char *name, char *pval, struct cali_config_tag *ptag)
{
	int ret = 0;
	struct cali_config_tag *pcurrent = ptag;

	if (!strcasecmp(name, "chipset-id")) {
		ret = 1;
		pcurrent->chipset_id = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "mlo-enable")) {
		ret = 1;
		pcurrent->mlo_enable = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "chipset-number")) {
		ret = 1;
		pcurrent->chipset_number = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "working-ap-mld-id")) {
		ret = 1;
		pcurrent->working_ap_mld_id = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "transition-timeout")) {
		ret = 1;
		pcurrent->transition_timeout = simple_strtol(pval, NULL, 10);
	} else if (!strcasecmp(name, "hw-agg-enable")) {
		ret = 1;
		pcurrent->hw_agg_enable = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "hw-agg-dup")) {
		ret = 1;
		pcurrent->hw_agg_dup = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "group-win--start")) {
		ret = 1;
		pcurrent->group_win_start = simple_strtol(pval, NULL, 0);
	} else if (!strcasecmp(name, "group-win-end")) {
		ret = 1;
		pcurrent->group_win_end = simple_strtol(pval, NULL, 0);
	}

	return ret;
}

int set_ap_mld_param(char *name, char *pval, struct cali_ap_mld_tag *ap_mld_info)
{
	int ret = 0;
	struct cali_ap_mld_tag *pcurrent = ap_mld_info;

	if (!strcasecmp(name, "mld-addr")) {
		ret = 1;
		mac_str_to_bin(pval, pcurrent->mld_addr);
		pcurrent->is_mld = 1;
	} else if (!strcasecmp(name, "ap-mld-id")) {
		ret = 1;
		pcurrent->ap_mld_id = simple_strtol(pval, NULL, 10);
		pcurrent->is_mld = 1;
	} else if (!strcasecmp(name, "ap-mld-tbl-idx")) {
		ret = 1;
		pcurrent->ap_mld_tbl_idx = simple_strtol(pval, NULL, 10);
		pcurrent->is_mld = 1;
	} else if (!strcasecmp(name, "valid-links")) {
		ret = 1;
		pcurrent->valid_links = simple_strtol(pval, NULL, 0);
		pcurrent->is_mld = 1;
	} else if (!strcasecmp(name, "is-mld")) {
		ret = 1;
		pcurrent->is_mld = simple_strtol(pval, NULL, 10);
	}

	return ret;
}

int set_sta_mld_param(char *name, char *pval, struct cali_sta_mld_tag *sta_mld_info)
{
	int ret = 0;
	struct cali_sta_mld_tag *pcurrent = sta_mld_info;

	if (!strcasecmp(name, "mld-addr")) {
		ret = 1;
		mac_str_to_bin(pval, pcurrent->mld_addr);
		pcurrent->is_mld = 1;
	} else if (!strcasecmp(name, "sta-mlo-mode")) {
		pcurrent->sta_mlo_mode = string_to_mlo_mode(pval);
		ret = 1;
		pcurrent->is_mld = 1;
	} else if (!strcasecmp(name, "valid-links")) {
		ret = 1;
		pcurrent->valid_links = simple_strtol(pval, NULL, 0);
		pcurrent->is_mld = 1;
	} else if (!strcasecmp(name, "sta-mld-tbl-idx")) {
		ret = 1;
		pcurrent->sta_mld_tbl_idx = simple_strtol(pval, NULL, 0);
		pcurrent->is_mld = 1;
	} else if (!strcasecmp(name, "eml-padding-delay")) {
		ret = 1;
		pcurrent->eml_padding_delay = simple_strtol(pval, NULL, 0);
		pcurrent->is_mld = 1;
	} else if (!strcasecmp(name, "eml-transition-delay")) {
		ret = 1;
		pcurrent->eml_transition_delay = simple_strtol(pval, NULL, 0);
		pcurrent->is_mld = 1;
	} else if (!strcasecmp(name, "eml-init-frame-type")) {
		pcurrent->eml_init_frame_type = string_to_init_frame_type(pval);
		ret = 1;
		pcurrent->is_mld = 1;
	} else if (!strcasecmp(name, "is-mld")) {
		ret = 1;
		pcurrent->is_mld = simple_strtol(pval, NULL, 10);
	}

	return ret;

}

int set_tdma_param(char *name, char *pval, struct cali_tdma_tag *ptag)
{
	struct cali_tdma_tag *pcurrent = ptag;
	int val = 0;
	int ret = 0;

	if (!strcasecmp(name, "tdma-en")) {
		ret = 1;
		val = simple_strtol(pval, NULL, 10);
		pcurrent->tdma_en = !!val;
	} else if (!strcasecmp(name, "mode")) {
		ret = 1;
		val = simple_strtol(pval, NULL, 0);
		pcurrent->tdma_mode = val;
	} else if (!strcasecmp(name, "ack-duration")) {
		ret = 1;
		val = simple_strtol(pval, NULL, 10);
		pcurrent->tdma_ack_duration = val;
	}

	return ret;
}

#endif // CFG_MERAK3000

void set_single_param(enum cali_cmd_type type, int idx, char *name, char *value, struct cali_config_tag *ptag)
{
	if (*name == 0 || *value == 0)
		return;

	switch (type) {
	case TYPE_RADIO:
		set_radio(name, value, ptag);
		break;
	case TYPE_BSS:
		set_bss_param(name, value, ptag);
		break;
	case TYPE_GTK:
		set_pgtk_param(name, value, &ptag->bss_info.key_gtk);
		break;
	case TYPE_IGTK:
		set_pgtk_param(name, value, &ptag->bss_info.key_igtk[0]);
		break;
	case TYPE_SU:
		if (set_user_ppdu_param(name, value, &ptag->su_user_info.ppdu_info))
			break;
		if (set_user_info_param(name, value, &ptag->su_user_info))
			break;
		set_user_mac_phy_param(name, value, &ptag->su_mac_phy_params);
		break;
	case TYPE_MU:
		if (set_user_ppdu_param(name, value, &ptag->mu_user_info[idx].ppdu_info))
			break;
		if (set_user_info_param(name, value, &ptag->mu_user_info[idx]))
			break;
		set_user_mac_phy_param(name, value, &ptag->mu_mac_phy_params[idx]);
		break;
	case TYPE_MACPHY:
		set_macphy_param(name, value, ptag);
		break;
	case TYPE_MUINFO:
		set_muinfo_param(name, value, ptag);
		break;
	case TYPE_SOUNDING:
		set_sounding_param(name, value, ptag);
		break;
	case TYPE_WMM:
		set_wmm_param(name, value, &ptag->wmm[idx]);
		break;
	case TYPE_RXSTATS:
		set_rxstats_param(name, value, ptag);
		break;
	case TYPE_CSI:
		set_csi_param(name, value, &ptag->csi_info);
		break;
	case TYPE_PPPC:
		set_pppc_param(name, value, &ptag->pppc_info);
		break;
#if defined(CFG_MERAK3000)
	case TYPE_MLD:
		set_global_mld_param(name, value, ptag);
		break;
	case TYPE_AP_MLD:
		set_ap_mld_param(name, value, &ptag->ap_mld_info[idx]);
		break;
	case TYPE_STA_MLD:
		set_sta_mld_param(name, value, &ptag->sta_mld_info[idx]);
		break;
	case TYPE_TDMA:
		set_tdma_param(name, value, &ptag->tdma_info);
#endif // CFG_MERAK3000
	default:
		break;
	}
}

void get_param(char *name, char *out_buf, struct cali_config_tag *ptag)
{
	struct cali_config_tag *pcurrent = NULL;

	if (!name || !out_buf)
		return;

	pcurrent = ptag;

	if (!strcmp(name, "radio"))
		sprintf(out_buf, "%s = %s\n", name, radio_list[pcurrent->bss_info.radio]);

	if (!strcmp(name, "mode"))
		sprintf(out_buf, "%s = %s\n", name, mode_list[pcurrent->bss_info.mac_mode]);

	if (!strcmp(name, "channel"))
		sprintf(out_buf, "%s = %d\n", name, pcurrent->bss_info.chan_ieee);

	if (!strcmp(name, "bw"))
		sprintf(out_buf, "%s = %d\n", name, pcurrent->bss_info.bw);

	if (strstr(name, "phy-mode"))
		sprintf(out_buf, "%s = %s\n", name, mode_list[pcurrent->bss_info.phy_mode]);

	if (strstr(name, "bss_mac_addr"))
		sprintf(out_buf, "%s = %02x:%02x:%02x:%02x:%02x:%02x\n", name,
				pcurrent->bss_info.local_addr[0], pcurrent->bss_info.local_addr[1],
				pcurrent->bss_info.local_addr[2], pcurrent->bss_info.local_addr[3],
				pcurrent->bss_info.local_addr[4], pcurrent->bss_info.local_addr[5]);

	if (strstr(name, "ciper"))
		sprintf(out_buf, "%s = %s-%d\n", name, ciper_list[pcurrent->bss_info.key_gtk.cipher],
				pcurrent->bss_info.key_gtk.key_len);

	if (strstr(name, "wep-index")) {
		if (pcurrent->bss_info.key_gtk.cipher == 1)
			sprintf(out_buf, "%s = %d\n", name, pcurrent->bss_info.key_gtk.key_index);
	}
	if (strstr(name, "key-len"))
		sprintf(out_buf, "%s = %d\n", name, pcurrent->bss_info.key_gtk.key_len);

	if (strstr(name, "key")) {
		int i = 0;
		char temp[8];

		for (; i < 32; i++) {
			if (pcurrent->bss_info.key_gtk.key[i] == 0)
				break;
			sprintf(temp, "%02x ", pcurrent->bss_info.key_gtk.key[i]);
			strcat(out_buf, temp);
		}
	}
	if (strstr(name, "key-hw-index"))
		sprintf(out_buf, "%s = %d\n", name, pcurrent->bss_info.key_gtk.key_index);
}

void get_regadd_len(char *buf, uint32_t *reg_addr, int *reg_len)
{
	char *pspace = NULL;

	if (!buf)
		return;
	pspace = strchr(buf, ' ');
	if (!pspace)
		return;
	*pspace = 0;
	pspace += 1;
	while (*pspace == ' ')
		pspace++;
	*reg_addr = simple_strtoul(buf, NULL, 0);
	*reg_len = simple_strtoul(pspace, NULL, 10);
}

void get_regaddr_value(char *buf, int buf_len, uint32_t *reg_addr, uint32_t *valarry, int *num)
{
	int i = 0;
	char *pch = buf;
	char *pnext = NULL;

	pnext = strchr(pch, ' ');
	while (pnext) {
		*pnext = 0;
		if (i == 0)
			*reg_addr = simple_strtoul(pch, NULL, 0);
		else
			*(valarry + i - 1) = simple_strtoul(pch, NULL, 0);
		i++;
		pch = pnext + 1;
		while (*pch == ' ')
			pch++;
		pnext = strchr(pch, ' ');
	}
	if (i >= 1) {
		if (pch < buf + buf_len) {
			*(valarry + i - 1) = simple_strtoul(pch, NULL, 0);
			i++;
		}
	}
	*num = i - 1;
}

int show_reg_output(char *buf, int bufsize, uint32_t regaddr, uint32_t *regval, int num)
{
	int i = 0;
	int len = 0;

	for (; i < num; i++) {
		if (i == num - 1)
			len += scnprintf(buf + len, bufsize - len, "\n0x%08x: 0x%x\n", (regaddr + i * 4), regval[i]);
		else
			len += scnprintf(buf + len, bufsize - len, "\n0x%08x: 0x%x", (regaddr + i * 4), regval[i]);
	}
	return len;
}
#ifdef __KERNEL__
static int wifi_calcmd_open(struct inode *inode, struct file *filp)
{
	filp->private_data = inode->i_private;
	return 0;
}

static ssize_t wifi_calcmd_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{

	return simple_read_from_buffer(buf, count, ppos, calcmd, strlen(calcmd));

}
#endif

#ifndef __KERNEL__
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

static int hex2byte(const char *hex)
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

static int hexstr2bin(const char *hex, u8 *buf, size_t len)
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
#endif

#if defined(CFG_MERAK3000)
void cls_wifi_handle_time_sync(char *calcmd, int radio_index)
{
	// TODO. suppor it later
}

void cls_wifi_handle_time_get(char *calcmd, int radio_index)
{
	uint32_t us_low, us_hi, ns;

	if (cls_wifi_cal_leaf_timer_read(radio_index, &us_low, &us_hi, &ns)) {
		pr_err("read leaf-timer failed\n");
		return;
	}

	pr_err("tick: %08x %08x, ns: %08x\r\n", us_low, us_hi, ns);
}

#endif // CFG_MERAK3000


#ifdef __KERNEL__
static ssize_t wifi_calcmd_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
#else
int wifi_calcmd_write(char *buf, int count)
#endif
{
	int len = 0;
	int idx_multi = 0;
	enum cali_cmd_type set_type = TYPE_MAX;
	enum cali_cmd_type show_type = TYPE_MAX;
	enum cali_action_type action_type = TYPE_ACTION_MAX;
	char type[16];
	char show_name[32];
	char cmd_name[32];
	int radio_index;
	int phy_band;
	struct cls_wifi_hw *cls_wifi_hw;
	int payload_pattern = 0;
	int payload_type = 0;
	char *pch = NULL;
	char *pend = NULL;
	char *ptype = NULL;
	int array_len = 0;
	int i = 0;

	memset(calcmd, 0, sizeof(calcmd));

	if (count > MAX_CMD_SIZE)
		count = MAX_CMD_SIZE - 1;
#ifndef __KERNEL__
	memcpy(calcmd, buf, count);
    current_radio = g_cls_phy_band;
#else
	if (copy_from_user(calcmd, buf, count))
		return -EFAULT;
#endif
	if (current_radio == RADIO_2P4G_INDEX) {
		radio_index = RADIO_2P4G_INDEX;
		phy_band = CALI_PHY_BAND_2G4;
		cls_wifi_hw = g_radio_cls_wifi_hw[radio_index];
	} else if (current_radio == RADIO_5G_INDEX) {
		radio_index = RADIO_5G_INDEX;
		phy_band = CALI_PHY_BAND_5G;
		cls_wifi_hw = g_radio_cls_wifi_hw[radio_index];
#ifndef __KERNEL__
    } else {
        yc_printf("%s", "unsupported radio");
        return -1;
#endif
    }

	len = strlen(calcmd);
	pend = calcmd + (len - 1);
	while ((*pend == ' ') || (*pend == '\n')) {
		*pend = 0;
		len--;
		pend--;
	}

	memset(cmd_name, 0, sizeof(cmd_name));
	pch = strchr(calcmd, ' ');
	if (pch)
		strncpy(cmd_name, calcmd, pch - calcmd);
#ifndef __KERNEL__
 else {
	//strncpy(cmd_name, calcmd, len);
	if (len < 32) {
		memcpy(cmd_name, calcmd, len);
		cmd_name[len] = 0;
	} else {
		yc_printf("Invalid-cmd(%s)\r\n", calcmd);
		return -1;
	}
    }
#else
	else
		strncpy(cmd_name, calcmd, len);
#endif


#if CAL_DBG
			pr_err("current_radio : %d, cmd_len: %d\n", current_radio, len);
			pr_err("calcmd:[%s]\n", calcmd);
#endif

	array_len = ARRAY_SIZE(cmd_type);
	if (!strcasecmp(cmd_name, "set")) {
		pch = strchr(calcmd, ' ');
		if (!pch)
			return -1;
		pch += 1;
		while (*pch == ' ')
			pch++;

		memset(type, 0, sizeof(type));
		ptype = strchr(pch, ' ');
		if (!ptype)
			return -1;
		strncpy(type, pch, ptype - pch);
		for (i = 0; i < array_len; i++) {
			if (!strcasecmp(type, cmd_type[i]))
				break;
		}

		if (i < array_len) {
			set_type = i;
#if CAL_DBG
		pr_err("%s: set_type= %d\n", __func__, set_type);
#endif
			if (set_type == TYPE_RADIO || set_type == TYPE_RXSTATS) {
				len -= (pch - calcmd);
				set_all_param(set_type, idx_multi, pch, len, &g_cal_config[0]);
				return count;
			}
			if (current_radio == RADIO_2P4G_INDEX) {
				radio_index = RADIO_2P4G_INDEX;
				phy_band = CALI_PHY_BAND_2G4;
				cls_wifi_hw = g_radio_cls_wifi_hw[radio_index];
			} else if (current_radio == RADIO_5G_INDEX) {
				radio_index = RADIO_5G_INDEX;
				phy_band = CALI_PHY_BAND_5G;
				cls_wifi_hw = g_radio_cls_wifi_hw[radio_index];
			} else {
				pr_err("Please configure radio firstly\n");
				return -1;
			}
			if ((set_type == TYPE_MU) || (set_type == TYPE_WMM) ||
						(set_type == TYPE_AP_MLD) ||
						(set_type == TYPE_STA_MLD)) {
				pch += strlen(cmd_type[i]);
				while (*pch == ' ')
					pch++;
				if (sscanf(pch, "%d ", &idx_multi) > 0 && idx_multi > 0) {
					idx_multi -= 1;
					pch = strchr(pch, ' ');
					if (!pch)
						return -1;
					pch += 1;
					len -= pch - calcmd;
					set_all_param(set_type, idx_multi, pch, len, &g_cal_config[current_radio]);
				}
			} else {
				pch += strlen(cmd_type[i]);
				len -= pch - calcmd;
				set_all_param(set_type, idx_multi, pch, len, &g_cal_config[current_radio]);
			}
		}
	} else if (!strcasecmp(cmd_name, "update")) {
		pr_err("Calibration Task ID: %d, Config ID: %d, CLen=%zu\n",
				TASK_CAL, CAL_PARAM_CONFIG_REQ, sizeof(struct cali_config_tag));

		g_cal_config[radio_index].magic = CALI_CFG_MAGIC;
		cls_wifi_send_cal_param_config_req(cls_wifi_hw, radio_index,
				CAL_PARAM_BASIC, sizeof(g_cal_config[0]),
				&g_cal_config[radio_index], 0);
#ifdef __KERNEL__
		cls_wifi_dif_boot_cali(cls_wifi_hw);
#endif
		g_cal_config[radio_index].su_user_info.ppdu_info.ssn_reset = 0;
	} else if (!strcasecmp(cmd_name, "update-params-only")) {
		pr_err("Calibration Task ID: %d, Config ID: %d, CLen=%ld\n",
				TASK_CAL, CAL_PARAM_CONFIG_REQ, sizeof(struct cali_config_tag));

		// g_cal_config[radio_index].magic = CALI_CFG_MAGIC;
		cls_wifi_send_cal_param_update_only_req(cls_wifi_hw, radio_index,
			CAL_PARAM_BASIC, sizeof(g_cal_config[0]), &g_cal_config[radio_index], 0);
		// g_cal_config[radio_index].su_user_info.ppdu_info.ssn_reset = 0;
	} else if (!strcasecmp(cmd_name, "reset")) {
		pr_err("Calibration Task ID: %d, Config ID: %d, CLen=%zu\n",
				TASK_CAL, CAL_PARAM_CONFIG_REQ, sizeof(struct cali_config_tag));

		cali_set_default_config(&g_cal_config[radio_index], phy_band);
		g_cal_config[radio_index].magic = CALI_CFG_MAGIC;
		cls_wifi_send_cal_param_config_req(cls_wifi_hw, radio_index,
				CAL_PARAM_BASIC, sizeof(g_cal_config[0]),
				&g_cal_config[radio_index], 0);
	} else if (!strcasecmp(cmd_name, "tx-su") || (!strcasecmp(cmd_name, "tx-mld")) ||
					(!strcasecmp(cmd_name, "tx-hw-agg"))) {
		struct cal_tx_su_req tx_su_param;

		memset(&tx_su_param, 0, sizeof(struct cal_tx_su_req));
		tx_su_param.en_sample = 1;
		tx_su_param.eof_padding = 8;
		action_type = TYPE_ACTION_TXSU;
		payload_pattern = g_cal_config[radio_index].su_user_info.ppdu_info.payload_pattern;
		payload_type = (g_cal_config[radio_index].su_user_info.ppdu_info.frame_ctrl & 0xf)>>2;
		pr_err("Calibration Task ID: %d, Config ID: %d, payload_pattern: %d\n",
					TASK_CAL, CAL_TX_SU_REQ, payload_pattern);

		pch = strchr(calcmd, ' ');
		if (pch) {
			pch++;
			len -= pch - calcmd;
			parse_action_param(action_type, pch, len, (void *)&tx_su_param);
#if CAL_DBG
			pr_err("tx-su: en_sample[%d], num_sample[%d]\n",
					tx_su_param.en_sample, tx_su_param.num_sample);
#endif
		}

		if (payload_pattern <= 0xFF || payload_pattern == 0x2FF) {
			tx_su_param.payload_len = 0;
			cls_wifi_send_cal_tx_su_req(cls_wifi_hw, &tx_su_param);
#ifdef __KERNEL__
			if(!tx_su_param.terminate){
				cls_wifi_dif_trigger_online_cali_once(cls_wifi_hw);
			}
#endif
		} else {
			u32 offset = offsetof(struct ipc_shared_env_tag, data_a2e_buf);
			int len = 0;

			len = read_payload_from_file(cls_wifi_hw, payload_type, offset);

			if (len) {
				tx_su_param.payload_len = len;
				cls_wifi_send_cal_tx_su_req(cls_wifi_hw, &tx_su_param);
#ifdef __KERNEL__
				if(!tx_su_param.terminate){
					cls_wifi_dif_trigger_online_cali_once(cls_wifi_hw);
				}
#endif
			} else
				pr_err("no tx when the file is empty\n");
		}
	} else if (!strcasecmp(cmd_name, "tx-mu")) {
		struct cal_tx_mu_req tx_mu_param;

		memset(&tx_mu_param, 0, sizeof(struct cal_tx_mu_req));
		tx_mu_param.en_sample = 1;
		action_type = TYPE_ACTION_TXMU;
		pch = strchr(calcmd, ' ');
		if (pch) {
			pch++;
			len -= pch - calcmd;
			parse_action_param(action_type, pch, len, (void *)&tx_mu_param);
#if CAL_DBG && defined(CFG_MERAK3000)
			pr_err("tx-mu: en_sample[%d], num_sample[%d], tx-tick(%08x)\n",
				tx_mu_param.en_sample, tx_mu_param.num_sample, tx_mu_param.mld_params.tx_tick);
#else
			pr_err("tx-mu: en_sample[%d], num_sample[%d]\n",
				tx_mu_param.en_sample, tx_mu_param.num_sample);
#endif
		}

		cls_wifi_send_cal_tx_mu_req(cls_wifi_hw, &tx_mu_param);
	} else if (!strcasecmp(cmd_name, "sounding")) {
		struct cal_sounding_req snd_param;

		pr_err("Calibration Task ID: %d, Config ID: %d\n",
					TASK_CAL, CAL_SOUNDING_REQ);

		memset(&snd_param, 0, sizeof(struct cal_sounding_req));
		snd_param.en_sample = 1;
		action_type = TYPE_ACTION_TXSND;
		pch = strchr(calcmd, ' ');
		if (pch) {
			pch++;
			len -= pch - calcmd;
			parse_action_param(action_type, pch, len, (void *)&snd_param);
#if CAL_DBG
			pr_err("sounding: en_sample[%d], num_sample[%d]\n",
					snd_param.en_sample, snd_param.num_sample);
#endif
		}

		cls_wifi_send_cal_sounding_req(cls_wifi_hw, &snd_param);
	} else if (!strcasecmp(cmd_name, "show")) {
		memset(out_buf, 0, sizeof(out_buf));
		memset(show_name, 0, sizeof(show_name));
		pch = strchr(calcmd, ' ');
		if (!pch)
			return -1;
		pch += 1;
		while (*pch == ' ')
			pch++;

		memset(type, 0, sizeof(type));
		ptype = strchr(pch, ' ');
		if (!ptype)
			strncpy(type, pch, calcmd + len - pch);
		else
			strncpy(type, pch, ptype - pch);

		for (i = 0; i < array_len; i++) {
			if (!strcasecmp(type, "rx-stats"))
				continue;
			if (!strcasecmp(type, cmd_type[i]))
				break;
		}

		if (i < array_len) {
			char *pend = NULL;

			pch += strlen(cmd_type[i]);
			show_type = i;
			while (*pch == ' ')
				pch++;
			pend = strchr(pch, ' ');
			if (pend) {
				*pend = 0;
				strncpy(show_name, pch, pend - pch);
			} else {
				if (pch < calcmd + len)
					strncpy(show_name, pch, calcmd + len - pch);
			}
			out_buf_len = show_param(show_type, show_name, out_buf,
							sizeof(out_buf) - 1, &g_cal_config[current_radio]);
#if CAL_DBG
			pr_err("show type : %d, show_name:[%s], out_buf_len:%d\n",
					show_type, show_name, out_buf_len);
#endif
#ifndef __KERNEL__
#if !defined(CFG_MERAK2000)
			yc_printf("%s", out_buf);
#else
            uart_puts(out_buf);
#endif
#endif
		} else {
			if (!strncasecmp(pch, "rx-last", strlen("rx-last"))) {
				pr_err("Calibration Task ID: %d, Config ID: %d, radio_index:%d\n",
					TASK_CAL, CAL_RX_STATUS_REQ, radio_index);

				cls_wifi_send_cal_rx_status_req(cls_wifi_hw, radio_index);
#ifndef __KERNEL__
                out_buf_len = show_status_rx(out_buf, sizeof(out_buf) - 1,
					(struct cal_per_radio_status *)cls_wifi_hw->ipc_env->shared->data_e2a_buf);
#else
				out_buf_len = show_status_rx(out_buf, sizeof(out_buf) - 1, &g_rx_status);
#endif
			} else if (!strncasecmp(pch, "rx-stats", strlen("rx-stats"))) {
				uint32_t clear = 0;
				uint32_t show_mpdu_num = 0;
				pch += strlen("rx-stats");
				if (*pch != 0) {
					while (*pch == ' ')
						pch++;
					if (!strcasecmp(pch, "clear"))
						clear = 1;
					else if (!strcasecmp(pch, "get_num_only"))
						show_mpdu_num = 1;
				}
#ifdef __KERNEL__
				memset(&g_rx_stats, 0, sizeof(g_rx_stats));
#endif
				pr_err("Calibration Task ID: %d, Config ID: %d, radio_index:%d, clear:%u\n",
					TASK_CAL, CAL_RX_STATS_REQ, radio_index, clear);
				cls_wifi_send_cal_rx_stats_req(cls_wifi_hw, radio_index, clear);
#ifndef __KERNEL__
				if (show_mpdu_num)
					out_buf_len = show_stats_rx_mpdu_num(out_buf, sizeof(out_buf) - 1,
							(struct cal_per_radio_stats *)cls_wifi_hw->ipc_env->shared->data_e2a_buf);
				 else
					out_buf_len = show_stats_rx(out_buf, sizeof(out_buf) - 1,
							(struct cal_per_radio_stats *)cls_wifi_hw->ipc_env->shared->data_e2a_buf);
#else
				if (show_mpdu_num)
					out_buf_len = show_stats_rx_mpdu_num(out_buf, sizeof(out_buf) - 1, &g_rx_stats);
				else
					out_buf_len = show_stats_rx(out_buf, sizeof(out_buf) - 1, &g_rx_stats);
#endif
#ifndef __KERNEL__
#if !defined(CFG_MERAK2000)
				yc_printf("%s", out_buf);
#else
                uart_puts(out_buf);
#endif
#endif
			} else if (!strncasecmp(pch, "tx-stats", strlen("tx-stats"))) {
				bool clear = false;
				pr_err("Calibration Task ID: %d, Config ID: %d\n",
					TASK_CAL, CAL_TX_STATS_REQ);

				pch += strlen("tx-stats");
				if (*pch != 0) {
					while (*pch == ' ')
						pch++;
					if (!strcasecmp(pch, "clear"))
						clear = true;
				}
				cls_wifi_send_cal_tx_stats_req(cls_wifi_hw, clear);
#ifndef __KERNEL__
				out_buf_len = show_stats_tx(out_buf, sizeof(out_buf) - 1,
						(struct cal_tx_stats *)cls_wifi_hw->ipc_env->shared->data_e2a_buf);
#if !defined(CFG_MERAK2000)
				yc_printf("%s", out_buf);
#else
				uart_puts(out_buf);
#endif /*CFG_MERAK2000*/
#else
				out_buf_len = show_stats_tx(out_buf, sizeof(out_buf) - 1, &g_tx_stats);
#endif
			} else if (!strncasecmp(pch, "rssi-stats-avg", strlen("rssi-stats-avg"))) {
				pr_err("Calibration Task ID: %d, Config ID: %d\n",
					TASK_CAL, CAL_RSSI_STATUS_REQ);

				cls_wifi_send_rssi_stats_req(cls_wifi_hw, 1, 0);
				out_buf_len = show_stats_rssi(out_buf, sizeof(out_buf) - 1,
						cls_wifi_hw->cal_env->rssi_status_cfm.status,
						cls_wifi_hw->cal_env->rssi_status_cfm.rssi);
			} else if (!strncasecmp(pch, "rssi-stats", strlen("rssi-stats"))) {
				pr_err("Calibration Task ID: %d, Config ID: %d\n",
					TASK_CAL, CAL_RSSI_STATUS_REQ);

				cls_wifi_send_rssi_stats_req(cls_wifi_hw, 0, 0);
				out_buf_len = show_stats_rssi(out_buf, sizeof(out_buf) - 1,
						cls_wifi_hw->cal_env->rssi_status_cfm.status,
						cls_wifi_hw->cal_env->rssi_status_cfm.rssi);
			} else if (!strncasecmp(pch, "leg-rssi-stats-avg",
						strlen("leg-rssi-stats-avg"))) {
				pr_err("Calibration Task ID: %d, Config ID: %d\n",
					TASK_CAL, CAL_RSSI_STATUS_REQ);

				cls_wifi_send_rssi_stats_req(cls_wifi_hw, 1, 1);
				out_buf_len = show_stats_rssi(out_buf, sizeof(out_buf) - 1,
						cls_wifi_hw->cal_env->rssi_status_cfm.status,
						cls_wifi_hw->cal_env->rssi_status_cfm.rssi);
			} else if (!strncasecmp(pch, "leg-rssi-stats", strlen("leg-rssi-stats"))) {
				pr_err("Calibration Task ID: %d, Config ID: %d\n",
					TASK_CAL, CAL_RSSI_STATUS_REQ);

				cls_wifi_send_rssi_stats_req(cls_wifi_hw, 0, 1);
				out_buf_len = show_stats_rssi(out_buf, sizeof(out_buf) - 1,
						cls_wifi_hw->cal_env->rssi_status_cfm.status,
						cls_wifi_hw->cal_env->rssi_status_cfm.rssi);
			} else if (!strncasecmp(pch, "temperature", strlen("temperature"))) {
				cls_wifi_send_get_temp_req(cls_wifi_hw);
				out_buf_len = show_temperature(out_buf, sizeof(out_buf) - 1,
						cls_wifi_hw->cal_env->get_temp_cfm.status,
						cls_wifi_hw->cal_env->get_temp_cfm.temp);
			} else if (!strncasecmp(pch, "csi-last", strlen("csi-last"))) {
				pr_err("Rx last CSI\n");
				cls_wifi_send_cal_get_csi_req(cls_wifi_hw, radio_index);
#ifndef __KERNEL__
				out_buf_len = show_csi_result(out_buf, sizeof(out_buf) - 1,
					cls_wifi_hw->cal_env->get_csi_cfm.status,
					(struct cls_csi_report *)cls_wifi_hw->ipc_env->shared->data_e2a_buf);
#else
				out_buf_len = show_csi_result(out_buf, sizeof(out_buf) - 1,
					cls_wifi_hw->cal_env->get_csi_cfm.status,
					&g_cali_csi_report);
#endif

			}
		}
	} else if (!strcasecmp(cmd_name, "write-mem")) {
		uint32_t val_buf[CAL_MEM_OP_WR_LEN_MAX];
		uint32_t reg_addr;
		int val_num = 0;
#ifdef __KERNEL__
		int radio_idx = radio_index;
#else
		int radio_idx;
#endif
		pch = strchr(calcmd, ' ');
		if (!pch)
			return -EINVAL;
		while (*pch == ' ')
			pch += 1;
		get_regaddr_value(pch, len - (pch - calcmd), &reg_addr, val_buf, &val_num);
		if (val_num <= 0)
			return -EINVAL;
#ifdef __KERNEL__
#if !defined(MERAK2000)
		if (((reg_addr >= WIFI40_BASE_ADDR) &&
					(reg_addr < WIFI160_BASE_ADDR)) ||
				((reg_addr >= DIF40_BASE_ADDR) &&
					(reg_addr < DIF160_BASE_ADDR)))
			radio_idx = RADIO_2P4G_INDEX;
		else if (((reg_addr >= WIFI160_BASE_ADDR) &&
					(reg_addr < WIFI160_BASE_ADDR + WIFI_MEM_SIZE)) ||
				((reg_addr >= DIF160_BASE_ADDR) &&
						(reg_addr < DIF160_BASE_ADDR + DIF_MEM_SIZE)))
			radio_idx = RADIO_5G_INDEX;
		else
			return -EINVAL;
#endif
#else
		radio_idx = current_radio;
#endif

		if (cls_wifi_cal_mem_write(radio_idx, reg_addr, val_num, val_buf)) {
			pr_err("write memory failed, (0x%x-%d)\n", reg_addr, val_num);
			return -EINVAL;
		}
	} else if (!strcasecmp(cmd_name, "read-mem")) {
		uint32_t reg_addr;
		int reg_num = -1;
#ifdef __KERNEL__
		int radio_idx = radio_index;
		uint32_t *val_buf = NULL;
#else
		int radio_idx;
		uint32_t val_buf[CAL_MEM_OP_RD_LEN_MAX];
#endif

		pch = strchr(calcmd, ' ');
		if (!pch)
			return -EINVAL;
		while (*pch == ' ')
			pch += 1;
		get_regadd_len(pch, &reg_addr, &reg_num);
		if (reg_num <= 0)
			return -EINVAL;
#ifdef __KERNEL__
		val_buf = kzalloc(reg_num * sizeof(u32), GFP_KERNEL);
#if !defined(MERAK2000)
		if (((reg_addr >= WIFI40_BASE_ADDR) &&
					(reg_addr < WIFI160_BASE_ADDR)) ||
				((reg_addr >= DIF40_BASE_ADDR) &&
					(reg_addr < DIF160_BASE_ADDR)))
			radio_idx = RADIO_2P4G_INDEX;
		else if (((reg_addr >= WIFI160_BASE_ADDR) &&
					(reg_addr < WIFI160_BASE_ADDR + WIFI_MEM_SIZE)) ||
				((reg_addr >= DIF160_BASE_ADDR) &&
						(reg_addr < DIF160_BASE_ADDR + DIF_MEM_SIZE)))
			radio_idx = RADIO_5G_INDEX;
		else {
#ifdef __KERNEL__
			kfree(val_buf);
#endif
			return -EINVAL;
		}
#endif
#else
		radio_idx = current_radio;
#endif
		if (cls_wifi_cal_mem_read(radio_idx, reg_addr, reg_num, val_buf)) {
			pr_err("read memory failed, (0x%x-%d)\n", reg_addr, reg_num);
#ifdef __KERNEL__
			kfree(val_buf);
#endif
			return -EINVAL;
		}
		out_buf_len = show_reg_output(out_buf, sizeof(out_buf) - 1, reg_addr, val_buf, reg_num);
#ifndef  __KERNEL__
		uart_puts(out_buf);
#else
		kfree(val_buf);
#endif
#ifdef __KERNEL__
	} else if (!strcasecmp(cmd_name, "reload")) {
		pr_err("reload default config from %s\n", CALI_CONFIG_FW_NAME);
		set_def_cal_config(&g_cal_config[radio_index], CALI_CONFIG_FW_NAME, cls_wifi_hw->dev);
#endif
	} else if (!strcasecmp(cmd_name, "dif-sample")) {
		struct cal_dif_sample_req dif_param;

		memset(&dif_param, 0, sizeof(struct cal_dif_sample_req));
		action_type = TYPE_ACTION_DIFSAMPLE;
		pch = strchr(calcmd, ' ');
		if (pch) {
			pch++;
			len -= pch - calcmd;
			parse_action_param(TYPE_ACTION_DIFSAMPLE, pch, len, (void *)&dif_param);
#if CAL_DBG
			pr_err("dif_sample: num_sample[%d], chan[%d]\n",
				   dif_param.num_sample, dif_param.chan);
#endif
		}
	cls_wifi_send_cal_dif_smp_req(cls_wifi_hw, &dif_param);
	} else if (!strcasecmp(cmd_name, "radar-detect")) {
		struct cal_radar_detect_req radar_detect;
		memset(&radar_detect, 0, sizeof(struct cal_radar_detect_req));
		action_type = TYPE_ACTION_RADAR_DETECT;
		pch = strchr(calcmd, ' ');
		if (pch) {
			pch++;
			len -= pch - calcmd;
			parse_action_param(TYPE_ACTION_RADAR_DETECT, pch, len, (void *)&radar_detect);
#if CAL_DBG
			pr_err("radar-detect: enable[%d]\n", radar_detect.enable);
#endif
		}
		cls_wifi_send_radar_detect_req(cls_wifi_hw, &radar_detect);
	} else if (!strcasecmp(cmd_name, "interference-detect")) {
		struct cal_interference_detect_req interference_detect;
		memset(&interference_detect, 0, sizeof(struct cal_interference_detect_req));
		action_type = TYPE_ACTION_INTERFERENCE_DETECT;
		pch = strchr(calcmd, ' ');
		if (pch) {
			pch++;
			len -= pch - calcmd;
			parse_action_param(TYPE_ACTION_INTERFERENCE_DETECT, pch, len,
						(void *)&interference_detect);
#if CAL_DBG
			pr_err("interference-detect: enable[%d]\n", interference_detect.enable);
#endif
		}
		cls_wifi_send_interference_detect_req(cls_wifi_hw, &interference_detect);
	} else if (!strcasecmp(cmd_name, "log")) {
		struct cal_log_set_req set_req;

		memset(&set_req, 0, sizeof(set_req));

		pch = strchr(calcmd, ' ');
		if (pch) {
			pch++;
			len -= pch - calcmd;
			parse_action_param(TYPE_ACTION_LOG_SET, pch, len, (void *)&set_req);

			cls_wifi_send_cal_log_set_req(cls_wifi_hw, &set_req);
#if CAL_DBG
			pr_err("log: module=%u level=%u\n", set_req.log_module,
				set_req.log_level);
		}
#endif
	} else if (!strcasecmp(cmd_name, "hard-reset")) {
		cls_wifi_send_reset(cls_wifi_hw);
		cls_wifi_cal_fw_init_req(cls_wifi_hw, radio_index);
		cls_wifi_send_cal_param_config_req(cls_wifi_hw, radio_index,
				CAL_PARAM_BASIC, sizeof(g_cal_config[0]),
				&g_cal_config[radio_index], 0);
	} else if (!strcasecmp(cmd_name, "irf")) {
#ifndef __KERNEL__
	int irf_cmd_distribute(struct cls_wifi_hw *cls_wifi_hw,char *cmd_str);
#endif
		pch = strchr(calcmd, ' ');
		if (pch) {
			irf_cmd_distribute(cls_wifi_hw,pch+1);
		}
	} else if (!strcasecmp(cmd_name, "rssi")) {
		struct cal_rssi_start_req rssi_start_req;

		memset(&rssi_start_req, 0, sizeof(struct cal_rssi_start_req));
		action_type = TYPE_ACTION_RSSI;
		pch = strchr(calcmd, ' ');
		if (pch) {
			pch++;
			len -= pch - calcmd;
			parse_action_param(TYPE_ACTION_RSSI, pch, len,
						(void *)&rssi_start_req);
#if CAL_DBG
			pr_err("rssi_start: enable[%hhu] debug[%hhu] max_num[%u]\n",
					rssi_start_req.enable, rssi_start_req.debug,
					rssi_start_req.max_num);
#endif
		}
		cls_wifi_send_rssi_start_req(cls_wifi_hw, &rssi_start_req);
#ifdef __KERNEL__
	} else if (!strcasecmp(cmd_name, "test-uncache-rw")) {
		struct cal_test_uncache_rw_req req;

		pch = strchr(calcmd, ' ');
		if (pch) {
			pch++;
			len -= pch - calcmd;
			parse_action_param(TYPE_ACTION_TEST_UNCACHE_RW, pch, len, (void *)&req);

			cls_wifi_test_uncache_rw(cls_wifi_hw, &req);
		}
#endif
#ifndef __KERNEL__
	} else if (!strcasecmp(cmd_name, "msg")) {
		int hex_len = 0;
		char * string = NULL;
		u8 msg[MAX_CMD_SIZE/2] = {0};
		int len;
		pch = strchr(calcmd, ' ');
		if (!pch)
			return -1;
		pch += 1;
		while (*pch == ' ')
			pch++;

		hex_len = count - (pch - calcmd);

		CLS_WIFI_DBG(CLS_WIFI_FN_ENTRY_STR);

		pr_warn("%s %d, count [%lu:%lu].\n", __func__, __LINE__, count, hex_len);

		if(hex_len & 1)
		{
			pr_warn("%s %d, hex string len [%lu] is not even, count--.\n", __func__, __LINE__, hex_len);
			hex_len--;
		}

		len = hex_len / 2;
		string = pch;

		pr_warn("%s %d, string [%c %c %c %c %c %c %c %c]\n", __func__, __LINE__,
				string[0], string[1], string[2], string[3], string[4], string[5], string[6], string[7]);

		hexstr2bin(string, msg, len);

		pr_warn("%s %d, msg [%02x %02x %02x %02x %02x %02x %02x %02x]\n", __func__, __LINE__,
				msg[0], msg[1], msg[2], msg[3], msg[4], msg[5], msg[6], msg[7]);

		cls_wifi_send_message(cls_wifi_hw, msg);
#endif
	}
#if defined(CFG_MERAK3000)
	else if (!strcasecmp(cmd_name, "time-sync")) {
		cls_wifi_handle_time_sync(calcmd, radio_index);
	} else if (!strcasecmp(cmd_name, "time-get")) {
		cls_wifi_handle_time_get(calcmd, radio_index);
	}
#endif // CFG_MERAK3000
	return count;
}
#ifdef __KERNEL__
const static struct file_operations wifi_calcmd_fops = {
	.owner = THIS_MODULE,
	.open = wifi_calcmd_open,
	.read = wifi_calcmd_read,
	.write = wifi_calcmd_write,
};
#endif
int show_radio(char *name, char *outbuf, int size, struct cali_config_tag *ptag)
{
	int len = 0;

	if (current_radio < 0)
		return len;
	if (!strcmp(name, "all") || (*name == 0)) {
		len += scnprintf(outbuf + len, size - len,
						"\nradio :%4s\n",
						radio_list[current_radio]);
	}
	return len;
}

int show_bss_param(char *name, char *outbuf, int size, struct cali_config_tag *ptag)
{
	int len = 0;

	if (!strcmp(name, "all") || (*name == 0)) {
		len += scnprintf(outbuf + len, size - len,
						"\nradio:%-10smode:%-11schannel:%-4d mfp %u",
						radio_list[ptag->bss_info.radio],
						mode_list[ptag->bss_info.mac_mode],
						ptag->bss_info.chan_ieee, ptag->bss_info.mfp);

		len += scnprintf(outbuf + len, size - len,
						"\nbw:%-13dphy-mode:%-7slocal-addr:%02x:%02x:%02x:%02x:%02x:%02x",
						ptag->bss_info.bw,
						phymode_list[ptag->bss_info.phy_mode],
						ptag->bss_info.local_addr[0], ptag->bss_info.local_addr[1],
						ptag->bss_info.local_addr[2], ptag->bss_info.local_addr[3],
						ptag->bss_info.local_addr[4], ptag->bss_info.local_addr[5]);

	   len += scnprintf(outbuf + len, size - len,
						"\ntx-ppdu-cnt:%-4dtx-interval:%-4d\n",
						ptag->tx_ppdu_cnt,
						ptag->tx_interval);

	} else if (!strcmp(name, "radio")) {
		len += scnprintf(outbuf + len, size - len,
						"\nradio :%4s\n",
						radio_list[ptag->bss_info.radio]);
	} else if (!strcmp(name, "mode")) {
		len += scnprintf(outbuf + len, size - len,
						"\nmode:%3s\n",
						mode_list[ptag->bss_info.mac_mode]);
	} else if (!strcmp(name, "channel")) {
		len += scnprintf(outbuf + len, size - len,
						"\nchannel:%4d\n",
						ptag->bss_info.chan_ieee);
	} else if (!strcmp(name, "bw")) {
		len += scnprintf(outbuf + len, size - len,
						"\nbw   :%4d\n",
						ptag->bss_info.bw);
	} else if (!strcmp(name, "phy-mode")) {
		len += scnprintf(outbuf + len, size - len,
						"\nphy-mode:%4s\n",
						phymode_list[ptag->bss_info.phy_mode]);
	} else if (!strcmp(name, "local-addr")) {
		len += scnprintf(outbuf + len, size - len,
						"\nlocal-addr:%02x:%02x:%02x:%02x:%02x:%02x\n",
						ptag->bss_info.local_addr[0], ptag->bss_info.local_addr[1],
						ptag->bss_info.local_addr[2], ptag->bss_info.local_addr[3],
						ptag->bss_info.local_addr[4], ptag->bss_info.local_addr[5]);
	}
	return len;
}

int show_ptk_param(char *name, char *outbuf, int size, struct cali_config_tag *ptag)
{
	int len = 0;
	int idx = 0;
	char out_key[128];

	if (!strcmp(name, "all") || (name[0] == 0)) {
		for (; idx < CALI_MU_USER_NUM; idx++) {
			if (!memcmp(ptag->key_ptk[idx].key_addr, zero_mac, 6))
				continue;

			memset(out_key, 0, sizeof(out_key));
			len += scnprintf(outbuf + len, size - len,
					"\nmac-addr:%02x:%02x:%02x:%02x:%02x:%02x\tcipher:%4s-%d\tkey-len:%3d\thw-index:%3d\twep-index:%3d",
					ptag->key_ptk[idx].key_addr[0], ptag->key_ptk[idx].key_addr[1],
					ptag->key_ptk[idx].key_addr[2], ptag->key_ptk[idx].key_addr[3],
					ptag->key_ptk[idx].key_addr[4], ptag->key_ptk[idx].key_addr[5],
					ciper_list[ptag->key_ptk[idx].cipher], ptag->key_ptk[idx].key_len,
					ptag->key_ptk[idx].key_len,
					ptag->key_ptk[idx].key_hw_index,
					ptag->key_ptk[idx].key_index);
			hex_to_string(ptag->key_ptk[idx].key, out_key, ptag->key_ptk[idx].key_len);
			len += scnprintf(outbuf + len, size - len,
							"\nkey:%s\n",
							out_key);
		}
	}
	return len;
}

int comprise_output_buf(char *outbuf, int size, struct cali_user_info_tag *p_user_info_tag,
							struct cali_per_user_mac_phy_params_tag *p_user_mac_phy)
{
	int len = 0;

	if (!p_user_info_tag || !p_user_mac_phy)
		return len;

	len += scnprintf(outbuf + len, size - len,
			"\npeer-addr:%02x:%02x:%02x:%02x:%02x:%02x	 mpdu-type:0x%-12xmpdu-subtype:0x%-9xmpdu-protect:%-11dmpdu-htc:%-15dmpdu-ackpolicy:%d",
			p_user_info_tag->peer_addr[0], p_user_info_tag->peer_addr[1],
			p_user_info_tag->peer_addr[2], p_user_info_tag->peer_addr[3],
			p_user_info_tag->peer_addr[4], p_user_info_tag->peer_addr[5],
			(p_user_info_tag->ppdu_info.frame_ctrl & 0xf) >> 2,
			(p_user_info_tag->ppdu_info.frame_ctrl & 0xf0) >> 4,
			p_user_info_tag->ppdu_info.ppdu_encry,
			p_user_info_tag->ppdu_info.ht_ctrl,
			p_user_info_tag->ppdu_info.ack_policy);

	len += scnprintf(outbuf + len, size - len,
			"\nmpdu-body:%-22dmpdu-ssn:%-15dssn-reset:%-14dmpdu-subframe:%-10dmpdu-body-len:%-10damsdu-num:%-10d",
			p_user_info_tag->ppdu_info.payload_pattern,
			p_user_info_tag->ppdu_info.ssn,
			p_user_info_tag->ppdu_info.ssn_reset,
			p_user_info_tag->ppdu_info.ampdu_num,
			p_user_info_tag->ppdu_info.msdu_len,
			p_user_info_tag->ppdu_info.amsdu_num);

	len += scnprintf(outbuf + len, size - len,
			"\nppdu-protection:%-16dmpdu-duration:%-10dmpdu-seq-ctrl:0x%-8xmpdu-qos-ctrl:0x%-8xppdu-bf:%-16dignore-cca:%d\n",
			p_user_info_tag->ppdu_info.protection,
			p_user_info_tag->ppdu_info.duration,
			p_user_info_tag->ppdu_info.seq_ctrl,
			p_user_info_tag->ppdu_info.qos_ctrl,
			p_user_info_tag->ppdu_info.ppdu_bf,
			p_user_info_tag->ppdu_info.ignore_cca);

	len += scnprintf(outbuf + len, size - len,
			"\ntx-tid:%-25daid:%-20dpaid:%-19dsta-index:%-14dnon-qos-ack:%-12dbw-signal-en:%dsr-disallow:%d\n",
			p_user_info_tag->tx_tid,
			p_user_info_tag->aid,
			p_user_info_tag->paid,
			p_user_info_tag->sta_index,
			p_user_info_tag->ppdu_info.non_qos_ack,
			p_user_info_tag->ppdu_info.bw_signal_en,
			p_user_info_tag->ppdu_info.sr_disallow);

	len += scnprintf(outbuf + len, size - len,
			"\nba-tid:%-25dba-tx-size:%-13dba-rx-size:%-13dba-ack-policy:%-10sba-ssn:%-17dmpdu-fc-flags:0x%x\n",
			p_user_info_tag->ba_info.ba_tid,
			p_user_info_tag->ba_info.ba_tx_size,
			p_user_info_tag->ba_info.ba_rx_size,
			ba_ack_list[p_user_info_tag->ba_info.ba_ack_policy],
			p_user_info_tag->ba_info.ba_ssn,
			(p_user_info_tag->ppdu_info.frame_ctrl & 0xff00) >> 8);

	len += scnprintf(outbuf + len, size - len,
			"\nfec:%-28dpacket-ext:%-13dmin-mpdu-space:%-9sdcm:%-6d",
			p_user_mac_phy->fec,
			p_user_mac_phy->packet_ext,
			mpdu_space_list[p_user_mac_phy->min_mpdu_space],
			p_user_mac_phy->dcm);

#if defined(CFG_MERAK3000)
	len += scnprintf(outbuf + len, size - len,
			"\nsr_drop_pwr:%-20dsr_adj_mcs_delta:%-20d",
			p_user_info_tag->ppdu_info.sr_drop_pwr_en,
			p_user_info_tag->ppdu_info.sr_adj_mcs_delta);
#endif

	len += scnprintf(outbuf + len, size - len,
			"\nmcs:%-28dmcs-legacy:%-13dcbf-host-addr:0x%-8xcbf-report-len:%-9duser-pos:%d",
			p_user_mac_phy->mcs,
			p_user_mac_phy->mcs_legacy,
			p_user_mac_phy->cbf_host_addr,
			p_user_mac_phy->cbf_report_len,
			p_user_mac_phy->user_pos);

	len += scnprintf(outbuf + len, size - len,
			"\nstart-sts:%-22dss-num:%-17dru-index:%-15dru-size:%-16dru-offset:%ddrop-mcs-en:%d\n",
			p_user_mac_phy->start_sts,
			p_user_mac_phy->ss_num,
			p_user_mac_phy->ru_index,
			p_user_mac_phy->ru_size,
			p_user_mac_phy->ru_offset,
			p_user_mac_phy->dropMCSEnTx);

	len += scnprintf(outbuf + len, size - len, "eof-padding:%-20d\n",
			p_user_mac_phy->eof_padding);
	return len;
}

int show_su_param(char *name, char *outbuf, int size, struct cali_config_tag *ptag)
{
	int len = 0;
	struct cali_user_info_tag *p_user_info_tag = &ptag->su_user_info;
	struct cali_per_user_mac_phy_params_tag *p_user_mac_phy = &ptag->su_mac_phy_params;

	if (!p_user_info_tag || !p_user_mac_phy)
		return len;

	if (!strcmp(name, "all") || (name[0] == 0))
		len = comprise_output_buf(outbuf, size, p_user_info_tag, p_user_mac_phy);

	return len;
}

int show_gtk_param(char *name, char *outbuf, int size, struct cali_config_tag *ptag)
{
	int len = 0;
	char out_key[128];
	struct cali_key_info_tag *pkey_info = NULL;

	if (!ptag)
		return len;
	pkey_info = &ptag->bss_info.key_gtk;
	if (!pkey_info)
		return len;

	memset(out_key, 0, sizeof(out_key));

	if (!strcmp(name, "all") || (name[0] == 0)) {
		if (!memcmp(pkey_info->key_addr, zero_mac, 6))
			return len;
		len += scnprintf(outbuf + len, size - len,
				"\nGTK: mac-addr:%02x:%02x:%02x:%02x:%02x:%02x\tcipher:%4s-%-3d\tkey-len:%3d\thw-index:%3d\twep-index:%3d",
				pkey_info->key_addr[0], pkey_info->key_addr[1],
				pkey_info->key_addr[2], pkey_info->key_addr[3],
				pkey_info->key_addr[4], pkey_info->key_addr[5],
				ciper_list[pkey_info->cipher], pkey_info->key_len,
				pkey_info->key_len,
				pkey_info->key_hw_index,
				pkey_info->key_index);
		hex_to_string(pkey_info->key, out_key, pkey_info->key_len);
		len += scnprintf(outbuf + len, size - len,
				"\nkey:%s\n",
				out_key);
	}

	pkey_info = &ptag->bss_info.key_igtk[0];
	len += scnprintf(outbuf + len, size - len,
			"\nIGTK: mac-addr:%02x:%02x:%02x:%02x:%02x:%02x\tcipher:%4s-%-3d\tkey-len:%3d\thw-index:%3d\twep-index:%3d",
			pkey_info->key_addr[0], pkey_info->key_addr[1],
			pkey_info->key_addr[2], pkey_info->key_addr[3],
			pkey_info->key_addr[4], pkey_info->key_addr[5],
			ciper_list[pkey_info->cipher], pkey_info->key_len,
			pkey_info->key_len,
			pkey_info->key_hw_index,
			pkey_info->key_index);
	hex_to_string(pkey_info->key, out_key, pkey_info->key_len);
	len += scnprintf(outbuf + len, size - len,
			"\nkey:%s\n",
			out_key);

	return len;
}

int show_macphy_param(char *name, char *outbuf, int size, struct cali_config_tag *ptag)
{
	int len = 0;
	struct cali_mac_phy_params_tag *pmac_phy = NULL;

	if (!ptag)
		return len;
	pmac_phy = &ptag->mac_phy_params;
	if (!pmac_phy)
		return len;
	if (!strcasecmp(name, "all") || *name == 0) {
		len += scnprintf(outbuf + len, size - len,
				"\nppdu-type:%-22sgi:%-21sstbc:%-27dgid:%-28dbw-ppdu:%d\n",
				ppdu_type_list[pmac_phy->format_mod],
				gi_list[pmac_phy->gi],
				pmac_phy->stbc,
				pmac_phy->group_id,
				pmac_phy->bw_ppdu);

		len += scnprintf(outbuf + len, size - len,
				"\nspatial-reuse:%-18dn-tx-prot:%-14dn-tx:%-27dmidable:%-24ddoppler:%d\n",
				pmac_phy->spatial_reuse,
				pmac_phy->n_tx_prot,
				pmac_phy->n_tx,
				pmac_phy->midable,
				pmac_phy->doppler);

		len += scnprintf(outbuf + len, size - len,
				"\nnum-ext-nss:%-20dbss-color:%-14dantenna-set:%-20dx-power-level:%-18dhe-ltf:%d\n",
				pmac_phy->num_ext_nss,
				pmac_phy->bss_color,
				pmac_phy->antenna_set,
				pmac_phy->tx_power_level,
				pmac_phy->he_ltf);

		len += scnprintf(outbuf + len, size - len,
				"\npreamble-type:%-18dprot-tx-power:0x%-8xprot-preamble-type:%-13dprot-nav-frm-ex:%-16sprot-format-mod:%s\n",
				pmac_phy->preamble_type,
				pmac_phy->prot_tx_power,
				pmac_phy->prot_preamble_type,
				nav_frm_list[pmac_phy->prot_nav_frm_ex],
				ppdu_type_list[pmac_phy->prot_format_mod]);

		len += scnprintf(outbuf + len, size - len,
				"\nprot-bw:%-24dprot-mcs:%-15ddyn-pre-punc-type:%-14dnum-he-ltf:%-21dcenter-26tone-ru:%d\n",
				pmac_phy->prot_bw,
				pmac_phy->prot_mcs,
				pmac_phy->dyn_pre_punc_type,
				pmac_phy->num_he_ltf,
				pmac_phy->center_26tone_ru);

		len += scnprintf(outbuf + len, size - len,
				"\nsigB-comp:%-22dsigB-dcm:%-15dsigB-mcs:%-23dbeamformed:%-21dsigB-ru-alloc-1/2:0x%x/0x%xinact_subchan_bitmap %-8x\n",
				pmac_phy->sigB_comp,
				pmac_phy->sigB_dcm,
				pmac_phy->sigB_mcs,
				pmac_phy->beamformed,
				pmac_phy->inact_subchan_bitmap,
				pmac_phy->sigB_ru_alloc_1,
				pmac_phy->sigB_ru_alloc_2);
		len += scnprintf(outbuf + len, size - len,
				"\nsmart-ant-en %u smart-ant-parma0 0x%08x smart-ant-param1 0x%08x\n",
				pmac_phy->smart_ant_en,
				pmac_phy->smart_ant_param0,
				pmac_phy->smart_ant_param1);

#if defined(CFG_MERAK3000)
		len += scnprintf(outbuf + len, size - len,
				"\nsr_ppdu_min_mcs %u\n",
				pmac_phy->sr_ppdu_min_mcs);
#endif
	}
	return len;
}

int show_mu_param(char *name, char *outbuf, int size, struct cali_config_tag *ptag)
{
	int len = 0;
	int idx = 0;
	struct cali_user_info_tag *p_user_info_tag = NULL;
	struct cali_per_user_mac_phy_params_tag *p_user_mac_phy = NULL;
	int total_len = 0;

	if (!strcmp(name, "all") || (name[0] == 0)) {
		for (; idx < CALI_MU_USER_NUM && size > 0; idx++) {
			p_user_info_tag = &ptag->mu_user_info[idx];
			p_user_mac_phy = &ptag->mu_mac_phy_params[idx];
			if (!memcmp(p_user_info_tag->peer_addr, zero_mac, 6))
				continue;

			len = comprise_output_buf(outbuf, size, p_user_info_tag, p_user_mac_phy);
			outbuf  += len;
			size -= len;
			total_len += len;
		}
	}

	return total_len;
}

int show_sounding_param(char *name, char *outbuf, int size, struct cali_config_tag *ptag)
{
	int len = 0;
	struct cali_sounding_info_tag *p_sounding = &ptag->sounding_info;

	len += scnprintf(outbuf + len, size - len,
			"\ntype:%-11dnb-sta:%-17dfeedback-type:%-10dnc-idx:%-9dng:%d\n",
			p_sounding->type,
			p_sounding->nb_sta,
			p_sounding->feedback_type,
			p_sounding->nc_idx,
			p_sounding->ng);
	return len;
}

int show_muinfo_param(char *name, char *outbuf, int size, struct cali_config_tag *ptag)
{
	int len = 0;
	struct cali_mu_info_tag *p_mu_info = &ptag->mu_info;

	len += scnprintf(outbuf + len, size - len,
			"\nmu-type:%-5d\t\tmu-ack:%-10d\t\tmu-prot:%d\tmu-user:%d\n",
			p_mu_info->mu_type,
			p_mu_info->mu_ack,
			p_mu_info->mu_prot,
			ptag->mu_users);
	return len;
}

int show_wmm_param(char *name, char *outbuf, int size, struct cali_config_tag *ptag)
{
	int len = 0;
	int idx = 0;
	struct cali_wmm_params_tag *p_wmm = NULL;

	if (!strcmp(name, "all") || (name[0] == 0)) {
		len += scnprintf(outbuf + len, size - len,
				"\ncurrent-ac:%-5d\n",
				ptag->current_ac);

		for (; idx < CALI_AC_NUM; idx++) {
			p_wmm = &ptag->wmm[idx];
			len += scnprintf(outbuf + len, size - len,
					"\naifsn:%-5d\t\tcw-min:%-10d\t\tcw-max:%d\n",
					p_wmm->aifsn,
					p_wmm->cw_min,
					p_wmm->cw_max);
		}
	}
	return len;
}

int show_xtal_cal_status_param(char *name, char *outbuf, int size, struct cali_config_tag *ptag)
{
	int len = 0;

	len += scnprintf(outbuf + len, size - len,
					 "\nxtal_cal_status:%d\n",
					 g_xtal_cali_status);
	return len;
}

int show_pppc_param(char *name, char *outbuf, int size, struct cali_config_tag *ptag)
{
	int len = 0;
	int i;

	len += scnprintf(outbuf + len, size - len,
                    "\npppc_enable_status:%d\n",
                    ptag->pppc_info.pppc_en);
	len += scnprintf(outbuf + len, size - len,
                    "\npower_template_num:%d\n",
					ptag->pppc_info.template_num);
	for (i = 0; i < ptag->pppc_info.template_num; i++) {
		len += scnprintf(outbuf + len, size - len,
                    "(%-4d,%-5d)%-5s",ptag->pppc_info.template_power_ppducnt[i][0],
                    ptag->pppc_info.template_power_ppducnt[i][1], "");
		if ((i + 1) % 5 == 0 && (i + 1) != ptag->pppc_info.template_num)
			len += scnprintf(outbuf + len, size - len, "\n");
	}
	len += scnprintf(outbuf + len, size - len, "\n");

	return len;
}

int show_param(enum cali_cmd_type type, char *name, char *outbuf, int size, struct cali_config_tag *ptag)
{
	int ret = 0;

	switch (type) {
	case TYPE_RADIO:
		ret = show_radio(name, outbuf, size, ptag);
		break;
	case TYPE_BSS:
		ret = show_bss_param(name, outbuf, size, ptag);
		break;
	case TYPE_PTK:
		ret = show_ptk_param(name, outbuf, size, ptag);
		break;
	case TYPE_GTK:
		ret = show_gtk_param(name, outbuf, size, ptag);
		break;
	case TYPE_SU:
		ret = show_su_param(name, outbuf, size, ptag);
		break;
	case TYPE_MU:
		ret = show_mu_param(name, outbuf, size, ptag);
		break;
	case TYPE_MACPHY:
		ret = show_macphy_param(name, outbuf, size, ptag);
		break;
	case TYPE_MUINFO:
		ret = show_muinfo_param(name, outbuf, size, ptag);
		break;
	case TYPE_SOUNDING:
		ret = show_sounding_param(name, outbuf, size, ptag);
		break;
	case TYPE_WMM:
		ret = show_wmm_param(name, outbuf, size, ptag);
		break;
	case TYPE_XTAL_CAL:
		ret = show_xtal_cal_status_param(name, outbuf, size, ptag);
		break;
	case TYPE_PPPC:
		ret = show_pppc_param(name, outbuf, size, ptag);
		break;
	default:
		break;
	}
	return ret;
}

int show_status_rx(char *outbuf, int count, struct cal_per_radio_status *pstatus)
{
	int len = 0;
	struct cal_mac_radio_status *pmacsta = NULL;
	struct cal_phy_radio_status *pphysta = NULL;
	uint8_t format_mod = 0x00;
	char format_string[8] = {0};

	if (!pstatus)
		return 0;

	pmacsta = &pstatus->mac_status;
	format_mod = pmacsta->rx_vector.format_mod;
	if (format_mod < 0x2)
		strcpy(format_string, "NON-HT");
	else if (format_mod < 0x4)
		strcpy(format_string, "HT");
	else if (format_mod == 0x4)
		strcpy(format_string, "VHT");
	else if (format_mod < 0x9)
		strcpy(format_string, "HE");
	else if (format_mod < 0xc)
		strcpy(format_string, "EHT");

	len = scnprintf(outbuf + len, count - len,
			"\nMAC status:\n\tLast MPDU:\tType:0x%02x\t\tIdx in AMPDU:%-11uLength:%-9uFormat:%-17sBW:%u",
			pmacsta->rx_type_curr,
			pmacsta->rx_mpdu_idx,
			pmacsta->rx_mpdu_len,
			format_string,
			(format_mod == CALI_FORMAT_MOD_HE_ER) ? 20 :
			pmacsta->rx_vector.ch_bandwidth);

	if (format_mod == CALI_FORMAT_MOD_HE_ER)
		len += scnprintf(outbuf + len, count - len,
				" (%u-tone)",
				pmacsta->rx_vector.ch_bandwidth);

	switch (format_mod) {
	case 0x00:
	case 0x01:
		len += scnprintf(outbuf + len, count - len,
				"\n\t\t\tLength:%-17urate:%-19dDyn bw:%-9uChn bw:%-17u\n",
				pmacsta->rx_vector.non_ht.length,
				pmacsta->rx_vector.non_ht.rate,
				pmacsta->rx_vector.non_ht.dyn_bw,
				pmacsta->rx_vector.non_ht.chn_bw);
		break;
	case 0x02:
	case 0x03:
		len += scnprintf(outbuf + len, count - len,
				"\n\t\t\tgi_type:%-16sstbc:%-19umcs:%-12ufec:%-20slength:%u\n",
				gi_list[pmacsta->rx_vector.ht.gi_type],
				pmacsta->rx_vector.ht.stbc,
				pmacsta->rx_vector.ht.mcs,
				pmacsta->rx_vector.ht.fec ? "LDPC" : "BCC",
				pmacsta->rx_vector.ht.length);
		break;
	case 0x04:
		len += scnprintf(outbuf + len, count - len,
				"\n\t\t\tgi_type:%-16sstbc:%-19dmcs:%-12dfec:%-20slength:%d\n\t\t\tgroup_id:%-15dnss:%d\n",
				gi_list[pmacsta->rx_vector.vht.gi_type],
				pmacsta->rx_vector.vht.stbc,
				pmacsta->rx_vector.vht.mcs,
				pmacsta->rx_vector.vht.fec ? "LDPC" : "BCC",
				pmacsta->rx_vector.vht.length,
				pmacsta->rx_vector.vht.group_id,
				pmacsta->rx_vector.vht.nss + 1);
		break;
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x08:
		len += scnprintf(outbuf + len, count - len,
				"\n\t\t\tgi_type:%-16sstbc:%-19dmcs:%-12dfec:%-20slength:%d\n\t\t\tdcm:%-20dnss:%-20dhe-ltf-type:%u",
				gi_list[pmacsta->rx_vector.he.gi_type],
				pmacsta->rx_vector.he.stbc,
				pmacsta->rx_vector.he.mcs,
				pmacsta->rx_vector.he.fec ? "LDPC" : "BCC",
				pmacsta->rx_vector.he.length,
				pmacsta->rx_vector.he.dcm,
				pmacsta->rx_vector.he.nss + 1,
				pmacsta->rx_vector.he.he_ltf_type);
		break;
	case 0x09:
	case 0x0a:
	case 0x0b:
		len += scnprintf(outbuf + len, count - len,
				"\n\t\t\tgi_type:%-16sstbc:%-19dmcs:%-12dfec:%-20slength:%d\n\t\t\tnss:%-20dhe-ltf-type:%-12usta_idx:%d\n",
				gi_list[pmacsta->rx_vector.eht.gi_type],
				pmacsta->rx_vector.eht.stbc,
				pmacsta->rx_vector.eht.mcs,
				pmacsta->rx_vector.eht.fec ? "LDPC" : "BCC",
				pmacsta->rx_vector.eht.length,
				pmacsta->rx_vector.eht.nss + 1,
				pmacsta->rx_vector.eht.he_ltf_type,
				pmacsta->rx_vector.eht.sta_idx);
		break;
	default:
		break;
	}

	len += scnprintf(outbuf + len, count - len,
			"\n\tLast 32 MPDU:\tFCS err:0x%08x  \tDecrypt err:0x%08x\n",
			pmacsta->rx_fcs_status,
			pmacsta->rx_decr_status);

	len += scnprintf(outbuf + len, count - len,
			"\tLast 1000 MSDU:\tPER:%u\n",
			pmacsta->rx_per);

	pphysta = &pstatus->phy_status;
	len += scnprintf(outbuf + len, count - len,
			"\nPHY status:\n\tLast PPDU:\tFormat:%u\n\tLast 32 PSDU:\tCRC err:0x%08x\n",
			pphysta->rx_fmt_curr,
			pphysta->rx_crc_status);

#if !defined(__KERNEL__) && defined(CFG_MERAK2000)
    uart_puts(outbuf);

    yc_printf("\nLast MPDU:\n");
    cali_hex_dump(pmacsta->rx_mpdu, pmacsta->rx_mpdu_len);
#endif

	return len;
}

int show_stats_tx(char *outbuf, int count, struct cal_tx_stats *pstats)
{
	int len = 0;

	len += scnprintf(outbuf + len, count - len,
			"ctrl_frame:		send %u success %u\n"
			"mgmt_frame:		send %u success %u\n"
			"data_frame:		send %u success %u\n"
			"qos_data_frame:		send %u success %u\n"
			"null_data_frame:		send %u success %u\n"
			"qos_null_data_frame:	send %u success %u\n"
			"ampdu:			send %u success %u\n"
			"mpdu:			send %u success %u\n"
			"retries:			%u\n"
			"sr_frame:		send %u success %u skip %u\n",
			pstats->ctrl_frame, pstats->ctrl_frame_succ,
			pstats->mgmt_frame, pstats->mgmt_frame_succ,
			pstats->data_frame, pstats->data_frame_succ,
			pstats->qos_data_frame, pstats->qos_data_frame_succ,
			pstats->null_data_frame, pstats->null_data_frame_succ,
			pstats->qos_null_data_frame, pstats->qos_null_data_frame_succ,
			pstats->ampdu, pstats->ampdu_succ,
			pstats->mpdu, pstats->mpdu_succ,
			pstats->retries, pstats->sr_frame, pstats->sr_succ, pstats->sr_skip);

#ifdef CFG_MERAK3000
		len += scnprintf(outbuf + len, count - len,
			"tx_mld:                 send %u success %u fail %u, delay %u\n"
			"nstr:			send %u conflict %u\n"
			"eml:			send %u conflict %u data_tx %u data_fail %u init_frm_tx %u init_frm_fail %u\n"
			"str:			send %u\n",
			pstats->tx_mld_push, pstats->tx_mld_succ, pstats->tx_mld_fail,
			pstats->tx_mld_push_delay,
			pstats->tx_mld_nstr, pstats->tx_mld_nstr_conflict,
			pstats->tx_mld_eml, pstats->tx_mld_eml_conflict,
			pstats->tx_mld_eml_data_frm_done, pstats->tx_mld_eml_data_frm_fail,
			pstats->tx_mld_eml_init_frm_done, pstats->tx_mld_eml_init_frm_fail,
			pstats->tx_mld_str);
#endif // CFG_MERAK3000
	return len;
}

int show_stats_rx(char *outbuf, int count, struct cal_per_radio_stats *pstats)
{
	int len = 0;
	int i = 0;
	int rx_type_mgmt = 0;
	int rx_type_ctrl = 0;
	int rx_type_data = 0;
	struct cal_mac_radio_stats *pmacstats = NULL;
	struct cal_phy_radio_stats *pphystats = NULL;
	struct cal_mac_sta_stats   *pstastats = NULL;
	struct cal_mac_sta_stats   *pbasesta = NULL;
	int sinr_agv[2][2];

	if (!pstats)
		return 0;

	pmacstats = &pstats->mac_stats;
	pphystats = &pstats->phy_stats;
	pbasesta = &pstats->sta_stats[0];

	for (; i < 16; i++) {
		if (pmacstats->rx_type_mgmt[i] != 0)
			rx_type_mgmt += pmacstats->rx_type_mgmt[i];
		if (pmacstats->rx_type_ctrl[i] != 0)
			rx_type_ctrl += pmacstats->rx_type_ctrl[i];
		if (pmacstats->rx_type_data[i] != 0)
			rx_type_data += pmacstats->rx_type_data[i];
	}

	len = scnprintf(outbuf + len, count - len,
			"\nMAC statistics:\n\tA-MPDU:%-17uMPDU:%-19uMSDU:%-10u",
			pmacstats->rx_ampdu,
			pmacstats->rx_mpdu,
			pmacstats->rx_msdu);

	len += scnprintf(outbuf + len, count - len,
			"\n\tType:\t\t\tMGMT:%-19uCTRL:%-19uDATA:%-10u",
			rx_type_mgmt,
			rx_type_ctrl,
			rx_type_data);

	len += scnprintf(outbuf + len, count - len,
			"\n\tFCS err:\t\tMGMT:%-19uCTRL:%-19uDATA:%-10u",
			pmacstats->rx_fcs_err_mgmt,
			pmacstats->rx_fcs_err_ctrl,
			pmacstats->rx_fcs_err_data);

	len += scnprintf(outbuf + len, count - len,
			"\n\tDecrypt:\t\tPASS:%-19uFAIL:%-10u",
			pmacstats->rx_decr_pass,
			pmacstats->rx_decr_fail);

	len += scnprintf(outbuf + len, count - len,
			"\n\tOth err:\t\tUNDF:%-19uMFP:%u\n",
			pmacstats->rx_undef_err,
			pmacstats->rx_mfp_cnt);

#if defined(CFG_MERAK3000)
	len += scnprintf(outbuf + len, count - len,
			"\tMLD_PPDU:%-15uMLD_MPDU:%u\n",
			pmacstats->rx_mld_ppdu, pmacstats->rx_mld_mpdu);
#endif

	len += scnprintf(outbuf + len, count - len,
			"\nPHY statistics:\n\tMPDU format:\t\tNon-HT:%-17uNon-HT OFDM:%-12uVHT:%-12uHT:%-10u",
			pphystats->rx_fmt_non_ht,
			pphystats->rx_fmt_non_ht_dup_ofdm,
			pphystats->rx_fmt_vht,
			pphystats->rx_fmt_ht_mf);

	len += scnprintf(outbuf + len, count - len,
			"\n\tHE SU:%-18uHE MU:%-18uHE ER SU:%-15uHE TB:%-10u",
			pphystats->rx_fmt_he_su,
			pphystats->rx_fmt_he_mu,
			pphystats->rx_fmt_he_er_su,
			pphystats->rx_fmt_he_tb);

	len += scnprintf(outbuf + len, count - len,
			"\n\tEHT MU(SU):%-13uEHT TB:%-17uEHT ER:%-15u",
			pphystats->rx_fmt_eht_mu_su,
			pphystats->rx_fmt_eht_tb,
			pphystats->rx_fmt_eht_er);

	len += scnprintf(outbuf + len, count - len,
			"\n\tA-MPDU CRC check:\tPass:%-19uFail:%-10u\n",
			pphystats->rx_crc_pass,
			pphystats->rx_crc_fail);

	len += scnprintf(outbuf + len, count - len,
			"\n\tCSI:%u\n",
			pphystats->rx_csi_cnt);

	if (pphystats->sinr_cnt == 0) {
		sinr_agv[0][0] = 0;
		sinr_agv[0][1] = 0;
		sinr_agv[1][0] = 0;
		sinr_agv[1][1] = 0;
	} else {
		sinr_agv[0][0] = pphystats->sinr_total[0][0] / pphystats->sinr_cnt;
		sinr_agv[0][1] = pphystats->sinr_total[0][1] / pphystats->sinr_cnt;
		sinr_agv[1][0] = pphystats->sinr_total[1][0] / pphystats->sinr_cnt;
		sinr_agv[1][1] = pphystats->sinr_total[1][1] / pphystats->sinr_cnt;
	}

	len += scnprintf(outbuf + len, count - len,
			"last PPDU SINR %d    %d  %d    %d\n""Agv cnt:%d SINR %d    %d    %d    %d\n",
			pphystats->sinr[0] & 0xFF,(pphystats->sinr[0] >> 8) & 0xFF,
			pphystats->sinr[1] & 0xFF,(pphystats->sinr[1] >> 8) & 0xFF,
			pphystats->sinr_cnt, sinr_agv[0][0],sinr_agv[0][1],sinr_agv[1][0],sinr_agv[1][1]);

	len += scnprintf(outbuf + len, count - len, "\nSTA statistics:\n");
	for (i = 0; i < CALI_MU_USER_NUM; i++) {
		pstastats = pbasesta + i;
		if (memcmp(pstastats->mac_addr, zero_mac, 6)) {
			len += scnprintf(outbuf + len, count - len,
					"\t%d <%02x:%02x:%02x:%02x:%02x:%02x>:\tA-MPDU:%-17uMPDU:%-19uMSDU:%-10u",
					i, pstastats->mac_addr[0], pstastats->mac_addr[1],
					pstastats->mac_addr[2], pstastats->mac_addr[3],
					pstastats->mac_addr[4], pstastats->mac_addr[5],
					pstastats->rx_ampdu, pstastats->rx_mpdu,
					pstastats->rx_msdu);
#if defined(CFG_MERAK3000)
					// Chip-test Team want's rx_mld_ppdu in the same line.
					// And no space between name and ':'
					len += scnprintf(outbuf + len, count - len,
							"  MLD_PPDU:%-8u    MLD_MPDU:%u",
							pstastats->rx_mld_ppdu,
							pstastats->rx_mld_mpdu);
#endif
					len += scnprintf(outbuf + len, count - len, "\n");
		}
	}
	return len;
}

int show_stats_rx_mpdu_num(char *outbuf, int count, struct cal_per_radio_stats *pstats)
{
	struct cal_mac_radio_stats *pmacstats = NULL;
	int len = 0;

	if (!pstats)
		return 0;

	pmacstats = &pstats->mac_stats;

	len += scnprintf(outbuf + len, count - len, "PPDU_Count: %u\n", pmacstats->rx_mpdu);

	return len;
}

int show_stats_rssi(char *outbuf, int count, int32_t status, int8_t rssi)
{
	int len = 0;

	if (status == CO_OK)
		len += scnprintf(outbuf + len, count - len, "%hhd\n", rssi);
	else
		len += scnprintf(outbuf + len, count - len, "not available\n");

	return len;
}

int show_temperature(char *outbuf, int count, int32_t status, int8_t temp)
{
	int len = 0;

	if (status == CO_OK)
		len += scnprintf(outbuf + len, count - len, "%hhd\n", temp);
	else
		len += scnprintf(outbuf + len, count - len, "not available\n");

	return len;
}

int show_csi_result(char *outbuf, int count, int32_t status, struct cls_csi_report *pcsi)
{
	int len = 0;

	if (!pcsi || status != CO_OK) {
		len += scnprintf(outbuf + len, count - len, "not available\n");

		return len;
	}

	len += scnprintf(outbuf + len, count - len,
		"timestamp: %-10u %-10u MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n",
		pcsi->info_aligned.info.timestamp_hi, pcsi->info_aligned.info.timestamp_lo,
		pcsi->info_aligned.info.sta_mac_addr[0], pcsi->info_aligned.info.sta_mac_addr[1],
		pcsi->info_aligned.info.sta_mac_addr[2], pcsi->info_aligned.info.sta_mac_addr[3],
		pcsi->info_aligned.info.sta_mac_addr[4], pcsi->info_aligned.info.sta_mac_addr[5]);

	len += scnprintf(outbuf + len, count - len,
		"rx nss: %u tx nss: %u rssi: %3d chan: %-3u bw: %u subcarrier num: %-3u\n",
		pcsi->info_aligned.info.rx_nss, pcsi->info_aligned.info.tx_nss,
		pcsi->info_aligned.info.rssi,
		pcsi->info_aligned.info.channel, pcsi->info_aligned.info.bw,
		pcsi->info_aligned.info.subcarrier_num);

	len += scnprintf(outbuf + len, count - len,
		"agc: %3d %3d %3d %3d %3d %3d %3d %3d\n",
		pcsi->info_aligned.info.agc[0], pcsi->info_aligned.info.agc[1],
		pcsi->info_aligned.info.agc[2], pcsi->info_aligned.info.agc[3],
		pcsi->info_aligned.info.agc[4], pcsi->info_aligned.info.agc[5],
		pcsi->info_aligned.info.agc[6], pcsi->info_aligned.info.agc[7]);

	len += scnprintf(outbuf + len, count - len,
		"ppdu id: %-4u"
#if defined(CFG_MERAK3000)
		" csi_format: %-4u"
#endif
		" csi_length: %-4u\n",
		pcsi->hdr.ppdu_id
#if defined(CFG_MERAK3000)
		, pcsi->hdr.csi_format
#endif
		, pcsi->hdr.csi_length);

#if !defined(__KERNEL__) && defined(CFG_MERAK2000)
	uint32_t csi_bank_len;
	uint8_t *rx_csi_iq = (uint8_t *)pcsi;

	uart_puts(outbuf);
	yc_printf("\nLast CSI:\n");
	csi_bank_len = pcsi->hdr.csi_length * 4;
	rx_csi_iq += (sizeof(union cls_csi_report_info_aligned) + sizeof(struct cls_csi_info_head));
	cali_hex_dump(rx_csi_iq, csi_bank_len);
#endif

	return len;
}
#ifdef __KERNEL__
int dump_reg_result(uint32_t reg_addr, uint32_t reg_len, char *outbuf, int buf_len)
{
	int len = 0;
	int i = 0;
	void __iomem *base_addr;
	uint32_t reg_value;
	struct wpu_regs_map *reg_map = NULL;

	reg_map = cls_wifi_cal_get_wpu_regs_map(current_radio, reg_addr);
	if (!reg_map)
		return len;
	base_addr = reg_map->io_base_addr + reg_addr - reg_map->reg_base_addr;
	for (; i < reg_len; i++) {
		if ((base_addr + i * 4) > (reg_map->io_base_addr + reg_map->reg_size))
			break;
		reg_value = CLS_REG_RAW_READ32(base_addr, i * 4);
		len += scnprintf(outbuf + len, buf_len - len,
						"\n[0x%lx] = 0x%08x",
						(unsigned long)(base_addr + i * 4),
						reg_value);
	}
	return len;
}

static int calcmd_show_open(struct inode *inode, struct file *filp)
{
	filp->private_data = inode->i_private;
	return 0;
}

static ssize_t calcmd_show_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
	if (out_buf_len == 0)
		out_buf_len = strlen(out_buf);
	return simple_read_from_buffer(buf, count, ppos, out_buf, out_buf_len);
}

const static struct file_operations calcmd_show_fops = {
	.owner = THIS_MODULE,
	.open = calcmd_show_open,
	.read = calcmd_show_read,
};

int cali_debugfs_init(void)
{
	if (wifi_debug_dir)
		return 0;

	wifi_debug_dir = debugfs_create_dir("calidir", NULL);
	if (IS_ERR_OR_NULL(wifi_debug_dir))
		pr_warn("-----create wifi_debug_dir failed-----\n");

	debugfs_create_file("calicmd", 0x666, wifi_debug_dir, NULL, &wifi_calcmd_fops);
	debugfs_create_file("show", 0x444, wifi_debug_dir, NULL, &calcmd_show_fops);
	return 0;
}

void  cali_debugfs_deinit(void)
{
	if (!wifi_debug_dir)
		return;

	debugfs_remove_recursive(wifi_debug_dir);
	wifi_debug_dir = NULL;
}
#endif
