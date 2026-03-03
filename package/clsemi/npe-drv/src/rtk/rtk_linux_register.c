#include <linux/bitops.h>
#include <linux/phy.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/compat.h>
#include <hal/phy/phydef.h>
#include <hal/common/miim.h>
#include <phy/inc/phy_init.h>
#include <osal/lib.h>
#include <osal/print.h>
#include <common/error.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <hal/phy/nic_rtl8226/nic_rtl8226.h>
#include <hwp/hw_profile.h>
#define PHY_ID_RTL8214FC	0x001cc981
#define PHY_ID_RTL8261BE	0x001ccaf3
#define PHY_ID_RTL8221B		0x001cc849
#define PHY_ID_RTL8224		0x001ccad0

#define RTK_MAX_PORT_NUM   12

static struct {
	int addr;
	struct mii_bus *bus;
} rtk_port_map[RTK_MAX_PORT_NUM];

static int  rtk_port_num  = 0;
static int  rtk_phy_num  = 0;
static int  s_sdk_initd  = 0;
static int  s_phy_reged  = 0;
static phy_hwp_portDescp_t  rtl_port_descp[RTK_MAX_PORT_NUM + 1];
static phy_hwp_phyDescp_t   rtl_phy_Descp[RTK_MAX_PORT_NUM + 1];
extern hwp_hwProfile_t phy_driver_hwp;

static int rtk_get_portid(struct phy_device *phydev)
{
	struct mii_bus *bus =  phydev->mdio.bus; 
	int	addr = phydev->mdio.addr;
	int mac_id = 0; 
	for ( ; mac_id < rtk_port_num; mac_id ++) {
		if((bus == rtk_port_map[mac_id].bus) &&
				(addr == rtk_port_map[mac_id].addr))
			return mac_id;
	}

	printk(KERN_ERR "RTLSDK:%s addr%d not FOUND in port map %d\n", __FUNCTION__, addr, rtk_port_num);

	return -1;
}

static void rtlsdk_phy_need_init(void)
{
	if (s_sdk_initd) {
		printk(KERN_ERR "RTLSDK: phy need init\n");
		s_sdk_initd = 0;
	}
}

static int check_phyDescp_exist(int addr, struct mii_bus *bus) 
{
	int mac_id = 0; 
	for ( ; mac_id < rtk_port_num; mac_id ++) {
		if((bus == rtk_port_map[mac_id].bus) &&
				(addr == rtk_port_map[mac_id].addr))
			return 1;
	}

	return 0;
}

static inline void rtl_fill_sdkDesc(
		struct mii_bus *mdio_bus,
		uint8   eth,        
		uint8   medi,           
		uint8   phy_addr,      
		uint32  chip,     
		uint8   phy_max 
)
{
	uint8   mac_id;        
	uint8   phy_idx;       
	mac_id = rtk_port_num;
	phy_idx = rtk_phy_num;

	if (phy_idx + phy_max > RTK_MAX_PORT_NUM ) {
		printk(KERN_ERR"RTLSDK: Please Check mac_id config!!!\n");
		return;
	}

	if (chip == RTK_PHYTYPE_RTL8226B) {
		int sdsid = 0;
		sdsid = phy_driver_hwp.swDescp[0]->port.descp[mac_id].sds_idx;
		phy_driver_hwp.swDescp[0]->serdes.descp[sdsid].mode = RTK_MII_HISGMII;
	}

	rtl_phy_Descp[phy_idx].chip = chip;
	rtl_phy_Descp[phy_idx].mac_id = mac_id;
	rtl_phy_Descp[phy_idx].phy_max = phy_max;
	rtl_phy_Descp[phy_idx + 1].chip = HWP_END;

	for ( ; mac_id < rtk_port_num + phy_max; mac_id ++) {
		rtl_port_descp[mac_id].phy_idx = phy_idx;
		rtl_port_descp[mac_id].mac_id = mac_id;
		rtl_port_descp[mac_id].eth = eth;
		rtl_port_descp[mac_id].medi = medi;
		rtl_port_descp[mac_id].phy_addr = phy_addr;

		rtk_port_map[mac_id].addr = phy_addr;
		rtk_port_map[ mac_id].bus = mdio_bus;
		phy_addr++;
	}

	rtl_port_descp[mac_id].mac_id = HWP_END;

	rtk_port_num += phy_max;
	rtk_phy_num ++; 
}

static int set_phyinfo2rtk(
		struct phy_device *phydev) 
{
	struct device_node *phy_node = phydev->mdio.dev.of_node;
	unsigned int port = 0;
	const __be32 *_id; 
	int addr;
	struct mii_bus *mdio_bus;

