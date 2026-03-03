// SPDX-License-Identifier: GPL-2.0
/*
 * PCIe EP controller driver for Clourney SoCs
 *
 * Copyright (C) 2022 Clourney Co., Ltd.
 *
 */
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/of_device.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/resource.h>
#include <linux/types.h>
#include <linux/regmap.h>
#include <linux/of_gpio.h>
#include "pcie-designware.h"

#define PCIE_CLK_SEL_PARA          0x20C
#define PCIE_PERI_SUB_RST_PARA0    0x08
#define PCIE_GSYS_LTSSM_ENABLE     0x10
#define PCIE_GSYS_RDLH_LINK_UP     0xc0
#define PCIE_GSYS_PCIE_DEVICE_TYPE 0x1c0
#define PCIE_GSYS_SYS_INT          0x600
#define PCIE_GSYS_PHY_CLK_CONTROL  0x828

#define PCIE_PHY_SUP_DIG_IDCODE_LO           0x1000
#define PCIE_PHY_SUP_DIG_MPLLA_OVRD_IN_0     0x1004
#define PCIE_PHY_SUP_SUP_DIG_MPLLA_OVRD_IN_4 0x1008
#define PCIE_PHY_SUP_DIG_MPLLB_OVRD_IN_3     0x100c
#define PCIE_PHY_SUP_DIG_SUP_OVRD_IN_2       0x1010

#define GEN3_EQ_CONTROL_OFF                     0x8a8
#define GEN3_EQ_CONTROL_OFF_PSET_REQ_VEC_SHIFT  8
#define GEN3_EQ_CONTROL_OFF_PSET_REQ_VEC_MASK   GENMASK(23, 8)
#define GEN3_EQ_CONTROL_OFF_FB_MODE_MASK        GENMASK(3, 0)

#define GEN3_RELATED_OFF                        0x890
#define GEN3_RELATED_OFF_GEN3_ZRXDC_NONCOMPL    BIT(0)
#define GEN3_RELATED_OFF_RATE_SHADOW_SEL_SHIFT  24
#define GEN3_RELATED_OFF_RATE_SHADOW_SEL_MASK   GENMASK(25, 24)

#define CAP_SPCIE_CAP_OFF                       0x154
#define CAP_SPCIE_CAP_OFF_DSP_TX_PRESET0_MASK   GENMASK(3, 0)
#define CAP_SPCIE_CAP_OFF_USP_TX_PRESET0_MASK   GENMASK(11, 8)
#define CAP_SPCIE_CAP_OFF_USP_TX_PRESET0_SHIFT  8

#define PCIE_UNCORR_ERR_STARUS_OFFSET   0x104
#define PCIE_UNCORR_ERR_MASK_OFFSET     0x108
#define PCIE_CORR_ERR_STARUS_OFFSET     0x110
#define PCIE_CORR_ERR_MASK_OFFSET       0x114
#define PCIE_ROOT_ERR_STARUS_OFFSET     0x130

#define PCIE_ERR_REPORT_OFFSET          0x78
#define PCIE_ERR_REPORT_EN_VAL          0x7
#define PCIE_ROOT_ERR_CMD_OFFSET        0x12c
#define PCIE_ROOT_ERR_CMD_EN_VAL        0x7

#define LINK_UP_TIMER_OUT           -1
#define GEN3_GEN4_EQ_PRESET_INIT    5
#define SERDES_REG_POLLING_TIMES    50
#define LINK_STATE_MAX_POLLING_TIME 1000
#define PCIE_TYPE0_HDR_DBI2_OFFSET  0x100000
#define to_dw_plat_pcie_from_dw_pcie(dw_pcie)  dw_pcie->dw_plat_pcie
struct dw_plat_pcie_of_data {
	enum dw_pcie_device_mode	mode;
};

struct link_state_record {
	u16 ltssm;
	u16 sec;
	long nsec;
};

struct dw_plat_pcie_t {
	int dma_irq;
	struct dw_pcie		*pci;
	struct regmap		*regmap;
	enum dw_pcie_device_mode	mode;
	void __iomem		*reset_base;
	void __iomem		*clk_base;
	void __iomem		*apb_base;
};

static const struct of_device_id dw_plat_pcie_of_match[];

