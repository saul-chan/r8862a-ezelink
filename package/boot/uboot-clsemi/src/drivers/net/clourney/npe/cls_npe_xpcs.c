#include <common.h>
#include <command.h>
#include <net.h>
#include <netdev.h>
#include <miiphy.h>
#include <phy.h>
#include <asm/io.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include "cls_npe.h"
#include "cls_npe_xpcs_serdes_regs.h"

#define xpcs_read(address,reg)  __xpcs_read(address, reg)
#define xpcs_write(address,reg,value)   __xpcs_write(address, reg, value)
//#define xpcs_read(address,reg) (pr_info("xpcs_read reg [%s] \n", #reg), __xpcs_read(address, reg))
//#define xpcs_write(address,reg,value) pr_info("xpcs_write reg [%s] value[%s] \n", #reg ,#value);  __xpcs_write(address, reg, value)

#define XPCS_WRITE_EXT(priv, reg, field, value)  xpcs_write_ext(priv, reg, field##_END, field##_BEGIN, value)
#define XPCS_READ_EXT(priv, reg, field)  (xpcs_read(priv, reg) & GENMASK(field##_END, field##_BEGIN))
typedef struct {
	uint32_t reg;
	uint32_t value;
} serdes_reg_st;


typedef struct {
	union {
		u32  raw;
		struct {
			u32  CL37_ANCMP_INTR:1,
				 CL37_ANSGM_STS_DUPLEX:1,
				 CL37_ANSGM_STS_SPEED:2,
				 CL37_ANSGM_STS_LINK:1,
				 LP_EEE_CAP:1,
				 LP_CK_STP:1,
				 Reserved_7:1,
				 USXG_AN_STS_EEE:2,
				 USXG_AN_STS_SPEED:3,
				 USXG_AN_STS_DUPLEX:1,
				 USXG_AN_STS_LINK:1,
				 Reserved_15:1;
		};
	};
} vr_mii_an_intr_sts_t;

enum {
	USXG_MODE_10G_SXGMII 	=   (0),
	USXG_MODE_5G_SXGMII 	=   (1),
	USXG_MODE_2_5G_SXGMII 	=   (2),
	USXG_MODE_10G_DXGMII  	=   (3),
	USXG_MODE_5G_DXGMII  	=   (4),
	USXG_MODE_10G_QXGMII 	=   (5),
	USXG_MODE_MAX_NUM     	=   (6)
};

static u32 __xpcs_read(void *priv, int reg)
{
	struct cls_xpcs_priv *xpcs_priv = (struct cls_xpcs_priv *)priv;
	u32	val = 0;

	val = readl(xpcs_priv->ioaddr + reg);
   if(g_debug_cls) {
		printf("==) addr[%#x] value[%#x]\n",xpcs_priv->ioaddr + reg, val);
	}
	return val;
}

static int __xpcs_write(void *priv, int reg, u32 value)
{
	struct cls_xpcs_priv *xpcs_priv = (struct cls_xpcs_priv *)priv;

	if(g_debug_cls) {
		printf("==) addr[%#x] value[%#x]\n",xpcs_priv->ioaddr + reg, value);
	}

	writel(value, xpcs_priv->ioaddr + reg);

	return 0;
}

static int xpcs_write_ext(void *priv, int reg, int field_end, int field_begin, int value)
{
	uint32_t __val = __xpcs_read(priv, reg);
	__val &= ~GENMASK(field_end, field_begin);
	__val |= (value << (field_begin)) & GENMASK(field_end, field_begin);
	__xpcs_write(priv, reg, __val);
	return 0;
}

static void cls_set_to_rgmii(struct cls_xpcs_priv *xpcs_priv)
{

}

void cls_serdes_load_and_modify(struct cls_xpcs_priv *xpcs_priv)
{
	int status = 0;

	writel(0, xpcs_priv->id == 0 ? SERDES0_SRAM_BYPASS : SERDES1_SRAM_BYPASS);		//加载serdes FW使能
	mdelay(1);
	if (g_debug_cls)
		printf("Wait for XPCS%d SRAM initialization to complete ...\n", xpcs_priv->id);
	status = 0;
	if (readl_poll_timeout(xpcs_priv->ioaddr + VR_XS_PMA_MP_12G_16G_25G_SRAM,
				status, (status & BIT(0)), 10000)) {
		printf("Line %d: wait status[%d] timeout\n", __LINE__, status);
	}

	//该等待期间可修改加载到SRAM的firmware
	mdelay(1);

	xpcs_bypass_serdes_ack(xpcs_priv);

	mdelay(1);

	writel(3, xpcs_priv->ioaddr + VR_XS_PMA_MP_12G_16G_25G_SRAM);		//指示Serdes0 SRAM外部加载完成

	if (g_debug_cls)
		printf("Indicates that Serdes%d external loading is complete ...\n", xpcs_priv->id);
	status = 0;
	if (readl_poll_timeout(xpcs_priv->ioaddr + SR_XS_PCS_CTRL1,
				status, !(status & BIT(15)), 10000)) {
		printf("Line %d: wait status[%d] timeout\n", __LINE__, status);
	}

	return;
}

