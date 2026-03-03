/*
 * Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */


#include <limits.h>
#include <glob.h>
#include <fnmatch.h>
#include <stdarg.h>
#include <stdlib.h>
#include <net/if.h>
#include "nl80211_adapter.h"


static bool follow_up_cb;
static struct nl80211_state *nls = NULL;

static void nl80211_close(void)
{
	if (nls) {
		if (nls->nl80211)
			genl_family_put(nls->nl80211);

		if (nls->nl_sock)
			nl_socket_free(nls->nl_sock);

		if (nls->nl_cache)
			nl_cache_free(nls->nl_cache);

		free(nls);
		nls = NULL;
	}
}

static int nl80211_init(void)
{
	int err, fd;

	if (!nls) {
		nls = malloc(sizeof(struct nl80211_state));
		if (!nls) {
			err = -ENOMEM;
			goto err;
		}

		memset(nls, 0, sizeof(*nls));

		nls->nl_sock = nl_socket_alloc();
		if (!nls->nl_sock) {
			err = -ENOMEM;
			goto err;
		}

		if (genl_connect(nls->nl_sock)) {
			err = -ENOLINK;
			goto err;
		}

		fd = nl_socket_get_fd(nls->nl_sock);
		if (fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC) < 0) {
			err = -EINVAL;
			goto err;
		}

		if (genl_ctrl_alloc_cache(nls->nl_sock, &nls->nl_cache)) {
			err = -ENOMEM;
			goto err;
		}

		nls->nl80211 = genl_ctrl_search_by_name(nls->nl_cache, "nl80211");
		if (!nls->nl80211) {
			err = -ENOENT;
			goto err;
		}

		nls->nlctrl = genl_ctrl_search_by_name(nls->nl_cache, "nlctrl");
		if (!nls->nlctrl) {
			err = -ENOENT;
			goto err;
		}
	}

	return 0;

err:
	nl80211_close();
	return err;
}


static int nl80211_msg_error(struct sockaddr_nl *nla, struct nlmsgerr *err, void *arg)
{
	int *ret = arg;
	*ret = err->error;

	return NL_STOP;
}

static int nl80211_msg_finish(struct nl_msg *msg, void *arg)
{
	int *ret = arg;
	*ret = 0;

	return NL_SKIP;
}

static int nl80211_msg_ack(struct nl_msg *msg, void *arg)
{
	int *ret = arg;
	*ret = 0;

	return NL_STOP;
}

static int nl80211_msg_response(struct nl_msg *msg, void *arg)
{
	return NL_SKIP;
}

static void nl80211_free(struct nl80211_msg_conveyor *cv)
{
	if (cv) {
		if (cv->cb)
			nl_cb_put(cv->cb);

		if (cv->msg)
			nlmsg_free(cv->msg);

		cv->cb  = NULL;
		cv->msg = NULL;
	}
}

static struct nl80211_msg_conveyor * nl80211_new(struct genl_family *family, int cmd, int flags)
{
	static struct nl80211_msg_conveyor cv;
	struct nl_msg *req = NULL;
	struct nl_cb *cb = NULL;

	req = nlmsg_alloc();
	if (!req)
		goto err;

	cb = nl_cb_alloc(NL_CB_DEFAULT);
	if (!cb)
		goto err;

	genlmsg_put(req, 0, 0, genl_family_get_id(family), 0, flags, cmd, 0);
	cv.msg = req;
	cv.cb  = cb;

	return &cv;

err:
	DBG_ERROR("%s returns NULL\n", __func__);
	if (req)
		nlmsg_free(req);

	return NULL;
}


static struct nl80211_msg_conveyor * nl80211_msg(const char *devname, int cmd, int flags)
{
	int ifidx = -1, phyidx = -1;
	struct nl80211_msg_conveyor *cv;

	if (devname == NULL)
		return NULL;

	if (nl80211_init() < 0)
		return NULL;

	if (!strncmp(devname, "phy", 3))
		phyidx = atoi(&devname[3]);
	else
		ifidx = if_nametoindex(devname);

	/* Valid ifidx must be greater than 0 */
	if ((ifidx <= 0) && (phyidx < 0)) {
		DBG_ERROR("%s ifidx=%d phyidx=%d\n", __func__, ifidx, phyidx);
		return NULL;
	}

	cv = nl80211_new(nls->nl80211, cmd, flags);
	if (!cv)
		return NULL;

	if (ifidx > 0)
		NLA_PUT_U32(cv->msg, NL80211_ATTR_IFINDEX, ifidx);
	else if (phyidx > -1)
		NLA_PUT_U32(cv->msg, NL80211_ATTR_WIPHY, phyidx);

	return cv;

nla_put_failure:
	nl80211_free(cv);
	return NULL;
}

static int nl80211_send(struct nl80211_msg_conveyor *cv,
                        int (*cb_func)(struct nl_msg *, void *),
                        void *cb_arg)
{
	static struct nl80211_msg_conveyor rcv;
	int err;

	if (cb_func)
		nl_cb_set(cv->cb, NL_CB_VALID, NL_CB_CUSTOM, cb_func, cb_arg);
	else
		nl_cb_set(cv->cb, NL_CB_VALID, NL_CB_CUSTOM, nl80211_msg_response, &rcv);

	err = nl_send_auto_complete(nls->nl_sock, cv->msg);

	if (err < 0)
		goto out;

	err = 1;

	nl_cb_err(cv->cb,               NL_CB_CUSTOM, nl80211_msg_error,  &err);
	nl_cb_set(cv->cb, NL_CB_FINISH, NL_CB_CUSTOM, nl80211_msg_finish, &err);
	nl_cb_set(cv->cb, NL_CB_ACK,    NL_CB_CUSTOM, nl80211_msg_ack,    &err);

	while (err > 0)
		nl_recvmsgs(nls->nl_sock, cv->cb);

out:
	nl80211_free(cv);
	if (err)
		DBG_ERROR("%s return err=%d\n", __func__, err);
	return err;
}

static int nl80211_request(const char *devname, int cmd, int flags,
                           int (*cb_func)(struct nl_msg *, void *),
                           void *cb_arg)
{
	struct nl80211_msg_conveyor *cv;

	cv = nl80211_msg(devname, cmd, flags);
	if (!cv) {
		DBG_ERROR("%s nl80211_msg() returns NULL\n", __func__);
		return -ENOMEM;
	}

	return nl80211_send(cv, cb_func, cb_arg);
}

static struct nl80211_msg_conveyor *build_nl80211_request(const char *devname, int nl80211_cmd, int flags)
{
	struct nl80211_msg_conveyor *cv;

	cv = nl80211_msg(devname, nl80211_cmd, flags);
	if (!cv) {
		DBG_ERROR("%s nl80211_msg() returns NULL\n", __func__);
		return NULL;
	}

	return cv;
}


static struct nl80211_msg_conveyor *build_cls_nl80211_request(const char *devname, int cls_cmd, int flags)
{
	struct nl80211_msg_conveyor *cv;

	cv = nl80211_msg(devname, NL80211_CMD_VENDOR, flags);
	if (!cv) {
		DBG_ERROR("%s nl80211_msg() returns NULL\n", __func__);
		return NULL;
	}

	nla_put_u32(cv->msg, NL80211_ATTR_VENDOR_ID, CLSEMI_OUI);
	nla_put_u32(cv->msg, NL80211_ATTR_VENDOR_SUBCMD, cls_cmd);

	return cv;
}


static struct nlattr ** nl80211_parse(struct nl_msg *msg)
{
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	static struct nlattr *attr[NL80211_ATTR_MAX + 1];

	nla_parse(attr, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
	          genlmsg_attrlen(gnlh, 0), NULL);

	return attr;
}


static int nl80211_subscribe_cb(struct nl_msg *msg, void *arg)
{
	struct nl80211_group_conveyor *cv = arg;

	struct nlattr **attr = nl80211_parse(msg);
	struct nlattr *mgrpinfo[CTRL_ATTR_MCAST_GRP_MAX + 1];
	struct nlattr *mgrp;
	int mgrpidx;

	if (!attr[CTRL_ATTR_MCAST_GROUPS])
		return NL_SKIP;

	nla_for_each_nested(mgrp, attr[CTRL_ATTR_MCAST_GROUPS], mgrpidx) {
		nla_parse(mgrpinfo, CTRL_ATTR_MCAST_GRP_MAX, nla_data(mgrp), nla_len(mgrp), NULL);

		if (mgrpinfo[CTRL_ATTR_MCAST_GRP_ID] && mgrpinfo[CTRL_ATTR_MCAST_GRP_NAME] &&
			!strncmp(nla_data(mgrpinfo[CTRL_ATTR_MCAST_GRP_NAME]), cv->name,
				nla_len(mgrpinfo[CTRL_ATTR_MCAST_GRP_NAME]))) {
			cv->id = nla_get_u32(mgrpinfo[CTRL_ATTR_MCAST_GRP_ID]);
			break;
		}
	}

	return NL_SKIP;
}

static int nl80211_subscribe(const char *family, const char *group)
{
	struct nl80211_group_conveyor cv = { .name = group, .id = -ENOENT };
	struct nl80211_msg_conveyor *req;
	int err;

	if (nl80211_init() < 0)
		return -1;

	req = nl80211_new(nls->nlctrl, CTRL_CMD_GETFAMILY, 0);
	if (req) {
		NLA_PUT_STRING(req->msg, CTRL_ATTR_FAMILY_NAME, family);
		err = nl80211_send(req, nl80211_subscribe_cb, &cv);

		if (err)
			return err;

		return nl_socket_add_membership(nls->nl_sock, cv.id);

nla_put_failure:
		nl80211_free(req);
	}

	return -ENOMEM;
}

static int nl80211_wait_cb(struct nl_msg *msg, void *arg)
{
	struct nl80211_event_conveyor *cv = arg;
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));

	if (cv->wait[gnlh->cmd / 32] & (1 << (gnlh->cmd % 32)))
		cv->recv = gnlh->cmd;

	return NL_SKIP;
}

static int nl80211_wait_seq_check(struct nl_msg *msg, void *arg)
{
	return NL_OK;
}

