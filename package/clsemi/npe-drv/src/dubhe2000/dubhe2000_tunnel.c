#include "dubhe2000.h"

enum {
	TUNNEL_ENTRY_INSTRUCT_TYPE = 0,
	BEGINNING_TUNNEL_ENTRY_TYPE,
	L2_TUNNEL_ENTRY_TYPE,
	L3_TUNNEL_ENTRY_TYPE,
	TUNNEL_ENTRY_HEADER_TYPE,
	FIRST_TUNNEL_EXIT_TCAM_TYPE = 0x10,
	SECOND_TUNNEL_EXIT_TCAM_TYPE,
	EGRESS_TUNNEL_EXIT_TBL_TYPE,
	INVALID_TUNNEL_TYPE_TYPE = 0xFF,
};

int dubhe2000_tunnel_table_get_new_index(u8 tunnel_type)
{
	t_TunnelEntryInstructionTable tunnel_instruct;
	t_BeginningofPacketTunnelEntryInstructionTable beginning_instruct;
	t_L2TunnelEntryInstructionTable l2_instruct;
	t_L3TunnelEntryInstructionTable l3_instruct;
	t_TunnelEntryHeaderData tunnel_header;
	t_TunnelExitLookupTCAM first_exit;
	t_SecondTunnelExitLookupTCAM second_exit;
	//t_EgressTunnelExitTable egress_exit;
	int i;

	if (tunnel_type == TUNNEL_ENTRY_INSTRUCT_TYPE) {
		for (i = 0; i < TunnelEntryInstructionTable_nr_entries; i++) {
			memset(&tunnel_instruct, 0, sizeof(tunnel_instruct));

			rd_TunnelEntryInstructionTable(g_adapter, i, &tunnel_instruct);

			if (dubhe1000_check_u8_array_is_zero((void *)&tunnel_instruct, sizeof(tunnel_instruct)))
				return i;
		}
	} else if (tunnel_type == BEGINNING_TUNNEL_ENTRY_TYPE) {
		for (i = 0; i < BeginningofPacketTunnelEntryInstructionTable_nr_entries; i++) {
			memset(&beginning_instruct, 0, sizeof(beginning_instruct));

			rd_BeginningofPacketTunnelEntryInstructionTable(g_adapter, i, &beginning_instruct);

			if (dubhe1000_check_u8_array_is_zero((void *)&beginning_instruct, sizeof(beginning_instruct)))
				return i;
		}
	} else if (tunnel_type == L2_TUNNEL_ENTRY_TYPE) {
		for (i = 0; i < L2TunnelEntryInstructionTable_nr_entries; i++) {
			memset(&l2_instruct, 0, sizeof(l2_instruct));

			rd_L2TunnelEntryInstructionTable(g_adapter, i, &l2_instruct);

			if (dubhe1000_check_u8_array_is_zero((void *)&l2_instruct, sizeof(l2_instruct)))
				return i;
		}
	} else if (tunnel_type == L3_TUNNEL_ENTRY_TYPE) {
		for (i = 0; i < L3TunnelEntryInstructionTable_nr_entries; i++) {
			memset(&l3_instruct, 0, sizeof(l3_instruct));

			rd_L3TunnelEntryInstructionTable(g_adapter, i, &l3_instruct);

			if (dubhe1000_check_u8_array_is_zero((void *)&l3_instruct, sizeof(l3_instruct)))
				return i;
		}
	} else if (tunnel_type == TUNNEL_ENTRY_HEADER_TYPE) {
		for (i = 0; i < TunnelEntryHeaderData_nr_entries; i++) {
			memset(&tunnel_header, 0, sizeof(tunnel_header));

			rd_TunnelEntryHeaderData(g_adapter, i, &tunnel_header);

			if (dubhe1000_check_u8_array_is_zero((void *)&tunnel_header, sizeof(tunnel_header)))
				return i;
		}
	} else if (tunnel_type == FIRST_TUNNEL_EXIT_TCAM_TYPE) {
		for (i = 0; i < TunnelExitLookupTCAM_nr_entries; i++) {
			memset(&first_exit, 0, sizeof(first_exit));

			rd_TunnelExitLookupTCAM(g_adapter, i, &first_exit);

			if (!first_exit.valid)
				return i;
		}
	} else if (tunnel_type == SECOND_TUNNEL_EXIT_TCAM_TYPE) {
		for (i = 0; i < SecondTunnelExitLookupTCAM_nr_entries; i++) {
			memset(&second_exit, 0, sizeof(second_exit));

			rd_SecondTunnelExitLookupTCAM(g_adapter, i, &second_exit);

			if (!second_exit.valid)
				return i;
		}
	}

	return -EINVAL;
}

/*
 * 1. All pppoe discover packets should be send to cpuport
 * 2. For pppoe session packet,
 *     If sessiod-id has been added by tc_acl, it will be processed by tunnel_exit.
 *     If not, it will be send to cpuport
 */