static void cls_set_to_qsgmii(struct cls_xpcs_priv *xpcs_priv)
{
	int i;
	u32 val = 0;
	u32 tmp;

	if (xpcs_priv->id != 1) {
		printf("QSGMII Only support on XPCS1\n");
		return;
	}

	//1. Write 4'b0001 to Bits [3:0] of SR_XS_PCS_CTRL2 Register (Optional- Required only if the configuration has KR/10GBASE-R support).
	val = xpcs_read(xpcs_priv, SR_XS_PCS_CTRL2);
	val &= ~GENMASK(PCS_TYPE_SEL_BIT_E, PCS_TYPE_SEL_BIT);
	val |= PCS_TYPE_SEL_10GBX;
	xpcs_write(xpcs_priv, SR_XS_PCS_CTRL2,
			val);

	//2. Clear Bit[13] of SR_PMA_CTRL1 Register (only for Backplane Ethernet PCS configurations) or SR_XS_PCS_CTRL1 Register(for other configuration) to 0.
	val = xpcs_read(xpcs_priv, SR_XS_PCS_CTRL1);
	xpcs_write(xpcs_priv, SR_XS_PCS_CTRL1,
			val & ~SR_XS_PCS_CTRL1_SPEED);

	//3. Program bits [2:1] of VR_MII_AN_CTRL Register to 2'b11.this asserts xgxs_qsgmii_mode_o'port of DWC_xpcs.
	//  PCS_MODE to 2'b11 qsgmii
	//  MII_AN_INTR_EN to 1, to Enable Auto-negotiation complete interrupt(optional step)
	//  MII_CTRL to 0 or 1, based on your MAC capability.
	//  TX_CONFIG to 1 (PHY side USXGMII) or 0 (MAC side USXGMII) based on your requirement
	//  If TX_CONFIG is set to 1, program SGMII_LINK_STS bit to suitable value to indicate the link status to the MAC side if USXGMII link.
	for (i = 0; i < 4; i++) {

		val = xpcs_read(xpcs_priv,
				SR_MII_OFFSET(i) + SR_MII_CTRL);
		val = xpcs_write(xpcs_priv,
				SR_MII_OFFSET(i) + SR_MII_CTRL,
				val & ~AN_ENABLE);

		xpcs_write(xpcs_priv, SR_MII_OFFSET(i) + VR_MII_AN_CTRL,
				VR_MII_CTRL_QSGMII_AN_EN);

		val = xpcs_read(xpcs_priv,
				SR_MII_OFFSET(i) + VR_MII_DIG_CTRL1);
		xpcs_write(xpcs_priv,
				SR_MII_OFFSET(i) + VR_MII_DIG_CTRL1,
				val | MII_MAC_AUTO_SW);

		val = xpcs_read(xpcs_priv,
				SR_MII_OFFSET(i) + SR_MII_CTRL);

		if (xpcs_priv->autoneg)
			val = xpcs_write(xpcs_priv,
					SR_MII_OFFSET(i) + SR_MII_CTRL,
					val | AN_ENABLE);
	}

	DO_SERDES_CONFIG(xpcs_priv, 1, QSGMII);
#if 0
	//4. Set Bit [11] of SR_XS_PCS_CTRL1 Register (or SR_MII_CTRL Register for 1000BaseX-Only PCS config-urations) to 1 to power down DWC_xpcs.
	val = xpcs_read(xpcs_priv, SR_XS_PCS_CTRL1);
	xpcs_write(xpcs_priv, SR_XS_PCS_CTRL1,
			val | BIT(11));

	//5. Wait till Bits[4:2] of VR_XS_PCS_DIG_STS Register (or VR_MII_DIG_STS Register for 1000BaseX-Only PCS configurations) are set to any value other than 3'100.
	if (readl_poll_timeout(xpcs_priv->ioaddr + VR_XS_PCS_DIG_STS, tmp,
				((tmp & GENMASK(4,2)) != (4 << 2)), 10000)) {
		printf("Wait VR_XS_PCS_DIG_STS status error[%#x]\n", tmp);
	}

	//6. Clear Bit (11] of the SR_XS_PCS_CTRL1 Register (of SR_MII_CTRL Register for 1000BaseX-Only PCS configurations) to 0 to power up DWC_xpcs.
	val = xpcs_read(xpcs_priv, SR_XS_PCS_CTRL1);
	xpcs_write(xpcs_priv, SR_XS_PCS_CTRL1,
			val & ~BIT(11));
#endif
}
typedef struct {
	union {
		u32  raw;
		struct {
			uint32_t reserved_4_0:5,
					 ss5:1,
					 ss6:1,
					 reserved_7:1,
					 duplex_mode:1,
					 restart_an:1,
					 reserved_10:1,
					 lpm:1,
					 an_enable:1,
					 ss13:1,
					 lbe:1,
					 rst:1;
		};
	};
} sr_mii_ctr_reg_st;

