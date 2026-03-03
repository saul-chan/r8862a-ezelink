/*
 * Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <errno.h>
#include "vbss.h"


/*****************************	Data type definitions	**************************/
#define TID_MAX 9

typedef char	string_32[33];
typedef char	string_64[65];
typedef char	string_128[129];
typedef char	string_256[257];
typedef char	string_512[513];
typedef char	string_1024[1025];


enum WPA_AUTHORIZE_TYPE {
	WPA_AUTHORIZE_TYPE_NONE = 0,
	WPA_AUTHORIZE_TYPE_WPA,
	WPA_AUTHORIZE_TYPE_WPA2,
	WPA_AUTHORIZE_TYPE_SAE
};

enum WPA_CIPHER_TYPE {
	WPA_CIPHER_NONE,
	WPA_CIPHER_AES,
	WPA_CIPHER_TKIP,
	WPA_CIPHER_TKIP_AES
};

enum {
	VBSS_VAP_ORIGIN,
	VBSS_VAP_ROAMING,
	VBSS_VAP_ROLE_MAX
};

/*
 * Infos of VBSS VAP
 */
struct vbss_vap_info {
	uint8_t bssid[ETH_ALEN];
	string_32 ssid;
	string_32 ifname;
	enum WPA_AUTHORIZE_TYPE auth_type;
	string_64 pwd;
	uint8_t role;
};

#define WPA_TK_MAX_LEN 32
#define WPA_GMK_LEN 32
#define WPA_GTK_MAX_LEN 32
#define WPA_IGTK_MAX_LEN 32
#define SAE_PMK_LEN 32
#define SAE_PMKID_LEN 16
#define WLAN_SUPP_RATES_MAX 32

struct vbss_key_info {
	/* Security related information */
	uint16_t auth_alg;
	uint16_t wpa_alg;
	/* Pairwise key, need to be installed to driver/FW */
	uint8_t tk[WPA_TK_MAX_LEN];

	/* Group Key related, set */
	uint8_t GMK[WPA_GMK_LEN];
	uint8_t GTK[2][WPA_GTK_MAX_LEN];
	int GTK_len;
	int GN;
	int GM;

	/* when 11w enabled */
	uint8_t IGTK[2][WPA_IGTK_MAX_LEN];
	uint8_t BIGTK[2][WPA_IGTK_MAX_LEN];
	int GN_igtk;
	int GM_igtk;

	/* when beacon protection enabled */
	int GN_bigtk;
	int GM_bigtk;

	/* WPA3 related */
	uint8_t pmk[SAE_PMK_LEN];
	uint8_t pmkid[SAE_PMKID_LEN];
};

struct ieee80211_ht_capabilities {
	uint16_t ht_capabilities_info;
	uint8_t a_mpdu_params;
	uint8_t supported_mcs_set[16];
	uint16_t ht_extended_capabilities;
	uint32_t tx_bf_capability_info;
	uint8_t asel_capabilities;
} __attribute__((packed));

struct ieee80211_vht_capabilities {
	uint32_t vht_capabilities_info;
	struct {
		uint16_t rx_map;
		uint16_t rx_highest;
		uint16_t tx_map;
		uint16_t tx_highest;
	} vht_supported_mcs_set;
} __attribute__((packed));

struct ieee80211_vht_operation {
	uint8_t vht_op_info_chwidth;
	uint8_t vht_op_info_chan_center_freq_seg0_idx;
	uint8_t vht_op_info_chan_center_freq_seg1_idx;
	uint16_t vht_basic_mcs_set;
} __attribute__((packed));

struct ieee80211_he_capabilities {
	uint8_t he_mac_capab_info[6];
	uint8_t he_phy_capab_info[11];
	uint8_t optional[37];
} __attribute__((packed));
#define IEEE80211_HE_CAPAB_MIN_LEN (6 + 11)

struct ieee80211_he_6ghz_band_cap {
	uint16_t capab;
} __attribute__((packed));

/*
 * Infos of VBSS STA
 */
struct vbss_sta_seq_num {
	uint16_t sta_idx;
	uint64_t tx_pn;
	uint64_t rx_pn[TID_MAX];
	uint32_t tx_seq_num[TID_MAX];
	uint32_t rx_seq_num[TID_MAX];
};

/* STA info set to driver or get from driver */
struct cls_vbss_driver_sta_info {
	uint8_t mac[ETH_ALEN];
	struct vbss_sta_seq_num sta_seq_info;
};

