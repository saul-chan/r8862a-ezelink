#include "dubhe2000.h"

/*
 * the entry0 in acl1 TCAM is reserved for svp function
 * the requirement of svp function is that:
 *  The IP packet with the svp(l4) should be sent to wireless interface in AC-VI or AC-VO queue.
 * When a packet will be forward to wireless interface by fwt,
 * we should make clear whether the l4 proto is svp(protocol id 119) or the dscp has already been updated.
 * There are 3 options:
  (1) if IPv4/IPv6 flag(Tocputag) is enable, parse l4 protocol in driver. If true, change tos field in driver
  (2) acl1 engine, update TOS=184(EF,dscp=46,IP Precedence=5). The prcession of npe -> wifi can handle it well
  (3) acl3 engine, update a special metaData's flow-id. We check flow-id and update tos-184.
  (4) acl0(nat) and acl2(ipv6) are Not applicable because they have specific uses,
  For option2, only a fixed acl1 tcam entry0 can cover everything. But not a origin packet.
 */
void dubhe2000_switch_generic_svp_config(struct dubhe1000_adapter *adapter, bool is_enable)
{
	u8 data[17]; // the length of t_IngressConfigurableACL1TCAM compareData/mask
	struct dubhe2000_tc_acl1_ip_hashkey *hashkey = (void *)data;
	t_IngressConfigurableACL1TCAM acl1_tcam;
	t_IngressConfigurableACL1TCAMAnswer acl1_answer;
	u8 l4_svp_proto = 119; //svp
	u8 tcam_index = 0;     // index0 for svp

	memset(&acl1_tcam, 0, sizeof(acl1_tcam));
	acl1_tcam.valid = 1;

	// only l4_proto is valid. source port should be ignored or hardcode for port0 ?
	memset(data, 0, sizeof(data));
	dubhe2000_ingress_acl1_ip_hashkey_build(data, 0, 0, 0, 0, l4_svp_proto, 0);
	hashkey->valid = 1 << 4; // l4_proto
	memcpy(acl1_tcam.compareData, data, sizeof(acl1_tcam.compareData));

	memset(data, 0, sizeof(data));
	dubhe2000_ingress_acl1_ip_hashkey_build(data, 0, 0, 0, 0, 0xFF, 0);
	hashkey->valid = 1 << 4; // l4_proto
	memcpy(acl1_tcam.mask, data, sizeof(acl1_tcam.mask));

	wr_IngressConfigurableACL1TCAM(adapter, tcam_index, &acl1_tcam);

	memset(&acl1_answer, 0, sizeof(acl1_answer));
	if (is_enable) {
		acl1_answer.updateTosExp = 1;
		acl1_answer.newTosExp = 0b10111000; //184
		acl1_answer.tosMask = 0b11111100;   //ECN field won't be selected.
		acl1_answer.updateCounter = 1;
		acl1_answer.counter = SWITCH_INGRESS_ACL_COUNTER_SVP;
	}
	wr_IngressConfigurableACL1TCAMAnswer(adapter, tcam_index, &acl1_answer);
}

/* The non-ip packet with a router's DMAC will be dropped and counted in
 * Invalid Routing Protocol Drop, like 1905.
 * In Fact, we should forward it to cpu port.
 * ACL3 exclude MAC DA file. So select ACl1 TCAM index1 will be selected
 */
