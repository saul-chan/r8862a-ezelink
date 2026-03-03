/*
 * Motorcomm DSA Switch driver
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

#include <linux/module.h>
#include <linux/of_device.h>
#include <net/dsa.h>

#include <linux/of_mdio.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>

#ifndef LINUX_VERSION_CODE
#include <linux/version.h>
#else
#define KERNEL_VERSION(a, b, c) (((a) << 16) + ((b) << 8) + (c))
#endif
#include "motorcomm_mdio.h"

/* MDIO marco -- start */
#define REG_ADDR_BIT1_ADDR      0
#define REG_ADDR_BIT1_DATA      1
#define REG_ADDR_BIT0_WRITE     0
#define REG_ADDR_BIT0_READ      1
/* MDIO marco -- end */

int yt_mdio_switch_write(struct motorcomm_priv *priv, u32 reg_addr, u32 reg_value)
{
    u8 devAddr;
    u8 regAddr;
    u16 regVal;

    if (NULL == priv || NULL == priv->miiBus || NULL == priv->miiBus->write)
    {
        return -EINVAL;
    }

    devAddr           = priv->devAddr;
    regAddr           = (priv->switchId<<2)|(REG_ADDR_BIT1_ADDR<<1)|(REG_ADDR_BIT0_WRITE);
    /* Set reg_addr[31:16] */
    regVal = (reg_addr >> 16)&0xffff;
    mutex_lock(&priv->miiBus->mdio_lock);
    priv->miiBus->write(priv->miiBus, devAddr, regAddr, regVal);
    mutex_unlock(&priv->miiBus->mdio_lock);

    /* Set reg_addr[15:0] */
    regVal = reg_addr&0xffff;
    mutex_lock(&priv->miiBus->mdio_lock);
    priv->miiBus->write(priv->miiBus, devAddr, regAddr, regVal);
    mutex_unlock(&priv->miiBus->mdio_lock);

    /* Write Data [31:16] out */
    regAddr           = (priv->switchId<<2)|(REG_ADDR_BIT1_DATA<<1)|(REG_ADDR_BIT0_WRITE);
    regVal = (reg_value >> 16)&0xffff;
    mutex_lock(&priv->miiBus->mdio_lock);
    priv->miiBus->write(priv->miiBus, devAddr, regAddr, regVal);
    mutex_unlock(&priv->miiBus->mdio_lock);

    /* Write Data [15:0] out */
    regVal = reg_value&0xffff;
    mutex_lock(&priv->miiBus->mdio_lock);
    priv->miiBus->write(priv->miiBus, devAddr, regAddr, regVal);
    mutex_unlock(&priv->miiBus->mdio_lock);

    return 0;
}

int yt_mdio_switch_read(struct motorcomm_priv *priv, u32 reg_addr, u32 *pRegValue)
{
    u32 rData;
    u8 devAddr;
    u8 regAddr;
    u16 regVal;

    if (NULL == priv || NULL == priv->miiBus || NULL == priv->miiBus->read || NULL == priv->miiBus->write)
    {
        return -EINVAL;
    }

    devAddr           = priv->devAddr;
    regAddr           = (priv->switchId<<2)|(REG_ADDR_BIT1_ADDR<<1)|(REG_ADDR_BIT0_READ);
    /* Set reg_addr[31:16] */
    regVal = (reg_addr >> 16)&0xffff;
    mutex_lock(&priv->miiBus->mdio_lock);
    priv->miiBus->write(priv->miiBus, devAddr, regAddr, regVal);
    mutex_unlock(&priv->miiBus->mdio_lock);

    /* Set reg_addr[15:0] */
    regVal = reg_addr&0xffff;
    mutex_lock(&priv->miiBus->mdio_lock);
    priv->miiBus->write(priv->miiBus, devAddr, regAddr, regVal);
    mutex_unlock(&priv->miiBus->mdio_lock);

    regAddr           = (priv->switchId<<2)|(REG_ADDR_BIT1_DATA<<1)|(REG_ADDR_BIT0_READ);
    /* Read Data [31:16] */
    regVal = 0x0;
    mutex_lock(&priv->miiBus->mdio_lock);
    regVal = priv->miiBus->read(priv->miiBus, devAddr, regAddr);
    mutex_unlock(&priv->miiBus->mdio_lock);
    rData = (u32)(regVal<<16);

    /* Read Data [15:0] */
    regVal = 0x0;
    mutex_lock(&priv->miiBus->mdio_lock);
    regVal = priv->miiBus->read(priv->miiBus, devAddr, regAddr);
    mutex_unlock(&priv->miiBus->mdio_lock);
    rData |= regVal;

    *pRegValue = rData;

    return 0;
}