static int dw_pcie_clean_apb_reg(struct dw_plat_pcie_t *dw_plat_pcie, u32 offset, u32 val)
{
	int ret = -1;

	if (!dw_plat_pcie)
		return ret;

	ret = dw_pcie_write(dw_plat_pcie->apb_base + offset, 4, val);
	if (ret)
		dev_err(dw_plat_pcie->pci->dev, "clean apb address failed\n");

	return ret;
}

static int dw_pcie_modify_apb_reg(struct dw_plat_pcie_t *dw_plat_pcie, u32 offset, u32 val)
{
	int ret = -1;
	u32 tmp;

	if (!dw_plat_pcie)
		return ret;

	ret = dw_pcie_read(dw_plat_pcie->apb_base + offset, 4, &tmp);
	if (ret)
		dev_err(dw_plat_pcie->pci->dev, "modify apb address failed in read\n");

	ret = dw_pcie_write(dw_plat_pcie->apb_base + offset, 4, (tmp | val));
	if (ret)
		dev_err(dw_plat_pcie->pci->dev, "modify apb address failed in write\n");

	return ret;
}

static int dw_pcie_read_apb_reg(struct dw_plat_pcie_t *dw_plat_pcie, u32 offset, u32 *val)
{
	int ret;

	if (!dw_plat_pcie)
		return ret;

	ret = dw_pcie_read(dw_plat_pcie->apb_base + offset, 4, val);
	if (ret)
		dev_err(dw_plat_pcie->pci->dev, "read apb address failed\n");

	return ret;
}

static int dw_pcie_clean_rst_reg(struct dw_plat_pcie_t *dw_plat_pcie, u32 offset, u32 val)
{
	int ret = -1;

	if (!dw_plat_pcie)
		return ret;

	ret = dw_pcie_write(dw_plat_pcie->reset_base + offset, 4, val);
	if (ret)
		dev_err(dw_plat_pcie->pci->dev, "clean reset address failed\n");

	return ret;
}

static int dw_pcie_modify_rst_reg(struct dw_plat_pcie_t *dw_plat_pcie, u32 offset, u32 val)
{
	int ret = -1;
	u32 tmp;

	if (!dw_plat_pcie)
		return ret;

	ret = dw_pcie_read(dw_plat_pcie->reset_base + offset, 4, &tmp);
	if (ret)
		dev_err(dw_plat_pcie->pci->dev, "modify reset address failed in read\n");

	ret = dw_pcie_write(dw_plat_pcie->reset_base + offset, 4, (tmp | val));
	if (ret)
		dev_err(dw_plat_pcie->pci->dev, "modify reset address failed in write\n");

	return ret;
}

static int dw_pcie_read_rst_reg(struct dw_plat_pcie_t *dw_plat_pcie, u32 offset, u32 *val)
{
	int ret = -1;

	if (!dw_plat_pcie)
		return ret;

	ret = dw_pcie_read(dw_plat_pcie->reset_base + offset, 4, val);
	if (ret)
		dev_err(dw_plat_pcie->pci->dev, "read reset address failed in read\n");

	return ret;
}

static int dw_pcie_clean_clk_reg(struct dw_plat_pcie_t *dw_plat_pcie, u32 offset, u32 val)
{
	int ret = -1;

	if (!dw_plat_pcie)
		return ret;

	dw_pcie_write(dw_plat_pcie->clk_base + offset, 4, val);
	if (ret)
		dev_err(dw_plat_pcie->pci->dev, "clean clock address failed\n");

	return ret;
}

static int dw_pcie_modify_clk_reg(struct dw_plat_pcie_t *dw_plat_pcie, u32 offset, u32 val)
{
	int ret = -1;
	u32 tmp;

	if (!dw_plat_pcie)
		return ret;

	ret = dw_pcie_read(dw_plat_pcie->clk_base + offset, 4, &tmp);
	if (ret)
		dev_err(dw_plat_pcie->pci->dev, "modify clock address failed in read\n");

	ret = dw_pcie_write(dw_plat_pcie->clk_base + offset, 4, (tmp | val));
	if (ret)
		dev_err(dw_plat_pcie->pci->dev, "modify clock address failed in write\n");

	return ret;
}