static int __nl80211_wait(const char *family, const char *group, ...)
{
	struct nl80211_event_conveyor cv = { };
	struct nl_cb *cb;
	int err = 0;
	int cmd;
	va_list ap;

	if (nl80211_subscribe(family, group))
		return -ENOENT;

	cb = nl_cb_alloc(NL_CB_DEFAULT);

	if (!cb)
		return -ENOMEM;

	nl_cb_err(cb,                  NL_CB_CUSTOM, nl80211_msg_error,      &err);
	nl_cb_set(cb, NL_CB_SEQ_CHECK, NL_CB_CUSTOM, nl80211_wait_seq_check, NULL);
	nl_cb_set(cb, NL_CB_VALID,     NL_CB_CUSTOM, nl80211_wait_cb,        &cv );

	va_start(ap, group);

	for (cmd = va_arg(ap, int); cmd != 0; cmd = va_arg(ap, int))
		cv.wait[cmd / 32] |= (1 << (cmd % 32));

	va_end(ap);

	while (!cv.recv && !err)
		nl_recvmsgs(nls->nl_sock, cb);

	nl_cb_put(cb);

	return err;
}

#define nl80211_wait(family, group, ...) \
	__nl80211_wait(family, group, __VA_ARGS__, 0)

static inline struct nlattr ** cls_parse_nl80211_vendor(struct nl_msg *msg)
{
	struct nlattr **tb = nl80211_parse(msg);
	struct nlattr *nl_vendor;
	static struct nlattr *tb_vendor[CLS_NL80211_ATTR_MAX + 1];

	nl_vendor = tb[NL80211_ATTR_VENDOR_DATA];
	if (!nl_vendor) {
		DBG_ERROR("No VENDOR_ATTR found!\n");
		return NULL;
	}
	nla_parse(tb_vendor, CLS_NL80211_ATTR_MAX, nla_data(nl_vendor), nla_len(nl_vendor), NULL);

	return tb_vendor;
}


static int nl80211_get_cls_vendor_cb(struct nl_msg *msg, void *arg)
{
	struct nlattr **tb_vendor = cls_parse_nl80211_vendor(msg);
	struct nlattr *attr;
	struct attr_ret_arg *cls_arg = (struct attr_ret_arg *)arg;

	if (cls_arg->attr_id > CLS_NL80211_CMD_MAX || cls_arg->arg_len <= 0 || !cls_arg->arg) {
		DBG_ERROR("%s() invalid attr_ret_arg\n", __func__);
		cls_arg->ret = -CLSAPI_ERR_INVALID_PARAM;
		return NL_STOP;
	}

	if (!tb_vendor) {
		cls_arg->ret = -CLSAPI_ERR_INVALID_PARAM;
		return NL_STOP;
	}

	attr = tb_vendor[cls_arg->attr_id];
	if (!attr) {
		DBG_ERROR("nl80211: CLS attr %d couldn't be found\n", cls_arg->attr_id);
		cls_arg->ret = -CLSAPI_ERR_INVALID_PARAM;
		return NL_STOP;
	}
	if (cls_arg->arg_len != nla_len(attr)) {
		DBG_ERROR("arg_len(%d) != nla_len(%d) for cls attr %d\n", cls_arg->arg_len, nla_len(attr), cls_arg->attr_id);
		cls_arg->ret = -CLSAPI_ERR_INVALID_PARAM;
		return NL_STOP;
	}

	memset(cls_arg->arg, 0, cls_arg->arg_len);
	memcpy(cls_arg->arg, nla_data(attr), cls_arg->arg_len);
	cls_arg->ret = NL_OK;

	return NL_OK;
}


static int nl80211_get_attr_cb(struct nl_msg *msg, void *arg)
{
	struct nlattr **tb = nl80211_parse(msg);
	struct nlattr *attr;
	struct attr_ret_arg *attr_ret = (struct attr_ret_arg *)arg;

	if (!tb) {
		attr_ret->ret = -CLSAPI_ERR_NO_DATA;
		return NL_STOP;
	}

	attr = tb[attr_ret->attr_id];
	if (!attr) {
		DBG_ERROR("nl80211: attr %d couldn't be found\n", attr_ret->attr_id);
		attr_ret->ret = -CLSAPI_ERR_INVALID_PARAM;
		return NL_STOP;
	}

	if (attr_ret->arg_len < nla_len(attr)) {
		DBG_ERROR("arg_len(%d) < nla_len(%d) for attr %d\n", attr_ret->arg_len, nla_len(attr), attr_ret->attr_id);
		attr_ret->ret = -CLSAPI_ERR_INVALID_PARAM;
		return NL_STOP;
	}

	memset(attr_ret->arg, 0, attr_ret->arg_len);
	memcpy(attr_ret->arg, nla_data(attr), nla_len(attr));
	attr_ret->ret = CLSAPI_OK;

	return NL_OK;
}


static inline void set_attr_arg(struct attr_ret_arg *attr_ret, const uint16_t attr_id,
		void *arg, const uint16_t arg_len)
{
	attr_ret->attr_id = attr_id;
	attr_ret->arg = arg;
	attr_ret->arg_len = arg_len;
}

static void get_sta_tid_stats(struct sta_info *sinfo,
			      struct nlattr *attr)
{
	struct nlattr *tid_stats[NL80211_TID_STATS_MAX + 1], *tidattr;
	struct nlattr *txq_stats[NL80211_TXQ_STATS_MAX + 1];
	static struct nla_policy txq_stats_policy[NL80211_TXQ_STATS_MAX + 1] = {
		[NL80211_TXQ_STATS_BACKLOG_BYTES] = { .type = NLA_U32 },
		[NL80211_TXQ_STATS_BACKLOG_PACKETS] = { .type = NLA_U32 },
	};
	int rem;

	nla_for_each_nested(tidattr, attr, rem) {
		if (nla_parse_nested(tid_stats, NL80211_TID_STATS_MAX,
				     tidattr, NULL) != 0 ||
		    !tid_stats[NL80211_TID_STATS_TXQ_STATS] ||
		    nla_parse_nested(txq_stats, NL80211_TXQ_STATS_MAX,
				     tid_stats[NL80211_TID_STATS_TXQ_STATS],
				     txq_stats_policy) != 0)
			continue;
		/* sum the backlogs over all TIDs for station */
		if (txq_stats[NL80211_TXQ_STATS_BACKLOG_BYTES])
			sinfo->backlog_bytes += nla_get_u32(
				txq_stats[NL80211_TXQ_STATS_BACKLOG_BYTES]);
		if (txq_stats[NL80211_TXQ_STATS_BACKLOG_PACKETS])
			sinfo->backlog_bytes += nla_get_u32(
				txq_stats[NL80211_TXQ_STATS_BACKLOG_PACKETS]);
	}
}

static int nl80211_get_sta_list_cb(struct nl_msg *msg, void *arg)
{
	struct nlattr **tb = nl80211_parse(msg);
	struct cb_arg_ret *cb_arg = (struct cb_arg_ret *)arg;
	int max_len = *(int *)(cb_arg->arg2);
	int copied_len = *(int *)(cb_arg->arg3);
	uint8_t *sta_mac;

	cb_arg->ret = NL_OK;

	if (!tb[NL80211_ATTR_MAC]) {
		DBG_ERROR("sta MAC address missing!");
		cb_arg->ret = NL_SKIP;
		goto ret;
	}

	if (copied_len < max_len) {
		sta_mac = cb_arg->arg1 + copied_len * ETH_ALEN;
		memcpy(sta_mac, nla_data(tb[NL80211_ATTR_MAC]), ETH_ALEN);
		(*(int *)(cb_arg->arg3))++;
	}

ret:
	return cb_arg->ret;
}

int nl80211_get_assoc_list(const char *ifname, const uint8_t (*sta_macs)[ETH_ALEN], int *list_len)
{
	struct nl80211_msg_conveyor *cv;
	struct cb_arg_ret cb_arg = {0};
	int ret = CLSAPI_OK;
	int copied_cnt = 0;

	cv = build_nl80211_request(ifname, NL80211_CMD_GET_STATION, NLM_F_DUMP);
	if (!cv)
		return -CLSAPI_ERR_NL80211;

	cb_arg.arg1 = (uint8_t *)sta_macs;
	cb_arg.arg2 = (uint8_t *)list_len;
	cb_arg.arg3 = (uint8_t *)&copied_cnt;
	ret = nl80211_send(cv, nl80211_get_sta_list_cb, &cb_arg);
	if (ret)
		return ret;

	*list_len = copied_cnt;

	return cb_arg.ret;
}

