// Generated code for flexswitch-interface
#ifndef FLEXSWITCH_H
#define FLEXSWITCH_H

#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include "dubhe2000.h"

static const int FLEXSW_ERR = -1;
static const int FLEXSW_OK = 0;

//---- constants for register DrainPortDrop -----
#define DrainPortDrop_nr_entries (6)
// bit width of each register field:
#define DrainPortDrop_packets_width (32)

//---- constants for register IngressMultipleSpanningTreeState -----
#define IngressMultipleSpanningTreeState_nr_entries (16)
// bit width of each register field:
#define IngressMultipleSpanningTreeState_portSptState_width (12)

//---- constants for register SourcePortTable -----
#define SourcePortTable_nr_entries (6)
// bit width of each register field:
#define SourcePortTable_aclRule2_width (3)
#define SourcePortTable_ignoreVlanMembership_width (1)
#define SourcePortTable_aclRule0_width (3)
#define SourcePortTable_natPortState_width (1)
#define SourcePortTable_enableDefaultPortAcl_width (1)
#define SourcePortTable_imUnderVlanMembership_width (1)
#define SourcePortTable_typeSel_width (2)
#define SourcePortTable_spt_width (3)
#define SourcePortTable_defaultPcpIf_width (3)
#define SourcePortTable_dropUnknownDa_width (1)
#define SourcePortTable_aclRule1_width (3)
#define SourcePortTable_preLookupAclBits_width (2)
#define SourcePortTable_priorityVid_width (12)
#define SourcePortTable_enableFromCpuTag_width (1)
#define SourcePortTable_natActionTableEnable_width (1)
#define SourcePortTable_enablePriorityTag_width (1)
#define SourcePortTable_defaultCfiDeiIf_width (1)
#define SourcePortTable_enableL2ActionTable_width (1)
#define SourcePortTable_defaultVid_width (12)
#define SourcePortTable_destInputMirror_width (3)
#define SourcePortTable_learnMulticastSaMac_width (1)
#define SourcePortTable_defaultVidIf_width (12)
#define SourcePortTable_vlanAssignment_width (2)
#define SourcePortTable_vidSel_width (2)
#define SourcePortTable_aclRule3_width (3)
#define SourcePortTable_useAcl1_width (1)
#define SourcePortTable_pcpSel_width (2)
#define SourcePortTable_defaultCfiDei_width (1)
#define SourcePortTable_nrVlansVidOperationIf_width (2)
#define SourcePortTable_defaultVidOrder_width (2)
#define SourcePortTable_typeSelIf_width (2)
#define SourcePortTable_minAllowedVlans_width (2)
#define SourcePortTable_inputMirrorEnabled_width (1)
#define SourcePortTable_forcePortAclAction_width (1)
#define SourcePortTable_disableTunnelExit_width (1)
#define SourcePortTable_cfiDeiSelIf_width (2)
#define SourcePortTable_colorFromL3_width (1)
#define SourcePortTable_pcpSelIf_width (2)
#define SourcePortTable_disableRouting_width (1)
#define SourcePortTable_useAcl2_width (1)
#define SourcePortTable_useAcl3_width (1)
#define SourcePortTable_useAcl0_width (1)
#define SourcePortTable_cfiDeiSel_width (2)
#define SourcePortTable_prioFromL3_width (1)
#define SourcePortTable_defaultPcp_width (3)
#define SourcePortTable_l2ActionTablePortState_width (1)
#define SourcePortTable_maxAllowedVlans_width (2)
#define SourcePortTable_vlanSingleOpIf_width (3)
#define SourcePortTable_firstHitSecondMissSendToCpu_width (1)
#define SourcePortTable_vidSelIf_width (2)
#define SourcePortTable_imUnderPortIsolation_width (1)
#define SourcePortTable_vlanSingleOp_width (3)
#define SourcePortTable_learningEn_width (1)

//---- constants for register L2ActionTablePerPortDrop -----
#define L2ActionTablePerPortDrop_nr_entries (6)
// bit width of each register field:
#define L2ActionTablePerPortDrop_packets_width (32)

//---- constants for register RouterMTUTable -----
#define RouterMTUTable_nr_entries (24)
// bit width of each register field:
#define RouterMTUTable_maxIPv6MTU_width (16)
#define RouterMTUTable_maxIPv4MTU_width (16)

//---- constants for register DebugEPPCounter -----
#define DebugEPPCounter_nr_entries (16)
// bit width of each register field:
#define DebugEPPCounter_packets_width (16)

//---- constants for register EgressConfigurableACL1TCAMAnswer -----
#define EgressConfigurableACL1TCAMAnswer_nr_entries (16)
// bit width of each register field:
#define EgressConfigurableACL1TCAMAnswer_metaDataValid_width (1)
#define EgressConfigurableACL1TCAMAnswer_destPort_width (3)
#define EgressConfigurableACL1TCAMAnswer_dropEnable_width (1)
#define EgressConfigurableACL1TCAMAnswer_metaDataPrio_width (1)
#define EgressConfigurableACL1TCAMAnswer_counter_width (6)
#define EgressConfigurableACL1TCAMAnswer_tunnelEntryPrio_width (1)
#define EgressConfigurableACL1TCAMAnswer_tunnelEntryUcMc_width (1)
#define EgressConfigurableACL1TCAMAnswer_updateCounter_width (1)
#define EgressConfigurableACL1TCAMAnswer_tunnelEntryPtr_width (4)
#define EgressConfigurableACL1TCAMAnswer_forceSendToCpuOrigPkt_width (1)
#define EgressConfigurableACL1TCAMAnswer_sendToCpu_width (1)
#define EgressConfigurableACL1TCAMAnswer_tunnelEntry_width (1)
#define EgressConfigurableACL1TCAMAnswer_sendToPort_width (1)
#define EgressConfigurableACL1TCAMAnswer_metaData_width (16)

//---- constants for register IngressEgressPacketFilteringDrop -----
#define IngressEgressPacketFilteringDrop_nr_entries (6)
// bit width of each register field:
#define IngressEgressPacketFilteringDrop_packets_width (32)

//---- constants for register IngressVIDMACRangeAssignmentAnswer -----
#define IngressVIDMACRangeAssignmentAnswer_nr_entries (4)
// bit width of each register field:
#define IngressVIDMACRangeAssignmentAnswer_ingressVid_width (12)
#define IngressVIDMACRangeAssignmentAnswer_order_width (2)

//---- constants for register NextHopMPLSTable -----
#define NextHopMPLSTable_nr_entries (1024)
// bit width of each register field:
#define NextHopMPLSTable_expSel_width (2)
#define NextHopMPLSTable_mplsOperation_width (3)
#define NextHopMPLSTable_exp_width (3)
#define NextHopMPLSTable_label_width (20)

//---- constants for register EgressPortDisabledDrop -----
#define EgressPortDisabledDrop_nr_entries (6)
// bit width of each register field:
#define EgressPortDisabledDrop_packets_width (32)

//---- constants for register IPv4TOSFieldToPacketColorMappingTable -----
#define IPv4TOSFieldToPacketColorMappingTable_nr_entries (256)
// bit width of each register field:
#define IPv4TOSFieldToPacketColorMappingTable_color_width (2)

//---- constants for register EnableEnqueueToPortsAndQueues -----
#define EnableEnqueueToPortsAndQueues_nr_entries (6)
// bit width of each register field:
#define EnableEnqueueToPortsAndQueues_qon_width (8)

//---- constants for register MPLSEXPFieldToEgressQueueMappingTable -----
#define MPLSEXPFieldToEgressQueueMappingTable_nr_entries (8)
// bit width of each register field:
#define MPLSEXPFieldToEgressQueueMappingTable_pQueue_width (3)

//---- constants for register PortReserved -----
#define PortReserved_nr_entries (6)
// bit width of each register field:
#define PortReserved_cells_width (10)

//---- constants for register ResourceLimiterSet -----
#define ResourceLimiterSet_nr_entries (4)
// bit width of each register field:
#define ResourceLimiterSet_yellowLimit_width (10)
#define ResourceLimiterSet_yellowAccumulated_width (10)
#define ResourceLimiterSet_maxCells_width (8)
#define ResourceLimiterSet_redLimit_width (10)

//---- constants for register EgressConfigurableACL0TCAMAnswer -----
#define EgressConfigurableACL0TCAMAnswer_nr_entries (16)
// bit width of each register field:
#define EgressConfigurableACL0TCAMAnswer_counter_width (6)
#define EgressConfigurableACL0TCAMAnswer_sendToCpu_width (1)
#define EgressConfigurableACL0TCAMAnswer_metaDataValid_width (1)
#define EgressConfigurableACL0TCAMAnswer_destPort_width (3)
#define EgressConfigurableACL0TCAMAnswer_dropEnable_width (1)
#define EgressConfigurableACL0TCAMAnswer_metaDataPrio_width (1)
#define EgressConfigurableACL0TCAMAnswer_natOpPrio_width (1)
#define EgressConfigurableACL0TCAMAnswer_tunnelEntryPrio_width (1)
#define EgressConfigurableACL0TCAMAnswer_tunnelEntryUcMc_width (1)
#define EgressConfigurableACL0TCAMAnswer_natOpPtr_width (10)
#define EgressConfigurableACL0TCAMAnswer_updateCounter_width (1)
#define EgressConfigurableACL0TCAMAnswer_tunnelEntryPtr_width (4)
#define EgressConfigurableACL0TCAMAnswer_forceSendToCpuOrigPkt_width (1)
#define EgressConfigurableACL0TCAMAnswer_natOpValid_width (1)
#define EgressConfigurableACL0TCAMAnswer_tunnelEntry_width (1)
#define EgressConfigurableACL0TCAMAnswer_sendToPort_width (1)
#define EgressConfigurableACL0TCAMAnswer_metaData_width (16)

//---- constants for register EgressResourceManagerPointer -----
#define EgressResourceManagerPointer_nr_entries (6)
// bit width of each register field:
#define EgressResourceManagerPointer_q1_width (2)
#define EgressResourceManagerPointer_q0_width (2)
#define EgressResourceManagerPointer_q3_width (2)
#define EgressResourceManagerPointer_q2_width (2)
#define EgressResourceManagerPointer_q5_width (2)
#define EgressResourceManagerPointer_q4_width (2)
#define EgressResourceManagerPointer_q7_width (2)
#define EgressResourceManagerPointer_q6_width (2)

//---- constants for register IPUnicastReceivedCounter -----
#define IPUnicastReceivedCounter_nr_entries (6)
// bit width of each register field:
#define IPUnicastReceivedCounter_packets_width (32)

//---- constants for register EgressConfigurableACL0RulesSetup -----
#define EgressConfigurableACL0RulesSetup_nr_entries (8)
// bit width of each register field:
#define EgressConfigurableACL0RulesSetup_fieldSelectBitmask_width (18)

//---- constants for register SMONSetSearch -----
#define SMONSetSearch_nr_entries (4)
// bit width of each register field:
#define SMONSetSearch_srcPort_width (3)
#define SMONSetSearch_vid_width (12)

//---- constants for register L2MulticastStormControlRateConfiguration -----
#define L2MulticastStormControlRateConfiguration_nr_entries (6)
// bit width of each register field:
#define L2MulticastStormControlRateConfiguration_tokens_width (12)
#define L2MulticastStormControlRateConfiguration_packetsNotBytes_width (1)
#define L2MulticastStormControlRateConfiguration_tick_width (3)
#define L2MulticastStormControlRateConfiguration_ifgCorrection_width (8)

//---- constants for register SPOverflowDrop -----
#define SPOverflowDrop_nr_entries (6)
// bit width of each register field:
#define SPOverflowDrop_packets_width (32)

//---- constants for register PortShaperBucketThresholdConfiguration -----
#define PortShaperBucketThresholdConfiguration_nr_entries (6)
// bit width of each register field:
#define PortShaperBucketThresholdConfiguration_threshold_width (17)

//---- constants for register AllowSpecialFrameCheckForL2ActionTable -----
#define AllowSpecialFrameCheckForL2ActionTable_nr_entries (4)
// bit width of each register field:
#define AllowSpecialFrameCheckForL2ActionTable_dontAllowIGMP_width (1)
#define AllowSpecialFrameCheckForL2ActionTable_dontAllowBPDU_width (1)
#define AllowSpecialFrameCheckForL2ActionTable_dontAllowAH_width (1)
#define AllowSpecialFrameCheckForL2ActionTable_dontAllowCAPWAP_width (1)
#define AllowSpecialFrameCheckForL2ActionTable_dontAllowGRE_width (1)
#define AllowSpecialFrameCheckForL2ActionTable_dontAllowMPLS_width (1)
#define AllowSpecialFrameCheckForL2ActionTable_dontAllowARP_width (1)
#define AllowSpecialFrameCheckForL2ActionTable_dontAllowRARP_width (1)
#define AllowSpecialFrameCheckForL2ActionTable_dontAllowDNS_width (1)
#define AllowSpecialFrameCheckForL2ActionTable_dontAllowBOOTPDHCP_width (1)
#define AllowSpecialFrameCheckForL2ActionTable_dontAllowIPV4_width (1)
#define AllowSpecialFrameCheckForL2ActionTable_dontAllowIPV6_width (1)
#define AllowSpecialFrameCheckForL2ActionTable_dontAllowL21588_width (1)
#define AllowSpecialFrameCheckForL2ActionTable_dontAllowL41588_width (1)
#define AllowSpecialFrameCheckForL2ActionTable_dontAllowESP_width (1)
#define AllowSpecialFrameCheckForL2ActionTable_dontAllow8021XEAPOL_width (1)
#define AllowSpecialFrameCheckForL2ActionTable_dontAllowLLDP_width (1)
#define AllowSpecialFrameCheckForL2ActionTable_dontAllowICMP_width (1)
#define AllowSpecialFrameCheckForL2ActionTable_dontAllowL2McReserved_width (1)
#define AllowSpecialFrameCheckForL2ActionTable_dontAllowTCP_width (1)
#define AllowSpecialFrameCheckForL2ActionTable_dontAllowUDP_width (1)
#define AllowSpecialFrameCheckForL2ActionTable_dontAllowSCTP_width (1)

//---- constants for register PortTailDropSettings -----
#define PortTailDropSettings_nr_entries (6)
// bit width of each register field:
#define PortTailDropSettings_enable_width (1)
#define PortTailDropSettings_mode_width (1)

//---- constants for register PortShaperRateConfiguration -----
#define PortShaperRateConfiguration_nr_entries (6)
// bit width of each register field:
#define PortShaperRateConfiguration_tokens_width (13)
#define PortShaperRateConfiguration_packetsNotBytes_width (1)
#define PortShaperRateConfiguration_tick_width (3)
#define PortShaperRateConfiguration_ifgCorrection_width (8)

//---- constants for register DWRRBucketMiscConfiguration -----
#define DWRRBucketMiscConfiguration_nr_entries (6)
// bit width of each register field:
#define DWRRBucketMiscConfiguration_packetsNotBytes_width (1)
#define DWRRBucketMiscConfiguration_threshold_width (5)
#define DWRRBucketMiscConfiguration_ifgCorrection_width (8)

//---- constants for register EgressConfigurableACL1RulesSetup -----
#define EgressConfigurableACL1RulesSetup_nr_entries (4)
// bit width of each register field:
#define EgressConfigurableACL1RulesSetup_fieldSelectBitmask_width (20)

//---- constants for register EgressConfigurableACL0LargeTable -----
#define EgressConfigurableACL0LargeTable_nr_entries (1024)
// bit width of each register field:
#define EgressConfigurableACL0LargeTable_compareData_width (135)
#define EgressConfigurableACL0LargeTable_counter_width (6)
#define EgressConfigurableACL0LargeTable_sendToCpu_width (1)
#define EgressConfigurableACL0LargeTable_metaDataValid_width (1)
#define EgressConfigurableACL0LargeTable_destPort_width (3)
#define EgressConfigurableACL0LargeTable_dropEnable_width (1)
#define EgressConfigurableACL0LargeTable_metaDataPrio_width (1)
#define EgressConfigurableACL0LargeTable_natOpPrio_width (1)
#define EgressConfigurableACL0LargeTable_tunnelEntryPrio_width (1)
#define EgressConfigurableACL0LargeTable_tunnelEntryUcMc_width (1)
#define EgressConfigurableACL0LargeTable_valid_width (1)
#define EgressConfigurableACL0LargeTable_natOpPtr_width (10)
#define EgressConfigurableACL0LargeTable_updateCounter_width (1)
#define EgressConfigurableACL0LargeTable_tunnelEntryPtr_width (4)
#define EgressConfigurableACL0LargeTable_forceSendToCpuOrigPkt_width (1)
#define EgressConfigurableACL0LargeTable_natOpValid_width (1)
#define EgressConfigurableACL0LargeTable_tunnelEntry_width (1)
#define EgressConfigurableACL0LargeTable_sendToPort_width (1)
#define EgressConfigurableACL0LargeTable_metaData_width (16)

//---- constants for register RouterPortEgressSAMACAddress -----
#define RouterPortEgressSAMACAddress_nr_entries (4)
// bit width of each register field:
#define RouterPortEgressSAMACAddress_selectMacEntryPortMask_width (6)
#define RouterPortEgressSAMACAddress_altMacAddress_width (48)

//---- constants for register OutputDisable -----
#define OutputDisable_nr_entries (6)
// bit width of each register field:
#define OutputDisable_egressQueue3Disabled_width (1)
#define OutputDisable_egressQueue7Disabled_width (1)
#define OutputDisable_egressQueue2Disabled_width (1)
#define OutputDisable_egressQueue6Disabled_width (1)
#define OutputDisable_egressQueue1Disabled_width (1)
#define OutputDisable_egressQueue5Disabled_width (1)
#define OutputDisable_egressQueue0Disabled_width (1)
#define OutputDisable_egressQueue4Disabled_width (1)

//---- constants for register L2AgingCollisionTable -----
#define L2AgingCollisionTable_nr_entries (32)
// bit width of each register field:
#define L2AgingCollisionTable_stat_width (1)
#define L2AgingCollisionTable_valid_width (1)
#define L2AgingCollisionTable_hit_width (1)

//---- constants for register IngressAdmissionControlInitialPointer -----
#define IngressAdmissionControlInitialPointer_nr_entries (64)
// bit width of each register field:
#define IngressAdmissionControlInitialPointer_mmpPtr_width (5)
#define IngressAdmissionControlInitialPointer_mmpValid_width (1)
#define IngressAdmissionControlInitialPointer_mmpOrder_width (2)

//---- constants for register L2TunnelEntryInstructionTable -----
#define L2TunnelEntryInstructionTable_nr_entries (16)
// bit width of each register field:
#define L2TunnelEntryInstructionTable_l3Type_width (2)
#define L2TunnelEntryInstructionTable_updateEtherType_width (1)
#define L2TunnelEntryInstructionTable_outerEtherType_width (16)
#define L2TunnelEntryInstructionTable_hasUdp_width (1)

//---- constants for register IngressConfigurableACL0LargeTable -----
#define IngressConfigurableACL0LargeTable_nr_entries (2048)
// bit width of each register field:
#define IngressConfigurableACL0LargeTable_newL4Value_width (16)
#define IngressConfigurableACL0LargeTable_color_width (2)
#define IngressConfigurableACL0LargeTable_metaDataPrio_width (1)
#define IngressConfigurableACL0LargeTable_forceColorPrio_width (1)
#define IngressConfigurableACL0LargeTable_enableUpdateIp_width (1)
#define IngressConfigurableACL0LargeTable_mmpValid_width (1)
#define IngressConfigurableACL0LargeTable_sendToPort_width (1)
#define IngressConfigurableACL0LargeTable_metaData_width (16)
#define IngressConfigurableACL0LargeTable_eQueue_width (3)
#define IngressConfigurableACL0LargeTable_newIpValue_width (32)
#define IngressConfigurableACL0LargeTable_updateL4SpOrDp_width (1)
#define IngressConfigurableACL0LargeTable_natOpValid_width (1)
#define IngressConfigurableACL0LargeTable_natOpPrio_width (1)
#define IngressConfigurableACL0LargeTable_enableUpdateL4_width (1)
#define IngressConfigurableACL0LargeTable_forceQueuePrio_width (1)
#define IngressConfigurableACL0LargeTable_valid_width (1)
#define IngressConfigurableACL0LargeTable_natOpPtr_width (11)
#define IngressConfigurableACL0LargeTable_updateCounter_width (1)
#define IngressConfigurableACL0LargeTable_forceQueue_width (1)
#define IngressConfigurableACL0LargeTable_tosMask_width (8)
#define IngressConfigurableACL0LargeTable_compareData_width (330)
#define IngressConfigurableACL0LargeTable_imPrio_width (1)
#define IngressConfigurableACL0LargeTable_updateTosExp_width (1)
#define IngressConfigurableACL0LargeTable_sendToCpu_width (1)
#define IngressConfigurableACL0LargeTable_updateSaOrDa_width (1)
#define IngressConfigurableACL0LargeTable_mmpOrder_width (2)
#define IngressConfigurableACL0LargeTable_inputMirror_width (1)
#define IngressConfigurableACL0LargeTable_forceColor_width (1)
#define IngressConfigurableACL0LargeTable_destInputMirror_width (3)
#define IngressConfigurableACL0LargeTable_metaDataValid_width (1)
#define IngressConfigurableACL0LargeTable_dropEnable_width (1)
#define IngressConfigurableACL0LargeTable_counter_width (6)
#define IngressConfigurableACL0LargeTable_newTosExp_width (8)
#define IngressConfigurableACL0LargeTable_forceSendToCpuOrigPkt_width (1)
#define IngressConfigurableACL0LargeTable_mmpPtr_width (5)
#define IngressConfigurableACL0LargeTable_destPort_width (3)

//---- constants for register EgressPortDepth -----
#define EgressPortDepth_nr_entries (6)
// bit width of each register field:
#define EgressPortDepth_packets_width (10)

//---- constants for register EgressConfigurableACL0SmallTable -----
#define EgressConfigurableACL0SmallTable_nr_entries (256)
// bit width of each register field:
#define EgressConfigurableACL0SmallTable_compareData_width (135)
#define EgressConfigurableACL0SmallTable_counter_width (6)
#define EgressConfigurableACL0SmallTable_sendToCpu_width (1)
#define EgressConfigurableACL0SmallTable_metaDataValid_width (1)
#define EgressConfigurableACL0SmallTable_destPort_width (3)
#define EgressConfigurableACL0SmallTable_dropEnable_width (1)
#define EgressConfigurableACL0SmallTable_metaDataPrio_width (1)
#define EgressConfigurableACL0SmallTable_natOpPrio_width (1)
#define EgressConfigurableACL0SmallTable_tunnelEntryPrio_width (1)
#define EgressConfigurableACL0SmallTable_tunnelEntryUcMc_width (1)
#define EgressConfigurableACL0SmallTable_valid_width (1)
#define EgressConfigurableACL0SmallTable_natOpPtr_width (10)
#define EgressConfigurableACL0SmallTable_updateCounter_width (1)
#define EgressConfigurableACL0SmallTable_tunnelEntryPtr_width (4)
#define EgressConfigurableACL0SmallTable_forceSendToCpuOrigPkt_width (1)
#define EgressConfigurableACL0SmallTable_natOpValid_width (1)
#define EgressConfigurableACL0SmallTable_tunnelEntry_width (1)
#define EgressConfigurableACL0SmallTable_sendToPort_width (1)
#define EgressConfigurableACL0SmallTable_metaData_width (16)

//---- constants for register SelectWhichEgressQoSMappingTableToUse -----
#define SelectWhichEgressQoSMappingTableToUse_nr_entries (128)
// bit width of each register field:
#define SelectWhichEgressQoSMappingTableToUse_whichTablePtr_width (1)
#define SelectWhichEgressQoSMappingTableToUse_updatePcp_width (1)
#define SelectWhichEgressQoSMappingTableToUse_pcp_width (3)
#define SelectWhichEgressQoSMappingTableToUse_whichTableToUse_width (3)
#define SelectWhichEgressQoSMappingTableToUse_updateCfiDei_width (1)
#define SelectWhichEgressQoSMappingTableToUse_cfiDei_width (1)

//---- constants for register DWRRWeightConfiguration -----
#define DWRRWeightConfiguration_nr_entries (48)
// bit width of each register field:
#define DWRRWeightConfiguration_weight_width (8)

//---- constants for register LinkAggregateWeight -----
#define LinkAggregateWeight_nr_entries (256)
// bit width of each register field:
#define LinkAggregateWeight_ports_width (6)

//---- constants for register L3RoutingDefault -----
#define L3RoutingDefault_nr_entries (4)
// bit width of each register field:
#define L3RoutingDefault_mmpValid_width (1)
#define L3RoutingDefault_nextHop_width (10)
#define L3RoutingDefault_mmpOrder_width (2)
#define L3RoutingDefault_pktDrop_width (1)
#define L3RoutingDefault_mmpPtr_width (5)
#define L3RoutingDefault_sendToCpu_width (1)

//---- constants for register TransmittedPacketsonEgressVRF -----
#define TransmittedPacketsonEgressVRF_nr_entries (4)
// bit width of each register field:
#define TransmittedPacketsonEgressVRF_packets_width (32)

//---- constants for register IngressAdmissionControlReset -----
#define IngressAdmissionControlReset_nr_entries (32)
// bit width of each register field:
#define IngressAdmissionControlReset_bucketReset_width (1)

//---- constants for register PrioShaperBucketCapacityConfiguration -----
#define PrioShaperBucketCapacityConfiguration_nr_entries (48)
// bit width of each register field:
#define PrioShaperBucketCapacityConfiguration_bucketCapacity_width (17)

//---- constants for register EgressVLANTranslationTCAM -----
#define EgressVLANTranslationTCAM_nr_entries (128)
// bit width of each register field:
#define EgressVLANTranslationTCAM_outermostVidTypemask_width (1)
#define EgressVLANTranslationTCAM_dstPortmask_width (3)
#define EgressVLANTranslationTCAM_outermostVid_width (12)
#define EgressVLANTranslationTCAM_outermostVidType_width (1)
#define EgressVLANTranslationTCAM_valid_width (1)
#define EgressVLANTranslationTCAM_outermostVidmask_width (12)
#define EgressVLANTranslationTCAM_dstPort_width (3)

//---- constants for register L2LookupCollisionTable -----
#define L2LookupCollisionTable_nr_entries (32)
// bit width of each register field:
#define L2LookupCollisionTable_gid_width (12)
#define L2LookupCollisionTable_macAddr_width (48)

//---- constants for register NextHopPacketModifications -----
#define NextHopPacketModifications_nr_entries (1024)
// bit width of each register field:
#define NextHopPacketModifications_innerPcpSel_width (2)
#define NextHopPacketModifications_innerCfiDei_width (1)
#define NextHopPacketModifications_outerCfiDeiSel_width (2)
#define NextHopPacketModifications_innerCfiDeiSel_width (2)
#define NextHopPacketModifications_msptPtr_width (4)
#define NextHopPacketModifications_outerVlanAppend_width (1)
#define NextHopPacketModifications_outerCfiDei_width (1)
#define NextHopPacketModifications_innerEthType_width (2)
#define NextHopPacketModifications_outerVid_width (12)
#define NextHopPacketModifications_outerPcpSel_width (2)
#define NextHopPacketModifications_innerPcp_width (3)
#define NextHopPacketModifications_valid_width (1)
#define NextHopPacketModifications_outerEthType_width (2)
#define NextHopPacketModifications_outerPcp_width (3)
#define NextHopPacketModifications_innerVlanAppend_width (1)
#define NextHopPacketModifications_innerVid_width (12)

//---- constants for register IPv4TOSFieldToEgressQueueMappingTable -----
#define IPv4TOSFieldToEgressQueueMappingTable_nr_entries (256)
// bit width of each register field:
#define IPv4TOSFieldToEgressQueueMappingTable_pQueue_width (3)

//---- constants for register IngressVIDInnerVIDRangeAssignmentAnswer -----
#define IngressVIDInnerVIDRangeAssignmentAnswer_nr_entries (4)
// bit width of each register field:
#define IngressVIDInnerVIDRangeAssignmentAnswer_ingressVid_width (12)
#define IngressVIDInnerVIDRangeAssignmentAnswer_order_width (2)

//---- constants for register SecondTunnelExitLookupTCAMAnswer -----
#define SecondTunnelExitLookupTCAMAnswer_nr_entries (16)
// bit width of each register field:
#define SecondTunnelExitLookupTCAMAnswer_newVid_width (12)
#define SecondTunnelExitLookupTCAMAnswer_updateL4Protocol_width (1)
#define SecondTunnelExitLookupTCAMAnswer_ethType_width (16)
#define SecondTunnelExitLookupTCAMAnswer_dropPkt_width (1)
#define SecondTunnelExitLookupTCAMAnswer_replaceVid_width (1)
#define SecondTunnelExitLookupTCAMAnswer_tunnelExitEgressPtr_width (5)
#define SecondTunnelExitLookupTCAMAnswer_updateEthType_width (1)
#define SecondTunnelExitLookupTCAMAnswer_l4Protocol_width (8)
#define SecondTunnelExitLookupTCAMAnswer_dontExit_width (1)
#define SecondTunnelExitLookupTCAMAnswer_whereToRemove_width (2)
#define SecondTunnelExitLookupTCAMAnswer_removeVlan_width (1)
#define SecondTunnelExitLookupTCAMAnswer_howManyBytesToRemove_width (8)

//---- constants for register EgressPortConfiguration -----
#define EgressPortConfiguration_nr_entries (6)
// bit width of each register field:
#define EgressPortConfiguration_vid_width (12)
#define EgressPortConfiguration_dropStaggedVlans_width (1)
#define EgressPortConfiguration_disabled_width (1)
#define EgressPortConfiguration_dropCtaggedVlans_width (1)
#define EgressPortConfiguration_typeSel_width (2)
#define EgressPortConfiguration_cfiDei_width (1)
#define EgressPortConfiguration_removeSNAP_width (1)
#define EgressPortConfiguration_dropSingleTaggedVlans_width (1)
#define EgressPortConfiguration_dropSStaggedVlans_width (1)
#define EgressPortConfiguration_pcp_width (3)
#define EgressPortConfiguration_vidSel_width (2)
#define EgressPortConfiguration_pcpSel_width (2)
#define EgressPortConfiguration_colorRemap_width (1)
#define EgressPortConfiguration_vlanSingleOp_width (3)
#define EgressPortConfiguration_dropCCtaggedVlans_width (1)
#define EgressPortConfiguration_dropSCtaggedVlans_width (1)
#define EgressPortConfiguration_cfiDeiSel_width (2)
#define EgressPortConfiguration_dropUntaggedVlans_width (1)
#define EgressPortConfiguration_moreThanOneVlans_width (1)
#define EgressPortConfiguration_useEgressQueueRemapping_width (1)
#define EgressPortConfiguration_dropDualTaggedVlans_width (1)
#define EgressPortConfiguration_dropCStaggedVlans_width (1)

//---- constants for register EgressSpanningTreeDrop -----
#define EgressSpanningTreeDrop_nr_entries (6)
// bit width of each register field:
#define EgressSpanningTreeDrop_packets_width (32)

//---- constants for register IngressConfigurableACL0TCAM -----
#define IngressConfigurableACL0TCAM_nr_entries (16)
// bit width of each register field:
#define IngressConfigurableACL0TCAM_valid_width (1)
#define IngressConfigurableACL0TCAM_mask_width (330)
#define IngressConfigurableACL0TCAM_compareData_width (330)

//---- constants for register L2AgingStatusShadowTable -----
#define L2AgingStatusShadowTable_nr_entries (4096)
// bit width of each register field:
#define L2AgingStatusShadowTable_valid_width (1)

//---- constants for register TCXoffFFAThreshold -----
#define TCXoffFFAThreshold_nr_entries (8)
// bit width of each register field:
#define TCXoffFFAThreshold_cells_width (10)
#define TCXoffFFAThreshold_enable_width (1)
#define TCXoffFFAThreshold_trip_width (1)

//---- constants for register EgressConfigurableACL1TCAM -----
#define EgressConfigurableACL1TCAM_nr_entries (16)
// bit width of each register field:
#define EgressConfigurableACL1TCAM_valid_width (1)
#define EgressConfigurableACL1TCAM_mask_width (540)
#define EgressConfigurableACL1TCAM_compareData_width (540)

//---- constants for register L2AgingTable -----
#define L2AgingTable_nr_entries (4096)
// bit width of each register field:
#define L2AgingTable_stat_width (1)
#define L2AgingTable_valid_width (1)
#define L2AgingTable_hit_width (1)

//---- constants for register IngressRouterTable -----
#define IngressRouterTable_nr_entries (4)
// bit width of each register field:
#define IngressRouterTable_acceptIPv6_width (1)
#define IngressRouterTable_ecmpUseIpL4Dp_width (1)
#define IngressRouterTable_acceptIPv4_width (1)
#define IngressRouterTable_ecmpUseIpSa_width (1)
#define IngressRouterTable_ipv4HitUpdates_width (1)
#define IngressRouterTable_ecmpUseIpProto_width (1)
#define IngressRouterTable_ipv6HitUpdates_width (1)
#define IngressRouterTable_minTtlToCpu_width (1)
#define IngressRouterTable_ecmpUseIpDa_width (1)
#define IngressRouterTable_mmpOrder_width (2)
#define IngressRouterTable_mplsHitUpdates_width (1)
#define IngressRouterTable_ecmpUseIpTos_width (1)
#define IngressRouterTable_mmpPtr_width (5)
#define IngressRouterTable_ecmpUseIpL4Sp_width (1)
#define IngressRouterTable_mmpValid_width (1)
#define IngressRouterTable_acceptMPLS_width (1)
#define IngressRouterTable_minTTL_width (8)

//---- constants for register NextHopPacketInsertMPLSHeader -----
#define NextHopPacketInsertMPLSHeader_nr_entries (16)
// bit width of each register field:
#define NextHopPacketInsertMPLSHeader_ttl0_width (8)
#define NextHopPacketInsertMPLSHeader_ttl1_width (8)
#define NextHopPacketInsertMPLSHeader_ttl2_width (8)
#define NextHopPacketInsertMPLSHeader_ttl3_width (8)
#define NextHopPacketInsertMPLSHeader_copyTtl3_width (1)
#define NextHopPacketInsertMPLSHeader_copyTtl2_width (1)
#define NextHopPacketInsertMPLSHeader_copyTtl1_width (1)
#define NextHopPacketInsertMPLSHeader_copyTtl0_width (1)
#define NextHopPacketInsertMPLSHeader_howManyLabelsToInsert_width (3)
#define NextHopPacketInsertMPLSHeader_exp3_width (3)
#define NextHopPacketInsertMPLSHeader_exp2_width (3)
#define NextHopPacketInsertMPLSHeader_exp1_width (3)
#define NextHopPacketInsertMPLSHeader_exp0_width (3)
#define NextHopPacketInsertMPLSHeader_expFromQueue3_width (1)
#define NextHopPacketInsertMPLSHeader_expFromQueue2_width (1)
#define NextHopPacketInsertMPLSHeader_expFromQueue1_width (1)
#define NextHopPacketInsertMPLSHeader_expFromQueue0_width (1)
#define NextHopPacketInsertMPLSHeader_whichEthernetType_width (1)
#define NextHopPacketInsertMPLSHeader_mplsLabel0_width (20)
#define NextHopPacketInsertMPLSHeader_mplsLabel1_width (20)
#define NextHopPacketInsertMPLSHeader_mplsLabel2_width (20)
#define NextHopPacketInsertMPLSHeader_mplsLabel3_width (20)

//---- constants for register MACInterfaceCountersForTX -----
#define MACInterfaceCountersForTX_nr_entries (6)
// bit width of each register field:
#define MACInterfaceCountersForTX_packets_width (32)
#define MACInterfaceCountersForTX_halt_width (32)
#define MACInterfaceCountersForTX_error_width (32)

//---- constants for register TunnelEntryInstructionTable -----
#define TunnelEntryInstructionTable_nr_entries (16)
// bit width of each register field:
#define TunnelEntryInstructionTable_incVlansInLength_width (1)
#define TunnelEntryInstructionTable_lengthPosOffset_width (14)
#define TunnelEntryInstructionTable_insertLength_width (1)
#define TunnelEntryInstructionTable_tunnelEntryType_width (2)
#define TunnelEntryInstructionTable_lengthNegOffset_width (14)
#define TunnelEntryInstructionTable_tunnelHeaderLen_width (7)
#define TunnelEntryInstructionTable_tunnelHeaderPtr_width (4)
#define TunnelEntryInstructionTable_lengthPos_width (7)

//---- constants for register IPUnicastRoutedCounter -----
#define IPUnicastRoutedCounter_nr_entries (6)
// bit width of each register field:
#define IPUnicastRoutedCounter_packets_width (32)

//---- constants for register IngressAdmissionControlCurrentSize -----
#define IngressAdmissionControlCurrentSize_nr_entries (32)
// bit width of each register field:
#define IngressAdmissionControlCurrentSize_tokens1_width (16)
#define IngressAdmissionControlCurrentSize_tokens0_width (16)

//---- constants for register L2DAHashLookupTable -----
#define L2DAHashLookupTable_nr_entries (4096)
// bit width of each register field:
#define L2DAHashLookupTable_gid_width (12)
#define L2DAHashLookupTable_macAddr_width (48)

//---- constants for register IPMulticastReceivedCounter -----
#define IPMulticastReceivedCounter_nr_entries (6)
// bit width of each register field:
#define IPMulticastReceivedCounter_packets_width (32)

//---- constants for register TCFFAUsed -----
#define TCFFAUsed_nr_entries (8)
// bit width of each register field:
#define TCFFAUsed_cells_width (10)

//---- constants for register EgressNATOperation -----
#define EgressNATOperation_nr_entries (1024)
// bit width of each register field:
#define EgressNATOperation_replaceSrc_width (1)
#define EgressNATOperation_replaceL4Port_width (1)
#define EgressNATOperation_replaceIP_width (1)
#define EgressNATOperation_ipAddress_width (32)
#define EgressNATOperation_port_width (16)

//---- constants for register IngressConfigurableACL1TCAMAnswer -----
#define IngressConfigurableACL1TCAMAnswer_nr_entries (8)
// bit width of each register field:
#define IngressConfigurableACL1TCAMAnswer_counter_width (6)
#define IngressConfigurableACL1TCAMAnswer_newEthType_width (2)
#define IngressConfigurableACL1TCAMAnswer_newL4Value_width (16)
#define IngressConfigurableACL1TCAMAnswer_color_width (2)
#define IngressConfigurableACL1TCAMAnswer_updateVid_width (1)
#define IngressConfigurableACL1TCAMAnswer_forceColorPrio_width (1)
#define IngressConfigurableACL1TCAMAnswer_forceSendToCpuOrigPkt_width (1)
#define IngressConfigurableACL1TCAMAnswer_forceVid_width (12)
#define IngressConfigurableACL1TCAMAnswer_tunnelEntryPtr_width (4)
#define IngressConfigurableACL1TCAMAnswer_enableUpdateIp_width (1)
#define IngressConfigurableACL1TCAMAnswer_mmpValid_width (1)
#define IngressConfigurableACL1TCAMAnswer_sendToPort_width (1)
#define IngressConfigurableACL1TCAMAnswer_ptp_width (1)
#define IngressConfigurableACL1TCAMAnswer_newPcpValue_width (3)
#define IngressConfigurableACL1TCAMAnswer_cfiDeiPrio_width (1)
#define IngressConfigurableACL1TCAMAnswer_eQueue_width (3)
#define IngressConfigurableACL1TCAMAnswer_newIpValue_width (32)
#define IngressConfigurableACL1TCAMAnswer_updateL4SpOrDp_width (1)
#define IngressConfigurableACL1TCAMAnswer_forceColor_width (1)
#define IngressConfigurableACL1TCAMAnswer_natOpPrio_width (1)
#define IngressConfigurableACL1TCAMAnswer_enableUpdateL4_width (1)
#define IngressConfigurableACL1TCAMAnswer_forceQueuePrio_width (1)
#define IngressConfigurableACL1TCAMAnswer_tunnelEntryPrio_width (1)
#define IngressConfigurableACL1TCAMAnswer_updateTosExp_width (1)
#define IngressConfigurableACL1TCAMAnswer_natOpValid_width (1)
#define IngressConfigurableACL1TCAMAnswer_natOpPtr_width (11)
#define IngressConfigurableACL1TCAMAnswer_metaDataPrio_width (1)
#define IngressConfigurableACL1TCAMAnswer_pcpPrio_width (1)
#define IngressConfigurableACL1TCAMAnswer_metaData_width (16)
#define IngressConfigurableACL1TCAMAnswer_forceQueue_width (1)
#define IngressConfigurableACL1TCAMAnswer_tunnelEntry_width (1)
#define IngressConfigurableACL1TCAMAnswer_vidPrio_width (1)
#define IngressConfigurableACL1TCAMAnswer_imPrio_width (1)
#define IngressConfigurableACL1TCAMAnswer_ethPrio_width (1)
#define IngressConfigurableACL1TCAMAnswer_sendToCpu_width (1)
#define IngressConfigurableACL1TCAMAnswer_updateSaOrDa_width (1)
#define IngressConfigurableACL1TCAMAnswer_mmpOrder_width (2)
#define IngressConfigurableACL1TCAMAnswer_destPort_width (3)
#define IngressConfigurableACL1TCAMAnswer_updateCounter_width (1)
#define IngressConfigurableACL1TCAMAnswer_newVidValue_width (12)
#define IngressConfigurableACL1TCAMAnswer_inputMirror_width (1)
#define IngressConfigurableACL1TCAMAnswer_forceVidPrio_width (1)
#define IngressConfigurableACL1TCAMAnswer_forceVidValid_width (1)
#define IngressConfigurableACL1TCAMAnswer_destInputMirror_width (3)
#define IngressConfigurableACL1TCAMAnswer_metaDataValid_width (1)
#define IngressConfigurableACL1TCAMAnswer_dropEnable_width (1)
#define IngressConfigurableACL1TCAMAnswer_updateEType_width (1)
#define IngressConfigurableACL1TCAMAnswer_newTosExp_width (8)
#define IngressConfigurableACL1TCAMAnswer_updateCfiDei_width (1)
#define IngressConfigurableACL1TCAMAnswer_tosMask_width (8)
#define IngressConfigurableACL1TCAMAnswer_tunnelEntryUcMc_width (1)
#define IngressConfigurableACL1TCAMAnswer_noLearning_width (1)
#define IngressConfigurableACL1TCAMAnswer_updatePcp_width (1)
#define IngressConfigurableACL1TCAMAnswer_mmpPtr_width (5)
#define IngressConfigurableACL1TCAMAnswer_newCfiDeiValue_width (1)

//---- constants for register IngressConfigurableACL3TCAMAnswer -----
#define IngressConfigurableACL3TCAMAnswer_nr_entries (16)
// bit width of each register field:
#define IngressConfigurableACL3TCAMAnswer_eQueue_width (3)
#define IngressConfigurableACL3TCAMAnswer_metaDataValid_width (1)
#define IngressConfigurableACL3TCAMAnswer_color_width (2)
#define IngressConfigurableACL3TCAMAnswer_mmpValid_width (1)
#define IngressConfigurableACL3TCAMAnswer_sendToCpu_width (1)
#define IngressConfigurableACL3TCAMAnswer_forceColorPrio_width (1)
#define IngressConfigurableACL3TCAMAnswer_mmpOrder_width (2)
#define IngressConfigurableACL3TCAMAnswer_forceSendToCpuOrigPkt_width (1)
#define IngressConfigurableACL3TCAMAnswer_dropEnable_width (1)
#define IngressConfigurableACL3TCAMAnswer_forceColor_width (1)
#define IngressConfigurableACL3TCAMAnswer_forceQueuePrio_width (1)
#define IngressConfigurableACL3TCAMAnswer_metaDataPrio_width (1)
#define IngressConfigurableACL3TCAMAnswer_mmpPtr_width (5)
#define IngressConfigurableACL3TCAMAnswer_forceQueue_width (1)
#define IngressConfigurableACL3TCAMAnswer_destPort_width (3)
#define IngressConfigurableACL3TCAMAnswer_sendToPort_width (1)
#define IngressConfigurableACL3TCAMAnswer_metaData_width (16)

//---- constants for register TunnelExitLookupTCAMAnswer -----
#define TunnelExitLookupTCAMAnswer_nr_entries (16)
// bit width of each register field:
#define TunnelExitLookupTCAMAnswer_secondShift_width (8)
#define TunnelExitLookupTCAMAnswer_lookupMask_width (80)
#define TunnelExitLookupTCAMAnswer_direct_width (1)
#define TunnelExitLookupTCAMAnswer_secondIncludeVlan_width (1)
#define TunnelExitLookupTCAMAnswer_srcPortMask_width (6)
#define TunnelExitLookupTCAMAnswer_key_width (80)
#define TunnelExitLookupTCAMAnswer_sendToCpu_width (1)
#define TunnelExitLookupTCAMAnswer_tabIndex_width (2)

//---- constants for register ReservedSourceMACAddressRange -----
#define ReservedSourceMACAddressRange_nr_entries (4)
// bit width of each register field:
#define ReservedSourceMACAddressRange_stopAddr_width (48)
#define ReservedSourceMACAddressRange_enable_width (6)
#define ReservedSourceMACAddressRange_eQueue_width (3)
#define ReservedSourceMACAddressRange_forceQueue_width (1)
#define ReservedSourceMACAddressRange_color_width (2)
#define ReservedSourceMACAddressRange_mmpValid_width (1)
#define ReservedSourceMACAddressRange_sendToCpu_width (1)
#define ReservedSourceMACAddressRange_startAddr_width (48)
#define ReservedSourceMACAddressRange_mmpOrder_width (2)
#define ReservedSourceMACAddressRange_dropEnable_width (1)
#define ReservedSourceMACAddressRange_mmpPtr_width (5)
#define ReservedSourceMACAddressRange_forceColor_width (1)

//---- constants for register QueueOffDrop -----
#define QueueOffDrop_nr_entries (6)
// bit width of each register field:
#define QueueOffDrop_packets_width (32)

//---- constants for register TunnelEntryHeaderData -----
#define TunnelEntryHeaderData_nr_entries (16)
// bit width of each register field:
#define TunnelEntryHeaderData_data_width (640)

//---- constants for register QueueShaperBucketThresholdConfiguration -----
#define QueueShaperBucketThresholdConfiguration_nr_entries (48)
// bit width of each register field:
#define QueueShaperBucketThresholdConfiguration_threshold_width (17)

//---- constants for register IngressConfigurableACL0TCAMAnswer -----
#define IngressConfigurableACL0TCAMAnswer_nr_entries (16)
// bit width of each register field:
#define IngressConfigurableACL0TCAMAnswer_newL4Value_width (16)
#define IngressConfigurableACL0TCAMAnswer_color_width (2)
#define IngressConfigurableACL0TCAMAnswer_metaDataPrio_width (1)
#define IngressConfigurableACL0TCAMAnswer_forceColorPrio_width (1)
#define IngressConfigurableACL0TCAMAnswer_enableUpdateIp_width (1)
#define IngressConfigurableACL0TCAMAnswer_mmpValid_width (1)
#define IngressConfigurableACL0TCAMAnswer_sendToPort_width (1)
#define IngressConfigurableACL0TCAMAnswer_metaData_width (16)
#define IngressConfigurableACL0TCAMAnswer_eQueue_width (3)
#define IngressConfigurableACL0TCAMAnswer_newIpValue_width (32)
#define IngressConfigurableACL0TCAMAnswer_updateL4SpOrDp_width (1)
#define IngressConfigurableACL0TCAMAnswer_natOpValid_width (1)
#define IngressConfigurableACL0TCAMAnswer_natOpPrio_width (1)
#define IngressConfigurableACL0TCAMAnswer_enableUpdateL4_width (1)
#define IngressConfigurableACL0TCAMAnswer_forceQueuePrio_width (1)
#define IngressConfigurableACL0TCAMAnswer_natOpPtr_width (11)
#define IngressConfigurableACL0TCAMAnswer_updateCounter_width (1)
#define IngressConfigurableACL0TCAMAnswer_forceQueue_width (1)
#define IngressConfigurableACL0TCAMAnswer_tosMask_width (8)
#define IngressConfigurableACL0TCAMAnswer_imPrio_width (1)
#define IngressConfigurableACL0TCAMAnswer_updateTosExp_width (1)
#define IngressConfigurableACL0TCAMAnswer_sendToCpu_width (1)
#define IngressConfigurableACL0TCAMAnswer_updateSaOrDa_width (1)
#define IngressConfigurableACL0TCAMAnswer_mmpOrder_width (2)
#define IngressConfigurableACL0TCAMAnswer_inputMirror_width (1)
#define IngressConfigurableACL0TCAMAnswer_forceColor_width (1)
#define IngressConfigurableACL0TCAMAnswer_destInputMirror_width (3)
#define IngressConfigurableACL0TCAMAnswer_metaDataValid_width (1)
#define IngressConfigurableACL0TCAMAnswer_dropEnable_width (1)
#define IngressConfigurableACL0TCAMAnswer_counter_width (6)
#define IngressConfigurableACL0TCAMAnswer_newTosExp_width (8)
#define IngressConfigurableACL0TCAMAnswer_forceSendToCpuOrigPkt_width (1)
#define IngressConfigurableACL0TCAMAnswer_mmpPtr_width (5)
#define IngressConfigurableACL0TCAMAnswer_destPort_width (3)

//---- constants for register EgressConfigurableACLMatchCounter -----
#define EgressConfigurableACLMatchCounter_nr_entries (64)
// bit width of each register field:
#define EgressConfigurableACLMatchCounter_packets_width (32)

//---- constants for register IngressVIDOuterVIDRangeSearchData -----
#define IngressVIDOuterVIDRangeSearchData_nr_entries (4)
// bit width of each register field:
#define IngressVIDOuterVIDRangeSearchData_start_width (12)
#define IngressVIDOuterVIDRangeSearchData_vtype_width (1)
#define IngressVIDOuterVIDRangeSearchData_end_width (12)
#define IngressVIDOuterVIDRangeSearchData_ports_width (6)

//---- constants for register EgressVLANTranslationTCAMAnswer -----
#define EgressVLANTranslationTCAMAnswer_nr_entries (128)
// bit width of each register field:
#define EgressVLANTranslationTCAMAnswer_newVid_width (12)
#define EgressVLANTranslationTCAMAnswer_ethType_width (16)

//---- constants for register L2ActionTableSourcePort -----
#define L2ActionTableSourcePort_nr_entries (128)
// bit width of each register field:
#define L2ActionTableSourcePort_useSpecialAllow_width (1)
#define L2ActionTableSourcePort_dropPortMove_width (1)
#define L2ActionTableSourcePort_sendToCpu_width (1)
#define L2ActionTableSourcePort_drop_width (1)
#define L2ActionTableSourcePort_noPortMove_width (1)
#define L2ActionTableSourcePort_mmpOrder_width (2)
#define L2ActionTableSourcePort_forceSendToCpuOrigPkt_width (1)
#define L2ActionTableSourcePort_noLearningUc_width (1)
#define L2ActionTableSourcePort_dropAll_width (1)
#define L2ActionTableSourcePort_allowPtr_width (2)
#define L2ActionTableSourcePort_mmpPtr_width (5)
#define L2ActionTableSourcePort_mmpValid_width (1)
#define L2ActionTableSourcePort_noLearningMc_width (1)

//---- constants for register FloodingActionSendtoPort -----
#define FloodingActionSendtoPort_nr_entries (6)
// bit width of each register field:
#define FloodingActionSendtoPort_enable_width (1)
#define FloodingActionSendtoPort_destPort_width (3)

//---- constants for register EgressMPLSTTLTable -----
#define EgressMPLSTTLTable_nr_entries (4)
// bit width of each register field:
#define EgressMPLSTTLTable_newTTL_width (8)
#define EgressMPLSTTLTable_addNewTTL_width (1)

//---- constants for register MBSCDrop -----
#define MBSCDrop_nr_entries (6)
// bit width of each register field:
#define MBSCDrop_packets_width (32)

//---- constants for register EgressResourceManagerDrop -----
#define EgressResourceManagerDrop_nr_entries (6)
// bit width of each register field:
#define EgressResourceManagerDrop_packets_width (32)

//---- constants for register MACRXBrokenPackets -----
#define MACRXBrokenPackets_nr_entries (6)
// bit width of each register field:
#define MACRXBrokenPackets_packets_width (32)

//---- constants for register IngressVIDOuterVIDRangeAssignmentAnswer -----
#define IngressVIDOuterVIDRangeAssignmentAnswer_nr_entries (4)
// bit width of each register field:
#define IngressVIDOuterVIDRangeAssignmentAnswer_ingressVid_width (12)
#define IngressVIDOuterVIDRangeAssignmentAnswer_order_width (2)

//---- constants for register IngressVIDMACRangeSearchData -----
#define IngressVIDMACRangeSearchData_nr_entries (4)
// bit width of each register field:
#define IngressVIDMACRangeSearchData_saOrDa_width (1)
#define IngressVIDMACRangeSearchData_start_width (48)
#define IngressVIDMACRangeSearchData_end_width (48)
#define IngressVIDMACRangeSearchData_ports_width (6)

//---- constants for register IngressNATHitStatus -----
#define IngressNATHitStatus_nr_entries (2048)
// bit width of each register field:
#define IngressNATHitStatus_hit_width (1)

//---- constants for register ColorRemapFromIngressAdmissionControl -----
#define ColorRemapFromIngressAdmissionControl_nr_entries (32)
// bit width of each register field:
#define ColorRemapFromIngressAdmissionControl_color2Tos_width (24)
#define ColorRemapFromIngressAdmissionControl_colorMode_width (2)
#define ColorRemapFromIngressAdmissionControl_enable_width (1)
#define ColorRemapFromIngressAdmissionControl_color2Dei_width (3)
#define ColorRemapFromIngressAdmissionControl_tosMask_width (8)

//---- constants for register HairpinEnable -----
#define HairpinEnable_nr_entries (6)
// bit width of each register field:
#define HairpinEnable_allowUc_width (1)
#define HairpinEnable_allowMc_width (1)
#define HairpinEnable_allowFlood_width (1)

//---- constants for register MPLSEXPFieldToPacketColorMappingTable -----
#define MPLSEXPFieldToPacketColorMappingTable_nr_entries (8)
// bit width of each register field:
#define MPLSEXPFieldToPacketColorMappingTable_color_width (2)

//---- constants for register IPMulticastACLDropCounter -----
#define IPMulticastACLDropCounter_nr_entries (6)
// bit width of each register field:
#define IPMulticastACLDropCounter_packets_width (32)

//---- constants for register QueueShaperBucketCapacityConfiguration -----
#define QueueShaperBucketCapacityConfiguration_nr_entries (48)
// bit width of each register field:
#define QueueShaperBucketCapacityConfiguration_bucketCapacity_width (17)

//---- constants for register IngressVIDInnerVIDRangeSearchData -----
#define IngressVIDInnerVIDRangeSearchData_nr_entries (4)
// bit width of each register field:
#define IngressVIDInnerVIDRangeSearchData_start_width (12)
#define IngressVIDInnerVIDRangeSearchData_vtype_width (1)
#define IngressVIDInnerVIDRangeSearchData_end_width (12)
#define IngressVIDInnerVIDRangeSearchData_ports_width (6)

//---- constants for register NextHopHitStatus -----
#define NextHopHitStatus_nr_entries (1024)
// bit width of each register field:
#define NextHopHitStatus_mpls_width (1)
#define NextHopHitStatus_ipv4_width (1)
#define NextHopHitStatus_ipv6_width (1)

//---- constants for register EgressQueueToMPLSEXPMappingTable -----
#define EgressQueueToMPLSEXPMappingTable_nr_entries (8)
// bit width of each register field:
#define EgressQueueToMPLSEXPMappingTable_exp_width (3)

//---- constants for register EgressRouterTable -----
#define EgressRouterTable_nr_entries (4)
// bit width of each register field:
#define EgressRouterTable_newTTL_width (8)
#define EgressRouterTable_addNewTTL_width (1)

//---- constants for register MACRXLongPacketDrop -----
#define MACRXLongPacketDrop_nr_entries (6)
// bit width of each register field:
#define MACRXLongPacketDrop_packets_width (32)

//---- constants for register PortXoffFFAThreshold -----
#define PortXoffFFAThreshold_nr_entries (6)
// bit width of each register field:
#define PortXoffFFAThreshold_cells_width (10)
#define PortXoffFFAThreshold_enable_width (1)
#define PortXoffFFAThreshold_trip_width (1)

//---- constants for register TCXonFFAThreshold -----
#define TCXonFFAThreshold_nr_entries (8)
// bit width of each register field:
#define TCXonFFAThreshold_cells_width (10)

//---- constants for register LinkAggregationToPhysicalPortsMembers -----
#define LinkAggregationToPhysicalPortsMembers_nr_entries (6)
// bit width of each register field:
#define LinkAggregationToPhysicalPortsMembers_members_width (6)

//---- constants for register L3TunnelEntryInstructionTable -----
#define L3TunnelEntryInstructionTable_nr_entries (16)
// bit width of each register field:
#define L3TunnelEntryInstructionTable_l4Protocol_width (8)
#define L3TunnelEntryInstructionTable_updateL4Type_width (2)

//---- constants for register SecondTunnelExitLookupTCAM -----
#define SecondTunnelExitLookupTCAM_nr_entries (16)
// bit width of each register field:
#define SecondTunnelExitLookupTCAM_tabKeymask_width (2)
#define SecondTunnelExitLookupTCAM_pktKey_width (80)
#define SecondTunnelExitLookupTCAM_pktKeymask_width (80)
#define SecondTunnelExitLookupTCAM_tabKey_width (2)
#define SecondTunnelExitLookupTCAM_valid_width (1)

//---- constants for register MapQueuetoPriority -----
#define MapQueuetoPriority_nr_entries (6)
// bit width of each register field:
#define MapQueuetoPriority_prio3_width (3)
#define MapQueuetoPriority_prio2_width (3)
#define MapQueuetoPriority_prio1_width (3)
#define MapQueuetoPriority_prio0_width (3)
#define MapQueuetoPriority_prio7_width (3)
#define MapQueuetoPriority_prio6_width (3)
#define MapQueuetoPriority_prio5_width (3)
#define MapQueuetoPriority_prio4_width (3)

//---- constants for register NextHopDAMAC -----
#define NextHopDAMAC_nr_entries (1024)
// bit width of each register field:
#define NextHopDAMAC_daMac_width (48)

//---- constants for register ReceivedPacketsonIngressVRF -----
#define ReceivedPacketsonIngressVRF_nr_entries (4)
// bit width of each register field:
#define ReceivedPacketsonIngressVRF_packets_width (32)

//---- constants for register IngressConfigurableACL1PreLookup -----
#define IngressConfigurableACL1PreLookup_nr_entries (512)
// bit width of each register field:
#define IngressConfigurableACL1PreLookup_valid_width (1)
#define IngressConfigurableACL1PreLookup_rulePtr_width (3)

//---- constants for register MACRXMaximumPacketLength -----
#define MACRXMaximumPacketLength_nr_entries (6)
// bit width of each register field:
#define MACRXMaximumPacketLength_bytes_width (32)

//---- constants for register EgressPortFilteringDrop -----
#define EgressPortFilteringDrop_nr_entries (6)
// bit width of each register field:
#define EgressPortFilteringDrop_packets_width (32)

//---- constants for register IngressConfigurableACL0RulesSetup -----
#define IngressConfigurableACL0RulesSetup_nr_entries (8)
// bit width of each register field:
#define IngressConfigurableACL0RulesSetup_fieldSelectBitmask_width (14)

//---- constants for register MACRXShortPacketDrop -----
#define MACRXShortPacketDrop_nr_entries (6)
// bit width of each register field:
#define MACRXShortPacketDrop_packets_width (32)

//---- constants for register IngressConfigurableACL2PreLookup -----
#define IngressConfigurableACL2PreLookup_nr_entries (64)
// bit width of each register field:
#define IngressConfigurableACL2PreLookup_valid_width (1)
#define IngressConfigurableACL2PreLookup_rulePtr_width (2)

//---- constants for register PortTCTailDropTotalThreshold -----
#define PortTCTailDropTotalThreshold_nr_entries (48)
// bit width of each register field:
#define PortTCTailDropTotalThreshold_cells_width (10)
#define PortTCTailDropTotalThreshold_enable_width (1)
#define PortTCTailDropTotalThreshold_trip_width (1)

//---- constants for register VLANPCPToQueueMappingTable -----
#define VLANPCPToQueueMappingTable_nr_entries (8)
// bit width of each register field:
#define VLANPCPToQueueMappingTable_pQueue_width (3)

//---- constants for register L2MulticastStormControlBucketCapacityConfiguration -----
#define L2MulticastStormControlBucketCapacityConfiguration_nr_entries (6)
// bit width of each register field:
#define L2MulticastStormControlBucketCapacityConfiguration_bucketCapacity_width (16)

//---- constants for register L2MulticastTable -----
#define L2MulticastTable_nr_entries (64)
// bit width of each register field:
#define L2MulticastTable_mcPortMask_width (6)
#define L2MulticastTable_metaData_width (16)

//---- constants for register ReservedDestinationMACAddressRange -----
#define ReservedDestinationMACAddressRange_nr_entries (4)
// bit width of each register field:
#define ReservedDestinationMACAddressRange_stopAddr_width (48)
#define ReservedDestinationMACAddressRange_enable_width (6)
#define ReservedDestinationMACAddressRange_eQueue_width (3)
#define ReservedDestinationMACAddressRange_forceQueue_width (1)
#define ReservedDestinationMACAddressRange_color_width (2)
#define ReservedDestinationMACAddressRange_mmpValid_width (1)
#define ReservedDestinationMACAddressRange_sendToCpu_width (1)
#define ReservedDestinationMACAddressRange_startAddr_width (48)
#define ReservedDestinationMACAddressRange_mmpOrder_width (2)
#define ReservedDestinationMACAddressRange_dropEnable_width (1)
#define ReservedDestinationMACAddressRange_mmpPtr_width (5)
#define ReservedDestinationMACAddressRange_forceColor_width (1)

//---- constants for register TCTailDropFFAThreshold -----
#define TCTailDropFFAThreshold_nr_entries (8)
// bit width of each register field:
#define TCTailDropFFAThreshold_cells_width (10)
#define TCTailDropFFAThreshold_enable_width (1)
#define TCTailDropFFAThreshold_trip_width (1)

//---- constants for register IngressConfigurableACL1SmallTable -----
#define IngressConfigurableACL1SmallTable_nr_entries (8)
// bit width of each register field:
#define IngressConfigurableACL1SmallTable_counter_width (6)
#define IngressConfigurableACL1SmallTable_newEthType_width (2)
#define IngressConfigurableACL1SmallTable_newL4Value_width (16)
#define IngressConfigurableACL1SmallTable_color_width (2)
#define IngressConfigurableACL1SmallTable_updateVid_width (1)
#define IngressConfigurableACL1SmallTable_forceColorPrio_width (1)
#define IngressConfigurableACL1SmallTable_forceSendToCpuOrigPkt_width (1)
#define IngressConfigurableACL1SmallTable_forceVid_width (12)
#define IngressConfigurableACL1SmallTable_tunnelEntryPtr_width (4)
#define IngressConfigurableACL1SmallTable_enableUpdateIp_width (1)
#define IngressConfigurableACL1SmallTable_mmpValid_width (1)
#define IngressConfigurableACL1SmallTable_sendToPort_width (1)
#define IngressConfigurableACL1SmallTable_ptp_width (1)
#define IngressConfigurableACL1SmallTable_newPcpValue_width (3)
#define IngressConfigurableACL1SmallTable_cfiDeiPrio_width (1)
#define IngressConfigurableACL1SmallTable_eQueue_width (3)
#define IngressConfigurableACL1SmallTable_newIpValue_width (32)
#define IngressConfigurableACL1SmallTable_updateL4SpOrDp_width (1)
#define IngressConfigurableACL1SmallTable_forceColor_width (1)
#define IngressConfigurableACL1SmallTable_natOpPrio_width (1)
#define IngressConfigurableACL1SmallTable_enableUpdateL4_width (1)
#define IngressConfigurableACL1SmallTable_forceQueuePrio_width (1)
#define IngressConfigurableACL1SmallTable_tunnelEntryPrio_width (1)
#define IngressConfigurableACL1SmallTable_updateTosExp_width (1)
#define IngressConfigurableACL1SmallTable_valid_width (1)
#define IngressConfigurableACL1SmallTable_natOpValid_width (1)
#define IngressConfigurableACL1SmallTable_natOpPtr_width (11)
#define IngressConfigurableACL1SmallTable_metaDataPrio_width (1)
#define IngressConfigurableACL1SmallTable_pcpPrio_width (1)
#define IngressConfigurableACL1SmallTable_metaData_width (16)
#define IngressConfigurableACL1SmallTable_forceQueue_width (1)
#define IngressConfigurableACL1SmallTable_tunnelEntry_width (1)
#define IngressConfigurableACL1SmallTable_vidPrio_width (1)
#define IngressConfigurableACL1SmallTable_compareData_width (135)
#define IngressConfigurableACL1SmallTable_imPrio_width (1)
#define IngressConfigurableACL1SmallTable_ethPrio_width (1)
#define IngressConfigurableACL1SmallTable_sendToCpu_width (1)
#define IngressConfigurableACL1SmallTable_updateSaOrDa_width (1)
#define IngressConfigurableACL1SmallTable_mmpOrder_width (2)
#define IngressConfigurableACL1SmallTable_destPort_width (3)
#define IngressConfigurableACL1SmallTable_updateCounter_width (1)
#define IngressConfigurableACL1SmallTable_newVidValue_width (12)
#define IngressConfigurableACL1SmallTable_inputMirror_width (1)
#define IngressConfigurableACL1SmallTable_forceVidPrio_width (1)
#define IngressConfigurableACL1SmallTable_forceVidValid_width (1)
#define IngressConfigurableACL1SmallTable_destInputMirror_width (3)
#define IngressConfigurableACL1SmallTable_metaDataValid_width (1)
#define IngressConfigurableACL1SmallTable_dropEnable_width (1)
#define IngressConfigurableACL1SmallTable_updateEType_width (1)
#define IngressConfigurableACL1SmallTable_newTosExp_width (8)
#define IngressConfigurableACL1SmallTable_updateCfiDei_width (1)
#define IngressConfigurableACL1SmallTable_tosMask_width (8)
#define IngressConfigurableACL1SmallTable_tunnelEntryUcMc_width (1)
#define IngressConfigurableACL1SmallTable_noLearning_width (1)
#define IngressConfigurableACL1SmallTable_updatePcp_width (1)
#define IngressConfigurableACL1SmallTable_mmpPtr_width (5)
#define IngressConfigurableACL1SmallTable_newCfiDeiValue_width (1)

//---- constants for register IngressVIDEthernetTypeRangeSearchData -----
#define IngressVIDEthernetTypeRangeSearchData_nr_entries (4)
// bit width of each register field:
#define IngressVIDEthernetTypeRangeSearchData_start_width (16)
#define IngressVIDEthernetTypeRangeSearchData_end_width (16)
#define IngressVIDEthernetTypeRangeSearchData_ports_width (6)

//---- constants for register L2FloodingStormControlRateConfiguration -----
#define L2FloodingStormControlRateConfiguration_nr_entries (6)
// bit width of each register field:
#define L2FloodingStormControlRateConfiguration_tokens_width (12)
#define L2FloodingStormControlRateConfiguration_packetsNotBytes_width (1)
#define L2FloodingStormControlRateConfiguration_tick_width (3)
#define L2FloodingStormControlRateConfiguration_ifgCorrection_width (8)

//---- constants for register IngressConfigurableACL1LargeTable -----
#define IngressConfigurableACL1LargeTable_nr_entries (128)
// bit width of each register field:
#define IngressConfigurableACL1LargeTable_counter_width (6)
#define IngressConfigurableACL1LargeTable_newEthType_width (2)
#define IngressConfigurableACL1LargeTable_newL4Value_width (16)
#define IngressConfigurableACL1LargeTable_color_width (2)
#define IngressConfigurableACL1LargeTable_updateVid_width (1)
#define IngressConfigurableACL1LargeTable_forceColorPrio_width (1)
#define IngressConfigurableACL1LargeTable_forceSendToCpuOrigPkt_width (1)
#define IngressConfigurableACL1LargeTable_forceVid_width (12)
#define IngressConfigurableACL1LargeTable_tunnelEntryPtr_width (4)
#define IngressConfigurableACL1LargeTable_enableUpdateIp_width (1)
#define IngressConfigurableACL1LargeTable_mmpValid_width (1)
#define IngressConfigurableACL1LargeTable_sendToPort_width (1)
#define IngressConfigurableACL1LargeTable_ptp_width (1)
#define IngressConfigurableACL1LargeTable_newPcpValue_width (3)
#define IngressConfigurableACL1LargeTable_cfiDeiPrio_width (1)
#define IngressConfigurableACL1LargeTable_eQueue_width (3)
#define IngressConfigurableACL1LargeTable_newIpValue_width (32)
#define IngressConfigurableACL1LargeTable_updateL4SpOrDp_width (1)
#define IngressConfigurableACL1LargeTable_forceColor_width (1)
#define IngressConfigurableACL1LargeTable_natOpPrio_width (1)
#define IngressConfigurableACL1LargeTable_enableUpdateL4_width (1)
#define IngressConfigurableACL1LargeTable_forceQueuePrio_width (1)
#define IngressConfigurableACL1LargeTable_tunnelEntryPrio_width (1)
#define IngressConfigurableACL1LargeTable_updateTosExp_width (1)
#define IngressConfigurableACL1LargeTable_valid_width (1)
#define IngressConfigurableACL1LargeTable_natOpValid_width (1)
#define IngressConfigurableACL1LargeTable_natOpPtr_width (11)
#define IngressConfigurableACL1LargeTable_metaDataPrio_width (1)
#define IngressConfigurableACL1LargeTable_pcpPrio_width (1)
#define IngressConfigurableACL1LargeTable_metaData_width (16)
#define IngressConfigurableACL1LargeTable_forceQueue_width (1)
#define IngressConfigurableACL1LargeTable_tunnelEntry_width (1)
#define IngressConfigurableACL1LargeTable_vidPrio_width (1)
#define IngressConfigurableACL1LargeTable_compareData_width (135)
#define IngressConfigurableACL1LargeTable_imPrio_width (1)
#define IngressConfigurableACL1LargeTable_ethPrio_width (1)
#define IngressConfigurableACL1LargeTable_sendToCpu_width (1)
#define IngressConfigurableACL1LargeTable_updateSaOrDa_width (1)
#define IngressConfigurableACL1LargeTable_mmpOrder_width (2)
#define IngressConfigurableACL1LargeTable_destPort_width (3)
#define IngressConfigurableACL1LargeTable_updateCounter_width (1)
#define IngressConfigurableACL1LargeTable_newVidValue_width (12)
#define IngressConfigurableACL1LargeTable_inputMirror_width (1)
#define IngressConfigurableACL1LargeTable_forceVidPrio_width (1)
#define IngressConfigurableACL1LargeTable_forceVidValid_width (1)
#define IngressConfigurableACL1LargeTable_destInputMirror_width (3)
#define IngressConfigurableACL1LargeTable_metaDataValid_width (1)
#define IngressConfigurableACL1LargeTable_dropEnable_width (1)
#define IngressConfigurableACL1LargeTable_updateEType_width (1)
#define IngressConfigurableACL1LargeTable_newTosExp_width (8)
#define IngressConfigurableACL1LargeTable_updateCfiDei_width (1)
#define IngressConfigurableACL1LargeTable_tosMask_width (8)
#define IngressConfigurableACL1LargeTable_tunnelEntryUcMc_width (1)
#define IngressConfigurableACL1LargeTable_noLearning_width (1)
#define IngressConfigurableACL1LargeTable_updatePcp_width (1)
#define IngressConfigurableACL1LargeTable_mmpPtr_width (5)
#define IngressConfigurableACL1LargeTable_newCfiDeiValue_width (1)

//---- constants for register PrioShaperRateConfiguration -----
#define PrioShaperRateConfiguration_nr_entries (48)
// bit width of each register field:
#define PrioShaperRateConfiguration_tokens_width (13)
#define PrioShaperRateConfiguration_packetsNotBytes_width (1)
#define PrioShaperRateConfiguration_tick_width (3)
#define PrioShaperRateConfiguration_ifgCorrection_width (8)

//---- constants for register IPMulticastRoutedCounter -----
#define IPMulticastRoutedCounter_nr_entries (6)
// bit width of each register field:
#define IPMulticastRoutedCounter_packets_width (32)

//---- constants for register PFCIncCountersforingressports0to5 -----
#define PFCIncCountersforingressports0to5_nr_entries (48)
// bit width of each register field:
#define PFCIncCountersforingressports0to5_cells_width (10)

//---- constants for register L2FloodingStormControlBucketCapacityConfiguration -----
#define L2FloodingStormControlBucketCapacityConfiguration_nr_entries (6)
// bit width of each register field:
#define L2FloodingStormControlBucketCapacityConfiguration_bucketCapacity_width (16)

//---- constants for register IPv6ClassofServiceFieldToPacketColorMappingTable -----
#define IPv6ClassofServiceFieldToPacketColorMappingTable_nr_entries (256)
// bit width of each register field:
#define IPv6ClassofServiceFieldToPacketColorMappingTable_color_width (2)

//---- constants for register L2DestinationTable -----
#define L2DestinationTable_nr_entries (4128)
// bit width of each register field:
#define L2DestinationTable_l2ActionTableDaStatus_width (1)
#define L2DestinationTable_destPortormcAddr_width (6)
#define L2DestinationTable_pktDrop_width (1)
#define L2DestinationTable_l2ActionTableSaStatus_width (1)
#define L2DestinationTable_uc_width (1)
#define L2DestinationTable_metaData_width (16)

//---- constants for register EgressMultipleSpanningTreeState -----
#define EgressMultipleSpanningTreeState_nr_entries (16)
// bit width of each register field:
#define EgressMultipleSpanningTreeState_portSptState_width (12)

//---- constants for register PortTCXonTotalThreshold -----
#define PortTCXonTotalThreshold_nr_entries (48)
// bit width of each register field:
#define PortTCXonTotalThreshold_cells_width (10)

//---- constants for register HardwareLearningCounter -----
#define HardwareLearningCounter_nr_entries (6)
// bit width of each register field:
#define HardwareLearningCounter_cnt_width (13)

//---- constants for register L2ReservedMulticastAddressAction -----
#define L2ReservedMulticastAddressAction_nr_entries (256)
// bit width of each register field:
#define L2ReservedMulticastAddressAction_dropMask_width (6)
#define L2ReservedMulticastAddressAction_sendToCpuMask_width (6)

//---- constants for register L2BroadcastStormControlBucketThresholdConfiguration -----
#define L2BroadcastStormControlBucketThresholdConfiguration_nr_entries (6)
// bit width of each register field:
#define L2BroadcastStormControlBucketThresholdConfiguration_threshold_width (16)

//---- constants for register BeginningofPacketTunnelEntryInstructionTable -----
#define BeginningofPacketTunnelEntryInstructionTable_nr_entries (16)
// bit width of each register field:
#define BeginningofPacketTunnelEntryInstructionTable_l3Type_width (2)
#define BeginningofPacketTunnelEntryInstructionTable_ipHeaderOffset_width (6)
#define BeginningofPacketTunnelEntryInstructionTable_hasUdp_width (1)

//---- constants for register TOSQoSMappingTable -----
#define TOSQoSMappingTable_nr_entries (512)
// bit width of each register field:
#define TOSQoSMappingTable_updateExp_width (1)
#define TOSQoSMappingTable_updatePcp_width (1)
#define TOSQoSMappingTable_pcp_width (3)
#define TOSQoSMappingTable_updateCfiDei_width (1)
#define TOSQoSMappingTable_newTos_width (8)
#define TOSQoSMappingTable_cfiDei_width (1)
#define TOSQoSMappingTable_newExp_width (3)

//---- constants for register L2LookupCollisionTableMasks -----
#define L2LookupCollisionTableMasks_nr_entries (4)
// bit width of each register field:
#define L2LookupCollisionTableMasks_gid_width (12)
#define L2LookupCollisionTableMasks_macAddr_width (48)

//---- constants for register IngressConfigurableACL2RulesSetup -----
#define IngressConfigurableACL2RulesSetup_nr_entries (4)
// bit width of each register field:
#define IngressConfigurableACL2RulesSetup_fieldSelectBitmask_width (28)

//---- constants for register UnknownEgressDrop -----
#define UnknownEgressDrop_nr_entries (6)
// bit width of each register field:
#define UnknownEgressDrop_packets_width (32)

//---- constants for register EgressNATHitStatus -----
#define EgressNATHitStatus_nr_entries (1024)
// bit width of each register field:
#define EgressNATHitStatus_hit_width (1)

//---- constants for register PFCDecCountersforingressports0to5 -----
#define PFCDecCountersforingressports0to5_nr_entries (48)
// bit width of each register field:
#define PFCDecCountersforingressports0to5_cells_width (10)

//---- constants for register DebugIPPCounter -----
#define DebugIPPCounter_nr_entries (23)
// bit width of each register field:
#define DebugIPPCounter_packets_width (16)

//---- constants for register PrioShaperBucketThresholdConfiguration -----
#define PrioShaperBucketThresholdConfiguration_nr_entries (48)
// bit width of each register field:
#define PrioShaperBucketThresholdConfiguration_threshold_width (17)

//---- constants for register PortTailDropFFAThreshold -----
#define PortTailDropFFAThreshold_nr_entries (6)
// bit width of each register field:
#define PortTailDropFFAThreshold_cells_width (10)
#define PortTailDropFFAThreshold_enable_width (1)
#define PortTailDropFFAThreshold_trip_width (1)

//---- constants for register MPLSQoSMappingTable -----
#define MPLSQoSMappingTable_nr_entries (512)
// bit width of each register field:
#define MPLSQoSMappingTable_cfiDei_width (1)
#define MPLSQoSMappingTable_updatePcp_width (1)
#define MPLSQoSMappingTable_pcp_width (3)
#define MPLSQoSMappingTable_updateCfiDei_width (1)
#define MPLSQoSMappingTable_exp_width (3)

//---- constants for register TunnelExitTooSmallPacketModificationToSmallDrop -----
#define TunnelExitTooSmallPacketModificationToSmallDrop_nr_entries (6)
// bit width of each register field:
#define TunnelExitTooSmallPacketModificationToSmallDrop_packets_width (32)

//---- constants for register IPv6ClassofServiceFieldToEgressQueueMappingTable -----
#define IPv6ClassofServiceFieldToEgressQueueMappingTable_nr_entries (256)
// bit width of each register field:
#define IPv6ClassofServiceFieldToEgressQueueMappingTable_pQueue_width (3)

//---- constants for register QueueShaperRateConfiguration -----
#define QueueShaperRateConfiguration_nr_entries (48)
// bit width of each register field:
#define QueueShaperRateConfiguration_tokens_width (13)
#define QueueShaperRateConfiguration_packetsNotBytes_width (1)
#define QueueShaperRateConfiguration_tick_width (3)
#define QueueShaperRateConfiguration_ifgCorrection_width (8)

//---- constants for register EgressQueueDepth -----
#define EgressQueueDepth_nr_entries (48)
// bit width of each register field:
#define EgressQueueDepth_packets_width (10)

//---- constants for register IngressConfigurableACL2TCAMAnswer -----
#define IngressConfigurableACL2TCAMAnswer_nr_entries (24)
// bit width of each register field:
#define IngressConfigurableACL2TCAMAnswer_counter_width (6)
#define IngressConfigurableACL2TCAMAnswer_newEthType_width (2)
#define IngressConfigurableACL2TCAMAnswer_newL4Value_width (16)
#define IngressConfigurableACL2TCAMAnswer_color_width (2)
#define IngressConfigurableACL2TCAMAnswer_updateVid_width (1)
#define IngressConfigurableACL2TCAMAnswer_forceColorPrio_width (1)
#define IngressConfigurableACL2TCAMAnswer_forceSendToCpuOrigPkt_width (1)
#define IngressConfigurableACL2TCAMAnswer_forceVid_width (12)
#define IngressConfigurableACL2TCAMAnswer_tunnelEntryPtr_width (4)
#define IngressConfigurableACL2TCAMAnswer_enableUpdateIp_width (1)
#define IngressConfigurableACL2TCAMAnswer_mmpValid_width (1)
#define IngressConfigurableACL2TCAMAnswer_sendToPort_width (1)
#define IngressConfigurableACL2TCAMAnswer_ptp_width (1)
#define IngressConfigurableACL2TCAMAnswer_newPcpValue_width (3)
#define IngressConfigurableACL2TCAMAnswer_cfiDeiPrio_width (1)
#define IngressConfigurableACL2TCAMAnswer_eQueue_width (3)
#define IngressConfigurableACL2TCAMAnswer_newIpValue_width (32)
#define IngressConfigurableACL2TCAMAnswer_updateL4SpOrDp_width (1)
#define IngressConfigurableACL2TCAMAnswer_forceColor_width (1)
#define IngressConfigurableACL2TCAMAnswer_natOpPrio_width (1)
#define IngressConfigurableACL2TCAMAnswer_enableUpdateL4_width (1)
#define IngressConfigurableACL2TCAMAnswer_forceQueuePrio_width (1)
#define IngressConfigurableACL2TCAMAnswer_tunnelEntryPrio_width (1)
#define IngressConfigurableACL2TCAMAnswer_updateTosExp_width (1)
#define IngressConfigurableACL2TCAMAnswer_natOpValid_width (1)
#define IngressConfigurableACL2TCAMAnswer_natOpPtr_width (11)
#define IngressConfigurableACL2TCAMAnswer_metaDataPrio_width (1)
#define IngressConfigurableACL2TCAMAnswer_pcpPrio_width (1)
#define IngressConfigurableACL2TCAMAnswer_metaData_width (16)
#define IngressConfigurableACL2TCAMAnswer_forceQueue_width (1)
#define IngressConfigurableACL2TCAMAnswer_tunnelEntry_width (1)
#define IngressConfigurableACL2TCAMAnswer_vidPrio_width (1)
#define IngressConfigurableACL2TCAMAnswer_imPrio_width (1)
#define IngressConfigurableACL2TCAMAnswer_ethPrio_width (1)
#define IngressConfigurableACL2TCAMAnswer_sendToCpu_width (1)
#define IngressConfigurableACL2TCAMAnswer_updateSaOrDa_width (1)
#define IngressConfigurableACL2TCAMAnswer_mmpOrder_width (2)
#define IngressConfigurableACL2TCAMAnswer_destPort_width (3)
#define IngressConfigurableACL2TCAMAnswer_updateCounter_width (1)
#define IngressConfigurableACL2TCAMAnswer_newVidValue_width (12)
#define IngressConfigurableACL2TCAMAnswer_inputMirror_width (1)
#define IngressConfigurableACL2TCAMAnswer_forceVidPrio_width (1)
#define IngressConfigurableACL2TCAMAnswer_forceVidValid_width (1)
#define IngressConfigurableACL2TCAMAnswer_destInputMirror_width (3)
#define IngressConfigurableACL2TCAMAnswer_metaDataValid_width (1)
#define IngressConfigurableACL2TCAMAnswer_dropEnable_width (1)
#define IngressConfigurableACL2TCAMAnswer_updateEType_width (1)
#define IngressConfigurableACL2TCAMAnswer_newTosExp_width (8)
#define IngressConfigurableACL2TCAMAnswer_updateCfiDei_width (1)
#define IngressConfigurableACL2TCAMAnswer_tosMask_width (8)
#define IngressConfigurableACL2TCAMAnswer_tunnelEntryUcMc_width (1)
#define IngressConfigurableACL2TCAMAnswer_noLearning_width (1)
#define IngressConfigurableACL2TCAMAnswer_updatePcp_width (1)
#define IngressConfigurableACL2TCAMAnswer_mmpPtr_width (5)
#define IngressConfigurableACL2TCAMAnswer_newCfiDeiValue_width (1)

//---- constants for register DefaultPacketToCPUModification -----
#define DefaultPacketToCPUModification_nr_entries (6)
// bit width of each register field:
#define DefaultPacketToCPUModification_origCpuPkt_width (1)

//---- constants for register RouterPortMACAddress -----
#define RouterPortMACAddress_nr_entries (16)
// bit width of each register field:
#define RouterPortMACAddress_macAddress_width (48)
#define RouterPortMACAddress_selectMacEntryPortMask_width (6)
#define RouterPortMACAddress_macMask_width (48)
#define RouterPortMACAddress_valid_width (1)
#define RouterPortMACAddress_vrf_width (2)
#define RouterPortMACAddress_altMacAddress_width (48)

//---- constants for register EgressConfigurableACL0TCAM -----
#define EgressConfigurableACL0TCAM_nr_entries (16)
// bit width of each register field:
#define EgressConfigurableACL0TCAM_valid_width (1)
#define EgressConfigurableACL0TCAM_mask_width (135)
#define EgressConfigurableACL0TCAM_compareData_width (135)

//---- constants for register IngressConfigurableACL2TCAM -----
#define IngressConfigurableACL2TCAM_nr_entries (24)
// bit width of each register field:
#define IngressConfigurableACL2TCAM_valid_width (1)
#define IngressConfigurableACL2TCAM_mask_width (540)
#define IngressConfigurableACL2TCAM_compareData_width (540)

//---- constants for register EgressACLRulePointerTCAM -----
#define EgressACLRulePointerTCAM_nr_entries (64)
// bit width of each register field:
#define EgressACLRulePointerTCAM_ucSwitchedmask_width (1)
#define EgressACLRulePointerTCAM_vrfmask_width (2)
#define EgressACLRulePointerTCAM_destPortMask_width (6)
#define EgressACLRulePointerTCAM_vid_width (12)
#define EgressACLRulePointerTCAM_ucSwitched_width (1)
#define EgressACLRulePointerTCAM_flooded_width (1)
#define EgressACLRulePointerTCAM_l3Type_width (2)
#define EgressACLRulePointerTCAM_srcPort_width (3)
#define EgressACLRulePointerTCAM_l4Type_width (3)
#define EgressACLRulePointerTCAM_routed_width (1)
#define EgressACLRulePointerTCAM_floodedmask_width (1)
#define EgressACLRulePointerTCAM_l3Typemask_width (2)
#define EgressACLRulePointerTCAM_valid_width (1)
#define EgressACLRulePointerTCAM_srcPortmask_width (3)
#define EgressACLRulePointerTCAM_vrf_width (2)
#define EgressACLRulePointerTCAM_mcSwitched_width (1)
#define EgressACLRulePointerTCAM_mcSwitchedmask_width (1)
#define EgressACLRulePointerTCAM_l4Typemask_width (3)
#define EgressACLRulePointerTCAM_routedmask_width (1)
#define EgressACLRulePointerTCAM_vidmask_width (12)
#define EgressACLRulePointerTCAM_destPortMaskmask_width (6)

//---- constants for register EgressQueueToPCPAndCFIDEIMappingTable -----
#define EgressQueueToPCPAndCFIDEIMappingTable_nr_entries (8)
// bit width of each register field:
#define EgressQueueToPCPAndCFIDEIMappingTable_cfiDei_width (1)
#define EgressQueueToPCPAndCFIDEIMappingTable_pcp_width (3)

//---- constants for register L2AgingCollisionShadowTable -----
#define L2AgingCollisionShadowTable_nr_entries (32)
// bit width of each register field:
#define L2AgingCollisionShadowTable_valid_width (1)

//---- constants for register RouterEgressQueueToVLANData -----
#define RouterEgressQueueToVLANData_nr_entries (8)
// bit width of each register field:
#define RouterEgressQueueToVLANData_cfiDei_width (1)
#define RouterEgressQueueToVLANData_pcp_width (3)

//---- constants for register LinkAggregationMembership -----
#define LinkAggregationMembership_nr_entries (6)
// bit width of each register field:
#define LinkAggregationMembership_la_width (3)

//---- constants for register IngressConfigurableACL3RulesSetup -----
#define IngressConfigurableACL3RulesSetup_nr_entries (4)
// bit width of each register field:
#define IngressConfigurableACL3RulesSetup_fieldSelectBitmask_width (10)

//---- constants for register L2ActionTable -----
#define L2ActionTable_nr_entries (128)
// bit width of each register field:
#define L2ActionTable_useSpecialAllow_width (1)
#define L2ActionTable_dropPortMove_width (1)
#define L2ActionTable_sendToCpu_width (1)
#define L2ActionTable_drop_width (1)
#define L2ActionTable_noPortMove_width (1)
#define L2ActionTable_mmpOrder_width (2)
#define L2ActionTable_forceSendToCpuOrigPkt_width (1)
#define L2ActionTable_noLearningUc_width (1)
#define L2ActionTable_dropAll_width (1)
#define L2ActionTable_allowPtr_width (2)
#define L2ActionTable_mmpPtr_width (5)
#define L2ActionTable_mmpValid_width (1)
#define L2ActionTable_noLearningMc_width (1)

//---- constants for register TunnelExitLookupTCAM -----
#define TunnelExitLookupTCAM_nr_entries (16)
// bit width of each register field:
#define TunnelExitLookupTCAM_l4Protocolmask_width (8)
#define TunnelExitLookupTCAM_ethType_width (16)
#define TunnelExitLookupTCAM_ethTypemask_width (16)
#define TunnelExitLookupTCAM_valid_width (1)
#define TunnelExitLookupTCAM_l4Dp_width (16)
#define TunnelExitLookupTCAM_l3Type_width (3)
#define TunnelExitLookupTCAM_ipDa_width (128)
#define TunnelExitLookupTCAM_ipSamask_width (128)
#define TunnelExitLookupTCAM_snapLlc_width (1)
#define TunnelExitLookupTCAM_l4Type_width (2)
#define TunnelExitLookupTCAM_l4Spmask_width (16)
#define TunnelExitLookupTCAM_snapLlcmask_width (1)
#define TunnelExitLookupTCAM_l3Typemask_width (3)
#define TunnelExitLookupTCAM_ipSa_width (128)
#define TunnelExitLookupTCAM_l4Dpmask_width (16)
#define TunnelExitLookupTCAM_ipDamask_width (128)
#define TunnelExitLookupTCAM_l4Protocol_width (8)
#define TunnelExitLookupTCAM_l4Typemask_width (2)
#define TunnelExitLookupTCAM_l4Sp_width (16)

//---- constants for register IngressConfigurableACLMatchCounter -----
#define IngressConfigurableACLMatchCounter_nr_entries (64)
// bit width of each register field:
#define IngressConfigurableACLMatchCounter_packets_width (32)

//---- constants for register OutputMirroringTable -----
#define OutputMirroringTable_nr_entries (6)
// bit width of each register field:
#define OutputMirroringTable_outputMirrorEnabled_width (1)
#define OutputMirroringTable_outputMirrorPort_width (3)

//---- constants for register PortUsed -----
#define PortUsed_nr_entries (6)
// bit width of each register field:
#define PortUsed_cells_width (10)

//---- constants for register SecondTunnelExitMissAction -----
#define SecondTunnelExitMissAction_nr_entries (4)
// bit width of each register field:
#define SecondTunnelExitMissAction_dropIfMiss_width (1)

//---- constants for register IngressConfigurableACL0SmallTable -----
#define IngressConfigurableACL0SmallTable_nr_entries (256)
// bit width of each register field:
#define IngressConfigurableACL0SmallTable_newL4Value_width (16)
#define IngressConfigurableACL0SmallTable_color_width (2)
#define IngressConfigurableACL0SmallTable_metaDataPrio_width (1)
#define IngressConfigurableACL0SmallTable_forceColorPrio_width (1)
#define IngressConfigurableACL0SmallTable_enableUpdateIp_width (1)
#define IngressConfigurableACL0SmallTable_mmpValid_width (1)
#define IngressConfigurableACL0SmallTable_sendToPort_width (1)
#define IngressConfigurableACL0SmallTable_metaData_width (16)
#define IngressConfigurableACL0SmallTable_eQueue_width (3)
#define IngressConfigurableACL0SmallTable_newIpValue_width (32)
#define IngressConfigurableACL0SmallTable_updateL4SpOrDp_width (1)
#define IngressConfigurableACL0SmallTable_natOpValid_width (1)
#define IngressConfigurableACL0SmallTable_natOpPrio_width (1)
#define IngressConfigurableACL0SmallTable_enableUpdateL4_width (1)
#define IngressConfigurableACL0SmallTable_forceQueuePrio_width (1)
#define IngressConfigurableACL0SmallTable_valid_width (1)
#define IngressConfigurableACL0SmallTable_natOpPtr_width (11)
#define IngressConfigurableACL0SmallTable_updateCounter_width (1)
#define IngressConfigurableACL0SmallTable_forceQueue_width (1)
#define IngressConfigurableACL0SmallTable_tosMask_width (8)
#define IngressConfigurableACL0SmallTable_compareData_width (330)
#define IngressConfigurableACL0SmallTable_imPrio_width (1)
#define IngressConfigurableACL0SmallTable_updateTosExp_width (1)
#define IngressConfigurableACL0SmallTable_sendToCpu_width (1)
#define IngressConfigurableACL0SmallTable_updateSaOrDa_width (1)
#define IngressConfigurableACL0SmallTable_mmpOrder_width (2)
#define IngressConfigurableACL0SmallTable_inputMirror_width (1)
#define IngressConfigurableACL0SmallTable_forceColor_width (1)
#define IngressConfigurableACL0SmallTable_destInputMirror_width (3)
#define IngressConfigurableACL0SmallTable_metaDataValid_width (1)
#define IngressConfigurableACL0SmallTable_dropEnable_width (1)
#define IngressConfigurableACL0SmallTable_counter_width (6)
#define IngressConfigurableACL0SmallTable_newTosExp_width (8)
#define IngressConfigurableACL0SmallTable_forceSendToCpuOrigPkt_width (1)
#define IngressConfigurableACL0SmallTable_mmpPtr_width (5)
#define IngressConfigurableACL0SmallTable_destPort_width (3)

//---- constants for register PortTCReserved -----
#define PortTCReserved_nr_entries (48)
// bit width of each register field:
#define PortTCReserved_cells_width (10)

//---- constants for register L2QoSMappingTable -----
#define L2QoSMappingTable_nr_entries (64)
// bit width of each register field:
#define L2QoSMappingTable_cfiDei_width (1)
#define L2QoSMappingTable_updatePcp_width (1)
#define L2QoSMappingTable_pcp_width (3)
#define L2QoSMappingTable_updateCfiDei_width (1)

//---- constants for register EgressTunnelExitTable -----
#define EgressTunnelExitTable_nr_entries (16)
// bit width of each register field:
#define EgressTunnelExitTable_l4Protocol_width (8)
#define EgressTunnelExitTable_howManyBytesToRemove_width (8)
#define EgressTunnelExitTable_updateL4Protocol_width (1)
#define EgressTunnelExitTable_updateEthType_width (1)
#define EgressTunnelExitTable_ethType_width (16)
#define EgressTunnelExitTable_whereToRemove_width (2)
#define EgressTunnelExitTable_removeVlan_width (1)

//---- constants for register MACInterfaceCountersForRX -----
#define MACInterfaceCountersForRX_nr_entries (6)
// bit width of each register field:
#define MACInterfaceCountersForRX_packets_width (32)
#define MACInterfaceCountersForRX_error_width (32)

//---- constants for register NextHopTable -----
#define NextHopTable_nr_entries (1024)
// bit width of each register field:
#define NextHopTable_nextHopPacketMod_width (10)
#define NextHopTable_l2Uc_width (1)
#define NextHopTable_validEntry_width (1)
#define NextHopTable_srv6Sid_width (1)
#define NextHopTable_destPortormcAddr_width (6)
#define NextHopTable_tunnelExit_width (1)
#define NextHopTable_pktDrop_width (1)
#define NextHopTable_tunnelExitPtr_width (4)
#define NextHopTable_tunnelEntryPtr_width (4)
#define NextHopTable_sendToCpu_width (1)
#define NextHopTable_tunnelEntry_width (1)
#define NextHopTable_metaData_width (16)

//---- constants for register EgressACLRulePointerTCAMAnswer -----
#define EgressACLRulePointerTCAMAnswer_nr_entries (64)
// bit width of each register field:
#define EgressACLRulePointerTCAMAnswer_rulePtr0_width (3)
#define EgressACLRulePointerTCAMAnswer_rulePtr1_width (2)

//---- constants for register SourcePortDefaultACLAction -----
#define SourcePortDefaultACLAction_nr_entries (6)
// bit width of each register field:
#define SourcePortDefaultACLAction_counter_width (6)
#define SourcePortDefaultACLAction_newEthType_width (2)
#define SourcePortDefaultACLAction_newL4Value_width (16)
#define SourcePortDefaultACLAction_color_width (2)
#define SourcePortDefaultACLAction_updateVid_width (1)
#define SourcePortDefaultACLAction_tunnelEntryPtr_width (4)
#define SourcePortDefaultACLAction_enableUpdateIp_width (1)
#define SourcePortDefaultACLAction_mmpValid_width (1)
#define SourcePortDefaultACLAction_sendToPort_width (1)
#define SourcePortDefaultACLAction_ptp_width (1)
#define SourcePortDefaultACLAction_newPcpValue_width (3)
#define SourcePortDefaultACLAction_eQueue_width (3)
#define SourcePortDefaultACLAction_newCfiDeiValue_width (1)
#define SourcePortDefaultACLAction_newIpValue_width (32)
#define SourcePortDefaultACLAction_updateL4SpOrDp_width (1)
#define SourcePortDefaultACLAction_sendToCpu_width (1)
#define SourcePortDefaultACLAction_enableUpdateL4_width (1)
#define SourcePortDefaultACLAction_forceVid_width (12)
#define SourcePortDefaultACLAction_natOpValid_width (1)
#define SourcePortDefaultACLAction_natOpPtr_width (11)
#define SourcePortDefaultACLAction_updateCounter_width (1)
#define SourcePortDefaultACLAction_metaData_width (16)
#define SourcePortDefaultACLAction_forceQueue_width (1)
#define SourcePortDefaultACLAction_tunnelEntry_width (1)
#define SourcePortDefaultACLAction_updateTosExp_width (1)
#define SourcePortDefaultACLAction_updateSaOrDa_width (1)
#define SourcePortDefaultACLAction_mmpOrder_width (2)
#define SourcePortDefaultACLAction_newVidValue_width (12)
#define SourcePortDefaultACLAction_inputMirror_width (1)
#define SourcePortDefaultACLAction_forceColor_width (1)
#define SourcePortDefaultACLAction_forceVidValid_width (1)
#define SourcePortDefaultACLAction_destInputMirror_width (3)
#define SourcePortDefaultACLAction_metaDataValid_width (1)
#define SourcePortDefaultACLAction_dropEnable_width (1)
#define SourcePortDefaultACLAction_updateEType_width (1)
#define SourcePortDefaultACLAction_newTosExp_width (8)
#define SourcePortDefaultACLAction_updateCfiDei_width (1)
#define SourcePortDefaultACLAction_forceSendToCpuOrigPkt_width (1)
#define SourcePortDefaultACLAction_tunnelEntryUcMc_width (1)
#define SourcePortDefaultACLAction_noLearning_width (1)
#define SourcePortDefaultACLAction_updatePcp_width (1)
#define SourcePortDefaultACLAction_mmpPtr_width (5)
#define SourcePortDefaultACLAction_tosMask_width (8)
#define SourcePortDefaultACLAction_destPort_width (3)

//---- constants for register VLANTable -----
#define VLANTable_nr_entries (4096)
// bit width of each register field:
#define VLANTable_vid_width (12)
#define VLANTable_typeSel_width (2)
#define VLANTable_cfiDeiIf_width (1)
#define VLANTable_cfiDei_width (1)
#define VLANTable_mmpValid_width (1)
#define VLANTable_vlanSingleOp_width (3)
#define VLANTable_msptPtr_width (4)
#define VLANTable_pcp_width (3)
#define VLANTable_gid_width (12)
#define VLANTable_pcpIf_width (3)
#define VLANTable_vidSel_width (2)
#define VLANTable_pcpSel_width (2)
#define VLANTable_vlanPortMask_width (6)
#define VLANTable_nrVlansVidOperationIf_width (12)
#define VLANTable_typeSelIf_width (2)
#define VLANTable_pcpSelIf_width (2)
#define VLANTable_cfiDeiSelIf_width (2)
#define VLANTable_mmpOrder_width (2)
#define VLANTable_sendIpMcToCpu_width (1)
#define VLANTable_cfiDeiSel_width (2)
#define VLANTable_allowRouting_width (1)
#define VLANTable_vlanSingleOpIf_width (3)
#define VLANTable_vidIf_width (12)
#define VLANTable_vidSelIf_width (2)
#define VLANTable_mmpPtr_width (5)

//---- constants for register IngressPortPacketTypeFilter -----
#define IngressPortPacketTypeFilter_nr_entries (6)
// bit width of each register field:
#define IngressPortPacketTypeFilter_dropUntaggedVlans_width (1)
#define IngressPortPacketTypeFilter_dropIPv4Packets_width (1)
#define IngressPortPacketTypeFilter_dropIPv4MulticastPackets_width (1)
#define IngressPortPacketTypeFilter_dropDualTaggedVlans_width (1)
#define IngressPortPacketTypeFilter_dropL2MulticastFrames_width (1)
#define IngressPortPacketTypeFilter_dropSStaggedVlans_width (1)
#define IngressPortPacketTypeFilter_moreThanOneVlans_width (1)
#define IngressPortPacketTypeFilter_dropSingleTaggedVlans_width (1)
#define IngressPortPacketTypeFilter_dropIPv6MulticastPackets_width (1)
#define IngressPortPacketTypeFilter_dropCCtaggedVlans_width (1)
#define IngressPortPacketTypeFilter_dropIPv6Packets_width (1)
#define IngressPortPacketTypeFilter_dropSCtaggedVlans_width (1)
#define IngressPortPacketTypeFilter_dropMPLSPackets_width (1)
#define IngressPortPacketTypeFilter_dropCtaggedVlans_width (1)
#define IngressPortPacketTypeFilter_dropCStaggedVlans_width (1)
#define IngressPortPacketTypeFilter_dropStaggedVlans_width (1)
#define IngressPortPacketTypeFilter_dropL2BroadcastFrames_width (1)

//---- constants for register PortFFAUsed -----
#define PortFFAUsed_nr_entries (6)
// bit width of each register field:
#define PortFFAUsed_cells_width (10)

//---- constants for register DefaultLearningStatus -----
#define DefaultLearningStatus_nr_entries (6)
// bit width of each register field:
#define DefaultLearningStatus_stat_width (1)
#define DefaultLearningStatus_valid_width (1)
#define DefaultLearningStatus_hit_width (1)
#define DefaultLearningStatus_learnLimit_width (13)

//---- constants for register IngressVIDEthernetTypeRangeAssignmentAnswer -----
#define IngressVIDEthernetTypeRangeAssignmentAnswer_nr_entries (4)
// bit width of each register field:
#define IngressVIDEthernetTypeRangeAssignmentAnswer_ingressVid_width (12)
#define IngressVIDEthernetTypeRangeAssignmentAnswer_order_width (2)

//---- constants for register L2MulticastStormControlBucketThresholdConfiguration -----
#define L2MulticastStormControlBucketThresholdConfiguration_nr_entries (6)
// bit width of each register field:
#define L2MulticastStormControlBucketThresholdConfiguration_threshold_width (16)

//---- constants for register IngressConfigurableACL3TCAM -----
#define IngressConfigurableACL3TCAM_nr_entries (16)
// bit width of each register field:
#define IngressConfigurableACL3TCAM_valid_width (1)
#define IngressConfigurableACL3TCAM_mask_width (80)
#define IngressConfigurableACL3TCAM_compareData_width (80)

//---- constants for register L2BroadcastStormControlBucketCapacityConfiguration -----
#define L2BroadcastStormControlBucketCapacityConfiguration_nr_entries (6)
// bit width of each register field:
#define L2BroadcastStormControlBucketCapacityConfiguration_bucketCapacity_width (16)

//---- constants for register NATActionTable -----
#define NATActionTable_nr_entries (512)
// bit width of each register field:
#define NATActionTable_action_width (2)

//---- constants for register IngressConfigurableACL1RulesSetup -----
#define IngressConfigurableACL1RulesSetup_nr_entries (8)
// bit width of each register field:
#define IngressConfigurableACL1RulesSetup_fieldSelectBitmask_width (33)

//---- constants for register VLANPCPAndDEIToColorMappingTable -----
#define VLANPCPAndDEIToColorMappingTable_nr_entries (16)
// bit width of each register field:
#define VLANPCPAndDEIToColorMappingTable_color_width (2)

//---- constants for register SMONByteCounter -----
#define SMONByteCounter_nr_sets (3)
#define SMONByteCounter_nr_entries (8)
// bit width of each register field:
#define SMONByteCounter_bytes_width (32)

//---- constants for register IngressEgressPortPacketTypeFilter -----
#define IngressEgressPortPacketTypeFilter_nr_entries (6)
// bit width of each register field:
#define IngressEgressPortPacketTypeFilter_dropUntaggedVlans_width (1)
#define IngressEgressPortPacketTypeFilter_dropIPv4Packets_width (1)
#define IngressEgressPortPacketTypeFilter_dropIPv4MulticastPackets_width (1)
#define IngressEgressPortPacketTypeFilter_dropL2MulticastFrames_width (1)
#define IngressEgressPortPacketTypeFilter_dropDualTaggedVlans_width (1)
#define IngressEgressPortPacketTypeFilter_dropCtaggedVlans_width (1)
#define IngressEgressPortPacketTypeFilter_dropSStaggedVlans_width (1)
#define IngressEgressPortPacketTypeFilter_moreThanOneVlans_width (1)
#define IngressEgressPortPacketTypeFilter_srcPortFilter_width (6)
#define IngressEgressPortPacketTypeFilter_dropSingleTaggedVlans_width (1)
#define IngressEgressPortPacketTypeFilter_dropIPv6MulticastPackets_width (1)
#define IngressEgressPortPacketTypeFilter_dropCCtaggedVlans_width (1)
#define IngressEgressPortPacketTypeFilter_dropSCtaggedVlans_width (1)
#define IngressEgressPortPacketTypeFilter_dropIPv6Packets_width (1)
#define IngressEgressPortPacketTypeFilter_dropRouted_width (1)
#define IngressEgressPortPacketTypeFilter_dropMPLSPackets_width (1)
#define IngressEgressPortPacketTypeFilter_dropL2FloodingFrames_width (1)
#define IngressEgressPortPacketTypeFilter_dropCStaggedVlans_width (1)
#define IngressEgressPortPacketTypeFilter_dropStaggedVlans_width (1)
#define IngressEgressPortPacketTypeFilter_dropL2BroadcastFrames_width (1)

//---- constants for register ColorRemapFromEgressPort -----
#define ColorRemapFromEgressPort_nr_entries (6)
// bit width of each register field:
#define ColorRemapFromEgressPort_color2Tos_width (24)
#define ColorRemapFromEgressPort_colorMode_width (2)
#define ColorRemapFromEgressPort_color2Dei_width (3)
#define ColorRemapFromEgressPort_tosMask_width (8)

//---- constants for register IngressNATOperation -----
#define IngressNATOperation_nr_entries (2048)
// bit width of each register field:
#define IngressNATOperation_replaceSrc_width (1)
#define IngressNATOperation_replaceL4Port_width (1)
#define IngressNATOperation_replaceIP_width (1)
#define IngressNATOperation_ipAddress_width (32)
#define IngressNATOperation_port_width (16)

//---- constants for register IPQoSMappingTable -----
#define IPQoSMappingTable_nr_entries (256)
// bit width of each register field:
#define IPQoSMappingTable_updateExp_width (1)
#define IPQoSMappingTable_ecnTos_width (2)
#define IPQoSMappingTable_updatePcp_width (1)
#define IPQoSMappingTable_pcp_width (3)
#define IPQoSMappingTable_updateCfiDei_width (1)
#define IPQoSMappingTable_cfiDei_width (1)
#define IPQoSMappingTable_newExp_width (3)

//---- constants for register IngressAdmissionControlTokenBucketConfiguration -----
#define IngressAdmissionControlTokenBucketConfiguration_nr_entries (32)
// bit width of each register field:
#define IngressAdmissionControlTokenBucketConfiguration_colorBlind_width (1)
#define IngressAdmissionControlTokenBucketConfiguration_tick0_width (3)
#define IngressAdmissionControlTokenBucketConfiguration_tick1_width (3)
#define IngressAdmissionControlTokenBucketConfiguration_bucketCapacity0_width (16)
#define IngressAdmissionControlTokenBucketConfiguration_tokenMode_width (2)
#define IngressAdmissionControlTokenBucketConfiguration_dropMask_width (3)
#define IngressAdmissionControlTokenBucketConfiguration_byteCorrection_width (8)
#define IngressAdmissionControlTokenBucketConfiguration_tokens0_width (12)
#define IngressAdmissionControlTokenBucketConfiguration_maxLength_width (15)
#define IngressAdmissionControlTokenBucketConfiguration_tokens1_width (12)
#define IngressAdmissionControlTokenBucketConfiguration_bucketCapacity1_width (16)
#define IngressAdmissionControlTokenBucketConfiguration_bucketMode_width (1)

//---- constants for register L2BroadcastStormControlRateConfiguration -----
#define L2BroadcastStormControlRateConfiguration_nr_entries (6)
// bit width of each register field:
#define L2BroadcastStormControlRateConfiguration_tokens_width (12)
#define L2BroadcastStormControlRateConfiguration_packetsNotBytes_width (1)
#define L2BroadcastStormControlRateConfiguration_tick_width (3)
#define L2BroadcastStormControlRateConfiguration_ifgCorrection_width (8)

//---- constants for register PortTCXoffTotalThreshold -----
#define PortTCXoffTotalThreshold_nr_entries (48)
// bit width of each register field:
#define PortTCXoffTotalThreshold_cells_width (10)
#define PortTCXoffTotalThreshold_enable_width (1)
#define PortTCXoffTotalThreshold_trip_width (1)

//---- constants for register SMONPacketCounter -----
#define SMONPacketCounter_nr_sets (3)
#define SMONPacketCounter_nr_entries (8)
// bit width of each register field:
#define SMONPacketCounter_packets_width (32)

//---- constants for register L2FloodingStormControlBucketThresholdConfiguration -----
#define L2FloodingStormControlBucketThresholdConfiguration_nr_entries (6)
// bit width of each register field:
#define L2FloodingStormControlBucketThresholdConfiguration_threshold_width (16)

//---- constants for register CPUReasonCodeOperation -----
#define CPUReasonCodeOperation_nr_entries (16)
// bit width of each register field:
#define CPUReasonCodeOperation_end_width (16)
#define CPUReasonCodeOperation_mutableCpu_width (1)
#define CPUReasonCodeOperation_start_width (16)
#define CPUReasonCodeOperation_origCpuPkt_width (1)
#define CPUReasonCodeOperation_eQueue_width (3)
#define CPUReasonCodeOperation_forceQueue_width (1)
#define CPUReasonCodeOperation_port_width (3)
#define CPUReasonCodeOperation_forceUpdateOrigCpuPkt_width (1)

//---- constants for register HashBasedL3RoutingTable -----
#define HashBasedL3RoutingTable_nr_entries (2048)
// bit width of each register field:
#define HashBasedL3RoutingTable_ecmpShift_width (3)
#define HashBasedL3RoutingTable_mpls_width (1)
#define HashBasedL3RoutingTable_destIPAddr_width (128)
#define HashBasedL3RoutingTable_useECMP_width (1)
#define HashBasedL3RoutingTable_ipVersion_width (1)
#define HashBasedL3RoutingTable_nextHopPointer_width (10)
#define HashBasedL3RoutingTable_vrf_width (2)
#define HashBasedL3RoutingTable_ecmpMask_width (6)

//---- constants for register PortShaperBucketCapacityConfiguration -----
#define PortShaperBucketCapacityConfiguration_nr_entries (6)
// bit width of each register field:
#define PortShaperBucketCapacityConfiguration_bucketCapacity_width (17)

//---- constants for register L3RoutingTCAM -----
#define L3RoutingTCAM_nr_entries (16)
// bit width of each register field:
#define L3RoutingTCAM_proto_width (2)
#define L3RoutingTCAM_vrfMaskN_width (2)
#define L3RoutingTCAM_destIPAddr_width (128)
#define L3RoutingTCAM_protoMaskN_width (2)
#define L3RoutingTCAM_valid_width (1)
#define L3RoutingTCAM_vrf_width (2)
#define L3RoutingTCAM_destIPAddrMaskN_width (128)

//---- constants for register PortXonFFAThreshold -----
#define PortXonFFAThreshold_nr_entries (6)
// bit width of each register field:
#define PortXonFFAThreshold_cells_width (10)

//---- constants for register L3LPMResult -----
#define L3LPMResult_nr_entries (16)
// bit width of each register field:
#define L3LPMResult_ecmpShift_width (3)
#define L3LPMResult_nextHopPointer_width (10)
#define L3LPMResult_useECMP_width (1)
#define L3LPMResult_ecmpMask_width (6)

//---- constants for register IngressConfigurableACL1TCAM -----
#define IngressConfigurableACL1TCAM_nr_entries (8)
// bit width of each register field:
#define IngressConfigurableACL1TCAM_valid_width (1)
#define IngressConfigurableACL1TCAM_mask_width (135)
#define IngressConfigurableACL1TCAM_compareData_width (135)

//---- constants for register PSErrorCounter -----
#define PSErrorCounter_nr_entries (6)
// bit width of each register field:
#define PSErrorCounter_overflow_width (32)
#define PSErrorCounter_underrun_width (32)

//---- constants for register IngressConfigurableACL0PreLookup -----
#define IngressConfigurableACL0PreLookup_nr_entries (16)
// bit width of each register field:
#define IngressConfigurableACL0PreLookup_valid_width (1)
#define IngressConfigurableACL0PreLookup_rulePtr_width (3)

//---- constants for register DWRRBucketCapacityConfiguration -----
#define DWRRBucketCapacityConfiguration_nr_entries (6)
// bit width of each register field:
#define DWRRBucketCapacityConfiguration_bucketCapacity_width (18)

//---- constants for register PortPauseSettings -----
#define PortPauseSettings_nr_entries (6)
// bit width of each register field:
#define PortPauseSettings_pattern_width (8)
#define PortPauseSettings_reserved_width (2)
#define PortPauseSettings_enable_width (1)
#define PortPauseSettings_force_width (8)
#define PortPauseSettings_mode_width (1)

//
// Structs for register / tables
//


// ------- Struct declaration for register: ForceNonVLANPacketToSpecificColor --------- 
typedef struct {

  // field: color
  //  Initial color of the packet
  uint64_t color     :  2; // bit 1 to 2

  // field: forceColor
  //  When set, packets which are non-VLAN tagged are forced to a color.
  uint64_t forceColor:  1; // bit 0
} t_ForceNonVLANPacketToSpecificColor;

// ------- Struct declaration for register: IKEDecoderDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_IKEDecoderDrop;

// ------- Struct declaration for register: DrainPortDrop --------- 
typedef struct {

  // field: packets
  //  Number of packets.
  uint64_t packets: 32; // bit 0 to 31
} t_DrainPortDrop;

// ------- Struct declaration for register: IPPDebugnextHopPtrFinal --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value: 10; // bit 0 to 9
} t_IPPDebugnextHopPtrFinal;

// ------- Struct declaration for register: IngressMultipleSpanningTreeState --------- 
typedef struct {

  // field: portSptState
  //  The ingress spanning tree state for this MSTI. Bit[1:0] is the state for
  //  port \#0, bit[3:2] is the state for port \#1, etc. \tabThree{Forwarding}
  //  {Discarding} {Learning}
  uint64_t portSptState: 12; // bit 0 to 11
} t_IngressMultipleSpanningTreeState;

// ------- Struct declaration for register: SourcePortTable --------- 
typedef struct {

  // field: preLookupAclBits
  //  Pre lookup bits which is used by this port in the pre-lookup tables in the
  //  ingress ACLS. Same value is used for all pre ACL lookups which has the
  //  source port bits in it.
  uint64_t preLookupAclBits           :  2; // bit 118 to 119

  // field: forcePortAclAction
  //  If enabled then the default acl for this port will always be done, if the
  //  ACL is hit then the port ACL will overwrite the ACL result.\tabTwo{Disabled.
  //  Not action forced.}{Enabled. The port ACL overwrites and result from the
  //  ingress ACL.}
  uint64_t forcePortAclAction         :  1; // bit 117

  // field: enableDefaultPortAcl
  //  If enabled then the default acl for this port will be done if the ACL misses
  //  in its lookup. \tabTwo{Disabled. No default action taken.}{Enabled. If ACL
  //  lookup misses then this ACL actil will be carried out instead.}
  uint64_t enableDefaultPortAcl       :  1; // bit 116

  // field: l2ActionTablePortState
  //  What is the source port status bit. Used in table \register{L2 Action Table}
  //  and \register{L2 Action Table Source Port}.
  uint64_t l2ActionTablePortState     :  1; // bit 115

  // field: enableL2ActionTable
  //  On packets coming in on this port should be checked with the \register{L2
  //  Action Table} and \register{L2 Action Table Source Port}. \tabTwo{No, Do not
  //  lookup on the \register{L2 Action Table} and \register{L2 Action Table
  //  Source Port}.}{Yes. Do Lookup in the \register{L2 Action Table} and
  //  \register{L2 Action Table Source Port}}
  uint64_t enableL2ActionTable        :  1; // bit 114

  // field: natPortState
  //  What is this ports NAT status. \tabTwo{Private}{Public}
  uint64_t natPortState               :  1; // bit 113

  // field: natActionTableEnable
  //  Packets coming in on this source port should be checked in the \register{NAT
  //  Action Table}. \tabTwo{No.}{Yes.}
  uint64_t natActionTableEnable       :  1; // bit 112

  // field: disableRouting
  //  On this source port are the packets allowed to do L3 routing.
  //  \tabTwo{No}{Yes}
  uint64_t disableRouting             :  1; // bit 111

  // field: firstHitSecondMissSendToCpu
  //  If first tunnel lookup exit hit but second tunnel exit lookup fails then
  //  send the packet to the CPU. \tabTwo{Do nothing.}{Send the packet to the
  //  CPU.}
  uint64_t firstHitSecondMissSendToCpu:  1; // bit 110

  // field: disableTunnelExit
  //  On this source port are the packets allowed to do a tunnel exit.
  //  \tabTwo{Yes}{No}
  uint64_t disableTunnelExit          :  1; // bit 109

  // field: enableFromCpuTag
  //  This option can validate the from CPU tag decoding on packets from non-CPU
  //  ports. The CPU port is not affected by this field and always decode the from
  //  CPU tag.
  uint64_t enableFromCpuTag           :  1; // bit 108

  // field: priorityVid
  //  The VID used in the outer VLAN tag comparison, see \field{Source Port
  //  Table}{enablePriorityTag}.
  uint64_t priorityVid                : 12; // bit 96 to 107

  // field: enablePriorityTag
  //  An outer VLAN tag with VID matching \field{Source Port Table}{priorityVid}
  //  will have PCP bits extracted and used to determine output queue but in
  //  remaining VLAN processing this tag will not be treated as a VLAN tag. If the
  //  packet has an inner VLAN tag this will be treated as an outer VLAN tag in
  //  the following VLAN processing. The VID will only be matched in a VLAN header
  //  located immediately after DA and SA MAC, i.e. no custom tags allowed. In
  //  egress processing the outer VLAN tag will be removed. \tabTwo{Disable
  //  comparison.} {Enable comparison.}
  uint64_t enablePriorityTag          :  1; // bit 95

  // field: spt
  //  The spanning tree state for this ingress port. The state Disabled implies
  //  that spanning tree protocol is not enabled and hence frames will be
  //  forwarded on this egress port. \tabFive{Disabled.} {Blocking.} {Listening.}
  //  {Learning.} {Forwarding.}
  uint64_t spt                        :  3; // bit 92 to 94

  // field: destInputMirror
  //  Destination \ifdef{\texLinkAgg}{physical}{} port for input mirroring. Only
  //  valid if \field{Source Port Table}{inputMirrorEnabled} is set.
  uint64_t destInputMirror            :  3; // bit 89 to 91

  // field: imUnderPortIsolation
  //  If set, input mirroring to a destination that isolated the source port in
  //  the \field{Ingress Egress Port Packet Type Filter}{srcPortFilter} will be
  //  ignored.
  uint64_t imUnderPortIsolation       :  1; // bit 88

  // field: imUnderVlanMembership
  //  If set, input mirroring to a destination that not a member of the VLAN will
  //  be ignored.
  uint64_t imUnderVlanMembership      :  1; // bit 87

  // field: inputMirrorEnabled
  //  If set, input mirroring is enabled on this port. In addition to the normal
  //  processing of the packet a copy of the unmodified input packet will be send
  //  to the \field{Source Port Table}{destInputMirror} port and exit on that
  //  port. The copy will be subject to the normal resource limitations in the
  //  switch.
  uint64_t inputMirrorEnabled         :  1; // bit 86

  // field: learnMulticastSaMac
  //  If set, the learning engine allows Ethernet multicast source MAC addresses
  //  to be learned.
  uint64_t learnMulticastSaMac        :  1; // bit 85

  // field: ignoreVlanMembership
  //  By default packets on non-VLAN member source port are dropped before
  //  entering the L2 lookup process. Set this field to one ignore the VLAN
  //  membership check on the source port. However L2 lookup can never forward
  //  packets to non-VLAN member destinations.
  uint64_t ignoreVlanMembership       :  1; // bit 84

  // field: maxAllowedVlans
  //  The maximum number of VLAN headers a packet is allowed to have to enter on
  //  this port. Otherwise the packet will be dropped and the \register{Maximum
  //  Allowed VLAN Drop} will be incremented. \tabFour {Only untagged packets are
  //  accepted.} { 0 to 1 tags are accepted.} {Any number of VLANs are accepted.}
  //  {Any number of VLANs are accepted.}
  uint64_t maxAllowedVlans            :  2; // bit 82 to 83

  // field: minAllowedVlans
  //  The minimum number of VLAN headers a packet must have to be allowed on this
  //  port. Otherwise the packet will be dropped and the \register{Minimum Allowed
  //  VLAN Drop} will be incremented. \tabFour {All packets are accepted.} { 1 or
  //  more tags are accepted.} { 2 or more tags are accepted.} { No packets are
  //  accepted. }
  uint64_t minAllowedVlans            :  2; // bit 80 to 81

  // field: defaultVidOrder
  //  When a new hit is done in the result in the L2,L3,L4 VID range checks the
  //  ingress VID will only be changed if the result has a higher order value.
  uint64_t defaultVidOrder            :  2; // bit 78 to 79

  // field: defaultPcp
  //  The default PCP bits. \ifdef{\texVlanIngressPortOperations}{ This is used in
  //  source port VLAN operations (see .\field{Source Port Table}{pcpSel}).}{} It
  //  is used when creating an internal VLAN header for incoming packets that has
  //  no VLAN header.
  uint64_t defaultPcp                 :  3; // bit 75 to 77

  // field: defaultCfiDei
  //  The default CFI / DEI bit. \ifdef{\texVlanIngressPortOperations}{ This is
  //  used in source port VLAN operations (see \field{Source Port
  //  Table}{cfiDeiSel}).}{} It is used when creating an internal VLAN header for
  //  incoming packets that has no VLAN header.
  uint64_t defaultCfiDei              :  1; // bit 74

  // field: defaultVid
  //  The default VID. \ifdef{\texVlanIngressPortOperations}{This is used in
  //  source port VLAN operations (see \field{Source Port Table}{vidSel}).}{} It
  //  is used to assign Ingress VID (see \field{Source Port
  //  Table}{vlanAssignment}). It is used when creating an internal VLAN header
  //  for incoming packets that has no VLAN header.
  uint64_t defaultVid                 : 12; // bit 62 to 73

  // field: vlanAssignment
  //  Controls how a packets Ingress VID is assigned. If the selected source is
  //  from a VLAN header in the incoming packet and the packet doesn't have that
  //  header, then this table entry's \field{Source Port Table}{defaultVid} will
  //  be used. \tabThree{ packet based - the Ingress VID is assigned from the
  //  incoming packets outermost VLAN header.} { port-based - the packets Ingress
  //  VID is assigned from this table entry's \field{Source Port
  //  Table}{defaultVid}} { mixed - if there are two VLANs in the incoming packet,
  //  the inner VLAN is chosen. If the incoming packet has only 0 or 1 VLAN, then
  //  it will select this table entry's \field{Source Port Table}{defaultVid}}
  uint64_t vlanAssignment             :  2; // bit 60 to 61

  // field: typeSel
  //  Selects which TPID to use when building a new VLAN header in a source port
  //  push or swap operation. \tabThree{C-VLAN - 0x8100.} {S-VLAN - 0x88A8.} {User
  //  defined VLAN type from register \register{Egress Ethernet Type for VLAN
  //  tag}.}
  uint64_t typeSel                    :  2; // bit 58 to 59

  // field: defaultPcpIf
  //  The default PCP bits if \field{Source Port Table}{nrVlansVidOperationIf} is
  //  true. \ifdef{\texVlanIngressPortOperations}{ This is used in source port
  //  VLAN operations (see .\field{Source Port Table}{pcpSel}).}{} It is used when
  //  creating an internal VLAN header for incoming packets that has no VLAN
  //  header.
  uint64_t defaultPcpIf               :  3; // bit 55 to 57

  // field: defaultCfiDeiIf
  //  The default CFI / DEI bit if \field{Source Port
  //  Table}{nrVlansVidOperationIf} is true.
  //  \ifdef{\texVlanIngressPortOperations}{ This is used in source port VLAN
  //  operations (see \field{Source Port Table}{cfiDeiSel}).}{} It is used when
  //  creating an internal VLAN header for incoming packets that has no VLAN
  //  header.
  uint64_t defaultCfiDeiIf            :  1; // bit 54

  // field: defaultVidIf
  //  The default VID if \field{Source Port Table}{nrVlansVidOperationIf} is true.
  //  \ifdef{\texVlanIngressPortOperations}{This is used in source port VLAN
  //  operations (see \field{Source Port Table}{vidSel}).}{} It is used to assign
  //  Ingress VID (see \field{Source Port Table}{vlanAssignment}). It is used when
  //  creating an internal VLAN header for incoming packets that has no VLAN
  //  header.
  uint64_t defaultVidIf               : 12; // bit 42 to 53

  // field: typeSelIf
  //  If the field \field{Source Port Table}{nrVlansVidOperationIf} is true then
  //  this operation will override the default port vid operation \field{Source
  //  Port Table}{typeSel}. Selects which TPID to use when building a new VLAN
  //  header in a source port push or swap operation. \tabThree{C-VLAN - 0x8100.}
  //  {S-VLAN - 0x88A8.} {User defined VLAN type from register \register{Egress
  //  Ethernet Type for VLAN tag}.}
  uint64_t typeSelIf                  :  2; // bit 40 to 41

  // field: pcpSelIf
  //  If the field \field{Source Port Table}{nrVlansVidOperationIf} is true then
  //  this operation will override the default port vid operation \field{Source
  //  Port Table}{pcpSel}. Selects which PCP to use when building a new VLAN
  //  header in a source port push or swap operation. If the selected VLAN header
  //  doesn't exist in the packet then this table entry's \field{Source Port
  //  Table}{defaultPcpIf} will be used. \tabThree{From outermost VLAN in the
  //  original packet. (if any)} {From this table entry's \field{Source Port
  //  Table}{defaultPcp}.} {From the second VLAN in the original packet (if any).}
  uint64_t pcpSelIf                   :  2; // bit 38 to 39

  // field: cfiDeiSelIf
  //  If the field \field{Source Port Table}{nrVlansVidOperationIf} is true then
  //  this operation will override the default port vid operation \field{Source
  //  Port Table}{cfiDeiSel}. Selects which CFI/DEI to use when building a new
  //  VLAN header in a source port push or swap operation. If the selected VLAN
  //  header doesn't exist in the packet then this table entry's \field{Source
  //  Port Table}{defaultCfiDeiIf} will be used. \tabThree{From outermost VLAN in
  //  the original packet (if any).} {From this table entry's \field{Source Port
  //  Table}{defaultCfiDei}.} {From the second VLAN in the original packet (if
  //  any).}
  uint64_t cfiDeiSelIf                :  2; // bit 36 to 37

  // field: vidSelIf
  //  If the field \field{Source Port Table}{nrVlansVidOperationIf} is true then
  //  this operation will override the default port vid operation \field{Source
  //  Port Table}{vidSel}. Selects which VID to use when building a new VLAN
  //  header in a source port push or swap operation. If the selected VLAN header
  //  doesn't exist in the packet then this table entry's \field{Source Port
  //  Table}{defaultVidIf} will be used. \ifdef{\texVlanLutOperations}{The
  //  \register{Ingress VLAN LUT Operation} will also use this selection but
  //  instead of defaultVid it will use \field{Egress VLAN LUT Operation}{vid}.}{}
  //  \tabThree{From outermost VLAN in the original packet (if any).} {From this
  //  table entry's \field{Source Port Table}{defaultVid}.} {From the second VLAN
  //  in the original packet (if any).}
  uint64_t vidSelIf                   :  2; // bit 34 to 35

  // field: vlanSingleOpIf
  //  If the field \field{Source Port Table}{nrVlansVidOperationIf} is true then
  //  this operation will override the default port vid operation \field{Source
  //  Port Table}{vlanSingleOp}. The source port VLAN operation to perform on the
  //  packet. \tabFive{No operation.} {Swap.} {Push.} {Pop.} {Penultimate
  //  pop(remove all VLAN headers).}
  uint64_t vlanSingleOpIf             :  3; // bit 31 to 33

  // field: nrVlansVidOperationIf
  //  This alternative VID operation for port VLAN operation is selected if the
  //  following operation is true. \tabFour{Nr of VLANS in incoming packet is
  //  zero.} {Nr of VLANS in incoming packet is one.} {Nr of VLANS in incoming
  //  packet is two.} {Reserved and Disabled}
  uint64_t nrVlansVidOperationIf      :  2; // bit 29 to 30

  // field: pcpSel
  //  Selects which PCP to use when building a new VLAN header in a source port
  //  push or swap operation. If the selected VLAN header doesn't exist in the
  //  packet then this table entry's \field{Source Port Table}{defaultPcp} will be
  //  used. \tabThree{From outermost VLAN in the original packet. (if any)} {From
  //  this table entry's \field{Source Port Table}{defaultPcp}.} {From the second
  //  VLAN in the original packet (if any).}
  uint64_t pcpSel                     :  2; // bit 27 to 28

  // field: cfiDeiSel
  //  Selects which CFI/DEI to use when building a new VLAN header in a source
  //  port push or swap operation. If the selected VLAN header doesn't exist in
  //  the packet then this table entry's \field{Source Port Table}{defaultCfiDei}
  //  will be used. \tabThree{From outermost VLAN in the original packet (if
  //  any).} {From this table entry's \field{Source Port Table}{defaultCfiDei}.}
  //  {From the second VLAN in the original packet (if any).}
  uint64_t cfiDeiSel                  :  2; // bit 25 to 26

  // field: vidSel
  //  Selects which VID to use when building a new VLAN header in a source port
  //  push or swap operation. If the selected VLAN header doesn't exist in the
  //  packet then this table entry's \field{Source Port Table}{defaultVid} will be
  //  used. \ifdef{\texVlanLutOperations}{The \register{Ingress VLAN LUT
  //  Operation} will also use this selection but instead of defaultVid it will
  //  use \field{Egress VLAN LUT Operation}{vid}.}{} \tabThree{From outermost VLAN
  //  in the original packet (if any).} {From this table entry's \field{Source
  //  Port Table}{defaultVid}.} {From the second VLAN in the original packet (if
  //  any).}
  uint64_t vidSel                     :  2; // bit 23 to 24

  // field: vlanSingleOp
  //  The source port VLAN operation to perform on the packet. \tabFive{No
  //  operation.} {Swap.} {Push.} {Pop.} {Penultimate pop(remove all VLAN
  //  headers).}
  uint64_t vlanSingleOp               :  3; // bit 20 to 22

  // field: aclRule3
  //  Pointer into the \register{Ingress Configurable ACL 3 Rules Setup} table
  //  selecting which ACL fields to select to do the ACL lookup with.
  uint64_t aclRule3                   :  3; // bit 17 to 19

  // field: useAcl3
  //  Use ACL on this source port. \tabTwo{No. No ACL lookup is done} {Yes. The
  //  aclRule3 pointer selects which fields that are part of the lookup}.
  uint64_t useAcl3                    :  1; // bit 16

  // field: aclRule2
  //  Pointer into the \register{Ingress Configurable ACL 2 Rules Setup} table
  //  selecting which ACL fields to select to do the ACL lookup with.
  uint64_t aclRule2                   :  3; // bit 13 to 15

  // field: useAcl2
  //  Use ACL on this source port. \tabTwo{No. No ACL lookup is done} {Yes. The
  //  aclRule2 pointer selects which fields that are part of the lookup}.
  uint64_t useAcl2                    :  1; // bit 12

  // field: aclRule1
  //  Pointer into the \register{Ingress Configurable ACL 1 Rules Setup} table
  //  selecting which ACL fields to select to do the ACL lookup with.
  uint64_t aclRule1                   :  3; // bit 9 to 11

  // field: useAcl1
  //  Use ACL on this source port. \tabTwo{No. No ACL lookup is done} {Yes. The
  //  aclRule1 pointer selects which fields that are part of the lookup}.
  uint64_t useAcl1                    :  1; // bit 8

  // field: aclRule0
  //  Pointer into the \register{Ingress Configurable ACL 0 Rules Setup} table
  //  selecting which ACL fields to select to do the ACL lookup with.
  uint64_t aclRule0                   :  3; // bit 5 to 7

  // field: useAcl0
  //  Use ACL on this source port. \tabTwo{No. No ACL lookup is done} {Yes. The
  //  aclRule0 pointer selects which fields that are part of the lookup}.
  uint64_t useAcl0                    :  1; // bit 4

  // field: colorFromL3
  //  If the packet is IP/MPLS and this bit is set the packet initial color will
  //  be selected from Layer 3 decoding.
  uint64_t colorFromL3                :  1; // bit 3

  // field: prioFromL3
  //  If the packet is IP/MPLS and this is set the egress queue will be selected
  //  from Layer 3 decoding described in \hyperref[sec:EgressQueue]{Determine
  //  Egress Queue}.
  uint64_t prioFromL3                 :  1; // bit 2

  // field: dropUnknownDa
  //  If set to one packets with unknown destination MAC address from this port
  //  will be dropped.
  uint64_t dropUnknownDa              :  1; // bit 1

  // field: learningEn
  //  If hardware learning is turned on and this is set to one, the unknown source
  //  MAC address from this port will be learned.
  uint64_t learningEn                 :  1; // bit 0
} t_SourcePortTable;

// ------- Struct declaration for register: IngressConfigurableACL1SearchMask --------- 
typedef struct {

  // field: masklarge
  //  Which bits to compare in the \register{Ingress Configurable ACL 1 Large
  //  Table} lookup. A bit set to 1 means the corresponding bit in the search data
  //  is compared and 0 means the bit is ignored.
  uint8_t masklarge[17]; // bit 135 to 269

  // field: masksmall
  //  Which bits to compare in the \register{Ingress Configurable ACL 1 Small
  //  Table} lookup. A bit set to 1 means the corresponding bit in the search data
  //  is compared and 0 means the bit is ignored.
  uint8_t masksmall[17]; // bit 0 to 134
} t_IngressConfigurableACL1SearchMask;

// ------- Struct declaration for register: IPPDebugdropPktAfterL2Decode --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value:  1; // bit 0
} t_IPPDebugdropPktAfterL2Decode;

// ------- Struct declaration for register: IngressPacketFilteringDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_IngressPacketFilteringDrop;

// ------- Struct declaration for register: L2ActionTablePerPortDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_L2ActionTablePerPortDrop;

// ------- Struct declaration for register: RouterMTUTable --------- 
typedef struct {

  // field: maxIPv6MTU
  //  The maximum MTU allowed for IPv6 packets
  uint64_t maxIPv6MTU: 16; // bit 16 to 31

  // field: maxIPv4MTU
  //  The maximum MTU allowed for IPv4 packets
  uint64_t maxIPv4MTU: 16; // bit 0 to 15
} t_RouterMTUTable;

// ------- Struct declaration for register: SecondTunnelExitDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_SecondTunnelExitDrop;

// ------- Struct declaration for register: L2IEEE1588DecoderDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_L2IEEE1588DecoderDrop;

// ------- Struct declaration for register: IPPPMDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_IPPPMDrop;

// ------- Struct declaration for register: DebugEPPCounter --------- 
typedef struct {

  // field: packets
  //  Number of packets.
  uint64_t packets: 16; // bit 0 to 15
} t_DebugEPPCounter;

// ------- Struct declaration for register: RequeueOverflowDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets
  uint64_t packets: 32; // bit 0 to 31
} t_RequeueOverflowDrop;

// ------- Struct declaration for register: CoreVersion --------- 
typedef struct {

  // field: version
  //  Version of the core.
  uint64_t version: 32; // bit 0 to 31
} t_CoreVersion;

// ------- Struct declaration for register: IPPDebugdstPortmask --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value:  6; // bit 0 to 5
} t_IPPDebugdstPortmask;

// ------- Struct declaration for register: LearningPacketControl --------- 
typedef struct {

  // field: hwHitWriteBack
  //  If set, the hit update of a learned destination MAC will be pushed to
  //  \register{Hit Update Data FIFO} and written to the \hyperref[sec:FIB]{FIB}
  //  simulatneously. Otherwise the result is only pushed to the FIFO and then
  //  software decides the \hyperref[sec:FIB]{FIB} writes.
  uint64_t hwHitWriteBack     :  1; // bit 2

  // field: hwAgingWriteBack
  //  If set, the aging result will be pushed to the \register{Aging Data FIFO}
  //  and written to the \hyperref[sec:FIB]{FIB} simultaneously. Otherwise the
  //  result is only pushed to the FIFO and software has to read it out then send
  //  in a corresponding learning packet if this aging result should be written to
  //  the tables.
  uint64_t hwAgingWriteBack   :  1; // bit 1

  // field: hwLearningWriteBack
  //  If set, the hardwrae learning result from unknown or port moved source MAC
  //  will be pushed to the \register{Learning Data FIFO} and written to the
  //  \hyperref[sec:FIB]{FIB} simultaneously. Otherwise the result is only pushed
  //  to the FIFO.
  uint64_t hwLearningWriteBack:  1; // bit 0
} t_LearningPacketControl;

// ------- Struct declaration for register: IPPDebugl2DaHashKey --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value: 60; // bit 0 to 59
} t_IPPDebugl2DaHashKey;

// ------- Struct declaration for register: EgressConfigurableACL1TCAMAnswer --------- 
typedef struct {

  // field: tunnelEntryPrio
  //  If multiple tunnelEntry are set and this prio bit is set then this
  //  tunnelEntryPtr will be selected.
  uint64_t tunnelEntryPrio      :  1; // bit 38

  // field: tunnelEntryPtr
  //  The tunnel entry which this packet shall enter upon exiting the switch.
  uint64_t tunnelEntryPtr       :  4; // bit 34 to 37

  // field: tunnelEntryUcMc
  //  Shall this entry point to the \register{Tunnel Entry Instruction Table} with
  //  or without a egress port offset. \tabTwo{Unicast \register{Tunnel Entry
  //  Instruction Table} without offset for each port}{Multicast \register{Tunnel
  //  Entry Instruction Table} with offset for each port.}
  uint64_t tunnelEntryUcMc      :  1; // bit 33

  // field: tunnelEntry
  //  Shall all of these packets enter into a tunnel.
  uint64_t tunnelEntry          :  1; // bit 32

  // field: counter
  //  Which counter in \register{Egress Configurable ACL Match Counter} to update.
  uint64_t counter              :  6; // bit 26 to 31

  // field: updateCounter
  //  When set the selected statistics counter will be updated.
  uint64_t updateCounter        :  1; // bit 25

  // field: destPort
  //  The port which the packet shall be sent to.
  uint64_t destPort             :  3; // bit 22 to 24

  // field: sendToPort
  //  Send the packet to a specific port. \tabTwo{Disabled.}{Send to port
  //  configured in destPort.}
  uint64_t sendToPort           :  1; // bit 21

  // field: dropEnable
  //  If set, the packet shall be dropped and the \register{Egress Configurable
  //  ACL Drop} counter is incremented.
  uint64_t dropEnable           :  1; // bit 20

  // field: metaDataPrio
  //  If multiple ACLs hit this meta_data shall take priority.
  uint64_t metaDataPrio         :  1; // bit 19

  // field: metaData
  //  Meta data for packets going to the CPU.
  uint64_t metaData             : 16; // bit 3 to 18

  // field: metaDataValid
  //  Is the meta_data field valid.
  uint64_t metaDataValid        :  1; // bit 2

  // field: forceSendToCpuOrigPkt
  //  If packet shall be sent to CPU then setting this bit will force the packet
  //  to be the incoming originial packet. \ifdef{\texTunneling}{The exception to
  //  this is rule is the tunnel exit which will still be carried out.}{}.
  uint64_t forceSendToCpuOrigPkt:  1; // bit 1

  // field: sendToCpu
  //  If set, the packet shall be sent to the CPU port.
  uint64_t sendToCpu            :  1; // bit 0
} t_EgressConfigurableACL1TCAMAnswer;

// ------- Struct declaration for register: LearningDAMAC --------- 
typedef struct {

  // field: enable
  //  Shall the switch accept learning packets? \tabTwo{No}{Yes}
  uint64_t enable:  1; // bit 48

  // field: mac
  //  The destination MAC address to be used by software when injecting new
  //  addresses to be learned
  uint64_t mac   : 48; // bit 0 to 47
} t_LearningDAMAC;

// ------- Struct declaration for register: MinimumBufferFree --------- 
typedef struct {

  // field: cells
  //  Number of cells.
  uint64_t cells: 10; // bit 0 to 9
} t_MinimumBufferFree;

// ------- Struct declaration for register: IngressEgressPacketFilteringDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_IngressEgressPacketFilteringDrop;

// ------- Struct declaration for register: IngressSpanningTreeDropBlocking --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_IngressSpanningTreeDropBlocking;

// ------- Struct declaration for register: IngressVIDMACRangeAssignmentAnswer --------- 
typedef struct {

  // field: order
  //  Order for this assignment. If the ingress VID can be assigned from other
  //  packet field ranges, the one with the highest order wins.
  uint64_t order     :  2; // bit 12 to 13

  // field: ingressVid
  //  Ingress VID.
  uint64_t ingressVid: 12; // bit 0 to 11
} t_IngressVIDMACRangeAssignmentAnswer;

// ------- Struct declaration for register: RARPPacketDecoderOptions --------- 
typedef struct {

  // field: toCpu
  //  If a packet comes in on this source port then send the packet to the CPU
  //  port.\tabTwo{Do not sent to CPU. Normal Processing of packet.}{Send to CPU ,
  //  bypass normal packet processing.}
  uint64_t toCpu  :  6; // bit 23 to 28

  // field: drop
  //  If a packet comes in on this source port then drop the packet.\tabTwo{Do not
  //  drop this packet.}{Drop this packet and update the drop counter.}
  uint64_t drop   :  6; // bit 17 to 22

  // field: eth
  //  The value to be used to find this packet type.
  uint64_t eth    : 16; // bit 1 to 16

  // field: enabled
  //  Is this decoding enabled. \tabTwo{No}{Yes}
  uint64_t enabled:  1; // bit 0
} t_RARPPacketDecoderOptions;

// ------- Struct declaration for register: ForceUnknownL3PacketToSpecificEgressQueue --------- 
typedef struct {

  // field: eQueue
  //  The egress queue to be assigned if the forceQueue field in this entry is set
  //  to 1.
  uint64_t eQueue    :  3; // bit 1 to 3

  // field: forceQueue
  //  If set, the packet shall have a forced egress queue. Please see Egress Queue
  //  Selection Diagram in Figure \ref{fig:determineEgressQueue}
  uint64_t forceQueue:  1; // bit 0
} t_ForceUnknownL3PacketToSpecificEgressQueue;

// ------- Struct declaration for register: DebugCounterspVidOpSetup --------- 
typedef struct {

  // field: hitValue
  //  Value to compare to update debug counter. Both the incoming value and this
  //  value is ANDed with the mask before comparsion is carried out. If comparsion
  //  results in true the counter is updated
  uint64_t hitValue:  3; // bit 3 to 5

  // field: mask
  //  Mask for comparison to update debug counter.
  uint64_t mask    :  3; // bit 0 to 2
} t_DebugCounterspVidOpSetup;

// ------- Struct declaration for register: EPPDebugimActive --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value:  1; // bit 0
} t_EPPDebugimActive;

// ------- Struct declaration for register: EmptyMaskDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_EmptyMaskDrop;

// ------- Struct declaration for register: PrioShaperEnable --------- 
typedef struct {

  // field: enable
  //  Bitmask where the index is the Egress Port * 8 + Egress Prio
  uint64_t enable: 48; // bit 0 to 47
} t_PrioShaperEnable;

// ------- Struct declaration for register: UnknownIngressDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_UnknownIngressDrop;

// ------- Struct declaration for register: EgressEthernetTypeforVLANtag --------- 
typedef struct {

  // field: typeValue
  //  Ethernet Type value.
  uint64_t typeValue: 16; // bit 0 to 15
} t_EgressEthernetTypeforVLANtag;

// ------- Struct declaration for register: NextHopMPLSTable --------- 
typedef struct {

  // field: label
  //  MPLS label to use when building a new MPLS tag in a swap or push operation.
  uint64_t label        : 20; // bit 8 to 27

  // field: exp
  //  Value to use for the EXP field when building a new MPLS tag in a swap or
  //  push operation.
  uint64_t exp          :  3; // bit 5 to 7

  // field: expSel
  //  Select which EXP bits to use when building a new MPLS tag in Push or Swap
  //  operation. \tabThree{From this entries EXP field.} {From egress queue
  //  remapping in \register{Egress Queue To MPLS EXP Mapping Table}} {From the
  //  MPLS label (outermost MPLS tag if a swap and innermost if a push.}
  uint64_t expSel       :  2; // bit 3 to 4

  // field: mplsOperation
  //  The egress MPLS tag operation to perform on the packet. \tabFive{No
  //  operation.} {Swap.} {Push.} {Pop.} {Penultimate Pop(remove all MPLS tags).}
  uint64_t mplsOperation:  3; // bit 0 to 2
} t_NextHopMPLSTable;

// ------- Struct declaration for register: EgressPortDisabledDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_EgressPortDisabledDrop;

// ------- Struct declaration for register: LinkAggregationCtrl --------- 
typedef struct {

  // field: useVlanIdInHash
  //  The packets VLAN Identifier tag shall be part of the hash key when
  //  calculating the link aggregate hash value.
  uint64_t useVlanIdInHash :  1; // bit 7

  // field: useNextHopInHash
  //  For routed packets the next hop entry shall be part of the hash key when
  //  calculating the link aggregate hash value.
  uint64_t useNextHopInHash:  1; // bit 6

  // field: useTosInHash
  //  The incoming packets TOS byte shall be part of the hash key when calculating
  //  the link aggregate hash value
  uint64_t useTosInHash    :  1; // bit 5

  // field: useL4InHash
  //  The packets L4 SP / DP and L4 protocol byte shall be part of the hash key
  //  when calculating the link aggregate hash value
  uint64_t useL4InHash     :  1; // bit 4

  // field: useIpInHash
  //  The packets IP source and destination addresses shall be part of the hash
  //  key when calculating the link aggregate hash value
  uint64_t useIpInHash     :  1; // bit 3

  // field: useDaMacInHash
  //  The packets destination MAC addresses shall be part of the hash key when
  //  calculating the link aggregate hash value
  uint64_t useDaMacInHash  :  1; // bit 2

  // field: useSaMacInHash
  //  The packets source MAC address shall be part of the hash key when
  //  calculating the link aggregate hash value
  uint64_t useSaMacInHash  :  1; // bit 1

  // field: enable
  //  Is Link aggregation enabled or not. \tabTwo{Link Aggregation is disabled}
  //  {Link Aggregation is enabled}
  uint64_t enable          :  1; // bit 0
} t_LinkAggregationCtrl;

// ------- Struct declaration for register: PacketBufferStatus --------- 
typedef struct {

  // field: empty
  //  Empty flags for the egress ports
  uint64_t empty:  6; // bit 0 to 5
} t_PacketBufferStatus;

// ------- Struct declaration for register: IPPDebugl2DaTcamHitsAndCast --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value: 15; // bit 0 to 14
} t_IPPDebugl2DaTcamHitsAndCast;

// ------- Struct declaration for register: IPv4TOSFieldToPacketColorMappingTable --------- 
typedef struct {

  // field: color
  //  Packet initial color.
  uint64_t color:  2; // bit 0 to 1
} t_IPv4TOSFieldToPacketColorMappingTable;

// ------- Struct declaration for register: EnableEnqueueToPortsAndQueues --------- 
typedef struct {

  // field: qon
  //  If a bit is set, the corresponding queue is on.
  uint64_t qon:  8; // bit 0 to 7
} t_EnableEnqueueToPortsAndQueues;

// ------- Struct declaration for register: MPLSEXPFieldToEgressQueueMappingTable --------- 
typedef struct {

  // field: pQueue
  //  Egress queue
  uint64_t pQueue:  3; // bit 0 to 2
} t_MPLSEXPFieldToEgressQueueMappingTable;

// ------- Struct declaration for register: CoreTickSelect --------- 
typedef struct {

  // field: clkSelect
  //  Select the source clock for the Core Tick divider. 0: disabled, 1: core
  //  clock, 2: debug_write_data[0], 3: reserved
  uint64_t clkSelect:  2; // bit 0 to 1
} t_CoreTickSelect;

// ------- Struct declaration for register: DebugCounterdoPktUpdateSetup --------- 
typedef struct {

  // field: hitValue
  //  Value to compare to update debug counter. Both the incoming value and this
  //  value is ANDed with the mask before comparsion is carried out. If comparsion
  //  results in true the counter is updated
  uint64_t hitValue:  6; // bit 6 to 11

  // field: mask
  //  Mask for comparison to update debug counter.
  uint64_t mask    :  6; // bit 0 to 5
} t_DebugCounterdoPktUpdateSetup;

// ------- Struct declaration for register: EPPDebugisIPv6 --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value:  1; // bit 0
} t_EPPDebugisIPv6;

// ------- Struct declaration for register: EPPDebugdebugMatchEPP0 --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value: 15; // bit 0 to 14
} t_EPPDebugdebugMatchEPP0;

// ------- Struct declaration for register: PortReserved --------- 
typedef struct {

  // field: cells
  //  Number of cells
  uint64_t cells: 10; // bit 0 to 9
} t_PortReserved;

// ------- Struct declaration for register: ResourceLimiterSet --------- 
typedef struct {

  // field: maxCells
  //  Maximum allowed packet length in cells for this limiter. Packet with cells
  //  more than this value will be dropped.
  uint64_t maxCells         :  8; // bit 30 to 37

  // field: redLimit
  //  When the buffer memory is heavily congested (red), the incoming packet will
  //  be terminated and dropped if the enqueued cells in the corresponding egress
  //  queue is more than this value.
  uint64_t redLimit         : 10; // bit 20 to 29

  // field: yellowLimit
  //  When the buffer memory is slightly congested (yellow) and \field{Resource
  //  Limiter Set}{yellowAccumulated} is reached, the packet will be terminated
  //  and dropped if the enqueued cells in the corresponding queue is more than
  //  this value.
  uint64_t yellowLimit      : 10; // bit 10 to 19

  // field: yellowAccumulated
  //  When the buffer memory is slightly congested (yellow), the ERM allows
  //  accumulation of cells with the same queue or higher scheduling priorities to
  //  the limit in this field before appling the \field{Resource Limiter
  //  Set}{yellowLimit}.
  uint64_t yellowAccumulated: 10; // bit 0 to 9
} t_ResourceLimiterSet;

// ------- Struct declaration for register: InvalidRoutingProtocolDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_InvalidRoutingProtocolDrop;

// ------- Struct declaration for register: EgressConfigurableACL0TCAMAnswer --------- 
typedef struct {

  // field: tunnelEntryPrio
  //  If multiple tunnelEntry are set and this prio bit is set then this
  //  tunnelEntryPtr will be selected.
  uint64_t tunnelEntryPrio      :  1; // bit 50

  // field: tunnelEntryPtr
  //  The tunnel entry which this packet shall enter upon exiting the switch.
  uint64_t tunnelEntryPtr       :  4; // bit 46 to 49

  // field: tunnelEntryUcMc
  //  Shall this entry point to the \register{Tunnel Entry Instruction Table} with
  //  or without a egress port offset. \tabTwo{Unicast \register{Tunnel Entry
  //  Instruction Table} without offset for each port}{Multicast \register{Tunnel
  //  Entry Instruction Table} with offset for each port.}
  uint64_t tunnelEntryUcMc      :  1; // bit 45

  // field: tunnelEntry
  //  Shall all of these packets enter into a tunnel.
  uint64_t tunnelEntry          :  1; // bit 44

  // field: natOpPrio
  //  If multiple natOpValid are set and this prio bit is set then this natOpPtr
  //  value will be selected.
  uint64_t natOpPrio            :  1; // bit 43

  // field: natOpPtr
  //  NAT operation pointer.
  uint64_t natOpPtr             : 10; // bit 33 to 42

  // field: natOpValid
  //  NAT operation pointer is valid.
  uint64_t natOpValid           :  1; // bit 32

  // field: counter
  //  Which counter in \register{Egress Configurable ACL Match Counter} to update.
  uint64_t counter              :  6; // bit 26 to 31

  // field: updateCounter
  //  When set the selected statistics counter will be updated.
  uint64_t updateCounter        :  1; // bit 25

  // field: destPort
  //  The port which the packet shall be sent to.
  uint64_t destPort             :  3; // bit 22 to 24

  // field: sendToPort
  //  Send the packet to a specific port. \tabTwo{Disabled.}{Send to port
  //  configured in destPort.}
  uint64_t sendToPort           :  1; // bit 21

  // field: dropEnable
  //  If set, the packet shall be dropped and the \register{Egress Configurable
  //  ACL Drop} counter is incremented.
  uint64_t dropEnable           :  1; // bit 20

  // field: metaDataPrio
  //  If multiple ACLs hit this meta_data shall take priority.
  uint64_t metaDataPrio         :  1; // bit 19

  // field: metaData
  //  Meta data for packets going to the CPU.
  uint64_t metaData             : 16; // bit 3 to 18

  // field: metaDataValid
  //  Is the meta_data field valid.
  uint64_t metaDataValid        :  1; // bit 2

  // field: forceSendToCpuOrigPkt
  //  If packet shall be sent to CPU then setting this bit will force the packet
  //  to be the incoming originial packet. \ifdef{\texTunneling}{The exception to
  //  this is rule is the tunnel exit which will still be carried out.}{}.
  uint64_t forceSendToCpuOrigPkt:  1; // bit 1

  // field: sendToCpu
  //  If set, the packet shall be sent to the CPU port.
  uint64_t sendToCpu            :  1; // bit 0
} t_EgressConfigurableACL0TCAMAnswer;

// ------- Struct declaration for register: EgressResourceManagerPointer --------- 
typedef struct {

  // field: q7
  //  Pointer to the \register{Resource Limiter Set} for egress queue 7.
  uint64_t q7:  2; // bit 14 to 15

  // field: q6
  //  Pointer to the \register{Resource Limiter Set} for egress queue 6.
  uint64_t q6:  2; // bit 12 to 13

  // field: q5
  //  Pointer to the \register{Resource Limiter Set} for egress queue 5.
  uint64_t q5:  2; // bit 10 to 11

  // field: q4
  //  Pointer to the \register{Resource Limiter Set} for egress queue 4.
  uint64_t q4:  2; // bit 8 to 9

  // field: q3
  //  Pointer to the \register{Resource Limiter Set} for egress queue 3.
  uint64_t q3:  2; // bit 6 to 7

  // field: q2
  //  Pointer to the \register{Resource Limiter Set} for egress queue 2.
  uint64_t q2:  2; // bit 4 to 5

  // field: q1
  //  Pointer to the \register{Resource Limiter Set} for egress queue 1.
  uint64_t q1:  2; // bit 2 to 3

  // field: q0
  //  Pointer to the \register{Resource Limiter Set} for egress queue 0.
  uint64_t q0:  2; // bit 0 to 1
} t_EgressResourceManagerPointer;

// ------- Struct declaration for register: IPUnicastReceivedCounter --------- 
typedef struct {

  // field: packets
  //  Number of packets.
  uint64_t packets: 32; // bit 0 to 31
} t_IPUnicastReceivedCounter;

// ------- Struct declaration for register: EgressConfigurableACL0RulesSetup --------- 
typedef struct {

  // field: fieldSelectBitmask
  //  Bitmask of which fields to select. Set a bit to one to select this specific
  //  field, set zero to not select field. At Maximum 7 bits should be set.
  uint64_t fieldSelectBitmask: 18; // bit 0 to 17
} t_EgressConfigurableACL0RulesSetup;

// ------- Struct declaration for register: SMONSetSearch --------- 
typedef struct {

  // field: vid
  //  VLAN ID
  uint64_t vid    : 12; // bit 3 to 14

  // field: srcPort
  //  Source port
  uint64_t srcPort:  3; // bit 0 to 2
} t_SMONSetSearch;

// ------- Struct declaration for register: L2MulticastStormControlRateConfiguration --------- 
typedef struct {

  // field: ifgCorrection
  //  Extra bytes per packet to correct for IFG in byte mode. Default is 4 byte
  //  FCS plus 20 byte IFG.
  uint64_t ifgCorrection  :  8; // bit 16 to 23

  // field: tick
  //  Select one of the five available core ticks. The tick frequencies are
  //  configured globaly in the core Tick Configuration register.
  uint64_t tick           :  3; // bit 13 to 15

  // field: tokens
  //  The number of tokens added each tick
  uint64_t tokens         : 12; // bit 1 to 12

  // field: packetsNotBytes
  //  If set the bucket will count packets, if cleared bytes
  uint64_t packetsNotBytes:  1; // bit 0
} t_L2MulticastStormControlRateConfiguration;

// ------- Struct declaration for register: SPOverflowDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets on this ingress port.
  uint64_t packets: 32; // bit 0 to 31
} t_SPOverflowDrop;

// ------- Struct declaration for register: PortShaperBucketThresholdConfiguration --------- 
typedef struct {

  // field: threshold
  //  Minimum number of tokens in bucket for the status to be set to accept.
  uint64_t threshold: 17; // bit 0 to 16
} t_PortShaperBucketThresholdConfiguration;

// ------- Struct declaration for register: AllowSpecialFrameCheckForL2ActionTable --------- 
typedef struct {

  // field: dontAllowMPLS
  //  Allow MPLS frames. \tabTwo{Allow frame.}{Do not allow frame.}
  uint64_t dontAllowMPLS        :  1; // bit 21

  // field: dontAllowTCP
  //  Allow TCP frames. \tabTwo{Allow frame.}{Do not allow frame.}
  uint64_t dontAllowTCP         :  1; // bit 20

  // field: dontAllowUDP
  //  Allow UDP frames. \tabTwo{Allow frame.}{Do not allow frame.}
  uint64_t dontAllowUDP         :  1; // bit 19

  // field: dontAllowIPV6
  //  Allow IPV6 frames. \tabTwo{Allow frame.}{Do not allow frame.}
  uint64_t dontAllowIPV6        :  1; // bit 18

  // field: dontAllowIPV4
  //  Allow IPV4 frames. \tabTwo{Allow frame.}{Do not allow frame.}
  uint64_t dontAllowIPV4        :  1; // bit 17

  // field: dontAllowL2McReserved
  //  Allow L2 Reserved Da frames, see register \register{L2 Reserved Multicast
  //  Address Base}. \tabTwo{Allow frame.}{Do not allow frame.}
  uint64_t dontAllowL2McReserved:  1; // bit 16

  // field: dontAllowIGMP
  //  Allow IGMP frames. \tabTwo{Allow frame.}{Do not allow frame.}
  uint64_t dontAllowIGMP        :  1; // bit 15

  // field: dontAllowICMP
  //  Allow ICMP frames. \tabTwo{Allow frame.}{Do not allow frame.}
  uint64_t dontAllowICMP        :  1; // bit 14

  // field: dontAllowL41588
  //  Allow L4 1588 frames. \tabTwo{Allow frame.}{Do not allow frame.}
  uint64_t dontAllowL41588      :  1; // bit 13

  // field: dontAllowL21588
  //  Allow L2 1588 frames. \tabTwo{Allow frame.}{Do not allow frame.}
  uint64_t dontAllowL21588      :  1; // bit 12

  // field: dontAllowAH
  //  Allow AH frames. \tabTwo{Allow frame.}{Do not allow frame.}
  uint64_t dontAllowAH          :  1; // bit 11

  // field: dontAllowESP
  //  Allow ESP frames. \tabTwo{Allow frame.}{Do not allow frame.}
  uint64_t dontAllowESP         :  1; // bit 10

  // field: dontAllowGRE
  //  Allow GRE frames. \tabTwo{Allow frame.}{Do not allow frame.}
  uint64_t dontAllowGRE         :  1; // bit 9

  // field: dontAllowLLDP
  //  Allow LLDP frames. \tabTwo{Allow frame.}{Do not allow frame.}
  uint64_t dontAllowLLDP        :  1; // bit 8

  // field: dontAllowSCTP
  //  Allow STCP frames. \tabTwo{Allow frame.}{Do not allow frame.}
  uint64_t dontAllowSCTP        :  1; // bit 7

  // field: dontAllowBOOTPDHCP
  //  Allow BOOTP_DHCP frames. \tabTwo{Allow frame.}{Do not allow frame.}
  uint64_t dontAllowBOOTPDHCP   :  1; // bit 6

  // field: dontAllowDNS
  //  Allow DNS frames. \tabTwo{Allow frame.}{Do not allow frame.}
  uint64_t dontAllowDNS         :  1; // bit 5

  // field: dontAllowRARP
  //  Allow RARP frames. \tabTwo{Allow frame.}{Do not allow frame.}
  uint64_t dontAllowRARP        :  1; // bit 4

  // field: dontAllowARP
  //  Allow ARP frames. \tabTwo{Allow frame.}{Do not allow frame.}
  uint64_t dontAllowARP         :  1; // bit 3

  // field: dontAllowCAPWAP
  //  Allow CAPWAP frames. \tabTwo{Allow frame.}{Do not allow frame.}
  uint64_t dontAllowCAPWAP      :  1; // bit 2

  // field: dontAllow8021XEAPOL
  //  Allow 802.1X EAPOL frames. \tabTwo{Allow frame.}{Do not allow frame.}
  uint64_t dontAllow8021XEAPOL  :  1; // bit 1

  // field: dontAllowBPDU
  //  Allow BPDU frames. \tabTwo{Allow frame.}{Do not allow frame.}
  uint64_t dontAllowBPDU        :  1; // bit 0
} t_AllowSpecialFrameCheckForL2ActionTable;

// ------- Struct declaration for register: DebugCounterdebugMatchEPP0Setup --------- 
typedef struct {

  // field: hitValue
  //  Value to compare to update debug counter. Both the incoming value and this
  //  value is ANDed with the mask before comparsion is carried out. If comparsion
  //  results in true the counter is updated
  uint64_t hitValue: 15; // bit 15 to 29

  // field: mask
  //  Mask for comparison to update debug counter.
  uint64_t mask    : 15; // bit 0 to 14
} t_DebugCounterdebugMatchEPP0Setup;

// ------- Struct declaration for register: PortTailDropSettings --------- 
typedef struct {

  // field: mode
  //  On a port where both pausing and tail-drop is enabled the modes must match
  //  for the calculation of used FFA to be correct.\begin{fieldValues} \dscValue
  //  [0] Priority mode \dscValue [1] Port mode\end{fieldValues}
  uint64_t mode  :  1; // bit 1

  // field: enable
  //  \begin{fieldValues} \dscValue [0] Tail-drop is disabled for this source port
  //  \dscValue [1] Tail-drop is enabled for this source port\end{fieldValues}
  uint64_t enable:  1; // bit 0
} t_PortTailDropSettings;

// ------- Struct declaration for register: PortShaperRateConfiguration --------- 
typedef struct {

  // field: ifgCorrection
  //  Extra bytes per packet to correct for IFG in byte mode. Default is 4 byte
  //  FCS plus 20 byte IFG.
  uint64_t ifgCorrection  :  8; // bit 17 to 24

  // field: tick
  //  Select one of the five available core ticks. The tick frequencies are
  //  configured globaly in the core Tick Configuration register.
  uint64_t tick           :  3; // bit 14 to 16

  // field: tokens
  //  The number of tokens added each tick
  uint64_t tokens         : 13; // bit 1 to 13

  // field: packetsNotBytes
  //  If set the bucket will count packets, if cleared bytes
  uint64_t packetsNotBytes:  1; // bit 0
} t_PortShaperRateConfiguration;

// ------- Struct declaration for register: DWRRBucketMiscConfiguration --------- 
typedef struct {

  // field: ifgCorrection
  //  Extra bytes per packet to correct for IFG in byte mode.
  uint64_t ifgCorrection  :  8; // bit 6 to 13

  // field: packetsNotBytes
  //  If set the bucket will count packets, if cleared bytes
  uint64_t packetsNotBytes:  1; // bit 5

  // field: threshold
  //  When the number of bytes in any bucket goes below 2**thr, all buckets mapped
  //  to the same prio will be replenished.
  uint64_t threshold      :  5; // bit 0 to 4
} t_DWRRBucketMiscConfiguration;

// ------- Struct declaration for register: EgressConfigurableACL1RulesSetup --------- 
typedef struct {

  // field: fieldSelectBitmask
  //  Bitmask of which fields to select. Set a bit to one to select this specific
  //  field, set zero to not select field. At Maximum 20 bits should be set.
  uint64_t fieldSelectBitmask: 20; // bit 0 to 19
} t_EgressConfigurableACL1RulesSetup;

// ------- Struct declaration for register: CAPWAPPacketDecoderOptions --------- 
typedef struct {

  // field: toCpu
  //  If a packet comes in on this source port then send the packet to the CPU
  //  port.\tabTwo{Do not sent to CPU. Normal Processing of packet.}{Send to CPU ,
  //  bypass normal packet processing.}
  uint64_t toCpu  :  6; // bit 39 to 44

  // field: drop
  //  If a packet comes in on this source port then drop the packet.\tabTwo{Do not
  //  drop this packet.}{Drop this packet and update the drop counter.}
  uint64_t drop   :  6; // bit 33 to 38

  // field: udp2
  //  The value to be used to find this packet type.
  uint64_t udp2   : 16; // bit 17 to 32

  // field: udp1
  //  The value to be used to find this packet type.
  uint64_t udp1   : 16; // bit 1 to 16

  // field: enabled
  //  Is this decoding enabled. \tabTwo{No}{Yes}
  uint64_t enabled:  1; // bit 0
} t_CAPWAPPacketDecoderOptions;

// ------- Struct declaration for register: EPPPacketTailCounter --------- 
typedef struct {

  // field: packets
  //  Number of packet tails.
  uint64_t packets: 32; // bit 0 to 31
} t_EPPPacketTailCounter;

// ------- Struct declaration for register: EgressConfigurableACL0LargeTable --------- 
typedef struct {

  // field: tunnelEntryPrio
  //  This is a result field used when this entry is hit. If multiple tunnelEntry
  //  are set and this prio bit is set then this tunnelEntryPtr will be selected.
  uint64_t tunnelEntryPrio      :  1; // bit 186

  // field: tunnelEntryPtr
  //  This is a result field used when this entry is hit. The tunnel entry which
  //  this packet shall enter upon exiting the switch.
  uint64_t tunnelEntryPtr       :  4; // bit 182 to 185

  // field: tunnelEntryUcMc
  //  This is a result field used when this entry is hit. Shall this entry point
  //  to the \register{Tunnel Entry Instruction Table} with or without a egress
  //  port offset. \tabTwo{Unicast \register{Tunnel Entry Instruction Table}
  //  without offset for each port}{Multicast \register{Tunnel Entry Instruction
  //  Table} with offset for each port.}
  uint64_t tunnelEntryUcMc      :  1; // bit 181

  // field: tunnelEntry
  //  This is a result field used when this entry is hit. Shall all of these
  //  packets enter into a tunnel.
  uint64_t tunnelEntry          :  1; // bit 180

  // field: natOpPrio
  //  This is a result field used when this entry is hit. If multiple natOpValid
  //  are set and this prio bit is set then this natOpPtr value will be selected.
  uint64_t natOpPrio            :  1; // bit 179

  // field: natOpPtr
  //  This is a result field used when this entry is hit. NAT operation pointer.
  uint64_t natOpPtr             : 10; // bit 169 to 178

  // field: natOpValid
  //  This is a result field used when this entry is hit. NAT operation pointer is
  //  valid.
  uint64_t natOpValid           :  1; // bit 168

  // field: counter
  //  This is a result field used when this entry is hit. Which counter in
  //  \register{Egress Configurable ACL Match Counter} to update.
  uint64_t counter              :  6; // bit 162 to 167

  // field: updateCounter
  //  This is a result field used when this entry is hit. When set the selected
  //  statistics counter will be updated.
  uint64_t updateCounter        :  1; // bit 161

  // field: destPort
  //  This is a result field used when this entry is hit. The port which the
  //  packet shall be sent to.
  uint64_t destPort             :  3; // bit 158 to 160

  // field: sendToPort
  //  This is a result field used when this entry is hit. Send the packet to a
  //  specific port. \tabTwo{Disabled.}{Send to port configured in destPort.}
  uint64_t sendToPort           :  1; // bit 157

  // field: dropEnable
  //  This is a result field used when this entry is hit. If set, the packet shall
  //  be dropped and the \register{Egress Configurable ACL Drop} counter is
  //  incremented.
  uint64_t dropEnable           :  1; // bit 156

  // field: metaDataPrio
  //  This is a result field used when this entry is hit. If multiple ACLs hit
  //  this meta_data shall take priority.
  uint64_t metaDataPrio         :  1; // bit 155

  // field: metaData
  //  This is a result field used when this entry is hit. Meta data for packets
  //  going to the CPU.
  uint64_t metaData             : 16; // bit 139 to 154

  // field: metaDataValid
  //  This is a result field used when this entry is hit. Is the meta_data field
  //  valid.
  uint64_t metaDataValid        :  1; // bit 138

  // field: forceSendToCpuOrigPkt
  //  This is a result field used when this entry is hit. If packet shall be sent
  //  to CPU then setting this bit will force the packet to be the incoming
  //  originial packet. \ifdef{\texTunneling}{The exception to this is rule is the
  //  tunnel exit which will still be carried out.}{}.
  uint64_t forceSendToCpuOrigPkt:  1; // bit 137

  // field: sendToCpu
  //  This is a result field used when this entry is hit. If set, the packet shall
  //  be sent to the CPU port.
  uint64_t sendToCpu            :  1; // bit 136

  // field: compareData
  //  The data which shall be compared in this entry.
  uint8_t compareData[17]; // bit 1 to 135

  // field: valid
  //  Is this entry valid. \tabTwo{No}{Yes}.
  uint64_t valid                :  1; // bit 0
} t_EgressConfigurableACL0LargeTable;

// ------- Struct declaration for register: PSPacketHeadCounter --------- 
typedef struct {

  // field: packets
  //  Number of packet headers.
  uint64_t packets: 32; // bit 0 to 31
} t_PSPacketHeadCounter;

// ------- Struct declaration for register: NATActionTableForceOriginalPacket --------- 
typedef struct {

  // field: reasonTwo
  //  Force the packet to the CPU from the NAT action table Reason type 2 to be
  //  the originial (unmodified) packet. \ifdef{\texTunneling}{The exception to
  //  this is rule is the tunnel exit which will still be carried out.}{}
  uint64_t reasonTwo:  1; // bit 1

  // field: reasonOne
  //  Force the packet to the CPU from the NAT action table Reason type 1 to be
  //  the originial (unmodified) packet. \ifdef{\texTunneling}{The exception to
  //  this is rule is the tunnel exit which will still be carried out.}{}
  uint64_t reasonOne:  1; // bit 0
} t_NATActionTableForceOriginalPacket;

// ------- Struct declaration for register: RouterPortEgressSAMACAddress --------- 
typedef struct {

  // field: altMacAddress
  //  The alternative base destination MAC address that is used to identify
  //  packets to the router.
  uint64_t altMacAddress         : 48; // bit 6 to 53

  // field: selectMacEntryPortMask
  //  Portmask to select which SA MAC address to use as router MAC address. One
  //  bit per destination port. \tabTwo{use incoming packets DA MAC address.}{use
  //  altMacAddress.}
  uint64_t selectMacEntryPortMask:  6; // bit 0 to 5
} t_RouterPortEgressSAMACAddress;

// ------- Struct declaration for register: OutputDisable --------- 
typedef struct {

  // field: egressQueue7Disabled
  //  If set, stop scheduling new packets for output from queue 7 on this egress
  //  port.
  uint64_t egressQueue7Disabled:  1; // bit 7

  // field: egressQueue6Disabled
  //  If set, stop scheduling new packets for output from queue 6 on this egress
  //  port.
  uint64_t egressQueue6Disabled:  1; // bit 6

  // field: egressQueue5Disabled
  //  If set, stop scheduling new packets for output from queue 5 on this egress
  //  port.
  uint64_t egressQueue5Disabled:  1; // bit 5

  // field: egressQueue4Disabled
  //  If set, stop scheduling new packets for output from queue 4 on this egress
  //  port.
  uint64_t egressQueue4Disabled:  1; // bit 4

  // field: egressQueue3Disabled
  //  If set, stop scheduling new packets for output from queue 3 on this egress
  //  port.
  uint64_t egressQueue3Disabled:  1; // bit 3

  // field: egressQueue2Disabled
  //  If set, stop scheduling new packets for output from queue 2 on this egress
  //  port.
  uint64_t egressQueue2Disabled:  1; // bit 2

  // field: egressQueue1Disabled
  //  If set, stop scheduling new packets for output from queue 1 on this egress
  //  port.
  uint64_t egressQueue1Disabled:  1; // bit 1

  // field: egressQueue0Disabled
  //  If set, stop scheduling new packets for output from queue 0 on this egress
  //  port.
  uint64_t egressQueue0Disabled:  1; // bit 0
} t_OutputDisable;

// ------- Struct declaration for register: L2AgingCollisionTable --------- 
typedef struct {

  // field: hit
  //  If this is set, then the corresponding \register{L2 Lookup Collision Table}
  //  entry has a L2 SA/DA search hit since the last aging scan.
  uint64_t hit  :  1; // bit 2

  // field: stat
  //  If this is set, then the corresponding \register{L2 Lookup Collision Table}
  //  entry will not be aged out.
  uint64_t stat :  1; // bit 1

  // field: valid
  //  If this is set, then the corresponding \register{L2 Lookup Collision Table}
  //  entry is valid.
  uint64_t valid:  1; // bit 0
} t_L2AgingCollisionTable;

// ------- Struct declaration for register: BOOTPandDHCPDecoderDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_BOOTPandDHCPDecoderDrop;

// ------- Struct declaration for register: IngressResourceManagerDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_IngressResourceManagerDrop;

// ------- Struct declaration for register: IngressAdmissionControlInitialPointer --------- 
typedef struct {

  // field: mmpOrder
  //  Order of the initial ingress MMP pointer.
  uint64_t mmpOrder:  2; // bit 6 to 7

  // field: mmpPtr
  //  Initial pointer to the ingress MMP.
  uint64_t mmpPtr  :  5; // bit 1 to 5

  // field: mmpValid
  //  If set, this entry contains a valid MMP pointer
  uint64_t mmpValid:  1; // bit 0
} t_IngressAdmissionControlInitialPointer;

// ------- Struct declaration for register: L2TunnelEntryInstructionTable --------- 
typedef struct {

  // field: outerEtherType
  //  EtherType preceding the tunnel entry point.
  uint64_t outerEtherType : 16; // bit 4 to 19

  // field: updateEtherType
  //  Shall the Ethernet Type be updated.\tabTwo{No}{Yes}
  uint64_t updateEtherType:  1; // bit 3

  // field: hasUdp
  //  If the header is a IPv4 or IPv6 then a insert an UDP header after IP header.
  uint64_t hasUdp         :  1; // bit 2

  // field: l3Type
  //  Insert header type. \tabFour{ IPv4}{IPv6}{MPLS}{Other.}
  uint64_t l3Type         :  2; // bit 0 to 1
} t_L2TunnelEntryInstructionTable;

// ------- Struct declaration for register: CheckTunnelExitPacketDecoderSize --------- 
typedef struct {

  // field: checkMPLS
  //  Check MPLS Packets. \tabTwo{No.}{Yes.}
  uint64_t checkMPLS:  1; // bit 3

  // field: checkIPv6
  //  Check IPv6 Packets. \tabTwo{No.}{Yes.}
  uint64_t checkIPv6:  1; // bit 2

  // field: checkIPv4
  //  Check IPv4 Packets. \tabTwo{No.}{Yes.}
  uint64_t checkIPv4:  1; // bit 1

  // field: checkL2
  //  Check L2 Packets. \tabTwo{No.}{Yes.}
  uint64_t checkL2  :  1; // bit 0
} t_CheckTunnelExitPacketDecoderSize;

// ------- Struct declaration for register: MaximumAllowedVLANDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_MaximumAllowedVLANDrop;

// ------- Struct declaration for register: LACPDecoderDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_LACPDecoderDrop;

// ------- Struct declaration for register: L2ActionTablePortMoveDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_L2ActionTablePortMoveDrop;

// ------- Struct declaration for register: IngressConfigurableACLDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_IngressConfigurableACLDrop;

// ------- Struct declaration for register: PBPacketHeadCounter --------- 
typedef struct {

  // field: packets
  //  Number of packet headers.
  uint64_t packets: 32; // bit 0 to 31
} t_PBPacketHeadCounter;

// ------- Struct declaration for register: IngressConfigurableACL0LargeTable --------- 
typedef struct {

  // field: forceQueuePrio
  //  This is a result field used when this entry is hit. If multiple forceQueue
  //  are set and this prio bit is set then this forceQueue value will be
  //  selected.
  uint64_t forceQueuePrio       :  1; // bit 466

  // field: eQueue
  //  This is a result field used when this entry is hit. The egress queue to be
  //  assigned if the forceQueue field in this entry is set to 1.
  uint64_t eQueue               :  3; // bit 463 to 465

  // field: forceQueue
  //  This is a result field used when this entry is hit. If set, the packet shall
  //  have a forced egress queue. Please see Egress Queue Selection Diagram in
  //  Figure \ref{fig:determineEgressQueue}
  uint64_t forceQueue           :  1; // bit 462

  // field: mmpOrder
  //  This is a result field used when this entry is hit. Ingress MMP pointer
  //  order.
  uint64_t mmpOrder             :  2; // bit 460 to 461

  // field: mmpPtr
  //  This is a result field used when this entry is hit. Ingress MMP pointer.
  uint64_t mmpPtr               :  5; // bit 455 to 459

  // field: mmpValid
  //  This is a result field used when this entry is hit. If set, this entry
  //  contains a valid MMP pointer
  uint64_t mmpValid             :  1; // bit 454

  // field: forceColorPrio
  //  This is a result field used when this entry is hit. If multiple forceColor
  //  are set and this prio bit is set then this forceVid value will be selected.
  uint64_t forceColorPrio       :  1; // bit 453

  // field: color
  //  This is a result field used when this entry is hit. Initial color of the
  //  packet if the forceColor field is set.
  uint64_t color                :  2; // bit 451 to 452

  // field: forceColor
  //  This is a result field used when this entry is hit. If set, the packet shall
  //  have a forced color.
  uint64_t forceColor           :  1; // bit 450

  // field: natOpPrio
  //  This is a result field used when this entry is hit. If multiple natOpValid
  //  are set and this prio bit is set then this natOpPtr value will be selected.
  uint64_t natOpPrio            :  1; // bit 449

  // field: natOpPtr
  //  This is a result field used when this entry is hit. NAT operation pointer.
  uint64_t natOpPtr             : 11; // bit 438 to 448

  // field: natOpValid
  //  This is a result field used when this entry is hit. NAT operation pointer is
  //  valid.
  uint64_t natOpValid           :  1; // bit 437

  // field: newL4Value
  //  This is a result field used when this entry is hit. Update the L4 SP or DP
  //  with this value
  uint64_t newL4Value           : 16; // bit 421 to 436

  // field: updateL4SpOrDp
  //  This is a result field used when this entry is hit. Update the source or
  //  destination L4 port. \tabTwo{Source L4 Port}{Destination L4 Port}
  uint64_t updateL4SpOrDp       :  1; // bit 420

  // field: enableUpdateL4
  //  This is a result field used when this entry is hit. If this entry is hit
  //  then update L4 Source Port or Destination port in ingress packet processing,
  //  this value will be used in the Egress ACL. \tabTwo{Disable}{Enable}
  uint64_t enableUpdateL4       :  1; // bit 419

  // field: newIpValue
  //  This is a result field used when this entry is hit. Update the SA or DA IPv4
  //  address value.
  uint64_t newIpValue           : 32; // bit 387 to 418

  // field: updateSaOrDa
  //  This is a result field used when this entry is hit. Update the SA or DA IPv4
  //  address. The Destiantion IP address updated will be used in the routing
  //  functionality and Egress ACL functionality. If the source IP address is
  //  updated then the updated value will be used in the egress ACL keys.
  //  \tabTwo{Source IP Address}{Destination IP Address}
  uint64_t updateSaOrDa         :  1; // bit 386

  // field: enableUpdateIp
  //  This is a result field used when this entry is hit. If this entry is hit
  //  then update SA or DA IPv4 address in ingress packet processing, this value
  //  will be used by the routing function and egress ACL if this is exists, this
  //  only works for IPv4. \tabTwo{Disable}{Enable}
  uint64_t enableUpdateIp       :  1; // bit 385

  // field: tosMask
  //  This is a result field used when this entry is hit. Mask for TOS value.
  //  Setting a bit to one means this bit will be selected from the newTosExp
  //  field , while setting this bit to zero means that the bit will be selected
  //  from the packets already existing TOS byte bit.
  uint64_t tosMask              :  8; // bit 377 to 384

  // field: newTosExp
  //  This is a result field used when this entry is hit. New TOS/EXP value.
  uint64_t newTosExp            :  8; // bit 369 to 376

  // field: updateTosExp
  //  This is a result field used when this entry is hit. Force TOS/EXP update.
  uint64_t updateTosExp         :  1; // bit 368

  // field: counter
  //  This is a result field used when this entry is hit. Which counter in
  //  \register{Ingress Configurable ACL Match Counter} to update.
  uint64_t counter              :  6; // bit 362 to 367

  // field: updateCounter
  //  This is a result field used when this entry is hit. When set the selected
  //  statistics counter will be updated.
  uint64_t updateCounter        :  1; // bit 361

  // field: imPrio
  //  This is a result field used when this entry is hit. If multiple input mirror
  //  are set and this prio bit is set then this input mirror will be selected.
  uint64_t imPrio               :  1; // bit 360

  // field: destInputMirror
  //  This is a result field used when this entry is hit. Destination
  //  \ifdef{\texLinkAgg}{physical}{} port for input mirroring.
  uint64_t destInputMirror      :  3; // bit 357 to 359

  // field: inputMirror
  //  This is a result field used when this entry is hit. If set, input mirroring
  //  is enabled for this rule. In addition to the normal processing of the packet
  //  a copy of the unmodified input packet will be send to the destination Input
  //  Mirror port and exit on that port. The copy will be subject to the normal
  //  resource limitations in the switch.
  uint64_t inputMirror          :  1; // bit 356

  // field: destPort
  //  This is a result field used when this entry is hit. The port which the
  //  packet shall be sent to.
  uint64_t destPort             :  3; // bit 353 to 355

  // field: sendToPort
  //  This is a result field used when this entry is hit. Send the packet to a
  //  specific port. \tabTwo{Disabled.}{Send to port configured in destPort.}
  uint64_t sendToPort           :  1; // bit 352

  // field: dropEnable
  //  This is a result field used when this entry is hit. If set, the packet shall
  //  be dropped and the \register{Ingress Configurable ACL Drop} counter is
  //  incremented.
  uint64_t dropEnable           :  1; // bit 351

  // field: metaDataPrio
  //  This is a result field used when this entry is hit. If multiple ACLs hit
  //  this meta_data shall take priority.
  uint64_t metaDataPrio         :  1; // bit 350

  // field: metaData
  //  This is a result field used when this entry is hit. Meta data for packets
  //  going to the CPU.
  uint64_t metaData             : 16; // bit 334 to 349

  // field: metaDataValid
  //  This is a result field used when this entry is hit. Is the meta_data field
  //  valid.
  uint64_t metaDataValid        :  1; // bit 333

  // field: forceSendToCpuOrigPkt
  //  This is a result field used when this entry is hit. If packet shall be sent
  //  to CPU then setting this bit will force the packet to be the incoming
  //  originial packet. \ifdef{\texTunneling}{The exception to this is rule is the
  //  tunnel exit which will still be carried out.}{}.
  uint64_t forceSendToCpuOrigPkt:  1; // bit 332

  // field: sendToCpu
  //  This is a result field used when this entry is hit. If set, the packet shall
  //  be sent to the CPU port.
  uint64_t sendToCpu            :  1; // bit 331

  // field: compareData
  //  The data which shall be compared in this entry.
  uint8_t compareData[42]; // bit 1 to 330

  // field: valid
  //  Is this entry valid. \tabTwo{No}{Yes}.
  uint64_t valid                :  1; // bit 0
} t_IngressConfigurableACL0LargeTable;

// ------- Struct declaration for register: EgressPortDepth --------- 
typedef struct {

  // field: packets
  //  Number of packet currently queued.
  uint64_t packets: 10; // bit 0 to 9
} t_EgressPortDepth;

// ------- Struct declaration for register: EgressConfigurableACL0SmallTable --------- 
typedef struct {

  // field: tunnelEntryPrio
  //  This is a result field used when this entry is hit. If multiple tunnelEntry
  //  are set and this prio bit is set then this tunnelEntryPtr will be selected.
  uint64_t tunnelEntryPrio      :  1; // bit 186

  // field: tunnelEntryPtr
  //  This is a result field used when this entry is hit. The tunnel entry which
  //  this packet shall enter upon exiting the switch.
  uint64_t tunnelEntryPtr       :  4; // bit 182 to 185

  // field: tunnelEntryUcMc
  //  This is a result field used when this entry is hit. Shall this entry point
  //  to the \register{Tunnel Entry Instruction Table} with or without a egress
  //  port offset. \tabTwo{Unicast \register{Tunnel Entry Instruction Table}
  //  without offset for each port}{Multicast \register{Tunnel Entry Instruction
  //  Table} with offset for each port.}
  uint64_t tunnelEntryUcMc      :  1; // bit 181

  // field: tunnelEntry
  //  This is a result field used when this entry is hit. Shall all of these
  //  packets enter into a tunnel.
  uint64_t tunnelEntry          :  1; // bit 180

  // field: natOpPrio
  //  This is a result field used when this entry is hit. If multiple natOpValid
  //  are set and this prio bit is set then this natOpPtr value will be selected.
  uint64_t natOpPrio            :  1; // bit 179

  // field: natOpPtr
  //  This is a result field used when this entry is hit. NAT operation pointer.
  uint64_t natOpPtr             : 10; // bit 169 to 178

  // field: natOpValid
  //  This is a result field used when this entry is hit. NAT operation pointer is
  //  valid.
  uint64_t natOpValid           :  1; // bit 168

  // field: counter
  //  This is a result field used when this entry is hit. Which counter in
  //  \register{Egress Configurable ACL Match Counter} to update.
  uint64_t counter              :  6; // bit 162 to 167

  // field: updateCounter
  //  This is a result field used when this entry is hit. When set the selected
  //  statistics counter will be updated.
  uint64_t updateCounter        :  1; // bit 161

  // field: destPort
  //  This is a result field used when this entry is hit. The port which the
  //  packet shall be sent to.
  uint64_t destPort             :  3; // bit 158 to 160

  // field: sendToPort
  //  This is a result field used when this entry is hit. Send the packet to a
  //  specific port. \tabTwo{Disabled.}{Send to port configured in destPort.}
  uint64_t sendToPort           :  1; // bit 157

  // field: dropEnable
  //  This is a result field used when this entry is hit. If set, the packet shall
  //  be dropped and the \register{Egress Configurable ACL Drop} counter is
  //  incremented.
  uint64_t dropEnable           :  1; // bit 156

  // field: metaDataPrio
  //  This is a result field used when this entry is hit. If multiple ACLs hit
  //  this meta_data shall take priority.
  uint64_t metaDataPrio         :  1; // bit 155

  // field: metaData
  //  This is a result field used when this entry is hit. Meta data for packets
  //  going to the CPU.
  uint64_t metaData             : 16; // bit 139 to 154

  // field: metaDataValid
  //  This is a result field used when this entry is hit. Is the meta_data field
  //  valid.
  uint64_t metaDataValid        :  1; // bit 138

  // field: forceSendToCpuOrigPkt
  //  This is a result field used when this entry is hit. If packet shall be sent
  //  to CPU then setting this bit will force the packet to be the incoming
  //  originial packet. \ifdef{\texTunneling}{The exception to this is rule is the
  //  tunnel exit which will still be carried out.}{}.
  uint64_t forceSendToCpuOrigPkt:  1; // bit 137

  // field: sendToCpu
  //  This is a result field used when this entry is hit. If set, the packet shall
  //  be sent to the CPU port.
  uint64_t sendToCpu            :  1; // bit 136

  // field: compareData
  //  The data which shall be compared in this entry.
  uint8_t compareData[17]; // bit 1 to 135

  // field: valid
  //  Is this entry valid. \tabTwo{No}{Yes}.
  uint64_t valid                :  1; // bit 0
} t_EgressConfigurableACL0SmallTable;

// ------- Struct declaration for register: SelectWhichEgressQoSMappingTableToUse --------- 
typedef struct {

  // field: pcp
  //  Packets new PCP.
  uint64_t pcp            :  3; // bit 7 to 9

  // field: updatePcp
  //  Update Pcp field in outgoing packet. \tabTwo{Do not update.}{Update.}
  uint64_t updatePcp      :  1; // bit 6

  // field: cfiDei
  //  Packets new CFI/DEI.
  uint64_t cfiDei         :  1; // bit 5

  // field: updateCfiDei
  //  Update CfiDei field in outgoing packet. \tabTwo{Do not update.}{Update.}
  uint64_t updateCfiDei   :  1; // bit 4

  // field: whichTablePtr
  //  Which index of the tables to use. For most QoS tables there exists multiple
  //  tables to choose from.
  uint64_t whichTablePtr  :  1; // bit 3

  // field: whichTableToUse
  //  Select which table type to use. \tabSix{ None. No remapping} { \register{L2
  //  QoS Mapping Table} } { \register{IP QoS Mapping Table} } { \register{TOS QoS
  //  Mapping Table} } { \ifdef{\texMpls}{\register{MPLS QoS Mapping Table}
  //  }{Reserved} } { Use this tables remapping of DEI and PCP bits.}
  uint64_t whichTableToUse:  3; // bit 0 to 2
} t_SelectWhichEgressQoSMappingTableToUse;

// ------- Struct declaration for register: EPPDebugimExtra --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value:  1; // bit 0
} t_EPPDebugimExtra;

// ------- Struct declaration for register: DWRRWeightConfiguration --------- 
typedef struct {

  // field: weight
  //  The relative weight of the queue. A queue with weight 0 is not part of the
  //  round robin scheduling but will always be selected last.
  uint64_t weight:  8; // bit 0 to 7
} t_DWRRWeightConfiguration;

// ------- Struct declaration for register: LinkAggregateWeight --------- 
typedef struct {

  // field: ports
  //  One bit per physical port.
  uint64_t ports:  6; // bit 0 to 5
} t_LinkAggregateWeight;

// ------- Struct declaration for register: L3RoutingDefault --------- 
typedef struct {

  // field: mmpOrder
  //  Ingress MMP pointer order.
  uint64_t mmpOrder :  2; // bit 18 to 19

  // field: mmpPtr
  //  Ingress MMP pointer.
  uint64_t mmpPtr   :  5; // bit 13 to 17

  // field: mmpValid
  //  If set, this entry contains a valid MMP pointer
  uint64_t mmpValid :  1; // bit 12

  // field: sendToCpu
  //  If set then the packet will be sent to the CPU.
  uint64_t sendToCpu:  1; // bit 11

  // field: pktDrop
  //  If set the packet will be drop and the \register{L3 Lookup Drop} counter
  //  incremented.
  uint64_t pktDrop  :  1; // bit 10

  // field: nextHop
  //  The default next hop to be used. Index into the \register{Next Hop Table}.
  uint64_t nextHop  : 10; // bit 0 to 9
} t_L3RoutingDefault;

// ------- Struct declaration for register: TransmittedPacketsonEgressVRF --------- 
typedef struct {

  // field: packets
  //  Number of packets.
  uint64_t packets: 32; // bit 0 to 31
} t_TransmittedPacketsonEgressVRF;

// ------- Struct declaration for register: GREPacketDecoderOptions --------- 
typedef struct {

  // field: toCpu
  //  If a packet comes in on this source port then send the packet to the CPU
  //  port.\tabTwo{Do not sent to CPU. Normal Processing of packet.}{Send to CPU ,
  //  bypass normal packet processing.}
  uint64_t toCpu  :  6; // bit 47 to 52

  // field: drop
  //  If a packet comes in on this source port then drop the packet.\tabTwo{Do not
  //  drop this packet.}{Drop this packet and update the drop counter.}
  uint64_t drop   :  6; // bit 41 to 46

  // field: udp2
  //  The value to be used to find this packet type.
  uint64_t udp2   : 16; // bit 25 to 40

  // field: udp1
  //  The value to be used to find this packet type.
  uint64_t udp1   : 16; // bit 9 to 24

  // field: l4Proto
  //  The value to be used to find this packet type.
  uint64_t l4Proto:  8; // bit 1 to 8

  // field: enabled
  //  Is this decoding enabled. \tabTwo{No}{Yes}
  uint64_t enabled:  1; // bit 0
} t_GREPacketDecoderOptions;

// ------- Struct declaration for register: IngressAdmissionControlReset --------- 
typedef struct {

  // field: bucketReset
  //  if set, reload with full tokens for token buckets in this entry.
  uint64_t bucketReset:  1; // bit 0
} t_IngressAdmissionControlReset;

// ------- Struct declaration for register: QueueShaperEnable --------- 
typedef struct {

  // field: enable
  //  Bitmask where the index is the Egress Port * 8 + Egress Queue
  uint64_t enable: 48; // bit 0 to 47
} t_QueueShaperEnable;

// ------- Struct declaration for register: XoffFFAThreshold --------- 
typedef struct {

  // field: trip
  //  \begin{fieldValues} \dscValue [0] Normal operation \dscValue [1] Force this
  //  threshold to be counted as exceeded \end{fieldValues} Only valid if this
  //  Xoff threshold is enabled.
  uint64_t trip  :  1; // bit 11

  // field: enable
  //  \begin{fieldValues} \dscValue [0] This Xoff threshold is disabled \dscValue
  //  [1] This Xoff threshold is enabled\end{fieldValues}
  uint64_t enable:  1; // bit 10

  // field: cells
  //  Xoff threshold for the total number of used FFA cells
  uint64_t cells : 10; // bit 0 to 9
} t_XoffFFAThreshold;

// ------- Struct declaration for register: PrioShaperBucketCapacityConfiguration --------- 
typedef struct {

  // field: bucketCapacity
  //  Capacity of the token bucket
  uint64_t bucketCapacity: 17; // bit 0 to 16
} t_PrioShaperBucketCapacityConfiguration;

// ------- Struct declaration for register: IEEE8021XandEAPOLPacketDecoderOptions --------- 
typedef struct {

  // field: toCpu
  //  If a packet comes in on this source port then send the packet to the CPU
  //  port.\tabTwo{Do not sent to CPU. Normal Processing of packet.}{Send to CPU ,
  //  bypass normal packet processing.}
  uint64_t toCpu  :  6; // bit 23 to 28

  // field: drop
  //  If a packet comes in on this source port then drop the packet.\tabTwo{Do not
  //  drop this packet.}{Drop this packet and update the drop counter.}
  uint64_t drop   :  6; // bit 17 to 22

  // field: eth
  //  The value to be used to find this packet type.
  uint64_t eth    : 16; // bit 1 to 16

  // field: enabled
  //  Is this decoding enabled. \tabTwo{No}{Yes}
  uint64_t enabled:  1; // bit 0
} t_IEEE8021XandEAPOLPacketDecoderOptions;

// ------- Struct declaration for register: EgressVLANTranslationTCAM --------- 
typedef struct {

  // field: outermostVidType
  //  The outermost VID is a S-tag or C-Tag. \tabTwo{Customer tag}{Service tag}
  uint64_t outermostVidType    :  1; // bit 32

  // field: outermostVidTypemask
  //  Mask for outermostVidType.
  uint64_t outermostVidTypemask:  1; // bit 31

  // field: outermostVid
  //  The outermost VID of the modified packet.
  uint64_t outermostVid        : 12; // bit 19 to 30

  // field: outermostVidmask
  //  Mask for outermostVid.
  uint64_t outermostVidmask    : 12; // bit 7 to 18

  // field: dstPort
  //  The destination port which the packet is going out on
  uint64_t dstPort             :  3; // bit 4 to 6

  // field: dstPortmask
  //  Mask for dstPort.
  uint64_t dstPortmask         :  3; // bit 1 to 3

  // field: valid
  //  Is this entry valid. \tabTwo{No}{Yes}
  uint64_t valid               :  1; // bit 0
} t_EgressVLANTranslationTCAM;

// ------- Struct declaration for register: L2LookupCollisionTable --------- 
typedef struct {

  // field: gid
  //  Global identifier for learning
  uint64_t gid    : 12; // bit 48 to 59

  // field: macAddr
  //  MAC address
  uint64_t macAddr: 48; // bit 0 to 47
} t_L2LookupCollisionTable;

// ------- Struct declaration for register: NextHopPacketModifications --------- 
typedef struct {

  // field: msptPtr
  //  The multiple spanning tree to be used by packets for egress spanning tree
  //  check for this next hop. Points to an entry in \register{Egress Multiple
  //  Spanning Tree State}
  uint64_t msptPtr        :  4; // bit 47 to 50

  // field: innerCfiDei
  //  The CFI/DEI bit to use when building an inner VLAN header. If selected by
  //  \field{Next Hop Packet Modifications}{innerCfiDeiSel}.
  uint64_t innerCfiDei    :  1; // bit 46

  // field: innerPcp
  //  The PCP bits to use when building an inner VLAN header. If selected by
  //  \field{Next Hop Packet Modifications}{innerPcpSel}.
  uint64_t innerPcp       :  3; // bit 43 to 45

  // field: innerVid
  //  The VID used when building an inner VLAN header.
  uint64_t innerVid       : 12; // bit 31 to 42

  // field: innerEthType
  //  Pointer to the VLAN type. \tabThree{C-VLAN - 0x8100.} {S-VLAN - 0x88A8.}
  //  {User defined VLAN.}
  uint64_t innerEthType   :  2; // bit 29 to 30

  // field: innerCfiDeiSel
  //  Selects which CFI/DEI bit to use when building an inner VLAN header.
  //  \tabThree{From innermost VLAN header in the original packet (if any).} {From
  //  this entrie's \field{Next Hop Packet Modifications}{innerCfiDei} field.}
  //  {From \register{Router Egress Queue To VLAN Data}.}
  uint64_t innerCfiDeiSel :  2; // bit 27 to 28

  // field: innerPcpSel
  //  Selects which PCP bits to use when building an inner VLAN header.
  //  \tabThree{From innermost VLAN header in the original packet (if any).} {From
  //  this entrie's \field{Next Hop Packet Modifications}{innerPcp} field.} {From
  //  \register{Router Egress Queue To VLAN Data}.}
  uint64_t innerPcpSel    :  2; // bit 25 to 26

  // field: innerVlanAppend
  //  Insert/push an inner VLAN header in the packet. The information used to
  //  create the new VLAN header is controlled by the fields \field{Next Hop
  //  Packet Modifications}{innerVid}, \field{Next Hop Packet
  //  Modifications}{innerPcpSel}, \field{Next Hop Packet
  //  Modifications}{innerCfiDeiSel} and \field{Next Hop Packet
  //  Modifications}{innerEthType}. If the selected innermost VLAN header field
  //  doesn't exist in the packet then the new VLAN header field will be taken
  //  from \register{Router Egress Queue To VLAN Data}. \tabTwo{No operation}
  //  {Insert/push an inner VLAN tag.}
  uint64_t innerVlanAppend:  1; // bit 24

  // field: outerCfiDei
  //  The CFI/DEI bit to use when building an outer VLAN header. If selected by
  //  \field{Next Hop Packet Modifications}{outerCfiDeiSel}.
  uint64_t outerCfiDei    :  1; // bit 23

  // field: outerPcp
  //  The PCP bits to use when building an outer VLAN header. If selected by
  //  \field{Next Hop Packet Modifications}{outerPcpSel}.
  uint64_t outerPcp       :  3; // bit 20 to 22

  // field: outerVid
  //  The VID used when building an outer VLAN header.
  uint64_t outerVid       : 12; // bit 8 to 19

  // field: outerEthType
  //  Pointer to the VLAN type. \tabThree{C-VLAN - 0x8100.} {S-VLAN - 0x88A8.}
  //  {User defined VLAN.}
  uint64_t outerEthType   :  2; // bit 6 to 7

  // field: outerCfiDeiSel
  //  Selects which CFI/DEI bit to use when building an outer VLAN header.
  //  \tabThree{From outermost VLAN header in the original packet (if any).} {From
  //  this entrie's \field{Next Hop Packet Modifications}{outerCfiDei} field.}
  //  {From \register{Router Egress Queue To VLAN Data}.}
  uint64_t outerCfiDeiSel :  2; // bit 4 to 5

  // field: outerPcpSel
  //  Selects which PCP bits to use when building an outer VLAN header.
  //  \tabThree{From outermost VLAN header in the original packet (if any).} {From
  //  this entrie's \field{Next Hop Packet Modifications}{outerPcp} field.} {From
  //  \register{Router Egress Queue To VLAN Data}.}
  uint64_t outerPcpSel    :  2; // bit 2 to 3

  // field: outerVlanAppend
  //  Insert/push an outer VLAN header in the packet. The information used to
  //  create the new VLAN header is controlled by the fields \field{Next Hop
  //  Packet Modifications}{outerVid}, \field{Next Hop Packet
  //  Modifications}{outerPcpSel}, \field{Next Hop Packet
  //  Modifications}{outerCfiDeiSel} and \field{Next Hop Packet
  //  Modifications}{outerEthType}. If the selected outermost VLAN header field
  //  doesn't exist in the packet then the new VLAN header field will be taken
  //  from \register{Router Egress Queue To VLAN Data}. \tabTwo{No operation.}
  //  {Insert/push an outer VLAN tag.}
  uint64_t outerVlanAppend:  1; // bit 1

  // field: valid
  //  Is this a valid entry. If the router points to an entry with this field
  //  cleared the packet will be sent to CPU. \tabTwo{Invalid} {Valid}
  uint64_t valid          :  1; // bit 0
} t_NextHopPacketModifications;

// ------- Struct declaration for register: DisableCPUtagonCPUPort --------- 
typedef struct {

  // field: disableReason0
  //  When set, the CPU port will no longer add a CPU Tag to packets going to the
  //  CPU port with reason code 0(default reason). \tabTwo{To CPU Tag enabled}{To
  //  CPU Tag disabled}
  uint64_t disableReason0:  1; // bit 1

  // field: disable
  //  When set, the CPU port will no longer add a CPU Tag to packets going to the
  //  CPU port. \tabTwo{To CPU Tag enabled}{To CPU Tag disabled}
  uint64_t disable       :  1; // bit 0
} t_DisableCPUtagonCPUPort;

// ------- Struct declaration for register: ESPDecoderDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_ESPDecoderDrop;

// ------- Struct declaration for register: IngressEthernetTypeforVLANtag --------- 
typedef struct {

  // field: valid
  //  User defined VLAN is valid. \tabTwo{Not Valid.}{Valid.}
  uint64_t valid    :  1; // bit 17

  // field: type
  //  User defined VLAN type. \tabTwo{Customer VLAN.}{Service VLAN.}
  uint64_t type     :  1; // bit 16

  // field: typeValue
  //  Ethernet Type value.
  uint64_t typeValue: 16; // bit 0 to 15
} t_IngressEthernetTypeforVLANtag;

// ------- Struct declaration for register: IPPDebugisBroadcast --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value:  1; // bit 0
} t_IPPDebugisBroadcast;

// ------- Struct declaration for register: IPv4TOSFieldToEgressQueueMappingTable --------- 
typedef struct {

  // field: pQueue
  //  Egress queue.
  uint64_t pQueue:  3; // bit 0 to 2
} t_IPv4TOSFieldToEgressQueueMappingTable;

// ------- Struct declaration for register: RARPDecoderDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_RARPDecoderDrop;

// ------- Struct declaration for register: IngressVIDInnerVIDRangeAssignmentAnswer --------- 
typedef struct {

  // field: order
  //  Order for this assignment. If the ingress VID can be assigned from other
  //  packet field ranges, the one with the highest order wins.
  uint64_t order     :  2; // bit 12 to 13

  // field: ingressVid
  //  Ingress VID.
  uint64_t ingressVid: 12; // bit 0 to 11
} t_IngressVIDInnerVIDRangeAssignmentAnswer;

// ------- Struct declaration for register: SecondTunnelExitLookupTCAMAnswer --------- 
typedef struct {

  // field: tunnelExitEgressPtr
  //  Tunnel Exit Egress Pointer. Shall point to same tunnel / packet
  //  decapsulation operation but setup in egress pipeline in \register{Egress
  //  Tunnel Exit Table}
  uint64_t tunnelExitEgressPtr :  5; // bit 52 to 56

  // field: newVid
  //  The new to be used VID.
  uint64_t newVid              : 12; // bit 40 to 51

  // field: replaceVid
  //  Replace the assigned VID. This is the VID which shall be used in the VLAN
  //  table lookup. This forces a new VID into this packet and bypassing all but
  //  the ACL force VID operation.
  uint64_t replaceVid          :  1; // bit 39

  // field: dontExit
  //  Do not do a tunnel exit on this packet.
  uint64_t dontExit            :  1; // bit 38

  // field: dropPkt
  //  Drop the packet.
  uint64_t dropPkt             :  1; // bit 37

  // field: whereToRemove
  //  Where to do the tunnel exit from \tabFour{At Byte Zero}{After L2 and up to
  //  two VLAN headers.}{After L3 IPv4/IPv6 headers.}{Reserved.}
  uint64_t whereToRemove       :  2; // bit 35 to 36

  // field: l4Protocol
  //  If packet is removed after L3 headers then this new L4 Protocol will be
  //  written.
  uint64_t l4Protocol          :  8; // bit 27 to 34

  // field: updateL4Protocol
  //  If packet is removed after L3 headers then update the L4 Protocol in IP
  //  header.
  uint64_t updateL4Protocol    :  1; // bit 26

  // field: removeVlan
  //  If packet is removed after L2+VLAN headers then remove the VLAN headers on
  //  the incoming packet.
  uint64_t removeVlan          :  1; // bit 25

  // field: ethType
  //  If packet is removed after L2+VLAN headers then the New Ethernet Type which
  //  will overwrite the existing lowest 16 bits after the removal operation.
  uint64_t ethType             : 16; // bit 9 to 24

  // field: updateEthType
  //  If packet is removed after L2+VLAN headers then update the Ethernet Header
  //  Type Field
  uint64_t updateEthType       :  1; // bit 8

  // field: howManyBytesToRemove
  //  How many bytes to remove.
  uint64_t howManyBytesToRemove:  8; // bit 0 to 7
} t_SecondTunnelExitLookupTCAMAnswer;

// ------- Struct declaration for register: EgressPortConfiguration --------- 
typedef struct {

  // field: useEgressQueueRemapping
  //  Which remapping to final PCP, DEI, EXP and TOS fields shall be used for this
  //  port. \tabTwo{Only use Egress Queue Remapping Tables}{First use the Egress
  //  Queue Remapping Tables then use the \register{Select Which Egress QoS
  //  Mapping Table To Use} to determine the final DEI,CFI,TOS and EXP fields.}
  uint64_t useEgressQueueRemapping:  1; // bit 40

  // field: dropSStaggedVlans
  //  Drop or allow packets which has a S-VLAN followed by a S-VLAN tagged on this
  //  egress port. \tabTwo{Allow packets which has a S-VLAN tag followed by a
  //  S-VLAN tag.} {Drop packets which has a S-VLAN tag followed by a S-VLAN tag.}
  uint64_t dropSStaggedVlans      :  1; // bit 39

  // field: dropCCtaggedVlans
  //  Drop or allow packets which has a C-VLAN followed by a C-VLAN tagged on this
  //  egress port. \tabTwo{Allow packets which has a C-VLAN tag followed by a
  //  C-VLAN tag.} {Drop packets which has a C-VLAN tag followed by a C-VLAN tag.}
  uint64_t dropCCtaggedVlans      :  1; // bit 38

  // field: dropSCtaggedVlans
  //  Drop or allow packets which has a S-VLAN followed by a C-VLAN tagged on this
  //  egress port. \tabTwo{Allow packets which has a S-VLAN followed by a C-VLAN
  //  tag.} {Drop packets which has a S-VLAN tag followed by a C-VLAN tag.}
  uint64_t dropSCtaggedVlans      :  1; // bit 37

  // field: dropCStaggedVlans
  //  Drop or allow packets which has a C-VLAN followed by a S-VLAN tagged on this
  //  egress port. \tabTwo{Allow packets which has a C-VLAN tag followed by a
  //  S-VLAN tag.} {Drop packets which has a C-VLAN tag followed by a S-VLAN tag.}
  uint64_t dropCStaggedVlans      :  1; // bit 36

  // field: dropDualTaggedVlans
  //  Drop or allow packets which has more than one VLAN tag on this egress port.
  //  \tabTwo{Allow packets which has more than one VLAN tag.} {Drop packets which
  //  has more than one VLAN tag.}
  uint64_t dropDualTaggedVlans    :  1; // bit 35

  // field: dropSingleTaggedVlans
  //  Drop or Allow packets that has one VLAN tag on this egress port.
  //  \tabTwo{Allow untagged packets.}{Drop untagged packets.}
  uint64_t dropSingleTaggedVlans  :  1; // bit 34

  // field: dropUntaggedVlans
  //  Drop or Allow packets that are VLAN untagged on this egress port.
  //  \tabTwo{Allow untagged packets.}{Drop untagged packets.}
  uint64_t dropUntaggedVlans      :  1; // bit 33

  // field: moreThanOneVlans
  //  When filtering with dropCtaggedVlans or dropStaggedVlans then this field
  //  must be set to 1.
  uint64_t moreThanOneVlans       :  1; // bit 32

  // field: dropStaggedVlans
  //  Drop or allow service VLANs tagged packets on this egress port. Will only
  //  drop packets that has exactly one VLAN tag. Must set moreThanOneVlans when
  //  this is used. \tabTwo{Allow S-VLANs.}{Drop S-VLANs.}
  uint64_t dropStaggedVlans       :  1; // bit 31

  // field: dropCtaggedVlans
  //  Drop or allow customer VLANs tagged packets on this egress port. Will only
  //  drop packets that has exactly one VLAN tag. Must set moreThanOneVlans when
  //  this is used. \tabTwo{Allow C-VLANs.}{Drop C-VLANs.}
  uint64_t dropCtaggedVlans       :  1; // bit 30

  // field: disabled
  //  Disabling this port. All packets to this port is dropped and
  //  \register{Egress Port Disabled Drop} is incremented. \tabTwo{All packets
  //  will be sent out.} {All packets will be dropped.}
  uint64_t disabled               :  1; // bit 29

  // field: pcp
  //  The PCP used in egress port VLAN push or swap operation if selected by
  //  \field{Egress Port Configuration}{pcpSel}.
  uint64_t pcp                    :  3; // bit 26 to 28

  // field: cfiDei
  //  The CFI/DEI used in egress port VLAN push or swap operation if selected by
  //  \field{Egress Port Configuration}{cfiDeiSel}.
  uint64_t cfiDei                 :  1; // bit 25

  // field: vid
  //  The VID used in egress port VLAN push or swap operation if selected by
  //  \field{Egress Port Configuration}{vidSel}.
  uint64_t vid                    : 12; // bit 13 to 24

  // field: pcpSel
  //  Selects which PCP to use when building a new VLAN header in a egress port
  //  push or swap operation. If the selected outermost VLAN header doesn't exist
  //  in the packet then this table entry's \field{Egress Port
  //  Configuration}{cfiDei} will be used. \tabThree{From outermost VLAN in the
  //  packet (if any).} {From this table entry's \field{Egress Port
  //  Configuration}{pcp}.} {From \register{Egress Queue To PCP And CFI/DEI
  //  Mapping Table}.}
  uint64_t pcpSel                 :  2; // bit 11 to 12

  // field: cfiDeiSel
  //  Selects which CFI/DEI to use when building a new VLAN header in a egress
  //  port push or swap operation. If the selected outermost VLAN header doesn't
  //  exist in the packet then this table entry's \field{Egress Port
  //  Configuration}{cfiDei} will be used. \tabThree{From outermost VLAN in the
  //  packet (if any).} {From this table entry's \field{Egress Port
  //  Configuration}{cfiDei}.} {From \register{Egress Queue To PCP And CFI/DEI
  //  Mapping Table}.}
  uint64_t cfiDeiSel              :  2; // bit 9 to 10

  // field: vidSel
  //  Selects which VID to use when building a new VLAN header in a egress port
  //  push or swap operation. If the selected outermost VLAN header doesn't exist
  //  in the packet then this table entry's \field{Egress Port Configuration}{vid}
  //  will be used. \tabThree{From outermost VLAN in the packet (if any).} {From
  //  this table entry's \field{Egress Port Configuration}{vid}.} {From the
  //  Ingress VID as selected in the \register{Source Port Table}.}
  uint64_t vidSel                 :  2; // bit 7 to 8

  // field: typeSel
  //  Selects which TPID to use when building a new VLAN header in a push or swap
  //  operation. \tabThree{C-VLAN - 0x8100.} {S-VLAN - 0x88A8.} {User defined VLAN
  //  type from register \register{Egress Ethernet Type for VLAN tag} field
  //  \field{Egress Ethernet Type for VLAN tag}{typeValue}.}
  uint64_t typeSel                :  2; // bit 5 to 6

  // field: removeSNAP
  //  If a packet which has SNAP/LLC encoding then remove it before sending out
  //  the packet on this egress port. \tabTwo{No. Keep it.}{Yes. Remove it.}
  uint64_t removeSNAP             :  1; // bit 4

  // field: vlanSingleOp
  //  The egress port VLAN operation to perform on the packet. \tabFive{No
  //  operation.} {Swap.} {Push.} {Pop.} {Penultimate pop(remove all VLAN
  //  headers).}
  uint64_t vlanSingleOp           :  3; // bit 1 to 3

  // field: colorRemap
  //  If set, color remapping to outgoing packet headers is allowed. The default
  //  color remapping options are based on the egress port number from the
  //  \register{Color Remap From Egress Port} table.
  //  \ifdef{\texIngressAdmissionControl}{If a packet is subjected to ingress
  //  admission control, its ingress admission control pointer can provide remap
  //  options from the \register{Color Remap From Ingress Admission Control} table
  //  to override default options.}{}
  uint64_t colorRemap             :  1; // bit 0
} t_EgressPortConfiguration;

// ------- Struct declaration for register: AHDecoderDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_AHDecoderDrop;

// ------- Struct declaration for register: EgressSpanningTreeDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_EgressSpanningTreeDrop;

// ------- Struct declaration for register: IngressConfigurableACL0TCAM --------- 
typedef struct {

  // field: compareData
  //  The data which shall be compared in this entry. Observe that this compare
  //  data must be AND:ed by software before the entry is searched. The hardware
  //  does not do the AND between mask and compareData (In order to save area).
  uint8_t compareData[42]; // bit 331 to 660

  // field: mask
  //  Which bits to compare in this entry.
  uint8_t mask[42]; // bit 1 to 330

  // field: valid
  //  Is this entry valid. \tabTwo{No}{Yes}
  uint64_t valid      :  1; // bit 0
} t_IngressConfigurableACL0TCAM;

// ------- Struct declaration for register: NATActionTableDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_NATActionTableDrop;

// ------- Struct declaration for register: ERMYellowConfiguration --------- 
typedef struct {

  // field: redPortXoff
  //  When the buffer memory congestion status is yellow and the total number of
  //  cells enqueued on an egress port is larger than this value, \field{Resource
  //  Limiter Set}{redLimit} check for that port will be invoked. Only valid when
  //  \field{ERM Yellow Configuration}{redPortEn} is turned on.
  uint64_t redPortXoff: 10; // bit 26 to 35

  // field: redPortEn
  //  When the buffer memory congestion status is yellow and a single port
  //  consumes more than \field{ERM Yellow Configuration}{redPortXoff} cells, this
  //  field can apply the \field{Resource Limiter Set}{redLimit} check on a per
  //  port basis.
  uint64_t redPortEn  :  6; // bit 20 to 25

  // field: yellowXon
  //  Once the yellow congestion check is applied, number of free cells need to go
  //  above this value to disable the check again. The value needs to be larger
  //  than \field{ERM Yellow Configuration}{yellowXoff} to provide an effective
  //  hysteresis.
  uint64_t yellowXon  : 10; // bit 10 to 19

  // field: yellowXoff
  //  Number of free cells below this value will invoke yellow congestion checks
  //  for the incoming cells. The checks include the number of enqueued cells in
  //  the current queue, higher priority queues and optionally the total number of
  //  enqueued cells for the current egress port. Incoming packets might be
  //  terminated and dropped based on the check result.
  uint64_t yellowXoff : 10; // bit 0 to 9
} t_ERMYellowConfiguration;

// ------- Struct declaration for register: L2TunnelDecoderSetup --------- 
typedef struct {

  // field: defaultEthSType
  //  A configurable Ethernet Type which shall be used to determine a S-Type VLAN.
  uint64_t defaultEthSType     : 16; // bit 18 to 33

  // field: defaultEthSTypeValid
  //  The configurable Ethernet Type S-type is valid.
  uint64_t defaultEthSTypeValid:  1; // bit 17

  // field: defaultEthCType
  //  A configurable Ethernet Type which shall be used to determine a C-Type VLAN.
  uint64_t defaultEthCType     : 16; // bit 1 to 16

  // field: defaultEthCTypeValid
  //  The configurable Ethernet Type C-type is valid.
  uint64_t defaultEthCTypeValid:  1; // bit 0
} t_L2TunnelDecoderSetup;

// ------- Struct declaration for register: L2AgingStatusShadowTable --------- 
typedef struct {

  // field: valid
  //  If this is set, then the corresponding hash table entry is valid.
  uint64_t valid:  1; // bit 0
} t_L2AgingStatusShadowTable;

// ------- Struct declaration for register: IPPDebugdoL2Lookup --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value:  1; // bit 0
} t_IPPDebugdoL2Lookup;

// ------- Struct declaration for register: TCXoffFFAThreshold --------- 
typedef struct {

  // field: trip
  //  \begin{fieldValues} \dscValue [0] Normal operation \dscValue [1] Force this
  //  threshold to be counted as exceeded \end{fieldValues} Only valid if this
  //  Xoff threshold is enabled.
  uint64_t trip  :  1; // bit 11

  // field: enable
  //  \begin{fieldValues} \dscValue [0] This Xoff threshold is disabled \dscValue
  //  [1] This Xoff threshold is enabled\end{fieldValues}
  uint64_t enable:  1; // bit 10

  // field: cells
  //  Xoff threshold for the number of used FFA cells for this traffic class
  uint64_t cells : 10; // bit 0 to 9
} t_TCXoffFFAThreshold;

// ------- Struct declaration for register: L2MulticastStormControlEnable --------- 
typedef struct {

  // field: enable
  //  Bitmask where the index is the Egress Ports
  uint64_t enable:  6; // bit 0 to 5
} t_L2MulticastStormControlEnable;

// ------- Struct declaration for register: EgressConfigurableACL1TCAM --------- 
typedef struct {

  // field: compareData
  //  The data which shall be compared in this entry. Observe that this compare
  //  data must be AND:ed by software before the entry is searched. The hardware
  //  does not do the AND between mask and compareData (In order to save area).
  uint8_t compareData[68]; // bit 541 to 1080

  // field: mask
  //  Which bits to compare in this entry.
  uint8_t mask[68]; // bit 1 to 540

  // field: valid
  //  Is this entry valid. \tabTwo{No}{Yes}
  uint64_t valid      :  1; // bit 0
} t_EgressConfigurableACL1TCAM;

// ------- Struct declaration for register: IngressConfigurableACL0SearchMask --------- 
typedef struct {

  // field: masklarge
  //  Which bits to compare in the \register{Ingress Configurable ACL 0 Large
  //  Table} lookup. A bit set to 1 means the corresponding bit in the search data
  //  is compared and 0 means the bit is ignored.
  uint8_t masklarge[42]; // bit 330 to 659

  // field: masksmall
  //  Which bits to compare in the \register{Ingress Configurable ACL 0 Small
  //  Table} lookup. A bit set to 1 means the corresponding bit in the search data
  //  is compared and 0 means the bit is ignored.
  uint8_t masksmall[42]; // bit 0 to 329
} t_IngressConfigurableACL0SearchMask;

// ------- Struct declaration for register: PortMoveOptions --------- 
typedef struct {

  // field: allowPortMoveOnStatic
  //  This field configures which source ports that are allowed to change their
  //  static GID and MAC to other ports. One bit for each port where bit 0
  //  corresponds to port 0. When the L2 forwarding information base identifies a
  //  GID, MAC SA and source port combination that conflicts with a existing
  //  static entry, if the previous binded port has a coressponding bit set to 1
  //  in this field, it allows the learning engine to update the GID and MAC to
  //  the current source port.
  uint64_t allowPortMoveOnStatic:  6; // bit 0 to 5
} t_PortMoveOptions;

// ------- Struct declaration for register: L2AgingTable --------- 
typedef struct {

  // field: hit
  //  If set, then the corresponding hash table entry has a L2 DA search hit since
  //  the last aging scan.
  uint64_t hit  :  1; // bit 2

  // field: stat
  //  If set, then the corresponding hash table entry will not be aged out.
  uint64_t stat :  1; // bit 1

  // field: valid
  //  If set, then the corresponding hash table entry is valid.
  uint64_t valid:  1; // bit 0
} t_L2AgingTable;

// ------- Struct declaration for register: IngressRouterTable --------- 
typedef struct {

  // field: mmpOrder
  //  Ingress MMP pointer order.
  uint64_t mmpOrder      :  2; // bit 27 to 28

  // field: mmpPtr
  //  Ingress MMP pointer.
  uint64_t mmpPtr        :  5; // bit 22 to 26

  // field: mmpValid
  //  If set, this entry contains a valid MMP pointer. Only valid when packets get
  //  routed
  uint64_t mmpValid      :  1; // bit 21

  // field: ecmpUseIpL4Dp
  //  Use TCP/UDP destination port as part of ECMP hash key.
  uint64_t ecmpUseIpL4Dp :  1; // bit 20

  // field: ecmpUseIpL4Sp
  //  Use TCP/UDP source port as part of ECMP hash key.
  uint64_t ecmpUseIpL4Sp :  1; // bit 19

  // field: ecmpUseIpProto
  //  Use IP Protocol/Next Header as part of ECMP hash key.
  uint64_t ecmpUseIpProto:  1; // bit 18

  // field: ecmpUseIpTos
  //  Use IP TOS/Traffic Class as part of ECMP hash key.
  uint64_t ecmpUseIpTos  :  1; // bit 17

  // field: ecmpUseIpSa
  //  Use IP source address as part of ECMP hash key.
  uint64_t ecmpUseIpSa   :  1; // bit 16

  // field: ecmpUseIpDa
  //  Use IP destination address as part of ECMP hash key.
  uint64_t ecmpUseIpDa   :  1; // bit 15

  // field: mplsHitUpdates
  //  Enable updates of the \register{Next Hop Hit Status} for routed MPLS
  //  packets. \tabTwo{Disable} {Enable}
  uint64_t mplsHitUpdates:  1; // bit 14

  // field: ipv6HitUpdates
  //  Enable updates of the \register{Next Hop Hit Status} for routed IPv6
  //  packets. \tabTwo{Disable} {Enable}
  uint64_t ipv6HitUpdates:  1; // bit 13

  // field: ipv4HitUpdates
  //  Enable updates of the \register{Next Hop Hit Status} for routed IPv4
  //  packets. \tabTwo{Disable} {Enable}
  uint64_t ipv4HitUpdates:  1; // bit 12

  // field: minTtlToCpu
  //  If this is set then packets below minimum TTL will be send to CPU instead of
  //  dropped.
  uint64_t minTtlToCpu   :  1; // bit 11

  // field: minTTL
  //  Minimum TTL. Packets with a TTL below this value will not be accepted. The
  //  packet will be dropped and the \register{Expired TTL Drop} counter
  //  incremented. If the \field{Ingress Router Table}{minTtlToCpu} is set the
  //  packet will be sent to CPU instead of being dropped. The TTL check is done
  //  for \ifdef{\texRoutingAndMpls}{ IPv4, IPv6 and MPLS }{}
  //  \ifdef{\texRoutingOnly}{ IPv4 and IPv6 }{} \ifdef{\texMplsOnly}{ MPLS }{}
  //  routed packets.
  uint64_t minTTL        :  8; // bit 3 to 10

  // field: acceptMPLS
  //  Accept MPLS packets. If disabled and an MPLS packet reaches the router the
  //  packet will be dropped and the \register{Invalid Routing Protocol Drop}
  //  incremented. \tabTwo{Deny} {Accept}
  uint64_t acceptMPLS    :  1; // bit 2

  // field: acceptIPv6
  //  Accept IPv6 packets. If disabled and an IPv6 packet reaches the router the
  //  packet will be dropped and the \register{Invalid Routing Protocol Drop}
  //  incremented. \tabTwo{Deny} {Accept}
  uint64_t acceptIPv6    :  1; // bit 1

  // field: acceptIPv4
  //  Accept IPv4 packets. If disabled and an IPv4 packet reaches the router the
  //  packet will be dropped and the \register{Invalid Routing Protocol Drop}
  //  incremented. \tabTwo{Deny} {Accept}
  uint64_t acceptIPv4    :  1; // bit 0
} t_IngressRouterTable;

// ------- Struct declaration for register: NextHopPacketInsertMPLSHeader --------- 
typedef struct {

  // field: exp3
  //  EXP table value for MPLS label 3.
  uint64_t exp3                 :  3; // bit 133 to 135

  // field: expFromQueue3
  //  Where shall the EXP come from in the MPLS label 3. \tabTwo{From this table,
  //  field exp3.}{From the \register{Egress Queue To MPLS EXP Mapping Table}.}
  uint64_t expFromQueue3        :  1; // bit 132

  // field: ttl3
  //  TTL table value for MPLS label 3.
  uint64_t ttl3                 :  8; // bit 124 to 131

  // field: copyTtl3
  //  Where shall the TTL come from in the MPLS label 3. \tabTwo{From this table,
  //  field ttl3.}{From the inner packet.}
  uint64_t copyTtl3             :  1; // bit 123

  // field: mplsLabel3
  //  MPLS label 3 to be inserter.
  uint64_t mplsLabel3           : 20; // bit 103 to 122

  // field: exp2
  //  EXP table value for MPLS label 2.
  uint64_t exp2                 :  3; // bit 100 to 102

  // field: expFromQueue2
  //  Where shall the EXP come from in the MPLS label 2. \tabTwo{From this table,
  //  field exp2.}{From the \register{Egress Queue To MPLS EXP Mapping Table}.}
  uint64_t expFromQueue2        :  1; // bit 99

  // field: ttl2
  //  TTL table value for MPLS label 2.
  uint64_t ttl2                 :  8; // bit 91 to 98

  // field: copyTtl2
  //  Where shall the TTL come from in the MPLS label 2. \tabTwo{From this table,
  //  field ttl2.}{From the inner packet.}
  uint64_t copyTtl2             :  1; // bit 90

  // field: mplsLabel2
  //  MPLS label 2 to be inserter.
  uint64_t mplsLabel2           : 20; // bit 70 to 89

  // field: exp1
  //  EXP table value for MPLS label 1.
  uint64_t exp1                 :  3; // bit 67 to 69

  // field: expFromQueue1
  //  Where shall the EXP come from in the MPLS label 1. \tabTwo{From this table,
  //  field exp1.}{From the \register{Egress Queue To MPLS EXP Mapping Table}.}
  uint64_t expFromQueue1        :  1; // bit 66

  // field: ttl1
  //  TTL table value for MPLS label 1.
  uint64_t ttl1                 :  8; // bit 58 to 65

  // field: copyTtl1
  //  Where shall the TTL come from in the MPLS label 1. \tabTwo{From this table,
  //  field ttl1.}{From the inner packet.}
  uint64_t copyTtl1             :  1; // bit 57

  // field: mplsLabel1
  //  MPLS label 1 to be inserter.
  uint64_t mplsLabel1           : 20; // bit 37 to 56

  // field: exp0
  //  EXP table value for MPLS label 0.
  uint64_t exp0                 :  3; // bit 34 to 36

  // field: expFromQueue0
  //  Where shall the EXP come from in the MPLS label 0. \tabTwo{From this table,
  //  field exp0.}{From the \register{Egress Queue To MPLS EXP Mapping Table}.}
  uint64_t expFromQueue0        :  1; // bit 33

  // field: ttl0
  //  TTL table value for MPLS label 0.
  uint64_t ttl0                 :  8; // bit 25 to 32

  // field: copyTtl0
  //  Where shall the TTL come from in the MPLS label 0. \tabTwo{From this table,
  //  field ttl0.}{From the inner packet.}
  uint64_t copyTtl0             :  1; // bit 24

  // field: mplsLabel0
  //  First/Outermost MPLS label to be inserter.
  uint64_t mplsLabel0           : 20; // bit 4 to 23

  // field: whichEthernetType
  //  Which Ethernet Type shall be used for these MPLS labels.
  //  \tabTwo{0x8847}{0x8848}
  uint64_t whichEthernetType    :  1; // bit 3

  // field: howManyLabelsToInsert
  //  How many labels shall be inserted. Setting a zero here means no labels will
  //  be added.
  uint64_t howManyLabelsToInsert:  3; // bit 0 to 2
} t_NextHopPacketInsertMPLSHeader;

// ------- Struct declaration for register: MACInterfaceCountersForTX --------- 
typedef struct {

  // field: halt
  //  Halt errors. Incremented if first, last or valid_bytes is non-zero when halt
  //  is high.
  uint64_t halt   : 32; // bit 64 to 95

  // field: error
  //  Bus protocol errors.
  uint64_t error  : 32; // bit 32 to 63

  // field: packets
  //  Correct packets completed
  uint64_t packets: 32; // bit 0 to 31
} t_MACInterfaceCountersForTX;

// ------- Struct declaration for register: TunnelEntryInstructionTable --------- 
typedef struct {

  // field: tunnelHeaderLen
  //  The length of the tunnel header, in bytes, to insert from register
  //  \register{Tunnel Entry Header Data}.
  uint64_t tunnelHeaderLen :  7; // bit 43 to 49

  // field: tunnelHeaderPtr
  //  Points to which header to insert from register \register{Tunnel Entry Header
  //  Data}.
  uint64_t tunnelHeaderPtr :  4; // bit 39 to 42

  // field: incVlansInLength
  //  Should the outgoing packets number of VLANs be included in the length
  //  calculation? \tabTwo{No.}{Yes.}
  uint64_t incVlansInLength:  1; // bit 38

  // field: lengthPosOffset
  //  How much shall be incremented from the total packet (frame) length.
  uint64_t lengthPosOffset : 14; // bit 24 to 37

  // field: lengthNegOffset
  //  How much shall be decremented from the total packet (frame) length.
  uint64_t lengthNegOffset : 14; // bit 10 to 23

  // field: lengthPos
  //  If length shall be inserted , where shall it be inserted. A value of 0 means
  //  beginning of tunnel entry data.
  uint64_t lengthPos       :  7; // bit 3 to 9

  // field: insertLength
  //  Insert the a packet length fields. The 2 byte length of the frame will
  //  overwrite current 2 bytes in the header data to be inserted at \field{Tunnel
  //  Entry Instruction Table}{lengthPos}.\tabTwo{Yes. Insert a length field.}{No.
  //  Don't insert a length field.}
  uint64_t insertLength    :  1; // bit 2

  // field: tunnelEntryType
  //  A tunnel entry shall be done. Where shall the tunnel entry be done
  //  \tabFour{At Byte Zero described in \register{Beginning of Packet Tunnel
  //  Entry Instruction Table} }{After L2 and up to two VLAN headers. described in
  //  \register{L2 Tunnel Entry Instruction Table}}{After L3 IPv4/IPv6/MPLS
  //  headers.}{Reserved.}
  uint64_t tunnelEntryType :  2; // bit 0 to 1
} t_TunnelEntryInstructionTable;

// ------- Struct declaration for register: IPUnicastRoutedCounter --------- 
typedef struct {

  // field: packets
  //  Number of packets.
  uint64_t packets: 32; // bit 0 to 31
} t_IPUnicastRoutedCounter;

// ------- Struct declaration for register: EPPDebugomEnabled --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value:  1; // bit 0
} t_EPPDebugomEnabled;

// ------- Struct declaration for register: IngressAdmissionControlCurrentSize --------- 
typedef struct {

  // field: tokens1
  //  Number of tokens after the last visit for token bucket 1.
  uint64_t tokens1: 16; // bit 16 to 31

  // field: tokens0
  //  Number of tokens after the last visit for token bucket 0.
  uint64_t tokens0: 16; // bit 0 to 15
} t_IngressAdmissionControlCurrentSize;

// ------- Struct declaration for register: L2DAHashLookupTable --------- 
typedef struct {

  // field: gid
  //  Global identifier from the VLAN Table.
  uint64_t gid    : 12; // bit 48 to 59

  // field: macAddr
  //  MAC address.
  uint64_t macAddr: 48; // bit 0 to 47
} t_L2DAHashLookupTable;

// ------- Struct declaration for register: DebugCounternrVlansSetup --------- 
typedef struct {

  // field: hitValue
  //  Value to compare to update debug counter. Both the incoming value and this
  //  value is ANDed with the mask before comparsion is carried out. If comparsion
  //  results in true the counter is updated
  uint64_t hitValue:  2; // bit 2 to 3

  // field: mask
  //  Mask for comparison to update debug counter.
  uint64_t mask    :  2; // bit 0 to 1
} t_DebugCounternrVlansSetup;

// ------- Struct declaration for register: EPPPacketHeadCounter --------- 
typedef struct {

  // field: packets
  //  Number of packet headers.
  uint64_t packets: 32; // bit 0 to 31
} t_EPPPacketHeadCounter;

// ------- Struct declaration for register: IPPPacketTailCounter --------- 
typedef struct {

  // field: packets
  //  Number of packet tails.
  uint64_t packets: 32; // bit 0 to 31
} t_IPPPacketTailCounter;

// ------- Struct declaration for register: IPMulticastReceivedCounter --------- 
typedef struct {

  // field: packets
  //  Number of packets.
  uint64_t packets: 32; // bit 0 to 31
} t_IPMulticastReceivedCounter;

// ------- Struct declaration for register: DNSPacketDecoderOptions --------- 
typedef struct {

  // field: toCpu
  //  If a packet comes in on this source port then send the packet to the CPU
  //  port.\tabTwo{Do not sent to CPU. Normal Processing of packet.}{Send to CPU ,
  //  bypass normal packet processing.}
  uint64_t toCpu  :  6; // bit 23 to 28

  // field: drop
  //  If a packet comes in on this source port then drop the packet.\tabTwo{Do not
  //  drop this packet.}{Drop this packet and update the drop counter.}
  uint64_t drop   :  6; // bit 17 to 22

  // field: l4Port
  //  The value to be used to find this packet type.
  uint64_t l4Port : 16; // bit 1 to 16

  // field: enabled
  //  Is this decoding enabled. \tabTwo{No}{Yes}
  uint64_t enabled:  1; // bit 0
} t_DNSPacketDecoderOptions;

// ------- Struct declaration for register: TCFFAUsed --------- 
typedef struct {

  // field: cells
  //  Number of cells
  uint64_t cells: 10; // bit 0 to 9
} t_TCFFAUsed;

// ------- Struct declaration for register: EgressNATOperation --------- 
typedef struct {

  // field: port
  //  The new L4 Port.
  uint64_t port         : 16; // bit 35 to 50

  // field: ipAddress
  //  The new IP Address.
  uint64_t ipAddress    : 32; // bit 3 to 34

  // field: replaceL4Port
  //  Replace TCP/UDP port. \tabTwo{No.}{Yes.}
  uint64_t replaceL4Port:  1; // bit 2

  // field: replaceIP
  //  Replace IP address. \tabTwo{No.}{Yes.}
  uint64_t replaceIP    :  1; // bit 1

  // field: replaceSrc
  //  Replace Source or Destination. \tabTwo{Destination}{Source}
  uint64_t replaceSrc   :  1; // bit 0
} t_EgressNATOperation;

// ------- Struct declaration for register: EPPDebugdelSpecificVlan --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value:  1; // bit 0
} t_EPPDebugdelSpecificVlan;

// ------- Struct declaration for register: IngressConfigurableACL1TCAMAnswer --------- 
typedef struct {

  // field: forceVidPrio
  //  If multiple forceVid are set and this prio bit is set then this forceVid
  //  value will be selected.
  uint64_t forceVidPrio         :  1; // bit 184

  // field: forceVid
  //  The new Ingress VID.
  uint64_t forceVid             : 12; // bit 172 to 183

  // field: forceVidValid
  //  Override the Ingress VID, see chapter \hyperref[chap:VLAN Processing]{VLAN
  //  Processing}.
  uint64_t forceVidValid        :  1; // bit 171

  // field: forceQueuePrio
  //  If multiple forceQueue are set and this prio bit is set then this forceQueue
  //  value will be selected.
  uint64_t forceQueuePrio       :  1; // bit 170

  // field: eQueue
  //  The egress queue to be assigned if the forceQueue field in this entry is set
  //  to 1.
  uint64_t eQueue               :  3; // bit 167 to 169

  // field: forceQueue
  //  If set, the packet shall have a forced egress queue. Please see Egress Queue
  //  Selection Diagram in Figure \ref{fig:determineEgressQueue}
  uint64_t forceQueue           :  1; // bit 166

  // field: mmpOrder
  //  Ingress MMP pointer order.
  uint64_t mmpOrder             :  2; // bit 164 to 165

  // field: mmpPtr
  //  Ingress MMP pointer.
  uint64_t mmpPtr               :  5; // bit 159 to 163

  // field: mmpValid
  //  If set, this entry contains a valid MMP pointer
  uint64_t mmpValid             :  1; // bit 158

  // field: forceColorPrio
  //  If multiple forceColor are set and this prio bit is set then this forceVid
  //  value will be selected.
  uint64_t forceColorPrio       :  1; // bit 157

  // field: color
  //  Initial color of the packet if the forceColor field is set.
  uint64_t color                :  2; // bit 155 to 156

  // field: forceColor
  //  If set, the packet shall have a forced color.
  uint64_t forceColor           :  1; // bit 154

  // field: tunnelEntryPrio
  //  If multiple tunnelEntry are set and this prio bit is set then this
  //  tunnelEntryPtr will be selected.
  uint64_t tunnelEntryPrio      :  1; // bit 153

  // field: tunnelEntryPtr
  //  The tunnel entry which this packet shall enter upon exiting the switch.
  uint64_t tunnelEntryPtr       :  4; // bit 149 to 152

  // field: tunnelEntryUcMc
  //  Shall this entry point to the \register{Tunnel Entry Instruction Table} with
  //  or without a egress port offset. \tabTwo{Unicast \register{Tunnel Entry
  //  Instruction Table} without offset for each port}{Multicast \register{Tunnel
  //  Entry Instruction Table} with offset for each port.}
  uint64_t tunnelEntryUcMc      :  1; // bit 148

  // field: tunnelEntry
  //  Shall all of these packets enter into a tunnel.
  uint64_t tunnelEntry          :  1; // bit 147

  // field: ptp
  //  When the packet is sent to the CPU the packet will have the PTP bit in the
  //  To CPU Tag set to one. The timestamp in the To CPU Tag will also be set to
  //  the timestamp from the incoming packet.
  uint64_t ptp                  :  1; // bit 146

  // field: natOpPrio
  //  If multiple natOpValid are set and this prio bit is set then this natOpPtr
  //  value will be selected.
  uint64_t natOpPrio            :  1; // bit 145

  // field: natOpPtr
  //  NAT operation pointer.
  uint64_t natOpPtr             : 11; // bit 134 to 144

  // field: natOpValid
  //  NAT operation pointer is valid.
  uint64_t natOpValid           :  1; // bit 133

  // field: newL4Value
  //  Update the L4 SP or DP with this value
  uint64_t newL4Value           : 16; // bit 117 to 132

  // field: updateL4SpOrDp
  //  Update the source or destination L4 port. \tabTwo{Source L4
  //  Port}{Destination L4 Port}
  uint64_t updateL4SpOrDp       :  1; // bit 116

  // field: enableUpdateL4
  //  If this entry is hit then update L4 Source Port or Destination port in
  //  ingress packet processing, this value will be used in the Egress ACL.
  //  \tabTwo{Disable}{Enable}
  uint64_t enableUpdateL4       :  1; // bit 115

  // field: newIpValue
  //  Update the SA or DA IPv4 address value.
  uint64_t newIpValue           : 32; // bit 83 to 114

  // field: updateSaOrDa
  //  Update the SA or DA IPv4 address. The Destiantion IP address updated will be
  //  used in the routing functionality and Egress ACL functionality. If the
  //  source IP address is updated then the updated value will be used in the
  //  egress ACL keys. \tabTwo{Source IP Address}{Destination IP Address}
  uint64_t updateSaOrDa         :  1; // bit 82

  // field: enableUpdateIp
  //  If this entry is hit then update SA or DA IPv4 address in ingress packet
  //  processing, this value will be used by the routing function and egress ACL
  //  if this is exists, this only works for IPv4. \tabTwo{Disable}{Enable}
  uint64_t enableUpdateIp       :  1; // bit 81

  // field: ethPrio
  //  If multiple updateEType are set and this prio bit is set then this
  //  updateEType will be selected.
  uint64_t ethPrio              :  1; // bit 80

  // field: vidPrio
  //  If multiple updateVid are set and this prio bit is set then this updateVid
  //  will be selected.
  uint64_t vidPrio              :  1; // bit 79

  // field: pcpPrio
  //  If multiple updatePcp are set and this prio bit is set then this updatePcp
  //  will be selected.
  uint64_t pcpPrio              :  1; // bit 78

  // field: cfiDeiPrio
  //  If multiple updateCfiDei are set and this prio bit is set then this
  //  updateCfiDei will be selected.
  uint64_t cfiDeiPrio           :  1; // bit 77

  // field: newEthType
  //  Select which TPID to use in the outer VLAN header. \tabThree{C-VLAN -
  //  0x8100.} {S-VLAN - 0x88A8.} {User defined VLAN type from register
  //  \register{Egress Ethernet Type for VLAN tag}.}
  uint64_t newEthType           :  2; // bit 75 to 76

  // field: updateEType
  //  The VLANs TPID type should be updated. \tabTwo{Do not update the
  //  TPID.}{Update the TPID.}
  uint64_t updateEType          :  1; // bit 74

  // field: newVidValue
  //  The VID value to update to.
  uint64_t newVidValue          : 12; // bit 62 to 73

  // field: updateVid
  //  The VID value of the packets outermost VLAN should be updated. \tabTwo{Do
  //  not update the value.}{Update the value.}
  uint64_t updateVid            :  1; // bit 61

  // field: newPcpValue
  //  The PCP value to update to.
  uint64_t newPcpValue          :  3; // bit 58 to 60

  // field: updatePcp
  //  The PCP value of the packets outermost VLAN should be updated. \tabTwo{Do
  //  not update the value.}{Update the value.}
  uint64_t updatePcp            :  1; // bit 57

  // field: newCfiDeiValue
  //  The value to update to.
  uint64_t newCfiDeiValue       :  1; // bit 56

  // field: updateCfiDei
  //  The CFI/DEI value of the packets outermost VLAN should be updated.\tabTwo{Do
  //  not update the value.}{Update the value.}
  uint64_t updateCfiDei         :  1; // bit 55

  // field: tosMask
  //  Mask for TOS value. Setting a bit to one means this bit will be selected
  //  from the newTosExp field , while setting this bit to zero means that the bit
  //  will be selected from the packets already existing TOS byte bit.
  uint64_t tosMask              :  8; // bit 47 to 54

  // field: newTosExp
  //  New TOS/EXP value.
  uint64_t newTosExp            :  8; // bit 39 to 46

  // field: updateTosExp
  //  Force TOS/EXP update.
  uint64_t updateTosExp         :  1; // bit 38

  // field: counter
  //  Which counter in \register{Ingress Configurable ACL Match Counter} to
  //  update.
  uint64_t counter              :  6; // bit 32 to 37

  // field: updateCounter
  //  When set the selected statistics counter will be updated.
  uint64_t updateCounter        :  1; // bit 31

  // field: noLearning
  //  If set this packets MAC SA will not be learned.
  uint64_t noLearning           :  1; // bit 30

  // field: imPrio
  //  If multiple input mirror are set and this prio bit is set then this input
  //  mirror will be selected.
  uint64_t imPrio               :  1; // bit 29

  // field: destInputMirror
  //  Destination \ifdef{\texLinkAgg}{physical}{} port for input mirroring.
  uint64_t destInputMirror      :  3; // bit 26 to 28

  // field: inputMirror
  //  If set, input mirroring is enabled for this rule. In addition to the normal
  //  processing of the packet a copy of the unmodified input packet will be send
  //  to the destination Input Mirror port and exit on that port. The copy will be
  //  subject to the normal resource limitations in the switch.
  uint64_t inputMirror          :  1; // bit 25

  // field: destPort
  //  The port which the packet shall be sent to.
  uint64_t destPort             :  3; // bit 22 to 24

  // field: sendToPort
  //  Send the packet to a specific port. \tabTwo{Disabled.}{Send to port
  //  configured in destPort.}
  uint64_t sendToPort           :  1; // bit 21

  // field: dropEnable
  //  If set, the packet shall be dropped and the \register{Ingress Configurable
  //  ACL Drop} counter is incremented.
  uint64_t dropEnable           :  1; // bit 20

  // field: metaDataPrio
  //  If multiple ACLs hit this meta_data shall take priority.
  uint64_t metaDataPrio         :  1; // bit 19

  // field: metaData
  //  Meta data for packets going to the CPU.
  uint64_t metaData             : 16; // bit 3 to 18

  // field: metaDataValid
  //  Is the meta_data field valid.
  uint64_t metaDataValid        :  1; // bit 2

  // field: forceSendToCpuOrigPkt
  //  If packet shall be sent to CPU then setting this bit will force the packet
  //  to be the incoming originial packet. \ifdef{\texTunneling}{The exception to
  //  this is rule is the tunnel exit which will still be carried out.}{}.
  uint64_t forceSendToCpuOrigPkt:  1; // bit 1

  // field: sendToCpu
  //  If set, the packet shall be sent to the CPU port.
  uint64_t sendToCpu            :  1; // bit 0
} t_IngressConfigurableACL1TCAMAnswer;

// ------- Struct declaration for register: IngressConfigurableACL3TCAMAnswer --------- 
typedef struct {

  // field: forceQueuePrio
  //  If multiple forceQueue are set and this prio bit is set then this forceQueue
  //  value will be selected.
  uint64_t forceQueuePrio       :  1; // bit 41

  // field: eQueue
  //  The egress queue to be assigned if the forceQueue field in this entry is set
  //  to 1.
  uint64_t eQueue               :  3; // bit 38 to 40

  // field: forceQueue
  //  If set, the packet shall have a forced egress queue. Please see Egress Queue
  //  Selection Diagram in Figure \ref{fig:determineEgressQueue}
  uint64_t forceQueue           :  1; // bit 37

  // field: mmpOrder
  //  Ingress MMP pointer order.
  uint64_t mmpOrder             :  2; // bit 35 to 36

  // field: mmpPtr
  //  Ingress MMP pointer.
  uint64_t mmpPtr               :  5; // bit 30 to 34

  // field: mmpValid
  //  If set, this entry contains a valid MMP pointer
  uint64_t mmpValid             :  1; // bit 29

  // field: forceColorPrio
  //  If multiple forceColor are set and this prio bit is set then this forceVid
  //  value will be selected.
  uint64_t forceColorPrio       :  1; // bit 28

  // field: color
  //  Initial color of the packet if the forceColor field is set.
  uint64_t color                :  2; // bit 26 to 27

  // field: forceColor
  //  If set, the packet shall have a forced color.
  uint64_t forceColor           :  1; // bit 25

  // field: destPort
  //  The port which the packet shall be sent to.
  uint64_t destPort             :  3; // bit 22 to 24

  // field: sendToPort
  //  Send the packet to a specific port. \tabTwo{Disabled.}{Send to port
  //  configured in destPort.}
  uint64_t sendToPort           :  1; // bit 21

  // field: dropEnable
  //  If set, the packet shall be dropped and the \register{Ingress Configurable
  //  ACL Drop} counter is incremented.
  uint64_t dropEnable           :  1; // bit 20

  // field: metaDataPrio
  //  If multiple ACLs hit this meta_data shall take priority.
  uint64_t metaDataPrio         :  1; // bit 19

  // field: metaData
  //  Meta data for packets going to the CPU.
  uint64_t metaData             : 16; // bit 3 to 18

  // field: metaDataValid
  //  Is the meta_data field valid.
  uint64_t metaDataValid        :  1; // bit 2

  // field: forceSendToCpuOrigPkt
  //  If packet shall be sent to CPU then setting this bit will force the packet
  //  to be the incoming originial packet. \ifdef{\texTunneling}{The exception to
  //  this is rule is the tunnel exit which will still be carried out.}{}.
  uint64_t forceSendToCpuOrigPkt:  1; // bit 1

  // field: sendToCpu
  //  If set, the packet shall be sent to the CPU port.
  uint64_t sendToCpu            :  1; // bit 0
} t_IngressConfigurableACL3TCAMAnswer;

// ------- Struct declaration for register: TunnelExitLookupTCAMAnswer --------- 
typedef struct {

  // field: tabIndex
  //  Index to be used in second tunnel exit dleft lookup. This is used in
  //  conjunciton with the key extracted from this table or from packet data.
  uint64_t tabIndex         :  2; // bit 177 to 178

  // field: lookupMask
  //  Mask for second tunnel exit lookup data. Before the lookup in the second
  //  lookup takes place this value from first lookup/packet data is AND:ed with
  //  this value.
  uint8_t lookupMask[10]; // bit 97 to 176

  // field: key
  //  Direct Value to use in instead of value from packet.
  uint8_t key[10]; // bit 17 to 96

  // field: direct
  //  Use direct value in this table in the Second Tunnel Exit Lookup Table
  //  Lookup. \tabTwo{False}{True}
  uint64_t direct           :  1; // bit 16

  // field: secondIncludeVlan
  //  Shall second tunnel exit lookup shift be updated according to how many VLANs
  //  the packet has?\tabTwo{No}{Yes}
  uint64_t secondIncludeVlan:  1; // bit 15

  // field: secondShift
  //  Second tunnel exit lookup shift to get the data for the second lookup, this
  //  value is in number of bytes, this value can at maximum be 154.
  uint64_t secondShift      :  8; // bit 7 to 14

  // field: sendToCpu
  //  This packet shall be sent to the CPU.\tabTwo{No.}{Yes.}
  uint64_t sendToCpu        :  1; // bit 6

  // field: srcPortMask
  //  Which source ports shall this tunnel exit be done on? The portmask which has
  //  one bit per source port.\tabTwo{No, do not do tunnel exit}{Yes, if second
  //  tunnel lookup is a hit then do tunnel exit.}
  uint64_t srcPortMask      :  6; // bit 0 to 5
} t_TunnelExitLookupTCAMAnswer;

// ------- Struct declaration for register: ReservedSourceMACAddressRange --------- 
typedef struct {

  // field: enable
  //  Enable the reserved source MAC check per source port. One bit for each port
  //  where bit 0 corresponds to port 0. If a bit is set to one, the reserved
  //  source MAC range is activated for that source port.
  uint64_t enable    :  6; // bit 113 to 118

  // field: mmpOrder
  //  Ingress MMP pointer order.
  uint64_t mmpOrder  :  2; // bit 111 to 112

  // field: mmpPtr
  //  Ingress MMP pointer.
  uint64_t mmpPtr    :  5; // bit 106 to 110

  // field: mmpValid
  //  If set, this entry contains a valid MMP pointer
  uint64_t mmpValid  :  1; // bit 105

  // field: forceColor
  //  If set, the packet shall have a forced color.
  uint64_t forceColor:  1; // bit 104

  // field: color
  //  Inital color of the packet.
  uint64_t color     :  2; // bit 102 to 103

  // field: eQueue
  //  The egress queue to be assigned if the forceQueue field in this entry is set
  //  to 1.
  uint64_t eQueue    :  3; // bit 99 to 101

  // field: forceQueue
  //  If set, the packet shall have a forced egress queue. Please see Egress Queue
  //  Selection Diagram in Figure \ref{fig:determineEgressQueue}
  uint64_t forceQueue:  1; // bit 98

  // field: sendToCpu
  //  If the MAC address was within the range the packet shall be sent to the CPU.
  uint64_t sendToCpu :  1; // bit 97

  // field: dropEnable
  //  If the MAC address was within the range the packet shall be dropped and the
  //  \register{Reserved MAC SA Drop} counter incremented.
  uint64_t dropEnable:  1; // bit 96

  // field: stopAddr
  //  The end MAC address of the range. A packets source MAC address must be equal
  //  or less than this value to match the range.
  uint64_t stopAddr  : 48; // bit 48 to 95

  // field: startAddr
  //  The start MAC address of the range. A packets source MAC address must be
  //  equal or greater than this value to match the range.
  uint64_t startAddr : 48; // bit 0 to 47
} t_ReservedSourceMACAddressRange;

// ------- Struct declaration for register: L2FloodingStormControlEnable --------- 
typedef struct {

  // field: enable
  //  Bitmask where the index is the Egress Ports
  uint64_t enable:  6; // bit 0 to 5
} t_L2FloodingStormControlEnable;

// ------- Struct declaration for register: QueueOffDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_QueueOffDrop;

// ------- Struct declaration for register: IPPDebugnextHopPtrLpm --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value: 10; // bit 0 to 9
} t_IPPDebugnextHopPtrLpm;

// ------- Struct declaration for register: L4IEEE1588DecoderDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_L4IEEE1588DecoderDrop;

// ------- Struct declaration for register: TunnelEntryHeaderData --------- 
typedef struct {

  // field: data
  //  Tunnel header data (bytes) to be inserted at tunnel entry point in
  //  packet.Byte 0 is the start of the tunnel header.
  uint8_t data[80]; // bit 0 to 639
} t_TunnelEntryHeaderData;

// ------- Struct declaration for register: QueueShaperBucketThresholdConfiguration --------- 
typedef struct {

  // field: threshold
  //  Minimum number of tokens in bucket for the status to be set to accept.
  uint64_t threshold: 17; // bit 0 to 16
} t_QueueShaperBucketThresholdConfiguration;

// ------- Struct declaration for register: IngressConfigurableACL0TCAMAnswer --------- 
typedef struct {

  // field: forceQueuePrio
  //  If multiple forceQueue are set and this prio bit is set then this forceQueue
  //  value will be selected.
  uint64_t forceQueuePrio       :  1; // bit 135

  // field: eQueue
  //  The egress queue to be assigned if the forceQueue field in this entry is set
  //  to 1.
  uint64_t eQueue               :  3; // bit 132 to 134

  // field: forceQueue
  //  If set, the packet shall have a forced egress queue. Please see Egress Queue
  //  Selection Diagram in Figure \ref{fig:determineEgressQueue}
  uint64_t forceQueue           :  1; // bit 131

  // field: mmpOrder
  //  Ingress MMP pointer order.
  uint64_t mmpOrder             :  2; // bit 129 to 130

  // field: mmpPtr
  //  Ingress MMP pointer.
  uint64_t mmpPtr               :  5; // bit 124 to 128

  // field: mmpValid
  //  If set, this entry contains a valid MMP pointer
  uint64_t mmpValid             :  1; // bit 123

  // field: forceColorPrio
  //  If multiple forceColor are set and this prio bit is set then this forceVid
  //  value will be selected.
  uint64_t forceColorPrio       :  1; // bit 122

  // field: color
  //  Initial color of the packet if the forceColor field is set.
  uint64_t color                :  2; // bit 120 to 121

  // field: forceColor
  //  If set, the packet shall have a forced color.
  uint64_t forceColor           :  1; // bit 119

  // field: natOpPrio
  //  If multiple natOpValid are set and this prio bit is set then this natOpPtr
  //  value will be selected.
  uint64_t natOpPrio            :  1; // bit 118

  // field: natOpPtr
  //  NAT operation pointer.
  uint64_t natOpPtr             : 11; // bit 107 to 117

  // field: natOpValid
  //  NAT operation pointer is valid.
  uint64_t natOpValid           :  1; // bit 106

  // field: newL4Value
  //  Update the L4 SP or DP with this value
  uint64_t newL4Value           : 16; // bit 90 to 105

  // field: updateL4SpOrDp
  //  Update the source or destination L4 port. \tabTwo{Source L4
  //  Port}{Destination L4 Port}
  uint64_t updateL4SpOrDp       :  1; // bit 89

  // field: enableUpdateL4
  //  If this entry is hit then update L4 Source Port or Destination port in
  //  ingress packet processing, this value will be used in the Egress ACL.
  //  \tabTwo{Disable}{Enable}
  uint64_t enableUpdateL4       :  1; // bit 88

  // field: newIpValue
  //  Update the SA or DA IPv4 address value.
  uint64_t newIpValue           : 32; // bit 56 to 87

  // field: updateSaOrDa
  //  Update the SA or DA IPv4 address. The Destiantion IP address updated will be
  //  used in the routing functionality and Egress ACL functionality. If the
  //  source IP address is updated then the updated value will be used in the
  //  egress ACL keys. \tabTwo{Source IP Address}{Destination IP Address}
  uint64_t updateSaOrDa         :  1; // bit 55

  // field: enableUpdateIp
  //  If this entry is hit then update SA or DA IPv4 address in ingress packet
  //  processing, this value will be used by the routing function and egress ACL
  //  if this is exists, this only works for IPv4. \tabTwo{Disable}{Enable}
  uint64_t enableUpdateIp       :  1; // bit 54

  // field: tosMask
  //  Mask for TOS value. Setting a bit to one means this bit will be selected
  //  from the newTosExp field , while setting this bit to zero means that the bit
  //  will be selected from the packets already existing TOS byte bit.
  uint64_t tosMask              :  8; // bit 46 to 53

  // field: newTosExp
  //  New TOS/EXP value.
  uint64_t newTosExp            :  8; // bit 38 to 45

  // field: updateTosExp
  //  Force TOS/EXP update.
  uint64_t updateTosExp         :  1; // bit 37

  // field: counter
  //  Which counter in \register{Ingress Configurable ACL Match Counter} to
  //  update.
  uint64_t counter              :  6; // bit 31 to 36

  // field: updateCounter
  //  When set the selected statistics counter will be updated.
  uint64_t updateCounter        :  1; // bit 30

  // field: imPrio
  //  If multiple input mirror are set and this prio bit is set then this input
  //  mirror will be selected.
  uint64_t imPrio               :  1; // bit 29

  // field: destInputMirror
  //  Destination \ifdef{\texLinkAgg}{physical}{} port for input mirroring.
  uint64_t destInputMirror      :  3; // bit 26 to 28

  // field: inputMirror
  //  If set, input mirroring is enabled for this rule. In addition to the normal
  //  processing of the packet a copy of the unmodified input packet will be send
  //  to the destination Input Mirror port and exit on that port. The copy will be
  //  subject to the normal resource limitations in the switch.
  uint64_t inputMirror          :  1; // bit 25

  // field: destPort
  //  The port which the packet shall be sent to.
  uint64_t destPort             :  3; // bit 22 to 24

  // field: sendToPort
  //  Send the packet to a specific port. \tabTwo{Disabled.}{Send to port
  //  configured in destPort.}
  uint64_t sendToPort           :  1; // bit 21

  // field: dropEnable
  //  If set, the packet shall be dropped and the \register{Ingress Configurable
  //  ACL Drop} counter is incremented.
  uint64_t dropEnable           :  1; // bit 20

  // field: metaDataPrio
  //  If multiple ACLs hit this meta_data shall take priority.
  uint64_t metaDataPrio         :  1; // bit 19

  // field: metaData
  //  Meta data for packets going to the CPU.
  uint64_t metaData             : 16; // bit 3 to 18

  // field: metaDataValid
  //  Is the meta_data field valid.
  uint64_t metaDataValid        :  1; // bit 2

  // field: forceSendToCpuOrigPkt
  //  If packet shall be sent to CPU then setting this bit will force the packet
  //  to be the incoming originial packet. \ifdef{\texTunneling}{The exception to
  //  this is rule is the tunnel exit which will still be carried out.}{}.
  uint64_t forceSendToCpuOrigPkt:  1; // bit 1

  // field: sendToCpu
  //  If set, the packet shall be sent to the CPU port.
  uint64_t sendToCpu            :  1; // bit 0
} t_IngressConfigurableACL0TCAMAnswer;

// ------- Struct declaration for register: EgressConfigurableACLMatchCounter --------- 
typedef struct {

  // field: packets
  //  Number of packets.
  uint64_t packets: 32; // bit 0 to 31
} t_EgressConfigurableACLMatchCounter;

// ------- Struct declaration for register: AgingDataFIFOHighWatermarkLevel --------- 
typedef struct {

  // field: level
  //  Number of used entries.
  uint64_t level:  6; // bit 0 to 5
} t_AgingDataFIFOHighWatermarkLevel;

// ------- Struct declaration for register: LearningOverflow --------- 
typedef struct {

  // field: port
  //  Port number.
  uint64_t port   :  3; // bit 61 to 63

  // field: gid
  //  Global identifier from the VLAN Table.
  uint64_t gid    : 12; // bit 49 to 60

  // field: macAddr
  //  MAC address.
  uint64_t macAddr: 48; // bit 1 to 48

  // field: valid
  //  Indicates hardware has written a learning overflow to this status register,
  //  Write 0 to clear.
  uint64_t valid  :  1; // bit 0
} t_LearningOverflow;

// ------- Struct declaration for register: EgressMPLSDecodingOptions --------- 
typedef struct {

  // field: nibbleForIpv4
  //  The nibble value which is used to identify a IPv4 packet after a MPLS
  //  header. If the nibble does not match this value it is assumed to be an IPv6
  //  packet.
  uint64_t nibbleForIpv4:  4; // bit 0 to 3
} t_EgressMPLSDecodingOptions;

// ------- Struct declaration for register: IngressVIDOuterVIDRangeSearchData --------- 
typedef struct {

  // field: end
  //  End of VID range.
  uint64_t end  : 12; // bit 19 to 30

  // field: start
  //  Start of VID range.
  uint64_t start: 12; // bit 7 to 18

  // field: vtype
  //  Shall this entry match S-Type or C-Type VLAN. \tabTwo{C-Type}{S-Type}
  uint64_t vtype:  1; // bit 6

  // field: ports
  //  Ports that this range search is activated on.
  uint64_t ports:  6; // bit 0 to 5
} t_IngressVIDOuterVIDRangeSearchData;

// ------- Struct declaration for register: EgressVLANTranslationTCAMAnswer --------- 
typedef struct {

  // field: ethType
  //  The new Ethernet Type for the outgoing packet
  uint64_t ethType: 16; // bit 12 to 27

  // field: newVid
  //  The new VID for the outgoing packet.
  uint64_t newVid : 12; // bit 0 to 11
} t_EgressVLANTranslationTCAMAnswer;

// ------- Struct declaration for register: SCTPDecoderDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_SCTPDecoderDrop;

// ------- Struct declaration for register: DebugCounterl2DaTcamHitsAndCastSetup --------- 
typedef struct {

  // field: hitValue
  //  Value to compare to update debug counter. Both the incoming value and this
  //  value is ANDed with the mask before comparsion is carried out. If comparsion
  //  results in true the counter is updated
  uint64_t hitValue: 15; // bit 15 to 29

  // field: mask
  //  Mask for comparison to update debug counter.
  uint64_t mask    : 15; // bit 0 to 14
} t_DebugCounterl2DaTcamHitsAndCastSetup;

// ------- Struct declaration for register: IPPDebugl2DaHash --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value: 10; // bit 0 to 9
} t_IPPDebugl2DaHash;

// ------- Struct declaration for register: L2ActionTableSourcePort --------- 
typedef struct {

  // field: mmpOrder
  //  Ingress MMP pointer order.
  uint64_t mmpOrder             :  2; // bit 17 to 18

  // field: mmpPtr
  //  Ingress MMP pointer.
  uint64_t mmpPtr               :  5; // bit 12 to 16

  // field: mmpValid
  //  If set, this entry contains a valid MMP pointer
  uint64_t mmpValid             :  1; // bit 11

  // field: allowPtr
  //  Pointer to allow special packets defined in \register{Allow Special Frame
  //  Check For L2 Action Table}.
  uint64_t allowPtr             :  2; // bit 9 to 10

  // field: useSpecialAllow
  //  Use the special frame checks on this port. \tabTwo{No.}{Yes.}
  uint64_t useSpecialAllow      :  1; // bit 8

  // field: noPortMove
  //  No port move is allowed for this packet.
  uint64_t noPortMove           :  1; // bit 7

  // field: forceSendToCpuOrigPkt
  //  Force the packet to the CPU to be the originial,unmodified, packet.
  //  \ifdef{\texTunneling}{The exception to this is rule is the tunnel exit which
  //  will still be carried out.}{}
  uint64_t forceSendToCpuOrigPkt:  1; // bit 6

  // field: sendToCpu
  //  The packet shall be send to the CPU.
  uint64_t sendToCpu            :  1; // bit 5

  // field: dropPortMove
  //  The packet shall be dropped if the result from the learning lookup is
  //  port-move.
  uint64_t dropPortMove         :  1; // bit 4

  // field: drop
  //  The packet shall only drop on the ports which hits this action.
  uint64_t drop                 :  1; // bit 3

  // field: dropAll
  //  The packet shall drop all instances and update counter \register{L2 Action
  //  Table Drop}. However special packets which are allowed will still be allowed
  //  into the switch (using the field \field{L2 Action Table}{useSpecialAllow}
  //  set to one and register \register{Allow Special Frame Check For L2 Action
  //  Table})
  uint64_t dropAll              :  1; // bit 2

  // field: noLearningMc
  //  If the packet is a L2 Multicast then the packet shall not be learned. If a
  //  packet is a L2 Multicast depends on if the SA MAC MC bit is set.
  uint64_t noLearningMc         :  1; // bit 1

  // field: noLearningUc
  //  The packet shall not be learned. This is applied to L2 DA MAC unicast
  //  packets.
  uint64_t noLearningUc         :  1; // bit 0
} t_L2ActionTableSourcePort;

// ------- Struct declaration for register: FloodingActionSendtoPort --------- 
typedef struct {

  // field: destPort
  //  Once enabled this is the destination port to sent the packet to in case of
  //  flooding.
  uint64_t destPort:  3; // bit 1 to 3

  // field: enable
  //  Enable sent to port instead of flooding. \tabTwo{Disable}{Enable}
  uint64_t enable  :  1; // bit 0
} t_FloodingActionSendtoPort;

// ------- Struct declaration for register: EgressMPLSTTLTable --------- 
typedef struct {

  // field: newTTL
  //  New TTL for the packet. Only used when addNewTTL is set to 1
  uint64_t newTTL   :  8; // bit 1 to 8

  // field: addNewTTL
  //  Select if the router should decremented TTL in the outgoing packet or if it
  //  should be set to a fixed value. \tabTwo{Decrement TTL} {Set the TTL to
  //  \field{Egress MPLS TTL Table}{newTTL}}
  uint64_t addNewTTL:  1; // bit 0
} t_EgressMPLSTTLTable;

// ------- Struct declaration for register: EgressPortNATState --------- 
typedef struct {

  // field: portState
  //  Egress Port NAT state (Bit 0 is port 0, bit 1 is port 1 etc.).
  //  \tabTwo{Private}{Public}
  uint64_t portState:  6; // bit 0 to 5
} t_EgressPortNATState;

// ------- Struct declaration for register: DebugCounterl2DaHashKeySetup --------- 
typedef struct {

  // field: hitValue
  //  Value to compare to update debug counter. Both the incoming value and this
  //  value is ANDed with the mask before comparsion is carried out. If comparsion
  //  results in true the counter is updated
  uint64_t hitValue: 60; // bit 60 to 119

  // field: mask
  //  Mask for comparison to update debug counter.
  uint64_t mask    : 60; // bit 0 to 59
} t_DebugCounterl2DaHashKeySetup;

// ------- Struct declaration for register: IngressDropOptions --------- 
typedef struct {

  // field: learnL2HairpinDrop
  //  Allow learning when the packet is dropped due to hairpin configurations.
  uint64_t learnL2HairpinDrop       :  1; // bit 3

  // field: learnL2DestVlanMemberDrop
  //  Allow learning when the packt is dropped due to destination VLAN membership
  //  check.
  uint64_t learnL2DestVlanMemberDrop:  1; // bit 2

  // field: learnL2FloodDrop
  //  Allow learning when the packet is dropped due to unknown DA.
  uint64_t learnL2FloodDrop         :  1; // bit 1

  // field: learnL2DestDrop
  //  Allow learning when L2 Destination Table drops the packet.
  uint64_t learnL2DestDrop          :  1; // bit 0
} t_IngressDropOptions;

// ------- Struct declaration for register: MBSCDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_MBSCDrop;

// ------- Struct declaration for register: VLANMemberDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_VLANMemberDrop;

// ------- Struct declaration for register: PSPacketTailCounter --------- 
typedef struct {

  // field: packets
  //  Number of packet tails.
  uint64_t packets: 32; // bit 0 to 31
} t_PSPacketTailCounter;

// ------- Struct declaration for register: EgressResourceManagerDrop --------- 
typedef struct {

  // field: packets
  //  Number of packets.
  uint64_t packets: 32; // bit 0 to 31
} t_EgressResourceManagerDrop;

// ------- Struct declaration for register: MACRXBrokenPackets --------- 
typedef struct {

  // field: packets
  //  Number of packets.
  uint64_t packets: 32; // bit 0 to 31
} t_MACRXBrokenPackets;

// ------- Struct declaration for register: IngressVIDOuterVIDRangeAssignmentAnswer --------- 
typedef struct {

  // field: order
  //  Order for this assignment. If the ingress VID can be assigned from other
  //  packet field ranges, the one with the highest order wins.
  uint64_t order     :  2; // bit 12 to 13

  // field: ingressVid
  //  Ingress VID.
  uint64_t ingressVid: 12; // bit 0 to 11
} t_IngressVIDOuterVIDRangeAssignmentAnswer;

// ------- Struct declaration for register: IngressVIDMACRangeSearchData --------- 
typedef struct {

  // field: end
  //  End of MAC address range.
  uint64_t end   : 48; // bit 55 to 102

  // field: start
  //  Start of MAC address range.
  uint64_t start : 48; // bit 7 to 54

  // field: saOrDa
  //  Is this rule for source or destination MAC address. \tabTwo{Source
  //  MAC}{Destination MAC}
  uint64_t saOrDa:  1; // bit 6

  // field: ports
  //  Ports that this range search is activated on.
  uint64_t ports :  6; // bit 0 to 5
} t_IngressVIDMACRangeSearchData;

// ------- Struct declaration for register: L2ActionTableEgressPortState --------- 
typedef struct {

  // field: state
  //  What is the egress port status bits in the L2 Action Table for the egress
  //  port. Bit [0] are used for port 0, Bits [1] are used for port 1 and so on.
  uint64_t state:  6; // bit 0 to 5
} t_L2ActionTableEgressPortState;

// ------- Struct declaration for register: IngressNATHitStatus --------- 
typedef struct {

  // field: hit
  //  If set, the corresponding entry in the \register{Ingress NAT Operation} is
  //  hit.
  uint64_t hit:  1; // bit 0
} t_IngressNATHitStatus;

// ------- Struct declaration for register: ColorRemapFromIngressAdmissionControl --------- 
typedef struct {

  // field: color2Dei
  //  New DEI value based on packet color. This is located in the outermost VLAN
  //  of the transmitted packet.\newline \tabTwoByThree{bit 0}{DEI value for
  //  green} {bit 1}{DEI value for yellow} {bit 2}{DEI value for red}
  uint64_t color2Dei:  3; // bit 35 to 37

  // field: tosMask
  //  Mask for updating the TOS/TC field. For each bit in the mask, 0 means keep
  //  original value, 1 means update new value to that bit.
  uint64_t tosMask  :  8; // bit 27 to 34

  // field: color2Tos
  //  New TOS/TC value based on packet color. \newline \tabTwoByThree{bits
  //  [0:7]}{TOS/TC value for green} {bits [8:15]}{TOS/TC value for yellow} {bits
  //  [16:23]}{TOS/TC value for red}
  uint64_t color2Tos: 24; // bit 3 to 26

  // field: colorMode
  //  \tabFour{Remap disabled}{Remap to L3 only}{Remap to L2 only}{Remap to L2 and
  //  L3}
  uint64_t colorMode:  2; // bit 1 to 2

  // field: enable
  //  If set, the \field{Color Remap From Ingress Admission Control}{colorMode}
  //  field determines the remap process. Otherwise color remapping based on the
  //  ingress admission control is skipped.
  uint64_t enable   :  1; // bit 0
} t_ColorRemapFromIngressAdmissionControl;

// ------- Struct declaration for register: IPPDebugdebugMatchIPP0 --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value: 22; // bit 0 to 21
} t_IPPDebugdebugMatchIPP0;

// ------- Struct declaration for register: HairpinEnable --------- 
typedef struct {

  // field: allowUc
  //  Allow unicast to source port.
  uint64_t allowUc   :  1; // bit 2

  // field: allowMc
  //  Allow multicast to source port.
  uint64_t allowMc   :  1; // bit 1

  // field: allowFlood
  //  Allow flooding to source port.
  uint64_t allowFlood:  1; // bit 0
} t_HairpinEnable;

// ------- Struct declaration for register: IKEPacketDecoderOptions --------- 
typedef struct {

  // field: toCpu
  //  If a packet comes in on this source port then send the packet to the CPU
  //  port.\tabTwo{Do not sent to CPU. Normal Processing of packet.}{Send to CPU ,
  //  bypass normal packet processing.}
  uint64_t toCpu  :  6; // bit 39 to 44

  // field: drop
  //  If a packet comes in on this source port then drop the packet.\tabTwo{Do not
  //  drop this packet.}{Drop this packet and update the drop counter.}
  uint64_t drop   :  6; // bit 33 to 38

  // field: udp2
  //  The value to be used to find this packet type.
  uint64_t udp2   : 16; // bit 17 to 32

  // field: udp1
  //  The value to be used to find this packet type.
  uint64_t udp1   : 16; // bit 1 to 16

  // field: enabled
  //  Is this decoding enabled. \tabTwo{No}{Yes}
  uint64_t enabled:  1; // bit 0
} t_IKEPacketDecoderOptions;

// ------- Struct declaration for register: MPLSEXPFieldToPacketColorMappingTable --------- 
typedef struct {

  // field: color
  //  Packet initial color
  uint64_t color:  2; // bit 0 to 1
} t_MPLSEXPFieldToPacketColorMappingTable;

// ------- Struct declaration for register: IPMulticastACLDropCounter --------- 
typedef struct {

  // field: packets
  //  Number of packets.
  uint64_t packets: 32; // bit 0 to 31
} t_IPMulticastACLDropCounter;

// ------- Struct declaration for register: QueueShaperBucketCapacityConfiguration --------- 
typedef struct {

  // field: bucketCapacity
  //  Capacity of the token bucket
  uint64_t bucketCapacity: 17; // bit 0 to 16
} t_QueueShaperBucketCapacityConfiguration;

// ------- Struct declaration for register: IngressVIDInnerVIDRangeSearchData --------- 
typedef struct {

  // field: end
  //  End of VID range.
  uint64_t end  : 12; // bit 19 to 30

  // field: start
  //  Start of VID range.
  uint64_t start: 12; // bit 7 to 18

  // field: vtype
  //  Shall this entry match S-Type or C-Type VLAN. \tabTwo{C-Type}{S-Type}
  uint64_t vtype:  1; // bit 6

  // field: ports
  //  Ports that this range search is activated on.
  uint64_t ports:  6; // bit 0 to 5
} t_IngressVIDInnerVIDRangeSearchData;

// ------- Struct declaration for register: NextHopHitStatus --------- 
typedef struct {

  // field: mpls
  //  The next hop entry was hit with an MPLS packet.
  uint64_t mpls:  1; // bit 2

  // field: ipv6
  //  The next hop entry was hit with an IPv6 packet.
  uint64_t ipv6:  1; // bit 1

  // field: ipv4
  //  The next hop entry was hit with an IPv4 packet.
  uint64_t ipv4:  1; // bit 0
} t_NextHopHitStatus;

// ------- Struct declaration for register: EgressQueueToMPLSEXPMappingTable --------- 
typedef struct {

  // field: exp
  //  The outgoing Exp value for this queue.
  uint64_t exp:  3; // bit 0 to 2
} t_EgressQueueToMPLSEXPMappingTable;

// ------- Struct declaration for register: FFAUsedPFC --------- 
typedef struct {

  // field: cells
  //  Number of cells
  uint64_t cells: 10; // bit 0 to 9
} t_FFAUsedPFC;

// ------- Struct declaration for register: IPPDebugrouted --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value:  1; // bit 0
} t_IPPDebugrouted;

// ------- Struct declaration for register: EgressRouterTable --------- 
typedef struct {

  // field: newTTL
  //  New TTL for the packet. Only used when addNewTTL is set to 1
  uint64_t newTTL   :  8; // bit 1 to 8

  // field: addNewTTL
  //  Select if the router should decremented TTL in the outgoing packet or if it
  //  should be set to a fixed value. \tabTwo{Decrement TTL} {Set the TTL to
  //  \field{Egress Router Table}{newTTL}}
  uint64_t addNewTTL:  1; // bit 0
} t_EgressRouterTable;

// ------- Struct declaration for register: LearningAndAgingEnable --------- 
typedef struct {

  // field: lru
  //  If set, the learning engine will try to overwrite a least recently used
  //  non-static entry in either the hash table or the collision table when there
  //  is no free entry to use. Otherwise the learning engine will try to overwrite
  //  a non-static entry in the collision table.
  uint64_t lru           :  1; // bit 3

  // field: daHitEnable
  //  If set, MAC DA hit in the forwarding information base will update the hit
  //  bit for non-static entries.
  uint64_t daHitEnable   :  1; // bit 2

  // field: agingEnable
  //  If set, the aging engine will be activated.
  uint64_t agingEnable   :  1; // bit 1

  // field: learningEnable
  //  If set, the learning engine will be activated.
  uint64_t learningEnable:  1; // bit 0
} t_LearningAndAgingEnable;

// ------- Struct declaration for register: MACRXLongPacketDrop --------- 
typedef struct {

  // field: packets
  //  Number of packets.
  uint64_t packets: 32; // bit 0 to 31
} t_MACRXLongPacketDrop;

// ------- Struct declaration for register: PortXoffFFAThreshold --------- 
typedef struct {

  // field: trip
  //  \begin{fieldValues} \dscValue [0] Normal operation \dscValue [1] Force this
  //  threshold to be counted as exceeded \end{fieldValues} Only valid if this
  //  Xoff threshold is enabled.
  uint64_t trip  :  1; // bit 11

  // field: enable
  //  \begin{fieldValues} \dscValue [0] This Xoff threshold is disabled \dscValue
  //  [1] This Xoff threshold is enabled\end{fieldValues}
  uint64_t enable:  1; // bit 10

  // field: cells
  //  Xoff threshold for the number of used FFA cells for this source port
  uint64_t cells : 10; // bit 0 to 9
} t_PortXoffFFAThreshold;

// ------- Struct declaration for register: IPPDebugsrcPort --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value:  3; // bit 0 to 2
} t_IPPDebugsrcPort;

// ------- Struct declaration for register: TCXonFFAThreshold --------- 
typedef struct {

  // field: cells
  //  Xon threshold for the number of used FFA cells for this traffic class
  uint64_t cells: 10; // bit 0 to 9
} t_TCXonFFAThreshold;

// ------- Struct declaration for register: SCTPPacketDecoderOptions --------- 
typedef struct {

  // field: toCpu
  //  If a packet comes in on this source port then send the packet to the CPU
  //  port.\tabTwo{Do not sent to CPU. Normal Processing of packet.}{Send to CPU ,
  //  bypass normal packet processing.}
  uint64_t toCpu  :  6; // bit 15 to 20

  // field: drop
  //  If a packet comes in on this source port then drop the packet.\tabTwo{Do not
  //  drop this packet.}{Drop this packet and update the drop counter.}
  uint64_t drop   :  6; // bit 9 to 14

  // field: l4Proto
  //  The value to be used to find this packet type.
  uint64_t l4Proto:  8; // bit 1 to 8

  // field: enabled
  //  Is this decoding enabled. \tabTwo{No}{Yes}
  uint64_t enabled:  1; // bit 0
} t_SCTPPacketDecoderOptions;

// ------- Struct declaration for register: LinkAggregationToPhysicalPortsMembers --------- 
typedef struct {

  // field: members
  //  Physical ports that are members of this link aggregate. One bit per port.
  uint64_t members:  6; // bit 0 to 5
} t_LinkAggregationToPhysicalPortsMembers;

// ------- Struct declaration for register: L3TunnelEntryInstructionTable --------- 
typedef struct {

  // field: l4Protocol
  //  The new Next Header/Protocol byte
  uint64_t l4Protocol  :  8; // bit 2 to 9

  // field: updateL4Type
  //  If the packet is a IPv4 or IPv6 then the Next Header/Protocol field shall be
  //  updated. IPv4 Packet will see a updated header checksum.
  uint64_t updateL4Type:  2; // bit 0 to 1
} t_L3TunnelEntryInstructionTable;

// ------- Struct declaration for register: SecondTunnelExitLookupTCAM --------- 
typedef struct {

  // field: tabKey
  //  The key from the first tunnel exit lookup result table.
  uint64_t tabKey    :  2; // bit 163 to 164

  // field: tabKeymask
  //  Mask for tabKey.
  uint64_t tabKeymask:  2; // bit 161 to 162

  // field: pktKey
  //  The extracted key from the packet according to the first lookup.
  uint8_t pktKey[10]; // bit 81 to 160

  // field: pktKeymask
  //  Mask for pktKey.
  uint8_t pktKeymask[10]; // bit 1 to 80

  // field: valid
  //  Is this entry valid. \tabTwo{No}{Yes}
  uint64_t valid     :  1; // bit 0
} t_SecondTunnelExitLookupTCAM;

// ------- Struct declaration for register: Scratch --------- 
typedef struct {

  // field: scratch
  //  scratch field.
  uint64_t scratch: 64; // bit 0 to 63
} t_Scratch;

// ------- Struct declaration for register: MapQueuetoPriority --------- 
typedef struct {

  // field: prio7
  //  The priority for queue 7
  uint64_t prio7:  3; // bit 21 to 23

  // field: prio6
  //  The priority for queue 6
  uint64_t prio6:  3; // bit 18 to 20

  // field: prio5
  //  The priority for queue 5
  uint64_t prio5:  3; // bit 15 to 17

  // field: prio4
  //  The priority for queue 4
  uint64_t prio4:  3; // bit 12 to 14

  // field: prio3
  //  The priority for queue 3
  uint64_t prio3:  3; // bit 9 to 11

  // field: prio2
  //  The priority for queue 2
  uint64_t prio2:  3; // bit 6 to 8

  // field: prio1
  //  The priority for queue 1
  uint64_t prio1:  3; // bit 3 to 5

  // field: prio0
  //  The priority for queue 0
  uint64_t prio0:  3; // bit 0 to 2
} t_MapQueuetoPriority;

// ------- Struct declaration for register: NextHopDAMAC --------- 
typedef struct {

  // field: daMac
  //  The destination MAC address for the next hop.
  uint64_t daMac: 48; // bit 0 to 47
} t_NextHopDAMAC;

// ------- Struct declaration for register: DNSDecoderDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_DNSDecoderDrop;

// ------- Struct declaration for register: ReceivedPacketsonIngressVRF --------- 
typedef struct {

  // field: packets
  //  Number of packets.
  uint64_t packets: 32; // bit 0 to 31
} t_ReceivedPacketsonIngressVRF;

// ------- Struct declaration for register: EPPDebugreQueue --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value:  1; // bit 0
} t_EPPDebugreQueue;

// ------- Struct declaration for register: L2ReservedMulticastAddressDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_L2ReservedMulticastAddressDrop;

// ------- Struct declaration for register: IngressConfigurableACL1PreLookup --------- 
typedef struct {

  // field: rulePtr
  //  If the valid is entry then this rule pointer will be used.
  uint64_t rulePtr:  3; // bit 1 to 3

  // field: valid
  //  Is this entry valid. If not then use default port rule.
  uint64_t valid  :  1; // bit 0
} t_IngressConfigurableACL1PreLookup;

// ------- Struct declaration for register: EPPDebugisPPPoE --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value:  1; // bit 0
} t_EPPDebugisPPPoE;

// ------- Struct declaration for register: MACRXMaximumPacketLength --------- 
typedef struct {

  // field: bytes
  //  Number of bytes.
  uint64_t bytes: 32; // bit 0 to 31
} t_MACRXMaximumPacketLength;

// ------- Struct declaration for register: EgressPortFilteringDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_EgressPortFilteringDrop;

// ------- Struct declaration for register: IngressConfigurableACL0RulesSetup --------- 
typedef struct {

  // field: fieldSelectBitmask
  //  Bitmask of which fields to select. Set a bit to one to select this specific
  //  field, set zero to not select field. At Maximum 7 bits should be set.
  uint64_t fieldSelectBitmask: 14; // bit 0 to 13
} t_IngressConfigurableACL0RulesSetup;

// ------- Struct declaration for register: XonFFAThreshold --------- 
typedef struct {

  // field: cells
  //  Xon threshold for the total number of used FFA cells
  uint64_t cells: 10; // bit 0 to 9
} t_XonFFAThreshold;

// ------- Struct declaration for register: PortShaperEnable --------- 
typedef struct {

  // field: enable
  //  Bitmask where the index is the Egress Port
  uint64_t enable:  6; // bit 0 to 5
} t_PortShaperEnable;

// ------- Struct declaration for register: MACRXShortPacketDrop --------- 
typedef struct {

  // field: packets
  //  Number of packets.
  uint64_t packets: 32; // bit 0 to 31
} t_MACRXShortPacketDrop;

// ------- Struct declaration for register: LearningPacketDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_LearningPacketDrop;

// ------- Struct declaration for register: BOOTPandDHCPPacketDecoderOptions --------- 
typedef struct {

  // field: toCpu
  //  If a packet comes in on this source port then send the packet to the CPU
  //  port.\tabTwo{Do not sent to CPU. Normal Processing of packet.}{Send to CPU ,
  //  bypass normal packet processing.}
  uint64_t toCpu  :  6; // bit 39 to 44

  // field: drop
  //  If a packet comes in on this source port then drop the packet.\tabTwo{Do not
  //  drop this packet.}{Drop this packet and update the drop counter.}
  uint64_t drop   :  6; // bit 33 to 38

  // field: udp2
  //  The value to be used to find this packet type.
  uint64_t udp2   : 16; // bit 17 to 32

  // field: udp1
  //  The value to be used to find this packet type.
  uint64_t udp1   : 16; // bit 1 to 16

  // field: enabled
  //  Is this decoding enabled. \tabTwo{No}{Yes}
  uint64_t enabled:  1; // bit 0
} t_BOOTPandDHCPPacketDecoderOptions;

// ------- Struct declaration for register: ExpiredTTLDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_ExpiredTTLDrop;

// ------- Struct declaration for register: IPPDebugvlanVidOp --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value:  3; // bit 0 to 2
} t_IPPDebugvlanVidOp;

// ------- Struct declaration for register: LearningDataFIFO --------- 
typedef struct {

  // field: valid
  //  \tabTwo{FIFO is empty}{FIFO is not empty and the data is valid}.
  uint64_t valid                :  1; // bit 107

  // field: type
  //  Type of the learning request. \tabTwo{Hardware learning result}{Software
  //  learning result from a injected learning packet}
  uint64_t type                 :  1; // bit 106

  // field: status
  //  Entry status either refers to the \register{L2 Aging Table} or the
  //  \register{L2 Aging Collision Table} based on the \field{Learning Data
  //  FIFO}{destAddress} field.
  uint64_t status               :  3; // bit 103 to 105

  // field: metaData
  //  \field{L2 Destination Table}{metaData} field in the \register{L2 Destination
  //  Table}
  uint64_t metaData             : 16; // bit 87 to 102

  // field: l2ActionTableSaStatus
  //  \field{L2 Destination Table}{l2ActionTableSaStatus} field in the
  //  \register{L2 Destination Table}
  uint64_t l2ActionTableSaStatus:  1; // bit 86

  // field: l2ActionTableDaStatus
  //  \field{L2 Destination Table}{l2ActionTableDaStatus} field in the
  //  \register{L2 Destination Table}
  uint64_t l2ActionTableDaStatus:  1; // bit 85

  // field: drop
  //  The \field{L2 Destination Table}{pktDrop} field in the \register{L2
  //  Destination Table} decided by the learning unit.
  uint64_t drop                 :  1; // bit 84

  // field: portorptr
  //  The \field{L2 Destination Table}{destPort or mcAddr} field in the
  //  \register{L2 Destination Table} decided by the learning unit.
  uint64_t portorptr            :  6; // bit 78 to 83

  // field: uc
  //  The \field{L2 Destination Table}{uc} field in the \register{L2 Destination
  //  Table} decided by the learning unit.
  uint64_t uc                   :  1; // bit 77

  // field: destAddress
  //  The \register{L2 Destination Table} address decided by the learning unit.
  uint64_t destAddress          : 13; // bit 64 to 76

  // field: gid
  //  Global IDentifier from the \field{VLAN Table}{gid} field in the
  //  \register{VLAN Table}.
  uint64_t gid                  : 16; // bit 48 to 63

  // field: mac
  //  MAC address for a learning request.
  uint64_t mac                  : 48; // bit 0 to 47
} t_LearningDataFIFO;

// ------- Struct declaration for register: ERMRedConfiguration --------- 
typedef struct {

  // field: redMaxCells
  //  Maximum allowed packet length in cells when the buffer memory congestion
  //  status is red.
  uint64_t redMaxCells:  8; // bit 20 to 27

  // field: redXon
  //  Once the red congestion check is applied, number of free cells need to go
  //  above this value to disable the check again. The value needs to be larger
  //  than \field{ERM Red Configuration}{redXoff} to provide an effective
  //  hysteresis.
  uint64_t redXon     : 10; // bit 10 to 19

  // field: redXoff
  //  Number of free cells below this value will invoke the red congestion check
  //  for the incoming cells. The checks include the number of enqueued cells in
  //  the current queue and the packet length. The incoming packet might be
  //  terminated and dropped based on the check result.
  uint64_t redXoff    : 10; // bit 0 to 9
} t_ERMRedConfiguration;

// ------- Struct declaration for register: NATAddEgressPortforNATCalculation --------- 
typedef struct {

  // field: dontAddEgress
  //  Do not add egress port when calculating the egress NAT offset pointer.
  //  \tabTwo{Add Egress Port.}{Do not add Egress Port.}
  uint64_t dontAddEgress :  1; // bit 1

  // field: dontAddIngress
  //  Do not add egress port when calculating the ingress NAT offset pointer.
  //  \tabTwo{Add Egress Port.}{Do not add Egress Port.}
  uint64_t dontAddIngress:  1; // bit 0
} t_NATAddEgressPortforNATCalculation;

// ------- Struct declaration for register: CoreTickConfiguration --------- 
typedef struct {

  // field: stepDivider
  //  The four ticks derived from the master core tick are issued once every
  //  $rg_tick_div.stepDivider^{tick_number+1}$ master ticks. The master tick is
  //  tick number 0. If stepDivider is set to zero, there will be no ticks except
  //  possibly the master tick.
  uint64_t stepDivider:  4; // bit 20 to 23

  // field: clkDivider
  //  The master Core Tick will be issued once every $rg_tick_div.clkDivider/4$
  //  core clock cycles. If set to zero, there will be no tick.
  uint64_t clkDivider : 20; // bit 0 to 19
} t_CoreTickConfiguration;

// ------- Struct declaration for register: IngressConfigurableACL2PreLookup --------- 
typedef struct {

  // field: rulePtr
  //  If the valid is entry then this rule pointer will be used.
  uint64_t rulePtr:  2; // bit 1 to 2

  // field: valid
  //  Is this entry valid. If not then use default port rule.
  uint64_t valid  :  1; // bit 0
} t_IngressConfigurableACL2PreLookup;

// ------- Struct declaration for register: SourcePortDefaultACLActionDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_SourcePortDefaultACLActionDrop;

// ------- Struct declaration for register: PortTCTailDropTotalThreshold --------- 
typedef struct {

  // field: trip
  //  \begin{fieldValues} \dscValue [0] Normal operation \dscValue [1] Force this
  //  threshold to be counted as exceeded \end{fieldValues} Only valid if this
  //  tail-drop threshold is enabled.
  uint64_t trip  :  1; // bit 11

  // field: enable
  //  \begin{fieldValues} \dscValue [0] This tail-drop threshold is disabled
  //  \dscValue [1] This tail-drop threshold is enabled\end{fieldValues}
  uint64_t enable:  1; // bit 10

  // field: cells
  //  Tail-drop threshold in number of cells. When the sum of reserved and FFA
  //  cells used by this specific source port and traffic class combination
  //  reaches this threshold no further packets will be accepted for this source
  //  port and traffic class
  uint64_t cells : 10; // bit 0 to 9
} t_PortTCTailDropTotalThreshold;

// ------- Struct declaration for register: IngressConfigurableACL0Selection --------- 
typedef struct {

  // field: selectSmallOrLarge
  //  If set to zero then small hash table is selected. If set to one then large
  //  hash table is selected.
  uint64_t selectSmallOrLarge:  1; // bit 1

  // field: selectTcamOrTable
  //  If set to zero then TCAM answer is selected. If set to one then hash table
  //  answer is selected.
  uint64_t selectTcamOrTable :  1; // bit 0
} t_IngressConfigurableACL0Selection;

// ------- Struct declaration for register: VLANPCPToQueueMappingTable --------- 
typedef struct {

  // field: pQueue
  //  Egress queue.
  uint64_t pQueue:  3; // bit 0 to 2
} t_VLANPCPToQueueMappingTable;

// ------- Struct declaration for register: L2MulticastStormControlBucketCapacityConfiguration --------- 
typedef struct {

  // field: bucketCapacity
  //  Capacity of the token bucket
  uint64_t bucketCapacity: 16; // bit 0 to 15
} t_L2MulticastStormControlBucketCapacityConfiguration;

// ------- Struct declaration for register: TailDropFFAThreshold --------- 
typedef struct {

  // field: trip
  //  \begin{fieldValues} \dscValue [0] Normal operation \dscValue [1] Force this
  //  threshold to be counted as exceeded \end{fieldValues} Only valid if this
  //  tail-drop threshold is enabled.
  uint64_t trip  :  1; // bit 11

  // field: enable
  //  \begin{fieldValues} \dscValue [0] This tail-drop threshold is disabled
  //  \dscValue [1] This tail-drop threshold is enabled\end{fieldValues}
  uint64_t enable:  1; // bit 10

  // field: cells
  //  Tail-drop threshold in number of cells. When the total number of FFA cells
  //  used reaches this threshold no further packets will be accepted.
  uint64_t cells : 10; // bit 0 to 9
} t_TailDropFFAThreshold;

// ------- Struct declaration for register: ARPPacketDecoderOptions --------- 
typedef struct {

  // field: toCpu
  //  If a packet comes in on this source port then send the packet to the CPU
  //  port.\tabTwo{Do not sent to CPU. Normal Processing of packet.}{Send to CPU ,
  //  bypass normal packet processing.}
  uint64_t toCpu  :  6; // bit 23 to 28

  // field: drop
  //  If a packet comes in on this source port then drop the packet.\tabTwo{Do not
  //  drop this packet.}{Drop this packet and update the drop counter.}
  uint64_t drop   :  6; // bit 17 to 22

  // field: eth
  //  The value to be used to find this packet type.
  uint64_t eth    : 16; // bit 1 to 16

  // field: enabled
  //  Is this decoding enabled. \tabTwo{No}{Yes}
  uint64_t enabled:  1; // bit 0
} t_ARPPacketDecoderOptions;

// ------- Struct declaration for register: DebugCounterreQueuePortIdSetup --------- 
typedef struct {

  // field: hitValue
  //  Value to compare to update debug counter. Both the incoming value and this
  //  value is ANDed with the mask before comparsion is carried out. If comparsion
  //  results in true the counter is updated
  uint64_t hitValue:  3; // bit 3 to 5

  // field: mask
  //  Mask for comparison to update debug counter.
  uint64_t mask    :  3; // bit 0 to 2
} t_DebugCounterreQueuePortIdSetup;

// ------- Struct declaration for register: L2MulticastTable --------- 
typedef struct {

  // field: metaData
  //  Meta data for to CPU tag.
  uint64_t metaData  : 16; // bit 6 to 21

  // field: mcPortMask
  //  L2 portmask entry members. If set, the port is part of multicast group and
  //  shall be transmitted to.
  uint64_t mcPortMask:  6; // bit 0 to 5
} t_L2MulticastTable;

// ------- Struct declaration for register: IPPDebugnrVlans --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value:  2; // bit 0 to 1
} t_IPPDebugnrVlans;

// ------- Struct declaration for register: ReservedDestinationMACAddressRange --------- 
typedef struct {

  // field: enable
  //  Enable the reserved MAC DA check per source port. One bit for each port
  //  where bit 0 corresponds to port 0. If a bit is set to one, the reserved MAC
  //  DA range is activated for that source port.
  uint64_t enable    :  6; // bit 113 to 118

  // field: mmpOrder
  //  Ingress MMP pointer order.
  uint64_t mmpOrder  :  2; // bit 111 to 112

  // field: mmpPtr
  //  Ingress MMP pointer.
  uint64_t mmpPtr    :  5; // bit 106 to 110

  // field: mmpValid
  //  If set, this entry contains a valid MMP pointer
  uint64_t mmpValid  :  1; // bit 105

  // field: forceColor
  //  If set, the packet shall have a forced color.
  uint64_t forceColor:  1; // bit 104

  // field: color
  //  Inital color of the packet.
  uint64_t color     :  2; // bit 102 to 103

  // field: eQueue
  //  The egress queue to be assigned if the forceQueue field in this entry is set
  //  to 1.
  uint64_t eQueue    :  3; // bit 99 to 101

  // field: forceQueue
  //  If set, the packet shall have a forced egress queue. Please see Egress Queue
  //  Selection Diagram in Figure \ref{fig:determineEgressQueue}
  uint64_t forceQueue:  1; // bit 98

  // field: sendToCpu
  //  If the MAC address was within the range the packet shall be sent to the CPU.
  uint64_t sendToCpu :  1; // bit 97

  // field: dropEnable
  //  If the MAC address was within the range the packet shall be dropped and the
  //  \register{Reserved MAC DA Drop} counter incremented.
  uint64_t dropEnable:  1; // bit 96

  // field: stopAddr
  //  The end MAC address of the range. A packets destination MAC address must be
  //  equal or less than this value to match the range.
  uint64_t stopAddr  : 48; // bit 48 to 95

  // field: startAddr
  //  The start MAC address of the range. A packets destination MAC address must
  //  be equal or greater than this value to match the range.
  uint64_t startAddr : 48; // bit 0 to 47
} t_ReservedDestinationMACAddressRange;

// ------- Struct declaration for register: TCTailDropFFAThreshold --------- 
typedef struct {

  // field: trip
  //  \begin{fieldValues} \dscValue [0] Normal operation \dscValue [1] Force this
  //  threshold to be counted as exceeded \end{fieldValues} Only valid if this
  //  tail-drop threshold is enabled.
  uint64_t trip  :  1; // bit 11

  // field: enable
  //  \begin{fieldValues} \dscValue [0] This tail-drop threshold is disabled
  //  \dscValue [1] This tail-drop threshold is enabled\end{fieldValues}
  uint64_t enable:  1; // bit 10

  // field: cells
  //  Tail-drop threshold in number of cells. When the FFA cells used by the
  //  traffic class reaches this threshold no further packets will be accepted for
  //  this traffic class
  uint64_t cells : 10; // bit 0 to 9
} t_TCTailDropFFAThreshold;

// ------- Struct declaration for register: IngressConfigurableACL1SmallTable --------- 
typedef struct {

  // field: forceVidPrio
  //  This is a result field used when this entry is hit. If multiple forceVid are
  //  set and this prio bit is set then this forceVid value will be selected.
  uint64_t forceVidPrio         :  1; // bit 320

  // field: forceVid
  //  This is a result field used when this entry is hit. The new Ingress VID.
  uint64_t forceVid             : 12; // bit 308 to 319

  // field: forceVidValid
  //  This is a result field used when this entry is hit. Override the Ingress
  //  VID, see chapter \hyperref[chap:VLAN Processing]{VLAN Processing}.
  uint64_t forceVidValid        :  1; // bit 307

  // field: forceQueuePrio
  //  This is a result field used when this entry is hit. If multiple forceQueue
  //  are set and this prio bit is set then this forceQueue value will be
  //  selected.
  uint64_t forceQueuePrio       :  1; // bit 306

  // field: eQueue
  //  This is a result field used when this entry is hit. The egress queue to be
  //  assigned if the forceQueue field in this entry is set to 1.
  uint64_t eQueue               :  3; // bit 303 to 305

  // field: forceQueue
  //  This is a result field used when this entry is hit. If set, the packet shall
  //  have a forced egress queue. Please see Egress Queue Selection Diagram in
  //  Figure \ref{fig:determineEgressQueue}
  uint64_t forceQueue           :  1; // bit 302

  // field: mmpOrder
  //  This is a result field used when this entry is hit. Ingress MMP pointer
  //  order.
  uint64_t mmpOrder             :  2; // bit 300 to 301

  // field: mmpPtr
  //  This is a result field used when this entry is hit. Ingress MMP pointer.
  uint64_t mmpPtr               :  5; // bit 295 to 299

  // field: mmpValid
  //  This is a result field used when this entry is hit. If set, this entry
  //  contains a valid MMP pointer
  uint64_t mmpValid             :  1; // bit 294

  // field: forceColorPrio
  //  This is a result field used when this entry is hit. If multiple forceColor
  //  are set and this prio bit is set then this forceVid value will be selected.
  uint64_t forceColorPrio       :  1; // bit 293

  // field: color
  //  This is a result field used when this entry is hit. Initial color of the
  //  packet if the forceColor field is set.
  uint64_t color                :  2; // bit 291 to 292

  // field: forceColor
  //  This is a result field used when this entry is hit. If set, the packet shall
  //  have a forced color.
  uint64_t forceColor           :  1; // bit 290

  // field: tunnelEntryPrio
  //  This is a result field used when this entry is hit. If multiple tunnelEntry
  //  are set and this prio bit is set then this tunnelEntryPtr will be selected.
  uint64_t tunnelEntryPrio      :  1; // bit 289

  // field: tunnelEntryPtr
  //  This is a result field used when this entry is hit. The tunnel entry which
  //  this packet shall enter upon exiting the switch.
  uint64_t tunnelEntryPtr       :  4; // bit 285 to 288

  // field: tunnelEntryUcMc
  //  This is a result field used when this entry is hit. Shall this entry point
  //  to the \register{Tunnel Entry Instruction Table} with or without a egress
  //  port offset. \tabTwo{Unicast \register{Tunnel Entry Instruction Table}
  //  without offset for each port}{Multicast \register{Tunnel Entry Instruction
  //  Table} with offset for each port.}
  uint64_t tunnelEntryUcMc      :  1; // bit 284

  // field: tunnelEntry
  //  This is a result field used when this entry is hit. Shall all of these
  //  packets enter into a tunnel.
  uint64_t tunnelEntry          :  1; // bit 283

  // field: ptp
  //  This is a result field used when this entry is hit. When the packet is sent
  //  to the CPU the packet will have the PTP bit in the To CPU Tag set to one.
  //  The timestamp in the To CPU Tag will also be set to the timestamp from the
  //  incoming packet.
  uint64_t ptp                  :  1; // bit 282

  // field: natOpPrio
  //  This is a result field used when this entry is hit. If multiple natOpValid
  //  are set and this prio bit is set then this natOpPtr value will be selected.
  uint64_t natOpPrio            :  1; // bit 281

  // field: natOpPtr
  //  This is a result field used when this entry is hit. NAT operation pointer.
  uint64_t natOpPtr             : 11; // bit 270 to 280

  // field: natOpValid
  //  This is a result field used when this entry is hit. NAT operation pointer is
  //  valid.
  uint64_t natOpValid           :  1; // bit 269

  // field: newL4Value
  //  This is a result field used when this entry is hit. Update the L4 SP or DP
  //  with this value
  uint64_t newL4Value           : 16; // bit 253 to 268

  // field: updateL4SpOrDp
  //  This is a result field used when this entry is hit. Update the source or
  //  destination L4 port. \tabTwo{Source L4 Port}{Destination L4 Port}
  uint64_t updateL4SpOrDp       :  1; // bit 252

  // field: enableUpdateL4
  //  This is a result field used when this entry is hit. If this entry is hit
  //  then update L4 Source Port or Destination port in ingress packet processing,
  //  this value will be used in the Egress ACL. \tabTwo{Disable}{Enable}
  uint64_t enableUpdateL4       :  1; // bit 251

  // field: newIpValue
  //  This is a result field used when this entry is hit. Update the SA or DA IPv4
  //  address value.
  uint64_t newIpValue           : 32; // bit 219 to 250

  // field: updateSaOrDa
  //  This is a result field used when this entry is hit. Update the SA or DA IPv4
  //  address. The Destiantion IP address updated will be used in the routing
  //  functionality and Egress ACL functionality. If the source IP address is
  //  updated then the updated value will be used in the egress ACL keys.
  //  \tabTwo{Source IP Address}{Destination IP Address}
  uint64_t updateSaOrDa         :  1; // bit 218

  // field: enableUpdateIp
  //  This is a result field used when this entry is hit. If this entry is hit
  //  then update SA or DA IPv4 address in ingress packet processing, this value
  //  will be used by the routing function and egress ACL if this is exists, this
  //  only works for IPv4. \tabTwo{Disable}{Enable}
  uint64_t enableUpdateIp       :  1; // bit 217

  // field: ethPrio
  //  This is a result field used when this entry is hit. If multiple updateEType
  //  are set and this prio bit is set then this updateEType will be selected.
  uint64_t ethPrio              :  1; // bit 216

  // field: vidPrio
  //  This is a result field used when this entry is hit. If multiple updateVid
  //  are set and this prio bit is set then this updateVid will be selected.
  uint64_t vidPrio              :  1; // bit 215

  // field: pcpPrio
  //  This is a result field used when this entry is hit. If multiple updatePcp
  //  are set and this prio bit is set then this updatePcp will be selected.
  uint64_t pcpPrio              :  1; // bit 214

  // field: cfiDeiPrio
  //  This is a result field used when this entry is hit. If multiple updateCfiDei
  //  are set and this prio bit is set then this updateCfiDei will be selected.
  uint64_t cfiDeiPrio           :  1; // bit 213

  // field: newEthType
  //  This is a result field used when this entry is hit. Select which TPID to use
  //  in the outer VLAN header. \tabThree{C-VLAN - 0x8100.} {S-VLAN - 0x88A8.}
  //  {User defined VLAN type from register \register{Egress Ethernet Type for
  //  VLAN tag}.}
  uint64_t newEthType           :  2; // bit 211 to 212

  // field: updateEType
  //  This is a result field used when this entry is hit. The VLANs TPID type
  //  should be updated. \tabTwo{Do not update the TPID.}{Update the TPID.}
  uint64_t updateEType          :  1; // bit 210

  // field: newVidValue
  //  This is a result field used when this entry is hit. The VID value to update
  //  to.
  uint64_t newVidValue          : 12; // bit 198 to 209

  // field: updateVid
  //  This is a result field used when this entry is hit. The VID value of the
  //  packets outermost VLAN should be updated. \tabTwo{Do not update the
  //  value.}{Update the value.}
  uint64_t updateVid            :  1; // bit 197

  // field: newPcpValue
  //  This is a result field used when this entry is hit. The PCP value to update
  //  to.
  uint64_t newPcpValue          :  3; // bit 194 to 196

  // field: updatePcp
  //  This is a result field used when this entry is hit. The PCP value of the
  //  packets outermost VLAN should be updated. \tabTwo{Do not update the
  //  value.}{Update the value.}
  uint64_t updatePcp            :  1; // bit 193

  // field: newCfiDeiValue
  //  This is a result field used when this entry is hit. The value to update to.
  uint64_t newCfiDeiValue       :  1; // bit 192

  // field: updateCfiDei
  //  This is a result field used when this entry is hit. The CFI/DEI value of the
  //  packets outermost VLAN should be updated.\tabTwo{Do not update the
  //  value.}{Update the value.}
  uint64_t updateCfiDei         :  1; // bit 191

  // field: tosMask
  //  This is a result field used when this entry is hit. Mask for TOS value.
  //  Setting a bit to one means this bit will be selected from the newTosExp
  //  field , while setting this bit to zero means that the bit will be selected
  //  from the packets already existing TOS byte bit.
  uint64_t tosMask              :  8; // bit 183 to 190

  // field: newTosExp
  //  This is a result field used when this entry is hit. New TOS/EXP value.
  uint64_t newTosExp            :  8; // bit 175 to 182

  // field: updateTosExp
  //  This is a result field used when this entry is hit. Force TOS/EXP update.
  uint64_t updateTosExp         :  1; // bit 174

  // field: counter
  //  This is a result field used when this entry is hit. Which counter in
  //  \register{Ingress Configurable ACL Match Counter} to update.
  uint64_t counter              :  6; // bit 168 to 173

  // field: updateCounter
  //  This is a result field used when this entry is hit. When set the selected
  //  statistics counter will be updated.
  uint64_t updateCounter        :  1; // bit 167

  // field: noLearning
  //  This is a result field used when this entry is hit. If set this packets MAC
  //  SA will not be learned.
  uint64_t noLearning           :  1; // bit 166

  // field: imPrio
  //  This is a result field used when this entry is hit. If multiple input mirror
  //  are set and this prio bit is set then this input mirror will be selected.
  uint64_t imPrio               :  1; // bit 165

  // field: destInputMirror
  //  This is a result field used when this entry is hit. Destination
  //  \ifdef{\texLinkAgg}{physical}{} port for input mirroring.
  uint64_t destInputMirror      :  3; // bit 162 to 164

  // field: inputMirror
  //  This is a result field used when this entry is hit. If set, input mirroring
  //  is enabled for this rule. In addition to the normal processing of the packet
  //  a copy of the unmodified input packet will be send to the destination Input
  //  Mirror port and exit on that port. The copy will be subject to the normal
  //  resource limitations in the switch.
  uint64_t inputMirror          :  1; // bit 161

  // field: destPort
  //  This is a result field used when this entry is hit. The port which the
  //  packet shall be sent to.
  uint64_t destPort             :  3; // bit 158 to 160

  // field: sendToPort
  //  This is a result field used when this entry is hit. Send the packet to a
  //  specific port. \tabTwo{Disabled.}{Send to port configured in destPort.}
  uint64_t sendToPort           :  1; // bit 157

  // field: dropEnable
  //  This is a result field used when this entry is hit. If set, the packet shall
  //  be dropped and the \register{Ingress Configurable ACL Drop} counter is
  //  incremented.
  uint64_t dropEnable           :  1; // bit 156

  // field: metaDataPrio
  //  This is a result field used when this entry is hit. If multiple ACLs hit
  //  this meta_data shall take priority.
  uint64_t metaDataPrio         :  1; // bit 155

  // field: metaData
  //  This is a result field used when this entry is hit. Meta data for packets
  //  going to the CPU.
  uint64_t metaData             : 16; // bit 139 to 154

  // field: metaDataValid
  //  This is a result field used when this entry is hit. Is the meta_data field
  //  valid.
  uint64_t metaDataValid        :  1; // bit 138

  // field: forceSendToCpuOrigPkt
  //  This is a result field used when this entry is hit. If packet shall be sent
  //  to CPU then setting this bit will force the packet to be the incoming
  //  originial packet. \ifdef{\texTunneling}{The exception to this is rule is the
  //  tunnel exit which will still be carried out.}{}.
  uint64_t forceSendToCpuOrigPkt:  1; // bit 137

  // field: sendToCpu
  //  This is a result field used when this entry is hit. If set, the packet shall
  //  be sent to the CPU port.
  uint64_t sendToCpu            :  1; // bit 136

  // field: compareData
  //  The data which shall be compared in this entry.
  uint8_t compareData[17]; // bit 1 to 135

  // field: valid
  //  Is this entry valid. \tabTwo{No}{Yes}.
  uint64_t valid                :  1; // bit 0
} t_IngressConfigurableACL1SmallTable;

// ------- Struct declaration for register: IngressVIDEthernetTypeRangeSearchData --------- 
typedef struct {

  // field: end
  //  End of Ethernet type range.
  uint64_t end  : 16; // bit 22 to 37

  // field: start
  //  Start of Ethernet type range.
  uint64_t start: 16; // bit 6 to 21

  // field: ports
  //  Ports that this range search is activated on.
  uint64_t ports:  6; // bit 0 to 5
} t_IngressVIDEthernetTypeRangeSearchData;

// ------- Struct declaration for register: LearningDataFIFOHighWatermarkLevel --------- 
typedef struct {

  // field: level
  //  Number of used entries.
  uint64_t level:  6; // bit 0 to 5
} t_LearningDataFIFOHighWatermarkLevel;

// ------- Struct declaration for register: L2FloodingStormControlRateConfiguration --------- 
typedef struct {

  // field: ifgCorrection
  //  Extra bytes per packet to correct for IFG in byte mode. Default is 4 byte
  //  FCS plus 20 byte IFG.
  uint64_t ifgCorrection  :  8; // bit 16 to 23

  // field: tick
  //  Select one of the five available core ticks. The tick frequencies are
  //  configured globaly in the core Tick Configuration register.
  uint64_t tick           :  3; // bit 13 to 15

  // field: tokens
  //  The number of tokens added each tick
  uint64_t tokens         : 12; // bit 1 to 12

  // field: packetsNotBytes
  //  If set the bucket will count packets, if cleared bytes
  uint64_t packetsNotBytes:  1; // bit 0
} t_L2FloodingStormControlRateConfiguration;

// ------- Struct declaration for register: EgressSpanningTreeState --------- 
typedef struct {

  // field: sptState
  //  State of the spanning tree protocol. Bit[2:0] is port \#0, bit[5:3] is port
  //  \#1 etc. \tabFive{Disabled}{Blocking}{Listening}{Learning}{Forwarding}.
  uint64_t sptState: 18; // bit 0 to 17
} t_EgressSpanningTreeState;

// ------- Struct declaration for register: TimetoAge --------- 
typedef struct {

  // field: tick
  //  Select one of the 5 available ticks. The tick frequencies are configured
  //  globaly in the \register{Core Tick Configuration} register.
  uint64_t tick   :  3; // bit 32 to 34

  // field: tickCnt
  //  Number of ticks (see Chapter \hyperref[chap:Tick]{Tick}) between starts of
  //  the aging process.
  uint64_t tickCnt: 32; // bit 0 to 31
} t_TimetoAge;

// ------- Struct declaration for register: EPPDebugupdateTosExp --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value:  1; // bit 0
} t_EPPDebugupdateTosExp;

// ------- Struct declaration for register: IngressEgressAdmissionControlDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_IngressEgressAdmissionControlDrop;

// ------- Struct declaration for register: IngressConfigurableACL1LargeTable --------- 
typedef struct {

  // field: forceVidPrio
  //  This is a result field used when this entry is hit. If multiple forceVid are
  //  set and this prio bit is set then this forceVid value will be selected.
  uint64_t forceVidPrio         :  1; // bit 320

  // field: forceVid
  //  This is a result field used when this entry is hit. The new Ingress VID.
  uint64_t forceVid             : 12; // bit 308 to 319

  // field: forceVidValid
  //  This is a result field used when this entry is hit. Override the Ingress
  //  VID, see chapter \hyperref[chap:VLAN Processing]{VLAN Processing}.
  uint64_t forceVidValid        :  1; // bit 307

  // field: forceQueuePrio
  //  This is a result field used when this entry is hit. If multiple forceQueue
  //  are set and this prio bit is set then this forceQueue value will be
  //  selected.
  uint64_t forceQueuePrio       :  1; // bit 306

  // field: eQueue
  //  This is a result field used when this entry is hit. The egress queue to be
  //  assigned if the forceQueue field in this entry is set to 1.
  uint64_t eQueue               :  3; // bit 303 to 305

  // field: forceQueue
  //  This is a result field used when this entry is hit. If set, the packet shall
  //  have a forced egress queue. Please see Egress Queue Selection Diagram in
  //  Figure \ref{fig:determineEgressQueue}
  uint64_t forceQueue           :  1; // bit 302

  // field: mmpOrder
  //  This is a result field used when this entry is hit. Ingress MMP pointer
  //  order.
  uint64_t mmpOrder             :  2; // bit 300 to 301

  // field: mmpPtr
  //  This is a result field used when this entry is hit. Ingress MMP pointer.
  uint64_t mmpPtr               :  5; // bit 295 to 299

  // field: mmpValid
  //  This is a result field used when this entry is hit. If set, this entry
  //  contains a valid MMP pointer
  uint64_t mmpValid             :  1; // bit 294

  // field: forceColorPrio
  //  This is a result field used when this entry is hit. If multiple forceColor
  //  are set and this prio bit is set then this forceVid value will be selected.
  uint64_t forceColorPrio       :  1; // bit 293

  // field: color
  //  This is a result field used when this entry is hit. Initial color of the
  //  packet if the forceColor field is set.
  uint64_t color                :  2; // bit 291 to 292

  // field: forceColor
  //  This is a result field used when this entry is hit. If set, the packet shall
  //  have a forced color.
  uint64_t forceColor           :  1; // bit 290

  // field: tunnelEntryPrio
  //  This is a result field used when this entry is hit. If multiple tunnelEntry
  //  are set and this prio bit is set then this tunnelEntryPtr will be selected.
  uint64_t tunnelEntryPrio      :  1; // bit 289

  // field: tunnelEntryPtr
  //  This is a result field used when this entry is hit. The tunnel entry which
  //  this packet shall enter upon exiting the switch.
  uint64_t tunnelEntryPtr       :  4; // bit 285 to 288

  // field: tunnelEntryUcMc
  //  This is a result field used when this entry is hit. Shall this entry point
  //  to the \register{Tunnel Entry Instruction Table} with or without a egress
  //  port offset. \tabTwo{Unicast \register{Tunnel Entry Instruction Table}
  //  without offset for each port}{Multicast \register{Tunnel Entry Instruction
  //  Table} with offset for each port.}
  uint64_t tunnelEntryUcMc      :  1; // bit 284

  // field: tunnelEntry
  //  This is a result field used when this entry is hit. Shall all of these
  //  packets enter into a tunnel.
  uint64_t tunnelEntry          :  1; // bit 283

  // field: ptp
  //  This is a result field used when this entry is hit. When the packet is sent
  //  to the CPU the packet will have the PTP bit in the To CPU Tag set to one.
  //  The timestamp in the To CPU Tag will also be set to the timestamp from the
  //  incoming packet.
  uint64_t ptp                  :  1; // bit 282

  // field: natOpPrio
  //  This is a result field used when this entry is hit. If multiple natOpValid
  //  are set and this prio bit is set then this natOpPtr value will be selected.
  uint64_t natOpPrio            :  1; // bit 281

  // field: natOpPtr
  //  This is a result field used when this entry is hit. NAT operation pointer.
  uint64_t natOpPtr             : 11; // bit 270 to 280

  // field: natOpValid
  //  This is a result field used when this entry is hit. NAT operation pointer is
  //  valid.
  uint64_t natOpValid           :  1; // bit 269

  // field: newL4Value
  //  This is a result field used when this entry is hit. Update the L4 SP or DP
  //  with this value
  uint64_t newL4Value           : 16; // bit 253 to 268

  // field: updateL4SpOrDp
  //  This is a result field used when this entry is hit. Update the source or
  //  destination L4 port. \tabTwo{Source L4 Port}{Destination L4 Port}
  uint64_t updateL4SpOrDp       :  1; // bit 252

  // field: enableUpdateL4
  //  This is a result field used when this entry is hit. If this entry is hit
  //  then update L4 Source Port or Destination port in ingress packet processing,
  //  this value will be used in the Egress ACL. \tabTwo{Disable}{Enable}
  uint64_t enableUpdateL4       :  1; // bit 251

  // field: newIpValue
  //  This is a result field used when this entry is hit. Update the SA or DA IPv4
  //  address value.
  uint64_t newIpValue           : 32; // bit 219 to 250

  // field: updateSaOrDa
  //  This is a result field used when this entry is hit. Update the SA or DA IPv4
  //  address. The Destiantion IP address updated will be used in the routing
  //  functionality and Egress ACL functionality. If the source IP address is
  //  updated then the updated value will be used in the egress ACL keys.
  //  \tabTwo{Source IP Address}{Destination IP Address}
  uint64_t updateSaOrDa         :  1; // bit 218

  // field: enableUpdateIp
  //  This is a result field used when this entry is hit. If this entry is hit
  //  then update SA or DA IPv4 address in ingress packet processing, this value
  //  will be used by the routing function and egress ACL if this is exists, this
  //  only works for IPv4. \tabTwo{Disable}{Enable}
  uint64_t enableUpdateIp       :  1; // bit 217

  // field: ethPrio
  //  This is a result field used when this entry is hit. If multiple updateEType
  //  are set and this prio bit is set then this updateEType will be selected.
  uint64_t ethPrio              :  1; // bit 216

  // field: vidPrio
  //  This is a result field used when this entry is hit. If multiple updateVid
  //  are set and this prio bit is set then this updateVid will be selected.
  uint64_t vidPrio              :  1; // bit 215

  // field: pcpPrio
  //  This is a result field used when this entry is hit. If multiple updatePcp
  //  are set and this prio bit is set then this updatePcp will be selected.
  uint64_t pcpPrio              :  1; // bit 214

  // field: cfiDeiPrio
  //  This is a result field used when this entry is hit. If multiple updateCfiDei
  //  are set and this prio bit is set then this updateCfiDei will be selected.
  uint64_t cfiDeiPrio           :  1; // bit 213

  // field: newEthType
  //  This is a result field used when this entry is hit. Select which TPID to use
  //  in the outer VLAN header. \tabThree{C-VLAN - 0x8100.} {S-VLAN - 0x88A8.}
  //  {User defined VLAN type from register \register{Egress Ethernet Type for
  //  VLAN tag}.}
  uint64_t newEthType           :  2; // bit 211 to 212

  // field: updateEType
  //  This is a result field used when this entry is hit. The VLANs TPID type
  //  should be updated. \tabTwo{Do not update the TPID.}{Update the TPID.}
  uint64_t updateEType          :  1; // bit 210

  // field: newVidValue
  //  This is a result field used when this entry is hit. The VID value to update
  //  to.
  uint64_t newVidValue          : 12; // bit 198 to 209

  // field: updateVid
  //  This is a result field used when this entry is hit. The VID value of the
  //  packets outermost VLAN should be updated. \tabTwo{Do not update the
  //  value.}{Update the value.}
  uint64_t updateVid            :  1; // bit 197

  // field: newPcpValue
  //  This is a result field used when this entry is hit. The PCP value to update
  //  to.
  uint64_t newPcpValue          :  3; // bit 194 to 196

  // field: updatePcp
  //  This is a result field used when this entry is hit. The PCP value of the
  //  packets outermost VLAN should be updated. \tabTwo{Do not update the
  //  value.}{Update the value.}
  uint64_t updatePcp            :  1; // bit 193

  // field: newCfiDeiValue
  //  This is a result field used when this entry is hit. The value to update to.
  uint64_t newCfiDeiValue       :  1; // bit 192

  // field: updateCfiDei
  //  This is a result field used when this entry is hit. The CFI/DEI value of the
  //  packets outermost VLAN should be updated.\tabTwo{Do not update the
  //  value.}{Update the value.}
  uint64_t updateCfiDei         :  1; // bit 191

  // field: tosMask
  //  This is a result field used when this entry is hit. Mask for TOS value.
  //  Setting a bit to one means this bit will be selected from the newTosExp
  //  field , while setting this bit to zero means that the bit will be selected
  //  from the packets already existing TOS byte bit.
  uint64_t tosMask              :  8; // bit 183 to 190

  // field: newTosExp
  //  This is a result field used when this entry is hit. New TOS/EXP value.
  uint64_t newTosExp            :  8; // bit 175 to 182

  // field: updateTosExp
  //  This is a result field used when this entry is hit. Force TOS/EXP update.
  uint64_t updateTosExp         :  1; // bit 174

  // field: counter
  //  This is a result field used when this entry is hit. Which counter in
  //  \register{Ingress Configurable ACL Match Counter} to update.
  uint64_t counter              :  6; // bit 168 to 173

  // field: updateCounter
  //  This is a result field used when this entry is hit. When set the selected
  //  statistics counter will be updated.
  uint64_t updateCounter        :  1; // bit 167

  // field: noLearning
  //  This is a result field used when this entry is hit. If set this packets MAC
  //  SA will not be learned.
  uint64_t noLearning           :  1; // bit 166

  // field: imPrio
  //  This is a result field used when this entry is hit. If multiple input mirror
  //  are set and this prio bit is set then this input mirror will be selected.
  uint64_t imPrio               :  1; // bit 165

  // field: destInputMirror
  //  This is a result field used when this entry is hit. Destination
  //  \ifdef{\texLinkAgg}{physical}{} port for input mirroring.
  uint64_t destInputMirror      :  3; // bit 162 to 164

  // field: inputMirror
  //  This is a result field used when this entry is hit. If set, input mirroring
  //  is enabled for this rule. In addition to the normal processing of the packet
  //  a copy of the unmodified input packet will be send to the destination Input
  //  Mirror port and exit on that port. The copy will be subject to the normal
  //  resource limitations in the switch.
  uint64_t inputMirror          :  1; // bit 161

  // field: destPort
  //  This is a result field used when this entry is hit. The port which the
  //  packet shall be sent to.
  uint64_t destPort             :  3; // bit 158 to 160

  // field: sendToPort
  //  This is a result field used when this entry is hit. Send the packet to a
  //  specific port. \tabTwo{Disabled.}{Send to port configured in destPort.}
  uint64_t sendToPort           :  1; // bit 157

  // field: dropEnable
  //  This is a result field used when this entry is hit. If set, the packet shall
  //  be dropped and the \register{Ingress Configurable ACL Drop} counter is
  //  incremented.
  uint64_t dropEnable           :  1; // bit 156

  // field: metaDataPrio
  //  This is a result field used when this entry is hit. If multiple ACLs hit
  //  this meta_data shall take priority.
  uint64_t metaDataPrio         :  1; // bit 155

  // field: metaData
  //  This is a result field used when this entry is hit. Meta data for packets
  //  going to the CPU.
  uint64_t metaData             : 16; // bit 139 to 154

  // field: metaDataValid
  //  This is a result field used when this entry is hit. Is the meta_data field
  //  valid.
  uint64_t metaDataValid        :  1; // bit 138

  // field: forceSendToCpuOrigPkt
  //  This is a result field used when this entry is hit. If packet shall be sent
  //  to CPU then setting this bit will force the packet to be the incoming
  //  originial packet. \ifdef{\texTunneling}{The exception to this is rule is the
  //  tunnel exit which will still be carried out.}{}.
  uint64_t forceSendToCpuOrigPkt:  1; // bit 137

  // field: sendToCpu
  //  This is a result field used when this entry is hit. If set, the packet shall
  //  be sent to the CPU port.
  uint64_t sendToCpu            :  1; // bit 136

  // field: compareData
  //  The data which shall be compared in this entry.
  uint8_t compareData[17]; // bit 1 to 135

  // field: valid
  //  Is this entry valid. \tabTwo{No}{Yes}.
  uint64_t valid                :  1; // bit 0
} t_IngressConfigurableACL1LargeTable;

// ------- Struct declaration for register: PrioShaperRateConfiguration --------- 
typedef struct {

  // field: ifgCorrection
  //  Extra bytes per packet to correct for IFG in byte mode. Default is 4 byte
  //  FCS plus 20 byte IFG.
  uint64_t ifgCorrection  :  8; // bit 17 to 24

  // field: tick
  //  Select one of the five available core ticks. The tick frequencies are
  //  configured globaly in the core Tick Configuration register.
  uint64_t tick           :  3; // bit 14 to 16

  // field: tokens
  //  The number of tokens added each tick
  uint64_t tokens         : 13; // bit 1 to 13

  // field: packetsNotBytes
  //  If set the bucket will count packets, if cleared bytes
  uint64_t packetsNotBytes:  1; // bit 0
} t_PrioShaperRateConfiguration;

// ------- Struct declaration for register: IPPDebugnextHopPtrLpmHit --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value:  1; // bit 0
} t_IPPDebugnextHopPtrLpmHit;

// ------- Struct declaration for register: GREDecoderDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_GREDecoderDrop;

// ------- Struct declaration for register: SoftwareAgingStartLatch --------- 
typedef struct {

  // field: start
  //  When register is written with start bit set an age out process is started.
  uint64_t start:  1; // bit 0
} t_SoftwareAgingStartLatch;

// ------- Struct declaration for register: IPMulticastRoutedCounter --------- 
typedef struct {

  // field: packets
  //  Number of packets.
  uint64_t packets: 32; // bit 0 to 31
} t_IPMulticastRoutedCounter;

// ------- Struct declaration for register: PFCIncCountersforingressports0to5 --------- 
typedef struct {

  // field: cells
  //  Number of cells
  uint64_t cells: 10; // bit 0 to 9
} t_PFCIncCountersforingressports0to5;

// ------- Struct declaration for register: DebugCounterfromPortSetup --------- 
typedef struct {

  // field: hitValue
  //  Value to compare to update debug counter. Both the incoming value and this
  //  value is ANDed with the mask before comparsion is carried out. If comparsion
  //  results in true the counter is updated
  uint64_t hitValue:  6; // bit 6 to 11

  // field: mask
  //  Mask for comparison to update debug counter.
  uint64_t mask    :  6; // bit 0 to 5
} t_DebugCounterfromPortSetup;

// ------- Struct declaration for register: EPPDebugisIPv4 --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value:  1; // bit 0
} t_EPPDebugisIPv4;

// ------- Struct declaration for register: TunnelExitTooSmallPacketModificationDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_TunnelExitTooSmallPacketModificationDrop;

// ------- Struct declaration for register: DebugCounterdstPortmaskSetup --------- 
typedef struct {

  // field: hitValue
  //  Value to compare to update debug counter. Both the incoming value and this
  //  value is ANDed with the mask before comparsion is carried out. If comparsion
  //  results in true the counter is updated
  uint64_t hitValue:  6; // bit 6 to 11

  // field: mask
  //  Mask for comparison to update debug counter.
  uint64_t mask    :  6; // bit 0 to 5
} t_DebugCounterdstPortmaskSetup;

// ------- Struct declaration for register: HitUpdateDataFIFO --------- 
typedef struct {

  // field: valid
  //  \tabTwo{Empty FIFO, entry is not valid}{Valid entry}.
  uint64_t valid               :  1; // bit 73

  // field: updateCam
  //  If set, the corresponding entry has as valid aging request.
  uint64_t updateCam           :  1; // bit 72

  // field: camIndex
  //  Index to the entry in \register{L2 Aging Collision Table}.
  uint64_t camIndex            :  5; // bit 67 to 71

  // field: camStatus
  //  Entry status which shall be written to \register{L2 Aging Collision Table}.
  //  From bit 0 to bit 2 represents valid bit, static bit and hit bit
  //  respectively.
  uint64_t camStatus           :  3; // bit 64 to 66

  // field: reserved3
  //  Reserved.
  uint64_t reserved3           :  4; // bit 60 to 63

  // field: updateHashBucket3
  //  If set, the corresponding bucket has a valid hit update request.
  uint64_t updateHashBucket3   :  1; // bit 59

  // field: addressOfHashBucket3
  //  Address into bucket 3 of the \register{L2 Aging Table}.
  uint64_t addressOfHashBucket3: 10; // bit 49 to 58

  // field: statusOfHashBucket3
  //  Entry status which shall be written to \register{L2 Aging Table}. From bit 0
  //  to bit 2 represents valid bit, static bit and hit bit respectively.
  uint64_t statusOfHashBucket3 :  3; // bit 46 to 48

  // field: updateHashBucket2
  //  If set, the corresponding bucket has a valid hit update request.
  uint64_t updateHashBucket2   :  1; // bit 45

  // field: addressOfHashBucket2
  //  Address into bucket 2 of the \register{L2 Aging Table}.
  uint64_t addressOfHashBucket2: 10; // bit 35 to 44

  // field: statusOfHashBucket2
  //  Entry status which shall be written to \register{L2 Aging Table}. From bit 0
  //  to bit 2 represents valid bit, static bit and hit bit respectively.
  uint64_t statusOfHashBucket2 :  3; // bit 32 to 34

  // field: reserved1
  //  Reserved.
  uint64_t reserved1           :  4; // bit 28 to 31

  // field: updateHashBucket1
  //  If set, the corresponding bucket has a valid hit update request.
  uint64_t updateHashBucket1   :  1; // bit 27

  // field: addressOfHashBucket1
  //  Address into bucket 1 of the \register{L2 Aging Table}.
  uint64_t addressOfHashBucket1: 10; // bit 17 to 26

  // field: statusOfHashBucket1
  //  Entry status which shall be written to \register{L2 Aging Table}. From bit 0
  //  to bit 2 represents valid bit, static bit and hit bit respectively.
  uint64_t statusOfHashBucket1 :  3; // bit 14 to 16

  // field: updateHashBucket0
  //  If set, the corresponding bucket has a valid hit update request.
  uint64_t updateHashBucket0   :  1; // bit 13

  // field: addressOfHashBucket0
  //  Address into bucket 0 of the \register{L2 Aging Table}.
  uint64_t addressOfHashBucket0: 10; // bit 3 to 12

  // field: statusOfHashBucket0
  //  Entry status which shall be written to \register{L2 Aging Table}. From bit 0
  //  to bit 2 represents valid bit, static bit and hit bit respectively.
  uint64_t statusOfHashBucket0 :  3; // bit 0 to 2
} t_HitUpdateDataFIFO;

// ------- Struct declaration for register: DebugCounternextHopPtrHashSetup --------- 
typedef struct {

  // field: hitValue
  //  Value to compare to update debug counter. Both the incoming value and this
  //  value is ANDed with the mask before comparsion is carried out. If comparsion
  //  results in true the counter is updated
  uint64_t hitValue: 10; // bit 10 to 19

  // field: mask
  //  Mask for comparison to update debug counter.
  uint64_t mask    : 10; // bit 0 to 9
} t_DebugCounternextHopPtrHashSetup;

// ------- Struct declaration for register: SendtoCPU --------- 
typedef struct {

  // field: cpuMacAddr
  //  Packets with this destination MAC address will be sent to the CPU. Only
  //  valid if \field{Send to CPU}{uniqueCpuMac} on the source port is set.
  uint64_t cpuMacAddr  : 48; // bit 18 to 65

  // field: uniqueCpuMac
  //  If set then unicast packets can not be switched or routed to the CPU port.
  //  Other mechanism for sending to the CPU port are not affected (e.g. ACL's).
  //  This also enables detection of a specific MAC address, \field{Send to
  //  CPU}{cpuMacAddr}, that will be sent to the CPU.
  uint64_t uniqueCpuMac:  6; // bit 12 to 17

  // field: allowRstBpdu
  //  Send to CPU portmask, bit 0 port 0, bit 1 port 1 etc. If the source port bit
  //  is set then packets that have the destination MAC address equal to
  //  01:00:0C:CC:CC:CD are sent to the CPU port.
  uint64_t allowRstBpdu:  6; // bit 6 to 11

  // field: allowBpdu
  //  Send to CPU portmask, bit 0 port 0, bit 1 port 1 etc. If source port bit is
  //  set then packets that have the destination MAC address equal to
  //  01:80:C2:00:00:00 are sent to the CPU port.
  uint64_t allowBpdu   :  6; // bit 0 to 5
} t_SendtoCPU;

// ------- Struct declaration for register: L2FloodingStormControlBucketCapacityConfiguration --------- 
typedef struct {

  // field: bucketCapacity
  //  Capacity of the token bucket
  uint64_t bucketCapacity: 16; // bit 0 to 15
} t_L2FloodingStormControlBucketCapacityConfiguration;

// ------- Struct declaration for register: DebugCountersrcPortSetup --------- 
typedef struct {

  // field: hitValue
  //  Value to compare to update debug counter. Both the incoming value and this
  //  value is ANDed with the mask before comparsion is carried out. If comparsion
  //  results in true the counter is updated
  uint64_t hitValue:  3; // bit 3 to 5

  // field: mask
  //  Mask for comparison to update debug counter.
  uint64_t mask    :  3; // bit 0 to 2
} t_DebugCountersrcPortSetup;

// ------- Struct declaration for register: IPv6ClassofServiceFieldToPacketColorMappingTable --------- 
typedef struct {

  // field: color
  //  Packet initial color.
  uint64_t color:  2; // bit 0 to 1
} t_IPv6ClassofServiceFieldToPacketColorMappingTable;

// ------- Struct declaration for register: MinimumAllowedVLANDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_MinimumAllowedVLANDrop;

// ------- Struct declaration for register: L2DestinationTable --------- 
typedef struct {

  // field: metaData
  //  Meta data for to CPU tag.
  uint64_t metaData             : 16; // bit 10 to 25

  // field: l2ActionTableSaStatus
  //  The status SA bit to be used in the addressing for the table \register{L2
  //  Action Table} Lookup.
  uint64_t l2ActionTableSaStatus:  1; // bit 9

  // field: l2ActionTableDaStatus
  //  The status DA bit to be used in the addressing for the table \register{L2
  //  Action Table} Lookup.
  uint64_t l2ActionTableDaStatus:  1; // bit 8

  // field: pktDrop
  //  If set, the packet will be dropped and the \register{L2 Lookup Drop}
  //  incremented.
  uint64_t pktDrop              :  1; // bit 7

  // field: destPortormcAddr
  //  Destination port number or pointer into the \register{L2 Multicast Table}.
  uint64_t destPortormcAddr     :  6; // bit 1 to 6

  // field: uc
  //  Unicast if set; multicast if cleared. Multicast means that a lookup to the
  //  \register{L2 Multicast Table} will occur and determine a list of destination
  //  ports.
  uint64_t uc                   :  1; // bit 0
} t_L2DestinationTable;

// ------- Struct declaration for register: IPPDebugdropPktAfterL3Decode --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value:  1; // bit 0
} t_IPPDebugdropPktAfterL3Decode;

// ------- Struct declaration for register: EgressMultipleSpanningTreeState --------- 
typedef struct {

  // field: portSptState
  //  The egress spanning tree state for this MSTI. Bit[1:0] is the state for port
  //  \#0, bit[3:2] is the state for port \#1, etc. \tabThree{Forwarding}
  //  {Discarding} {Learning}
  uint64_t portSptState: 12; // bit 0 to 11
} t_EgressMultipleSpanningTreeState;

// ------- Struct declaration for register: EPPDebugdoPktUpdate --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value:  6; // bit 0 to 5
} t_EPPDebugdoPktUpdate;

// ------- Struct declaration for register: TunnelExitMissActionDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_TunnelExitMissActionDrop;

// ------- Struct declaration for register: PortTCXonTotalThreshold --------- 
typedef struct {

  // field: cells
  //  Xon threshold for the sum of reserved and FFA cells used for this source
  //  port and traffic class combination
  uint64_t cells: 10; // bit 0 to 9
} t_PortTCXonTotalThreshold;

// ------- Struct declaration for register: DebugCounterdebugMatchIPP0Setup --------- 
typedef struct {

  // field: hitValue
  //  Value to compare to update debug counter. Both the incoming value and this
  //  value is ANDed with the mask before comparsion is carried out. If comparsion
  //  results in true the counter is updated
  uint64_t hitValue: 22; // bit 22 to 43

  // field: mask
  //  Mask for comparison to update debug counter.
  uint64_t mask    : 22; // bit 0 to 21
} t_DebugCounterdebugMatchIPP0Setup;

// ------- Struct declaration for register: HardwareLearningCounter --------- 
typedef struct {

  // field: cnt
  //  Number of learned L2 entries.
  uint64_t cnt: 13; // bit 0 to 12
} t_HardwareLearningCounter;

// ------- Struct declaration for register: L2ReservedMulticastAddressAction --------- 
typedef struct {

  // field: sendToCpuMask
  //  Received packets on these source ports will be sent to the CPU. Bit 0
  //  represents port 0, bit 1 represents port 1 etc. \ifdef{\texLLDP}{LLDP frames
  //  sent to the CPU takes priority over this.}{}
  uint64_t sendToCpuMask:  6; // bit 6 to 11

  // field: dropMask
  //  Determines which source ports that are not allowed to receive this multicast
  //  address. Each bit set to 1 will result in dropping this multicast address on
  //  that source port. Bit 0 is port 0, bit 1 is port 1 etc. Each drop will be
  //  counted in \register{L2 Reserved Multicast Address Drop}.
  uint64_t dropMask     :  6; // bit 0 to 5
} t_L2ReservedMulticastAddressAction;

// ------- Struct declaration for register: EPPDebugreQueuePkt --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value:  1; // bit 0
} t_EPPDebugreQueuePkt;

// ------- Struct declaration for register: SoftwareAgingEnable --------- 
typedef struct {

  // field: enable
  //  Enable software aging.
  uint64_t enable:  1; // bit 0
} t_SoftwareAgingEnable;

// ------- Struct declaration for register: L2ReservedMulticastAddressBase --------- 
typedef struct {

  // field: mask
  //  Bit comparison mask for the lower 2 bytes in macBase (marked with XX as in
  //  01:80:c2:XX:XX). If a bit is set in the mask then the corresponding bit will
  //  be compared. Otherwise the bits are dont care.
  uint64_t mask   : 16; // bit 40 to 55

  // field: macBase
  //  The first 40 bits of the reserved MAC address, and the lower 16 bits of it
  //  can be masked. The default is 01:80:c2:00:00
  uint64_t macBase: 40; // bit 0 to 39
} t_L2ReservedMulticastAddressBase;

// ------- Struct declaration for register: IPPDebugspVidOp --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value:  3; // bit 0 to 2
} t_IPPDebugspVidOp;

// ------- Struct declaration for register: EgressConfigurableACL0Selection --------- 
typedef struct {

  // field: selectSmallOrLarge
  //  If set to zero then small hash table is selected. If set to one then large
  //  hash table is selected.
  uint64_t selectSmallOrLarge:  1; // bit 1

  // field: selectTcamOrTable
  //  If set to zero then TCAM answer is selected. If set to one then hash table
  //  answer is selected.
  uint64_t selectTcamOrTable :  1; // bit 0
} t_EgressConfigurableACL0Selection;

// ------- Struct declaration for register: L2BroadcastStormControlBucketThresholdConfiguration --------- 
typedef struct {

  // field: threshold
  //  Minimum number of tokens in bucket for the status to be set to accept.
  uint64_t threshold: 16; // bit 0 to 15
} t_L2BroadcastStormControlBucketThresholdConfiguration;

// ------- Struct declaration for register: BufferOverflowDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_BufferOverflowDrop;

// ------- Struct declaration for register: ReservedMACSADrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_ReservedMACSADrop;

// ------- Struct declaration for register: EgressConfigurableACLDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_EgressConfigurableACLDrop;

// ------- Struct declaration for register: ReservedMACDADrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_ReservedMACDADrop;

// ------- Struct declaration for register: BeginningofPacketTunnelEntryInstructionTable --------- 
typedef struct {

  // field: hasUdp
  //  If the header is a IPv4 or IPv6 then a an UDP header is after the IP header.
  //  \tabTwo{No.}{Yes.}
  uint64_t hasUdp        :  1; // bit 8

  // field: ipHeaderOffset
  //  Where does the IPv4/IPv6 header start in this header. Only valid if the
  //  L3-Header type is IPv4 or IPv6.
  uint64_t ipHeaderOffset:  6; // bit 2 to 7

  // field: l3Type
  //  Inserted header type, when selecting MPLS/Other no updates will be done to
  //  the data. \tabFour{IPv4}{IPv6}{MPLS/Other.}{Reserved.}
  uint64_t l3Type        :  2; // bit 0 to 1
} t_BeginningofPacketTunnelEntryInstructionTable;

// ------- Struct declaration for register: TOSQoSMappingTable --------- 
typedef struct {

  // field: newExp
  //  New Exp value to be used.
  uint64_t newExp      :  3; // bit 15 to 17

  // field: updateExp
  //  If the packet enterns a new MPLS tunnel using the \register{Next Hop Packet
  //  Insert MPLS Header} then use this Exp for the outermost MPLS label.
  //  \tabTwo{No. Dont Remap.}{Yes. Remap to this new value}
  uint64_t updateExp   :  1; // bit 14

  // field: newTos
  //  The outgoing new TOS bits
  uint64_t newTos      :  8; // bit 6 to 13

  // field: pcp
  //  Packets new PCP
  uint64_t pcp         :  3; // bit 3 to 5

  // field: updatePcp
  //  Update Pcp field in outgoing packet. \tabTwo{Do not update.}{Update.}
  uint64_t updatePcp   :  1; // bit 2

  // field: cfiDei
  //  Packets new CFI/DEI
  uint64_t cfiDei      :  1; // bit 1

  // field: updateCfiDei
  //  Update CfiDei field in outgoing packet. \tabTwo{Do not update.}{Update.}
  uint64_t updateCfiDei:  1; // bit 0
} t_TOSQoSMappingTable;

// ------- Struct declaration for register: L2LookupCollisionTableMasks --------- 
typedef struct {

  // field: gid
  //  Global identifier for learning mask
  uint64_t gid    : 12; // bit 48 to 59

  // field: macAddr
  //  MAC address mask
  uint64_t macAddr: 48; // bit 0 to 47
} t_L2LookupCollisionTableMasks;

// ------- Struct declaration for register: ForwardFromCPU --------- 
typedef struct {

  // field: enable
  //  If set, any frame received on the CPU port is forwarded without
  //  consideration of the egress port's spanning tree state.
  uint64_t enable:  1; // bit 0
} t_ForwardFromCPU;

// ------- Struct declaration for register: IngressConfigurableACL2RulesSetup --------- 
typedef struct {

  // field: fieldSelectBitmask
  //  Bitmask of which fields to select. Set a bit to one to select this specific
  //  field, set zero to not select field. At Maximum 20 bits should be set.
  uint64_t fieldSelectBitmask: 28; // bit 0 to 27
} t_IngressConfigurableACL2RulesSetup;

// ------- Struct declaration for register: CheckIPv4HeaderChecksum --------- 
typedef struct {

  // field: dropErrorChkSum
  //  If set, always calculate the checksum of the received IPv4 packet. If the
  //  calculated value does not match the IPv4 checksum field, the packet is
  //  dropped.
  uint64_t dropErrorChkSum:  1; // bit 0
} t_CheckIPv4HeaderChecksum;

// ------- Struct declaration for register: UnknownEgressDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_UnknownEgressDrop;

// ------- Struct declaration for register: EgressNATHitStatus --------- 
typedef struct {

  // field: hit
  //  If set, the corresponding entry in the \register{Egress NAT Operation} is
  //  hit.
  uint64_t hit:  1; // bit 0
} t_EgressNATHitStatus;

// ------- Struct declaration for register: HitUpdateDataFIFOHighWatermarkLevel --------- 
typedef struct {

  // field: level
  //  Number of used entries
  uint64_t level:  6; // bit 0 to 5
} t_HitUpdateDataFIFOHighWatermarkLevel;

// ------- Struct declaration for register: ForceNonVLANPacketToSpecificQueue --------- 
typedef struct {

  // field: eQueue
  //  The egress queue to be assigned if the forceQueue field in this entry is set
  //  to 1.
  uint64_t eQueue    :  3; // bit 1 to 3

  // field: forceQueue
  //  If set, the packet shall have a forced egress queue. Please see Egress Queue
  //  Selection Diagram in Figure \ref{fig:determineEgressQueue}
  uint64_t forceQueue:  1; // bit 0
} t_ForceNonVLANPacketToSpecificQueue;

// ------- Struct declaration for register: PFCDecCountersforingressports0to5 --------- 
typedef struct {

  // field: cells
  //  Number of cells
  uint64_t cells: 10; // bit 0 to 9
} t_PFCDecCountersforingressports0to5;

// ------- Struct declaration for register: EPPDebugfromPort --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value:  6; // bit 0 to 5
} t_EPPDebugfromPort;

// ------- Struct declaration for register: DebugIPPCounter --------- 
typedef struct {

  // field: packets
  //  Number of packets.
  uint64_t packets: 16; // bit 0 to 15
} t_DebugIPPCounter;

// ------- Struct declaration for register: DebugCounternextHopPtrFinalSetup --------- 
typedef struct {

  // field: hitValue
  //  Value to compare to update debug counter. Both the incoming value and this
  //  value is ANDed with the mask before comparsion is carried out. If comparsion
  //  results in true the counter is updated
  uint64_t hitValue: 10; // bit 10 to 19

  // field: mask
  //  Mask for comparison to update debug counter.
  uint64_t mask    : 10; // bit 0 to 9
} t_DebugCounternextHopPtrFinalSetup;

// ------- Struct declaration for register: PrioShaperBucketThresholdConfiguration --------- 
typedef struct {

  // field: threshold
  //  Minimum number of tokens in bucket for the status to be set to accept.
  uint64_t threshold: 17; // bit 0 to 16
} t_PrioShaperBucketThresholdConfiguration;

// ------- Struct declaration for register: IPChecksumDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_IPChecksumDrop;

// ------- Struct declaration for register: IPPDebugfinalVid --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value: 13; // bit 0 to 12
} t_IPPDebugfinalVid;

// ------- Struct declaration for register: PortTailDropFFAThreshold --------- 
typedef struct {

  // field: trip
  //  \begin{fieldValues} \dscValue [0] Normal operation \dscValue [1] Force this
  //  threshold to be counted as exceeded \end{fieldValues} Only valid if this
  //  tail-drop threshold is enabled.
  uint64_t trip  :  1; // bit 11

  // field: enable
  //  \begin{fieldValues} \dscValue [0] This tail-drop threshold is disabled
  //  \dscValue [1] This tail-drop threshold is enabled\end{fieldValues}
  uint64_t enable:  1; // bit 10

  // field: cells
  //  Tail-drop threshold in number of cells. When the FFA cells used by the
  //  source port reaches this threshold no further packets will be accepted for
  //  this source port
  uint64_t cells : 10; // bit 0 to 9
} t_PortTailDropFFAThreshold;

// ------- Struct declaration for register: L2LookupDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_L2LookupDrop;

// ------- Struct declaration for register: MPLSQoSMappingTable --------- 
typedef struct {

  // field: exp
  //  The outgoing Exp value for this queue in the outermost MPLS label.
  uint64_t exp         :  3; // bit 6 to 8

  // field: pcp
  //  Packets new PCP.
  uint64_t pcp         :  3; // bit 3 to 5

  // field: updatePcp
  //  Update Pcp field in outgoing packet. \tabTwo{Do not update.}{Update.}
  uint64_t updatePcp   :  1; // bit 2

  // field: cfiDei
  //  Packets new CFI/DEI.
  uint64_t cfiDei      :  1; // bit 1

  // field: updateCfiDei
  //  Update CfiDei field in outgoing packet. \tabTwo{Do not update.}{Update.}
  uint64_t updateCfiDei:  1; // bit 0
} t_MPLSQoSMappingTable;

// ------- Struct declaration for register: ForceUnknownL3PacketToSpecificColor --------- 
typedef struct {

  // field: color
  //  Initial color of the packet
  uint64_t color     :  2; // bit 1 to 2

  // field: forceColor
  //  When set, unknown L3 packet types are forced to a color.
  uint64_t forceColor:  1; // bit 0
} t_ForceUnknownL3PacketToSpecificColor;

// ------- Struct declaration for register: TunnelExitTooSmallPacketModificationToSmallDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_TunnelExitTooSmallPacketModificationToSmallDrop;

// ------- Struct declaration for register: DebugCounternextHopPtrLpmSetup --------- 
typedef struct {

  // field: hitValue
  //  Value to compare to update debug counter. Both the incoming value and this
  //  value is ANDed with the mask before comparsion is carried out. If comparsion
  //  results in true the counter is updated
  uint64_t hitValue: 10; // bit 10 to 19

  // field: mask
  //  Mask for comparison to update debug counter.
  uint64_t mask    : 10; // bit 0 to 9
} t_DebugCounternextHopPtrLpmSetup;

// ------- Struct declaration for register: IPv6ClassofServiceFieldToEgressQueueMappingTable --------- 
typedef struct {

  // field: pQueue
  //  Egress queue.
  uint64_t pQueue:  3; // bit 0 to 2
} t_IPv6ClassofServiceFieldToEgressQueueMappingTable;

// ------- Struct declaration for register: DebugCountervlanVidOpSetup --------- 
typedef struct {

  // field: hitValue
  //  Value to compare to update debug counter. Both the incoming value and this
  //  value is ANDed with the mask before comparsion is carried out. If comparsion
  //  results in true the counter is updated
  uint64_t hitValue:  3; // bit 3 to 5

  // field: mask
  //  Mask for comparison to update debug counter.
  uint64_t mask    :  3; // bit 0 to 2
} t_DebugCountervlanVidOpSetup;

// ------- Struct declaration for register: QueueShaperRateConfiguration --------- 
typedef struct {

  // field: ifgCorrection
  //  Extra bytes per packet to correct for IFG in byte mode. Default is 4 byte
  //  FCS plus 20 byte IFG.
  uint64_t ifgCorrection  :  8; // bit 17 to 24

  // field: tick
  //  Select one of the five available core ticks. The tick frequencies are
  //  configured globaly in the core Tick Configuration register.
  uint64_t tick           :  3; // bit 14 to 16

  // field: tokens
  //  The number of tokens added each tick
  uint64_t tokens         : 13; // bit 1 to 13

  // field: packetsNotBytes
  //  If set the bucket will count packets, if cleared bytes
  uint64_t packetsNotBytes:  1; // bit 0
} t_QueueShaperRateConfiguration;

// ------- Struct declaration for register: EgressQueueDepth --------- 
typedef struct {

  // field: packets
  //  Number of packets currently queued.
  uint64_t packets: 10; // bit 0 to 9
} t_EgressQueueDepth;

// ------- Struct declaration for register: IngressConfigurableACL2TCAMAnswer --------- 
typedef struct {

  // field: forceVidPrio
  //  If multiple forceVid are set and this prio bit is set then this forceVid
  //  value will be selected.
  uint64_t forceVidPrio         :  1; // bit 184

  // field: forceVid
  //  The new Ingress VID.
  uint64_t forceVid             : 12; // bit 172 to 183

  // field: forceVidValid
  //  Override the Ingress VID, see chapter \hyperref[chap:VLAN Processing]{VLAN
  //  Processing}.
  uint64_t forceVidValid        :  1; // bit 171

  // field: forceQueuePrio
  //  If multiple forceQueue are set and this prio bit is set then this forceQueue
  //  value will be selected.
  uint64_t forceQueuePrio       :  1; // bit 170

  // field: eQueue
  //  The egress queue to be assigned if the forceQueue field in this entry is set
  //  to 1.
  uint64_t eQueue               :  3; // bit 167 to 169

  // field: forceQueue
  //  If set, the packet shall have a forced egress queue. Please see Egress Queue
  //  Selection Diagram in Figure \ref{fig:determineEgressQueue}
  uint64_t forceQueue           :  1; // bit 166

  // field: mmpOrder
  //  Ingress MMP pointer order.
  uint64_t mmpOrder             :  2; // bit 164 to 165

  // field: mmpPtr
  //  Ingress MMP pointer.
  uint64_t mmpPtr               :  5; // bit 159 to 163

  // field: mmpValid
  //  If set, this entry contains a valid MMP pointer
  uint64_t mmpValid             :  1; // bit 158

  // field: forceColorPrio
  //  If multiple forceColor are set and this prio bit is set then this forceVid
  //  value will be selected.
  uint64_t forceColorPrio       :  1; // bit 157

  // field: color
  //  Initial color of the packet if the forceColor field is set.
  uint64_t color                :  2; // bit 155 to 156

  // field: forceColor
  //  If set, the packet shall have a forced color.
  uint64_t forceColor           :  1; // bit 154

  // field: tunnelEntryPrio
  //  If multiple tunnelEntry are set and this prio bit is set then this
  //  tunnelEntryPtr will be selected.
  uint64_t tunnelEntryPrio      :  1; // bit 153

  // field: tunnelEntryPtr
  //  The tunnel entry which this packet shall enter upon exiting the switch.
  uint64_t tunnelEntryPtr       :  4; // bit 149 to 152

  // field: tunnelEntryUcMc
  //  Shall this entry point to the \register{Tunnel Entry Instruction Table} with
  //  or without a egress port offset. \tabTwo{Unicast \register{Tunnel Entry
  //  Instruction Table} without offset for each port}{Multicast \register{Tunnel
  //  Entry Instruction Table} with offset for each port.}
  uint64_t tunnelEntryUcMc      :  1; // bit 148

  // field: tunnelEntry
  //  Shall all of these packets enter into a tunnel.
  uint64_t tunnelEntry          :  1; // bit 147

  // field: ptp
  //  When the packet is sent to the CPU the packet will have the PTP bit in the
  //  To CPU Tag set to one. The timestamp in the To CPU Tag will also be set to
  //  the timestamp from the incoming packet.
  uint64_t ptp                  :  1; // bit 146

  // field: natOpPrio
  //  If multiple natOpValid are set and this prio bit is set then this natOpPtr
  //  value will be selected.
  uint64_t natOpPrio            :  1; // bit 145

  // field: natOpPtr
  //  NAT operation pointer.
  uint64_t natOpPtr             : 11; // bit 134 to 144

  // field: natOpValid
  //  NAT operation pointer is valid.
  uint64_t natOpValid           :  1; // bit 133

  // field: newL4Value
  //  Update the L4 SP or DP with this value
  uint64_t newL4Value           : 16; // bit 117 to 132

  // field: updateL4SpOrDp
  //  Update the source or destination L4 port. \tabTwo{Source L4
  //  Port}{Destination L4 Port}
  uint64_t updateL4SpOrDp       :  1; // bit 116

  // field: enableUpdateL4
  //  If this entry is hit then update L4 Source Port or Destination port in
  //  ingress packet processing, this value will be used in the Egress ACL.
  //  \tabTwo{Disable}{Enable}
  uint64_t enableUpdateL4       :  1; // bit 115

  // field: newIpValue
  //  Update the SA or DA IPv4 address value.
  uint64_t newIpValue           : 32; // bit 83 to 114

  // field: updateSaOrDa
  //  Update the SA or DA IPv4 address. The Destiantion IP address updated will be
  //  used in the routing functionality and Egress ACL functionality. If the
  //  source IP address is updated then the updated value will be used in the
  //  egress ACL keys. \tabTwo{Source IP Address}{Destination IP Address}
  uint64_t updateSaOrDa         :  1; // bit 82

  // field: enableUpdateIp
  //  If this entry is hit then update SA or DA IPv4 address in ingress packet
  //  processing, this value will be used by the routing function and egress ACL
  //  if this is exists, this only works for IPv4. \tabTwo{Disable}{Enable}
  uint64_t enableUpdateIp       :  1; // bit 81

  // field: ethPrio
  //  If multiple updateEType are set and this prio bit is set then this
  //  updateEType will be selected.
  uint64_t ethPrio              :  1; // bit 80

  // field: vidPrio
  //  If multiple updateVid are set and this prio bit is set then this updateVid
  //  will be selected.
  uint64_t vidPrio              :  1; // bit 79

  // field: pcpPrio
  //  If multiple updatePcp are set and this prio bit is set then this updatePcp
  //  will be selected.
  uint64_t pcpPrio              :  1; // bit 78

  // field: cfiDeiPrio
  //  If multiple updateCfiDei are set and this prio bit is set then this
  //  updateCfiDei will be selected.
  uint64_t cfiDeiPrio           :  1; // bit 77

  // field: newEthType
  //  Select which TPID to use in the outer VLAN header. \tabThree{C-VLAN -
  //  0x8100.} {S-VLAN - 0x88A8.} {User defined VLAN type from register
  //  \register{Egress Ethernet Type for VLAN tag}.}
  uint64_t newEthType           :  2; // bit 75 to 76

  // field: updateEType
  //  The VLANs TPID type should be updated. \tabTwo{Do not update the
  //  TPID.}{Update the TPID.}
  uint64_t updateEType          :  1; // bit 74

  // field: newVidValue
  //  The VID value to update to.
  uint64_t newVidValue          : 12; // bit 62 to 73

  // field: updateVid
  //  The VID value of the packets outermost VLAN should be updated. \tabTwo{Do
  //  not update the value.}{Update the value.}
  uint64_t updateVid            :  1; // bit 61

  // field: newPcpValue
  //  The PCP value to update to.
  uint64_t newPcpValue          :  3; // bit 58 to 60

  // field: updatePcp
  //  The PCP value of the packets outermost VLAN should be updated. \tabTwo{Do
  //  not update the value.}{Update the value.}
  uint64_t updatePcp            :  1; // bit 57

  // field: newCfiDeiValue
  //  The value to update to.
  uint64_t newCfiDeiValue       :  1; // bit 56

  // field: updateCfiDei
  //  The CFI/DEI value of the packets outermost VLAN should be updated.\tabTwo{Do
  //  not update the value.}{Update the value.}
  uint64_t updateCfiDei         :  1; // bit 55

  // field: tosMask
  //  Mask for TOS value. Setting a bit to one means this bit will be selected
  //  from the newTosExp field , while setting this bit to zero means that the bit
  //  will be selected from the packets already existing TOS byte bit.
  uint64_t tosMask              :  8; // bit 47 to 54

  // field: newTosExp
  //  New TOS/EXP value.
  uint64_t newTosExp            :  8; // bit 39 to 46

  // field: updateTosExp
  //  Force TOS/EXP update.
  uint64_t updateTosExp         :  1; // bit 38

  // field: counter
  //  Which counter in \register{Ingress Configurable ACL Match Counter} to
  //  update.
  uint64_t counter              :  6; // bit 32 to 37

  // field: updateCounter
  //  When set the selected statistics counter will be updated.
  uint64_t updateCounter        :  1; // bit 31

  // field: noLearning
  //  If set this packets MAC SA will not be learned.
  uint64_t noLearning           :  1; // bit 30

  // field: imPrio
  //  If multiple input mirror are set and this prio bit is set then this input
  //  mirror will be selected.
  uint64_t imPrio               :  1; // bit 29

  // field: destInputMirror
  //  Destination \ifdef{\texLinkAgg}{physical}{} port for input mirroring.
  uint64_t destInputMirror      :  3; // bit 26 to 28

  // field: inputMirror
  //  If set, input mirroring is enabled for this rule. In addition to the normal
  //  processing of the packet a copy of the unmodified input packet will be send
  //  to the destination Input Mirror port and exit on that port. The copy will be
  //  subject to the normal resource limitations in the switch.
  uint64_t inputMirror          :  1; // bit 25

  // field: destPort
  //  The port which the packet shall be sent to.
  uint64_t destPort             :  3; // bit 22 to 24

  // field: sendToPort
  //  Send the packet to a specific port. \tabTwo{Disabled.}{Send to port
  //  configured in destPort.}
  uint64_t sendToPort           :  1; // bit 21

  // field: dropEnable
  //  If set, the packet shall be dropped and the \register{Ingress Configurable
  //  ACL Drop} counter is incremented.
  uint64_t dropEnable           :  1; // bit 20

  // field: metaDataPrio
  //  If multiple ACLs hit this meta_data shall take priority.
  uint64_t metaDataPrio         :  1; // bit 19

  // field: metaData
  //  Meta data for packets going to the CPU.
  uint64_t metaData             : 16; // bit 3 to 18

  // field: metaDataValid
  //  Is the meta_data field valid.
  uint64_t metaDataValid        :  1; // bit 2

  // field: forceSendToCpuOrigPkt
  //  If packet shall be sent to CPU then setting this bit will force the packet
  //  to be the incoming originial packet. \ifdef{\texTunneling}{The exception to
  //  this is rule is the tunnel exit which will still be carried out.}{}.
  uint64_t forceSendToCpuOrigPkt:  1; // bit 1

  // field: sendToCpu
  //  If set, the packet shall be sent to the CPU port.
  uint64_t sendToCpu            :  1; // bit 0
} t_IngressConfigurableACL2TCAMAnswer;

// ------- Struct declaration for register: ESPHeaderPacketDecoderOptions --------- 
typedef struct {

  // field: toCpu
  //  If a packet comes in on this source port then send the packet to the CPU
  //  port.\tabTwo{Do not sent to CPU. Normal Processing of packet.}{Send to CPU ,
  //  bypass normal packet processing.}
  uint64_t toCpu  :  6; // bit 15 to 20

  // field: drop
  //  If a packet comes in on this source port then drop the packet.\tabTwo{Do not
  //  drop this packet.}{Drop this packet and update the drop counter.}
  uint64_t drop   :  6; // bit 9 to 14

  // field: l4Proto
  //  The value to be used to find this packet type.
  uint64_t l4Proto:  8; // bit 1 to 8

  // field: enabled
  //  Is this decoding enabled. \tabTwo{No}{Yes}
  uint64_t enabled:  1; // bit 0
} t_ESPHeaderPacketDecoderOptions;

// ------- Struct declaration for register: DefaultPacketToCPUModification --------- 
typedef struct {

  // field: origCpuPkt
  //  Force the packet to the CPU to be the originial,unmodified, packet.
  //  \ifdef{\texTunneling}{The exception to this is rule is the tunnel exit which
  //  will still be carried out.}{} \tabTwo{No, modification will happen to
  //  packet.}{Yes, force the packet to be unmodified.}
  uint64_t origCpuPkt:  1; // bit 0
} t_DefaultPacketToCPUModification;

// ------- Struct declaration for register: RouterPortMACAddress --------- 
typedef struct {

  // field: vrf
  //  The VRF to use for this router
  uint64_t vrf                   :  2; // bit 151 to 152

  // field: valid
  //  If set, this entry is valid for comparison.
  uint64_t valid                 :  1; // bit 150

  // field: altMacAddress
  //  The alternative base destination MAC address that is used to identify
  //  packets to the router.
  uint64_t altMacAddress         : 48; // bit 102 to 149

  // field: selectMacEntryPortMask
  //  Portmask to select which MAC address to use. One bit per source port.
  //  \tabTwo{use macAddress.}{use altMacAddress.}
  uint64_t selectMacEntryPortMask:  6; // bit 96 to 101

  // field: macMask
  //  Each bit says if the bit in the DA MAC shall be compared. \tabTwo{Dont
  //  compare bit.}{Compare bit.}
  uint64_t macMask               : 48; // bit 48 to 95

  // field: macAddress
  //  The base destination MAC address that is used to identify packets to the
  //  router.
  uint64_t macAddress            : 48; // bit 0 to 47
} t_RouterPortMACAddress;

// ------- Struct declaration for register: EPPPMDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_EPPPMDrop;

// ------- Struct declaration for register: EgressConfigurableACL0TCAM --------- 
typedef struct {

  // field: compareData
  //  The data which shall be compared in this entry. Observe that this compare
  //  data must be AND:ed by software before the entry is searched. The hardware
  //  does not do the AND between mask and compareData (In order to save area).
  uint8_t compareData[17]; // bit 136 to 270

  // field: mask
  //  Which bits to compare in this entry.
  uint8_t mask[17]; // bit 1 to 135

  // field: valid
  //  Is this entry valid. \tabTwo{No}{Yes}
  uint64_t valid      :  1; // bit 0
} t_EgressConfigurableACL0TCAM;

// ------- Struct declaration for register: IngressConfigurableACL2TCAM --------- 
typedef struct {

  // field: compareData
  //  The data which shall be compared in this entry. Observe that this compare
  //  data must be AND:ed by software before the entry is searched. The hardware
  //  does not do the AND between mask and compareData (In order to save area).
  uint8_t compareData[68]; // bit 541 to 1080

  // field: mask
  //  Which bits to compare in this entry.
  uint8_t mask[68]; // bit 1 to 540

  // field: valid
  //  Is this entry valid. \tabTwo{No}{Yes}
  uint64_t valid      :  1; // bit 0
} t_IngressConfigurableACL2TCAM;

// ------- Struct declaration for register: EgressACLRulePointerTCAM --------- 
typedef struct {

  // field: srcPort
  //  The packets source port.
  uint64_t srcPort         :  3; // bit 62 to 64

  // field: srcPortmask
  //  Mask for srcPort.
  uint64_t srcPortmask     :  3; // bit 59 to 61

  // field: l4Type
  //  The packets L4 Type. abEight{Not known.}{Is IPv4 or IPv6 but type is not any
  //  L4 type in this list.}{UDP}{TCP}{IGMP}{ICMP}{ICMPv6}{MLD}
  uint64_t l4Type          :  3; // bit 56 to 58

  // field: l4Typemask
  //  Mask for l4Type.
  uint64_t l4Typemask      :  3; // bit 53 to 55

  // field: l3Type
  //  The packets L3 Type. abFour{IPv4}{IPv6}{MPLS}{Other}
  uint64_t l3Type          :  2; // bit 51 to 52

  // field: l3Typemask
  //  Mask for l3Type.
  uint64_t l3Typemask      :  2; // bit 49 to 50

  // field: vid
  //  The index used in the VLAN table lookup.
  uint64_t vid             : 12; // bit 37 to 48

  // field: vidmask
  //  Mask for vid.
  uint64_t vidmask         : 12; // bit 25 to 36

  // field: mcSwitched
  //  The packet was L2 switched to a multicast group.
  uint64_t mcSwitched      :  1; // bit 24

  // field: mcSwitchedmask
  //  Mask for mcSwitched.
  uint64_t mcSwitchedmask  :  1; // bit 23

  // field: ucSwitched
  //  The packet was L2 switched to a unicast destination port.
  uint64_t ucSwitched      :  1; // bit 22

  // field: ucSwitchedmask
  //  Mask for ucSwitched.
  uint64_t ucSwitchedmask  :  1; // bit 21

  // field: flooded
  //  The packet was flooded due to L2 table miss.
  uint64_t flooded         :  1; // bit 20

  // field: floodedmask
  //  Mask for flooded.
  uint64_t floodedmask     :  1; // bit 19

  // field: vrf
  //  The VRF used when routed.
  uint64_t vrf             :  2; // bit 17 to 18

  // field: vrfmask
  //  Mask for vrf.
  uint64_t vrfmask         :  2; // bit 15 to 16

  // field: routed
  //  The packet was routed.
  uint64_t routed          :  1; // bit 14

  // field: routedmask
  //  Mask for routed.
  uint64_t routedmask      :  1; // bit 13

  // field: destPortMask
  //  The packets egress ports, one bit per port.
  uint64_t destPortMask    :  6; // bit 7 to 12

  // field: destPortMaskmask
  //  Mask for destPortMask.
  uint64_t destPortMaskmask:  6; // bit 1 to 6

  // field: valid
  //  Is this entry valid. \tabTwo{No}{Yes}
  uint64_t valid           :  1; // bit 0
} t_EgressACLRulePointerTCAM;

// ------- Struct declaration for register: LLDPConfiguration --------- 
typedef struct {

  // field: portmask
  //  One bit per source port, bit 0 for port 0, bit 1 for port 1 etc. \tabTwo{Do
  //  not sent a matched LLDP packet to the CPU from this port. Packet will pass
  //  through normal packet processing.}{Send a matched LLDP packet to CPU from
  //  this source port and hence bypassing normal processing.}
  uint64_t portmask  :  6; // bit 161 to 166

  // field: bpduOption
  //  If both LLDP and BPDU are valid, because the BPDU has same MAC address as
  //  LLDP, then this option allows the BPDU identification to be turned off
  //  \tabTwo{Don't do anything. Both LLDP and BPDU can be valid at same
  //  time.}{Remove BPDU valid causing that the packet will only be seen as a LLDP
  //  packet and not a BPDU frame and the new frame will not be sent to the CPU
  //  because the switch will no longer consider it a BPDU frame, this includes
  //  Rapid Spanning Tree BPUs also.}
  uint64_t bpduOption:  1; // bit 160

  // field: eth
  //  The Ethernet Type for a LLDP
  uint64_t eth       : 16; // bit 144 to 159

  // field: mac3
  //  DA MAC address to match for LLDP packet.
  uint64_t mac3      : 48; // bit 96 to 143

  // field: mac2
  //  DA MAC address to match for LLDP packet.
  uint64_t mac2      : 48; // bit 48 to 95

  // field: mac1
  //  DA MAC address to match for LLDP packet.
  uint64_t mac1      : 48; // bit 0 to 47
} t_LLDPConfiguration;

// ------- Struct declaration for register: IngressSpanningTreeDropListen --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_IngressSpanningTreeDropListen;

// ------- Struct declaration for register: EgressQueueToPCPAndCFIDEIMappingTable --------- 
typedef struct {

  // field: pcp
  //  Map from egress queue to PCP.
  uint64_t pcp   :  3; // bit 1 to 3

  // field: cfiDei
  //  Map from egress queue to CFI/DEI.
  uint64_t cfiDei:  1; // bit 0
} t_EgressQueueToPCPAndCFIDEIMappingTable;

// ------- Struct declaration for register: DebugCounterl2DaHashHitAndBucketSetup --------- 
typedef struct {

  // field: hitValue
  //  Value to compare to update debug counter. Both the incoming value and this
  //  value is ANDed with the mask before comparsion is carried out. If comparsion
  //  results in true the counter is updated
  uint64_t hitValue:  3; // bit 3 to 5

  // field: mask
  //  Mask for comparison to update debug counter.
  uint64_t mask    :  3; // bit 0 to 2
} t_DebugCounterl2DaHashHitAndBucketSetup;

// ------- Struct declaration for register: L2AgingCollisionShadowTable --------- 
typedef struct {

  // field: valid
  //  If this is set, then the corresponding \register{L2 Lookup Collision Table}
  //  entry is valid.
  uint64_t valid:  1; // bit 0
} t_L2AgingCollisionShadowTable;

// ------- Struct declaration for register: L2ActionTableDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_L2ActionTableDrop;

// ------- Struct declaration for register: AgingDataFIFO --------- 
typedef struct {

  // field: valid
  //  \tabTwo{Empty FIFO, entry is not valid}{Valid entry}.
  uint64_t valid               :  1; // bit 73

  // field: updateCam
  //  If set, the corresponding entry has as valid aging request.
  uint64_t updateCam           :  1; // bit 72

  // field: camIndex
  //  Index to the entry in \register{L2 Aging Collision Table}.
  uint64_t camIndex            :  5; // bit 67 to 71

  // field: camStatus
  //  Entry status which shall be written to \register{L2 Aging Collision Table}.
  //  From bit 0 to bit 2 represents valid bit, static bit and hit bit
  //  respectively.
  uint64_t camStatus           :  3; // bit 64 to 66

  // field: reserved3
  //  Reserved.
  uint64_t reserved3           :  4; // bit 60 to 63

  // field: updateHashBucket3
  //  If set, the corresponding bucket has a valid aging request.
  uint64_t updateHashBucket3   :  1; // bit 59

  // field: addressOfHashBucket3
  //  Address into bucket 3 of the \register{L2 Aging Table}.
  uint64_t addressOfHashBucket3: 10; // bit 49 to 58

  // field: statusOfHashBucket3
  //  Entry status which shall be written to \register{L2 Aging Table}. From bit 0
  //  to bit 2 represents valid bit, static bit and hit bit respectively.
  uint64_t statusOfHashBucket3 :  3; // bit 46 to 48

  // field: updateHashBucket2
  //  If set, the corresponding bucket has a valid aging request.
  uint64_t updateHashBucket2   :  1; // bit 45

  // field: addressOfHashBucket2
  //  Address into bucket 2 of the \register{L2 Aging Table}.
  uint64_t addressOfHashBucket2: 10; // bit 35 to 44

  // field: statusOfHashBucket2
  //  Entry status which shall be written to \register{L2 Aging Table}. From bit 0
  //  to bit 2 represents valid bit, static bit and hit bit respectively.
  uint64_t statusOfHashBucket2 :  3; // bit 32 to 34

  // field: reserved1
  //  Reserved.
  uint64_t reserved1           :  4; // bit 28 to 31

  // field: updateHashBucket1
  //  If set, the corresponding bucket has a valid aging request.
  uint64_t updateHashBucket1   :  1; // bit 27

  // field: addressOfHashBucket1
  //  Address into bucket 1 of the \register{L2 Aging Table}.
  uint64_t addressOfHashBucket1: 10; // bit 17 to 26

  // field: statusOfHashBucket1
  //  Entry status which shall be written to \register{L2 Aging Table}. From bit 0
  //  to bit 2 represents valid bit, static bit and hit bit respectively.
  uint64_t statusOfHashBucket1 :  3; // bit 14 to 16

  // field: updateHashBucket0
  //  If set, the corresponding bucket has a valid aging request.
  uint64_t updateHashBucket0   :  1; // bit 13

  // field: addressOfHashBucket0
  //  Address into bucket 0 of the \register{L2 Aging Table}.
  uint64_t addressOfHashBucket0: 10; // bit 3 to 12

  // field: statusOfHashBucket0
  //  Entry status which shall be written to \register{L2 Aging Table}. From bit 0
  //  to bit 2 represents valid bit, static bit and hit bit respectively.
  uint64_t statusOfHashBucket0 :  3; // bit 0 to 2
} t_AgingDataFIFO;

// ------- Struct declaration for register: IPPDebugrouterHit --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value:  1; // bit 0
} t_IPPDebugrouterHit;

// ------- Struct declaration for register: BufferFree --------- 
typedef struct {

  // field: cells
  //  Number of free cells.
  uint64_t cells: 10; // bit 0 to 9
} t_BufferFree;

// ------- Struct declaration for register: L2ActionTableSpecialPacketTypeDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_L2ActionTableSpecialPacketTypeDrop;

// ------- Struct declaration for register: AHHeaderPacketDecoderOptions --------- 
typedef struct {

  // field: toCpu
  //  If a packet comes in on this source port then send the packet to the CPU
  //  port.\tabTwo{Do not sent to CPU. Normal Processing of packet.}{Send to CPU ,
  //  bypass normal packet processing.}
  uint64_t toCpu  :  6; // bit 15 to 20

  // field: drop
  //  If a packet comes in on this source port then drop the packet.\tabTwo{Do not
  //  drop this packet.}{Drop this packet and update the drop counter.}
  uint64_t drop   :  6; // bit 9 to 14

  // field: l4Proto
  //  The value to be used to find this packet type.
  uint64_t l4Proto:  8; // bit 1 to 8

  // field: enabled
  //  Is this decoding enabled. \tabTwo{No}{Yes}
  uint64_t enabled:  1; // bit 0
} t_AHHeaderPacketDecoderOptions;

// ------- Struct declaration for register: DebugCounterl2DaHashSetup --------- 
typedef struct {

  // field: hitValue
  //  Value to compare to update debug counter. Both the incoming value and this
  //  value is ANDed with the mask before comparsion is carried out. If comparsion
  //  results in true the counter is updated
  uint64_t hitValue: 10; // bit 10 to 19

  // field: mask
  //  Mask for comparison to update debug counter.
  uint64_t mask    : 10; // bit 0 to 9
} t_DebugCounterl2DaHashSetup;

// ------- Struct declaration for register: RouterEgressQueueToVLANData --------- 
typedef struct {

  // field: pcp
  //  Map from egress queue to PCP
  uint64_t pcp   :  3; // bit 1 to 3

  // field: cfiDei
  //  Map from egress queue to CFI/DEI
  uint64_t cfiDei:  1; // bit 0
} t_RouterEgressQueueToVLANData;

// ------- Struct declaration for register: IngressSpanningTreeDropLearning --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_IngressSpanningTreeDropLearning;

// ------- Struct declaration for register: EPPDebugreQueuePortId --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value:  3; // bit 0 to 2
} t_EPPDebugreQueuePortId;

// ------- Struct declaration for register: LinkAggregationMembership --------- 
typedef struct {

  // field: la
  //  The Link aggregation which this port is a member of
  uint64_t la:  3; // bit 0 to 2
} t_LinkAggregationMembership;

// ------- Struct declaration for register: PBPacketTailCounter --------- 
typedef struct {

  // field: packets
  //  Number of packet tails.
  uint64_t packets: 32; // bit 0 to 31
} t_PBPacketTailCounter;

// ------- Struct declaration for register: FFAUsednonPFC --------- 
typedef struct {

  // field: cells
  //  Number of cells
  uint64_t cells: 10; // bit 0 to 9
} t_FFAUsednonPFC;

// ------- Struct declaration for register: IngressConfigurableACL3RulesSetup --------- 
typedef struct {

  // field: fieldSelectBitmask
  //  Bitmask of which fields to select. Set a bit to one to select this specific
  //  field, set zero to not select field. At Maximum 10 bits should be set.
  uint64_t fieldSelectBitmask: 10; // bit 0 to 9
} t_IngressConfigurableACL3RulesSetup;

// ------- Struct declaration for register: L2ActionTable --------- 
typedef struct {

  // field: mmpOrder
  //  Ingress MMP pointer order.
  uint64_t mmpOrder             :  2; // bit 17 to 18

  // field: mmpPtr
  //  Ingress MMP pointer.
  uint64_t mmpPtr               :  5; // bit 12 to 16

  // field: mmpValid
  //  If set, this entry contains a valid MMP pointer
  uint64_t mmpValid             :  1; // bit 11

  // field: allowPtr
  //  Pointer to allow special packets defined in \register{Allow Special Frame
  //  Check For L2 Action Table}.
  uint64_t allowPtr             :  2; // bit 9 to 10

  // field: useSpecialAllow
  //  Use the special frame checks on this port. \tabTwo{No.}{Yes.}
  uint64_t useSpecialAllow      :  1; // bit 8

  // field: noPortMove
  //  No port move is allowed for this packet.
  uint64_t noPortMove           :  1; // bit 7

  // field: forceSendToCpuOrigPkt
  //  Force the packet to the CPU to be the originial,unmodified, packet.
  //  \ifdef{\texTunneling}{The exception to this is rule is the tunnel exit which
  //  will still be carried out.}{}
  uint64_t forceSendToCpuOrigPkt:  1; // bit 6

  // field: sendToCpu
  //  The packet shall be send to the CPU.
  uint64_t sendToCpu            :  1; // bit 5

  // field: dropPortMove
  //  The packet shall be dropped if the result from the learning lookup is
  //  port-move.
  uint64_t dropPortMove         :  1; // bit 4

  // field: drop
  //  The packet shall only drop on the ports which hits this action.
  uint64_t drop                 :  1; // bit 3

  // field: dropAll
  //  The packet shall drop all instances and update counter \register{L2 Action
  //  Table Drop}. However special packets which are allowed will still be allowed
  //  into the switch (using the field \field{L2 Action Table}{useSpecialAllow}
  //  set to one and register \register{Allow Special Frame Check For L2 Action
  //  Table})
  uint64_t dropAll              :  1; // bit 2

  // field: noLearningMc
  //  If the packet is a L2 Multicast then the packet shall not be learned. If a
  //  packet is a L2 Multicast depends on if the SA MAC MC bit is set.
  uint64_t noLearningMc         :  1; // bit 1

  // field: noLearningUc
  //  The packet shall not be learned. This is applied to L2 DA MAC unicast
  //  packets.
  uint64_t noLearningUc         :  1; // bit 0
} t_L2ActionTable;

// ------- Struct declaration for register: IPPDebugnextHopPtrHashHit --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value:  1; // bit 0
} t_IPPDebugnextHopPtrHashHit;

// ------- Struct declaration for register: IEEE1588L2PacketDecoderOptions --------- 
typedef struct {

  // field: ptp
  //  If a packet is sent to the CPU and this bit is set and the packet has a
  //  timestamp then it will show having a valid timestamp in the CPU-header.
  uint64_t ptp    :  1; // bit 29

  // field: toCpu
  //  If a packet comes in on this source port then send the packet to the CPU
  //  port.\tabTwo{Do not sent to CPU. Normal Processing of packet.}{Send to CPU ,
  //  bypass normal packet processing.}
  uint64_t toCpu  :  6; // bit 23 to 28

  // field: drop
  //  If a packet comes in on this source port then drop the packet.\tabTwo{Do not
  //  drop this packet.}{Drop this packet and update the drop counter.}
  uint64_t drop   :  6; // bit 17 to 22

  // field: eth
  //  The value to be used to find this packet type.
  uint64_t eth    : 16; // bit 1 to 16

  // field: enabled
  //  Is this decoding enabled. \tabTwo{No}{Yes}
  uint64_t enabled:  1; // bit 0
} t_IEEE1588L2PacketDecoderOptions;

// ------- Struct declaration for register: TunnelExitLookupTCAM --------- 
typedef struct {

  // field: l4Dp
  //  L4 destination port, if packet is a TCP or UDP, otherwise set to zero.
  uint64_t l4Dp          : 16; // bit 621 to 636

  // field: l4Dpmask
  //  Mask for l4Dp.
  uint64_t l4Dpmask      : 16; // bit 605 to 620

  // field: l4Sp
  //  L4 Source port, if packet is a TCP or UDP, otherwise set to zero.
  uint64_t l4Sp          : 16; // bit 589 to 604

  // field: l4Spmask
  //  Mask for l4Sp.
  uint64_t l4Spmask      : 16; // bit 573 to 588

  // field: l4Protocol
  //  The L4 protocol from the IPv4 or IPv6 headers which shall be matched.
  uint64_t l4Protocol    :  8; // bit 565 to 572

  // field: l4Protocolmask
  //  Mask for l4Protocol.
  uint64_t l4Protocolmask:  8; // bit 557 to 564

  // field: l4Type
  //  The L4 type which shall be matched. If not UDP or TCP value 2 will be set in
  //  this register. \tabFour{TCP}{UDP}{Others}{Reserved.}
  uint64_t l4Type        :  2; // bit 555 to 556

  // field: l4Typemask
  //  Mask for l4Type.
  uint64_t l4Typemask    :  2; // bit 553 to 554

  // field: ipDa
  //  The IP Destination or MPLS Address. IPv4 is located in bits [31:0]. First
  //  MPLS bits are located at [19:0], second MPLS label [39:20],third MPLS label
  //  is [59:40] and forth label is at [79:60].
  uint8_t ipDa[16]; // bit 425 to 552

  // field: ipDamask
  //  Mask for ipDa.
  uint8_t ipDamask[16]; // bit 297 to 424

  // field: ipSa
  //  The IP Source Address. IPv4 is located in bits [31:0].
  uint8_t ipSa[16]; // bit 169 to 296

  // field: ipSamask
  //  Mask for ipSa.
  uint8_t ipSamask[16]; // bit 41 to 168

  // field: l3Type
  //  The L3 type which shall be matched. If unknown L3 type then this will set to
  //  7. \tabSix{IPv4}{IPv6}{One MPLS Label}{Two MPLS Labels}{Three MPLS
  //  labels}{Four MPLS labels}
  uint64_t l3Type        :  3; // bit 38 to 40

  // field: l3Typemask
  //  Mask for l3Type.
  uint64_t l3Typemask    :  3; // bit 35 to 37

  // field: ethType
  //  Ethernet Type for the incoming packet.
  uint64_t ethType       : 16; // bit 19 to 34

  // field: ethTypemask
  //  Mask for ethType.
  uint64_t ethTypemask   : 16; // bit 3 to 18

  // field: snapLlc
  //  This is a SNAP and LLC packet.
  uint64_t snapLlc       :  1; // bit 2

  // field: snapLlcmask
  //  Mask for snapLlc.
  uint64_t snapLlcmask   :  1; // bit 1

  // field: valid
  //  Is this entry valid. \tabTwo{No}{Yes}
  uint64_t valid         :  1; // bit 0
} t_TunnelExitLookupTCAM;

// ------- Struct declaration for register: IPPDebugnextHopPtrHash --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value: 10; // bit 0 to 9
} t_IPPDebugnextHopPtrHash;

// ------- Struct declaration for register: IngressConfigurableACLMatchCounter --------- 
typedef struct {

  // field: packets
  //  Number of packets.
  uint64_t packets: 32; // bit 0 to 31
} t_IngressConfigurableACLMatchCounter;

// ------- Struct declaration for register: OutputMirroringTable --------- 
typedef struct {

  // field: outputMirrorPort
  //  Destination of output mirroring. Only valid if outputMirrorEnabled is set.
  //  Notice if the design contains more than one switch slice, packets egressed
  //  on one slice cannot be mirrored to another slice.
  uint64_t outputMirrorPort   :  3; // bit 1 to 3

  // field: outputMirrorEnabled
  //  If set to one, output mirroring is enabled for this port.
  uint64_t outputMirrorEnabled:  1; // bit 0
} t_OutputMirroringTable;

// ------- Struct declaration for register: IEEE1588L4PacketDecoderOptions --------- 
typedef struct {

  // field: ptp
  //  If a packet is sent to the CPU and this bit is set and the packet has a
  //  timestamp then it will show having a valid timestamp in the CPU-header.
  uint64_t ptp        :  1; // bit 685

  // field: toCpu
  //  If a packet comes in on this source port then send the packet to the CPU
  //  port.\tabTwo{Do not sent to CPU. Normal Processing of packet.}{Send to CPU ,
  //  bypass normal packet processing.}
  uint64_t toCpu      :  6; // bit 679 to 684

  // field: drop
  //  If a packet comes in on this source port then drop the packet.\tabTwo{Do not
  //  drop this packet.}{Drop this packet and update the drop counter.}
  uint64_t drop       :  6; // bit 673 to 678

  // field: udp2
  //  UDP destination to match.
  uint64_t udp2       : 16; // bit 657 to 672

  // field: udp1
  //  UDP destination to match.
  uint64_t udp1       : 16; // bit 641 to 656

  // field: daipv6addr2
  //  IPv6 DA to match.
  uint8_t daipv6addr2[16]; // bit 513 to 640

  // field: daipv6mask1
  //  Bit mask for da_ipv6_addr1. For each bit of the mask, 1 means valid for
  //  comparison.
  uint8_t daipv6mask1[16]; // bit 385 to 512

  // field: daipv6addr1
  //  IPv6 DA to match. This address is maskable.
  uint8_t daipv6addr1[16]; // bit 257 to 384

  // field: daipv4addr5
  //  IPv4 DA to match.
  uint64_t daipv4addr5: 32; // bit 225 to 256

  // field: daipv4addr4
  //  IPv4 DA to match.
  uint64_t daipv4addr4: 32; // bit 193 to 224

  // field: daipv4addr3
  //  IPv4 DA to match.
  uint64_t daipv4addr3: 32; // bit 161 to 192

  // field: daipv4addr2
  //  IPv4 DA to match.
  uint64_t daipv4addr2: 32; // bit 129 to 160

  // field: daipv4addr1
  //  IPv4 DA to match.
  uint64_t daipv4addr1: 32; // bit 97 to 128

  // field: damac2
  //  DA MAC to match.
  uint64_t damac2     : 48; // bit 49 to 96

  // field: damac1
  //  DA MAC to match.
  uint64_t damac1     : 48; // bit 1 to 48

  // field: enabled
  //  Is this decoding enabled. \tabTwo{No}{Yes}
  uint64_t enabled    :  1; // bit 0
} t_IEEE1588L4PacketDecoderOptions;

// ------- Struct declaration for register: EPPDebugaddNewMpls --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value:  1; // bit 0
} t_EPPDebugaddNewMpls;

// ------- Struct declaration for register: DebugCounterfinalVidSetup --------- 
typedef struct {

  // field: hitValue
  //  Value to compare to update debug counter. Both the incoming value and this
  //  value is ANDed with the mask before comparsion is carried out. If comparsion
  //  results in true the counter is updated
  uint64_t hitValue: 13; // bit 13 to 25

  // field: mask
  //  Mask for comparison to update debug counter.
  uint64_t mask    : 13; // bit 0 to 12
} t_DebugCounterfinalVidSetup;

// ------- Struct declaration for register: PortUsed --------- 
typedef struct {

  // field: cells
  //  Number of cells
  uint64_t cells: 10; // bit 0 to 9
} t_PortUsed;

// ------- Struct declaration for register: ARPDecoderDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_ARPDecoderDrop;

// ------- Struct declaration for register: SecondTunnelExitMissAction --------- 
typedef struct {

  // field: dropIfMiss
  //  If miss in this table then drop packet \tabTwo{No}{Yes}
  uint64_t dropIfMiss:  1; // bit 0
} t_SecondTunnelExitMissAction;

// ------- Struct declaration for register: IngressConfigurableACL0SmallTable --------- 
typedef struct {

  // field: forceQueuePrio
  //  This is a result field used when this entry is hit. If multiple forceQueue
  //  are set and this prio bit is set then this forceQueue value will be
  //  selected.
  uint64_t forceQueuePrio       :  1; // bit 466

  // field: eQueue
  //  This is a result field used when this entry is hit. The egress queue to be
  //  assigned if the forceQueue field in this entry is set to 1.
  uint64_t eQueue               :  3; // bit 463 to 465

  // field: forceQueue
  //  This is a result field used when this entry is hit. If set, the packet shall
  //  have a forced egress queue. Please see Egress Queue Selection Diagram in
  //  Figure \ref{fig:determineEgressQueue}
  uint64_t forceQueue           :  1; // bit 462

  // field: mmpOrder
  //  This is a result field used when this entry is hit. Ingress MMP pointer
  //  order.
  uint64_t mmpOrder             :  2; // bit 460 to 461

  // field: mmpPtr
  //  This is a result field used when this entry is hit. Ingress MMP pointer.
  uint64_t mmpPtr               :  5; // bit 455 to 459

  // field: mmpValid
  //  This is a result field used when this entry is hit. If set, this entry
  //  contains a valid MMP pointer
  uint64_t mmpValid             :  1; // bit 454

  // field: forceColorPrio
  //  This is a result field used when this entry is hit. If multiple forceColor
  //  are set and this prio bit is set then this forceVid value will be selected.
  uint64_t forceColorPrio       :  1; // bit 453

  // field: color
  //  This is a result field used when this entry is hit. Initial color of the
  //  packet if the forceColor field is set.
  uint64_t color                :  2; // bit 451 to 452

  // field: forceColor
  //  This is a result field used when this entry is hit. If set, the packet shall
  //  have a forced color.
  uint64_t forceColor           :  1; // bit 450

  // field: natOpPrio
  //  This is a result field used when this entry is hit. If multiple natOpValid
  //  are set and this prio bit is set then this natOpPtr value will be selected.
  uint64_t natOpPrio            :  1; // bit 449

  // field: natOpPtr
  //  This is a result field used when this entry is hit. NAT operation pointer.
  uint64_t natOpPtr             : 11; // bit 438 to 448

  // field: natOpValid
  //  This is a result field used when this entry is hit. NAT operation pointer is
  //  valid.
  uint64_t natOpValid           :  1; // bit 437

  // field: newL4Value
  //  This is a result field used when this entry is hit. Update the L4 SP or DP
  //  with this value
  uint64_t newL4Value           : 16; // bit 421 to 436

  // field: updateL4SpOrDp
  //  This is a result field used when this entry is hit. Update the source or
  //  destination L4 port. \tabTwo{Source L4 Port}{Destination L4 Port}
  uint64_t updateL4SpOrDp       :  1; // bit 420

  // field: enableUpdateL4
  //  This is a result field used when this entry is hit. If this entry is hit
  //  then update L4 Source Port or Destination port in ingress packet processing,
  //  this value will be used in the Egress ACL. \tabTwo{Disable}{Enable}
  uint64_t enableUpdateL4       :  1; // bit 419

  // field: newIpValue
  //  This is a result field used when this entry is hit. Update the SA or DA IPv4
  //  address value.
  uint64_t newIpValue           : 32; // bit 387 to 418

  // field: updateSaOrDa
  //  This is a result field used when this entry is hit. Update the SA or DA IPv4
  //  address. The Destiantion IP address updated will be used in the routing
  //  functionality and Egress ACL functionality. If the source IP address is
  //  updated then the updated value will be used in the egress ACL keys.
  //  \tabTwo{Source IP Address}{Destination IP Address}
  uint64_t updateSaOrDa         :  1; // bit 386

  // field: enableUpdateIp
  //  This is a result field used when this entry is hit. If this entry is hit
  //  then update SA or DA IPv4 address in ingress packet processing, this value
  //  will be used by the routing function and egress ACL if this is exists, this
  //  only works for IPv4. \tabTwo{Disable}{Enable}
  uint64_t enableUpdateIp       :  1; // bit 385

  // field: tosMask
  //  This is a result field used when this entry is hit. Mask for TOS value.
  //  Setting a bit to one means this bit will be selected from the newTosExp
  //  field , while setting this bit to zero means that the bit will be selected
  //  from the packets already existing TOS byte bit.
  uint64_t tosMask              :  8; // bit 377 to 384

  // field: newTosExp
  //  This is a result field used when this entry is hit. New TOS/EXP value.
  uint64_t newTosExp            :  8; // bit 369 to 376

  // field: updateTosExp
  //  This is a result field used when this entry is hit. Force TOS/EXP update.
  uint64_t updateTosExp         :  1; // bit 368

  // field: counter
  //  This is a result field used when this entry is hit. Which counter in
  //  \register{Ingress Configurable ACL Match Counter} to update.
  uint64_t counter              :  6; // bit 362 to 367

  // field: updateCounter
  //  This is a result field used when this entry is hit. When set the selected
  //  statistics counter will be updated.
  uint64_t updateCounter        :  1; // bit 361

  // field: imPrio
  //  This is a result field used when this entry is hit. If multiple input mirror
  //  are set and this prio bit is set then this input mirror will be selected.
  uint64_t imPrio               :  1; // bit 360

  // field: destInputMirror
  //  This is a result field used when this entry is hit. Destination
  //  \ifdef{\texLinkAgg}{physical}{} port for input mirroring.
  uint64_t destInputMirror      :  3; // bit 357 to 359

  // field: inputMirror
  //  This is a result field used when this entry is hit. If set, input mirroring
  //  is enabled for this rule. In addition to the normal processing of the packet
  //  a copy of the unmodified input packet will be send to the destination Input
  //  Mirror port and exit on that port. The copy will be subject to the normal
  //  resource limitations in the switch.
  uint64_t inputMirror          :  1; // bit 356

  // field: destPort
  //  This is a result field used when this entry is hit. The port which the
  //  packet shall be sent to.
  uint64_t destPort             :  3; // bit 353 to 355

  // field: sendToPort
  //  This is a result field used when this entry is hit. Send the packet to a
  //  specific port. \tabTwo{Disabled.}{Send to port configured in destPort.}
  uint64_t sendToPort           :  1; // bit 352

  // field: dropEnable
  //  This is a result field used when this entry is hit. If set, the packet shall
  //  be dropped and the \register{Ingress Configurable ACL Drop} counter is
  //  incremented.
  uint64_t dropEnable           :  1; // bit 351

  // field: metaDataPrio
  //  This is a result field used when this entry is hit. If multiple ACLs hit
  //  this meta_data shall take priority.
  uint64_t metaDataPrio         :  1; // bit 350

  // field: metaData
  //  This is a result field used when this entry is hit. Meta data for packets
  //  going to the CPU.
  uint64_t metaData             : 16; // bit 334 to 349

  // field: metaDataValid
  //  This is a result field used when this entry is hit. Is the meta_data field
  //  valid.
  uint64_t metaDataValid        :  1; // bit 333

  // field: forceSendToCpuOrigPkt
  //  This is a result field used when this entry is hit. If packet shall be sent
  //  to CPU then setting this bit will force the packet to be the incoming
  //  originial packet. \ifdef{\texTunneling}{The exception to this is rule is the
  //  tunnel exit which will still be carried out.}{}.
  uint64_t forceSendToCpuOrigPkt:  1; // bit 332

  // field: sendToCpu
  //  This is a result field used when this entry is hit. If set, the packet shall
  //  be sent to the CPU port.
  uint64_t sendToCpu            :  1; // bit 331

  // field: compareData
  //  The data which shall be compared in this entry.
  uint8_t compareData[42]; // bit 1 to 330

  // field: valid
  //  Is this entry valid. \tabTwo{No}{Yes}.
  uint64_t valid                :  1; // bit 0
} t_IngressConfigurableACL0SmallTable;

// ------- Struct declaration for register: PortTCReserved --------- 
typedef struct {

  // field: cells
  //  Number of cells
  uint64_t cells: 10; // bit 0 to 9
} t_PortTCReserved;

// ------- Struct declaration for register: L2QoSMappingTable --------- 
typedef struct {

  // field: pcp
  //  Packets new PCP.
  uint64_t pcp         :  3; // bit 3 to 5

  // field: updatePcp
  //  Update Pcp field in outgoing packet. \tabTwo{Do not update.}{Update.}
  uint64_t updatePcp   :  1; // bit 2

  // field: cfiDei
  //  Packets new CFI/DEI.
  uint64_t cfiDei      :  1; // bit 1

  // field: updateCfiDei
  //  Update CfiDei field in outgoing packet. \tabTwo{Do not update.}{Update.}
  uint64_t updateCfiDei:  1; // bit 0
} t_L2QoSMappingTable;

// ------- Struct declaration for register: L2MulticastHandling --------- 
typedef struct {

  // field: unknownL2McFilterRule
  //  Select the filtering rules for unknown L2 multicast MAC DA in the
  //  \register{Ingress Egress Port Packet Type Filter}.\tabTwo{\field{Ingress
  //  Egress Port Packet Type Filter}{dropL2FloodingFrames}}{\field{Ingress Egress
  //  Port Packet Type Filter}{dropL2MulticastFrames}}
  uint64_t unknownL2McFilterRule:  1; // bit 4

  // field: inclMultiPorts
  //  If set, packets that end up in more than one destination port but not due to
  //  broadcast or flooding will have a L2 multicast flag.
  //  \ifdef{\texMirroring}{Observe that mirroring is not a valid multiport
  //  destination.}{}
  uint64_t inclMultiPorts       :  1; // bit 3

  // field: inclL2McLut
  //  If set, packets that are forwarded by \register{L2 Multicast Table} will
  //  internally be treated as the L2 multicast bit in the L2 DA address would
  //  have been set to one.
  uint64_t inclL2McLut          :  1; // bit 2

  // field: exclIPv6Mc
  //  If set, IPv6 packets with IPv6 multicast MAC address will NOT have a L2
  //  multicast flag.
  uint64_t exclIPv6Mc           :  1; // bit 1

  // field: exclIPv4Mc
  //  If set, IPv4 packets with IPv4 multicast MAC address will NOT have a L2
  //  multicast flag.
  uint64_t exclIPv4Mc           :  1; // bit 0
} t_L2MulticastHandling;

// ------- Struct declaration for register: EgressTunnelExitTable --------- 
typedef struct {

  // field: whereToRemove
  //  Where to do the tunnel exit from \tabFour{At Byte Zero}{After L2 and up to
  //  two VLAN headers.}{After L3 IPv4/IPv6 headers.}{Reserved.}
  uint64_t whereToRemove       :  2; // bit 35 to 36

  // field: l4Protocol
  //  If packet is removed after L3 headers then this new L4 Protocol will be
  //  written.
  uint64_t l4Protocol          :  8; // bit 27 to 34

  // field: updateL4Protocol
  //  If packet is removed after L3 headers then update the L4 Protocol in IP
  //  header.
  uint64_t updateL4Protocol    :  1; // bit 26

  // field: removeVlan
  //  If packet is removed after L2+VLAN headers then remove the VLAN headers on
  //  the incoming packet.
  uint64_t removeVlan          :  1; // bit 25

  // field: ethType
  //  If packet is removed after L2+VLAN headers then the New Ethernet Type which
  //  will overwrite the existing lowest 16 bits after the removal operation.
  uint64_t ethType             : 16; // bit 9 to 24

  // field: updateEthType
  //  If packet is removed after L2+VLAN headers then update the Ethernet Header
  //  Type Field
  uint64_t updateEthType       :  1; // bit 8

  // field: howManyBytesToRemove
  //  How many bytes to remove.
  uint64_t howManyBytesToRemove:  8; // bit 0 to 7
} t_EgressTunnelExitTable;

// ------- Struct declaration for register: MACInterfaceCountersForRX --------- 
typedef struct {

  // field: error
  //  Bus protocol errors.
  uint64_t error  : 32; // bit 32 to 63

  // field: packets
  //  Correct packets completed
  uint64_t packets: 32; // bit 0 to 31
} t_MACInterfaceCountersForRX;

// ------- Struct declaration for register: IngressConfigurableACL1Selection --------- 
typedef struct {

  // field: selectSmallOrLarge
  //  If set to zero then small hash table is selected. If set to one then large
  //  hash table is selected.
  uint64_t selectSmallOrLarge:  1; // bit 1

  // field: selectTcamOrTable
  //  If set to zero then TCAM answer is selected. If set to one then hash table
  //  answer is selected.
  uint64_t selectTcamOrTable :  1; // bit 0
} t_IngressConfigurableACL1Selection;

// ------- Struct declaration for register: NextHopTable --------- 
typedef struct {

  // field: metaData
  //  Meta data for to CPU tag.
  uint64_t metaData        : 16; // bit 31 to 46

  // field: tunnelExitPtr
  //  Pointer to tunnel exit described in \register{Egress Tunnel Exit Table}.
  uint64_t tunnelExitPtr   :  4; // bit 27 to 30

  // field: tunnelExit
  //  Shall this packet do a tunnel exit. \tabTwo{No}{Yes}
  uint64_t tunnelExit      :  1; // bit 26

  // field: tunnelEntryPtr
  //  The tunnel entry which this packet shall enter upon exiting the router. If
  //  field l2Uc is set to L2 multicast then \register{Tunnel Entry Instruction
  //  Table} uses the egress port as a offset from this base pointer.
  uint64_t tunnelEntryPtr  :  4; // bit 22 to 25

  // field: tunnelEntry
  //  Shall this packet enter into a tunnel.
  uint64_t tunnelEntry     :  1; // bit 21

  // field: sendToCpu
  //  If set then the packet will be sent to the CPU.
  uint64_t sendToCpu       :  1; // bit 20

  // field: pktDrop
  //  If set then the packet will be dropped and the \register{L3 Lookup Drop}
  //  incremented.
  uint64_t pktDrop         :  1; // bit 19

  // field: destPortormcAddr
  //  Destination port number or a pointer into the \register{L2 Multicast Table}
  uint64_t destPortormcAddr:  6; // bit 13 to 18

  // field: l2Uc
  //  L2 unicast or multicast. A multicast means that a lookup in the \register{L2
  //  Multicast Table} will take place to determine the destination portmask.
  //  \tabTwo{L2 multicast.} {L2 unicast.}
  uint64_t l2Uc            :  1; // bit 12

  // field: nextHopPacketMod
  //  Pointer into the \register{Next Hop Packet Modifications} table and the
  //  \register{Next Hop DA MAC} table.
  uint64_t nextHopPacketMod: 10; // bit 2 to 11

  // field: srv6Sid
  //  If set, this entry is a locally instantiated SRv6 segment identifier
  uint64_t srv6Sid         :  1; // bit 1

  // field: validEntry
  //  Is this a valid entry or not. If the entry is not valid then the packet
  //  shall be sent to the CPU for further processsing
  uint64_t validEntry      :  1; // bit 0
} t_NextHopTable;

// ------- Struct declaration for register: EgressACLRulePointerTCAMAnswer --------- 
typedef struct {

  // field: rulePtr1
  //  Rule Pointer for egress ACL engine 1.
  uint64_t rulePtr1:  2; // bit 3 to 4

  // field: rulePtr0
  //  Rule Pointer for egress ACL engine 0.
  uint64_t rulePtr0:  3; // bit 0 to 2
} t_EgressACLRulePointerTCAMAnswer;

// ------- Struct declaration for register: SourcePortDefaultACLAction --------- 
typedef struct {

  // field: natOpPtr
  //  NAT operation pointer.
  uint64_t natOpPtr             : 11; // bit 163 to 173

  // field: natOpValid
  //  NAT operation pointer is valid.
  uint64_t natOpValid           :  1; // bit 162

  // field: eQueue
  //  The egress queue to be assigned if the forceQueue field in this entry is set
  //  to 1.
  uint64_t eQueue               :  3; // bit 159 to 161

  // field: forceQueue
  //  If set, the packet shall have a forced egress queue. Please see Egress Queue
  //  Selection Diagram in Figure \ref{fig:determineEgressQueue}
  uint64_t forceQueue           :  1; // bit 158

  // field: mmpOrder
  //  Ingress MMP pointer order.
  uint64_t mmpOrder             :  2; // bit 156 to 157

  // field: mmpPtr
  //  Ingress MMP pointer.
  uint64_t mmpPtr               :  5; // bit 151 to 155

  // field: mmpValid
  //  If set, this entry contains a valid MMP pointer
  uint64_t mmpValid             :  1; // bit 150

  // field: color
  //  Initial color of the packet if the forceColor field is set.
  uint64_t color                :  2; // bit 148 to 149

  // field: forceColor
  //  If set, the packet shall have a forced color.
  uint64_t forceColor           :  1; // bit 147

  // field: tunnelEntryPtr
  //  The tunnel entry which this packet shall enter upon exiting the switch.
  uint64_t tunnelEntryPtr       :  4; // bit 143 to 146

  // field: tunnelEntryUcMc
  //  Shall this entry point to the \register{Tunnel Entry Instruction Table} with
  //  or without a egress port offset. \tabTwo{Unicast \register{Tunnel Entry
  //  Instruction Table} without offset for each port}{Multicast \register{Tunnel
  //  Entry Instruction Table} with offset for each port.}
  uint64_t tunnelEntryUcMc      :  1; // bit 142

  // field: tunnelEntry
  //  Shall all of these packets enter into a tunnel.
  uint64_t tunnelEntry          :  1; // bit 141

  // field: ptp
  //  When the packet is sent to the CPU the packet will have the PTP bit in the
  //  To CPU Tag set to one. The timestamp in the To CPU Tag will also be set to
  //  the timestamp from the incoming packet.
  uint64_t ptp                  :  1; // bit 140

  // field: destPort
  //  The port which the packet shall be sent to.
  uint64_t destPort             :  3; // bit 137 to 139

  // field: sendToPort
  //  Send the packet to a specific port. \tabTwo{Disabled.}{Send to port
  //  configured in destPort.}
  uint64_t sendToPort           :  1; // bit 136

  // field: forceSendToCpuOrigPkt
  //  If packet shall be sent to CPU then setting this bit will force the packet
  //  to be the incoming originial packet. \ifdef{\texTunneling}{The exception to
  //  this is rule is the tunnel exit which will still be carried out.}{}.
  uint64_t forceSendToCpuOrigPkt:  1; // bit 135

  // field: sendToCpu
  //  If set, the packet shall be sent to the CPU port.
  uint64_t sendToCpu            :  1; // bit 134

  // field: dropEnable
  //  If set, the packet shall be dropped and the \register{Ingress Configurable
  //  ACL Drop} counter is incremented.
  uint64_t dropEnable           :  1; // bit 133

  // field: newL4Value
  //  Update the L4 SP or DP with this value
  uint64_t newL4Value           : 16; // bit 117 to 132

  // field: updateL4SpOrDp
  //  Update the source or destination L4 port. \tabTwo{Source L4
  //  Port}{Destination L4 Port}
  uint64_t updateL4SpOrDp       :  1; // bit 116

  // field: enableUpdateL4
  //  If this entry is hit then update L4 Source Port or Destination port in
  //  ingress packet processing, this value will be used in the Egress ACL.
  //  \tabTwo{Disable}{Enable}
  uint64_t enableUpdateL4       :  1; // bit 115

  // field: newIpValue
  //  Update the SA or DA IPv4 address value.
  uint64_t newIpValue           : 32; // bit 83 to 114

  // field: updateSaOrDa
  //  Update the SA or DA IPv4 address. The Destiantion IP address updated will be
  //  used in the routing functionality and Egress ACL functionality. If the
  //  source IP address is updated then the updated value will be used in the
  //  egress ACL keys. \tabTwo{Source IP Address}{Destination IP Address}
  uint64_t updateSaOrDa         :  1; // bit 82

  // field: enableUpdateIp
  //  If this entry is hit then update SA or DA IPv4 address in ingress packet
  //  processing, this value will be used by the routing function and egress ACL
  //  if this is exists, this only works for IPv4. \tabTwo{Disable}{Enable}
  uint64_t enableUpdateIp       :  1; // bit 81

  // field: newEthType
  //  Select which TPID to use in the outer VLAN header. \tabThree{C-VLAN -
  //  0x8100.} {S-VLAN - 0x88A8.} {User defined VLAN type from register
  //  \register{Egress Ethernet Type for VLAN tag}.}
  uint64_t newEthType           :  2; // bit 79 to 80

  // field: updateEType
  //  The VLANs TPID type should be updated. \tabTwo{Do not update the
  //  TPID.}{Update the TPID.}
  uint64_t updateEType          :  1; // bit 78

  // field: newVidValue
  //  The VID value to update to.
  uint64_t newVidValue          : 12; // bit 66 to 77

  // field: updateVid
  //  The VID value of the packets outermost VLAN should be updated. \tabTwo{Do
  //  not update the value.}{Update the value.}
  uint64_t updateVid            :  1; // bit 65

  // field: newPcpValue
  //  The PCP value to update to.
  uint64_t newPcpValue          :  3; // bit 62 to 64

  // field: updatePcp
  //  The PCP value of the packets outermost VLAN should be updated. \tabTwo{Do
  //  not update the value.}{Update the value.}
  uint64_t updatePcp            :  1; // bit 61

  // field: newCfiDeiValue
  //  The value to update to.
  uint64_t newCfiDeiValue       :  1; // bit 60

  // field: updateCfiDei
  //  The CFI/DEI value of the packets outermost VLAN should be updated.\tabTwo{Do
  //  not update the value.}{Update the value.}
  uint64_t updateCfiDei         :  1; // bit 59

  // field: forceVid
  //  The new Ingress VID.
  uint64_t forceVid             : 12; // bit 47 to 58

  // field: forceVidValid
  //  Override the Ingress VID, see chapter \hyperref[chap:VLAN Processing]{VLAN
  //  Processing}.
  uint64_t forceVidValid        :  1; // bit 46

  // field: tosMask
  //  Mask for TOS value. Setting a bit to one means this bit will be selected
  //  from the newTosExp field , while setting this bit to zero means that the bit
  //  will be selected from the packets already existing TOS byte bit.
  uint64_t tosMask              :  8; // bit 38 to 45

  // field: newTosExp
  //  New TOS/EXP value.
  uint64_t newTosExp            :  8; // bit 30 to 37

  // field: updateTosExp
  //  Force TOS/EXP update.
  uint64_t updateTosExp         :  1; // bit 29

  // field: counter
  //  Which counter in \register{Ingress Configurable ACL Match Counter} to
  //  update.
  uint64_t counter              :  6; // bit 23 to 28

  // field: updateCounter
  //  When set the selected statistics counter will be updated.
  uint64_t updateCounter        :  1; // bit 22

  // field: noLearning
  //  If set this packets MAC SA will not be learned.
  uint64_t noLearning           :  1; // bit 21

  // field: destInputMirror
  //  Destination \ifdef{\texLinkAgg}{physical}{} port for input mirroring.
  uint64_t destInputMirror      :  3; // bit 18 to 20

  // field: inputMirror
  //  If set, input mirroring is enabled for this rule. In addition to the normal
  //  processing of the packet a copy of the unmodified input packet will be send
  //  to the destination Input Mirror port and exit on that port. The copy will be
  //  subject to the normal resource limitations in the switch.
  uint64_t inputMirror          :  1; // bit 17

  // field: metaData
  //  Meta data for packets going to the CPU.
  uint64_t metaData             : 16; // bit 1 to 16

  // field: metaDataValid
  //  Is the meta_data field valid.
  uint64_t metaDataValid        :  1; // bit 0
} t_SourcePortDefaultACLAction;

// ------- Struct declaration for register: VLANTable --------- 
typedef struct {

  // field: sendIpMcToCpu
  //  Send all IPv4 and IPv6 multicast packets to CPU, bypassing L2 processing and
  //  L3 routing.
  uint64_t sendIpMcToCpu        :  1; // bit 97

  // field: allowRouting
  //  Allow routing. \tabTwo{The router will not process the packet but L2
  //  processing will be done normally.} {Packet will be processed by the router.}
  uint64_t allowRouting         :  1; // bit 96

  // field: cfiDei
  //  The CFI/DEI used in VLAN push or swap operation if selected by \field{VLAN
  //  Table}{cfiDeiSel}
  uint64_t cfiDei               :  1; // bit 95

  // field: pcp
  //  The PCP used in VLAN push or swap operation if selected by \field{VLAN
  //  Table}{pcpSel}.
  uint64_t pcp                  :  3; // bit 92 to 94

  // field: vid
  //  The VID used in VLAN push or swap operation if selected by \field{VLAN
  //  Table}{vidSel}.
  uint64_t vid                  : 12; // bit 80 to 91

  // field: typeSel
  //  Selects which TPID to use when building a new VLAN header in a push or swap
  //  operation. \tabThree{C-VLAN - 0x8100.} {S-VLAN - 0x88A8.} {User defined VLAN
  //  type from register \register{Egress Ethernet Type for VLAN tag} field
  //  \field{Egress Ethernet Type for VLAN tag}{typeValue}.}
  uint64_t typeSel              :  2; // bit 78 to 79

  // field: cfiDeiIf
  //  If this data is used depends on if the \field{VLAN
  //  Table}{nrVlansVidOperationIf} is done on this port. Then the default
  //  operation is overriden with this value. The CFI/DEI used in VLAN push or
  //  swap operation if selected by \field{VLAN Table}{cfiDeiSel}
  uint64_t cfiDeiIf             :  1; // bit 77

  // field: pcpIf
  //  If this data is used depends on if the \field{VLAN
  //  Table}{nrVlansVidOperationIf} is done on this port. Then the default
  //  operation is overriden with this value. The PCP used in VLAN push or swap
  //  operation if selected by \field{VLAN Table}{pcpSel}.
  uint64_t pcpIf                :  3; // bit 74 to 76

  // field: vidIf
  //  If this data is used depends on if the \field{VLAN
  //  Table}{nrVlansVidOperationIf} is done on this port. Then the default
  //  operation is overriden with this value. The VID used in VLAN push or swap
  //  operation if selected by \field{VLAN Table}{vidSel}.
  uint64_t vidIf                : 12; // bit 62 to 73

  // field: typeSelIf
  //  This operation depends on if the \field{VLAN Table}{nrVlansVidOperationIf}
  //  is done on this port. Then the default operation is overriden with this
  //  value. Selects which TPID to use when building a new VLAN header in a push
  //  or swap operation. \tabThree{C-VLAN - 0x8100.} {S-VLAN - 0x88A8.} {User
  //  defined VLAN type from register \register{Egress Ethernet Type for VLAN tag}
  //  field \field{Egress Ethernet Type for VLAN tag}{typeValue}.}
  uint64_t typeSelIf            :  2; // bit 60 to 61

  // field: pcpSelIf
  //  This operation depends on if the \field{VLAN Table}{nrVlansVidOperationIf}
  //  is done on this port. Then the default operation is overriden with this
  //  value. Selects which PCP to use when building a new VLAN header in a push or
  //  swap operation. If the selected VLAN header doesn't exist in the packet then
  //  this table entry's \field{VLAN Table}{pcp} will be used. \tabThree{From
  //  outermost VLAN in the original packet. (if any)} {From this table entry's
  //  \field{VLAN Table}{pcp}.} {From the second VLAN in the original packet (if
  //  any).}
  uint64_t pcpSelIf             :  2; // bit 58 to 59

  // field: cfiDeiSelIf
  //  This operation depends on if the \field{VLAN Table}{nrVlansVidOperationIf}
  //  is done on this port. Then the default operation is overriden with this
  //  value. Selects which CFI/DEI to use when building a new VLAN header in a
  //  push or swap operation. If the selected VLAN header doesn't exist in the
  //  packet then this table entry's \field{VLAN Table}{cfiDei} will be used.
  //  \tabThree{From outermost VLAN in the original packet (if any).} {From this
  //  table entry's \field{VLAN Table}{cfiDei}.} {From the second VLAN in the
  //  original packet (if any).}
  uint64_t cfiDeiSelIf          :  2; // bit 56 to 57

  // field: vidSelIf
  //  This operation depends on if the \field{VLAN Table}{nrVlansVidOperationIf}
  //  is done on this port. Then the default operation is overriden with this
  //  value. Selects which VID to use when building a new VLAN header in a push or
  //  swap operation. If the selected VLAN header doesn't exist in the packet then
  //  this table entry's \field{VLAN Table}{vid} will be used. \tabThree{From the
  //  outermost VLAN in the original packet (if any).} {From this table entry's
  //  \field{VLAN Table}{vid}.} {From the second VLAN in the original packet (if
  //  any).}
  uint64_t vidSelIf             :  2; // bit 54 to 55

  // field: vlanSingleOpIf
  //  This operation depends on if the \field{VLAN Table}{nrVlansVidOperationIf}
  //  is done on this port. Then the default operation is overriden with this
  //  value. The ingress VLAN operation to perform on the packet. \tabFive{No
  //  operation.} {Swap.} {Push.} {Pop.} {Penultimate Pop(remove all VLANS).}
  uint64_t vlanSingleOpIf       :  3; // bit 51 to 53

  // field: nrVlansVidOperationIf
  //  A per source port setting. Port 0 uses bits [1:0], port 2 uses bits [3:2]
  //  and so on. If the packet coming in on the source port has this amount of
  //  VLANs then this operation will override the VLAN Tables VID operation and
  //  all associated data. This operation does take into account what operation
  //  the source port VID operation performed on the packet. If a already has 2
  //  VLANs and a push operation is done it will still be counted as a packet with
  //  two vlans. If a packet has zero vlans and a pop operation is carried out it
  //  will still have zero VLANs. Swap operations does not change the number of
  //  VLANs on the packet. \tabFour{Incoming packet after source port VID op has
  //  zero VLANs} {Incoming packet after source port VID op has one VLAN}
  //  {Incoming packet after source port VID op has Two VLANs} {Reserved and
  //  Disabled}
  uint64_t nrVlansVidOperationIf: 12; // bit 39 to 50

  // field: pcpSel
  //  Selects which PCP to use when building a new VLAN header in a push or swap
  //  operation. If the selected VLAN header doesn't exist in the packet then this
  //  table entry's \field{VLAN Table}{pcp} will be used. \tabThree{From outermost
  //  VLAN in the original packet. (if any)} {From this table entry's \field{VLAN
  //  Table}{pcp}.} {From the second VLAN in the original packet (if any).}
  uint64_t pcpSel               :  2; // bit 37 to 38

  // field: cfiDeiSel
  //  Selects which CFI/DEI to use when building a new VLAN header in a push or
  //  swap operation. If the selected VLAN header doesn't exist in the packet then
  //  this table entry's \field{VLAN Table}{cfiDei} will be used. \tabThree{From
  //  outermost VLAN in the original packet (if any).} {From this table entry's
  //  \field{VLAN Table}{cfiDei}.} {From the second VLAN in the original packet
  //  (if any).}
  uint64_t cfiDeiSel            :  2; // bit 35 to 36

  // field: vidSel
  //  Selects which VID to use when building a new VLAN header in a push or swap
  //  operation. If the selected VLAN header doesn't exist in the packet then this
  //  table entry's \field{VLAN Table}{vid} will be used. \tabThree{From the
  //  outermost VLAN in the original packet (if any).} {From this table entry's
  //  \field{VLAN Table}{vid}.} {From the second VLAN in the original packet (if
  //  any).}
  uint64_t vidSel               :  2; // bit 33 to 34

  // field: vlanSingleOp
  //  The ingress VLAN operation to perform on the packet. \tabFive{No operation.}
  //  {Swap.} {Push.} {Pop.} {Penultimate Pop(remove all VLANS).}
  uint64_t vlanSingleOp         :  3; // bit 30 to 32

  // field: msptPtr
  //  The multiple spanning tree to be used by packets on this VLAN. Points to
  //  entries in the \register{Ingress Multiple Spanning Tree State} and
  //  \register{Egress Multiple Spanning Tree State} tables
  uint64_t msptPtr              :  4; // bit 26 to 29

  // field: mmpOrder
  //  Ingress MMP pointer order.
  uint64_t mmpOrder             :  2; // bit 24 to 25

  // field: mmpPtr
  //  Ingress MMP pointer.
  uint64_t mmpPtr               :  5; // bit 19 to 23

  // field: mmpValid
  //  If set, this entry contains a valid MMP pointer
  uint64_t mmpValid             :  1; // bit 18

  // field: gid
  //  The packet will be assigned a global identifier that is used during L2
  //  lookup to allow multiple VLANs to share the same L2 tables.
  uint64_t gid                  : 12; // bit 6 to 17

  // field: vlanPortMask
  //  VLAN membership portmask. The packets source port must be a member of the
  //  VLAN, otherwise the packet will be dropped and the \register{VLAN Member
  //  Drop} will be incremented. The membership mask will also limit the
  //  destination ports for L2 unicast, multicast, broadcast and flooding. If this
  //  results in an empty destination port mask then the packet is dropped and the
  //  \register{Empty Mask Drop} will be incremented.
  uint64_t vlanPortMask         :  6; // bit 0 to 5
} t_VLANTable;

// ------- Struct declaration for register: IngressPortPacketTypeFilter --------- 
typedef struct {

  // field: dropSStaggedVlans
  //  Drop or allow packets which has a S-VLAN followed by a S-VLAN tagged on this
  //  source port. \tabTwo{Allow packets which has a S-VLAN tag followed by a
  //  S-VLAN tag.} {Drop packets which has a S-VLAN tag followed by a S-VLAN tag.}
  uint64_t dropSStaggedVlans       :  1; // bit 16

  // field: dropCCtaggedVlans
  //  Drop or allow packets which has a C-VLAN followed by a C-VLAN tagged on this
  //  ingress port. \tabTwo{Allow packets which has a C-VLANs tag followed by a
  //  C-VLAN tag.} {Drop packets which has a C-VLAN tag followed by a C-VLAN tag.}
  uint64_t dropCCtaggedVlans       :  1; // bit 15

  // field: dropSCtaggedVlans
  //  Drop or allow packets which has a S-VLAN followed by a C-VLAN tagged on this
  //  ingress port. \tabTwo{Allow packets which has a S-VLAN followed by a C-VLAN
  //  tag.} {Drop packets which has a S-VLAN tag followed by a C-VLAN tag.}
  uint64_t dropSCtaggedVlans       :  1; // bit 14

  // field: dropCStaggedVlans
  //  Drop or allow packets which has a C-VLAN followed by a S-VLAN tagged on this
  //  ingress port. \tabTwo{Allow packets which has a C-VLAN tag followed by a
  //  S-VLAN tag.} {Drop packets which has a C-VLAN tag followed by a S-VLAN tag.}
  uint64_t dropCStaggedVlans       :  1; // bit 13

  // field: dropDualTaggedVlans
  //  Drop or allow packets which has more than one VLAN tag on this ingress port.
  //  \tabTwo{Allow packets which has dual tags.}{Drop packets which has dual
  //  tags.}
  uint64_t dropDualTaggedVlans     :  1; // bit 12

  // field: dropL2MulticastFrames
  //  Drop or allow L2 multicast packets on this ingress port. Observe that this
  //  L2 multicast bit takes the register \register{L2 Multicast Handling} into
  //  account to determine if this packet is a L2 multicast packet or not.
  //  \tabTwo{Allow L2 multicast packets}{Drop L2 multicast packets.}
  uint64_t dropL2MulticastFrames   :  1; // bit 11

  // field: dropL2BroadcastFrames
  //  Drop or allow L2 broadcast packets on this ingress port. \tabTwo{Drop L2
  //  broadcast packets.}{Allow L2 broadcast packets.}
  uint64_t dropL2BroadcastFrames   :  1; // bit 10

  // field: dropIPv6MulticastPackets
  //  Drop or allow IPv6 multicast packets on this ingress port. \tabTwo{Allow
  //  IPv6 MC packets.}{Drop IPv6 MC packets.}
  uint64_t dropIPv6MulticastPackets:  1; // bit 9

  // field: dropIPv4MulticastPackets
  //  Drop or allow IPv4 multicast packets on this ingress port. \tabTwo{Allow
  //  IPv4 MC packets.}{Drop IPv4 MC packets.}
  uint64_t dropIPv4MulticastPackets:  1; // bit 8

  // field: dropMPLSPackets
  //  Drop or allow MPLS packets on this ingress port. \tabTwo{Allow MPLS
  //  packets.}{Drop MPLS packets.}
  uint64_t dropMPLSPackets         :  1; // bit 7

  // field: dropIPv6Packets
  //  Drop or allow IPv6 packets on this ingress port. \tabTwo{Allow IPv6
  //  packets.}{Drop IPv6 packets.}
  uint64_t dropIPv6Packets         :  1; // bit 6

  // field: dropIPv4Packets
  //  Drop or allow IPv4 packets on this ingress port. \tabTwo{Allow IPv4
  //  packets.}{Drop IPv4 packets.}
  uint64_t dropIPv4Packets         :  1; // bit 5

  // field: dropSingleTaggedVlans
  //  Drop or Allow packets that are VLAN untagged on this ingress port.
  //  \tabTwo{Allow untagged packets.}{Drop untagged packets.}
  uint64_t dropSingleTaggedVlans   :  1; // bit 4

  // field: dropUntaggedVlans
  //  Drop or Allow packets that are VLAN untagged on this ingress port.
  //  \tabTwo{Allow untagged packets.}{Drop untagged packets.}
  uint64_t dropUntaggedVlans       :  1; // bit 3

  // field: moreThanOneVlans
  //  When filtering with dropCtaggedVlans or dropStaggedVlans then this field
  //  must be set to 1.
  uint64_t moreThanOneVlans        :  1; // bit 2

  // field: dropStaggedVlans
  //  Drop or allow service VLANs tagged packets on this ingress port. Will only
  //  drop packets that has exactly one VLAN tag. Must set moreThanOneVlans when
  //  this is used. \tabTwo{Allow S-VLANs.}{Drop S-VLANs.}
  uint64_t dropStaggedVlans        :  1; // bit 1

  // field: dropCtaggedVlans
  //  Drop or allow customer VLAN tagged packet on this ingress port. Will only
  //  drop packets that has exactly one VLAN tag. Must set moreThanOneVlans when
  //  this is used. \tabTwo{Allow C-VLANs.}{Drop C-VLANs.}
  uint64_t dropCtaggedVlans        :  1; // bit 0
} t_IngressPortPacketTypeFilter;

// ------- Struct declaration for register: DrainPort --------- 
typedef struct {

  // field: drainMask
  //  Egress ports to be drained. One bit for each port in the current switch
  //  slice where bit 0 corresponds to local port 0.
  uint64_t drainMask:  6; // bit 0 to 5
} t_DrainPort;

// ------- Struct declaration for register: L3LookupDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_L3LookupDrop;

// ------- Struct declaration for register: TunnelExitTooSmalltoDecodeDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_TunnelExitTooSmalltoDecodeDrop;

// ------- Struct declaration for register: EPPDebugomImActive --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value:  1; // bit 0
} t_EPPDebugomImActive;

// ------- Struct declaration for register: PortFFAUsed --------- 
typedef struct {

  // field: cells
  //  Number of cells
  uint64_t cells: 10; // bit 0 to 9
} t_PortFFAUsed;

// ------- Struct declaration for register: LACPPacketDecoderOptions --------- 
typedef struct {

  // field: toCpu
  //  If a packet comes in on this source port then send the packet to the CPU
  //  port.\tabTwo{Do not sent to CPU. Normal Processing of packet.}{Send to CPU ,
  //  bypass normal packet processing.}
  uint64_t toCpu  :  6; // bit 55 to 60

  // field: drop
  //  If a packet comes in on this source port then drop the packet.\tabTwo{Do not
  //  drop this packet.}{Drop this packet and update the drop counter.}
  uint64_t drop   :  6; // bit 49 to 54

  // field: mac
  //  The value to be used to find this packet type.
  uint64_t mac    : 48; // bit 1 to 48

  // field: enabled
  //  Is this decoding enabled. \tabTwo{No}{Yes}
  uint64_t enabled:  1; // bit 0
} t_LACPPacketDecoderOptions;

// ------- Struct declaration for register: DefaultLearningStatus --------- 
typedef struct {

  // field: learnLimit
  //  Maximum number of entries can be learned on this port. 0 means no limit.
  uint64_t learnLimit: 13; // bit 3 to 15

  // field: hit
  //  For a new packet which is to be learned what value shall the hit bit have?
  uint64_t hit       :  1; // bit 2

  // field: stat
  //  For a new packet which is to be learned what value shall the static bit
  //  have?
  uint64_t stat      :  1; // bit 1

  // field: valid
  //  For a new packet which is to be learned what value shall the valid bit have?
  uint64_t valid     :  1; // bit 0
} t_DefaultLearningStatus;

// ------- Struct declaration for register: LearningConflict --------- 
typedef struct {

  // field: port
  //  Port number.
  uint64_t port   :  3; // bit 61 to 63

  // field: gid
  //  Global identifier from the VLAN Table.
  uint64_t gid    : 12; // bit 49 to 60

  // field: macAddr
  //  MAC address.
  uint64_t macAddr: 48; // bit 1 to 48

  // field: valid
  //  Indicates hardware has written a learning conflict to this status register.
  //  Write 0 to clear.
  uint64_t valid  :  1; // bit 0
} t_LearningConflict;

// ------- Struct declaration for register: IngressVIDEthernetTypeRangeAssignmentAnswer --------- 
typedef struct {

  // field: order
  //  Order for this assignment. If the ingress VID can be assigned from other
  //  packet field ranges, the one with the highest order wins.
  uint64_t order     :  2; // bit 12 to 13

  // field: ingressVid
  //  Ingress VID.
  uint64_t ingressVid: 12; // bit 0 to 11
} t_IngressVIDEthernetTypeRangeAssignmentAnswer;

// ------- Struct declaration for register: L2MulticastStormControlBucketThresholdConfiguration --------- 
typedef struct {

  // field: threshold
  //  Minimum number of tokens in bucket for the status to be set to accept.
  uint64_t threshold: 16; // bit 0 to 15
} t_L2MulticastStormControlBucketThresholdConfiguration;

// ------- Struct declaration for register: IngressConfigurableACL3TCAM --------- 
typedef struct {

  // field: compareData
  //  The data which shall be compared in this entry. Observe that this compare
  //  data must be AND:ed by software before the entry is searched. The hardware
  //  does not do the AND between mask and compareData (In order to save area).
  uint8_t compareData[10]; // bit 81 to 160

  // field: mask
  //  Which bits to compare in this entry.
  uint8_t mask[10]; // bit 1 to 80

  // field: valid
  //  Is this entry valid. \tabTwo{No}{Yes}
  uint64_t valid      :  1; // bit 0
} t_IngressConfigurableACL3TCAM;

// ------- Struct declaration for register: L2BroadcastStormControlBucketCapacityConfiguration --------- 
typedef struct {

  // field: bucketCapacity
  //  Capacity of the token bucket
  uint64_t bucketCapacity: 16; // bit 0 to 15
} t_L2BroadcastStormControlBucketCapacityConfiguration;

// ------- Struct declaration for register: NATActionTable --------- 
typedef struct {

  // field: action
  //  What to do with the packet depending on what port states are.\tabFour{No
  //  Operation}{Send to CPU Reason NAT Action Table Code 1}{Send to CPU Reason
  //  NAT Action Table Code 2}{Drop the packet. Update counter \register{NAT
  //  Action Table Drop}.}
  uint64_t action:  2; // bit 0 to 1
} t_NATActionTable;

// ------- Struct declaration for register: IngressConfigurableACL1RulesSetup --------- 
typedef struct {

  // field: fieldSelectBitmask
  //  Bitmask of which fields to select. Set a bit to one to select this specific
  //  field, set zero to not select field. At Maximum 7 bits should be set.
  uint64_t fieldSelectBitmask: 33; // bit 0 to 32
} t_IngressConfigurableACL1RulesSetup;

// ------- Struct declaration for register: VLANPCPAndDEIToColorMappingTable --------- 
typedef struct {

  // field: color
  //  Packet initial color.
  uint64_t color:  2; // bit 0 to 1
} t_VLANPCPAndDEIToColorMappingTable;

// ------- Struct declaration for register: CAPWAPDecoderDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_CAPWAPDecoderDrop;

// ------- Struct declaration for register: SMONByteCounter --------- 
typedef struct {

  // field: bytes
  //  Number of bytes.
  uint64_t bytes: 32; // bit 0 to 31
} t_SMONByteCounter;

// ------- Struct declaration for register: IngressEgressPortPacketTypeFilter --------- 
typedef struct {

  // field: srcPortFilter
  //  Each egress port has an optional way of ensuring that a specific source port
  //  does not send out a packet on a specific egress port. By setting a bit in
  //  this port mask, the packets originating from that source port will be
  //  dropped and not be allowed to reach this egress port.
  uint64_t srcPortFilter           :  6; // bit 19 to 24

  // field: dropRouted
  //  Drop or allow packets which has been routed on this egress port.
  //  \tabTwo{Allow packets which has been routed.} {Drop packets which has been
  //  routed.}
  uint64_t dropRouted              :  1; // bit 18

  // field: dropSStaggedVlans
  //  Drop or allow packets with has a S-VLAN followed by a S-VLAN tagged on this
  //  egress port. Note that after a VLAN push operation the pushed VLAN will be
  //  regarded as a C-VLAN. \tabTwo{Allow packets which has a S-VLAN tag followed
  //  by a S-VLAN tag.} {Drop packets which has a S-VLAN tag followed by a S-VLAN
  //  tag.}
  uint64_t dropSStaggedVlans       :  1; // bit 17

  // field: dropCCtaggedVlans
  //  Drop or allow packets with has a C-VLAN followed by a C-VLAN tagged on this
  //  egress port. Note that after a VLAN push operation the pushed VLAN will be
  //  regarded as a C-VLAN. \tabTwo{Allow packets which has a C-VLAN tag followed
  //  by a C-VLAN tag.} {Drop packets which has a C-VLAN tag followed by a C-VLAN
  //  tag.}
  uint64_t dropCCtaggedVlans       :  1; // bit 16

  // field: dropSCtaggedVlans
  //  Drop or allow packets with has a S-VLAN followed by a C-VLAN tagged on this
  //  egress port. Note that after a VLAN push operation the pushed VLAN will be
  //  regarded as a C-VLAN. \tabTwo{Allow packets which has a S-VLAN followed by a
  //  C-VLAN tag.} {Drop packets which has a S-VLAN tag followed by a C-VLAN tag.}
  uint64_t dropSCtaggedVlans       :  1; // bit 15

  // field: dropCStaggedVlans
  //  Drop or allow packets with has a C-VLAN followed by a S-VLAN tagged on this
  //  egress port. Note that after a VLAN push operation the pushed VLAN will be
  //  regarded as a C-VLAN. \tabTwo{Allow packets which has a C-VLAN tag followed
  //  by a S-VLAN tag.} {Drop packets which has a C-VLAN tag followed by a S-VLAN
  //  tag.}
  uint64_t dropCStaggedVlans       :  1; // bit 14

  // field: dropDualTaggedVlans
  //  Drop or allow packets with has more than one VLAN tag on this egress port.
  //  \tabTwo{Allow packets which has more than one VLAN tag.} {Drop packets which
  //  has more than one VLAN tag.}
  uint64_t dropDualTaggedVlans     :  1; // bit 13

  // field: dropL2MulticastFrames
  //  Drop or allow L2 multicast packets on this egress port. Observe that this L2
  //  multicast bit takes the register \register{L2 Multicast Handling} into
  //  account to determine if this packet is a L2 multicast packet or not.
  //  \tabTwo{Allow L2 multicast packets}{Drop L2 multicast packets.}
  uint64_t dropL2MulticastFrames   :  1; // bit 12

  // field: dropL2FloodingFrames
  //  Drop or allow L2 flooding packets on this egress port. Observe that this
  //  rule takes the \field{L2 Multicast Handling}{unknownL2McFilterRule} into
  //  account. \tabTwo{Allow L2 flooding packets.}{Drop L2 flooding packets.}
  uint64_t dropL2FloodingFrames    :  1; // bit 11

  // field: dropL2BroadcastFrames
  //  Drop or allow L2 broadcast packets on this egress port. \tabTwo{Allow L2
  //  broadcast packets.}{Drop L2 broadcast packets.}
  uint64_t dropL2BroadcastFrames   :  1; // bit 10

  // field: dropIPv6MulticastPackets
  //  Drop or allow IPv6 Multicast packets on this egress port. \tabTwo{Allow IPv6
  //  MC packets.}{Drop IPv6 MC packets.}
  uint64_t dropIPv6MulticastPackets:  1; // bit 9

  // field: dropIPv4MulticastPackets
  //  Drop or allow IPv4 Multicast packets on this egress port. \tabTwo{Allow IPv4
  //  MC packets.}{1 = Drop IPv4 MC packets.}
  uint64_t dropIPv4MulticastPackets:  1; // bit 8

  // field: dropMPLSPackets
  //  Drop or allow MPLS packets on this source port. \tabTwo{Allow MPLS
  //  packets.}{Drop MPLS packets.}
  uint64_t dropMPLSPackets         :  1; // bit 7

  // field: dropIPv6Packets
  //  Drop or allow IPv6 packets on this egress port. \tabTwo{Allow IPv6
  //  packets.}{Drop IPv6 packets.}
  uint64_t dropIPv6Packets         :  1; // bit 6

  // field: dropIPv4Packets
  //  Drop or allow IPv4 packets on this egress port. \tabTwo{Allow IPv4
  //  packets.}{Drop IPv4 packets.}
  uint64_t dropIPv4Packets         :  1; // bit 5

  // field: dropUntaggedVlans
  //  Drop or Allow packets that are VLAN untagged on this egress port.
  //  \tabTwo{Allow untagged packets.}{Drop untagged packets.}
  uint64_t dropUntaggedVlans       :  1; // bit 4

  // field: dropSingleTaggedVlans
  //  Drop or Allow packets that are VLAN untagged on this egress port.
  //  \tabTwo{Allow untagged packets.}{Drop untagged packets.}
  uint64_t dropSingleTaggedVlans   :  1; // bit 3

  // field: moreThanOneVlans
  //  When filtering with dropCtaggedVlans or dropStaggedVlans then this field
  //  must be set to 1.
  uint64_t moreThanOneVlans        :  1; // bit 2

  // field: dropStaggedVlans
  //  Drop or allow service VLAN tagged packets on this egress port. Must set
  //  moreThanOneVlans when this is used. Note that after a VLAN push operation
  //  the pushed VLAN will be regarded as a C-VLAN. \tabTwo{Allow S-VLANs.}{Drop
  //  S-VLANs.}
  uint64_t dropStaggedVlans        :  1; // bit 1

  // field: dropCtaggedVlans
  //  Drop or allow customer VLAN tagged packets on this egress port. Will only
  //  drop packets that has exactly one VLAN tag. Must set moreThanOneVlans when
  //  this is used. Note that after a VLAN push operation the pushed VLAN will be
  //  regarded as a C-VLAN. \tabTwo{Allow C-VLANs.}{Drop C-VLANs.}
  uint64_t dropCtaggedVlans        :  1; // bit 0
} t_IngressEgressPortPacketTypeFilter;

// ------- Struct declaration for register: IPPPacketHeadCounter --------- 
typedef struct {

  // field: packets
  //  Number of packet headers.
  uint64_t packets: 32; // bit 0 to 31
} t_IPPPacketHeadCounter;

// ------- Struct declaration for register: ColorRemapFromEgressPort --------- 
typedef struct {

  // field: color2Dei
  //  New DEI value based on packet color. This is located in the outermost VLAN
  //  of the transmitted packet.\newline \tabTwoByThree{bit 0}{DEI value for
  //  green} {bit 1}{DEI value for yellow} {bit 2}{DEI value for red}
  uint64_t color2Dei:  3; // bit 34 to 36

  // field: tosMask
  //  Mask for updating the TOS/TC field. For each bit in the mask, 0 means keep
  //  original value, 1 means update new value to that bit.
  uint64_t tosMask  :  8; // bit 26 to 33

  // field: color2Tos
  //  New TOS/TC value based on packet color. \newline \tabTwoByThree{bits
  //  [0:7]}{TOS/TC value for green} {bits [8:15]}{TOS/TC value for yellow} {bits
  //  [16:23]}{TOS/TC value for red}
  uint64_t color2Tos: 24; // bit 2 to 25

  // field: colorMode
  //  \tabFour{Skip remap}{Remap to L3 only}{Remap to L2 only}{Remap to L2 and L3}
  uint64_t colorMode:  2; // bit 0 to 1
} t_ColorRemapFromEgressPort;

// ------- Struct declaration for register: IngressNATOperation --------- 
typedef struct {

  // field: port
  //  The new L4 Port.
  uint64_t port         : 16; // bit 35 to 50

  // field: ipAddress
  //  The new IP Address.
  uint64_t ipAddress    : 32; // bit 3 to 34

  // field: replaceL4Port
  //  Replace TCP/UDP port. \tabTwo{No.}{Yes.}
  uint64_t replaceL4Port:  1; // bit 2

  // field: replaceIP
  //  Replace IP address. \tabTwo{No.}{Yes.}
  uint64_t replaceIP    :  1; // bit 1

  // field: replaceSrc
  //  Replace Source or Destination. \tabTwo{Destiantion}{Source}
  uint64_t replaceSrc   :  1; // bit 0
} t_IngressNATOperation;

// ------- Struct declaration for register: SNAPLLCDecodingOptions --------- 
typedef struct {

  // field: sendToCpu
  //  When a LLC is not equal to (dsap==0xAA and ssap==0xAA and ctrl==0x03) then
  //  packet will be sent to cpu. Bit 0 is from port 0, bit 1 is for port 1, etc.
  uint64_t sendToCpu:  6; // bit 16 to 21

  // field: ethSize
  //  What maximum size of packet shall be interpreted as SNAP packet.
  uint64_t ethSize  : 16; // bit 0 to 15
} t_SNAPLLCDecodingOptions;

// ------- Struct declaration for register: IPPDebugl2DaHashHitAndBucket --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value:  3; // bit 0 to 2
} t_IPPDebugl2DaHashHitAndBucket;

// ------- Struct declaration for register: IPQoSMappingTable --------- 
typedef struct {

  // field: newExp
  //  New Exp value to be used.
  uint64_t newExp      :  3; // bit 9 to 11

  // field: updateExp
  //  If the packet enterns a new MPLS tunnel using the \register{Next Hop Packet
  //  Insert MPLS Header} then use this Exp for the outermost MPLS label.
  //  \tabTwo{No. Dont Remap.}{Yes. Remap to this new value}
  uint64_t updateExp   :  1; // bit 8

  // field: ecnTos
  //  The outgoing TOS [1:0] ECN bits
  uint64_t ecnTos      :  2; // bit 6 to 7

  // field: pcp
  //  Packets new PCP
  uint64_t pcp         :  3; // bit 3 to 5

  // field: updatePcp
  //  Update Pcp field in outgoing packet. \tabTwo{Do not update.}{Update.}
  uint64_t updatePcp   :  1; // bit 2

  // field: cfiDei
  //  Packets new CFI/DEI
  uint64_t cfiDei      :  1; // bit 1

  // field: updateCfiDei
  //  Update CfiDei field in outgoing packet. \tabTwo{Do not update.}{Update.}
  uint64_t updateCfiDei:  1; // bit 0
} t_IPQoSMappingTable;

// ------- Struct declaration for register: IngressAdmissionControlTokenBucketConfiguration --------- 
typedef struct {

  // field: byteCorrection
  //  Extra bytes per packet for IFG correction, only valid under byte mode.
  //  Default is 4 byte FCS plus 20 byte IFG.
  uint64_t byteCorrection :  8; // bit 84 to 91

  // field: tokenMode
  //  \tabFour{Count in bytes and add extra bytes for metering.}{Count in bytes
  //  and substract extra bytes for metering.}{Count in packets.}{No tokens are
  //  counted.}
  uint64_t tokenMode      :  2; // bit 82 to 83

  // field: maxLength
  //  Maximum allowed packet length in bytes. Packets with bytes larger than this
  //  value will be dropped before metering.
  uint64_t maxLength      : 15; // bit 67 to 81

  // field: dropMask
  //  Drop mask for the three colors obtained from the metering result. For each
  //  bit set to 1 the corresponding color shall drop the packet. Bit 0, 1, 2
  //  represents drop or not for green, yellow and red respectively
  uint64_t dropMask       :  3; // bit 64 to 66

  // field: colorBlind
  //  \tabTwo{color-aware: The metering result is based on the initial coloring
  //  from the ingress process pipeline.}{color-blind: The metering ignores any
  //  pre-coloring.}
  uint64_t colorBlind     :  1; // bit 63

  // field: bucketMode
  //  \tabTwo{srTCM}{trTCM}
  uint64_t bucketMode     :  1; // bit 62

  // field: tick1
  //  Select one of the 5 available ticks for token bucket 1. The tick frequencies
  //  are configured globaly in the Core Tick Configuration register.
  uint64_t tick1          :  3; // bit 59 to 61

  // field: tokens1
  //  Number of tokens added each tick for token bucket 1.
  uint64_t tokens1        : 12; // bit 47 to 58

  // field: bucketCapacity1
  //  Capacity for token bucket 1.
  uint64_t bucketCapacity1: 16; // bit 31 to 46

  // field: tick0
  //  Select one of the 5 available ticks for token bucket 0. The tick frequencies
  //  are configured globaly in the Core Tick Configuration register.
  uint64_t tick0          :  3; // bit 28 to 30

  // field: tokens0
  //  Number of tokens added each tick for token bucket 0.
  uint64_t tokens0        : 12; // bit 16 to 27

  // field: bucketCapacity0
  //  Capacity for token bucket 0.
  uint64_t bucketCapacity0: 16; // bit 0 to 15
} t_IngressAdmissionControlTokenBucketConfiguration;

// ------- Struct declaration for register: L2BroadcastStormControlRateConfiguration --------- 
typedef struct {

  // field: ifgCorrection
  //  Extra bytes per packet to correct for IFG in byte mode. Default is 4 byte
  //  FCS plus 20 byte IFG.
  uint64_t ifgCorrection  :  8; // bit 16 to 23

  // field: tick
  //  Select one of the five available core ticks. The tick frequencies are
  //  configured globaly in the core Tick Configuration register.
  uint64_t tick           :  3; // bit 13 to 15

  // field: tokens
  //  The number of tokens added each tick
  uint64_t tokens         : 12; // bit 1 to 12

  // field: packetsNotBytes
  //  If set the bucket will count packets, if cleared bytes
  uint64_t packetsNotBytes:  1; // bit 0
} t_L2BroadcastStormControlRateConfiguration;

// ------- Struct declaration for register: L2BroadcastStormControlEnable --------- 
typedef struct {

  // field: enable
  //  Bitmask where the index is the Egress Ports
  uint64_t enable:  6; // bit 0 to 5
} t_L2BroadcastStormControlEnable;

// ------- Struct declaration for register: PortTCXoffTotalThreshold --------- 
typedef struct {

  // field: trip
  //  \begin{fieldValues} \dscValue [0] Normal operation \dscValue [1] Force this
  //  threshold to be counted as exceeded \end{fieldValues} Only valid if this
  //  Xoff threshold is enabled.
  uint64_t trip  :  1; // bit 11

  // field: enable
  //  \begin{fieldValues} \dscValue [0] This Xoff threshold is disabled \dscValue
  //  [1] This Xoff threshold is enabled\end{fieldValues}
  uint64_t enable:  1; // bit 10

  // field: cells
  //  Xoff threshold for the sum of reserved and FFA cells used for this source
  //  port and traffic class combination
  uint64_t cells : 10; // bit 0 to 9
} t_PortTCXoffTotalThreshold;

// ------- Struct declaration for register: EgressConfigurableACL0SearchMask --------- 
typedef struct {

  // field: masklarge
  //  Which bits to compare in the \register{Egress Configurable ACL 0 Large
  //  Table} lookup. A bit set to 1 means the corresponding bit in the search data
  //  is compared and 0 means the bit is ignored.
  uint8_t masklarge[17]; // bit 135 to 269

  // field: masksmall
  //  Which bits to compare in the \register{Egress Configurable ACL 0 Small
  //  Table} lookup. A bit set to 1 means the corresponding bit in the search data
  //  is compared and 0 means the bit is ignored.
  uint8_t masksmall[17]; // bit 0 to 134
} t_EgressConfigurableACL0SearchMask;

// ------- Struct declaration for register: SMONPacketCounter --------- 
typedef struct {

  // field: packets
  //  Number of packets.
  uint64_t packets: 32; // bit 0 to 31
} t_SMONPacketCounter;

// ------- Struct declaration for register: L2FloodingStormControlBucketThresholdConfiguration --------- 
typedef struct {

  // field: threshold
  //  Minimum number of tokens in bucket for the status to be set to accept.
  uint64_t threshold: 16; // bit 0 to 15
} t_L2FloodingStormControlBucketThresholdConfiguration;

// ------- Struct declaration for register: CPUReasonCodeOperation --------- 
typedef struct {

  // field: end
  //  End of CPU reason code.
  uint64_t end                  : 16; // bit 26 to 41

  // field: start
  //  Start of CPU reason code.
  uint64_t start                : 16; // bit 10 to 25

  // field: origCpuPkt
  //  Force the packet to the CPU to be the originial,unmodified, packet.
  //  \tabTwo{No, modification will happen to packet.}{Yes, force the packet to be
  //  unmodified.}
  uint64_t origCpuPkt           :  1; // bit 9

  // field: forceUpdateOrigCpuPkt
  //  If this reason code is hit shall the origCpuPkt field be updated?
  //  \tabTwo{No, no update.}{Yes, update.}
  uint64_t forceUpdateOrigCpuPkt:  1; // bit 8

  // field: eQueue
  //  Egress queue
  uint64_t eQueue               :  3; // bit 5 to 7

  // field: forceQueue
  //  Force the packet to the CPU port with a new egress queue when the CPU reason
  //  code hit in the range.
  uint64_t forceQueue           :  1; // bit 4

  // field: port
  //  The new destination to replace the CPU port.
  uint64_t port                 :  3; // bit 1 to 3

  // field: mutableCpu
  //  Force the packet to another port instead of the CPU port when the CPU reason
  //  code hit in the range.
  uint64_t mutableCpu           :  1; // bit 0
} t_CPUReasonCodeOperation;

// ------- Struct declaration for register: HashBasedL3RoutingTable --------- 
typedef struct {

  // field: ecmpShift
  //  How many bits the masked ECMP hash will be right shifted.
  uint64_t ecmpShift     :  3; // bit 149 to 151

  // field: ecmpMask
  //  How many bits of the ECMP hash byte will be used when calculating the ECMP
  //  offset. This byte is AND:ed with the ECMP hash to determine which bits shall
  //  be used as offset.
  uint64_t ecmpMask      :  6; // bit 143 to 148

  // field: useECMP
  //  Enables the use of ECMP hash byte to calculate the next hop pointer.
  //  \tabTwo{Use ECMP hash.} {Do not use ECMP hash.}
  uint64_t useECMP       :  1; // bit 142

  // field: nextHopPointer
  //  Index into the \register{Next Hop Table} for this destination.
  uint64_t nextHopPointer: 10; // bit 132 to 141

  // field: destIPAddr
  //  The IP or MPLS address to be matched. If the entry is an IPv4 entry then
  //  only bits [31:0] is used. If the entry is a MPLS entry then bits
  //  [\cSRCPORTW-1:0] contains the source port while bits
  //  [\cSRCPORTW+19:\cSRCPORTW] contains the MPLS label to match.
  uint8_t destIPAddr[16]; // bit 4 to 131

  // field: vrf
  //  This entries VRF. The packets assigned VRF will be compared with this field.
  uint64_t vrf           :  2; // bit 2 to 3

  // field: mpls
  //  This is an MPLS entry, \tabTwo{IP entry.} {MPLS entry.}
  uint64_t mpls          :  1; // bit 1

  // field: ipVersion
  //  Select if this is an IPv4 or IPv6 entry. \tabTwo{IPv4 entry.} {IPv6 entry.}
  uint64_t ipVersion     :  1; // bit 0
} t_HashBasedL3RoutingTable;

// ------- Struct declaration for register: PortShaperBucketCapacityConfiguration --------- 
typedef struct {

  // field: bucketCapacity
  //  Capacity of the token bucket
  uint64_t bucketCapacity: 17; // bit 0 to 16
} t_PortShaperBucketCapacityConfiguration;

// ------- Struct declaration for register: L3RoutingTCAM --------- 
typedef struct {

  // field: valid
  //  If set, this entry is valid
  uint64_t valid          :  1; // bit 264

  // field: destIPAddrMaskN
  //  Mask for the destIPAddr field. For each bit in the mask, 0 means the bit is
  //  valid for comparison, 1 means the comparison ignores this bit.
  uint8_t destIPAddrMaskN[16]; // bit 136 to 263

  // field: vrfMaskN
  //  Mask for the vrf field. For each bit in the mask, 0 means the bit is valid
  //  for comparison, 1 means the comparison ignores this bit.
  uint64_t vrfMaskN       :  2; // bit 134 to 135

  // field: protoMaskN
  //  Mask for the proto field. For each bit in the mask, 0 means the bit is valid
  //  for comparison, 1 means the comparison ignores this bit.
  uint64_t protoMaskN     :  2; // bit 132 to 133

  // field: destIPAddr
  //  The IP or MPLS address to be matched. If the entry is an IPv4 entry then
  //  bits [31:0] should be set to the IPv4 address. If the entry is an MPLS entry
  //  then bits [\cSRCPORTW-1:0] should contain the source port while bits
  //  [\cSRCPORTW+19:\cSRCPORTW] should contain the MPLS label. destIPAddrMaskN
  //  determines the bits in the field that can be ignored for comparison.
  uint8_t destIPAddr[16]; // bit 4 to 131

  // field: vrf
  //  This entries VRF. The packets assigned VRF will be compared with this field.
  //  vrfMaskN determines the bits in the field that can be ignored for
  //  comparison.
  uint64_t vrf            :  2; // bit 2 to 3

  // field: proto
  //  Select if this is an IPv4, IPv6 or MPLS entry. \ifdef{\texRoutingAndMpls}{
  //  \tabFour{Reserved} {MPLS Entry.} {IPv4 entry.} {IPv6 entry.} }{}
  //  \ifdef{\texRoutingOnly}{ \tabFour{Reserved} {Reserved.} {IPv4 entry.} {IPv6
  //  entry.} }{} \ifdef{\texMplsOnly}{ \tabFour{Reserved} {MPLS Entry.}
  //  {Reserved.} {Reserved.} }{} protoMaskN determines the bits in the field that
  //  can be ignored for comparison.
  uint64_t proto          :  2; // bit 0 to 1
} t_L3RoutingTCAM;

// ------- Struct declaration for register: PortXonFFAThreshold --------- 
typedef struct {

  // field: cells
  //  Xon threshold for the number of used FFA cells for this source port
  uint64_t cells: 10; // bit 0 to 9
} t_PortXonFFAThreshold;

// ------- Struct declaration for register: IEEE8021XandEAPOLDecoderDrop --------- 
typedef struct {

  // field: packets
  //  Number of dropped packets.
  uint64_t packets: 32; // bit 0 to 31
} t_IEEE8021XandEAPOLDecoderDrop;

// ------- Struct declaration for register: L3LPMResult --------- 
typedef struct {

  // field: nextHopPointer
  //  Index into the \register{Next Hop Table} for this destination.
  uint64_t nextHopPointer: 10; // bit 10 to 19

  // field: ecmpShift
  //  How many bits the masked ECMP hash will be right shifted.
  uint64_t ecmpShift     :  3; // bit 7 to 9

  // field: ecmpMask
  //  How many bits of the ECMP hash byte will be used when calculating the ECMP
  //  offset. This byte is AND:ed with the ECMP hash to determine which bits shall
  //  be used as offset.
  uint64_t ecmpMask      :  6; // bit 1 to 6

  // field: useECMP
  //  Enables the use of ECMP hash byte to calculate the next hop pointer.
  //  \tabTwo{Use ECMP hash.} {Do not use ECMP hash.}
  uint64_t useECMP       :  1; // bit 0
} t_L3LPMResult;

// ------- Struct declaration for register: IngressConfigurableACL1TCAM --------- 
typedef struct {

  // field: compareData
  //  The data which shall be compared in this entry. Observe that this compare
  //  data must be AND:ed by software before the entry is searched. The hardware
  //  does not do the AND between mask and compareData (In order to save area).
  uint8_t compareData[17]; // bit 136 to 270

  // field: mask
  //  Which bits to compare in this entry.
  uint8_t mask[17]; // bit 1 to 135

  // field: valid
  //  Is this entry valid. \tabTwo{No}{Yes}
  uint64_t valid      :  1; // bit 0
} t_IngressConfigurableACL1TCAM;

// ------- Struct declaration for register: PSErrorCounter --------- 
typedef struct {

  // field: overflow
  //  Number of FIFO overflows in the PS-converter. This error will cause packet
  //  corruptions.
  uint64_t overflow: 32; // bit 32 to 63

  // field: underrun
  //  Number of packets which have empty cycles caused by the internal
  //  PS-converter but not the external halt during packet transmissions.
  uint64_t underrun: 32; // bit 0 to 31
} t_PSErrorCounter;

// ------- Struct declaration for register: IngressConfigurableACL0PreLookup --------- 
typedef struct {

  // field: rulePtr
  //  If the valid is entry then this rule pointer will be used.
  uint64_t rulePtr:  3; // bit 1 to 3

  // field: valid
  //  Is this entry valid. If not then use default port rule.
  uint64_t valid  :  1; // bit 0
} t_IngressConfigurableACL0PreLookup;

// ------- Struct declaration for register: IPPDebugisFlooding --------- 
typedef struct {

  // field: value
  //  Status from last processed packet.
  uint64_t value:  1; // bit 0
} t_IPPDebugisFlooding;

// ------- Struct declaration for register: DWRRBucketCapacityConfiguration --------- 
typedef struct {

  // field: bucketCapacity
  //  Capacity of the byte bucket
  uint64_t bucketCapacity: 18; // bit 0 to 17
} t_DWRRBucketCapacityConfiguration;

// ------- Struct declaration for register: PortPauseSettings --------- 
typedef struct {

  // field: pattern
  //  Each bit refers to one traffic class (bit 0 = TC 0) \begin{fieldValues}
  //  \dscValue [0] Not paused \dscValue [1] Paused \end{fieldValues}
  uint64_t pattern :  8; // bit 12 to 19

  // field: force
  //  Each bit refers to one traffic class (bit 0 = TC 0) \begin{fieldValues}
  //  \dscValue [0] No force \dscValue [1] Force the pause state to that set in
  //  the pattern field \end{fieldValues} Only valid if pausing is enabled.
  uint64_t force   :  8; // bit 4 to 11

  // field: reserved
  //  Reserved.
  uint64_t reserved:  2; // bit 2 to 3

  // field: mode
  //  On a port where both pausing and tail-drop is enabled the modes must match
  //  for the calculation of used FFA to be correct.\begin{fieldValues} \dscValue
  //  [0] Priority mode \dscValue [1] Port mode\end{fieldValues}
  uint64_t mode    :  1; // bit 1

  // field: enable
  //  \begin{fieldValues} \dscValue [0] Pausing disabled \dscValue [1] Pausing
  //  enabled\end{fieldValues}
  uint64_t enable  :  1; // bit 0
} t_PortPauseSettings;

//
// API Functions for register / tables
//

// ForceNonVLANPacketToSpecificColor access functions.
extern int rd_ForceNonVLANPacketToSpecificColor(struct dubhe1000_adapter *adapter,t_ForceNonVLANPacketToSpecificColor* data);
extern int wr_ForceNonVLANPacketToSpecificColor(struct dubhe1000_adapter *adapter,t_ForceNonVLANPacketToSpecificColor* data);

// IKEDecoderDrop access functions.
extern int rd_IKEDecoderDrop(struct dubhe1000_adapter *adapter,t_IKEDecoderDrop* data);
extern int wr_IKEDecoderDrop(struct dubhe1000_adapter *adapter,t_IKEDecoderDrop* data);

// DrainPortDrop access functions.
extern int rd_DrainPortDrop(struct dubhe1000_adapter *adapter, uint64_t idx,t_DrainPortDrop* data);
extern int wr_DrainPortDrop(struct dubhe1000_adapter *adapter, uint64_t idx,t_DrainPortDrop* data);

// IPPDebugnextHopPtrFinal access functions.
extern int rd_IPPDebugnextHopPtrFinal(struct dubhe1000_adapter *adapter,t_IPPDebugnextHopPtrFinal* data);
extern int wr_IPPDebugnextHopPtrFinal(struct dubhe1000_adapter *adapter,t_IPPDebugnextHopPtrFinal* data);

// IngressMultipleSpanningTreeState access functions.
extern int rd_IngressMultipleSpanningTreeState(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressMultipleSpanningTreeState* data);
extern int wr_IngressMultipleSpanningTreeState(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressMultipleSpanningTreeState* data);

// SourcePortTable access functions.
extern int rd_SourcePortTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_SourcePortTable* data);
extern int wr_SourcePortTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_SourcePortTable* data);

// IngressConfigurableACL1SearchMask access functions.
extern int rd_IngressConfigurableACL1SearchMask(struct dubhe1000_adapter *adapter,t_IngressConfigurableACL1SearchMask* data);
extern int wr_IngressConfigurableACL1SearchMask(struct dubhe1000_adapter *adapter,t_IngressConfigurableACL1SearchMask* data);

// IPPDebugdropPktAfterL2Decode access functions.
extern int rd_IPPDebugdropPktAfterL2Decode(struct dubhe1000_adapter *adapter,t_IPPDebugdropPktAfterL2Decode* data);
extern int wr_IPPDebugdropPktAfterL2Decode(struct dubhe1000_adapter *adapter,t_IPPDebugdropPktAfterL2Decode* data);

// IngressPacketFilteringDrop access functions.
extern int rd_IngressPacketFilteringDrop(struct dubhe1000_adapter *adapter,t_IngressPacketFilteringDrop* data);
extern int wr_IngressPacketFilteringDrop(struct dubhe1000_adapter *adapter,t_IngressPacketFilteringDrop* data);

// L2ActionTablePerPortDrop access functions.
extern int rd_L2ActionTablePerPortDrop(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2ActionTablePerPortDrop* data);
extern int wr_L2ActionTablePerPortDrop(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2ActionTablePerPortDrop* data);

// RouterMTUTable access functions.
extern int rd_RouterMTUTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_RouterMTUTable* data);
extern int wr_RouterMTUTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_RouterMTUTable* data);

// SecondTunnelExitDrop access functions.
extern int rd_SecondTunnelExitDrop(struct dubhe1000_adapter *adapter,t_SecondTunnelExitDrop* data);
extern int wr_SecondTunnelExitDrop(struct dubhe1000_adapter *adapter,t_SecondTunnelExitDrop* data);

// L2IEEE1588DecoderDrop access functions.
extern int rd_L2IEEE1588DecoderDrop(struct dubhe1000_adapter *adapter,t_L2IEEE1588DecoderDrop* data);
extern int wr_L2IEEE1588DecoderDrop(struct dubhe1000_adapter *adapter,t_L2IEEE1588DecoderDrop* data);

// IPPPMDrop access functions.
extern int rd_IPPPMDrop(struct dubhe1000_adapter *adapter,t_IPPPMDrop* data);
extern int wr_IPPPMDrop(struct dubhe1000_adapter *adapter,t_IPPPMDrop* data);

// DebugEPPCounter access functions.
extern int rd_DebugEPPCounter(struct dubhe1000_adapter *adapter, uint64_t idx,t_DebugEPPCounter* data);
extern int wr_DebugEPPCounter(struct dubhe1000_adapter *adapter, uint64_t idx,t_DebugEPPCounter* data);

// RequeueOverflowDrop access functions.
extern int rd_RequeueOverflowDrop(struct dubhe1000_adapter *adapter,t_RequeueOverflowDrop* data);
extern int wr_RequeueOverflowDrop(struct dubhe1000_adapter *adapter,t_RequeueOverflowDrop* data);

// CoreVersion access functions.
extern int rd_CoreVersion(struct dubhe1000_adapter *adapter,t_CoreVersion* data);

// IPPDebugdstPortmask access functions.
extern int rd_IPPDebugdstPortmask(struct dubhe1000_adapter *adapter,t_IPPDebugdstPortmask* data);
extern int wr_IPPDebugdstPortmask(struct dubhe1000_adapter *adapter,t_IPPDebugdstPortmask* data);

// LearningPacketControl access functions.
extern int rd_LearningPacketControl(struct dubhe1000_adapter *adapter,t_LearningPacketControl* data);
extern int wr_LearningPacketControl(struct dubhe1000_adapter *adapter,t_LearningPacketControl* data);

// IPPDebugl2DaHashKey access functions.
extern int rd_IPPDebugl2DaHashKey(struct dubhe1000_adapter *adapter,t_IPPDebugl2DaHashKey* data);
extern int wr_IPPDebugl2DaHashKey(struct dubhe1000_adapter *adapter,t_IPPDebugl2DaHashKey* data);

// EgressConfigurableACL1TCAMAnswer access functions.
extern int rd_EgressConfigurableACL1TCAMAnswer(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressConfigurableACL1TCAMAnswer* data);
extern int wr_EgressConfigurableACL1TCAMAnswer(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressConfigurableACL1TCAMAnswer* data);

// LearningDAMAC access functions.
extern int rd_LearningDAMAC(struct dubhe1000_adapter *adapter,t_LearningDAMAC* data);
extern int wr_LearningDAMAC(struct dubhe1000_adapter *adapter,t_LearningDAMAC* data);

// MinimumBufferFree access functions.
extern int rd_MinimumBufferFree(struct dubhe1000_adapter *adapter,t_MinimumBufferFree* data);

// IngressEgressPacketFilteringDrop access functions.
extern int rd_IngressEgressPacketFilteringDrop(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressEgressPacketFilteringDrop* data);
extern int wr_IngressEgressPacketFilteringDrop(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressEgressPacketFilteringDrop* data);

// IngressSpanningTreeDropBlocking access functions.
extern int rd_IngressSpanningTreeDropBlocking(struct dubhe1000_adapter *adapter,t_IngressSpanningTreeDropBlocking* data);
extern int wr_IngressSpanningTreeDropBlocking(struct dubhe1000_adapter *adapter,t_IngressSpanningTreeDropBlocking* data);

// IngressVIDMACRangeAssignmentAnswer access functions.
extern int rd_IngressVIDMACRangeAssignmentAnswer(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressVIDMACRangeAssignmentAnswer* data);
extern int wr_IngressVIDMACRangeAssignmentAnswer(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressVIDMACRangeAssignmentAnswer* data);

// RARPPacketDecoderOptions access functions.
extern int rd_RARPPacketDecoderOptions(struct dubhe1000_adapter *adapter,t_RARPPacketDecoderOptions* data);
extern int wr_RARPPacketDecoderOptions(struct dubhe1000_adapter *adapter,t_RARPPacketDecoderOptions* data);

// ForceUnknownL3PacketToSpecificEgressQueue access functions.
extern int rd_ForceUnknownL3PacketToSpecificEgressQueue(struct dubhe1000_adapter *adapter,t_ForceUnknownL3PacketToSpecificEgressQueue* data);
extern int wr_ForceUnknownL3PacketToSpecificEgressQueue(struct dubhe1000_adapter *adapter,t_ForceUnknownL3PacketToSpecificEgressQueue* data);

// DebugCounterspVidOpSetup access functions.
extern int rd_DebugCounterspVidOpSetup(struct dubhe1000_adapter *adapter,t_DebugCounterspVidOpSetup* data);
extern int wr_DebugCounterspVidOpSetup(struct dubhe1000_adapter *adapter,t_DebugCounterspVidOpSetup* data);

// EPPDebugimActive access functions.
extern int rd_EPPDebugimActive(struct dubhe1000_adapter *adapter,t_EPPDebugimActive* data);
extern int wr_EPPDebugimActive(struct dubhe1000_adapter *adapter,t_EPPDebugimActive* data);

// EmptyMaskDrop access functions.
extern int rd_EmptyMaskDrop(struct dubhe1000_adapter *adapter,t_EmptyMaskDrop* data);
extern int wr_EmptyMaskDrop(struct dubhe1000_adapter *adapter,t_EmptyMaskDrop* data);

// PrioShaperEnable access functions.
extern int rd_PrioShaperEnable(struct dubhe1000_adapter *adapter,t_PrioShaperEnable* data);
extern int wr_PrioShaperEnable(struct dubhe1000_adapter *adapter,t_PrioShaperEnable* data);

// UnknownIngressDrop access functions.
extern int rd_UnknownIngressDrop(struct dubhe1000_adapter *adapter,t_UnknownIngressDrop* data);
extern int wr_UnknownIngressDrop(struct dubhe1000_adapter *adapter,t_UnknownIngressDrop* data);

// EgressEthernetTypeforVLANtag access functions.
extern int rd_EgressEthernetTypeforVLANtag(struct dubhe1000_adapter *adapter,t_EgressEthernetTypeforVLANtag* data);
extern int wr_EgressEthernetTypeforVLANtag(struct dubhe1000_adapter *adapter,t_EgressEthernetTypeforVLANtag* data);

// NextHopMPLSTable access functions.
extern int rd_NextHopMPLSTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_NextHopMPLSTable* data);
extern int wr_NextHopMPLSTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_NextHopMPLSTable* data);

// EgressPortDisabledDrop access functions.
extern int rd_EgressPortDisabledDrop(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressPortDisabledDrop* data);
extern int wr_EgressPortDisabledDrop(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressPortDisabledDrop* data);

// LinkAggregationCtrl access functions.
extern int rd_LinkAggregationCtrl(struct dubhe1000_adapter *adapter,t_LinkAggregationCtrl* data);
extern int wr_LinkAggregationCtrl(struct dubhe1000_adapter *adapter,t_LinkAggregationCtrl* data);

// PacketBufferStatus access functions.
extern int rd_PacketBufferStatus(struct dubhe1000_adapter *adapter,t_PacketBufferStatus* data);

// IPPDebugl2DaTcamHitsAndCast access functions.
extern int rd_IPPDebugl2DaTcamHitsAndCast(struct dubhe1000_adapter *adapter,t_IPPDebugl2DaTcamHitsAndCast* data);
extern int wr_IPPDebugl2DaTcamHitsAndCast(struct dubhe1000_adapter *adapter,t_IPPDebugl2DaTcamHitsAndCast* data);

// IPv4TOSFieldToPacketColorMappingTable access functions.
extern int rd_IPv4TOSFieldToPacketColorMappingTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_IPv4TOSFieldToPacketColorMappingTable* data);
extern int wr_IPv4TOSFieldToPacketColorMappingTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_IPv4TOSFieldToPacketColorMappingTable* data);

// EnableEnqueueToPortsAndQueues access functions.
extern int rd_EnableEnqueueToPortsAndQueues(struct dubhe1000_adapter *adapter, uint64_t idx,t_EnableEnqueueToPortsAndQueues* data);
extern int wr_EnableEnqueueToPortsAndQueues(struct dubhe1000_adapter *adapter, uint64_t idx,t_EnableEnqueueToPortsAndQueues* data);

// MPLSEXPFieldToEgressQueueMappingTable access functions.
extern int rd_MPLSEXPFieldToEgressQueueMappingTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_MPLSEXPFieldToEgressQueueMappingTable* data);
extern int wr_MPLSEXPFieldToEgressQueueMappingTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_MPLSEXPFieldToEgressQueueMappingTable* data);

// CoreTickSelect access functions.
extern int rd_CoreTickSelect(struct dubhe1000_adapter *adapter,t_CoreTickSelect* data);
extern int wr_CoreTickSelect(struct dubhe1000_adapter *adapter,t_CoreTickSelect* data);

// DebugCounterdoPktUpdateSetup access functions.
extern int rd_DebugCounterdoPktUpdateSetup(struct dubhe1000_adapter *adapter,t_DebugCounterdoPktUpdateSetup* data);
extern int wr_DebugCounterdoPktUpdateSetup(struct dubhe1000_adapter *adapter,t_DebugCounterdoPktUpdateSetup* data);

// EPPDebugisIPv6 access functions.
extern int rd_EPPDebugisIPv6(struct dubhe1000_adapter *adapter,t_EPPDebugisIPv6* data);
extern int wr_EPPDebugisIPv6(struct dubhe1000_adapter *adapter,t_EPPDebugisIPv6* data);

// EPPDebugdebugMatchEPP0 access functions.
extern int rd_EPPDebugdebugMatchEPP0(struct dubhe1000_adapter *adapter,t_EPPDebugdebugMatchEPP0* data);
extern int wr_EPPDebugdebugMatchEPP0(struct dubhe1000_adapter *adapter,t_EPPDebugdebugMatchEPP0* data);

// PortReserved access functions.
extern int rd_PortReserved(struct dubhe1000_adapter *adapter, uint64_t idx,t_PortReserved* data);
extern int wr_PortReserved(struct dubhe1000_adapter *adapter, uint64_t idx,t_PortReserved* data);

// ResourceLimiterSet access functions.
extern int rd_ResourceLimiterSet(struct dubhe1000_adapter *adapter, uint64_t idx,t_ResourceLimiterSet* data);
extern int wr_ResourceLimiterSet(struct dubhe1000_adapter *adapter, uint64_t idx,t_ResourceLimiterSet* data);

// InvalidRoutingProtocolDrop access functions.
extern int rd_InvalidRoutingProtocolDrop(struct dubhe1000_adapter *adapter,t_InvalidRoutingProtocolDrop* data);
extern int wr_InvalidRoutingProtocolDrop(struct dubhe1000_adapter *adapter,t_InvalidRoutingProtocolDrop* data);

// EgressConfigurableACL0TCAMAnswer access functions.
extern int rd_EgressConfigurableACL0TCAMAnswer(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressConfigurableACL0TCAMAnswer* data);
extern int wr_EgressConfigurableACL0TCAMAnswer(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressConfigurableACL0TCAMAnswer* data);

// EgressResourceManagerPointer access functions.
extern int rd_EgressResourceManagerPointer(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressResourceManagerPointer* data);
extern int wr_EgressResourceManagerPointer(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressResourceManagerPointer* data);

// IPUnicastReceivedCounter access functions.
extern int rd_IPUnicastReceivedCounter(struct dubhe1000_adapter *adapter, uint64_t idx,t_IPUnicastReceivedCounter* data);
extern int wr_IPUnicastReceivedCounter(struct dubhe1000_adapter *adapter, uint64_t idx,t_IPUnicastReceivedCounter* data);

// EgressConfigurableACL0RulesSetup access functions.
extern int rd_EgressConfigurableACL0RulesSetup(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressConfigurableACL0RulesSetup* data);
extern int wr_EgressConfigurableACL0RulesSetup(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressConfigurableACL0RulesSetup* data);

// SMONSetSearch access functions.
extern int rd_SMONSetSearch(struct dubhe1000_adapter *adapter, uint64_t idx,t_SMONSetSearch* data);
extern int wr_SMONSetSearch(struct dubhe1000_adapter *adapter, uint64_t idx,t_SMONSetSearch* data);

// L2MulticastStormControlRateConfiguration access functions.
extern int rd_L2MulticastStormControlRateConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2MulticastStormControlRateConfiguration* data);
extern int wr_L2MulticastStormControlRateConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2MulticastStormControlRateConfiguration* data);

// SPOverflowDrop access functions.
extern int rd_SPOverflowDrop(struct dubhe1000_adapter *adapter, uint64_t idx,t_SPOverflowDrop* data);

// PortShaperBucketThresholdConfiguration access functions.
extern int rd_PortShaperBucketThresholdConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_PortShaperBucketThresholdConfiguration* data);
extern int wr_PortShaperBucketThresholdConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_PortShaperBucketThresholdConfiguration* data);

// AllowSpecialFrameCheckForL2ActionTable access functions.
extern int rd_AllowSpecialFrameCheckForL2ActionTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_AllowSpecialFrameCheckForL2ActionTable* data);
extern int wr_AllowSpecialFrameCheckForL2ActionTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_AllowSpecialFrameCheckForL2ActionTable* data);

// DebugCounterdebugMatchEPP0Setup access functions.
extern int rd_DebugCounterdebugMatchEPP0Setup(struct dubhe1000_adapter *adapter,t_DebugCounterdebugMatchEPP0Setup* data);
extern int wr_DebugCounterdebugMatchEPP0Setup(struct dubhe1000_adapter *adapter,t_DebugCounterdebugMatchEPP0Setup* data);

// PortTailDropSettings access functions.
extern int rd_PortTailDropSettings(struct dubhe1000_adapter *adapter, uint64_t idx,t_PortTailDropSettings* data);
extern int wr_PortTailDropSettings(struct dubhe1000_adapter *adapter, uint64_t idx,t_PortTailDropSettings* data);

// PortShaperRateConfiguration access functions.
extern int rd_PortShaperRateConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_PortShaperRateConfiguration* data);
extern int wr_PortShaperRateConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_PortShaperRateConfiguration* data);

// DWRRBucketMiscConfiguration access functions.
extern int rd_DWRRBucketMiscConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_DWRRBucketMiscConfiguration* data);
extern int wr_DWRRBucketMiscConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_DWRRBucketMiscConfiguration* data);

// EgressConfigurableACL1RulesSetup access functions.
extern int rd_EgressConfigurableACL1RulesSetup(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressConfigurableACL1RulesSetup* data);
extern int wr_EgressConfigurableACL1RulesSetup(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressConfigurableACL1RulesSetup* data);

// CAPWAPPacketDecoderOptions access functions.
extern int rd_CAPWAPPacketDecoderOptions(struct dubhe1000_adapter *adapter,t_CAPWAPPacketDecoderOptions* data);
extern int wr_CAPWAPPacketDecoderOptions(struct dubhe1000_adapter *adapter,t_CAPWAPPacketDecoderOptions* data);

// EPPPacketTailCounter access functions.
extern int rd_EPPPacketTailCounter(struct dubhe1000_adapter *adapter,t_EPPPacketTailCounter* data);
extern int wr_EPPPacketTailCounter(struct dubhe1000_adapter *adapter,t_EPPPacketTailCounter* data);

// EgressConfigurableACL0LargeTable access functions.
extern int rd_EgressConfigurableACL0LargeTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressConfigurableACL0LargeTable* data);
extern int wr_EgressConfigurableACL0LargeTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressConfigurableACL0LargeTable* data);

// PSPacketHeadCounter access functions.
extern int rd_PSPacketHeadCounter(struct dubhe1000_adapter *adapter,t_PSPacketHeadCounter* data);
extern int wr_PSPacketHeadCounter(struct dubhe1000_adapter *adapter,t_PSPacketHeadCounter* data);

// NATActionTableForceOriginalPacket access functions.
extern int rd_NATActionTableForceOriginalPacket(struct dubhe1000_adapter *adapter,t_NATActionTableForceOriginalPacket* data);
extern int wr_NATActionTableForceOriginalPacket(struct dubhe1000_adapter *adapter,t_NATActionTableForceOriginalPacket* data);

// RouterPortEgressSAMACAddress access functions.
extern int rd_RouterPortEgressSAMACAddress(struct dubhe1000_adapter *adapter, uint64_t idx,t_RouterPortEgressSAMACAddress* data);
extern int wr_RouterPortEgressSAMACAddress(struct dubhe1000_adapter *adapter, uint64_t idx,t_RouterPortEgressSAMACAddress* data);

// OutputDisable access functions.
extern int rd_OutputDisable(struct dubhe1000_adapter *adapter, uint64_t idx,t_OutputDisable* data);
extern int wr_OutputDisable(struct dubhe1000_adapter *adapter, uint64_t idx,t_OutputDisable* data);

// L2AgingCollisionTable access functions.
extern int rd_L2AgingCollisionTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2AgingCollisionTable* data);
extern int wr_L2AgingCollisionTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2AgingCollisionTable* data);

// BOOTPandDHCPDecoderDrop access functions.
extern int rd_BOOTPandDHCPDecoderDrop(struct dubhe1000_adapter *adapter,t_BOOTPandDHCPDecoderDrop* data);
extern int wr_BOOTPandDHCPDecoderDrop(struct dubhe1000_adapter *adapter,t_BOOTPandDHCPDecoderDrop* data);

// IngressResourceManagerDrop access functions.
extern int rd_IngressResourceManagerDrop(struct dubhe1000_adapter *adapter,t_IngressResourceManagerDrop* data);
extern int wr_IngressResourceManagerDrop(struct dubhe1000_adapter *adapter,t_IngressResourceManagerDrop* data);

// IngressAdmissionControlInitialPointer access functions.
extern int rd_IngressAdmissionControlInitialPointer(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressAdmissionControlInitialPointer* data);
extern int wr_IngressAdmissionControlInitialPointer(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressAdmissionControlInitialPointer* data);

// L2TunnelEntryInstructionTable access functions.
extern int rd_L2TunnelEntryInstructionTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2TunnelEntryInstructionTable* data);
extern int wr_L2TunnelEntryInstructionTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2TunnelEntryInstructionTable* data);

// CheckTunnelExitPacketDecoderSize access functions.
extern int rd_CheckTunnelExitPacketDecoderSize(struct dubhe1000_adapter *adapter,t_CheckTunnelExitPacketDecoderSize* data);
extern int wr_CheckTunnelExitPacketDecoderSize(struct dubhe1000_adapter *adapter,t_CheckTunnelExitPacketDecoderSize* data);

// MaximumAllowedVLANDrop access functions.
extern int rd_MaximumAllowedVLANDrop(struct dubhe1000_adapter *adapter,t_MaximumAllowedVLANDrop* data);
extern int wr_MaximumAllowedVLANDrop(struct dubhe1000_adapter *adapter,t_MaximumAllowedVLANDrop* data);

// LACPDecoderDrop access functions.
extern int rd_LACPDecoderDrop(struct dubhe1000_adapter *adapter,t_LACPDecoderDrop* data);
extern int wr_LACPDecoderDrop(struct dubhe1000_adapter *adapter,t_LACPDecoderDrop* data);

// L2ActionTablePortMoveDrop access functions.
extern int rd_L2ActionTablePortMoveDrop(struct dubhe1000_adapter *adapter,t_L2ActionTablePortMoveDrop* data);
extern int wr_L2ActionTablePortMoveDrop(struct dubhe1000_adapter *adapter,t_L2ActionTablePortMoveDrop* data);

// IngressConfigurableACLDrop access functions.
extern int rd_IngressConfigurableACLDrop(struct dubhe1000_adapter *adapter,t_IngressConfigurableACLDrop* data);
extern int wr_IngressConfigurableACLDrop(struct dubhe1000_adapter *adapter,t_IngressConfigurableACLDrop* data);

// PBPacketHeadCounter access functions.
extern int rd_PBPacketHeadCounter(struct dubhe1000_adapter *adapter,t_PBPacketHeadCounter* data);
extern int wr_PBPacketHeadCounter(struct dubhe1000_adapter *adapter,t_PBPacketHeadCounter* data);

// IngressConfigurableACL0LargeTable access functions.
extern int rd_IngressConfigurableACL0LargeTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACL0LargeTable* data);
extern int wr_IngressConfigurableACL0LargeTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACL0LargeTable* data);

// EgressPortDepth access functions.
extern int rd_EgressPortDepth(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressPortDepth* data);

// EgressConfigurableACL0SmallTable access functions.
extern int rd_EgressConfigurableACL0SmallTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressConfigurableACL0SmallTable* data);
extern int wr_EgressConfigurableACL0SmallTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressConfigurableACL0SmallTable* data);

// SelectWhichEgressQoSMappingTableToUse access functions.
extern int rd_SelectWhichEgressQoSMappingTableToUse(struct dubhe1000_adapter *adapter, uint64_t idx,t_SelectWhichEgressQoSMappingTableToUse* data);
extern int wr_SelectWhichEgressQoSMappingTableToUse(struct dubhe1000_adapter *adapter, uint64_t idx,t_SelectWhichEgressQoSMappingTableToUse* data);

// EPPDebugimExtra access functions.
extern int rd_EPPDebugimExtra(struct dubhe1000_adapter *adapter,t_EPPDebugimExtra* data);
extern int wr_EPPDebugimExtra(struct dubhe1000_adapter *adapter,t_EPPDebugimExtra* data);

// DWRRWeightConfiguration access functions.
extern int rd_DWRRWeightConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_DWRRWeightConfiguration* data);
extern int wr_DWRRWeightConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_DWRRWeightConfiguration* data);

// LinkAggregateWeight access functions.
extern int rd_LinkAggregateWeight(struct dubhe1000_adapter *adapter, uint64_t idx,t_LinkAggregateWeight* data);
extern int wr_LinkAggregateWeight(struct dubhe1000_adapter *adapter, uint64_t idx,t_LinkAggregateWeight* data);

// L3RoutingDefault access functions.
extern int rd_L3RoutingDefault(struct dubhe1000_adapter *adapter, uint64_t idx,t_L3RoutingDefault* data);
extern int wr_L3RoutingDefault(struct dubhe1000_adapter *adapter, uint64_t idx,t_L3RoutingDefault* data);

// TransmittedPacketsonEgressVRF access functions.
extern int rd_TransmittedPacketsonEgressVRF(struct dubhe1000_adapter *adapter, uint64_t idx,t_TransmittedPacketsonEgressVRF* data);
extern int wr_TransmittedPacketsonEgressVRF(struct dubhe1000_adapter *adapter, uint64_t idx,t_TransmittedPacketsonEgressVRF* data);

// GREPacketDecoderOptions access functions.
extern int rd_GREPacketDecoderOptions(struct dubhe1000_adapter *adapter,t_GREPacketDecoderOptions* data);
extern int wr_GREPacketDecoderOptions(struct dubhe1000_adapter *adapter,t_GREPacketDecoderOptions* data);

// IngressAdmissionControlReset access functions.
extern int rd_IngressAdmissionControlReset(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressAdmissionControlReset* data);
extern int wr_IngressAdmissionControlReset(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressAdmissionControlReset* data);

// QueueShaperEnable access functions.
extern int rd_QueueShaperEnable(struct dubhe1000_adapter *adapter,t_QueueShaperEnable* data);
extern int wr_QueueShaperEnable(struct dubhe1000_adapter *adapter,t_QueueShaperEnable* data);

// XoffFFAThreshold access functions.
extern int rd_XoffFFAThreshold(struct dubhe1000_adapter *adapter,t_XoffFFAThreshold* data);
extern int wr_XoffFFAThreshold(struct dubhe1000_adapter *adapter,t_XoffFFAThreshold* data);

// PrioShaperBucketCapacityConfiguration access functions.
extern int rd_PrioShaperBucketCapacityConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_PrioShaperBucketCapacityConfiguration* data);
extern int wr_PrioShaperBucketCapacityConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_PrioShaperBucketCapacityConfiguration* data);

// IEEE8021XandEAPOLPacketDecoderOptions access functions.
extern int rd_IEEE8021XandEAPOLPacketDecoderOptions(struct dubhe1000_adapter *adapter,t_IEEE8021XandEAPOLPacketDecoderOptions* data);
extern int wr_IEEE8021XandEAPOLPacketDecoderOptions(struct dubhe1000_adapter *adapter,t_IEEE8021XandEAPOLPacketDecoderOptions* data);

// EgressVLANTranslationTCAM access functions.
extern int rd_EgressVLANTranslationTCAM(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressVLANTranslationTCAM* data);
extern int wr_EgressVLANTranslationTCAM(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressVLANTranslationTCAM* data);

// L2LookupCollisionTable access functions.
extern int rd_L2LookupCollisionTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2LookupCollisionTable* data);
extern int wr_L2LookupCollisionTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2LookupCollisionTable* data);

// NextHopPacketModifications access functions.
extern int rd_NextHopPacketModifications(struct dubhe1000_adapter *adapter, uint64_t idx,t_NextHopPacketModifications* data);
extern int wr_NextHopPacketModifications(struct dubhe1000_adapter *adapter, uint64_t idx,t_NextHopPacketModifications* data);

// DisableCPUtagonCPUPort access functions.
extern int rd_DisableCPUtagonCPUPort(struct dubhe1000_adapter *adapter,t_DisableCPUtagonCPUPort* data);
extern int wr_DisableCPUtagonCPUPort(struct dubhe1000_adapter *adapter,t_DisableCPUtagonCPUPort* data);

// ESPDecoderDrop access functions.
extern int rd_ESPDecoderDrop(struct dubhe1000_adapter *adapter,t_ESPDecoderDrop* data);
extern int wr_ESPDecoderDrop(struct dubhe1000_adapter *adapter,t_ESPDecoderDrop* data);

// IngressEthernetTypeforVLANtag access functions.
extern int rd_IngressEthernetTypeforVLANtag(struct dubhe1000_adapter *adapter,t_IngressEthernetTypeforVLANtag* data);
extern int wr_IngressEthernetTypeforVLANtag(struct dubhe1000_adapter *adapter,t_IngressEthernetTypeforVLANtag* data);

// IPPDebugisBroadcast access functions.
extern int rd_IPPDebugisBroadcast(struct dubhe1000_adapter *adapter,t_IPPDebugisBroadcast* data);
extern int wr_IPPDebugisBroadcast(struct dubhe1000_adapter *adapter,t_IPPDebugisBroadcast* data);

// IPv4TOSFieldToEgressQueueMappingTable access functions.
extern int rd_IPv4TOSFieldToEgressQueueMappingTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_IPv4TOSFieldToEgressQueueMappingTable* data);
extern int wr_IPv4TOSFieldToEgressQueueMappingTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_IPv4TOSFieldToEgressQueueMappingTable* data);

// RARPDecoderDrop access functions.
extern int rd_RARPDecoderDrop(struct dubhe1000_adapter *adapter,t_RARPDecoderDrop* data);
extern int wr_RARPDecoderDrop(struct dubhe1000_adapter *adapter,t_RARPDecoderDrop* data);

// IngressVIDInnerVIDRangeAssignmentAnswer access functions.
extern int rd_IngressVIDInnerVIDRangeAssignmentAnswer(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressVIDInnerVIDRangeAssignmentAnswer* data);
extern int wr_IngressVIDInnerVIDRangeAssignmentAnswer(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressVIDInnerVIDRangeAssignmentAnswer* data);

// SecondTunnelExitLookupTCAMAnswer access functions.
extern int rd_SecondTunnelExitLookupTCAMAnswer(struct dubhe1000_adapter *adapter, uint64_t idx,t_SecondTunnelExitLookupTCAMAnswer* data);
extern int wr_SecondTunnelExitLookupTCAMAnswer(struct dubhe1000_adapter *adapter, uint64_t idx,t_SecondTunnelExitLookupTCAMAnswer* data);

// EgressPortConfiguration access functions.
extern int rd_EgressPortConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressPortConfiguration* data);
extern int wr_EgressPortConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressPortConfiguration* data);

// AHDecoderDrop access functions.
extern int rd_AHDecoderDrop(struct dubhe1000_adapter *adapter,t_AHDecoderDrop* data);
extern int wr_AHDecoderDrop(struct dubhe1000_adapter *adapter,t_AHDecoderDrop* data);

// EgressSpanningTreeDrop access functions.
extern int rd_EgressSpanningTreeDrop(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressSpanningTreeDrop* data);
extern int wr_EgressSpanningTreeDrop(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressSpanningTreeDrop* data);

// IngressConfigurableACL0TCAM access functions.
extern int rd_IngressConfigurableACL0TCAM(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACL0TCAM* data);
extern int wr_IngressConfigurableACL0TCAM(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACL0TCAM* data);

// NATActionTableDrop access functions.
extern int rd_NATActionTableDrop(struct dubhe1000_adapter *adapter,t_NATActionTableDrop* data);
extern int wr_NATActionTableDrop(struct dubhe1000_adapter *adapter,t_NATActionTableDrop* data);

// ERMYellowConfiguration access functions.
extern int rd_ERMYellowConfiguration(struct dubhe1000_adapter *adapter,t_ERMYellowConfiguration* data);
extern int wr_ERMYellowConfiguration(struct dubhe1000_adapter *adapter,t_ERMYellowConfiguration* data);

// L2TunnelDecoderSetup access functions.
extern int rd_L2TunnelDecoderSetup(struct dubhe1000_adapter *adapter,t_L2TunnelDecoderSetup* data);
extern int wr_L2TunnelDecoderSetup(struct dubhe1000_adapter *adapter,t_L2TunnelDecoderSetup* data);

// L2AgingStatusShadowTable access functions.
extern int rd_L2AgingStatusShadowTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2AgingStatusShadowTable* data);
extern int wr_L2AgingStatusShadowTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2AgingStatusShadowTable* data);

// IPPDebugdoL2Lookup access functions.
extern int rd_IPPDebugdoL2Lookup(struct dubhe1000_adapter *adapter,t_IPPDebugdoL2Lookup* data);
extern int wr_IPPDebugdoL2Lookup(struct dubhe1000_adapter *adapter,t_IPPDebugdoL2Lookup* data);

// TCXoffFFAThreshold access functions.
extern int rd_TCXoffFFAThreshold(struct dubhe1000_adapter *adapter, uint64_t idx,t_TCXoffFFAThreshold* data);
extern int wr_TCXoffFFAThreshold(struct dubhe1000_adapter *adapter, uint64_t idx,t_TCXoffFFAThreshold* data);

// L2MulticastStormControlEnable access functions.
extern int rd_L2MulticastStormControlEnable(struct dubhe1000_adapter *adapter,t_L2MulticastStormControlEnable* data);
extern int wr_L2MulticastStormControlEnable(struct dubhe1000_adapter *adapter,t_L2MulticastStormControlEnable* data);

// EgressConfigurableACL1TCAM access functions.
extern int rd_EgressConfigurableACL1TCAM(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressConfigurableACL1TCAM* data);
extern int wr_EgressConfigurableACL1TCAM(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressConfigurableACL1TCAM* data);

// IngressConfigurableACL0SearchMask access functions.
extern int rd_IngressConfigurableACL0SearchMask(struct dubhe1000_adapter *adapter,t_IngressConfigurableACL0SearchMask* data);
extern int wr_IngressConfigurableACL0SearchMask(struct dubhe1000_adapter *adapter,t_IngressConfigurableACL0SearchMask* data);

// PortMoveOptions access functions.
extern int rd_PortMoveOptions(struct dubhe1000_adapter *adapter,t_PortMoveOptions* data);
extern int wr_PortMoveOptions(struct dubhe1000_adapter *adapter,t_PortMoveOptions* data);

// L2AgingTable access functions.
extern int rd_L2AgingTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2AgingTable* data);
extern int wr_L2AgingTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2AgingTable* data);

// IngressRouterTable access functions.
extern int rd_IngressRouterTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressRouterTable* data);
extern int wr_IngressRouterTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressRouterTable* data);

// NextHopPacketInsertMPLSHeader access functions.
extern int rd_NextHopPacketInsertMPLSHeader(struct dubhe1000_adapter *adapter, uint64_t idx,t_NextHopPacketInsertMPLSHeader* data);
extern int wr_NextHopPacketInsertMPLSHeader(struct dubhe1000_adapter *adapter, uint64_t idx,t_NextHopPacketInsertMPLSHeader* data);

// MACInterfaceCountersForTX access functions.
extern int rd_MACInterfaceCountersForTX(struct dubhe1000_adapter *adapter, uint64_t idx,t_MACInterfaceCountersForTX* data);

// TunnelEntryInstructionTable access functions.
extern int rd_TunnelEntryInstructionTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_TunnelEntryInstructionTable* data);
extern int wr_TunnelEntryInstructionTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_TunnelEntryInstructionTable* data);

// IPUnicastRoutedCounter access functions.
extern int rd_IPUnicastRoutedCounter(struct dubhe1000_adapter *adapter, uint64_t idx,t_IPUnicastRoutedCounter* data);
extern int wr_IPUnicastRoutedCounter(struct dubhe1000_adapter *adapter, uint64_t idx,t_IPUnicastRoutedCounter* data);

// EPPDebugomEnabled access functions.
extern int rd_EPPDebugomEnabled(struct dubhe1000_adapter *adapter,t_EPPDebugomEnabled* data);
extern int wr_EPPDebugomEnabled(struct dubhe1000_adapter *adapter,t_EPPDebugomEnabled* data);

// IngressAdmissionControlCurrentSize access functions.
extern int rd_IngressAdmissionControlCurrentSize(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressAdmissionControlCurrentSize* data);

// L2DAHashLookupTable access functions.
extern int rd_L2DAHashLookupTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2DAHashLookupTable* data);
extern int wr_L2DAHashLookupTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2DAHashLookupTable* data);

// DebugCounternrVlansSetup access functions.
extern int rd_DebugCounternrVlansSetup(struct dubhe1000_adapter *adapter,t_DebugCounternrVlansSetup* data);
extern int wr_DebugCounternrVlansSetup(struct dubhe1000_adapter *adapter,t_DebugCounternrVlansSetup* data);

// EPPPacketHeadCounter access functions.
extern int rd_EPPPacketHeadCounter(struct dubhe1000_adapter *adapter,t_EPPPacketHeadCounter* data);
extern int wr_EPPPacketHeadCounter(struct dubhe1000_adapter *adapter,t_EPPPacketHeadCounter* data);

// IPPPacketTailCounter access functions.
extern int rd_IPPPacketTailCounter(struct dubhe1000_adapter *adapter,t_IPPPacketTailCounter* data);
extern int wr_IPPPacketTailCounter(struct dubhe1000_adapter *adapter,t_IPPPacketTailCounter* data);

// IPMulticastReceivedCounter access functions.
extern int rd_IPMulticastReceivedCounter(struct dubhe1000_adapter *adapter, uint64_t idx,t_IPMulticastReceivedCounter* data);
extern int wr_IPMulticastReceivedCounter(struct dubhe1000_adapter *adapter, uint64_t idx,t_IPMulticastReceivedCounter* data);

// DNSPacketDecoderOptions access functions.
extern int rd_DNSPacketDecoderOptions(struct dubhe1000_adapter *adapter,t_DNSPacketDecoderOptions* data);
extern int wr_DNSPacketDecoderOptions(struct dubhe1000_adapter *adapter,t_DNSPacketDecoderOptions* data);

// TCFFAUsed access functions.
extern int rd_TCFFAUsed(struct dubhe1000_adapter *adapter, uint64_t idx,t_TCFFAUsed* data);

// EgressNATOperation access functions.
extern int rd_EgressNATOperation(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressNATOperation* data);
extern int wr_EgressNATOperation(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressNATOperation* data);

// EPPDebugdelSpecificVlan access functions.
extern int rd_EPPDebugdelSpecificVlan(struct dubhe1000_adapter *adapter,t_EPPDebugdelSpecificVlan* data);
extern int wr_EPPDebugdelSpecificVlan(struct dubhe1000_adapter *adapter,t_EPPDebugdelSpecificVlan* data);

// IngressConfigurableACL1TCAMAnswer access functions.
extern int rd_IngressConfigurableACL1TCAMAnswer(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACL1TCAMAnswer* data);
extern int wr_IngressConfigurableACL1TCAMAnswer(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACL1TCAMAnswer* data);

// IngressConfigurableACL3TCAMAnswer access functions.
extern int rd_IngressConfigurableACL3TCAMAnswer(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACL3TCAMAnswer* data);
extern int wr_IngressConfigurableACL3TCAMAnswer(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACL3TCAMAnswer* data);

// TunnelExitLookupTCAMAnswer access functions.
extern int rd_TunnelExitLookupTCAMAnswer(struct dubhe1000_adapter *adapter, uint64_t idx,t_TunnelExitLookupTCAMAnswer* data);
extern int wr_TunnelExitLookupTCAMAnswer(struct dubhe1000_adapter *adapter, uint64_t idx,t_TunnelExitLookupTCAMAnswer* data);

// ReservedSourceMACAddressRange access functions.
extern int rd_ReservedSourceMACAddressRange(struct dubhe1000_adapter *adapter, uint64_t idx,t_ReservedSourceMACAddressRange* data);
extern int wr_ReservedSourceMACAddressRange(struct dubhe1000_adapter *adapter, uint64_t idx,t_ReservedSourceMACAddressRange* data);

// L2FloodingStormControlEnable access functions.
extern int rd_L2FloodingStormControlEnable(struct dubhe1000_adapter *adapter,t_L2FloodingStormControlEnable* data);
extern int wr_L2FloodingStormControlEnable(struct dubhe1000_adapter *adapter,t_L2FloodingStormControlEnable* data);

// QueueOffDrop access functions.
extern int rd_QueueOffDrop(struct dubhe1000_adapter *adapter, uint64_t idx,t_QueueOffDrop* data);
extern int wr_QueueOffDrop(struct dubhe1000_adapter *adapter, uint64_t idx,t_QueueOffDrop* data);

// IPPDebugnextHopPtrLpm access functions.
extern int rd_IPPDebugnextHopPtrLpm(struct dubhe1000_adapter *adapter,t_IPPDebugnextHopPtrLpm* data);
extern int wr_IPPDebugnextHopPtrLpm(struct dubhe1000_adapter *adapter,t_IPPDebugnextHopPtrLpm* data);

// L4IEEE1588DecoderDrop access functions.
extern int rd_L4IEEE1588DecoderDrop(struct dubhe1000_adapter *adapter,t_L4IEEE1588DecoderDrop* data);
extern int wr_L4IEEE1588DecoderDrop(struct dubhe1000_adapter *adapter,t_L4IEEE1588DecoderDrop* data);

// TunnelEntryHeaderData access functions.
extern int rd_TunnelEntryHeaderData(struct dubhe1000_adapter *adapter, uint64_t idx,t_TunnelEntryHeaderData* data);
extern int wr_TunnelEntryHeaderData(struct dubhe1000_adapter *adapter, uint64_t idx,t_TunnelEntryHeaderData* data);

// QueueShaperBucketThresholdConfiguration access functions.
extern int rd_QueueShaperBucketThresholdConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_QueueShaperBucketThresholdConfiguration* data);
extern int wr_QueueShaperBucketThresholdConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_QueueShaperBucketThresholdConfiguration* data);

// IngressConfigurableACL0TCAMAnswer access functions.
extern int rd_IngressConfigurableACL0TCAMAnswer(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACL0TCAMAnswer* data);
extern int wr_IngressConfigurableACL0TCAMAnswer(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACL0TCAMAnswer* data);

// EgressConfigurableACLMatchCounter access functions.
extern int rd_EgressConfigurableACLMatchCounter(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressConfigurableACLMatchCounter* data);
extern int wr_EgressConfigurableACLMatchCounter(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressConfigurableACLMatchCounter* data);

// AgingDataFIFOHighWatermarkLevel access functions.
extern int rd_AgingDataFIFOHighWatermarkLevel(struct dubhe1000_adapter *adapter,t_AgingDataFIFOHighWatermarkLevel* data);
extern int wr_AgingDataFIFOHighWatermarkLevel(struct dubhe1000_adapter *adapter,t_AgingDataFIFOHighWatermarkLevel* data);

// LearningOverflow access functions.
extern int rd_LearningOverflow(struct dubhe1000_adapter *adapter,t_LearningOverflow* data);
extern int wr_LearningOverflow(struct dubhe1000_adapter *adapter,t_LearningOverflow* data);

// EgressMPLSDecodingOptions access functions.
extern int rd_EgressMPLSDecodingOptions(struct dubhe1000_adapter *adapter,t_EgressMPLSDecodingOptions* data);
extern int wr_EgressMPLSDecodingOptions(struct dubhe1000_adapter *adapter,t_EgressMPLSDecodingOptions* data);

// IngressVIDOuterVIDRangeSearchData access functions.
extern int rd_IngressVIDOuterVIDRangeSearchData(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressVIDOuterVIDRangeSearchData* data);
extern int wr_IngressVIDOuterVIDRangeSearchData(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressVIDOuterVIDRangeSearchData* data);

// EgressVLANTranslationTCAMAnswer access functions.
extern int rd_EgressVLANTranslationTCAMAnswer(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressVLANTranslationTCAMAnswer* data);
extern int wr_EgressVLANTranslationTCAMAnswer(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressVLANTranslationTCAMAnswer* data);

// SCTPDecoderDrop access functions.
extern int rd_SCTPDecoderDrop(struct dubhe1000_adapter *adapter,t_SCTPDecoderDrop* data);
extern int wr_SCTPDecoderDrop(struct dubhe1000_adapter *adapter,t_SCTPDecoderDrop* data);

// DebugCounterl2DaTcamHitsAndCastSetup access functions.
extern int rd_DebugCounterl2DaTcamHitsAndCastSetup(struct dubhe1000_adapter *adapter,t_DebugCounterl2DaTcamHitsAndCastSetup* data);
extern int wr_DebugCounterl2DaTcamHitsAndCastSetup(struct dubhe1000_adapter *adapter,t_DebugCounterl2DaTcamHitsAndCastSetup* data);

// IPPDebugl2DaHash access functions.
extern int rd_IPPDebugl2DaHash(struct dubhe1000_adapter *adapter,t_IPPDebugl2DaHash* data);
extern int wr_IPPDebugl2DaHash(struct dubhe1000_adapter *adapter,t_IPPDebugl2DaHash* data);

// L2ActionTableSourcePort access functions.
extern int rd_L2ActionTableSourcePort(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2ActionTableSourcePort* data);
extern int wr_L2ActionTableSourcePort(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2ActionTableSourcePort* data);

// FloodingActionSendtoPort access functions.
extern int rd_FloodingActionSendtoPort(struct dubhe1000_adapter *adapter, uint64_t idx,t_FloodingActionSendtoPort* data);
extern int wr_FloodingActionSendtoPort(struct dubhe1000_adapter *adapter, uint64_t idx,t_FloodingActionSendtoPort* data);

// EgressMPLSTTLTable access functions.
extern int rd_EgressMPLSTTLTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressMPLSTTLTable* data);
extern int wr_EgressMPLSTTLTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressMPLSTTLTable* data);

// EgressPortNATState access functions.
extern int rd_EgressPortNATState(struct dubhe1000_adapter *adapter,t_EgressPortNATState* data);
extern int wr_EgressPortNATState(struct dubhe1000_adapter *adapter,t_EgressPortNATState* data);

// DebugCounterl2DaHashKeySetup access functions.
extern int rd_DebugCounterl2DaHashKeySetup(struct dubhe1000_adapter *adapter,t_DebugCounterl2DaHashKeySetup* data);
extern int wr_DebugCounterl2DaHashKeySetup(struct dubhe1000_adapter *adapter,t_DebugCounterl2DaHashKeySetup* data);

// IngressDropOptions access functions.
extern int rd_IngressDropOptions(struct dubhe1000_adapter *adapter,t_IngressDropOptions* data);
extern int wr_IngressDropOptions(struct dubhe1000_adapter *adapter,t_IngressDropOptions* data);

// MBSCDrop access functions.
extern int rd_MBSCDrop(struct dubhe1000_adapter *adapter, uint64_t idx,t_MBSCDrop* data);
extern int wr_MBSCDrop(struct dubhe1000_adapter *adapter, uint64_t idx,t_MBSCDrop* data);

// VLANMemberDrop access functions.
extern int rd_VLANMemberDrop(struct dubhe1000_adapter *adapter,t_VLANMemberDrop* data);
extern int wr_VLANMemberDrop(struct dubhe1000_adapter *adapter,t_VLANMemberDrop* data);

// PSPacketTailCounter access functions.
extern int rd_PSPacketTailCounter(struct dubhe1000_adapter *adapter,t_PSPacketTailCounter* data);
extern int wr_PSPacketTailCounter(struct dubhe1000_adapter *adapter,t_PSPacketTailCounter* data);

// EgressResourceManagerDrop access functions.
extern int rd_EgressResourceManagerDrop(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressResourceManagerDrop* data);
extern int wr_EgressResourceManagerDrop(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressResourceManagerDrop* data);

// MACRXBrokenPackets access functions.
extern int rd_MACRXBrokenPackets(struct dubhe1000_adapter *adapter, uint64_t idx,t_MACRXBrokenPackets* data);

// IngressVIDOuterVIDRangeAssignmentAnswer access functions.
extern int rd_IngressVIDOuterVIDRangeAssignmentAnswer(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressVIDOuterVIDRangeAssignmentAnswer* data);
extern int wr_IngressVIDOuterVIDRangeAssignmentAnswer(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressVIDOuterVIDRangeAssignmentAnswer* data);

// IngressVIDMACRangeSearchData access functions.
extern int rd_IngressVIDMACRangeSearchData(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressVIDMACRangeSearchData* data);
extern int wr_IngressVIDMACRangeSearchData(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressVIDMACRangeSearchData* data);

// L2ActionTableEgressPortState access functions.
extern int rd_L2ActionTableEgressPortState(struct dubhe1000_adapter *adapter,t_L2ActionTableEgressPortState* data);
extern int wr_L2ActionTableEgressPortState(struct dubhe1000_adapter *adapter,t_L2ActionTableEgressPortState* data);

// IngressNATHitStatus access functions.
extern int rd_IngressNATHitStatus(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressNATHitStatus* data);
extern int wr_IngressNATHitStatus(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressNATHitStatus* data);

// ColorRemapFromIngressAdmissionControl access functions.
extern int rd_ColorRemapFromIngressAdmissionControl(struct dubhe1000_adapter *adapter, uint64_t idx,t_ColorRemapFromIngressAdmissionControl* data);
extern int wr_ColorRemapFromIngressAdmissionControl(struct dubhe1000_adapter *adapter, uint64_t idx,t_ColorRemapFromIngressAdmissionControl* data);

// IPPDebugdebugMatchIPP0 access functions.
extern int rd_IPPDebugdebugMatchIPP0(struct dubhe1000_adapter *adapter,t_IPPDebugdebugMatchIPP0* data);
extern int wr_IPPDebugdebugMatchIPP0(struct dubhe1000_adapter *adapter,t_IPPDebugdebugMatchIPP0* data);

// HairpinEnable access functions.
extern int rd_HairpinEnable(struct dubhe1000_adapter *adapter, uint64_t idx,t_HairpinEnable* data);
extern int wr_HairpinEnable(struct dubhe1000_adapter *adapter, uint64_t idx,t_HairpinEnable* data);

// IKEPacketDecoderOptions access functions.
extern int rd_IKEPacketDecoderOptions(struct dubhe1000_adapter *adapter,t_IKEPacketDecoderOptions* data);
extern int wr_IKEPacketDecoderOptions(struct dubhe1000_adapter *adapter,t_IKEPacketDecoderOptions* data);

// MPLSEXPFieldToPacketColorMappingTable access functions.
extern int rd_MPLSEXPFieldToPacketColorMappingTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_MPLSEXPFieldToPacketColorMappingTable* data);
extern int wr_MPLSEXPFieldToPacketColorMappingTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_MPLSEXPFieldToPacketColorMappingTable* data);

// IPMulticastACLDropCounter access functions.
extern int rd_IPMulticastACLDropCounter(struct dubhe1000_adapter *adapter, uint64_t idx,t_IPMulticastACLDropCounter* data);
extern int wr_IPMulticastACLDropCounter(struct dubhe1000_adapter *adapter, uint64_t idx,t_IPMulticastACLDropCounter* data);

// QueueShaperBucketCapacityConfiguration access functions.
extern int rd_QueueShaperBucketCapacityConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_QueueShaperBucketCapacityConfiguration* data);
extern int wr_QueueShaperBucketCapacityConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_QueueShaperBucketCapacityConfiguration* data);

// IngressVIDInnerVIDRangeSearchData access functions.
extern int rd_IngressVIDInnerVIDRangeSearchData(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressVIDInnerVIDRangeSearchData* data);
extern int wr_IngressVIDInnerVIDRangeSearchData(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressVIDInnerVIDRangeSearchData* data);

// NextHopHitStatus access functions.
extern int rd_NextHopHitStatus(struct dubhe1000_adapter *adapter, uint64_t idx,t_NextHopHitStatus* data);
extern int wr_NextHopHitStatus(struct dubhe1000_adapter *adapter, uint64_t idx,t_NextHopHitStatus* data);

// EgressQueueToMPLSEXPMappingTable access functions.
extern int rd_EgressQueueToMPLSEXPMappingTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressQueueToMPLSEXPMappingTable* data);
extern int wr_EgressQueueToMPLSEXPMappingTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressQueueToMPLSEXPMappingTable* data);

// FFAUsedPFC access functions.
extern int rd_FFAUsedPFC(struct dubhe1000_adapter *adapter,t_FFAUsedPFC* data);

// IPPDebugrouted access functions.
extern int rd_IPPDebugrouted(struct dubhe1000_adapter *adapter,t_IPPDebugrouted* data);
extern int wr_IPPDebugrouted(struct dubhe1000_adapter *adapter,t_IPPDebugrouted* data);

// EgressRouterTable access functions.
extern int rd_EgressRouterTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressRouterTable* data);
extern int wr_EgressRouterTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressRouterTable* data);

// LearningAndAgingEnable access functions.
extern int rd_LearningAndAgingEnable(struct dubhe1000_adapter *adapter,t_LearningAndAgingEnable* data);
extern int wr_LearningAndAgingEnable(struct dubhe1000_adapter *adapter,t_LearningAndAgingEnable* data);

// MACRXLongPacketDrop access functions.
extern int rd_MACRXLongPacketDrop(struct dubhe1000_adapter *adapter, uint64_t idx,t_MACRXLongPacketDrop* data);

// PortXoffFFAThreshold access functions.
extern int rd_PortXoffFFAThreshold(struct dubhe1000_adapter *adapter, uint64_t idx,t_PortXoffFFAThreshold* data);
extern int wr_PortXoffFFAThreshold(struct dubhe1000_adapter *adapter, uint64_t idx,t_PortXoffFFAThreshold* data);

// IPPDebugsrcPort access functions.
extern int rd_IPPDebugsrcPort(struct dubhe1000_adapter *adapter,t_IPPDebugsrcPort* data);
extern int wr_IPPDebugsrcPort(struct dubhe1000_adapter *adapter,t_IPPDebugsrcPort* data);

// TCXonFFAThreshold access functions.
extern int rd_TCXonFFAThreshold(struct dubhe1000_adapter *adapter, uint64_t idx,t_TCXonFFAThreshold* data);
extern int wr_TCXonFFAThreshold(struct dubhe1000_adapter *adapter, uint64_t idx,t_TCXonFFAThreshold* data);

// SCTPPacketDecoderOptions access functions.
extern int rd_SCTPPacketDecoderOptions(struct dubhe1000_adapter *adapter,t_SCTPPacketDecoderOptions* data);
extern int wr_SCTPPacketDecoderOptions(struct dubhe1000_adapter *adapter,t_SCTPPacketDecoderOptions* data);

// LinkAggregationToPhysicalPortsMembers access functions.
extern int rd_LinkAggregationToPhysicalPortsMembers(struct dubhe1000_adapter *adapter, uint64_t idx,t_LinkAggregationToPhysicalPortsMembers* data);
extern int wr_LinkAggregationToPhysicalPortsMembers(struct dubhe1000_adapter *adapter, uint64_t idx,t_LinkAggregationToPhysicalPortsMembers* data);

// L3TunnelEntryInstructionTable access functions.
extern int rd_L3TunnelEntryInstructionTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_L3TunnelEntryInstructionTable* data);
extern int wr_L3TunnelEntryInstructionTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_L3TunnelEntryInstructionTable* data);

// SecondTunnelExitLookupTCAM access functions.
extern int rd_SecondTunnelExitLookupTCAM(struct dubhe1000_adapter *adapter, uint64_t idx,t_SecondTunnelExitLookupTCAM* data);
extern int wr_SecondTunnelExitLookupTCAM(struct dubhe1000_adapter *adapter, uint64_t idx,t_SecondTunnelExitLookupTCAM* data);

// Scratch access functions.
extern int rd_Scratch(struct dubhe1000_adapter *adapter,t_Scratch* data);
extern int wr_Scratch(struct dubhe1000_adapter *adapter,t_Scratch* data);

// MapQueuetoPriority access functions.
extern int rd_MapQueuetoPriority(struct dubhe1000_adapter *adapter, uint64_t idx,t_MapQueuetoPriority* data);
extern int wr_MapQueuetoPriority(struct dubhe1000_adapter *adapter, uint64_t idx,t_MapQueuetoPriority* data);

// NextHopDAMAC access functions.
extern int rd_NextHopDAMAC(struct dubhe1000_adapter *adapter, uint64_t idx,t_NextHopDAMAC* data);
extern int wr_NextHopDAMAC(struct dubhe1000_adapter *adapter, uint64_t idx,t_NextHopDAMAC* data);

// DNSDecoderDrop access functions.
extern int rd_DNSDecoderDrop(struct dubhe1000_adapter *adapter,t_DNSDecoderDrop* data);
extern int wr_DNSDecoderDrop(struct dubhe1000_adapter *adapter,t_DNSDecoderDrop* data);

// ReceivedPacketsonIngressVRF access functions.
extern int rd_ReceivedPacketsonIngressVRF(struct dubhe1000_adapter *adapter, uint64_t idx,t_ReceivedPacketsonIngressVRF* data);
extern int wr_ReceivedPacketsonIngressVRF(struct dubhe1000_adapter *adapter, uint64_t idx,t_ReceivedPacketsonIngressVRF* data);

// EPPDebugreQueue access functions.
extern int rd_EPPDebugreQueue(struct dubhe1000_adapter *adapter,t_EPPDebugreQueue* data);
extern int wr_EPPDebugreQueue(struct dubhe1000_adapter *adapter,t_EPPDebugreQueue* data);

// L2ReservedMulticastAddressDrop access functions.
extern int rd_L2ReservedMulticastAddressDrop(struct dubhe1000_adapter *adapter,t_L2ReservedMulticastAddressDrop* data);
extern int wr_L2ReservedMulticastAddressDrop(struct dubhe1000_adapter *adapter,t_L2ReservedMulticastAddressDrop* data);

// IngressConfigurableACL1PreLookup access functions.
extern int rd_IngressConfigurableACL1PreLookup(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACL1PreLookup* data);
extern int wr_IngressConfigurableACL1PreLookup(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACL1PreLookup* data);

// EPPDebugisPPPoE access functions.
extern int rd_EPPDebugisPPPoE(struct dubhe1000_adapter *adapter,t_EPPDebugisPPPoE* data);
extern int wr_EPPDebugisPPPoE(struct dubhe1000_adapter *adapter,t_EPPDebugisPPPoE* data);

// MACRXMaximumPacketLength access functions.
extern int rd_MACRXMaximumPacketLength(struct dubhe1000_adapter *adapter, uint64_t idx,t_MACRXMaximumPacketLength* data);
extern int wr_MACRXMaximumPacketLength(struct dubhe1000_adapter *adapter, uint64_t idx,t_MACRXMaximumPacketLength* data);

// EgressPortFilteringDrop access functions.
extern int rd_EgressPortFilteringDrop(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressPortFilteringDrop* data);
extern int wr_EgressPortFilteringDrop(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressPortFilteringDrop* data);

// IngressConfigurableACL0RulesSetup access functions.
extern int rd_IngressConfigurableACL0RulesSetup(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACL0RulesSetup* data);
extern int wr_IngressConfigurableACL0RulesSetup(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACL0RulesSetup* data);

// XonFFAThreshold access functions.
extern int rd_XonFFAThreshold(struct dubhe1000_adapter *adapter,t_XonFFAThreshold* data);
extern int wr_XonFFAThreshold(struct dubhe1000_adapter *adapter,t_XonFFAThreshold* data);

// PortShaperEnable access functions.
extern int rd_PortShaperEnable(struct dubhe1000_adapter *adapter,t_PortShaperEnable* data);
extern int wr_PortShaperEnable(struct dubhe1000_adapter *adapter,t_PortShaperEnable* data);

// MACRXShortPacketDrop access functions.
extern int rd_MACRXShortPacketDrop(struct dubhe1000_adapter *adapter, uint64_t idx,t_MACRXShortPacketDrop* data);

// LearningPacketDrop access functions.
extern int rd_LearningPacketDrop(struct dubhe1000_adapter *adapter,t_LearningPacketDrop* data);
extern int wr_LearningPacketDrop(struct dubhe1000_adapter *adapter,t_LearningPacketDrop* data);

// BOOTPandDHCPPacketDecoderOptions access functions.
extern int rd_BOOTPandDHCPPacketDecoderOptions(struct dubhe1000_adapter *adapter,t_BOOTPandDHCPPacketDecoderOptions* data);
extern int wr_BOOTPandDHCPPacketDecoderOptions(struct dubhe1000_adapter *adapter,t_BOOTPandDHCPPacketDecoderOptions* data);

// ExpiredTTLDrop access functions.
extern int rd_ExpiredTTLDrop(struct dubhe1000_adapter *adapter,t_ExpiredTTLDrop* data);
extern int wr_ExpiredTTLDrop(struct dubhe1000_adapter *adapter,t_ExpiredTTLDrop* data);

// IPPDebugvlanVidOp access functions.
extern int rd_IPPDebugvlanVidOp(struct dubhe1000_adapter *adapter,t_IPPDebugvlanVidOp* data);
extern int wr_IPPDebugvlanVidOp(struct dubhe1000_adapter *adapter,t_IPPDebugvlanVidOp* data);

// LearningDataFIFO access functions.
extern int rd_LearningDataFIFO(struct dubhe1000_adapter *adapter,t_LearningDataFIFO* data);

// ERMRedConfiguration access functions.
extern int rd_ERMRedConfiguration(struct dubhe1000_adapter *adapter,t_ERMRedConfiguration* data);
extern int wr_ERMRedConfiguration(struct dubhe1000_adapter *adapter,t_ERMRedConfiguration* data);

// NATAddEgressPortforNATCalculation access functions.
extern int rd_NATAddEgressPortforNATCalculation(struct dubhe1000_adapter *adapter,t_NATAddEgressPortforNATCalculation* data);
extern int wr_NATAddEgressPortforNATCalculation(struct dubhe1000_adapter *adapter,t_NATAddEgressPortforNATCalculation* data);

// CoreTickConfiguration access functions.
extern int rd_CoreTickConfiguration(struct dubhe1000_adapter *adapter,t_CoreTickConfiguration* data);
extern int wr_CoreTickConfiguration(struct dubhe1000_adapter *adapter,t_CoreTickConfiguration* data);

// IngressConfigurableACL2PreLookup access functions.
extern int rd_IngressConfigurableACL2PreLookup(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACL2PreLookup* data);
extern int wr_IngressConfigurableACL2PreLookup(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACL2PreLookup* data);

// SourcePortDefaultACLActionDrop access functions.
extern int rd_SourcePortDefaultACLActionDrop(struct dubhe1000_adapter *adapter,t_SourcePortDefaultACLActionDrop* data);
extern int wr_SourcePortDefaultACLActionDrop(struct dubhe1000_adapter *adapter,t_SourcePortDefaultACLActionDrop* data);

// PortTCTailDropTotalThreshold access functions.
extern int rd_PortTCTailDropTotalThreshold(struct dubhe1000_adapter *adapter, uint64_t idx,t_PortTCTailDropTotalThreshold* data);
extern int wr_PortTCTailDropTotalThreshold(struct dubhe1000_adapter *adapter, uint64_t idx,t_PortTCTailDropTotalThreshold* data);

// IngressConfigurableACL0Selection access functions.
extern int rd_IngressConfigurableACL0Selection(struct dubhe1000_adapter *adapter,t_IngressConfigurableACL0Selection* data);
extern int wr_IngressConfigurableACL0Selection(struct dubhe1000_adapter *adapter,t_IngressConfigurableACL0Selection* data);

// VLANPCPToQueueMappingTable access functions.
extern int rd_VLANPCPToQueueMappingTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_VLANPCPToQueueMappingTable* data);
extern int wr_VLANPCPToQueueMappingTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_VLANPCPToQueueMappingTable* data);

// L2MulticastStormControlBucketCapacityConfiguration access functions.
extern int rd_L2MulticastStormControlBucketCapacityConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2MulticastStormControlBucketCapacityConfiguration* data);
extern int wr_L2MulticastStormControlBucketCapacityConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2MulticastStormControlBucketCapacityConfiguration* data);

// TailDropFFAThreshold access functions.
extern int rd_TailDropFFAThreshold(struct dubhe1000_adapter *adapter,t_TailDropFFAThreshold* data);
extern int wr_TailDropFFAThreshold(struct dubhe1000_adapter *adapter,t_TailDropFFAThreshold* data);

// ARPPacketDecoderOptions access functions.
extern int rd_ARPPacketDecoderOptions(struct dubhe1000_adapter *adapter,t_ARPPacketDecoderOptions* data);
extern int wr_ARPPacketDecoderOptions(struct dubhe1000_adapter *adapter,t_ARPPacketDecoderOptions* data);

// DebugCounterreQueuePortIdSetup access functions.
extern int rd_DebugCounterreQueuePortIdSetup(struct dubhe1000_adapter *adapter,t_DebugCounterreQueuePortIdSetup* data);
extern int wr_DebugCounterreQueuePortIdSetup(struct dubhe1000_adapter *adapter,t_DebugCounterreQueuePortIdSetup* data);

// L2MulticastTable access functions.
extern int rd_L2MulticastTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2MulticastTable* data);
extern int wr_L2MulticastTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2MulticastTable* data);

// IPPDebugnrVlans access functions.
extern int rd_IPPDebugnrVlans(struct dubhe1000_adapter *adapter,t_IPPDebugnrVlans* data);
extern int wr_IPPDebugnrVlans(struct dubhe1000_adapter *adapter,t_IPPDebugnrVlans* data);

// ReservedDestinationMACAddressRange access functions.
extern int rd_ReservedDestinationMACAddressRange(struct dubhe1000_adapter *adapter, uint64_t idx,t_ReservedDestinationMACAddressRange* data);
extern int wr_ReservedDestinationMACAddressRange(struct dubhe1000_adapter *adapter, uint64_t idx,t_ReservedDestinationMACAddressRange* data);

// TCTailDropFFAThreshold access functions.
extern int rd_TCTailDropFFAThreshold(struct dubhe1000_adapter *adapter, uint64_t idx,t_TCTailDropFFAThreshold* data);
extern int wr_TCTailDropFFAThreshold(struct dubhe1000_adapter *adapter, uint64_t idx,t_TCTailDropFFAThreshold* data);

// IngressConfigurableACL1SmallTable access functions.
extern int rd_IngressConfigurableACL1SmallTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACL1SmallTable* data);
extern int wr_IngressConfigurableACL1SmallTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACL1SmallTable* data);

// IngressVIDEthernetTypeRangeSearchData access functions.
extern int rd_IngressVIDEthernetTypeRangeSearchData(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressVIDEthernetTypeRangeSearchData* data);
extern int wr_IngressVIDEthernetTypeRangeSearchData(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressVIDEthernetTypeRangeSearchData* data);

// LearningDataFIFOHighWatermarkLevel access functions.
extern int rd_LearningDataFIFOHighWatermarkLevel(struct dubhe1000_adapter *adapter,t_LearningDataFIFOHighWatermarkLevel* data);
extern int wr_LearningDataFIFOHighWatermarkLevel(struct dubhe1000_adapter *adapter,t_LearningDataFIFOHighWatermarkLevel* data);

// L2FloodingStormControlRateConfiguration access functions.
extern int rd_L2FloodingStormControlRateConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2FloodingStormControlRateConfiguration* data);
extern int wr_L2FloodingStormControlRateConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2FloodingStormControlRateConfiguration* data);

// EgressSpanningTreeState access functions.
extern int rd_EgressSpanningTreeState(struct dubhe1000_adapter *adapter,t_EgressSpanningTreeState* data);
extern int wr_EgressSpanningTreeState(struct dubhe1000_adapter *adapter,t_EgressSpanningTreeState* data);

// TimetoAge access functions.
extern int rd_TimetoAge(struct dubhe1000_adapter *adapter,t_TimetoAge* data);
extern int wr_TimetoAge(struct dubhe1000_adapter *adapter,t_TimetoAge* data);

// EPPDebugupdateTosExp access functions.
extern int rd_EPPDebugupdateTosExp(struct dubhe1000_adapter *adapter,t_EPPDebugupdateTosExp* data);
extern int wr_EPPDebugupdateTosExp(struct dubhe1000_adapter *adapter,t_EPPDebugupdateTosExp* data);

// IngressEgressAdmissionControlDrop access functions.
extern int rd_IngressEgressAdmissionControlDrop(struct dubhe1000_adapter *adapter,t_IngressEgressAdmissionControlDrop* data);
extern int wr_IngressEgressAdmissionControlDrop(struct dubhe1000_adapter *adapter,t_IngressEgressAdmissionControlDrop* data);

// IngressConfigurableACL1LargeTable access functions.
extern int rd_IngressConfigurableACL1LargeTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACL1LargeTable* data);
extern int wr_IngressConfigurableACL1LargeTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACL1LargeTable* data);

// PrioShaperRateConfiguration access functions.
extern int rd_PrioShaperRateConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_PrioShaperRateConfiguration* data);
extern int wr_PrioShaperRateConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_PrioShaperRateConfiguration* data);

// IPPDebugnextHopPtrLpmHit access functions.
extern int rd_IPPDebugnextHopPtrLpmHit(struct dubhe1000_adapter *adapter,t_IPPDebugnextHopPtrLpmHit* data);
extern int wr_IPPDebugnextHopPtrLpmHit(struct dubhe1000_adapter *adapter,t_IPPDebugnextHopPtrLpmHit* data);

// GREDecoderDrop access functions.
extern int rd_GREDecoderDrop(struct dubhe1000_adapter *adapter,t_GREDecoderDrop* data);
extern int wr_GREDecoderDrop(struct dubhe1000_adapter *adapter,t_GREDecoderDrop* data);

// SoftwareAgingStartLatch access functions.
extern int wr_SoftwareAgingStartLatch(struct dubhe1000_adapter *adapter,t_SoftwareAgingStartLatch* data);

// IPMulticastRoutedCounter access functions.
extern int rd_IPMulticastRoutedCounter(struct dubhe1000_adapter *adapter, uint64_t idx,t_IPMulticastRoutedCounter* data);
extern int wr_IPMulticastRoutedCounter(struct dubhe1000_adapter *adapter, uint64_t idx,t_IPMulticastRoutedCounter* data);

// PFCIncCountersforingressports0to5 access functions.
extern int rd_PFCIncCountersforingressports0to5(struct dubhe1000_adapter *adapter, uint64_t idx,t_PFCIncCountersforingressports0to5* data);

// DebugCounterfromPortSetup access functions.
extern int rd_DebugCounterfromPortSetup(struct dubhe1000_adapter *adapter,t_DebugCounterfromPortSetup* data);
extern int wr_DebugCounterfromPortSetup(struct dubhe1000_adapter *adapter,t_DebugCounterfromPortSetup* data);

// EPPDebugisIPv4 access functions.
extern int rd_EPPDebugisIPv4(struct dubhe1000_adapter *adapter,t_EPPDebugisIPv4* data);
extern int wr_EPPDebugisIPv4(struct dubhe1000_adapter *adapter,t_EPPDebugisIPv4* data);

// TunnelExitTooSmallPacketModificationDrop access functions.
extern int rd_TunnelExitTooSmallPacketModificationDrop(struct dubhe1000_adapter *adapter,t_TunnelExitTooSmallPacketModificationDrop* data);
extern int wr_TunnelExitTooSmallPacketModificationDrop(struct dubhe1000_adapter *adapter,t_TunnelExitTooSmallPacketModificationDrop* data);

// DebugCounterdstPortmaskSetup access functions.
extern int rd_DebugCounterdstPortmaskSetup(struct dubhe1000_adapter *adapter,t_DebugCounterdstPortmaskSetup* data);
extern int wr_DebugCounterdstPortmaskSetup(struct dubhe1000_adapter *adapter,t_DebugCounterdstPortmaskSetup* data);

// HitUpdateDataFIFO access functions.
extern int rd_HitUpdateDataFIFO(struct dubhe1000_adapter *adapter,t_HitUpdateDataFIFO* data);

// DebugCounternextHopPtrHashSetup access functions.
extern int rd_DebugCounternextHopPtrHashSetup(struct dubhe1000_adapter *adapter,t_DebugCounternextHopPtrHashSetup* data);
extern int wr_DebugCounternextHopPtrHashSetup(struct dubhe1000_adapter *adapter,t_DebugCounternextHopPtrHashSetup* data);

// SendtoCPU access functions.
extern int rd_SendtoCPU(struct dubhe1000_adapter *adapter,t_SendtoCPU* data);
extern int wr_SendtoCPU(struct dubhe1000_adapter *adapter,t_SendtoCPU* data);

// L2FloodingStormControlBucketCapacityConfiguration access functions.
extern int rd_L2FloodingStormControlBucketCapacityConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2FloodingStormControlBucketCapacityConfiguration* data);
extern int wr_L2FloodingStormControlBucketCapacityConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2FloodingStormControlBucketCapacityConfiguration* data);

// DebugCountersrcPortSetup access functions.
extern int rd_DebugCountersrcPortSetup(struct dubhe1000_adapter *adapter,t_DebugCountersrcPortSetup* data);
extern int wr_DebugCountersrcPortSetup(struct dubhe1000_adapter *adapter,t_DebugCountersrcPortSetup* data);

// IPv6ClassofServiceFieldToPacketColorMappingTable access functions.
extern int rd_IPv6ClassofServiceFieldToPacketColorMappingTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_IPv6ClassofServiceFieldToPacketColorMappingTable* data);
extern int wr_IPv6ClassofServiceFieldToPacketColorMappingTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_IPv6ClassofServiceFieldToPacketColorMappingTable* data);

// MinimumAllowedVLANDrop access functions.
extern int rd_MinimumAllowedVLANDrop(struct dubhe1000_adapter *adapter,t_MinimumAllowedVLANDrop* data);
extern int wr_MinimumAllowedVLANDrop(struct dubhe1000_adapter *adapter,t_MinimumAllowedVLANDrop* data);

// L2DestinationTable access functions.
extern int rd_L2DestinationTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2DestinationTable* data);
extern int wr_L2DestinationTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2DestinationTable* data);

// IPPDebugdropPktAfterL3Decode access functions.
extern int rd_IPPDebugdropPktAfterL3Decode(struct dubhe1000_adapter *adapter,t_IPPDebugdropPktAfterL3Decode* data);
extern int wr_IPPDebugdropPktAfterL3Decode(struct dubhe1000_adapter *adapter,t_IPPDebugdropPktAfterL3Decode* data);

// EgressMultipleSpanningTreeState access functions.
extern int rd_EgressMultipleSpanningTreeState(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressMultipleSpanningTreeState* data);
extern int wr_EgressMultipleSpanningTreeState(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressMultipleSpanningTreeState* data);

// EPPDebugdoPktUpdate access functions.
extern int rd_EPPDebugdoPktUpdate(struct dubhe1000_adapter *adapter,t_EPPDebugdoPktUpdate* data);
extern int wr_EPPDebugdoPktUpdate(struct dubhe1000_adapter *adapter,t_EPPDebugdoPktUpdate* data);

// TunnelExitMissActionDrop access functions.
extern int rd_TunnelExitMissActionDrop(struct dubhe1000_adapter *adapter,t_TunnelExitMissActionDrop* data);
extern int wr_TunnelExitMissActionDrop(struct dubhe1000_adapter *adapter,t_TunnelExitMissActionDrop* data);

// PortTCXonTotalThreshold access functions.
extern int rd_PortTCXonTotalThreshold(struct dubhe1000_adapter *adapter, uint64_t idx,t_PortTCXonTotalThreshold* data);
extern int wr_PortTCXonTotalThreshold(struct dubhe1000_adapter *adapter, uint64_t idx,t_PortTCXonTotalThreshold* data);

// DebugCounterdebugMatchIPP0Setup access functions.
extern int rd_DebugCounterdebugMatchIPP0Setup(struct dubhe1000_adapter *adapter,t_DebugCounterdebugMatchIPP0Setup* data);
extern int wr_DebugCounterdebugMatchIPP0Setup(struct dubhe1000_adapter *adapter,t_DebugCounterdebugMatchIPP0Setup* data);

// HardwareLearningCounter access functions.
extern int rd_HardwareLearningCounter(struct dubhe1000_adapter *adapter, uint64_t idx,t_HardwareLearningCounter* data);
extern int wr_HardwareLearningCounter(struct dubhe1000_adapter *adapter, uint64_t idx,t_HardwareLearningCounter* data);

// L2ReservedMulticastAddressAction access functions.
extern int rd_L2ReservedMulticastAddressAction(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2ReservedMulticastAddressAction* data);
extern int wr_L2ReservedMulticastAddressAction(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2ReservedMulticastAddressAction* data);

// EPPDebugreQueuePkt access functions.
extern int rd_EPPDebugreQueuePkt(struct dubhe1000_adapter *adapter,t_EPPDebugreQueuePkt* data);
extern int wr_EPPDebugreQueuePkt(struct dubhe1000_adapter *adapter,t_EPPDebugreQueuePkt* data);

// SoftwareAgingEnable access functions.
extern int rd_SoftwareAgingEnable(struct dubhe1000_adapter *adapter,t_SoftwareAgingEnable* data);
extern int wr_SoftwareAgingEnable(struct dubhe1000_adapter *adapter,t_SoftwareAgingEnable* data);

// L2ReservedMulticastAddressBase access functions.
extern int rd_L2ReservedMulticastAddressBase(struct dubhe1000_adapter *adapter,t_L2ReservedMulticastAddressBase* data);
extern int wr_L2ReservedMulticastAddressBase(struct dubhe1000_adapter *adapter,t_L2ReservedMulticastAddressBase* data);

// IPPDebugspVidOp access functions.
extern int rd_IPPDebugspVidOp(struct dubhe1000_adapter *adapter,t_IPPDebugspVidOp* data);
extern int wr_IPPDebugspVidOp(struct dubhe1000_adapter *adapter,t_IPPDebugspVidOp* data);

// EgressConfigurableACL0Selection access functions.
extern int rd_EgressConfigurableACL0Selection(struct dubhe1000_adapter *adapter,t_EgressConfigurableACL0Selection* data);
extern int wr_EgressConfigurableACL0Selection(struct dubhe1000_adapter *adapter,t_EgressConfigurableACL0Selection* data);

// L2BroadcastStormControlBucketThresholdConfiguration access functions.
extern int rd_L2BroadcastStormControlBucketThresholdConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2BroadcastStormControlBucketThresholdConfiguration* data);
extern int wr_L2BroadcastStormControlBucketThresholdConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2BroadcastStormControlBucketThresholdConfiguration* data);

// BufferOverflowDrop access functions.
extern int rd_BufferOverflowDrop(struct dubhe1000_adapter *adapter,t_BufferOverflowDrop* data);
extern int wr_BufferOverflowDrop(struct dubhe1000_adapter *adapter,t_BufferOverflowDrop* data);

// ReservedMACSADrop access functions.
extern int rd_ReservedMACSADrop(struct dubhe1000_adapter *adapter,t_ReservedMACSADrop* data);
extern int wr_ReservedMACSADrop(struct dubhe1000_adapter *adapter,t_ReservedMACSADrop* data);

// EgressConfigurableACLDrop access functions.
extern int rd_EgressConfigurableACLDrop(struct dubhe1000_adapter *adapter,t_EgressConfigurableACLDrop* data);
extern int wr_EgressConfigurableACLDrop(struct dubhe1000_adapter *adapter,t_EgressConfigurableACLDrop* data);

// ReservedMACDADrop access functions.
extern int rd_ReservedMACDADrop(struct dubhe1000_adapter *adapter,t_ReservedMACDADrop* data);
extern int wr_ReservedMACDADrop(struct dubhe1000_adapter *adapter,t_ReservedMACDADrop* data);

// BeginningofPacketTunnelEntryInstructionTable access functions.
extern int rd_BeginningofPacketTunnelEntryInstructionTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_BeginningofPacketTunnelEntryInstructionTable* data);
extern int wr_BeginningofPacketTunnelEntryInstructionTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_BeginningofPacketTunnelEntryInstructionTable* data);

// TOSQoSMappingTable access functions.
extern int rd_TOSQoSMappingTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_TOSQoSMappingTable* data);
extern int wr_TOSQoSMappingTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_TOSQoSMappingTable* data);

// L2LookupCollisionTableMasks access functions.
extern int rd_L2LookupCollisionTableMasks(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2LookupCollisionTableMasks* data);
extern int wr_L2LookupCollisionTableMasks(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2LookupCollisionTableMasks* data);

// ForwardFromCPU access functions.
extern int rd_ForwardFromCPU(struct dubhe1000_adapter *adapter,t_ForwardFromCPU* data);
extern int wr_ForwardFromCPU(struct dubhe1000_adapter *adapter,t_ForwardFromCPU* data);

// IngressConfigurableACL2RulesSetup access functions.
extern int rd_IngressConfigurableACL2RulesSetup(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACL2RulesSetup* data);
extern int wr_IngressConfigurableACL2RulesSetup(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACL2RulesSetup* data);

// CheckIPv4HeaderChecksum access functions.
extern int rd_CheckIPv4HeaderChecksum(struct dubhe1000_adapter *adapter,t_CheckIPv4HeaderChecksum* data);
extern int wr_CheckIPv4HeaderChecksum(struct dubhe1000_adapter *adapter,t_CheckIPv4HeaderChecksum* data);

// UnknownEgressDrop access functions.
extern int rd_UnknownEgressDrop(struct dubhe1000_adapter *adapter, uint64_t idx,t_UnknownEgressDrop* data);
extern int wr_UnknownEgressDrop(struct dubhe1000_adapter *adapter, uint64_t idx,t_UnknownEgressDrop* data);

// EgressNATHitStatus access functions.
extern int rd_EgressNATHitStatus(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressNATHitStatus* data);
extern int wr_EgressNATHitStatus(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressNATHitStatus* data);

// HitUpdateDataFIFOHighWatermarkLevel access functions.
extern int rd_HitUpdateDataFIFOHighWatermarkLevel(struct dubhe1000_adapter *adapter,t_HitUpdateDataFIFOHighWatermarkLevel* data);
extern int wr_HitUpdateDataFIFOHighWatermarkLevel(struct dubhe1000_adapter *adapter,t_HitUpdateDataFIFOHighWatermarkLevel* data);

// ForceNonVLANPacketToSpecificQueue access functions.
extern int rd_ForceNonVLANPacketToSpecificQueue(struct dubhe1000_adapter *adapter,t_ForceNonVLANPacketToSpecificQueue* data);
extern int wr_ForceNonVLANPacketToSpecificQueue(struct dubhe1000_adapter *adapter,t_ForceNonVLANPacketToSpecificQueue* data);

// PFCDecCountersforingressports0to5 access functions.
extern int rd_PFCDecCountersforingressports0to5(struct dubhe1000_adapter *adapter, uint64_t idx,t_PFCDecCountersforingressports0to5* data);

// EPPDebugfromPort access functions.
extern int rd_EPPDebugfromPort(struct dubhe1000_adapter *adapter,t_EPPDebugfromPort* data);
extern int wr_EPPDebugfromPort(struct dubhe1000_adapter *adapter,t_EPPDebugfromPort* data);

// DebugIPPCounter access functions.
extern int rd_DebugIPPCounter(struct dubhe1000_adapter *adapter, uint64_t idx,t_DebugIPPCounter* data);
extern int wr_DebugIPPCounter(struct dubhe1000_adapter *adapter, uint64_t idx,t_DebugIPPCounter* data);

// DebugCounternextHopPtrFinalSetup access functions.
extern int rd_DebugCounternextHopPtrFinalSetup(struct dubhe1000_adapter *adapter,t_DebugCounternextHopPtrFinalSetup* data);
extern int wr_DebugCounternextHopPtrFinalSetup(struct dubhe1000_adapter *adapter,t_DebugCounternextHopPtrFinalSetup* data);

// PrioShaperBucketThresholdConfiguration access functions.
extern int rd_PrioShaperBucketThresholdConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_PrioShaperBucketThresholdConfiguration* data);
extern int wr_PrioShaperBucketThresholdConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_PrioShaperBucketThresholdConfiguration* data);

// IPChecksumDrop access functions.
extern int rd_IPChecksumDrop(struct dubhe1000_adapter *adapter,t_IPChecksumDrop* data);
extern int wr_IPChecksumDrop(struct dubhe1000_adapter *adapter,t_IPChecksumDrop* data);

// IPPDebugfinalVid access functions.
extern int rd_IPPDebugfinalVid(struct dubhe1000_adapter *adapter,t_IPPDebugfinalVid* data);
extern int wr_IPPDebugfinalVid(struct dubhe1000_adapter *adapter,t_IPPDebugfinalVid* data);

// PortTailDropFFAThreshold access functions.
extern int rd_PortTailDropFFAThreshold(struct dubhe1000_adapter *adapter, uint64_t idx,t_PortTailDropFFAThreshold* data);
extern int wr_PortTailDropFFAThreshold(struct dubhe1000_adapter *adapter, uint64_t idx,t_PortTailDropFFAThreshold* data);

// L2LookupDrop access functions.
extern int rd_L2LookupDrop(struct dubhe1000_adapter *adapter,t_L2LookupDrop* data);
extern int wr_L2LookupDrop(struct dubhe1000_adapter *adapter,t_L2LookupDrop* data);

// MPLSQoSMappingTable access functions.
extern int rd_MPLSQoSMappingTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_MPLSQoSMappingTable* data);
extern int wr_MPLSQoSMappingTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_MPLSQoSMappingTable* data);

// ForceUnknownL3PacketToSpecificColor access functions.
extern int rd_ForceUnknownL3PacketToSpecificColor(struct dubhe1000_adapter *adapter,t_ForceUnknownL3PacketToSpecificColor* data);
extern int wr_ForceUnknownL3PacketToSpecificColor(struct dubhe1000_adapter *adapter,t_ForceUnknownL3PacketToSpecificColor* data);

// TunnelExitTooSmallPacketModificationToSmallDrop access functions.
extern int rd_TunnelExitTooSmallPacketModificationToSmallDrop(struct dubhe1000_adapter *adapter, uint64_t idx,t_TunnelExitTooSmallPacketModificationToSmallDrop* data);
extern int wr_TunnelExitTooSmallPacketModificationToSmallDrop(struct dubhe1000_adapter *adapter, uint64_t idx,t_TunnelExitTooSmallPacketModificationToSmallDrop* data);

// DebugCounternextHopPtrLpmSetup access functions.
extern int rd_DebugCounternextHopPtrLpmSetup(struct dubhe1000_adapter *adapter,t_DebugCounternextHopPtrLpmSetup* data);
extern int wr_DebugCounternextHopPtrLpmSetup(struct dubhe1000_adapter *adapter,t_DebugCounternextHopPtrLpmSetup* data);

// IPv6ClassofServiceFieldToEgressQueueMappingTable access functions.
extern int rd_IPv6ClassofServiceFieldToEgressQueueMappingTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_IPv6ClassofServiceFieldToEgressQueueMappingTable* data);
extern int wr_IPv6ClassofServiceFieldToEgressQueueMappingTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_IPv6ClassofServiceFieldToEgressQueueMappingTable* data);

// DebugCountervlanVidOpSetup access functions.
extern int rd_DebugCountervlanVidOpSetup(struct dubhe1000_adapter *adapter,t_DebugCountervlanVidOpSetup* data);
extern int wr_DebugCountervlanVidOpSetup(struct dubhe1000_adapter *adapter,t_DebugCountervlanVidOpSetup* data);

// QueueShaperRateConfiguration access functions.
extern int rd_QueueShaperRateConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_QueueShaperRateConfiguration* data);
extern int wr_QueueShaperRateConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_QueueShaperRateConfiguration* data);

// EgressQueueDepth access functions.
extern int rd_EgressQueueDepth(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressQueueDepth* data);

// IngressConfigurableACL2TCAMAnswer access functions.
extern int rd_IngressConfigurableACL2TCAMAnswer(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACL2TCAMAnswer* data);
extern int wr_IngressConfigurableACL2TCAMAnswer(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACL2TCAMAnswer* data);

// ESPHeaderPacketDecoderOptions access functions.
extern int rd_ESPHeaderPacketDecoderOptions(struct dubhe1000_adapter *adapter,t_ESPHeaderPacketDecoderOptions* data);
extern int wr_ESPHeaderPacketDecoderOptions(struct dubhe1000_adapter *adapter,t_ESPHeaderPacketDecoderOptions* data);

// DefaultPacketToCPUModification access functions.
extern int rd_DefaultPacketToCPUModification(struct dubhe1000_adapter *adapter, uint64_t idx,t_DefaultPacketToCPUModification* data);
extern int wr_DefaultPacketToCPUModification(struct dubhe1000_adapter *adapter, uint64_t idx,t_DefaultPacketToCPUModification* data);

// RouterPortMACAddress access functions.
extern int rd_RouterPortMACAddress(struct dubhe1000_adapter *adapter, uint64_t idx,t_RouterPortMACAddress* data);
extern int wr_RouterPortMACAddress(struct dubhe1000_adapter *adapter, uint64_t idx,t_RouterPortMACAddress* data);

// EPPPMDrop access functions.
extern int rd_EPPPMDrop(struct dubhe1000_adapter *adapter,t_EPPPMDrop* data);
extern int wr_EPPPMDrop(struct dubhe1000_adapter *adapter,t_EPPPMDrop* data);

// EgressConfigurableACL0TCAM access functions.
extern int rd_EgressConfigurableACL0TCAM(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressConfigurableACL0TCAM* data);
extern int wr_EgressConfigurableACL0TCAM(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressConfigurableACL0TCAM* data);

// IngressConfigurableACL2TCAM access functions.
extern int rd_IngressConfigurableACL2TCAM(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACL2TCAM* data);
extern int wr_IngressConfigurableACL2TCAM(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACL2TCAM* data);

// EgressACLRulePointerTCAM access functions.
extern int rd_EgressACLRulePointerTCAM(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressACLRulePointerTCAM* data);
extern int wr_EgressACLRulePointerTCAM(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressACLRulePointerTCAM* data);

// LLDPConfiguration access functions.
extern int rd_LLDPConfiguration(struct dubhe1000_adapter *adapter,t_LLDPConfiguration* data);
extern int wr_LLDPConfiguration(struct dubhe1000_adapter *adapter,t_LLDPConfiguration* data);

// IngressSpanningTreeDropListen access functions.
extern int rd_IngressSpanningTreeDropListen(struct dubhe1000_adapter *adapter,t_IngressSpanningTreeDropListen* data);
extern int wr_IngressSpanningTreeDropListen(struct dubhe1000_adapter *adapter,t_IngressSpanningTreeDropListen* data);

// EgressQueueToPCPAndCFIDEIMappingTable access functions.
extern int rd_EgressQueueToPCPAndCFIDEIMappingTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressQueueToPCPAndCFIDEIMappingTable* data);
extern int wr_EgressQueueToPCPAndCFIDEIMappingTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressQueueToPCPAndCFIDEIMappingTable* data);

// DebugCounterl2DaHashHitAndBucketSetup access functions.
extern int rd_DebugCounterl2DaHashHitAndBucketSetup(struct dubhe1000_adapter *adapter,t_DebugCounterl2DaHashHitAndBucketSetup* data);
extern int wr_DebugCounterl2DaHashHitAndBucketSetup(struct dubhe1000_adapter *adapter,t_DebugCounterl2DaHashHitAndBucketSetup* data);

// L2AgingCollisionShadowTable access functions.
extern int rd_L2AgingCollisionShadowTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2AgingCollisionShadowTable* data);
extern int wr_L2AgingCollisionShadowTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2AgingCollisionShadowTable* data);

// L2ActionTableDrop access functions.
extern int rd_L2ActionTableDrop(struct dubhe1000_adapter *adapter,t_L2ActionTableDrop* data);
extern int wr_L2ActionTableDrop(struct dubhe1000_adapter *adapter,t_L2ActionTableDrop* data);

// AgingDataFIFO access functions.
extern int rd_AgingDataFIFO(struct dubhe1000_adapter *adapter,t_AgingDataFIFO* data);

// IPPDebugrouterHit access functions.
extern int rd_IPPDebugrouterHit(struct dubhe1000_adapter *adapter,t_IPPDebugrouterHit* data);
extern int wr_IPPDebugrouterHit(struct dubhe1000_adapter *adapter,t_IPPDebugrouterHit* data);

// BufferFree access functions.
extern int rd_BufferFree(struct dubhe1000_adapter *adapter,t_BufferFree* data);

// L2ActionTableSpecialPacketTypeDrop access functions.
extern int rd_L2ActionTableSpecialPacketTypeDrop(struct dubhe1000_adapter *adapter,t_L2ActionTableSpecialPacketTypeDrop* data);
extern int wr_L2ActionTableSpecialPacketTypeDrop(struct dubhe1000_adapter *adapter,t_L2ActionTableSpecialPacketTypeDrop* data);

// AHHeaderPacketDecoderOptions access functions.
extern int rd_AHHeaderPacketDecoderOptions(struct dubhe1000_adapter *adapter,t_AHHeaderPacketDecoderOptions* data);
extern int wr_AHHeaderPacketDecoderOptions(struct dubhe1000_adapter *adapter,t_AHHeaderPacketDecoderOptions* data);

// DebugCounterl2DaHashSetup access functions.
extern int rd_DebugCounterl2DaHashSetup(struct dubhe1000_adapter *adapter,t_DebugCounterl2DaHashSetup* data);
extern int wr_DebugCounterl2DaHashSetup(struct dubhe1000_adapter *adapter,t_DebugCounterl2DaHashSetup* data);

// RouterEgressQueueToVLANData access functions.
extern int rd_RouterEgressQueueToVLANData(struct dubhe1000_adapter *adapter, uint64_t idx,t_RouterEgressQueueToVLANData* data);
extern int wr_RouterEgressQueueToVLANData(struct dubhe1000_adapter *adapter, uint64_t idx,t_RouterEgressQueueToVLANData* data);

// IngressSpanningTreeDropLearning access functions.
extern int rd_IngressSpanningTreeDropLearning(struct dubhe1000_adapter *adapter,t_IngressSpanningTreeDropLearning* data);
extern int wr_IngressSpanningTreeDropLearning(struct dubhe1000_adapter *adapter,t_IngressSpanningTreeDropLearning* data);

// EPPDebugreQueuePortId access functions.
extern int rd_EPPDebugreQueuePortId(struct dubhe1000_adapter *adapter,t_EPPDebugreQueuePortId* data);
extern int wr_EPPDebugreQueuePortId(struct dubhe1000_adapter *adapter,t_EPPDebugreQueuePortId* data);

// LinkAggregationMembership access functions.
extern int rd_LinkAggregationMembership(struct dubhe1000_adapter *adapter, uint64_t idx,t_LinkAggregationMembership* data);
extern int wr_LinkAggregationMembership(struct dubhe1000_adapter *adapter, uint64_t idx,t_LinkAggregationMembership* data);

// PBPacketTailCounter access functions.
extern int rd_PBPacketTailCounter(struct dubhe1000_adapter *adapter,t_PBPacketTailCounter* data);
extern int wr_PBPacketTailCounter(struct dubhe1000_adapter *adapter,t_PBPacketTailCounter* data);

// FFAUsednonPFC access functions.
extern int rd_FFAUsednonPFC(struct dubhe1000_adapter *adapter,t_FFAUsednonPFC* data);

// IngressConfigurableACL3RulesSetup access functions.
extern int rd_IngressConfigurableACL3RulesSetup(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACL3RulesSetup* data);
extern int wr_IngressConfigurableACL3RulesSetup(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACL3RulesSetup* data);

// L2ActionTable access functions.
extern int rd_L2ActionTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2ActionTable* data);
extern int wr_L2ActionTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2ActionTable* data);

// IPPDebugnextHopPtrHashHit access functions.
extern int rd_IPPDebugnextHopPtrHashHit(struct dubhe1000_adapter *adapter,t_IPPDebugnextHopPtrHashHit* data);
extern int wr_IPPDebugnextHopPtrHashHit(struct dubhe1000_adapter *adapter,t_IPPDebugnextHopPtrHashHit* data);

// IEEE1588L2PacketDecoderOptions access functions.
extern int rd_IEEE1588L2PacketDecoderOptions(struct dubhe1000_adapter *adapter,t_IEEE1588L2PacketDecoderOptions* data);
extern int wr_IEEE1588L2PacketDecoderOptions(struct dubhe1000_adapter *adapter,t_IEEE1588L2PacketDecoderOptions* data);

// TunnelExitLookupTCAM access functions.
extern int rd_TunnelExitLookupTCAM(struct dubhe1000_adapter *adapter, uint64_t idx,t_TunnelExitLookupTCAM* data);
extern int wr_TunnelExitLookupTCAM(struct dubhe1000_adapter *adapter, uint64_t idx,t_TunnelExitLookupTCAM* data);

// IPPDebugnextHopPtrHash access functions.
extern int rd_IPPDebugnextHopPtrHash(struct dubhe1000_adapter *adapter,t_IPPDebugnextHopPtrHash* data);
extern int wr_IPPDebugnextHopPtrHash(struct dubhe1000_adapter *adapter,t_IPPDebugnextHopPtrHash* data);

// IngressConfigurableACLMatchCounter access functions.
extern int rd_IngressConfigurableACLMatchCounter(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACLMatchCounter* data);
extern int wr_IngressConfigurableACLMatchCounter(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACLMatchCounter* data);

// OutputMirroringTable access functions.
extern int rd_OutputMirroringTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_OutputMirroringTable* data);
extern int wr_OutputMirroringTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_OutputMirroringTable* data);

// IEEE1588L4PacketDecoderOptions access functions.
extern int rd_IEEE1588L4PacketDecoderOptions(struct dubhe1000_adapter *adapter,t_IEEE1588L4PacketDecoderOptions* data);
extern int wr_IEEE1588L4PacketDecoderOptions(struct dubhe1000_adapter *adapter,t_IEEE1588L4PacketDecoderOptions* data);

// EPPDebugaddNewMpls access functions.
extern int rd_EPPDebugaddNewMpls(struct dubhe1000_adapter *adapter,t_EPPDebugaddNewMpls* data);
extern int wr_EPPDebugaddNewMpls(struct dubhe1000_adapter *adapter,t_EPPDebugaddNewMpls* data);

// DebugCounterfinalVidSetup access functions.
extern int rd_DebugCounterfinalVidSetup(struct dubhe1000_adapter *adapter,t_DebugCounterfinalVidSetup* data);
extern int wr_DebugCounterfinalVidSetup(struct dubhe1000_adapter *adapter,t_DebugCounterfinalVidSetup* data);

// PortUsed access functions.
extern int rd_PortUsed(struct dubhe1000_adapter *adapter, uint64_t idx,t_PortUsed* data);

// ARPDecoderDrop access functions.
extern int rd_ARPDecoderDrop(struct dubhe1000_adapter *adapter,t_ARPDecoderDrop* data);
extern int wr_ARPDecoderDrop(struct dubhe1000_adapter *adapter,t_ARPDecoderDrop* data);

// SecondTunnelExitMissAction access functions.
extern int rd_SecondTunnelExitMissAction(struct dubhe1000_adapter *adapter, uint64_t idx,t_SecondTunnelExitMissAction* data);
extern int wr_SecondTunnelExitMissAction(struct dubhe1000_adapter *adapter, uint64_t idx,t_SecondTunnelExitMissAction* data);

// IngressConfigurableACL0SmallTable access functions.
extern int rd_IngressConfigurableACL0SmallTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACL0SmallTable* data);
extern int wr_IngressConfigurableACL0SmallTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACL0SmallTable* data);

// PortTCReserved access functions.
extern int rd_PortTCReserved(struct dubhe1000_adapter *adapter, uint64_t idx,t_PortTCReserved* data);
extern int wr_PortTCReserved(struct dubhe1000_adapter *adapter, uint64_t idx,t_PortTCReserved* data);

// L2QoSMappingTable access functions.
extern int rd_L2QoSMappingTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2QoSMappingTable* data);
extern int wr_L2QoSMappingTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2QoSMappingTable* data);

// L2MulticastHandling access functions.
extern int rd_L2MulticastHandling(struct dubhe1000_adapter *adapter,t_L2MulticastHandling* data);
extern int wr_L2MulticastHandling(struct dubhe1000_adapter *adapter,t_L2MulticastHandling* data);

// EgressTunnelExitTable access functions.
extern int rd_EgressTunnelExitTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressTunnelExitTable* data);
extern int wr_EgressTunnelExitTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressTunnelExitTable* data);

// MACInterfaceCountersForRX access functions.
extern int rd_MACInterfaceCountersForRX(struct dubhe1000_adapter *adapter, uint64_t idx,t_MACInterfaceCountersForRX* data);

// IngressConfigurableACL1Selection access functions.
extern int rd_IngressConfigurableACL1Selection(struct dubhe1000_adapter *adapter,t_IngressConfigurableACL1Selection* data);
extern int wr_IngressConfigurableACL1Selection(struct dubhe1000_adapter *adapter,t_IngressConfigurableACL1Selection* data);

// NextHopTable access functions.
extern int rd_NextHopTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_NextHopTable* data);
extern int wr_NextHopTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_NextHopTable* data);

// EgressACLRulePointerTCAMAnswer access functions.
extern int rd_EgressACLRulePointerTCAMAnswer(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressACLRulePointerTCAMAnswer* data);
extern int wr_EgressACLRulePointerTCAMAnswer(struct dubhe1000_adapter *adapter, uint64_t idx,t_EgressACLRulePointerTCAMAnswer* data);

// SourcePortDefaultACLAction access functions.
extern int rd_SourcePortDefaultACLAction(struct dubhe1000_adapter *adapter, uint64_t idx,t_SourcePortDefaultACLAction* data);
extern int wr_SourcePortDefaultACLAction(struct dubhe1000_adapter *adapter, uint64_t idx,t_SourcePortDefaultACLAction* data);

// VLANTable access functions.
extern int rd_VLANTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_VLANTable* data);
extern int wr_VLANTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_VLANTable* data);

// IngressPortPacketTypeFilter access functions.
extern int rd_IngressPortPacketTypeFilter(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressPortPacketTypeFilter* data);
extern int wr_IngressPortPacketTypeFilter(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressPortPacketTypeFilter* data);

// DrainPort access functions.
extern int rd_DrainPort(struct dubhe1000_adapter *adapter,t_DrainPort* data);
extern int wr_DrainPort(struct dubhe1000_adapter *adapter,t_DrainPort* data);

// L3LookupDrop access functions.
extern int rd_L3LookupDrop(struct dubhe1000_adapter *adapter,t_L3LookupDrop* data);
extern int wr_L3LookupDrop(struct dubhe1000_adapter *adapter,t_L3LookupDrop* data);

// TunnelExitTooSmalltoDecodeDrop access functions.
extern int rd_TunnelExitTooSmalltoDecodeDrop(struct dubhe1000_adapter *adapter,t_TunnelExitTooSmalltoDecodeDrop* data);
extern int wr_TunnelExitTooSmalltoDecodeDrop(struct dubhe1000_adapter *adapter,t_TunnelExitTooSmalltoDecodeDrop* data);

// EPPDebugomImActive access functions.
extern int rd_EPPDebugomImActive(struct dubhe1000_adapter *adapter,t_EPPDebugomImActive* data);
extern int wr_EPPDebugomImActive(struct dubhe1000_adapter *adapter,t_EPPDebugomImActive* data);

// PortFFAUsed access functions.
extern int rd_PortFFAUsed(struct dubhe1000_adapter *adapter, uint64_t idx,t_PortFFAUsed* data);

// LACPPacketDecoderOptions access functions.
extern int rd_LACPPacketDecoderOptions(struct dubhe1000_adapter *adapter,t_LACPPacketDecoderOptions* data);
extern int wr_LACPPacketDecoderOptions(struct dubhe1000_adapter *adapter,t_LACPPacketDecoderOptions* data);

// DefaultLearningStatus access functions.
extern int rd_DefaultLearningStatus(struct dubhe1000_adapter *adapter, uint64_t idx,t_DefaultLearningStatus* data);
extern int wr_DefaultLearningStatus(struct dubhe1000_adapter *adapter, uint64_t idx,t_DefaultLearningStatus* data);

// LearningConflict access functions.
extern int rd_LearningConflict(struct dubhe1000_adapter *adapter,t_LearningConflict* data);
extern int wr_LearningConflict(struct dubhe1000_adapter *adapter,t_LearningConflict* data);

// IngressVIDEthernetTypeRangeAssignmentAnswer access functions.
extern int rd_IngressVIDEthernetTypeRangeAssignmentAnswer(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressVIDEthernetTypeRangeAssignmentAnswer* data);
extern int wr_IngressVIDEthernetTypeRangeAssignmentAnswer(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressVIDEthernetTypeRangeAssignmentAnswer* data);

// L2MulticastStormControlBucketThresholdConfiguration access functions.
extern int rd_L2MulticastStormControlBucketThresholdConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2MulticastStormControlBucketThresholdConfiguration* data);
extern int wr_L2MulticastStormControlBucketThresholdConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2MulticastStormControlBucketThresholdConfiguration* data);

// IngressConfigurableACL3TCAM access functions.
extern int rd_IngressConfigurableACL3TCAM(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACL3TCAM* data);
extern int wr_IngressConfigurableACL3TCAM(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACL3TCAM* data);

// L2BroadcastStormControlBucketCapacityConfiguration access functions.
extern int rd_L2BroadcastStormControlBucketCapacityConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2BroadcastStormControlBucketCapacityConfiguration* data);
extern int wr_L2BroadcastStormControlBucketCapacityConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2BroadcastStormControlBucketCapacityConfiguration* data);

// NATActionTable access functions.
extern int rd_NATActionTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_NATActionTable* data);
extern int wr_NATActionTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_NATActionTable* data);

// IngressConfigurableACL1RulesSetup access functions.
extern int rd_IngressConfigurableACL1RulesSetup(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACL1RulesSetup* data);
extern int wr_IngressConfigurableACL1RulesSetup(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACL1RulesSetup* data);

// VLANPCPAndDEIToColorMappingTable access functions.
extern int rd_VLANPCPAndDEIToColorMappingTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_VLANPCPAndDEIToColorMappingTable* data);
extern int wr_VLANPCPAndDEIToColorMappingTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_VLANPCPAndDEIToColorMappingTable* data);

// CAPWAPDecoderDrop access functions.
extern int rd_CAPWAPDecoderDrop(struct dubhe1000_adapter *adapter,t_CAPWAPDecoderDrop* data);
extern int wr_CAPWAPDecoderDrop(struct dubhe1000_adapter *adapter,t_CAPWAPDecoderDrop* data);

// SMONByteCounter access functions.
extern int rd_SMONByteCounter(struct dubhe1000_adapter *adapter,uint64_t set, uint64_t idx,t_SMONByteCounter* data);
extern int wr_SMONByteCounter(struct dubhe1000_adapter *adapter,uint64_t set, uint64_t idx,t_SMONByteCounter* data);

// IngressEgressPortPacketTypeFilter access functions.
extern int rd_IngressEgressPortPacketTypeFilter(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressEgressPortPacketTypeFilter* data);
extern int wr_IngressEgressPortPacketTypeFilter(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressEgressPortPacketTypeFilter* data);

// IPPPacketHeadCounter access functions.
extern int rd_IPPPacketHeadCounter(struct dubhe1000_adapter *adapter,t_IPPPacketHeadCounter* data);
extern int wr_IPPPacketHeadCounter(struct dubhe1000_adapter *adapter,t_IPPPacketHeadCounter* data);

// ColorRemapFromEgressPort access functions.
extern int rd_ColorRemapFromEgressPort(struct dubhe1000_adapter *adapter, uint64_t idx,t_ColorRemapFromEgressPort* data);
extern int wr_ColorRemapFromEgressPort(struct dubhe1000_adapter *adapter, uint64_t idx,t_ColorRemapFromEgressPort* data);

// IngressNATOperation access functions.
extern int rd_IngressNATOperation(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressNATOperation* data);
extern int wr_IngressNATOperation(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressNATOperation* data);

// SNAPLLCDecodingOptions access functions.
extern int rd_SNAPLLCDecodingOptions(struct dubhe1000_adapter *adapter,t_SNAPLLCDecodingOptions* data);
extern int wr_SNAPLLCDecodingOptions(struct dubhe1000_adapter *adapter,t_SNAPLLCDecodingOptions* data);

// IPPDebugl2DaHashHitAndBucket access functions.
extern int rd_IPPDebugl2DaHashHitAndBucket(struct dubhe1000_adapter *adapter,t_IPPDebugl2DaHashHitAndBucket* data);
extern int wr_IPPDebugl2DaHashHitAndBucket(struct dubhe1000_adapter *adapter,t_IPPDebugl2DaHashHitAndBucket* data);

// IPQoSMappingTable access functions.
extern int rd_IPQoSMappingTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_IPQoSMappingTable* data);
extern int wr_IPQoSMappingTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_IPQoSMappingTable* data);

// IngressAdmissionControlTokenBucketConfiguration access functions.
extern int rd_IngressAdmissionControlTokenBucketConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressAdmissionControlTokenBucketConfiguration* data);
extern int wr_IngressAdmissionControlTokenBucketConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressAdmissionControlTokenBucketConfiguration* data);

// L2BroadcastStormControlRateConfiguration access functions.
extern int rd_L2BroadcastStormControlRateConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2BroadcastStormControlRateConfiguration* data);
extern int wr_L2BroadcastStormControlRateConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2BroadcastStormControlRateConfiguration* data);

// L2BroadcastStormControlEnable access functions.
extern int rd_L2BroadcastStormControlEnable(struct dubhe1000_adapter *adapter,t_L2BroadcastStormControlEnable* data);
extern int wr_L2BroadcastStormControlEnable(struct dubhe1000_adapter *adapter,t_L2BroadcastStormControlEnable* data);

// PortTCXoffTotalThreshold access functions.
extern int rd_PortTCXoffTotalThreshold(struct dubhe1000_adapter *adapter, uint64_t idx,t_PortTCXoffTotalThreshold* data);
extern int wr_PortTCXoffTotalThreshold(struct dubhe1000_adapter *adapter, uint64_t idx,t_PortTCXoffTotalThreshold* data);

// EgressConfigurableACL0SearchMask access functions.
extern int rd_EgressConfigurableACL0SearchMask(struct dubhe1000_adapter *adapter,t_EgressConfigurableACL0SearchMask* data);
extern int wr_EgressConfigurableACL0SearchMask(struct dubhe1000_adapter *adapter,t_EgressConfigurableACL0SearchMask* data);

// SMONPacketCounter access functions.
extern int rd_SMONPacketCounter(struct dubhe1000_adapter *adapter,uint64_t set, uint64_t idx,t_SMONPacketCounter* data);
extern int wr_SMONPacketCounter(struct dubhe1000_adapter *adapter,uint64_t set, uint64_t idx,t_SMONPacketCounter* data);

// L2FloodingStormControlBucketThresholdConfiguration access functions.
extern int rd_L2FloodingStormControlBucketThresholdConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2FloodingStormControlBucketThresholdConfiguration* data);
extern int wr_L2FloodingStormControlBucketThresholdConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_L2FloodingStormControlBucketThresholdConfiguration* data);

// CPUReasonCodeOperation access functions.
extern int rd_CPUReasonCodeOperation(struct dubhe1000_adapter *adapter, uint64_t idx,t_CPUReasonCodeOperation* data);
extern int wr_CPUReasonCodeOperation(struct dubhe1000_adapter *adapter, uint64_t idx,t_CPUReasonCodeOperation* data);

// HashBasedL3RoutingTable access functions.
extern int rd_HashBasedL3RoutingTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_HashBasedL3RoutingTable* data);
extern int wr_HashBasedL3RoutingTable(struct dubhe1000_adapter *adapter, uint64_t idx,t_HashBasedL3RoutingTable* data);

// PortShaperBucketCapacityConfiguration access functions.
extern int rd_PortShaperBucketCapacityConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_PortShaperBucketCapacityConfiguration* data);
extern int wr_PortShaperBucketCapacityConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_PortShaperBucketCapacityConfiguration* data);

// L3RoutingTCAM access functions.
extern int rd_L3RoutingTCAM(struct dubhe1000_adapter *adapter, uint64_t idx,t_L3RoutingTCAM* data);
extern int wr_L3RoutingTCAM(struct dubhe1000_adapter *adapter, uint64_t idx,t_L3RoutingTCAM* data);

// PortXonFFAThreshold access functions.
extern int rd_PortXonFFAThreshold(struct dubhe1000_adapter *adapter, uint64_t idx,t_PortXonFFAThreshold* data);
extern int wr_PortXonFFAThreshold(struct dubhe1000_adapter *adapter, uint64_t idx,t_PortXonFFAThreshold* data);

// IEEE8021XandEAPOLDecoderDrop access functions.
extern int rd_IEEE8021XandEAPOLDecoderDrop(struct dubhe1000_adapter *adapter,t_IEEE8021XandEAPOLDecoderDrop* data);
extern int wr_IEEE8021XandEAPOLDecoderDrop(struct dubhe1000_adapter *adapter,t_IEEE8021XandEAPOLDecoderDrop* data);

// L3LPMResult access functions.
extern int rd_L3LPMResult(struct dubhe1000_adapter *adapter, uint64_t idx,t_L3LPMResult* data);
extern int wr_L3LPMResult(struct dubhe1000_adapter *adapter, uint64_t idx,t_L3LPMResult* data);

// IngressConfigurableACL1TCAM access functions.
extern int rd_IngressConfigurableACL1TCAM(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACL1TCAM* data);
extern int wr_IngressConfigurableACL1TCAM(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACL1TCAM* data);

// PSErrorCounter access functions.
extern int rd_PSErrorCounter(struct dubhe1000_adapter *adapter, uint64_t idx,t_PSErrorCounter* data);

// IngressConfigurableACL0PreLookup access functions.
extern int rd_IngressConfigurableACL0PreLookup(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACL0PreLookup* data);
extern int wr_IngressConfigurableACL0PreLookup(struct dubhe1000_adapter *adapter, uint64_t idx,t_IngressConfigurableACL0PreLookup* data);

// IPPDebugisFlooding access functions.
extern int rd_IPPDebugisFlooding(struct dubhe1000_adapter *adapter,t_IPPDebugisFlooding* data);
extern int wr_IPPDebugisFlooding(struct dubhe1000_adapter *adapter,t_IPPDebugisFlooding* data);

// DWRRBucketCapacityConfiguration access functions.
extern int rd_DWRRBucketCapacityConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_DWRRBucketCapacityConfiguration* data);
extern int wr_DWRRBucketCapacityConfiguration(struct dubhe1000_adapter *adapter, uint64_t idx,t_DWRRBucketCapacityConfiguration* data);

// PortPauseSettings access functions.
extern int rd_PortPauseSettings(struct dubhe1000_adapter *adapter, uint64_t idx,t_PortPauseSettings* data);
extern int wr_PortPauseSettings(struct dubhe1000_adapter *adapter, uint64_t idx,t_PortPauseSettings* data);
#endif

 // EOF
