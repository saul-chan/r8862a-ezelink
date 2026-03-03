/*
 * Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */

#ifndef _CLSAPI_VBSS_H
#define _CLSAPI_VBSS_H

#define VBSS_AC_PORT	0x4359	// YC
#define VBSS_AGENT_PORT	(VBSS_AC_PORT + 1)

#define RET_CODE_OK				0
#define RET_CODE_ERROR			-1

#define MSG_OK					"OK"
#define MSG_ERROR				"ERROR"

#define MSG_AGENT_PING		"ping"
	/* Reqest msg format:	"ping"
	 * Reply msg format:	"pong"
	 */
#define MSG_AGENT_ENABLE_VBSS	"enable_vbss"
	/* Reqest msg format:	"enable_vbss <primary ifname> <0 | 1>" + '\0'
	 * Reply msg format:	"OK" or "FAIL"
	 */
#define MSG_AGENT_GET_VAP_INFO	"get_vap_info"
	/* Reqest msg format:	"get_vap_info <ifname>" + '\0' + <bin VAP MAC>
	 * Reply msg format:	<bin of vap_info>
	 */
#define MSG_AGENT_ADD_VAP_INFO	"add_vap_info"
	/* Reqest msg format:	"add_vap_info" + '\0' + <bin of vap_info>
	 * Reply msg format:	"OK" or "FAIL"
	 */
#define MSG_AGENT_DEL_VAP	"del_vap"
	/* Reqest msg format:	"del_vap <ifname>" + '\0'
	 * Reply msg format:	"OK" or "FAIL"
	 */
#define MSG_AGENT_GET_STA_INFO	"get_sta_info"
	/* Reqest msg format:	"get_sta_info <ifname>" + '\0' + <bin STA MAC>
	 * Reply msg format:	<bin of vap_info>
	 */
#define MSG_AGENT_ADD_STA_INFO	"add_sta_info"
	/* Reqest msg format:	"add_sta_info <ifname>" + '\0' + <bin of sta_info>
	 * Reply msg format:	"OK" or "FAIL"
	 */
#define MSG_AGENT_TRG_SWITCH	"trigger_switch"
	/* Reqest msg format:	"trigger_switch <ifname>" + '\0' + <bin of STA MAC>
	 * Reply msg format:	"OK" or "FAIL"
	 */
#define MSG_AGENT_SWITCH_DONE	"switch_done"
	/* Reqest msg format:	"switch_done <ifname>" + '\0' + <bin STA MAC>
	 * Reply msg format:	"OK" or "FAIL"
	 */
#define MSG_AGENT_PKT_TO_AC	"send_fake_pkt_to_ac"
	/* Reqest msg format:	"send_fake_pkt_to_ac <ifname>" + '\0' + <bin STA MAC> + <STA_IP>
	 * Reply msg format:	"OK"
	 */

#define MACFMT "%02x:%02x:%02x:%02x:%02x:%02x"
#define MACARG(src) (src)[0], (src)[1], (src)[2], (src)[3], (src)[4], (src)[5]

// try to get ip by mac from arp cache
// Return values:
//   <0: Errors
//   =0: OK
static int get_ip_from_arp_cache(uint32_t *ip, uint8_t *mac)
{
	char command[128] = {0};
	char str_ip[64] = {0};
	FILE *fp = NULL;
	struct in_addr ip_in;

	snprintf(command, sizeof(command), "ip -4 neigh | grep "MACFMT"|awk -F\' \' '{print $1}'", MACARG(mac));
	printf("%s: command:%s\n", __func__, command);

	if ((fp = popen(command, "r")) == NULL ) {
		printf("popen fail!\n");
		return -1;
	}

	if (fgets(str_ip, sizeof(str_ip), fp) != NULL) {
		if (inet_aton(str_ip, &ip_in) == 1) {
			*ip = ip_in.s_addr;
			printf("interpret successfully str_ip:%s, ip:%x\n",str_ip, *ip);
		}
		else {
			printf("interpret fail\n");
			*ip = 0;
		}
	} else
		*ip = 0;

	printf("%s:str_ip=%s,ip:%x\n", __func__, str_ip, *ip);
	pclose(fp);
	return 0;
}


#endif /* _CLSAPI_VBSS_H */