void parse_bitrate(struct nlattr *bitrate_attr, struct sta_info *sinfo, bool tx)
{
	struct nlattr *rinfo[NL80211_RATE_INFO_MAX + 1];
	static struct nla_policy rate_policy[NL80211_RATE_INFO_MAX + 1] = {
		[NL80211_RATE_INFO_BITRATE] = { .type = NLA_U16 },
		[NL80211_RATE_INFO_BITRATE32] = { .type = NLA_U32 },
		[NL80211_RATE_INFO_MCS] = { .type = NLA_U8 },
		[NL80211_RATE_INFO_40_MHZ_WIDTH] = { .type = NLA_FLAG },
		[NL80211_RATE_INFO_SHORT_GI] = { .type = NLA_FLAG },
	};

	if (nla_parse_nested(rinfo, NL80211_RATE_INFO_MAX,
				bitrate_attr, rate_policy)) {
		printf("failed to parse nested rate attributes!");
		return;
	}

	if (rinfo[NL80211_RATE_INFO_BITRATE32]) {
		if (tx)
			sinfo->current_tx_rate = nla_get_u32(rinfo[NL80211_RATE_INFO_BITRATE32]);
		else
			sinfo->current_rx_rate = nla_get_u32(rinfo[NL80211_RATE_INFO_BITRATE32]);
	} else if (rinfo[NL80211_RATE_INFO_BITRATE]) {
		if (tx)
			sinfo->current_tx_rate = nla_get_u16(rinfo[NL80211_RATE_INFO_BITRATE]);
		else
			sinfo->current_rx_rate = nla_get_u16(rinfo[NL80211_RATE_INFO_BITRATE]);
	}

	if (rinfo[NL80211_RATE_INFO_MCS]) {
		if (tx) {
			sinfo->tx_mcs = nla_get_u8(rinfo[NL80211_RATE_INFO_MCS]);
			sinfo->flags |= STA_DRV_DATA_TX_MCS;
		} else {
			sinfo->rx_mcs = nla_get_u8(rinfo[NL80211_RATE_INFO_MCS]);
			sinfo->flags |= STA_DRV_DATA_RX_MCS;
		}
	}

	if (rinfo[NL80211_RATE_INFO_VHT_MCS]) {
		if (tx) {
			sinfo->tx_vhtmcs = nla_get_u8(rinfo[NL80211_RATE_INFO_VHT_MCS]);
			sinfo->flags |= STA_DRV_DATA_TX_VHT_MCS;
		} else {
			sinfo->rx_vhtmcs = nla_get_u8(rinfo[NL80211_RATE_INFO_VHT_MCS]);
			sinfo->flags |= STA_DRV_DATA_RX_VHT_MCS;
		}
	}

	if (rinfo[NL80211_RATE_INFO_HE_MCS]) {
		if (tx) {
			sinfo->tx_he_mcs = nla_get_u8(rinfo[NL80211_RATE_INFO_HE_MCS]);
			sinfo->flags |= STA_DRV_DATA_TX_HE_MCS;
		} else {
			sinfo->rx_he_mcs = nla_get_u8(rinfo[NL80211_RATE_INFO_HE_MCS]);
			sinfo->flags |= STA_DRV_DATA_RX_HE_MCS;
		}
	}

	if (rinfo[NL80211_RATE_INFO_40_MHZ_WIDTH])
		sinfo->bandwidth = CLSAPI_WIFI_BW_40;
	if (rinfo[NL80211_RATE_INFO_80_MHZ_WIDTH])
		sinfo->bandwidth = CLSAPI_WIFI_BW_80;
	if (rinfo[NL80211_RATE_INFO_80P80_MHZ_WIDTH])
		sinfo->bandwidth = CLSAPI_WIFI_BW_80_80;
	if (rinfo[NL80211_RATE_INFO_160_MHZ_WIDTH])
		sinfo->bandwidth = CLSAPI_WIFI_BW_160;

	if (rinfo[NL80211_RATE_INFO_VHT_NSS]) {
		if (tx) {
			sinfo->tx_vht_nss = nla_get_u8(rinfo[NL80211_RATE_INFO_VHT_NSS]);
			sinfo->flags |= STA_DRV_DATA_TX_VHT_NSS;
		} else {
			sinfo->rx_vht_nss = nla_get_u8(rinfo[NL80211_RATE_INFO_VHT_NSS]);
			sinfo->flags |= STA_DRV_DATA_RX_VHT_NSS;
		}
	}

	if (rinfo[NL80211_RATE_INFO_HE_NSS]) {
		if (tx) {
			sinfo->tx_he_nss = nla_get_u8(rinfo[NL80211_RATE_INFO_HE_NSS]);
			sinfo->flags |= STA_DRV_DATA_TX_HE_NSS;
		} else {
			sinfo->rx_he_nss = nla_get_u8(rinfo[NL80211_RATE_INFO_HE_NSS]);
			sinfo->flags |= STA_DRV_DATA_RX_HE_NSS;
		}
	}
	if (rinfo[NL80211_RATE_INFO_SHORT_GI]) {
		if (tx)
			sinfo->flags |= STA_DRV_DATA_TX_SHORT_GI;
		else
			sinfo->flags |= STA_DRV_DATA_RX_SHORT_GI;
	}
#if 0
	if (rinfo[NL80211_RATE_INFO_HE_GI])
		if (rinfo[NL80211_RATE_INFO_HE_DCM])
			if (rinfo[NL80211_RATE_INFO_HE_RU_ALLOC])
#endif
}


static int nl80211_get_sta_info_cb(struct nl_msg *msg, void *arg)
{
	struct nlattr **tb = nl80211_parse(msg);
	struct cb_arg_ret *cb_arg = (struct cb_arg_ret *)arg;
	struct sta_info *sinfo = (struct sta_info *)(cb_arg->arg1);

	struct nlattr *stats[NL80211_STA_INFO_MAX + 1];
	static struct nla_policy stats_policy[NL80211_STA_INFO_MAX + 1] = {
		[NL80211_STA_INFO_INACTIVE_TIME] = { .type = NLA_U32 },
		[NL80211_STA_INFO_RX_BYTES] = { .type = NLA_U32 },
		[NL80211_STA_INFO_TX_BYTES] = { .type = NLA_U32 },
		[NL80211_STA_INFO_RX_PACKETS] = { .type = NLA_U32 },
		[NL80211_STA_INFO_TX_PACKETS] = { .type = NLA_U32 },
		[NL80211_STA_INFO_TX_FAILED] = { .type = NLA_U32 },
		[NL80211_STA_INFO_RX_BYTES64] = { .type = NLA_U64 },
		[NL80211_STA_INFO_TX_BYTES64] = { .type = NLA_U64 },
		[NL80211_STA_INFO_SIGNAL] = { .type = NLA_U8 },
		[NL80211_STA_INFO_ACK_SIGNAL] = { .type = NLA_U8 },
		[NL80211_STA_INFO_RX_DURATION] = { .type = NLA_U64 },
		[NL80211_STA_INFO_TX_DURATION] = { .type = NLA_U64 },
		[NL80211_STA_INFO_CONNECTED_TIME] = { .type = NLA_U32 },
	};

	if (!tb[NL80211_ATTR_MAC]) {
		DBG_ERROR("sta MAC address missing!");
		cb_arg->ret = NL_SKIP;
		goto ret;
	}
	if (!tb[NL80211_ATTR_STA_INFO]) {
		DBG_ERROR("sta stats missing!");
		cb_arg->ret = NL_SKIP;
		goto ret;
	}
	if (nla_parse_nested(stats, NL80211_STA_INFO_MAX,
			     tb[NL80211_ATTR_STA_INFO],
			     stats_policy)) {
		DBG_ERROR("failed to parse nested attributes!");
		cb_arg->ret = NL_SKIP;
		goto ret;
	}

	memcpy(sinfo->mac, nla_data(tb[NL80211_ATTR_MAC]), ETH_ALEN);

	if (stats[NL80211_STA_INFO_INACTIVE_TIME])
		sinfo->inactive_msec =
			nla_get_u32(stats[NL80211_STA_INFO_INACTIVE_TIME]);
	/* For backwards compatibility, fetch the 32-bit counters first. */
	if (stats[NL80211_STA_INFO_RX_BYTES])
		sinfo->rx_bytes = nla_get_u32(stats[NL80211_STA_INFO_RX_BYTES]);
	if (stats[NL80211_STA_INFO_TX_BYTES])
		sinfo->tx_bytes = nla_get_u32(stats[NL80211_STA_INFO_TX_BYTES]);
	if (stats[NL80211_STA_INFO_RX_BYTES64] &&
	    stats[NL80211_STA_INFO_TX_BYTES64]) {
		/*
		 * The driver supports 64-bit counters, so use them to override
		 * the 32-bit values.
		 */
		sinfo->rx_bytes =
			nla_get_u64(stats[NL80211_STA_INFO_RX_BYTES64]);
		sinfo->tx_bytes =
			nla_get_u64(stats[NL80211_STA_INFO_TX_BYTES64]);
		sinfo->bytes_64bit = 1;
	}
	if (stats[NL80211_STA_INFO_RX_PACKETS])
		sinfo->rx_packets =
			nla_get_u32(stats[NL80211_STA_INFO_RX_PACKETS]);
	if (stats[NL80211_STA_INFO_TX_PACKETS])
		sinfo->tx_packets =
			nla_get_u32(stats[NL80211_STA_INFO_TX_PACKETS]);
	if (stats[NL80211_STA_INFO_RX_DURATION])
		sinfo->rx_airtime =
			nla_get_u64(stats[NL80211_STA_INFO_RX_DURATION]);
	if (stats[NL80211_STA_INFO_TX_DURATION])
		sinfo->tx_airtime =
			nla_get_u64(stats[NL80211_STA_INFO_TX_DURATION]);
	if (stats[NL80211_STA_INFO_TX_FAILED])
		sinfo->tx_retry_failed =
			nla_get_u32(stats[NL80211_STA_INFO_TX_FAILED]);
	if (stats[NL80211_STA_INFO_SIGNAL])
		sinfo->signal = nla_get_u8(stats[NL80211_STA_INFO_SIGNAL]);
	if (stats[NL80211_STA_INFO_ACK_SIGNAL]) {
		sinfo->last_ack_rssi =
			nla_get_u8(stats[NL80211_STA_INFO_ACK_SIGNAL]);
		sinfo->flags |= STA_DRV_DATA_LAST_ACK_RSSI;
	}

	if (stats[NL80211_STA_INFO_CONNECTED_TIME]) {
		sinfo->connected_sec =
			nla_get_u32(stats[NL80211_STA_INFO_CONNECTED_TIME]);
		sinfo->flags |= STA_DRV_DATA_CONN_TIME;
	}

	//tx = true
	if (stats[NL80211_STA_INFO_TX_BITRATE])
		parse_bitrate(stats[NL80211_STA_INFO_TX_BITRATE], sinfo, true);

	//rx = false
	if (stats[NL80211_STA_INFO_RX_BITRATE])
		parse_bitrate(stats[NL80211_STA_INFO_RX_BITRATE], sinfo, false);

	if (stats[NL80211_STA_INFO_TID_STATS])
		get_sta_tid_stats(sinfo, stats[NL80211_STA_INFO_TID_STATS]);

	cb_arg->ret = NL_OK;
ret:
	return cb_arg->ret;
}


int nl80211_get_assoc_info(const char *ifname, const uint8_t sta_mac[ETH_ALEN], struct sta_info *sinfo)
{
	struct nl80211_msg_conveyor *cv;
	struct cb_arg_ret cb_arg;
	int ret = CLSAPI_OK;

	/* Get station info */
	cv = build_nl80211_request(ifname, NL80211_CMD_GET_STATION, 0);
	if (!cv)
		return -CLSAPI_ERR_NL80211;
	nla_put(cv->msg, NL80211_ATTR_MAC, ETH_ALEN, sta_mac);
	cb_arg.arg1 = (uint8_t *)sinfo;
	ret = nl80211_send(cv, nl80211_get_sta_info_cb, &cb_arg);
	if (ret)
		return ret;

	return cb_arg.ret;
}

