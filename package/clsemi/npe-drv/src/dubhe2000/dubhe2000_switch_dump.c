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

/* dubhe2000_switch_dummp.c
 * Shared functions for print Switch all tables
 */

#include "dubhe2000.h"

#define CLS_SWITCH_MEMBER_BUILD_VALUE(structure, member, buffer) do {                                                  \
	memset(buffer, 0, sizeof(buffer));                                                                             \
	snprintf(buffer, 256, "\t\t\"%s\" =\t0x%llx\n", #member, (u64)(structure)->member);                            \
	pr_info("%s", buffer);                                                                                         \
} while (0)

#define CLS_SWITCH_MEMBER_BUILD_U32(structure, member, buffer) do {                                                    \
	memset(buffer, 0, sizeof(buffer));                                                                             \
	snprintf(buffer, 256, "\t\t\"%s\" =\t%u\n", #member, (u32)(structure)->member);                                \
	pr_info("%s", buffer);                                                                                         \
} while (0)

//biggest width is TunnelEntryHeaderData 640; 10 * u64 can cover all case
#define CLS_SWITCH_MEMBER_BUILD_ARRAY(structure, member, buffer) do {                                                  \
	int i, j, offset, len;                                                                                         \
	u64 value[10];                                                                                                 \
	memset(buffer, 0, sizeof(buffer));                                                                             \
	memset(value, 0, sizeof(value));                                                                               \
	memcpy(value, (structure)->member, sizeof((structure)->member));                                               \
	offset = snprintf(buffer, 64, "\t\t\"%s\" =\t", #member);                                                      \
	len = offset;                                                                                                  \
	for (i = 9; i > 0; i--) {                                                                                      \
		if (value[i]) {                                                                                        \
			for (j = i; j > 0; j--) {                                                                      \
				offset = snprintf(buffer + len, 64, " 0x%llx", value[j]);                              \
				len += offset;                                                                         \
			}                                                                                              \
			break;                                                                                         \
		}                                                                                                      \
	}                                                                                                              \
	snprintf(buffer + len, 64, " 0x%llx\n", value[0]);                                                             \
	pr_info("%s", buffer);                                                                                         \
} while (0)

//FOR IngressNATOperation/EgressNATOperation/acl newIpValue
#define CLS_SWITCH_MEMBER_BUILD_VALUE_IPV4(structure, member, buffer) do {                                             \
	u8 ip[4];                                                                                                      \
	u32 value = (structure)->member;                                                                               \
	memset(buffer, 0, sizeof(buffer));                                                                             \
	memcpy(ip, &value, 4);                                                                                         \
	snprintf(buffer, 64, "\t\t\"%s\" =\t%u.%u.%u.%u\n", #member, ip[3], ip[2], ip[1], ip[0]);                      \
	pr_info("%s", buffer);                                                                                         \
} while (0)

//FOR HashBasedL3RoutingTable/L3RoutingTCAM
#define CLS_SWITCH_MEMBER_BUILD_ARRAY_IPV4(structure, member, buffer) do {                                             \
	u8 ip[4];                                                                                                      \
	memset(buffer, 0, sizeof(buffer));                                                                             \
	memcpy(ip, (structure)->member, 4);                                                                            \
	snprintf(buffer, 64, "\t\t\"%s\" =\t%u.%u.%u.%u\n", #member, ip[3], ip[2], ip[1], ip[0]);                      \
	pr_info("%s", buffer);                                                                                         \
} while (0)

// 2001:1234::1 --- u32 ipAddr[4] = {0x1, 0x0, 0x0, 0x20011234};
#define CLS_SWITCH_MEMBER_BUILD_ARRAY_IPV6(structure, member, buffer) do {                                             \
	u16 ip[8];                                                                                                     \
	memset(buffer, 0, sizeof(buffer));                                                                             \
	memcpy(ip, (structure)->member, 16);                                                                           \
	snprintf(buffer, 64, "\t\t\"%s\" =\t%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\n", #member,                       \
		 ip[0], ip[1], ip[2], ip[3], ip[4], ip[5], ip[6], ip[7]);                                              \
	pr_info("%s", buffer);                                                                                         \
} while (0)

#define CLS_SWITCH_MEMBER_BUILD_MACADDR(structure, member, buffer) do {                                                \
	u8 mac[ETH_ALEN];                                                                                              \
	u64 value = (structure)->member;                                                                               \
	memset(buffer, 0, sizeof(buffer));                                                                             \
	memcpy(mac, &value, ETH_ALEN);                                                                                 \
	snprintf(buffer, 64, "\t\t\"%s\" =\t%02x:%02x:%02x:%02x:%02x:%02x\n", #member,                                 \
		 mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);                                                      \
	pr_info("%s", buffer);                                                                                         \
} while (0)

void cls_switch_dump_DrainPortDrop_entry(void *data)
{
	t_DrainPortDrop *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, packets, msg);
}

void cls_switch_dump_IngressMultipleSpanningTreeState_entry(void *data)
{
	t_IngressMultipleSpanningTreeState *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, portSptState, msg);
}

void cls_switch_dump_SourcePortTable_entry(void *data)
{
	t_SourcePortTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, aclRule2, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ignoreVlanMembership, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, aclRule0, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, natPortState, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, enableDefaultPortAcl, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, imUnderVlanMembership, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, typeSel, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, spt, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, defaultPcpIf, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropUnknownDa, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, aclRule1, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, preLookupAclBits, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, priorityVid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, enableFromCpuTag, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, natActionTableEnable, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, enablePriorityTag, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, defaultCfiDeiIf, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, enableL2ActionTable, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, defaultVid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, destInputMirror, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, learnMulticastSaMac, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, defaultVidIf, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, vlanAssignment, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, vidSel, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, aclRule3, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, useAcl1, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, pcpSel, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, defaultCfiDei, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, nrVlansVidOperationIf, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, defaultVidOrder, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, typeSelIf, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, minAllowedVlans, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, inputMirrorEnabled, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forcePortAclAction, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, disableTunnelExit, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, cfiDeiSelIf, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, colorFromL3, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, pcpSelIf, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, disableRouting, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, useAcl2, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, useAcl3, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, useAcl0, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, cfiDeiSel, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, prioFromL3, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, defaultPcp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, l2ActionTablePortState, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, maxAllowedVlans, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, vlanSingleOpIf, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, firstHitSecondMissSendToCpu, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, vidSelIf, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, imUnderPortIsolation, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, vlanSingleOp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, learningEn, msg);
}

void cls_switch_dump_L2ActionTablePerPortDrop_entry(void *data)
{
	t_L2ActionTablePerPortDrop *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, packets, msg);
}

void cls_switch_dump_RouterMTUTable_entry(void *data)
{
	t_RouterMTUTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, maxIPv6MTU, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, maxIPv4MTU, msg);
}

void cls_switch_dump_DebugEPPCounter_entry(void *data)
{
	t_DebugEPPCounter *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, packets, msg);
}

void cls_switch_dump_EgressConfigurableACL1TCAMAnswer_entry(void *data)
{
	t_EgressConfigurableACL1TCAMAnswer *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaDataValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, destPort, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropEnable, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaDataPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, counter, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelEntryPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelEntryUcMc, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateCounter, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelEntryPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceSendToCpuOrigPkt, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, sendToCpu, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelEntry, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, sendToPort, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaData, msg);
}

void cls_switch_dump_IngressEgressPacketFilteringDrop_entry(void *data)
{
	t_IngressEgressPacketFilteringDrop *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, packets, msg);
}

void cls_switch_dump_IngressVIDMACRangeAssignmentAnswer_entry(void *data)
{
	t_IngressVIDMACRangeAssignmentAnswer *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ingressVid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, order, msg);
}

void cls_switch_dump_NextHopMPLSTable_entry(void *data)
{
	t_NextHopMPLSTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, expSel, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mplsOperation, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, exp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, label, msg);
}

void cls_switch_dump_EgressPortDisabledDrop_entry(void *data)
{
	t_EgressPortDisabledDrop *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, packets, msg);
}

void cls_switch_dump_IPv4TOSFieldToPacketColorMappingTable_entry(void *data)
{
	t_IPv4TOSFieldToPacketColorMappingTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, color, msg);
}

void cls_switch_dump_EnableEnqueueToPortsAndQueues_entry(void *data)
{
	t_EnableEnqueueToPortsAndQueues *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, qon, msg);
}

void cls_switch_dump_MPLSEXPFieldToEgressQueueMappingTable_entry(void *data)
{
	t_MPLSEXPFieldToEgressQueueMappingTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, pQueue, msg);
}

void cls_switch_dump_PortReserved_entry(void *data)
{
	t_PortReserved *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, cells, msg);
}

void cls_switch_dump_ResourceLimiterSet_entry(void *data)
{
	t_ResourceLimiterSet *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, yellowLimit, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, yellowAccumulated, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, maxCells, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, redLimit, msg);
}

void cls_switch_dump_EgressConfigurableACL0TCAMAnswer_entry(void *data)
{
	t_EgressConfigurableACL0TCAMAnswer *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, counter, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, sendToCpu, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaDataValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, destPort, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropEnable, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaDataPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, natOpPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelEntryPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelEntryUcMc, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, natOpPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateCounter, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelEntryPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceSendToCpuOrigPkt, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, natOpValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelEntry, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, sendToPort, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaData, msg);
}

void cls_switch_dump_EgressResourceManagerPointer_entry(void *data)
{
	t_EgressResourceManagerPointer *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, q1, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, q0, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, q3, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, q2, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, q5, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, q4, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, q7, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, q6, msg);
}

void cls_switch_dump_IPUnicastReceivedCounter_entry(void *data)
{
	t_IPUnicastReceivedCounter *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, packets, msg);
}

void cls_switch_dump_EgressConfigurableACL0RulesSetup_entry(void *data)
{
	t_EgressConfigurableACL0RulesSetup *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, fieldSelectBitmask, msg);
}

void cls_switch_dump_SMONSetSearch_entry(void *data)
{
	t_SMONSetSearch *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, srcPort, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, vid, msg);
}

void cls_switch_dump_L2MulticastStormControlRateConfiguration_entry(void *data)
{
	t_L2MulticastStormControlRateConfiguration *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tokens, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, packetsNotBytes, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tick, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ifgCorrection, msg);
}

void cls_switch_dump_SPOverflowDrop_entry(void *data)
{
	t_SPOverflowDrop *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, packets, msg);
}

void cls_switch_dump_PortShaperBucketThresholdConfiguration_entry(void *data)
{
	t_PortShaperBucketThresholdConfiguration *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, threshold, msg);
}

void cls_switch_dump_AllowSpecialFrameCheckForL2ActionTable_entry(void *data)
{
	t_AllowSpecialFrameCheckForL2ActionTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dontAllowIGMP, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dontAllowBPDU, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dontAllowAH, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dontAllowCAPWAP, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dontAllowGRE, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dontAllowMPLS, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dontAllowARP, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dontAllowRARP, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dontAllowDNS, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dontAllowBOOTPDHCP, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dontAllowIPV4, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dontAllowIPV6, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dontAllowL21588, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dontAllowL41588, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dontAllowESP, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dontAllow8021XEAPOL, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dontAllowLLDP, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dontAllowICMP, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dontAllowL2McReserved, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dontAllowTCP, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dontAllowUDP, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dontAllowSCTP, msg);
}

void cls_switch_dump_PortTailDropSettings_entry(void *data)
{
	t_PortTailDropSettings *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, enable, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mode, msg);
}

void cls_switch_dump_PortShaperRateConfiguration_entry(void *data)
{
	t_PortShaperRateConfiguration *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tokens, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, packetsNotBytes, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tick, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ifgCorrection, msg);
}

void cls_switch_dump_DWRRBucketMiscConfiguration_entry(void *data)
{
	t_DWRRBucketMiscConfiguration *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, packetsNotBytes, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, threshold, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ifgCorrection, msg);
}

void cls_switch_dump_EgressConfigurableACL1RulesSetup_entry(void *data)
{
	t_EgressConfigurableACL1RulesSetup *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, fieldSelectBitmask, msg);
}

void cls_switch_dump_EgressConfigurableACL0LargeTable_entry(void *data)
{
	t_EgressConfigurableACL0LargeTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_ARRAY(tmp, compareData, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, counter, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, sendToCpu, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaDataValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, destPort, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropEnable, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaDataPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, natOpPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelEntryPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelEntryUcMc, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, valid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, natOpPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateCounter, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelEntryPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceSendToCpuOrigPkt, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, natOpValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelEntry, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, sendToPort, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaData, msg);
}

void cls_switch_dump_RouterPortEgressSAMACAddress_entry(void *data)
{
	t_RouterPortEgressSAMACAddress *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, selectMacEntryPortMask, msg);
	CLS_SWITCH_MEMBER_BUILD_MACADDR(tmp, altMacAddress, msg);
}