static int dw_pcie_read_clk_reg(struct dw_plat_pcie_t *dw_plat_pcie, u32 offset, u32 *val)
{
	int ret = -1;

	if (!dw_plat_pcie)
		return ret;

	ret = dw_pcie_read(dw_plat_pcie->clk_base + offset, 4, val);
	if (ret)
		dev_err(dw_plat_pcie->pci->dev, "read clock address failed\n");

	return ret;
}

static int dw_plat_pcie_establish_link(struct dw_pcie *pci)
{
	u32 link_timers;
	u32 link_state;
	struct dw_plat_pcie_t *dw_plat_pcie = to_dw_plat_pcie_from_dw_pcie(pci);

	/* enable LTSSM */
	dw_pcie_modify_apb_reg(dw_plat_pcie, PCIE_GSYS_LTSSM_ENABLE, 1);
	for (link_timers = 0; link_timers < LINK_STATE_MAX_POLLING_TIME; link_timers++) {
		dw_pcie_read_apb_reg(dw_plat_pcie, PCIE_GSYS_RDLH_LINK_UP, &link_state);
		if (unlikely(link_state == 0x3)) {
			dev_dbg(pci->dev, "pcie rc link up done\n");
			break;
		}
		udelay(20);
	}
	if (unlikely(link_timers >= LINK_STATE_MAX_POLLING_TIME))
		dev_warn(pci->dev, "polling too many times, exit\n");

	dw_pcie_modify_apb_reg(dw_plat_pcie, PCIE_GSYS_LTSSM_ENABLE, 0);
	return 0;
}

static void dw_plat_set_num_vectors(struct pcie_port *pp)
{
	pp->num_vectors = MSI_DEF_NUM_VECTORS;
}

static int dw_plat_pcie_host_init(struct pcie_port *pp)
{
	struct dw_pcie *pci = to_dw_pcie_from_pp(pp);

	dw_pcie_setup_rc(pp);
	dw_pcie_wait_for_link(pci);
	dw_pcie_msi_init(pp);

	return 0;
}

static const struct dw_pcie_host_ops dw_plat_pcie_host_ops = {
	.host_init = dw_plat_pcie_host_init,
	.set_num_vectors = dw_plat_set_num_vectors,
};

static const struct dw_pcie_ops dw_pcie_ops = {
	.start_link = dw_plat_pcie_establish_link,
};

static void dw_plat_pcie_ep_init(struct dw_pcie_ep *ep)
{
	struct dw_pcie *pci = to_dw_pcie_from_ep(ep);
	enum pci_barno bar;

	for (bar = 0; bar < PCI_STD_NUM_BARS; bar++)
		dw_pcie_ep_reset_bar(pci, bar);

	/* disable rom bar */
	dw_pcie_writel_dbi2(pci, 0x30, 0);
}

int cls_pcie_ep_raise_legacy_irq(struct dw_pcie_ep *ep, u8 func_no)
{
	uint32_t val1;
	uint32_t val2;
	struct dw_pcie *pci = to_dw_pcie_from_ep(ep);
	struct device *dev = pci->dev;
	struct dw_plat_pcie_t *dw_plat_pcie = to_dw_plat_pcie_from_dw_pcie(pci);

	dev_err(dev, "cls ep trigger legacy irq\n");
	dw_pcie_modify_apb_reg(dw_plat_pcie, PCIE_GSYS_SYS_INT, 1);
	dw_pcie_clean_apb_reg(dw_plat_pcie, PCIE_GSYS_SYS_INT, 0);

	return 0;
}
EXPORT_SYMBOL(cls_pcie_ep_raise_legacy_irq);

static int dw_plat_pcie_ep_raise_irq(struct dw_pcie_ep *ep, u8 func_no,
				     enum pci_epc_irq_type type,
				     u16 interrupt_num)
{
	struct dw_pcie *pci = to_dw_pcie_from_ep(ep);

	switch (type) {
	case PCI_EPC_IRQ_LEGACY:
		return cls_pcie_ep_raise_legacy_irq(ep, func_no);
	case PCI_EPC_IRQ_MSI:
		return dw_pcie_ep_raise_msi_irq(ep, func_no, interrupt_num);
	case PCI_EPC_IRQ_MSIX:
		return dw_pcie_ep_raise_msix_irq(ep, func_no, interrupt_num);
	default:
		dev_err(pci->dev, "unknown irq type\n");
	}

	return 0;
}

