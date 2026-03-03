/*
 * Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "wpa_adapter.h"


static struct wpa_ctrl wpa_ctrl;


static inline void wpa_ctrl_close()
{
	unlink(wpa_ctrl.local.sun_path);
	if (wpa_ctrl.s >= 0)
		close(wpa_ctrl.s);
}


static int wpa_ctrl_connect(const char *ctrl_path)
{
	struct wpa_ctrl *ctrl = &wpa_ctrl;
	static int counter = 0;
	int ret;
	int tries = 0;
	int flags;

	if (ctrl_path == NULL)
		return -CLSAPI_ERR_INVALID_PARAM;

	ctrl->s = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (ctrl->s < 0)
		return -CLSAPI_ERR_WPA;

	ctrl->local.sun_family = AF_UNIX;
	counter++;
try_again:
	ret = snprintf(ctrl->local.sun_path,
			  sizeof(ctrl->local.sun_path),
			  CONFIG_CTRL_IFACE_CLIENT_DIR "/"
			  CONFIG_CTRL_IFACE_CLIENT_PREFIX "%d-%d",
			  (int) getpid(), counter);

	if (ret < 0 || ret >= sizeof(ctrl->local.sun_path)) {
		close(ctrl->s);
		return -CLSAPI_ERR_INTERNAL_ERR;
	}
	tries++;
	fchmod(ctrl->s, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

	if (bind(ctrl->s, (struct sockaddr *) &ctrl->local,
		    sizeof(ctrl->local)) < 0) {
		if (errno == EADDRINUSE && tries < 2) {
			unlink(ctrl->local.sun_path);
			goto try_again;
		}
		close(ctrl->s);
		return -CLSAPI_ERR_WPA;
	}

	ret = lchown(ctrl->local.sun_path, -1, 101);
	ret = lchown(ctrl->local.sun_path, 101, 101);
	ctrl->dest.sun_family = AF_UNIX;

	if (strlen(ctrl_path) + 1 > sizeof(ctrl->dest.sun_path)) {
		wpa_ctrl_close();
		return -CLSAPI_ERR_INVALID_PARAM;
	}
	cls_strncpy(ctrl->dest.sun_path, ctrl_path, sizeof(ctrl->dest.sun_path));

	if (connect(ctrl->s, (struct sockaddr *) &ctrl->dest, sizeof(ctrl->dest)) < 0) {
		DBG_ERROR("%s() L%d connect() fail, sun_path=%s'\n", __func__, __LINE__, ctrl->dest.sun_path);
		wpa_ctrl_close();
		return -CLSAPI_ERR_WPA;
	}

	flags = fcntl(ctrl->s, F_GETFL);
	if (flags >= 0) {
		flags |= O_NONBLOCK;
		if (fcntl(ctrl->s, F_SETFL, flags) < 0)
			DBG_ERROR("fcntl(ctrl->s, O_NONBLOCK)");
	}

	return CLSAPI_OK;
}


static inline int hostapd_ctrl_connect(const char *ifname)
{
	char ctrl_path[128];

	int ret = snprintf(ctrl_path, sizeof(ctrl_path), "%s/%s", CONFIG_CTRL_IFACE_DIR_HOSTAPD, ifname);
	if (ret < 0 || ret >= sizeof(ctrl_path))
		return -CLSAPI_ERR_INVALID_PARAM;

	return wpa_ctrl_connect(ctrl_path);
}

static int wpa_supplicant_ctrl_connect(const char *ifname)
{
	char ctrl_path[128];

	int ret = snprintf(ctrl_path, sizeof(ctrl_path), "%s/%s", CONFIG_CTRL_IFACE_DIR_WPA_SUPPLICANT, ifname);
	if (ret < 0 || ret >= sizeof(ctrl_path))
		return -CLSAPI_ERR_INVALID_PARAM;

	return wpa_ctrl_connect(ctrl_path);
}

/** Send hostapd or wpa_supplicant ctrl request, wait and recv() replies.
 * Returns:
 *   CLSAPI_OK: OK
 *   others:        Errors
 */