void dubhe2000_tunnel_exit_init_config(struct dubhe1000_adapter *adapter)
{
	t_SourcePortTable source_port;
	t_TunnelExitLookupTCAM exit_tcam;
	t_TunnelExitLookupTCAMAnswer exit_answer;
	u8 tmp_mask[10] = { 0xFF, 0xFF, 0xFF, 0xFF, 0x0, 0x0, 0xFF, 0xFF, 0x0, 0x0 };
	int i;

	//step1: Source Port Table, only wan port; disableTunnelExit = 0, firstHitSecondMissToCpu = 1
	for (i = 0; i <= DUBHE2000_PORT_MAX; i++) {
		memset(&source_port, 0, sizeof(source_port));
		rd_SourcePortTable(adapter, i, &source_port);

		if (i == adapter->wan_port) {
			source_port.disableTunnelExit = 0;
			source_port.firstHitSecondMissSendToCpu = 1;
		} else {
			source_port.disableTunnelExit = 0;
			source_port.firstHitSecondMissSendToCpu = 0;
		}

		wr_SourcePortTable(adapter, i, &source_port); //tunnel should be enable for rx packet in wan port
	}

	//step2: for pppoe discover packet to cpuport
	// pppoe discovery: Tunnel Exit Lookup TCAM
	memset(&exit_tcam, 0, sizeof(exit_tcam));
	exit_tcam.valid = 1;
	exit_tcam.ethTypemask = 0xFFFF;
	exit_tcam.ethType = 0x8863; //PPPoE discovery
	/* other filed in exit_tcam is zero, will not be compared*/
	wr_TunnelExitLookupTCAM(adapter, 0, &exit_tcam); //hardcode index0

	// pppoe discovery: Tunnel Exit Lookup TCAM Answer
	memset(&exit_answer, 0, sizeof(exit_answer));
	exit_answer.srcPortMask = 1 << adapter->wan_port; //only wan
	exit_answer.sendToCpu = 1;
	wr_TunnelExitLookupTCAMAnswer(adapter, 0, &exit_answer);

	//step3: for pppoe session packet
	// pppoe session: Tunnel Exit Lookup TCAM
	memset(&exit_tcam, 0, sizeof(exit_tcam));
	exit_tcam.valid = 1;
	exit_tcam.ethTypemask = 0xFFFF;
	exit_tcam.ethType = 0x8864; //PPPoE Session
	/* other filed in exit_tcam is zero, will not be compared*/
	wr_TunnelExitLookupTCAM(adapter, 1, &exit_tcam); // hardcode 1

	//pppoe session: Tunnel Exit Lookup TCAM Answer
	memset(&exit_answer, 0, sizeof(exit_answer));
	exit_answer.srcPortMask = 1 << adapter->wan_port; //only wan
	exit_answer.sendToCpu = 0;
	/*
	 * Because tc_acl's params will include sessiod-id, Second tunnel exit will start from pppoe header.
	 * And we should cover the case how many vlans in packet
	 */
	exit_answer.secondShift = 14;	   //the length of mac header
	exit_answer.secondIncludeVlan = 1; //it means that "secondShift += the length of vlan headers(0/1/2)"
	exit_answer.direct = 0;		   //For second lookup, value from packet to use in instead of direct value.
	memset(exit_answer.key, 0, sizeof(exit_answer.key));			  // unused because dircet = 0
	memcpy(exit_answer.lookupMask, tmp_mask, sizeof(exit_answer.lookupMask)); //same as pktKeymask
	exit_answer.tabIndex = 0;						  //unused, only tabkey also can work
	wr_TunnelExitLookupTCAMAnswer(adapter, 1, &exit_answer);

	/*
	 * Second Tunnel Exit Lookup TCAM/ANswer and EgressTunnelExitTable will be configured after tc_acl
	 * When IPV4 or IPV6 and seesion-id are known, dubhe2000_tunnel_exit_config_session will be called
	 */
}

int dubhe2000_tunnel_exit_delete_session(struct dubhe1000_adapter *adapter, u32 index)
{
	t_SecondTunnelExitLookupTCAM second_tcam;
	t_SecondTunnelExitLookupTCAMAnswer second_answer;
	t_EgressTunnelExitTable egress_exit;

	if (index >= SecondTunnelExitLookupTCAM_nr_entries) {
		pr_err("[%s] invalid index\n", __func__);
		return -EINVAL;
	}

	memset(&second_tcam, 0, sizeof(second_tcam));
	wr_SecondTunnelExitLookupTCAM(adapter, index, &second_tcam);

	memset(&second_answer, 0, sizeof(second_answer));
	wr_SecondTunnelExitLookupTCAMAnswer(adapter, index, &second_answer);

	memset(&egress_exit, 0, sizeof(egress_exit));
	wr_EgressTunnelExitTable(adapter, index, &egress_exit);

	return 0;
}