	if (NULL == phydev) {
		printk(KERN_ERR "RTLSDK: phydev is null!!!\n");
		return -1;
	}

	addr = phydev->mdio.addr;
	mdio_bus = phydev->mdio.bus;

	if(check_phyDescp_exist(addr, mdio_bus)) {
		printk(KERN_ERR "RTLSDK: add[%d] config has exist!!!\n", addr);
		return 0;
	}

	//printk(KERN_ERR "%s Line %d ==>addr[%#x]", __func__, __LINE__, addr);
	if (phy_node) {
		_id = of_get_property(phy_node, "port-id", NULL);
		if(_id) {
			port = be32_to_cpup(_id);
			if (port >= 4) {
				printk(KERN_ERR "PHY:%d is not a valid port id", port);
				return -1;
			}
		}
	}

	if (phydev->phy_id == 0x1cc981) {
			//printk(KERN_ERR"======>IS port %d phy_id %d\n", rtk_port_num, rtk_phy_num);
		if (rtk_port_num % 4) {
			rtl_fill_sdkDesc(
					NULL,
					HWP_GE,
					HWP_COPPER,
					0,   
					RTK_PHYTYPE_NONE,
					4 - (rtk_port_num % 4));
		}

		rtl_fill_sdkDesc(
				mdio_bus,
				HWP_GE,
				HWP_COPPER,
				addr - port,   
				RTK_PHYTYPE_RTL8214FC,
				4);
	} else if (phydev->phy_id == 0x1ccaf3) { //8261B 
		//printk(KERN_ERR"======>IS 8261BE\n");
		rtl_fill_sdkDesc(
				mdio_bus,
				HWP_XGE,
				HWP_COPPER,
				addr,   
				RTK_PHYTYPE_RTL8261B,
				1);
	} else if (phydev->phy_id == 0x001cc849) { //8221B 
		//printk(KERN_ERR"======>IS 8221B port %d phy_id %d\n", rtk_port_num, rtk_phy_num);
		rtl_fill_sdkDesc(
				mdio_bus,
				HWP_2_5GE,
				HWP_COPPER,
				addr,   
				RTK_PHYTYPE_RTL8226B,
				1);
	} else if (phydev->phy_id == PHY_ID_RTL8224) { //8224 
		if (rtk_port_num % 4) {
			rtl_fill_sdkDesc(
					NULL,
					HWP_GE,
					HWP_COPPER,
					0,   
					RTK_PHYTYPE_NONE,
					4 - (rtk_port_num % 4));
		}
		printk(KERN_ERR"======>IS 8224 port %d phy_id %d\n", rtk_port_num, rtk_phy_num);
		rtl_fill_sdkDesc(
				mdio_bus,
				HWP_2_5GE,
				HWP_COPPER,
				addr - port,   
				RTK_PHYTYPE_RTL8224,
				4);
	} 
	else {
		goto NO_FOUND_CONFIG;
	}

	rtlsdk_phy_need_init();
	return 0;

NO_FOUND_CONFIG:
	printk(KERN_ERR"RTK SDK:NO FOUND PHY CONFIG!!\n");
	return -1;
}

void rtksdk_phy_init(void)
{
	rtk_phy_initInfo_t initInfo;
	if (s_sdk_initd) {
		printk(KERN_ERR "RTLSDK: sdk has already initd\n");
		return;
	}

	if (!rtk_phy_num) {
		printk(KERN_ERR "RTLSDK: No Found rtl phy\n");
		return;
	}

	initInfo.port_desc = rtl_port_descp;
	initInfo.phy_desc  = rtl_phy_Descp;

	s_sdk_initd = 1;

	rtk_init(&initInfo);

	printk(KERN_ERR "RTLSDK: INIT OK");
}

EXPORT_SYMBOL_GPL(rtksdk_phy_init);

unsigned int mii_mgr_read(unsigned int port,unsigned int phy_register,unsigned int *read_data)
{
	struct mii_bus *bus = rtk_port_map[port].bus;
	uint32_t phy_addr = rtk_port_map[port].addr;

	if (!bus) {
		printk(KERN_ERR "%s: bus is null\n", __FUNCTION__);
		return 0;
	}
	mutex_lock_nested(&bus->mdio_lock, MDIO_MUTEX_NESTED);

	*read_data = bus->read(bus, phy_addr, phy_register);

	mutex_unlock(&bus->mdio_lock);

   //printk(KERN_ERR "%s port:%d reg:%#x data:%#x", __FUNCTION__, port, phy_register, *read_data);

	return 0;
}