struct vbss_sta_info {
	uint8_t mac_addr[ETH_ALEN];
	uint8_t bssid[ETH_ALEN];
	/* AID for the sta */
	uint16_t aid;
	/* sta_info flags */
	uint32_t flags;
	/* support rates */
	uint8_t supported_rates[WLAN_SUPP_RATES_MAX];
	int supported_rates_len;
	/* Capability related */
	struct ieee80211_ht_capabilities ht_capabilities;
	struct ieee80211_vht_capabilities vht_capabilities;
	struct ieee80211_vht_operation vht_operation;
	uint8_t vht_opmode;
	struct ieee80211_he_capabilities he_capab;
	size_t he_capab_len;
	struct ieee80211_he_6ghz_band_cap he_6ghz_capab;

	struct vbss_key_info key_info;

	struct cls_vbss_driver_sta_info driver_sta;
};


/*
 * convert MAC address in string format "11:22:33:dd:ee:ff" to 6 byte hex.
 */
static inline int mac_aton(const char *str_mac, unsigned char hex_mac[ETH_ALEN])
{
#define STR_MAC_FORMAT	"aa:bb:cc:dd:ee:ff"
	int ret;

	if (strlen(str_mac) != sizeof(STR_MAC_FORMAT) - 1)
		return RET_CODE_ERROR;

	ret = sscanf(str_mac, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &hex_mac[0], &hex_mac[1], &hex_mac[2],
			&hex_mac[3], &hex_mac[4], &hex_mac[5]);
	if (ret == ETH_ALEN)
		return 0;
	else
		return RET_CODE_ERROR;
}

static inline int auth_atoi(const char *str_auth, int *int_auth)
{
	if (!str_auth)
		return RET_CODE_ERROR;

	if (strncmp(str_auth, "open", sizeof("open")) == 0)
		*int_auth = WPA_AUTHORIZE_TYPE_NONE;
	else if (strncmp(str_auth, "wpa", sizeof("wpa")) == 0)
		*int_auth = WPA_AUTHORIZE_TYPE_WPA;
	else if (strncmp(str_auth, "wpa2", sizeof("wpa2")) == 0)
		*int_auth = WPA_AUTHORIZE_TYPE_WPA2;
	else if (strncmp(str_auth, "sae", sizeof("sae")) == 0)
		*int_auth = WPA_AUTHORIZE_TYPE_SAE;
	else
		return RET_CODE_ERROR;

	return RET_CODE_OK;
}


