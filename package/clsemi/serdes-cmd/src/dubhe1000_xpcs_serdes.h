#ifndef __DUBHE1000_XPCS_SERDES_H__
#define __DUBHE1000_XPCS_SERDES_H__ 
#include "sys_hal.h"
#define SPEED_10		10
#define SPEED_100		100
#define SPEED_1000		1000
#define SPEED_2500		2500
#define SPEED_5000		5000
#define SPEED_10000		10000
#define SPEED_14000		14000
#define SPEED_20000		20000
#define SPEED_25000		25000
#define SPEED_40000		40000
#define SPEED_50000		50000
#define SPEED_56000		56000
#define SPEED_100000		100000
#define SPEED_200000		200000


#define SWITCH_BASE_ADDR                        (0x53000000)
#define XGMAC_BASE_ADDR(num)     				(num == 0 ? 0x53C00000 : \
												(num == 1 ? 0x53D00000 : \
												(num == 2 ? 0x53900000 : \
												(num == 3 ? 0x53A00000 : 0x53B00000))))

#define XPCS_BASE_ADDR(num)     				(num == 0 ? 0x52000000 : 0x52800000)
#define WIFI_5G_SUB_RST_PARA   0x90442818
#define DIF_2G_SUB_RST_PARA    0x9044281c
#define DIF_5G_SUB_RST_PARA    0x90442820
#define DPD_SUB_RST_PARA       0x90442824 
#define DIF_COM_SUB_RST_PARA   0x90442828 
#define AFE_SRC_SUB_RST_PARA   0x9044282C 
#define WIFI_2G_SUB_RST_PARA   0x90442814
#define WIFI_2G_SUB_CG_PARA0   0x90443018
#define WIFI_5G_SUB_CG_PARA0   0x90443020
#define DIF_2G_SUB_CG_PARA0    0x90443028
#define DIF_5G_SUB_CG_PARA0    0x90443030

#define FWD_SUB_CG_PARA 0x9044304C
#define PCIE_TOP_CLK_SEL_PARA               (0x9044120C) 
#define PCIE_TOP_PLL_PARA1				    (0x90441004)	
#define PCIE_TOP_PLL_PARA2					(0x90441008)	
#define PCIE_TOP_PLL_PARA3                  (0x9044100C)  

