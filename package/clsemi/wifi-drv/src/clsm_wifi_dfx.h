#ifndef __CLSM_WIFI_DFX_H__
#define __CLSM_WIFI_DFX_H__

#include <linux/workqueue.h>
#include <linux/if_ether.h>
#include "cls_wifi_fw_trace.h"
#include <net/netlink.h>
#include <net/cfg80211.h>

#include "lmac_mac.h"
#include "lmac_msg.h"

#define CLSM_WIFI_DFX_CFG           1

enum CLS_WIFI_STATS_TYPE {
	CLSM_WIFI_STATS_TYPE_RADIO = 1,
	CLSM_WIFI_STATS_TYPE_VAP = 2,
	CLSM_WIFI_STATS_TYPE_PEER_STA = 4,

	CLSM_WIFI_STATS_RESET_REPLY = 0x08,
	__CLS_WIFI_STATS_TYPE_MAX,
};

#if CLSM_WIFI_DFX_CFG
typedef struct _clsm_radio_extstats
{
	uint32_t radio_index;
	/**
	 * The timestamp in seconds since system boot up
	 */
	uint32_t	tstamp;
	/**
	 * Associated Station count or if Station is associated
	 */
	uint32_t	assoc_sta_num;
	/**
	 * Current active channel
	 */
	uint32_t	channel;
	/**
	 * Attenuation
	 */
	uint32_t	atten;
	/**
	 * Total CCA
	 */
	uint32_t	cca_total;
	/**
	 * Transmit CCA
	 */
	uint32_t	cca_tx;
	/**
	 * Receive CCA
	 */
	uint32_t	cca_rx;
	/**
	 * CCA interference
	 */
	uint32_t	cca_int;
	/**
	 * CCA Idle
	 */
	uint32_t	cca_idle;
	/**
	 * Received packets counter
	 */
	uint32_t	rx_pkts;
	/**
	 * Receive gain in dBm
	 */
	uint32_t	rx_gain;
	/**
	 * Received packet counter with frame check error
	 */
	uint32_t	rx_cnt_crc;
	/**
	 * Received noise level in dBm
	 */
	float		rx_noise;
	/**
	 * Transmitted packets counter
	 */
	uint32_t	tx_pkts;
	/**
	 * Deferred packet counter in transmission
	 */
	uint32_t	tx_defers;
	/**
	 * Time-out counter for transimitted packets
	 */
	uint32_t	tx_touts;
	/**
	 * Retried packets counter in transmission
	 */
	uint32_t	tx_retries;
	/**
	 * Counter of short preamble errors
	 */
	uint32_t	cnt_sp_fail;
	/**
	 * Counter of long preamble errors
	 */
	uint32_t	cnt_lp_fail;
	/**
	 * MCS index for last received packet
	 */
	uint32_t	last_rx_mcs;
	/**
	 * MCS index for last transimtted packet
	 */
	uint32_t	last_tx_mcs;
	/**
	 * Received signal strength indicator in dBm
	 */
	int32_t		last_rssi;
} _clsm_radio_extstats;

typedef struct _clsm_vap_extstats
{
	/**
	 * The number of transmitted unicast packets to the vap.
	 */
	uint64_t	tx_unicast;
	/**
	 * The number of transmitted multicast packets to the vap.
	 */
	uint64_t	tx_multicast;
	/**
	 * The number of transmitted broadcast packets to the vap.
	 */
	uint64_t	tx_broadcast;
	/**
	 * The number of received unicast packets from the vap.
	 */
	uint64_t	rx_unicast;
	/**
	 * The number of received multicast packets from the vap.
	 */
	uint64_t	rx_multicast;
	/**
	 * The number of received broadcast packets form the vap.
	 */
	uint64_t	rx_broadcast;

	/**
	 * tx succeed of num
	 */
	uint32_t tx_trans;
	/**
	 *  tx failed(include retry failed) of num
	 */
	uint32_t tx_ftrans;
	/**
	 * tx succeed but it have retry of num
	 */
	uint32_t tx_retries;
	/**
	 * tx succeed but it have once retry of num
	 */
	uint32_t tx_sretries;
	/**
	 * tx AMPDU of number
	 * if send SMPDU, this add 0.
	 * other add 1.
	 */
	uint32_t tx_aggpkts;
	/**
	 * tx no-ACK PPDU of number
	 */
	uint32_t tx_toutpkts;
	/**
	 * mgmt tx stats & rx stats
	 */
	struct dbg_get_mgmt_stats_cfm mgmt_stats;
} clsm_vap_extstats;

