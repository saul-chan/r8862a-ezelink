/*
 * Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */

#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "wtp_ipc.h"

int capwap_enable;
int capwap_auth_enable;

static struct dl_list *hostapd_wtp_conn_list;
static int event_sock;

static int hostapd_wtp_connect(struct hostapd_wtp_conn_item *item)
{
	struct hostapd_wtp_conn *conn_sock = NULL;
	char *cfile = NULL;
	char *file = NULL;
	int flen = 0;
	int flags = 0;
	int conn_cnt = 0;

	if (!item)
		return -1;
	conn_sock = &item->conn_sock;
	conn_sock->sock = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (conn_sock->sock == -1)
		goto fail;
	memset(&conn_sock->local, 0, sizeof(struct sockaddr_un));
	conn_sock->local.sun_family = AF_UNIX;
	flen = strlen(HOSTAPD_DATA_PREFIX) + strlen(item->ifname) + 2;
	cfile = calloc(1, flen);
	if (!cfile)
		goto fail;
	snprintf(cfile, flen, HOSTAPD_DATA_PREFIX"_%s", item->ifname);
	if (access(cfile, F_OK) != -1) {
		wpa_printf(MSG_WARNING, "file %s existed unexcepted, delete it first\n", cfile);
		unlink(cfile);
	}

	strncpy(conn_sock->local.sun_path, cfile, sizeof(conn_sock->local.sun_path)-1);
	/* Set client socket file permissions so that bind() creates the client
	 * socket with these permissions and there is no need to try to change
	 * them with chmod() after bind() which would have potential issues with
	 * race conditions. These permissions are needed to make sure the server
	 * side (wpa_supplicant or hostapd) can reply to the control interface
	 * messages.
	 *
	 * The lchown() calls below after bind() are also part of the needed
	 * operations to allow the response to go through. Those are using the
	 * no-deference-symlinks version to avoid races.
	 */
	fchmod(conn_sock->sock, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	if (bind(conn_sock->sock, (struct sockaddr *)&conn_sock->local, sizeof(conn_sock->local)) < 0)
		goto fail;
	if (lchown(conn_sock->local.sun_path, -1, 101) == -1)
		wpa_printf(MSG_WARNING, "set group failed by lchown\n");
	if (lchown(conn_sock->local.sun_path, 101, 101) == -1)
		wpa_printf(MSG_WARNING, "set owner and group failed by lchown\n");

	flen = strlen(WTP_DATA_SERVER_PATH) + strlen(item->ifname) + 2;
	file = calloc(1, flen);
	if (!file)
		goto fail;
	snprintf(file, flen, WTP_DATA_SERVER_PATH"_%s", item->ifname);

	memset(&conn_sock->dest, 0, sizeof(struct sockaddr_un));
	conn_sock->dest.sun_family = AF_UNIX;
	strncpy(conn_sock->dest.sun_path, file, sizeof(conn_sock->dest.sun_path)-1);
try_again:
	if (connect(conn_sock->sock, (struct sockaddr *)&conn_sock->dest, sizeof(conn_sock->dest)) == -1) {
		conn_cnt++;
		if (conn_cnt < WTP_CONNECT_MAX_RETRY) {
			usleep(200000);
			goto try_again;
		} else {
			wpa_printf(MSG_ERROR, "connect to wtp data server failed\n");
			unlink(conn_sock->local.sun_path);
			goto fail;
		}
	}
	wpa_printf(MSG_DEBUG, "connect to wtp data server successfully\n");
	/*
	 * Make socket non-blocking so that we don't hang forever if
	 * target dies unexpectedly.
	 */
	flags = fcntl(conn_sock->sock, F_GETFL);
	if (flags >= 0) {
		flags |= O_NONBLOCK;
		if (fcntl(conn_sock->sock, F_SETFL, flags) < 0)
			wpa_printf(MSG_DEBUG, "set socket to O_NONBLOCK\n");
	}
	free(cfile);
	free(file);
	return 0;
fail:
	if (conn_sock->sock != -1)
		close(conn_sock->sock);
	if (cfile)
		free(cfile);
	if (file)
		free(file);
	return -1;
}

struct hostapd_wtp_conn *hostapd_wtp_conn_add(char *ifname)
{
	struct hostapd_wtp_conn_item *item = NULL;
	int ret = -1;

	dl_list_for_each(item, hostapd_wtp_conn_list, struct hostapd_wtp_conn_item, list) {
		if (strcmp(item->ifname, ifname) == 0)
			return &item->conn_sock;
	}

	item = calloc(1, sizeof(struct hostapd_wtp_conn_item));
	if (!item)
		return NULL;
	strncpy(item->ifname, ifname, sizeof(item->ifname)-1);
	ret = hostapd_wtp_connect(item);
	if (ret) {
		free(item);
		return NULL;
	}
	dl_list_add_tail(hostapd_wtp_conn_list, &item->list);
	return &item->conn_sock;
}

int hostapd_wtp_conn_del(char *ifname)
{
	struct hostapd_wtp_conn_item *item = NULL;

	dl_list_for_each(item, hostapd_wtp_conn_list, struct hostapd_wtp_conn_item, list) {
		if (strcmp(item->ifname, ifname) == 0) {
			dl_list_del(&item->list);
			if (item->conn_sock.sock >= 0)
				close(item->conn_sock.sock);
			unlink(item->conn_sock.local.sun_path);
			free(item);
			break;
		}
	}
	return 0;
}

int hostapd_send_frame_to_wtp(char *ifname, const struct ieee80211_mgmt *data, size_t len)
{
	struct hostapd_wtp_conn_item *item = NULL;
	struct hostapd_wtp_conn *conn_sock = NULL;
	int retry_cnt = 0;
	int ret = -1;
	char resp[WTP_RESPONSE_MAX_LEN] = {0};

	dl_list_for_each(item, hostapd_wtp_conn_list, struct hostapd_wtp_conn_item, list) {
		if (strcmp(item->ifname, ifname) == 0) {
			conn_sock = &item->conn_sock;
			break;
		}
	}
	if (!conn_sock) {
		wpa_printf(MSG_DEBUG, "Can't find connection for %s, add it.\n", ifname);
		conn_sock = hostapd_wtp_conn_add(ifname);
		if (!conn_sock) {
			wpa_printf(MSG_ERROR, "Add hostapd data connection for %s failed.\n", ifname);
			return ret;
		}
	}

	wpa_printf(MSG_DEBUG, "Receive hostapd mgmt fame for ifname=%s\n", ifname);
retry_send:
	ret = send(conn_sock->sock, data, len, 0);
	retry_cnt++;
	if (ret < 0) {
		if (errno == EAGAIN || errno == EBUSY || errno == EWOULDBLOCK) {
			/*No buffer, wait for 50ms and try again, max three times*/
			if (retry_cnt < WTP_CONNECT_MAX_RETRY) {
				usleep(50000);
				goto retry_send;
			}
		}
		wpa_printf(MSG_WARNING, "send frame to wtp failed: errno=%d\n", errno);
		return -1;
	}
	wpa_printf(MSG_DEBUG, "send data successfully\n");

	retry_cnt = 0;
retry_recv:
	ret = recv(conn_sock->sock, &resp, sizeof(resp)-1, 0);
	retry_cnt++;
	if (ret < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			if (retry_cnt < WTP_CONNECT_MAX_RETRY) {
				usleep(50000);
				goto retry_recv;
			}
		}
		wpa_printf(MSG_ERROR, "receive response from WTP failed: %s", strerror(errno));
		return ret;
	}
	wpa_printf(MSG_DEBUG, "receive response from WTP success: response=%s\n", resp);
	if (!strcmp(resp, RESPONSE_SUCCESS))
		ret = 0;
	else
		ret = -1;
	return ret;
}