/**
 * Get value of standard NL80211 attribute (not nested attribute).
 */
int nl80211_get_attr_value(const char *ifname, const enum nl80211_commands cmd,
	const struct nl80211_attr_tbl *put_attrs, const enum nl80211_attrs attr_id,
	void *buf, const uint16_t buf_len)
{
	int ret;
	struct nl80211_msg_conveyor *cv;
	struct attr_ret_arg attr_ret;

	cv = build_nl80211_request(ifname, cmd, 0);
	if (!cv)
		return -CLSAPI_ERR_NL80211;

	/* input attributes */
	if (put_attrs) {
		int i;
		struct nl80211_attr_id_value *put_attr = put_attrs->attrs;
		for (i = 0; i < put_attrs->n_attr; i++, put_attr++)
			nla_put(cv->msg, put_attr->id, put_attr->val_len, put_attr->value);
	}

	set_attr_arg(&attr_ret, attr_id, buf, buf_len);
	ret = nl80211_send(cv, nl80211_get_attr_cb, &attr_ret);
	if (ret || attr_ret.ret)
		return -CLSAPI_ERR_NL80211;

	return CLSAPI_OK;
}

/**
 * Set value of standard NL80211 attribute (not nested attribute).
 */
int nl80211_set_attr_value(const char *ifname, const enum nl80211_commands cmd,
	const struct nl80211_attr_tbl *put_attrs)
{
	struct nl80211_msg_conveyor *cv;

	cv = build_nl80211_request(ifname, cmd, 0);
	if (!cv)
		return -CLSAPI_ERR_NL80211;

	/* input attributes */
	if (put_attrs) {
		int i;
		struct nl80211_attr_id_value *put_attr = put_attrs->attrs;
		for (i = 0; i < put_attrs->n_attr; i++, put_attr++)
			nla_put(cv->msg, put_attr->id, put_attr->val_len, put_attr->value);
	}

	return nl80211_send(cv, NULL, NULL);
}

/**
 * Get value of CLS Vendor NL80211 attribute.
 */
int nl80211_get_cls_attr_value(const char *ifname, const enum cls_nl80211_vendor_subcmds cls_subcmd,
	const struct nl80211_attr_tbl *put_attrs, const enum cls_nl80211_vendor_attr cls_attr_id,
	void *buf, const uint16_t buf_len)
{
	int ret;
	struct nlattr *params;
	struct nl80211_msg_conveyor *cv;
	struct attr_ret_arg cls_arg;

	cv = build_cls_nl80211_request(ifname, cls_subcmd, 0);
	if (!cv)
		return -CLSAPI_ERR_NL80211;

	params = nla_nest_start(cv->msg, NL80211_ATTR_VENDOR_DATA);
	if (!params) {
		nlmsg_free(cv->msg);
		return -CLSAPI_ERR_NL80211;
	}
	/* input attributes */
	if (put_attrs) {
		int i;
		struct nl80211_attr_id_value *put_attr = put_attrs->attrs;
		for (i = 0; i < put_attrs->n_attr; i++, put_attr++)
			nla_put(cv->msg, put_attr->id, put_attr->val_len, put_attr->value);
	}
	nla_nest_end(cv->msg, params);

	set_attr_arg(&cls_arg, cls_attr_id, buf, buf_len);
	ret = nl80211_send(cv, nl80211_get_cls_vendor_cb, &cls_arg);
	if (ret || cls_arg.ret)
		return -CLSAPI_ERR_NL80211;

	return CLSAPI_OK;
}

/**
 * Set value of CLS Vendor NL80211 attribute.
 */
int nl80211_set_cls_attr_value(const char *ifname, const enum cls_nl80211_vendor_subcmds cls_subcmd,
	const struct nl80211_attr_tbl *put_attrs)
{
	struct nlattr *params;
	struct nl80211_msg_conveyor *cv;

	cv = build_cls_nl80211_request(ifname, cls_subcmd, 0);
	if (!cv)
		return -CLSAPI_ERR_NL80211;

	params = nla_nest_start(cv->msg, NL80211_ATTR_VENDOR_DATA);
	if (!params) {
		nlmsg_free(cv->msg);
		return -CLSAPI_ERR_NO_MEM;
	}
	/* input attributes */
	if (put_attrs) {
		int i;
		struct nl80211_attr_id_value *put_attr = put_attrs->attrs;
		for (i = 0; i < put_attrs->n_attr; i++, put_attr++)
			nla_put(cv->msg, put_attr->id, put_attr->val_len, put_attr->value);
	}
	nla_nest_end(cv->msg, params);

	return nl80211_send(cv, NULL, NULL);

}

static int nl80211_get_chan_list_cb(struct nl_msg *msg, void *arg)
{
	struct cb_arg_ret *cb_arg = (struct cb_arg_ret *)arg;
	enum clsapi_wifi_band tgt_band = *(enum clsapi_wifi_band *)(cb_arg->arg1);
	enum clsapi_wifi_bw bw = *(enum clsapi_wifi_bw *)(cb_arg->arg4);
	uint8_t *channel_list = cb_arg->arg2;
	uint8_t *write_idx = cb_arg->arg3;

	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];
	struct nlattr *tb_band[NL80211_BAND_ATTR_MAX + 1];
	struct nlattr *tb_freq[NL80211_FREQUENCY_ATTR_MAX + 1];
	static struct nla_policy freq_policy[NL80211_FREQUENCY_ATTR_MAX + 1] = {
		[NL80211_FREQUENCY_ATTR_FREQ] = { .type = NLA_U32 },
		[NL80211_FREQUENCY_ATTR_DISABLED] = { .type = NLA_FLAG },
		[NL80211_FREQUENCY_ATTR_NO_IR] = { .type = NLA_FLAG },
		[__NL80211_FREQUENCY_ATTR_NO_IBSS] = { .type = NLA_FLAG },
	};

	struct nlattr *nl_band;
	struct nlattr *nl_freq;
	int rem_band, rem_freq;

	// one call NL80211_CMD_GET_WIPHY will result in splitted multiple response, so this cb will be
	// called multiple times.
	static uint8_t max_len;

	if (follow_up_cb == false) {
		// this is the first time the cb is called. record max_len and reset write_idx to 0.
		follow_up_cb = true;
		max_len = *(cb_arg->arg3);
		*write_idx = 0;
	}

	nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);
	/* needed for split dump */
	if (tb_msg[NL80211_ATTR_WIPHY_BANDS]) {
		nla_for_each_nested(nl_band, tb_msg[NL80211_ATTR_WIPHY_BANDS], rem_band) {
			if (tgt_band != nl_band->nla_type)
				continue;

			nla_parse(tb_band, NL80211_BAND_ATTR_MAX, nla_data(nl_band), nla_len(nl_band), NULL);
			if (tb_band[NL80211_BAND_ATTR_FREQS]) {
				nla_for_each_nested(nl_freq, tb_band[NL80211_BAND_ATTR_FREQS], rem_freq) {
					nla_parse(tb_freq, NL80211_FREQUENCY_ATTR_MAX, nla_data(nl_freq), nla_len(nl_freq), freq_policy);
					if (!tb_freq[NL80211_FREQUENCY_ATTR_FREQ])
						continue;

					if (tb_freq[NL80211_FREQUENCY_ATTR_DISABLED] || tb_freq[NL80211_FREQUENCY_ATTR_NO_IR] ||
						tb_freq[__NL80211_FREQUENCY_ATTR_NO_IBSS])
						continue;

					if (tb_freq[NL80211_FREQUENCY_ATTR_NO_20MHZ] &&
						(bw == CLSAPI_WIFI_BW_20 || bw == CLSAPI_WIFI_BW_20_NOHT))
						continue;
					if (tb_freq[NL80211_FREQUENCY_ATTR_NO_HT40_MINUS] &&
						tb_freq[NL80211_FREQUENCY_ATTR_NO_HT40_PLUS] && bw == CLSAPI_WIFI_BW_40)
						continue;
					if (tb_freq[NL80211_FREQUENCY_ATTR_NO_80MHZ] && bw == CLSAPI_WIFI_BW_80)
						continue;
					if (tb_freq[NL80211_FREQUENCY_ATTR_NO_160MHZ] && bw == CLSAPI_WIFI_BW_160)
						continue;
					if (nla_get_u32(tb_freq[NL80211_FREQUENCY_ATTR_FREQ]) == 5160 && bw != CLSAPI_WIFI_BW_20)
						continue;

					if (*write_idx < max_len) {
						channel_list[*write_idx] = freq_mhz_to_channel(nla_get_u32(tb_freq[NL80211_FREQUENCY_ATTR_FREQ]));
						(*write_idx)++;
					}
				}
			}
		}
	}

	cb_arg->ret = NL_OK;

	return cb_arg->ret;
}

int nl80211_get_chan_list(const char *ifname, const enum clsapi_wifi_band band, const enum clsapi_wifi_bw bw,
		uint8_t *channels, uint8_t *channels_len)
{
	struct nl80211_msg_conveyor *cv;
	struct cb_arg_ret cb_arg;
	int ret = CLSAPI_OK;

	follow_up_cb = false;
	cv = build_nl80211_request(ifname, NL80211_CMD_GET_WIPHY, NLM_F_DUMP);
	if (!cv)
		return -CLSAPI_ERR_NL80211;

	nla_put_flag(cv->msg, NL80211_ATTR_SPLIT_WIPHY_DUMP);

	cb_arg.arg1 = (uint8_t *)&band;
	cb_arg.arg2 = (uint8_t *)channels;
	cb_arg.arg3 = (uint8_t *)channels_len;
	cb_arg.arg4 = (uint8_t *)&bw;
	ret = nl80211_send(cv, nl80211_get_chan_list_cb, &cb_arg);

	if (ret || cb_arg.ret)
		return -CLSAPI_ERR_NL80211;

	return CLSAPI_OK;
}

static int set_scan_status(const char *ifname, const char *status)
{
	FILE *scan_status_fh = NULL;
	string_128 scan_status_file = {0};

	snprintf(scan_status_file, sizeof(scan_status_file), "%s%s", CLSAPI_WIFI_SCAN_STATUS_FILE, ifname);

	scan_status_fh = fopen(scan_status_file, "w+");
	if (scan_status_fh == NULL) {
		DBG_ERROR("Failed in open %s\n", scan_status_file);
		return -CLSAPI_ERR_FILE_OPERATION;
	}

	fputs(status, scan_status_fh);
	fclose(scan_status_fh);

	return CLSAPI_OK;
}

