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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <arpa/inet.h>
#include <linux/nl80211.h>
#include <cls/cls_nl80211_vendor.h>

#define CLS_CSI_DAEMON_VERSION	1
#define CLS_CSI_REPORTER_PORT 37387
#define MACFMT "%02x:%02x:%02x:%02x:%02x:%02x"
#define MACARG(src) (src)[0], (src)[1], (src)[2], (src)[3], (src)[4], (src)[5]

#define CLSCSI_REPORTER_DBG_ERR		1
#define CLSCSI_REPORTER_DBG_INFO	2
#define CLSCSI_REPORTER_DBG_DBG		3


static int g_float_iq, g_dbg = CLSCSI_REPORTER_DBG_ERR;
static char g_server_ip[64] = {0};
static uint8_t pkt_buffer[65 * 1024];

struct handler_args {
	const char *group;
	int id;
};

struct sock_ctrl {
	struct nl_sock *sock;
	int id;
};

#define CSI_MAX_SUBCARRIER_NUM 2048
#define CSI_HEAD_INFO_LEN 16
#define CSI_EXTRA_REPORT_INFO_LEN 128
#define CSI_REPORT_INFO_MAX_SIZE (CSI_EXTRA_REPORT_INFO_LEN - CSI_HEAD_INFO_LEN)

/* struct of CSI report info, struct is aligned to driver */
struct __attribute__((packed)) cls_csi_report_info {
	uint32_t timestamp_hi;
	uint32_t timestamp_lo;
	uint8_t dest_mac_addr[6];
	uint8_t sta_mac_addr[6];
	int8_t rssi;
	uint8_t rx_nss;
	uint8_t tx_nss;
	uint8_t channel;
	uint8_t bw;
	uint16_t subcarrier_num;
	/* enum hw_format_mod */
	uint8_t ppdu_format;
	/* enum csi_src_ltf_type */
	uint8_t ltf_type;
	uint8_t agc[8];
};

union cls_csi_report_info_aligned {
	struct cls_csi_report_info info;
	uint8_t csi_inf_reserved[CSI_REPORT_INFO_MAX_SIZE];
};

/* Head of CSI info */
struct cls_csi_info_head {
	uint16_t ppdu_id;
	uint16_t csi_length;
	uint32_t reserved[3];
};

/* The information in 1 subcarrier */
struct cls_csi_info_per_subcarrier {
	uint16_t h00_q;
	uint16_t h00_i;
	uint16_t h01_q;
	uint16_t h01_i;
	uint16_t h10_q;
	uint16_t h10_i;
	uint16_t h11_q;
	uint16_t h11_i;
};

struct cls_csi_report_info_iq {
	struct cls_csi_info_per_subcarrier subc[CSI_MAX_SUBCARRIER_NUM];
};

struct cls_csi_report {
	union cls_csi_report_info_aligned info_aligned;
	struct cls_csi_info_head hdr;
	struct cls_csi_report_info_iq csi_iq;
};

static struct cls_csi_report csi_rpt_buf;

enum cls_csi_val_type {
	CLS_CSI_VAL_UINT8,
	CLS_CSI_VAL_UINT16,
	CLS_CSI_VAL_UINT32,
	CLS_CSI_VAL_INT8,
	CLS_CSI_VAL_MAC,

	_CLS_CSI_VAL_MAX,
};

enum cls_csi_frm_type {
	CLS_CSI_RPT_VERSION = 0,
	CLS_CSI_RPT_TS_HI,
	CLS_CSI_RPT_TS_LOW,
	CLS_CSI_RPT_DEST_MAC,
	CLS_CSI_RPT_STA_MAC,
	CLS_CSI_RPT_RSSI,
	CLS_CSI_RPT_RX_NSS,
	CLS_CSI_RPT_TX_NSS,
	CLS_CSI_RPT_CHANNEL,
	CLS_CSI_RPT_BW,
	CLS_CSI_RPT_SUBC_NUM,	/* Sub-Carrier num */
	CLS_CSI_RPT_PPDU_FMT,
	CLS_CSI_RPT_LTF_TYPE,
	CLS_CSI_RPT_AGC,
	CLS_CSI_RPT_PPDU_ID,
	CLS_CSI_RPT_IQ_LEN,
	CLS_CSI_RPT_IQ_FMT,		/* fixed or float CSI I/Q */
	CLS_CSI_RPT_IQ_DATA,

	/* Keep MAX at the bottom */
	_CLS_CSI_RPT_MAX
};

#define encap_csi_tl(buf, type, len) do { \
	*((uint8_t *)(buf)++) = (type); \
	*((uint32_t *)(buf)) = htonl(len); \
	(buf) += sizeof(uint32_t); \
} while (0)

