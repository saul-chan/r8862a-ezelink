/******************************************************************************
*                                                                             *
*  Copyright (c), 2023, Motorcomm Electronic Technology Co.,Ltd.              *
*  Motorcomm Confidential and Proprietary.                                    *
*                                                                             *
*******************************************************************************
*  File Name     : bprofile_yt9218m_default_demo.c
*  Version       : Initial Draft
*  Created       :
*  Last Modified :
*  Description   : board profile for yt9218m board,port(8 utp+2 uninited external port)
*
******************************************************************************/

/**************************************************
 *      Include header files                       *
 **************************************************/
#include "cal_bprofile.h"
#include "yt_error.h"
/**************************************************
 *      Constants or macros Declaration            *
 **************************************************/

/**************************************************
 *      Global variables                           *
 **************************************************/

/**************************************************
 *      Functions Declaration                      *
 **************************************************/
yt_ret_t cal_profile_yt9218m_init(yt_hwProfile_info_t *hwprofile_info);

yt_swDescp_t yt9218m_swDescp;

const board_profile_identify_t yt9218mProfileIdentifier = {BOARD_ID_YT9218MB, "yt9218 Default Demo"};

/* port descp */
const yt_portDescp_t yt9218mPortDescp[] =
{
    /*macid	attribute		phy_index	phy_addr	serdes_index	ethtype			medium				smi */
    {0,		PORT_ATTR_ETH,	0,			0,			INVALID_ID,		ETH_TYPE_GE,	PORT_MEDI_COPPER,	YT_SMI_INT},
    {1,		PORT_ATTR_ETH,	0,			1,			INVALID_ID,		ETH_TYPE_GE,	PORT_MEDI_COPPER,	YT_SMI_INT},
    {2,		PORT_ATTR_ETH,	0,			2,			INVALID_ID,		ETH_TYPE_GE,	PORT_MEDI_COPPER,	YT_SMI_INT},
    {3,		PORT_ATTR_ETH,	0,			3,			INVALID_ID,		ETH_TYPE_GE,	PORT_MEDI_COPPER,	YT_SMI_INT},
    {4,		PORT_ATTR_ETH,	0,			4,			INVALID_ID,		ETH_TYPE_GE,	PORT_MEDI_COPPER,	YT_SMI_INT},
    {5,		PORT_ATTR_ETH,	0,			5,			INVALID_ID,		ETH_TYPE_GE,	PORT_MEDI_COPPER,	YT_SMI_INT},
    {6,		PORT_ATTR_ETH,	0,			6,			INVALID_ID,		ETH_TYPE_GE,	PORT_MEDI_COPPER,	YT_SMI_INT},
    {7,		PORT_ATTR_ETH,	0,			7,			INVALID_ID,		ETH_TYPE_GE,	PORT_MEDI_COPPER,	YT_SMI_INT},
    {8,		PORT_ATTR_NONE,	INVALID_ID,			0,			INVALID_ID,		ETH_TYPE_GE,	PORT_MEDI_COPPER,	YT_SMI_EXT},
    {9,		PORT_ATTR_NONE,	INVALID_ID,			0,			INVALID_ID,		ETH_TYPE_GE,	PORT_MEDI_COPPER,	YT_SMI_EXT},
#ifdef INTER_MCU
    /* internel cpu port */
    {10,    PORT_ATTR_INT_CPU,			    INVALID_ID,	INVALID_ID,	PORT_ATTR_ETH,	ETH_TYPE_GE,	PORT_MEDI_COPPER,	INVALID_ID},
#endif
    {INVALID_ID,INVALID_ID,INVALID_ID,INVALID_ID,INVALID_ID,INVALID_ID,INVALID_ID,YT_SMI_NONE},
};

/* phy.descp */
const yt_phyDescp_t yt9218mPhyDescp[] =
{
	/*phy_id chip_model				start_mac_id	phy_max*/
	{0,         YT_PHY_MODEL_INT861X,	0,				8},
	{INVALID_ID,INVALID_ID,INVALID_ID,INVALID_ID},
};
    
/* LED description */
const yt_sled_remapInfo_t yt9218mRemapInfo[] = 
{
    {9, 0}, {8, 0}, {7, 0}, {6, 0}, {5, 0}, {4, 0}, {3, 0}, {2, 0}, {1, 0}, {0, 0},
    {9, 1}, {8, 1}, {7, 1}, {6, 1}, {5, 1}, {4, 1}, {3, 1}, {2, 1}, {1, 1}, {0, 1},
    {9, 2}, {8, 2}, {7, 2}, {6, 2}, {5, 2}, {4, 2}, {3, 2}, {2, 2}, {1, 2}, {0, 2}
};

yt_sled_param_t yt9218mSLEDParam = {
    LED_SERIAL_ACTIVE_MODE_LOW,
    SLED_DATANUM_YT9218_30, 
    0,
    yt9218mRemapInfo
};

const yt_ledDescp_t yt9218mLEDDescp = {LED_MODE_SERIAL, &yt9218mSLEDParam};


/* hardware profile */
const yt_hwProfile_t yt9218m_default_demo =
{
    .pIdentifier = &yt9218mProfileIdentifier,
    .profile_init = cal_profile_yt9218m_init,
};

yt_ret_t cal_profile_yt9218m_init(yt_hwProfile_info_t *hwprofile_info)
{
    uint8_t	i;

    hwprofile_info->pIdentifier = &yt9218mProfileIdentifier;

    /* switch info */
    hwprofile_info->switch_count = 1;

    yt9218m_swDescp.chip_id = YT_SW_ID_9218;
    yt9218m_swDescp.chip_model = YT_SW_MODEL_9218;

    i = 0;
    while(yt9218mPortDescp[i].mac_id != INVALID_ID)
    {
        yt9218m_swDescp.pPortDescp[i] = &yt9218mPortDescp[i];
        i++;
    }
    yt9218m_swDescp.port_num = i;

    i = 0;
    while(yt9218mPhyDescp[i].phy_index != INVALID_ID)
    {
        yt9218m_swDescp.pPhyDescp[i] = &yt9218mPhyDescp[i];
        i++;
    }

    yt9218mSLEDParam.remapInfoNum = sizeof(yt9218mRemapInfo)/sizeof(yt_sled_remapInfo_t);
    yt9218m_swDescp.pLEDDescp = &yt9218mLEDDescp;
    
    hwprofile_info->pSwDescp[0] = &yt9218m_swDescp;

    return CMM_ERR_OK;
}
