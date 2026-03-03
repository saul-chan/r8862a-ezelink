

//#include <common/debug/rt_log.h>
#include <hal/phy/phydef.h>
#include <hal/common/miim.h>
#include <phy/inc/phy_init.h>
#include <osal/lib.h>
#include <osal/print.h>


/*
 * This table is used to describe your hardware board design, especially for mapping relation between port and phy.
 * Port related information
 * .mac_id      = port id.
 * .phy_idex    = used to indicate which PHY entry is used by this port in glued_phy_Descp[].
 * .eth         = Ethernet speed type (refer to rt_port_ethType_t).
 * .medi        = Port media type (refer to rt_port_medium_t).
 */
phy_hwp_portDescp_t  my_port_descp[] = {
        { .mac_id =  0,  .phy_idx = 0, .eth = HWP_GE,   .medi = HWP_COPPER,},
        { .mac_id =  1,  .phy_idx = 0, .eth = HWP_GE,   .medi = HWP_COPPER,},
        { .mac_id =  2,  .phy_idx = 0, .eth = HWP_GE,   .medi = HWP_COPPER,},
        { .mac_id =  3,  .phy_idx = 0, .eth = HWP_GE,   .medi = HWP_COPPER,},
        { .mac_id = HWP_END },
    };

/*
 * PHY related information
 * .chip        = PHY Chip model (refer to phy_type_t).
 * .mac_id      = The first port id of this PHY. For example, the 8218D is connected to
 *                port 0 ~ 7, then the .mac_id  = 0.
 * .phy_max     = The MAX port number of this PHY. For examplem the 8218D is an octet PHY,
 *                so this number is 8.
 */

phy_hwp_phyDescp_t     my_phy_Descp[] = {
        [0] = { .chip = RTK_PHYTYPE_RTL8214FC, .mac_id = 0, .phy_max = 4 },
        [1] = { .chip = HWP_END },
    };


rtk_port_phyTestMode_t      testMode;
rtk_port_phyPolarCtrl_t     polarCtrl;

void all_rtk_phy_api(void)
{
	osal_memset(&testMode, 0, sizeof(testMode));
	osal_memset(&polarCtrl, 0, sizeof(polarCtrl));
}

int32 cmd_linkMedia_get() 
{
	return rtk_port_linkMedia_get(0, 0, NULL, NULL);
}

int32 cmd_speedDuplex_get() 
{
	return rtk_port_speedDuplex_get( 0, 0, NULL, NULL);
}

int32 cmd_adminEnable_set() 
{
	return rtk_port_adminEnable_set(0, 0, ENABLED);
}

int32 cmd_phyAutoNegoEnable_get() 
{
	return rtk_port_phyAutoNegoEnable_get(0, 0, NULL);
}

int32 cmd_phyAutoNegoEnable_set() 
{
	return rtk_port_phyAutoNegoEnable_set(0, 0, ENABLED);
}

int32 cmd_phyAutoNegoAbilityLocal_get() 
{
	return rtk_port_phyAutoNegoAbilityLocal_get(0, 0, NULL);
}

int32 cmd_phyAutoNegoAbility_get() 
{
	return rtk_port_phyAutoNegoAbility_get( 0, 0, NULL);
}

int32 cmd_phyAutoNegoAbility_set() 
{
	return rtk_port_phyAutoNegoAbility_set( 0, 0, NULL);
}

int32 cmd_phyForceModeAbility_get() 
{
	return rtk_port_phyForceModeAbility_get( 0, 0, NULL, NULL, NULL);
}

int32 cmd_phyForceModeAbility_set() 
{
	return rtk_port_phyForceModeAbility_set( 0, 0, PORT_SPEED_1000M, PORT_FULL_DUPLEX, ENABLED);
}