#define encap_csi_tlv_uint8(buf, type, val) do { \
	encap_csi_tl(buf, type, sizeof(uint8_t)); \
	*((uint8_t *)(buf)++) = (val); \
} while (0)

#define encap_csi_tlv_uint16(buf, type, val) do { \
	encap_csi_tl(buf, type, sizeof(uint16_t)); \
	*((uint16_t *)(buf)) = htons(val); \
	(buf) += sizeof(uint16_t); \
} while (0)

#define encap_csi_tlv_uint32(buf, type, val) do { \
	encap_csi_tl(buf, type, sizeof(uint32_t)); \
	*((uint32_t *)(buf)) = htonl(val); \
	(buf) += sizeof(uint32_t); \
} while (0)

#define encap_csi_tlv_int8(buf, type, val) do { \
	encap_csi_tl(buf, type, sizeof(int8_t)); \
	*((int8_t *)(buf)++) = (val); \
} while (0)

#define encap_val_uint16(buf, val) do { \
	*((uint16_t *)(buf)) = htons(val); \
	(buf) += sizeof(uint16_t); \
} while (0)

#define encap_val_float(buf, val) do { \
	uint32_t *u32 = (uint32_t *)(&(val)); \
	*((uint32_t *)(buf)) = htonl(*u32); \
	(buf) += sizeof(float); \
} while (0)

#define encap_csi_tlv_uint8_array(buf, type, len, val) do { \
	encap_csi_tl(buf, type, len); \
	memcpy(buf, val, len); \
	(buf) += len; \
} while (0)

/* CIS I/Q format:
 *	bit 0~10:	decimals
 *	bit 11:		integer
 *	bit 12:		sign bit, 0: positive, 1: negative
 *	bit 13~15:	same to bit12
 */
#define iq_to_float(int_iq, float_iq) do { \
	(float_iq) = (float)((int_iq) & 0xFFF) / 0x800; \
	if ((int_iq) & 0x1000) \
		(float_iq) *= -1; \
} while (0)

static int _cmdError(struct sockaddr_nl *nla, struct nlmsgerr *err, void *arg)
{
	int *ret = arg;

	*ret = err->error;

	return NL_STOP;
}

static int _cmdAck(struct nl_msg *msg, void *arg)
{
	int *ret = arg;
	*ret = 0;

	return NL_STOP;
}
static int _eventNoSeqCheck(struct nl_msg *msg, void *arg)
{
	return NL_OK;
}

static int _cmdFamily(struct nl_msg *msg, void *arg)
{
	struct handler_args *grp = arg;
	struct nlattr *tb[CTRL_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *mcgrp;
	int rem_mcgrp;

	nla_parse(tb, CTRL_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
	genlmsg_attrlen(gnlh, 0), NULL);

	if (!tb[CTRL_ATTR_MCAST_GROUPS])
		return NL_SKIP;

	nla_for_each_nested(mcgrp, tb[CTRL_ATTR_MCAST_GROUPS], rem_mcgrp) {
		struct nlattr *tb_mcgrp[CTRL_ATTR_MCAST_GRP_MAX + 1];

		nla_parse(tb_mcgrp, CTRL_ATTR_MCAST_GRP_MAX,
		nla_data(mcgrp), nla_len(mcgrp), NULL);

		if (!tb_mcgrp[CTRL_ATTR_MCAST_GRP_NAME] || !tb_mcgrp[CTRL_ATTR_MCAST_GRP_ID])
			continue;
		if (strncmp(nla_data(tb_mcgrp[CTRL_ATTR_MCAST_GRP_NAME]),
			grp->group, nla_len(tb_mcgrp[CTRL_ATTR_MCAST_GRP_NAME])))
			continue;
		grp->id = nla_get_u32(tb_mcgrp[CTRL_ATTR_MCAST_GRP_ID]);
		break;
	}

	return NL_SKIP;
}

static int _getNlMulticastId(struct nl_sock *sock, const char *family, const char *group)
{
	struct nl_msg *msg;
	struct nl_cb *cb;
	int ret, ctrlid;
	struct handler_args grp = {
		.group = group,
		.id = -ENOENT,
	};

	msg = nlmsg_alloc();
	if (!msg)
		return -ENOMEM;

	cb = nl_cb_alloc(NL_CB_DEFAULT);
	if (!cb) {
		ret = -ENOMEM;
		goto out_fail_cb;
	}

	ctrlid = genl_ctrl_resolve(sock, "nlctrl");
	genlmsg_put(msg, 0, 0, ctrlid, 0, 0, CTRL_CMD_GETFAMILY, 0);
	NLA_PUT_STRING(msg, CTRL_ATTR_FAMILY_NAME, family);

	ret = nl_send_auto_complete(sock, msg);
	if (ret < 0)
		goto nla_put_failure;

	ret = 1;
	nl_cb_err(cb, NL_CB_CUSTOM, _cmdError, &ret);
	nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, _cmdAck, &ret);
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, _cmdFamily, &grp);

	while (ret > 0)
		ret = nl_recvmsgs(sock, cb);

	if (ret == 0)
		ret = grp.id;