void dubhe2000_switch_generic_nonip_config(struct dubhe1000_adapter *adapter, u8 if_mask)
{
	u8 data[17]; //the length of t_IngressConfigurableACL1TCAM compareData/mask
	struct dubhe2000_tc_acl1_nonip_hashkey *hashkey = (void *)data;
	t_IngressConfigurableACL1TCAM acl1_tcam;
	t_IngressConfigurableACL1TCAMAnswer acl1_answer;
	u64 mac_empty = 0, mac_mask = 0xFFFFFFFFFFFF;
	u8 mac_tmp[ETH_ALEN], tcam_index; // index1/2 for nonip
	int i, j;

	memset(&acl1_tcam, 0, sizeof(acl1_tcam));
	memset(&acl1_answer, 0, sizeof(acl1_answer));

	if (!if_mask) { //clear index1 directly if enable == 0
		tcam_index = ACL1_TCAM_INX_NO_IP_LAN;
		wr_IngressConfigurableACL1TCAM(adapter, tcam_index, &acl1_tcam);
		wr_IngressConfigurableACL1TCAMAnswer(adapter, tcam_index, &acl1_answer);

		tcam_index = ACL1_TCAM_INX_NO_IP_WAN;
		wr_IngressConfigurableACL1TCAM(adapter, tcam_index, &acl1_tcam);
		wr_IngressConfigurableACL1TCAMAnswer(adapter, tcam_index, &acl1_answer);

		pr_err("[%s] clear all nonip acl\n", __func__);
		return;
	}

	//enable
	for (i = 0; i < 2; i++) {
		if (!(if_mask & BIT(i))) {
			pr_err("[%s] index %d disabled, ignore it\n",__func__, i);
			continue;
		}

		/* tcam_index = ACL1_TCAM_INX_NO_IP_WAN if i == 1
		 * tcam_index = ACL1_TCAM_INX_NO_IP_LAN if i == 0
		 */
		tcam_index = i?(ACL1_TCAM_INX_NO_IP_WAN):(ACL1_TCAM_INX_NO_IP_LAN);
		pr_err("i = %d, tcam_index = %d\n", i, tcam_index);

		if (dubhe1000_check_u8_array_is_zero(default_gw_da_mac[i], ETH_ALEN)) {
			pr_err("[%s] index %d(%s) mac not configured. Try again!\n",
					__func__, i, i == 0 ? "br" : "wan");
			continue;
		}

		for (j = 0; j < ETH_ALEN; j++)
			mac_tmp[j] = default_gw_da_mac[i][ETH_ALEN - j - 1];

		acl1_tcam.valid = 1;

		memset(data, 0, sizeof(data));
		dubhe2000_ingress_acl1_nonip_hashkey_build(data, mac_tmp, (u8 *)(&mac_empty), 0, 0, 0, 0, 0);
		hashkey->valid = 1 << 0; // mac da
		memcpy(acl1_tcam.compareData, data, sizeof(acl1_tcam.compareData));

		memset(data, 0, sizeof(data));
		dubhe2000_ingress_acl1_nonip_hashkey_build(data, (u8 *)(&mac_mask), (u8 *)(&mac_empty), 0, 0, 0, 0, 0);
		hashkey->valid = 1 << 0; // mac da
		memcpy(acl1_tcam.mask, data, sizeof(acl1_tcam.mask));

		wr_IngressConfigurableACL1TCAM(adapter, tcam_index, &acl1_tcam);

		acl1_answer.sendToCpu = 1;
		acl1_answer.forceSendToCpuOrigPkt = 1;
		acl1_answer.updateCounter = 1;
		acl1_answer.counter = SWITCH_INGRESS_ACL_COUNTER_NONIP;

		wr_IngressConfigurableACL1TCAMAnswer(adapter, tcam_index, &acl1_answer);

		pr_info("[%s] dmac[%pM] nonip packet will be forward to cpuport!\n", __func__, default_gw_da_mac[i]);
	}
}