static int clear_scan_result(const char *ifname)
{
	FILE *scan_result_fh = NULL;
	string_128 scan_result_file = {0};

	snprintf(scan_result_file, sizeof(scan_result_file), "%s%s", CLSAPI_WIFI_SCAN_RESULT_FILE, ifname);

	scan_result_fh = fopen(scan_result_file, "w");
	if (scan_result_fh == NULL) {
		DBG_ERROR("Failed in open %s\n", scan_result_file);
		return -CLSAPI_ERR_FILE_OPERATION;
	}

	fclose(scan_result_fh);

	return CLSAPI_OK;
}

static int append_scan_result(const char *ifname, const uint8_t *buffer, const int buf_len)
{
	FILE *scan_result_fh = NULL;
	string_128 scan_result_file = {0};

	snprintf(scan_result_file, sizeof(scan_result_file), "%s%s", CLSAPI_WIFI_SCAN_RESULT_FILE, ifname);

	scan_result_fh = fopen(scan_result_file, "a+");
	if (scan_result_fh == NULL) {
		DBG_ERROR("Failed in open %s\n", scan_result_file);
		return -CLSAPI_ERR_FILE_OPERATION;
	}

	fwrite(buffer, 1, buf_len, scan_result_fh);
	fclose(scan_result_fh);

	return CLSAPI_OK;
}

static void parse_rsn_cipher(uint8_t idx, uint32_t *ciphers)
{
	switch (idx) {
	case 0:  /* NONE */
		*ciphers |= CLSAPI_WIFI_CIPHER_NONE;
		break;

	case 1:  /* WEP40 */
		*ciphers |= CLSAPI_WIFI_CIPHER_WEP40;
		break;

	case 2:  /* TKIP */
		*ciphers |= CLSAPI_WIFI_CIPHER_TKIP;
		break;

	case 4:  /* CCMP-128 */
		*ciphers |= CLSAPI_WIFI_CIPHER_CCMP;
		break;

	case 5:  /* WEP104 */
		*ciphers |= CLSAPI_WIFI_CIPHER_WEP104;
		break;

	case 8:  /* GCMP-128 */
		*ciphers |= CLSAPI_WIFI_CIPHER_GCMP;
		break;

	case 9:  /* GCMP-256 */
		*ciphers |= CLSAPI_WIFI_CIPHER_GCMP256;
		break;

	case 10:  /* CCMP-256 */
		*ciphers |= CLSAPI_WIFI_CIPHER_CCMP256;
		break;

	case 3:  /* WRAP */
	case 6:  /* AES-128-CMAC */
	case 7:  /* No group addressed */
	case 11: /* BIP-GMAC-128 */
	case 12: /* BIP-GMAC-256 */
	case 13: /* BIP-CMAC-256 */
		break;
	}
}

void parse_rsn_ie(struct clsapi_wifi_sanned_crypto *crypto, uint8_t *data, uint8_t len, uint16_t defcipher, uint8_t defauth)
{
	uint16_t i, count;
	uint8_t wpa_version = 0;

	static uint8_t ms_oui[3]        = {0x00, 0x50, 0xf2};
	static uint8_t ieee80211_oui[3] = {0x00, 0x0f, 0xac};

	data += 2;
	len -= 2;

	if (!memcmp(data, ms_oui, 3))
		wpa_version |= CLSAPI_WIFI_SEC_PROTO_WPA; // WPA
	else if (!memcmp(data, ieee80211_oui, 3))
		wpa_version |= CLSAPI_WIFI_SEC_PROTO_WPA2; // WPA2

	if (len < 4) {
		crypto->group_ciphers |= defcipher;
		crypto->pair_ciphers  |= defcipher;
		crypto->auth_suites   |= defauth;
		return;
	}

	if (!memcmp(data, ms_oui, 3) || !memcmp(data, ieee80211_oui, 3))
		parse_rsn_cipher(data[3], &crypto->group_ciphers);

	data += 4;
	len -= 4;

	if (len < 2) {
		crypto->pair_ciphers |= defcipher;
		crypto->auth_suites  |= defauth;
		return;
	}

	count = data[0] | (data[1] << 8);
	if (2 + (count * 4) > len)
		return;

	for (i = 0; i < count; i++)
		if (!memcmp(data + 2 + (i * 4), ms_oui, 3) || !memcmp(data + 2 + (i * 4), ieee80211_oui, 3))
			parse_rsn_cipher(data[2 + (i * 4) + 3], &crypto->pair_ciphers);

	data += 2 + (count * 4);
	len -= 2 + (count * 4);

	if (len < 2) {
		crypto->auth_suites |= defauth;
		return;
	}

	count = data[0] | (data[1] << 8);
	if (2 + (count * 4) > len)
		return;

	for (i = 0; i < count; i++) {
		if (!memcmp(data + 2 + (i * 4), ms_oui, 3) || !memcmp(data + 2 + (i * 4), ieee80211_oui, 3)) {
			switch (data[2 + (i * 4) + 3]) {
			case 1:  /* IEEE 802.1x */
				crypto->sec_protos |= wpa_version;
				crypto->auth_suites |= CLSAPI_WIFI_KEY_MGMT_8021X;
				break;

			case 2:  /* PSK */
				crypto->sec_protos |= wpa_version;
				crypto->auth_suites |= CLSAPI_WIFI_KEY_MGMT_PSK;
				break;

			case 8:  /* SAE */
				crypto->sec_protos |= CLSAPI_WIFI_SEC_PROTO_WPA3;
				crypto->auth_suites |= CLSAPI_WIFI_KEY_MGMT_SAE;
				break;

			case 11: /* 802.1x Suite-B */
			case 12: /* 802.1x Suite-B-192 */
				crypto->sec_protos |= CLSAPI_WIFI_SEC_PROTO_WPA3;
				crypto->auth_suites |= CLSAPI_WIFI_KEY_MGMT_8021X;
				break;

			case 18: /* OWE */
				crypto->sec_protos |= CLSAPI_WIFI_SEC_PROTO_OWE;
				crypto->auth_suites |= CLSAPI_WIFI_KEY_MGMT_OWE;
				break;

			case 3:  /* FT/IEEE 802.1X */
			case 4:  /* FT/PSK */
			case 5:  /* IEEE 802.1X/SHA-256 */
			case 6:  /* PSK/SHA-256 */
			case 7:  /* TPK Handshake */
			case 9:  /* FT/SAE */
			case 10: /* undefined */
			case 13: /* FT/802.1x SHA-384 */
			case 14: /* FILS SHA-256 */
			case 15: /* FILS SHA-384 */
			case 16: /* FT/FILS SHA-256 */
			case 17: /* FT/FILS SHA-384 */
				break;
			}
		}
	}

	data += 2 + (count * 4);
	len -= 2 + (count * 4);
}

static void nl80211_get_scan_result_ie(struct nlattr **bss,
				struct clsapi_scan_ap_info *scan_ap_info)
{
	int freq = 0;
	float rates[32];
	int rate_count = 0;
	uint8_t id = 0, len = 0, *data = NULL, cw = 0;
	static unsigned char ms_oui[3] = {0x00, 0x50, 0xf2};
	int ielen = nla_len(bss[NL80211_BSS_INFORMATION_ELEMENTS]);
	unsigned char *ie = nla_data(bss[NL80211_BSS_INFORMATION_ELEMENTS]);

	if (bss[NL80211_BSS_FREQUENCY]) {
		freq = nla_get_u32(bss[NL80211_BSS_FREQUENCY]);
	}