#define FWD_TOP_CLK_SEL_PARA                    (0x9041620c)
#define FWD_TOP_PLL_PARA3                       (0x9041600c)
#define FWD_TOP_PLL_PARA1						(0x90416004)
#define FWD_TOP_PLL_PARA2						(0x90416008)
#define DSMEN_BEGIN               	      	 0
#define DSMEN_END                    		 0
#define FBDIV_BEGIN                    		 4
#define FBDIV_END                     		 15
#define FRAC_BEGIN                     		 0
#define FRAC_END                     		 23
#define PLLEN_BEGIN                      	 3
#define PLLEN_END                            3
#define POSTDIV1_BEGIN                     	 24
#define POSTDIV1_END                     	 26
#define POSTDIV2_BEGIN                       28
#define POSTDIV2_END                     	 30
#define REFDIV_BEGIN                         16
#define REFDIV_END                           21
#define PLL_PARA3_BEGIN                       0
#define PLL_PARA3_END                         0
#define PLL_PARA_0_BEGIN                        (0)
#define PLL_PARA_0_END                       	(0)
#define CLK_SEL_PARA_BEGIN						(0)
#define CLK_SEL_PARA_END						(1)
#define FWD_SUB_RST_PARA       					(0x90442830)
#define SERDES0_SRAM_BYPASS       				(0x53E00000 + 0x218)
#define SERDES1_SRAM_BYPASS      				(0x53E00000 + 0x21C)
#define SR_MII_CTRL   							(0x1F0000*0x4)
#define SR_MII_1_CTRL                           (0x1A0000*0x4)
#define SR_MII_2_CTRL							(0x1B0000*0x4)
#define SR_MII_3_CTRL							(0x1C0000*0x4)
#define SR_XS_PCS_BASE							(0x030000*4)
#define SR_PMA_STATUS1     						((0x1*0x4) + (0x010000*0x4))
#define SR_XS_PCS_CTRL1     					(0x030000*0x4)
#define SR_XS_PCS_CTRL2     					((0x7*4) + SR_XS_PCS_CTRL1)
#define VR_MII_AN_CTRL       					((0x8001*4) + SR_MII_CTRL)
#define VR_MII_AN_INTR_STS      				((0x8002*4) + SR_MII_CTRL)
#define VR_MII_DIG_CTRL1       					((0x8000*4) + SR_MII_CTRL)
#define VR_MII_LINK_TIMER_CTRL       			((0x800A*4) + SR_MII_CTRL)
#define VR_MII_MP_12G_16G_25G_DFE_TAP_CTRL0     ((0x805E*0x4) + SR_MII_CTRL)
#define VR_MII_MP_12G_16G_25G_MPLL_CMN_CTRL     ((0x8070*0x4) + SR_MII_CTRL)
#define VR_MII_MP_12G_16G_25G_RX_ATTN_CTRL		((0x8057*0x4) + SR_MII_CTRL)
#define VR_MII_MP_12G_16G_25G_TX_BOOST_CTRL		((0x8033*0x4) + SR_MII_CTRL)
#define VR_MII_MP_12G_16G_25G_TX_GENCTRL0		((0x8030*0x4) + SR_MII_CTRL)
#define VR_MII_MP_12G_16G_25G_TX_STS            ((0x8040*0x4) + SR_MII_CTRL)
#define VR_MII_MP_25G_TX_GENCTRL2				((0x8032*0x4) + SR_MII_CTRL)
#define VR_MII_DIG_STS       					((0x8010*4) + SR_MII_CTRL)
#define VR_MII_1_DIG_STS       					((0x8010*4) + SR_MII_1_CTRL)
#define VR_MII_2_DIG_STS       					((0x8010*4) + SR_MII_2_CTRL)
#define VR_MII_3_DIG_STS       					((0x8010*4) + SR_MII_3_CTRL)
#define VR_MII_1_AN_CTRL       					((0x8001*4) + SR_MII_1_CTRL)
#define VR_MII_1_AN_INTR_STS      				((0x8002*4) + SR_MII_1_CTRL)
#define VR_MII_1_DIG_CTRL1       				((0x8000*4) + SR_MII_1_CTRL)
#define VR_MII_1_LINK_TIMER_CTRL       			((0x800A*4) + SR_MII_1_CTRL)
#define VR_MII_2_AN_CTRL       					((0x8001*4) + SR_MII_2_CTRL)
#define VR_MII_2_AN_INTR_STS      				((0x8002*4) + SR_MII_2_CTRL)
#define VR_MII_2_DIG_CTRL1       				((0x8000*4) + SR_MII_2_CTRL)
#define VR_MII_2_LINK_TIMER_CTRL       			((0x800A*4) + SR_MII_2_CTRL)
#define VR_MII_3_AN_CTRL       					((0x8001*4) + SR_MII_3_CTRL)
#define VR_MII_3_AN_INTR_STS      				((0x8002*4) + SR_MII_3_CTRL)
#define VR_MII_3_DIG_CTRL1       				((0x8000*4) + SR_MII_3_CTRL)
#define VR_MII_3_LINK_TIMER_CTRL       			((0x800A*4) + SR_MII_3_CTRL)
#define VR_MII_MP_12G_16G_25G_MISC_CTRL0      	((0x8090*0x4) + SR_MII_CTRL)
#define VR_MII_MP_12G_16G_25G_MISC_STS          ((0x8098*0x4) + SR_MII_CTRL)
#define VR_MII_MP_12G_16G_25G_REF_CLK_CTRL   	((0x8091*0x4) + SR_MII_CTRL)
#define VR_MII_MP_12G_16G_25G_RX_EQ_CTRL4    	((0x805C*0x4) + SR_MII_CTRL)
#define VR_MII_MP_12G_16G_25G_RX_GENCTRL0    	((0x8050*0x4) + SR_MII_CTRL)
#define VR_MII_MP_12G_16G_25G_RX_GENCTRL1       ((0x8051*0x4) + SR_MII_CTRL)
#define VR_MII_MP_12G_16G_25G_RX_RATE_CTRL      ((0x8054*0x4) + SR_MII_CTRL)
#define VR_MII_MP_12G_16G_25G_RX_STS       		((0x8060*0x4) + SR_MII_CTRL)
#define VR_MII_MP_12G_16G_25G_SRAM   			((0x809B*0x4) + SR_MII_CTRL)
#define VR_MII_MP_12G_16G_25G_TX_EQ_CTRL0       ((0x8036*0x4) + SR_MII_CTRL)
#define VR_MII_MP_12G_16G_25G_TX_EQ_CTRL1       ((0x8037*0x4) + SR_MII_CTRL)
#define VR_MII_MP_12G_16G_25G_TX_GENCTRL1       ((0x8031*0x4) + SR_MII_CTRL)
#define VR_MII_MP_12G_16G_25G_TX_RATE_CTRL      ((0x8034*0x4) + SR_MII_CTRL)
#define VR_MII_MP_12G_16G_25G_VCO_CAL_LD0       ((0x8092*0x4) + SR_MII_CTRL)
#define VR_MII_MP_12G_16G_MPLLA_CTRL0           ((0x8071*0x4) + SR_MII_CTRL)
#define VR_MII_MP_12G_16G_MPLLA_CTRL2       	((0x8073*0x4) + SR_MII_CTRL)
#define VR_MII_MP_12G_16G_RX_GENCTRL2           ((0x8052*0x4) + SR_MII_CTRL)
#define VR_MII_MP_12G_16G_TX_GENCTRL2           ((0x8032*0x4) + SR_MII_CTRL)
#define VR_MII_MP_16G_25G_RX_EQ_CTRL0       	((0x8058*0x4) + SR_MII_CTRL)
#define VR_MII_MP_16G_25G_RX_EQ_CTRL5           ((0x805D*0x4) + SR_MII_CTRL)
#define VR_MII_MP_16G_25G_RX_GENCTRL4       	((0x8068*0x4) + SR_MII_CTRL)
#define VR_MII_MP_16G_25G_RX_IQ_CTRL0       	((0x806B*0x4) + SR_MII_CTRL)
#define VR_MII_MP_16G_25G_RX_MISC_CTRL0       	((0x8069*0x4) + SR_MII_CTRL)
#define VR_MII_MP_16G_25G_TX_GENCTRL1       	((0x8031*0x4) + SR_MII_CTRL)
#define VR_MII_MP_16G_25G_VCO_CAL_REF0       	((0x8096*0x4) + SR_MII_CTRL)
#define VR_MII_MP_16G_MPLLA_CTRL3       		((0x8077*0x4) + SR_MII_CTRL)
#define VR_MII_MP_16G_RX_CDR_CTRL1        		((0x8064*0x4) + SR_MII_CTRL)
#define VR_MII_RX_LSTS       					((0x8020*0x4) + SR_MII_CTRL)
#define VR_MII_SNPS_CR_ADDR       				((0x80A1*0x4) + SR_MII_CTRL)
#define VR_MII_SNPS_CR_CTRL       				((0x80A0*0x4) + SR_MII_CTRL)
#define VR_MII_SNPS_CR_DATA       				((0x80A2*0x4) + SR_MII_CTRL)
#define VR_XS_PCS_DEBUG_CTRL       				((0x8005*0x4) + SR_XS_PCS_BASE)
#define VR_XS_PCS_DIG_CTRL1       				((0x8000*0x4) + SR_XS_PCS_BASE)
#define VR_XS_PCS_KR_CTRL       				((0x8007*0x4) + SR_XS_PCS_BASE)
#define VR_XS_PMA_RX_LSTS       				((0x018020*0x4))
#define VR_XS_PMA_MP_12G_16G_25G_MISC_CTRL0     ((0x0070*0x4) + VR_XS_PMA_RX_LSTS)
#define VR_XS_PMA_MP_12G_16G_25G_MISC_STS       ((0x0078*0x4) + VR_XS_PMA_RX_LSTS)
#define VR_XS_PMA_MP_12G_16G_25G_REF_CLK_CTRL   ((0x0071*0x4) + VR_XS_PMA_RX_LSTS)
#define VR_XS_PMA_MP_12G_16G_25G_RX_EQ_CTRL4    ((0x003C*0x4) + VR_XS_PMA_RX_LSTS)
#define VR_XS_PMA_MP_12G_16G_25G_RX_GENCTRL0    ((0x0030*0x4) + VR_XS_PMA_RX_LSTS)
#define VR_XS_PMA_MP_12G_16G_25G_RX_GENCTRL1    ((0x0031*0x4) + VR_XS_PMA_RX_LSTS)
#define VR_XS_PMA_MP_12G_16G_25G_RX_RATE_CTRL   ((0x0034*0x4) + VR_XS_PMA_RX_LSTS)
#define VR_XS_PMA_MP_12G_16G_25G_RX_STS       	((0x0040*0x4) + VR_XS_PMA_RX_LSTS)
#define VR_XS_PMA_MP_12G_16G_25G_SRAM     		((0x007B*0x4) + VR_XS_PMA_RX_LSTS)
#define VR_XS_PMA_MP_12G_16G_25G_TX_EQ_CTRL0    ((0x0016*0x4) + VR_XS_PMA_RX_LSTS)
#define VR_XS_PMA_MP_12G_16G_25G_TX_EQ_CTRL1    ((0x0017*0x4) + VR_XS_PMA_RX_LSTS)
#define VR_XS_PMA_MP_12G_16G_25G_TX_GENCTRL1    ((0x0011*0x4) + VR_XS_PMA_RX_LSTS)
#define VR_XS_PMA_MP_12G_16G_25G_TX_RATE_CTRL   ((0x0014*0x4) + VR_XS_PMA_RX_LSTS)
#define VR_XS_PMA_MP_12G_16G_25G_VCO_CAL_LD0    ((0x0072*0x4) + VR_XS_PMA_RX_LSTS)
#define VR_XS_PMA_MP_12G_16G_MPLLA_CTRL0        ((0x0051*0x4) + VR_XS_PMA_RX_LSTS)
#define VR_XS_PMA_MP_12G_16G_MPLLA_CTRL2        ((0x0053*0x4) + VR_XS_PMA_RX_LSTS)
#define VR_XS_PMA_MP_12G_16G_RX_GENCTRL2        ((0x0032*0x4) + VR_XS_PMA_RX_LSTS)
#define VR_XS_PMA_MP_12G_16G_TX_GENCTRL2       	((0x0012*0x4) + VR_XS_PMA_RX_LSTS)
#define VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0       	((0x0038*0x4) + VR_XS_PMA_RX_LSTS)
#define VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL5       	((0x003D*0x4) + VR_XS_PMA_RX_LSTS)
#define VR_XS_PMA_MP_16G_25G_RX_GENCTRL4       	((0x0048*0x4) + VR_XS_PMA_RX_LSTS)
#define VR_XS_PMA_MP_16G_25G_RX_IQ_CTRL0        ((0x004B*0x4) + VR_XS_PMA_RX_LSTS)
#define VR_XS_PMA_MP_16G_25G_RX_MISC_CTRL0      ((0x0049*0x4) + VR_XS_PMA_RX_LSTS)
#define VR_XS_PMA_MP_16G_25G_VCO_CAL_REF0       ((0x0076*0x4) + VR_XS_PMA_RX_LSTS)
#define VR_XS_PMA_MP_16G_MPLLA_CTRL3     		((0x0057*0x4) + VR_XS_PMA_RX_LSTS)
#define VR_XS_PMA_MP_16G_RX_CDR_CTRL1       	((0x0044*0x4) + VR_XS_PMA_RX_LSTS)
#define VR_XS_PMA_MP_6G_RXGENCTRL0       		((0x0038*0x4) + VR_XS_PMA_RX_LSTS)
#define VR_XS_PMA_SNPS_CR_ADDR       			((0x0081*0x4) + VR_XS_PMA_RX_LSTS)
#define VR_XS_PMA_SNPS_CR_CTRL       			((0x0080*0x4) + VR_XS_PMA_RX_LSTS)
#define VR_XS_PMA_SNPS_CR_DATA       			((0x0082*0x4) + VR_XS_PMA_RX_LSTS)
#define VR_XS_PMA_MP_12G_16G_25G_DFE_TAP_CTRL0	((0x003E*0x4) + VR_XS_PMA_RX_LSTS)
#define VR_XS_PMA_MP_12G_16G_25G_MPLL_CMN_CTRL	((0x0050*0x4) + VR_XS_PMA_RX_LSTS)
#define VR_XS_PMA_MP_12G_16G_25G_RX_ATTN_CTRL	((0x0037*0x4) + VR_XS_PMA_RX_LSTS)
#define VR_XS_PMA_MP_12G_16G_25G_TX_BOOST_CTRL	((0x0013*0x4) + VR_XS_PMA_RX_LSTS)
#define VR_XS_PMA_MP_12G_16G_25G_TX_GENCTRL0	((0x0010*0x4) + VR_XS_PMA_RX_LSTS)
#define VR_XS_PMA_MP_12G_16G_25G_TX_STS			((0x0020*0x4) + VR_XS_PMA_RX_LSTS)
#define ADDRESS_BEGIN							0
#define ADDRESS_END								15
#define AN_ENABLE_BEGIN							12
#define AN_ENABLE_END							12
#define CL37_ANCMPLT_INTR_BEGIN					0
#define CL37_ANCMPLT_INTR_END					0
#define CL37_LINK_TIME_BEGIN					0
#define CL37_LINK_TIME_END						15
#define CL37_TMR_OVR_RIDE_BEGIN					3
#define CL37_TMR_OVR_RIDE_END					3
#define CONT_ADAPT_0_BEGIN						0
#define CONT_ADAPT_0_END						0
#define CTLE_BOOST_0_BEGIN						0
#define CTLE_BOOST_0_END						4
#define CTLE_POLE_0_BEGIN						5
#define CTLE_POLE_0_END							6
#define DATA_BEGIN								0
#define DATA_END								15
#define EN_2_5G_MODE_BEGIN						2
#define EN_2_5G_MODE_END						2
#define EXT_LD_DN_BEGIN							1
#define EXT_LD_DN_END							1
#define INIT_DN_BEGIN							0
#define INIT_DN_END								0
#define MAC_AUTO_SW_BEGIN						9
#define MAC_AUTO_SW_END							9
#define MII_AN_INTR_EN_BEGIN					0
#define MII_AN_INTR_EN_END						0
#define MII_CTRL_BEGIN							8
#define MII_CTRL_END							8
#define MPLLA_BANDWIDTH_BEGIN					0
#define MPLLA_BANDWIDTH_END						15
#define MPLLA_DIV10_CLK_EN_BEGIN				9
#define MPLLA_DIV10_CLK_EN_END					9
#define MPLLA_DIV16P5_CLK_EN_BEGIN				10
#define MPLLA_DIV16P5_CLK_EN_END				10
#define MPLLA_DIV8_CLK_EN_BEGIN					8
#define MPLLA_DIV8_CLK_EN_END					8
#define MPLLA_MULTIPLIER_BEGIN					0
#define MPLLA_MULTIPLIER_END					7
#define PCS_MODE_BEGIN							1
#define PCS_MODE_END							2
#define PCS_TYPE_SEL_BEGIN						0
#define PCS_TYPE_SEL_END						3
#define REF_RPT_CLK_EN_BEGIN					8
#define REF_RPT_CLK_EN_END						8
#define RLU_BEGIN								2
#define RLU_END									2
#define RST_BEGIN								15
#define RST_END									15
#define RX0_ADPT_MODE_BEGIN						4
#define RX0_ADPT_MODE_END						5
#define RX0_DELTA_IQ_BEGIN						8
#define RX0_DELTA_IQ_END						11
#define RX0_MISC_BEGIN							0
#define RX0_MISC_END							7
#define RX0_RATE_1_0_BEGIN						0
#define RX0_RATE_1_0_END						1
#define RX0_RATE_2_0_BEGIN						0
#define RX0_RATE_2_0_END						2
#define RX0_WIDTH_BEGIN							8
#define RX0_WIDTH_END							9
#define RX2TX_LB_EN_0_BEGIN						4
#define RX2TX_LB_EN_0_END						4
#define RX_ACK_0_BEGIN							0
#define RX_ACK_0_END							0
#define RX_ADAPT_ACK_BEGIN						12
#define RX_ADAPT_ACK_END						12
#define RX_ADPT_PROG_0_BEGIN					12
#define RX_ADPT_PROG_0_END						12
#define RX_ADPT_SEL_0_BEGIN						0
#define RX_ADPT_SEL_0_END						0
#define RX_AD_REQ_BEGIN							12
#define RX_AD_REQ_END							12
#define RX_DFE_BYP_0_BEGIN						8
#define RX_DFE_BYP_0_END						8
#define RX_DT_EN_0_BEGIN						8
#define RX_DT_EN_0_END							8
#define RX_DT_EN_CTL_BEGIN						6
#define RX_DT_EN_CTL_END						6
#define RX_RST_0_BEGIN							4
#define RX_RST_0_END							4
#define RX_VALID_0_BEGIN						12
#define RX_VALID_0_END							12
#define SGMII_LINK_STS_BEGIN					4
#define SGMII_LINK_STS_END						4
#define SIG_DET_0_BEGIN							4
#define SIG_DET_0_END							4
#define SS13_BEGIN								13
#define SS13_END								13
#define SS3_BEGIN								5
#define SS3_END									5
#define SS6_BEGIN								6
#define SS6_END									6
#define START_BUSY_BEGIN						0
#define START_BUSY_END							0
#define SUPRESS_LOS_DET_BEGIN					4
#define SUPRESS_LOS_DET_END						4
#define TX0_RATE_BEGIN							0
#define TX0_RATE_END							2
#define TX0_WIDTH_BEGIN							8
#define TX0_WIDTH_END							9
#define TX2RX_LB_EN_0_BEGIN						0
#define TX2RX_LB_EN_0_END						0