static int wpa_ctrl_request(const char *ifname, const char *cmd, size_t cmd_len,
		char *reply, size_t *reply_len)
{
	struct wpa_ctrl *ctrl = &wpa_ctrl;
	struct timeval tv;
	int res = 0, send_cnt = 0;
	fd_set rfds;

	errno = 0;

retry_send:
	send_cnt++;
	if (send(ctrl->s, cmd, cmd_len, 0) < 0) {
		if (errno == EAGAIN || errno == EBUSY || errno == EWOULDBLOCK) {
			if (send_cnt > 5) {
				res = -1;
				goto out;
			}
			sleep(1);
			goto retry_send;
		}
	}

	for (;;) {
		tv.tv_sec = 10;
		tv.tv_usec = 0;
		FD_ZERO(&rfds);
		FD_SET(ctrl->s, &rfds);
		res = select(ctrl->s + 1, &rfds, NULL, NULL, &tv);
		if (res < 0 && errno == EINTR)
			continue;
		if (res < 0)
			goto out;
		if (FD_ISSET(ctrl->s, &rfds)) {
			res = recv(ctrl->s, reply, *reply_len, 0);
			if (res < 0)
				goto out;
			*reply_len = res;
			res = 0;
			break;
		} else {
			res = -2;
			goto out;
		}
	}

out:
	wpa_ctrl_close();
	return res;
}

/** Send hostapd ctrl request, wait and recv() replies.
 * Returns:
 *	 CLSAPI_OK: OK
 *	 others:		Errors
 */
static int hostapd_ctrl_request(const char *ifname, const char *cmd, size_t cmd_len,
		char *reply, size_t *reply_len)
{
	int ret;

	ret = hostapd_ctrl_connect(ifname);
	if (ret < 0)
		return ret;

	ret = wpa_ctrl_request(ifname, cmd, cmd_len, reply, reply_len);
	if (ret < 0)
		return -CLSAPI_ERR_HOSTAPD;
	return CLSAPI_OK;
}

static int wpa_supplicant_ctrl_request(const char *ifname, const char *cmd, size_t cmd_len,
		char *reply, size_t *reply_len)
{
	int ret;

	ret = wpa_supplicant_ctrl_connect(ifname);
	if (ret < 0)
		return ret;

	ret = wpa_ctrl_request(ifname, cmd, cmd_len, reply, reply_len);
	if (ret < 0)
		return -CLSAPI_ERR_HOSTAPD;

	return ret;
}

int hostapd_add_vbss_vap(const struct vbss_vap_info *vap, char *new_ifname, const int name_len)
{
	string_1024 cmd;
	size_t reply_len = name_len;
	int cmd_len, ret;
	char *pos;
	string_32 primary_vap;
	unsigned int radio_id = 255;
	string_1024 reply;
	struct vbss_vap_info new_vap;

	/* CLS_HOSTAPD CMD_ADD_VBSS_VAP cmd format:
	 *		"CLS add_vbss_vap \0" + <bin of struct vbss_vap_info>
	 * reply format:
	 *		string of ifname which new created
	 */
	ret = local_get_radio_id(new_ifname, &radio_id);
	if (ret < 0)
		return -CLSAPI_ERR_INVALID_PARAM;

	memcpy(&new_vap, vap, sizeof(struct vbss_vap_info));
	memcpy(new_vap.ifname, new_ifname, strlen(new_ifname) + 1);
	get_primary_vap_name(radio_id, primary_vap, sizeof(primary_vap) - 1);
	create_new_bss_config(new_ifname, &new_vap);
	snprintf(cmd, sizeof(cmd), "%s %s", CLS_HOSTAPD_CMD_PREFIX, CLS_HOSTAPD_CMD_ADD_VBSS_VAP);
	cmd_len = strlen(cmd) + 1;
	pos = cmd + cmd_len;
	memcpy(pos, &new_vap, sizeof(struct vbss_vap_info));
	cmd_len += sizeof(struct vbss_vap_info);
	ret = hostapd_ctrl_request(primary_vap, cmd, cmd_len, reply, &reply_len);
	if (reply_len <= 0)
		return -CLSAPI_ERR_HOSTAPD;

	return ret;
}


