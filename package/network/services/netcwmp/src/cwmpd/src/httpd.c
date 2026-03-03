/************************************************************************
 *                                                                      *
 * Netcwmp/Opencwmp Project                                             *
 * A software client for enabling TR-069 in embedded devices (CPE).     *
 *                                                                      *
 * Copyright (C) 2013-2014  netcwmp.netcwmp group                            *
 *                                                                      *
 * This program is free software; you can redistribute it and/or        *
 * modify it under the terms of the GNU General Public License          *
 * as published by the Free Software Foundation; either version 2       *
 * of the License, or (at your option) any later version.               *
 *                                                                      *
 * This program is distributed in the hope that it will be useful,      *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 * GNU General Public License for more details.                         *
 *                                                                      *
 * You should have received a copy of the GNU Lesser General Public     *
 * License along with this library; if not, write to the                *
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,          *
 * Boston, MA  02111-1307 USA                                           *
 *                                                                      *
 * Copyright 2013-2014  Mr.x(Mr.x) <netcwmp@gmail.com>          *
 *                                                                      *
 ***********************************************************************/

#include <cwmp/http.h>
#include <cwmp/event.h>
#include "cwmp_httpd.h"
#include "cwmp/stun.h"
#include "clsapi_base.h"

#define MAX_CLIENT_NUMS 8


static char * AuthRealm = "cwmpd";
static char * AuthQop = "auth";
static char   AuthOpaque[33] = {0};
static int	  AuthNonce = 0;

const char * RESPONSE_200 = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Length: 2\r\nContent-Type: text/xml; charset=\"utf-8\"\r\n\r\nOK";
const char * RESPONSE_400 = "HTTP/1.1 400 Bad request\r\nServer: CWMP-Agent\r\nConnection: close\r\nContent-Length: 5\r\n\r\nError";
const char * RESPONSE_401 = "HTTP/1.1 401 Unauthorized\r\nWWW-Authenticate: Digest qop=\"%s\",nonce=\"%s\",opaque=\"%s\",realm=\"%s\"\r\nServer: TR069Agent\r\nContent-Length: 0\r\n\r\n";


struct http_session_fd_t
{
    //int fd;
    time_t time;
    http_socket_t * sock;
};


struct http_session_fd_t sessionfd[MAX_CLIENT_NUMS];


void setnonblocking(int fd)
{
#ifdef WIN32
#else
    int opts;
    opts=fcntl(fd, F_GETFL);
    if (opts < 0)
    {
        cwmp_log_error("setnonblocking fcntl GETFL failed: fd(%d)\n", fd);
        return;
    }
    opts = opts | O_NONBLOCK;
    if (fcntl(fd, F_SETFL, opts) < 0)
    {
        cwmp_log_error("setnonblocking fcntl SETFL failed: fd(%d)\n", fd);
        return;
    }
    return;
#endif
}



int httpd_response_unauthorization(http_socket_t * sock)
{

    char buffer[256];
    char nonce[33];
    FUNCTION_TRACE();
    AuthNonce ++;
    TRsnprintf(buffer, 256,  "%d", AuthNonce);
    MD5(nonce, buffer, NULL);

    nonce[32] = 0;

    TRsnprintf(buffer, 256, RESPONSE_401, AuthQop, nonce, AuthOpaque, AuthRealm);


    return	http_socket_write(sock, buffer, TRstrlen(buffer));
}

int httpd_response_ok(http_socket_t * sock)
{
    FUNCTION_TRACE();
    return	http_socket_write(sock, RESPONSE_200, TRstrlen(RESPONSE_200));
}

int httpd_response_unkonw_error(http_socket_t * sock)
{
    FUNCTION_TRACE();
    return	http_socket_write(sock, RESPONSE_400, TRstrlen(RESPONSE_400));
}

