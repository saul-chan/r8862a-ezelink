#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <uloop.h>
#include <netinet/if_ether.h>
#include <linux/if_packet.h>
#include <net/if_arp.h>
#include <net/ethernet.h>
#include "vbss.h"
#include "clsapi_wifi.h"


static int sockfd;

static int vbss_sendto_msg(struct sockaddr_in *peer_addr, char *msg_buf, int buf_len)
{
	printf("%s() send reply with len=%d to %s\n\n", __func__, buf_len, inet_ntoa(peer_addr->sin_addr));
	return sendto(sockfd, msg_buf, buf_len, 0, (struct sockaddr *)peer_addr, sizeof(struct sockaddr));
}


static inline int vbss_sendto_err(struct sockaddr_in *peer_addr, const int err_code, const char *err)
{
	int len;
	char reply[128];

	len = snprintf(reply, sizeof(reply), "%s: code=%d error %s", MSG_ERROR, err_code, err?err:"");
	return vbss_sendto_msg(peer_addr, reply, len);
}


// #define MSG_AGENT_PING		"ping"
	/* Reqest msg format:	"ping"
	 * Reply msg format:	"pong"
	 */
static int handle_msg_ping(struct sockaddr_in *peer_addr, char *msg_buf, int msg_len)
{
	printf("%s(msg_len=%d msg_buf='%s')\n", __func__, msg_len, msg_buf);

	return vbss_sendto_msg(peer_addr, "pong", sizeof("pong"));
}


// #define MSG_AGENT_ENABLE_VBSS	"enable_vbss"
	/* Reqest msg format:	"enable_vbss <primary ifname> <0 | 1>" + '\0'
	 * Reply msg format:	"OK" or "FAIL"
	 */
static int handle_msg_enable_vbss(struct sockaddr_in *peer_addr, char *msg_buf, int msg_len)
{
	int ret;
	char *ifname = msg_buf + sizeof(MSG_AGENT_ENABLE_VBSS);
	char *pos = strstr(ifname, " ");
	char *onoff;

	*pos = '\0';
	onoff = pos + 1;

	printf("%s(msg_len=%d msg_buf='%s') ifname=%s onoff=%s\n",
			__func__, msg_len, msg_buf, ifname, onoff);

	ret = clsapi_wifi_set_vbss_enabled(ifname, atoi(onoff));
	if (ret < 0)
		return vbss_sendto_err(peer_addr, ret, NULL);

	return vbss_sendto_msg(peer_addr, MSG_OK, sizeof(MSG_OK));
}


//#define MSG_AGENT_GET_VAP_INFO	"get_vap_info"
	/* Reqest msg format:	"get_vap_info <ifname>"
	 * Reply msg format:	<bin of vap_info>
	 */
static int handle_msg_get_vap_info(struct sockaddr_in *peer_addr, char *msg_buf, int msg_len)
{
	int ret;
	char *ifname = msg_buf + sizeof(MSG_AGENT_GET_VAP_INFO);
	struct vbss_vap_info vap_info;

	printf("%s(msg_len=%d msg_buf='%s') ifname=%s\n",
			__func__, msg_len, msg_buf, ifname);

	ret = clsapi_wifi_set_vbss_stop_rekey(ifname);
	if (ret < 0)
		return vbss_sendto_err(peer_addr, ret, NULL);
	ret = clsapi_wifi_get_vbss_vap(ifname, &vap_info);
	if (ret < 0)
		return vbss_sendto_err(peer_addr, ret, NULL);

	return vbss_sendto_msg(peer_addr, (char *)&vap_info, sizeof(vap_info));
}


// MSG_AGENT_ADD_VAP_INFO	"add_vap_info"
	/* Reqest msg format:	"add_vap_info ifname" + '\0' + <bin of vap_info>
	 * Reply msg format:	"OK <ifname>" or "FAIL <error msg>"
	 */
static int handle_msg_add_vap_info(struct sockaddr_in *peer_addr, char *msg_buf, int msg_len)
{
	int ret;
	char new_ifname[128], reply[256];
	struct vbss_vap_info *vap_info;

	strncpy(new_ifname, msg_buf + sizeof(MSG_AGENT_ADD_VAP_INFO), sizeof(new_ifname)-1);
	vap_info = (struct vbss_vap_info *)(msg_buf + sizeof(MSG_AGENT_ADD_VAP_INFO) + strlen(new_ifname) + 1);

	printf("%s(msg_len=%d msg_buf='%s') new_ifname=%s VAP BSSID " MACFMT "\n",
			__func__, msg_len, msg_buf, new_ifname, MACARG(vap_info->bssid));

	ret = clsapi_wifi_add_vbss_vap(vap_info, new_ifname, sizeof(new_ifname));
	if (ret < 0)
		return vbss_sendto_err(peer_addr, ret, NULL);

	ret = snprintf(reply, sizeof(reply), "%s %s", MSG_OK, new_ifname);
	return vbss_sendto_msg(peer_addr, reply, strlen(reply) + 1);
}