void cls_switch_dump_OutputDisable_entry(void *data)
{
	t_OutputDisable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, egressQueue3Disabled, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, egressQueue7Disabled, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, egressQueue2Disabled, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, egressQueue6Disabled, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, egressQueue1Disabled, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, egressQueue5Disabled, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, egressQueue0Disabled, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, egressQueue4Disabled, msg);
}

void cls_switch_dump_L2AgingCollisionTable_entry(void *data)
{
	t_L2AgingCollisionTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, stat, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, valid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, hit, msg);
}

void cls_switch_dump_IngressAdmissionControlInitialPointer_entry(void *data)
{
	t_IngressAdmissionControlInitialPointer *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpOrder, msg);
}

void cls_switch_dump_L2TunnelEntryInstructionTable_entry(void *data)
{
	t_L2TunnelEntryInstructionTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, l3Type, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateEtherType, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, outerEtherType, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, hasUdp, msg);
}

void cls_switch_dump_IngressConfigurableACL0LargeTable_entry(void *data)
{
	t_IngressConfigurableACL0LargeTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_U32(tmp, newL4Value, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, color, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaDataPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceColorPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, enableUpdateIp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, sendToPort, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaData, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, eQueue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE_IPV4(tmp, newIpValue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateL4SpOrDp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, natOpValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, natOpPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, enableUpdateL4, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceQueuePrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, valid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, natOpPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateCounter, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceQueue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tosMask, msg);
	CLS_SWITCH_MEMBER_BUILD_ARRAY(tmp, compareData, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, imPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateTosExp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, sendToCpu, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateSaOrDa, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpOrder, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, inputMirror, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceColor, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, destInputMirror, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaDataValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropEnable, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, counter, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, newTosExp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceSendToCpuOrigPkt, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, destPort, msg);
}

void cls_switch_dump_EgressPortDepth_entry(void *data)
{
	t_EgressPortDepth *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, packets, msg);
}

void cls_switch_dump_EgressConfigurableACL0SmallTable_entry(void *data)
{
	t_EgressConfigurableACL0SmallTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_ARRAY(tmp, compareData, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, counter, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, sendToCpu, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaDataValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, destPort, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropEnable, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaDataPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, natOpPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelEntryPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelEntryUcMc, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, valid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, natOpPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateCounter, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelEntryPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceSendToCpuOrigPkt, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, natOpValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelEntry, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, sendToPort, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaData, msg);
}

void cls_switch_dump_SelectWhichEgressQoSMappingTableToUse_entry(void *data)
{
	t_SelectWhichEgressQoSMappingTableToUse *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, whichTablePtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updatePcp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, pcp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, whichTableToUse, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateCfiDei, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, cfiDei, msg);
}

void cls_switch_dump_DWRRWeightConfiguration_entry(void *data)
{
	t_DWRRWeightConfiguration *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, weight, msg);
}

void cls_switch_dump_LinkAggregateWeight_entry(void *data)
{
	t_LinkAggregateWeight *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ports, msg);
}

void cls_switch_dump_L3RoutingDefault_entry(void *data)
{
	t_L3RoutingDefault *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, nextHop, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpOrder, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, pktDrop, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, sendToCpu, msg);
}

void cls_switch_dump_TransmittedPacketsonEgressVRF_entry(void *data)
{
	t_TransmittedPacketsonEgressVRF *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, packets, msg);
}

void cls_switch_dump_IngressAdmissionControlReset_entry(void *data)
{
	t_IngressAdmissionControlReset *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, bucketReset, msg);
}

void cls_switch_dump_PrioShaperBucketCapacityConfiguration_entry(void *data)
{
	t_PrioShaperBucketCapacityConfiguration *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, bucketCapacity, msg);
}

void cls_switch_dump_EgressVLANTranslationTCAM_entry(void *data)
{
	t_EgressVLANTranslationTCAM *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, outermostVidTypemask, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dstPortmask, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, outermostVid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, outermostVidType, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, valid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, outermostVidmask, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dstPort, msg);
}

void cls_switch_dump_L2LookupCollisionTable_entry(void *data)
{
	t_L2LookupCollisionTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, gid, msg);
	CLS_SWITCH_MEMBER_BUILD_MACADDR(tmp, macAddr, msg);
}

void cls_switch_dump_NextHopPacketModifications_entry(void *data)
{
	t_NextHopPacketModifications *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, innerPcpSel, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, innerCfiDei, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, outerCfiDeiSel, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, innerCfiDeiSel, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, msptPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, outerVlanAppend, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, outerCfiDei, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, innerEthType, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, outerVid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, outerPcpSel, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, innerPcp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, valid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, outerEthType, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, outerPcp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, innerVlanAppend, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, innerVid, msg);
}

void cls_switch_dump_IPv4TOSFieldToEgressQueueMappingTable_entry(void *data)
{
	t_IPv4TOSFieldToEgressQueueMappingTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, pQueue, msg);
}

void cls_switch_dump_IngressVIDInnerVIDRangeAssignmentAnswer_entry(void *data)
{
	t_IngressVIDInnerVIDRangeAssignmentAnswer *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ingressVid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, order, msg);
}

void cls_switch_dump_SecondTunnelExitLookupTCAMAnswer_entry(void *data)
{
	t_SecondTunnelExitLookupTCAMAnswer *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, newVid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateL4Protocol, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ethType, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropPkt, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, replaceVid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelExitEgressPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateEthType, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, l4Protocol, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dontExit, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, whereToRemove, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, removeVlan, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, howManyBytesToRemove, msg);
}

void cls_switch_dump_EgressPortConfiguration_entry(void *data)
{
	t_EgressPortConfiguration *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, vid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropStaggedVlans, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, disabled, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropCtaggedVlans, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, typeSel, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, cfiDei, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, removeSNAP, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropSingleTaggedVlans, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropSStaggedVlans, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, pcp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, vidSel, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, pcpSel, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, colorRemap, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, vlanSingleOp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropCCtaggedVlans, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropSCtaggedVlans, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, cfiDeiSel, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropUntaggedVlans, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, moreThanOneVlans, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, useEgressQueueRemapping, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropDualTaggedVlans, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropCStaggedVlans, msg);
}

void cls_switch_dump_EgressSpanningTreeDrop_entry(void *data)
{
	t_EgressSpanningTreeDrop *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, packets, msg);
}

void cls_switch_dump_IngressConfigurableACL0TCAM_entry(void *data)
{
	t_IngressConfigurableACL0TCAM *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, valid, msg);
	CLS_SWITCH_MEMBER_BUILD_ARRAY(tmp, mask, msg);
	CLS_SWITCH_MEMBER_BUILD_ARRAY(tmp, compareData, msg);
}

void cls_switch_dump_L2AgingStatusShadowTable_entry(void *data)
{
	t_L2AgingStatusShadowTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, valid, msg);
}

void cls_switch_dump_TCXoffFFAThreshold_entry(void *data)
{
	t_TCXoffFFAThreshold *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, cells, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, enable, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, trip, msg);
}

void cls_switch_dump_EgressConfigurableACL1TCAM_entry(void *data)
{
	t_EgressConfigurableACL1TCAM *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, valid, msg);
	CLS_SWITCH_MEMBER_BUILD_ARRAY(tmp, mask, msg);
	CLS_SWITCH_MEMBER_BUILD_ARRAY(tmp, compareData, msg);
}

void cls_switch_dump_L2AgingTable_entry(void *data)
{
	t_L2AgingTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, stat, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, valid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, hit, msg);
}

void cls_switch_dump_IngressRouterTable_entry(void *data)
{
	t_IngressRouterTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, acceptIPv6, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ecmpUseIpL4Dp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, acceptIPv4, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ecmpUseIpSa, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ipv4HitUpdates, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ecmpUseIpProto, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ipv6HitUpdates, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, minTtlToCpu, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ecmpUseIpDa, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpOrder, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mplsHitUpdates, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ecmpUseIpTos, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ecmpUseIpL4Sp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, acceptMPLS, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, minTTL, msg);
}

void cls_switch_dump_NextHopPacketInsertMPLSHeader_entry(void *data)
{
	t_NextHopPacketInsertMPLSHeader *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ttl0, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ttl1, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ttl2, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ttl3, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, copyTtl3, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, copyTtl2, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, copyTtl1, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, copyTtl0, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, howManyLabelsToInsert, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, exp3, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, exp2, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, exp1, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, exp0, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, expFromQueue3, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, expFromQueue2, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, expFromQueue1, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, expFromQueue0, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, whichEthernetType, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mplsLabel0, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mplsLabel1, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mplsLabel2, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mplsLabel3, msg);
}

void cls_switch_dump_MACInterfaceCountersForTX_entry(void *data)
{
	t_MACInterfaceCountersForTX *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, packets, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, halt, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, error, msg);
}

void cls_switch_dump_TunnelEntryInstructionTable_entry(void *data)
{
	t_TunnelEntryInstructionTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, incVlansInLength, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, lengthPosOffset, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, insertLength, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelEntryType, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, lengthNegOffset, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelHeaderLen, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelHeaderPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, lengthPos, msg);
}

void cls_switch_dump_IPUnicastRoutedCounter_entry(void *data)
{
	t_IPUnicastRoutedCounter *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, packets, msg);
}

void cls_switch_dump_IngressAdmissionControlCurrentSize_entry(void *data)
{
	t_IngressAdmissionControlCurrentSize *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tokens1, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tokens0, msg);
}

void cls_switch_dump_L2DAHashLookupTable_entry(void *data)
{
	t_L2DAHashLookupTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, gid, msg);
	CLS_SWITCH_MEMBER_BUILD_MACADDR(tmp, macAddr, msg);
}

void cls_switch_dump_IPMulticastReceivedCounter_entry(void *data)
{
	t_IPMulticastReceivedCounter *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, packets, msg);
}

void cls_switch_dump_TCFFAUsed_entry(void *data)
{
	t_TCFFAUsed *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, cells, msg);
}

void cls_switch_dump_EgressNATOperation_entry(void *data)
{
	t_EgressNATOperation *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, replaceSrc, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, replaceL4Port, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, replaceIP, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE_IPV4(tmp, ipAddress, msg);
	CLS_SWITCH_MEMBER_BUILD_U32(tmp, port, msg);
}

void cls_switch_dump_IngressConfigurableACL1TCAMAnswer_entry(void *data)
{
	t_IngressConfigurableACL1TCAMAnswer *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, counter, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, newEthType, msg);
	CLS_SWITCH_MEMBER_BUILD_U32(tmp, newL4Value, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, color, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateVid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceColorPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceSendToCpuOrigPkt, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceVid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelEntryPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, enableUpdateIp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, sendToPort, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ptp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, newPcpValue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, cfiDeiPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, eQueue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE_IPV4(tmp, newIpValue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateL4SpOrDp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceColor, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, natOpPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, enableUpdateL4, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceQueuePrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelEntryPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateTosExp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, natOpValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, natOpPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaDataPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, pcpPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaData, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceQueue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelEntry, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, vidPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, imPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ethPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, sendToCpu, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateSaOrDa, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpOrder, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, destPort, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateCounter, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, newVidValue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, inputMirror, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceVidPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceVidValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, destInputMirror, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaDataValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropEnable, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateEType, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, newTosExp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateCfiDei, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tosMask, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelEntryUcMc, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, noLearning, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updatePcp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, newCfiDeiValue, msg);
}

void cls_switch_dump_IngressConfigurableACL3TCAMAnswer_entry(void *data)
{
	t_IngressConfigurableACL3TCAMAnswer *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, eQueue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaDataValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, color, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, sendToCpu, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceColorPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpOrder, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceSendToCpuOrigPkt, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropEnable, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceColor, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceQueuePrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaDataPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceQueue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, destPort, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, sendToPort, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaData, msg);
}

void cls_switch_dump_TunnelExitLookupTCAMAnswer_entry(void *data)
{
	t_TunnelExitLookupTCAMAnswer *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, secondShift, msg);
	CLS_SWITCH_MEMBER_BUILD_ARRAY(tmp, lookupMask, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, direct, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, secondIncludeVlan, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, srcPortMask, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, key, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, sendToCpu, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tabIndex, msg);
}

void cls_switch_dump_ReservedSourceMACAddressRange_entry(void *data)
{
	t_ReservedSourceMACAddressRange *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_MACADDR(tmp, stopAddr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, enable, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, eQueue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceQueue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, color, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, sendToCpu, msg);
	CLS_SWITCH_MEMBER_BUILD_MACADDR(tmp, startAddr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpOrder, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropEnable, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceColor, msg);
}