struct dl_list *hostapd_wtp_data_socket_init(void)
{
	struct hostapd_wtp_conn_item *item = NULL;
	struct dirent *dent = NULL;
	DIR *dir = NULL;
	int ret = -1;

	hostapd_wtp_conn_list = calloc(1, sizeof(struct dl_list));
	if (!hostapd_wtp_conn_list)
		return NULL;
	dl_list_init(hostapd_wtp_conn_list);
	dir = opendir(HOSTAPD_DIR);
	if (!dir)
		return NULL;
	while ((dent = readdir(dir))) {
		if (strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0)
			continue;
		wpa_printf(MSG_DEBUG, "wtpIpcInit: select interface %s\n", dent->d_name);
		item = calloc(1, sizeof(struct hostapd_wtp_conn_item));
		if (!item)
			goto fail;
		strncpy(item->ifname, dent->d_name, sizeof(item->ifname)-1);
		ret = hostapd_wtp_connect(item);
		if (!ret)
			dl_list_add_tail(hostapd_wtp_conn_list, &item->list);
		else
			free(item);
	}
	closedir(dir);
	wpa_printf(MSG_DEBUG, "Create hostap data connection list successfully\n");
	return hostapd_wtp_conn_list;
fail:
	closedir(dir);
	dl_list_for_each(item, hostapd_wtp_conn_list, struct hostapd_wtp_conn_item, list) {
		dl_list_del(&item->list);
		os_free(item);
	}
	return NULL;
}

