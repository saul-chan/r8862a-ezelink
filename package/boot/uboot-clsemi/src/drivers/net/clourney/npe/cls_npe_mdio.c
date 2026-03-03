#include <common.h>
#include <dm.h>
#include <errno.h>
#include <miiphy.h>
#include <malloc.h>
#include <pci.h>
#include <linux/compiler.h>
#include <linux/err.h>
#include <asm/io.h>
#include <power/regulator.h>
#include <linux/iopoll.h>
#include "cls_npe.h"

#define XGMAC_MDIO_C22P			0x00000220
#define XGMAC_MDIO_ADDR			0x00000200
#define XGMAC_MDIO_DATA			0x00000204
#define MDIO_CLK_CSR_SHIFT      (19)
#define MDIO_CLK_CSR_MASK       (GENMASK(21, 19))

#define MII_BUSY 0x00000001
#define MII_WRITE 0x00000002
#define MII_DATA_MASK (GENMASK(15, 0))

/* XGMAC defines */
#define MII_XGMAC_SADDR			BIT(18)
#define MII_XGMAC_CMD_SHIFT		16
#define MII_XGMAC_CMD_PSE       30
#define MII_XGMAC_WRITE			((1 << MII_XGMAC_CMD_SHIFT) | (1 << MII_XGMAC_CMD_PSE))
#define MII_XGMAC_READ			((3 << MII_XGMAC_CMD_SHIFT) | (1 << MII_XGMAC_CMD_PSE))
#define MII_XGMAC_BUSY			BIT(22)
#define MII_XGMAC_MAX_C22ADDR		4
#define MII_XGMAC_C22P_MASK		GENMASK(MII_XGMAC_MAX_C22ADDR, 0)
#define MII_XGMAC_PA_SHIFT		16
#define MII_XGMAC_DA_SHIFT		21

#define MII_ADDR_C45		(1<<30)
#define MII_DEVADDR_C45_SHIFT	16
#define MII_REGADDR_C45_MASK	GENMASK(15, 0)

#ifndef GET_BIT
#define GET_BIT(_x, _pos) \
	(((_x) >> (_pos)) & 1)
#endif

#ifndef CLEAR_BIT
#define CLEAR_BIT(data, bit)		((data) & (~(0x1 << (bit))))
#endif

static char * mdio_name_list[] = {
 "MDIO0",
 "MDIO1",
 "MDIO2",
 "MDIO3",
 "MDIO4",
};

extern int g_skip_qsgmii;
int g_clk_csr_h = 0;
struct mii_dev * mii_bus_list[CLS_NPE_MAX_XGMAC_NUM] = {0};
extern void rtl_set_phyaddr_to_sdk(struct phy_device *phydev, int port);

static int xgmac_c45_format(struct cls_xgmac_priv *priv, int phyaddr,
		int phyreg, u32 *hw_addr)
{
	u32 tmp;

	/* Set port as Clause 45 */
	tmp = readl(priv->ioaddr + XGMAC_MDIO_C22P);
	tmp &= ~BIT(phyaddr);
	writel(tmp, priv->ioaddr + XGMAC_MDIO_C22P);

	*hw_addr = (phyaddr << MII_XGMAC_PA_SHIFT) | (phyreg & 0xffff);
	*hw_addr |= (phyreg >> MII_DEVADDR_C45_SHIFT) << MII_XGMAC_DA_SHIFT;
	return 0;
}

static int xgmac_c22_format(struct cls_xgmac_priv *priv, int phyaddr,
		int phyreg, u32 *hw_addr)
{
	u32 tmp;

	if (phyaddr & ~MII_XGMAC_C22P_MASK)
		return -ENODEV;

	/* Set port as Clause 22 */
	tmp = readl(priv->ioaddr + XGMAC_MDIO_C22P);
	tmp &= ~MII_XGMAC_C22P_MASK;
	tmp |= BIT(phyaddr);
	writel(tmp, priv->ioaddr + XGMAC_MDIO_C22P);

	*hw_addr = (phyaddr << MII_XGMAC_PA_SHIFT) | (phyreg & 0x1f);
	return 0;
}