void cls_switch_dump_QueueOffDrop_entry(void *data)
{
	t_QueueOffDrop *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, packets, msg);
}

void cls_switch_dump_TunnelEntryHeaderData_entry(void *data)
{
	t_TunnelEntryHeaderData *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_ARRAY(tmp, data, msg);
}

void cls_switch_dump_QueueShaperBucketThresholdConfiguration_entry(void *data)
{
	t_QueueShaperBucketThresholdConfiguration *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, threshold, msg);
}

void cls_switch_dump_IngressConfigurableACL0TCAMAnswer_entry(void *data)
{
	t_IngressConfigurableACL0TCAMAnswer *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_U32(tmp, newL4Value, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, color, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaDataPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceColorPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, enableUpdateIp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, sendToPort, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaData, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, eQueue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE_IPV4(tmp, newIpValue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateL4SpOrDp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, natOpValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, natOpPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, enableUpdateL4, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceQueuePrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, natOpPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateCounter, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceQueue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tosMask, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, imPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateTosExp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, sendToCpu, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateSaOrDa, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpOrder, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, inputMirror, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceColor, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, destInputMirror, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaDataValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropEnable, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, counter, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, newTosExp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceSendToCpuOrigPkt, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, destPort, msg);
}

void cls_switch_dump_EgressConfigurableACLMatchCounter_entry(void *data)
{
	t_EgressConfigurableACLMatchCounter *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, packets, msg);
}

void cls_switch_dump_IngressVIDOuterVIDRangeSearchData_entry(void *data)
{
	t_IngressVIDOuterVIDRangeSearchData *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, start, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, vtype, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, end, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ports, msg);
}

void cls_switch_dump_EgressVLANTranslationTCAMAnswer_entry(void *data)
{
	t_EgressVLANTranslationTCAMAnswer *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, newVid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ethType, msg);
}

void cls_switch_dump_L2ActionTableSourcePort_entry(void *data)
{
	t_L2ActionTableSourcePort *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, useSpecialAllow, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropPortMove, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, sendToCpu, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, drop, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, noPortMove, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpOrder, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceSendToCpuOrigPkt, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, noLearningUc, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropAll, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, allowPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, noLearningMc, msg);
}

void cls_switch_dump_FloodingActionSendtoPort_entry(void *data)
{
	t_FloodingActionSendtoPort *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, enable, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, destPort, msg);
}

void cls_switch_dump_EgressMPLSTTLTable_entry(void *data)
{
	t_EgressMPLSTTLTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, newTTL, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, addNewTTL, msg);
}

void cls_switch_dump_MBSCDrop_entry(void *data)
{
	t_MBSCDrop *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, packets, msg);
}

void cls_switch_dump_EgressResourceManagerDrop_entry(void *data)
{
	t_EgressResourceManagerDrop *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, packets, msg);
}

void cls_switch_dump_MACRXBrokenPackets_entry(void *data)
{
	t_MACRXBrokenPackets *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, packets, msg);
}

void cls_switch_dump_IngressVIDOuterVIDRangeAssignmentAnswer_entry(void *data)
{
	t_IngressVIDOuterVIDRangeAssignmentAnswer *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ingressVid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, order, msg);
}

void cls_switch_dump_IngressVIDMACRangeSearchData_entry(void *data)
{
	t_IngressVIDMACRangeSearchData *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, saOrDa, msg);
	CLS_SWITCH_MEMBER_BUILD_MACADDR(tmp, start, msg);
	CLS_SWITCH_MEMBER_BUILD_MACADDR(tmp, end, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ports, msg);
}

void cls_switch_dump_IngressNATHitStatus_entry(void *data)
{
	t_IngressNATHitStatus *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, hit, msg);
}

void cls_switch_dump_ColorRemapFromIngressAdmissionControl_entry(void *data)
{
	t_ColorRemapFromIngressAdmissionControl *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, color2Tos, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, colorMode, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, enable, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, color2Dei, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tosMask, msg);
}

void cls_switch_dump_HairpinEnable_entry(void *data)
{
	t_HairpinEnable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, allowUc, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, allowMc, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, allowFlood, msg);
}

void cls_switch_dump_MPLSEXPFieldToPacketColorMappingTable_entry(void *data)
{
	t_MPLSEXPFieldToPacketColorMappingTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, color, msg);
}

void cls_switch_dump_IPMulticastACLDropCounter_entry(void *data)
{
	t_IPMulticastACLDropCounter *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, packets, msg);
}

void cls_switch_dump_QueueShaperBucketCapacityConfiguration_entry(void *data)
{
	t_QueueShaperBucketCapacityConfiguration *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, bucketCapacity, msg);
}

void cls_switch_dump_IngressVIDInnerVIDRangeSearchData_entry(void *data)
{
	t_IngressVIDInnerVIDRangeSearchData *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, start, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, vtype, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, end, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ports, msg);
}

void cls_switch_dump_NextHopHitStatus_entry(void *data)
{
	t_NextHopHitStatus *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mpls, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ipv4, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ipv6, msg);
}

void cls_switch_dump_EgressQueueToMPLSEXPMappingTable_entry(void *data)
{
	t_EgressQueueToMPLSEXPMappingTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, exp, msg);
}

void cls_switch_dump_EgressRouterTable_entry(void *data)
{
	t_EgressRouterTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, newTTL, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, addNewTTL, msg);
}

void cls_switch_dump_MACRXLongPacketDrop_entry(void *data)
{
	t_MACRXLongPacketDrop *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, packets, msg);
}

void cls_switch_dump_PortXoffFFAThreshold_entry(void *data)
{
	t_PortXoffFFAThreshold *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, cells, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, enable, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, trip, msg);
}

void cls_switch_dump_TCXonFFAThreshold_entry(void *data)
{
	t_TCXonFFAThreshold *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, cells, msg);
}

void cls_switch_dump_LinkAggregationToPhysicalPortsMembers_entry(void *data)
{
	t_LinkAggregationToPhysicalPortsMembers *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, members, msg);
}

void cls_switch_dump_L3TunnelEntryInstructionTable_entry(void *data)
{
	t_L3TunnelEntryInstructionTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, l4Protocol, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateL4Type, msg);
}

void cls_switch_dump_SecondTunnelExitLookupTCAM_entry(void *data)
{
	t_SecondTunnelExitLookupTCAM *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tabKeymask, msg);
	CLS_SWITCH_MEMBER_BUILD_ARRAY(tmp, pktKey, msg);
	CLS_SWITCH_MEMBER_BUILD_ARRAY(tmp, pktKeymask, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tabKey, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, valid, msg);
}

void cls_switch_dump_MapQueuetoPriority_entry(void *data)
{
	t_MapQueuetoPriority *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, prio3, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, prio2, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, prio1, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, prio0, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, prio7, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, prio6, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, prio5, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, prio4, msg);
}

void cls_switch_dump_NextHopDAMAC_entry(void *data)
{
	t_NextHopDAMAC *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_MACADDR(tmp, daMac, msg);
}

void cls_switch_dump_ReceivedPacketsonIngressVRF_entry(void *data)
{
	t_ReceivedPacketsonIngressVRF *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, packets, msg);
}

void cls_switch_dump_IngressConfigurableACL1PreLookup_entry(void *data)
{
	t_IngressConfigurableACL1PreLookup *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, valid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, rulePtr, msg);
}

void cls_switch_dump_MACRXMaximumPacketLength_entry(void *data)
{
	t_MACRXMaximumPacketLength *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, bytes, msg);
}

void cls_switch_dump_EgressPortFilteringDrop_entry(void *data)
{
	t_EgressPortFilteringDrop *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, packets, msg);
}

void cls_switch_dump_IngressConfigurableACL0RulesSetup_entry(void *data)
{
	t_IngressConfigurableACL0RulesSetup *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, fieldSelectBitmask, msg);
}

void cls_switch_dump_MACRXShortPacketDrop_entry(void *data)
{
	t_MACRXShortPacketDrop *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, packets, msg);
}

void cls_switch_dump_IngressConfigurableACL2PreLookup_entry(void *data)
{
	t_IngressConfigurableACL2PreLookup *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, valid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, rulePtr, msg);
}

void cls_switch_dump_PortTCTailDropTotalThreshold_entry(void *data)
{
	t_PortTCTailDropTotalThreshold *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, cells, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, enable, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, trip, msg);
}

void cls_switch_dump_VLANPCPToQueueMappingTable_entry(void *data)
{
	t_VLANPCPToQueueMappingTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, pQueue, msg);
}

void cls_switch_dump_L2MulticastStormControlBucketCapacityConfiguration_entry(void *data)
{
	t_L2MulticastStormControlBucketCapacityConfiguration *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, bucketCapacity, msg);
}

void cls_switch_dump_L2MulticastTable_entry(void *data)
{
	t_L2MulticastTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mcPortMask, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaData, msg);
}

void cls_switch_dump_ReservedDestinationMACAddressRange_entry(void *data)
{
	t_ReservedDestinationMACAddressRange *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_MACADDR(tmp, stopAddr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, enable, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, eQueue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceQueue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, color, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, sendToCpu, msg);
	CLS_SWITCH_MEMBER_BUILD_MACADDR(tmp, startAddr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpOrder, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropEnable, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceColor, msg);
}

void cls_switch_dump_TCTailDropFFAThreshold_entry(void *data)
{
	t_TCTailDropFFAThreshold *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, cells, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, enable, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, trip, msg);
}

void cls_switch_dump_IngressConfigurableACL1SmallTable_entry(void *data)
{
	t_IngressConfigurableACL1SmallTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, counter, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, newEthType, msg);
	CLS_SWITCH_MEMBER_BUILD_U32(tmp, newL4Value, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, color, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateVid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceColorPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceSendToCpuOrigPkt, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceVid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelEntryPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, enableUpdateIp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, sendToPort, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ptp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, newPcpValue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, cfiDeiPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, eQueue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE_IPV4(tmp, newIpValue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateL4SpOrDp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceColor, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, natOpPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, enableUpdateL4, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceQueuePrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelEntryPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateTosExp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, valid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, natOpValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, natOpPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaDataPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, pcpPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaData, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceQueue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelEntry, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, vidPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_ARRAY(tmp, compareData, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, imPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ethPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, sendToCpu, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateSaOrDa, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpOrder, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, destPort, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateCounter, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, newVidValue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, inputMirror, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceVidPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceVidValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, destInputMirror, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaDataValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropEnable, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateEType, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, newTosExp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateCfiDei, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tosMask, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelEntryUcMc, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, noLearning, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updatePcp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, newCfiDeiValue, msg);
}

void cls_switch_dump_IngressVIDEthernetTypeRangeSearchData_entry(void *data)
{
	t_IngressVIDEthernetTypeRangeSearchData *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, start, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, end, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ports, msg);
}

void cls_switch_dump_L2FloodingStormControlRateConfiguration_entry(void *data)
{
	t_L2FloodingStormControlRateConfiguration *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tokens, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, packetsNotBytes, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tick, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ifgCorrection, msg);
}

void cls_switch_dump_IngressConfigurableACL1LargeTable_entry(void *data)
{
	t_IngressConfigurableACL1LargeTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, counter, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, newEthType, msg);
	CLS_SWITCH_MEMBER_BUILD_U32(tmp, newL4Value, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, color, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateVid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceColorPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceSendToCpuOrigPkt, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceVid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelEntryPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, enableUpdateIp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, sendToPort, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ptp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, newPcpValue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, cfiDeiPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, eQueue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE_IPV4(tmp, newIpValue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateL4SpOrDp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceColor, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, natOpPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, enableUpdateL4, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceQueuePrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelEntryPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateTosExp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, valid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, natOpValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, natOpPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaDataPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, pcpPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaData, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceQueue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelEntry, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, vidPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_ARRAY(tmp, compareData, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, imPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ethPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, sendToCpu, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateSaOrDa, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpOrder, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, destPort, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateCounter, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, newVidValue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, inputMirror, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceVidPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceVidValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, destInputMirror, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaDataValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropEnable, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateEType, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, newTosExp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateCfiDei, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tosMask, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelEntryUcMc, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, noLearning, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updatePcp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, newCfiDeiValue, msg);
}

void cls_switch_dump_PrioShaperRateConfiguration_entry(void *data)
{
	t_PrioShaperRateConfiguration *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tokens, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, packetsNotBytes, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tick, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ifgCorrection, msg);
}

void cls_switch_dump_IPMulticastRoutedCounter_entry(void *data)
{
	t_IPMulticastRoutedCounter *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, packets, msg);
}

void cls_switch_dump_PFCIncCountersforingressports0to5_entry(void *data)
{
	t_PFCIncCountersforingressports0to5 *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, cells, msg);
}