int httpd_build_server(cwmp_t * cwmp)
{
    http_socket_t * lsnsock;
    pool_t * pool;
    int rc;
    int lsnfd, maxfd, nready;
    int i;

    int fd, newfd;
    http_socket_t * s;
    http_request_t * request;

    char * auth;
    time_t now;
    fd_set readset, rdset;
    struct timeval timeout;
    int port;

    char  cpe_user[INI_BUFFERSIZE] = {0};
    char  cpe_pwd[INI_BUFFERSIZE] = {0};



    port = cwmp->httpd_port;

    pool = pool_create(POOL_DEFAULT_SIZE);
    rc = http_socket_server(&lsnsock, port, 5, -1, pool);
    if (rc != CWMP_OK)
    {
        cwmp_log_error("build httpd server faild. %s\n", strerror(errno));
        exit(-1);
    }

    lsnfd = http_socket_get_fd(lsnsock);

    for (i=0; i < MAX_CLIENT_NUMS; i++)
    {

        sessionfd[i].time = 0;
        sessionfd[i].sock = NULL;
    }

    FD_ZERO(&readset);
    FD_SET(lsnfd, &readset);

    maxfd = lsnfd;
    /*maxi = -1;*/
    while (1)
    {
        FD_ZERO(&rdset);
        rdset = readset;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;
        if ((nready = select(maxfd + 1, &rdset, NULL, NULL, &timeout)) <= 0)
        {
            sleep(1);
            //cwmp_log_info("select new connection timeout. no new request.\n");
            now = time(NULL);
            for (i=0; i<MAX_CLIENT_NUMS; i++)
            {
                //cwmp_log_debug("socket time: %d, timeout %d, fd is %d\n", sessionfd[i].time, now -  sessionfd[i].time,
                //               sessionfd[i].sock == NULL? -1 : http_socket_get_fd(sessionfd[i].sock));
                fd = http_socket_get_fd(sessionfd[i].sock);
                if ((sessionfd[i].sock != NULL) && (now -  sessionfd[i].time > 15))
                {
                    cwmp_log_info("close a timeout socket. fd is %d.\n", fd);
                    FD_CLR(fd, &readset);
                    //http_socket_close(sessionfd[i].sock);
                    http_socket_destroy(sessionfd[i].sock);
                    sessionfd[i].time = 0;
                    sessionfd[i].sock = NULL;
                }
            }
            continue;
        }

        if (FD_ISSET(lsnfd, &rdset))
        {
            http_socket_t * newsock;
            //FIXME
            http_socket_accept(lsnsock, &newsock);
            newfd = http_socket_get_fd(newsock);

            for (i=0; i<MAX_CLIENT_NUMS; i++)
            {
                if (sessionfd[i].sock == NULL)
                {
                    sessionfd[i].sock = newsock;
                    sessionfd[i].time = time(NULL);
                    break;
                }
            }
            if (i == MAX_CLIENT_NUMS)
            {
                //http_socket_close(newsock);
                http_socket_destroy(newsock);

                cwmp_log_error("too many ACS request connection");
                continue;
            }
            FD_SET(newfd, &readset);
            if (newfd > maxfd)
            {
                maxfd = newfd;
            }

            newfd = -1;
            if (--nready <= 0)
            {
                continue;
            }

        }

        //readpool = pool_create(POOL_DEFAULT_SIZE);
        cwmp_log_debug("nready is %d.\n", nready);
        for (i=0; (i<MAX_CLIENT_NUMS) && (nready > 0) ; i++)
        {
            s = sessionfd[i].sock;
            fd = http_socket_get_fd(s);

            if ((fd != -1) && FD_ISSET(fd, &rdset))
            {
                nready--;
                sessionfd[i].time = time(NULL);
                http_request_create(&request, http_socket_get_pool(s));
                rc = http_read_request(s, request, http_socket_get_pool(s));
                if (rc <= 0)
                {
                    httpd_response_unkonw_error(s);
                    goto faild;
                }

                if (request->method != HTTP_GET)
                {
                    httpd_response_unkonw_error(s);
                    goto faild;
                }

                if (cwmp->cpe_auth)
                {
                    auth = http_get_variable(request->parser, "Authorization");

                    if (!auth)
                    {
                        httpd_response_unauthorization(s);
                        continue;
                    }


                    cwmp_conf_get("cwmp:cpe_username", cpe_user);
                    cwmp_conf_get("cwmp:cpe_password", cpe_pwd);

                    cwmp_log_debug("cpe username: %s, cpe password: %s\n", cpe_user, cpe_pwd);


                    if (http_check_digest_auth(AuthRealm, auth, cpe_user, cpe_pwd) != 0)
                    {
                        httpd_response_unauthorization(s);
						goto faild;
                    }
                }

                httpd_response_ok(s);

                //get a new request from acs
                cwmp->new_request = CWMP_YES;
				cwmp_log_debug("set cwmp new request to %d\n", cwmp->new_request);
				cwmp_event_set_value(cwmp, INFORM_CONNECTIONREQUEST, 1, NULL, 0, 0, 0);



faild:

                FD_CLR(fd, &readset);
                sessionfd[i].time = 0;
                sessionfd[i].sock = NULL;
                http_socket_destroy(s);


            }

        }



    }

}

