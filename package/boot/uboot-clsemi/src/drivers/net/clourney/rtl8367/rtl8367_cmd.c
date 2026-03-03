#include <stdio.h>
#include <common.h>
#include <command.h>
#include <net.h>
#include <netdev.h>
#include <miiphy.h>
#include <phy.h>
#include <asm/io.h>
#include <linux/io.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include "rtk_types.h"
#include "port.h"
#include "l2.h"
#include "cpu.h"
#include <stat.h>
#include <miiphy.h>
#include <phy.h>
#define MAX_PRINT_STR_LEN   (2048 + 512)

#define GET_NEXT_PARAM(val) 			if(argc <= 0) { printf("NO many parameters Exist(post:%d,argc:%d)!\n", post, argc); return -1; };argc--; val = argv[post++]
#define NEW_VAL(x,y)  					x##y
#define GET_ID_BY_STR(str, map) 		get_Id_by_str(str, map, sizeof(map)/sizeof(map[0]))
#define PRINT_ID(val)  					print_id(#val, val, NEW_VAL(val,_map), sizeof(NEW_VAL(val,_map))/sizeof(NEW_VAL(val,_map)[0]))
#define CONVERT_PARAMFROMSTR(val) 		post += convert_parameters_from_str(&val, &argc, &argv[post], NEW_VAL(val,_list), sizeof(NEW_VAL(val,_list))/sizeof(NEW_VAL(val,_list)[0]))
#define CONVERT_PARAMTOSTR(val) 		convert_parameters_to_str(&val, NEW_VAL(val,_list), sizeof(NEW_VAL(val,_list))/sizeof(NEW_VAL(val,_list)[0]))
#define GET_NEXT_ID(val, err_id)		char * NEW_VAL(str_,val) = NULL; GET_NEXT_PARAM(NEW_VAL(str_,val));	\
																   NEW_VAL(val,_t) val = get_Id_by_str(NEW_VAL(str_,val), \
																		   NEW_VAL(val,_map), \
																		   sizeof(NEW_VAL(val,_map))/sizeof(NEW_VAL(val,_map)[0]));\
																		   if (val == err_id) { \
																			   printf("Unkown "#val"[%s]\n", NEW_VAL(str_,val)); \
																			   return -1; \
																		   }
#define PRINT_HEX_PARAM(val) printf("%20s:%#20x\n", #val , val)

typedef struct {
	char * name;
	int  offset;
	int  type;
} parameters_convert_st;

enum {
	T_UINT32,
	T_MAC,
	T_IPADDR,
	T_UINT64,
};

enum {
	RTK_PORT_PHYAUTONEGOABILITY_SET_ID,
	RTK_PORT_PHYAUTONEGOABILITY_GET_ID,
	RTK_PORT_PHYFORCEMODEABILITY_SET_ID,
	RTK_PORT_PHYFORCEMODEABILITY_GET_ID,
	RTK_PORT_PHYSTATUS_GET_ID,
	RTK_PORT_MACFORCELINK_SET_ID,
	RTK_PORT_MACFORCELINK_GET_ID,
	RTK_PORT_MACFORCELINKEXT_SET_ID,
	RTK_PORT_MACFORCELINKEXT_GET_ID,
	RTK_PORT_MACSTATUS_GET_ID,
	RTK_PORT_MACLOCALLOOPBACKENABLE_SET_ID,
	RTK_PORT_MACLOCALLOCALLOOPBACKENABLE_GET_ID,
	RTK_PORT_PHYREG_SET_ID,
	RTK_PORT_PHYREG_GET_ID,
	RTK_PORT_PHYENABLEALL_SET_ID,
	RTK_PORT_PHYENABLEALL_GET_ID,
	RTK_PORT_PHYCOMBOPORTMEDIA_SET_ID,
	RTK_PORT_PHYCOMBOPORTMEDIA_GET_ID,
	RTK_PORT_SGMIILINKSTATUS_GET_ID,
	RTK_PORT_SGMIINWAY_SET_ID,
	RTK_PORT_SGMIINWAY_GET_ID,
	RTK_PORT_PHYMDX_SET_ID,
	RTK_PORT_PHYMDX_GET_ID,
	RTK_PORT_PHYMDXSTATUS_GET_ID,
	RTK_PORT_MAXPACKETLENGTH_SET_ID,
	RTK_PORT_MAXPACKETLENGTH_GET_ID,
	RTK_CPU_ENABLE_SET_ID,
	RTK_CPU_ENABLE_GET_ID,
	RTK_L2_INIT_ID,
	RTK_L2_ADDR_ADD_ID,
	RTK_L2_ADDR_GET_ID,
	RTK_L2_ADDR_NEXT_GET_ID,
	RTK_L2_ADDR_NEXT_DEL_ID,
	RTK_L2_FLOODPORTMASK_SET_ID,
	RTK_L2_FLOODPORTMASK_GET_ID,
	RTK_L2_LOCALPKTPERMIT_SET_ID,
	RTK_L2_LOCALPKTPERMIT_GET_ID,
	RTK_L2_ENTRY_GET_ID,
	RTK_REG_READ,
	RTK_REG_WRITE,
	RTK_PORT_RGMIIDELAYEXT_GET_ID,
	RTK_PORT_RGMIIDELAYEXT_SET_ID,
	RTK_STAT_PORT_GETALL_ID,
	RTK_STAT_PORT_RESET_ID,
	CMD_MAX,
};

char * func_str_list [] = {
	"rtk_port_phyAutoNegoAbility_set",
	"rtk_port_phyAutoNegoAbility_get",
	"rtk_port_phyForceModeAbility_set",
	"rtk_port_phyForceModeAbility_get",
	"rtk_port_phyStatus_get",
	"rtk_port_macForceLink_set",
	"rtk_port_macForceLink_get",
	"rtk_port_macForceLinkExt_set",
	"rtk_port_macForceLinkExt_get",
	"rtk_port_macStatus_get",
	"rtk_port_macLocaloopbackEnable_set",
	"rtk_port_macLocalLoopbackEnable_get",
	"rtk_port_phyReg_set",
	"rtk_port_phyReg_get",
	"rtk_port_phyEnableAll_set",
	"rtk_port_phyEnableAll_get",
	"rtk_port_phyComboPortMedia_set",
	"rtk_port_phyComboPortMedia_get",
	"rtk_port_sgmiiLinkStatus_get",
	"rtk_port_sgmiiNway_set",
	"rtk_port_sgmiiNway_get",
	"rtk_port_phyMdx_set",
	"rtk_port_phyMdx_get",
	"rtk_port_phyMdxStatus_get",
	"rtk_port_maxPacketLength_set",
	"rtk_port_maxPacketLength_get",
	"rtk_cpu_enable_set",
	"rtk_cpu_enable_get",
	"rtk_l2_init",
	"rtk_l2_addr_add",
	"rtk_l2_addr_get",
	"rtk_l2_addr_next_get",
	"rtk_l2_addr_next_del",
	"rtk_l2_floodPortMask_set",
	"rtk_l2_floodPortMask_get",
	"rtk_l2_localPktPermit_set",
	"rtk_l2_localPktPermit_get",
	"rtk_l2_entry_get",
	"regRd",
	"regWr",
	"rtk_port_rgmiiDelayExt_get",
	"rtk_port_rgmiiDelayExt_set",
	"rtk_stat_port_getAll",
	"rtk_stat_port_reset",
};

typedef struct {
	char * name;
	int id;
} em_str_to_id_t;

em_str_to_id_t rtk_port_map[]= {
	{"UTP_PORT0",UTP_PORT0},
	{"UTP_PORT1",UTP_PORT1},
	{"UTP_PORT2",UTP_PORT2},
	{"UTP_PORT3",UTP_PORT3},
	{"UTP_PORT4",UTP_PORT4},
	{"UTP_PORT5",UTP_PORT5},
	{"UTP_PORT6",UTP_PORT6},
	{"UTP_PORT7",UTP_PORT7},

	{"EXT_PORT0",EXT_PORT0},
	{"EXT_PORT1",EXT_PORT1},
	{"EXT_PORT2",EXT_PORT2},
	{"UNDEFINE_PORT",UNDEFINE_PORT},

};

em_str_to_id_t rtk_port_speed_map[] = {
	{"PORT_SPEED_10M", PORT_SPEED_10M},
	{"PORT_SPEED_100M", PORT_SPEED_100M},
	{"PORT_SPEED_1000M", PORT_SPEED_1000M},
	{"PORT_SPEED_500M", PORT_SPEED_500M},
	{"PORT_SPEED_2500M", PORT_SPEED_2500M},
	{"PORT_SPEED_END",  PORT_SPEED_END}

};

em_str_to_id_t rtk_port_duplex_map[] = {
	{"PORT_HALF_DUPLEX",PORT_HALF_DUPLEX},
	{"PORT_FULL_DUPLEX",PORT_FULL_DUPLEX},
	{"PORT_DUPLEX_END",PORT_DUPLEX_END},
};


em_str_to_id_t rtk_port_linkStatus_map[] = {
	{"PORT_LINKDOWN", PORT_LINKDOWN},
	{"PORT_LINKUP", PORT_LINKUP},
	{"PORT_SPEED_END", PORT_SPEED_END}
};

em_str_to_id_t rtk_mode_ext_map[] = {
	{"MODE_EXT_DISABLE",MODE_EXT_DISABLE},
	{"MODE_EXT_RGMII",MODE_EXT_RGMII},
	{"MODE_EXT_MII_MAC",MODE_EXT_MII_MAC},
	{"MODE_EXT_MII_PHY",MODE_EXT_MII_PHY},
	{"MODE_EXT_TMII_MAC",MODE_EXT_TMII_MAC},
	{"MODE_EXT_TMII_PHY",MODE_EXT_TMII_PHY},
	{"MODE_EXT_GMII",MODE_EXT_GMII},
	{"MODE_EXT_RMII_MAC",MODE_EXT_RMII_MAC},
	{"MODE_EXT_RMII_PHY",MODE_EXT_RMII_PHY},
	{"MODE_EXT_SGMII",MODE_EXT_SGMII},
	{"MODE_EXT_HSGMII",MODE_EXT_HSGMII},
	{"MODE_EXT_1000X_100FX",MODE_EXT_1000X_100FX},
	{"MODE_EXT_1000X",MODE_EXT_1000X},
	{"MODE_EXT_100FX",MODE_EXT_100FX},
	{"MODE_EXT_RGMII_2",MODE_EXT_RGMII_2},
	{"MODE_EXT_MII_MAC_2",MODE_EXT_MII_MAC_2},
	{"MODE_EXT_MII_PHY_2",MODE_EXT_MII_PHY_2},
	{"MODE_EXT_TMII_MAC_2",MODE_EXT_TMII_MAC_2},
	{"MODE_EXT_TMII_PHY_2",MODE_EXT_TMII_PHY_2},
	{"MODE_EXT_RMII_MAC_2",MODE_EXT_RMII_MAC_2},
	{"MODE_EXT_RMII_PHY_2",MODE_EXT_RMII_PHY_2},
	{"MODE_EXT_FIBER_2P5G",MODE_EXT_FIBER_2P5G},
	{"MODE_EXT_END", MODE_EXT_END}
};

em_str_to_id_t rtk_enable_map[] = {
	{"DISABLED",    DISABLED},
	{"ENABLED",    ENABLED},
	{"RTK_ENABLE_END", RTK_ENABLE_END}
};

em_str_to_id_t rtk_l2_read_method_map[] = {
	{"READMETHOD_MAC",READMETHOD_MAC},
	{"READMETHOD_ADDRESS",READMETHOD_ADDRESS},
	{"READMETHOD_NEXT_ADDRESS",READMETHOD_NEXT_ADDRESS},
	{"READMETHOD_NEXT_L2UC",READMETHOD_NEXT_L2UC},
	{"READMETHOD_NEXT_L2MC",READMETHOD_NEXT_L2MC},
	{"READMETHOD_NEXT_L3MC",READMETHOD_NEXT_L3MC},
	{"READMETHOD_NEXT_L2L3MC",READMETHOD_NEXT_L2L3MC},
	{"READMETHOD_NEXT_L2UCSPA",READMETHOD_NEXT_L2UCSPA},
	{"READMETHOD_END", READMETHOD_END}
};

em_str_to_id_t rtk_l2_flood_type_map[] = {
	{"FLOOD_UNKNOWNDA",FLOOD_UNKNOWNDA},
	{"FLOOD_UNKNOWNMC",FLOOD_UNKNOWNMC},
	{"FLOOD_BC",FLOOD_BC},
	{"FLOOD_END", FLOOD_END}
};

em_str_to_id_t  rtk_port_phy_mdix_mode_map[] = {
	{"PHY_AUTO_CROSSOVER_MODE",PHY_AUTO_CROSSOVER_MODE},
	{"PHY_FORCE_MDI_MODE",PHY_FORCE_MDI_MODE},
	{"PHY_FORCE_MDIX_MODE",PHY_FORCE_MDIX_MODE},
	{"PHY_FORCE_MODE_END", PHY_FORCE_MODE_END}
};

em_str_to_id_t  rtk_port_phy_mdix_status_map[] = {
	{"PHY_STATUS_AUTO_MDI_MODE",PHY_STATUS_AUTO_MDI_MODE},
	{"PHY_STATUS_AUTO_MDIX_MODE",PHY_STATUS_AUTO_MDIX_MODE},
	{"PHY_STATUS_FORCE_MDI_MODE",PHY_STATUS_FORCE_MDI_MODE},
	{"PHY_STATUS_FORCE_MDIX_MODE",PHY_STATUS_FORCE_MDIX_MODE},
	{"PHY_STATUS_FORCE_MODE_END", PHY_STATUS_FORCE_MODE_END}
};

em_str_to_id_t  rtk_port_media_map[] = {
	{"PORT_MEDIA_COPPER",PORT_MEDIA_COPPER},
	{"PORT_MEDIA_FIBER",PORT_MEDIA_FIBER},
	{"PORT_MEDIA_AUTO",PORT_MEDIA_AUTO},
	{"PORT_MEDIA_RGMII",PORT_MEDIA_RGMII},
	{"PORT_MEDIA_END", PORT_MEDIA_END}
};

em_str_to_id_t  rtk_port_phy_reg_map[] = {
	{"PHY_REG_CONTROL",PHY_REG_CONTROL},
	{"PHY_REG_STATUS",PHY_REG_STATUS},
	{"PHY_REG_IDENTIFIER_1",PHY_REG_IDENTIFIER_1},
	{"PHY_REG_IDENTIFIER_2",PHY_REG_IDENTIFIER_2},
	{"PHY_REG_AN_ADVERTISEMENT",PHY_REG_AN_ADVERTISEMENT},
	{"PHY_REG_AN_LINKPARTNER",PHY_REG_AN_LINKPARTNER},
	{"PHY_REG_1000_BASET_CONTROL",PHY_REG_1000_BASET_CONTROL},
	{"PHY_REG_1000_BASET_STATUS",PHY_REG_1000_BASET_STATUS},
	{"PHY_REG_END", PHY_REG_END}
};

parameters_convert_st rtk_port_phy_ability_list[] = {
	{"autonegotiation",	offsetof(rtk_port_phy_ability_t,AutoNegotiation),	T_UINT32},
	{"half_10",			offsetof(rtk_port_phy_ability_t,Half_10),	 		T_UINT32},
	{"full_10",			offsetof(rtk_port_phy_ability_t,Full_10),	 		T_UINT32},
	{"half_100",		offsetof(rtk_port_phy_ability_t,Half_100),	 		T_UINT32},
	{"full_100",		offsetof(rtk_port_phy_ability_t,Full_100),	        T_UINT32},
	{"full_1000",		offsetof(rtk_port_phy_ability_t,Full_1000),	 		T_UINT32},
	{"fc",				offsetof(rtk_port_phy_ability_t,FC),	 	        T_UINT32},
	{"asyfc",			offsetof(rtk_port_phy_ability_t,AsyFC),	            T_UINT32},
};

parameters_convert_st rtk_port_mac_ability_list[] = {
	{"forcemode",	offsetof(rtk_port_mac_ability_t,forcemode),	 T_UINT32 },
	{"speed",		offsetof(rtk_port_mac_ability_t,speed),	 T_UINT32 },
	{"duplex",		offsetof(rtk_port_mac_ability_t,duplex),	 T_UINT32 },
	{"link",		offsetof(rtk_port_mac_ability_t,link),	 T_UINT32 },
	{"nway",		offsetof(rtk_port_mac_ability_t,nway),	 T_UINT32 },
	{"txpause",		offsetof(rtk_port_mac_ability_t,txpause),	 T_UINT32 },
	{"rxpause",		offsetof(rtk_port_mac_ability_t,rxpause),	 T_UINT32 },
};

parameters_convert_st rtk_l2_ucastAddr_list[] = {
	{"mac",	offsetof(rtk_l2_ucastAddr_t,mac),	 T_MAC },
	{"ivl",	offsetof(rtk_l2_ucastAddr_t,ivl),	 T_UINT32 },
	{"cvid",	offsetof(rtk_l2_ucastAddr_t,cvid),	 T_UINT32 },
	{"fid",	offsetof(rtk_l2_ucastAddr_t,fid),	 T_UINT32 },
	{"efid",	offsetof(rtk_l2_ucastAddr_t,efid),	 T_UINT32 },
	{"port",	offsetof(rtk_l2_ucastAddr_t,port),	 T_UINT32 },
	{"sa_block",	offsetof(rtk_l2_ucastAddr_t,sa_block),	 T_UINT32 },
	{"da_block",	offsetof(rtk_l2_ucastAddr_t,da_block),	 T_UINT32 },
	{"auth",	offsetof(rtk_l2_ucastAddr_t,auth),	 T_UINT32 },
	{"is_static",	offsetof(rtk_l2_ucastAddr_t,is_static),	 T_UINT32 },
	{"priority",	offsetof(rtk_l2_ucastAddr_t,priority),	 T_UINT32 },
	{"sa_pri_en",	offsetof(rtk_l2_ucastAddr_t,sa_pri_en),	 T_UINT32 },
	{"fwd_pri_en",	offsetof(rtk_l2_ucastAddr_t,fwd_pri_en),	 T_UINT32 },
	{"address",	offsetof(rtk_l2_ucastAddr_t,address),	 T_UINT32 },
};

parameters_convert_st rtk_mac_list[] = {
	{"mac" , offsetof(rtk_mac_t, octet), T_MAC}
};

parameters_convert_st rtk_portmask_list[] = {
	{"mask" , offsetof(rtk_portmask_t, bits), T_UINT32}
};


parameters_convert_st rtk_l2_addr_table_list[] = {
	{"index",	offsetof(rtk_l2_addr_table_t, index),	     T_UINT32 },
	{"sip",	offsetof(rtk_l2_addr_table_t, sip),	     T_IPADDR   },
	{"dip",	offsetof(rtk_l2_addr_table_t, dip),	     T_IPADDR   },
	{"mac",	offsetof(rtk_l2_addr_table_t, mac),	     T_MAC  },
	{"sa_block",	offsetof(rtk_l2_addr_table_t, sa_block),	     T_UINT32 },
	{"auth",	offsetof(rtk_l2_addr_table_t, auth),	     T_UINT32 },
	{"portmask",	offsetof(rtk_l2_addr_table_t, portmask),	     T_UINT32 },
	{"age",	offsetof(rtk_l2_addr_table_t, age),	     T_UINT32 },
	{"ivl",	offsetof(rtk_l2_addr_table_t, ivl),	     T_UINT32 },
	{"cvid",	offsetof(rtk_l2_addr_table_t, cvid),	     T_UINT32 },
	{"fid",	offsetof(rtk_l2_addr_table_t, fid),	     T_UINT32 },
	{"is_ipmul",	offsetof(rtk_l2_addr_table_t, is_ipmul),	     T_UINT32 },
	{"is_static",	offsetof(rtk_l2_addr_table_t, is_static),	     T_UINT32 },
	{"is_ipvidmul",	offsetof(rtk_l2_addr_table_t, is_ipvidmul),	     T_UINT32 },
	{"l3_vid",	offsetof(rtk_l2_addr_table_t, l3_vid),	     T_UINT32 },
};

parameters_convert_st rtk_stat_port_cntr_list[] = {
{"ifInOctets", offsetof(rtk_stat_port_cntr_t, ifInOctets), T_UINT64 },
{"dot3StatsFCSErrors", offsetof(rtk_stat_port_cntr_t, dot3StatsFCSErrors), T_UINT32 },
{"dot3StatsSymbolErrors", offsetof(rtk_stat_port_cntr_t, dot3StatsSymbolErrors), T_UINT32 },
{"dot3InPauseFrames", offsetof(rtk_stat_port_cntr_t, dot3InPauseFrames), T_UINT32 },
{"dot3ControlInUnknownOpcodes", offsetof(rtk_stat_port_cntr_t, dot3ControlInUnknownOpcodes), T_UINT32 },
{"etherStatsFragments", offsetof(rtk_stat_port_cntr_t, etherStatsFragments), T_UINT32 },
{"etherStatsJabbers", offsetof(rtk_stat_port_cntr_t, etherStatsJabbers), T_UINT32 },
{"ifInUcastPkts", offsetof(rtk_stat_port_cntr_t, ifInUcastPkts), T_UINT32 },
{"etherStatsDropEvents", offsetof(rtk_stat_port_cntr_t, etherStatsDropEvents), T_UINT32 },
{"etherStatsOctets", offsetof(rtk_stat_port_cntr_t, etherStatsOctets), T_UINT64 },
{"etherStatsUndersizePkts", offsetof(rtk_stat_port_cntr_t, etherStatsUndersizePkts), T_UINT32 },
{"etherStatsOversizePkts", offsetof(rtk_stat_port_cntr_t, etherStatsOversizePkts), T_UINT32 },
{"etherStatsPkts64Octets", offsetof(rtk_stat_port_cntr_t, etherStatsPkts64Octets), T_UINT32 },
{"etherStatsPkts65to127Octets", offsetof(rtk_stat_port_cntr_t, etherStatsPkts65to127Octets), T_UINT32 },
{"etherStatsPkts128to255Octets", offsetof(rtk_stat_port_cntr_t, etherStatsPkts128to255Octets), T_UINT32 },
{"etherStatsPkts256to511Octets", offsetof(rtk_stat_port_cntr_t, etherStatsPkts256to511Octets), T_UINT32 },
{"etherStatsPkts512to1023Octets", offsetof(rtk_stat_port_cntr_t, etherStatsPkts512to1023Octets), T_UINT32 },
{"etherStatsPkts1024toMaxOctets", offsetof(rtk_stat_port_cntr_t, etherStatsPkts1024toMaxOctets), T_UINT32 },
{"etherStatsMcastPkts", offsetof(rtk_stat_port_cntr_t, etherStatsMcastPkts), T_UINT32 },
{"etherStatsBcastPkts", offsetof(rtk_stat_port_cntr_t, etherStatsBcastPkts), T_UINT32 },
{"ifOutOctets", offsetof(rtk_stat_port_cntr_t, ifOutOctets), T_UINT64 },
{"dot3StatsSingleCollisionFrames", offsetof(rtk_stat_port_cntr_t, dot3StatsSingleCollisionFrames), T_UINT32 },
{"dot3StatsMultipleCollisionFrames", offsetof(rtk_stat_port_cntr_t, dot3StatsMultipleCollisionFrames), T_UINT32 },
{"dot3StatsDeferredTransmissions", offsetof(rtk_stat_port_cntr_t, dot3StatsDeferredTransmissions), T_UINT32 },
{"dot3StatsLateCollisions", offsetof(rtk_stat_port_cntr_t, dot3StatsLateCollisions), T_UINT32 },
{"etherStatsCollisions", offsetof(rtk_stat_port_cntr_t, etherStatsCollisions), T_UINT32 },
{"dot3StatsExcessiveCollisions", offsetof(rtk_stat_port_cntr_t, dot3StatsExcessiveCollisions), T_UINT32 },
{"dot3OutPauseFrames", offsetof(rtk_stat_port_cntr_t, dot3OutPauseFrames), T_UINT32 },
{"dot1dBasePortDelayExceededDiscards", offsetof(rtk_stat_port_cntr_t, dot1dBasePortDelayExceededDiscards), T_UINT32 },
{"dot1dTpPortInDiscards", offsetof(rtk_stat_port_cntr_t, dot1dTpPortInDiscards), T_UINT32 },
{"ifOutUcastPkts", offsetof(rtk_stat_port_cntr_t, ifOutUcastPkts), T_UINT32 },
{"ifOutMulticastPkts", offsetof(rtk_stat_port_cntr_t, ifOutMulticastPkts), T_UINT32 },
{"ifOutBrocastPkts", offsetof(rtk_stat_port_cntr_t, ifOutBrocastPkts), T_UINT32 },
{"outOampduPkts", offsetof(rtk_stat_port_cntr_t, outOampduPkts), T_UINT32 },
{"inOampduPkts", offsetof(rtk_stat_port_cntr_t, inOampduPkts), T_UINT32 },
{"pktgenPkts", offsetof(rtk_stat_port_cntr_t, pktgenPkts), T_UINT32 },
{"inMldChecksumError", offsetof(rtk_stat_port_cntr_t, inMldChecksumError), T_UINT32 },
{"inIgmpChecksumError", offsetof(rtk_stat_port_cntr_t, inIgmpChecksumError), T_UINT32 },
{"inMldSpecificQuery", offsetof(rtk_stat_port_cntr_t, inMldSpecificQuery), T_UINT32 },
{"inMldGeneralQuery", offsetof(rtk_stat_port_cntr_t, inMldGeneralQuery), T_UINT32 },
{"inIgmpSpecificQuery", offsetof(rtk_stat_port_cntr_t, inIgmpSpecificQuery), T_UINT32 },
{"inIgmpGeneralQuery", offsetof(rtk_stat_port_cntr_t, inIgmpGeneralQuery), T_UINT32 },
{"inMldLeaves", offsetof(rtk_stat_port_cntr_t, inMldLeaves), T_UINT32 },
{"inIgmpLeaves", offsetof(rtk_stat_port_cntr_t, inIgmpLeaves), T_UINT32 },
{"inIgmpJoinsSuccess", offsetof(rtk_stat_port_cntr_t, inIgmpJoinsSuccess), T_UINT32 },
{"inIgmpJoinsFail", offsetof(rtk_stat_port_cntr_t, inIgmpJoinsFail), T_UINT32 },
{"inMldJoinsSuccess", offsetof(rtk_stat_port_cntr_t, inMldJoinsSuccess), T_UINT32 },
{"inMldJoinsFail", offsetof(rtk_stat_port_cntr_t, inMldJoinsFail), T_UINT32 },
{"inReportSuppressionDrop", offsetof(rtk_stat_port_cntr_t, inReportSuppressionDrop), T_UINT32 },
{"inLeaveSuppressionDrop", offsetof(rtk_stat_port_cntr_t, inLeaveSuppressionDrop), T_UINT32 },
{"outIgmpReports", offsetof(rtk_stat_port_cntr_t, outIgmpReports), T_UINT32 },
{"outIgmpLeaves", offsetof(rtk_stat_port_cntr_t, outIgmpLeaves), T_UINT32 },
{"outIgmpGeneralQuery", offsetof(rtk_stat_port_cntr_t, outIgmpGeneralQuery), T_UINT32 },
{"outIgmpSpecificQuery", offsetof(rtk_stat_port_cntr_t, outIgmpSpecificQuery), T_UINT32 },
{"outMldReports", offsetof(rtk_stat_port_cntr_t, outMldReports), T_UINT32 },
{"outMldLeaves", offsetof(rtk_stat_port_cntr_t, outMldLeaves), T_UINT32 },
{"outMldGeneralQuery", offsetof(rtk_stat_port_cntr_t, outMldGeneralQuery), T_UINT32 },
{"outMldSpecificQuery", offsetof(rtk_stat_port_cntr_t, outMldSpecificQuery), T_UINT32 },
{"inKnownMulticastPkts", offsetof(rtk_stat_port_cntr_t, inKnownMulticastPkts), T_UINT32 },
{"ifInMulticastPkts", offsetof(rtk_stat_port_cntr_t, ifInMulticastPkts), T_UINT32 },
{"ifInBroadcastPkts", offsetof(rtk_stat_port_cntr_t, ifInBroadcastPkts), T_UINT32 },
{"ifOutDiscards", offsetof(rtk_stat_port_cntr_t, ifOutDiscards), T_UINT32 },
};
extern int g_debug_cls;
extern struct mii_dev * mii_bus_list[5];
int get_Id_by_str(char * str, em_str_to_id_t * map, int map_size)
{
	int i;
	for (i=0; i< map_size - 1; i++) {
		if (0 == strcasecmp(map[i].name, str))
			return map[i].id;
	}

	return map[i].id;
}

void print_id(char * name , int id, em_str_to_id_t * map, int map_size)
{
	int i;
	for (i=0; i< map_size - 1; i++)
		if (map[i].id == id)
			break;

	printf("%40s:%40s\n", name, map[i].name);
}

void paramter_set(int type, char * address, char * value_str)
{

	switch (type)
	{
	case T_UINT32:
		{
			rtk_uint32 value = simple_strtoul(value_str, NULL, 0);
			memcpy(address, &value,  sizeof(rtk_uint32));
		}
		break;
	case T_UINT64:
		{
			rtk_uint64 value = simple_strtoull(value_str, NULL, 0);
			memcpy(address, &value,  sizeof(rtk_uint64));
		}
		break;
	case T_MAC:
		{

			rtk_mac_t  * mac = (rtk_mac_t *) address;
		//	if (sscanf(value_str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &mac->octet[5], &mac->octet[4], &mac->octet[3], &mac->octet[2], &mac->octet[1], &mac->octet[0]) != 6) {
			if (sscanf(value_str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &mac->octet[0], &mac->octet[1], &mac->octet[2], &mac->octet[3], &mac->octet[4], &mac->octet[5]) != 6) {
				printf("Can't convert MAC String(%s) to ether_addr!!!\n", value_str);
				return;
			}
		}
		break;
	case T_IPADDR:
		{
			unsigned char * ip  = (unsigned char *) address;
			if (sscanf(value_str, "%hhu.%hhu.%hhu.%hhu", ip+3, ip+2, ip+1, ip) != 4) {
				printf("Can't convert ipaddr to string: %s\n", value_str);
				return;
			}
		}
		break;
	default:
		printf("Unkown Parameter type[%d] V:[%s]!\n", type, value_str);
		break;
	}
}

int convert_parameters_from_str(void * obj, int * pargc, char ** argv, parameters_convert_st * paramter_list, int list_size)
{
	int index;
	int count = 0;

	for (index = 0; index < *pargc - 1; index +=2) {
		int param_index = 0;
		if (*pargc - count < 2) {
			printf("argc is end\n");
			goto END;
		}

		for (param_index = 0; param_index < list_size; param_index ++) {
			if ( 0 == strcasecmp(argv[index], paramter_list[param_index].name)) {
				printf("set para [%s] value[%s]\n",argv[index], argv[index + 1]);
				paramter_set(paramter_list[param_index].type, ((char *) (obj)) +  paramter_list[param_index].offset, argv[index + 1]);
				count += 2;
				break;
			}

			if (param_index == list_size) {
				printf("Unkown Parameter [%s]\n", argv[index]);
				break;
			}

		}
	}

END:
    *pargc -= count;

	return count;
}


int paramter_print(void * buf, int type, char * address, char * name)
{

	switch (type)
	{
	case T_UINT32:
		{
			//return sprintf(buf, "%35s: %u\n", name, *((rtk_uint32 *) address));
			return printf("%35s: %u\n", name, *((rtk_uint32 *) address));
		}
		break;
	case T_UINT64:
		{
			//return sprintf(buf, "%35s: %llu\n", name, *((rtk_uint64 *) address));
			return printf("%35s: %llu\n", name, *((rtk_uint64 *) address));
		}
		break;
	case T_MAC:
		{
			char mac_str[20] = {0};
			rtk_mac_t  * mac = (rtk_mac_t *) address;

			//snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x", mac->octet[5], mac->octet[4], mac->octet[3], mac->octet[2], mac->octet[1], mac->octet[0]);
			snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x", mac->octet[0], mac->octet[1], mac->octet[2], mac->octet[3], mac->octet[4], mac->octet[5]);
			//return sprintf(buf, "%20s:%20s\n", name, mac_str);
			return printf("%20s:%20s\n", name, mac_str);
		}
		break;
	case T_IPADDR:
		{

			char ip_str[20] = {0};
	        unsigned char * ip = (unsigned char *) address;

			snprintf(ip_str, sizeof(ip_str), "%d.%d.%d.%d", ip[3], ip[2], ip[1], ip[0]);

			//return sprintf(buf, "%20s:%20s\n", name, ip_str);
			return printf("%20s:%20s\n", name, ip_str);
		}
		break;
	default:
		printf("Unkown Parameter type[%d] Name:[%s]!\n", type, name);
		break;
	}
	return 0;
}


char * convert_parameters_to_str(void * obj, parameters_convert_st * paramter_list, int list_size)
{
//	static char buf[MAX_PRINT_STR_LEN] = {0};
	 char buf[1] = {0};
	int offset = 0, param_index ;
	memset(buf, 0, sizeof(buf));

	for (param_index  = 0; param_index  < list_size; param_index ++) {
		offset += paramter_print(buf + offset, paramter_list[param_index].type, ((char *) (obj)) +  paramter_list[param_index].offset, paramter_list[param_index].name);
	}
//	printf("%s\n", buf);
	return buf;
}

static int do_switch(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	int func_id, post = 0 ;
	char * cmd  = NULL;
    static int is_initd = false;

	GET_NEXT_PARAM(cmd);
	GET_NEXT_PARAM(cmd);

	for (func_id = 0; func_id < sizeof(func_str_list)/sizeof(char*); func_id++) {
		if ( 0 == strcasecmp(func_str_list[func_id], cmd))
			break;
	}

	if ( func_id == CMD_MAX) {
		printf("Can't find the cmd[%s]\n", cmd);
		return 0;
	}

	if (!is_initd) {
		rtk_api_ret_t ret;
		ret = rtk_switch_init();
		printf("rtk_switch_init ret = %d\n", ret);
		is_initd = true;
	}

	switch(func_id)
	{
	case RTK_PORT_PHYAUTONEGOABILITY_SET_ID:
		{
			rtk_api_ret_t ret;

			rtk_port_phy_ability_t rtk_port_phy_ability = {0};

			printf("is rtk_port_phyAutoNegoAbility_set command!\n");

			GET_NEXT_ID(rtk_port, UNDEFINE_PORT);

			ret = rtk_port_phyAutoNegoAbility_get(rtk_port, &rtk_port_phy_ability);
			if(g_debug_cls) {
				printf("read==)ret:%d\n", ret);
				CONVERT_PARAMTOSTR(rtk_port_phy_ability);
				printf("<===END\n");
			}

			CONVERT_PARAMFROMSTR(rtk_port_phy_ability);

			ret = rtk_port_phyAutoNegoAbility_set(rtk_port, &rtk_port_phy_ability);
			printf("ret:%d\n", ret);
			CONVERT_PARAMTOSTR(rtk_port_phy_ability);
		}
		break;
	case RTK_PORT_PHYAUTONEGOABILITY_GET_ID:
		{
			rtk_api_ret_t ret;
			rtk_port_phy_ability_t rtk_port_phy_ability = {0};

			printf("is rtk_port_phyAutoNegoAbility_get command!\n");

			GET_NEXT_ID(rtk_port, UNDEFINE_PORT);

			ret = rtk_port_phyAutoNegoAbility_get(rtk_port, &rtk_port_phy_ability);
			CONVERT_PARAMTOSTR(rtk_port_phy_ability);
		}
		break;
	case RTK_PORT_PHYFORCEMODEABILITY_SET_ID:
		{
			rtk_api_ret_t ret;
			rtk_port_phy_ability_t rtk_port_phy_ability = {0};

			printf("is rtk_port_phyForceModeAbility_set command!\n");

			GET_NEXT_ID(rtk_port, UNDEFINE_PORT);

			ret = rtk_port_phyForceModeAbility_get(rtk_port, &rtk_port_phy_ability);
			if(g_debug_cls) {
				printf("read==)ret:%d\n", ret);
				CONVERT_PARAMTOSTR(rtk_port_phy_ability);
				printf("<===END\n");
			}

			CONVERT_PARAMFROMSTR(rtk_port_phy_ability);
			ret = rtk_port_phyForceModeAbility_set(rtk_port, &rtk_port_phy_ability);
			printf("ret:%d\n", ret);
		}
		break;
	case RTK_PORT_PHYFORCEMODEABILITY_GET_ID:
		{
			rtk_api_ret_t ret;
			rtk_port_phy_ability_t rtk_port_phy_ability = {0};

			printf("is rtk_port_phyForceModeAbility_get command!\n");

			GET_NEXT_ID(rtk_port, UNDEFINE_PORT);

			ret = rtk_port_phyForceModeAbility_get(rtk_port, &rtk_port_phy_ability);
			printf("ret:%d\n", ret);
			CONVERT_PARAMTOSTR(rtk_port_phy_ability);
		}
		break;
	case RTK_PORT_PHYSTATUS_GET_ID:
		{
			rtk_api_ret_t ret;
			printf("is rtk_port_phyStatus_get command!\n");
			rtk_port_speed_t rtk_port_speed;
			rtk_port_duplex_t rtk_port_duplex;

			GET_NEXT_ID(rtk_port, UNDEFINE_PORT);
			GET_NEXT_ID(rtk_port_linkStatus, PORT_LINKSTATUS_END);
			ret = rtk_port_phyStatus_get(rtk_port, &rtk_port_linkStatus, &rtk_port_speed, &rtk_port_duplex);
			printf("ret:%d\n", ret);
			PRINT_ID(rtk_port_linkStatus);
			PRINT_ID(rtk_port_speed);
			PRINT_ID(rtk_port_duplex);

		}
	case RTK_PORT_MACFORCELINK_SET_ID:
		{
			rtk_api_ret_t ret;
			printf("is rtk_port_macForceLink_set command!\n");
			rtk_port_mac_ability_t rtk_port_mac_ability = {0};

			GET_NEXT_ID(rtk_port, UNDEFINE_PORT);

			ret = rtk_port_macForceLink_get(rtk_port, &rtk_port_mac_ability);
			if(g_debug_cls) {
				printf("read==)ret:%d\n", ret);
				CONVERT_PARAMTOSTR(rtk_port_mac_ability);
				printf("<===END\n");
			}

			CONVERT_PARAMFROMSTR(rtk_port_mac_ability);

			ret = rtk_port_macForceLink_set(rtk_port, &rtk_port_mac_ability);
			printf("ret:%d\n", ret);
		}
		break;
	case RTK_PORT_MACFORCELINK_GET_ID:
		{
			rtk_api_ret_t ret;
			rtk_port_mac_ability_t rtk_port_mac_ability;
			printf("is rtk_port_macForceLink_get command!\n");

			GET_NEXT_ID(rtk_port, UNDEFINE_PORT);

			ret = rtk_port_macForceLink_get(rtk_port, &rtk_port_mac_ability);

			printf("ret:%d\n", ret);
			CONVERT_PARAMTOSTR(rtk_port_mac_ability);
		}
		break;
	case RTK_PORT_MACFORCELINKEXT_SET_ID:
		{
			rtk_api_ret_t ret;

			rtk_mode_ext_t mode_ext;
			rtk_port_mac_ability_t rtk_port_mac_ability = {0};

			printf("is rtk_port_macForceLinkExt_set command!\n");

			GET_NEXT_ID(rtk_port, UNDEFINE_PORT);

			ret = rtk_port_macForceLinkExt_get(rtk_port, &mode_ext, &rtk_port_mac_ability);
			if(g_debug_cls) {
				printf("read==)ret:%d\n", ret);
				PRINT_HEX_PARAM(mode_ext);
				//PRINT_ID(rtk_mode_ext);
				CONVERT_PARAMTOSTR(rtk_port_mac_ability);
				printf("<===END\n");
			}

			GET_NEXT_ID(rtk_mode_ext, MODE_EXT_END);
			CONVERT_PARAMFROMSTR(rtk_port_mac_ability);

		    //printf("Setting -->\n");
			//CONVERT_PARAMTOSTR(rtk_port_mac_ability);
		    //printf("END\n");

			ret = rtk_port_macForceLinkExt_set(rtk_port, rtk_mode_ext, &rtk_port_mac_ability);
			printf("ret:%d\n", ret);
		}
		break;
	case RTK_PORT_MACFORCELINKEXT_GET_ID:
		{
			rtk_api_ret_t ret;
			rtk_mode_ext_t rtk_mode_ext;
			rtk_port_mac_ability_t rtk_port_mac_ability;

			printf("is rtk_port_macForceLinkExt_get command!\n");

			GET_NEXT_ID(rtk_port, UNDEFINE_PORT);

			ret = rtk_port_macForceLinkExt_get(rtk_port, &rtk_mode_ext, &rtk_port_mac_ability);
			printf("ret:%d\n", ret);

			PRINT_ID(rtk_mode_ext);

			CONVERT_PARAMTOSTR(rtk_port_mac_ability);
		}
		break;
	case RTK_PORT_MACSTATUS_GET_ID:
		{
			rtk_api_ret_t ret;
			rtk_port_mac_ability_t rtk_port_mac_ability;
			printf("is rtk_port_macStatus_get command!\n");

			GET_NEXT_ID(rtk_port, UNDEFINE_PORT);

			ret = rtk_port_macStatus_get(rtk_port, &rtk_port_mac_ability);

			printf("ret:%d\n", ret);
			CONVERT_PARAMTOSTR(rtk_port_mac_ability);
		}
		break;
	case RTK_PORT_MACLOCALLOOPBACKENABLE_SET_ID:
		{
			rtk_api_ret_t ret;
			printf("is rtk_port_macLocalLoopbackEnable_set command!\n");

			GET_NEXT_ID(rtk_port, UNDEFINE_PORT);
			GET_NEXT_ID(rtk_enable, RTK_ENABLE_END);
			ret = rtk_port_macLocalLoopbackEnable_set(rtk_port, rtk_enable);
			printf("ret:%d\n", ret);
		}
		break;
	case RTK_PORT_MACLOCALLOCALLOOPBACKENABLE_GET_ID:
		{
			rtk_api_ret_t ret;
			rtk_enable_t rtk_enable;

			printf("is rtk_port_macLocalLoopbackEnable_get command!\n");
			GET_NEXT_ID(rtk_port, UNDEFINE_PORT);

			ret = rtk_port_macLocalLoopbackEnable_get(rtk_port, &rtk_enable);
			printf("ret:%d\n", ret);
			PRINT_ID(rtk_enable);
		}
		break;
	case RTK_PORT_PHYREG_SET_ID:
		{
			rtk_api_ret_t ret;
			char * str_val = NULL;
			printf("is rtk_port_phyReg_set command!\n");

			GET_NEXT_ID(rtk_port, UNDEFINE_PORT);
			GET_NEXT_ID(rtk_port_phy_reg, PHY_REG_END);

			GET_NEXT_PARAM(str_val);
			rtk_port_phy_data_t regData = simple_strtoul(str_val, NULL, 0);

			ret = rtk_port_phyReg_set(rtk_port, rtk_port_phy_reg, regData);
			printf("ret:%d\n", ret);
		}
		break;
	case RTK_PORT_PHYREG_GET_ID:
		{
			rtk_api_ret_t ret;
			rtk_port_phy_data_t rtk_port_phy_data;
			printf("is rtk_port_phyReg_get command!\n");
			GET_NEXT_ID(rtk_port, UNDEFINE_PORT);
			GET_NEXT_ID(rtk_port_phy_reg, PHY_REG_END);
			ret = rtk_port_phyReg_get(rtk_port, rtk_port_phy_reg, &rtk_port_phy_data);
			printf("ret:%d\n", ret);
			PRINT_HEX_PARAM(rtk_port_phy_data);
		}
		break;
	case RTK_PORT_PHYENABLEALL_SET_ID:
		{
			rtk_api_ret_t ret;
			printf("is rtk_port_phyEnableAll_set command!\n");

			GET_NEXT_ID(rtk_enable, RTK_ENABLE_END);

			ret = rtk_port_phyEnableAll_set(rtk_enable);
			printf("ret:%d\n", ret);
		}
		break;
	case RTK_PORT_PHYENABLEALL_GET_ID:
		{
			rtk_api_ret_t ret;
			rtk_enable_t rtk_enable;
			printf("is rtk_port_phyEnableAll_get command!\n");
			ret = rtk_port_phyEnableAll_get(&rtk_enable);
			printf("ret:%d\n", ret);
			PRINT_ID(rtk_enable);
		}
		break;
	case RTK_PORT_PHYCOMBOPORTMEDIA_SET_ID:
		{
			rtk_api_ret_t ret;
			printf("is rtk_port_phyComboPortMedia_set command!\n");

			GET_NEXT_ID(rtk_port, UNDEFINE_PORT);
			GET_NEXT_ID(rtk_port_media, PORT_MEDIA_END);
			ret = rtk_port_phyComboPortMedia_set(rtk_port, rtk_port_media);
			printf("ret:%d\n", ret);
		}
		break;
	case RTK_PORT_PHYCOMBOPORTMEDIA_GET_ID:
		{
			rtk_api_ret_t ret;
			rtk_port_media_t rtk_port_media;
			printf("is rtk_port_phyComboPortMedia_get command!\n");
			GET_NEXT_ID(rtk_port, UNDEFINE_PORT);
			ret = rtk_port_phyComboPortMedia_get(rtk_port, &rtk_port_media);
			printf("ret:%d\n", ret);
			PRINT_ID(rtk_port_media);
		}
		break;
	case RTK_PORT_SGMIILINKSTATUS_GET_ID:
		{
			rtk_api_ret_t ret;
			printf("is rtk_port_sgmiiLinkStatus_get command!\n");
			rtk_data_t signalDetect;
			rtk_data_t sync;
			rtk_port_linkStatus_t rtk_port_linkStatus;

			GET_NEXT_ID(rtk_port, UNDEFINE_PORT);
			ret = rtk_port_sgmiiLinkStatus_get(rtk_port, &signalDetect, &sync, &rtk_port_linkStatus);
			printf("ret:%d\n", ret);
			PRINT_HEX_PARAM(signalDetect);
			PRINT_HEX_PARAM(sync);
			PRINT_ID(rtk_port_linkStatus);
		}
		break;
	case RTK_PORT_SGMIINWAY_SET_ID:
		{
			rtk_api_ret_t ret;
			printf("is rtk_port_sgmiiNway_set command!\n");

			GET_NEXT_ID(rtk_port, UNDEFINE_PORT);
			GET_NEXT_ID(rtk_enable, RTK_ENABLE_END);

			ret= rtk_port_sgmiiNway_set(rtk_port, rtk_enable);
			printf("ret:%d\n", ret);
		}
		break;
	case RTK_PORT_SGMIINWAY_GET_ID:
		{
			rtk_api_ret_t ret;
			rtk_enable_t rtk_enable;

			GET_NEXT_ID(rtk_port, UNDEFINE_PORT);

			ret = rtk_port_sgmiiNway_get(rtk_port, &rtk_enable);

			printf("ret:%d\n", ret);
			PRINT_ID(rtk_enable);
		}
		break;
	case RTK_PORT_PHYMDX_SET_ID:
		{
			rtk_api_ret_t ret;

			printf("is rtk_port_phyMdx_set command!\n");

			GET_NEXT_ID(rtk_port, UNDEFINE_PORT);
			GET_NEXT_ID(rtk_port_phy_mdix_mode, PHY_FORCE_MODE_END);

			ret = rtk_port_phyMdx_set(rtk_port, rtk_port_phy_mdix_mode);
			printf("ret:%d\n", ret);
		}
		break;
	case RTK_PORT_PHYMDX_GET_ID:
		{
			rtk_api_ret_t ret;
			rtk_port_phy_mdix_mode_t rtk_port_phy_mdix_mode;

			printf("is rtk_port_phyMdx_get command!\n");

			GET_NEXT_ID(rtk_port, UNDEFINE_PORT);

			ret = rtk_port_phyMdx_get(rtk_port, &rtk_port_phy_mdix_mode);

			printf("ret:%d\n", ret);
			PRINT_ID(rtk_port_phy_mdix_mode);
		}
		break;
	case RTK_PORT_PHYMDXSTATUS_GET_ID:
		{
			rtk_api_ret_t ret;
			rtk_port_phy_mdix_status_t rtk_port_phy_mdix_status;
			printf("is rtk_port_phyMdxStatus_get command!\n");

			GET_NEXT_ID(rtk_port, UNDEFINE_PORT);
			ret = rtk_port_phyMdxStatus_get(rtk_port, &rtk_port_phy_mdix_status);
			printf("ret:%d\n", ret);
			PRINT_ID(rtk_port_phy_mdix_status);
		}
		break;
	case RTK_PORT_MAXPACKETLENGTH_SET_ID:
		{
			rtk_api_ret_t ret;
			rtk_uint32 length;
			char * str_val = NULL;

			printf("is rtk_port_maxPacketLength_set command!\n");

			GET_NEXT_ID(rtk_port, UNDEFINE_PORT);
			GET_NEXT_PARAM(str_val);
			length= simple_strtoul(str_val, NULL, 0);

			ret = rtk_port_maxPacketLength_set(rtk_port, length);
			printf("ret:%d\n", ret);
		}
		break;
	case RTK_PORT_MAXPACKETLENGTH_GET_ID:
		{
			rtk_api_ret_t ret;
			rtk_uint32 length;
			printf("is rtk_port_maxPacketLength_get command!\n");
			GET_NEXT_ID(rtk_port, UNDEFINE_PORT);
			ret = rtk_port_maxPacketLength_get(rtk_port, &length);
			printf("ret:%d\n", ret);
			PRINT_HEX_PARAM(length);
		}
		break;
	case RTK_CPU_ENABLE_SET_ID:
		{
			rtk_api_ret_t ret;
			printf("is rtk_cpu_enable_set command!\n");

			GET_NEXT_ID(rtk_enable, RTK_ENABLE_END);
			ret = rtk_cpu_enable_set(rtk_enable);
			printf("ret:%d\n", ret);
		}
		break;
	case RTK_CPU_ENABLE_GET_ID:
		{
			rtk_api_ret_t ret;
			rtk_enable_t rtk_enable;
			printf("is rtk_cpu_enable_get command!\n");

			ret = rtk_cpu_enable_get(&rtk_enable);
			printf("ret:%d\n", ret);
			PRINT_ID(rtk_enable);
		}
		break;
	case RTK_L2_INIT_ID:
		{
			rtk_api_ret_t ret;
			printf("is rtk_l2_init command!\n");
			ret = rtk_l2_init();
			printf("ret:%d\n", ret);
		}
		break;
	case RTK_L2_ADDR_ADD_ID:
		{
			rtk_api_ret_t ret;
			rtk_l2_ucastAddr_t rtk_l2_ucastAddr={0};
			rtk_mac_t rtk_mac;
			printf("is rtk_l2_addr_add command!\n");

			CONVERT_PARAMFROMSTR(rtk_mac);
#if 0
			ret = rtk_l2_addr_get(&rtk_mac, &rtk_l2_ucastAddr);
			if(g_debug_cls) {
				printf("read==)ret:%d\n", ret);
				CONVERT_PARAMTOSTR(rtk_l2_ucastAddr);
				printf("<===END\n");
			}
#endif
			CONVERT_PARAMFROMSTR(rtk_l2_ucastAddr);
			ret = rtk_l2_addr_add(&rtk_mac, &rtk_l2_ucastAddr);
			printf("ret:%d\n", ret);
		}
		break;
	case RTK_L2_ADDR_GET_ID:
		{
			rtk_api_ret_t ret;
			rtk_l2_ucastAddr_t rtk_l2_ucastAddr={0};
			rtk_mac_t rtk_mac;

			printf("is rtk_l2_addr_get command!\n");

			CONVERT_PARAMFROMSTR(rtk_mac);
			CONVERT_PARAMFROMSTR(rtk_l2_ucastAddr);

			ret = rtk_l2_addr_get(&rtk_mac, &rtk_l2_ucastAddr);
			printf("ret:%d\n", ret);
			CONVERT_PARAMTOSTR(rtk_l2_ucastAddr);
			CONVERT_PARAMTOSTR(rtk_mac);
		}
		break;
	case RTK_L2_ADDR_NEXT_GET_ID:
		{
			rtk_api_ret_t ret;
			rtk_uint32 address = 0;
			rtk_l2_ucastAddr_t rtk_l2_ucastAddr = {0};
			char * str_address = NULL;

			printf("is rtk_l2_addr_next_get command!\n");

			GET_NEXT_ID(rtk_l2_read_method, READMETHOD_END);
			GET_NEXT_ID(rtk_port, UNDEFINE_PORT);

			GET_NEXT_PARAM(str_address);

			address = simple_strtoul(str_address, NULL, 0);

			ret = rtk_l2_addr_next_get(rtk_l2_read_method, rtk_port, &address, &rtk_l2_ucastAddr);
			printf("ret:%d\n", ret);
			CONVERT_PARAMTOSTR(rtk_l2_ucastAddr);
		}
		break;
	case RTK_L2_ADDR_NEXT_DEL_ID:
		{
			rtk_api_ret_t ret;
			rtk_l2_ucastAddr_t rtk_l2_ucastAddr;
			rtk_mac_t rtk_mac;
			printf("is rtk_l2_addr_next_del command!\n");

			CONVERT_PARAMFROMSTR(rtk_mac);
			CONVERT_PARAMFROMSTR(rtk_l2_ucastAddr);
			ret = rtk_l2_addr_del(&rtk_mac, &rtk_l2_ucastAddr);
			printf("ret:%d\n", ret);
		}

		break;
	case RTK_L2_FLOODPORTMASK_SET_ID:
		{
			rtk_api_ret_t ret;
			rtk_portmask_t rtk_portmask;
			printf("is rtk_l2_floodPortMask_set command!\n");

			GET_NEXT_ID(rtk_l2_flood_type, FLOOD_END);

			ret = rtk_l2_floodPortMask_get(rtk_l2_flood_type, &rtk_portmask);
			if(g_debug_cls) {
				printf("read==)ret:%d\n", ret);
				CONVERT_PARAMTOSTR(rtk_portmask);
				printf("<===END\n");
			}

			CONVERT_PARAMFROMSTR(rtk_portmask);

			ret = rtk_l2_floodPortMask_set(rtk_l2_flood_type, &rtk_portmask);
			printf("ret:%d\n", ret);
		}
		break;
	case RTK_L2_FLOODPORTMASK_GET_ID:
		{
			rtk_api_ret_t ret;
			rtk_portmask_t rtk_portmask;
			printf("is rtk_l2_floodPortMask_get command!\n");
			GET_NEXT_ID(rtk_l2_flood_type, FLOOD_END);
			ret = rtk_l2_floodPortMask_get(rtk_l2_flood_type, &rtk_portmask);
			printf("ret:%d\n", ret);
			CONVERT_PARAMTOSTR(rtk_portmask);
		}
		break;
	case RTK_L2_LOCALPKTPERMIT_SET_ID:
		{
			rtk_api_ret_t ret;
			printf("is rtk_l2_localPktPermit_set command!\n");
			GET_NEXT_ID(rtk_port, UNDEFINE_PORT);
			GET_NEXT_ID(rtk_enable, RTK_ENABLE_END);
			ret = rtk_l2_localPktPermit_set(rtk_port, rtk_enable);
			printf("ret:%d\n", ret);
		}
		break;
	case RTK_L2_LOCALPKTPERMIT_GET_ID:
		{
			rtk_api_ret_t ret;
			rtk_enable_t rtk_enable;
			printf("is rtk_l2_localPktPermit_get command!\n");

			GET_NEXT_ID(rtk_port, UNDEFINE_PORT);

			ret = rtk_l2_localPktPermit_get(rtk_port, &rtk_enable);
			printf("ret:%d\n", ret);
			PRINT_ID(rtk_enable);
		}
		break;
	case RTK_L2_ENTRY_GET_ID:
		{
			rtk_api_ret_t ret;
			rtk_l2_addr_table_t rtk_l2_addr_table = {};

			printf("is rtk_l2_entry_set command!\n");

			CONVERT_PARAMFROMSTR(rtk_l2_addr_table);

			ret = rtk_l2_entry_get(&rtk_l2_addr_table);
			printf("ret:%d\n", ret);

			CONVERT_PARAMTOSTR(rtk_l2_addr_table);
		}
		break;
	case RTK_REG_READ:
		{
			rtk_api_ret_t ret;
			rtk_uint32 value;
			rtk_uint32 reg = simple_strtoul(argv[post++], NULL, 0);
#ifdef CONFIG_DAL_RTL8367C
			ret = rtl8367c_getAsicReg(reg, &value);
#else
			ret = rtl8367d_getAsicReg(reg, &value);
#endif
			printf("ret:%d, reg:%d, value:%#x\n", ret, reg, value);

		}
		break;
	case RTK_REG_WRITE:
		{
			rtk_api_ret_t ret;
			rtk_uint32  reg;
			rtk_uint32 value;

			reg = simple_strtoul(argv[post++], NULL, 0);
			value = simple_strtoul(argv[post++], NULL, 0);

#ifdef CONFIG_DAL_RTL8367C
			ret = rtl8367c_setAsicReg(reg, value);
#else
			ret = rtl8367d_setAsicReg(reg, value);
#endif

			printf("ret:%d, reg:%d, value:%#x\n", ret, reg, value);
		}
		break;
	case RTK_PORT_RGMIIDELAYEXT_GET_ID:
		{
			rtk_api_ret_t ret;
			rtk_data_t txDelay;
			rtk_data_t rxDelay;

			GET_NEXT_ID(rtk_port, UNDEFINE_PORT);

			ret = rtk_port_rgmiiDelayExt_get(rtk_port, &txDelay, &rxDelay);

			printf("ret:%d txDelay:%d rxDelay:%d\n", ret, txDelay, rxDelay);

		}
		break;

	case RTK_PORT_RGMIIDELAYEXT_SET_ID:
		{

			rtk_api_ret_t ret;
			rtk_data_t txDelay;
			rtk_data_t rxDelay;
            char * str_txDelay;
			char * str_rxDelay;

			GET_NEXT_ID(rtk_port, UNDEFINE_PORT);
			GET_NEXT_PARAM(str_txDelay);
			GET_NEXT_PARAM(str_rxDelay);

			txDelay = simple_strtoul(str_txDelay, NULL, 0);
			rxDelay = simple_strtoul(str_rxDelay, NULL, 0);

			ret = rtk_port_rgmiiDelayExt_set(rtk_port,  txDelay,  rxDelay);

			printf("ret:%d txDelay:%d rxDelay:%d\n", ret, txDelay, rxDelay);
		}
		break;

	case RTK_STAT_PORT_GETALL_ID:
		{
			rtk_api_ret_t ret;
			rtk_stat_port_cntr_t rtk_stat_port_cntr = {0};

			printf("is rtk_stat_port_getall command!\n");

			GET_NEXT_ID(rtk_port, UNDEFINE_PORT);

			ret = rtk_stat_port_getAll(rtk_port, &rtk_stat_port_cntr);

			printf("ret:%d\n", ret);

			CONVERT_PARAMTOSTR(rtk_stat_port_cntr);
		}
		break;

	case RTK_STAT_PORT_RESET_ID:
		{
			rtk_api_ret_t ret;

			printf("is rtk_stat_port_reset command!\n");

			GET_NEXT_ID(rtk_port, UNDEFINE_PORT);

			ret = rtk_stat_port_reset(rtk_port);

			printf("ret:%d\n", ret);
		}
		break;
	}
	return 0;
}

static struct mii_dev *bus;

unsigned int mii_mgr_read(unsigned int phy_addr, unsigned int phy_register, unsigned int * read_data)
{
	int devad = MDIO_DEVAD_NONE;

	if(!bus) {
		bus =  mii_bus_list[0];

		if (!bus) {
			printf("Can't find mdio dev!!\n");
			return 0;
		}
		/* Save the chosen bus */
	//	miiphy_set_current_dev(bus->name);
	}


	if (read_data ) {
		*read_data = bus->read(bus, phy_addr, devad, phy_register);
#if 0
		if (g_debug_cls)
			printf("%s phy[%#x] reg[%#x:%d] val[%#x:%d]\n",
					__FUNCTION__,
					phy_addr,
					phy_register,
					phy_register,
					*read_data,
					*read_data);
#endif
	}

	return 0;
}


unsigned int mii_mgr_write(unsigned int phy_addr, unsigned int phy_register, unsigned int write_data)
{
	int devad = MDIO_DEVAD_NONE;

#if 0
	if (g_debug_cls)
		printf("%s phy[%#x] reg[%#x:%d] val[%#x:%d]\n",
				__FUNCTION__,
				phy_addr,
				phy_register,
				phy_register,
				write_data,
				write_data);
#endif
	if(!bus) {

		bus = mii_bus_list[0];

		if (!bus) {
			printf("Can't find mdio dev!!\n");
			return 0;
		}

		/* Save the chosen bus */
		//miiphy_set_current_dev(bus->name);
	}

	 bus->write(bus, phy_addr, devad,
			phy_register, write_data);

	return 0;
}

U_BOOT_CMD(
		switch,	30,	1,	do_switch,
		"clourney rtl8367 utility commands",
		"rtk_port_phyAutoNegoAbility_set\n"
		"rtk_port_phyAutoNegoAbility_get\n"
		"rtk_port_phyForceModeAbility_set\n"
		"rtk_port_phyForceModeAbility_get\n"
		"rtk_port_phyStatus_get\n"
		"rtk_port_macForceLink_set\n"
		"rtk_port_macForceLink_get\n"
		"rtk_port_macForceLinkExt_set\n"
		"rtk_port_macForceLinkExt_get\n"
		"rtk_port_macStatus_get\n"
		"rtk_port_macLocaloopbackEnable_set\n"
		"rtk_port_macLocalLoopbackEnable_get\n"
		"rtk_port_phyReg_set\n"
		"rtk_port_phyReg_get\n"
		"rtk_port_phyEnableAll_set\n"
		"rtk_port_phyEnableAll_get\n"
		"rtk_port_phyComboPortMedia_set\n"
		"rtk_port_phyComboPortMedia_get\n"
		"rtk_port_sgmiiLinkStatus_get\n"
		"rtk_port_sgmiiNway_set\n"
		"rtk_port_sgmiiNway_get\n"
		"rtk_port_phyMdx_set\n"
		"rtk_port_phyMdx_get\n"
		"rtk_port_phyMdxStatus_get\n"
		"rtk_port_maxPacketLength_set\n"
		"rtk_port_maxPacketLength_get\n"
		"rtk_cpu_enable_set\n"
		"rtk_cpu_enable_get\n"
		"rtk_l2_init\n"
		"rtk_l2_addr_add\n"
		"rtk_l2_addr_get\n"
		"rtk_l2_addr_next_get\n"
		"rtk_l2_addr_next_del\n"
		"rtk_l2_floodPortMask_set\n"
		"rtk_l2_floodPortMask_get\n"
		"rtk_l2_localPktPermit_set\n"
		"rtk_l2_localPktPermit_get\n"
		"rtk_l2_entry_get\n"
		"regRd\n"
		"regWr\n"
		"rtk_port_rgmiiDelayExt_get\n"
		"rtk_port_rgmiiDelayExt_set\n"
		"rtk_stat_port_getAll\n"
		"rtk_stat_port_reset\n"

);