int hostapd_del_vbss_vap(const char *ifname)
{
	string_1024 cmd = {0};
	string_1024 reply;
	size_t reply_len = sizeof(reply);
	int cmd_len;
	int ret;
	string_32 primary_vap;
	unsigned int radio_id = -1;

	/* CLS_HOSTAPD CMD_DEL_VBSS_VAP cmd format:
	 *		"CLS del_vbss_vap "
	 * reply format:
	 *		"OK": success
	 *		<others>: fail
	 */
	ret = local_get_radio_id(ifname, &radio_id);
	if (ret < 0)
		return -CLSAPI_ERR_INVALID_PARAM;
	if (remove_bss(radio_id, ifname) == -1)
		return -CLSAPI_ERR_INVALID_PARAM;
	get_primary_vap_name(radio_id, primary_vap, sizeof(primary_vap) - 1);
	cmd_len = snprintf(cmd, sizeof(cmd), "%s %s%s", CLS_HOSTAPD_CMD_PREFIX,
						CLS_HOSTAPD_CMD_DEL_VBSS_VAP, ifname) + 1;
	ret = hostapd_ctrl_request(primary_vap, cmd, cmd_len, reply, &reply_len);
	if (strncmp(reply, CLS_HOSTAPD_REPLY_OK, sizeof(CLS_HOSTAPD_REPLY_OK) - 1)) {
		DBG_ERROR("%s(ifname=%s) cmd='%s' cmd_len=%d reply_len=%d reply='%s'\n",
				__func__, ifname, cmd, cmd_len, reply_len, reply);
		return -CLSAPI_ERR_HOSTAPD;
	}

	return ret;
}

int hostapd_get_vbss_vap(const char *new_ifname, struct vbss_vap_info *vap)
{
	string_1024 cmd;
	int cmd_len, ret;
	string_1024 reply;
	size_t reply_len = sizeof(reply);
	struct vbss_vap_info *tmp = NULL;

	uint8_t vap_mac[ETH_ALEN];

	/* CLS_HOSTAPD CMD_GET_VBSS_VAP cmd format:
	 *		"CLS get_vbss_vap \0" + <vap mac>
	 * reply format:
	 *		struct vbss_vap_info
	 */
	if (!get_interface_mac(new_ifname, vap_mac))
		return -CLSAPI_ERR_INVALID_PARAM;
	snprintf(cmd, sizeof(cmd), "%s %s" MACFMT, CLS_HOSTAPD_CMD_PREFIX,
			CLS_HOSTAPD_CMD_GET_VBSS_VAP, MACARG(vap_mac));
	cmd_len = strlen(cmd) + 1;
	ret = hostapd_ctrl_request(new_ifname, cmd, cmd_len, reply, &reply_len);
	if (reply_len <= 0)
		return -CLSAPI_ERR_HOSTAPD;
	if (ret == 0) {
		tmp = (struct vbss_vap_info *)reply;
		vap->auth_type = tmp->auth_type;
		memcpy(vap->pwd, tmp->pwd, sizeof(vap->pwd) - 1);
	}
	return ret;
}
int hostapd_add_vbss_sta(const char *ifname, const struct vbss_sta_info *sta)
{
	string_1024 cmd;
	string_1024 reply;
	size_t reply_len = sizeof(reply);
	int cmd_len, ret;
	char *pos;

	/* CLS_HOSTAPD CMD_ADD_VBSS_STA cmd format:
	 *		"CLS add_vbss_sta \0" + <bin of struct vbss_sta_info>
	 * reply format:
	 *		"OK": success
	 *		<others>: fail
	 */
	snprintf(cmd, sizeof(cmd), "%s %s", CLS_HOSTAPD_CMD_PREFIX, CLS_HOSTAPD_CMD_ADD_VBSS_STA);
	cmd_len = strlen(cmd) + 1;
	pos = cmd + cmd_len;
	memcpy(pos, sta, sizeof(struct vbss_sta_info));
	cmd_len += sizeof(struct vbss_sta_info);
	ret = hostapd_ctrl_request(ifname, cmd, cmd_len, reply, &reply_len);
	if (strncmp(reply, CLS_HOSTAPD_REPLY_OK, sizeof(CLS_HOSTAPD_REPLY_OK) - 1)) {
		DBG_ERROR("%s(ifname=%s STA " MACFMT ") cmd='%s' cmd_len=%d reply_len=%d reply='%s'\n",
				__func__, ifname, MACARG(sta->mac_addr), cmd, cmd_len, reply_len, reply);
		return -CLSAPI_ERR_HOSTAPD;
	}

	return ret;
}