#define delta_time(before, later)	((later.tv_sec) - (before.tv_sec))

static int get_local_ns_ip(unsigned int *local_ns_ip)
{
	int fd;
	struct ifreq ifr;
	char ifname[CWMP_STR_LEN_1025] = {0};

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
		return fd;
	ifr.ifr_addr.sa_family = AF_INET;
	clsapi_base_get_conf_param(UCI_CFG_NETWORK, UCI_SECTION_WAN, UCI_PARA_DEVICE, ifname);
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ-1);
	if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
		close(fd);
		return -1;
	}
	*local_ns_ip = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;

	close(fd);
	return 0;
}

static int open_stun_sock(const unsigned short udp_port)
{
	int sock;
	int opt = 1;
	struct sockaddr_in my_addr;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
		goto fail;
	if (get_local_ns_ip(&my_addr.sin_addr.s_addr) < 0)
		goto fail;

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(udp_port);
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {
		cwmp_log_error("setsockopt error\n");
		goto fail;
	}
	if (bind(sock, (struct sockaddr *) &my_addr, sizeof(my_addr)) < 0)
		goto fail;

	return sock;

fail:
	if (sock >= 0)
		close(sock);
	return -errno;
}

/* Per spec G.2.1.1 "If no STUNServerAddress is given, the address of the ACS determined from
 * the host portion of the ACS URL MUST be used as the STUN server address." */
static int set_stun_server_addr(cwmp_t *cwmp)
{

	struct hostent *he = gethostbyname(cwmp->stun.stun_conf.server_addr);
	if (he == NULL) {
		cwmp_log_error("ERROR: can't resolve STUN server addr %s, use host portion of ACS URL instead. Implement later\n",
			cwmp->stun.stun_conf.server_addr);
		return -1;
	}

	cwmp->stun.server_addr.sin_family	= AF_INET;
	memcpy(&(cwmp->stun.server_addr.sin_addr), he->h_addr_list[0], sizeof(struct in_addr));
	cwmp->stun.server_addr.sin_port		= htons(cwmp->stun.stun_conf.server_port);
	cwmp_log_debug("STUN server: %s:%d\n", inet_ntoa(cwmp->stun.server_addr.sin_addr),
		cwmp->stun.stun_conf.server_port);

	return 0;
}

#define stun_state_change(cwmp, new_state)	\
	do { \
		if (new_state > CWMP_STUN_STATE_MAX) {	\
			cwmp_log_error("Unknown state: %d", new_state);	\
			goto FAIL;	\
		}	\
		switch (new_state) {	\
		case CWMP_STUN_STATE_INIT:	\
		case CWMP_STUN_STATE_NAT_DISCOVER:	\
		case CWMP_STUN_STATE_NAT_TIMEOUT_DISCV:	\
		case CWMP_STUN_STATE_BINDING_CHANGED: \
			break;	\
		case CWMP_STUN_STATE_RUNNING:	\
			/* Inform STUN param value changing */ \
			cwmp_event_set_value(cwmp, INFORM_VALUECHANGE, EVENT_REF_UNTIL_REBOOTED, NULL, 0, 0, 0); \
			cwmp->new_request = CWMP_YES; \
			clock_gettime(CLOCK_MONOTONIC, &last_resp_time);	\
			break;	\
		}	\
		cwmp_log_info("\n[STUN] state %d ==> %d\n", cwmp->stun.state, new_state);	\
		cwmp->stun.state = new_state;	\
	} while (0)

