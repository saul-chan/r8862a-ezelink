#ifndef 	__CLS_NPE_XPCS_SERDES_REGS_H__
#define     __CLS_NPE_XPCS_SERDES_REGS_H__

#define SERDES0_SRAM_BYPASS                             0x53e00218
#define SERDES1_SRAM_BYPASS                             0x53e0021c
#define SERDES0_SRAM_BYPASS_OFFSET           	 0x0218
#define SERDES1_SRAM_BYPASS_OFFSET      		 0x021c

/* XPCS */
#define SR_XS_PCS_BASE				(0x030000 * 4)
#define SR_XS_PCS_CTRL2     		((0x7 * 4) + SR_XS_PCS_BASE)
#define VR_XS_PCS_DIG_CTRL1 		(0x8000 * 4 + SR_XS_PCS_BASE)
#define VR_XS_PCS_KR_CTRL   		(0x8007 * 4 + SR_XS_PCS_BASE)
#define VR_XS_PCS_DIG_STS           (0x8010 * 4 + SR_XS_PCS_BASE)
#define SR_XS_PCS_CTRL1             SR_XS_PCS_BASE

#define SR_MII_CTRL								(0x1F0000*0x4)
#define SR_MII_1_CTRL                           (0x1A0000*0x4)
#define SR_MII_2_CTRL							(0x1B0000*0x4)
#define SR_MII_3_CTRL							(0x1C0000*0x4)
#define VR_XS_PMA_RX_LSTS       				((0x018020*0x4))

#define VR_MII_AN_CTRL       					((0x8001*4) + SR_MII_CTRL)
#define VR_MII_AN_INTR_STS      				((0x8002*4) + SR_MII_CTRL)
#define VR_MII_DIG_CTRL1       					((0x8000*4) + SR_MII_CTRL)
#define VR_MII_LINK_TIMER_CTRL       			((0x800A*4) + SR_MII_CTRL)

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

#define VR_MII_MP_12G_16G_25G_DFE_TAP_CTRL0     ((0x805E*0x4) + SR_MII_CTRL)
#define VR_MII_MP_12G_16G_25G_MPLL_CMN_CTRL     ((0x8070*0x4) + SR_MII_CTRL)
#define VR_MII_MP_12G_16G_25G_RX_ATTN_CTRL		((0x8057*0x4) + SR_MII_CTRL)
#define VR_MII_MP_12G_16G_25G_TX_BOOST_CTRL		((0x8033*0x4) + SR_MII_CTRL)
#define VR_MII_MP_12G_16G_25G_TX_GENCTRL0		((0x8030*0x4) + SR_MII_CTRL)
#define VR_MII_MP_12G_16G_25G_TX_STS            ((0x8040*0x4) + SR_MII_CTRL)
#define VR_MII_MP_25G_TX_GENCTRL2				((0x8032*0x4) + SR_MII_CTRL)
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
#define VR_XS_PMA_MP_12G_16G_25G_MISC_CTRL0     ((0x0070*0x4) + VR_XS_PMA_RX_LSTS)
#define VR_XS_PMA_MP_12G_16G_25G_MISC_STS       ((0x0078*0x4) + VR_XS_PMA_RX_LSTS)
#define VR_XS_PMA_MP_12G_16G_25G_REF_CLK_CTRL   ((0x0071*0x4) + VR_XS_PMA_RX_LSTS)
#define VR_XS_PMA_MP_12G_16G_25G_RX_EQ_CTRL4    ((0x003C*0x4) + VR_XS_PMA_RX_LSTS)
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
#define MII_CTRL                     0
#define PCS_TYPE_SEL_10GBX   		(1)
#define PCS_TYPE_SEL_10GBR   		(0)
#define PCS_TYPE_SEL_5GB     		(15)
#define PCS_TYPE_SEL_2_5GB     		(14)
#define PCS_TYPE_SEL_BIT      		(0)
#define PCS_TYPE_SEL_BIT_E     		(3)
#define PCS_MODE_BIT                (1)
#define PCS_MODE_BIT_E     	    	(2)
#define USXG_MODE_BIT               (10)
#define USXG_MODE_BIT_E             (12)
#define PCS_SGMII_MODE			    (0x2 << PCS_MODE_BIT)
#define PCS_QSGMII_MODE			    (0X3 << PCS_MODE_BIT)
#define MII_CTRL_BIT                (8)
#define MII_CTRL_8_BIT_MII          (1 << MII_CTRL_BIT)
#define MII_AN_INTR_EN			    (0x1)
#define TX_CONFIG                   (BIT(3))
#define VR_MII_CTRL_SGMII_AN_EN		(PCS_SGMII_MODE | MII_AN_INTR_EN)
#define VR_MII_CTRL_QSGMII_AN_EN	(PCS_QSGMII_MODE| MII_AN_INTR_EN)
#define VR_MII_CTRL_UXGMII_AN_EN	(MII_AN_INTR_EN)
#define AN_ENABLE  				   	BIT(12)
#define EN_2_5G_MODE                BIT(2)
#define MII_MAC_AUTO_SW			    (0x0200)
#define C37_TMR_OVR_RIDEA           BIT(3)
#define SR_XS_PCS_CTRL1_SPEED    	BIT(13)
#define	PCS_DIG_CTRL_USXG_EN       BIT(9)
#define AN_IND_TX_EN               BIT(9)
#define XPCS_SPEED_10				0
#define XPCS_SPEED_100				1
#define XPCS_SPEED_1000	    		2
#define XPCS_SPEED_10000			3
#define XPCS_SPEED_2500				4
#define XPCS_SPEED_5000				5
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
#define TX_CLK_RDY_0_BEGIN						12
#define TX_CLK_RDY_0_END						12
#define TX_CLK_RDY_BEGIN						12
#define TX_CLK_RDY_END						    15
#define TX_CLK_RDY_3_1_BEGIN					13
#define TX_CLK_RDY_3_1_END						15
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
#define USRA_RST_2_BEGIN						15
#define USRA_RST_2_END							15
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