void cls_xpcs_speed(struct cls_xpcs_priv *xpcs_priv, unsigned int speed)
{
	sr_mii_ctr_reg_st sr_mii_ctr_reg = {0};
	uint32_t  dig_reg = xpcs_read(xpcs_priv,  VR_MII_DIG_CTRL1);
	sr_mii_ctr_reg.raw = xpcs_read(xpcs_priv,  SR_MII_CTRL);

	if (sr_mii_ctr_reg.an_enable && (dig_reg & MII_MAC_AUTO_SW)) {
		printf("%s has already enable MII_MAC_AUTO_SW, so no need to set speed[%d]\n", speed);
		return;
	}

	switch (speed) {
	case 5000:
		sr_mii_ctr_reg.ss5 = 1;
		sr_mii_ctr_reg.ss6 = 0;
		sr_mii_ctr_reg.ss13 = 1;
		break;
	case SPEED_2500:
		if (xpcs_priv->phy_interface == PHY_INTERFACE_MODE_USXGMII) {
			sr_mii_ctr_reg.ss5 = 1;
			sr_mii_ctr_reg.ss6 = 0;
			sr_mii_ctr_reg.ss13 = 0;
		} else {
			sr_mii_ctr_reg.ss5 = 0;
			sr_mii_ctr_reg.ss6 = 1;
			sr_mii_ctr_reg.ss13 = 0;
		}
		break;
	case SPEED_10000:
		sr_mii_ctr_reg.ss5 = 0;
		sr_mii_ctr_reg.ss6 = 1;
		sr_mii_ctr_reg.ss13 = 1;
		break;
	case SPEED_1000:
		sr_mii_ctr_reg.ss5 = 0;
		sr_mii_ctr_reg.ss6 = 1;
		sr_mii_ctr_reg.ss13 = 0;
		break;
	case SPEED_100:
		sr_mii_ctr_reg.ss5 = 0;
		sr_mii_ctr_reg.ss6 = 0;
		sr_mii_ctr_reg.ss13 = 1;
		break;
	case SPEED_10:
		sr_mii_ctr_reg.ss5 = 0;
		sr_mii_ctr_reg.ss6 = 0;
		sr_mii_ctr_reg.ss13 = 0;
		break;
	default:
		break;
	}

	xpcs_write(xpcs_priv,
			SR_MII_CTRL,
			sr_mii_ctr_reg.raw);
	return;
}