static const struct pci_epc_features dw_plat_pcie_epc_features = {
	.linkup_notifier = false,
	.msi_capable = true,
	.msix_capable = false,
};

static const struct pci_epc_features*
dw_plat_pcie_get_features(struct dw_pcie_ep *ep)
{
	return &dw_plat_pcie_epc_features;
}

static const struct dw_pcie_ep_ops pcie_ep_ops = {
	.ep_init = dw_plat_pcie_ep_init,
	.raise_irq = dw_plat_pcie_ep_raise_irq,
	.get_features = dw_plat_pcie_get_features,
};

static int dw_plat_add_pcie_port(struct dw_plat_pcie_t *dw_plat_pcie,
				 struct platform_device *pdev)
{
	int ret;
	struct dw_pcie *pci = dw_plat_pcie->pci;
	struct pcie_port *pp = &pci->pp;
	struct device *dev = &pdev->dev;

	pp->irq = platform_get_irq(pdev, 1);
	if (pp->irq < 0)
		return pp->irq;

	if (IS_ENABLED(CONFIG_PCI_MSI)) {
		pp->msi_irq = platform_get_irq(pdev, 0);
		if (pp->msi_irq < 0)
			return pp->msi_irq;
	}
	dev_err(dev, "irq: %d msi_irq: %d\n", pp->irq, pp->msi_irq);

	pp->ops = &dw_plat_pcie_host_ops;

	ret = dw_pcie_host_init(pp);
	if (ret) {
		dev_err(dev, "Failed to initialize host\n");
		return ret;
	}

	return 0;
}

static int dw_plat_add_pcie_ep(struct dw_plat_pcie_t *dw_plat_pcie,
			       struct platform_device *pdev)
{
	int ret;
	struct dw_pcie_ep *ep;
	struct resource *res;
	struct device *dev = &pdev->dev;
	struct dw_pcie *pci = dw_plat_pcie->pci;

	ep = &pci->ep;
	ep->ops = &pcie_ep_ops;

	pci->dbi_base2 = pci->dbi_base + PCIE_TYPE0_HDR_DBI2_OFFSET;
	pci->atu_base = pci->dbi_base + DEFAULT_DBI_ATU_OFFSET;
	pci->iatu_unroll_enabled = 1;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "addr_space");
	if (!res)
		return -EINVAL;

	ep->phys_base = res->start;
	ep->addr_size = resource_size(res);

	ret = dw_pcie_ep_init(ep);
	if (ret) {
		dev_err(dev, "Failed to initialize endpoint\n");
		return ret;
	}

	return 0;
}

u32 dw_plat_serdes_reg_read(struct dw_pcie *pci, u32 reg)
{
	int cnt = 0;
	u32 val;
	struct dw_plat_pcie_t *dw_plat_pcie = to_dw_plat_pcie_from_dw_pcie(pci);

	dw_pcie_clean_apb_reg(dw_plat_pcie, PCIE_PHY_SUP_DIG_MPLLA_OVRD_IN_0, 0);
	if (!!reg)
		dw_pcie_modify_apb_reg(dw_plat_pcie, PCIE_PHY_SUP_DIG_MPLLA_OVRD_IN_0, reg);
	else
		dw_pcie_clean_apb_reg(dw_plat_pcie, PCIE_PHY_SUP_DIG_MPLLA_OVRD_IN_0, 0);

	dw_pcie_modify_apb_reg(dw_plat_pcie, PCIE_PHY_SUP_DIG_SUP_OVRD_IN_2, 0x2);

	do {
		dw_pcie_read_apb_reg(dw_plat_pcie, PCIE_PHY_SUP_DIG_IDCODE_LO, &val);
		if (val & 0x2) {
			dw_pcie_modify_apb_reg(dw_plat_pcie, PCIE_PHY_SUP_DIG_IDCODE_LO, 0x2);
			break;
		}
		if (cnt++ > SERDES_REG_POLLING_TIMES) {
			dev_err(pci->dev, "polling serdes register timeout\n");
			return LINK_UP_TIMER_OUT;
		}
	} while (1);

	dw_pcie_read_apb_reg(dw_plat_pcie, PCIE_PHY_SUP_DIG_MPLLB_OVRD_IN_3, &val);

	return val;
}
EXPORT_SYMBOL(dw_plat_serdes_reg_read);

