#ifndef _CLS_NPE_FWD_H_
#define _CLS_NPE_FWD_H_

#define  BOARD_HW_VER_ADDR 		 0x90444404
#define  FWD_CLK_PARA_REG    	 0x9041620c
#define  FWD_SUB_CG_PARA_REG     0x9044304c
#define  FWD_APB_SUB_RST_REG     0x90442830
#define  XGMAC_PHY_INTF_SEL_REG  0x53E0000C
#define  XGMAC_PHY_INTF_EN_REG   0x53E001e4
#define  DIV_RST_PARA_REG        0x90442844
#define  XGMAC_TOP_REG_REG       0x53E00000
#define  TEN_G_MODE_SEL          (0xD0)
#define  MDIO_SEL_OFFSET(index)  (0x284 + ((index) * 4))
#define FWD_TOP_CLK_SEL_PARA                    (0x9041620c)
#define FWD_TOP_PLL_PARA0                       (0x90416002)
#define FWD_TOP_PLL_PARA1					    (0x90416004)
#define FWD_TOP_PLL_PARA2						(0x90416008)
#define FWD_TOP_PLL_PARA3                       (0x9041600c)
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
#define PLL_PARA_0_BEGIN                     (0)
#define PLL_PARA_0_END                       (0)
#define CLK_SEL_PARA_BEGIN					 (0)
#define CLK_SEL_PARA_END					 (1)
#define XGMAC_PHY_RGMII_RX_DELAY(id)  (id == 0 ? 0x53E002CC: \
									  (id == 4 ? 0x53E002D4: \
									  (id == 2 ? 0x53E002CC : 0x53E002CC)))

#define XGMAC_PHY_RGMII_TX_DELAY(id)  (id == 0 ? 0x53E002C8: \
									  (id == 4 ? 0x53E002D0: \
									  (id == 2 ? 0x53E002C8: 0x53E002C8)))

#define XGMAC_RGMII_IO_SPEED_ADDR(id)  (id == 0 ? 0x9046004c: \
									   (id == 4 ? 0x9041006c: \
									   (id == 2 ? 0x90420098: 0x9046004c)))

#endif /* _CLS_NPE_FWD_H_ */
