#include <common.h>
#include <command.h>
#include <phy.h>
#include <linux/compat.h>
#include <malloc.h>
#include <hal/phy/phydef.h>
#include <hal/common/miim.h>
#include <phy/inc/phy_init.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/debug/rt_log.h>
#include <common/util/rt_util.h>
#include <osal/lib.h>
#include <osal/print.h>
// PHY MMD devices
#define MMD_AN          7
#define MMD_VEND2       31
#define RTK_MAX_PORT_NUM   12
static struct {
	int addr;
	struct mii_dev *bus;
} rtk_port_map[RTK_MAX_PORT_NUM];

extern int g_debug_cls;
static int  rtk_port_num  = 0;
static int  rtk_phy_num  = 0;
static int  s_sdk_initd  = 0;
static phy_hwp_portDescp_t  rtl_port_descp[RTK_MAX_PORT_NUM + 1];
static phy_hwp_phyDescp_t   rtl_phy_Descp[RTK_MAX_PORT_NUM + 1];

#ifdef CLS_DEBUG_RTL
static phy_hwp_portDescp_t  rtl_port_descp1[] = {
	{ .mac_id =  0,  .phy_idx = 0, .eth = HWP_GE,   .medi = HWP_COPPER,},
	{ .mac_id =  1,  .phy_idx = 0, .eth = HWP_GE,   .medi = HWP_COPPER,},
	{ .mac_id =  2,  .phy_idx = 0, .eth = HWP_GE,   .medi = HWP_COPPER,},
	{ .mac_id =  3,  .phy_idx = 0, .eth = HWP_GE,   .medi = HWP_COPPER,},
	{ .mac_id =  4,  .phy_idx = 0, .eth = HWP_GE,   .medi = HWP_COPPER,},
	{ .mac_id = HWP_END },
};

static phy_hwp_phyDescp_t     rtl_phy_Descp1[] = {
	[0] = { .chip = RTK_PHYTYPE_RTL8214FC, .mac_id = 0, .phy_max = 4 },
	[1] = { .chip = RTK_PHYTYPE_RTL8226B, .mac_id = 4, .phy_max = 1 },
	[2] = { .chip = HWP_END },
};
#endif

static int rtk_get_portid(struct phy_device *phydev)
{
	struct mii_bus *bus =  phydev->bus; 
	int	addr = phydev->addr;
	int mac_id = 0; 
	for ( ; mac_id < rtk_port_num; mac_id ++) {
		if((bus == rtk_port_map[mac_id].bus) &&
				(addr == rtk_port_map[mac_id].addr))
			return mac_id;
	}
	printf("%s addr%d not FOUND in port map\n", __FUNCTION__, addr);
	return -1;
}
int check_phyDescp_exist(int addr, struct mii_dev *bus) 
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
		struct mii_dev *mdio_bus,
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
		printf("=======>rtl Please Check mac_id config!!!\n");
		return;
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