	while (ielen >= 2 && ielen >= ie[1])
	{
		id = ie[0];
		len = ie[1];
		data = ie + 2;
		switch (id) {
		case 1: // Supported rates
		case 50: // Extended Supported Rates
			for (int i = 0; i < len; i++) {
				float r = (data[i] & 0x7F) * 0.5;

				rates[rate_count++] = r;
			}
			break;
		case 0: /* SSID */
		case 114: /* Mesh ID */
			if (scan_ap_info->ssid[0] == 0) {
				uint8_t ssid_len = (len >= sizeof(scan_ap_info->ssid)) ? sizeof(scan_ap_info->ssid) - 1 : len;
				memcpy(scan_ap_info->ssid, (const char *)data, ssid_len);
				scan_ap_info->ssid[ssid_len] = '\0';
			}
			break;

		case 48: /* RSN */
			parse_rsn_ie(&scan_ap_info->crypto, data, len, CLSAPI_WIFI_CIPHER_CCMP, CLSAPI_WIFI_KEY_MGMT_8021X);
			break;

		case 45: /* HT Capabilities */
			scan_ap_info->hwmodes |= CLSAPI_HWMODE_IEEE80211_N;

			// get max BW
			scan_ap_info->max_bw = CLSAPI_WIFI_BW_20; // 20MHz is mandatory for 11n device
			if (*data & 0x0002)
				scan_ap_info->max_bw = CLSAPI_WIFI_BW_40;
			break;

		case 61: /* HT Operation */
			scan_ap_info->hwmodes |= CLSAPI_HWMODE_IEEE80211_N;

			// get max BW
			scan_ap_info->max_bw = CLSAPI_WIFI_BW_20; // 20MHz is mandatory for 11n device
			if (ie[3] & 0x03)
				scan_ap_info->max_bw = CLSAPI_WIFI_BW_40;
			break;

		case 191: /* VHT Capabilities */
			if (freq >= 5000) {
				scan_ap_info->hwmodes |= CLSAPI_HWMODE_IEEE80211_AC;

				// get max BW
				scan_ap_info->max_bw = CLSAPI_WIFI_BW_80; // 80MHz is mandatory for 11ac device
				cw = (*data & 0x0C) >> 2;
				if (cw == 0)
					scan_ap_info->max_bw = CLSAPI_WIFI_BW_80;		// 80MHz
				else if (cw == 1)
					scan_ap_info->max_bw = CLSAPI_WIFI_BW_160;		// 160MHz
				else if (cw == 2)
					scan_ap_info->max_bw = CLSAPI_WIFI_BW_80_80;	// 160MHz/80+80MHz
			}
			break;

		case 192: /* VHT Operation */
			if (freq >= 5000) {
				scan_ap_info->hwmodes |= CLSAPI_HWMODE_IEEE80211_AC;

				// get max BW
				scan_ap_info->max_bw = CLSAPI_WIFI_BW_80; // 80MHz is mandatory for 11ac device
				cw = *data;
				if (cw == 1)
					scan_ap_info->max_bw = CLSAPI_WIFI_BW_80;		// 80MHz
				else if (cw == 2)
					scan_ap_info->max_bw = CLSAPI_WIFI_BW_160;		// 160MHz
				else if (cw == 3)
					scan_ap_info->max_bw = CLSAPI_WIFI_BW_80_80;	// 160MHz/80+80MHz
			}
			break;

		case 221: /* Vendor */
			if (len < 4)
				break;

			if (!memcmp(data, ms_oui, 3)) { // Microsoft Corp.
				switch (ie[5]) {
				case 1:	// WPA1
					parse_rsn_ie(&scan_ap_info->crypto, ie + 6, len - 4, CLSAPI_WIFI_CIPHER_TKIP,
						CLSAPI_WIFI_KEY_MGMT_PSK);
					break;

				case 2: // WMM
					break;

				case 4: // WPS
					scan_ap_info->flags |= CLSAPI_WIFI_AP_FLAG_WPS_CAPA;
					break;
				}
			}
			break;

		case 255: /* Extension ID Extension */
			uint8_t ext_tag = *data;
			uint8_t *ext_data = NULL;

			if (len < 3)
				break;

			ext_data = ie + 3;
			switch (ext_tag) {
			case 35: /* HE Capabilities */
				scan_ap_info->hwmodes |= CLSAPI_HWMODE_IEEE80211_AX;

				cw = ext_data[6];
				scan_ap_info->max_bw = CLSAPI_WIFI_BW_20;  			// 11ax allows 20MHz only device
				if (cw & (1 << 1))
					scan_ap_info->max_bw = CLSAPI_WIFI_BW_40;		// 40MHz & 80MHz
				if (cw & (1 << 2))
					scan_ap_info->max_bw = CLSAPI_WIFI_BW_80;		// 80MHz
				if (cw & (1 << 3))
					scan_ap_info->max_bw = CLSAPI_WIFI_BW_160;		// 160MHz
				if (cw & (1 << 4))
					scan_ap_info->max_bw = CLSAPI_WIFI_BW_80_80;	// 160MHz/80+80MHz

				break;
			}

			break;
		}

		ielen -= len + 2;
		ie += len + 2;
	}
	bool has_ofdm = false;
	bool has_legacy_b = false;

	for (int i = 0; i < rate_count; i++) {
		if (rates[i] == 6.0 || rates[i] == 12.0 || rates[i] == 24.0)
			has_ofdm = true;

		if (rates[i] == 1.0 || rates[i] == 2.0 || rates[i] == 5.5 || rates[i] == 11.0)
			has_legacy_b = true;
	}

	if (freq < 2500) {
		if (has_legacy_b)
			scan_ap_info->hwmodes |= CLSAPI_HWMODE_IEEE80211_B;
		if (has_ofdm)
			scan_ap_info->hwmodes |= CLSAPI_HWMODE_IEEE80211_G;
	} else {
		if (has_ofdm)
			scan_ap_info->hwmodes |= CLSAPI_HWMODE_IEEE80211_A;
	}
}

static int nl80211_get_scan_result_cb(struct nl_msg *msg, void *arg)
{
	uint16_t caps;
	struct cb_arg_ret *cb_arg = (struct cb_arg_ret *)arg;
	char *ifname = (char *)cb_arg->arg1;

	struct clsapi_scan_ap_info scan_ap_info = {0};
	struct nlattr **tb = nl80211_parse(msg);
	struct nlattr *bss[NL80211_BSS_MAX + 1];

	static struct nla_policy bss_policy[NL80211_BSS_MAX + 1] = {
		[NL80211_BSS_FREQUENCY]            = { .type = NLA_U32 },
		[NL80211_BSS_BSSID]                = { 0 },
		[NL80211_BSS_CAPABILITY]           = { .type = NLA_U16 },
		[NL80211_BSS_INFORMATION_ELEMENTS] = { 0 },
		[NL80211_BSS_SIGNAL_MBM]           = { .type = NLA_U32 },
	};

	// set default values
	scan_ap_info.max_bw = CLSAPI_WIFI_BW_20_NOHT;
	scan_ap_info.crypto.sec_protos = CLSAPI_WIFI_SEC_PROTO_OPEN_NONE;

	if (!tb[NL80211_ATTR_BSS] ||
		nla_parse_nested(bss, NL80211_BSS_MAX, tb[NL80211_ATTR_BSS], bss_policy) ||
		!bss[NL80211_BSS_BSSID]){
		return NL_SKIP;
	}

	if (bss[NL80211_BSS_CAPABILITY])
		caps = nla_get_u16(bss[NL80211_BSS_CAPABILITY]);
	else
		caps = 0;

	if (caps & (1 << 0))
		scan_ap_info.flags |= CLSAPI_WIFI_AP_FLAG_AP_MODE; // AP mode

	memcpy(scan_ap_info.bssid, nla_data(bss[NL80211_BSS_BSSID]), ETH_ALEN);

	if (bss[NL80211_BSS_FREQUENCY])
		scan_ap_info.channel = freq_mhz_to_channel(nla_get_u32(bss[NL80211_BSS_FREQUENCY]));

	if (bss[NL80211_BSS_INFORMATION_ELEMENTS])
		nl80211_get_scan_result_ie(bss, &scan_ap_info);

	if (bss[NL80211_BSS_SIGNAL_MBM])
		scan_ap_info.rssi = (uint8_t)((int32_t)nla_get_u32(bss[NL80211_BSS_SIGNAL_MBM]) / 100);

	if ((caps & (1 << 4)) && !scan_ap_info.crypto.sec_protos) {
		// WEP
		scan_ap_info.crypto.sec_protos  |= CLSAPI_WIFI_SEC_PROTO_WEP;
		scan_ap_info.crypto.auth_modes   = CLSAPI_WIFI_AUTH_ALG_OPEN | CLSAPI_WIFI_AUTH_ALG_SHARED;
		scan_ap_info.crypto.pair_ciphers = CLSAPI_WIFI_CIPHER_WEP40 | CLSAPI_WIFI_CIPHER_WEP104;
	}

	append_scan_result(ifname, (uint8_t *)&scan_ap_info, sizeof(scan_ap_info));

	return NL_OK;
}

int nl80211_trigger_scan(const char *ifname, const uint32_t *freqs, const uint8_t freqs_len, uint64_t flags)
{
	struct nl80211_msg_conveyor *cv;
	struct cb_arg_ret cb_arg = {0};
	int ret = CLSAPI_OK;

	// clear scan status
	set_scan_status(ifname, CLSAPI_WIFI_SCAN_STATUS_RESET);

	if (ret < 0)
		return ret;

	// Send NL80211_CMD_TRIGGER_SCAN to cfg80211
	cv = build_nl80211_request(ifname, NL80211_CMD_TRIGGER_SCAN, 0);
	if (!cv) {
		DBG_ERROR("build request\n");
		return -CLSAPI_ERR_NL80211;
	}

	if (freqs && freqs_len) {
		int i = 0;
		struct nl_msg *freqs_msg = nlmsg_alloc();

		if (!freqs_msg)
			return -CLSAPI_ERR_NO_MEM;

		for (i = 0; i < freqs_len; i++)
			nla_put_u32(freqs_msg, i, freqs[i]);

		if (i)
			nla_put_nested(cv->msg, NL80211_ATTR_SCAN_FREQUENCIES, freqs_msg);
		nlmsg_free(freqs_msg);
	}

	if (flags)
		nla_put_u32(cv->msg, NL80211_ATTR_SCAN_FLAGS, flags);

	ret = nl80211_send(cv, NULL, NULL);
	set_scan_status(ifname, CLSAPI_WIFI_SCAN_STATUS_SCANNING);
	if (ret == -EBUSY)
		return -CLSAPI_ERR_SCAN_BUSY;
	else if (ret == -EINVAL)
		return -CLSAPI_ERR_SCAN_NOT_SUPPORT;
	else if (ret)
		return -CLSAPI_ERR_NL80211;

	if (nl80211_wait("nl80211", "scan",
				NL80211_CMD_NEW_SCAN_RESULTS, NL80211_CMD_SCAN_ABORTED)) {
		set_scan_status(ifname, CLSAPI_WIFI_SCAN_STATUS_ABORTED);
		return -CLSAPI_ERR_NL80211;
	}

	if (build_nl80211_request(ifname, NL80211_CMD_GET_SCAN, NLM_F_DUMP)) {
		set_scan_status(ifname, CLSAPI_WIFI_SCAN_STATUS_COMPLETE);
		clear_scan_result(ifname);

		cb_arg.arg1 = (uint8_t *)ifname;
		ret = nl80211_send(cv, nl80211_get_scan_result_cb, &cb_arg);
	} else
		return -CLSAPI_ERR_NL80211;

	return CLSAPI_OK;
}

static int nl80211_get_csi_sta_list_cb(struct nl_msg *msg, void *arg)
{
	int rem;
	struct nlattr **tb_vendor = cls_parse_nl80211_vendor(msg);
	struct nlattr *attr;
	struct cb_arg_ret *cb_arg = (struct cb_arg_ret *)arg;
	uint8_t (*sta_macs)[ETH_ALEN] = (uint8_t (*)[6])(cb_arg->arg1);
	int i = 0, max_len = *(int *)(cb_arg->arg2);
	int *copied_len = (int *)(cb_arg->arg3);

	if (!tb_vendor) {
		cb_arg->ret = -CLSAPI_ERR_INVALID_PARAM;
		return NL_STOP;
	}
	if (!tb_vendor[CLS_NL80211_ATTR_CSI_STA_MACS]) {
		DBG_ERROR("nl80211: CLS attr %d couldn't be found\n", CLS_NL80211_ATTR_CSI_STA_MACS);
		cb_arg->ret = -CLSAPI_ERR_INVALID_PARAM;
		return NL_STOP;
	}

	memset(sta_macs, 0, max_len * ETH_ALEN);
	nla_for_each_nested(attr, tb_vendor[CLS_NL80211_ATTR_CSI_STA_MACS], rem) {
		if (i < max_len)
			memcpy(sta_macs[i++], nla_data(attr), ETH_ALEN);
	}

	*copied_len = i;
	cb_arg->ret = NL_OK;
	return NL_OK;
}

