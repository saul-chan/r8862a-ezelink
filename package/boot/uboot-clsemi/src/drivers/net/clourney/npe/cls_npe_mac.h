/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 1999 - 2006 Clourneysemi Corporation. */
#ifndef __CLS_NPE_MAC_H__
#define  __CLS_NPE_MAC_H__
#include "cls_npe.h"
#define XGMAC_INTERFACE_MODE_RGMII    	(1)
#define XGMAC_INTERFACE_MODE_RMII      	(2)
#define XGMAC_INTERFACE_MODE_NON_RGMII 	(0)


struct mac_tx_configuration_reg_st {
	union {
		u32  raw;
		struct {
			u32  TE:1,          				 /*R/W Transmitter Enable*/
				 DDIC:1,         				 /*R/W Disable DIC Algorithm*/
				 Reserved_2:1,                 	 /*R*/
				 ISM:1,         				 /*R/W IFG Stretch Ratio*/
				 ISR:4,          				 /*R/W IFG Stretch Mode*/
				 IPG:3,          				 /*R/W Inter-Packet Gap*/
				 IFP:1,          				 /*R/W IPG Control*/
				 TC:1,           				 /*R/W Transmit Configuration in RGMII*/
				 LUD:1,          				 /*R/W Link Up or Down*/
				 Reserved_15_14:2,             	 /*R*/
				 JD:1,                           /*R/W Jabber Disable*/
				 Reserved_17:1,                	 /*R*/
				 PCHM:1,                         /*R/W PCHM Mode*/
				 PEN:1,                          /*R/W PCH Enable*/
				 SARC:3,                         /*R/W Source Address Insertion or Replacement Control*/
				 Reserved_23:1,                  /*R */
				 VNE:1,                          /*R/W VxLAN-NVGRE Mode*/
				 VNM:1, 						 /*R/W VxLAN-NVGRE Mode*/
				 Reserved_26:1,                  /*R*/
				 GT9WH:1,                        /*R/W G9991 Without Ethernet Header encapsulation enable*/
				 G9991EN:1,                      /*R/W G999_1 Mode enable*/
				 SS:3;                           /*Speed Selection*/
		};
	};
}__packed;

struct mac_rx_configuration_reg_st {
	union {
		u32 raw;
		struct {
			u32  RE:1,          				 /*R/W Receiver Enable*/
				 ACS:1,         				 /*R/W Automatic Pad or CRC Stripping*/
				 CST:1,                 		 /*R/W CRC Stripping for Type packets*/
				 DCRCC:1,         			 	 /*R/W Disable CRC Checking for Received Packets*/
				 SPEN:1,          			 	 /*R/W Slow Protocol Detection Enable*/
				 USP:1,          				 /*R/W Unicast Slow Protocol Packet Detect*/
				 GPSLCE:1,          			 /*R/W Giant Packet Size Limit Control Enable*/
				 WD:1,           				 /*R/W Watchdog Diable*/
				 JE:1,          				 /*R/W Jumbo Packet Enable*/
				 IPC:1,                          /*R/W Checksum Offload*/
				 LM:1,                           /*R/W Loopback Mode*/
				 S2KP:1,               		     /*R/W IEEE 802.3as Support for 2k Packets*/
				 HDSMS:3,                        /*R/W Maximum Size for Splitting the Header Data*/
				 PRXM:1,                         /*R/W PCH Rx Mode*/
				 GPSL:14,                        /*R/W Giant Packet Size Limit*/
				 ELEN:1,                		 /*R/W External lookup enable*/
				 ARPEN:1;                        /*R/W ARP enable*/
		};
	};
}__packed;