#define SR_MII_OFFSET(_x)           ({ 				\
		typeof(_x) (x) = (_x); \
		(((x) == 0 ? 0 :  ((((x) - 1) * 4 *0x10000) + SR_MII_1_CTRL - SR_MII_CTRL))); \
		})
#define XPCS_FIELD(reg, field, value)   {reg, value, field##_END, field##_BEGIN}
#define CREATE_HSGMII_SERDES_CONFIG_(name)   {  \
	XPCS_FIELD(VR_##name##_MP_12G_16G_MPLLA_CTRL0, MPLLA_MULTIPLIER, 40), \
	XPCS_FIELD(VR_##name##_MP_16G_MPLLA_CTRL3, MPLLA_BANDWIDTH, 40983), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_VCO_CAL_LD0, VCO_LD_VAL_0, 1360), \
	XPCS_FIELD(VR_##name##_MP_16G_25G_VCO_CAL_REF0, VCO_REF_LD_0, 34), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_TX_RATE_CTRL, TX0_RATE, 2), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_RX_RATE_CTRL, RX0_RATE_1_0, 2), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_RX_EQ_CTRL4, CONT_ADAPT_0, 0), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_TX_GENCTRL2, TX0_WIDTH, 1), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_RX_GENCTRL2, RX0_WIDTH, 1), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_MPLLA_CTRL2, MPLLA_DIV16P5_CLK_EN, 0), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_MPLLA_CTRL2, MPLLA_DIV10_CLK_EN, 1), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_MPLLA_CTRL2, MPLLA_DIV8_CLK_EN, 0), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_TX_GENCTRL1, VBOOST_EN_0, 0), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_TX_GENCTRL1, TX_CLK_RDY_0, 0), \
	XPCS_FIELD(VR_##name##_MP_16G_25G_RX_EQ_CTRL0, CTLE_BOOST_0, 14), \
	XPCS_FIELD(VR_##name##_MP_16G_25G_RX_EQ_CTRL0, VGA1_GAIN_0, 4), \
	XPCS_FIELD(VR_##name##_MP_16G_25G_RX_EQ_CTRL0, VGA2_GAIN_0, 4), \
	XPCS_FIELD(VR_##name##_MP_16G_25G_RX_EQ_CTRL0, CTLE_POLE_0, 0), \
	XPCS_FIELD(VR_##name##_MP_16G_RX_CDR_CTRL1, VCO_STEP_CTRL_0, 1), \
	XPCS_FIELD(VR_##name##_MP_16G_RX_CDR_CTRL1, VCO_TEMP_COMP_EN_0, 1), \
	XPCS_FIELD(VR_##name##_MP_16G_RX_CDR_CTRL1, VCO_FRQBAND_0, 10), \
	XPCS_FIELD(VR_##name##_MP_16G_25G_RX_MISC_CTRL0, RX0_MISC, 23), \
	XPCS_FIELD(VR_##name##_MP_16G_25G_RX_GENCTRL4, RX_DFE_BYP_0, 1), \
	XPCS_FIELD(VR_##name##_MP_16G_25G_RX_IQ_CTRL0, RX0_DELTA_IQ, 0), \
	XPCS_FIELD(VR_##name##_MP_16G_25G_RX_EQ_CTRL5, RX0_ADPT_MODE, 0), \
	XPCS_FIELD(VR_##name##_MP_16G_25G_RX_EQ_CTRL5, RX_ADPT_PROG_0, 0), \
	XPCS_FIELD(VR_##name##_MP_16G_25G_RX_EQ_CTRL5, RX_ADPT_SEL_0, 0), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_REF_CLK_CTRL, REF_RPT_CLK_EN, 1), \
}

#define CREATE_SGMII_SERDES_CONFIG_(name)    {  \
	XPCS_FIELD(VR_##name##_MP_12G_16G_MPLLA_CTRL0, MPLLA_MULTIPLIER, 32), \
	XPCS_FIELD(VR_##name##_MP_16G_MPLLA_CTRL3, MPLLA_BANDWIDTH, 41022), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_VCO_CAL_LD0, VCO_LD_VAL_0, 1344), \
	XPCS_FIELD(VR_##name##_MP_16G_25G_VCO_CAL_REF0, VCO_REF_LD_0, 42), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_TX_RATE_CTRL, TX0_RATE, 3), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_RX_RATE_CTRL, RX0_RATE_1_0, 3), \
	\
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_RX_EQ_CTRL4, CONT_ADAPT_0, 0), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_TX_GENCTRL2, TX0_WIDTH, 1), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_RX_GENCTRL2, RX0_WIDTH, 1), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_MPLLA_CTRL2, MPLLA_DIV16P5_CLK_EN, 0), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_MPLLA_CTRL2, MPLLA_DIV10_CLK_EN, 1), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_MPLLA_CTRL2, MPLLA_DIV8_CLK_EN, 0), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_TX_GENCTRL1, VBOOST_EN_0, 0), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_TX_GENCTRL1, TX_CLK_RDY_0, 0), \
	XPCS_FIELD(VR_##name##_MP_16G_25G_RX_EQ_CTRL0, CTLE_BOOST_0, 14), \
	XPCS_FIELD(VR_##name##_MP_16G_25G_RX_EQ_CTRL0, VGA1_GAIN_0, 4), \
	XPCS_FIELD(VR_##name##_MP_16G_25G_RX_EQ_CTRL0, VGA2_GAIN_0, 4), \
	XPCS_FIELD(VR_##name##_MP_16G_25G_RX_EQ_CTRL0, CTLE_POLE_0, 0), \
	XPCS_FIELD(VR_##name##_MP_16G_RX_CDR_CTRL1, VCO_STEP_CTRL_0, 1), \
	XPCS_FIELD(VR_##name##_MP_16G_RX_CDR_CTRL1, VCO_TEMP_COMP_EN_0, 1), \
	\
	XPCS_FIELD(VR_##name##_MP_16G_RX_CDR_CTRL1, VCO_FRQBAND_0, 1), \
	XPCS_FIELD(VR_##name##_MP_16G_25G_RX_MISC_CTRL0, RX0_MISC, 22), \
	\
	XPCS_FIELD(VR_##name##_MP_16G_25G_RX_GENCTRL4, RX_DFE_BYP_0, 1), \
	XPCS_FIELD(VR_##name##_MP_16G_25G_RX_IQ_CTRL0, RX0_DELTA_IQ, 0), \
	XPCS_FIELD(VR_##name##_MP_16G_25G_RX_EQ_CTRL5, RX0_ADPT_MODE, 0), \
	XPCS_FIELD(VR_##name##_MP_16G_25G_RX_EQ_CTRL5, RX_ADPT_PROG_0, 0), \
	XPCS_FIELD(VR_##name##_MP_16G_25G_RX_EQ_CTRL5, RX_ADPT_SEL_0, 0), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_REF_CLK_CTRL, REF_RPT_CLK_EN, 1), \
}

#define CREATE_SGMII_FFE_SERDES_CONFIG_(name)  {  \
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_TX_GENCTRL1, TX_CLK_RDY_0, 1), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_TX_EQ_CTRL0, TX_EQ_MAIN, 40), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_TX_EQ_CTRL0, TX_EQ_PRE, 0), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_TX_EQ_CTRL1, TX_EQ_POST_1, 0), \
	XPCS_FIELD(VR_XS_PCS_DEBUG_CTRL, SUPRESS_LOS_DET, 0), \
	XPCS_FIELD(VR_XS_PCS_DEBUG_CTRL, RX_DT_EN_CTL, 0), \
}

#define CREATE_HSGMII_FFE_SERDES_CONFIG_(name)  {  \
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_TX_GENCTRL1, TX_CLK_RDY_0, 1), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_TX_EQ_CTRL0, TX_EQ_MAIN, 40), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_TX_EQ_CTRL0, TX_EQ_PRE, 0), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_TX_EQ_CTRL1, TX_EQ_POST_1, 0), \
	XPCS_FIELD(VR_XS_PCS_DEBUG_CTRL, SUPRESS_LOS_DET, 0), \
	XPCS_FIELD(VR_XS_PCS_DEBUG_CTRL, RX_DT_EN_CTL, 0), \
}

#define CREATE_QSGMII_SERDES_CONFIG_(name)   {  \
	XPCS_FIELD(VR_##name##_MP_12G_16G_MPLLA_CTRL0, MPLLA_MULTIPLIER, 32), \
	XPCS_FIELD(VR_##name##_MP_16G_MPLLA_CTRL3, MPLLA_BANDWIDTH, 41013), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_VCO_CAL_LD0, VCO_LD_VAL_0, 1344), \
	XPCS_FIELD(VR_##name##_MP_16G_25G_VCO_CAL_REF0, VCO_REF_LD_0, 42), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_RX_EQ_CTRL4, CONT_ADAPT_0, 0), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_TX_RATE_CTRL, TX0_RATE, 1), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_RX_RATE_CTRL, RX0_RATE_1_0, 1), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_TX_GENCTRL2, TX0_WIDTH, 3), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_RX_GENCTRL2, RX0_WIDTH, 3), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_MPLLA_CTRL2, MPLLA_DIV16P5_CLK_EN, 0), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_MPLLA_CTRL2, MPLLA_DIV10_CLK_EN, 1), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_MPLLA_CTRL2, MPLLA_DIV8_CLK_EN, 0), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_TX_GENCTRL1, VBOOST_EN_0, 0), \
	XPCS_FIELD(VR_##name##_MP_16G_25G_RX_EQ_CTRL0, CTLE_BOOST_0, 14), \
	XPCS_FIELD(VR_##name##_MP_16G_25G_RX_EQ_CTRL0, VGA1_GAIN_0, 4), \
	XPCS_FIELD(VR_##name##_MP_16G_25G_RX_EQ_CTRL0, VGA2_GAIN_0, 4), \
	XPCS_FIELD(VR_##name##_MP_16G_25G_RX_EQ_CTRL0, CTLE_POLE_0, 0), \
	XPCS_FIELD(VR_##name##_MP_16G_RX_CDR_CTRL1, VCO_STEP_CTRL_0, 1), \
	XPCS_FIELD(VR_##name##_MP_16G_RX_CDR_CTRL1, VCO_TEMP_COMP_EN_0, 1), \
	XPCS_FIELD(VR_##name##_MP_16G_RX_CDR_CTRL1, VCO_FRQBAND_0, 1), \
	XPCS_FIELD(VR_##name##_MP_16G_25G_RX_MISC_CTRL0, RX0_MISC, 20), \
	XPCS_FIELD(VR_##name##_MP_16G_25G_RX_GENCTRL4, RX_DFE_BYP_0, 0), \
	XPCS_FIELD(VR_##name##_MP_16G_25G_RX_IQ_CTRL0, RX0_DELTA_IQ, 0), \
	XPCS_FIELD(VR_##name##_MP_16G_25G_RX_EQ_CTRL5, RX0_ADPT_MODE, 0), \
	XPCS_FIELD(VR_##name##_MP_16G_25G_RX_EQ_CTRL5, RX_ADPT_PROG_0, 0), \
	XPCS_FIELD(VR_##name##_MP_16G_25G_RX_EQ_CTRL5, RX_ADPT_SEL_0, 0), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_TX_GENCTRL1, TX_CLK_RDY_0, 0), \
}
#define CREATE_QSGMII_FFE_SERDES_CONFIG_(name)  {  \
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_TX_GENCTRL1, TX_CLK_RDY, 3), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_TX_EQ_CTRL0, TX_EQ_MAIN, 36), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_TX_EQ_CTRL0, TX_EQ_PRE, 0), \
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_TX_EQ_CTRL1, TX_EQ_POST_2, 0), \
}

#define CREATE_USXGMII_SERDES_CONFIG_(name, data_rate)   {  \
	XPCS_FIELD(VR_##name##_MP_12G_16G_MPLLA_CTRL0, MPLLA_MULTIPLIER, 33),\
	XPCS_FIELD(VR_##name##_MP_16G_MPLLA_CTRL3, MPLLA_BANDWIDTH, 41022),\
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_VCO_CAL_LD0, VCO_LD_VAL_0, 1353),\
	XPCS_FIELD(VR_##name##_MP_16G_25G_VCO_CAL_REF0, VCO_REF_LD_0, 41),\
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_RX_EQ_CTRL4, CONT_ADAPT_0, 1),\
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_TX_RATE_CTRL, TX0_RATE, data_rate),\
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_RX_RATE_CTRL, RX0_RATE_2_0, data_rate),\
	XPCS_FIELD(VR_##name##_MP_12G_16G_TX_GENCTRL2, TX0_WIDTH, 3),\
	XPCS_FIELD(VR_##name##_MP_12G_16G_RX_GENCTRL2, RX0_WIDTH, 3),\
	XPCS_FIELD(VR_##name##_MP_12G_16G_MPLLA_CTRL2, MPLLA_DIV16P5_CLK_EN, 1),\
	XPCS_FIELD(VR_##name##_MP_12G_16G_MPLLA_CTRL2, MPLLA_DIV10_CLK_EN, 1),\
	XPCS_FIELD(VR_##name##_MP_12G_16G_MPLLA_CTRL2, MPLLA_DIV8_CLK_EN, 0),\
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_TX_GENCTRL1, TX_CLK_RDY_0, 1),\
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_TX_GENCTRL1, VBOOST_EN_0, 1),\
	XPCS_FIELD(VR_##name##_MP_16G_25G_RX_EQ_CTRL0, VGA1_GAIN_0, 5),\
	XPCS_FIELD(VR_##name##_MP_16G_25G_RX_EQ_CTRL0, VGA2_GAIN_0, 5),\
	XPCS_FIELD(VR_##name##_MP_16G_25G_RX_EQ_CTRL0, CTLE_POLE_0, 2),\
	XPCS_FIELD(VR_##name##_MP_16G_25G_RX_EQ_CTRL0, CTLE_BOOST_0, 16),\
\
	XPCS_FIELD(VR_##name##_MP_16G_RX_CDR_CTRL1, VCO_FRQBAND_0, 1),\
	XPCS_FIELD(VR_##name##_MP_16G_RX_CDR_CTRL1, VCO_STEP_CTRL_0, 1),\
	XPCS_FIELD(VR_##name##_MP_16G_RX_CDR_CTRL1, VCO_TEMP_COMP_EN_0, 1),\
	XPCS_FIELD(VR_##name##_MP_16G_25G_RX_MISC_CTRL0, RX0_MISC, 18),\
	XPCS_FIELD(VR_##name##_MP_16G_25G_RX_GENCTRL4, RX_DFE_BYP_0, 0),\
	XPCS_FIELD(VR_##name##_MP_16G_RX_CDR_CTRL1, RX0_DELTA_IQ, 0),\
	XPCS_FIELD(VR_##name##_MP_16G_25G_RX_EQ_CTRL5, RX0_ADPT_MODE, 3),\
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_TX_GENCTRL1, TX_CLK_RDY_0, 0),\
}

#define CREATE_USXGMII_FFE_SERDES_CONFIG_(name)  {  \
	XPCS_FIELD(VR_##name##_MP_12G_16G_25G_TX_GENCTRL1, TX_CLK_RDY_0, 1), \
}

#define CREATE_SERDES_CONFIG_0(mode,...)    CREATE_##mode##_SERDES_CONFIG_(XS_PMA, ##__VA_ARGS__)
#define CREATE_SERDES_CONFIG_1(mode,...)    CREATE_##mode##_SERDES_CONFIG_(XS_PMA, ##__VA_ARGS__)
#define CREATE_SERDES_CONFIG(id, mode,...)  CREATE_SERDES_CONFIG_##id(mode, ##__VA_ARGS__)

#define DO_SERDES_CONFIG_(xpcs_priv, id, name, mode, ...) do { \
	int i = 0;\
	u32 tmp; \
	serdes_reg_ext_st serdes_config[] = CREATE_SERDES_CONFIG(id, mode, ##__VA_ARGS__);\
	serdes_reg_ext_st serdes_ffe[] = CREATE_SERDES_CONFIG(id, mode##_FFE); \
	for (i = 0; \
			i < sizeof(serdes_config) / sizeof(*serdes_config); \
			i++) { \
		xpcs_write_ext(xpcs_priv, \
				serdes_config[i].reg, \
				serdes_config[i].field_end, \
				serdes_config[i].field_begin, \
				serdes_config[i].value); \
	} \
	xpcs_write_ext(xpcs_priv, VR_XS_PCS_DIG_CTRL1, \
			VR_RST_END, \
			VR_RST_BEGIN, \
			1); \
	\
	cls_serdes_load_and_modify(xpcs_priv); \
	\
	if (readl_poll_timeout(xpcs_priv->ioaddr + VR_XS_PCS_DIG_CTRL1, \
				tmp, !(tmp & BIT(15)) ,10000)) { \
		printf("Line %d: wait status[%d] timeout\n", __LINE__, tmp); \
	} \
	for (i = 0; \
			i < sizeof(serdes_ffe) / sizeof(*serdes_ffe); \
			i++) { \
		xpcs_write_ext(xpcs_priv, \
				serdes_ffe[i].reg, \
				serdes_ffe[i].field_end, \
				serdes_ffe[i].field_begin, \
				serdes_ffe[i].value); \
	} \
} while(0)


#define DO_SERDES_CONFIG_0(xpcs_priv, mode, ...)        DO_SERDES_CONFIG_(xpcs_priv, 0, XS_PMA, mode, ##__VA_ARGS__)
#define DO_SERDES_CONFIG_1(xpcs_priv, mode, ...)        DO_SERDES_CONFIG_(xpcs_priv, 1, XS_PMA, mode, ##__VA_ARGS__)
#define DO_SERDES_CONFIG(xpcs_priv, id, mode, ...)      DO_SERDES_CONFIG_##id(xpcs_priv, mode, ##__VA_ARGS__)

typedef struct {
	uint32_t reg;
	uint32_t value;
    uint16_t field_end;
    uint16_t field_begin;
} serdes_reg_ext_st;

#endif