static int xgmac_mdio_read(struct mii_dev *bus, int phyaddr, int devad, int phyreg)
{
	struct cls_xgmac_priv *priv = bus->priv;
	u32 tmp, addr, value = MII_XGMAC_BUSY;
	int ret = 0;

	if (g_debug_cls&1)
		printf("mdio read phyaddr=%#x devad=%#x phyreg=%#x\n", phyaddr, devad, phyreg);

	/* Wait until any existing MII operation is complete */
	if (readl_poll_timeout(priv->ioaddr + XGMAC_MDIO_DATA, tmp,
				!(tmp & MII_XGMAC_BUSY), 10000)) {
		ret = -EBUSY;
		goto err_disable_clks;
	}

	if (devad != MDIO_DEVAD_NONE) {
		phyreg |= devad << MII_DEVADDR_C45_SHIFT;
		phyreg |= MII_ADDR_C45;
	}

	if (phyreg & MII_ADDR_C45) {
		phyreg &= ~MII_ADDR_C45;

		ret = xgmac_c45_format(priv, phyaddr, phyreg, &addr);
		if (ret)
			goto err_disable_clks;
	} else {
		ret = xgmac_c22_format(priv, phyaddr, phyreg, &addr);
		if (ret)
			goto err_disable_clks;

		//value |= MII_XGMAC_SADDR;
	}
    if (g_clk_csr_h)
	value |= BIT(31) | (priv->clk_csr << MDIO_CLK_CSR_SHIFT) & MDIO_CLK_CSR_MASK;
	else
	value |= (priv->clk_csr << MDIO_CLK_CSR_SHIFT) & MDIO_CLK_CSR_MASK;
	value |= MII_XGMAC_READ;

	/* Wait until any existing MII operation is complete */
	if (readl_poll_timeout(priv->ioaddr + XGMAC_MDIO_DATA, tmp,
				!(tmp & MII_XGMAC_BUSY), 10000)) {
		ret = -EBUSY;
		goto err_disable_clks;
	}

	/* Set the MII address register to read */
	writel(addr, priv->ioaddr + XGMAC_MDIO_ADDR);

	if (g_debug_cls&1)
	printf("=r=)write Address[%#x], value=[%#x]\n",priv->ioaddr + XGMAC_MDIO_ADDR, addr);

	if (GET_BIT(priv->first_pre, phyaddr)) {
		if (g_debug_cls&1)
			printf("Clear MII_XGMAC_CMD_PSE\n");
		value = CLEAR_BIT(value, MII_XGMAC_CMD_PSE);
		//priv->first_pre = CLEAR_BIT(priv->first_pre, phyaddr);
	}

	writel(value, priv->ioaddr + XGMAC_MDIO_DATA);

	if (g_debug_cls&1)
	printf("=r=)write Data[%#x], value=[%#x]\n",priv->ioaddr + XGMAC_MDIO_DATA, value);

	/* Wait until any existing MII operation is complete */
	if (readl_poll_timeout(priv->ioaddr + XGMAC_MDIO_DATA, tmp,
				!(tmp & MII_XGMAC_BUSY), 10000)) {
		ret = -EBUSY;
		goto err_disable_clks;
	}

	/* Read the data from the MII data register */
	ret = (int)readl(priv->ioaddr + XGMAC_MDIO_DATA) & GENMASK(15, 0);

#if 0
	if (ret != 0xffff) {
		printf("MDIO add pre form addr(%#x) ret=%#x\n",phyaddr, ret);
		priv->first_pre = 0;
	}
#endif

	if (g_debug_cls&1)
		printf("=r=)read Data[%#x], value=[%#x]\n",priv->ioaddr + XGMAC_MDIO_DATA, ret);

	if (g_debug_cls&2)
		printf("mdio read phyaddr=%#x phyreg=%d value=%#x\n", phyaddr, phyreg, ret);


err_disable_clks:
	pm_runtime_put(priv->device);

	return ret;

	return -ETIMEDOUT;
}