unsigned int mii_mgr_write(unsigned int port, unsigned int phy_register,unsigned int write_data)
{
	struct mii_bus *bus = rtk_port_map[port].bus;
	uint32_t phy_addr = rtk_port_map[port].addr;
	if (!bus) { 
		printk(KERN_ERR "%s: bus is null\n", __FUNCTION__);
		return 0;
	}

    //printk(KERN_ERR "%s port:%d reg:%#x data:%#x",__FUNCTION__, port, phy_register, write_data);
	mutex_lock_nested(&bus->mdio_lock, MDIO_MUTEX_NESTED);

	bus->write(bus, phy_addr, phy_register, write_data);

	mutex_unlock(&bus->mdio_lock);
	
	return 0;
}

int32 phy_hal_mii_read(
    uint32      unit,
    rtk_port_t  port,
    uint32      phy_reg,
    uint32      *pData)
{
	mii_mgr_read(port,
		phy_reg, pData);
    return RT_ERR_OK;
} /* end of phy_hal_mii_read */

/* Function Name:
 *      phy_hal_mii_write
 * Description:
 *      Set PHY registers by Clause 22.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      phy_reg - PHY register
 *      data    - write data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      1. phy_reg valid range is 0 ~ 31
 */
int32
phy_hal_mii_write(
    uint32      unit,
    rtk_port_t  port,
    uint32      phy_reg,
    uint32      data)
{

	mii_mgr_write(port,
			phy_reg, data);
    return RT_ERR_OK;
} /* end of phy_hal_mii_write */

/* Function Name:
 *      phy_hal_mmd_read
 * Description:
 *      Get PHY registers by Clause 45.
 *      If the MDC/MDIO controller is Clause 22, the API shall implement Clause 22's register 13 and 14 to access MMD register of the PHY.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      mmdAddr - mmd device address
 *      mmdReg  - mmd reg id
 * Output:
 *      pData   - pointer buffer of read data
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 */
int32
phy_hal_mmd_read(
    uint32      unit,
    rtk_port_t  port,
    uint32      mmdAddr,
    uint32      mmdReg,
    uint32      *pData)
{
	mii_mgr_read(port,
			mdiobus_c45_addr(mmdAddr, mmdReg),
			pData);

    return RT_ERR_OK;
} /* end of phy_hal_mmd_read */


/* Function Name:
 *      phy_hal_mmd_write
 * Description:
 *      Set PHY registers by Clause 45.
 *      If the MDC/MDIO controller is Clause 22, the API shall implement Clause 22's register 13 and 14 to access MMD register of the PHY.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      mmdAddr - mmd device address
 *      mmdReg  - mmd reg id
 *      data    - write data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 */
int32
phy_hal_mmd_write(
    uint32      unit,
    rtk_port_t  port,
    uint32      mmdAddr,
    uint32      mmdReg,
    uint32      data)
{
	mii_mgr_write(port,
			mdiobus_c45_addr(mmdAddr, mmdReg),
			data);

    return RT_ERR_OK;
} 

static int rtl8261be_config_init(struct phy_device *phydev)
{
	phydev->irq = PHY_POLL;
	//phy_write(phydev, 0x1e, 0);

	return 0;
}

static int rtl8214fc_config_init(struct phy_device *phydev)
{
	phydev->irq = PHY_POLL;
	phy_write(phydev, 0x1e, 0);

	return 0;
}