nla_put_failure:
	nl_cb_put(cb);
out_fail_cb:
	nlmsg_free(msg);
	return ret;
}

int send_data_to_svr(uint8_t *buf, size_t len)
{
	int sock_fd;
	struct sockaddr_in server_addr;
	static int i;

	sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock_fd < 0) {
		perror("Unable to create socket");
		exit(EXIT_FAILURE);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(CLS_CSI_REPORTER_PORT);
	server_addr.sin_addr.s_addr = inet_addr(g_server_ip);
	if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		perror("Connection failed\n");
		close(sock_fd);
		return -1;
	}

	len = send(sock_fd, buf, len, 0);
	close(sock_fd);

	return 0;
}

static void cls_nl80211_csi_report(const uint8_t *data, size_t len)
{
	struct nlattr *tb[CLS_NL80211_ATTR_MAX + 1];
	struct cls_csi_report *csi_rpt = &csi_rpt_buf;
	uint8_t *pos = pkt_buffer, *rx_pos = (uint8_t *)&csi_rpt_buf;
	size_t pkt_len = 0, iq_len = 0;
	int i = 0, j = 0, n_subc = 0;
	uint16_t *uint16_iq = NULL;
	float f_iq;
	uint32_t ppdu_id = 0, frag_len = 0, frag_offset = 0;
	static uint32_t msg_len;
	uint8_t more_frag = 0;

	if (nla_parse(tb, CLS_NL80211_ATTR_MAX, (struct nlattr *) data, len, NULL) || !tb[CLS_NL80211_ATTR_CSI_REPORT] ||
		!tb[CLS_NL80211_ATTR_CSI_REPORT_PPDU_ID] || !tb[CLS_NL80211_ATTR_CSI_REPORT_MORE_FRAG] ||
		!tb[CLS_NL80211_ATTR_CSI_REPORT_FRAG_OFFSET])
		return;

	ppdu_id = nla_get_u32(tb[CLS_NL80211_ATTR_CSI_REPORT_PPDU_ID]);
	more_frag = nla_get_u8(tb[CLS_NL80211_ATTR_CSI_REPORT_MORE_FRAG]);
	frag_offset = nla_get_u32(tb[CLS_NL80211_ATTR_CSI_REPORT_FRAG_OFFSET]);
	frag_len = nla_get_u32(tb[CLS_NL80211_ATTR_CSI_REPORT_FRAG_LEN]);
	if (g_dbg >= CLSCSI_REPORTER_DBG_DBG)
		printf("ppdu_id=%d more_frag=%d frag_offset=%d frag_len=%d\n", ppdu_id, more_frag, frag_offset, frag_len);

	if (more_frag || frag_offset) {
		// fragments handling

		if (frag_offset == 0) {
			// this is the 1st frag
			msg_len = 0;
		}
		msg_len += frag_len;
		if (msg_len > sizeof(csi_rpt_buf)) {
			printf("ppdu_id=%d Message too long, %d, drop!\n", ppdu_id, msg_len);
			return;
		}
		memcpy(rx_pos + frag_offset, nla_data(tb[CLS_NL80211_ATTR_CSI_REPORT]), frag_len);
		if (more_frag) {
			// waiting for more frags...
			return;
		}
	} else {
		// No frag at all
		csi_rpt = nla_data(tb[CLS_NL80211_ATTR_CSI_REPORT]);
		msg_len = frag_len;
	}

	// Here, got full CSI report.

	iq_len = 4 * (csi_rpt->hdr.csi_length);
	n_subc = csi_rpt->info_aligned.info.subcarrier_num;

	// Basic validation
	if (msg_len != offsetof(struct cls_csi_report, csi_iq) + iq_len) {
		printf("Wrong message len %d for ppdu %d!\n", msg_len, ppdu_id);
		msg_len = 0;
		return;
	}

	if (ppdu_id != csi_rpt->hdr.ppdu_id) {
		printf("Mismatched ppdu_id: in attr is %d; in report is %d, drop!\n",
			ppdu_id, csi_rpt->hdr.ppdu_id);
		return;
	}

	if (g_dbg >= CLSCSI_REPORTER_DBG_INFO)
		printf(MACFMT " channel=%d ppdu_id=%u n_subc=%u\n", MACARG(csi_rpt->info_aligned.info.sta_mac_addr),
			csi_rpt->info_aligned.info.channel, csi_rpt->hdr.ppdu_id, n_subc);

	/* Frame format:
	 *	Daemon version +
	 *	Timestamp high +
	 *	Timestamp low +
	 *	Dest MAC address +
	 *	STA MAC address +
	 *	RSSI +
	 *	RX NSS +
	 *	TX NSS +
	 *	Channel +
	 *	BW +
	 *	Subcarrier num +
	 *	PPDU format +
	 *	LTF type +
	 *	AGC +
	 *	PPDU ID +
	 *	I/Q data len +
	 *	I/Q data format +
	 * New params should be added HERE
	 *	To-Do
	 *
	 * Make sure I/Q data at the bottom
	 *	I/Q data
	 */
	encap_csi_tlv_uint8(pos, CLS_CSI_RPT_VERSION, CLS_CSI_DAEMON_VERSION);
	encap_csi_tlv_uint32(pos, CLS_CSI_RPT_TS_HI, csi_rpt->info_aligned.info.timestamp_hi);
	encap_csi_tlv_uint32(pos, CLS_CSI_RPT_TS_LOW, csi_rpt->info_aligned.info.timestamp_lo);
	encap_csi_tlv_uint8_array(pos, CLS_CSI_RPT_DEST_MAC, 6, csi_rpt->info_aligned.info.dest_mac_addr);
	encap_csi_tlv_uint8_array(pos, CLS_CSI_RPT_STA_MAC, 6, csi_rpt->info_aligned.info.sta_mac_addr);
	encap_csi_tlv_int8(pos, CLS_CSI_RPT_RSSI, csi_rpt->info_aligned.info.rssi);
	encap_csi_tlv_uint8(pos, CLS_CSI_RPT_RX_NSS, csi_rpt->info_aligned.info.rx_nss);
	encap_csi_tlv_uint8(pos, CLS_CSI_RPT_TX_NSS, csi_rpt->info_aligned.info.tx_nss);
	encap_csi_tlv_uint8(pos, CLS_CSI_RPT_CHANNEL, csi_rpt->info_aligned.info.channel);
	encap_csi_tlv_uint8(pos, CLS_CSI_RPT_BW, csi_rpt->info_aligned.info.bw);
	encap_csi_tlv_uint16(pos, CLS_CSI_RPT_SUBC_NUM, csi_rpt->info_aligned.info.subcarrier_num);
	encap_csi_tlv_uint8(pos, CLS_CSI_RPT_PPDU_FMT, csi_rpt->info_aligned.info.ppdu_format);
	encap_csi_tlv_uint8(pos, CLS_CSI_RPT_LTF_TYPE, csi_rpt->info_aligned.info.ltf_type);
	encap_csi_tlv_uint8_array(pos, CLS_CSI_RPT_AGC, 8, csi_rpt->info_aligned.info.agc);

	encap_csi_tlv_uint16(pos, CLS_CSI_RPT_PPDU_ID, csi_rpt->hdr.ppdu_id);
	if (g_float_iq)
		iq_len *= 2;
	encap_csi_tlv_uint16(pos, CLS_CSI_RPT_IQ_LEN, iq_len);
	encap_csi_tlv_uint8(pos, CLS_CSI_RPT_IQ_FMT, g_float_iq);

	/* New params should be added HERE */

	/* Make sure I/Q data at the bottom */
	// set CSI I/Q type and len
	encap_csi_tl(pos, CLS_CSI_RPT_IQ_DATA, iq_len);

	// set CIS I/Q data
	/* From performance consideration, 'if (g_float_iq == 0)' is not optimized into for{} */
	if (g_float_iq == 0) {
		for (i = 0; i < n_subc; i++) {
			uint16_iq = &(csi_rpt->csi_iq.subc[i].h00_q);
			for (j = 0; j < 8; j++, uint16_iq++)
				encap_val_uint16(pos, *uint16_iq);
		}
	} else {
		for (i = 0; i < n_subc; i++) {
			uint16_iq = &(csi_rpt->csi_iq.subc[i].h00_q);
			for (j = 0; j < 8; j++, uint16_iq++) {
				iq_to_float(*uint16_iq, f_iq);
				encap_val_float(pos, f_iq);
			}
		}
	}

	pkt_len = pos - pkt_buffer;
	send_data_to_svr(pkt_buffer, pkt_len);
}