int32 cmd_phyForceFlowctrlMode_get() 
{
	return rtk_port_phyForceFlowctrlMode_get( 0, 0, NULL);
}
int32 cmd_phyForceFlowctrlMode_set() 
{
	return rtk_port_phyForceFlowctrlMode_set( 0, 0, NULL);
}
int32 cmd_phyMasterSlave_get() 
{
	return rtk_port_phyMasterSlave_get( 0, 0, NULL, NULL);
}
int32 cmd_phyMasterSlave_set() 
{
	return rtk_port_phyMasterSlave_set( 0, 0, PORT_AUTO_MODE);
}
int32 cmd_phyReg_get() 
{
	return rtk_port_phyReg_get( 0, 0, 0x1234, 31, NULL);
}
int32 cmd_phyReg_set() 
{
	return rtk_port_phyReg_set( 0, 0, 0x1234, 31, 0x3260);
}
int32 cmd_phyExtParkPageReg_get() 
{
	return rtk_port_phyExtParkPageReg_get( 0, 0, 0x4321, 0x3820, 0, 31, NULL);
}
int32 cmd_phyExtParkPageReg_set() 
{
	return rtk_port_phyExtParkPageReg_set( 0, 0, 0x4321, 0x3820, 0, 31, 0x3260);
}
int32 cmd_phymaskExtParkPageReg_set() 
{
	return rtk_port_phymaskExtParkPageReg_set( 0, NULL, 0x4321, 0x3820, 0, 31, 0x3260);
}
int32 cmd_phyMmdReg_get() 
{
	return rtk_port_phyMmdReg_get( 0, 0, 7, 3311, NULL);
}
int32 cmd_phyMmdReg_set() 
{
	return rtk_port_phyMmdReg_set( 0, 0, 7, 3311, 0x3260);
}
int32 cmd_phymaskMmdReg_set() 
{
	return rtk_port_phymaskMmdReg_set( 0, NULL, 7, 3311, 0x3260);
}
int32 cmd_phyComboPortMedia_get() 
{
	return rtk_port_phyComboPortMedia_get(0, 0, NULL);
}
int32 cmd_phyComboPortMedia_set() 
{
	return rtk_port_phyComboPortMedia_set(0, 0, PORT_MEDIA_COPPER);
}
int32 cmd_greenEnable_get() 
{
	return rtk_port_greenEnable_get(0, 0, NULL);
}
int32 cmd_greenEnable_set() 
{
	return rtk_port_greenEnable_set(0, 0, ENABLED);
}
int32 cmd_phyCrossOverMode_get() 
{
	return rtk_port_phyCrossOverMode_get(0, 0, NULL);
}
int32 cmd_phyCrossOverMode_set() 
{
	return rtk_port_phyCrossOverMode_set(0, 0, PORT_CROSSOVER_MODE_AUTO);
}
int32 cmd_phyCrossOverStatus_get() 
{
	return rtk_port_phyCrossOverStatus_get(0, 0, NULL);
}
int32 cmd_linkDownPowerSavingEnable_get() 
{
	return rtk_port_linkDownPowerSavingEnable_get(0, 0, NULL);
}
int32 cmd_linkDownPowerSavingEnable_set() 
{
	return rtk_port_linkDownPowerSavingEnable_set(0, 0, ENABLED);
}
int32 cmd_gigaLiteEnable_get() 
{
	return rtk_port_gigaLiteEnable_get(0, 0, NULL);
}
int32 cmd_gigaLiteEnable_set() 
{
	return rtk_port_gigaLiteEnable_set(0, 0, ENABLED);
}
int32 cmd_phyReconfig_register() 
{
	return rtk_port_phyReconfig_register(0, NULL);
}
int32 cmd_phyReconfig_unregister() 
{
	return rtk_port_phyReconfig_unregister(0);
}
int32 cmd_downSpeedEnable_get() 
{
	return rtk_port_downSpeedEnable_get(0, 0, NULL);
}
int32 cmd_downSpeedEnable_set() 
{
	return rtk_port_downSpeedEnable_set(0, 0, ENABLED);
}
int32 cmd_downSpeedStatus_get() 
{
	return rtk_port_downSpeedStatus_get(0, 0, NULL);
}
int32 cmd_phyLoopBackEnable_get() 
{
	return rtk_port_phyLoopBackEnable_get(0, 0, NULL);
}
int32 cmd_phyLoopBackEnable_set() 
{
	return rtk_port_phyLoopBackEnable_set(0, 0, ENABLED);
}
int32 cmd_phyPolar_get() 
{
	return rtk_port_phyPolar_get(0, 0, NULL);
}
int32 cmd_phyPolar_set() 
{
	return rtk_port_phyPolar_set(0, 0, &polarCtrl);
}
int32 cmd_phySdsRxCaliStatus_get() 
{
	return rtk_port_phySdsRxCaliStatus_get(0, 0, 12, NULL);
}
int32 cmd_phyReset_set() 
{
	return rtk_port_phyReset_set(0, 0);
}
int32 cmd_phyLinkStatus_get() 
{
	return rtk_port_phyLinkStatus_get(0, 0, NULL);
}
int32 cmd_phyPeerAutoNegoAbility_get() 
{
	return rtk_port_phyPeerAutoNegoAbility_get(0, 0, NULL);
}
int32 cmd_phyMacIntfSerdesMode_get() 
{
	return rtk_port_phyMacIntfSerdesMode_get(0, 0, NULL);
}
int32 cmd_phyLedMode_set() 
{
	return rtk_port_phyLedMode_set(0, 0, NULL);
}
int32 cmd_phyLedCtrl_get() 
{
	return rtk_port_phyLedCtrl_get(0, 0, NULL);
}
int32 cmd_phyLedCtrl_set() 
{
	return rtk_port_phyLedCtrl_set(0, 0, NULL);
}
int32 cmd_phyMacIntfSerdesLinkStatus_get() 
{
	return rtk_port_phyMacIntfSerdesLinkStatus_get(0, 0, NULL);
}
int32 cmd_phySdsEyeParam_get() 
{
	return rtk_port_phySdsEyeParam_get(0, 0, 12, NULL);
}
int32 cmd_phySdsEyeParam_set() 
{
	return rtk_port_phySdsEyeParam_set(0, 0, 12, NULL);
}
int32 cmd_phyMdiLoopbackEnable_get() 
{
	return rtk_port_phyMdiLoopbackEnable_get(0, 0, NULL);
}
int32 cmd_phyMdiLoopbackEnable_set() 
{
	return rtk_port_phyMdiLoopbackEnable_set(0, 0, ENABLED);
}
int32 cmd_phyIntr_init() 
{
	return rtk_port_phyIntr_init(0, 0, RTK_PHY_INTR_COMMON);
}
int32 cmd_phyIntrEnable_get() 
{
	return rtk_port_phyIntrEnable_get(0, 0, RTK_PHY_INTR_STATUS_RLFD, NULL);
}
int32 cmd_phyIntrEnable_set() 
{
	return rtk_port_phyIntrEnable_set(0, 0, RTK_PHY_INTR_STATUS_RLFD, ENABLED);
}
int32 cmd_phyIntrStatus_get() 
{
	return rtk_port_phyIntrStatus_get(0, 0, RTK_PHY_INTR_COMMON, NULL);
}
int32 cmd_phyIntrMask_get() 
{
	return rtk_port_phyIntrMask_get(0, 0, RTK_PHY_INTR_COMMON, NULL);
}
int32 cmd_phyIntrMask_set() 
{
	return rtk_port_phyIntrMask_set(0, 0, RTK_PHY_INTR_COMMON, 0);
}
int32 cmd_phySdsTestMode_set() 
{
	return rtk_port_phySdsTestMode_set(0, 0, 2, RTK_SDS_TESTMODE_PRBS11);
}
int32 cmd_phySdsTestModeCnt_get() 
{
	return rtk_port_phySdsTestModeCnt_get(0, 0, 2, NULL);
}
int32 cmd_phySdsLeq_get() 
{
	return rtk_port_phySdsLeq_get(0, 0, 12, NULL, NULL);
}
int32 cmd_phySdsLeq_set() 
{
	return rtk_port_phySdsLeq_set(0, 0, 12, ENABLED, 5678);
}
int32 cmd_phySds_get() 
{
	return rtk_port_phySds_get(0, 0, NULL);
}
int32 cmd_phySds_set() 
{
	return rtk_port_phySds_set(0, 0, NULL);
}
int32 cmd_phyCtrl_get() 
{
	return rtk_port_phyCtrl_get(0, 0, RTK_PHY_CTRL_AN_COMPLETE, NULL);
}
int32 cmd_phyCtrl_set() 
{
	return rtk_port_phyCtrl_set(0, 0, RTK_PHY_CTRL_AN_COMPLETE, 10);
}
int32 cmd_phyDbgCounter_get() 
{
	return rtk_port_phyDbgCounter_get(0, 0, PHY_DBG_CNT_RX, NULL);
}