void rtl_set_phyaddr_to_sdk(struct phy_device *phydev, int port) 
{
	int addr;
	uint8   eth;           
	uint8   medi;           
	uint8   phy_addr;      
	uint32  chip;         
	uint8   phy_max; 
	struct mii_dev *mdio_bus;

	if (NULL == phydev) {
		printf("=======>RTLSDK: phydev is null!!!\n");
		return;
	}

	addr = phydev->addr;
	mdio_bus = phydev->bus;

	if(check_phyDescp_exist(addr, mdio_bus)) {
		printf("=======>RTLSDK: add[%d] config has exist!!!\n", addr);
		return;
	}

	if (phydev->phy_id == 0x1cc981) {
		printf("======>IS 8214fc port %d phy_id %d\n", rtk_port_num, rtk_phy_num);
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
		printf("======>IS 8261BE\n");
		rtl_fill_sdkDesc(
				mdio_bus,
				HWP_XGE,
				HWP_COPPER,
				addr,   
				RTK_PHYTYPE_RTL8261BE,
				1);
	} else if (phydev->phy_id == 0x001cc849) { //8221B 
		printf("======>IS 8221B port %d phy_id %d\n", rtk_port_num, rtk_phy_num);
		rtl_fill_sdkDesc(
				mdio_bus,
				HWP_2_5GE,
				HWP_COPPER,
				addr,   
				RTK_PHYTYPE_RTL8226B,
				1);
	}  else if (phydev->phy_id == 0x001ccad0) { //8224 
		if (rtk_port_num % 4) {
			rtl_fill_sdkDesc(
					NULL,
					HWP_GE,
					HWP_COPPER,
					0,   
					RTK_PHYTYPE_NONE,
					4 - (rtk_port_num % 4));
		}
		printf("======>IS 8224 port %d phy_id %d\n", rtk_port_num, rtk_phy_num);
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

	return;
NO_FOUND_CONFIG:
	printf("RTK SDK:NO FOUND PHY CONFIG!!\n");
}

/* Function Name:
 *      phy_hal_mii_read
 * Description:
 *      Get PHY registers data by Clause 22.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      phy_reg - PHY register
 * Output:
 *      pData   - pointer buffer of read data
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      1. phy_reg valid range is 0 ~ 31
 */
int32
phy_hal_mii_read(
    uint32      unit,
    rtk_port_t  port,
    uint32      phy_reg,
    uint32      *pData)
{
	unsigned int phy_addr; 
	int devad = MDIO_DEVAD_NONE;
	struct mii_dev *bus;
	if (port > rtk_port_num) { 
		printf("ERR! %s port %d\n",__FUNCTION__, port);
		return RT_ERR_OK; 
	}
    bus = rtk_port_map[port].bus;
	phy_addr = rtk_port_map[port].addr;
	if (!bus) {
		//printf("Can't find mdio dev!!\n");
		return 0;
	}
	/* Save the chosen bus */
	//miiphy_set_current_dev(bus->name);


	if (pData) {
		*pData = bus->read(bus, phy_addr, devad, phy_reg);
	}
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

	unsigned int phy_addr; 
	struct mii_dev *bus;
	int devad = MDIO_DEVAD_NONE;
	if (port > rtk_port_num) { 
		printf("ERR! %s port %d\n",__FUNCTION__, port);
		return RT_ERR_OK; 
	}

    bus = rtk_port_map[port].bus;
	phy_addr = rtk_port_map[port].addr;

	if (!bus) {
		//printf("Can't find mdio dev!!\n");
		return 0;
	}

	/* Save the chosen bus */
	//miiphy_set_current_dev(bus->name);

	bus->write(bus, phy_addr, devad,
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

	unsigned int phy_addr; 
	int devad = mmdAddr;
	struct mii_dev *bus;
	if (port > rtk_port_num) { 
		printf("ERR! %s port %d\n",__FUNCTION__, port);
		return RT_ERR_OK; 
	}

    bus = rtk_port_map[port].bus;
	phy_addr = rtk_port_map[port].addr;
	if (!bus) {
		//printf("Can't find mdio dev!!\n");
		return 0;
	}

	if (pData) {
		*pData = bus->read(bus, phy_addr, devad, mmdReg);
	}
   if (g_debug_cls) 
	printf("%s entry unit=%d, port=%d, mmdAddr=%#x mmdReg=%#x Data=%#x\n",
			__FUNCTION__,
			unit,
			port,
			mmdAddr,
			mmdReg,
			pData ? *pData : 0);

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

	unsigned int phy_addr; 
	struct mii_dev *bus;
	int devad = mmdAddr;
	if (port > rtk_port_num) { 
		printf("ERR! %s port %d\n",__FUNCTION__, port);
		return RT_ERR_OK; 
	}

	bus = rtk_port_map[port].bus;
	phy_addr = rtk_port_map[port].addr;

	if (!bus) {
		//printf("Can't find mdio dev!!\n");
		return 0;
	}

	/* Save the chosen bus */
	//miiphy_set_current_dev(bus->name);

	bus->write(bus, phy_addr, devad,
			mmdReg, data);

   if (g_debug_cls) 
	printf("%s entry unit=%d, port=%d, mmdAddr=%#x mmdReg=%#x data=%#x\n",
			__FUNCTION__,
			unit,
			port,
			mmdAddr,
			mmdReg,
			data);

    return RT_ERR_OK;
} 


static int rtl_probe(struct phy_device *phydev)
{
	return 0;
}

static int rtl_config(struct phy_device *phydev)
{
#if 0
	rtk_phy_initInfo_t initInfo;
	initInfo.port_desc = rtl8214_port_descp;
	initInfo.phy_desc = rtl8214_phy_Descp;
	rtk_init(&initInfo);
#endif
	return 0;
}

#ifdef CLS_DEBUG_RTL
int rtlsdk_phyconfig(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	rtk_phy_initInfo_t initInfo;
	if(argc > 1) {
		printf("====>is rtl8214 only\n");
		initInfo.port_desc = rtl_port_descp1;
		initInfo.phy_desc = rtl_phy_Descp1;
	}else {
		printf("====>config rtl8214 and rtl8261B\n");
		initInfo.port_desc = rtl_port_descp;
		initInfo.phy_desc = rtl_phy_Descp;
	}

	rtk_init(&initInfo);
}
#endif
extern int rtk_disable;
void rtlsdk_phy_init(void)
{
	int i;
	rtk_phy_initInfo_t initInfo;
	if (s_sdk_initd) {
		printf("RTLSDK: sdk has already initd\n");
		return;
	}

	if (!rtk_phy_num) {
		printf("RTLSDK: No Found rtl phy\n");
		return;
	}

	initInfo.port_desc = rtl_port_descp;
	initInfo.phy_desc  = rtl_phy_Descp;

	s_sdk_initd = 1;

    printf("==>Port Desc \n");
	for (i = 0; initInfo.port_desc[i].mac_id != HWP_END; i++) {
		printf("index = %d, mac_id = %d, phy_idx = %d, eth = %d, medi = %d, phy_addr = %d, rel_phy_addr = %d, %#x\n",
				i,
				initInfo.port_desc[i].mac_id,
				initInfo.port_desc[i].phy_idx,
				initInfo.port_desc[i].eth,
				initInfo.port_desc[i].medi,		
				initInfo.port_desc[i].phy_addr,		
				rtk_port_map[i].addr,
				rtk_port_map[i].bus
			  );
	}

	printf("====>Phy Desc\n");
	for (i = 0; initInfo.phy_desc[i].chip != HWP_END; i++) {
		printf("index = %d, mac_id = %d, chip = %d, phy_max = %d\n",
				i,
				initInfo.phy_desc[i].mac_id,
				initInfo.phy_desc[i].chip,
				initInfo.phy_desc[i].phy_max
			  );
	}

   if (!rtk_disable)
	rtk_init(&initInfo);
}

void rtlsdk_phy_need_init(void)
{
	if (s_sdk_initd)
		s_sdk_initd = 0;
}

int rtl_startup(struct phy_device *phydev)  
{
	int err, old_link = phydev->link;
	rtk_port_t port = rtk_get_portid(phydev);
	rtk_port_linkStatus_t port_status = {0};
	rtk_port_speed_t  speed;
	rtk_port_duplex_t duplex;
	rtk_port_media_t  port_media = {0};

	/* Update the link, but return if there was an error */
	err = genphy_update_link(phydev);
	if (err) {
		printf("%s Update link err phy_add %d\n", phydev->addr);
		return err;
	}

    //printf("===>phydev link %d\n", phydev->link);
#if 0
	/* why bother the PHY if nothing can have changed */
	if (phydev->autoneg == AUTONEG_ENABLE && old_link && phydev->link)
		return 0;
#endif
	phydev->speed = SPEED_1000;
	phydev->duplex = 0;
	phydev->pause = 0;
	phydev->asym_pause = 0;

	if (port == -1) {
		printk(KERN_ERR "RTK: Cannot get portid \n");
		return -1;
	}

	err = rtk_port_phyLinkStatus_get(0, port, &port_status);
	if (RT_ERR_OK != err) {
		printk(KERN_ERR "RTK: rtk_port_linkMedia_get port[%d] failed, err=%d\n", port, err);
		return err;
	}

	phydev->link = port_status == PORT_LINKUP ? 1 : 0;

    //printf("===2>phydev link %d\n", phydev->link);

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
#if 0
static int rtl_startup(struct phy_device *phydev)
{
	int ret;

	ret = genphy_update_link(phydev);
	if (ret)
		return ret;

	return genphy_parse_link(phydev);
}
#endif

static int rtl_read_mmd(struct phy_device *phydev, int devnum, int regnum)
{
	u32 val;
	val = phy_read(phydev, devnum, regnum);
	return val;
}

static int rtl8221b_read_mmd(struct phy_device *phydev, int devnum, u16 regnum, u32 * val)
{
	if (val) {
		*val = phy_read(phydev, devnum, regnum);
	}
	return 0;
}

static int rtl_write_mmd(struct phy_device *phydev, int devnum, int regnum, u16 val)
{
	return phy_write(phydev, devnum, regnum, val);
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

	if(err = phy_duplex_get(0, port, &duplex)) {
		printk(KERN_ERR "RTK: phy_duplex_get[%d] failed, err=%d\n", port, err);
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
#if 0
static int rtl8221b_read_status(struct phy_device *phydev)
{
	int ret = 0, i;
	u32 val;

	// Link status must be read twice
	for (i = 0; i < 2; i++) {
		rtl8221b_read_mmd(phydev, MMD_VEND2, 0xA402, &val);
	}
	phydev->link = val & BIT(2) ? 1 : 0;
	if (!phydev->link)
		goto out;

	// Read duplex status
	ret = rtl8221b_read_mmd(phydev, MMD_VEND2, 0xA434, &val);
	if (ret)
		goto out;
	phydev->duplex = !!(val & BIT(3));

	// Read speed
	ret = rtl8221b_read_mmd(phydev, MMD_VEND2, 0xA434, &val);
	switch (val & 0x0630) {
	case 0x0000:
		phydev->speed = SPEED_10;
		break;
	case 0x0010:
		phydev->speed = SPEED_100;
		break;
	case 0x0020:
		phydev->speed = SPEED_1000;
		break;
	case 0x0200:
		phydev->speed = SPEED_10000;
		break;
	case 0x0210:
		phydev->speed = SPEED_2500;
		break;
	case 0x0220:
		phydev->speed = 5000;
		break;
	default:
		break;
	}
out:
	return ret;
}

static int rtl8221b_advertise_aneg(struct phy_device *phydev)
{

	int ret = 0;
	u32 v;

	pr_info("In %s\n", __func__);

	ret = rtl8221b_read_mmd(phydev, MMD_AN, 16, &v);
	if (ret)
		goto out;

	v |= BIT(5); // HD 10M
	v |= BIT(6); // FD 10M
	v |= BIT(7); // HD 100M
	v |= BIT(8); // FD 100M

	ret = rtl_write_mmd(phydev, MMD_AN, 16, v);

	// Allow 1GBit
	ret = rtl8221b_read_mmd(phydev, MMD_VEND2, 0xA412, &v);
	if (ret)
		goto out;
	v |= BIT(9); // FD 1000M

	ret = rtl_write_mmd(phydev, MMD_VEND2, 0xA412, v);
	if (ret)
		goto out;

	// Allow 2.5G
	ret = rtl8221b_read_mmd(phydev, MMD_AN, 32, &v);
	if (ret)
		goto out;

	v |= BIT(7);
	ret = rtl_write_mmd(phydev, MMD_AN, 32, v);

out:
	return ret;
}

static int rtl8221b_config_aneg(struct phy_device *phydev)
{
	int ret = 0;
	u32 v;

	pr_debug("In %s\n", __func__);
	if (phydev->autoneg == AUTONEG_ENABLE) {
		ret = rtl8221b_advertise_aneg(phydev);
		if (ret)
			goto out;
		// AutoNegotiationEnable
		ret = rtl8221b_read_mmd(phydev, MMD_AN, 0, &v);
		if (ret)
			goto out;

		v |= BIT(12); // Enable AN
		ret = rtl_write_mmd(phydev, MMD_AN, 0, v);
		if (ret)
			goto out;

		// RestartAutoNegotiation
		ret = rtl8221b_read_mmd(phydev, MMD_VEND2, 0xA400, &v);
		if (ret)
			goto out;
		v |= BIT(9);

		ret = rtl_write_mmd(phydev, MMD_VEND2, 0xA400, v);
	}

//	TODO: ret = __genphy_config_aneg(phydev, ret);

out:
	return ret;
}
#endif

static int rtl8221b_advertise_aneg(rtk_port_t port, struct phy_device *phydev) 
{
	rtk_port_phy_ability_t ability = {0};

	if (phy_8226_autoNegoAbility_get(0, port, &ability)) {
		printf("phy_8226_autoNegoAbility_get Failed\n");	
		return 0;
	}

	ability.Half_10 = 1;
	ability.Full_10 = 1;
	ability.Half_100 = 1;
	ability.Full_100 = 1;
	ability.Half_1000 = 1;
	ability.Full_1000 = 1;
	ability.adv_2_5G = 1;

	if (phy_autoNegoAbility_set(0, port, &ability)) {
		printf("phy_8226_autoNegoAbility_set Failed\n");	
	}

	return 0;
}

static int rtl8221b_config_aneg(struct phy_device *phydev)
{
	rtk_port_t port = rtk_get_portid(phydev);
	int ret = 0;
	u32 v;

	pr_debug("In %s\n", __func__);
	if (phydev->autoneg == AUTONEG_ENABLE) {
		ret = rtl8221b_advertise_aneg(port, phydev);
		if (ret)
			goto out;

		if (phy_autoNegoEnable_set(0, port, ENABLED)) {
			printf("phy_autoNegoEnable_set Failed\n");
			goto out;
		}
	}

out:
	return ret;
}

static struct phy_driver RTL8214_driver = {
	.name = "RealTek RTL8214 4 port 1Gbps Ethernet",
	.uid = 0x1cc981,
	.mask = 0xffffff,
	.features = PHY_GBIT_FEATURES,
	.probe = &rtl_probe,
	.config = &rtl_config,
	.startup = &rtl_startup,
	.shutdown = &genphy_shutdown,
	.read_mmd = &rtl_read_mmd,
	.write_mmd = &rtl_write_mmd
};

static struct phy_driver RTL8261B_driver = {
	.name = "RealTek RTL8261B 10Gbps Ethernet",
	.uid = 0x1ccaf3,
	.mask = 0xffffff,
	.features = PHY_10G_FEATURES,
	.probe = &rtl_probe,
	.config = &rtl_config,
	.startup = &rtl_startup,
	.shutdown = &genphy_shutdown,
	.read_mmd = &rtl_read_mmd,
	.write_mmd = &rtl_write_mmd
};
#if 1
static struct phy_driver RTL8221B_driver = {
	.name           = "REALTEK RTL8221B",
	.uid = 0x001cc849,
	.mask = 0xffffff,
	.features       = PHY_GBIT_FEATURES,
	.probe          = &rtl_probe,
	.probe = &rtl_probe,
	.config    = &rtl8221b_config_aneg,
	.startup    = &rtl8221b_read_status,
	.shutdown = &genphy_shutdown,
	.read_mmd = &rtl_read_mmd,
	.write_mmd = &rtl_write_mmd
};
#else
static struct phy_driver RTL8221B_driver = {
	.name           = "REALTEK RTL8221B",
	.uid = 0x001cc849,
	.mask = 0xffffff,
	.features       = PHY_GBIT_FEATURES,
	.probe          = &rtl_probe,
	.startup    = &rtl8221b_read_status,
	.config    = &rtl8221b_config_aneg,
	.shutdown = &genphy_shutdown,
	.read_mmd = &rtl_read_mmd,
	.write_mmd = &rtl_write_mmd
};
#endif

static struct phy_driver RTL8224_driver = {
	.name           = "REALTEK RTL8224",
	.uid = 0x001ccad0,
	.mask = 0xffffff,
	.features       = PHY_GBIT_FEATURES,
	.probe          = &rtl_probe,
	.probe = &rtl_probe,
	.config    = &rtl_config,
	.startup    = &rtl_startup,
	.shutdown = &genphy_shutdown,
	.read_mmd = &rtl_read_mmd,
	.write_mmd = &rtl_write_mmd
};

int rtlsdk_phy_register(void)
{
	phy_register(&RTL8214_driver);
	phy_register(&RTL8224_driver);
	phy_register(&RTL8261B_driver);
	phy_register(&RTL8221B_driver);
	return 0;
}

#ifdef CLS_DEBUG_RTL
U_BOOT_CMD(
		rtlsdk_config,	3,	1,	rtlsdk_phyconfig,
		"clourney rtl utility commands",
		""
		);
#endif