// #define MSG_AGENT_DEL_VAP	"del_vap"
	/* Reqest msg format:	"del_vap <ifname>" + '\0'
	 * Reply msg format:	"OK" or "FAIL"
	 */
static int handle_msg_del_vap(struct sockaddr_in *peer_addr, char *msg_buf, int msg_len)
{
	int ret;
	char *ifname = msg_buf + sizeof(MSG_AGENT_DEL_VAP);

	printf("%s(msg_len=%d msg_buf='%s') ifname=%s\n",
			__func__, msg_len, msg_buf, ifname);

	ret = clsapi_wifi_del_vbss_vap(ifname);
	if (ret < 0)
		return vbss_sendto_err(peer_addr, ret, NULL);

	return vbss_sendto_msg(peer_addr, MSG_OK, sizeof(MSG_OK));
}

/**
 * @brief send ARP request
 * param_in interface interface to send
 * param_in sta_mac MAC address of station
 * param_in ac_ip IP of AC
 * param_in station_ip IP of station
 * return 0 or -1 if fail
 */
static int send_arp_request(char *interface, uint8_t *sta_mac, uint32_t ac_ip, uint32_t sta_ip)
{
	int sockfd;
	uint8_t broadcast_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	struct sockaddr_ll dest_addr;
	char sendbuff[50] = {0};
	struct ethhdr *eth = (struct ethhdr *)(sendbuff);
#define ARP_REQUEST_PKT_LEN 42

	if ((sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1) {
		perror("socket");
		return 1;
	}

	// ether header
	memcpy(eth->h_source, sta_mac, ETH_ALEN);
	memcpy(eth->h_dest, broadcast_mac, ETH_ALEN);
	eth->h_proto = htons(ETH_P_ARP);

	// arp header
	struct arphdr *arph = (struct arphdr*)(sendbuff + sizeof(struct ethhdr));
	arph->ar_hrd = htons(1);
	arph->ar_pro = htons(ETH_P_IP);
	arph->ar_hln = ETH_ALEN;
	arph->ar_pln = 4;
	arph->ar_op = htons(ARPOP_REQUEST);

	// arp body
	unsigned char *arp_body = (unsigned char *)arph + sizeof(struct arphdr);
	memcpy(arp_body, sta_mac, ETH_ALEN);
	arp_body += ETH_ALEN;
	*(unsigned int *)arp_body = sta_ip;
	arp_body += 4;
	arp_body += ETH_ALEN;
	*(unsigned int *)arp_body = ac_ip;

	dest_addr.sll_halen = ETH_ALEN;
	dest_addr.sll_ifindex = if_nametoindex(interface);

	if (sendto(sockfd, sendbuff, ARP_REQUEST_PKT_LEN, 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr)) == -1) {
		perror("sendto");
		return 1;
	}

	close(sockfd);
	return 0;
}


//#define MSG_AGENT_GET_STA_INFO	"get_sta_info"
	/* Reqest msg format:	"get_sta_info <ifname>" + '\0' + <sta_mac in hexbin>
	 * Reply msg format:	<bin of sta_info>
	 */
static int handle_msg_get_sta_info(struct sockaddr_in *peer_addr, char *msg_buf, int msg_len)
{
	int ret;
	struct vbss_sta_info sta_info;
	char *ifname = msg_buf + sizeof(MSG_AGENT_GET_STA_INFO);
	uint8_t *sta_mac = (uint8_t *)ifname + strlen(ifname) + 1;

	printf("%s(msg_len=%d msg_buf='%s') ifname=%s STA MAC " MACFMT "\n",
			__func__, msg_len, msg_buf, ifname, MACARG(sta_mac));

	printf("Stop TXQ of VBSS VAP (ifname=%s) ...\n", ifname);
	ret = clsapi_wifi_stop_vbss_vap_txq(ifname);
	if (ret < 0)
		return vbss_sendto_err(peer_addr, ret, NULL);

	ret = clsapi_wifi_get_vbss_sta(ifname, sta_mac, &sta_info);
	if (ret < 0)
		return vbss_sendto_err(peer_addr, ret, NULL);

	return vbss_sendto_msg(peer_addr, (char *)&sta_info, sizeof(sta_info));
}

// MSG_AGENT_PKT_TO_AC "send fake pkt to ac"
	/* Reqest msg format:	"send fake pkt to ac <ifname>" + '\0' + <sta_mac> + sta_ip
	 * Reply msg format:	"OK"
	 */