void cls_switch_dump_L2FloodingStormControlBucketCapacityConfiguration_entry(void *data)
{
	t_L2FloodingStormControlBucketCapacityConfiguration *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, bucketCapacity, msg);
}

void cls_switch_dump_IPv6ClassofServiceFieldToPacketColorMappingTable_entry(void *data)
{
	t_IPv6ClassofServiceFieldToPacketColorMappingTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, color, msg);
}

void cls_switch_dump_L2DestinationTable_entry(void *data)
{
	t_L2DestinationTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, l2ActionTableDaStatus, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, destPortormcAddr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, pktDrop, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, l2ActionTableSaStatus, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, uc, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaData, msg);
}

void cls_switch_dump_EgressMultipleSpanningTreeState_entry(void *data)
{
	t_EgressMultipleSpanningTreeState *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, portSptState, msg);
}

void cls_switch_dump_PortTCXonTotalThreshold_entry(void *data)
{
	t_PortTCXonTotalThreshold *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, cells, msg);
}

void cls_switch_dump_HardwareLearningCounter_entry(void *data)
{
	t_HardwareLearningCounter *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, cnt, msg);
}

void cls_switch_dump_L2ReservedMulticastAddressAction_entry(void *data)
{
	t_L2ReservedMulticastAddressAction *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropMask, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, sendToCpuMask, msg);
}

void cls_switch_dump_L2BroadcastStormControlBucketThresholdConfiguration_entry(void *data)
{
	t_L2BroadcastStormControlBucketThresholdConfiguration *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, threshold, msg);
}

void cls_switch_dump_BeginningofPacketTunnelEntryInstructionTable_entry(void *data)
{
	t_BeginningofPacketTunnelEntryInstructionTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, l3Type, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ipHeaderOffset, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, hasUdp, msg);
}

void cls_switch_dump_TOSQoSMappingTable_entry(void *data)
{
	t_TOSQoSMappingTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateExp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updatePcp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, pcp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateCfiDei, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, newTos, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, cfiDei, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, newExp, msg);
}

void cls_switch_dump_L2LookupCollisionTableMasks_entry(void *data)
{
	t_L2LookupCollisionTableMasks *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, gid, msg);
	CLS_SWITCH_MEMBER_BUILD_MACADDR(tmp, macAddr, msg);
}

void cls_switch_dump_IngressConfigurableACL2RulesSetup_entry(void *data)
{
	t_IngressConfigurableACL2RulesSetup *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, fieldSelectBitmask, msg);
}

void cls_switch_dump_UnknownEgressDrop_entry(void *data)
{
	t_UnknownEgressDrop *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, packets, msg);
}

void cls_switch_dump_EgressNATHitStatus_entry(void *data)
{
	t_EgressNATHitStatus *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, hit, msg);
}

void cls_switch_dump_PFCDecCountersforingressports0to5_entry(void *data)
{
	t_PFCDecCountersforingressports0to5 *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, cells, msg);
}

void cls_switch_dump_DebugIPPCounter_entry(void *data)
{
	t_DebugIPPCounter *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, packets, msg);
}

void cls_switch_dump_PrioShaperBucketThresholdConfiguration_entry(void *data)
{
	t_PrioShaperBucketThresholdConfiguration *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, threshold, msg);
}

void cls_switch_dump_PortTailDropFFAThreshold_entry(void *data)
{
	t_PortTailDropFFAThreshold *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, cells, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, enable, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, trip, msg);
}

void cls_switch_dump_MPLSQoSMappingTable_entry(void *data)
{
	t_MPLSQoSMappingTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, cfiDei, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updatePcp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, pcp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateCfiDei, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, exp, msg);
}

void cls_switch_dump_TunnelExitTooSmallPacketModificationToSmallDrop_entry(void *data)
{
	t_TunnelExitTooSmallPacketModificationToSmallDrop *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, packets, msg);
}

void cls_switch_dump_IPv6ClassofServiceFieldToEgressQueueMappingTable_entry(void *data)
{
	t_IPv6ClassofServiceFieldToEgressQueueMappingTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, pQueue, msg);
}

void cls_switch_dump_QueueShaperRateConfiguration_entry(void *data)
{
	t_QueueShaperRateConfiguration *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tokens, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, packetsNotBytes, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tick, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ifgCorrection, msg);
}

void cls_switch_dump_EgressQueueDepth_entry(void *data)
{
	t_EgressQueueDepth *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, packets, msg);
}

void cls_switch_dump_IngressConfigurableACL2TCAMAnswer_entry(void *data)
{
	t_IngressConfigurableACL2TCAMAnswer *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, counter, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, newEthType, msg);
	CLS_SWITCH_MEMBER_BUILD_U32(tmp, newL4Value, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, color, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateVid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceColorPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceSendToCpuOrigPkt, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceVid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelEntryPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, enableUpdateIp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, sendToPort, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ptp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, newPcpValue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, cfiDeiPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, eQueue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE_IPV4(tmp, newIpValue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateL4SpOrDp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceColor, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, natOpPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, enableUpdateL4, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceQueuePrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelEntryPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateTosExp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, natOpValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, natOpPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaDataPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, pcpPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaData, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceQueue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelEntry, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, vidPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, imPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ethPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, sendToCpu, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateSaOrDa, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpOrder, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, destPort, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateCounter, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, newVidValue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, inputMirror, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceVidPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceVidValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, destInputMirror, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaDataValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropEnable, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateEType, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, newTosExp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateCfiDei, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tosMask, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelEntryUcMc, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, noLearning, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updatePcp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, newCfiDeiValue, msg);
}

void cls_switch_dump_DefaultPacketToCPUModification_entry(void *data)
{
	t_DefaultPacketToCPUModification *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, origCpuPkt, msg);
}

void cls_switch_dump_RouterPortMACAddress_entry(void *data)
{
	t_RouterPortMACAddress *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_MACADDR(tmp, macAddress, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, selectMacEntryPortMask, msg);
	CLS_SWITCH_MEMBER_BUILD_MACADDR(tmp, macMask, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, valid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, vrf, msg);
	CLS_SWITCH_MEMBER_BUILD_MACADDR(tmp, altMacAddress, msg);
}

void cls_switch_dump_EgressConfigurableACL0TCAM_entry(void *data)
{
	t_EgressConfigurableACL0TCAM *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, valid, msg);
	CLS_SWITCH_MEMBER_BUILD_ARRAY(tmp, mask, msg);
	CLS_SWITCH_MEMBER_BUILD_ARRAY(tmp, compareData, msg);
}

void cls_switch_dump_IngressConfigurableACL2TCAM_entry(void *data)
{
	t_IngressConfigurableACL2TCAM *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, valid, msg);
	CLS_SWITCH_MEMBER_BUILD_ARRAY(tmp, mask, msg);
	CLS_SWITCH_MEMBER_BUILD_ARRAY(tmp, compareData, msg);
}

void cls_switch_dump_EgressACLRulePointerTCAM_entry(void *data)
{
	t_EgressACLRulePointerTCAM *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ucSwitchedmask, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, vrfmask, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, destPortMask, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, vid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ucSwitched, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, flooded, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, l3Type, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, srcPort, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, l4Type, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, routed, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, floodedmask, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, l3Typemask, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, valid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, srcPortmask, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, vrf, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mcSwitched, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mcSwitchedmask, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, l4Typemask, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, routedmask, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, vidmask, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, destPortMaskmask, msg);
}

void cls_switch_dump_EgressQueueToPCPAndCFIDEIMappingTable_entry(void *data)
{
	t_EgressQueueToPCPAndCFIDEIMappingTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, cfiDei, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, pcp, msg);
}

void cls_switch_dump_L2AgingCollisionShadowTable_entry(void *data)
{
	t_L2AgingCollisionShadowTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, valid, msg);
}

void cls_switch_dump_RouterEgressQueueToVLANData_entry(void *data)
{
	t_RouterEgressQueueToVLANData *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, cfiDei, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, pcp, msg);
}

void cls_switch_dump_LinkAggregationMembership_entry(void *data)
{
	t_LinkAggregationMembership *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, la, msg);
}

void cls_switch_dump_IngressConfigurableACL3RulesSetup_entry(void *data)
{
	t_IngressConfigurableACL3RulesSetup *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, fieldSelectBitmask, msg);
}

void cls_switch_dump_L2ActionTable_entry(void *data)
{
	t_L2ActionTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, useSpecialAllow, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropPortMove, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, sendToCpu, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, drop, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, noPortMove, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpOrder, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceSendToCpuOrigPkt, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, noLearningUc, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropAll, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, allowPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, noLearningMc, msg);
}

void cls_switch_dump_TunnelExitLookupTCAM_entry(void *data)
{
	t_TunnelExitLookupTCAM *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, l4Protocolmask, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ethType, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ethTypemask, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, valid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, l4Dp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, l3Type, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, snapLlc, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, l4Type, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, l4Spmask, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, snapLlcmask, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, l3Typemask, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, l4Dpmask, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, l4Protocol, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, l4Typemask, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, l4Sp, msg);

	if (tmp->l3Type == 1) { //IPV6
		CLS_SWITCH_MEMBER_BUILD_ARRAY_IPV6(tmp, ipDa, msg);
		CLS_SWITCH_MEMBER_BUILD_ARRAY_IPV6(tmp, ipSamask, msg);
		CLS_SWITCH_MEMBER_BUILD_ARRAY_IPV6(tmp, ipSa, msg);
		CLS_SWITCH_MEMBER_BUILD_ARRAY_IPV6(tmp, ipDamask, msg);
	} else {
		CLS_SWITCH_MEMBER_BUILD_ARRAY_IPV4(tmp, ipDa, msg);
		CLS_SWITCH_MEMBER_BUILD_ARRAY_IPV4(tmp, ipSamask, msg);
		CLS_SWITCH_MEMBER_BUILD_ARRAY_IPV4(tmp, ipSa, msg);
		CLS_SWITCH_MEMBER_BUILD_ARRAY_IPV4(tmp, ipDamask, msg);
	}
}

void cls_switch_dump_IngressConfigurableACLMatchCounter_entry(void *data)
{
	t_IngressConfigurableACLMatchCounter *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, packets, msg);
}

void cls_switch_dump_OutputMirroringTable_entry(void *data)
{
	t_OutputMirroringTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, outputMirrorEnabled, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, outputMirrorPort, msg);
}

void cls_switch_dump_PortUsed_entry(void *data)
{
	t_PortUsed *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, cells, msg);
}

void cls_switch_dump_SecondTunnelExitMissAction_entry(void *data)
{
	t_SecondTunnelExitMissAction *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropIfMiss, msg);
}

void cls_switch_dump_IngressConfigurableACL0SmallTable_entry(void *data)
{
	t_IngressConfigurableACL0SmallTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_U32(tmp, newL4Value, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, color, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaDataPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceColorPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, enableUpdateIp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, sendToPort, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaData, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, eQueue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE_IPV4(tmp, newIpValue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateL4SpOrDp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, natOpValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, natOpPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, enableUpdateL4, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceQueuePrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, valid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, natOpPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateCounter, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceQueue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tosMask, msg);
	CLS_SWITCH_MEMBER_BUILD_ARRAY(tmp, compareData, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, imPrio, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateTosExp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, sendToCpu, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateSaOrDa, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpOrder, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, inputMirror, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceColor, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, destInputMirror, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaDataValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropEnable, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, counter, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, newTosExp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceSendToCpuOrigPkt, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, destPort, msg);
}

void cls_switch_dump_PortTCReserved_entry(void *data)
{
	t_PortTCReserved *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, cells, msg);
}

void cls_switch_dump_L2QoSMappingTable_entry(void *data)
{
	t_L2QoSMappingTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, cfiDei, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updatePcp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, pcp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateCfiDei, msg);
}

void cls_switch_dump_EgressTunnelExitTable_entry(void *data)
{
	t_EgressTunnelExitTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, l4Protocol, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, howManyBytesToRemove, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateL4Protocol, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateEthType, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ethType, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, whereToRemove, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, removeVlan, msg);
}

void cls_switch_dump_MACInterfaceCountersForRX_entry(void *data)
{
	t_MACInterfaceCountersForRX *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, packets, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, error, msg);
}

void cls_switch_dump_NextHopTable_entry(void *data)
{
	t_NextHopTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, nextHopPacketMod, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, l2Uc, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, validEntry, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, srv6Sid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, destPortormcAddr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelExit, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, pktDrop, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelExitPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelEntryPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, sendToCpu, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelEntry, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaData, msg);
}

void cls_switch_dump_EgressACLRulePointerTCAMAnswer_entry(void *data)
{
	t_EgressACLRulePointerTCAMAnswer *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, rulePtr0, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, rulePtr1, msg);
}