//acl2 tcam index0
void dubhe2000_switch_generic_ipv6_frag_config(struct dubhe1000_adapter *adapter, bool is_enable)
{
	u8 data[68]; //the length of t_IngressConfigurableACL2TCAM compareData/mask
	struct dubhe2000_tc_acl2_ipv6_hashkey *hashkey = (void *)data;
	t_IngressConfigurableACL2TCAM acl2_tcam;
	t_IngressConfigurableACL2TCAMAnswer acl2_answer;
	u64 mac_empty = 0, mac_mask = 0xFFFFFFFFFFFF;
	u32 ip_empty[4];
	u8 mac_tmp[ETH_ALEN], tcam_index = ACL1_TCAM_INX_IPV6_FRAG; // index0 for ipv6 frag
	int i;

	memset(&acl2_tcam, 0, sizeof(acl2_tcam));
	memset(&acl2_answer, 0, sizeof(acl2_answer));

	if (!is_enable) { //clear index0 directly
		wr_IngressConfigurableACL2TCAM(adapter, tcam_index, &acl2_tcam);
		wr_IngressConfigurableACL2TCAMAnswer(adapter, tcam_index, &acl2_answer);

		return;
	}

	//enable
	if (dubhe1000_check_u8_array_is_zero(default_gw_da_mac[LAN_MAC_INX], ETH_ALEN)) {
		pr_err("[%s] br's mac not configured. Try again if currently in router mode!\n", __func__);
		return;
	}

	memset(ip_empty, 0, sizeof(ip_empty));

	for (i = 0; i < ETH_ALEN; i++)
		mac_tmp[i] = default_gw_da_mac[LAN_MAC_INX][ETH_ALEN - i - 1];

	acl2_tcam.valid = 1;

	memset(data, 0, sizeof(data));
	dubhe2000_ingress_acl2_ipv6_hashkey_build(data, mac_tmp /*macda*/, (u8 *)(&mac_empty) /*macsa*/,
						  ip_empty /*sip*/, ip_empty /*dip*/, 0 /*tos*/, 0 /*sport*/,
						  0 /*dport*/, 0 /*ipv6_flow_table*/, 0 /*l4 proto*/,
						  L4_TYPE_SPECIAL_IP /*l4 type*/, 0 /*source port*/);
	hashkey->valid = (1 << 0) + (1 << 18); // mac da + l4 type
	memcpy(acl2_tcam.compareData, data, sizeof(acl2_tcam.compareData));

	memset(data, 0, sizeof(data));
	dubhe2000_ingress_acl2_ipv6_hashkey_build(data, (u8 *)(&mac_mask) /*macda*/, (u8 *)(&mac_empty) /*macsa*/,
						  ip_empty /*sip*/, ip_empty /*dip*/, 0 /*tos*/, 0 /*sport*/,
						  0 /*dport*/, 0 /*ipv6_flow_table*/, 0 /*l4 proto*/,
						  L4_TYPE_MASK /*l4 type*/, 0 /*source port*/);

	hashkey->valid = (1 << 0) + (1 << 18); // mac da + l4 type
	memcpy(acl2_tcam.mask, data, sizeof(acl2_tcam.mask));

	wr_IngressConfigurableACL2TCAM(adapter, tcam_index, &acl2_tcam);

	acl2_answer.sendToCpu = 1;
	acl2_answer.forceSendToCpuOrigPkt = 1;
	acl2_answer.updateCounter = 1;
	acl2_answer.counter = SWITCH_INGRESS_ACL_COUNTER_IPV6_FRAG;

	wr_IngressConfigurableACL2TCAMAnswer(adapter, tcam_index, &acl2_answer);

	pr_info("[%s] dmac[%pM] routed ipv6 frag will be forward to cpuport!\n", __func__, default_gw_da_mac[LAN_MAC_INX]);
}

/* IPV4 frag:
 * Only acl3 tcam(include IPv4 Options) can recognize ipv4 frag.
 * And for IPv4 Options: bit2 is MF; bit3 is DF; bit4 is reserved
 * The ACL3 Tcam index0/1/2 is used to forward ip IPV4 frag and option to CPUPort
 *
 * IPV6 frag:
 * 'L4_Type=1 and L3_Type=1'is a workaroud for ipv6 frag.
 *
 * In fact, only frag in L3 process should be forward. And the frag in L2 process is non-essentials.
 * The ACL3 TCAM cannot handle ipv4 frag well. Unable to distinguish between L2 and L3 process.
 * Though it means that all ipv4 frag will forward to CPU port, we can config it only in router mode
 * For ipv6 frag, we can select DMAC + L3_Type + L4_Type in acl2 Tcam.
 * This acl entry will only be configured in router mode.
 * It means that ipv6 frag will forward to CPU port during L3 process in router mode.
 */