static void nl80211_vendor_event_cls(struct nlattr **tb)
{
	uint32_t vendor_id, subcmd;
	uint8_t *data = NULL;
	size_t len = 0;

	if (!tb[NL80211_ATTR_VENDOR_ID] || !tb[NL80211_ATTR_VENDOR_SUBCMD])
		return;

	vendor_id = nla_get_u32(tb[NL80211_ATTR_VENDOR_ID]);
	subcmd = nla_get_u32(tb[NL80211_ATTR_VENDOR_SUBCMD]);
	if (vendor_id != CLSEMI_OUI || subcmd != CLS_NL80211_CMD_REPORT_CSI)
		return;

	/* Handle CLS CSI Report */
	if (!tb[NL80211_ATTR_VENDOR_DATA]) {
		printf("Error: No vendor data found in CSI report!");
		return;
	}

	data = nla_data(tb[NL80211_ATTR_VENDOR_DATA]);
	len = nla_len(tb[NL80211_ATTR_VENDOR_DATA]);
	cls_nl80211_csi_report(data, len);
}

int nl80211_event_handler(struct nl_msg *msg, void *arg)
{
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	int ifidx = -1, wiphy_idx = -1;

	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
	genlmsg_attrlen(gnlh, 0), NULL);

	if (tb[NL80211_ATTR_IFINDEX])
		ifidx = nla_get_u32(tb[NL80211_ATTR_IFINDEX]);
	else if (tb[NL80211_ATTR_WIPHY])
		wiphy_idx = nla_get_u32(tb[NL80211_ATTR_WIPHY]);

	if (ifidx < 0 && wiphy_idx < 0)
		return NL_SKIP;

	if (gnlh->cmd != NL80211_CMD_VENDOR)
		return NL_SKIP;

	nl80211_vendor_event_cls(tb);

	return NL_SKIP;
}