#define stun_init()	\
	do { \
		cwmp->stun.nat_detected = -1;	\
		cwmp->stun.nat_timeout_val = 0;	\
		if (cwmp->stun.stun_conf.keepalive_retry_count == 0) \
			cwmp->stun.stun_conf.keepalive_retry_count = 5;	 \
		close(stun_sock);	\
		close(timeout_discv_sock);	\
		timeout_discv_sock = -1;	\
		memset(&cwmp->stun.server_addr, 0, sizeof(struct sockaddr_in));	\
		memset(&cwmp->stun.nat_mapped_addr, 0, sizeof(struct sockaddr_in));	\
		stun_sock = open_stun_sock(cwmp->udp_port);	\
		if (stun_sock < 0) {	\
			cwmp_log_error("[STUN] Failed in create socket\n\n");	\
			goto FAIL;	\
		}	\
		last_req_time.tv_sec = 0;	\
		nat_discv_retry_cnt = 0;	\
		keepalive_retry_count = 0;	\
		is_mapped_addr = 0;	\
		stun_state_change(cwmp, CWMP_STUN_STATE_NAT_DISCOVER);	\
	} while (0)

#define CWMP_STUN_NAT_DISCV_RETRY_MAX	10
#define CWMP_STUN_SELECT_TIMEOUT		2
#define CWMP_STUN_DISCV_INTERVAL		2
#define CWMP_STUN_RESP_MISS_MAX			3

