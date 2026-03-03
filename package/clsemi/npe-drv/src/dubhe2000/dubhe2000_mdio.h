
/* dubhe1000_mdio.h
 * Structures, enums, and macros for the mac + pcs
 */

#ifndef _DUBHE1000_MDIO_H_
#define _DUBHE1000_MDIO_H_

struct dubhe1000_mdio {
	void __iomem *ioaddr;
	struct mii_bus *bus;
	u32 clk_csr;
	u64 pse_bitmap;
	u32 id;
	struct dubhe1000_adapter *adapter;
};

int dubhe1000_mdio_unregister(struct dubhe1000_adapter *adapter, int id);
void *dubhe1000_mdio_register(struct dubhe1000_adapter *adapter, int id);
int dubhe1000_mdio_register_by_phy(struct net_device *ndev);
#endif /* _DUBHE1000_MDIO_H_*/
