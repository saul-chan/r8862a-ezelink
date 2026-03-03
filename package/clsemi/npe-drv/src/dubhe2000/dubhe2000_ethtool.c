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

/* ethtool support for dubhe1000 */

#include "dubhe2000.h"
#include <linux/jiffies.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>

static void dubhe1000_get_drvinfo(struct net_device *netdev, struct ethtool_drvinfo *drvinfo)
{
#if (DUBHE1000_SWITCH_EN)
	struct dubhe1000_mac *port = netdev_priv(netdev);
	struct dubhe1000_adapter *adapter = port->adapter;
#else
	struct dubhe1000_adapter *adapter = netdev_priv(netdev);
#endif

	strlcpy(drvinfo->driver, cls_npe_driver_name, sizeof(drvinfo->driver));

	strlcpy(drvinfo->bus_info, adapter->pdev->name, sizeof(drvinfo->bus_info));
}

static u32 dubhe1000_get_msglevel(struct net_device *netdev)
{
#if (DUBHE1000_SWITCH_EN)
	struct dubhe1000_mac *port = netdev_priv(netdev);
	struct dubhe1000_adapter *adapter = port->adapter;
#else
	struct dubhe1000_adapter *adapter = netdev_priv(netdev);
#endif

	return adapter->msg_enable;
}

static void dubhe1000_set_msglevel(struct net_device *netdev, u32 msglevel)
{
#if (DUBHE1000_SWITCH_EN)
	struct dubhe1000_mac *port = netdev_priv(netdev);
	struct dubhe1000_adapter *adapter = port->adapter;
#else
	struct dubhe1000_adapter *adapter = netdev_priv(netdev);
#endif

	adapter->msg_enable = msglevel;
}

static int dubhe1000_get_link_ksettings(struct net_device *dev, struct ethtool_link_ksettings *cmd)
{
	struct dubhe1000_mac *port = netdev_priv(dev);

#ifdef CONFIG_DUBHE2000_PHYLINK
	if (!port->phylink)
		return -EOPNOTSUPP;

	return phylink_ethtool_ksettings_get(port->phylink, cmd);
#else
	return -EOPNOTSUPP;
#endif
}

static int dubhe1000_set_link_ksettings(struct net_device *dev,
				    const struct ethtool_link_ksettings *cmd)
{
	struct dubhe1000_mac *port = netdev_priv(dev);

#ifdef CONFIG_DUBHE2000_PHYLINK
	if (!port->phylink)
		return -EOPNOTSUPP;

	return phylink_ethtool_ksettings_set(port->phylink, cmd);
#else
	return -EOPNOTSUPP;
#endif
}

static const struct ethtool_ops dubhe1000_ethtool_ops = {
	.supported_coalesce_params	= ETHTOOL_COALESCE_RX_USECS,
	.get_drvinfo			= dubhe1000_get_drvinfo,
	.get_msglevel			= dubhe1000_get_msglevel,
	.set_msglevel			= dubhe1000_set_msglevel,
	.get_link_ksettings		= dubhe1000_get_link_ksettings,
	.set_link_ksettings		= dubhe1000_set_link_ksettings
};

void dubhe1000_set_ethtool_ops(struct net_device *netdev)
{
	netdev->ethtool_ops = &dubhe1000_ethtool_ops;
}