int nl80211_get_csi_sta_list(const char *ifname, const uint8_t (*sta_macs)[ETH_ALEN], int *list_len)
{
	int ret = CLSAPI_OK, copied_cnt = 0;
	struct nlattr *params;
	struct nl80211_msg_conveyor *cv;
	struct cb_arg_ret cb_arg = {0};

	cv = build_cls_nl80211_request(ifname, CLS_NL80211_CMD_GET_CSI, 0);
	if (!cv)
		return -CLSAPI_ERR_NL80211;

	params = nla_nest_start(cv->msg, NL80211_ATTR_VENDOR_DATA);
	if (!params) {
		nlmsg_free(cv->msg);
		return -CLSAPI_ERR_NL80211;
	}

	nla_put_u32(cv->msg, CLS_NL80211_ATTR_ATTR_ID, CLS_NL80211_ATTR_CSI_STA_MACS);
	nla_nest_end(cv->msg, params);

	cb_arg.arg1 = (uint8_t *)sta_macs;
	cb_arg.arg2 = (uint8_t *)list_len;
	cb_arg.arg3 = (uint8_t *)&copied_cnt;
	ret = nl80211_send(cv, nl80211_get_csi_sta_list_cb, &cb_arg);
	if (ret || cb_arg.ret)
		return -CLSAPI_ERR_NL80211;

	*list_len = copied_cnt;

	return CLSAPI_OK;
}

static int nl80211_get_dynamic_cca_ed_thr_cb(struct nl_msg *msg, void *arg)
{
	struct nlattr **tb_vendor = cls_parse_nl80211_vendor(msg);
	struct cb_arg_ret *cb_arg = (struct cb_arg_ret *)arg;
	struct clsapi_cca_ed_config_req *ed_req = (struct clsapi_cca_ed_config_req *)(cb_arg->arg1);

	if (!tb_vendor) {
		cb_arg->ret = -CLSAPI_ERR_INVALID_PARAM;
		return NL_STOP;
	}

	if (!tb_vendor[CLS_NL80211_ATTR_ED_CCA20PRISETHR] || !tb_vendor[CLS_NL80211_ATTR_ED_CCA20PFALLTHR] ||
		!tb_vendor[CLS_NL80211_ATTR_ED_CCA20SRISETHR] || !tb_vendor[CLS_NL80211_ATTR_ED_CCA20SFALLTHR]) {
		DBG_ERROR("nl80211: CLS CCA attrs couldn't be found\n");
		return NL_STOP;
	}

	memset(ed_req, 0, sizeof(struct clsapi_cca_ed_config_req));

	ed_req->cca20p_risethr = nla_get_u8(tb_vendor[CLS_NL80211_ATTR_ED_CCA20PRISETHR]);
	ed_req->cca20p_fallthr = nla_get_u8(tb_vendor[CLS_NL80211_ATTR_ED_CCA20PFALLTHR]);
	ed_req->cca20s_risethr = nla_get_u8(tb_vendor[CLS_NL80211_ATTR_ED_CCA20SRISETHR]);
	ed_req->cca20s_fallthr = nla_get_u8(tb_vendor[CLS_NL80211_ATTR_ED_CCA20SFALLTHR]);

	cb_arg->ret = NL_OK;

	return NL_OK;
}

int nl80211_get_dynamic_cca_ed_thr(const char *ifname, struct clsapi_cca_ed_config_req *ed_req)
{
	int ret = CLSAPI_OK;
	struct nlattr *params;
	struct cb_arg_ret cb_arg = {0};
	struct nl80211_msg_conveyor *cv;

	cv = build_cls_nl80211_request(ifname, CLS_NL80211_CMD_GET_CCA_ED_THR, 0);
	if (!cv)
		return -CLSAPI_ERR_NL80211;

	params = nla_nest_start(cv->msg, NL80211_ATTR_VENDOR_DATA);
	if (!params) {
		nlmsg_free(cv->msg);
		return -CLSAPI_ERR_NL80211;
	}
	nla_nest_end(cv->msg, params);

	cb_arg.arg1 = (uint8_t *)ed_req;
	ret = nl80211_send(cv, nl80211_get_dynamic_cca_ed_thr_cb, &cb_arg);

	if (ret || cb_arg.ret)
		return -CLSAPI_ERR_NL80211;

	return CLSAPI_OK;
}

int nl80211_get_txpower_cb(struct nl_msg *msg, void *arg)
{
	int res = 0;
	struct nlattr **tb = nl80211_parse(msg);
	struct cb_arg_ret *cb_arg = (struct cb_arg_ret *)arg;
	uint8_t *buf = (uint8_t *)cb_arg->arg1;

	if (!tb) {
		DBG_ERROR("Internal error--nl80211 parse failed\n");
		cb_arg->ret = -CLSAPI_ERR_INVALID_PARAM;
		return NL_STOP;
	}

	if (tb[NL80211_ATTR_WIPHY_TX_POWER_LEVEL]) {
		res = nla_get_u32(tb[NL80211_ATTR_WIPHY_TX_POWER_LEVEL]);
		*buf = res / 100;
	} else {
		DBG_ERROR("Internal error--nl80211 reply, there is no txpower\n");
		return NL_STOP;
	}

	return NL_OK;
}

int nl80211_get_txpower(const char *ifname, int8_t *buf)
{
	int ret = CLSAPI_OK;
	struct cb_arg_ret cb_arg = {0};
	struct nl80211_msg_conveyor *cv;

	cv = build_nl80211_request(ifname, NL80211_CMD_GET_INTERFACE, 0);
	if (!cv) {
		DBG_ERROR("Internal error--nl80211 request build fail\n");
		return -CLSAPI_ERR_NL80211;
	}

	cb_arg.arg1 = (uint8_t *)buf;
	ret = nl80211_send(cv, nl80211_get_txpower_cb, &cb_arg);
	if (ret)
		return ret;

	return cb_arg.ret;
}

static int nl80211_freq2channel(int freq)
{
	if (freq == 2484)
		return 14;
	else if (freq < 2484)
		return (freq - 2407) / 5;
	else if (freq >= 4910 && freq <= 4980)
		return (freq - 4000) / 5;
	else if (freq >= 56160 + 2160 * 1 && freq <= 56160 + 2160 * 6)
		return (freq - 56160) / 2160;
	else
		return (freq - 5000) / 5;
}

static int nl80211_get_txpwrlist_cb(struct nl_msg *msg, void *arg)
{
	int *dbm_max = arg;
	int ch_cur, ch_cmp, bands_remain, freqs_remain;

	struct nlattr **attr = nl80211_parse(msg);
	struct nlattr *bands[NL80211_BAND_ATTR_MAX + 1];
	struct nlattr *freqs[NL80211_FREQUENCY_ATTR_MAX + 1];
	struct nlattr *band, *freq;

	static struct nla_policy freq_policy[NL80211_FREQUENCY_ATTR_MAX + 1] = {
		[NL80211_FREQUENCY_ATTR_FREQ]         = { .type = NLA_U32  },
		[NL80211_FREQUENCY_ATTR_DISABLED]     = { .type = NLA_FLAG },
		[NL80211_FREQUENCY_ATTR_PASSIVE_SCAN] = { .type = NLA_FLAG },
		[NL80211_FREQUENCY_ATTR_NO_IBSS]      = { .type = NLA_FLAG },
		[NL80211_FREQUENCY_ATTR_RADAR]        = { .type = NLA_FLAG },
		[NL80211_FREQUENCY_ATTR_MAX_TX_POWER] = { .type = NLA_U32  },
	};

	ch_cur = *dbm_max; /* value int* is initialized with channel by caller */
	*dbm_max = -1;

	nla_for_each_nested(band, attr[NL80211_ATTR_WIPHY_BANDS], bands_remain) {
		nla_parse(bands, NL80211_BAND_ATTR_MAX, nla_data(band), nla_len(band), NULL);

		nla_for_each_nested(freq, bands[NL80211_BAND_ATTR_FREQS], freqs_remain) {
			nla_parse(freqs, NL80211_FREQUENCY_ATTR_MAX, nla_data(freq), nla_len(freq), freq_policy);

			ch_cmp = nl80211_freq2channel(nla_get_u32(
				freqs[NL80211_FREQUENCY_ATTR_FREQ]));

			if ((!ch_cur || (ch_cmp == ch_cur)) && freqs[NL80211_FREQUENCY_ATTR_MAX_TX_POWER]) {
				*dbm_max = (int)(0.01 * nla_get_u32(freqs[NL80211_FREQUENCY_ATTR_MAX_TX_POWER]));

				break;
			}
		}
	}

	return NL_SKIP;
}

int nl80211_get_max_txpower(const char *ifname, int ch_cur, int *dbm_max)
{
	int err;

	/* initialize the value pointer with channel for callback */
	*dbm_max = ch_cur;

	err = nl80211_request(ifname, NL80211_CMD_GET_WIPHY, 0,
			nl80211_get_txpwrlist_cb, dbm_max);

	if (!err)
		return 0;
	else
		return -1;
}

int nl80211_get_protocol_features_cb(struct nl_msg *msg, void *arg)
{
	uint32_t *features = arg;
	struct nlattr **attr = nl80211_parse(msg);

	if (attr[NL80211_ATTR_PROTOCOL_FEATURES])
		*features = nla_get_u32(attr[NL80211_ATTR_PROTOCOL_FEATURES]);

	return NL_SKIP;
}

int nl80211_get_protocol_features(const char *ifname)
{
	struct nl80211_msg_conveyor *req;
	uint32_t features = 0;

	req = nl80211_msg(ifname, NL80211_CMD_GET_PROTOCOL_FEATURES, 0);
	if (req) {
		nl80211_send(req, nl80211_get_protocol_features_cb, &features);
		nl80211_free(req);
	}

	return features;
}