static void cls_set_to_hsgmii(struct cls_xpcs_priv *xpcs_priv)
{
	u32 val = 0;

	//1. Write 4'b0001 to Bits [3:0] of SR_XS_PCS_CTRL2
	val = xpcs_read(xpcs_priv, SR_XS_PCS_CTRL2);
	val &= ~GENMASK(PCS_TYPE_SEL_BIT_E,PCS_TYPE_SEL_BIT);
	val |= PCS_TYPE_SEL_10GBX;
	xpcs_write(xpcs_priv, SR_XS_PCS_CTRL2,
			val);

	val = xpcs_read(xpcs_priv, SR_MII_CTRL);
	xpcs_write(xpcs_priv,
			 SR_MII_CTRL,
			(val & ~(AN_ENABLE|BIT(6)|BIT(13))) | BIT(6));

	//2. Clear Bit 0 of VR_XS_PCS_XAUL_CTRL to 0. This bit controls the xpcs_rxaui mode o output.
	//3. Set Bit 13 of SR_PMA_CTRL1 (only for Backplane Ethernet PCS configurations) or SR_XS_PCS_CTRL1 to 0. This bit controls the xpcs_kx_kx4n_o output. This bit is inverted to drive thexpes_ky-kx4n_o output.
	XPCS_WRITE_EXT(xpcs_priv, SR_XS_PCS_CTRL1, SS13, 0);
	//  PCS_MODE to 2'b10 SGMII
	//  MII_AN_INTR_EN to 1, to Enable Auto-negotiation complete interrupt(optional step)
	//  MII_CTRL to 0 or 1, based on your MAC capability.
	//  TX_CONFIG to 1 (PHY side USXGMII) or 0 (MAC side USXGMII) based on your requirement
	//  If TX_CONFIG is set to 1, program SGMII_LINK_STS bit to suitable value to indicate the link status to the MAC side if USXGMII link.
	xpcs_write(xpcs_priv, VR_MII_AN_CTRL,
			VR_MII_CTRL_SGMII_AN_EN);
#if 0
	//Enable
	val = xpcs_read(xpcs_priv, VR_MII_DIG_CTRL1);
	xpcs_write(xpcs_priv,  VR_MII_DIG_CTRL1,
			val | MII_MAC_AUTO_SW);
#endif
	switch (xpcs_priv->id)
	{
	case 0:
		DO_SERDES_CONFIG(xpcs_priv, 0, HSGMII);
		break;
	case 1:
		DO_SERDES_CONFIG(xpcs_priv, 1, HSGMII);
		break;
	default:
	}

	//1.6ms
	xpcs_write(xpcs_priv, VR_MII_LINK_TIMER_CTRL, 0x7A1);
	val = xpcs_read(xpcs_priv, VR_MII_DIG_CTRL1);
	xpcs_write(xpcs_priv,  VR_MII_DIG_CTRL1,
			(val & (~MII_MAC_AUTO_SW)) | EN_2_5G_MODE | C37_TMR_OVR_RIDEA);
	//10.Enable Clause 37 auto-negotiation by programming bit [12] of SR_MII_CTRL Register to 1.
	val = xpcs_read(xpcs_priv,  SR_MII_CTRL);

	if (xpcs_priv->autoneg &&
			xpcs_priv->hsgmii_autoneg != HSGMII_AUTONEG_OFF)
		xpcs_write(xpcs_priv, SR_MII_CTRL, val | AN_ENABLE | BIT(6));
	else
		cls_xpcs_speed(xpcs_priv, SPEED_2500);

	if (g_debug_cls)
		printf("====)HSGMII MII_CTRL %#x \n", val);
}

static void cls_set_to_sgmii(struct cls_xpcs_priv *xpcs_priv)
{
	u32 val = 0;

	//1. Write 4'b0001 to Bits [3:0] of SR_XS_PCS_CTRL2
	val = xpcs_read(xpcs_priv, SR_XS_PCS_CTRL2);
	val &= ~GENMASK(PCS_TYPE_SEL_BIT_E,PCS_TYPE_SEL_BIT);
	val |= PCS_TYPE_SEL_10GBX;
	xpcs_write(xpcs_priv, SR_XS_PCS_CTRL2,
			val);

	//2. Clear Bit[13] of SR_PMA_CTRL1 Register (only for Backplane Ethernet PCS configurations) or SR_XS_PCS_CTRL1 Register(for other configuration) to 0.
	val = xpcs_read(xpcs_priv, SR_XS_PCS_CTRL1);
	xpcs_write(xpcs_priv, SR_XS_PCS_CTRL1,
			val & ~SR_XS_PCS_CTRL1_SPEED);

	val = xpcs_read(xpcs_priv, SR_MII_CTRL);

	xpcs_write(xpcs_priv, SR_MII_CTRL,
			val & ~AN_ENABLE);
	//2. Clear Bit 0 of VR_XS_PCS_XAUL_CTRL to 0. This bit controls the xpcs_rxaui mode o output.
	//3. Set Bit 13 of SR_PMA_CTRL1 (only for Backplane Ethernet PCS configurations) or SR_XS_PCS_CTRL1 to 0. This bit controls the xpcs_kx_kx4n_o output. This bit is inverted to drive thexpes_ky-kx4n_o output.
	//  PCS_MODE to 2'b10 SGMII
	//  MII_AN_INTR_EN to 1, to Enable Auto-negotiation complete interrupt(optional step)
	//  MII_CTRL to 0 or 1, based on your MAC capability.
	//  TX_CONFIG to 1 (PHY side USXGMII) or 0 (MAC side USXGMII) based on your requirement
	//  If TX_CONFIG is set to 1, program SGMII_LINK_STS bit to suitable value to indicate the link status to the MAC side if USXGMII link.
	xpcs_write(xpcs_priv, VR_MII_AN_CTRL,
			VR_MII_CTRL_SGMII_AN_EN);

	//set serdes
	switch (xpcs_priv->id)
	{
	case 0:
		DO_SERDES_CONFIG(xpcs_priv, 0, SGMII);
		break;
	case 1:
		DO_SERDES_CONFIG(xpcs_priv, 1, SGMII);
		break;
	default:
	}
	//Enable
	//10 ms timer for Autoneg
	xpcs_write(xpcs_priv, VR_MII_LINK_TIMER_CTRL, 0x2FAF);
	val = xpcs_read(xpcs_priv, VR_MII_DIG_CTRL1);
	xpcs_write(xpcs_priv,  VR_MII_DIG_CTRL1,
			(val & ~ EN_2_5G_MODE) | MII_MAC_AUTO_SW | C37_TMR_OVR_RIDEA);

	val = xpcs_read(xpcs_priv, SR_MII_CTRL);

	if (xpcs_priv->autoneg)
		val = xpcs_write(xpcs_priv,
				SR_MII_CTRL,
				val | AN_ENABLE);
	else
	cls_xpcs_speed(xpcs_priv, SPEED_1000);
}