static int connect_sock(const char *dest_ip, const int dest_port, const int local_port)
{
	int sockfd;
	struct sockaddr_in dest_addr, local_addr;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(dest_port);
	if (inet_pton(AF_INET, dest_ip, &dest_addr.sin_addr) <= 0) {
		perror("inet_pton error");
		goto error;
	}

	memset(&local_addr, 0, sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(local_port);
	local_addr.sin_addr.s_addr = INADDR_ANY;
	if (bind(sockfd, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
		perror("bind error\n");
		goto error;
	}

	if (connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
		printf("connect() error\n");
		goto error;
	}

	return sockfd;

error:
	close(sockfd);
	return RET_CODE_ERROR;
}

static inline int connect_agent(const char *agent_ip)
{
	return connect_sock(agent_ip, VBSS_AGENT_PORT, VBSS_AC_PORT);
}

static int vbss_send_msg(const char *agent_ip, char *msg_buf, const int msg_len, int *sockfd)
{
	int len;

	*sockfd = connect_agent(agent_ip);
	len = send(*sockfd, msg_buf, msg_len, 0);
	printf("%s() send %d bytes msg '%s' to %s, tx_len=%d\n", __func__, msg_len, msg_buf, agent_ip, len);

	return len;
}

static inline int vbss_wait_reply(int *fd, char *reply, int reply_len)
{
	int ret;
	ret = recv(*fd, reply, reply_len, 0);

	close(*fd);
	*fd = 0;
	return ret;
}


#define CMD_AGENT_PING			"ping"			// ping          <agent IP>
#define CMD_AGENT_ENABLE_VBSS	"enable_vbss"	// enable_vbss   <agent IP> <primary ifname> <0 | 1>
#define CMD_AGENT_ADD_VAP		"add_vap"		// add_vap       <agent IP> <ifname> <bssid> <ssid> <wpa2 | open> [key]
#define CMD_AGENT_COPY_VAP_INFO	"copy_vap_info"	// copy_vap_info <src IP> <ifname> <dest IP> <ifname>
#define CMD_AGENT_DEL_VAP		"del_vap"		// del_vap       <agent IP> <ifname>
#define CMD_AGENT_COPY_STA_INFO	"copy_sta_info"	// copy_sta_info <sta mac> <src IP> <ifname> <dest IP> <ifname>
#define CMD_AGENT_ROAM_STA		"roam_sta"		// roam_sta      <sta mac> <src IP> <ifname> <dest IP> <ifname>
#define CMD_AGENT_TRG_SWITCH	"trg_switch"	// trg_switch    <sta mac> <src IP> <ifname> <dest IP> <ifname>
#define CMD_AGENT_SWITCH_DONE	"switch_done"	// switch_done   <sta mac> <src IP> <ifname> <dest IP> <ifname>

void usage()
{
	printf("Usage:\n");
	printf("  vbss_ac <cmd> <param>\n");
	printf("  cmds:\n");
	printf("    ping          <agent IP>\n");
	printf("    enable_vbss   <agent IP> <primary ifname> <0 | 1>\n");
	printf("    add_vap       <agent IP> <ifname> <bssid> <ssid> <wpa2 | open> [key]\n");
	printf("    copy_vap_info <src IP> <ifname> <dest IP> <ifname>\n");
	printf("    del_vap       <agent IP> <ifname>\n");
	printf("    copy_sta_info <sta mac> <src IP> <ifname> <dest IP> <ifname>\n");
	printf("    roam_sta      <sta mac> <src IP> <ifname> <dest IP> <ifname> [arp]\n");
	printf("    trg_switch    <sta mac> <src IP> <ifname> <dest IP> <ifname>\n");
	printf("    switch_done   <sta mac> <src IP> <ifname> <dest IP> <ifname>\n");
	printf("\nNote: MAC address format: 00:11:22:33:44:55\n\n");
}



// #define CMD_AGENT_PING			"ping"			// ping <agent IP>

static int handle_ping_agent(const int argc, char **argv)
{
	int ret = RET_CODE_OK, len, fd = 0;
	char *dest_ip;
	char msg_buf[1024], reply[1024];

	if (argc < 1) {
		usage();
		return RET_CODE_ERROR;
	}
	dest_ip = argv[0];
	printf("%s cmd=%s dest_ip=%s\n", __func__, CMD_AGENT_PING, dest_ip);

	/* send ping to agent */
	len = snprintf(msg_buf, sizeof(msg_buf), "%s", MSG_AGENT_PING) + 1;
	ret = vbss_send_msg(dest_ip, msg_buf, len, &fd);
	if (ret != len)
		goto error;

	printf("wait reply from '%s' ...\n", dest_ip);
	ret = vbss_wait_reply(&fd, reply, sizeof(reply));
	if (ret <= 0)
		goto error;
	printf("reply len=%d errno=%d reply='%s'\n", ret, errno, reply);
	if (strncmp(reply, "pong", sizeof("pong")) == 0)
		printf("Pong\n");
	else
		printf("Error, no response from agent %s\n", dest_ip);
	ret = RET_CODE_OK;

error:
	if (!ret)
		printf("========== %s() OK\n\n", __func__);
	else
		printf("error: ret=%d errno=%d\n", ret, errno);
	if (fd)
		close(fd);
	return ret;
}

// CMD_AGENT_ENABLED_VBSS	"enable_vbss"	// enable_vbss   <agent IP> <primary ifname> <0 | 1>

// #define MSG_AGENT_ENABLE_VBSS	"enable_vbss"
	/* Reqest msg format:	"enable_vbss <primary ifname> <0 | 1>" + '\0'
	 * Reply msg format:	"OK" or "FAIL"
	 */
static int handle_enable_vbss(const int argc, char **argv)
{
	int ret, len, fd = 0;
	char *ifname, *agent_ip, *onoff;
	char msg_buf[1024], reply[1024];

	if (argc < 2) {
		usage();
		return RET_CODE_ERROR;
	}
	agent_ip = argv[0];
	ifname = argv[1];
	onoff = argv[2];

	printf("%s cmd=%s %s %s %s\n", __func__, CMD_AGENT_ENABLE_VBSS, agent_ip, ifname, onoff);

	/* enable/disable vbss */
	len = snprintf(msg_buf, sizeof(msg_buf), "%s %s %s", MSG_AGENT_ENABLE_VBSS, ifname, onoff) + 1;
	ret = vbss_send_msg(agent_ip, msg_buf, len, &fd);
	if (ret != len)
		goto error;

	printf("wait reply from '%s' ...\n", agent_ip);
	ret = vbss_wait_reply(&fd, reply, sizeof(reply));
	if (ret <= 0)
		goto error;
	printf("reply len=%d errno=%d reply='%s'\n", ret, errno, reply);

	ret = RET_CODE_OK;

error:
	if (!ret)
		printf("========== %s() OK\n\n", __func__);
	else
		printf("error: ret=%d errno=%d\n", ret, errno);
	if (fd)
		close(fd);
	return ret;
}


// CMD_AGENT_ADD_VAP		"add_vap"		// add_vap <agent IP> <ifname> <bssid> <ssid> <wpa2 | open> [key]

// MSG_AGENT_ADD_VAP_INFO	"add_vap_info"
	/* Reqest msg format:	"add_vap_info <ifname>" + '\0' + <bin of vap_info>
	 * Reply msg format:	"OK" or "FAIL"
	 */
static int handle_add_vap(const int argc, char **argv)
{
	int ret, len, fd = 0;
	char *ifname, *agent_ip, *bssid, *ssid, *auth;
	char *key = NULL;
	char msg_buf[1024], reply[1024];
	struct vbss_vap_info vap_info = {0};

	if (argc < 5) {
		usage();
		return RET_CODE_ERROR;
	}
	agent_ip = argv[0];
	ifname = argv[1];
	bssid = argv[2];
	ssid = argv[3];
	auth = argv[4];
	if (strncmp(auth, "open", sizeof("open")) != 0) {
		/* Not OPEN, need key */
		if (argc < 6) {
			usage();
			return RET_CODE_ERROR;
		}
		key = argv[5];
	}
	printf("%s cmd=%s %s %s %s %s %s\n", __func__, CMD_AGENT_ADD_VAP, agent_ip, ifname, bssid, auth, key?key:"");

	/* fill vap info */
	if (mac_aton(bssid, (uint8_t *)&(vap_info.bssid)))
		return RET_CODE_ERROR;
	strncpy(vap_info.ssid, ssid, sizeof(vap_info.ssid) - 1);
	strncpy(vap_info.ifname, ifname, sizeof(vap_info.ifname) - 1);
	if (auth_atoi(auth, (int *)(&(vap_info.auth_type))))
		return RET_CODE_ERROR;
	if(vap_info.auth_type)
		strncpy(vap_info.pwd, key, sizeof(vap_info.pwd) - 1);
	vap_info.role = VBSS_VAP_ORIGIN;

	/* add vap info to agent_ip */
	len = snprintf(msg_buf, sizeof(msg_buf), "%s %s", MSG_AGENT_ADD_VAP_INFO, ifname) + 1;
	memcpy(msg_buf + len, &vap_info, sizeof(vap_info));
	len += sizeof(vap_info);
	ret = vbss_send_msg(agent_ip, msg_buf, len, &fd);
	if (ret != len)
		goto error;

	printf("wait reply from '%s' ...\n", agent_ip);
	ret = vbss_wait_reply(&fd, reply, sizeof(reply));
	if (ret <= 0)
		goto error;
	printf("reply len=%d errno=%d reply='%s'\n", ret, errno, reply);

	ret = RET_CODE_OK;

error:
	if (!ret)
		printf("========== %s() OK\n\n", __func__);
	else
		printf("error: ret=%d errno=%d\n", ret, errno);
	if (fd)
		close(fd);
	return ret;
}


// CMD_AGENT_COPY_VAP_INFO	"copy_vap_info"	// copy_vap_info <src IP> <ifname> <dest IP> <ifname>

// MSG_AGENT_GET_VAP_INFO	"get_vap_info"
	/* Reqest msg format:	"get_vap_info <ifname>"
	 * Reply msg format:	<bin of vap_info>
	 */
// MSG_AGENT_ADD_VAP_INFO	"add_vap_info"
	/* Reqest msg format:	"add_vap_info <ifname>" + '\0' + <bin of vap_info>
	 * Reply msg format:	"OK" or "FAIL"
	 */
static int handle_copy_vap_info(const int argc, char **argv)
{
	int ret, len, src_fd = 0, dest_fd = 0;
	char *src_ifname, *src_ip, *dest_ip, *dest_ifname;
	char msg_buf[1024], reply[1024];
	struct vbss_vap_info vap_info;

	if (argc < 4) {
		usage();
		return RET_CODE_ERROR;
	}
	src_ip = argv[0];
	src_ifname = argv[1];
	dest_ip = argv[2];
	dest_ifname = argv[3];
	printf("%s cmd=%s %s %s %s %s\n", __func__, CMD_AGENT_COPY_VAP_INFO, src_ip, src_ifname, dest_ip, dest_ifname);

	/* get vap info from src_ip */
	len = snprintf(msg_buf, sizeof(msg_buf), "%s %s", MSG_AGENT_GET_VAP_INFO, src_ifname) + 1;
	ret = vbss_send_msg(src_ip, msg_buf, len, &src_fd);
	if (ret != len)
		goto error;

	printf("wait reply from '%s' ...\n", src_ip);
	ret = vbss_wait_reply(&src_fd, (char *)&vap_info, sizeof(vap_info));
	if (ret <= 0)
		goto error;
	printf("reply len=%d got vap_info: bssid %02x:%02x:%02x:%02x:%02x:%02x ssid=%s\n", ret,
			vap_info.bssid[0], vap_info.bssid[1], vap_info.bssid[2],
			vap_info.bssid[3], vap_info.bssid[4], vap_info.bssid[5], vap_info.ssid);

	/* add vap info to dest_ip */
	vap_info.role = VBSS_VAP_ROAMING;
	len = snprintf(msg_buf, sizeof(msg_buf), "%s %s", MSG_AGENT_ADD_VAP_INFO, dest_ifname) + 1;
	memcpy(msg_buf + len, &vap_info, sizeof(vap_info));
	len += sizeof(vap_info);
	ret = vbss_send_msg(dest_ip, msg_buf, len, &dest_fd);
	if (ret != len)
		goto error;

	printf("wait reply from '%s' ...\n", dest_ip);
	ret = vbss_wait_reply(&dest_fd, reply, sizeof(reply));
	if (ret <= 0)
		goto error;
	printf("reply len=%d errno=%d reply='%s'\n", ret, errno, reply);

	ret = RET_CODE_OK;

error:
	if (!ret)
		printf("========== %s() OK\n\n", __func__);
	else
		printf("error: ret=%d errno=%d\n", ret, errno);
	if (src_fd)
		close(src_fd);
	if (dest_fd)
		close(dest_fd);
	return ret;
}

// CMD_AGENT_DEL_VAP		"del_vap"		// del_vap <agent IP> <ifname>

// #define MSG_AGENT_DEL_VAP	"del_vap"
	/* Reqest msg format:	"del_vap <ifname>" + '\0'
	 * Reply msg format:	"OK" or "FAIL"
	 */
static int handle_del_vap(const int argc, char **argv)
{
	int ret, len, fd = 0;
	char *ifname, *agent_ip;
	char msg_buf[1024], reply[1024];

	if (argc < 2) {
		usage();
		return RET_CODE_ERROR;
	}
	agent_ip = argv[0];
	ifname = argv[1];
	printf("%s cmd=%s %s %s\n", __func__, CMD_AGENT_DEL_VAP, agent_ip, ifname);

	/* del vap */
	len = snprintf(msg_buf, sizeof(msg_buf), "%s %s", MSG_AGENT_DEL_VAP, ifname) + 1;
	ret = vbss_send_msg(agent_ip, msg_buf, len, &fd);
	if (ret != len)
		goto error;

	printf("wait reply from '%s' ...\n", agent_ip);
	ret = vbss_wait_reply(&fd, reply, sizeof(reply));
	if (ret <= 0)
		goto error;
	printf("reply len=%d errno=%d reply='%s'\n", ret, errno, reply);

	ret = RET_CODE_OK;

error:
	if (!ret)
		printf("========== %s() OK\n\n", __func__);
	else
		printf("error: ret=%d errno=%d\n", ret, errno);
	if (fd)
		close(fd);
	return ret;
}

/* S4. "send fake pkt to ac" <sta mac> <src IP> <ifname> <dest IP> <ifname> */
static int fake_arp_request_to_ac(const int argc, char **argv)
{
	int ret, len, dest_fd = 0;
	char *str_sta_mac;
	unsigned char hex_sta_mac[ETH_ALEN];
	char *src_ifname, *dest_ip;
	char msg_buf[1024], reply[1024];
	uint32_t sta_ip = 0;

	if (argc < 5) {
		usage();
		return RET_CODE_ERROR;
	}

	str_sta_mac = argv[0];
	src_ifname = argv[2];
	dest_ip = argv[3];

	if (mac_aton(str_sta_mac, hex_sta_mac))
		return RET_CODE_ERROR;

	get_ip_from_arp_cache(&sta_ip, hex_sta_mac);

	// add command + ifname into msg
	len = snprintf(msg_buf, sizeof(msg_buf), "%s %s", MSG_AGENT_PKT_TO_AC, src_ifname) + 1;
	// add sta_mac into msg
	memcpy(msg_buf + len, hex_sta_mac, ETH_ALEN);
	// add sta_ip into msg
	memcpy(msg_buf + len + ETH_ALEN, &sta_ip, sizeof(sta_ip));

	len = len + ETH_ALEN + sizeof(sta_ip);
	ret = vbss_send_msg(dest_ip, msg_buf, len, &dest_fd);
	if (ret != len) {
		printf("send error msg_buf='%s' send dest_fd=%d ret=%d len=%d\n", msg_buf, dest_fd, ret, len);
		goto error;
	}

	ret = vbss_wait_reply(&dest_fd, reply, sizeof(reply));
	if (ret <= 0)
		goto error;

	ret =0;
error:
	if (!ret)
		printf("========== %s() OK\n\n", __func__);
	if (dest_fd > 0)
		close(dest_fd);
	return ret;
}

// CMD_AGENT_COPY_STA_INFO	"copy_sta_info"	// copy_sta_info <sta mac> <src IP> <ifname> <dest IP> <ifname>

// MSG_AGENT_GET_STA_INFO	"get_sta_info"
	/* Reqest msg format:	"get_sta_info <ifname>" + '\0' + <sta_mac in hexbin>
	 * Reply msg format:	<bin of sta_info>
	 */
// MSG_AGENT_ADD_STA_INFO	"add_sta_info"
	/* Reqest msg format:	"add_sta_info <ifname>" + '\0' + <bin of sta_info>
	 * Reply msg format:	"OK" or "FAIL <error msg>"
	 */
static int handle_copy_sta_info(const int argc, char **argv)
{
	int ret, len, src_fd = 0, dest_fd = 0;
	char *str_sta_mac;
	unsigned char hex_sta_mac[ETH_ALEN];
	char *src_ifname, *dest_ifname, *src_ip, *dest_ip;
	char msg_buf[1024], reply[1024];
	struct vbss_sta_info sta_info;

	if (argc < 5) {
		usage();
		return RET_CODE_ERROR;
	}
	str_sta_mac = argv[0];
	src_ip = argv[1];
	src_ifname = argv[2];
	dest_ip = argv[3];
	dest_ifname = argv[4];
	printf("%s cmd=%s %s %s %s %s %s\n", __func__, CMD_AGENT_COPY_STA_INFO, str_sta_mac, src_ip, src_ifname, dest_ip, dest_ifname);

	if (mac_aton(str_sta_mac, hex_sta_mac))
		return RET_CODE_ERROR;

	/* get sta info from src_ip */
	len = snprintf(msg_buf, sizeof(msg_buf), "%s %s", MSG_AGENT_GET_STA_INFO, src_ifname) + 1;
	memcpy(msg_buf + len, hex_sta_mac, ETH_ALEN);
	len += ETH_ALEN;
	ret = vbss_send_msg(src_ip, msg_buf, len, &src_fd);
	if (ret != len) {
		printf("msg_buf='%s' send srcfd=%d ret=%d len=%d\n", msg_buf, src_fd, ret, len);
		goto error;
	}

	printf("wait reply from '%s' ...\n", src_ip);
	ret = vbss_wait_reply(&src_fd, (char *)&sta_info, sizeof(sta_info));
	if (ret <= 0)
		goto error;

	printf("reply len=%d got sta_info: mac %02x:%02x:%02x:%02x:%02x:%02x\n", ret,
			sta_info.mac_addr[0], sta_info.mac_addr[1], sta_info.mac_addr[2],
			sta_info.mac_addr[3], sta_info.mac_addr[4], sta_info.mac_addr[5]);

	/* add sta info to dest_ip */
	memset(msg_buf, 0, sizeof(msg_buf));
	len = snprintf(msg_buf, sizeof(msg_buf), "%s %s", MSG_AGENT_ADD_STA_INFO, dest_ifname) + 1;
	memcpy(msg_buf + len, &sta_info, sizeof(sta_info));
	len += sizeof(sta_info);
	ret = vbss_send_msg(dest_ip, msg_buf, len, &dest_fd);
	if (ret != len)
		goto error;

	printf("wait reply from '%s' ...\n", dest_ip);
	ret = vbss_wait_reply(&dest_fd, reply, sizeof(reply));
	printf("reply len=%d errno=%d reply='%s'\n", ret, errno, reply);
	if (ret <= 0)
		goto error;

	ret = 0;

error:
	if (!ret)
		printf("========== %s() OK\n\n", __func__);
	else
		printf("error: ret=%d errno=%d\n", ret, errno);
	if (src_fd > 0)
		close(src_fd);
	if (dest_fd > 0)
		close(dest_fd);
	return ret;
}


// #define CMD_AGENT_ROAM_STA		"roam_sta"		// roam_sta      <sta mac> <src IP> <ifname> <dest IP> <ifname> [arp]
/* Roam STA procedures:
 * S1. "copy_vap_info"           <src IP> <ifname> <dest IP> <ifname>
 * S2. "copy_sta_info" <sta mac> <src IP> <ifname> <dest IP> <ifname>
 * S3. "del_vap"                 <src IP> <ifname>
 * S4. (optional)"send fake arp request"      <src IP> <ifname> <dest IP> <ifname>
 */
static int handle_roam_sta(const int argc, char **argv)
{
	char *str_sta_mac, *src_ip, *src_ifname, *dest_ip, *dest_ifname;
	char *arp_request = NULL;

	if (argc < 5) {
		usage();
		return RET_CODE_ERROR;
	}
	str_sta_mac = argv[0];
	src_ip = argv[1];
	src_ifname = argv[2];
	dest_ip = argv[3];
	dest_ifname = argv[4];

	printf("%s cmd=%s %s %s %s %s %s", __func__, CMD_AGENT_ROAM_STA, str_sta_mac, src_ip, src_ifname, dest_ip, dest_ifname);
	if (argc > 5){
		arp_request = argv[5];
		printf(" %s\n", arp_request);
	} else
		printf("\n");

	/* S1. "copy_vap_info" <src IP> <ifname> <dest IP> <ifname> */
	printf("\nS1. copy_vap_info %s %s %s %s\n", argv[0], argv[1], argv[2], argv[3]);
	if (handle_copy_vap_info(4, argv + 1)) {
		printf("copy_vap_info FAILED\n");
		return RET_CODE_ERROR;
	}

	/* S2. "copy_sta_info" <sta mac> <src IP> <ifname> <dest IP> <ifname> */
	printf("\nS2. copy_sta_info %s %s %s %s %s\n", argv[0], argv[1], argv[2], argv[3], argv[4]);
	if (handle_copy_sta_info(5, argv)) {
		printf("copy_sta_info FAILED\n");
		return RET_CODE_ERROR;
	}

	/* S3. "del_vap" <src IP> <ifname> */
	printf("\nS3. del_vap %s %s\n", argv[1], argv[2]);
	if (handle_del_vap(2, argv + 1)) {
		printf("del_vap FAILED\n");
		return RET_CODE_ERROR;
	}

	/* S4. "send fake pkt to ac" <sta mac> <src IP> <ifname> <dest IP> <ifname> */
	if (arp_request != NULL) {
		if (strcmp(arp_request, "arp") == 0) {
			printf("\nS4. send fake pkt to ac %s %s %s %s %s\n", argv[0], argv[1], argv[2], argv[3], argv[4]);
			if (fake_arp_request_to_ac(5, argv)) {
				printf("send fake pkt to ac FAILED\n");
				return RET_CODE_ERROR;
			}
		}
	}
	return RET_CODE_OK;
}


/* send msg with format of "<cmd> <ifname> <STA MAC in 112233445566>"
 */
static int send_msg_by_ifname_sta(const char *agent_ip, const char *ifname,
		const uint8_t sta_mac[ETH_ALEN], const char *msg)
{
	int ret, len, fd = 0;
	char msg_buf[1024], reply[1024];

	len = snprintf(msg_buf, sizeof(msg_buf), "%s %s", msg, ifname) + 1;
	memcpy(msg_buf + len, sta_mac, ETH_ALEN);
	len += ETH_ALEN;
	ret = vbss_send_msg(agent_ip, msg_buf, len, &fd);
	if (ret != len) {
		printf("msg_buf='%s' send fd=%d ret=%d len=%d\n", msg_buf, fd, ret, len);
		goto error;
	}

	printf("wait reply from '%s' ...\n", agent_ip);
	ret = vbss_wait_reply(&fd, reply, sizeof(reply));
	if (ret <= 0)
		goto error;

	printf("reply_len=%d reply=%s\n", ret, reply);
	if (strncmp(reply, MSG_OK, sizeof(MSG_OK)))
		goto error;

	ret = 0;

error:
	if (fd > 0)
		close(fd);
	return ret;
}


// CMD_AGENT_TRG_SWITCH		"trg_switch"	// trg_switch <sta mac> <src IP> <ifname> <dest IP> <ifname>

//#define MSG_AGENT_TRG_SWITCH	"trigger_switch"
	/* Reqest msg format:	"trigger_switch <ifname> <STA MAC in 112233445566>"
	 * Reply msg format:	"OK" or "FAIL"
	 */
static int handle_trigger_switch(const int argc, char **argv)
{
	int ret;
	char *str_sta_mac;
	unsigned char hex_sta_mac[ETH_ALEN];
	char *src_ifname, *dest_ifname, *src_ip, *dest_ip;

	if (argc < 5) {
		usage();
		return RET_CODE_ERROR;
	}
	str_sta_mac = argv[0];
	src_ip = argv[1];
	src_ifname = argv[2];
	dest_ip = argv[3];
	dest_ifname = argv[4];
	printf("%s cmd=%s %s %s %s %s %s\n", __func__, CMD_AGENT_TRG_SWITCH, str_sta_mac, src_ip, src_ifname, dest_ip, dest_ifname);

	if (mac_aton(str_sta_mac, hex_sta_mac))
		return RET_CODE_ERROR;

	/* send msg to src agent */
	ret = send_msg_by_ifname_sta(src_ip, src_ifname, hex_sta_mac, MSG_AGENT_TRG_SWITCH);
	if (ret)
		return ret;

	/* send msg to dest agent */
	ret = send_msg_by_ifname_sta(dest_ip, dest_ifname, hex_sta_mac, MSG_AGENT_TRG_SWITCH);
	if (ret)
		return ret;

	printf("%s() OK\n", __func__);
	return RET_CODE_OK;
}


// CMD_AGENT_SWITCH_DONE		"switch_done"	// switch_done <sta mac> <src IP> <ifname> <dest IP> <ifname>

//#define MSG_AGENT_SWITCH_DONE	"switch_done"
	/* Reqest msg format:	"switch_done <ifname> <STA MAC in 112233445566>"
	 * Reply msg format:	"OK" or "FAIL"
	 */
static int handle_switch_done(const int argc, char **argv)
{
	int ret;
	char *str_sta_mac;
	unsigned char hex_sta_mac[ETH_ALEN];
	char *src_ifname, *dest_ifname, *src_ip, *dest_ip;

	if (argc < 5) {
		usage();
		return RET_CODE_ERROR;
	}
	str_sta_mac = argv[0];
	src_ip = argv[1];
	src_ifname = argv[2];
	dest_ip = argv[3];
	dest_ifname = argv[4];
	printf("%s cmd=%s %s %s %s %s %s\n", __func__, CMD_AGENT_SWITCH_DONE, str_sta_mac, src_ip, src_ifname, dest_ip, dest_ifname);

	if (mac_aton(str_sta_mac, hex_sta_mac))
		return RET_CODE_ERROR;

	/* send msg to src agent */
	ret = send_msg_by_ifname_sta(src_ip, src_ifname, hex_sta_mac, MSG_AGENT_SWITCH_DONE);
	if (ret)
		return ret;

	/* send msg to dest agent */
	ret = send_msg_by_ifname_sta(dest_ip, dest_ifname, hex_sta_mac, MSG_AGENT_SWITCH_DONE);
	if (ret)
		return ret;

	printf("%s() OK\n", __func__);
	return RET_CODE_OK;
}


/*
* argv[0]: vbss_ac
* argv[1]: cmd
* argv[2]: params
*
* cmd:
* CMD_AGENT_COPY_VAP_INFO	"copy_vap_info"	// copy_vap_info <src IP> <ifname> <dest IP>
* CMD_AGENT_COPY_STA_INFO	"copy_sta_info"	// copy_sta_info <sta mac> <src IP> <ifname> <dest IP> <ifname>
* CMD_AGENT_TRG_SWITCH		"trg_switch"	// trg_switch    <sta mac> <src IP> <ifname> <dest IP> <ifname>
* CMD_AGENT_SWITCH_DONE		"switch_done"	// switch_done   <sta mac> <src IP> <ifname> <dest IP> <ifname>
*/
int main(int argc, char **argv)
{
	int ret;
	char *cmd = argv[1];

	if (argc < 2) {
		usage();
		return RET_CODE_ERROR;
	}

	if (strncmp(cmd, CMD_AGENT_COPY_VAP_INFO, sizeof(CMD_AGENT_COPY_VAP_INFO)) == 0) {
		ret = handle_copy_vap_info(argc - 2, argv + 2);
	}
	else if (strncmp(cmd, CMD_AGENT_COPY_STA_INFO, sizeof(CMD_AGENT_COPY_STA_INFO)) == 0) {
		ret = handle_copy_sta_info(argc - 2, argv + 2);
	}
	else if (strncmp(cmd, CMD_AGENT_TRG_SWITCH, sizeof(CMD_AGENT_TRG_SWITCH)) == 0) {
		ret = handle_trigger_switch(argc - 2, argv + 2);
	}
	else if (strncmp(cmd, CMD_AGENT_SWITCH_DONE, sizeof(CMD_AGENT_SWITCH_DONE)) == 0) {
		ret = handle_switch_done(argc - 2, argv + 2);
	}
	else if (strncmp(cmd, CMD_AGENT_PING, sizeof(CMD_AGENT_PING)) == 0) {
		ret = handle_ping_agent(argc - 2, argv + 2);
	}
	else if (strncmp(cmd, CMD_AGENT_ADD_VAP, sizeof(CMD_AGENT_ADD_VAP)) == 0) {
		ret = handle_add_vap(argc - 2, argv + 2);
	}
	else if (strncmp(cmd, CMD_AGENT_DEL_VAP, sizeof(CMD_AGENT_DEL_VAP)) == 0) {
		ret = handle_del_vap(argc - 2, argv + 2);
	}
	else if (strncmp(cmd, CMD_AGENT_ROAM_STA, sizeof(CMD_AGENT_ROAM_STA)) == 0) {
		ret = handle_roam_sta(argc - 2, argv + 2);
	}
	else if (strncmp(cmd, CMD_AGENT_ENABLE_VBSS, sizeof(CMD_AGENT_ENABLE_VBSS)) == 0) {
		ret = handle_enable_vbss(argc - 2, argv + 2);
	}
	else {
		usage();
		return RET_CODE_ERROR;
	}
	return ret;
}