void dubhe2000_switch_generic_ip_frag_config(struct dubhe1000_adapter *adapter, bool is_enable)
{
	u8 data[10]; // the length of t_IngressConfigurableACL3TCAM compareData/mask
	struct dubhe2000_tc_acl3_hashkey *hashkey = (void *)data;
	t_IngressConfigurableACL3TCAM acl3_tcam;
	t_IngressConfigurableACL3TCAMAnswer acl3_answer;
	/*idx0 for non end frag, indx1 for end frag, idx2 for ip option, idx3 for ipv6 frag*/
	u8 tcam_index;

	/* index0 ip frag non end package: MF=1*/
	tcam_index = 0;
	memset(&acl3_tcam, 0, sizeof(acl3_tcam));

	memset(data, 0, sizeof(data));
	dubhe2000_ingress_acl3_hashkey_build(data, 0, 0b00100, 0, 0, 0, 0, 0);
	hashkey->valid = 1 << 1; //ipv4_options
	memcpy(acl3_tcam.compareData, data, sizeof(acl3_tcam.compareData));

	memset(data, 0, sizeof(data));
	dubhe2000_ingress_acl3_hashkey_build(data, 0, 0b00100, 0, 0, 0, 0, 0);
	hashkey->valid = 1 << 1; //ipv4_options
	memcpy(acl3_tcam.mask, data, sizeof(acl3_tcam.mask));

	acl3_tcam.valid = 1;
	wr_IngressConfigurableACL3TCAM(adapter, tcam_index, &acl3_tcam);

	memset(&acl3_answer, 0, sizeof(acl3_answer));

	if (is_enable) {
		acl3_answer.sendToCpu = 1;
		acl3_answer.forceSendToCpuOrigPkt = 1;
	}
	wr_IngressConfigurableACL3TCAMAnswer(adapter, tcam_index, &acl3_answer);

	/* index1 ip frag end package: MF=0, Fragment_Offset!=0*/
	tcam_index = 1;
	memset(&acl3_tcam, 0, sizeof(acl3_tcam));

	memset(data, 0, sizeof(data));
	dubhe2000_ingress_acl3_hashkey_build(data, 0, 0b00001, 0, 0, 0, 0, 0);
	hashkey->valid = 1 << 1; //ipv4_options
	memcpy(acl3_tcam.compareData, data, sizeof(acl3_tcam.compareData));

	memset(data, 0, sizeof(data));
	dubhe2000_ingress_acl3_hashkey_build(data, 0, 0b00101, 0, 0, 0, 0, 0);
	hashkey->valid = 1 << 1; //ipv4_options
	memcpy(acl3_tcam.mask, data, sizeof(acl3_tcam.mask));

	acl3_tcam.valid = 1;
	wr_IngressConfigurableACL3TCAM(adapter, tcam_index, &acl3_tcam);

	memset(&acl3_answer, 0, sizeof(acl3_answer));

	if (is_enable) {
		acl3_answer.sendToCpu = 1;
		acl3_answer.forceSendToCpuOrigPkt = 1;
	}
	wr_IngressConfigurableACL3TCAMAnswer(adapter, tcam_index, &acl3_answer);

	/* index2 ip option*/
	tcam_index = 2;
	memset(&acl3_tcam, 0, sizeof(acl3_tcam));
	memset(data, 0, sizeof(data));
	dubhe2000_ingress_acl3_hashkey_build(data, 0, 0b00010, 0, 0, 0, 0, 0);
	hashkey->valid = 1 << 1; //ipv4_options
	memcpy(acl3_tcam.compareData, data, sizeof(acl3_tcam.compareData));

	memset(data, 0, sizeof(data));
	dubhe2000_ingress_acl3_hashkey_build(data, 0, 0b00010, 0, 0, 0, 0, 0);
	hashkey->valid = 1 << 1; //ipv4_options
	memcpy(acl3_tcam.mask, data, sizeof(acl3_tcam.mask));

	acl3_tcam.valid = 1;
	wr_IngressConfigurableACL3TCAM(adapter, tcam_index, &acl3_tcam);

	memset(&acl3_answer, 0, sizeof(acl3_answer));

	if (is_enable) {
		acl3_answer.sendToCpu = 1;
		acl3_answer.forceSendToCpuOrigPkt = 1;
	}
	wr_IngressConfigurableACL3TCAMAnswer(adapter, tcam_index, &acl3_answer);

	/*acl2 tcam index0 ipv6 frag*/
	dubhe2000_switch_generic_ipv6_frag_config(adapter, is_enable);
}

/*
 * the entry4/entry5 in acl3 TCAM is reserved for TCP RST/FIN function
 * the requirement of TCP function is that:
 * When packet is L4 TCP RST/FIN and is not a fragment. Original Packet will be forward to cpuport.
 * Ingress ACL 1/2/3 can config TCP Flags. This ACL2 is exclusive to non-router ipv6.
 * Both ACL1/ACl3 can meet TCP FLAGS needs.
 * TCP Flags is not necessary both in ACL1 and ACL3. ACL3 will be selected.
 */