//光纤
static void cls_set_to_usxgmii(struct cls_xpcs_priv *xpcs_priv, uint8_t  usxg_mode)
{
	u32 val = 0, port_num = 1, status;
	int rate = 0, i = 0;

	if (xpcs_priv->id != 0) {
		pr_info("USXGMII Only support on XPCS0\n");
		return;
	}

	if (usxg_mode > USXG_MODE_MAX_NUM) {
		printf("unknown usxg_mode(%d)", usxg_mode);
		return;
	}

	//1. SR_XS_PCS_CTRL2  bit 4'000 bits[3:0]
	val = xpcs_read(xpcs_priv, SR_XS_PCS_CTRL2);
	val &= ~GENMASK(PCS_TYPE_SEL_BIT_E, PCS_TYPE_SEL_BIT);
	val |= PCS_TYPE_SEL_10GBR;
	xpcs_write(xpcs_priv, SR_XS_PCS_CTRL2, val);

	//2. USXG_EN  VR_XS_PCS_DIG_CTRL1 set bit[9] 1
	val = xpcs_read(xpcs_priv, VR_XS_PCS_DIG_CTRL1);
	xpcs_write(xpcs_priv, VR_XS_PCS_DIG_CTRL1, val | PCS_DIG_CTRL_USXG_EN);

	//3. USXG_MODE(12:10) VR_XS_PCS_KR_CTRL
	val = xpcs_read(xpcs_priv, VR_XS_PCS_KR_CTRL);
	val &= ~GENMASK(USXG_2PT5G_GMII_END ,USXG_MODE_BIT);
	xpcs_write(xpcs_priv, VR_XS_PCS_KR_CTRL, val | (usxg_mode << USXG_MODE_BIT));

	//4. Program PHY to operation at 10.3125Gbps/5.15625Gbps/2.578125Gpbps
	//5. Base on the PHY
	switch(usxg_mode)
	{
	case USXG_MODE_10G_SXGMII:
	case USXG_MODE_10G_DXGMII:
	case USXG_MODE_10G_QXGMII:
		rate = 0;
		break;
	case USXG_MODE_5G_DXGMII:
	case USXG_MODE_5G_SXGMII:
		rate = 1;
		break;
	case USXG_MODE_2_5G_SXGMII:
		rate = 2;
		break;
	}

	DO_SERDES_CONFIG(xpcs_priv, 0, USXGMII, rate);

	printk("Waiting DPLL Lock\n");
	status = 0;
	if (readl_poll_timeout(xpcs_priv->ioaddr + VR_XS_PMA_RX_LSTS,
				status, (status & BIT(12)), 10000)) {
		printf("Line %d: wait status[%d] timeout[%#x]\n", __LINE__, status,xpcs_read(xpcs_priv, VR_XS_PMA_RX_LSTS));
		return;
	}

	XPCS_WRITE_EXT(xpcs_priv, VR_XS_PMA_MP_12G_16G_25G_RX_EQ_CTRL4, RX_AD_REQ, 1);	//发起RX adaptation

	printk("Waiting RX adapt response ...\n");
	status = 0;
	if (readl_poll_timeout(xpcs_priv->ioaddr + VR_XS_PMA_MP_12G_16G_25G_MISC_STS,
				status, status & BIT(RX_ADAPT_ACK_BEGIN),  10000)) {
		printk(KERN_ERR "Line %d: wait status[%d] timeout\n", __LINE__, status);
		return;
	}

	XPCS_WRITE_EXT(xpcs_priv, VR_XS_PMA_MP_12G_16G_25G_RX_EQ_CTRL4, RX_AD_REQ, 0);	//关闭RX adaptation

	switch(usxg_mode)
	{
	case USXG_MODE_10G_SXGMII:
	case USXG_MODE_5G_SXGMII:
	case USXG_MODE_2_5G_SXGMII:
		port_num  = 1;
		break;
	case USXG_MODE_10G_DXGMII:
	case USXG_MODE_5G_DXGMII:
		port_num  = 2;
		break;
	default:
		port_num = 4;
		break;
	}
	//6. In Backplane Ethernet PCS configuration, Program Bit[12](AN_EN) of
	//SR_AN_CTRL Registers to 0 and Bit[12](CL37_BP) of VR_XS_PCS_DIG_CTRL1 Registers to 1
	//7.Program Various bits of VR_MII_AN_CTRL Register as follows:
	//  MII_AN_INTR_EN to 1, to Enable Auto-negotiation complete interrupt(optional step)
	//  MII_CTRL to 0 or 1, based on your MAC capability.
	//  TX_CONFIG to 1 (PHY side USXGMII) or 0 (MAC side USXGMII) based on your requirement
	//  If TX_CONFIG is set to 1, program SGMII_LINK_STS bit to suitable value to indicate the link status to the MAC side if USXGMII link.
	for (i = 0; i < port_num; i++) {
		xpcs_write(xpcs_priv,
				SR_MII_OFFSET(i) + VR_MII_AN_CTRL,
				VR_MII_CTRL_UXGMII_AN_EN);

		val = xpcs_read(xpcs_priv,
				SR_MII_OFFSET(i) + VR_MII_DIG_CTRL1);

		xpcs_write(xpcs_priv,
				SR_MII_OFFSET(i) + VR_MII_DIG_CTRL1,
				val | MII_MAC_AUTO_SW);

	//8.(Optional step) Duration of link timer can be changed (default setting corresponds to 1.6ms) by programming VR_MII_LINK_TIMER_CTRL.
	//Register suitably and by setting bit [3] of VR_MII_DIG_CTRL1 Register to 1.

	//9. If DWC_xpcs is configured as PHY-side USXGMII, program SS5, SS6 and SS13 bits of SR_MII_CTRL Register to the desired speed mode.
	//The value programmed to this register gets advertised to the MAC during auto-negotiation.

		//10.Enable Clause 37 auto-negotiation by programming bit [12] of SR_MII_CTRL Register to 1.
		val = xpcs_read(xpcs_priv,
				SR_MII_OFFSET(i) +  SR_MII_CTRL);

		if (xpcs_priv->autoneg) {
			xpcs_write(xpcs_priv,
					SR_MII_OFFSET(i) + SR_MII_CTRL,
					val | AN_ENABLE);
			printf("USXGMII port%d AN_ENABLE %#x\n",
					i,
					xpcs_read(xpcs_priv, SR_MII_OFFSET(i) +  SR_MII_CTRL));
		}
	}
	//11.(If interrupt has been enabled) After the completion of auto-negotiation, DWC_xpcs generates an interrupt on sd_intr_o port.
	//12. Read the auto-negotiation status register (VR_MII_AN_INTR_STS Register), Bit [0] is set to indicate that auto-negotiation is complete.
	//Bits [14:8] indicate the link-speed, duplex mode, EEE capability and EEE clock-stop capability indicated by the link partner (PHY chip).
#if 0
	val = xpcs_read(xpcs_priv, VR_MII_AN_INTR_STS);
	if( BIT(0) & val) {
		speed =(val & GENMASK(14, 8)) >> 8
	}
#endif
	//13. Clear the Interrupt by writing 0 to bit[0] of VR_MIL_AN_INTR_STS Register.
	//14. Program SS13,SS6 and SS5 bits of SR_MIL_CTRL Register to configure DWC_xpcs to the USXGMII speed mode (for port 0) indicated by PHIY in step 12.
	//The values programmed to these bits will reflect in the output port xpcs_usxg_speed_o.
	//This step is required only if DWC_xpcs is configured as MAC-side USXGMIL
	//
	//15. Wait for some time, 1 micro second, so that @km abeks (elk xgmit ix I/ elk xgmit ox I/clk xg-mit tx_P(1,2,31 J/ clk xgmil rx p(1,2,31-4) get stabilized at the desired frequencies.
	//16. Program USRA_RST (bit[10]) of VR_XS_PCS_DIG_CTRL1 Register to 1 and wait for it to get self-cleared.
	//For multi-port configurations, program the following bits too (and wait for it to get self-cleared),
	//    •USRA_RST bit of VR_MII_1_DIG_CTRL1(for Port 1)
	//    •USRA_RST bit of VR_MII_2_DIG_CTRL1(for Port 2)
	//    •USRA_RST bit of VR_MII_3_DIG_CTRL1(for Port 3)
}

