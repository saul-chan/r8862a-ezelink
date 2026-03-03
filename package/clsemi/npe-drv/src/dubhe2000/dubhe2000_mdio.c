#include <linux/gpio/consumer.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/mii.h>
#include <linux/of_mdio.h>
#include <linux/pm_runtime.h>
#include <linux/phy.h>
#include <linux/property.h>
#include <linux/slab.h>

#include <linux/iopoll.h>
#include "dubhe2000.h"
#include "dubhe2000_mdio.h"

#define XGMAC_MDIO_C22P	   0x00000220
#define XGMAC_MDIO_ADDR	   0x00000200
#define XGMAC_MDIO_DATA	   0x00000204
#define MDIO_CLK_CSR_SHIFT 19
#define MDIO_CLK_CSR_MASK  GENMASK(21, 19)

#define MII_BUSY	   0x00000001
#define MII_WRITE	   0x00000002
#define MII_DATA_MASK	   GENMASK(15, 0)

/* XGMAC defines */
#define MII_XGMAC_SADDR	      BIT(18)
#define MII_XGMAC_CMD_SHIFT   16
#define MII_XGMAC_CMD_PSE     30
#define MII_XGMAC_WRITE	      (1 << MII_XGMAC_CMD_SHIFT)
#define MII_XGMAC_READ	      (3 << MII_XGMAC_CMD_SHIFT)
#define MII_XGMAC_BUSY	      BIT(22)
#define MII_XGMAC_MAX_C22ADDR 4
#define MII_XGMAC_C22P_MASK   GENMASK(MII_XGMAC_MAX_C22ADDR, 0)
#define MII_XGMAC_PA_SHIFT    16
#define MII_XGMAC_DA_SHIFT    21

#ifndef GET_BIT
#define GET_BIT(_x, _pos) (((_x) >> (_pos)) & 1)
#endif

static int xgmac_c45_format(struct dubhe1000_mdio *port, int phyaddr, int phyreg, u32 *hw_addr)
{
	u32 tmp;

	/* Set port as Clause 45 */
	tmp = readl(port->ioaddr + XGMAC_MDIO_C22P);
	tmp &= ~BIT(phyaddr);
	writel(tmp, port->ioaddr + XGMAC_MDIO_C22P);

	*hw_addr = (phyaddr << MII_XGMAC_PA_SHIFT) | (phyreg & 0xffff);
	*hw_addr |= (phyreg >> MII_DEVADDR_C45_SHIFT) << MII_XGMAC_DA_SHIFT;
	return 0;
}

static int xgmac_c22_format(struct dubhe1000_mdio *port, int phyaddr, int phyreg, u32 *hw_addr)
{
	u32 tmp;

	if (phyaddr & ~MII_XGMAC_C22P_MASK)
		return -ENODEV;

	/* Set port as Clause 22 */
	tmp = readl(port->ioaddr + XGMAC_MDIO_C22P);
	tmp &= ~MII_XGMAC_C22P_MASK;
	tmp |= BIT(phyaddr);
	writel(tmp, port->ioaddr + XGMAC_MDIO_C22P);

	*hw_addr = (phyaddr << MII_XGMAC_PA_SHIFT) | (phyreg & 0x1f);
	return 0;
}