int hostapd_del_vbss_sta(const char *ifname, const uint8_t sta_mac[ETH_ALEN])
{
	string_1024 cmd;
	string_1024 reply;
	size_t reply_len = sizeof(reply);
	int cmd_len;
	int ret;

	/* CLS_HOSTAPD CMD_DEL_VBSS_STA cmd format:
	 *		"CLS del_vbss_sta <STA MAC in format xx:xx:xx:xx:xx:xx>"
	 * reply format:
	 *		"OK": success
	 *		<others>: fail
	 */
	snprintf(cmd, sizeof(cmd), "%s %s" MACFMT, CLS_HOSTAPD_CMD_PREFIX,
			CLS_HOSTAPD_CMD_DEL_VBSS_STA, MACARG(sta_mac));
	cmd_len = strlen(cmd) + 1;
	ret = hostapd_ctrl_request(ifname, cmd, cmd_len, reply, &reply_len);
	if (strncmp(reply, CLS_HOSTAPD_REPLY_OK, sizeof(CLS_HOSTAPD_REPLY_OK) - 1)) {
		DBG_ERROR("%s(ifname=%s STA:" MACFMT ") cmd='%s' cmd_len=%d reply_len=%d reply='%s'\n",
				__func__, ifname, MACARG(sta_mac), cmd, cmd_len, reply_len, reply);
		return -CLSAPI_ERR_HOSTAPD;
	}

	return ret;
}


int hostapd_set_vbss_enabled(const char *phyname, const bool onoff)
{
	string_1024 cmd = {0};
	string_1024 reply;
	size_t reply_len;
	int cmd_len;
	int ret;

	/* CLS_HOSTAPD CMD_SET_VBSS_ENABLED cmd format:
	 *		"CLS set_vbss_enabled <phy name> <onoff>"
	 * reply format:
	 *		"OK": success
	 *		<others>: fail
	 */
	reply_len = sizeof(reply);
	snprintf(cmd, sizeof(cmd), "%s %s%s %s", CLS_HOSTAPD_CMD_PREFIX,
			CLS_HOSTAPD_CMD_SET_VBSS_ENABLED, phyname,
			(onoff ? "1" : "0"));
	cmd_len = strlen(cmd) + 1;
	ret = hostapd_ctrl_request(CONFIG_CTRL_IFACE_GLOBAL, cmd, cmd_len, reply, &reply_len);
	if (strncmp(reply, CLS_HOSTAPD_REPLY_OK, sizeof(CLS_HOSTAPD_REPLY_OK) - 1)) {
		DBG_ERROR("%s(phyname=%s onoff=%d) cmd='%s' cmd_len=%d reply_len=%d reply='%s'\n",
				__func__, phyname, onoff, cmd, cmd_len, reply_len, reply);
		return -CLSAPI_ERR_HOSTAPD;
	}

	return ret;
}