int hostapd_wtp_event_send(int event, char *data)
{
	struct hostapd_wtp_event_msg event_msg;
	int retry_cnt = 0;
	int ret = -1;
	char resp[WTP_RESPONSE_MAX_LEN] = {0};

	if (!event_sock) {
		wpa_printf(MSG_ERROR, "event socket isn't created\n");
		event_sock = hostapd_wtp_event_socket_init();
		if (event_sock < 0)
			return ret;
	}

	memset(&event_msg, 0, sizeof(event_msg));
	event_msg.event = event;
	if (data)
		strcpy(event_msg.data, data);

retry_send:
	ret = send(event_sock, &event_msg, sizeof(event_msg), 0);
	retry_cnt++;
	if (ret < 0) {
		if (errno == EAGAIN || errno == EBUSY || errno == EWOULDBLOCK) {
			/*No buffer, wait for 100ms and try again, max three times*/
			if (retry_cnt < WTP_CONNECT_MAX_RETRY) {
				usleep(100000);
				goto retry_send;
			}
		}
		wpa_printf(MSG_WARNING, "send event to wtp failed: errno=%d\n", errno);
		return -1;
	}

	retry_cnt = 0;
retry_recv:
	ret = recv(event_sock, &resp, sizeof(resp)-1, 0);
	retry_cnt++;
	if (ret < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			if (retry_cnt < WTP_CONNECT_MAX_RETRY) {
				usleep(50000);
				goto retry_recv;
			}
		}
		wpa_printf(MSG_ERROR, "receive response from WTP failed: %s", strerror(errno));
		return ret;
	}
	wpa_printf(MSG_DEBUG, "receive response from WTP success: response=%s\n", resp);
	if (!strcmp(resp, RESPONSE_SUCCESS))
		ret = 0;
	else
		ret = -1;
	return ret;
}