int dw_plat_serdes_reg_write(struct dw_pcie *pci, u32 reg, u32 val)
{
	int cnt = 0;
	int state;
	struct dw_plat_pcie_t *dw_plat_pcie = to_dw_plat_pcie_from_dw_pcie(pci);

	dw_pcie_clean_apb_reg(dw_plat_pcie, PCIE_PHY_SUP_DIG_MPLLA_OVRD_IN_0, 0);
	dw_pcie_clean_apb_reg(dw_plat_pcie, PCIE_PHY_SUP_SUP_DIG_MPLLA_OVRD_IN_4, 0);
	if (!!reg)
		dw_pcie_modify_apb_reg(dw_plat_pcie, PCIE_PHY_SUP_DIG_MPLLA_OVRD_IN_0, reg);
	else
		dw_pcie_clean_apb_reg(dw_plat_pcie, PCIE_PHY_SUP_DIG_MPLLA_OVRD_IN_0, 0);
	if (!!val)
		dw_pcie_modify_apb_reg(dw_plat_pcie, PCIE_PHY_SUP_SUP_DIG_MPLLA_OVRD_IN_4, val);
	else
		dw_pcie_clean_apb_reg(dw_plat_pcie, PCIE_PHY_SUP_SUP_DIG_MPLLA_OVRD_IN_4, 0);
	dw_pcie_modify_apb_reg(dw_plat_pcie, PCIE_PHY_SUP_DIG_SUP_OVRD_IN_2, 0x1);

	do {
		dw_pcie_read_apb_reg(dw_plat_pcie, PCIE_PHY_SUP_DIG_IDCODE_LO, &state);
		if (state & 0x1) {
			dw_pcie_modify_apb_reg(dw_plat_pcie, PCIE_PHY_SUP_DIG_IDCODE_LO, 0x1);
			break;
		}
		if (cnt++ > SERDES_REG_POLLING_TIMES) {
			dev_err(pci->dev, "polling serdes register timeout\n");
			return LINK_UP_TIMER_OUT;
		}
	} while (1);

	return 0;
}
EXPORT_SYMBOL(dw_plat_serdes_reg_write);

void config_gen3_gen4_eq_presets(struct dw_plat_pcie_t *dw_plat_pcie)
{
	struct dw_pcie *pci = dw_plat_pcie->pci;
	u32 val, offset, unit;

	/* Program init preset */
	for (unit = 0; unit < 1; unit++) {
		val = dw_pcie_readw_dbi(pci, CAP_SPCIE_CAP_OFF + (unit * 2));
		val &= ~CAP_SPCIE_CAP_OFF_DSP_TX_PRESET0_MASK;
		val |= GEN3_GEN4_EQ_PRESET_INIT;
		val &= ~CAP_SPCIE_CAP_OFF_USP_TX_PRESET0_MASK;
		val |= (GEN3_GEN4_EQ_PRESET_INIT <<
			   CAP_SPCIE_CAP_OFF_USP_TX_PRESET0_SHIFT);
		dw_pcie_writew_dbi(pci, CAP_SPCIE_CAP_OFF + (unit * 2), val);

		offset = dw_pcie_find_ext_capability(pci,
						     PCI_EXT_CAP_ID_PL_16GT) +
				PCI_PL_16GT_LE_CTRL;
		val = dw_pcie_readb_dbi(pci, offset + unit);
		val &= ~PCI_PL_16GT_LE_CTRL_DSP_TX_PRESET_MASK;
		val |= GEN3_GEN4_EQ_PRESET_INIT;
		val &= ~PCI_PL_16GT_LE_CTRL_USP_TX_PRESET_MASK;
		val |= (GEN3_GEN4_EQ_PRESET_INIT <<
			PCI_PL_16GT_LE_CTRL_USP_TX_PRESET_SHIFT);
		dw_pcie_writeb_dbi(pci, offset + unit, val);
	}

	val = dw_pcie_readl_dbi(pci, GEN3_RELATED_OFF);
	val &= ~GEN3_RELATED_OFF_RATE_SHADOW_SEL_MASK;
	dw_pcie_writel_dbi(pci, GEN3_RELATED_OFF, val);

	val = dw_pcie_readl_dbi(pci, GEN3_EQ_CONTROL_OFF);
	val &= ~GEN3_EQ_CONTROL_OFF_PSET_REQ_VEC_MASK;
	val |= (0x3ff << GEN3_EQ_CONTROL_OFF_PSET_REQ_VEC_SHIFT);
	val &= ~GEN3_EQ_CONTROL_OFF_FB_MODE_MASK;
	dw_pcie_writel_dbi(pci, GEN3_EQ_CONTROL_OFF, val);

	val = dw_pcie_readl_dbi(pci, GEN3_RELATED_OFF);
	val &= ~GEN3_RELATED_OFF_RATE_SHADOW_SEL_MASK;
	val |= (0x1 << GEN3_RELATED_OFF_RATE_SHADOW_SEL_SHIFT);
	dw_pcie_writel_dbi(pci, GEN3_RELATED_OFF, val);

	val = dw_pcie_readl_dbi(pci, GEN3_EQ_CONTROL_OFF);
	val &= ~GEN3_EQ_CONTROL_OFF_PSET_REQ_VEC_MASK;
	val |= (0x360 << GEN3_EQ_CONTROL_OFF_PSET_REQ_VEC_SHIFT);
	val &= ~GEN3_EQ_CONTROL_OFF_FB_MODE_MASK;
	dw_pcie_writel_dbi(pci, GEN3_EQ_CONTROL_OFF, val);

	val = dw_pcie_readl_dbi(pci, GEN3_RELATED_OFF);
	val &= ~GEN3_RELATED_OFF_RATE_SHADOW_SEL_MASK;
	dw_pcie_writel_dbi(pci, GEN3_RELATED_OFF, val);
}
EXPORT_SYMBOL(config_gen3_gen4_eq_presets);