struct mac_packet_filter_reg_st {
	union {
		u32 raw;
		struct {
			u32 PR:1,                    		/*R/W Promiscuous Mode*/
				HUC:1,                   		/*R/W Hash Unicast*/
				HMC:1,                   		/*R/W Hash Multicast*/
				DAIF:1,                 		/*R/W DA Inverse Filtering*/
				PM:1,                    		/*R/W Pass All Multicast*/
				DBF:1,                   		/*R/W Disable Broadcast Packets*/
				PCF:2,                   		/*R/W Pass Control Packets*/
				SAIF:1,                  		/*R/W SA Inverse Filtering*/
				SAF:1,                   		/*R/W Source Address Filter Enable*/
				HPF:1,                   		/*R/W Hash or Perfect Filter*/
				DHLFRS:2,                		/*R/W DA Hash Index or L3/L4 Filter Number in Receive Status*/
				Reserved_15_13:3,
				VTFE:1,                  		/*R/W VLAN Tag Filter Enable*/
				Reserved_19_17:3,        		/*R*/
				IPFE:1,                  		/*R/W Layer 3 and Layer 4 Filter Enable*/
				DNTU:1,                  		/*R/W Drop Non-TCP/UDP over IP Packets*/
				VUCC:1,                  		/*R/W Vxlan UDP/IPv6 Checksum Control*/
				Reserved_30_23:8,         		/*R*/
				RA:1;                    		/*R/W Receive ALL*/
		};
	};
}__packed;

struct mac_q0_tx_flow_ctrl_reg_st {
	union {
		u32 raw;
		struct {
			u32 PCB:1,                			/*R/W Flow Control Busy or Backpressure Activate*/
				TFE:1,                			/*R/W Transmit Flow Control Enable*/
				Reserved_3_2:2,       			/*R*/
				PLT:3,                  		/*R/W Pause Low Threshold*/
				DZPQ:1,                 		/*R/W Disable Zero-Quanta Pause*/
				Reserved_15_8:8,       			/*R*/
				PT:16;                  		/*R/W Pause Time*/
		};
	};
}__packed;

struct mac_rx_flow_ctrl_st {
	union {
		u32 raw;
		struct {
			u32 RFE:1,              		/*R/W Receive Flow Control Enable*/
				UP:1,                   	/*R/W Unicast Pause Packet Detect*/
				Reserved_7_2:6,         	/*R*/
				PFCE:1,                 	/*R/W Priority Based Flow Control Enable*/
				Reserved_31_9:23;       	/*R*/
		};
	};
}__packed;

struct mac_rxq_ctrl2_reg_st {
	union {
		u32 raw;
		struct {
			u32 PSRQ0:8,              		/*R/W Priorities Selected in the Receive Queue 0*/
				PSRQ1:8,         	  		/*R/W Priorities Selected in the Receive Queue 1*/
				PSRQ2:8,         	  		/*R/W Priorities Selected in the Receive Queue 2*/
				PSRQ3:8;         	  		/*R/W Priorities Selected in the Receive Queue 3*/
		};
	};
}__packed;

struct mac_rxq_ctrl3_reg_st {
	union {
		u32 raw;
		struct {
			u32 PSRQ4:8,              	/*R/W Priorities Selected in the Receive Queue 4*/
				PSRQ5:8,         	  	/*R/W Priorities Selected in the Receive Queue 5*/
				PSRQ6:8,         	  	/*R/W Priorities Selected in the Receive Queue 6*/
				PSRQ7:8;         	  	/*R/W Priorities Selected in the Receive Queue 7*/
		};
	};
}__packed;
/*
 *3'b 000 10G XGMII
 *3'b 001 Reserved
 *3'b 010 2.5G GMII
 *3'b 011 1G   GMII
 *3'b 100 100M	GMII
 *3'b 101 5G 	XGMII
 *3'b 110 2.5G XGMII
 *3'b 111 10M 	MII
 */
enum ss_selection_em {
	SS_10G_XGMII = 0,
	SS_Reserved = 1,
	SS_2Dot5G_GMII = 2,
	SS_1G_GMII = 3,
	SS_100M_GMII = 4,
	SS_5G_XGMII = 5,
	SS_2Dot5G_XGMII = 6,
	SS_10M_MII = 7,
};

void cls_eth_mac_pcs_config(struct udevice *dev, u8 enable);

#endif /*__CLS_NPE_MAC_H__*/
