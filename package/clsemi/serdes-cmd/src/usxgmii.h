REG_W(XGMAC0_SFT_RST_N, xgmac0_sft_rst_n, 0);
REG_W(XGMAC1_SFT_RST_N, xgmac1_sft_rst_n, 0);
REG_W(XGMAC2_SFT_RST_N, xgmac2_sft_rst_n, 0);		
REG_W(XGMAC3_SFT_RST_N, xgmac3_sft_rst_n, 0);

switch(usxg_mode)
{
case USXGMII_10G_SXGMII_EM:
case USXGMII_10G_DXGMII_EM:
case USXGMII_10G_QXGMII_EM:
	data_rate = 0;
	break;
case USXGMII_5G_SXGMII_EM:
case USXGMII_5G_DXGMII_EM:
	data_rate = 1;
	break;
case USXGMII_2_5G_SXGMII_EM:
	data_rate = 2;
	break;
default:
printf("Invalid Serdes%d USXGMII mode.\n", xpcs_num);
return;
}

REG_W(XPCS_BASE_ADDR(xpcs_num) + SR_XS_PCS_CTRL2, PCS_TYPE_SEL, 0);			//Select 10GBASE-R PCS Type
REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, USXG_EN, 1);				//Enable USXGMII mode
REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_KR_CTRL, USXG_2PT5G_GMII, 0);			//Use XGMII interface
REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_KR_CTRL, USXG_MODE, usxg_mode);		//USXGMII mode
REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_MPLLA_CTRL0, MPLLA_MULTIPLIER, 33);
REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_MPLLA_CTRL3, MPLLA_BANDWIDTH, 41022);
REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_VCO_CAL_LD0, VCO_LD_VAL_0, 1353);
REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_VCO_CAL_REF0, VCO_REF_LD_0, 41);
REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_RX_EQ_CTRL4, CONT_ADAPT_0, 1);
REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_RATE_CTRL, TX0_RATE, data_rate);		//确认下data_rate值
REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_RX_RATE_CTRL, RX0_RATE_1_0, data_rate);		//确认下data_rate值
REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_TX_GENCTRL2, TX0_WIDTH, 3);
REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_RX_GENCTRL2, RX0_WIDTH, 3);
REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_MPLLA_CTRL2, MPLLA_DIV16P5_CLK_EN, 1);	//使能DIV16.5
REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_MPLLA_CTRL2, MPLLA_DIV10_CLK_EN, 1);		//使能DIV10
REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_MPLLA_CTRL2, MPLLA_DIV8_CLK_EN, 0);		//关闭DIV8
REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_GENCTRL1, TX_CLK_RDY_0, 1);	//databook流程里面没有
REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_GENCTRL1, VBOOST_EN_0, 1);
REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0, VGA1_GAIN_0, 5);
REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0, VGA2_GAIN_0, 5);
REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0, CTLE_POLE_0, 2);
REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL0, CTLE_BOOST_0, 16);

REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_RX_CDR_CTRL1, VCO_FRQBAND_0, 1);
REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_RX_CDR_CTRL1, VCO_STEP_CTRL_0, 1);
REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_RX_CDR_CTRL1, VCO_TEMP_COMP_EN_0, 1);
REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_MISC_CTRL0, RX0_MISC, 18);
REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_GENCTRL4, RX_DFE_BYP_0, 0);
REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_IQ_CTRL0, RX0_DELTA_IQ, 0);
REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_16G_25G_RX_EQ_CTRL5, RX0_ADPT_MODE, 3);
REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_GENCTRL1, TX_CLK_RDY_0, 0);	//databook有，仿真代码没

VR_RST(xpcs_num);

REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_MP_12G_16G_25G_TX_GENCTRL1, TX_CLK_RDY_0, 1);
//REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DEBUG_CTRL, //RX_DT_EN_CTL, 1);			//不支持10Base-(K)R mode不配置
//REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DEBUG_CTRL, //SUPRESS_LOS_DET, 1);		//不支持10Base-(K)R mode不配置