irqreturn_t pcie_irq_handler(int irq, void *dev)
{
	uint32_t uncorr_err_status;
	uint32_t uncorr_err_mask;
	uint32_t corr_err_status;
	uint32_t corr_err_mask;
	uint32_t root_err_status;
	struct dw_pcie *pci;
	struct dw_plat_pcie_t *dw_plat_pcie;

	dw_plat_pcie = platform_get_drvdata(to_platform_device(dev));
	pci = dw_plat_pcie->pci;
	uncorr_err_status = dw_pcie_readl_dbi(pci, PCIE_UNCORR_ERR_STARUS_OFFSET);
	uncorr_err_mask = dw_pcie_readl_dbi(pci, PCIE_UNCORR_ERR_MASK_OFFSET);
	corr_err_status = dw_pcie_readl_dbi(pci, PCIE_CORR_ERR_STARUS_OFFSET);
	corr_err_mask = dw_pcie_readl_dbi(pci, PCIE_CORR_ERR_MASK_OFFSET);
	root_err_status = dw_pcie_readl_dbi(pci, PCIE_ROOT_ERR_STARUS_OFFSET);
	pr_info(">>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<\n");
	pr_info(">>>pcie link error happened: [irq = %x]<<<\n", irq);
	pr_info(">>>[reg = %x] : [uncorr_err_status = %x]<<<\n", PCIE_UNCORR_ERR_STARUS_OFFSET, uncorr_err_status);
	pr_info(">>>[reg = %x] : [uncorr_err_mask = %x]<<<\n", PCIE_UNCORR_ERR_MASK_OFFSET, uncorr_err_mask);
	pr_info(">>>[reg = %x] : [corr_err_status = %x]<<<\n", PCIE_CORR_ERR_STARUS_OFFSET, corr_err_status);
	pr_info(">>>[reg = %x] : [corr_err_mask = %x]<<<\n", PCIE_CORR_ERR_MASK_OFFSET, corr_err_mask);
	pr_info(">>>[reg = %x] : [root_err_status = %x]<<<\n", PCIE_ROOT_ERR_STARUS_OFFSET, root_err_status);
	pr_info(">>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<\n");
	return IRQ_HANDLED;
}

static void set_pcie_link_error_interrput_enable(struct dw_pcie *pci)
{
	dw_pcie_write_dbi(pci, PCIE_ERR_REPORT_OFFSET, 4, PCIE_ERR_REPORT_EN_VAL);
	dw_pcie_write_dbi(pci, PCIE_ROOT_ERR_CMD_OFFSET, 4, PCIE_ROOT_ERR_CMD_EN_VAL);
}