int init_phy_api(void)
{
    rtk_phy_initInfo_t initInfo;

    initInfo.port_desc = my_port_descp;
    initInfo.phy_desc = my_phy_Descp;
    rtk_init(&initInfo);

//	rtk_port_phyForceModeAbility_set( 0, 0, PORT_SPEED_1000M, PORT_FULL_DUPLEX, ENABLED);
//	rtk_port_phyForceModeAbility_set( 0, 1, PORT_SPEED_1000M, PORT_FULL_DUPLEX, ENABLED);
//	rtk_port_phyForceModeAbility_set( 0, 2, PORT_SPEED_1000M, PORT_FULL_DUPLEX, ENABLED);
//	rtk_port_phyForceModeAbility_set( 0, 3, PORT_SPEED_1000M, PORT_FULL_DUPLEX, ENABLED);
/*
	rtk_port_phyComboPortMedia_set(0, 0, PORT_MEDIA_COPPER);
	rtk_port_phyComboPortMedia_set(0, 1, PORT_MEDIA_COPPER);
	rtk_port_phyComboPortMedia_set(0, 2, PORT_MEDIA_COPPER);
	rtk_port_phyComboPortMedia_set(0, 3, PORT_MEDIA_COPPER);

    rtk_port_phyAutoNegoEnable_set(0, 0, ENABLED);
    rtk_port_phyAutoNegoEnable_set(0, 1, ENABLED);
    rtk_port_phyAutoNegoEnable_set(0, 2, ENABLED);
    rtk_port_phyAutoNegoEnable_set(0, 3, ENABLED);
    //rtk_port_downSpeedEnable_set(0, 8, ENABLED);
*/
    return 0;
}