static int xgmac_mdio_read(struct mii_bus *bus, int phyaddr, int phyreg)
{
	struct dubhe1000_mdio *port = bus->priv;
	u32 tmp, addr, value = MII_XGMAC_BUSY;
	int ret = 0;

	/* Wait until any existing MII operation is complete */
	if (readl_poll_timeout(port->ioaddr + XGMAC_MDIO_DATA, tmp, !(tmp & MII_XGMAC_BUSY), 100, 10000)) {
		ret = -EBUSY;
		goto err_disable_clks;
	}

	if (phyreg & MII_ADDR_C45) {
		phyreg &= ~MII_ADDR_C45;

		ret = xgmac_c45_format(port, phyaddr, phyreg, &addr);
		if (ret)
			goto err_disable_clks;
	} else {
		ret = xgmac_c22_format(port, phyaddr, phyreg, &addr);
		if (ret)
			goto err_disable_clks;

		//value |= MII_XGMAC_SADDR;
	}

	value |= (port->clk_csr << MDIO_CLK_CSR_SHIFT) & MDIO_CLK_CSR_MASK;
	value |= MII_XGMAC_READ;

	if (GET_BIT(port->pse_bitmap, phyaddr))
		value |= BIT(MII_XGMAC_CMD_PSE);

	/* Wait until any existing MII operation is complete */
	if (readl_poll_timeout(port->ioaddr + XGMAC_MDIO_DATA, tmp, !(tmp & MII_XGMAC_BUSY), 100, 10000)) {
		ret = -EBUSY;
		goto err_disable_clks;
	}

	/* Set the MII address register to read */
	writel(addr, port->ioaddr + XGMAC_MDIO_ADDR);
	writel(value, port->ioaddr + XGMAC_MDIO_DATA);

	/* Wait until any existing MII operation is complete */
	if (readl_poll_timeout(port->ioaddr + XGMAC_MDIO_DATA, tmp, !(tmp & MII_XGMAC_BUSY), 100, 10000)) {
		ret = -EBUSY;
		goto err_disable_clks;
	}

	/* Read the data from the MII data register */
	ret = (int)readl(port->ioaddr + XGMAC_MDIO_DATA) & GENMASK(15, 0);

err_disable_clks:
	pm_runtime_put(bus->parent);

	return ret;
}

static int xgmac_mdio_write(struct mii_bus *bus, int phyaddr, int phyreg, u16 phydata)
{
	struct dubhe1000_mdio *port = bus->priv;
	u32 addr, tmp, value = MII_XGMAC_BUSY;
	int ret;

	/* Wait until any existing MII operation is complete */
	if (readl_poll_timeout(port->ioaddr + XGMAC_MDIO_DATA, tmp, !(tmp & MII_XGMAC_BUSY), 100, 10000)) {
		ret = -EBUSY;
		goto err_disable_clks;
	}

	if (phyreg & MII_ADDR_C45) {
		phyreg &= ~MII_ADDR_C45;

		ret = xgmac_c45_format(port, phyaddr, phyreg, &addr);
		if (ret)
			goto err_disable_clks;
	} else {
		ret = xgmac_c22_format(port, phyaddr, phyreg, &addr);
		if (ret)
			goto err_disable_clks;

		//	value |= MII_XGMAC_SADDR;
	}

	value |= (port->clk_csr << MDIO_CLK_CSR_SHIFT) & MDIO_CLK_CSR_MASK;
	value |= phydata;
	value |= MII_XGMAC_WRITE;

	if (GET_BIT(port->pse_bitmap, phyaddr))
		value |= BIT(MII_XGMAC_CMD_PSE);

	/* Wait until any existing MII operation is complete */
	if (readl_poll_timeout(port->ioaddr + XGMAC_MDIO_DATA, tmp, !(tmp & MII_XGMAC_BUSY), 100, 10000)) {
		ret = -EBUSY;
		goto err_disable_clks;
	}

	/* Set the MII address register to write */
	writel(addr, port->ioaddr + XGMAC_MDIO_ADDR);
	writel(value, port->ioaddr + XGMAC_MDIO_DATA);

	/* Wait until any existing MII operation is complete */
	ret = readl_poll_timeout(port->ioaddr + XGMAC_MDIO_DATA, tmp, !(tmp & MII_XGMAC_BUSY), 100, 10000);

err_disable_clks:
	pm_runtime_put(bus->parent);

	return ret;
}

int dubhe1000_of_regs_exe(struct device_node *mdio_handle, char *name)
{
	const __be32 *paddr;
	void __iomem *mmu_addr = NULL;
	int len, i;

	if (!mdio_handle || !name)
		return -1;

	paddr = of_get_property(mdio_handle, name, &len);
	if (!paddr || len < (3 * sizeof(*paddr)))
		return -1;

	len /= sizeof(*paddr);
	for (i = 0; i < len - 2; i += 3) {
		u32 reg = be32_to_cpup(paddr + i);
		u32 mask = be32_to_cpup(paddr + i + 1);
		u32 val_bits = be32_to_cpup(paddr + i + 2);
		int val;

		val = 0;
		mmu_addr = ioremap(reg, 4);
		val = readl(mmu_addr);
		val &= ~mask;
		val |= val_bits;
		writel(val, mmu_addr);
		iounmap(mmu_addr);
	}

	return 0;
}