int dubhe2000_tunnel_exit_config_session(struct dubhe1000_adapter *adapter, u16 session_id, bool is_ipv4)
{
	t_SecondTunnelExitLookupTCAM second_tcam;
	t_SecondTunnelExitLookupTCAMAnswer second_answer;
	t_EgressTunnelExitTable egress_exit;
	u8 tmp_ipv4[10] = { 0x11, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x21, 0x0, 0x0 };
	u8 tmp_ipv6[10] = { 0x11, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x57, 0x0, 0x0 };
	u8 tmp_mask[10] = { 0xFF, 0xFF, 0xFF, 0xFF, 0x0, 0x0, 0xFF, 0xFF, 0x0, 0x0 };
	int index; //The index of the 3 second tunnel exit tables is the same
	u16 etype;

	index = dubhe2000_tunnel_table_get_new_index(SECOND_TUNNEL_EXIT_TCAM_TYPE);
	if (index < 0) {
		pr_err("[%s] Second Tunnel Exit TCAM: No Space!\n", __func__);
		return -EINVAL;
	}

	if (g_adapter->tc_debug)
		pr_info("[%s] Second Tunnel Exit TCAM: session_id=%d %s index=%d!\n", __func__, session_id,
			is_ipv4 ? "ipv4" : "ipv6", index);

	etype = is_ipv4 ? 0x0008 : 0xdd86;

	//step1: Second Tunnel Exit Lookup TCAM
	memset(&second_tcam, 0, sizeof(second_tcam));

	if (is_ipv4)
		memcpy(second_tcam.pktKey, tmp_ipv4, sizeof(second_tcam.pktKey));
	else
		memcpy(second_tcam.pktKey, tmp_ipv6, sizeof(second_tcam.pktKey));

	second_tcam.valid = 1;
	second_tcam.pktKey[1] = 0x0; //PPPoE code = 0x0: session type
	second_tcam.pktKey[2] = (session_id >> 8) & 0xFF;
	second_tcam.pktKey[3] = session_id & 0xFF;
	//mask: protocol version,type,code,session id,ppp Protocol-ID in PPPoE - Session Header
	memcpy(second_tcam.pktKeymask, tmp_mask, sizeof(tmp_mask));
	second_tcam.tabKey = 0;	    //tabIndex is unused in exit tcam
	second_tcam.tabKeymask = 0; //tabKey is unused
	wr_SecondTunnelExitLookupTCAM(adapter, index, &second_tcam);

	//step2: Second Tunnel Exit Lookup TCAM Answer
	memset(&second_answer, 0, sizeof(second_answer));
	second_answer.howManyBytesToRemove = 8; //remove PPPoE - Session Header
	second_answer.updateEthType = 1;
	second_answer.ethType = etype;
	/* pppoe sessiod pop will not remove vlan or change l4 part or change vid */
	second_answer.removeVlan = 0;
	second_answer.updateL4Protocol = 0;
	second_answer.l4Protocol = 0;
	second_answer.whereToRemove = 1; //After L2 and up to two VLAN headers
	second_answer.dropPkt = 0;
	second_answer.dontExit = 0;
	second_answer.replaceVid = 0;
	second_answer.newVid = 0;
	second_answer.tunnelExitEgressPtr = index;
	wr_SecondTunnelExitLookupTCAMAnswer(adapter, index, &second_answer);

	//step3: Egress Tunnel Exit Table
	/* index is equal to 'tunnelExitEgressPtr' in SecondTunnelExitLookupTCAMAnswer
	 * we should maintain this table consistency with SecondTunnelExitLookupTCAMAnswer
	 */
	memset(&egress_exit, 0, sizeof(egress_exit));
	egress_exit.howManyBytesToRemove = 8;
	egress_exit.updateEthType = 1;
	egress_exit.ethType = etype;
	egress_exit.removeVlan = 0;
	egress_exit.updateL4Protocol = 0;
	egress_exit.l4Protocol = 0;
	egress_exit.whereToRemove = 1; //After L2 and up to two VLAN headers
	wr_EgressTunnelExitTable(adapter, index, &egress_exit);

	return index;
}

//case1: PPPoE Session Header PUSH
void dubhe2000_tunnel_entry_init_config(struct dubhe1000_adapter *adapter)
{
	//no init work
}

