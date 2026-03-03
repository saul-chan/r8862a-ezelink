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

/* dubhe1000_mac.h
 * Structures, enums, and macros for the mac + pcs
 */

#ifndef _DUBHE1000_MAC_H_
#define _DUBHE1000_MAC_H_

#include "dubhe2000_osdep.h"
#include "dubhe2000.h"

#define XGMAC_INTERFACE_MODE_RGMII	1
#define XGMAC_INTERFACE_MODE_RMII	2
#define XGMAC_INTERFACE_MODE_NON_RGMII	0

/*
 *3'b 000 10G	XGMII
 *3'b 001 Reserved
 *3'b 010 2.5G	GMII
 *3'b 011 1G	GMII
 *3'b 100 100M	GMII
 *3'b 101 5G	XGMII
 *3'b 110 2.5G	XGMII
 *3'b 111 10M	MII
 */
enum SS_SELECTION_EM {
	SS_10G_XGMII = 0,
	SS_Reserved = 1,
	SS_2Dot5G_GMII = 2,
	SS_1G_GMII = 3,
	SS_100M_GMII = 4,
	SS_5G_XGMII = 5,
	SS_2Dot5G_XGMII = 6,
	SS_10M_MII = 7,
};

#ifdef CONFIG_DUBHE2000_PHYLINK
void dubhe1000_enable_xgmac_interrupt(struct dubhe1000_mac *port, bool enable);
void dubhe1000_xgmac_init(struct dubhe1000_mac *port, struct net_device *dev);
void dubhe1000_xgmac_flow_ctrl(struct dubhe1000_mac *port, u32 duplex);
void dubhe1000_xgmac_speed(struct dubhe1000_mac *port, unsigned int speed);
void dubhe1000_xgmac_duplex(struct dubhe1000_mac *port, int duplex);
void dubhe1000_enable_xgmac(struct dubhe1000_mac *port, bool enable);
int dubhe1000_xgmac_get_link_status(struct dubhe1000_mac *port);
#elif defined(CONFIG_DUBHE2000_DEVLINK)
void dubhe1000_mac_pcs_config(struct dubhe1000_adapter *adapter, u8 enable);
#endif
#endif /* _DUBHE1000_MAC_H_ */
