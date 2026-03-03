#include <linux/gpio/consumer.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/mii.h>
#include <linux/of_mdio.h>
#include <linux/pm_runtime.h>
#include <linux/phy.h>
#include <linux/property.h>
#include <linux/slab.h>

#include <linux/iopoll.h>
#include "dubhe2000.h"
#include "dubhe2000_xpcs.h"
#include "dubhe2000_xpcs_serdes_regs.h"

#define XPCS_WRITE_EXT(priv, reg, field, value) xpcs_write_ext(priv, reg, field##_END, field##_BEGIN, value)
#define XPCS_READ_EXT(priv, reg, field)		(xpcs_read(priv, reg) & GENMASK(field##_END, field##_BEGIN))

#define serdes_readl_poll_timeout(xpcs_priv, addr, val, cond, delay_us, timeout_us)                                    \
	read_poll_timeout_atomic(dubhe1000_serdes_reg_read, val, cond, delay_us, timeout_us, false, xpcs_priv, addr)

struct vr_mii_an_intr_sts_t {
	union {
		u32 raw;
		struct {
			u32	CL37_ANCMP_INTR:1,
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
};

struct sr_mii_ctr_reg_st {
	union {
		u32 raw;
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
};

static u32 xpcs_read(void *priv, int reg)
{
	struct dubhe1000_pcs *xpcs_priv = (struct dubhe1000_pcs *)priv;
	u32 val = 0;

	val = readl(xpcs_priv->ioaddr + reg);

	return val;
}

static int xpcs_write(void *priv, int reg, u32 value)
{
	struct dubhe1000_pcs *xpcs_priv = (struct dubhe1000_pcs *)priv;

	writel(value, xpcs_priv->ioaddr + reg);

	return 0;
}

static int xpcs_write_ext(void *priv, int reg, int field_end, int field_begin, int value)
{
	uint32_t __val = xpcs_read(priv, reg);

	__val &= ~GENMASK(field_end, field_begin);
	__val |= (value << (field_begin)) & GENMASK(field_end, field_begin);
	xpcs_write(priv, reg, __val);
	return 0;
}

static void dubhe1000_set_to_rgmii(struct dubhe1000_pcs *xpcs_priv)
{
}

void dubhe1000_xpcs_an_restart(struct dubhe1000_pcs *xpcs_priv, int port)
{
	u32 val = 0;

	val = xpcs_read(xpcs_priv, SR_MII_OFFSET(port) + SR_MII_CTRL);
	val = xpcs_write(xpcs_priv, SR_MII_OFFSET(port) + SR_MII_CTRL, val | BIT(9));
}

void dubhe1000_xpcs_clear_an_interrupt(struct dubhe1000_pcs *xpcs_priv, int port)
{
	u32 val = 0;
	struct vr_mii_an_intr_sts_t sts = { 0 };

	val = xpcs_read(xpcs_priv, SR_MII_OFFSET(port) + SR_MII_CTRL);

	if (!(val & AN_ENABLE))
		return;

	sts.raw = xpcs_read(xpcs_priv, SR_MII_OFFSET(port) + VR_MII_AN_INTR_STS);

	sts.CL37_ANCMP_INTR = 0;

	xpcs_write(xpcs_priv, SR_MII_OFFSET(port) + VR_MII_AN_INTR_STS, sts.raw);
}

uint32_t dubhe1000_xpcs_get_sts2(struct dubhe1000_pcs *xpcs_priv)
{
	return xpcs_read(xpcs_priv, SR_XS_PCS_STS2);
}

void dubhe1000_xpcs_get_link_state(struct dubhe1000_pcs *xpcs_priv, int port, struct phylink_link_state *state)
{
	int speed = 0;
	struct vr_mii_an_intr_sts_t sts = { 0 };
	struct sr_mii_ctr_reg_st sr_mii_ctr_reg = { 0 };

	sr_mii_ctr_reg.raw = xpcs_read(xpcs_priv, SR_MII_OFFSET(port) + SR_MII_CTRL);

	if (!sr_mii_ctr_reg.an_enable) {
		uint32_t is_err = 0;
		state->an_enabled = 0;

		state->interface = xpcs_priv->phy_interface;

		if (xpcs_priv->phy_interface == PHY_INTERFACE_MODE_USXGMII ||
			xpcs_priv->phy_interface == PHY_INTERFACE_MODE_10GBASER)
			speed = sr_mii_ctr_reg.ss5 | sr_mii_ctr_reg.ss6 << 1 | sr_mii_ctr_reg.ss13 << 2;
		else
			speed = sr_mii_ctr_reg.ss6 << 1 | sr_mii_ctr_reg.ss13 << 2;

		switch (speed) {
		case 5:
			state->speed = 5000;
			break;
		case 1:
			if (xpcs_priv->phy_interface == PHY_INTERFACE_MODE_USXGMII ||
			xpcs_priv->phy_interface == PHY_INTERFACE_MODE_10GBASER)
				state->speed = SPEED_2500;
			break;
		case 6:
			state->speed = SPEED_10000;
			break;
		case 2:
			if (xpcs_priv->phy_interface == PHY_INTERFACE_MODE_2500BASEX)
				state->speed = SPEED_2500;
			else
				state->speed = SPEED_1000;
			break;
		case 4:
			state->speed = SPEED_100;
			break;
		case 0:
			state->speed = SPEED_10;
			break;
		default:
			pr_err("Unknown the speed %d interface_type[%d]", speed, xpcs_priv->phy_interface);
			break;
		}

		state->duplex = sr_mii_ctr_reg.duplex_mode;

		is_err = !!(dubhe1000_xpcs_get_sts2(xpcs_priv) & BIT(10));
		state->link = !is_err;

	} else {
		state->an_enabled = 1;
		sts.raw = xpcs_read(xpcs_priv, SR_MII_OFFSET(port) + VR_MII_AN_INTR_STS);

		state->interface = xpcs_priv->phy_interface;

		switch (xpcs_priv->phy_interface) {
		case PHY_INTERFACE_MODE_SGMII:
		case PHY_INTERFACE_MODE_1000BASEX:
		case PHY_INTERFACE_MODE_2500BASEX:
		case PHY_INTERFACE_MODE_QSGMII:
			state->link = sts.CL37_ANSGM_STS_LINK;
			state->duplex = sts.CL37_ANSGM_STS_DUPLEX;
			speed = sts.CL37_ANSGM_STS_SPEED;
			break;
		case PHY_INTERFACE_MODE_10GBASER:
		case PHY_INTERFACE_MODE_USXGMII:
			state->link = sts.USXG_AN_STS_LINK;
			state->duplex = sts.USXG_AN_STS_DUPLEX;
			speed = sts.USXG_AN_STS_SPEED;
			break;
		default:
			pr_err("Unknown the interface[%d]", xpcs_priv->phy_interface);
		}

		switch (speed) {
		case XPCS_SPEED_10:
			state->speed = SPEED_10;
			break;
		case XPCS_SPEED_100:
			state->speed = SPEED_100;
			break;
		case XPCS_SPEED_1000:
			state->speed = SPEED_1000;
			break;
		case XPCS_SPEED_10000:
			state->speed = SPEED_10000;
			break;
		case XPCS_SPEED_2500:
			state->speed = SPEED_2500;
			break;
		case XPCS_SPEED_5000:
			state->speed = SPEED_5000;
			break;
		}

		if (sts.CL37_ANCMP_INTR)
			state->an_complete = 1;
	}
}

void dubhe1000_serdes_reg_write(struct dubhe1000_pcs *xpcs_priv, int reg_addr, int reg_val)
{
	int start_busy_status;

	start_busy_status = 1;
	if (readl_poll_timeout(xpcs_priv->ioaddr + VR_XS_PMA_SNPS_CR_CTRL, start_busy_status,
			       (start_busy_status & GENMASK(START_BUSY_END, START_BUSY_BEGIN)) != 1, 10, 10000)) {
		pr_err("Line %d: wait status[%d] timeout", __LINE__, start_busy_status);
		return;
	}

	XPCS_WRITE_EXT(xpcs_priv, VR_XS_PMA_SNPS_CR_ADDR, ADDRESS, reg_addr); // the PHY register address to be written
	XPCS_WRITE_EXT(xpcs_priv, VR_XS_PMA_SNPS_CR_DATA, DATA, reg_val);     // the data to be written to PHY register
	XPCS_WRITE_EXT(xpcs_priv, VR_XS_PMA_SNPS_CR_CTRL, WR_RDN, 1);	      // perform write
	XPCS_WRITE_EXT(xpcs_priv, VR_XS_PMA_SNPS_CR_CTRL, START_BUSY, 1);     // set CR Port to busy

	start_busy_status = 1;
	if (readl_poll_timeout(xpcs_priv->ioaddr + VR_XS_PMA_SNPS_CR_CTRL, start_busy_status,
			       (start_busy_status & GENMASK(START_BUSY_END, START_BUSY_BEGIN)) != 1, 10, 10000)) {
		pr_err("Line %d: wait status[%d] timeout", __LINE__, start_busy_status);
		return;
	}
}

int dubhe1000_serdes_reg_read(struct dubhe1000_pcs *xpcs_priv, int reg_addr)
{
	int reg_val = 0;
	int start_busy_status;

	start_busy_status = 1;
	if (readl_poll_timeout(xpcs_priv->ioaddr + VR_XS_PMA_SNPS_CR_CTRL, start_busy_status,
			       (start_busy_status & GENMASK(START_BUSY_END, START_BUSY_BEGIN)) != 1, 10, 10000)) {
		pr_err("Line %d: wait status[%d] timeout", __LINE__, start_busy_status);
		return 0;
	}

	XPCS_WRITE_EXT(xpcs_priv, VR_XS_PMA_SNPS_CR_ADDR, ADDRESS, reg_addr); // the PHY register address to be written
	XPCS_WRITE_EXT(xpcs_priv, VR_XS_PMA_SNPS_CR_CTRL, WR_RDN, 0);	      // perform read
	XPCS_WRITE_EXT(xpcs_priv, VR_XS_PMA_SNPS_CR_CTRL, START_BUSY, 1);     // set CR Port to busy

	start_busy_status = 1;
	if (readl_poll_timeout(xpcs_priv->ioaddr + VR_XS_PMA_SNPS_CR_CTRL, start_busy_status,
			       (start_busy_status & GENMASK(START_BUSY_END, START_BUSY_BEGIN)) != 1, 10, 10000)) {
		pr_err("Line %d: wait status[%d] timeout", __LINE__, start_busy_status);
		return 0;
	}

	reg_val = XPCS_READ_EXT(xpcs_priv, VR_XS_PMA_SNPS_CR_DATA, DATA); // read value from register

	return reg_val;
}

static int xpcs_serdes_calibration(struct dubhe1000_pcs *xpcs_priv)
{
	uint32_t val = 0;

	if (verbose) {
		val = dubhe1000_serdes_reg_read(xpcs_priv, 0x1b);
		pr_err("XPCS%d reg 0x1b, val [%#x]\n", xpcs_priv->id, val);
	}

	dubhe1000_serdes_reg_write(xpcs_priv, 0xe, 0x18);

	usleep_range(1, 2);

	if (serdes_readl_poll_timeout(xpcs_priv, 0x1b, val, !(val & BIT(10)), 10, 10000))
		pr_err("Line %d: wait calibration status[%d] timeout", __LINE__, val);

	dubhe1000_serdes_reg_write(xpcs_priv, 0xe, 0x10);

	return 0;
}

void dubhe1000_serdes_load_and_modify(struct dubhe1000_pcs *xpcs_priv)
{
	int status = 0;
	void __iomem *top;
	struct dubhe1000_adapter *adapter = xpcs_priv->adapter;

	if (!adapter) {
		pr_err("%s LINE %d adapter is NULL", __func__, __LINE__);
		return;
	}

	top = adapter->top_regs;
	if (!top) {
		pr_err("%s LINE %d top is NULL", __func__, __LINE__);
		return;
	}

	if (verbose)
		pr_err("%s LINE %d top[0x%px]", __func__, __LINE__, top);

	if (verbose)
		pr_info("Wait for XPCS%d SRAM initialization to complete ...\n", xpcs_priv->id);

	status = 0;
	if (readl_poll_timeout(xpcs_priv->ioaddr + VR_XS_PMA_MP_12G_16G_25G_SRAM, status, (status & BIT(0)), 10,
			       50000)) {
		pr_info("Line %d: wait status[%d] timeout", __LINE__, status);
	}

	/* the firmware loaded into SRAM can be modified during this waiting time */
	/* indicate that Serdes0 SRAM external loading is complete */
	writel(3, xpcs_priv->ioaddr + VR_XS_PMA_MP_12G_16G_25G_SRAM);

	mdelay(1);

	xpcs_serdes_calibration(xpcs_priv);

	if (verbose)
		pr_info("Indicates that Serdes%d external loading is complete ...\n", xpcs_priv->id);

	status = 0;
	if (readl_poll_timeout(xpcs_priv->ioaddr + SR_XS_PCS_CTRL1, status, !(status & BIT(15)), 10, 50000))
		pr_info("Line %d: wait status[%d] timeout", __LINE__, status);
}

static void dubhe1000_set_to_qsgmii(struct dubhe1000_pcs *xpcs_priv)
{
	int i;
	u32 val = 0;

	if (xpcs_priv->id != 1) {
		pr_info("QSGMII Only support on XPCS1\n");
		return;
	}

	//1. Write 4'b0001 to Bits [3:0] of SR_XS_PCS_CTRL2 Register (Optional- Required only if the
	//   configuration has KR/10GBASE-R support).
	val = xpcs_read(xpcs_priv, SR_XS_PCS_CTRL2);
	val &= ~GENMASK(PCS_TYPE_SEL_BIT_E, PCS_TYPE_SEL_BIT);
	val |= PCS_TYPE_SEL_10GBX;
	xpcs_write(xpcs_priv, SR_XS_PCS_CTRL2, val);

	//2. Clear Bit[13] of SR_PMA_CTRL1 Register (only for Backplane Ethernet PCS configurations) or
	//   SR_XS_PCS_CTRL1 Register(for other configuration) to 0.
	val = xpcs_read(xpcs_priv, SR_XS_PCS_CTRL1);
	xpcs_write(xpcs_priv, SR_XS_PCS_CTRL1, val & ~SR_XS_PCS_CTRL1_SPEED);

	//3. Program bits [2:1] of VR_MII_AN_CTRL Register to 2'b11.this asserts xgxs_qsgmii_mode_o'port of DWC_xpcs.
	//   PCS_MODE to 2'b11 qsgmii
	//   MII_AN_INTR_EN to 1, to Enable Auto-negotiation complete interrupt(optional step)
	//   MII_CTRL to 0 or 1, based on your MAC capability.
	//   TX_CONFIG to 1 (PHY side USXGMII) or 0 (MAC side USXGMII) based on your requirement
	//   If TX_CONFIG is set to 1, program SGMII_LINK_STS bit to suitable value to indicate the
	//   link status to the MAC side if USXGMII link.
	for (i = 0; i < 4; i++) {
		val = xpcs_read(xpcs_priv, SR_MII_OFFSET(i) + SR_MII_CTRL);
		val = xpcs_write(xpcs_priv, SR_MII_OFFSET(i) + SR_MII_CTRL, val & ~AN_ENABLE);

		xpcs_write(xpcs_priv, SR_MII_OFFSET(i) + VR_MII_AN_CTRL, VR_MII_CTRL_QSGMII_AN_EN);
		//	VR_MII_CTRL_QSGMII_AN_EN | AN_IND_TX_EN);

		if (xpcs_priv->autoneg) {
			val = xpcs_read(xpcs_priv, SR_MII_OFFSET(i) + VR_MII_DIG_CTRL1);
			xpcs_write(xpcs_priv, SR_MII_OFFSET(i) + VR_MII_DIG_CTRL1, val | MII_MAC_AUTO_SW);

			val = xpcs_read(xpcs_priv, SR_MII_OFFSET(i) + SR_MII_CTRL);
			val = xpcs_write(xpcs_priv, SR_MII_OFFSET(i) + SR_MII_CTRL, val | AN_ENABLE);
		}
	}

	DO_SERDES_CONFIG(xpcs_priv, 1, QSGMII);
}

void dubhe1000_xpcs_speed(struct dubhe1000_pcs *xpcs_priv, int port, unsigned int speed)
{
	struct sr_mii_ctr_reg_st sr_mii_ctr_reg = { 0 };
	uint32_t dig_reg = xpcs_read(xpcs_priv, SR_MII_OFFSET(port) + VR_MII_DIG_CTRL1);

	sr_mii_ctr_reg.raw = xpcs_read(xpcs_priv, SR_MII_OFFSET(port) + SR_MII_CTRL);

	if (sr_mii_ctr_reg.an_enable && (dig_reg & MII_MAC_AUTO_SW)) {
		if (verbose)
			printk(KERN_ERR "%s has already enable MII_MAC_AUTO_SW, so no need to set speed[%d]\n",
			       __func__, speed);
		return;
	}

	if (verbose)
		pr_err("%s: PCS%d port%d speed:%d\n",
				__func__, xpcs_priv->id, port, speed);

	switch (speed) {
	case 5000:
		sr_mii_ctr_reg.ss5 = 1;
		sr_mii_ctr_reg.ss6 = 0;
		sr_mii_ctr_reg.ss13 = 1;
		break;
	case SPEED_2500:
		if (xpcs_priv->phy_interface == PHY_INTERFACE_MODE_USXGMII ||
			xpcs_priv->phy_interface == PHY_INTERFACE_MODE_10GBASER) {
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

	xpcs_write(xpcs_priv, SR_MII_OFFSET(port) + SR_MII_CTRL, sr_mii_ctr_reg.raw);
}

//must
static void dubhe1000_set_to_hsgmii(struct dubhe1000_pcs *xpcs_priv)
{
	u32 val = 0;

	//1. Write 4'b0001 to Bits [3:0] of SR_XS_PCS_CTRL2
	val = xpcs_read(xpcs_priv, SR_XS_PCS_CTRL2);
	val &= ~GENMASK(PCS_TYPE_SEL_BIT_E, PCS_TYPE_SEL_BIT);
	val |= PCS_TYPE_SEL_10GBX;
	xpcs_write(xpcs_priv, SR_XS_PCS_CTRL2, val);
	val = xpcs_read(xpcs_priv, SR_MII_CTRL);
	xpcs_write(xpcs_priv, SR_MII_CTRL, (val & ~(AN_ENABLE | BIT(6) | BIT(13))) | BIT(6));
	//2. Clear Bit 0 of VR_XS_PCS_XAUL_CTRL to 0. This bit controls the xpcs_rxaui mode o output.
	//3. Set Bit 13 of SR_PMA_CTRL1 (only for Backplane Ethernet PCS configurations) or SR_XS_PCS_CTRL1 to 0.
	//   This bit controls the xpcs_kx_kx4n_o output. This bit is inverted to drive thexpes_ky-kx4n_o output.
	XPCS_WRITE_EXT(xpcs_priv, SR_XS_PCS_CTRL1, SS13, 0);
	//  PCS_MODE to 2'b10 SGMII
	//  MII_AN_INTR_EN to 1, to Enable Auto-negotiation complete interrupt(optional step)
	//  MII_CTRL to 0 or 1, based on your MAC capability.
	//  TX_CONFIG to 1 (PHY side USXGMII) or 0 (MAC side USXGMII) based on your requirement
	//  If TX_CONFIG is set to 1, program SGMII_LINK_STS bit to suitable value to indicate the link
	//  status to the MAC side if USXGMII link.
	xpcs_write(xpcs_priv, VR_MII_AN_CTRL, VR_MII_CTRL_SGMII_AN_EN);
	//VR_MII_CTRL_SGMII_AN_EN | AN_IND_TX_EN);

	switch (xpcs_priv->id) {
	case 0:
		DO_SERDES_CONFIG(xpcs_priv, 0, HSGMII);
		break;
	case 1:
		DO_SERDES_CONFIG(xpcs_priv, 1, HSGMII);
		break;
	default:
	}

	if (xpcs_priv->autoneg && (xpcs_priv->hsgmii_autoneg != HSGMII_AUTONEG_OFF)) {
		xpcs_write(xpcs_priv, VR_MII_LINK_TIMER_CTRL, 0x7A1);

		val = xpcs_read(xpcs_priv, VR_MII_DIG_CTRL1);

		xpcs_write(xpcs_priv, VR_MII_DIG_CTRL1, (val & (~MII_MAC_AUTO_SW)) | EN_2_5G_MODE | C37_TMR_OVR_RIDEA);

		//10.Enable Clause 37 auto-negotiation by programming bit [12] of SR_MII_CTRL Register to 1.
		val = xpcs_read(xpcs_priv, SR_MII_CTRL);
		xpcs_write(xpcs_priv, SR_MII_CTRL, val | AN_ENABLE | BIT(6));
	} else {
		val = xpcs_read(xpcs_priv, VR_MII_DIG_CTRL1);
		xpcs_write(xpcs_priv, VR_MII_DIG_CTRL1, (val & (~MII_MAC_AUTO_SW)) | EN_2_5G_MODE | C37_TMR_OVR_RIDEA);

		val = xpcs_read(xpcs_priv, SR_MII_CTRL);
		xpcs_write(xpcs_priv, SR_MII_CTRL, val | BIT(6));
	}
}

static void dubhe1000_set_to_sgmii(struct dubhe1000_pcs *xpcs_priv)
{
	u32 val = 0;

	//1. Write 4'b0001 to Bits [3:0] of SR_XS_PCS_CTRL2
	val = xpcs_read(xpcs_priv, SR_XS_PCS_CTRL2);
	val &= ~GENMASK(PCS_TYPE_SEL_BIT_E, PCS_TYPE_SEL_BIT);
	val |= PCS_TYPE_SEL_10GBX;
	xpcs_write(xpcs_priv, SR_XS_PCS_CTRL2, val);

	//2. Clear Bit[13] of SR_PMA_CTRL1 Register (only for Backplane Ethernet PCS configurations) or
	//   SR_XS_PCS_CTRL1 Register(for other configuration) to 0.
	val = xpcs_read(xpcs_priv, SR_XS_PCS_CTRL1);
	xpcs_write(xpcs_priv, SR_XS_PCS_CTRL1, val & ~SR_XS_PCS_CTRL1_SPEED);

	val = xpcs_read(xpcs_priv, SR_MII_CTRL);

	xpcs_write(xpcs_priv, SR_MII_CTRL, val & ~AN_ENABLE);
	//2. Clear Bit 0 of VR_XS_PCS_XAUL_CTRL to 0. This bit controls the xpcs_rxaui mode o output.
	//3. Set Bit 13 of SR_PMA_CTRL1 (only for Backplane Ethernet PCS configurations) or SR_XS_PCS_CTRL1 to 0.
	//   This bit controls the xpcs_kx_kx4n_o output. This bit is inverted to drive thexpes_ky-kx4n_o output.
	//   PCS_MODE to 2'b10 SGMII
	//   MII_AN_INTR_EN to 1, to Enable Auto-negotiation complete interrupt(optional step)
	//   MII_CTRL to 0 or 1, based on your MAC capability.
	//   TX_CONFIG to 1 (PHY side USXGMII) or 0 (MAC side USXGMII) based on your requirement
	//   If TX_CONFIG is set to 1, program SGMII_LINK_STS bit to suitable value to indicate the
	//   link status to the MAC side if USXGMII link.
	xpcs_write(xpcs_priv, VR_MII_AN_CTRL, VR_MII_CTRL_SGMII_AN_EN);
	//VR_MII_CTRL_SGMII_AN_EN | AN_IND_TX_EN);

	//set serdes
	switch (xpcs_priv->id) {
	case 0:
		DO_SERDES_CONFIG(xpcs_priv, 0, SGMII);
		break;
	case 1:
		DO_SERDES_CONFIG(xpcs_priv, 1, SGMII);
		break;
	default:
	}

	if (xpcs_priv->autoneg) {
		//Enable
		//10 ms timer for Autoneg
		xpcs_write(xpcs_priv, VR_MII_LINK_TIMER_CTRL, 0x2FAF);

		val = xpcs_read(xpcs_priv, VR_MII_DIG_CTRL1);
		xpcs_write(xpcs_priv, VR_MII_DIG_CTRL1, (val & (~EN_2_5G_MODE | MII_MAC_AUTO_SW)) | C37_TMR_OVR_RIDEA);

		val = xpcs_read(xpcs_priv, SR_MII_CTRL);

		val = xpcs_write(xpcs_priv, SR_MII_CTRL, val | AN_ENABLE);
	} else {
		val = xpcs_read(xpcs_priv, VR_MII_DIG_CTRL1);
		xpcs_write(xpcs_priv, VR_MII_DIG_CTRL1, (val & ~(EN_2_5G_MODE | MII_MAC_AUTO_SW)));
	}
}

void dubhe1000_usxgmii_reset_rate_adapter(struct dubhe1000_pcs *xpcs_priv, int port)
{
	int status;

	XPCS_WRITE_EXT(xpcs_priv, SR_MII_OFFSET(port) + VR_MII_DIG_CTRL1, USRA_RST_1, 1);

	if (readl_poll_timeout(xpcs_priv->ioaddr + SR_MII_OFFSET(port) + VR_MII_DIG_CTRL1, status,
			       (status & BIT(USRA_RST_1_BEGIN)) == 0, 10, 50000)) {
		printk(KERN_ERR "Line %d: wait status[%d] timeout\n", __LINE__, status);
		return;
	}
}

static void dubhe1000_set_to_usxgmii(struct dubhe1000_pcs *xpcs_priv, uint32_t usxg_mode)
{
	u32 val = 0, port_num = 1;
	int rate = 0, i = 0;

	if (xpcs_priv->id != 0) {
		pr_info("USXGMII Only support on XPCS0\n");
		return;
	}

	if (usxg_mode > USXG_MODE_MAX_NUM) {
		pr_info("unknown usxg_mode(%d)", usxg_mode);
		return;
	}

	if (xpcs_priv->phy_interface == PHY_INTERFACE_MODE_10GBASER)
		xpcs_priv->autoneg = false;

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
	val &= ~GENMASK(USXG_2PT5G_GMII_END, USXG_MODE_BIT);
	xpcs_write(xpcs_priv, VR_XS_PCS_KR_CTRL, val | (usxg_mode << USXG_MODE_BIT));

	//4. Program PHY to operation at 10.3125Gbps/5.15625Gbps/2.578125Gpbps
	//5. Base on the PHY
	switch (usxg_mode) {
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

#ifdef USXGMII_RX_ADPT_ENANLE
	{
		u32 status = 0;

		if (readl_poll_timeout(xpcs_priv->ioaddr + VR_XS_PMA_RX_LSTS, status, (status & BIT(12)), 10, 50000)) {
			printk(KERN_ERR "Line %d: wait status[%d] timeout[%#x]\n", __LINE__, status,
					xpcs_read(xpcs_priv, VR_XS_PMA_RX_LSTS));
			return;
		}

		XPCS_WRITE_EXT(xpcs_priv, VR_XS_PMA_MP_12G_16G_25G_RX_EQ_CTRL4, RX_AD_REQ, 1); // startRX adaptation

		pr_info("Waiting RX adapt response ...");
		status = 0;
		if (readl_poll_timeout(xpcs_priv->ioaddr + VR_XS_PMA_MP_12G_16G_25G_MISC_STS, status,
					status & BIT(RX_ADAPT_ACK_BEGIN), 10, 50000)) {
			printk(KERN_ERR "Line %d: wait status[%d] timeout", __LINE__, status);
			return;
		}
	}
#endif

	XPCS_WRITE_EXT(xpcs_priv, VR_XS_PMA_MP_12G_16G_25G_RX_EQ_CTRL4, RX_AD_REQ, 0); // stop RX adaptation

	switch (usxg_mode) {
	case USXG_MODE_10G_SXGMII:
	case USXG_MODE_5G_SXGMII:
	case USXG_MODE_2_5G_SXGMII:
		port_num = 1;
		break;
	case USXG_MODE_10G_DXGMII:
	case USXG_MODE_5G_DXGMII:
		port_num = 2;
		break;
	default:
		port_num = 4;
		break;
	}

	if (verbose)
		printk(KERN_ERR "====>%s uxgmii port num %d mode=%d\n", __func__, port_num, usxg_mode);
	//6. In Backplane Ethernet PCS configuration, Program Bit[12](AN_EN) of
	//   SR_AN_CTRL Registers to 0 and Bit[12](CL37_BP) of VR_XS_PCS_DIG_CTRL1 Registers to 1
	//7. Program Various bits of VR_MII_AN_CTRL Register as follows:
	//   MII_AN_INTR_EN to 1, to Enable Auto-negotiation complete interrupt(optional step)
	//   MII_CTRL to 0 or 1, based on your MAC capability.
	//   TX_CONFIG to 1 (PHY side USXGMII) or 0 (MAC side USXGMII) based on your requirement
	//   If TX_CONFIG is set to 1, program SGMII_LINK_STS bit to suitable value to indicate the
	//   link status to the MAC side if USXGMII link.
	for (i = 0; i < port_num; i++) {
		xpcs_write(xpcs_priv, SR_MII_OFFSET(i) + VR_MII_AN_CTRL, VR_MII_CTRL_UXGMII_AN_EN);
		//VR_MII_CTRL_UXGMII_AN_EN | AN_IND_TX_EN);

		if (xpcs_priv->autoneg) {
			val = xpcs_read(xpcs_priv, SR_MII_OFFSET(i) + VR_MII_DIG_CTRL1);

			xpcs_write(xpcs_priv, SR_MII_OFFSET(i) + VR_MII_DIG_CTRL1, val | MII_MAC_AUTO_SW);

			//8. (Optional step) Duration of link timer can be changed (default setting corresponds
			//   to 1.6ms) by programming VR_MII_LINK_TIMER_CTRL.
			//   Register suitably and by setting bit [3] of VR_MII_DIG_CTRL1 Register to 1.

			//9. If DWC_xpcs is configured as PHY-side USXGMII, program SS5, SS6 and SS13 bits of
			//   SR_MII_CTRL Register to the desired speed mode.
			//   The value programmed to this register gets advertised to the MAC during auto-negotiation.

			//10.Enable Clause 37 auto-negotiation by programming bit [12] of SR_MII_CTRL Register to 1.
			val = xpcs_read(xpcs_priv, SR_MII_OFFSET(i) + SR_MII_CTRL);
			xpcs_write(xpcs_priv, SR_MII_OFFSET(i) + SR_MII_CTRL, val | AN_ENABLE);
		} else {
			val = xpcs_read(xpcs_priv, SR_MII_OFFSET(i) + VR_MII_DIG_CTRL1);
			xpcs_write(xpcs_priv, SR_MII_OFFSET(i) + VR_MII_DIG_CTRL1, val & ~MII_MAC_AUTO_SW);
		}
	}

	//11. (If interrupt has been enabled) After the completion of auto-negotiation,
	//    DWC_xpcs generates an interrupt on sd_intr_o port.
	//12. Read the auto-negotiation status register (VR_MII_AN_INTR_STS Register),
	//    Bit [0] is set to indicate that auto-negotiation is complete.
	//    Bits [14:8] indicate the link-speed, duplex mode, EEE capability and EEE clock-stop capability
	//    indicated by the link partner (PHY chip).

	//13. Clear the Interrupt by writing 0 to bit[0] of VR_MIL_AN_INTR_STS Register.
	//14. Program SS13,SS6 and SS5 bits of SR_MIL_CTRL Register to configure DWC_xpcs to the USXGMII speed
	//    mode (for port 0) indicated by PHIY in step 12.
	//    The values programmed to these bits will reflect in the output port xpcs_usxg_speed_o.
	//    This step is required only if DWC_xpcs is configured as MAC-side USXGMIL
	//15. Wait for some time, 1 micro second, so that @km abeks (elk xgmit ix I/ elk xgmit ox I/clk xg-mit
	//    tx_P(1,2,31 J/ clk xgmil rx p(1,2,31-4) get stabilized at the desired frequencies.
	//16. Program USRA_RST (bit[10]) of VR_XS_PCS_DIG_CTRL1 Register to 1 and wait for it to get self-cleared.
	//    For multi-port configurations, program the following bits too (and wait for it to get self-cleared),
	//    USRA_RST bit of VR_MII_1_DIG_CTRL1(for Port 1)
	//    USRA_RST bit of VR_MII_2_DIG_CTRL1(for Port 2)
	//    USRA_RST bit of VR_MII_3_DIG_CTRL1(for Port 3)
}

int dubhe1000_set_pcs_mode(struct dubhe1000_pcs *xpcs_priv)
{
	u32 tmp = 0;
	int interface_type = xpcs_priv->phy_interface;
	uint8_t usxg_mode = xpcs_priv->usxg_mode;

	xpcs_write_ext(xpcs_priv, VR_XS_PCS_DIG_CTRL1, VR_RST_END, VR_RST_BEGIN, 1);

	/*Set Serdes firmware Loading params*/
	dubhe1000_serdes_load_and_modify(xpcs_priv);

	if (readl_poll_timeout(xpcs_priv->ioaddr + VR_XS_PCS_DIG_CTRL1, tmp, !(tmp & BIT(15)), 10, 10000))
		printk(KERN_ERR "Line %d: wait status[%d] timeout\n", __LINE__, tmp);

	switch (interface_type) {
	case PHY_INTERFACE_MODE_1000BASEX:
	case PHY_INTERFACE_MODE_SGMII:
		if (verbose)
			pr_info("set xpcs PHY_INTERFACE_MODE_SGMII\n");
		dubhe1000_set_to_sgmii(xpcs_priv);
		break;
	case PHY_INTERFACE_MODE_2500BASEX:
		if (verbose)
			pr_info("set xpcs PHY_INTERFACE_MODE_SGMII_2500\n");
		dubhe1000_set_to_hsgmii(xpcs_priv);
		break;
	case PHY_INTERFACE_MODE_QSGMII:
		if (verbose)
			pr_info("set xpcs PHY_INTERFACE_MODE_QSGMII\n");
		dubhe1000_set_to_qsgmii(xpcs_priv);
		break;
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
		if (verbose)
			pr_info("set xpcs PHY_INTERFACE_MODE_RGMII\n");
		dubhe1000_set_to_rgmii(xpcs_priv);
		break;
	case PHY_INTERFACE_MODE_10GBASER:
	case PHY_INTERFACE_MODE_USXGMII:
		if (verbose)
			pr_info("set xpcs PHY_INTERFACE_MODE_USXGMII\n");
		dubhe1000_set_to_usxgmii(xpcs_priv, usxg_mode);
		break;
	default:
		pr_info("Failed: unknown interface type[%d]\n", interface_type);
		return -EINVAL;
	}
	return 0;
}

int dubhe1000_xpcs_enable(struct dubhe1000_pcs *xpcs_priv, int port, int enable)
{
	return 0;
}