LOG("等待DPLL Lock，1表示lock\n");
status = 0;
if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_RX_LSTS, RX_VALID_0,
			status, status != 0, DEFAULT_TIMEOUT_US)) {
	printf("Line %d: wait status[%d] timeout", __LINE__, status);
	return;
}
#if 0
while (status == 0) {											//等待DPLL Lock，1表示lock
	status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PMA_RX_LSTS, RX_VALID_0);
}
#endif
if (rxadpt_en) {
	RX_Adapt_enable(xpcs_num, 0);
}
REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_CTRL, MII_CTRL, 0);				//4-bit MII
//REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_CTRL, SGMII_LINK_STS, 1);		//Link Up, PHY Side时配置
REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_CTRL, TX_CONFIG, 0);			//MAC Side
REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_CTRL, MII_AN_INTR_EN, 1);		//CL37 AN Complete Interrupt Enable
//!!! BIT 9  AUTOSW
//(Option)Duration=CL37_LINK_TIME*6.4ns
//#define   CL37_LINK_TIME_VAL        0  
//REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_LINK_TIMER_CTRL, CL37_LINK_TIME, CL37_LINK_TIME_VAL);
//REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, CL37_TMR_OVR_RIDE, 1);
/*
//enable auto-negotiation时不配置
//当不支持auto-negotiation时，需要获取对端设配的速率配置
if (usxg_mode == USXGMII_10G_SXGMII_EM) {		//10G-SXGMII
REG_W(XPCS_BASE_ADDR(xpcs_num) + SR_MII_CTRL, SS13, 1);
REG_W(XPCS_BASE_ADDR(xpcs_num) + SR_MII_CTRL, SS6, 1);
REG_W(XPCS_BASE_ADDR(xpcs_num) + SR_MII_CTRL, SS3, 0);
}
else if (usxg_mode == USXGMII_5G_SXGMII_EM) {		//5G-SXGMII
REG_W(XPCS_BASE_ADDR(xpcs_num) + SR_MII_CTRL, SS13, 1);
REG_W(XPCS_BASE_ADDR(xpcs_num) + SR_MII_CTRL, SS6, 0);
REG_W(XPCS_BASE_ADDR(xpcs_num) + SR_MII_CTRL, SS3, 1);
}
else if (usxg_mode == USXGMII_2_5G_SXGMII_EM) {		//2.5G-SXGMII
REG_W(XPCS_BASE_ADDR(xpcs_num) + SR_MII_CTRL, SS13, 0);
REG_W(XPCS_BASE_ADDR(xpcs_num) + SR_MII_CTRL, SS6, 0);
REG_W(XPCS_BASE_ADDR(xpcs_num) + SR_MII_CTRL, SS3, 1);
}
...
*/

//配置auto-negotiation
REG_W(XPCS_BASE_ADDR(xpcs_num) + SR_MII_CTRL, AN_ENABLE, 1);

LOG("CL37 AN Complete Interrupt, 1代表完成\n");
status = 0;
if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_INTR_STS, CL37_ANCMPLT_INTR,
			status, status != 0, DEFAULT_TIMEOUT_US)) {
	printf("Line %d: wait status[%d] timeout", __LINE__, status);
	return;
}
#if 0
while (status == 0) {						//CL37 AN Complete Interrupt, 1代表完成
	status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_INTR_STS, CL37_ANCMPLT_INTR);
}
#endif		
REG_W(XPCS_BASE_ADDR(xpcs_num) +  VR_MII_AN_INTR_STS, CL37_ANCMPLT_INTR, 0);

//Read REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_INTR_STS, USXG_AN_STS);
link_status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_INTR_STS, USXG_AN_STS_14);		//0---Link is Down; 1---Link is Up
duplex_mode = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_INTR_STS, USXG_AN_STS_13);		//0---Half Duplex; 1---Full Duplex
link_speed = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_INTR_STS, USXG_AN_STS_12_10);		//000:10M;001:100M;010:1000M;011: 10G;100:2.5G;101:5G
eee = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_INTR_STS, USXG_AN_STS_9);				//0:EEE not support; 1:EEE support
eee_clk = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_INTR_STS, USXG_AN_STS_8);			//0:EEE clock stop not support; 1:EEE clock-stop support

if (link_status == 1)
	printf("USXGMII Link Up. \n");
	else
	printf("USXGMII Link Down. \n");
if (duplex_mode == 1)
	printf("Full Duplex. \n");
	else
	printf("Half Duplex. \n");
if (link_speed == 0)
	printf("USXGMII Link Speed: 10Mbps. \n");
else if (link_speed == 1)
	printf("USXGMII Link Speed: 100Mbps. \n");
else if (link_speed == 2)
	printf("USXGMII Link Speed: 1000Mbps. \n");
else if (link_speed == 3)
	printf("USXGMII Link Speed: 10Gbps. \n");
else if (link_speed == 4)
	printf("USXGMII Link Speed: 2.5Gbps. \n");
else if (link_speed == 5)
	printf("USXGMII Link Speed: 5Gbps. \n");
	else
	printf("USXGMII Link Speed: Invalid. \n");