void dubhe1000_mdio_reset_phys(struct device_node *mdio_handle)
{
	if (!dubhe1000_of_regs_exe(mdio_handle, "regs,phys_reset_init"))
		msleep(100);

	if (!dubhe1000_of_regs_exe(mdio_handle, "regs,phys_dereset"))
		msleep(500);
}

static int xgmac_mdio_reset(struct mii_bus *bus)
{
	struct dubhe1000_mdio *port = bus->priv;

	dubhe1000_mdio_reset_phys(port->adapter->mdio_node[port->id]);

	return 0;
}

/**
 * dubhe1000_mdio_register
 * @ndev: net device structure
 * Description: it registers the MII bus
 */
void *dubhe1000_mdio_register(struct dubhe1000_adapter *adapter, int id)
{
	int err = 0;
	struct mii_bus *mdio_bus = NULL;
	struct dubhe1000_mac *mdio_mac = NULL;
	struct device_node *mdio_node = NULL;
	void __iomem *ioaddr = NULL;
	struct dubhe1000_mdio *mdio_obj = NULL;
	const __be32 *pre_addr;
	int len, i;

	if (id >= DUBHE1000_MDIO_COUNT) {
		pr_err("%d is not a valid mdio id\n", id);
		return ERR_PTR(-EINVAL);
	}

	mdio_node = adapter->mdio_node[id];
	if (!mdio_node)
		return ERR_PTR(-EINVAL);

	mdio_obj = adapter->mdio[id];
	if (mdio_obj)
		return mdio_obj;

	if (verbose)
		pr_info("Found MDIO%d BUS DTS\n", id);

	mdio_mac = adapter->mac[id];
	if (!mdio_mac) {
		struct device_node *np;

		for_each_compatible_node(np, NULL, "clourney,dwmac") {
			u32 tmp_id;

			if (of_property_read_u32(np, "id", &tmp_id))
				continue;

			if (tmp_id == id) {
				ioaddr = devm_of_iomap(adapter->dev, np, 0, NULL);
				if (IS_ERR(ioaddr)) {
					pr_info("GET XGMAC[%d] ioaddr form DTS err = 0x%px\n", id, ioaddr);
					continue;
				}

				break;
			}
		}
	} else
		ioaddr = mdio_mac->ioaddr;

	if (!ioaddr) {
		pr_err("XGMAC%d need MDIO%d,But it init Failed, Because cannot find XGMAC%d dts\n", id, id, id);
		return ERR_PTR(-EINVAL);
	}

	mdio_obj = devm_kzalloc(adapter->dev, sizeof(*mdio_obj), GFP_KERNEL);
	if (!mdio_obj)
		return ERR_PTR(-ENOMEM);

	mdio_bus = devm_mdiobus_alloc(adapter->dev);
	if (!mdio_bus) {
		pr_err("Failed to alloc mii bus\n");
		return ERR_PTR(-ENOMEM);
	}

	mdio_bus->name	= "D2K_MDIO";
	mdio_bus->read	= &xgmac_mdio_read;
	mdio_bus->write	= &xgmac_mdio_write;
	mdio_bus->reset	= &xgmac_mdio_reset;
	mdio_obj->id	= id;
	mdio_obj->bus	= mdio_bus;
	mdio_obj->ioaddr = ioaddr;
	mdio_obj->pse_bitmap = 0;
	mdio_obj->adapter = adapter;

	pre_addr = of_get_property(mdio_node, "pse-bitmap", &len);
	if (pre_addr && (len >= sizeof(*pre_addr))) {
		len /= sizeof(*pre_addr);
		for (i = 0; i < len; i++) {
			u32 reg = be32_to_cpup(pre_addr + i);

			mdio_obj->pse_bitmap |= BIT(reg);
		}
	}

	pr_err("MDIO%d PSE map %#llx\n", id, mdio_obj->pse_bitmap);

	/* Default to get clk_csr 100-125Mhz,
	 * or get clk_csr from device tree.
	 */
	if (of_property_read_u32(mdio_node, "clk_csr", &mdio_obj->clk_csr))
		mdio_obj->clk_csr = 1;

	snprintf(mdio_bus->id, MII_BUS_ID_SIZE, "%s-%x", mdio_bus->name, id);

	mdio_bus->priv = mdio_obj;
	mdio_bus->parent = adapter->dev;

	err = of_mdiobus_register(mdio_bus, mdio_node);
	if (err != 0) {
		pr_err("Cannot register the MDIO%d bus\n", id);
		goto bus_register_fail;
	}

	adapter->mdio[id] = mdio_obj;

	if (verbose)
		pr_err("MDIO%d Registered\n", id);

	return mdio_obj;

bus_register_fail:
	return ERR_PTR(-EINVAL);
}