void cls_switch_dump_SourcePortDefaultACLAction_entry(void *data)
{
	t_SourcePortDefaultACLAction *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, counter, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, newEthType, msg);
	CLS_SWITCH_MEMBER_BUILD_U32(tmp, newL4Value, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, color, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateVid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelEntryPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, enableUpdateIp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, sendToPort, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ptp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, newPcpValue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, eQueue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, newCfiDeiValue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE_IPV4(tmp, newIpValue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateL4SpOrDp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, sendToCpu, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, enableUpdateL4, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceVid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, natOpValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, natOpPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateCounter, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaData, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceQueue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelEntry, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateTosExp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateSaOrDa, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpOrder, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, newVidValue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, inputMirror, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceColor, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceVidValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, destInputMirror, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, metaDataValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropEnable, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateEType, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, newTosExp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateCfiDei, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceSendToCpuOrigPkt, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tunnelEntryUcMc, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, noLearning, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updatePcp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tosMask, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, destPort, msg);
}

void cls_switch_dump_VLANTable_entry(void *data)
{
	t_VLANTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, vid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, typeSel, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, cfiDeiIf, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, cfiDei, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpValid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, vlanSingleOp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, msptPtr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, pcp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, gid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, pcpIf, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, vidSel, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, pcpSel, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, vlanPortMask, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, nrVlansVidOperationIf, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, typeSelIf, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, pcpSelIf, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, cfiDeiSelIf, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpOrder, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, sendIpMcToCpu, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, cfiDeiSel, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, allowRouting, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, vlanSingleOpIf, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, vidIf, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, vidSelIf, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mmpPtr, msg);
}

void cls_switch_dump_IngressPortPacketTypeFilter_entry(void *data)
{
	t_IngressPortPacketTypeFilter *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropUntaggedVlans, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropIPv4Packets, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropIPv4MulticastPackets, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropDualTaggedVlans, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropL2MulticastFrames, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropSStaggedVlans, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, moreThanOneVlans, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropSingleTaggedVlans, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropIPv6MulticastPackets, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropCCtaggedVlans, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropIPv6Packets, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropSCtaggedVlans, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropMPLSPackets, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropCtaggedVlans, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropCStaggedVlans, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropStaggedVlans, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropL2BroadcastFrames, msg);
}

void cls_switch_dump_PortFFAUsed_entry(void *data)
{
	t_PortFFAUsed *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, cells, msg);
}

void cls_switch_dump_DefaultLearningStatus_entry(void *data)
{
	t_DefaultLearningStatus *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, stat, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, valid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, hit, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, learnLimit, msg);
}

void cls_switch_dump_IngressVIDEthernetTypeRangeAssignmentAnswer_entry(void *data)
{
	t_IngressVIDEthernetTypeRangeAssignmentAnswer *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ingressVid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, order, msg);
}

void cls_switch_dump_L2MulticastStormControlBucketThresholdConfiguration_entry(void *data)
{
	t_L2MulticastStormControlBucketThresholdConfiguration *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, threshold, msg);
}

void cls_switch_dump_IngressConfigurableACL3TCAM_entry(void *data)
{
	t_IngressConfigurableACL3TCAM *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, valid, msg);
	CLS_SWITCH_MEMBER_BUILD_ARRAY(tmp, mask, msg);
	CLS_SWITCH_MEMBER_BUILD_ARRAY(tmp, compareData, msg);
}

void cls_switch_dump_L2BroadcastStormControlBucketCapacityConfiguration_entry(void *data)
{
	t_L2BroadcastStormControlBucketCapacityConfiguration *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, bucketCapacity, msg);
}

void cls_switch_dump_NATActionTable_entry(void *data)
{
	t_NATActionTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, action, msg);
}

void cls_switch_dump_IngressConfigurableACL1RulesSetup_entry(void *data)
{
	t_IngressConfigurableACL1RulesSetup *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, fieldSelectBitmask, msg);
}

void cls_switch_dump_VLANPCPAndDEIToColorMappingTable_entry(void *data)
{
	t_VLANPCPAndDEIToColorMappingTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, color, msg);
}

void cls_switch_dump_IngressEgressPortPacketTypeFilter_entry(void *data)
{
	t_IngressEgressPortPacketTypeFilter *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropUntaggedVlans, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropIPv4Packets, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropIPv4MulticastPackets, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropL2MulticastFrames, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropDualTaggedVlans, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropCtaggedVlans, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropSStaggedVlans, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, moreThanOneVlans, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, srcPortFilter, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropSingleTaggedVlans, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropIPv6MulticastPackets, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropCCtaggedVlans, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropSCtaggedVlans, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropIPv6Packets, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropRouted, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropMPLSPackets, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropL2FloodingFrames, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropCStaggedVlans, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropStaggedVlans, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropL2BroadcastFrames, msg);
}

void cls_switch_dump_ColorRemapFromEgressPort_entry(void *data)
{
	t_ColorRemapFromEgressPort *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, color2Tos, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, colorMode, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, color2Dei, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tosMask, msg);
}

void cls_switch_dump_IngressNATOperation_entry(void *data)
{
	t_IngressNATOperation *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, replaceSrc, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, replaceL4Port, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, replaceIP, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE_IPV4(tmp, ipAddress, msg);
	CLS_SWITCH_MEMBER_BUILD_U32(tmp, port, msg);
}

void cls_switch_dump_IPQoSMappingTable_entry(void *data)
{
	t_IPQoSMappingTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateExp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ecnTos, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updatePcp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, pcp, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, updateCfiDei, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, cfiDei, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, newExp, msg);
}

void cls_switch_dump_IngressAdmissionControlTokenBucketConfiguration_entry(void *data)
{
	t_IngressAdmissionControlTokenBucketConfiguration *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, colorBlind, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tick0, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tick1, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, bucketCapacity0, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tokenMode, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, dropMask, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, byteCorrection, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tokens0, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, maxLength, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tokens1, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, bucketCapacity1, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, bucketMode, msg);
}

void cls_switch_dump_L2BroadcastStormControlRateConfiguration_entry(void *data)
{
	t_L2BroadcastStormControlRateConfiguration *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tokens, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, packetsNotBytes, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, tick, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ifgCorrection, msg);
}

void cls_switch_dump_PortTCXoffTotalThreshold_entry(void *data)
{
	t_PortTCXoffTotalThreshold *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, cells, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, enable, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, trip, msg);
}

void cls_switch_dump_L2FloodingStormControlBucketThresholdConfiguration_entry(void *data)
{
	t_L2FloodingStormControlBucketThresholdConfiguration *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, threshold, msg);
}

void cls_switch_dump_CPUReasonCodeOperation_entry(void *data)
{
	t_CPUReasonCodeOperation *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, end, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mutableCpu, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, start, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, origCpuPkt, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, eQueue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceQueue, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, port, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, forceUpdateOrigCpuPkt, msg);
}

void cls_switch_dump_HashBasedL3RoutingTable_entry(void *data)
{
	t_HashBasedL3RoutingTable *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ecmpShift, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mpls, msg);
	if (tmp->ipVersion == 1) //ipv6
		CLS_SWITCH_MEMBER_BUILD_ARRAY_IPV6(tmp, destIPAddr, msg);
	else
		CLS_SWITCH_MEMBER_BUILD_ARRAY_IPV4(tmp, destIPAddr, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, useECMP, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ipVersion, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, nextHopPointer, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, vrf, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ecmpMask, msg);
}

void cls_switch_dump_PortShaperBucketCapacityConfiguration_entry(void *data)
{
	t_PortShaperBucketCapacityConfiguration *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, bucketCapacity, msg);
}

void cls_switch_dump_L3RoutingTCAM_entry(void *data)
{
	t_L3RoutingTCAM *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, proto, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, vrfMaskN, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, protoMaskN, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, valid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, vrf, msg);
	if (tmp->proto == 3) { //ipv6
		CLS_SWITCH_MEMBER_BUILD_ARRAY_IPV6(tmp, destIPAddr, msg);
		CLS_SWITCH_MEMBER_BUILD_ARRAY_IPV6(tmp, destIPAddrMaskN, msg);
	} else {
		CLS_SWITCH_MEMBER_BUILD_ARRAY_IPV4(tmp, destIPAddr, msg);
		CLS_SWITCH_MEMBER_BUILD_ARRAY_IPV4(tmp, destIPAddrMaskN, msg);
	}
}

void cls_switch_dump_PortXonFFAThreshold_entry(void *data)
{
	t_PortXonFFAThreshold *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, cells, msg);
}

void cls_switch_dump_L3LPMResult_entry(void *data)
{
	t_L3LPMResult *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ecmpShift, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, nextHopPointer, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, useECMP, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, ecmpMask, msg);
}

void cls_switch_dump_IngressConfigurableACL1TCAM_entry(void *data)
{
	t_IngressConfigurableACL1TCAM *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, valid, msg);
	CLS_SWITCH_MEMBER_BUILD_ARRAY(tmp, mask, msg);
	CLS_SWITCH_MEMBER_BUILD_ARRAY(tmp, compareData, msg);
}

void cls_switch_dump_PSErrorCounter_entry(void *data)
{
	t_PSErrorCounter *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, overflow, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, underrun, msg);
}

void cls_switch_dump_IngressConfigurableACL0PreLookup_entry(void *data)
{
	t_IngressConfigurableACL0PreLookup *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, valid, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, rulePtr, msg);
}

void cls_switch_dump_DWRRBucketCapacityConfiguration_entry(void *data)
{
	t_DWRRBucketCapacityConfiguration *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, bucketCapacity, msg);
}

void cls_switch_dump_PortPauseSettings_entry(void *data)
{
	t_PortPauseSettings *tmp = data;
	char msg[512];

	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, pattern, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, reserved, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, enable, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, force, msg);
	CLS_SWITCH_MEMBER_BUILD_VALUE(tmp, mode, msg);
}