static int pcie_get_platform_resource(struct platform_device *pdev,
			struct dw_pcie *pci,
			struct dw_plat_pcie_t *dw_plat_pcie)
{
	uint32_t val;
	struct resource *irq;
	struct resource *dbi_res;
	struct resource *rst_res;
	struct resource *clk_res;
	struct resource *apb_res;
	struct device *dev = &pdev->dev;

	if (!pdev)
		return -ENOMEM;

	if (!dev)
		return -ENOMEM;

	if (!pci)
		return -ENOMEM;

	if (!dw_plat_pcie)
		return -ENOMEM;

	dbi_res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "dbi");
	pci->dbi_base = devm_ioremap_resource(dev, dbi_res);
	if (IS_ERR(pci->dbi_base))
		return PTR_ERR(pci->dbi_base);
	if (dbi_res)
		dev_dbg(dev, "DBI %pR, vaddr=%lx\n", dbi_res, (unsigned long)pci->dbi_base);

	rst_res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "reset");
	dw_plat_pcie->reset_base = devm_ioremap_resource(dev, rst_res);
	if (IS_ERR(dw_plat_pcie->reset_base))
		return PTR_ERR(dw_plat_pcie->reset_base);
	if (rst_res)
		dev_dbg(dev, "RST %pR, vaddr=%lx\n", rst_res, (unsigned long)dw_plat_pcie->reset_base);

	clk_res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "clk");
	dw_plat_pcie->clk_base = devm_ioremap_resource(dev, clk_res);
	if (IS_ERR(dw_plat_pcie->clk_base))
		return PTR_ERR(dw_plat_pcie->clk_base);
	if (clk_res)
		dev_dbg(dev, "CLK %pR, vaddr=%lx\n", clk_res, (unsigned long)dw_plat_pcie->clk_base);

	apb_res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "apb_base");
	dw_plat_pcie->apb_base = devm_ioremap_resource(dev, apb_res);
	if (IS_ERR(dw_plat_pcie->apb_base))
		return PTR_ERR(dw_plat_pcie->apb_base);
	if (apb_res)
		dev_dbg(dev, "APB %pR, vaddr=%lx\n", apb_res, (unsigned long)dw_plat_pcie->apb_base);

	/* switch to 100MHz ref clock */
	//dw_pcie_modify_clk_reg(dw_plat_pcie, PCIE_CLK_SEL_PARA, 1);
	//dw_pcie_read_rst_reg(dw_plat_pcie, PCIE_PERI_SUB_RST_PARA0, &val);

	if (dw_plat_pcie->mode == DW_PCIE_EP_TYPE)
		val |= BIT(1);
	else
		val |= BIT(2);

	if (!!val)
		dw_pcie_modify_apb_reg(dw_plat_pcie, PCIE_GSYS_PHY_CLK_CONTROL, val);
	else
		dw_pcie_clean_apb_reg(dw_plat_pcie, PCIE_GSYS_PHY_CLK_CONTROL, 0);
	dw_pcie_modify_rst_reg(dw_plat_pcie, PCIE_PERI_SUB_RST_PARA0, 0x7);

	platform_set_drvdata(pdev, dw_plat_pcie);

	/* The mapping must be removed manually because it conflicts with the NPE */
	if (dw_plat_pcie->reset_base) {
		devm_iounmap(dev, dw_plat_pcie->reset_base);
		/* Manual resource release */
		devm_release_mem_region(dev, rst_res->start, resource_size(rst_res));
	}

	/* The mapping must be removed manually because it conflicts with the others */
	if (dw_plat_pcie->clk_base) {
		devm_iounmap(dev, dw_plat_pcie->clk_base);
		/* Manual resource release */
		devm_release_mem_region(dev, clk_res->start, resource_size(clk_res));
	}

	irq = platform_get_resource(pdev, IORESOURCE_IRQ, 2);
	if (!irq) {
		dev_err(&pdev->dev, "irq resource for pcie\n");
		return -ENXIO;
	}

	if (devm_request_irq(&pdev->dev, irq->start, pcie_irq_handler, IRQF_SHARED, dev_name(&pdev->dev), &pdev->dev)) {
		dev_err(&pdev->dev, "failed to request IRQ\n");
		return -EBUSY;
	}

	return 0;
}