void dubhe2000_switch_generic_tcp_config(struct dubhe1000_adapter *adapter, bool is_enable)
{
	u8 data[10]; // the length of t_IngressConfigurableACL3TCAM compareData/mask
	struct dubhe2000_tc_acl3_hashkey *hashkey = (void *)data;
	t_IngressConfigurableACL3TCAM acl3_tcam;
	t_IngressConfigurableACL3TCAMAnswer acl3_answer;
	u8 tcam_index; // index4 for RST, index5 for FIN

	/* index3 RST*/
	tcam_index = 3;
	memset(&acl3_tcam, 0, sizeof(acl3_tcam));

	memset(data, 0, sizeof(data));
	dubhe2000_ingress_acl3_hashkey_build(data, 0, 0, 1 << TCP_FLAGS_BIT_RST, 0, 0, 0, 0);
	hashkey->valid = 1 << 2; //tcp_flags
	memcpy(acl3_tcam.compareData, data, sizeof(acl3_tcam.compareData));

	memset(data, 0, sizeof(data));
	dubhe2000_ingress_acl3_hashkey_build(data, 0, 0, 1 << TCP_FLAGS_BIT_RST, 0, 0, 0, 0);
	hashkey->valid = 1 << 2; //tcp_flags
	memcpy(acl3_tcam.mask, data, sizeof(acl3_tcam.mask));

	acl3_tcam.valid = 1;
	wr_IngressConfigurableACL3TCAM(adapter, tcam_index, &acl3_tcam);

	memset(&acl3_answer, 0, sizeof(acl3_answer));

	if (is_enable) {
		acl3_answer.sendToCpu = 1;
		acl3_answer.forceSendToCpuOrigPkt = 1;
	}
	wr_IngressConfigurableACL3TCAMAnswer(adapter, tcam_index, &acl3_answer);

	/* index4 FIN*/
	tcam_index = 4;
	memset(&acl3_tcam, 0, sizeof(acl3_tcam));

	memset(data, 0, sizeof(data));
	dubhe2000_ingress_acl3_hashkey_build(data, 0, 0, 1 << TCP_FLAGS_BIT_FIN, 0, 0, 0, 0);
	hashkey->valid = 1 << 2; //tcp_flags
	memcpy(acl3_tcam.compareData, data, sizeof(acl3_tcam.compareData));

	memset(data, 0, sizeof(data));
	dubhe2000_ingress_acl3_hashkey_build(data, 0, 0, 1 << TCP_FLAGS_BIT_FIN, 0, 0, 0, 0);
	hashkey->valid = 1 << 2; //tcp_flags
	memcpy(acl3_tcam.mask, data, sizeof(acl3_tcam.mask));

	acl3_tcam.valid = 1;
	wr_IngressConfigurableACL3TCAM(adapter, tcam_index, &acl3_tcam);

	memset(&acl3_answer, 0, sizeof(acl3_answer));

	if (is_enable) {
		acl3_answer.sendToCpu = 1;
		acl3_answer.forceSendToCpuOrigPkt = 1;
	}
	wr_IngressConfigurableACL3TCAMAnswer(adapter, tcam_index, &acl3_answer);
}
/* when packet is TCP and port=21(FTP control),
 * The ip address is also stored in ftp payload, not only in ip header.
 * It means that switch nat function cannot handle it. We should forward it to CPU Port
 * ACL1 TCAM index2 is sport=21, index3 is dport=21
 */
void dubhe2000_switch_generic_ftp_config(struct dubhe1000_adapter *adapter, bool is_enable)
{
	u8 data[17]; // the length of t_IngressConfigurableACL1TCAM compareData/mask
	struct dubhe2000_tc_acl1_ip_hashkey *hashkey = (void *)data;
	t_IngressConfigurableACL1TCAM acl1_tcam;
	t_IngressConfigurableACL1TCAMAnswer acl1_answer;
	u8 l4_tcp_proto = 0x6; //tcp
	u8 tcam_index;	       // index3/4

	/* sport = 21 */
	tcam_index = ACL1_TCAM_INX_FTP_SPORT;
	memset(&acl1_tcam, 0, sizeof(acl1_tcam));
	acl1_tcam.valid = 1;

	// only l4_proto is valid. source port should be ignored or hardcode for port0 ?
	memset(data, 0, sizeof(data));
	dubhe2000_ingress_acl1_ip_hashkey_build(data, 0, 0, 21, 0, l4_tcp_proto, 0);
	hashkey->valid = (1 << 2) + (1 << 4); // sport + l4_proto
	memcpy(acl1_tcam.compareData, data, sizeof(acl1_tcam.compareData));

	memset(data, 0, sizeof(data));
	dubhe2000_ingress_acl1_ip_hashkey_build(data, 0, 0, 0xFFFF, 0, 0xFF, 0);
	hashkey->valid = (1 << 2) + (1 << 4); // sport + l4_proto
	memcpy(acl1_tcam.mask, data, sizeof(acl1_tcam.mask));

	wr_IngressConfigurableACL1TCAM(adapter, tcam_index, &acl1_tcam);

	memset(&acl1_answer, 0, sizeof(acl1_answer));
	if (is_enable) {
		acl1_answer.sendToCpu = 1;
		acl1_answer.forceSendToCpuOrigPkt = 1;
		acl1_answer.updateCounter = 1;
		acl1_answer.counter = SWITCH_INGRESS_ACL_COUNTER_FTP;
	}
	wr_IngressConfigurableACL1TCAMAnswer(adapter, tcam_index, &acl1_answer);

	/* dport = 21 */
	tcam_index = ACL1_TCAM_INX_FTP_DPORT;
	memset(&acl1_tcam, 0, sizeof(acl1_tcam));
	acl1_tcam.valid = 1;

	// only l4_proto is valid. source port should be ignored or hardcode for port0 ?
	memset(data, 0, sizeof(data));
	dubhe2000_ingress_acl1_ip_hashkey_build(data, 0, 0, 0, 21, l4_tcp_proto, 0);
	hashkey->valid = (1 << 3) + (1 << 4); // sport + l4_proto
	memcpy(acl1_tcam.compareData, data, sizeof(acl1_tcam.compareData));

	memset(data, 0, sizeof(data));
	dubhe2000_ingress_acl1_ip_hashkey_build(data, 0, 0, 0, 0xFFFF, 0xFF, 0);
	hashkey->valid = (1 << 3) + (1 << 4); // sport + l4_proto
	memcpy(acl1_tcam.mask, data, sizeof(acl1_tcam.mask));

	wr_IngressConfigurableACL1TCAM(adapter, tcam_index, &acl1_tcam);

	memset(&acl1_answer, 0, sizeof(acl1_answer));
	if (is_enable) {
		acl1_answer.sendToCpu = 1;
		acl1_answer.forceSendToCpuOrigPkt = 1;
		acl1_answer.updateCounter = 1;
		acl1_answer.counter = SWITCH_INGRESS_ACL_COUNTER_FTP;
	}
	wr_IngressConfigurableACL1TCAMAnswer(adapter, tcam_index, &acl1_answer);
}
/* For iperf control packet(TCP sport=5021/5001 and dport=5021/5001),
 * we reassign its metadata to CPU channel. Because wifi channel maybe is full during tp case.
 * This is not related to IPv6 and need L4 DPORT/L4 DPORT. so we choose this ACL1 TCAM index4/index5
 */