int hostapd_wtp_event_socket_init(void)
{
	struct sockaddr_un local;
	struct sockaddr_un dest;
	int flags = 0;
	int conn_cnt = 0;

	event_sock = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (event_sock == -1)
		return -1;
	memset(&local, 0, sizeof(struct sockaddr_un));
	local.sun_family = AF_UNIX;
	if (access(HOSTAPD_EVENT_PATH, F_OK) != -1) {
		wpa_printf(MSG_WARNING, "file %s existed unexcepted, delete it first\n", HOSTAPD_EVENT_PATH);
		unlink(HOSTAPD_EVENT_PATH);
	}

	strncpy(local.sun_path, HOSTAPD_EVENT_PATH, sizeof(local.sun_path)-1);
	/* Set client socket file permissions so that bind() creates the client
	 * socket with these permissions and there is no need to try to change
	 * them with chmod() after bind() which would have potential issues with
	 * race conditions. These permissions are needed to make sure the server
	 * side (wpa_supplicant or hostapd) can reply to the control interface
	 * messages.
	 *
	 * The lchown() calls below after bind() are also part of the needed
	 * operations to allow the response to go through. Those are using the
	 * no-deference-symlinks version to avoid races.
	 */
	fchmod(event_sock, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	if (bind(event_sock, (struct sockaddr *)&local, sizeof(local)) < 0)
		goto fail;
	if (lchown(local.sun_path, -1, 101) == -1)
		wpa_printf(MSG_WARNING, "set group failed by lchown\n");
	if (lchown(local.sun_path, 101, 101) == -1)
		wpa_printf(MSG_WARNING, "set owner and group failed by lchown\n");

	memset(&dest, 0, sizeof(struct sockaddr_un));
	dest.sun_family = AF_UNIX;
	strncpy(dest.sun_path, WTP_EVENT_SERVER_PATH, sizeof(dest.sun_path)-1);
try_again:
	if (connect(event_sock, (struct sockaddr *)&dest, sizeof(dest)) == -1) {
		conn_cnt++;
		if (conn_cnt < WTP_CONNECT_MAX_RETRY) {
			usleep(200000);
			goto try_again;
		} else {
			wpa_printf(MSG_ERROR, "connect to wtp event server failed\n");
			unlink(local.sun_path);
			goto fail;
		}
	}
	wpa_printf(MSG_DEBUG, "connect to wtp event server successfully\n");
	/*
	 * Make socket non-blocking so that we don't hang forever if
	 * target dies unexpectedly.
	 */
	flags = fcntl(event_sock, F_GETFL);
	if (flags >= 0) {
		flags |= O_NONBLOCK;
		if (fcntl(event_sock, F_SETFL, flags) < 0)
			wpa_printf(MSG_DEBUG, "set socket to O_NONBLOCK\n");
	}
	return event_sock;
fail:
	if (event_sock != -1)
		close(event_sock);
	event_sock = 0;
	return -1;
}

int hostapd_wtp_eapol_socket_init(struct wpa_state_machine *sm)
{
	int client_socket;
	struct sockaddr_un server;
	char fpath[MAX_FILE_PATH] = {0};
	int flags = 0;

	sm->eapol_socket = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (sm->eapol_socket < 0) {
		wpa_printf(MSG_ERROR, "create wtp eapol socket errror");
		return -1;
	}

	memset(&server, 0, sizeof(server));
	server.sun_family = AF_UNIX;
	sprintf(fpath, "%s_"COMPACT_MACSTR, STA_EAPOL_PATH, MAC2STR(sm->addr));
	strcpy(server.sun_path, fpath);

	if (bind(sm->eapol_socket, (struct sockaddr *) &server, sizeof(server)) < 0) {
		wpa_printf(MSG_WARNING, "eapol socket bind(PF_UNIX) failed: %s", strerror(errno));
		if (connect(sm->eapol_socket, (struct sockaddr *) &server, sizeof(server)) < 0) {
			wpa_printf(MSG_WARNING, "eapol socket exists, but does not allow connections - ");
			wpa_printf(MSG_WARNING, "assuming it was left over from forced program termination");
			if (unlink(fpath) < 0) {
				wpa_printf(MSG_WARNING, "Couldn't unlink eapol socket '%s': %s",
					fpath, strerror(errno));
				goto fail;
			}
			if (bind(sm->eapol_socket, (struct sockaddr *) &server, sizeof(server)) < 0) {
				wpa_printf(MSG_ERROR, "eapol socket: bind(PF_UNIX): %s", strerror(errno));
				goto fail;
			}
			wpa_printf(MSG_DEBUG, "Successfully replaced leftover eapol socket '%s'", fpath);
		} else {
			wpa_printf(MSG_ERROR, "eapol socket exists and seems to be in use - cannot override it");
			wpa_printf(MSG_ERROR, "Delete '%s' manually if it is not used anymore", fpath);
			goto fail;
		}
	}

	if (chmod(fpath, 0770) < 0) { /*0770: S_IRWXU | S_IRWXG*/
		wpa_printf(MSG_ERROR, "chmod %s: %s", fpath, strerror(errno));
		goto fail;
	}

	/*
	 * Make socket non-blocking so that we don't hang forever if
	 * target dies unexpectedly.
	 */
	flags = fcntl(sm->eapol_socket, F_GETFL);
	if (flags >= 0) {
		flags |= O_NONBLOCK;
		if (fcntl(sm->eapol_socket, F_SETFL, flags) < 0)
			wpa_printf(MSG_ERROR, "set socket to O_NONBLOCK\n");
	}
	wpa_printf(MSG_DEBUG, "eapol socket is created successfully");
	return 0;
fail:
	if (sm->eapol_socket >= 0) {
		close(sm->eapol_socket);
		sm->eapol_socket = -1;
	}
	unlink(fpath);
	return -1;
}

int hostapd_send_eapol_to_wtp(struct wpa_state_machine *sm, const u8 *data, size_t data_len)
{
	struct sockaddr_un from;
	socklen_t fromlen;

	memcpy(&from, &(sm->wtp_eapol_sock), sizeof(struct sockaddr_un));
	fromlen = sm->wtp_eapol_socklen;

	wpa_printf(MSG_ERROR, "client socket: family=%d, path=%s", from.sun_family, from.sun_path);
	if (sendto(sm->eapol_socket, data, data_len, 0, (struct sockaddr *)&from, fromlen) < 0) {
		wpa_printf(MSG_ERROR, "send eapol to WTP failed: %s", strerror(errno));
		return -1;
	}
	wpa_printf(MSG_DEBUG, "send eapol to wtp successfully");
	return 0;
}

size_t hostapd_receive_eapol_from_wtp(struct wpa_state_machine *sm, char *data)
{
	int retry_cnt = 0;
	int len = 0;
	struct sockaddr_un from;
	socklen_t fromlen;
	char buf[WTP_RESPONSE_MAX_LEN + 1] = {0};

retry_recv:
	fromlen = sizeof(from);
	len = recvfrom(sm->eapol_socket, buf, WTP_RESPONSE_MAX_LEN, 0, (struct sockaddr *)&from, &fromlen);
	retry_cnt++;
	if (len <= 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			if (retry_cnt < 2*WTP_CONNECT_MAX_RETRY) {
				usleep(100000);
				goto retry_recv;
			}
		}
		wpa_printf(MSG_ERROR, "receive eapol from WTP failed: %d(%s)", errno, strerror(errno));
		return -1;
	}
	wpa_printf(MSG_DEBUG, "receive eapol from WTP success: %d(bytes)", len);
	memcpy(&(sm->wtp_eapol_sock), &from, sizeof(struct sockaddr_un));
	sm->wtp_eapol_socklen = fromlen;
	if (len > 0) {
		memcpy(data, buf, len);
		wpa_hexdump(MSG_DEBUG, "receive eapol from WTP:", data, len);
	}
	return len;
}

