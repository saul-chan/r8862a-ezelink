/*
 * Copyright (c) 2021-2024, Clourney Semiconductor. All rights reserved.
 *
 * This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
 * Reproduction and redistribution in binary or source form, with or without modification,
 * for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
 * must retain the above copyright notice.
 *
 * By using this software and/or documentation, you agree to the limited terms and conditions.
 */

#ifndef _DUBHE1000_FWD_H_
#define _DUBHE1000_FWD_H_

#include <linux/stddef.h>
#include <linux/module.h>
#include <linux/types.h>
#include <asm/io.h>
#include <linux/delay.h>
#include "dubhe2000.h"
//fix me
#define FWD_CLK_SEL_VAL	      0x1
#define DIV_DEASSERT_VAL      0x7
#define FWD_SUB_CLK_EN_VAL    0xf
#define FWD_ASSERT_VAL	      0x0
#define FWD_DEASSERT_CFG_VAL  0xA
#define FWD_DEASSERT_FULL_VAL 0xF
#define FWD_HOLD_TIME	      10

#define FWD_TOP_PLL_PARA3_REG_OFFSET 0xC
#define FWD_CLK_PARA_REG_OFFSET	     0x20C
#define DIV_RST_PARA_REG_OFFSET	     0x44
#define FWD_SUB_CG_PARA_REG_OFFSET   0x4C
#define FWD_APB_SUB_RST_REG_OFFSET   0x30

#define XGMAC_PHY_INTF_EN_REG				   0x01e4
#define NPE_DUBHE1000_ETH_INTERFACE_OFFSET(adapter, index) (adapter->top_regs + (0xC + (0x10 * index)))

#define TEN_G_MODE_SEL			0xD0
#define MDIO_SEL_OFFSET(adapter, index) (adapter->top_regs + (0x284 + ((index)*4)))

#define XGMAC_PHY_INTF_EN_REG_OFFSET(adapter, id)                                                                      \
	(adapter->top_regs + (id == 0 ? 0x01e4 : (id == 4 ? 0x01e8 : (id == 2 ? 0x01ec : 0x01e4))))

#define XGMAC_PHY_RGMII_RX_DELAY(adapter, id)                                                                          \
	(adapter->top_regs + (id == 0 ? 0x2CC : (id == 4 ? 0x2D4 : (id == 2 ? 0x2DC : 0x2CC))))

#define XGMAC_PHY_RGMII_TX_DELAY(adapter, id)                                                                          \
	(id == 0 ? adapter->io_dr_regs + 0x3C :                                                                        \
		   (id == 4 ? adapter->io_right_regs + 0x068 :                                                         \
			      (id == 2 ? adapter->io_left_regs + 0x094 : adapter->io_dr_regs + 0x03C)))

#define XGMAC_RGMII_IO_SPEED_ADDR(adapter, id)                                                                         \
	(id == 0 ? adapter->io_dr_regs + 0x40 :                                                                        \
		   (id == 4 ? adapter->io_right_regs + 0x06c :                                                         \
			      (id == 2 ? adapter->io_left_regs + 0x098 : adapter->io_dr_regs + 0x040)))

void dubhe1000_soft_reset_fwd(struct dubhe1000_adapter *adapter);
#endif /* _DUBHE1000_FWD_H_ */