static int fake_arp_request_to_ac(struct sockaddr_in *peer_addr, char *msg_buf, int msg_len)
{
	char *ifname = msg_buf + sizeof(MSG_AGENT_PKT_TO_AC);
	uint8_t *sta_mac = (uint8_t *)ifname + strlen(ifname) + 1;
	uint32_t sta_ip;
	// read from ip link to all eth*
	char *cmd_get_eth= "ip link|grep -w eth. |awk -F': ' '{print$2}'";
	FILE *tmp_fp = popen(cmd_get_eth, "r");
	char eth_if[8] = {0};

	memcpy(&sta_ip, sta_mac + ETH_ALEN, sizeof(sta_ip));
	printf("%s(msg_len=%d msg_buf='%s') ifname=%s ip: %x STA MAC " MACFMT "\n",
			__func__, msg_len, msg_buf, ifname, sta_ip, MACARG(sta_mac));

	if (sta_ip == 0) // no sta ip found, get it from my arp cache
		get_ip_from_arp_cache(&sta_ip, sta_mac);

	while (fgets(eth_if, sizeof(eth_if), tmp_fp) != NULL) {
		eth_if[strcspn(eth_if, "\n")] = '\0';
		printf("%s sending ARP request\n", eth_if);
		send_arp_request(eth_if, sta_mac, peer_addr->sin_addr.s_addr, sta_ip);
	}

	pclose(tmp_fp);
	return vbss_sendto_msg(peer_addr, MSG_OK, sizeof(MSG_OK));
}

// MSG_AGENT_ADD_STA_INFO	"add_sta_info"
	/* Reqest msg format:	"add_sta_info <ifname>" + '\0' + <bin of sta_info>
	 * Reply msg format:	"OK" or "FAIL <error msg>"
	 */
static int handle_msg_add_sta_info(struct sockaddr_in *peer_addr, char *msg_buf, int msg_len)
{
	int ret;
	char *pos = msg_buf + sizeof(MSG_AGENT_ADD_STA_INFO);
	char *ifname = pos;
	struct vbss_sta_info *sta_info;

	sta_info = (struct vbss_sta_info *)(pos + strlen(ifname) + 1);

	printf("%s(msg_len=%d msg_buf='%s') ifname=%s STA MAC " MACFMT "\n",
			__func__, msg_len, msg_buf, ifname, MACARG(sta_info->mac_addr));

	ret = clsapi_wifi_add_vbss_sta(ifname, sta_info);
	if (ret < 0)
		return vbss_sendto_err(peer_addr, ret, NULL);

	return vbss_sendto_msg(peer_addr, MSG_OK, sizeof(MSG_OK));
}


//#define MSG_AGENT_TRG_SWITCH	"trigger_switch"
	/* Reqest msg format:	"trigger_switch <ifname> <STA MAC in 112233445566>"
	 * Reply msg format:	"OK" or "FAIL"
	 */
static int handle_msg_trigger_switch(struct sockaddr_in *peer_addr, char *msg_buf, int msg_len)
{
	int ret;
	char *pos = msg_buf + sizeof(MSG_AGENT_TRG_SWITCH);
	char *ifname = pos;
	uint8_t *sta_mac = (uint8_t *)(pos + strlen(ifname) + 1);

	printf("%s(msg_len=%d msg_buf='%s') ifname=%s STA MAC " MACFMT "\n",
			__func__, msg_len, msg_buf, ifname, MACARG(sta_mac));
	ret = clsapi_wifi_trigger_vbss_switch(ifname, sta_mac);
	if (ret < 0)
		return vbss_sendto_err(peer_addr, ret, NULL);

	return vbss_sendto_msg(peer_addr, MSG_OK, sizeof(MSG_OK));
}


//#define MSG_AGENT_SWITCH_DONE	"switch_done"
	/* Reqest msg format:	"switch_done <ifname> <STA MAC in 112233445566>"
	 * Reply msg format:	"OK" or "FAIL"
	 */
static int handle_msg_switch_done(struct sockaddr_in *peer_addr, char *msg_buf, int msg_len)
{
	int ret;
	char *pos = msg_buf + sizeof(MSG_AGENT_SWITCH_DONE);
	char *ifname = pos;
	uint8_t *sta_mac = (uint8_t *)(pos + strlen(ifname) + 1);

	printf("%s(msg_len=%d msg_buf='%s') ifname=%s STA MAC " MACFMT "\n",
			__func__, msg_len, msg_buf, ifname, MACARG(sta_mac));
	ret = clsapi_wifi_set_vbss_switch_done(ifname, sta_mac);
	if (ret < 0)
		return vbss_sendto_err(peer_addr, ret, NULL);

	return vbss_sendto_msg(peer_addr, MSG_OK, sizeof(MSG_OK));
}