int hostapd_get_vbss_sta_info(const char *ifname, const uint8_t sta_mac[ETH_ALEN],
		struct vbss_sta_info *sta)
{
	string_1024 cmd;
	string_1024 reply;
	size_t reply_len = sizeof(reply);
	int cmd_len;
	int ret;

	snprintf(cmd, sizeof(cmd), "%s %s" MACFMT, CLS_HOSTAPD_CMD_PREFIX,
			CLS_HOSTAPD_CMD_GET_VBSS_STA, MACARG(sta_mac));
	cmd_len = strlen(cmd) + 1;
	ret = hostapd_ctrl_request(ifname, cmd, cmd_len, reply, &reply_len);
	if (ret == 0)
		memcpy(sta, reply, reply_len);

	return ret;
}


int hostapd_trigger_vbss_switch(const char *ifname, const uint8_t sta_mac[ETH_ALEN])
{
	string_1024 cmd;
	string_1024 reply;
	size_t reply_len = sizeof(reply);
	int cmd_len;
	int ret;

	cmd_len = snprintf(cmd, sizeof(cmd), "%s %s" MACFMT, CLS_HOSTAPD_CMD_PREFIX,
			CLS_HOSTAPD_CMD_TRG_VBSS_SWITCH, MACARG(sta_mac)) + 1;
	ret = hostapd_ctrl_request(ifname, cmd, cmd_len, reply, &reply_len);
	if (!ret && strncmp(reply, CLS_HOSTAPD_REPLY_OK, sizeof(CLS_HOSTAPD_REPLY_OK) - 1)) {
		DBG_ERROR("%s(ifname=%s STA " MACFMT ") cmd='%s' cmd_len=%d reply_len=%d reply='%s'\n",
				__func__, ifname, MACARG(sta_mac), cmd, cmd_len, reply_len, reply);
		return -CLSAPI_ERR_HOSTAPD;
	}

	return ret;
}


int hostapd_set_vbss_switch_done(const char *ifname, const uint8_t sta_mac[ETH_ALEN])
{
	string_1024 cmd;
	string_1024 reply;
	size_t reply_len = sizeof(reply);
	int cmd_len;
	int ret;

	cmd_len = snprintf(cmd, sizeof(cmd), "%s %s" MACFMT, CLS_HOSTAPD_CMD_PREFIX,
			CLS_HOSTAPD_CMD_SET_VBSS_SWITCH_DONE, MACARG(sta_mac)) + 1;
	ret = hostapd_ctrl_request(ifname, cmd, cmd_len, reply, &reply_len);
	if (!ret && strncmp(reply, CLS_HOSTAPD_REPLY_OK, sizeof(CLS_HOSTAPD_REPLY_OK) - 1)) {
		DBG_ERROR("%s(ifname=%s STA " MACFMT ") cmd='%s' cmd_len=%d reply_len=%d reply='%s'\n",
				__func__, ifname, MACARG(sta_mac), cmd, cmd_len, reply_len, reply);
		return -CLSAPI_ERR_HOSTAPD;
	}

	return ret;
}

int hostapd_set_vbss_stop_rekey(const char *ifname)
{
	string_1024 cmd;
	int cmd_len, ret;
	string_1024 reply;
	uint8_t vap_mac[ETH_ALEN];
	size_t reply_len = sizeof(reply);

	if (!get_interface_mac(ifname, vap_mac))
		return -CLSAPI_ERR_INVALID_PARAM;
	snprintf(cmd, sizeof(cmd), "%s %s" MACFMT, CLS_HOSTAPD_CMD_PREFIX, CLS_CMD_DISABLE_VBSS_REKEY,
				MACARG(vap_mac));
	cmd_len = strlen(cmd) + 1;
	ret = hostapd_ctrl_request(ifname, cmd, cmd_len, reply, &reply_len);
	if (reply_len <= 0)
		return -CLSAPI_ERR_HOSTAPD;

	return ret;
}

