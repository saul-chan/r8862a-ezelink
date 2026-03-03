#include <stdio.h>
#include <stdint.h>
#include "dubhe1000_xpcs_serdes.h"
#include "sys_hal.h"
#include <hal/phy/phydef.h>
#include <hal/common/miim.h>
#include <phy/inc/phy_init.h>
#include <osal/lib.h>
#include <osal/print.h>
#include <hal/phy/phydef.h>
#include <hal/common/miim.h>
#include <phy/inc/phy_init.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/debug/rt_log.h>
#include <common/util/rt_util.h>
#include <osal/lib.h>
#include <osal/print.h>
#define uint32 uint32_t
#define int32 int32_t
typedef uint32  rtk_port_t; 

int g_debug = 0;
#define MDIO_PRTAD_NONE			(-1)
#define MDIO_DEVAD_NONE			(-1)

#define MDIO_READ(phyaddr, devad, phyreg)  				xgmac_mdio_read(XGMAC_BASE_ADDR(mdio_index), 1, 1, phyaddr, devad, phyreg)
#define MDIO_WRITE(phyaddr, devad, phyreg, data)		xgmac_mdio_write(XGMAC_BASE_ADDR(mdio_index), 1, 1, phyaddr, devad, phyreg, data) 
/*
 * This table is used to describe your hardware board design, especially for mapping relation between port and phy.
 * Port related information
 * .mac_id      = port id.
 * .phy_idex    = used to indicate which PHY entry is used by this port in glued_phy_Descp[].
 * .eth         = Ethernet speed type (refer to rt_port_ethType_t).
 * .medi        = Port media type (refer to rt_port_medium_t).
 */
/*
 * PHY related information
 * .chip        = PHY Chip model (refer to phy_type_t).
 * .mac_id      = The first port id of this PHY. For example, the 8218D is connected to
 *                port 0 ~ 7, then the .mac_id  = 0.
 * .phy_max     = The MAX port number of this PHY. For examplem the 8218D is an octet PHY,
 *                so this number is 8.
 */
phy_hwp_portDescp_t  rtl8261be_port_descp[] = {
	{ .mac_id =  0,  .phy_idx = 0, .eth = HWP_XGE,   .medi = HWP_COPPER,},
	{ .mac_id = HWP_END },
};

phy_hwp_phyDescp_t     rtl8261be_phy_descp[] = {
	[0] = { .chip = RTK_PHYTYPE_RTL8261BE, .mac_id = 0, .phy_max = 1 },
	[1] = { .chip = HWP_END },
};

phy_hwp_portDescp_t  rtl8214fc_port_descp[] = {
	{ .mac_id =  0,  .phy_idx = 0, .eth = HWP_GE,   .medi = HWP_COPPER,},
	{ .mac_id =  1,  .phy_idx = 0, .eth = HWP_GE,   .medi = HWP_COPPER,},
	{ .mac_id =  2,  .phy_idx = 0, .eth = HWP_GE,   .medi = HWP_COPPER,},
	{ .mac_id =  3,  .phy_idx = 0, .eth = HWP_GE,   .medi = HWP_COPPER,},
	{ .mac_id = HWP_END },
};

phy_hwp_phyDescp_t     rtl8214fc_phy_descp[] = {
	[0] = { .chip = RTK_PHYTYPE_RTL8214FC, .mac_id = 0, .phy_max = 4 },
	[1] = { .chip = HWP_END },
};

int rtl8214fc_port_map[] = {0x8, 0x9, 0xa, 0xb};
int rtl8261be_port_map[] = {0x0};

phy_hwp_portDescp_t  *my_port_descp = rtl8214fc_port_descp;
phy_hwp_phyDescp_t  * my_phy_descp  = rtl8214fc_phy_descp;
int max_port_num = 3; 
int mdio_index = 0;
int * port_map = rtl8214fc_port_map;

int init_phy_api(void)
{
	rtk_phy_initInfo_t initInfo;

	initInfo.port_desc = my_port_descp;
	initInfo.phy_desc = my_phy_descp;

	rtk_init(&initInfo);

	phy_ctrl_set(0, 0, RTK_PHY_CTRL_SERDES_LOOPBACK_REMOTE, 1);

	return 0;
}

int32 cmd_phyLoopBackEnable_get() 
{
	return rtk_port_phyLoopBackEnable_get(0, 0, NULL);
}