static void
yt_reg_write(struct dsa_switch *ds, u32 reg_addr, u32 reg_value)
{
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->reg_mutex);
    yt_mdio_switch_write(priv, reg_addr, reg_value);
    mutex_unlock(&priv->reg_mutex);
}

static void
yt_reg_read(struct dsa_switch *ds, u32 reg_addr, u32 *pRegValue)
{
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->reg_mutex);
    yt_mdio_switch_read(priv, reg_addr, pRegValue);
    mutex_unlock(&priv->reg_mutex);
}

static void
yt_reg_rmw(struct dsa_switch *ds, u32 reg, u32 mask, u32 set)
{
    u32 val;
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->reg_mutex);
    yt_mdio_switch_read(priv, reg, &val);
    val &= ~mask;
    val |= set;
    yt_mdio_switch_write(priv, reg, val);
    mutex_unlock(&priv->reg_mutex);
}

static const struct motorcomm_reg_ops yt_reg_ops =
{
    .reg_write = yt_reg_write,
    .reg_read = yt_reg_read,
    .reg_rmw = yt_reg_rmw,
};

/* MDIO -- end */

extern struct mii_bus *mdio_bus_npe;

/* according to test board */
static int motorcomm_probe(struct platform_device *pdev)
{
    int ret;
    struct device *dev = &pdev->dev;
    struct motorcomm_priv *priv;
    const struct motorcomm_mdio_variant *var;
    struct device_node *np = pdev->dev.of_node;
    struct device_node *mdio;
    struct mii_bus *mdio_bus;
#if (KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE && KERNEL_VERSION(4, 20, 0) > LINUX_VERSION_CODE)
    var = of_device_get_match_data(dev);

    priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
    if (!priv)
    	return -ENOMEM;

    priv->ds = dsa_switch_alloc(dev, DSA_MAX_PORTS);
#elif (KERNEL_VERSION(5, 10, 0) <= LINUX_VERSION_CODE && KERNEL_VERSION(5, 11, 0) > LINUX_VERSION_CODE)
    struct dsa_switch* ds;

    var = of_device_get_match_data(dev);

    priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
    if (!priv) {
        return -ENOMEM;
	}
    ds = devm_kzalloc(dev, sizeof(*ds), GFP_KERNEL);
    priv->ds = ds;

    mdio = of_parse_phandle(np, "mdio-bus", 0);
    if (!mdio)
        return -EINVAL;

    mdio_bus = of_mdio_find_bus(mdio);

    if (!mdio_bus)
        return -EPROBE_DEFER;

    pr_debug("mdio_bus_npe=%px from dts\n", mdio_bus);
#endif
    if (!priv->ds) {
        return -ENOMEM;
	}

    priv->dev = dev;

    /* mdio ctrlif */
    priv->miiBus = mdio_bus;        /* porting according to platform */
    priv->ops = &yt_reg_ops;

    priv->ds->ops = var->ds_ops;
    priv->switchId = var->switchId;
    priv->devAddr = var->devAddr;

    priv->ds->priv = priv;
    priv->ds->dev = dev;
    priv->ds->num_ports = 10;

    dev_set_drvdata(dev, priv);
    mutex_init(&priv->reg_mutex);
    mutex_init(&priv->cfg_mutex);

    ret = dsa_register_switch(priv->ds);
    if (ret) {
        dev_err(dev, "unable to register switch ret = %d ?\n", ret);
        return ret;
    }

    var->ops->init(priv);

    return 0;
}

static int motorcomm_remove(struct platform_device *pdev)
{
    struct motorcomm_priv *priv = dev_get_drvdata(&pdev->dev);

    dsa_unregister_switch(priv->ds);

    mutex_destroy(&priv->reg_mutex);
    mutex_destroy(&priv->cfg_mutex);

	return 0;
}

extern struct motorcomm_mdio_variant yt921x_variant;
static const struct of_device_id motorcomm_of_match[] = {
    {
        .compatible = "motorcomm,yt921x",
        .data = &yt921x_variant,
    },
    { /* sentinel */ },
};
MODULE_DEVICE_TABLE(of, motorcomm_of_match);

static struct platform_driver motorcomm_driver = {
    .driver = {
        .name = "motorcomm_mdio",
        .of_match_table = of_match_ptr(motorcomm_of_match),
    },
    .probe  = motorcomm_probe,
    .remove = motorcomm_remove,
};
module_platform_driver(motorcomm_driver);

MODULE_LICENSE("GPL");
