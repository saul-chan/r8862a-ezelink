/*
 * Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */


#ifndef _CLSAPI_NL80211_ADAPTER_H
#define _CLSAPI_NL80211_ADAPTER_H

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <linux/nl80211.h>
#include <cls/cls_nl80211_vendor.h>
#include "wifi_common.h"
#include "clsapi_wifi.h"


#ifndef NL_CAPABILITY_VERSION_3_5_0
#define nla_nest_start(msg, attrtype) \
	nla_nest_start(msg, NLA_F_NESTED | (attrtype))
#endif

#define SURVEY_ENTRY_BUFSIZE	(24 * 1024)

enum cls_vbss_roam_results {
	CLS_VBSS_ROAM_RESULT_FAIL,
	CLS_VBSS_ROAM_RESULT_SUCCESS,
};

typedef uint64_t u64_l;
typedef uint32_t u32_l;
typedef uint16_t u16_l;

/*MIB MACRO*/
#define	BASIC_MIB_SET_START     0
#define	BASIC_MIB_SET_NUM       18
#define	EDCA_MIB_SET_START      20
#define	EDCA_MIB_SET_NUM        88
#define	TRIGGER_MIB_SET_START   108
#define	TRIGGER_MIB_SET_NUM     11
#define	AMPDU_MIB_SET_START     119
#define	AMPDU_MIB_SET_NUM       16
#define	BW_TXRX_MIB_SET_START   144
#define	BW_TXRX_MIB_SET_NUM     19
#define	BF_MIB_SET_START        172
#define	BF_MIB_SET_NUM          4

#define EDCA_MIB_SET_NUM_ITEM   11


#define MIB_ITEM_NB  (BASIC_MIB_SET_NUM + EDCA_MIB_SET_NUM + TRIGGER_MIB_SET_NUM \
                        + AMPDU_MIB_SET_NUM + BW_TXRX_MIB_SET_NUM + BF_MIB_SET_NUM)

struct basic_mib_info {
	u32_l info[BASIC_MIB_SET_NUM];
};

struct edca_mib_info {
	u32_l info[EDCA_MIB_SET_NUM];
};

struct trigger_based_mib_info {
	u32_l info[TRIGGER_MIB_SET_NUM];
};

struct ampdu_mib_info {
	u32_l info[AMPDU_MIB_SET_NUM];
};

struct bw_mib_info {
	u32_l info[BW_TXRX_MIB_SET_NUM];
};

struct bfr_mib_info {
	u32_l info[BF_MIB_SET_NUM];
};

struct mib_infor {
	struct basic_mib_info basic;
	struct edca_mib_info edca;
	struct trigger_based_mib_info tb;
	struct ampdu_mib_info ampdu;
	struct bw_mib_info bw;
	struct bfr_mib_info bfr;
};

enum MIB_UPDATE_FLAG
{
	MIB_UPDATE_BASIC = 1,
	MIB_UPDATE_EDCA = 2,
	MIB_UPDATE_TB = 4,
	MIB_UPDATE_AMPDU = 8,
	MIB_UPDATE_BW = 0x10,
	MIB_UPDATE_BFR = 0x20,
	MIB_UPDATE_STATIC_REG = 0x40,
	MIB_MAX_ITEM = 7,

	MIB_DUMP_VALUE = 0x400,

	MIB_UPDATE_ALL = 0x7F
};

struct clsapi_wifi_get_mib_req {
    u16_l get_mib_flag;  ///enum MIB_UPDATE_FLAG
    u16_l not_ignore_cfm;
};

struct clsapi_wifi_get_dbgcnt_req {
	u32_l reset;
};

struct clsapi_wifi_get_dbgcnt_cfm {
	u32_l success;
};

#define DBG_MACREG_ITEM   12
struct clsapi_wifi_mac_reg_info{
    u32_l mpif_underflow_dbg;
    u32_l mpif_tb_rx_err_dbg;
    u32_l mac_rx_hang_ctrl;
    u32_l mac_rx_hang_dbg0;
    u32_l mac_rx_hang_dbg1;
    u32_l mpif_underflow_dbg2;
    u32_l mpif_underflow_dbg3;
    u32_l rx_vector1_dbg[5];
};

