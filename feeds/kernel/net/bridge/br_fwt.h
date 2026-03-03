/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _BR_FWT_H
#define _BR_FWT_H

#include <linux/if_bridge.h>

/* cls fwt sub_port*/
/* |-BIT31:28-|-BIT27:24-|---BIT23---|--BIT22:16--|----BIT15----|--BIT14:12--|--BIT11:0--|
 * | is_4addr | node type| vif valid |   vif idx  | node valid  |  radio idx |  node idx |
 *
 * Note:
 * (1) sub_port: a station will be Unique Pointing by radio_idx and node_idx
 * (2) is_4addr and node_type is used to check if support 4addr
 */
/* get info from sub_port without check valid*/
#define CLS_IEEE80211_NODE_IDX_FROM_SUBPORT(x)		((x) & 0xFFF)
#define CLS_IEEE80211_RADIO_IDX_FROM_SUBPORT(x)		((x >> 12) & 0x7)
#define CLS_IEEE80211_VIF_IDX_FROM_SUBPORT(x)		(((x) >> 16) & 0x7F)
#define CLS_IEEE80211_NODE_TYPE_FROM_SUBPORT(x)		(((x) >> 24) & 0xF)
#define CLS_IEEE80211_4ADDR_FROM_SUBPORT(x)		(((x) >> 28) & 0xF)

/* build sub_port */
#define CLS_IEEE80211_NODE_TO_IDXS(radio_idx, sta_idx)	\
		((((radio_idx) & 0x7) << 12) | ((sta_idx) & 0xFFF))

#define CLS_IEEE80211_NODE_IDX_MAP(x)			((x) | 0x8000)
#define CLS_IEEE80211_VIF_IDX_MAP(x)			((x) | 0x80)

#define CLS_IEEE80211_NODE_TO_SUBPORT(node_idx, vif_idx, type, is_4addr)	\
		(((node_idx) & 0xFFFF) | (((vif_idx) & 0xFF) << 16) | \
		(((type) & 0xF) << 24) | (((is_4addr) & 0xF) << 28))

//check if device are connected with the same station in ap mode
#define CLS_IEEE80211_TYPE_UNMAP(x)			((x) & 0xFFFFFF)

/* valid check */
#define CLS_IEEE80211_NODE_IDX_VALID(x)			(!!((x) & 0x8000))
#define CLS_IEEE80211_VIF_IDX_VALID(x)			(!!((x) & 0x800000))
#define CLS_IEEE80211_IS_4ADDR_SUPPORT(x)		(!!((x) & 0xF0000000))

enum {
	CLS_FWT_ENTRY_TYPE_ETH = 0,
	CLS_FWT_ENTRY_TYPE_WIRELESS_INVALID = CLS_FWT_ENTRY_TYPE_ETH,// invalid in wireless
	CLS_FWT_ENTRY_TYPE_STA_IN_AP_MODE,
	CLS_FWT_ENTRY_TYPE_WDS_IN_AP_MODE,
	CLS_FWT_ENTRY_TYPE_AP_IN_STA_MODE,
	CLS_FWT_ENTRY_TYPE_WDS_IN_STA_MODE
};

void br_fdb_update_const_hook(struct net_device *dev,
			 const unsigned char *addr, u16 vid, u32 sub_port);
unsigned char br_fdb_delete_by_subport_hook(struct net_device *dev, u32 sub_port);

typedef int (*cls_fwt_add_entry_cbk)(const u8 *mac_be, u16 vid, struct net_device *dev,
				     u32 sub_port, const struct br_ip *group);
typedef int (*cls_fwt_delete_entry_cbk)(const u8 *mac_be, u16 vid);
typedef unsigned long (*cls_fwt_get_ageing_cbk)(const u8 *mac_be, const u16 vid);
typedef void (*cls_fwt_reset_ageing_cbk)(u16 index);
typedef int (*cls_fwt_should_update_cbk)(const u8 *mac_be, const u16 vid,
				       struct net_device *new_dev, u32 sub_port);
typedef  struct net_device *(*cls_fwt_get_entry_info_cbk)(const u8 *mac_be, u16 vlan_id,
							  u16 *index, u32 *sub_port);

extern int cls_fwt_g_enable;
extern int cls_fwt_dbg_enable;
extern int cls_fwt_switch_enable;
extern int cls_fwt_4addr_by_vif;
extern cls_fwt_add_entry_cbk g_add_fwt_entry_hook;
extern cls_fwt_get_entry_info_cbk g_get_fwt_entry_info_hook;
extern cls_fwt_reset_ageing_cbk g_reset_fwt_ageing_hook;
extern cls_fwt_should_update_cbk g_check_fwt_should_update_hook;

/* register CBK for copying bridge database to fwt interface */
void br_register_hooks_cbk_t(cls_fwt_add_entry_cbk add_func, cls_fwt_delete_entry_cbk delete_func,
			     cls_fwt_get_ageing_cbk get_stamp_func,
			     cls_fwt_reset_ageing_cbk reset_stamp_func,
			     cls_fwt_should_update_cbk should_update_func,
			     cls_fwt_get_entry_info_cbk get_info_func);

enum cls_fwt_port_id {
	CLS_FWT_PORT_UNSPEC	= 0,
	CLS_FWT_PORT_ETH_0	= 1,
	CLS_FWT_PORT_ETH_1	= 2,
	CLS_FWT_PORT_ETH_2	= 3,
	CLS_FWT_PORT_ETH_3	= 4,
	CLS_FWT_PORT_ETH_4	= 5,
	CLS_FWT_PORT_WLAN0	= 6,
	CLS_FWT_PORT_WLAN1	= 7,
	CLS_FWT_PORT_PCIE	= 8,
	CLS_FWT_PORT_LHOST	= 9,
	CLS_FWT_PORT_MCAST	= 10,
	CLS_FWT_PORT_BCAST	= 11,

	CLS_FWT_PORT_DROP	= 31,
};

enum {
	CLS_FWT_UPDATE_ACCEPT		= 0,
	CLS_FWT_UPDATE_BYPASS,
	CLS_FWT_UPDATE_AFTER_DEL_TABLE,
};

enum {
	CLS_FWT_SWITCH_MODE_MIN = 0,
	CLS_FWT_SWITCH_DISABLE = CLS_FWT_SWITCH_MODE_MIN,
	CLS_FWT_SWITCH_MODE_DIRECT,
	CLS_FWT_SWITCH_MODE_LEARNING_PKT,
	CLS_FWT_SWITCH_MODE_MAX = CLS_FWT_SWITCH_MODE_LEARNING_PKT,
};

static inline bool if_port_is_npe_eth_port(unsigned char if_port)
{
	return (if_port >= CLS_FWT_PORT_ETH_0 && if_port <= CLS_FWT_PORT_ETH_4);
}

/* cls fwt end */
#endif