static int vbss_recv_msg(struct sockaddr_in *peer_addr, char *msg_buf, const int msg_len)
{
	int ret;
	if (msg_len <=  0)
		return 0;

	printf("Rx from %s msg: '%s' len=%d\n", inet_ntoa(peer_addr->sin_addr), msg_buf, msg_len);
	if (strncmp(msg_buf, MSG_AGENT_PING, sizeof(MSG_AGENT_PING) - 1) == 0) {
		ret = handle_msg_ping(peer_addr, msg_buf, msg_len);
	}
	else if (strncmp(msg_buf, MSG_AGENT_ENABLE_VBSS, sizeof(MSG_AGENT_ENABLE_VBSS) - 1) == 0) {
		ret = handle_msg_enable_vbss(peer_addr, msg_buf, msg_len);
	}
	else if (strncmp(msg_buf, MSG_AGENT_GET_VAP_INFO, sizeof(MSG_AGENT_GET_VAP_INFO) - 1) == 0) {
		ret = handle_msg_get_vap_info(peer_addr, msg_buf, msg_len);
	}
	else if (strncmp(msg_buf, MSG_AGENT_ADD_VAP_INFO, sizeof(MSG_AGENT_ADD_VAP_INFO) - 1) == 0) {
		ret = handle_msg_add_vap_info(peer_addr, msg_buf, msg_len);
	}
	else if (strncmp(msg_buf, MSG_AGENT_DEL_VAP, sizeof(MSG_AGENT_DEL_VAP) - 1) == 0) {
		ret = handle_msg_del_vap(peer_addr, msg_buf, msg_len);
	}
	else if (strncmp(msg_buf, MSG_AGENT_GET_STA_INFO, sizeof(MSG_AGENT_GET_STA_INFO) - 1) == 0) {
		ret = handle_msg_get_sta_info(peer_addr, msg_buf, msg_len);
	}
	else if (strncmp(msg_buf, MSG_AGENT_PKT_TO_AC, sizeof(MSG_AGENT_PKT_TO_AC) - 1) == 0) {
		ret = fake_arp_request_to_ac(peer_addr, msg_buf, msg_len);
	}
	else if (strncmp(msg_buf, MSG_AGENT_ADD_STA_INFO, sizeof(MSG_AGENT_ADD_STA_INFO) - 1) == 0) {
		ret = handle_msg_add_sta_info(peer_addr, msg_buf, msg_len);
	}
	else if (strncmp(msg_buf, MSG_AGENT_TRG_SWITCH, sizeof(MSG_AGENT_TRG_SWITCH) - 1) == 0) {
		ret = handle_msg_trigger_switch(peer_addr, msg_buf, msg_len);
	}
	else if (strncmp(msg_buf, MSG_AGENT_SWITCH_DONE, sizeof(MSG_AGENT_SWITCH_DONE) - 1) == 0) {
		ret = handle_msg_switch_done(peer_addr, msg_buf, msg_len);
	}

	return ret;
}


static void handle_socket_event(struct uloop_fd *fd, unsigned int events)
{
	char msg_buf[1024];
	struct sockaddr_in peer_addr;
	int msg_len;
	unsigned int peer_addr_len;

	if (events & ULOOP_READ) {
		peer_addr_len = sizeof(peer_addr);
		msg_len = recvfrom(sockfd, msg_buf, sizeof(msg_buf), 0, (struct sockaddr *)&peer_addr, &peer_addr_len);
		if (msg_len > 0) {
			vbss_recv_msg(&peer_addr, msg_buf, msg_len);
		}
	}
}


/*
* argv[0]: vbss_agent
*/
int main(int argc, char **argv)
{
	struct sockaddr_in agent_addr;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		perror("socket");
		exit(1);
	}

	agent_addr.sin_family = AF_INET;
	agent_addr.sin_port = htons(VBSS_AGENT_PORT);
	agent_addr.sin_addr.s_addr = INADDR_ANY;
	if (bind(sockfd, (struct sockaddr*)&agent_addr, sizeof(agent_addr)) < 0) {
		perror("bind");
		exit(1);
	}

	uloop_init();

	struct uloop_fd *fd = malloc(sizeof(struct uloop_fd));
	if (!fd) {
		perror("malloc");
		exit(1);
	}
	fd->fd = sockfd;
	fd->cb = handle_socket_event;
	fd->eof = 0;
	fd->error = 0;
	uloop_fd_add(fd, ULOOP_READ | ULOOP_WRITE);

	uloop_run();

	uloop_done();

	return 0;
}