static int xgmac_mdio_write(struct mii_dev *bus, int phyaddr,int devad,
		int phyreg,  u16 phydata)
{
	struct cls_xgmac_priv *priv = bus->priv;
	u32 addr, tmp, value = MII_XGMAC_BUSY;
	int ret;

	if (g_debug_cls)
		printf("mdio write phyaddr=%#x devad=%#x phyreg=%x value=%#x\n", phyaddr, devad,  phyreg, phydata);

	/* Wait until any existing MII operation is complete */
	if (readl_poll_timeout(priv->ioaddr + XGMAC_MDIO_DATA, tmp,
				!(tmp & MII_XGMAC_BUSY), 10000)) {
		ret = -EBUSY;
		goto err_disable_clks;
	}

	if (devad != MDIO_DEVAD_NONE) {
		phyreg |= devad << MII_DEVADDR_C45_SHIFT;
		phyreg |= MII_ADDR_C45;
	}

	if (phyreg & MII_ADDR_C45) {
		phyreg &= ~MII_ADDR_C45;

		ret = xgmac_c45_format(priv, phyaddr, phyreg, &addr);
		if (ret)
			goto err_disable_clks;
	} else {
		ret = xgmac_c22_format(priv, phyaddr, phyreg, &addr);
		if (ret)
			goto err_disable_clks;

	//	value |= MII_XGMAC_SADDR;
	}

    if (g_clk_csr_h)
	value |= BIT(31) | (priv->clk_csr << MDIO_CLK_CSR_SHIFT) & MDIO_CLK_CSR_MASK;
	else
	value |= (priv->clk_csr << MDIO_CLK_CSR_SHIFT) & MDIO_CLK_CSR_MASK;
	value |= phydata;
	value |= MII_XGMAC_WRITE;

	if (GET_BIT(priv->first_pre, phyaddr)) {
		if (g_debug_cls&1)
			printf("Clear MII_XGMAC_CMD_PSE\n");
		value = CLEAR_BIT(value, MII_XGMAC_CMD_PSE);
	}

	/* Wait until any existing MII operation is complete */
	if (readl_poll_timeout(priv->ioaddr + XGMAC_MDIO_DATA, tmp,
				!(tmp & MII_XGMAC_BUSY), 10000)) {
		ret = -EBUSY;
		goto err_disable_clks;
	}

	if (g_debug_cls&1) {
		printf("=w=)write Address[%#x], value=[%#x]\n",priv->ioaddr + XGMAC_MDIO_ADDR, addr);
		printf("=w=)write Data[%#x], value=[%#x]\n",priv->ioaddr + XGMAC_MDIO_DATA, value);
	}

	/* Set the MII address register to write */
	writel(addr, priv->ioaddr + XGMAC_MDIO_ADDR);
	writel(value, priv->ioaddr + XGMAC_MDIO_DATA);

	/* Wait until any existing MII operation is complete */
	ret = readl_poll_timeout(priv->ioaddr + XGMAC_MDIO_DATA, tmp,
			!(tmp & MII_XGMAC_BUSY), 10000);

err_disable_clks:
	pm_runtime_put(priv->device);

	return ret;
}

#ifdef CONFIG_CLS_PHYLINK
//<ADDR MASK VALUE>
int cls_of_regs_exe(ofnode mdio_handle, char * name)
{
	const __be32 *paddr;
	int len, i;

	if (!name)
		return -1;

	paddr = ofnode_get_property(mdio_handle,
			name, &len);
	if (!paddr || len < (3 * sizeof(*paddr)))
		return -1;

	len /= sizeof(*paddr);
	for (i = 0; i < len - 2; i += 3) {
		u32 reg = be32_to_cpup(paddr + i);
		u32 mask = be32_to_cpup(paddr + i + 1);
		u32 val_bits = be32_to_cpup(paddr + i + 2);
		int val;
		val = 0;
		val = readl(reg);
		val &= ~mask;
		val |= val_bits;
		if (g_debug_cls)
			printf("mdio regs set addr[%#x] value[%#x]\n", reg, val);
		writel(val, reg);
	}

	return 0;
}

void  cls_mdio_reset_phys(ofnode mdio_handle)
{
	if (!cls_of_regs_exe(mdio_handle, "regs,phys_reset_init"))
		mdelay(100);

	if (!cls_of_regs_exe(mdio_handle, "regs,phys_dereset"))
		mdelay(500);
}