#define TX_CLK_RDY_3_1_BEGIN					13
#define TX_CLK_RDY_3_1_END						15

#define TX_CLK_RDY_0_BEGIN						12
#define TX_CLK_RDY_0_END						12
#define TX_CONFIG_BEGIN							3
#define TX_CONFIG_END							3
#define TX_EQ_MAIN_BEGIN						8
#define TX_EQ_MAIN_END							13
#define TX_EQ_OVR_RIDE_BEGIN					6
#define TX_EQ_OVR_RIDE_END						6
#define TX_EQ_POST_1_BEGIN						8
#define TX_EQ_POST_1_END						13
#define TX_EQ_POST_2_BEGIN						0
#define TX_EQ_POST_2_END						5
#define TX_EQ_PRE_BEGIN							0
#define TX_EQ_PRE_END							5
#define USRA_RST_1_BEGIN						10
#define USRA_RST_1_END							10
#define USRA_RST_2_BEGIN						5
#define USRA_RST_2_END							5
#define USXG_2PT5G_GMII_BEGIN					13
#define USXG_2PT5G_GMII_END						13
#define USXG_AN_STS_BEGIN						8
#define USXG_AN_STS_END							14
#define USXG_AN_STS_14_BEGIN					14
#define USXG_AN_STS_14_END						14
#define USXG_AN_STS_13_BEGIN					13
#define USXG_AN_STS_13_END						13
#define USXG_AN_STS_12_10_BEGIN					10
#define USXG_AN_STS_12_10_END					12
#define USXG_AN_STS_9_BEGIN						9
#define USXG_AN_STS_9_END						9
#define USXG_AN_STS_8_BEGIN						8
#define USXG_AN_STS_8_END						8
#define USXG_EN_BEGIN							9
#define USXG_EN_END								9
#define USXG_MODE_BEGIN							10
#define USXG_MODE_END							12
#define VBOOST_EN_0_BEGIN						4
#define VBOOST_EN_0_END							4
#define VCO_FRQBAND_0_BEGIN						8
#define VCO_FRQBAND_0_END						9
#define VCO_LD_VAL_0_BEGIN						0
#define VCO_LD_VAL_0_END						12
#define VCO_REF_LD_0_BEGIN						0
#define VCO_REF_LD_0_END						6
#define VCO_STEP_CTRL_0_BEGIN					4
#define VCO_STEP_CTRL_0_END						4
#define VCO_TEMP_COMP_EN_0_BEGIN				0
#define VCO_TEMP_COMP_EN_0_END					0
#define VGA1_GAIN_0_BEGIN						12
#define VGA1_GAIN_0_END							14
#define VGA2_GAIN_0_BEGIN						8
#define VGA2_GAIN_0_END							10
#define VR_RST_BEGIN							15
#define VR_RST_END								15
#define WR_RDN_BEGIN							1
#define WR_RDN_END								1
#define TX_REQ_0_BEGIN                   0
#define TX_REQ_0_END                     0
#define TX_ACK_0_BEGIN   				 0
#define TX_ACK_0_END 					 0
#define RX_REQ_0_BEGIN 					 0
#define RX_REQ_0_END                     0
#define DFE_TAP1_0_BEGIN				 0
#define DFE_TAP1_0_END				 	 7
#define MPLLB_SEL_0_BEGIN				 4
#define MPLLB_SEL_0_END				 	 4
#define REF_CLK_DIV2_BEGIN				 2
#define REF_CLK_DIV2_END				 2
#define REF_MPLLA_DIV2_BEGIN			 6
#define REF_MPLLA_DIV2_END				 6
#define RX0_EQ_ATT_LVL_BEGIN			 0
#define RX0_EQ_ATT_LVL_END				 2
#define RX_DIV16P5_CLK_EN_0_BEGIN		 12
#define RX_DIV16P5_CLK_EN_0_END			 12
#define TX0_IBOOST_BEGIN				 0
#define TX0_IBOOST_END				 	 3
#define TX_DT_EN_0_BEGIN				 12
#define TX_DT_EN_0_END				 	 12
#define TX_EQ_POST_BEGIN				 0
#define TX_EQ_POST_END				 8
#define VBOOST_LVL_BEGIN				 8
#define VBOOST_LVL_END				 	10
#define serdes0_sram_bypass_BEGIN				0
#define serdes0_sram_bypass_END					0
#define serdes1_sram_bypass_BEGIN				0
#define serdes1_sram_bypass_END					0
#define sft_rst_fwd_sub_n_BEGIN					0
#define sft_rst_fwd_sub_n_END					0
#define XGMAC0_SFT_RST_N  						(0x53E00000 + 0x290)
#define XGMAC1_SFT_RST_N  						(0x53E00000 + 0x294)
#define XGMAC2_SFT_RST_N  						(0x53E00000 + 0x298)
#define XGMAC3_SFT_RST_N  						(0x53E00000 + 0x29C)
#define XGMAC4_SFT_RST_N  						(0x53E00000 + 0x2A0)
#define xgmac0_sft_rst_n_BEGIN 					0
#define xgmac0_sft_rst_n_END                  	0
#define xgmac1_sft_rst_n_BEGIN 					0
#define xgmac1_sft_rst_n_END 					0
#define xgmac2_sft_rst_n_BEGIN 					0
#define xgmac2_sft_rst_n_END 					0
#define xgmac3_sft_rst_n_BEGIN 					0
#define xgmac3_sft_rst_n_END 					0
#define xgmac4_sft_rst_n_BEGIN 					0
#define xgmac4_sft_rst_n_END 					0
#define SR_MII_OFFSET(_x)           ({ 				\
		typeof(_x) (x) = (_x); \
		(((x) == 0 ? 0 :  ((((x) - 1) * 4 *0x10000) + SR_MII_1_CTRL - SR_MII_CTRL))); \
		})

#endif /*__DUBHE1000_XPCS_SERDES_H__*/