struct clsapi_wifi_get_mib_cfm {
    u32_l mib_flag;   ///enum MIB_UPDATE_FLAG
    u64_l cur_wpu_time;
    struct mib_infor mib;
    struct clsapi_wifi_mac_reg_info mac_reg;
};

enum nl80211_hwmode {
	NL80211_80211_A = 0,
	NL80211_80211_B,
	NL80211_80211_G,
	NL80211_80211_N,
	NL80211_80211_AC,
	NL80211_80211_AD,
	NL80211_80211_AX,

	/* keep last */
	NL80211_80211_COUNT
};

#define NL80211_80211_A       (1 << NL80211_80211_A)
#define NL80211_80211_B       (1 << NL80211_80211_B)
#define NL80211_80211_G       (1 << NL80211_80211_G)
#define NL80211_80211_N       (1 << NL80211_80211_N)
#define NL80211_80211_AC      (1 << NL80211_80211_AC)
#define NL80211_80211_AD      (1 << NL80211_80211_AD)
#define NL80211_80211_AX      (1 << NL80211_80211_AX)

enum nl80211_htmode {
	NL80211_HTMODE_HT20 = 0,
	NL80211_HTMODE_HT40,
	NL80211_HTMODE_VHT20,
	NL80211_HTMODE_VHT40,
	NL80211_HTMODE_VHT80,
	NL80211_HTMODE_VHT80_80,
	NL80211_HTMODE_VHT160,
	NL80211_HTMODE_NOHT,
	NL80211_HTMODE_HE20,
	NL80211_HTMODE_HE40,
	NL80211_HTMODE_HE80,
	NL80211_HTMODE_HE80_80,
	NL80211_HTMODE_HE160,
	NL80211_HTMODE_NOHT_G,
	NL80211_HTMODE_NOHT_A,

	/* keep last */
	NL80211_HTMODE_COUNT
};

#define NL80211_HTMODE_HT20       (1 << NL80211_HTMODE_HT20)
#define NL80211_HTMODE_HT40       (1 << NL80211_HTMODE_HT40)
#define NL80211_HTMODE_VHT20      (1 << NL80211_HTMODE_VHT20)
#define NL80211_HTMODE_VHT40      (1 << NL80211_HTMODE_VHT40)
#define NL80211_HTMODE_VHT80      (1 << NL80211_HTMODE_VHT80)
#define NL80211_HTMODE_VHT80_80   (1 << NL80211_HTMODE_VHT80_80)
#define NL80211_HTMODE_VHT160     (1 << NL80211_HTMODE_VHT160)
#define NL80211_HTMODE_NOHT       (1 << NL80211_HTMODE_NOHT)
#define NL80211_HTMODE_HE20       (1 << NL80211_HTMODE_HE20)
#define NL80211_HTMODE_HE40       (1 << NL80211_HTMODE_HE40)
#define NL80211_HTMODE_HE80       (1 << NL80211_HTMODE_HE80)
#define NL80211_HTMODE_HE80_80    (1 << NL80211_HTMODE_HE80_80)
#define NL80211_HTMODE_HE160      (1 << NL80211_HTMODE_HE160)
#define NL80211_HTMODE_NOHT_G     (1 << NL80211_HTMODE_NOHT_G)
#define NL80211_HTMODE_NOHT_A     (1 << NL80211_HTMODE_NOHT_A)

struct nl80211_modes {
	bool ok;
	uint32_t hw;
	uint32_t ht;

	uint8_t bands;

	uint16_t nl_ht;
	uint32_t nl_vht;
	uint16_t he_phy_cap[6];
};

#define NL80211_BAND_24       (1 << 0)
#define NL80211_BAND_5        (1 << 1)
#define NL80211_BAND_6        (1 << 2)
#define NL80211_BAND_60       (1 << 3)

struct nl80211_array_buf {
	void *buf;
	int count;
};

struct nl80211_state {
	struct nl_sock *nl_sock;
	struct nl_cache *nl_cache;
	struct genl_family *nl80211;
	struct genl_family *nlctrl;
};

