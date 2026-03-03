/* Motorcomm YT921x DSA Switch driver
* Copyright (c), 2022, Motorcomm Electronic Technology Co.,Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*/

#ifndef _YT921X_H
#define _YT921X_H

#include <linux/ethtool.h>
#include <linux/export.h>
#include <linux/gpio/consumer.h>
#include <linux/netdevice.h>
#include <linux/of.h>
#include <linux/of_mdio.h>
#include <linux/phy.h>
#include <linux/phy_fixed.h>
#include <linux/phylink.h>
#include <linux/rtnetlink.h>
#include <linux/spinlock.h>
#include <linux/timer.h>
#include <linux/workqueue.h>

/* macro for customized functions  */
//#define YT921X_DSA_WITHOUT_TAG
#define YT921X_DSA_WITH_VLAN_TAG
#define YT921X_UNKNOWN_PKT_TO_CPU

/* must sync with dts */
#define YT921X_CPU_PORT                     8
#define YT921X_ALL_PORTMASK                 0x10f
#define YT921X_USER_PORTMASK                0xf
#define YT921X_MC_QUEUE_MAX    4
#define YT921X_AC_QUEUE_MAX    8

/* CTRLIF */
#define YT921X_SWITCH_ID                    0x0
#define YT921X_SWITCH_PHY_ADDR              0x1d

/* port  <---> vlan id */
#define YT_PORT_VLAN_MAP    3
#define YT_PORT_XMIT_VLAN_MAP    10
#define YT_ETH_VLAN_TPID             0x8866

struct phylink {
	/* private: */
	struct net_device *netdev;
	const struct phylink_mac_ops *ops;

	unsigned long phylink_disable_state; /* bitmask of disables */
	struct phy_device *phydev;
	phy_interface_t link_interface;	/* PHY_INTERFACE_xxx */
	u8 link_an_mode;		/* MLO_AN_xxx */
	u8 link_port;			/* The current non-phy ethtool port */
	__ETHTOOL_DECLARE_LINK_MODE_MASK(supported);

	/* The link configuration settings */
	struct phylink_link_state link_config;

	/* The current settings */
	phy_interface_t cur_interface;

	struct gpio_desc *link_gpio;
	struct timer_list link_poll;
	void (*get_fixed_state)(struct net_device *dev,
				struct phylink_link_state *s);

	struct mutex state_mutex;
	struct phylink_link_state phy_state;
	struct work_struct resolve;

	bool mac_link_dropped;

	struct sfp_bus *sfp_bus;
};

/* function declaration */
void yt921x_init(struct motorcomm_priv *priv);

#endif