// mode < 0, disable this function; mode = 0 means only hit tcp; mode > 0 means tcp +sport/dport
void dubhe2000_switch_generic_iperf_config(struct dubhe1000_adapter *adapter, int mode)
{
	u8 data[17]; // the length of t_IngressConfigurableACL1TCAM compareData/mask
	struct dubhe2000_tc_acl1_ip_hashkey *hashkey = (void *)data;
	t_IngressConfigurableACL1TCAM acl1_tcam;
	t_IngressConfigurableACL1TCAMAnswer acl1_answer;
	u8 l4_tcp_proto = 0x6; //tcp
	u8 tcam_index;
	u16 port_num;
	u16 port_mask;

	//clear all acl entry index5/6
	tcam_index = ACL1_TCAM_INX_IPERF_SRC;

	acl1_tcam.valid = 0;
	memset(acl1_tcam.mask, 0xFF, sizeof(acl1_tcam.mask));
	memset(acl1_tcam.compareData, 0, sizeof(acl1_tcam.compareData));
	memset(&acl1_answer, 0, sizeof(acl1_answer));

	wr_IngressConfigurableACL1TCAM(adapter, tcam_index, &acl1_tcam);
	wr_IngressConfigurableACL1TCAMAnswer(adapter, tcam_index, &acl1_answer);

	tcam_index = ACL1_TCAM_INX_IPERF_DST;
	wr_IngressConfigurableACL1TCAM(adapter, tcam_index, &acl1_tcam);
	wr_IngressConfigurableACL1TCAMAnswer(adapter, tcam_index, &acl1_answer);

	if (mode < 0) {
		pr_info("[%s] disable!\n", __func__);

		return;
	} else if (mode == 0) {
		/*only tcp*/
		tcam_index = ACL1_TCAM_INX_IPERF_TCP;
		memset(&acl1_tcam, 0, sizeof(acl1_tcam));
		acl1_tcam.valid = 1;

		memset(data, 0, sizeof(data));
		dubhe2000_ingress_acl1_ip_hashkey_build(data, 0, 0, 0, 0, l4_tcp_proto, 0);
		hashkey->valid = (1 << 4); // l4_proto
		memcpy(acl1_tcam.compareData, data, sizeof(acl1_tcam.compareData));

		memset(data, 0, sizeof(data));
		dubhe2000_ingress_acl1_ip_hashkey_build(data, 0, 0, 0, 0, 0xFF, 0);
		hashkey->valid = (1 << 4); // l4_proto
		memcpy(acl1_tcam.mask, data, sizeof(acl1_tcam.mask));

		wr_IngressConfigurableACL1TCAM(adapter, tcam_index, &acl1_tcam);

		memset(&acl1_answer, 0, sizeof(acl1_answer));
		acl1_answer.metaDataValid = 1;
		acl1_answer.metaData = 0;
		acl1_answer.metaDataPrio = 1;
		acl1_answer.updateCounter = 1;
		acl1_answer.counter = SWITCH_INGRESS_ACL_COUNTER_IPERF_TCP;

		wr_IngressConfigurableACL1TCAMAnswer(adapter, tcam_index, &acl1_answer);

		pr_info("[%s] tcp will access CPU Channel!\n", __func__);
	} else {
		port_num = mode & 0xFFFF;
		port_mask = (mode >> 16) & 0x7FFF;
		port_mask = ~port_mask;

		/*tcp and sport*/
		tcam_index = ACL1_TCAM_INX_IPERF_SRC;
		memset(&acl1_tcam, 0, sizeof(acl1_tcam));
		acl1_tcam.valid = 1;

		memset(data, 0, sizeof(data));
		dubhe2000_ingress_acl1_ip_hashkey_build(data, 0, 0, port_num, 0, l4_tcp_proto, 0);
		hashkey->valid = (1 << 2) + (1 << 4); // l4_proto + sport
		memcpy(acl1_tcam.compareData, data, sizeof(acl1_tcam.compareData));

		memset(data, 0, sizeof(data));
		dubhe2000_ingress_acl1_ip_hashkey_build(data, 0, 0, port_mask, 0, 0xFF, 0);
		hashkey->valid = (1 << 2) + (1 << 4); // l4_proto + sport
		memcpy(acl1_tcam.mask, data, sizeof(acl1_tcam.mask));

		wr_IngressConfigurableACL1TCAM(adapter, tcam_index, &acl1_tcam);

		memset(&acl1_answer, 0, sizeof(acl1_answer));
		acl1_answer.metaDataValid = 1;
		acl1_answer.metaData = 0;
		acl1_answer.metaDataPrio = 1;
		acl1_answer.updateCounter = 1;
		acl1_answer.counter = SWITCH_INGRESS_ACL_COUNTER_IPERF_SPORT;
		wr_IngressConfigurableACL1TCAMAnswer(adapter, tcam_index, &acl1_answer);

		/*tcp and dport*/
		tcam_index = ACL1_TCAM_INX_IPERF_DST;
		memset(&acl1_tcam, 0, sizeof(acl1_tcam));
		acl1_tcam.valid = 1;

		memset(data, 0, sizeof(data));
		dubhe2000_ingress_acl1_ip_hashkey_build(data, 0, 0, 0, port_num, l4_tcp_proto, 0);
		hashkey->valid = (1 << 3) + (1 << 4); // l4_proto + dport
		memcpy(acl1_tcam.compareData, data, sizeof(acl1_tcam.compareData));

		memset(data, 0, sizeof(data));
		dubhe2000_ingress_acl1_ip_hashkey_build(data, 0, 0, 0, port_mask, 0xFF, 0);
		hashkey->valid = (1 << 3) + (1 << 4); // l4_proto + dport
		memcpy(acl1_tcam.mask, data, sizeof(acl1_tcam.mask));

		wr_IngressConfigurableACL1TCAM(adapter, tcam_index, &acl1_tcam);

		memset(&acl1_answer, 0, sizeof(acl1_answer));
		acl1_answer.metaDataValid = 1;
		acl1_answer.metaData = 0;
		acl1_answer.metaDataPrio = 1;
		acl1_answer.updateCounter = 1;
		acl1_answer.counter = SWITCH_INGRESS_ACL_COUNTER_IPERF_DPORT;

		wr_IngressConfigurableACL1TCAMAnswer(adapter, tcam_index, &acl1_answer);

		pr_info("[%s] tcp + port-0x%x(mask 0x%x) will access CPU Channel!\n",
				__func__, port_num, port_mask);
	}
}