/** Get infos from hostapd by standard cmd via ctrl iface
 * Inputs:
 *   ifname:  interface/bss name
 *   cmd_buf: standard cmd and/or parameters
 *   cmd_len: length of cmd and/or parameters (not total buffer length).
 *   reply:   buffer to carry reply infos
 *   repley_len: size of the reply buffer
 * Returns:
 *	 CLSAPI_OK: send and recv OK, "reply" carries cmd exec results from hostapd.
 *	 others:		send and recv Errors, no data in reply buffer.
 */
int hostapd_ctrl_cmd_get(const char *ifname, const char *cmd_buf, const int cmd_len, void *reply, size_t *reply_len)
{
	return hostapd_ctrl_request(ifname, cmd_buf, cmd_len, reply, reply_len);
}

/** Get infos from hostapd by standard cmd via ctrl iface
 * Inputs:
 *   ifname:  interface/bss name
 *   cmd_buf: cmd and/or parameters
 *   cmd_len: length of cmd and/or parameters (not total buffer length).
 */
int hostapd_ctrl_cmd_set(const char *ifname, const char *cmd_buf, const int cmd_len)
{
	string_1024 reply;
	size_t reply_len = sizeof(reply);
	int ret = hostapd_ctrl_request(ifname, cmd_buf, cmd_len, reply, &reply_len);

	if (!ret && strncmp(reply, CLS_HOSTAPD_REPLY_OK, sizeof(CLS_HOSTAPD_REPLY_OK) - 1)) {
		return -CLSAPI_ERR_HOSTAPD;
	}

	return CLSAPI_OK;
}

/** Get infos from hostapd by CLS cmd via ctrl iface
 * Inputs:
 *   ifname:  interface/bss name
 *   cmd_buf: CLS cmd and/or parameters
 *   cmd_len: length of cmd and/or parameters (not total buffer length).
 *   reply:   buffer to carry reply infos
 *   repley_len: size of the reply buffer
 * Returns:
 *	 CLSAPI_OK: send and recv OK, "reply" carries cmd exec results from hostapd.
 *	 others:		send and recv Errors, no data in reply buffer.
 */
int hostapd_ctrl_clscmd_get(const char *ifname, const char *clscmd_buf, const int cmd_len, void *reply, size_t *reply_len)
{
	char *new_cmd;
	int ret, new_cmd_len;

	// new_cmd = "CLS " + clscmd_buf;
	new_cmd = malloc(sizeof(CLS_HOSTAPD_CMD_PREFIX) + cmd_len);
	if (!new_cmd)
		return -CLSAPI_ERR_NO_MEM;

	new_cmd_len = snprintf(new_cmd, sizeof(CLS_HOSTAPD_CMD_PREFIX) + cmd_len, "%s ", CLS_HOSTAPD_CMD_PREFIX);
	memcpy(new_cmd + new_cmd_len, clscmd_buf, cmd_len);
	ret = hostapd_ctrl_request(ifname, new_cmd, new_cmd_len, reply, reply_len);

	free(new_cmd);
	return ret;
}

/** Get infos from hostapd by CLS cmd via ctrl iface
 * Inputs:
 *   ifname:  interface/bss name
 *   cmd_buf: CLS cmd and/or parameters
 *   cmd_len: length of cmd and/or parameters (not total buffer length).
 */