/* Note: The tunnelEntry/tunnelEntryPtr in corresponing Next_Hop_Table should be also cleared. */
int dubhe2000_tunnel_entry_delete_session(struct dubhe1000_adapter *adapter, u32 index)
{
	t_TunnelEntryInstructionTable tunnel_instruct;
	t_L2TunnelEntryInstructionTable l2_instruct;
	t_TunnelEntryHeaderData tunnel_header;

	if (index >= TunnelEntryInstructionTable_nr_entries) {
		pr_err("[%s] invalid index\n", __func__);
		return -EINVAL;
	}

	memset(&tunnel_instruct, 0, sizeof(tunnel_instruct));
	wr_TunnelEntryInstructionTable(adapter, index, &tunnel_instruct);

	memset(&l2_instruct, 0, sizeof(l2_instruct));
	rd_L2TunnelEntryInstructionTable(adapter, index, &l2_instruct);

	memset(&tunnel_header, 0, sizeof(tunnel_header));
	wr_TunnelEntryHeaderData(adapter, index, &tunnel_header);

	return 0;
}

/* Tunnel Entry will only use L2 Tunnel Entry Instruction Table for PPPOE Tunnel
 * TunnelEntryInstructionTable/L2TunnelEntryInstructionTable/TunnelEntryHeaderData have same index
 * Note: the return index must be assgined in corresponing Next_Hop_Table
 */
int dubhe2000_tunnel_entry_config_session(struct dubhe1000_adapter *adapter, u16 session_id, bool is_ipv4)
{
	t_TunnelEntryInstructionTable tunnel_instruct;
	t_L2TunnelEntryInstructionTable l2_instruct;
	t_TunnelEntryHeaderData tunnel_header;
	u8 tmp_ipv4[8] = { 0x11, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x21 };
	u8 tmp_ipv6[8] = { 0x11, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x57 };
	int index;

	index = dubhe2000_tunnel_table_get_new_index(TUNNEL_ENTRY_INSTRUCT_TYPE);
	if (index < 0) {
		pr_err("[%s] Tunnel Entry Header Data: No Space!\n", __func__);
		return -EINVAL;
	}

	if (g_adapter->tc_debug)
		pr_info("[%s] Tunnel Entry Header Type: session_id=%d %s index=%d!\n", __func__, session_id,
			is_ipv4 ? "ipv4" : "ipv6", index);

	//step1: Tunnel Entry Instruction Table
	memset(&tunnel_instruct, 0, sizeof(tunnel_instruct));
	/* After L2 and up to two VLAN headers. described in L2 Tunnel Entry Instruction Table */
	tunnel_instruct.tunnelEntryType = 1;
	//Let's calculate the Payload Length in PPPoE Sessoin: exclude mac header and pppoe header
	/* The insertLength's description of in datasheet is opposite */
	tunnel_instruct.insertLength = 1; //Yes. Insert(overwrite) a length field(2 Bytes).
	tunnel_instruct.lengthPos = 4;	  //In pppoe session, the offset of payload length is 4 Bytes
	/* The region of pppoe payload length doesn't include from the
	 * beginning of the ethernet header to the payload length domain
	 */
	tunnel_instruct.lengthNegOffset =
		20; // length cal start from ppp protocol-id. So it is ETH_HDR(14) + PPPoE Header(6)
	tunnel_instruct.lengthPosOffset = 0;
	tunnel_instruct.incVlansInLength = 0;
	tunnel_instruct.tunnelHeaderLen = 8;	 // the length of tunnel header: (PPPoE Header(6) + PPP-Protocol-ID(2))
	tunnel_instruct.tunnelHeaderPtr = index; // pointer to Tunnel Entry Header Data
	wr_TunnelEntryInstructionTable(adapter, index, &tunnel_instruct);

	//step2: L2 Tunnel Entry Instruction Table
	memset(&l2_instruct, 0, sizeof(l2_instruct));
	l2_instruct.l3Type = 3; // other, a pppoe session header will be inserted. not ipv4/ipv6
	l2_instruct.hasUdp = 0;
	l2_instruct.updateEtherType = 1;
	l2_instruct.outerEtherType = 0x6488; // the etype of pppoe session-id is 0x8864
	wr_L2TunnelEntryInstructionTable(adapter, index, &l2_instruct);

	//step3: Tunnel Entry Header Data
	memset(&tunnel_header, 0, sizeof(tunnel_header));
	if (is_ipv4)
		memcpy(&tunnel_header.data, &tmp_ipv4, sizeof(tmp_ipv4));
	else
		memcpy(&tunnel_header.data, &tmp_ipv6, sizeof(tmp_ipv6));

	tunnel_header.data[1] = 0x0; //PPPoE code = 0x0: session type
	tunnel_header.data[2] = (session_id >> 8) & 0xFF;
	tunnel_header.data[3] = session_id & 0xFF;

	wr_TunnelEntryHeaderData(adapter, index, &tunnel_header);

	return index;
}

void dubhe2000_tunnel_init_config(struct dubhe1000_adapter *adapter)
{
	//pppoe header pop
	dubhe2000_tunnel_exit_init_config(adapter);

	//pppoe header push
	dubhe2000_tunnel_entry_init_config(adapter);
}
