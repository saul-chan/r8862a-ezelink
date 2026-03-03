/* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*/

#ifndef _MOTORCOMM_MDIO_H
#define _MOTORCOMM_MDIO_H

#include <linux/phy.h>
#include <net/dsa.h>

struct motorcomm_reg_ops {
    void (*reg_write)(struct dsa_switch *, u32, u32);
    void (*reg_read)(struct dsa_switch *, u32, u32 *);
    void (*reg_rmw)(struct dsa_switch *, u32, u32, u32);
};

struct motorcomm_priv {
    struct device       *dev;
    struct dsa_switch   *ds;
	struct mii_bus *bus;
    u8  switchId;
    u8  devAddr;
    struct mii_bus *miiBus;
    const struct motorcomm_reg_ops *ops;
    /* protect among processes for registers access*/
    struct mutex reg_mutex;
    /* protect among processes for config access*/
    struct mutex cfg_mutex;
};

/**
 * struct motorcomm_chip_ops
 */
struct motorcomm_chip_ops {
    void (*init)(struct motorcomm_priv *);
};

struct motorcomm_mdio_variant {
    /* mdio param */
    u8  switchId;
    u8  devAddr;
    const struct dsa_switch_ops *ds_ops;
    const struct motorcomm_chip_ops *ops;
};
#endif /* _MOTORCOMM_DSA_H */