int cls_mdio_init(const char *name, ofnode mdio_node, void *priv)
{
	struct cls_xgmac_priv *mac_priv = priv;
	struct mii_dev *bus = mdio_alloc();

	if (!bus) {
		printf("Failed to allocate MDIO bus\n");
		return -ENOMEM;
	}

	bus->read = xgmac_mdio_read;
	bus->write = xgmac_mdio_write;
	snprintf(bus->name, sizeof(bus->name), "%s", name);
	bus->priv = priv;

	//mac_priv->bus = bus;

	mii_bus_list[mac_priv->id] = bus;

	if (mdio_node.np) {
		mac_priv->mdio_handle = mdio_node;
		/* Default to get clk_csr 100-125Mhz,
		 * or get clk_csr from device tree.
		 */
		mac_priv->clk_csr = 0;
		ofnode_read_u32(mdio_node, "clk_csr", &mac_priv->clk_csr);
		cls_eth_print("clk_csr = %d\n", mac_priv->clk_csr);

		cls_mdio_reset_phys(mac_priv->mdio_handle);
	}

	printf("Register MDIO(%s) in XGMAC%d, bus=%#x\n", name, mac_priv->id, bus);

	return mdio_register(bus);
}
#endif

#if 0
/**
 * xgmac_mdio_unregister
 * @ndev: net device structure
 * Description: it unregisters the MII bus
 */
int xgmac_mdio_unregister(struct net_device *ndev)
{
	struct cls_xgmac_priv *priv = netdev_priv(ndev);

	if (!priv->mii)
		return 0;

	mdiobus_unregister(priv->mii);
	priv->mii->priv = NULL;
	mdiobus_free(priv->mii);
	priv->mii = NULL;

	return 0;
}
#endif

int try_get_port_id_by_handle(struct cls_xgmac_priv *priv)
{
	int id;
	__be32 *_id;
	if (!ofnode_valid(priv->phy_handle)) {
		printf("xgmac%d: no phy_handle found !!!\n", priv->id);
		return -1;
	}

	_id = ofnode_get_property(priv->phy_handle, "port-id", NULL);
	id = be32_to_cpup(_id);
	if (id >= 32) {
		cls_eth_print("%d is not a valid mac id\n", id);
		return -1;
	}

	return id;
}

struct mii_dev * get_bus_by_phy_handle(struct cls_xgmac_priv *priv)
{
	__be32 *_id;
	int id;
	ofnode mdio_node, mac_node;
	struct mii_dev * bus = NULL;

	struct cls_eth_priv *adapter = priv->adapter;

	if (!adapter) {
		printf("xgmac%d: Can't found adapter!!!\n", priv->id);
		return NULL;
	}

	if (!ofnode_valid(priv->phy_handle)) {
		printf("xgmac%d: no phy_handle found !!!\n", priv->id);
		return NULL;
	}

	mdio_node = ofnode_get_parent(priv->phy_handle);
	if (!ofnode_valid(mdio_node)) {
		printf("xgmac%d: no mdio DTS found !!!\n", priv->id);
		return NULL;
	}

	mac_node = ofnode_get_parent(mdio_node);
	if (!ofnode_valid(mac_node)) {
		printf("xgmac%d: no mac DTS found !!!\n", priv->id);
		return NULL;
	}

	_id = ofnode_get_property(mac_node, "id", NULL);
	id = be32_to_cpup(_id);
	if (id >= CLS_NPE_MAX_XGMAC_NUM) {
		cls_eth_print("%d is not a valid mac id\n", id);
		return NULL;
	}

	printf("xgmac%d found bus in xgmac[%d]\n", priv->id, id);

	bus = mii_bus_list[id];
	if (!bus) {
		if (g_debug_cls)
			printf("===>cls_mdio_init init mdio%d !!!\n", id);
		cls_mdio_init(mdio_name_list[id], mdio_node, &adapter->xgmac[id]);
		adapter->xgmac[id].flags |= BIT(FLAGS_MDIO_INITD_BIT);
		bus = mii_bus_list[id];
	}

	if (!bus)
		printf("Can't found MDIO bus obj from XGMAC%d\n", id);

	return bus;
}