int32 cmd_phyLoopBackEnable_set() 
{
	return rtk_port_phyLoopBackEnable_set(0, 0, ENABLED);
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

	if (port > max_port_num) { 
		printf("ERR! %s port %d\n",__FUNCTION__, port);
		return RT_ERR_OK; 
	}

	phy_addr = port_map[port];

	*pData = MDIO_READ(phy_addr, devad, phy_reg);
	//printf("MDIO_READ ADDR[%#x] REG[%#x] VAL[%#x]\n", phy_addr, phy_reg, *pData);
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
	int devad = MDIO_DEVAD_NONE;
	if (port > max_port_num) { 
		printf("ERR! %s port %d\n",__FUNCTION__, port);
		return RT_ERR_OK; 
	}

	phy_addr = port_map[port];

	//printf("MDIO WR ADDR[%#x] REG[%#x] VAL[%#x]\n", phy_addr, phy_reg, data);
	MDIO_WRITE(phy_addr, devad, phy_reg, data);
#if  0 
	uint32_t page = 0;
	page = MDIO_READ(phy_addr, 0x1e);
	MDIO_WRITE(phy_addr, 0x1e,0);
	data = MDIO_READ(phy_addr, 0x9);
	printf("MDIO RD ADDR[%#x] REG[%#x] VAL[%#x]\n", phy_addr, 0x9, data);
	if (data == 0xc00)
	{
		print_callstack();
		exit(-1);
	}
	MDIO_WRITE(phy_addr, 0x1e,page);
#endif	 

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

	if (port > max_port_num) { 
		printf("ERR! %s port %d\n",__FUNCTION__, port);
		return RT_ERR_OK; 
	}

	phy_addr = port_map[port];

	*pData = MDIO_READ(phy_addr, mmdAddr, mmdReg);

	if (g_debug)
		printf("%s entry unit=%d, port=%d, mmdAddr=%#x mmdReg=%#x  \n",__FUNCTION__, unit, port, mmdAddr, mmdReg);
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
	if (port > max_port_num) { 
		printf("ERR! %s port %d\n",__FUNCTION__, port);
		return RT_ERR_OK; 
	}

	phy_addr = port_map[port];

	MDIO_WRITE(phy_addr, mmdAddr, mmdReg, data);
	if (g_debug)
		printf("%s entry unit=%d, port=%d, mmdAddr=%#x mmdReg=%#x data=%#x\n",__FUNCTION__, unit, port, mmdAddr, mmdReg, data);

	return RT_ERR_OK;
} 

struct  cmd_list_t {
	char * name;
	int32_t (*func)();
} cmd_list[] = {
	{"init",init_phy_api},
};

int main(int argc, char * const argv[], char * envp[])
{
	char * name, *cmd;
	int i, ret;
	for (i=0; envp[i] != NULL; i ++) {
		if (0 == strcasecmp("debug=1", envp[i]))
			g_debug = 1;
	}

	if (argc <= 1) {
		return 0;
	}

	cmd = strrchr(argv[0], '/'); 
	cmd = cmd == NULL ? argv[0] : &cmd[1];

	if (0 == strcasecmp("rtl8214fc_cmd", cmd)) {
		my_port_descp = rtl8214fc_port_descp;
		my_phy_descp  = rtl8214fc_phy_descp;
		port_map = rtl8214fc_port_map;
		max_port_num = 3; 
		mdio_index = 0;
	} else if (0 == strcasecmp("rtl8261be_cmd", cmd)) {
		my_port_descp = rtl8261be_port_descp;
		my_phy_descp  = rtl8261be_phy_descp;
		port_map = rtl8261be_port_map;
		max_port_num = 3; 
		mdio_index = 2;
	} else {
		printf("please choise cmd \"rtl8214fc_cmd/rtl8261be_cmd\"\n");
		exit(-1);
	} 

	name = argv[1];
	for (i = 0; i < sizeof(cmd_list)/sizeof(cmd_list[0]); i++) {
		if (strcasecmp(cmd_list[i].name, name) == 0)
		{
			printf("EXE FUN[%s] ret=%d\n", name, cmd_list[i].func());
		}
	}
	if (i == sizeof(cmd_list)/sizeof(cmd_list[0])) {
		if (0 == strcasecmp("set_loopback", argv[1])) {
			if (argc < 3)
			{
				printf("bad arg\n");
				return 0;
			}
			int enable = atoi(argv[2]);
			printf("set loopback %s \n", enable ? "enable" : "disable");
			if (enable) {
				rtk_port_phyLoopBackEnable_set(0, 0, ENABLED);
				rtk_port_phyLoopBackEnable_set(0, 1, ENABLED);
				rtk_port_phyLoopBackEnable_set(0, 2, ENABLED);
				rtk_port_phyLoopBackEnable_set(0, 3, ENABLED);

			}else {
				rtk_port_phyLoopBackEnable_set(0, 0, 0);
				rtk_port_phyLoopBackEnable_set(0, 1, 0);
				rtk_port_phyLoopBackEnable_set(0, 2, 0);
				rtk_port_phyLoopBackEnable_set(0, 3, 0);
			}
		}

	}

	return 0;
}