int dubhe1000_mdio_register_by_phy(struct net_device *ndev)
{
	struct dubhe1000_mac *port = netdev_priv(ndev);
	struct dubhe1000_adapter *adapter = port->adapter;
	struct device_node *mdio_node = NULL;
	struct device_node *mac_node = NULL;
	struct dubhe1000_mdio *mdio_obj = NULL;
	int addr, found, max_addr = PHY_MAX_ADDR;
	uint32_t id;

	if (!port->phy_node) {
		if (verbose)
			pr_err("Can't found PHY DTS in XGMAC%d\n", port->id);
		return 0;
	}

	mdio_node = of_get_parent(port->phy_node);
	if (!mdio_node) {
		pr_err("XGMAC%d: NO MDIO DTS found !!!\n", port->id);
		return 0;
	}

	if (of_property_read_u32(mdio_node, "id", &id)) {
		mac_node = of_get_parent(mdio_node);
		if (!mac_node) {
			pr_err("XGMAC%d: Can't find MDIO ID, Check mdio DTS !!!\n", port->id);
			return 0;
		}

		if (of_property_read_u32(mac_node, "id", &id)) {
			pr_err("XGMAC%d: Can't find MDIO ID !!!\n", port->id);
			return 0;
		}
	}

	if (id >= DUBHE1000_MDIO_COUNT) {
		pr_err("%d is not a valid mdio id\n", id);
		return 0;
	}

	if (verbose)
		pr_info("Found MDIO%d BUS DTS by phy_addr[%#x]\n", id, port->phy_addr);

	if (adapter->mdio_node[id] && (adapter->mdio_node[id] != mdio_node))
		pr_err("Found multiple MDIO%d DTS\n", id);

	adapter->mdio_node[id] = mdio_node;

	mdio_obj = dubhe1000_mdio_register(adapter, id);
	if (IS_ERR(mdio_obj) || !mdio_obj) {
		pr_err("XGMAC%d need MDIO%d,But it init Failed, Because XGMAC%d not inited\n", port->id, id, id);
		return 0;
	}

	found = 0;
	for (addr = 0; addr < max_addr; addr++) {
		struct phy_device *phydev = mdiobus_get_phy(mdio_obj->bus, addr);

		if (verbose)
			pr_err("phy addr %#x,addr %p", addr, phydev);

		if (!phydev)
			continue;

		/*
		 * If we're going to bind the MAC to this PHY bus,
		 * and no PHY number was provided to the MAC,
		 * use the one probed here.
		 */
		if (port->phy_addr == -1)
			port->phy_addr = addr;

		if (verbose)
			pr_err("phy addr %#x", addr);

		if (addr == port->phy_addr)
			phy_attached_info(phydev);

		found = 1;
	}

	if (!found && !mdio_node)
		pr_err("No PHY found\n");

	return 0;
}
/**
 * dubhe1000_mdio_unregister
 * @ndev: net device structure
 * Description: it unregisters the MII bus
 */
int dubhe1000_mdio_unregister(struct dubhe1000_adapter *adapter, int id)
{
	struct dubhe1000_mdio *mdio_obj;

	if (id >= DUBHE1000_MDIO_COUNT)
		return 0;

	mdio_obj = adapter->mdio[id];
	if (!mdio_obj)
		return 0;

	mdiobus_unregister(mdio_obj->bus);

	mdio_obj->bus->priv = NULL;

	return 0;
}