void dubhe1000_switch_drain_port(struct dubhe1000_adapter *adapter, int port, int enable)
{
	u32 value;
	u64 address;

	address = 4 * DRAIN_PORT;
	value = readl(adapter->switch_regs + address);

	if (enable) {
		value |= 1 << port;
		writel(value, adapter->switch_regs + address);
	} else {
		value &= ~(1 << port);
		writel(value, adapter->switch_regs + address);
	}
}

/*
 * Enable drain_port on all eth(non-cpu) port.
 * It means that Egress Packet Processing will drop all packets.
 * Note: drain_port should be disable when link up.
 */
void dubhe2000_switch_drain_port_enable_ethport(struct dubhe1000_adapter *adapter)
{
	int i;

	for (i = 0; i <= DUBHE2000_PORT_MAX; i++) {
		if (i == DUBHE2000_CPU_PORT)
			continue;

		dubhe1000_switch_drain_port(adapter, i, 1);
	}
}

void dubhe2000_switch_send_to_cpu_init(struct dubhe1000_adapter *adapter)
{
	t_SendtoCPU send2cpu;
	t_DefaultPacketToCPUModification tocpu_mod;
	int i;

	/*SendToCPU*/
	rd_SendtoCPU(adapter, &send2cpu);

	send2cpu.allowBpdu = 0x1F;    //BPDU won't be send2cpu when sport is cpuport
	send2cpu.allowRstBpdu = 0x1F; ////RSTBPDU won't send2cpu when sport is cpuport

	wr_SendtoCPU(adapter, &send2cpu);

	/*DefaultPacketToCPUModification*/
	tocpu_mod.origCpuPkt = 1;
	for (i = 0; i <= DUBHE2000_PORT_MAX; i++) {
		if (i == DUBHE2000_CPU_PORT)
			continue;

		wr_DefaultPacketToCPUModification(adapter, i, &tocpu_mod);
	}
}