#ifdef CONFIG_CLS_PHYLINK
int cls_phy_init(struct cls_xgmac_priv *priv, void *dev)
{
	int *pflags = NULL;
	struct phy_device *phydev;
	int mask = 0, ret, need_config = 0;
	struct cls_xgmac_priv *mdio_xgmac;

	if (g_debug_cls)
		printf("cls_phy_init __Enter\n");

	if (!priv->fixed_link) {
		priv->bus = get_bus_by_phy_handle(priv);
		if (!priv->bus) {
			printf("xgmac%d no found mido bus\n", priv->id);
			return -1;
		}

		mdio_xgmac = priv->bus->priv;
		if (priv->mdio_flags) {
			mdio_xgmac->first_pre |= BIT(priv->phy_addr);
			printf("Phy%d, MDIO%d Disable PSE on PHY addr %#x, mask %#x\n",
					priv->id,
					mdio_xgmac->id,
					priv->phy_addr,
					mdio_xgmac->first_pre);
		}
	}

	/* multi phy(qsgmii/2*uxgmii) reset first */
	if (priv->interface == XGMAC_INTERFACE_MODE_RGMII) {
		pflags = &priv->flags;
	} else  {
		pflags = &priv->xpcs->flags;
	}

	if (!(*pflags & BIT(FLAGS_PHY_RST_BIT))) {
		need_config = 1;
		*pflags |= BIT(FLAGS_PHY_RST_BIT);
	}

	if (priv->fixed_link)
		phydev = fixed_phy_create(priv->phy_handle);
	else {
		mask = BIT(priv->phy_addr);
		//printf("phy_find_by_mask ==)\n");
		phydev = phy_find_by_mask(priv->bus, mask, priv->phy_interface);
	}

	if (!phydev) {
		printf("Can't find the phy in XGMAC%d\n", priv->id);
		return -ENODEV;
	}

	//printf("phy_connect_dev ==)\n");
	phy_connect_dev(phydev, dev);

	if (need_config) {
		if (phydev->drv) {
			printf( "drv id[%#x] name[%s]\n",phydev->drv->uid, phydev->drv->name);
		}
		if ( (!g_skip_qsgmii && phydev->phy_id == 0x1cc981) ||
				phydev->phy_id  == 0x1ccaf3 ||
				phydev->phy_id == 0x001cc849 ||
				phydev->phy_id == 0x001ccad0
		   ) { //rtl8214fc rtl8261BE rtl8221B

#ifdef CONFIG_CLS_RTKSDK
			int port_id = try_get_port_id_by_handle(priv);

			rtl_set_phyaddr_to_sdk(phydev, port_id);
			if (-1 == port_id) {
				printf("rtl8214fc must set port-id in dts\n");
				return -ENODEV;
			}

			rtlsdk_phy_need_init();

			printf("rtlsdk port_id %d\n", port_id);
#else
			printf("!!!ERROR: Please open CONFIG_CLS_RTKSDK\n");
#endif
		} else if (phydev->phy_id  == 0x4f51e91b) { //YT8531
			phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0xa003);
			phy_write(phydev, MDIO_DEVAD_NONE, 0x1f, 0xff);

			phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0xa010);
			phy_write(phydev, MDIO_DEVAD_NONE, 0x1f, 0xDBEF);

			phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x0);
		} else if (phydev->phy_id  == 0x4f51ea19) { //YT8821
			if (priv->xpcs)
				priv->xpcs->hsgmii_autoneg = HSGMII_AUTONEG_OFF;
			printf("The phy is YT8821\n");
		}
	}

	phydev->supported &= PHY_GBIT_FEATURES;
	if (priv->max_speed) {
		u32 max_speed = priv->max_speed > SPEED_1000 ? SPEED_1000 : priv->max_speed;
		printf("XGMAC%d set phy max speed is %d\n",priv->id, max_speed);
		ret = phy_set_supported(phydev, max_speed);
		if (ret)
			printf("XGMAC%d set PHY MAX speed: %d Failed\n",priv->id, max_speed);
	}

	phydev->advertising = phydev->supported;

	priv->phydev = phydev;

//	if (need_config) {
	printf("Config PHY for MAC%d\n", priv->id);
	phy_config(phydev);
//	}

	printf("cls_phy_init  END\n");
	return 0;
}
#endif