if (eee == 1)
	printf("EEE support. \n");
	else
	printf("EEE not support. \n");
if (eee_clk == 1)
	printf("EEE clock-stop support. \n");
	else
	printf("EEE clock-stop not support. \n");

	REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_AN_INTR_STS, CL37_ANCMPLT_INTR, 0);		//Clear interrupt
	//wait 1us;												//等待XGMII clock0稳定
	sleep_nano(1 * 1000);
	REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, USRA_RST_1, 1);				//port0
	//REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, USXG_EN, 1);					//仿真上冗余

	LOG("Wait for USRA_RST_1 Set 0\n");
	status = 1;
	if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, USRA_RST_1,
				status, status != 1, DEFAULT_TIMEOUT_US)) {
		printf("Line %d: wait status[%d] timeout", __LINE__, status);
		return;
	}
#if 0
while (status == 1) {						//
	status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_XS_PCS_DIG_CTRL1, USRA_RST_1);
}
#endif
if ((usxg_mode == USXGMII_10G_DXGMII_EM) || (usxg_mode == USXGMII_5G_DXGMII_EM)) {
	//REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_1_DIG_CTRL1, MAC_AUTO_SW, 1);			//databook没有
	REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_1_DIG_CTRL1, USRA_RST_2, 1);
	LOG("Wait for USRA_RST_2 Set 0\n");
	status = 1;
	if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_MII_1_DIG_CTRL1, USRA_RST_2,
				status, status != 1, DEFAULT_TIMEOUT_US)) {
		printf("Line %d: wait status[%d] timeout", __LINE__, status);
		return;
	}
#if 0
	while (status == 1) {						//
		status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_MII_1_DIG_CTRL1, USRA_RST_2);
	}
#endif
}

if (usxg_mode == USXGMII_10G_QXGMII_EM) {
	//REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_1_DIG_CTRL1, MAC_AUTO_SW, 1);			//databook没有
	REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_1_DIG_CTRL1, USRA_RST_2, 1);
	REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_2_DIG_CTRL1, USRA_RST_2, 1);
	REG_W(XPCS_BASE_ADDR(xpcs_num) + VR_MII_3_DIG_CTRL1, USRA_RST_2, 1);

	LOG("Wait for Port1 USRA_RST_2 Set 0\n");
	status = 1;
	if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_MII_1_DIG_CTRL1, USRA_RST_2,
				status, status != 1, DEFAULT_TIMEOUT_US)) {
		printf("Line %d: wait status[%d] timeout", __LINE__, status);
		return;
	}
#if 0
	while (status == 1) {						//
		status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_MII_1_DIG_CTRL1, USRA_RST_2);
	}
#endif

	LOG("Wait for Port2 USRA_RST_2 Set 0\n");
	status = 1;
	if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_MII_2_DIG_CTRL1, USRA_RST_2,
				status, status != 1, DEFAULT_TIMEOUT_US)) {
		printf("Line %d: wait status[%d] timeout", __LINE__, status);
		return;
	}
#if 0
	while (status == 1) {						//
		status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_MII_2_DIG_CTRL1, USRA_RST_2);
	}
#endif

	LOG("Wait for Port3 USRA_RST_2 Set 0\n");
	status = 1;
	if (reg_poll_timeout(XPCS_BASE_ADDR(xpcs_num) + VR_MII_3_DIG_CTRL1, USRA_RST_2, status, status != 1, DEFAULT_TIMEOUT_US)) {
		printf("Line %d: wait status[%d] timeout", __LINE__, status);
		return;
	}
#if 0
	while (status == 1) {
		status = REG_R(XPCS_BASE_ADDR(xpcs_num) + VR_MII_3_DIG_CTRL1, USRA_RST_2);
	}
#endif
}

if (serdes_mode == USXGMII_EM)
	REG_W(XGMAC0_SFT_RST_N, xgmac0_sft_rst_n, 1);
	else if (serdes_mode == MP_USXGMII_EM ) {
		REG_W(XGMAC0_SFT_RST_N, xgmac0_sft_rst_n, 1);
		REG_W(XGMAC1_SFT_RST_N, xgmac1_sft_rst_n, 1);
		//REG_W(XGMAC2_SFT_RST_N, xgmac2_sft_rst_n, 1);		//Dubhe1000不支持，Dubhe2000支持
		//REG_W(XGMAC3_SFT_RST_N, xgmac3_sft_rst_n, 1);		//Dubhe1000不支持，Dubhe2000支持
}