int hostapd_ctrl_clscmd_set(const char *ifname, const char *clscmd_buf, const int cmd_len)
{
	char *new_cmd;
	int ret, new_cmd_len;
	string_1024 reply;
	size_t reply_len = sizeof(reply);

	// new_cmd = "CLS " + clscmd_buf;
	new_cmd = malloc(sizeof(CLS_HOSTAPD_CMD_PREFIX) + cmd_len);
	if (!new_cmd)
		return -CLSAPI_ERR_NO_MEM;

	new_cmd_len = snprintf(new_cmd, sizeof(CLS_HOSTAPD_CMD_PREFIX) + cmd_len, "%s ", CLS_HOSTAPD_CMD_PREFIX);
	memcpy(new_cmd + new_cmd_len, clscmd_buf, cmd_len);
	ret = hostapd_ctrl_request(ifname, new_cmd, new_cmd_len, reply, &reply_len);

	if (!ret && strncmp(reply, CLS_HOSTAPD_REPLY_OK, sizeof(CLS_HOSTAPD_REPLY_OK) - 1)) {
		DBG_ERROR("%s(ifname=%s cmd='%s' cmd_len=%d reply_len=%d reply='%s'\n",
				__func__, ifname, new_cmd, new_cmd_len, reply_len, reply);
		free(new_cmd);
		return -CLSAPI_ERR_HOSTAPD;
	}

	free(new_cmd);
	return CLSAPI_OK;
}

int hostapd_get_wps_status(const char *ifname, struct clsapi_wifi_wps_status *wps_status)
{
	int ret;
	int cmd_len;
	string_64 cmd;
	size_t reply_len = sizeof(struct clsapi_wifi_wps_status);

	cmd_len = snprintf(cmd, sizeof(cmd), "%s %s", CLS_HOSTAPD_CMD_PREFIX,
			CLS_HOSTAPD_CMD_GET_WPS_STATUS) + 1;

	ret = hostapd_ctrl_request(ifname, cmd, cmd_len, (char *)wps_status, &reply_len);
	return ret;
}

int hostapd_cancel_wps(const char *ifname)
{
	int ret;
	int cmd_len;
	string_64 cmd;
	string_1024 reply;
	size_t reply_len = sizeof(reply);

	cmd_len = snprintf(cmd, sizeof(cmd), "%s %s", CLS_HOSTAPD_CMD_PREFIX,
			CLS_HOSTAPD_CMD_WPS_CANCEL) + 1;

	ret = hostapd_ctrl_request(ifname, cmd, cmd_len, reply, &reply_len);
	if (ret < 0)
		return -CLSAPI_ERR_SYSTEM_ERROR;

	if (!strncmp(reply, CLS_HOSTAPD_REPLY_OK, sizeof(CLS_HOSTAPD_REPLY_OK) - 1))
		ret = CLSAPI_OK;
	else
		ret = -CLSAPI_ERR_HOSTAPD;

	return ret;
}

int hostapd_start_ap_wps_pbc(const char *ifname)
{
	int ret;
	int cmd_len;
	string_64 cmd;
	string_1024 reply;
	size_t reply_len = sizeof(reply);

	cmd_len = snprintf(cmd, sizeof(cmd), "%s %s", CLS_HOSTAPD_CMD_PREFIX,
			CLS_HOSTAPD_CMD_AP_WPS_PBC) + 1;

	ret = hostapd_ctrl_request(ifname, cmd, cmd_len, reply, &reply_len);
	if (ret < 0)
		return -CLSAPI_ERR_SYSTEM_ERROR;

	if (!strncmp(reply, CLS_HOSTAPD_REPLY_OK, sizeof(CLS_HOSTAPD_REPLY_OK) - 1))
		ret = CLSAPI_OK;
	else
		ret = -CLSAPI_ERR_HOSTAPD;

	return ret;
}

int wpad_start_sta_wps_pbc(const char *ifname)
{
	int ret;
	string_1024 reply;
	size_t reply_len = sizeof(reply);
	const char *wps_pbc = "WPS_PBC";

	ret = wpa_supplicant_ctrl_request(ifname, wps_pbc, strlen(wps_pbc), reply, &reply_len);
	if (ret < 0)
		return -CLSAPI_ERR_SYSTEM_ERROR;

	if (strncmp(reply, CLS_HOSTAPD_REPLY_FAIL, sizeof(CLS_HOSTAPD_REPLY_FAIL) - 1) == 0)
		ret = -CLSAPI_ERR_HOSTAPD;
	else
		ret = CLSAPI_OK;

	return ret;
}