int cls_xpcs_check_link_up(struct cls_xpcs_priv  *xpcs_priv, int port)
{
	u32 val = 0;
	int speed = 0;
	vr_mii_an_intr_sts_t sts = {0};

	val = xpcs_read(xpcs_priv,
			SR_MII_OFFSET(port) + SR_MII_CTRL);

	if (!xpcs_priv->autoneg)
		return 1;

	if (!(val & AN_ENABLE)) {
		if (g_debug_cls)
			printf("XPCS%d port Not Enable AN [%#x]\n",xpcs_priv->id, port, val);
		return 2;
	}

	sts.raw = xpcs_read(xpcs_priv,
			SR_MII_OFFSET(port) + VR_MII_AN_INTR_STS);

    if (g_debug_cls)
    printf("pcs[%d] port[%d] sts [%#x]\n",xpcs_priv->id, port, sts.raw);

	switch(xpcs_priv->phy_interface) {
	case PHY_INTERFACE_MODE_SGMII:
	case PHY_INTERFACE_MODE_2500BASEX:
	case PHY_INTERFACE_MODE_QSGMII:
	case PHY_INTERFACE_MODE_SGMII_2500:
    if (g_debug_cls)
		printf("IS SGMII/HSGMII/QSGMII,STS_LINK %d speed %d\n", sts.CL37_ANSGM_STS_LINK ,sts.CL37_ANSGM_STS_SPEED);
		if (sts.CL37_ANSGM_STS_LINK && sts.CL37_ANSGM_STS_SPEED)
			return  1;
		else
			return 0;
	case PHY_INTERFACE_MODE_USXGMII:
    if (g_debug_cls)
		printf("IS USXGMII,STS_LINK %d speed %d\n", sts.USXG_AN_STS_LINK ,sts.USXG_AN_STS_SPEED);
		if(sts.USXG_AN_STS_LINK && sts.USXG_AN_STS_SPEED)
			return 1;
		else
			return 0;
	default:
		printf("Unkown the interface[%d] \n", xpcs_priv->phy_interface);
	}

	return 0;
}