int rtk_read_status(struct phy_device *phydev)  
{
	int err, old_link = phydev->link;
	rtk_port_t port = rtk_get_portid(phydev);
	rtk_port_linkStatus_t port_status = {0};
	rtk_port_speed_t  speed;
	rtk_port_duplex_t duplex;

	/* Update the link, but return if there was an error */
	err = genphy_update_link(phydev);
	if (err)
		return err;

	/* why bother the PHY if nothing can have changed */
	if (phydev->autoneg == AUTONEG_ENABLE && old_link && phydev->link)
		return 0;

	phydev->speed = SPEED_UNKNOWN;
	phydev->duplex = DUPLEX_UNKNOWN;
	phydev->pause = 0;
	phydev->asym_pause = 0;

	err = genphy_read_lpa(phydev);
	if (err < 0)
		return err;

	if (port == -1) {
		printk(KERN_ERR "RTLSDK: Cannot get portid");
		return -1;
	}

	err = rtk_port_phyLinkStatus_get(0, port, &port_status);
	if (RT_ERR_OK != err) {
		printk(KERN_ERR "RTLSDK: rtk_port_linkMedia_get failed, err=%d\n", err);
		return err;
	}

	phydev->link = port_status == PORT_LINKUP ? 1 : 0;

	if (!phydev->link) 
		return 0;


	rtk_port_speedDuplex_get(
			0,
			port,
			&speed,
			&duplex);

	switch(speed)
	{
	case PORT_SPEED_10M:
		phydev->speed = SPEED_10; 
		break;
	case PORT_SPEED_100M:
		phydev->speed = SPEED_100; 
		break;
	case PORT_SPEED_1000M:
		phydev->speed = SPEED_1000; 
		break;
	case PORT_SPEED_2_5G:
	case PORT_SPEED_2_5G_LITE:
		phydev->speed = SPEED_2500; 
		break;
	case PORT_SPEED_5G:
	case PORT_SPEED_5G_LITE:
		phydev->speed = SPEED_5000; 
		break;
	case PORT_SPEED_10G:
	case PORT_SPEED_10G_LITE:
		phydev->speed = SPEED_10000; 
		break;
	default:
		speed = -1;
	}

	phydev->duplex = duplex == PORT_HALF_DUPLEX ? DUPLEX_HALF : DUPLEX_FULL;
#if 0
	if (phydev->autoneg == AUTONEG_ENABLE && phydev->autoneg_complete) {
		phy_resolve_aneg_linkmode(phydev);
	} else if (phydev->autoneg == AUTONEG_DISABLE) {
		err = genphy_read_status_fixed(phydev);
		if (err < 0)
			return err;
	}
#endif
	return 0;
}


static int rtl8221b_advertise_aneg(rtk_port_t port, struct phy_device *phydev) 
{
	rtk_port_phy_ability_t ability = {0};
    int err = phy_autoNegoAbility_get(0, port, &ability);
	if (err) {
		printk(KERN_ERR "RTLSDK: port=%d phy_autoNegoAbility_get Failed, err=%d\n", port, err);	
		return 1;
	}
	
	ability.Half_10 = 1;
    ability.Full_10 = 1;
	ability.Half_100 = 1;
	ability.Full_100 = 1;
	ability.Half_1000 = 1;
	ability.Full_1000 = 1;
	ability.adv_2_5G = 1;

	err = phy_autoNegoAbility_set(0, port, &ability);
	if (err) {
		printk(KERN_ERR"RTLSDK: port=%d, phy_8226_autoNegoAbility_set Failed, err=%d\n", port, err);	
	}

	return 0;
}

static int rtl8221b_config_init(struct phy_device *phydev)
{
	phydev->irq = PHY_POLL;
	//phy_write(phydev, 0x1e, 0);

	return 0;
}

static int rtl8221b_config_aneg(struct phy_device *phydev)
{
	rtk_port_t port = rtk_get_portid(phydev);
	int ret = 0;
 
	//pr_debug("In %s\n", __func__);
	
	if (phydev->autoneg == AUTONEG_ENABLE) {
		ret = rtl8221b_advertise_aneg(port, phydev);
		if (ret)
			goto out;

		if (phy_autoNegoEnable_set(0, port, ENABLED)) {
			printk(KERN_ERR"RTLSDK: phy_autoNegoEnable_set Failed\n");
			goto out;
		}
	}

out:
	return ret;
}

static int rtl8221b_read_status(struct phy_device *phydev)
{
	int err;
	rtk_port_t port = rtk_get_portid(phydev);
	rtk_port_speed_t  speed;
	rtk_port_duplex_t duplex;

	if (phy_speed_get(0, port, &speed)) {
		phydev->link = 0;
		return 0;
	}

	phydev->link = 1;

	err = phy_duplex_get(0, port, &duplex);
	if(err) {
		printk(KERN_ERR "RTLSDK: phy_duplex_get[%d] failed, err=%d\n", port, err);
		return err;
	}

	switch(speed)
	{
	case PORT_SPEED_10M:
		phydev->speed = SPEED_10; 
		break;
	case PORT_SPEED_100M:
		phydev->speed = SPEED_100; 
		break;
	case PORT_SPEED_1000M:
		phydev->speed = SPEED_1000; 
		break;
	case PORT_SPEED_2_5G:
		phydev->speed = SPEED_2500; 
		break;
	case PORT_SPEED_5G:
		phydev->speed = 5000; 
		break;
	case PORT_SPEED_10G:
		phydev->speed = SPEED_10000; 
		break;
	case PORT_SPEED_2_5G_LITE:
	case PORT_SPEED_5G_LITE:
	case PORT_SPEED_10G_LITE:
	default:
		speed = -1;
	}

	phydev->duplex = duplex == PORT_HALF_DUPLEX ? DUPLEX_HALF : DUPLEX_FULL;

	return 0;
}
static int rtl8221b_get_features(struct phy_device *phydev)
{
	linkmode_mod_bit(ETHTOOL_LINK_MODE_2500baseT_Full_BIT, phydev->supported, 1);
	return genphy_read_abilities(phydev);
}