int udp_connection_thread(cwmp_t * cwmp)
{
	int	ret	= 0, recv_bytes, msg_type, is_mapped_addr;
	int	nready = 0;
	int	nat_discv_retry_cnt = 0;
	int keepalive_retry_count = 0;
	int	stun_sock = -1, timeout_discv_sock = -1, maxfd;
	fd_set recv_set;
	struct timeval timeout;
	struct timespec now, last_req_time, last_resp_time;
	struct sockaddr_in nat_mapped_addr, recvfrom_addr;
	stun_req_attrs_t request_attrs;
	int sockaddr_in_len = sizeof(struct sockaddr_in);
	unsigned char msg_buf[1024];
	unsigned int local_ns_ip;
	int	binding_keepalive_intval = 0;
	int conn_state = 0;
	int timeout_discv_intval = 0;
	int timeout_discv_send = 0;
	int is_expired = 0;

	cwmp->stun.nat_detected = -1;
	timeout.tv_sec = CWMP_STUN_SELECT_TIMEOUT;
	timeout.tv_usec = 0;
	is_mapped_addr = 0;
	memset(&request_attrs, 0, sizeof(stun_req_attrs_t));
	conn_state = cwmp->conn_state;

	while (1) {
		if (conn_state != cwmp->conn_state) {
			stun_state_change(cwmp, CWMP_STUN_STATE_INIT);
			conn_state = cwmp->conn_state;
		}
		if (cwmp->stun.stun_conf.enable == 0 || cwmp->conn_state == 0) {
			sleep(5);
			continue;
		}

		/* Receive incoming STUN messages */
		FD_ZERO(&recv_set);
		maxfd = 0;
		request_attrs.username = cwmp->stun.stun_conf.username;
		request_attrs.password = cwmp->stun.stun_conf.password;
		if (stun_sock > 0) {
			FD_SET(stun_sock, &recv_set);
			maxfd = stun_sock + 1;
		}
		if (timeout_discv_sock > 0) {
			FD_SET(timeout_discv_sock, &recv_set);
			if (maxfd <= timeout_discv_sock)
				maxfd = timeout_discv_sock + 1;
		}
		if (stun_sock > 0 || timeout_discv_sock > 0) {
			nready = select(maxfd + 1, &recv_set, NULL, NULL, &timeout);
			if ( nready > 0) {
				if (stun_sock > 0 && FD_ISSET(stun_sock, &recv_set)) {
					recv_bytes = recvfrom(stun_sock, msg_buf, sizeof(msg_buf), MSG_DONTWAIT, (struct sockaddr*)&recvfrom_addr, (socklen_t *)&sockaddr_in_len);
					if (recv_bytes < 0) {
						continue;
					}
					//cwmp_log_debug("[STUN] recvfrom %s:%d %d bytes", inet_ntoa(recvfrom_addr.sin_addr), ntohs(recvfrom_addr.sin_port), recv_bytes);
					/* check STUN server for security */
					msg_type = get_stun_msg_type(msg_buf, recv_bytes);
					cwmp_log_debug("[STUN] new STUN messages: msg_type=0x%04x recvfrom %s:%d %d bytes", msg_type,
						inet_ntoa(recvfrom_addr.sin_addr), ntohs(recvfrom_addr.sin_port), recv_bytes);
					switch (msg_type) {
					case STUN_MSG_TYPE_BINDING_RESPONSE:
						cwmp_log_info("\n[STUN] Receive binding response ... \n");
						if (cwmp->stun.state == CWMP_STUN_STATE_RUNNING)
							keepalive_retry_count--;
						ret = get_mapped_addr(msg_buf, recv_bytes, NULL, NULL, &nat_mapped_addr);
						if (ret < 0) {
							cwmp_log_error("[STUN] Failed to get mapped address\n");
							break;
						}
						is_mapped_addr = 1;
						clock_gettime(CLOCK_MONOTONIC, &last_resp_time);
						cwmp_log_info("[STUN][%d] Mapped address %s:%d\n", last_resp_time.tv_sec, inet_ntoa(nat_mapped_addr.sin_addr),
							ntohs(nat_mapped_addr.sin_port));
						if (cwmp->stun.state == CWMP_STUN_STATE_NAT_TIMEOUT_DISCV)
							is_expired = 0;
						break;

					case STUN_MSG_TYPE_UDP_CONN_REQUEST:
						cwmp_log_info("\n[STUN] New UDP Connection Request ... \n");
						if (cwmp->stun.state == CWMP_STUN_STATE_RUNNING) {
							cwmp_event_set_value(cwmp, INFORM_CONNECTIONREQUEST, 1, NULL, 0, 0, 0);
							cwmp->new_request = CWMP_YES;
						}
						break;

					case STUN_MSG_TYPE_BINDING_ERR_RESP:
						break;

					default:
						cwmp_log_error("[STUN] Unkown message %d\n", msg_type);
					}
				}
				if (timeout_discv_sock > 0 && FD_ISSET(timeout_discv_sock, &recv_set)) {
					recv_bytes = recvfrom(timeout_discv_sock, msg_buf, sizeof(msg_buf), MSG_DONTWAIT, (struct sockaddr*)&recvfrom_addr, (socklen_t *)&sockaddr_in_len);
					if (recv_bytes < 0) {
						continue;
					}
					msg_type = get_stun_msg_type(msg_buf, recv_bytes);
					cwmp_log_debug("[STUN] new STUN messages: msg_type=0x%04x recvfrom %s:%d %d bytes", msg_type,
						inet_ntoa(recvfrom_addr.sin_addr), ntohs(recvfrom_addr.sin_port), recv_bytes);
					switch (msg_type) {
					case STUN_MSG_TYPE_BINDING_RESPONSE:
						cwmp_log_info("\n[STUN] Receive binding response on timeout_discv_sock ... \n");
						is_expired = 1;
						break;
					default:
						cwmp_log_error("[STUN] Unkown message %d\n", msg_type);
					}
				}
			}
			else
				sleep(1);
		}

		if (cwmp->stun.nat_detected == 0)
			continue;

		clock_gettime(CLOCK_MONOTONIC, &now);
		//cwmp_log_debug("[STUN] now %d", now.tv_sec);

		/* state machine */
		switch (cwmp->stun.state) {
		case CWMP_STUN_STATE_RUNNING:
			if (is_mapped_addr) {
				if (memcmp(&cwmp->stun.nat_mapped_addr, &nat_mapped_addr, sizeof(nat_mapped_addr)) != 0) {
					cwmp_log_info("[STUN] Mapped address changed to %s:%d!! \n", inet_ntoa(nat_mapped_addr.sin_addr),
						ntohs(nat_mapped_addr.sin_port));
					memcpy(&cwmp->stun.nat_mapped_addr, &nat_mapped_addr, sizeof(nat_mapped_addr));
					stun_state_change(cwmp, CWMP_STUN_STATE_BINDING_CHANGED);
				}
			}
			else {
				if (delta_time(last_resp_time, now) > binding_keepalive_intval * CWMP_STUN_RESP_MISS_MAX) {
					/* Binding Response missed too much, what should I do ?? */
					stun_state_change(cwmp, CWMP_STUN_STATE_INIT);
				}
			}

			/* Send Keep alive Binding Request periodically */
			if (delta_time(last_req_time, now) > binding_keepalive_intval) {
				//cwmp_log_info("[STUN] Sending Binding Request for Keep alive purpose ... ");
				if (keepalive_retry_count >= cwmp->stun.stun_conf.keepalive_retry_count) {
					cwmp_log_info("[STUN] keep alive failed %d times, change to init state", keepalive_retry_count);
					stun_state_change(cwmp, CWMP_STUN_STATE_INIT);
					break;
				}
				ret = tx_binding_request(stun_sock, &(cwmp->stun.server_addr), &request_attrs, cwmp->stun.state);
				keepalive_retry_count++;
				if (ret < 0)
					cwmp_log_error("[STUN] Send binding request for keep alive failed");
				clock_gettime(CLOCK_MONOTONIC, &last_req_time);
				cwmp_log_info("[STUN][%d] tx binding request for keep alive", last_req_time.tv_sec);
			}

			break;

		case CWMP_STUN_STATE_INIT:
			cwmp_log_debug("[STUN] Starting STUN thread ... \n");
			stun_init();

			break;

		case CWMP_STUN_STATE_NAT_DISCOVER:
			cwmp_log_debug("[STUN] Detecting NAT ...\n");
			cwmp_log_debug("[STUN] is_mapped_addr=%d\n", is_mapped_addr);
			if (is_mapped_addr) {
				/* is mapped address equal local address? */
				if (get_local_ns_ip(&local_ns_ip) < 0)
					goto FAIL;
				cwmp_log_debug("[STUN] Local IP/port: %s:%d", inet_ntoa(*(struct in_addr *)(&local_ns_ip)), cwmp->udp_port);
				if (local_ns_ip != nat_mapped_addr.sin_addr.s_addr || cwmp->udp_port != ntohs(nat_mapped_addr.sin_port)) {
					/* NAT detected */
					cwmp_log_info("[STUN] NAT Detected ... \n");
					cwmp->stun.nat_detected = 1;
					memcpy(&cwmp->stun.nat_mapped_addr, &nat_mapped_addr, sizeof(nat_mapped_addr));
					//stun_state_change(cwmp, CWMP_STUN_STATE_RUNNING);
					stun_state_change(cwmp, CWMP_STUN_STATE_NAT_TIMEOUT_DISCV);
				}
				else {
					/* No NAT, I have public IP and port */
					cwmp->stun.nat_detected = 0;
					stun_state_change(cwmp, CWMP_STUN_STATE_RUNNING);
				}

				break; /* no need to send Binding Request now */
			}

			if (cwmp->stun.server_addr.sin_port == 0) {
				ret = set_stun_server_addr(cwmp);
				if (ret < 0)
					goto FAIL;
			}

			/* interval between Binding Requests equals CWMP_STUN_SELECT_TIMEOUT */
			if (delta_time(last_req_time, now) > CWMP_STUN_DISCV_INTERVAL) {
				ret = tx_binding_request(stun_sock, &(cwmp->stun.server_addr), &request_attrs, cwmp->stun.state);
				if (ret < 0)
					goto FAIL;
				clock_gettime(CLOCK_MONOTONIC, &last_req_time);
				cwmp_log_info("[STUN][%d] tx binding request", last_req_time.tv_sec);
				if (nat_discv_retry_cnt++ > CWMP_STUN_NAT_DISCV_RETRY_MAX) {
					cwmp_log_error("Tried %d Binding Request but NO response received, quit", nat_discv_retry_cnt);
					stun_state_change(cwmp, CWMP_STUN_STATE_INIT);
				}
			}

			break;

		case CWMP_STUN_STATE_NAT_TIMEOUT_DISCV:
		{
			cwmp_log_debug("[STUN] Detect NAT timeout value ...\n");
			if (timeout_discv_sock < 0) {
				timeout_discv_sock = open_stun_sock(cwmp->udp_port+1);
				if (timeout_discv_sock < 0)
					goto FAIL;

				/* Send one Binding Request via primary port to refresh the timeout value */
				cwmp_log_debug("[STUN] Create timeout_discv_sock and send request on stun_sock\n");
				ret = tx_binding_request(stun_sock, &(cwmp->stun.server_addr), &request_attrs, cwmp->stun.state);
				if (ret < 0)
					goto FAIL;

				/* record the beginning time of NAT Timeout Discovery */
				cwmp->stun.nat_timeout_val = cwmp->stun.stun_conf.max_keepalive_period;
				timeout_discv_intval = cwmp->stun.stun_conf.max_keepalive_period/2;
				timeout_discv_send = 0;
			}
			if (cwmp->stun.nat_timeout_val - timeout_discv_intval <= 1) {
				/* NAT binding timeout detect done*/
				cwmp_log_info("[STUN] NAT timeout detect done base on time dichotomy\n");
				if (cwmp->stun.nat_timeout_val < cwmp->stun.stun_conf.min_keepalive_period) {
					cwmp_log_error("[STUN] NAT timeout %d short than minkeepalive %d",
						cwmp->stun.nat_timeout_val, cwmp->stun.stun_conf.min_keepalive_period);
					stun_state_change(cwmp, CWMP_STUN_STATE_INIT);
				}
				else if (cwmp->stun.nat_timeout_val < cwmp->stun.stun_conf.max_keepalive_period) {
					binding_keepalive_intval = cwmp->stun.nat_timeout_val;
				}
				else if (cwmp->stun.stun_conf.max_keepalive_period < cwmp->stun.nat_timeout_val) {
					binding_keepalive_intval = cwmp->stun.stun_conf.max_keepalive_period;
				}
				cwmp_log_info("[STUN] NAT timeout value: %d conf_keepalive [%d, %d], final intval %d",
					cwmp->stun.nat_timeout_val, cwmp->stun.stun_conf.min_keepalive_period,
					cwmp->stun.stun_conf.max_keepalive_period, binding_keepalive_intval);
				/* Primary NAT binding timeout, create it again and fetch new binding info ?? */
				/* Althrough NAT timeout, stun_sock is there, send Binding Request again will get new binding info
				 * and trigger CWMP_STUN_STATE_BINDING_CHANGED and Inform ACS */

				/* All discovery work done, tell ACS the binding info */
				stun_state_change(cwmp, CWMP_STUN_STATE_RUNNING);
				break;
			}
			if (!timeout_discv_send) {
				/* send Binding Request in another ports via STUN_ATTR_TYPE_RESPONSE_ADDR */
				if (delta_time(last_req_time, now) > timeout_discv_intval) {
					memcpy(&(request_attrs.resp_addr), &(cwmp->stun.nat_mapped_addr), sizeof(struct sockaddr_in));
					request_attrs.resp_addr.sin_port = nat_mapped_addr.sin_port;
					ret = tx_binding_request(timeout_discv_sock, &(cwmp->stun.server_addr), &request_attrs, cwmp->stun.state);
					if (ret < 0)
						goto FAIL;
					cwmp_log_debug("[STUN] Sending request on timeout_discv_sock\n");
					is_expired = 0;
					is_mapped_addr = 0;
					timeout_discv_send = 1;
				}
			} else {
				/* Check if client can receive Binding Response in primary port after sending on timeout_discv_sock
				 * nat binding is alive if received; otherwise, timeout
				 */
				if (is_mapped_addr) {
					/*If receive Binding Response in primary port after sending on timeout_discv_sock, NAT binding is alive */
					cwmp->stun.nat_timeout_val = timeout_discv_intval;
					cwmp_log_debug("nat timeout val:%d keepalive [%d, %d]", cwmp->stun.nat_timeout_val,
						cwmp->stun.stun_conf.min_keepalive_period, cwmp->stun.stun_conf.max_keepalive_period);
					timeout_discv_intval = timeout_discv_intval + (cwmp->stun.stun_conf.max_keepalive_period - timeout_discv_intval)/2;
					if (cwmp->stun.nat_timeout_val >= cwmp->stun.stun_conf.max_keepalive_period) {
						/* NAT timeout longer than max_keepalive_period, stop now */
						cwmp_log_info("[STUN] NAT timeout longer than max_keepalive_period, stop now\n");
						stun_state_change(cwmp, CWMP_STUN_STATE_RUNNING);
						binding_keepalive_intval = cwmp->stun.stun_conf.max_keepalive_period;
						cwmp_log_info("[STUN] NAT timeout value: %d conf_keepalive [%d, %d], final intval %d",
							cwmp->stun.nat_timeout_val, cwmp->stun.stun_conf.min_keepalive_period,
							cwmp->stun.stun_conf.max_keepalive_period, binding_keepalive_intval);
					} else {
						cwmp_log_info("[STUN] Receive Binding Response in primary port, start NAT detect with timeout:%d\n", timeout_discv_intval);
						ret = tx_binding_request(stun_sock, &(cwmp->stun.server_addr), &request_attrs, cwmp->stun.state);
						if (ret < 0)
							goto FAIL;
						/* record the beginning time of NAT Timeout Discovery */
						clock_gettime(CLOCK_MONOTONIC, &last_req_time);
						timeout_discv_send = 0;
					}
					break;
				}
				if (is_expired) {
					/*If receive Binding Response in timeout_discv_sock, NAT binding is timeout */
					cwmp_log_info("[STUN] NAT binding is timeout \n");
					cwmp->stun.nat_timeout_val = timeout_discv_intval;
					timeout_discv_intval = timeout_discv_intval - (timeout_discv_intval - cwmp->stun.stun_conf.min_keepalive_period)/2;
					cwmp_log_info("[STUN] Receive Binding Response on timeout_discv_sock, start NAT detect with timeout:%d\n", timeout_discv_intval);
					ret = tx_binding_request(stun_sock, &(cwmp->stun.server_addr), &request_attrs, cwmp->stun.state);
					if (ret < 0)
						goto FAIL;
					/* record the beginning time of NAT Timeout Discovery */
					clock_gettime(CLOCK_MONOTONIC, &last_req_time);
					timeout_discv_send = 0;
				}
			}
			break;
		}

		case CWMP_STUN_STATE_BINDING_CHANGED:
			//cwmp_log_debug("[STUN] Binding Info changed ...\n");
			//1 need discovery NAT timeout value again?
			stun_state_change(cwmp, CWMP_STUN_STATE_RUNNING);

			break;

		default:
			break;
		}
	}

FAIL:
	cwmp_log_error("[STUN] Exit STUN thread ... \n\n");
	memset(&cwmp->stun, 0, sizeof(cwmp->stun));
	cwmp->stun.nat_detected = -1;
	close(stun_sock);
	close(timeout_discv_sock);

	return -1;
}