static void usage(const char *program_name)
{
//	printf("Usage: %s -s <server_ip> [-f]\n", program_name);
	printf("Usage: %s -s <server_ip> [-d]\n", program_name);
	printf("\n");
	printf(" where:\n");
	printf("    '<server_ip>' IP address of CSI server\n");
//	printf("    '-f', to report CSI I/Q data in float format\n");
	printf("    '-d', show more debug logs\n");
	printf("\n");
}

static int process_options(int argc, char **argv)
{
	int opt;

	if (argc < 2) {
		usage(argv[0]);
		return -1;
	}

	while ((opt = getopt(argc, argv, "dfs:")) != -1) {
		switch (opt) {
		case 'd':
			g_dbg++;
			break;

		case 'f':
			g_float_iq = 1;
			break;

		case 's':
			strncpy(g_server_ip, optarg, sizeof(g_server_ip));
			printf("server ip: %s\n", g_server_ip);
			break;

		default:
			printf("Unknown option '%c'\n", opt);
			usage(argv[0]);
			return -1;
		}
	}

	return 0;
}

int main(int argc, char **argv)
{
	struct nl_sock *nl_sock = nl_socket_alloc();
	struct nl_cb *nl_cb = nl_cb_alloc(NL_CB_DEFAULT);
	int ret = 0, mid = -1;

	ret = process_options(argc, argv);
	if (ret)
		goto RETURN;

	nl_sock = nl_socket_alloc();
	if (!nl_sock) {
		perror("failed to nl_socket_alloc()\n");
		ret = -1;
		goto RETURN;
	}
	if (genl_connect(nl_sock)) {
		perror("failed to connect genl_connect\n");
		ret = -1;
		goto RETURN;
	}

	nl_socket_set_buffer_size(nl_sock, 262144, 8192);
	setsockopt(nl_socket_get_fd(nl_sock), SOL_NETLINK, NETLINK_EXT_ACK, &ret, sizeof(ret));

	/* Listen vendor event to monitor CSI report event */
	mid = _getNlMulticastId(nl_sock, "nl80211", "vendor");
	if (mid >= 0) {
		ret = nl_socket_add_membership(nl_sock, mid);
		if (ret)
			goto RETURN;
		printf("Listening vendor event ...\n");
	}

	nl_cb_set(nl_cb, NL_CB_SEQ_CHECK, NL_CB_CUSTOM, _eventNoSeqCheck, NULL);
	nl_cb_set(nl_cb, NL_CB_VALID, NL_CB_CUSTOM, nl80211_event_handler, NULL);

	while (1)
		nl_recvmsgs(nl_sock, nl_cb);

RETURN:
	nl_cb_put(nl_cb);
	if (nl_sock)
		nl_socket_free(nl_sock);

	return ret;
}