void cls_serdes_reg_write(struct cls_xpcs_priv *xpcs_priv, int reg_addr, int reg_val)
{
	int start_busy_status;

	start_busy_status = 1;
	if (readl_poll_timeout(xpcs_priv->ioaddr + VR_XS_PMA_SNPS_CR_CTRL,
				start_busy_status, (start_busy_status &
					GENMASK(START_BUSY_END,START_BUSY_BEGIN)) != 1, 10000)) {
		printf("Line %d: wait status[%d] timeout", __LINE__, start_busy_status);
		return;
	}

	XPCS_WRITE_EXT(xpcs_priv, VR_XS_PMA_SNPS_CR_ADDR, ADDRESS, reg_addr);		//要写入的PHY寄存器地址
	XPCS_WRITE_EXT(xpcs_priv, VR_XS_PMA_SNPS_CR_DATA, DATA, reg_val);	    	//要写入PHY寄存器的数据
	XPCS_WRITE_EXT(xpcs_priv, VR_XS_PMA_SNPS_CR_CTRL, WR_RDN, 1); 	    		//Write执行
	XPCS_WRITE_EXT(xpcs_priv, VR_XS_PMA_SNPS_CR_CTRL, START_BUSY, 1);			//CR Port设置为busy

	start_busy_status = 1;
	if (readl_poll_timeout(xpcs_priv->ioaddr + VR_XS_PMA_SNPS_CR_CTRL,
				start_busy_status, (start_busy_status &
					GENMASK(START_BUSY_END,START_BUSY_BEGIN)) != 1, 10000)) {
		printf("Line %d: wait status[%d] timeout", __LINE__, start_busy_status);
		return;
	}
}