int wpad_start_sta_wps_pin(const char *ifname, const char *macaddr, const char* pin)
{
	int ret;
	int cmd_len;
	string_64 cmd;
	string_1024 reply;
	size_t reply_len = sizeof(reply);

	cmd_len = snprintf(cmd, sizeof(cmd), "%s %s %s",
			CLS_HOSTAPD_CMD_AP_WPS_PIN, macaddr, pin) + 1;

	ret = wpa_supplicant_ctrl_request(ifname, cmd, cmd_len, reply, &reply_len);
	if (ret < 0)
		return -CLSAPI_ERR_SYSTEM_ERROR;

	if (strncmp(reply, CLS_HOSTAPD_REPLY_FAIL, sizeof(CLS_HOSTAPD_REPLY_FAIL) - 1) == 0)
		ret = -CLSAPI_ERR_HOSTAPD;
	else
		ret = CLSAPI_OK;

	return ret;
}

int hostapd_start_ap_wps_pin(const char *ifname, const char *uuid, const char* pin)
{
	int ret;
	int cmd_len;
	string_64 cmd;
	string_1024 reply;
	size_t reply_len = sizeof(reply);

	cmd_len = snprintf(cmd, sizeof(cmd), "%s %s %s",
			CLS_HOSTAPD_CMD_AP_WPS_PIN, uuid, pin) + 1;

	ret = hostapd_ctrl_request(ifname, cmd, cmd_len, reply, &reply_len);
	if (ret < 0)
		return -CLSAPI_ERR_SYSTEM_ERROR;

	if (!strncmp(reply, CLS_HOSTAPD_REPLY_OK, sizeof(CLS_HOSTAPD_REPLY_OK) - 1))
		ret = CLSAPI_OK;
	else
		ret = -CLSAPI_ERR_HOSTAPD;

	return ret;
}

int hostapd_disable_wifi_runtime_wps_ap_pin(const char *ifname)
{
	int ret;
	int cmd_len;
	string_64 cmd;
	string_64 reply;
	size_t reply_len = sizeof(reply);

	cmd_len = snprintf(cmd, sizeof(cmd), "%s disable",
			CLS_HOSTAPD_CMD_WPS_AP_PIN) + 1;

	ret = hostapd_ctrl_request(ifname, cmd, cmd_len, reply, &reply_len);
	if (ret < 0)
		return -CLSAPI_ERR_SYSTEM_ERROR;

	if (!strncmp(reply, CLS_HOSTAPD_REPLY_OK, sizeof(CLS_HOSTAPD_REPLY_OK) - 1))
		ret = CLSAPI_OK;
	else
		ret = -CLSAPI_ERR_HOSTAPD;

	return ret;
}

int hostapd_set_wifi_random_runtime_wps_ap_pin(const char *ifname, string_8 pin)
{
	int ret;
	int cmd_len;
	string_64 cmd;
	size_t reply_len = WPS_AP_PIN_MAX_LEN;

	cmd_len = snprintf(cmd, sizeof(cmd), "%s random",
			CLS_HOSTAPD_CMD_WPS_AP_PIN) + 1;

	ret = hostapd_ctrl_request(ifname, cmd, cmd_len, pin, &reply_len);
	if (ret < 0)
		return -CLSAPI_ERR_SYSTEM_ERROR;

	if (reply_len <= 0)
		return -CLSAPI_ERR_HOSTAPD;

	return ret;
}

int hostapd_get_channel_survey(const char *ifname, struct clsapi_channel_survey *channel_survey)
{
	int ret;
	int cmd_len;
	string_64 cmd;
	size_t reply_len = sizeof(struct clsapi_channel_survey);

	cmd_len = snprintf(cmd, sizeof(cmd), "%s %s", CLS_HOSTAPD_CMD_PREFIX,
			CLS_HOSTAPD_CMD_CHANEL_SURVEY) + 1;
	ret = hostapd_ctrl_request(ifname, cmd, cmd_len, (char *)channel_survey, &reply_len);

	return ret;
}