struct nl80211_event_conveyor {
	uint32_t wait[(NL80211_CMD_MAX / 32) + !!(NL80211_CMD_MAX % 32)];
	int recv;
};

struct nl80211_group_conveyor {
	const char *name;
	int id;
};

struct nl80211_msg_conveyor {
	struct nl_msg *msg;
	struct nl_cb *cb;
};

/*
 * struct to carry info of parse attribute and return
 */
struct attr_ret_arg {
	uint16_t	attr_id;	/* attribute id */
	void		*arg;		/* buffer to carry the attribute */
	uint16_t	arg_len;	/* buffer length */
	int			ret;		/* parse result: 0-OK; !0-Error */
};

/*
 * struct to carry one nl80211 attribute
 */
struct nl80211_attr_id_value {
	uint16_t	id;	/* nl80211 attribute id */
	void		*value;		/* buffer to carry the attribute value */
	uint16_t	val_len;	/* buffer length */
};

struct nl80211_attr_tbl {
	uint16_t	n_attr;		// number of attribute
	struct nl80211_attr_id_value *attrs;
};

/*
 * struct to carry function callback args and return value
 */
struct cb_arg_ret {
	int			ret;		/* parse result: 0-OK; !0-Error */
	uint8_t		*arg1;		/* args expected to be filled in cb function */
	uint8_t		*arg2;		/* args expected to be filled in cb function */
	uint8_t		*arg3;		/* args expected to be filled in cb function */
	uint8_t		*arg4;		/* args expected to be filled in cb function */
};

int nl80211_get_assoc_list(const char *ifname, const uint8_t (*sta_macs)[ETH_ALEN], int *list_len);
int nl80211_get_assoc_info(const char *ifname, const uint8_t sta_mac[ETH_ALEN], struct sta_info *sinfo);
int nl80211_get_attr_value(const char *ifname, const enum nl80211_commands cmd,
	const struct nl80211_attr_tbl *put_attrs, const enum nl80211_attrs attr_id,
	void *buf, const uint16_t buf_len);
int nl80211_set_attr_value(const char *ifname, const enum nl80211_commands cmd,
	const struct nl80211_attr_tbl *put_attrs);
int nl80211_get_cls_attr_value(const char *ifname, const enum cls_nl80211_vendor_subcmds cls_subcmd,
	const struct nl80211_attr_tbl *put_attrs, const enum cls_nl80211_vendor_attr cls_attr_id,
	void *buf, const uint16_t buf_len);
int nl80211_set_cls_attr_value(const char *ifname, const enum cls_nl80211_vendor_subcmds cls_subcmd,
	const struct nl80211_attr_tbl *put_attrs);

/* Get supported channel list per band */
int nl80211_get_chan_list(const char *ifname, const enum clsapi_wifi_band band, const enum clsapi_wifi_bw bw,
		uint8_t *channels, uint8_t *channels_len);

/* Trigger scan on interface on target channel list with flags
 * \param freqs [in] scan on specified freq in MHz. If it's NULL or len=0, scan on all available channels.
 * \param flags [i] scan flags. lower 32bits are mapping to enum nl80211_scan_flags, higher 32bits are
 *                  mapping to CLS definitions.
 * Return:
 *   0: OK
 *   !0: Errors
 */
int nl80211_trigger_scan(const char *ifname, const uint32_t *freqs, const uint8_t freqs_len, uint64_t flags);

int nl80211_get_scan_result(const char *ifname);

int nl80211_get_csi_sta_list(const char *ifname, const uint8_t (*sta_macs)[ETH_ALEN], int *list_len);

int nl80211_get_dynamic_cca_ed_thr(const char *ifname, struct clsapi_cca_ed_config_req *ed_req);

int nl80211_get_txpower(const char *ifname, int8_t *buf);

int nl80211_get_max_txpower(const char *ifname, int ch_cur, int *dbm_max);

int nl80211_get_hwmodelist(const char *ifname, int *buf);

int nl80211_get_htmodelist(const char *ifname, int *buf);

int nl80211_get_survey(const char *ifname, char *buf, int *len);
#endif /* _CLSAPI_NL80211_ADAPTER_H */