/*
 * The DWRR scheduler only acts on queues mapped to the same priority.
 */
void dubhe2000_switch_use_dwrr_scheduler(struct dubhe1000_adapter *adapter)
{
	int port;
	t_MapQueuetoPriority data;

	for (port = 0; port <= DUBHE2000_PORT_MAX; ++port) {
		memset(&data, 0, sizeof(data));
		wr_MapQueuetoPriority(adapter, port, &data);
	}
}

void dubhe2000_switch_init_queue_weight(struct dubhe1000_adapter *adapter)
{
	t_DWRRWeightConfiguration data;
	int port, queue;
	u8 weight[] = { 128, 64, 32, 16, 8, 4, 2, 1 };

	for (port = 0; port <= DUBHE2000_PORT_MAX; ++port) {
		for (queue = 0; queue < 8; ++queue) {
			adapter->queue_weight[port][queue] = weight[queue];
			data.weight = weight[queue];
			wr_DWRRWeightConfiguration(adapter, port * 8 + queue, &data);
		}
	}
}

void dubhe2000_switch_set_queue_weight(struct dubhe1000_adapter *adapter)
{
	t_DWRRWeightConfiguration data;
	int port, queue;

	for (port = 0; port <= DUBHE2000_PORT_MAX; ++port) {
		for (queue = 0; queue < 8; ++queue) {
			data.weight = adapter->queue_weight[port][queue];
			wr_DWRRWeightConfiguration(adapter, port * 8 + queue, &data);
		}
	}
}

void dubhe2000_change_wan_port(struct dubhe1000_adapter *adapter)
{
	int i;
	t_SourcePortTable source_port;
	t_EgressPortNATState eport_nat_state;

	pr_info("wan port change to %d\n", adapter->wan_port);

	memset(&eport_nat_state, 0, sizeof(eport_nat_state));
	for (i = 0; i <= DUBHE2000_PORT_MAX; i++) {
		memset(&source_port, 0, sizeof(source_port));
		rd_SourcePortTable(adapter, i, &source_port);

		if (i == adapter->wan_port) {
			source_port.natPortState = 1;
			eport_nat_state.portState = 1 << i;
		} else {
			source_port.natPortState = 0;
		}

		wr_SourcePortTable(adapter, i, &source_port);
	}

	wr_EgressPortNATState(adapter, &eport_nat_state);

	dubhe2000_tunnel_init_config(adapter);
}

void dubhe2000_switch_init_pcp_to_queue(struct dubhe1000_adapter *adapter)
{
	int i;
	t_VLANPCPToQueueMappingTable data;

	for (i = 0; i < 8; ++i) {
		data.pQueue = 0;
		wr_VLANPCPToQueueMappingTable(adapter, i, &data);
	}
}

void dubhe2000_switch_generic_init_config(struct dubhe1000_adapter *adapter)
{
	/* ACL */
	dubhe2000_switch_generic_svp_config(adapter, 0);
	dubhe2000_switch_generic_nonip_config(adapter, 0);

	dubhe2000_switch_generic_ip_frag_config(adapter, 0);
	dubhe2000_switch_generic_tcp_config(adapter, 0);

	dubhe2000_switch_generic_ftp_config(adapter, 0);
	dubhe2000_switch_generic_iperf_config(adapter, 5201);

#ifdef CONFIG_DUBHE2000_PHYLINK
	/*
	 * We should enable drain port on all eth port.
	 * And drain port will be updated when link down/up
	 */
	dubhe2000_switch_drain_port_enable_ethport(adapter);
#endif

	dubhe2000_switch_send_to_cpu_init(adapter);
	dubhe2000_switch_init_pcp_to_queue(adapter);
}