#define DEFINE_SWITCH_DUMP_FUNC(REGNAME)                                                                               \
	void cls_switch_dump_##REGNAME(int idx)                                                                        \
	{                                                                                                              \
		u32 index, start, end;                                                                                 \
		t_##REGNAME data;                                                                                      \
		char msg[128];                                                                                         \
		int offset;                                                                                            \
		pr_info("==%s:\n", #REGNAME);                                                                          \
		if (idx < 0) {                                                                                         \
			start = 0;                                                                                     \
			end = REGNAME##_nr_entries;                                                                    \
		} else {                                                                                               \
			if (idx >= REGNAME##_nr_entries) {                                                             \
				pr_info("==%s: invalid index\n", #REGNAME);                                            \
				return;                                                                                \
			}                                                                                              \
			start = idx;                                                                                   \
			end = idx + 1;                                                                                 \
		}                                                                                                      \
		for (index = start; index < end; index++) {                                                            \
			memset(msg, 0, sizeof(msg));                                                                   \
			memset(&data, 0, sizeof(data));                                                                \
			rd_##REGNAME(g_adapter, index, &data);                                                         \
			if (!dubhe1000_check_u8_array_is_zero((char *)(&data), sizeof(data))) {                        \
				offset = snprintf(msg, 32, "\t[0x%x]@{\n", index);                                     \
				pr_info("%s", msg);                                                                    \
				cls_switch_dump_##REGNAME##_entry(&data);                                              \
				pr_info("\t}\n");                                                                      \
			}                                                                                              \
		}                                                                                                      \
		pr_info("==%s END!\n", #REGNAME);                                                                      \
	}

DEFINE_SWITCH_DUMP_FUNC(DrainPortDrop);
DEFINE_SWITCH_DUMP_FUNC(IngressMultipleSpanningTreeState);
DEFINE_SWITCH_DUMP_FUNC(SourcePortTable);
DEFINE_SWITCH_DUMP_FUNC(L2ActionTablePerPortDrop);
DEFINE_SWITCH_DUMP_FUNC(RouterMTUTable);
DEFINE_SWITCH_DUMP_FUNC(DebugEPPCounter);
DEFINE_SWITCH_DUMP_FUNC(EgressConfigurableACL1TCAMAnswer);
DEFINE_SWITCH_DUMP_FUNC(IngressEgressPacketFilteringDrop);
DEFINE_SWITCH_DUMP_FUNC(IngressVIDMACRangeAssignmentAnswer);
DEFINE_SWITCH_DUMP_FUNC(NextHopMPLSTable);
DEFINE_SWITCH_DUMP_FUNC(EgressPortDisabledDrop);
DEFINE_SWITCH_DUMP_FUNC(IPv4TOSFieldToPacketColorMappingTable);
DEFINE_SWITCH_DUMP_FUNC(EnableEnqueueToPortsAndQueues);
DEFINE_SWITCH_DUMP_FUNC(MPLSEXPFieldToEgressQueueMappingTable);
DEFINE_SWITCH_DUMP_FUNC(PortReserved);
DEFINE_SWITCH_DUMP_FUNC(ResourceLimiterSet);
DEFINE_SWITCH_DUMP_FUNC(EgressConfigurableACL0TCAMAnswer);
DEFINE_SWITCH_DUMP_FUNC(EgressResourceManagerPointer);
DEFINE_SWITCH_DUMP_FUNC(IPUnicastReceivedCounter);
DEFINE_SWITCH_DUMP_FUNC(EgressConfigurableACL0RulesSetup);
DEFINE_SWITCH_DUMP_FUNC(SMONSetSearch);
DEFINE_SWITCH_DUMP_FUNC(L2MulticastStormControlRateConfiguration);
DEFINE_SWITCH_DUMP_FUNC(SPOverflowDrop);
DEFINE_SWITCH_DUMP_FUNC(PortShaperBucketThresholdConfiguration);
DEFINE_SWITCH_DUMP_FUNC(AllowSpecialFrameCheckForL2ActionTable);
DEFINE_SWITCH_DUMP_FUNC(PortTailDropSettings);
DEFINE_SWITCH_DUMP_FUNC(PortShaperRateConfiguration);
DEFINE_SWITCH_DUMP_FUNC(DWRRBucketMiscConfiguration);
DEFINE_SWITCH_DUMP_FUNC(EgressConfigurableACL1RulesSetup);
DEFINE_SWITCH_DUMP_FUNC(EgressConfigurableACL0LargeTable);
DEFINE_SWITCH_DUMP_FUNC(RouterPortEgressSAMACAddress);
DEFINE_SWITCH_DUMP_FUNC(OutputDisable);
DEFINE_SWITCH_DUMP_FUNC(L2AgingCollisionTable);
DEFINE_SWITCH_DUMP_FUNC(IngressAdmissionControlInitialPointer);
DEFINE_SWITCH_DUMP_FUNC(L2TunnelEntryInstructionTable);
DEFINE_SWITCH_DUMP_FUNC(IngressConfigurableACL0LargeTable);
DEFINE_SWITCH_DUMP_FUNC(EgressPortDepth);
DEFINE_SWITCH_DUMP_FUNC(EgressConfigurableACL0SmallTable);
DEFINE_SWITCH_DUMP_FUNC(SelectWhichEgressQoSMappingTableToUse);
DEFINE_SWITCH_DUMP_FUNC(DWRRWeightConfiguration);
DEFINE_SWITCH_DUMP_FUNC(LinkAggregateWeight);
DEFINE_SWITCH_DUMP_FUNC(L3RoutingDefault);
DEFINE_SWITCH_DUMP_FUNC(TransmittedPacketsonEgressVRF);
DEFINE_SWITCH_DUMP_FUNC(IngressAdmissionControlReset);
DEFINE_SWITCH_DUMP_FUNC(PrioShaperBucketCapacityConfiguration);
DEFINE_SWITCH_DUMP_FUNC(EgressVLANTranslationTCAM);
DEFINE_SWITCH_DUMP_FUNC(L2LookupCollisionTable);
DEFINE_SWITCH_DUMP_FUNC(NextHopPacketModifications);
DEFINE_SWITCH_DUMP_FUNC(IPv4TOSFieldToEgressQueueMappingTable);
DEFINE_SWITCH_DUMP_FUNC(IngressVIDInnerVIDRangeAssignmentAnswer);
DEFINE_SWITCH_DUMP_FUNC(SecondTunnelExitLookupTCAMAnswer);
DEFINE_SWITCH_DUMP_FUNC(EgressPortConfiguration);
DEFINE_SWITCH_DUMP_FUNC(EgressSpanningTreeDrop);
DEFINE_SWITCH_DUMP_FUNC(IngressConfigurableACL0TCAM);
DEFINE_SWITCH_DUMP_FUNC(L2AgingStatusShadowTable);
DEFINE_SWITCH_DUMP_FUNC(TCXoffFFAThreshold);
DEFINE_SWITCH_DUMP_FUNC(EgressConfigurableACL1TCAM);
DEFINE_SWITCH_DUMP_FUNC(L2AgingTable);
DEFINE_SWITCH_DUMP_FUNC(IngressRouterTable);
DEFINE_SWITCH_DUMP_FUNC(NextHopPacketInsertMPLSHeader);
DEFINE_SWITCH_DUMP_FUNC(MACInterfaceCountersForTX);
DEFINE_SWITCH_DUMP_FUNC(TunnelEntryInstructionTable);
DEFINE_SWITCH_DUMP_FUNC(IPUnicastRoutedCounter);
DEFINE_SWITCH_DUMP_FUNC(IngressAdmissionControlCurrentSize);
DEFINE_SWITCH_DUMP_FUNC(L2DAHashLookupTable);
DEFINE_SWITCH_DUMP_FUNC(IPMulticastReceivedCounter);
DEFINE_SWITCH_DUMP_FUNC(TCFFAUsed);
DEFINE_SWITCH_DUMP_FUNC(EgressNATOperation);
DEFINE_SWITCH_DUMP_FUNC(IngressConfigurableACL1TCAMAnswer);
DEFINE_SWITCH_DUMP_FUNC(IngressConfigurableACL3TCAMAnswer);
DEFINE_SWITCH_DUMP_FUNC(TunnelExitLookupTCAMAnswer);
DEFINE_SWITCH_DUMP_FUNC(ReservedSourceMACAddressRange);
DEFINE_SWITCH_DUMP_FUNC(QueueOffDrop);
DEFINE_SWITCH_DUMP_FUNC(TunnelEntryHeaderData);
DEFINE_SWITCH_DUMP_FUNC(QueueShaperBucketThresholdConfiguration);
DEFINE_SWITCH_DUMP_FUNC(IngressConfigurableACL0TCAMAnswer);
DEFINE_SWITCH_DUMP_FUNC(EgressConfigurableACLMatchCounter);
DEFINE_SWITCH_DUMP_FUNC(IngressVIDOuterVIDRangeSearchData);
DEFINE_SWITCH_DUMP_FUNC(EgressVLANTranslationTCAMAnswer);
DEFINE_SWITCH_DUMP_FUNC(L2ActionTableSourcePort);
DEFINE_SWITCH_DUMP_FUNC(FloodingActionSendtoPort);
DEFINE_SWITCH_DUMP_FUNC(EgressMPLSTTLTable);
DEFINE_SWITCH_DUMP_FUNC(MBSCDrop);
DEFINE_SWITCH_DUMP_FUNC(EgressResourceManagerDrop);
DEFINE_SWITCH_DUMP_FUNC(MACRXBrokenPackets);
DEFINE_SWITCH_DUMP_FUNC(IngressVIDOuterVIDRangeAssignmentAnswer);
DEFINE_SWITCH_DUMP_FUNC(IngressVIDMACRangeSearchData);
DEFINE_SWITCH_DUMP_FUNC(IngressNATHitStatus);
DEFINE_SWITCH_DUMP_FUNC(ColorRemapFromIngressAdmissionControl);
DEFINE_SWITCH_DUMP_FUNC(HairpinEnable);
DEFINE_SWITCH_DUMP_FUNC(MPLSEXPFieldToPacketColorMappingTable);
DEFINE_SWITCH_DUMP_FUNC(IPMulticastACLDropCounter);
DEFINE_SWITCH_DUMP_FUNC(QueueShaperBucketCapacityConfiguration);
DEFINE_SWITCH_DUMP_FUNC(IngressVIDInnerVIDRangeSearchData);
DEFINE_SWITCH_DUMP_FUNC(NextHopHitStatus);
DEFINE_SWITCH_DUMP_FUNC(EgressQueueToMPLSEXPMappingTable);
DEFINE_SWITCH_DUMP_FUNC(EgressRouterTable);
DEFINE_SWITCH_DUMP_FUNC(MACRXLongPacketDrop);
DEFINE_SWITCH_DUMP_FUNC(PortXoffFFAThreshold);
DEFINE_SWITCH_DUMP_FUNC(TCXonFFAThreshold);
DEFINE_SWITCH_DUMP_FUNC(LinkAggregationToPhysicalPortsMembers);
DEFINE_SWITCH_DUMP_FUNC(L3TunnelEntryInstructionTable);
DEFINE_SWITCH_DUMP_FUNC(SecondTunnelExitLookupTCAM);
DEFINE_SWITCH_DUMP_FUNC(MapQueuetoPriority);
DEFINE_SWITCH_DUMP_FUNC(NextHopDAMAC);
DEFINE_SWITCH_DUMP_FUNC(ReceivedPacketsonIngressVRF);
DEFINE_SWITCH_DUMP_FUNC(IngressConfigurableACL1PreLookup);
DEFINE_SWITCH_DUMP_FUNC(MACRXMaximumPacketLength);
DEFINE_SWITCH_DUMP_FUNC(EgressPortFilteringDrop);
DEFINE_SWITCH_DUMP_FUNC(IngressConfigurableACL0RulesSetup);
DEFINE_SWITCH_DUMP_FUNC(MACRXShortPacketDrop);
DEFINE_SWITCH_DUMP_FUNC(IngressConfigurableACL2PreLookup);
DEFINE_SWITCH_DUMP_FUNC(PortTCTailDropTotalThreshold);
DEFINE_SWITCH_DUMP_FUNC(VLANPCPToQueueMappingTable);
DEFINE_SWITCH_DUMP_FUNC(L2MulticastStormControlBucketCapacityConfiguration);
DEFINE_SWITCH_DUMP_FUNC(L2MulticastTable);
DEFINE_SWITCH_DUMP_FUNC(ReservedDestinationMACAddressRange);
DEFINE_SWITCH_DUMP_FUNC(TCTailDropFFAThreshold);
DEFINE_SWITCH_DUMP_FUNC(IngressConfigurableACL1SmallTable);
DEFINE_SWITCH_DUMP_FUNC(IngressVIDEthernetTypeRangeSearchData);
DEFINE_SWITCH_DUMP_FUNC(L2FloodingStormControlRateConfiguration);
DEFINE_SWITCH_DUMP_FUNC(IngressConfigurableACL1LargeTable);
DEFINE_SWITCH_DUMP_FUNC(PrioShaperRateConfiguration);
DEFINE_SWITCH_DUMP_FUNC(IPMulticastRoutedCounter);
DEFINE_SWITCH_DUMP_FUNC(PFCIncCountersforingressports0to5);
DEFINE_SWITCH_DUMP_FUNC(L2FloodingStormControlBucketCapacityConfiguration);
DEFINE_SWITCH_DUMP_FUNC(IPv6ClassofServiceFieldToPacketColorMappingTable);
DEFINE_SWITCH_DUMP_FUNC(L2DestinationTable);
DEFINE_SWITCH_DUMP_FUNC(EgressMultipleSpanningTreeState);
DEFINE_SWITCH_DUMP_FUNC(PortTCXonTotalThreshold);
DEFINE_SWITCH_DUMP_FUNC(HardwareLearningCounter);
DEFINE_SWITCH_DUMP_FUNC(L2ReservedMulticastAddressAction);
DEFINE_SWITCH_DUMP_FUNC(L2BroadcastStormControlBucketThresholdConfiguration);
DEFINE_SWITCH_DUMP_FUNC(BeginningofPacketTunnelEntryInstructionTable);
DEFINE_SWITCH_DUMP_FUNC(TOSQoSMappingTable);
DEFINE_SWITCH_DUMP_FUNC(L2LookupCollisionTableMasks);
DEFINE_SWITCH_DUMP_FUNC(IngressConfigurableACL2RulesSetup);
DEFINE_SWITCH_DUMP_FUNC(UnknownEgressDrop);
DEFINE_SWITCH_DUMP_FUNC(EgressNATHitStatus);
DEFINE_SWITCH_DUMP_FUNC(PFCDecCountersforingressports0to5);
DEFINE_SWITCH_DUMP_FUNC(DebugIPPCounter);
DEFINE_SWITCH_DUMP_FUNC(PrioShaperBucketThresholdConfiguration);
DEFINE_SWITCH_DUMP_FUNC(PortTailDropFFAThreshold);
DEFINE_SWITCH_DUMP_FUNC(MPLSQoSMappingTable);
DEFINE_SWITCH_DUMP_FUNC(TunnelExitTooSmallPacketModificationToSmallDrop);
DEFINE_SWITCH_DUMP_FUNC(IPv6ClassofServiceFieldToEgressQueueMappingTable);
DEFINE_SWITCH_DUMP_FUNC(QueueShaperRateConfiguration);
DEFINE_SWITCH_DUMP_FUNC(EgressQueueDepth);
DEFINE_SWITCH_DUMP_FUNC(IngressConfigurableACL2TCAMAnswer);
DEFINE_SWITCH_DUMP_FUNC(DefaultPacketToCPUModification);
DEFINE_SWITCH_DUMP_FUNC(RouterPortMACAddress);
DEFINE_SWITCH_DUMP_FUNC(EgressConfigurableACL0TCAM);
DEFINE_SWITCH_DUMP_FUNC(IngressConfigurableACL2TCAM);
DEFINE_SWITCH_DUMP_FUNC(EgressACLRulePointerTCAM);
DEFINE_SWITCH_DUMP_FUNC(EgressQueueToPCPAndCFIDEIMappingTable);
DEFINE_SWITCH_DUMP_FUNC(L2AgingCollisionShadowTable);
DEFINE_SWITCH_DUMP_FUNC(RouterEgressQueueToVLANData);
DEFINE_SWITCH_DUMP_FUNC(LinkAggregationMembership);
DEFINE_SWITCH_DUMP_FUNC(IngressConfigurableACL3RulesSetup);
DEFINE_SWITCH_DUMP_FUNC(L2ActionTable);
DEFINE_SWITCH_DUMP_FUNC(TunnelExitLookupTCAM);
DEFINE_SWITCH_DUMP_FUNC(IngressConfigurableACLMatchCounter);
DEFINE_SWITCH_DUMP_FUNC(OutputMirroringTable);
DEFINE_SWITCH_DUMP_FUNC(PortUsed);
DEFINE_SWITCH_DUMP_FUNC(SecondTunnelExitMissAction);
DEFINE_SWITCH_DUMP_FUNC(IngressConfigurableACL0SmallTable);
DEFINE_SWITCH_DUMP_FUNC(PortTCReserved);
DEFINE_SWITCH_DUMP_FUNC(L2QoSMappingTable);
DEFINE_SWITCH_DUMP_FUNC(EgressTunnelExitTable);
DEFINE_SWITCH_DUMP_FUNC(MACInterfaceCountersForRX);
DEFINE_SWITCH_DUMP_FUNC(NextHopTable);
DEFINE_SWITCH_DUMP_FUNC(EgressACLRulePointerTCAMAnswer);
DEFINE_SWITCH_DUMP_FUNC(SourcePortDefaultACLAction);
DEFINE_SWITCH_DUMP_FUNC(VLANTable);
DEFINE_SWITCH_DUMP_FUNC(IngressPortPacketTypeFilter);
DEFINE_SWITCH_DUMP_FUNC(PortFFAUsed);
DEFINE_SWITCH_DUMP_FUNC(DefaultLearningStatus);
DEFINE_SWITCH_DUMP_FUNC(IngressVIDEthernetTypeRangeAssignmentAnswer);
DEFINE_SWITCH_DUMP_FUNC(L2MulticastStormControlBucketThresholdConfiguration);
DEFINE_SWITCH_DUMP_FUNC(IngressConfigurableACL3TCAM);
DEFINE_SWITCH_DUMP_FUNC(L2BroadcastStormControlBucketCapacityConfiguration);
DEFINE_SWITCH_DUMP_FUNC(NATActionTable);
DEFINE_SWITCH_DUMP_FUNC(IngressConfigurableACL1RulesSetup);
DEFINE_SWITCH_DUMP_FUNC(VLANPCPAndDEIToColorMappingTable);
DEFINE_SWITCH_DUMP_FUNC(IngressEgressPortPacketTypeFilter);
DEFINE_SWITCH_DUMP_FUNC(ColorRemapFromEgressPort);
DEFINE_SWITCH_DUMP_FUNC(IngressNATOperation);
DEFINE_SWITCH_DUMP_FUNC(IPQoSMappingTable);
DEFINE_SWITCH_DUMP_FUNC(IngressAdmissionControlTokenBucketConfiguration);
DEFINE_SWITCH_DUMP_FUNC(L2BroadcastStormControlRateConfiguration);
DEFINE_SWITCH_DUMP_FUNC(PortTCXoffTotalThreshold);
DEFINE_SWITCH_DUMP_FUNC(L2FloodingStormControlBucketThresholdConfiguration);
DEFINE_SWITCH_DUMP_FUNC(CPUReasonCodeOperation);
DEFINE_SWITCH_DUMP_FUNC(HashBasedL3RoutingTable);
DEFINE_SWITCH_DUMP_FUNC(PortShaperBucketCapacityConfiguration);
DEFINE_SWITCH_DUMP_FUNC(L3RoutingTCAM);
DEFINE_SWITCH_DUMP_FUNC(PortXonFFAThreshold);
DEFINE_SWITCH_DUMP_FUNC(L3LPMResult);
DEFINE_SWITCH_DUMP_FUNC(IngressConfigurableACL1TCAM);
DEFINE_SWITCH_DUMP_FUNC(PSErrorCounter);
DEFINE_SWITCH_DUMP_FUNC(IngressConfigurableACL0PreLookup);
DEFINE_SWITCH_DUMP_FUNC(DWRRBucketCapacityConfiguration);
DEFINE_SWITCH_DUMP_FUNC(PortPauseSettings);

#define DEFINE_SWITCH_DFX_ARRAY(REGNAME) { #REGNAME, cls_switch_dump_##REGNAME }

struct cls_switch_dump_func {
	char *name;
	void (*func)(int idx);
};

struct cls_switch_dump_func switch_dfx_array[] = {
	DEFINE_SWITCH_DFX_ARRAY(DrainPortDrop),
	DEFINE_SWITCH_DFX_ARRAY(IngressMultipleSpanningTreeState),
	DEFINE_SWITCH_DFX_ARRAY(SourcePortTable),
	DEFINE_SWITCH_DFX_ARRAY(L2ActionTablePerPortDrop),
	DEFINE_SWITCH_DFX_ARRAY(RouterMTUTable),
	DEFINE_SWITCH_DFX_ARRAY(DebugEPPCounter),
	DEFINE_SWITCH_DFX_ARRAY(EgressConfigurableACL1TCAMAnswer),
	DEFINE_SWITCH_DFX_ARRAY(IngressEgressPacketFilteringDrop),
	DEFINE_SWITCH_DFX_ARRAY(IngressVIDMACRangeAssignmentAnswer),
	DEFINE_SWITCH_DFX_ARRAY(NextHopMPLSTable),
	DEFINE_SWITCH_DFX_ARRAY(EgressPortDisabledDrop),
	DEFINE_SWITCH_DFX_ARRAY(IPv4TOSFieldToPacketColorMappingTable),
	DEFINE_SWITCH_DFX_ARRAY(EnableEnqueueToPortsAndQueues),
	DEFINE_SWITCH_DFX_ARRAY(MPLSEXPFieldToEgressQueueMappingTable),
	DEFINE_SWITCH_DFX_ARRAY(PortReserved),
	DEFINE_SWITCH_DFX_ARRAY(ResourceLimiterSet),
	DEFINE_SWITCH_DFX_ARRAY(EgressConfigurableACL0TCAMAnswer),
	DEFINE_SWITCH_DFX_ARRAY(EgressResourceManagerPointer),
	DEFINE_SWITCH_DFX_ARRAY(IPUnicastReceivedCounter),
	DEFINE_SWITCH_DFX_ARRAY(EgressConfigurableACL0RulesSetup),
	DEFINE_SWITCH_DFX_ARRAY(SMONSetSearch),
	DEFINE_SWITCH_DFX_ARRAY(L2MulticastStormControlRateConfiguration),
	DEFINE_SWITCH_DFX_ARRAY(SPOverflowDrop),
	DEFINE_SWITCH_DFX_ARRAY(PortShaperBucketThresholdConfiguration),
	DEFINE_SWITCH_DFX_ARRAY(AllowSpecialFrameCheckForL2ActionTable),
	DEFINE_SWITCH_DFX_ARRAY(PortTailDropSettings),
	DEFINE_SWITCH_DFX_ARRAY(PortShaperRateConfiguration),
	DEFINE_SWITCH_DFX_ARRAY(DWRRBucketMiscConfiguration),
	DEFINE_SWITCH_DFX_ARRAY(EgressConfigurableACL1RulesSetup),
	DEFINE_SWITCH_DFX_ARRAY(EgressConfigurableACL0LargeTable),
	DEFINE_SWITCH_DFX_ARRAY(RouterPortEgressSAMACAddress),
	DEFINE_SWITCH_DFX_ARRAY(OutputDisable),
	DEFINE_SWITCH_DFX_ARRAY(L2AgingCollisionTable),
	DEFINE_SWITCH_DFX_ARRAY(IngressAdmissionControlInitialPointer),
	DEFINE_SWITCH_DFX_ARRAY(L2TunnelEntryInstructionTable),
	DEFINE_SWITCH_DFX_ARRAY(IngressConfigurableACL0LargeTable),
	DEFINE_SWITCH_DFX_ARRAY(EgressPortDepth),
	DEFINE_SWITCH_DFX_ARRAY(EgressConfigurableACL0SmallTable),
	DEFINE_SWITCH_DFX_ARRAY(SelectWhichEgressQoSMappingTableToUse),
	DEFINE_SWITCH_DFX_ARRAY(DWRRWeightConfiguration),
	DEFINE_SWITCH_DFX_ARRAY(LinkAggregateWeight),
	DEFINE_SWITCH_DFX_ARRAY(L3RoutingDefault),
	DEFINE_SWITCH_DFX_ARRAY(TransmittedPacketsonEgressVRF),
	DEFINE_SWITCH_DFX_ARRAY(IngressAdmissionControlReset),
	DEFINE_SWITCH_DFX_ARRAY(PrioShaperBucketCapacityConfiguration),
	DEFINE_SWITCH_DFX_ARRAY(EgressVLANTranslationTCAM),
	DEFINE_SWITCH_DFX_ARRAY(L2LookupCollisionTable),
	DEFINE_SWITCH_DFX_ARRAY(NextHopPacketModifications),
	DEFINE_SWITCH_DFX_ARRAY(IPv4TOSFieldToEgressQueueMappingTable),
	DEFINE_SWITCH_DFX_ARRAY(IngressVIDInnerVIDRangeAssignmentAnswer),
	DEFINE_SWITCH_DFX_ARRAY(SecondTunnelExitLookupTCAMAnswer),
	DEFINE_SWITCH_DFX_ARRAY(EgressPortConfiguration),
	DEFINE_SWITCH_DFX_ARRAY(EgressSpanningTreeDrop),
	DEFINE_SWITCH_DFX_ARRAY(IngressConfigurableACL0TCAM),
	DEFINE_SWITCH_DFX_ARRAY(L2AgingStatusShadowTable),
	DEFINE_SWITCH_DFX_ARRAY(TCXoffFFAThreshold),
	DEFINE_SWITCH_DFX_ARRAY(EgressConfigurableACL1TCAM),
	DEFINE_SWITCH_DFX_ARRAY(L2AgingTable),
	DEFINE_SWITCH_DFX_ARRAY(IngressRouterTable),
	DEFINE_SWITCH_DFX_ARRAY(NextHopPacketInsertMPLSHeader),
	DEFINE_SWITCH_DFX_ARRAY(MACInterfaceCountersForTX),
	DEFINE_SWITCH_DFX_ARRAY(TunnelEntryInstructionTable),
	DEFINE_SWITCH_DFX_ARRAY(IPUnicastRoutedCounter),
	DEFINE_SWITCH_DFX_ARRAY(IngressAdmissionControlCurrentSize),
	DEFINE_SWITCH_DFX_ARRAY(L2DAHashLookupTable),
	DEFINE_SWITCH_DFX_ARRAY(IPMulticastReceivedCounter),
	DEFINE_SWITCH_DFX_ARRAY(TCFFAUsed),
	DEFINE_SWITCH_DFX_ARRAY(EgressNATOperation),
	DEFINE_SWITCH_DFX_ARRAY(IngressConfigurableACL1TCAMAnswer),
	DEFINE_SWITCH_DFX_ARRAY(IngressConfigurableACL3TCAMAnswer),
	DEFINE_SWITCH_DFX_ARRAY(TunnelExitLookupTCAMAnswer),
	DEFINE_SWITCH_DFX_ARRAY(ReservedSourceMACAddressRange),
	DEFINE_SWITCH_DFX_ARRAY(QueueOffDrop),
	DEFINE_SWITCH_DFX_ARRAY(TunnelEntryHeaderData),
	DEFINE_SWITCH_DFX_ARRAY(QueueShaperBucketThresholdConfiguration),
	DEFINE_SWITCH_DFX_ARRAY(IngressConfigurableACL0TCAMAnswer),
	DEFINE_SWITCH_DFX_ARRAY(EgressConfigurableACLMatchCounter),
	DEFINE_SWITCH_DFX_ARRAY(IngressVIDOuterVIDRangeSearchData),
	DEFINE_SWITCH_DFX_ARRAY(EgressVLANTranslationTCAMAnswer),
	DEFINE_SWITCH_DFX_ARRAY(L2ActionTableSourcePort),
	DEFINE_SWITCH_DFX_ARRAY(FloodingActionSendtoPort),
	DEFINE_SWITCH_DFX_ARRAY(EgressMPLSTTLTable),
	DEFINE_SWITCH_DFX_ARRAY(MBSCDrop),
	DEFINE_SWITCH_DFX_ARRAY(EgressResourceManagerDrop),
	DEFINE_SWITCH_DFX_ARRAY(MACRXBrokenPackets),
	DEFINE_SWITCH_DFX_ARRAY(IngressVIDOuterVIDRangeAssignmentAnswer),
	DEFINE_SWITCH_DFX_ARRAY(IngressVIDMACRangeSearchData),
	DEFINE_SWITCH_DFX_ARRAY(IngressNATHitStatus),
	DEFINE_SWITCH_DFX_ARRAY(ColorRemapFromIngressAdmissionControl),
	DEFINE_SWITCH_DFX_ARRAY(HairpinEnable),
	DEFINE_SWITCH_DFX_ARRAY(MPLSEXPFieldToPacketColorMappingTable),
	DEFINE_SWITCH_DFX_ARRAY(IPMulticastACLDropCounter),
	DEFINE_SWITCH_DFX_ARRAY(QueueShaperBucketCapacityConfiguration),
	DEFINE_SWITCH_DFX_ARRAY(IngressVIDInnerVIDRangeSearchData),
	DEFINE_SWITCH_DFX_ARRAY(NextHopHitStatus),
	DEFINE_SWITCH_DFX_ARRAY(EgressQueueToMPLSEXPMappingTable),
	DEFINE_SWITCH_DFX_ARRAY(EgressRouterTable),
	DEFINE_SWITCH_DFX_ARRAY(MACRXLongPacketDrop),
	DEFINE_SWITCH_DFX_ARRAY(PortXoffFFAThreshold),
	DEFINE_SWITCH_DFX_ARRAY(TCXonFFAThreshold),
	DEFINE_SWITCH_DFX_ARRAY(LinkAggregationToPhysicalPortsMembers),
	DEFINE_SWITCH_DFX_ARRAY(L3TunnelEntryInstructionTable),
	DEFINE_SWITCH_DFX_ARRAY(SecondTunnelExitLookupTCAM),
	DEFINE_SWITCH_DFX_ARRAY(MapQueuetoPriority),
	DEFINE_SWITCH_DFX_ARRAY(NextHopDAMAC),
	DEFINE_SWITCH_DFX_ARRAY(ReceivedPacketsonIngressVRF),
	DEFINE_SWITCH_DFX_ARRAY(IngressConfigurableACL1PreLookup),
	DEFINE_SWITCH_DFX_ARRAY(MACRXMaximumPacketLength),
	DEFINE_SWITCH_DFX_ARRAY(EgressPortFilteringDrop),
	DEFINE_SWITCH_DFX_ARRAY(IngressConfigurableACL0RulesSetup),
	DEFINE_SWITCH_DFX_ARRAY(MACRXShortPacketDrop),
	DEFINE_SWITCH_DFX_ARRAY(IngressConfigurableACL2PreLookup),
	DEFINE_SWITCH_DFX_ARRAY(PortTCTailDropTotalThreshold),
	DEFINE_SWITCH_DFX_ARRAY(VLANPCPToQueueMappingTable),
	DEFINE_SWITCH_DFX_ARRAY(L2MulticastStormControlBucketCapacityConfiguration),
	DEFINE_SWITCH_DFX_ARRAY(L2MulticastTable),
	DEFINE_SWITCH_DFX_ARRAY(ReservedDestinationMACAddressRange),
	DEFINE_SWITCH_DFX_ARRAY(TCTailDropFFAThreshold),
	DEFINE_SWITCH_DFX_ARRAY(IngressConfigurableACL1SmallTable),
	DEFINE_SWITCH_DFX_ARRAY(IngressVIDEthernetTypeRangeSearchData),
	DEFINE_SWITCH_DFX_ARRAY(L2FloodingStormControlRateConfiguration),
	DEFINE_SWITCH_DFX_ARRAY(IngressConfigurableACL1LargeTable),
	DEFINE_SWITCH_DFX_ARRAY(PrioShaperRateConfiguration),
	DEFINE_SWITCH_DFX_ARRAY(IPMulticastRoutedCounter),
	DEFINE_SWITCH_DFX_ARRAY(PFCIncCountersforingressports0to5),
	DEFINE_SWITCH_DFX_ARRAY(L2FloodingStormControlBucketCapacityConfiguration),
	DEFINE_SWITCH_DFX_ARRAY(IPv6ClassofServiceFieldToPacketColorMappingTable),
	DEFINE_SWITCH_DFX_ARRAY(L2DestinationTable),
	DEFINE_SWITCH_DFX_ARRAY(EgressMultipleSpanningTreeState),
	DEFINE_SWITCH_DFX_ARRAY(PortTCXonTotalThreshold),
	DEFINE_SWITCH_DFX_ARRAY(HardwareLearningCounter),
	DEFINE_SWITCH_DFX_ARRAY(L2ReservedMulticastAddressAction),
	DEFINE_SWITCH_DFX_ARRAY(L2BroadcastStormControlBucketThresholdConfiguration),
	DEFINE_SWITCH_DFX_ARRAY(BeginningofPacketTunnelEntryInstructionTable),
	DEFINE_SWITCH_DFX_ARRAY(TOSQoSMappingTable),
	DEFINE_SWITCH_DFX_ARRAY(L2LookupCollisionTableMasks),
	DEFINE_SWITCH_DFX_ARRAY(IngressConfigurableACL2RulesSetup),
	DEFINE_SWITCH_DFX_ARRAY(UnknownEgressDrop),
	DEFINE_SWITCH_DFX_ARRAY(EgressNATHitStatus),
	DEFINE_SWITCH_DFX_ARRAY(PFCDecCountersforingressports0to5),
	DEFINE_SWITCH_DFX_ARRAY(DebugIPPCounter),
	DEFINE_SWITCH_DFX_ARRAY(PrioShaperBucketThresholdConfiguration),
	DEFINE_SWITCH_DFX_ARRAY(PortTailDropFFAThreshold),
	DEFINE_SWITCH_DFX_ARRAY(MPLSQoSMappingTable),
	DEFINE_SWITCH_DFX_ARRAY(TunnelExitTooSmallPacketModificationToSmallDrop),
	DEFINE_SWITCH_DFX_ARRAY(IPv6ClassofServiceFieldToEgressQueueMappingTable),
	DEFINE_SWITCH_DFX_ARRAY(QueueShaperRateConfiguration),
	DEFINE_SWITCH_DFX_ARRAY(EgressQueueDepth),
	DEFINE_SWITCH_DFX_ARRAY(IngressConfigurableACL2TCAMAnswer),
	DEFINE_SWITCH_DFX_ARRAY(DefaultPacketToCPUModification),
	DEFINE_SWITCH_DFX_ARRAY(RouterPortMACAddress),
	DEFINE_SWITCH_DFX_ARRAY(EgressConfigurableACL0TCAM),
	DEFINE_SWITCH_DFX_ARRAY(IngressConfigurableACL2TCAM),
	DEFINE_SWITCH_DFX_ARRAY(EgressACLRulePointerTCAM),
	DEFINE_SWITCH_DFX_ARRAY(EgressQueueToPCPAndCFIDEIMappingTable),
	DEFINE_SWITCH_DFX_ARRAY(L2AgingCollisionShadowTable),
	DEFINE_SWITCH_DFX_ARRAY(RouterEgressQueueToVLANData),
	DEFINE_SWITCH_DFX_ARRAY(LinkAggregationMembership),
	DEFINE_SWITCH_DFX_ARRAY(IngressConfigurableACL3RulesSetup),
	DEFINE_SWITCH_DFX_ARRAY(L2ActionTable),
	DEFINE_SWITCH_DFX_ARRAY(TunnelExitLookupTCAM),
	DEFINE_SWITCH_DFX_ARRAY(IngressConfigurableACLMatchCounter),
	DEFINE_SWITCH_DFX_ARRAY(OutputMirroringTable),
	DEFINE_SWITCH_DFX_ARRAY(PortUsed),
	DEFINE_SWITCH_DFX_ARRAY(SecondTunnelExitMissAction),
	DEFINE_SWITCH_DFX_ARRAY(IngressConfigurableACL0SmallTable),
	DEFINE_SWITCH_DFX_ARRAY(PortTCReserved),
	DEFINE_SWITCH_DFX_ARRAY(L2QoSMappingTable),
	DEFINE_SWITCH_DFX_ARRAY(EgressTunnelExitTable),
	DEFINE_SWITCH_DFX_ARRAY(MACInterfaceCountersForRX),
	DEFINE_SWITCH_DFX_ARRAY(NextHopTable),
	DEFINE_SWITCH_DFX_ARRAY(EgressACLRulePointerTCAMAnswer),
	DEFINE_SWITCH_DFX_ARRAY(SourcePortDefaultACLAction),
	DEFINE_SWITCH_DFX_ARRAY(VLANTable),
	DEFINE_SWITCH_DFX_ARRAY(IngressPortPacketTypeFilter),
	DEFINE_SWITCH_DFX_ARRAY(PortFFAUsed),
	DEFINE_SWITCH_DFX_ARRAY(DefaultLearningStatus),
	DEFINE_SWITCH_DFX_ARRAY(IngressVIDEthernetTypeRangeAssignmentAnswer),
	DEFINE_SWITCH_DFX_ARRAY(L2MulticastStormControlBucketThresholdConfiguration),
	DEFINE_SWITCH_DFX_ARRAY(IngressConfigurableACL3TCAM),
	DEFINE_SWITCH_DFX_ARRAY(L2BroadcastStormControlBucketCapacityConfiguration),
	DEFINE_SWITCH_DFX_ARRAY(NATActionTable),
	DEFINE_SWITCH_DFX_ARRAY(IngressConfigurableACL1RulesSetup),
	DEFINE_SWITCH_DFX_ARRAY(VLANPCPAndDEIToColorMappingTable),
	DEFINE_SWITCH_DFX_ARRAY(IngressEgressPortPacketTypeFilter),
	DEFINE_SWITCH_DFX_ARRAY(ColorRemapFromEgressPort),
	DEFINE_SWITCH_DFX_ARRAY(IngressNATOperation),
	DEFINE_SWITCH_DFX_ARRAY(IPQoSMappingTable),
	DEFINE_SWITCH_DFX_ARRAY(IngressAdmissionControlTokenBucketConfiguration),
	DEFINE_SWITCH_DFX_ARRAY(L2BroadcastStormControlRateConfiguration),
	DEFINE_SWITCH_DFX_ARRAY(PortTCXoffTotalThreshold),
	DEFINE_SWITCH_DFX_ARRAY(L2FloodingStormControlBucketThresholdConfiguration),
	DEFINE_SWITCH_DFX_ARRAY(CPUReasonCodeOperation),
	DEFINE_SWITCH_DFX_ARRAY(HashBasedL3RoutingTable),
	DEFINE_SWITCH_DFX_ARRAY(PortShaperBucketCapacityConfiguration),
	DEFINE_SWITCH_DFX_ARRAY(L3RoutingTCAM),
	DEFINE_SWITCH_DFX_ARRAY(PortXonFFAThreshold),
	DEFINE_SWITCH_DFX_ARRAY(L3LPMResult),
	DEFINE_SWITCH_DFX_ARRAY(IngressConfigurableACL1TCAM),
	DEFINE_SWITCH_DFX_ARRAY(PSErrorCounter),
	DEFINE_SWITCH_DFX_ARRAY(IngressConfigurableACL0PreLookup),
	DEFINE_SWITCH_DFX_ARRAY(DWRRBucketCapacityConfiguration),
	DEFINE_SWITCH_DFX_ARRAY(PortPauseSettings),
};

void dubhe2000_switch_dump_dfx(struct dubhe1000_adapter *adapter, char *cmd)
{
	int i, idx = -1;
	char *space = strchr(cmd, ' ');

	if (space) {
		*space = 0;

		if (sscanf(space + 1, "%i", &idx) != 1)
			idx = -1;
	}

	for (i = 0; i < ARRAY_SIZE(switch_dfx_array); i++) {
		if (!strncasecmp(switch_dfx_array[i].name, cmd, strlen(switch_dfx_array[i].name) + 1)) {
			switch_dfx_array[i].func(idx);
			return;
		}
	}

	pr_info("[%s] invalid cmd=%s\n", __func__, cmd);
}