typedef struct _clsm_persta_extstats
{
	/* TX path */
	/**
	 * The number of transmitted bytes to the node.
	 */
	uint64_t	tx_bytes;
	/**
	 * The number of transmitted packets to the node.
	 */
	uint32_t	tx_pkts;
	/**
	 * The number of transmit discards to the node.
	 */
	uint32_t	tx_discard;
	/**
	 * The number of data packets transmitted through
	 * * wireless media for each Access Classes(AC).
	 */
	uint32_t	tx_wifi_sent[AC_MAX];
	/**
	 * The number of dropped data packets failed to transmit through
	 * wireless media for each Access Classes(AC)
	 */
	uint32_t	tx_wifi_drop[AC_MAX];
	/**
	 * The number of transmit errors to the node.
	 */
	uint32_t	tx_err;
	/**
	 * The number of transmitted unicast packets to the node.
	 */
	uint64_t	tx_unicast;
	/**
	 * The number of transmitted multicast packets to the node.
	 */
	uint64_t	tx_multicast;
	/**
	 * The number of transmitted broadcast packets to the node.
	 */
	uint64_t	tx_broadcast;
	/**
	 * TX PHY rate in megabits per second (Mbps).
	 */
	uint32_t	tx_phy_rate;

	/**
	 * The number of transmitted management frames to the node.
	 */
	uint32_t	tx_mgmt;

	/* RX path */
	/**
	 * The number of received bytes from the node.
	 */
	uint64_t	rx_bytes;
	/**
	 * The number of received packets from the node.
	 */
	uint32_t	rx_pkts;
	/**
	 * The numbder of received packets discarded from the node.
	 */
	uint32_t	rx_discard;
	/**
	 * The number of received packets in error from the node.
	 */
	uint32_t	rx_err;
	/**
	 * The number of received unicast packets from the node.
	 */
	uint64_t	rx_unicast;
	/**
	 * The number of received multicast packets from the node.
	 */
	uint64_t	rx_multicast;
	/**
	 * The number of received broadcast packets form the node.
	 */
	uint64_t	rx_broadcast;
	/**
	 * The number of received unknown packets from the node.
	 */
	uint32_t	rx_unknown;
	/**
	 * RX PHY rate in megabits per second (MBPS).
	 */
	uint32_t	rx_phy_rate;
	/**
	 * The number of received management frames from the node.
	 */
	uint32_t	rx_mgmt;
	/**
	 * The number of received control from the node.
	 */
	uint32_t	rx_ctrl;

	/**
	 * The MAC address of the node.
	 */
	uint8_t mac_addr[6];
	/**
	 * The rssi of the node.
	 */
	int8_t		rssi;
	/**
	 * The bandwidth of the node.
	 */
	int8_t		bw;
	/**
	 * Last Tx MCS index.
	 */
	uint8_t		tx_mcs;
	/**
	 * Last Rx MCS index.
	 */
	uint8_t		rx_mcs;
	/**
	 * Number of spatial streams used in last transmission.
	 */
	uint8_t		tx_nss;
	/**
	 * Number of spatial streams received in last reception.
	 */
	uint8_t		rx_nss;
	/**
	 * Bandwidth used in last transmission.
	 */
	uint8_t		tx_bw;
	/**
	 * Bandwidth used in last reception.
	 */
	uint8_t		rx_bw;
	/**
	 * Was short guard interval used in last transmission?
	 */
	uint8_t		tx_sgi;
	/**
	 * Was short guard interval used in last reception?
	 */
	uint8_t		rx_sgi;
} clsm_persta_extstats;

typedef struct _clsm_wpu_extstats
{
	/**
	 * cpu idle %:
	 */
	uint32_t cpu_idle_percent;

} clsm_wpu_extstats;

typedef struct _clsm_dfx_reset
{
	uint32_t flag;  ///enum CLS_WIFI_STATS_TYPE

} clsm_dfx_reset;


/////////////////////NL80211
int clsemi_vndr_cmds_dfx_get_radio_info(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len);

int clsemi_vndr_cmds_dfx_get_vap_info(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len);

int clsemi_vndr_cmds_dfx_get_peer_sta_info(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len);

int clsemi_vndr_cmds_dfx_get_wpu_info(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len);

int clsemi_vndr_cmds_dfx_reset(struct wiphy *wiphy,
		struct wireless_dev *wdev,
		const void *data, int len);

#endif
#endif

