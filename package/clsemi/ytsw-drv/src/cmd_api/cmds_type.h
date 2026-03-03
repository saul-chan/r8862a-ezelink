parameters_convert_st yt_port_mask_t_list[] = {
{"portbits", offsetof(yt_port_mask_t, portbits), sizeof(((yt_port_mask_t*)0)->portbits)/sizeof(uint32_t), T_UINT32_T },
};
em_str_to_id_t yt_enable_t_map[] = {
{"YT_DISABLE", YT_DISABLE },
{"YT_ENABLE", YT_ENABLE },
};
parameters_convert_st yt_tpid_profiles_t_list[] = {
{"tpid", offsetof(yt_tpid_profiles_t, tpid), sizeof(((yt_tpid_profiles_t*)0)->tpid)/sizeof(uint16_t), T_UINT16_T },
};
em_str_to_id_t yt_vlan_type_t_map[] = {
{"VLAN_TYPE_CVLAN", VLAN_TYPE_CVLAN },
{"VLAN_TYPE_SVLAN", VLAN_TYPE_SVLAN },
};
em_str_to_id_t yt_vlan_aft_t_map[] = {
{"VLAN_AFT_ALL", VLAN_AFT_ALL },
{"VLAN_AFT_TAGGED", VLAN_AFT_TAGGED },
{"VLAN_AFT_UNTAGGED", VLAN_AFT_UNTAGGED },
};
em_str_to_id_t yt_egr_tag_mode_t_map[] = {
{"VLAN_TAG_MODE_UNTAGGED", VLAN_TAG_MODE_UNTAGGED },
{"VLAN_TAG_MODE_TAGGED", VLAN_TAG_MODE_TAGGED },
{"VLAN_TAG_MODE_TAGGED_EXPECT_PVID", VLAN_TAG_MODE_TAGGED_EXPECT_PVID },
{"VLAN_TAG_MODE_PRIO_TAGGED", VLAN_TAG_MODE_PRIO_TAGGED },
{"VLAN_TAG_MODE_KEEP_ALL", VLAN_TAG_MODE_KEEP_ALL },
{"VLAN_TAG_MODE_KEEP_TAGGED_MODE", VLAN_TAG_MODE_KEEP_TAGGED_MODE },
{"VLAN_TAG_MODE_ENTRY_BASED", VLAN_TAG_MODE_ENTRY_BASED },
};
parameters_convert_st yt_vlan_protocol_key_t_list[] = {
{"l2_type", offsetof(yt_vlan_protocol_key_t, l2_type), 0, T_YT_L2_TYPE_T },
{"eth_type", offsetof(yt_vlan_protocol_key_t, eth_type), 0, T_UINT16_T },
{"key_valid", offsetof(yt_vlan_protocol_key_t, key_valid), 0, T_YT_BOOL_T },
};
parameters_convert_st yt_vlan_protocol_action_t_list[] = {
{"cvid", offsetof(yt_vlan_protocol_action_t, cvid), 0, T_UINT16_T },
{"cvlan_action", offsetof(yt_vlan_protocol_action_t, cvlan_action), 0, T_YT_VLAN_TRANS_ACTION_T },
{"svid", offsetof(yt_vlan_protocol_action_t, svid), 0, T_UINT16_T },
{"svlan_action", offsetof(yt_vlan_protocol_action_t, svlan_action), 0, T_YT_VLAN_TRANS_ACTION_T },
};
parameters_convert_st yt_vlan_range_group_t_list[] = {
{"vid_range0_min", offsetof(yt_vlan_range_group_t, vid_range0_min), 0, T_UINT16_T },
{"vid_range0_max", offsetof(yt_vlan_range_group_t, vid_range0_max), 0, T_UINT16_T },
{"vid_range1_min", offsetof(yt_vlan_range_group_t, vid_range1_min), 0, T_UINT16_T },
{"vid_range1_max", offsetof(yt_vlan_range_group_t, vid_range1_max), 0, T_UINT16_T },
{"vid_range2_min", offsetof(yt_vlan_range_group_t, vid_range2_min), 0, T_UINT16_T },
{"vid_range2_max", offsetof(yt_vlan_range_group_t, vid_range2_max), 0, T_UINT16_T },
{"vid_range3_min", offsetof(yt_vlan_range_group_t, vid_range3_min), 0, T_UINT16_T },
{"vid_range3_max", offsetof(yt_vlan_range_group_t, vid_range3_max), 0, T_UINT16_T },
};
em_str_to_id_t yt_vlan_range_trans_mode_t_map[] = {
{"VLAN_RANGE_TRANS_MODE_CVLAN", VLAN_RANGE_TRANS_MODE_CVLAN },
{"VLAN_RANGE_TRANS_MODE_SVLAN", VLAN_RANGE_TRANS_MODE_SVLAN },
};
parameters_convert_st yt_vlan_trans_tbl_t_list[] = {
{"svid_valid", offsetof(yt_vlan_trans_tbl_t, svid_valid), 0, T_YT_BOOL_T },
{"svid", offsetof(yt_vlan_trans_tbl_t, svid), 0, T_UINT16_T },
{"stag_format_valid", offsetof(yt_vlan_trans_tbl_t, stag_format_valid), 0, T_YT_BOOL_T },
{"stag_format", offsetof(yt_vlan_trans_tbl_t, stag_format), 0, T_YT_VLAN_FORMAT_T },
{"cvid_valid", offsetof(yt_vlan_trans_tbl_t, cvid_valid), 0, T_YT_BOOL_T },
{"cvid", offsetof(yt_vlan_trans_tbl_t, cvid), 0, T_UINT16_T },
{"ctag_format_valid", offsetof(yt_vlan_trans_tbl_t, ctag_format_valid), 0, T_YT_BOOL_T },
{"ctag_format", offsetof(yt_vlan_trans_tbl_t, ctag_format), 0, T_YT_VLAN_FORMAT_T },
{"valid_port_mask", offsetof(yt_vlan_trans_tbl_t, valid_port_mask), 0, T_UINT16_T },
};
parameters_convert_st yt_vlan_trans_action_tbl_t_list[] = {
{"svid_action", offsetof(yt_vlan_trans_action_tbl_t, svid_action), 0, T_YT_VLAN_TRANS_ACTION_T },
{"assign_svid", offsetof(yt_vlan_trans_action_tbl_t, assign_svid), 0, T_UINT16_T },
{"cvid_action", offsetof(yt_vlan_trans_action_tbl_t, cvid_action), 0, T_YT_VLAN_TRANS_ACTION_T },
{"assign_cvid", offsetof(yt_vlan_trans_action_tbl_t, assign_cvid), 0, T_UINT16_T },
};
parameters_convert_st yt_egr_vlan_trans_tbl_t_list[] = {
{"svid_valid", offsetof(yt_egr_vlan_trans_tbl_t, svid_valid), 0, T_YT_BOOL_T },
{"cvid_valid", offsetof(yt_egr_vlan_trans_tbl_t, cvid_valid), 0, T_YT_BOOL_T },
{"mvr_valid", offsetof(yt_egr_vlan_trans_tbl_t, mvr_valid), 0, T_YT_BOOL_T },
{"valid_port_mask", offsetof(yt_egr_vlan_trans_tbl_t, valid_port_mask), 0, T_UINT16_T },
{"vid_range_mode", offsetof(yt_egr_vlan_trans_tbl_t, vid_range_mode), 0, T_YT_VLAN_RANGE_TRANS_MODE_T },
{"vid_range_min", offsetof(yt_egr_vlan_trans_tbl_t, vid_range_min), 0, T_UINT16_T },
{"vid_range_max", offsetof(yt_egr_vlan_trans_tbl_t, vid_range_max), 0, T_UINT16_T },
{"vid", offsetof(yt_egr_vlan_trans_tbl_t, vid), 0, T_UINT16_T },
{"original_ctag_format_valid", offsetof(yt_egr_vlan_trans_tbl_t, original_ctag_format_valid), 0, T_YT_BOOL_T },
{"original_stag_format_valid", offsetof(yt_egr_vlan_trans_tbl_t, original_stag_format_valid), 0, T_YT_BOOL_T },
};
parameters_convert_st yt_egr_vlan_trans_action_tbl_t_list[] = {
{"svid_enable", offsetof(yt_egr_vlan_trans_action_tbl_t, svid_enable), 0, T_YT_BOOL_T },
{"cvid_enable", offsetof(yt_egr_vlan_trans_action_tbl_t, cvid_enable), 0, T_YT_BOOL_T },
{"svid", offsetof(yt_egr_vlan_trans_action_tbl_t, svid), 0, T_UINT16_T },
{"cvid", offsetof(yt_egr_vlan_trans_action_tbl_t, cvid), 0, T_UINT16_T },
};
em_str_to_id_t yt_acl_udf_type_t_map[] = {
{"ACL_UDF_TYPE_RAW", ACL_UDF_TYPE_RAW },
{"ACL_UDF_TYPE_L3", ACL_UDF_TYPE_L3 },
{"ACL_UDF_TYPE_L4", ACL_UDF_TYPE_L4 },
};
em_str_to_id_t yt_igrAcl_key_type_t_map[] = {
{"YT_IGRACL_TEMPLATE_MAC_DA", YT_IGRACL_TEMPLATE_MAC_DA },
{"YT_IGRACL_TEMPLATE_MAC_SA", YT_IGRACL_TEMPLATE_MAC_SA },
{"YT_IGRACL_TEMPLATE_L2_TYPE", YT_IGRACL_TEMPLATE_L2_TYPE },
{"YT_IGRACL_TEMPLATE_L3_TYPE", YT_IGRACL_TEMPLATE_L3_TYPE },
{"YT_IGRACL_TEMPLATE_CDEI", YT_IGRACL_TEMPLATE_CDEI },
{"YT_IGRACL_TEMPLATE_CPRI", YT_IGRACL_TEMPLATE_CPRI },
{"YT_IGRACL_TEMPLATE_CTAG_FMT", YT_IGRACL_TEMPLATE_CTAG_FMT },
{"YT_IGRACL_TEMPLATE_SDEI", YT_IGRACL_TEMPLATE_SDEI },
{"YT_IGRACL_TEMPLATE_SPRI", YT_IGRACL_TEMPLATE_SPRI },
{"YT_IGRACL_TEMPLATE_STAG_FMT", YT_IGRACL_TEMPLATE_STAG_FMT },
{"YT_IGRACL_TEMPLATE_SVID", YT_IGRACL_TEMPLATE_SVID },
{"YT_IGRACL_TEMPLATE_CVID", YT_IGRACL_TEMPLATE_CVID },
{"YT_IGRACL_TEMPLATE_IPV4_DA", YT_IGRACL_TEMPLATE_IPV4_DA },
{"YT_IGRACL_TEMPLATE_IPV4_SA", YT_IGRACL_TEMPLATE_IPV4_SA },
{"YT_IGRACL_TEMPLATE_L4_DPORT", YT_IGRACL_TEMPLATE_L4_DPORT },
{"YT_IGRACL_TEMPLATE_L4_SPORT", YT_IGRACL_TEMPLATE_L4_SPORT },
{"YT_IGRACL_TEMPLATE_L4_TYPE", YT_IGRACL_TEMPLATE_L4_TYPE },
{"YT_IGRACL_TEMPLATE_IP_FRAGMENT", YT_IGRACL_TEMPLATE_IP_FRAGMENT },
{"YT_IGRACL_TEMPLATE_IP_1ST_FRAGMENT", YT_IGRACL_TEMPLATE_IP_1ST_FRAGMENT },
{"YT_IGRACL_TEMPLATE_IPV6_DA", YT_IGRACL_TEMPLATE_IPV6_DA },
{"YT_IGRACL_TEMPLATE_IPV6_SA", YT_IGRACL_TEMPLATE_IPV6_SA },
{"YT_IGRACL_TEMPLATE_IP_OPTION", YT_IGRACL_TEMPLATE_IP_OPTION },
{"YT_IGRACL_TEMPLATE_PPPOE_FLAG", YT_IGRACL_TEMPLATE_PPPOE_FLAG },
{"YT_IGRACL_TEMPLATE_TCP_FLAGS", YT_IGRACL_TEMPLATE_TCP_FLAGS },
{"YT_IGRACL_TEMPLATE_IP_PROTOCOL", YT_IGRACL_TEMPLATE_IP_PROTOCOL },
{"YT_IGRACL_TEMPLATE_TOS", YT_IGRACL_TEMPLATE_TOS },
{"YT_IGRACL_TEMPLATE_IS_IGMP", YT_IGRACL_TEMPLATE_IS_IGMP },
{"YT_IGRACL_TEMPLATE_UDF_0", YT_IGRACL_TEMPLATE_UDF_0 },
{"YT_IGRACL_TEMPLATE_UDF_1", YT_IGRACL_TEMPLATE_UDF_1 },
{"YT_IGRACL_TEMPLATE_UDF_2", YT_IGRACL_TEMPLATE_UDF_2 },
{"YT_IGRACL_TEMPLATE_UDF_3", YT_IGRACL_TEMPLATE_UDF_3 },
{"YT_IGRACL_TEMPLATE_UDF_4", YT_IGRACL_TEMPLATE_UDF_4 },
{"YT_IGRACL_TEMPLATE_UDF_5", YT_IGRACL_TEMPLATE_UDF_5 },
{"YT_IGRACL_TEMPLATE_UDF_6", YT_IGRACL_TEMPLATE_UDF_6 },
{"YT_IGRACL_TEMPLATE_UDF_7", YT_IGRACL_TEMPLATE_UDF_7 },
{"YT_IGRACL_TEMPLATE_INNER_CVID", YT_IGRACL_TEMPLATE_INNER_CVID },
{"YT_IGRACL_TEMPLATE_INNER_SVID", YT_IGRACL_TEMPLATE_INNER_SVID },
{"YT_IGRACL_TEMPLATE_INNER_SPRI", YT_IGRACL_TEMPLATE_INNER_SPRI },
{"YT_IGRACL_TEMPLATE_INNER_CPRI", YT_IGRACL_TEMPLATE_INNER_CPRI },
{"YT_IGRACL_TEMPLATE_INNER_SDEI", YT_IGRACL_TEMPLATE_INNER_SDEI },
{"YT_IGRACL_TEMPLATE_INNER_CDEI", YT_IGRACL_TEMPLATE_INNER_CDEI },
{"YT_IGRACL_TEMPLATE_ETHER_TYPE", YT_IGRACL_TEMPLATE_ETHER_TYPE },
{"YT_IGRACL_TEMPLATE_SRC_PORTMASK", YT_IGRACL_TEMPLATE_SRC_PORTMASK },
{"YT_IGRACL_TEMPLATE_MAX", YT_IGRACL_TEMPLATE_MAX },
};
em_str_to_id_t yt_acl_action_type_t_map[] = {
{"ACL_ACT_TYPE_FWD", ACL_ACT_TYPE_FWD },
{"ACL_ACT_TYPE_INTPRI_MAP", ACL_ACT_TYPE_INTPRI_MAP },
{"ACL_ACT_TYPE_VID_REPLACE", ACL_ACT_TYPE_VID_REPLACE },
{"ACL_ACT_TYPE_PRI_REPLACE", ACL_ACT_TYPE_PRI_REPLACE },
{"ACL_ACT_TYPE_VLAN_ASSIGN", ACL_ACT_TYPE_VLAN_ASSIGN },
{"ACL_ACT_TYPE_DSCP_REPLACE", ACL_ACT_TYPE_DSCP_REPLACE },
{"ACL_ACT_TYPE_METER_ASSIGN", ACL_ACT_TYPE_METER_ASSIGN },
{"ACL_ACT_TYPE_FLOWSTAT", ACL_ACT_TYPE_FLOWSTAT },
{"ACL_ACT_TYPE_MIRROR_ENABLE", ACL_ACT_TYPE_MIRROR_ENABLE },
{"ACL_ACT_TYPE_BYPASS_ENABLE", ACL_ACT_TYPE_BYPASS_ENABLE },
{"ACL_ACT_TYPE_MAX", ACL_ACT_TYPE_MAX },
};
em_str_to_id_t yt_ctrlpkt_l2_action_t_map[] = {
{"L2_ACTION_FWD", L2_ACTION_FWD },
{"L2_ACTION_TRAP", L2_ACTION_TRAP },
{"L2_ACTION_DROP", L2_ACTION_DROP },
{"L2_ACTION_COPY", L2_ACTION_COPY },
{"L2_ACTION_END", L2_ACTION_END },
};
em_str_to_id_t yt_port_link_status_t_map[] = {
{"PORT_LINK_DOWN", PORT_LINK_DOWN },
{"PORT_LINK_UP", PORT_LINK_UP },
};
parameters_convert_st yt_port_linkStatus_all_t_list[] = {
{"link_status", offsetof(yt_port_linkStatus_all_t, link_status), 0, T_YT_PORT_LINK_STATUS_T },
{"link_speed", offsetof(yt_port_linkStatus_all_t, link_speed), 0, T_YT_PORT_SPEED_T },
{"link_duplex", offsetof(yt_port_linkStatus_all_t, link_duplex), 0, T_YT_PORT_DUPLEX_T },
{"rx_fc_en", offsetof(yt_port_linkStatus_all_t, rx_fc_en), 0, T_YT_BOOL_T },
{"tx_fc_en", offsetof(yt_port_linkStatus_all_t, tx_fc_en), 0, T_YT_BOOL_T },
};
parameters_convert_st yt_cascade_info_t_list[] = {
{"en", offsetof(yt_cascade_info_t, en), 0, T_YT_ENABLE_T },
{"ports", offsetof(yt_cascade_info_t, ports), 0, T_YT_PORT_CASCADE_T },
};
parameters_convert_st yt_port_force_ctrl_t_list[] = {
{"speed_dup", offsetof(yt_port_force_ctrl_t, speed_dup), 0, T_YT_PORT_SPEED_DUPLEX_T },
{"rx_fc_en", offsetof(yt_port_force_ctrl_t, rx_fc_en), 0, T_YT_BOOL_T },
{"tx_fc_en", offsetof(yt_port_force_ctrl_t, tx_fc_en), 0, T_YT_BOOL_T },
};
em_str_to_id_t yt_extif_mode_t_map[] = {
{"YT_EXTIF_MODE_MII", YT_EXTIF_MODE_MII },
{"YT_EXTIF_MODE_REMII", YT_EXTIF_MODE_REMII },
{"YT_EXTIF_MODE_RMII_MAC", YT_EXTIF_MODE_RMII_MAC },
{"YT_EXTIF_MODE_RMII_PHY", YT_EXTIF_MODE_RMII_PHY },
{"YT_EXTIF_MODE_RGMII", YT_EXTIF_MODE_RGMII },
{"YT_EXTIF_MODE_XMII_DISABLE", YT_EXTIF_MODE_XMII_DISABLE },
{"YT_EXTIF_MODE_SG_MAC", YT_EXTIF_MODE_SG_MAC },
{"YT_EXTIF_MODE_SG_PHY", YT_EXTIF_MODE_SG_PHY },
{"YT_EXTIF_MODE_FIB_1000", YT_EXTIF_MODE_FIB_1000 },
{"YT_EXTIF_MODE_FIB_100", YT_EXTIF_MODE_FIB_100 },
{"YT_EXTIF_MODE_BX2500", YT_EXTIF_MODE_BX2500 },
{"YT_EXTIF_MODE_SGFIB_RSVD", YT_EXTIF_MODE_SGFIB_RSVD },
};
parameters_convert_st yt_port_an_ability_t_list[] = {
{"half_10_en", offsetof(yt_port_an_ability_t, half_10_en), 0, T_YT_BOOL_T },
{"full_10_en", offsetof(yt_port_an_ability_t, full_10_en), 0, T_YT_BOOL_T },
{"half_100_en", offsetof(yt_port_an_ability_t, half_100_en), 0, T_YT_BOOL_T },
{"full_100_en", offsetof(yt_port_an_ability_t, full_100_en), 0, T_YT_BOOL_T },
{"full_1000_en", offsetof(yt_port_an_ability_t, full_1000_en), 0, T_YT_BOOL_T },
{"fc_en", offsetof(yt_port_an_ability_t, fc_en), 0, T_YT_BOOL_T },
{"asyFC_en", offsetof(yt_port_an_ability_t, asyFC_en), 0, T_YT_BOOL_T },
};
em_str_to_id_t yt_port_speed_duplex_t_map[] = {
{"PORT_SPEED_DUP_10HALF", PORT_SPEED_DUP_10HALF },
{"PORT_SPEED_DUP_10FULL", PORT_SPEED_DUP_10FULL },
{"PORT_SPEED_DUP_100HALF", PORT_SPEED_DUP_100HALF },
{"PORT_SPEED_DUP_100FULL", PORT_SPEED_DUP_100FULL },
{"PORT_SPEED_DUP_1000FULL", PORT_SPEED_DUP_1000FULL },
{"PORT_SPEED_DUP_2500FULL", PORT_SPEED_DUP_2500FULL },
{"PORT_SPEED_DUP_END", PORT_SPEED_DUP_END },
};
em_str_to_id_t yt_phy_type_t_map[] = {
{"PHY_INTERNAL", PHY_INTERNAL },
{"PHY_EXTERNAL", PHY_EXTERNAL },
};
parameters_convert_st yt_port_cableDiag_t_list[] = {
{"pair_valid", offsetof(yt_port_cableDiag_t, pair_valid), sizeof(((yt_port_cableDiag_t*)0)->pair_valid)/sizeof(yt_bool_t), T_YT_BOOL_T },
{"pair_status", offsetof(yt_port_cableDiag_t, pair_status), sizeof(((yt_port_cableDiag_t*)0)->pair_status)/sizeof(yt_port_cable_status_t), T_YT_PORT_CABLE_STATUS_T },
{"pair_length", offsetof(yt_port_cableDiag_t, pair_length), sizeof(((yt_port_cableDiag_t*)0)->pair_length)/sizeof(uint16_t), T_UINT16_T },
};
em_str_to_id_t yt_utp_crossover_mode_t_map[] = {
{"YT_UTP_CROSSOVER_MODE_MDI", YT_UTP_CROSSOVER_MODE_MDI },
{"YT_UTP_CROSSOVER_MODE_MDIX", YT_UTP_CROSSOVER_MODE_MDIX },
{"YT_UTP_CROSSOVER_MODE_RESV", YT_UTP_CROSSOVER_MODE_RESV },
{"YT_UTP_CROSSOVER_MODE_AUTO", YT_UTP_CROSSOVER_MODE_AUTO },
};
em_str_to_id_t yt_utp_crossover_status_t_map[] = {
{"YT_UTP_CROSSOVER_STATUS_MDI", YT_UTP_CROSSOVER_STATUS_MDI },
{"YT_UTP_CROSSOVER_STATUS_MDIX", YT_UTP_CROSSOVER_STATUS_MDIX },
};
em_str_to_id_t yt_utp_template_testmode_t_map[] = {
{"YT_UTP_TEMPLATE_TMODE_10M_10MSINE", YT_UTP_TEMPLATE_TMODE_10M_10MSINE },
{"YT_UTP_TEMPLATE_TMODE_10M_PRANDOM", YT_UTP_TEMPLATE_TMODE_10M_PRANDOM },
{"YT_UTP_TEMPLATE_TMODE_10M_LINKPULSE", YT_UTP_TEMPLATE_TMODE_10M_LINKPULSE },
{"YT_UTP_TEMPLATE_TMODE_10M_5MSINE", YT_UTP_TEMPLATE_TMODE_10M_5MSINE },
{"YT_UTP_TEMPLATE_TMODE_10M_NORMAL", YT_UTP_TEMPLATE_TMODE_10M_NORMAL },
{"YT_UTP_TEMPLATE_TMODE_100M_MDI", YT_UTP_TEMPLATE_TMODE_100M_MDI },
{"YT_UTP_TEMPLATE_TMODE_100M_MDIX", YT_UTP_TEMPLATE_TMODE_100M_MDIX },
{"YT_UTP_TEMPLATE_TMODE_1000M_T1", YT_UTP_TEMPLATE_TMODE_1000M_T1 },
{"YT_UTP_TEMPLATE_TMODE_1000M_T2", YT_UTP_TEMPLATE_TMODE_1000M_T2 },
{"YT_UTP_TEMPLATE_TMODE_1000M_T3", YT_UTP_TEMPLATE_TMODE_1000M_T3 },
{"YT_UTP_TEMPLATE_TMODE_1000M_T4", YT_UTP_TEMPLATE_TMODE_1000M_T4 },
{"YT_UTP_TEMPLATE_TMODE_SDS2500M", YT_UTP_TEMPLATE_TMODE_SDS2500M },
{"YT_UTP_TEMPLATE_TMODE_SDS1000M", YT_UTP_TEMPLATE_TMODE_SDS1000M },
};
em_str_to_id_t yt_phy_loopback_mode_t_map[] = {
{"YT_PHY_LOOPBACK_MODE_INTERNAL", YT_PHY_LOOPBACK_MODE_INTERNAL },
{"YT_PHY_LOOPBACK_MODE_EXTERNAL", YT_PHY_LOOPBACK_MODE_EXTERNAL },
{"YT_PHY_LOOPBACK_MODE_REMOTE", YT_PHY_LOOPBACK_MODE_REMOTE },
};
parameters_convert_st yt_mac_addr_t_list[] = {
{"addr", offsetof(yt_mac_addr_t, addr), sizeof(((yt_mac_addr_t*)0)->addr)/sizeof(uint8_t), T_MAC },
};
em_str_to_id_t yt_l2_fdb_type_t_map[] = {
{"FDB_TYPE_INVALID", FDB_TYPE_INVALID },
{"FDB_TYPE_DYNAMIC", FDB_TYPE_DYNAMIC },
{"FDB_TYPE_STATIC", FDB_TYPE_STATIC },
{"FDB_TYPE_PENDING", FDB_TYPE_PENDING },
};
parameters_convert_st l2_ucastMacAddr_info_t_list[] = {
{"port", offsetof(l2_ucastMacAddr_info_t, port), 0, T_YT_PORT_T },
{"vid", offsetof(l2_ucastMacAddr_info_t, vid), 0, T_YT_VLAN_T },
{"macaddr", offsetof(l2_ucastMacAddr_info_t, macaddr), 0, T_YT_MAC_ADDR_T },
{"type", offsetof(l2_ucastMacAddr_info_t, type), 0, T_YT_L2_FDB_TYPE_T },
};
em_str_to_id_t yt_generate_way_t_map[] = {
{"LOOP_DETECT_GENERATE_WAY_HW", LOOP_DETECT_GENERATE_WAY_HW },
{"LOOP_DETECT_GENERATE_WAY_SW", LOOP_DETECT_GENERATE_WAY_SW },
};
parameters_convert_st yt_remote_id_t_list[] = {
{"remoteID", offsetof(yt_remote_id_t, remoteID), 0, T_UINT8_T },
};
parameters_convert_st yt_tx_pkt_t_list[] = {
{"p_pkt_buff", offsetof(yt_tx_pkt_t, p_pkt_buff), 0, T_UINT64 },
{"pkt_len", offsetof(yt_tx_pkt_t, pkt_len), 0, T_UINT32_T },
{"tx_pkt_info", offsetof(yt_tx_pkt_t, tx_pkt_info), 0, T_YT_TX_PKT_INFO_T },
};
em_str_to_id_t yt_cpuport_mode_t_map[] = {
{"CPUPORT_MODE_INTERNAL", CPUPORT_MODE_INTERNAL },
{"CPUPORT_MODE_EXTERNAL", CPUPORT_MODE_EXTERNAL },
};
parameters_convert_st yt_port_rate_mode_t_list[] = {
{"rate_mode", offsetof(yt_port_rate_mode_t, rate_mode), 0, T_YT_RATE_MODE_T },
{"inc_gap", offsetof(yt_port_rate_mode_t, inc_gap), 0, T_YT_BYTE_RATE_GAP_T },
};
parameters_convert_st yt_rate_meter_mode_t_list[] = {
{"meter_mode", offsetof(yt_rate_meter_mode_t, meter_mode), 0, T_YT_METER_MODE_T },
{"rate_mode", offsetof(yt_rate_meter_mode_t, rate_mode), 0, T_YT_RATE_MODE_T },
{"color_mode", offsetof(yt_rate_meter_mode_t, color_mode), 0, T_YT_COLOR_MODE_T },
{"drop_color", offsetof(yt_rate_meter_mode_t, drop_color), 0, T_YT_DROP_COLOR_T },
{"inc_gap", offsetof(yt_rate_meter_mode_t, inc_gap), 0, T_YT_BYTE_RATE_GAP_T },
{"cf_mode", offsetof(yt_rate_meter_mode_t, cf_mode), 0, T_YT_CF_MODE_T },
};
parameters_convert_st yt_qos_two_rate_t_list[] = {
{"eir", offsetof(yt_qos_two_rate_t, eir), 0, T_UINT32_T },
{"cir", offsetof(yt_qos_two_rate_t, cir), 0, T_UINT32_T },
};
parameters_convert_st yt_shaping_mode_t_list[] = {
{"shp_mode", offsetof(yt_shaping_mode_t, shp_mode), 0, T_YT_RATE_MODE_T },
};
parameters_convert_st yt_qid_t_list[] = {
{"port", offsetof(yt_qid_t, port), 0, T_UINT8_T },
{"qtype", offsetof(yt_qid_t, qtype), 0, T_YT_QUEUE_TYPE_T },
{"qid", offsetof(yt_qid_t, qid), 0, T_UINT8_T },
};
parameters_convert_st yt_stat_mib_port_cnt_t_list[] = {
{"RX_BROADCAST", offsetof(yt_stat_mib_port_cnt_t, RX_BROADCAST), 0, T_UINT32 },
{"RX_PAUSE", offsetof(yt_stat_mib_port_cnt_t, RX_PAUSE), 0, T_UINT32 },
{"RX_MULTICAST", offsetof(yt_stat_mib_port_cnt_t, RX_MULTICAST), 0, T_UINT32 },
{"RX_FCS_ERR", offsetof(yt_stat_mib_port_cnt_t, RX_FCS_ERR), 0, T_UINT32 },
{"RX_ALIGNMENT_ERR", offsetof(yt_stat_mib_port_cnt_t, RX_ALIGNMENT_ERR), 0, T_UINT32 },
{"RX_UNDERSIZE", offsetof(yt_stat_mib_port_cnt_t, RX_UNDERSIZE), 0, T_UINT32 },
{"RX_FRAGMENT", offsetof(yt_stat_mib_port_cnt_t, RX_FRAGMENT), 0, T_UINT32 },
{"RX_64B", offsetof(yt_stat_mib_port_cnt_t, RX_64B), 0, T_UINT32 },
{"RX_65_127B", offsetof(yt_stat_mib_port_cnt_t, RX_65_127B), 0, T_UINT32 },
{"RX_128_255B", offsetof(yt_stat_mib_port_cnt_t, RX_128_255B), 0, T_UINT32 },
{"RX_256_511B", offsetof(yt_stat_mib_port_cnt_t, RX_256_511B), 0, T_UINT32 },
{"RX_512_1023B", offsetof(yt_stat_mib_port_cnt_t, RX_512_1023B), 0, T_UINT32 },
{"RX_1024_1518B", offsetof(yt_stat_mib_port_cnt_t, RX_1024_1518B), 0, T_UINT32 },
{"RX_JUMBO", offsetof(yt_stat_mib_port_cnt_t, RX_JUMBO), 0, T_UINT32 },
{"RX_OKBYTE", offsetof(yt_stat_mib_port_cnt_t, RX_OKBYTE), 0, T_UINT64 },
{"RX_NOT_OKBYTE", offsetof(yt_stat_mib_port_cnt_t, RX_NOT_OKBYTE), 0, T_UINT64 },
{"RX_OVERSIZE", offsetof(yt_stat_mib_port_cnt_t, RX_OVERSIZE), 0, T_UINT32 },
{"RX_DISCARD", offsetof(yt_stat_mib_port_cnt_t, RX_DISCARD), 0, T_UINT32 },
{"TX_BROADCAST", offsetof(yt_stat_mib_port_cnt_t, TX_BROADCAST), 0, T_UINT32 },
{"TX_PAUSE", offsetof(yt_stat_mib_port_cnt_t, TX_PAUSE), 0, T_UINT32 },
{"TX_MULTICAST", offsetof(yt_stat_mib_port_cnt_t, TX_MULTICAST), 0, T_UINT32 },
{"TX_UNDERSIZE", offsetof(yt_stat_mib_port_cnt_t, TX_UNDERSIZE), 0, T_UINT32 },
{"TX_64B", offsetof(yt_stat_mib_port_cnt_t, TX_64B), 0, T_UINT32 },
{"TX_65_127B", offsetof(yt_stat_mib_port_cnt_t, TX_65_127B), 0, T_UINT32 },
{"TX_128_255B", offsetof(yt_stat_mib_port_cnt_t, TX_128_255B), 0, T_UINT32 },
{"TX_256_511B", offsetof(yt_stat_mib_port_cnt_t, TX_256_511B), 0, T_UINT32 },
{"TX_512_1023B", offsetof(yt_stat_mib_port_cnt_t, TX_512_1023B), 0, T_UINT32 },
{"TX_1024_1518B", offsetof(yt_stat_mib_port_cnt_t, TX_1024_1518B), 0, T_UINT32 },
{"TX_JUMBO", offsetof(yt_stat_mib_port_cnt_t, TX_JUMBO), 0, T_UINT32 },
{"TX_OKBYTE", offsetof(yt_stat_mib_port_cnt_t, TX_OKBYTE), 0, T_UINT64 },
{"TX_COLLISION", offsetof(yt_stat_mib_port_cnt_t, TX_COLLISION), 0, T_UINT32 },
{"TX_EXCESSIVE_COLLISION", offsetof(yt_stat_mib_port_cnt_t, TX_EXCESSIVE_COLLISION), 0, T_UINT32 },
{"TX_MULTI_COLLISION", offsetof(yt_stat_mib_port_cnt_t, TX_MULTI_COLLISION), 0, T_UINT32 },
{"TX_SINGLE_COLLISION", offsetof(yt_stat_mib_port_cnt_t, TX_SINGLE_COLLISION), 0, T_UINT32 },
{"TX_OK_PKT", offsetof(yt_stat_mib_port_cnt_t, TX_OK_PKT), 0, T_UINT32 },
{"TX_DEFER", offsetof(yt_stat_mib_port_cnt_t, TX_DEFER), 0, T_UINT32 },
{"TX_LATE_COLLISION", offsetof(yt_stat_mib_port_cnt_t, TX_LATE_COLLISION), 0, T_UINT32 },
{"RX_OAM_COUNTER", offsetof(yt_stat_mib_port_cnt_t, RX_OAM_COUNTER), 0, T_UINT32 },
{"TX_OAM_COUNTER", offsetof(yt_stat_mib_port_cnt_t, TX_OAM_COUNTER), 0, T_UINT32 },
{"RX_UNICAST", offsetof(yt_stat_mib_port_cnt_t, RX_UNICAST), 0, T_UINT32 },
{"TX_UNICAST", offsetof(yt_stat_mib_port_cnt_t, TX_UNICAST), 0, T_UINT32 },
};
em_str_to_id_t yt_stat_type_t_map[] = {
{"YT_STATE_TYPE_FLOW", YT_STATE_TYPE_FLOW },
{"YT_STATE_TYPE_CPU_CODE", YT_STATE_TYPE_CPU_CODE },
{"YT_STATE_TYPE_DROP_CODE", YT_STATE_TYPE_DROP_CODE },
{"YT_STATE_TYPE_PORT", YT_STATE_TYPE_PORT },
};
em_str_to_id_t yt_stat_mode_t_map[] = {
{"YT_STATE_MODE_BYTE", YT_STATE_MODE_BYTE },
{"YT_STATE_MODE_PACKET", YT_STATE_MODE_PACKET },
};
em_str_to_id_t yt_switch_chip_t_map[] = {
{"SWCHIP_YT9215SC", SWCHIP_YT9215SC },
{"SWCHIP_YT9215S", SWCHIP_YT9215S },
{"SWCHIP_YT9215RB", SWCHIP_YT9215RB },
{"SWCHIP_YT9215SL", SWCHIP_YT9215SL },
{"SWCHIP_YT9218N", SWCHIP_YT9218N },
{"SWCHIP_YT9218M", SWCHIP_YT9218M },
{"SWCHIP_YT9213NB", SWCHIP_YT9213NB },
{"SWCHIP_YT9214NB", SWCHIP_YT9214NB },
};
em_str_to_id_t yt_l2_type_t_map[] = {
{"L2_ETHV2", L2_ETHV2 },
{"L2_ETHSAP", L2_ETHSAP },
{"L2_ETHSNAP", L2_ETHSNAP },
{"L2_ETHMAX", L2_ETHMAX },
};
em_str_to_id_t yt_vlan_trans_action_t_map[] = {
{"VLAN_TRANS_ACTION_INVALID", VLAN_TRANS_ACTION_INVALID },
{"VLAN_TRANS_ACTION_ADD", VLAN_TRANS_ACTION_ADD },
{"VLAN_TRANS_ACTION_REPLACE", VLAN_TRANS_ACTION_REPLACE },
};
em_str_to_id_t yt_vlan_format_t_map[] = {
{"VLAN_FMT_UNTAGGED", VLAN_FMT_UNTAGGED },
{"VLAN_FMT_PRIO_TAGGED", VLAN_FMT_PRIO_TAGGED },
{"VLAN_FMT_TAGGED", VLAN_FMT_TAGGED },
};
em_str_to_id_t yt_port_speed_t_map[] = {
{"PORT_SPEED_10M", PORT_SPEED_10M },
{"PORT_SPEED_100M", PORT_SPEED_100M },
{"PORT_SPEED_1000M", PORT_SPEED_1000M },
{"PORT_SPEED_2500M", PORT_SPEED_2500M },
{"PORT_SPEED_END", PORT_SPEED_END },
};
em_str_to_id_t yt_port_duplex_t_map[] = {
{"PORT_DUPLEX_HALF", PORT_DUPLEX_HALF },
{"PORT_DUPLEX_FULL", PORT_DUPLEX_FULL },
};
parameters_convert_st yt_port_cascade_t_list[] = {
{"valid", offsetof(yt_port_cascade_t, valid), 0, T_YT_BOOL_T },
{"port_num", offsetof(yt_port_cascade_t, port_num), 0, T_YT_PORT_T },
};
em_str_to_id_t yt_nic_tx_flag_t_map[] = {
{"NIC_TX_BY_DEFAULT", NIC_TX_BY_DEFAULT },
{"NIC_TX_BY_PORTLIST", NIC_TX_BY_PORTLIST },
{"NIC_TX_BY_VLAN", NIC_TX_BY_VLAN },
};
parameters_convert_st yt_tx_pkt_info_t_list[] = {
{"port_mask", offsetof(yt_tx_pkt_info_t, port_mask), 0, T_YT_PORT_MASK_T },
{"vlan_id", offsetof(yt_tx_pkt_info_t, vlan_id), 0, T_YT_VLAN_T },
{"tx_flag", offsetof(yt_tx_pkt_info_t, tx_flag), 0, T_YT_NIC_TX_FLAG_T },
};
em_str_to_id_t yt_rate_mode_t_map[] = {
{"RATE_MODE_BYTE", RATE_MODE_BYTE },
{"RATE_MODE_PACKET", RATE_MODE_PACKET },
};
em_str_to_id_t yt_byte_rate_gap_t_map[] = {
{"BYTE_RATE_GAP_EXCLUDE", BYTE_RATE_GAP_EXCLUDE },
{"BYTE_RATE_GAP_INCLUDE", BYTE_RATE_GAP_INCLUDE },
};
em_str_to_id_t yt_meter_mode_t_map[] = {
{"METER_MODE_RFC4115", METER_MODE_RFC4115 },
{"METER_MODE_RFC2698", METER_MODE_RFC2698 },
};
em_str_to_id_t yt_color_mode_t_map[] = {
{"COLOR_AWARE", COLOR_AWARE },
{"COLOR_BLIND", COLOR_BLIND },
};
em_str_to_id_t yt_drop_color_t_map[] = {
{"DROP_COLOR_GYR", DROP_COLOR_GYR },
{"DROP_COLOR_YR", DROP_COLOR_YR },
{"DROP_COLOR_R", DROP_COLOR_R },
{"DROP_COLOR_NONE", DROP_COLOR_NONE },
};
em_str_to_id_t yt_cf_mode_t_map[] = {
{"CF_MODE_NONE", CF_MODE_NONE },
{"CF_MODE_LEAKY", CF_MODE_LEAKY },
};
em_str_to_id_t yt_queue_type_t_map[] = {
{"UNICAST_QUEUE", UNICAST_QUEUE },
{"MULTICAST_QUEUE", MULTICAST_QUEUE },
};
em_str_to_id_t yt_port_cable_status_t_map[] = {
{"PORT_CABLE_STATUS_OK", PORT_CABLE_STATUS_OK },
{"PORT_CABLE_STATUS_UNKNOWN", PORT_CABLE_STATUS_UNKNOWN },
{"PORT_CABLE_STATUS_SHORT", PORT_CABLE_STATUS_SHORT },
{"PORT_CABLE_STATUS_OPEN", PORT_CABLE_STATUS_OPEN },
{"PORT_CABLE_STATUS_END", PORT_CABLE_STATUS_END },
};