int cls_serdes_reg_read(struct cls_xpcs_priv  *xpcs_priv, int reg_addr)
{
	int reg_val = 0;
	int start_busy_status;

	start_busy_status = 1;
	if (readl_poll_timeout(xpcs_priv->ioaddr + VR_XS_PMA_SNPS_CR_CTRL,
				start_busy_status, (start_busy_status &
					GENMASK(START_BUSY_END,START_BUSY_BEGIN)) != 1, 10000)) {
		printf("Line %d: wait status[%d] timeout", __LINE__, start_busy_status);
		return;
	}

	XPCS_WRITE_EXT(xpcs_priv, VR_XS_PMA_SNPS_CR_ADDR, ADDRESS, reg_addr);		//要写入的PHY寄存器地址
	XPCS_WRITE_EXT(xpcs_priv, VR_XS_PMA_SNPS_CR_CTRL, WR_RDN, 0); 	    	//Read执行
	XPCS_WRITE_EXT(xpcs_priv, VR_XS_PMA_SNPS_CR_CTRL, START_BUSY, 1);			//CR Port设置为busy

	start_busy_status = 1;
	if (readl_poll_timeout(xpcs_priv->ioaddr + VR_XS_PMA_SNPS_CR_CTRL,
				start_busy_status, (start_busy_status &
					GENMASK(START_BUSY_END,START_BUSY_BEGIN)) != 1, 10000)) {
		printf("Line %d: wait status[%d] timeout", __LINE__, start_busy_status);
		return;
	}

	reg_val = XPCS_READ_EXT(xpcs_priv, VR_XS_PMA_SNPS_CR_DATA, DATA);			//读出寄存器值

	return reg_val;
}

int xpcs_bypass_serdes_ack(struct cls_xpcs_priv *xpcs_priv)
{
	uint32_t val = 0;
	val = cls_serdes_reg_read(xpcs_priv, 0x3003);

	if (g_debug_cls)
		printf("XPCS%d reg 0x3003, val [%#x]\n", xpcs_priv->id, val);

	cls_serdes_reg_write(xpcs_priv, 0x3003, 0x4);

	val = cls_serdes_reg_read(xpcs_priv, 0x300e);

	if (g_debug_cls)
		printf("XPCS%d reg 0x300e, val [%#x]\n", xpcs_priv->id, val);

	cls_serdes_reg_write(xpcs_priv, 0x300e, 0x2);
}

int cls_xpcs_config(struct cls_xpcs_priv *xpcs_priv)
{
	u16	val;
    u32 tmp = 0;
	int interface_type = xpcs_priv->phy_interface;
	uint8_t  usxg_mode = xpcs_priv->usxg_mode;

	xpcs_write_ext(xpcs_priv, \
			VR_XS_PCS_DIG_CTRL1, \
			VR_RST_END, \
			VR_RST_BEGIN, \
			1); \

	/*Set Serdes firmware Loading params*/
	cls_serdes_load_and_modify(xpcs_priv);

	if (readl_poll_timeout(xpcs_priv->ioaddr +  VR_XS_PCS_DIG_CTRL1,
				tmp, !(tmp & BIT(15)) ,10000)) {
		printf("Line %d: wait status[%d] timeout\n", __LINE__, tmp);
	}
#if 0
	printf("PCS BYP_PWRUP Enable!!!!\n");
	xpcs_write(xpcs_priv,
			VR_MII_DIG_CTRL1,
			BIT(1));
#endif
	switch (interface_type) {
	case PHY_INTERFACE_MODE_SGMII:
		if (g_debug_cls)
			printf("set xpcs PHY_INTERFACE_MODE_SGMII \n");
		cls_set_to_sgmii(xpcs_priv);
		break;
	case PHY_INTERFACE_MODE_SGMII_2500:
	case PHY_INTERFACE_MODE_2500BASEX:
		if (g_debug_cls)
			printf("set xpcs PHY_INTERFACE_MODE_SGMII_2500\n");
		cls_set_to_hsgmii(xpcs_priv);
		break;
	case PHY_INTERFACE_MODE_QSGMII:
		if (g_debug_cls)
			printf("set xpcs PHY_INTERFACE_MODE_QSGMII\n");
		cls_set_to_qsgmii(xpcs_priv);
		break;
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
		if (g_debug_cls)
			printf("set xpcs PHY_INTERFACE_MODE_RGMII\n");
		cls_set_to_rgmii(xpcs_priv);
		break;
	case PHY_INTERFACE_MODE_USXGMII:
		if (g_debug_cls)
			printf("set xpcs PHY_INTERFACE_MODE_USXGMII\n");
		cls_set_to_usxgmii(xpcs_priv, usxg_mode);
		break;
	default:
		printf("Faild: unknown interface type[%d]\n", interface_type);
		return -1;
	}

	return 0;
}