static int rtl8261be_get_features(struct phy_device *phydev)
{
	linkmode_mod_bit(ETHTOOL_LINK_MODE_10000baseT_Full_BIT, phydev->supported, 1);
	linkmode_mod_bit(ETHTOOL_LINK_MODE_5000baseT_Full_BIT, phydev->supported, 1);
	linkmode_mod_bit(ETHTOOL_LINK_MODE_2500baseT_Full_BIT, phydev->supported, 1);
	return genphy_read_abilities(phydev);
}

static int rtl8214fc_get_features(struct phy_device *phydev)
{
	/* Don't call genphy_read_abilities() to workaround the issue that register 15
	 * keeps changing for a while after phy is up
	 */
	genphy_read_abilities(phydev);
	linkmode_mod_bit(ETHTOOL_LINK_MODE_1000baseT_Full_BIT, phydev->supported, 1);
	linkmode_mod_bit(ETHTOOL_LINK_MODE_100baseT_Full_BIT, phydev->supported, 1);
	linkmode_mod_bit(ETHTOOL_LINK_MODE_10baseT_Full_BIT, phydev->supported, 1);
	return 0;
}

static int rtl8224_get_features(struct phy_device *phydev)
{
	linkmode_mod_bit(ETHTOOL_LINK_MODE_2500baseT_Full_BIT, phydev->supported, 1);
	return genphy_read_abilities(phydev);
}

static struct phy_driver realtek_drvs[] = {
	{
	PHY_ID_MATCH_MODEL(PHY_ID_RTL8214FC),
		.name		= "Realtek RTL8214FC",
		.get_features	= rtl8214fc_get_features,
		.flags 		= PHY_POLL,
		.read_status   = rtk_read_status,
		.config_init   = rtl8214fc_config_init,
		.config_aneg    = genphy_config_aneg,
		.probe		= set_phyinfo2rtk,
		.resume		= genphy_resume,
		.suspend	= genphy_suspend,
		.set_loopback	= genphy_loopback,
	},
	{
	PHY_ID_MATCH_MODEL(PHY_ID_RTL8261BE),
		.name		= "Realtek RTL8261BE",
		.get_features	= rtl8261be_get_features,
		.flags 		= PHY_POLL,
		.read_status   = rtk_read_status,
		.config_init   = rtl8261be_config_init,
		.config_aneg    = genphy_config_aneg,
		.probe		= set_phyinfo2rtk,
		.resume		= genphy_resume,
		.suspend	= genphy_suspend,
		.set_loopback	= genphy_loopback,
	},
	{
	PHY_ID_MATCH_MODEL(PHY_ID_RTL8221B),
	.name           = "REALTEK RTL8221B",
	.get_features       = rtl8221b_get_features,
	.flags 		= PHY_POLL,
	.probe =  set_phyinfo2rtk,
	.config_init    = rtl8221b_config_init,
	.read_status    = rtl8221b_read_status,
	.config_aneg    = rtl8221b_config_aneg,
	.resume		= genphy_resume,
	.suspend	= genphy_suspend,
	.set_loopback	= genphy_loopback,
	},
	{
		PHY_ID_MATCH_MODEL(PHY_ID_RTL8224),
		.name		= "Realtek RTL8224",
		.get_features	= rtl8224_get_features,
		.flags 		= PHY_POLL,
		.read_status   = rtk_read_status,
		.config_init   = rtl8261be_config_init,
		.config_aneg    = genphy_config_aneg,
		.probe		= set_phyinfo2rtk,
		.resume		= genphy_resume,
		.suspend	= genphy_suspend,
		.set_loopback	= genphy_loopback,
	},
};

void rtk_phy_register(void) 
{
	if(s_phy_reged) {
		printk(KERN_ERR"rtl phy has already registered!!");
		return;
	}

	if(phy_drivers_register(realtek_drvs,
			ARRAY_SIZE(realtek_drvs),
			THIS_MODULE)) {
		printk(KERN_ERR"rtl phy registered Failed!!");
		return;
	}

	s_phy_reged = true;

}

void rtk_phy_unregister(void)
{
	if (s_phy_reged)
		phy_drivers_unregister(realtek_drvs,
				ARRAY_SIZE(realtek_drvs));
}