static int dw_plat_pcie_probe(struct platform_device *pdev)
{
	int ret;
	int perst_gpio;
	struct dw_pcie *pci;
	struct device *dev = &pdev->dev;
	struct dw_plat_pcie_t *dw_plat_pcie;
	const struct of_device_id *match;
	const struct dw_plat_pcie_of_data *data;
	struct fwnode_handle *fwnode = &dev->of_node->fwnode;

	match = of_match_device(dw_plat_pcie_of_match, dev);
	if (!match) {
		dev_err(dev, "of match device fail\n");
		return -EINVAL;
	}

	data = (struct dw_plat_pcie_of_data *)match->data;

	dw_plat_pcie = devm_kzalloc(dev, sizeof(struct dw_plat_pcie_t), GFP_KERNEL);
	if (!dw_plat_pcie)
		return -ENOMEM;

	pci = devm_kzalloc(dev, sizeof(*pci), GFP_KERNEL);
	if (!pci)
		return -ENOMEM;

	pci->dev = dev;
	pci->ops = &dw_pcie_ops;

	dw_plat_pcie->pci = pci;
	dw_plat_pcie->mode = (enum dw_pcie_device_mode)data->mode;
	pci->dw_plat_pcie = dw_plat_pcie;
	ret = pcie_get_platform_resource(pdev, pci, dw_plat_pcie);
	if (ret < 0) {
		dev_err(dev, "pcie get platform resources\n");
		return ret;
	}

	udelay(100);
	switch (dw_plat_pcie->mode) {
	case DW_PCIE_RC_TYPE:
		if (!IS_ENABLED(CONFIG_PCIE_DW_PLAT_HOST))
			return -ENODEV;
		perst_gpio = of_get_named_gpio(to_of_node(fwnode), "perst-gpio", 0);
		if (perst_gpio >= 0) {
			dev_dbg(dev, "perst_gpio: %d\n", perst_gpio);
			gpio_direction_output(perst_gpio, 0);
			gpio_set_value(perst_gpio, 0);
			udelay(100);
			gpio_set_value(perst_gpio, 1);
		}
		ret = dw_plat_pcie_establish_link(pci);
		if (ret < 0) {
			dev_err(dev, "dw plat pcie establish link fail: %d\n", ret);
			return ret;
		}

		ret = dw_plat_add_pcie_port(dw_plat_pcie, pdev);
		if (ret < 0) {
			dev_err(dev, "dw plat add pcie port fail: %d\n", ret);
			return ret;
		}
		break;
	case DW_PCIE_EP_TYPE:
		if (!IS_ENABLED(CONFIG_PCIE_DW_PLAT_HOST))
			return -ENODEV;

		dw_pcie_clean_apb_reg(dw_plat_pcie, PCIE_GSYS_PCIE_DEVICE_TYPE, 0x0);
		udelay(30);

		ret = dw_plat_add_pcie_ep(dw_plat_pcie, pdev);
		if (ret < 0)
			return ret;
		break;
	default:
		dev_err(dev, "INVALID device type %d\n", dw_plat_pcie->mode);
	}

	set_pcie_link_error_interrput_enable(pci);

	return 0;
}

static const struct dw_plat_pcie_of_data dw_plat_pcie_rc_of_data = {
	.mode = DW_PCIE_RC_TYPE,
};

static const struct dw_plat_pcie_of_data dw_plat_pcie_ep_of_data = {
	.mode = DW_PCIE_EP_TYPE,
};

static const struct of_device_id dw_plat_pcie_of_match[] = {
	{
		.compatible = "clourney,cls-pcie-rc",
		.data = &dw_plat_pcie_rc_of_data,
	},
	{
		.compatible = "clourney,cls-pcie-ep",
		.data = &dw_plat_pcie_ep_of_data,
	},
	{},
};

static struct platform_driver dw_plat_pcie_driver = {
	.driver = {
		.name	= "clourney,cls-pcie-rc",
		.of_match_table = dw_plat_pcie_of_match,
		.suppress_bind_attrs = true,
	},
	.probe = dw_plat_pcie_probe,
};
builtin_platform_driver(dw_plat_pcie_driver);