static int nl80211_get_modelist_cb(struct nl_msg *msg, void *arg)
{
	struct nl80211_modes *m = arg;
	int bands_remain, freqs_remain;
	struct nlattr **attr = nl80211_parse(msg);
	struct nlattr *bands[NL80211_BAND_ATTR_MAX + 1];
	struct nlattr *freqs[NL80211_FREQUENCY_ATTR_MAX + 1];
	struct nlattr *band, *freq;
	uint32_t freq_mhz;

	if (attr[NL80211_ATTR_WIPHY_BANDS]) {
		nla_for_each_nested(band, attr[NL80211_ATTR_WIPHY_BANDS], bands_remain) {
			nla_parse(bands, NL80211_BAND_ATTR_MAX, nla_data(band), nla_len(band), NULL);

			if (bands[NL80211_BAND_ATTR_HT_CAPA])
				m->nl_ht = nla_get_u16(bands[NL80211_BAND_ATTR_HT_CAPA]);

			if (bands[NL80211_BAND_ATTR_VHT_CAPA])
				m->nl_vht = nla_get_u32(bands[NL80211_BAND_ATTR_VHT_CAPA]);

			if (bands[NL80211_BAND_ATTR_IFTYPE_DATA]) {
				struct nlattr *tb[NL80211_BAND_IFTYPE_ATTR_MAX + 1];
				struct nlattr *nl_iftype;
				int rem_band;
				int len;

				nla_for_each_nested(nl_iftype, bands[NL80211_BAND_ATTR_IFTYPE_DATA], rem_band) {
					nla_parse(tb, NL80211_BAND_IFTYPE_ATTR_MAX,
							nla_data(nl_iftype), nla_len(nl_iftype), NULL);
					if (tb[NL80211_BAND_IFTYPE_ATTR_HE_CAP_PHY]) {
						len = nla_len(tb[NL80211_BAND_IFTYPE_ATTR_HE_CAP_PHY]);
						if (len > (sizeof(m->he_phy_cap) - 1))
							len = sizeof(m->he_phy_cap) - 1;
						memcpy(&((__u8 *)m->he_phy_cap)[1],
								nla_data(tb[NL80211_BAND_IFTYPE_ATTR_HE_CAP_PHY]), len);
					}
				}
			}

			if (bands[NL80211_BAND_ATTR_FREQS]) {
				nla_for_each_nested(freq, bands[NL80211_BAND_ATTR_FREQS],
						freqs_remain) {
					nla_parse(freqs, NL80211_FREQUENCY_ATTR_MAX,
							nla_data(freq), nla_len(freq), NULL);

					if (!freqs[NL80211_FREQUENCY_ATTR_FREQ])
						continue;

					freq_mhz = nla_get_u32(freqs[NL80211_FREQUENCY_ATTR_FREQ]);

					if (freq_mhz > 2400 && freq_mhz < 2485)
						m->bands |= NL80211_BAND_24;
					else if (freq_mhz > 5000 && freq_mhz < 5850)
						m->bands |= NL80211_BAND_5;
					else if (freq_mhz > 6000 && freq_mhz < 7120)
						m->bands |= NL80211_BAND_6;
					else if (freq_mhz >= 56160)
						m->bands |= NL80211_BAND_60;
				}
			}
		}

		m->ok = 1;
	}

	return NL_SKIP;
}

static void nl80211_eval_modelist(struct nl80211_modes *m)
{
	/* Treat any nonzero capability as 11n */
	if (m->nl_ht > 0) {
		m->hw |= NL80211_80211_N;
		m->ht |= NL80211_HTMODE_HT20;

		if (m->nl_ht & (1 << 1))
			m->ht |= NL80211_HTMODE_HT40;
	}

	if (m->he_phy_cap[0] != 0) {
		m->hw |= NL80211_80211_AX;
		m->ht |= NL80211_HTMODE_HE20;

		if (m->he_phy_cap[0] & BIT(9))
			m->ht |= NL80211_HTMODE_HE40;
		if (m->he_phy_cap[0] & BIT(10))
			m->ht |= NL80211_HTMODE_HE40 | NL80211_HTMODE_HE80;
		if (m->he_phy_cap[0] & BIT(11))
			m->ht |= NL80211_HTMODE_HE160;
		if (m->he_phy_cap[0] & BIT(12))
			m->ht |= NL80211_HTMODE_HE160 | NL80211_HTMODE_HE80_80;
	}

	if (m->bands & NL80211_BAND_24) {
		m->hw |= NL80211_80211_B;
		m->hw |= NL80211_80211_G;
	}

	if (m->bands & NL80211_BAND_5) {
		/* Treat any nonzero capability as 11ac */
		if (m->nl_vht > 0) {
			m->hw |= NL80211_80211_AC;
			m->ht |= NL80211_HTMODE_VHT20 | NL80211_HTMODE_VHT40 | NL80211_HTMODE_VHT80;

			switch ((m->nl_vht >> 2) & 3) {
			case 2:
				m->ht |= NL80211_HTMODE_VHT80_80;
				/* fall through */

			case 1:
				m->ht |= NL80211_HTMODE_VHT160;
			}
		} else
			m->hw |= NL80211_80211_A;
	}

	if (m->bands & NL80211_BAND_60)
		m->hw |= NL80211_80211_AD;
}

int nl80211_get_hwmodelist(const char *ifname, int *buf)
{
	struct nl80211_msg_conveyor *cv;
	struct nl80211_modes m = {};
	uint32_t features = nl80211_get_protocol_features(ifname);
	int flags;

	flags = features & NL80211_PROTOCOL_FEATURE_SPLIT_WIPHY_DUMP ? NLM_F_DUMP : 0;
	cv = nl80211_msg(ifname, NL80211_CMD_GET_WIPHY, flags);
	if (!cv)
		goto out;

	NLA_PUT_FLAG(cv->msg, NL80211_ATTR_SPLIT_WIPHY_DUMP);
	if (nl80211_send(cv, nl80211_get_modelist_cb, &m))
		goto nla_put_failure;

	nl80211_eval_modelist(&m);

	*buf = m.hw;

	return 0;

nla_put_failure:
	nl80211_free(cv);
out:
	return -1;
}

int nl80211_get_htmodelist(const char *ifname, int *buf)
{
	struct nl80211_msg_conveyor *cv;
	struct nl80211_modes m = {};
	uint32_t features = nl80211_get_protocol_features(ifname);
	int flags;

	flags = features & NL80211_PROTOCOL_FEATURE_SPLIT_WIPHY_DUMP ? NLM_F_DUMP : 0;
	cv = nl80211_msg(ifname, NL80211_CMD_GET_WIPHY, flags);
	if (!cv)
		goto out;

	NLA_PUT_FLAG(cv->msg, NL80211_ATTR_SPLIT_WIPHY_DUMP);
	if (nl80211_send(cv, nl80211_get_modelist_cb, &m))
		goto nla_put_failure;

	nl80211_eval_modelist(&m);

	*buf = m.ht;

	return 0;

nla_put_failure:
	nl80211_free(cv);
out:
	return -1;
}

static int nl80211_get_survey_cb(struct nl_msg *msg, void *arg)
{
	struct nl80211_array_buf *arr = arg;
	struct survey_entry *e = arr->buf;
	struct nlattr **attr = nl80211_parse(msg);
	struct nlattr *sinfo[NL80211_SURVEY_INFO_MAX + 1];
	int rc;

	static struct nla_policy survey_policy[NL80211_SURVEY_INFO_MAX + 1] = {
		[NL80211_SURVEY_INFO_FREQUENCY] = { .type = NLA_U32 },
		[NL80211_SURVEY_INFO_NOISE]  = { .type = NLA_U8     },
		[NL80211_SURVEY_INFO_TIME] = { .type = NLA_U64   },
		[NL80211_SURVEY_INFO_TIME_BUSY] = { .type = NLA_U64   },
		[NL80211_SURVEY_INFO_TIME_EXT_BUSY] = { .type = NLA_U64   },
		[NL80211_SURVEY_INFO_TIME_RX] = { .type = NLA_U64   },
		[NL80211_SURVEY_INFO_TIME_TX] = { .type = NLA_U64   },
	};

	rc = nla_parse_nested(sinfo, NL80211_SURVEY_INFO_MAX,
				attr[NL80211_ATTR_SURVEY_INFO],
				survey_policy);
	if (rc)
		return NL_SKIP;

	/* advance to end of array */
	e += arr->count;
	memset(e, 0, sizeof(*e));

	if (sinfo[NL80211_SURVEY_INFO_FREQUENCY])
		e->mhz = nla_get_u32(sinfo[NL80211_SURVEY_INFO_FREQUENCY]);

	if (sinfo[NL80211_SURVEY_INFO_NOISE])
		e->noise = nla_get_u8(sinfo[NL80211_SURVEY_INFO_NOISE]);

	if (sinfo[NL80211_SURVEY_INFO_TIME])
		e->active_time = nla_get_u64(sinfo[NL80211_SURVEY_INFO_TIME]);

	if (sinfo[NL80211_SURVEY_INFO_TIME_BUSY])
		e->busy_time = nla_get_u64(sinfo[NL80211_SURVEY_INFO_TIME_BUSY]);

	if (sinfo[NL80211_SURVEY_INFO_TIME_EXT_BUSY])
		e->busy_time_ext = nla_get_u64(sinfo[NL80211_SURVEY_INFO_TIME_EXT_BUSY]);

	if (sinfo[NL80211_SURVEY_INFO_TIME_RX])
		e->rxtime = nla_get_u64(sinfo[NL80211_SURVEY_INFO_TIME_RX]);

	if (sinfo[NL80211_SURVEY_INFO_TIME_TX])
		e->txtime = nla_get_u64(sinfo[NL80211_SURVEY_INFO_TIME_TX]);

	arr->count++;
	return NL_SKIP;
}

int nl80211_get_survey(const char *ifname, char *buf, int *len)
{
	struct nl80211_array_buf arr = { .buf = buf, .count = 0 };
	int rc;

	rc = nl80211_request(ifname, NL80211_CMD_GET_SURVEY,
			NLM_F_DUMP, nl80211_get_survey_cb, &arr);
	if (!rc)
		*len = (arr.count * sizeof(struct survey_entry));
	else
		*len = 0;

	return 0;
}

