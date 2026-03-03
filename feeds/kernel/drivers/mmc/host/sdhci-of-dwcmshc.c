// SPDX-License-Identifier: GPL-2.0
/*
 * Driver for Synopsys DesignWare Cores Mobile Storage Host Controller
 *
 * Copyright (C) 2018 Synaptics Incorporated
 *
 * Author: Jisheng Zhang <jszhang@kernel.org>
 */

#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/sizes.h>
#include <linux/delay.h>
#include "sdhci-pltfm.h"

void __iomem *sdhci_rxdly_base;
#define SDHCI_DWCMSHC_RXDLY_LV2		0
#define SDHCI_DWCMSHC_RXDLY_LV1		4
#define SDHCI_TUNING_LOOP_COUNT		15
#define TAP_STEP			1
#define TAP_MID(start, len)		((start) + ((len - 1) / 2))

#define SDHCI_DWCMSHC_ARG2_STUFF	GENMASK(31, 16)

/* DWCMSHC specific Mode Select value */
#define DWCMSHC_CTRL_HS400		0x7

#define BOUNDARY_OK(addr, len) \
	((addr | (SZ_128M - 1)) == ((addr + len - 1) | (SZ_128M - 1)))

struct dwcmshc_priv {
	struct clk	*bus_clk;
};

static void dwcmshc_set_tap_value(uint32_t tap)
{
	uint32_t val_lv2 = (tap - 1) / 8;
	uint32_t val_lv1 = (tap - 1) % 8;

	writel(val_lv2, sdhci_rxdly_base + SDHCI_DWCMSHC_RXDLY_LV2);  // 设置区间
	writel(val_lv1, sdhci_rxdly_base + SDHCI_DWCMSHC_RXDLY_LV1);  // 设置具体拍数
}

static int dwcmshc_sw_execute_tuning(struct sdhci_host *host, u32 opcode)
{
	int best_start = -1, best_len = 0;
	int start = -1, len = 0;
	int tap, best_tap, ret = 0;
	char tuning_loop = SDHCI_TUNING_LOOP_COUNT;

	// 2. 扫描所有可能的tap值
	for (tap = 1; tap <= tuning_loop; tap += TAP_STEP) {
		// 设置当前tap值
		dwcmshc_set_tap_value(tap);

		// 发送CMD21 tuning命令
		if (mmc_send_tuning(host->mmc, opcode, NULL)) {
			// 失败
			if (start != -1) {
				if (len > best_len) {
					best_len = len;
					best_start = start;
				}
				break;
			}
		} else {
			// 成功
			dev_dbg(mmc_dev(host->mmc), "mmc_send_tuning ok tap(%d)\n", tap);
			if (start == -1)
				start = tap;
			len += TAP_STEP;
		}

		if (host->tuning_delay > 0)
			mdelay(host->tuning_delay);
		else
			mdelay(1);
	}

	// 3. 确定最佳窗口
	if (best_len == 0 && start != -1) {
		best_start = start;
		best_len = len;
	}

	if (best_len <= 0) {
		dev_err(mmc_dev(host->mmc),
			"%s failed: no valid window found\n", __func__);
		dwcmshc_set_tap_value(0);
		return -EIO;
	}

	// 4. 选择窗口中间值
	best_tap = TAP_MID(best_start, best_len);
	dwcmshc_set_tap_value(best_tap);
	dev_dbg(mmc_dev(host->mmc), "dwcmshc_set_tap_value best_tap(%d)\n", best_tap);

	// 5. 验证最终结果
	ret = mmc_send_tuning(host->mmc, opcode, NULL);
	if (ret)
		dev_err(mmc_dev(host->mmc),
			"mmc_send_tuning with best_tap(%d) fail(%d)\n",
			best_tap, ret);
	return ret;
}

/*
 * If DMA addr spans 128MB boundary, we split the DMA transfer into two
 * so that each DMA transfer doesn't exceed the boundary.
 */
static void dwcmshc_adma_write_desc(struct sdhci_host *host, void **desc,
				    dma_addr_t addr, int len, unsigned int cmd)
{
	int tmplen, offset;

	if (likely(!len || BOUNDARY_OK(addr, len))) {
		sdhci_adma_write_desc(host, desc, addr, len, cmd);
		return;
	}

	offset = addr & (SZ_128M - 1);
	tmplen = SZ_128M - offset;
	sdhci_adma_write_desc(host, desc, addr, tmplen, cmd);

	addr += tmplen;
	len -= tmplen;
	sdhci_adma_write_desc(host, desc, addr, len, cmd);
}

static void dwcmshc_check_auto_cmd23(struct mmc_host *mmc,
				     struct mmc_request *mrq)
{
	struct sdhci_host *host = mmc_priv(mmc);

	/*
	 * No matter V4 is enabled or not, ARGUMENT2 register is 32-bit
	 * block count register which doesn't support stuff bits of
	 * CMD23 argument on dwcmsch host controller.
	 */
	if (mrq->sbc && (mrq->sbc->arg & SDHCI_DWCMSHC_ARG2_STUFF))
		host->flags &= ~SDHCI_AUTO_CMD23;
	else
		host->flags |= SDHCI_AUTO_CMD23;
}

static void dwcmshc_request(struct mmc_host *mmc, struct mmc_request *mrq)
{
	dwcmshc_check_auto_cmd23(mmc, mrq);

	sdhci_request(mmc, mrq);
}

static void dwcmshc_set_uhs_signaling(struct sdhci_host *host,
				      unsigned int timing)
{
	u16 ctrl_2;

	ctrl_2 = sdhci_readw(host, SDHCI_HOST_CONTROL2);
	/* Select Bus Speed Mode for host */
	ctrl_2 &= ~SDHCI_CTRL_UHS_MASK;
	if ((timing == MMC_TIMING_MMC_HS200) ||
	    (timing == MMC_TIMING_UHS_SDR104))
		ctrl_2 |= SDHCI_CTRL_UHS_SDR104;
	else if (timing == MMC_TIMING_UHS_SDR12)
		ctrl_2 |= SDHCI_CTRL_UHS_SDR12;
	else if ((timing == MMC_TIMING_UHS_SDR25) ||
		 (timing == MMC_TIMING_MMC_HS))
		ctrl_2 |= SDHCI_CTRL_UHS_SDR25;
	else if (timing == MMC_TIMING_UHS_SDR50)
		ctrl_2 |= SDHCI_CTRL_UHS_SDR50;
	else if ((timing == MMC_TIMING_UHS_DDR50) ||
		 (timing == MMC_TIMING_MMC_DDR52))
		ctrl_2 |= SDHCI_CTRL_UHS_DDR50;
	else if (timing == MMC_TIMING_MMC_HS400)
		ctrl_2 |= DWCMSHC_CTRL_HS400;
	sdhci_writew(host, ctrl_2, SDHCI_HOST_CONTROL2);
}

static const struct sdhci_ops sdhci_dwcmshc_ops = {
	.set_clock		= sdhci_set_clock,
	.set_bus_width		= sdhci_set_bus_width,
	.set_uhs_signaling	= dwcmshc_set_uhs_signaling,
	.get_max_clock		= sdhci_pltfm_clk_get_max_clock,
	.reset			= sdhci_reset,
	.adma_write_desc	= dwcmshc_adma_write_desc,
	.platform_execute_tuning = dwcmshc_sw_execute_tuning,
};

static const struct sdhci_pltfm_data sdhci_dwcmshc_pdata = {
	.ops = &sdhci_dwcmshc_ops,
	/* bringup stage, no clk source in dts*/
	.quirks = SDHCI_QUIRK_CAP_CLOCK_BASE_BROKEN |
		SDHCI_QUIRK_BROKEN_TIMEOUT_VAL |
		SDHCI_QUIRK_32BIT_DMA_ADDR |
		SDHCI_QUIRK_32BIT_DMA_SIZE |
		SDHCI_QUIRK_32BIT_ADMA_SIZE |
		SDHCI_QUIRK_BROKEN_ADMA_ZEROLEN_DESC,
	.quirks2 = SDHCI_QUIRK2_PRESET_VALUE_BROKEN |
		SDHCI_QUIRK2_USE_32BIT_BLK_CNT,
};

static int dwcmshc_probe(struct platform_device *pdev)
{
	struct sdhci_pltfm_host *pltfm_host;
	struct sdhci_host *host;
	struct dwcmshc_priv *priv;
	struct resource *sdhci_rxdly_res;
	int err;
	u32 extra;

	host = sdhci_pltfm_init(pdev, &sdhci_dwcmshc_pdata,
				sizeof(struct dwcmshc_priv));
	if (IS_ERR(host))
		return PTR_ERR(host);

	sdhci_rxdly_res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	sdhci_rxdly_base = devm_ioremap_resource(&pdev->dev, sdhci_rxdly_res);
	if (IS_ERR(sdhci_rxdly_base))
		return PTR_ERR(sdhci_rxdly_base);
	/*
	 * extra adma table cnt for cross 128M boundary handling.
	 */
	extra = DIV_ROUND_UP_ULL(dma_get_required_mask(&pdev->dev), SZ_128M);
	if (extra > SDHCI_MAX_SEGS)
		extra = SDHCI_MAX_SEGS;
	host->adma_table_cnt += extra;

	pltfm_host = sdhci_priv(host);
	priv = sdhci_pltfm_priv(pltfm_host);

	/* bringup stage, no clk source in dts*/
	pltfm_host->clk = devm_clk_get(&pdev->dev, "core");
	if (IS_ERR(pltfm_host->clk)) {
		err = PTR_ERR(pltfm_host->clk);
		dev_err(&pdev->dev, "failed to get core clk: %d\n", err);
		goto free_pltfm;
	}
	err = clk_prepare_enable(pltfm_host->clk);
	if (err)
		goto free_pltfm;

	priv->bus_clk = devm_clk_get(&pdev->dev, "bus");
	if (!IS_ERR(priv->bus_clk))
		clk_prepare_enable(priv->bus_clk);

	err = mmc_of_parse(host->mmc);
	if (err)
		goto err_clk;

	sdhci_get_of_property(pdev);

	host->mmc_host_ops.request = dwcmshc_request;

	err = sdhci_add_host(host);
	if (err)
		goto err_clk;

	return 0;

err_clk:
	clk_disable_unprepare(pltfm_host->clk);
	clk_disable_unprepare(priv->bus_clk);
free_pltfm:
	sdhci_pltfm_free(pdev);
	return err;
}

static int dwcmshc_remove(struct platform_device *pdev)
{
	struct sdhci_host *host = platform_get_drvdata(pdev);
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct dwcmshc_priv *priv = sdhci_pltfm_priv(pltfm_host);

	sdhci_remove_host(host, 0);

	clk_disable_unprepare(pltfm_host->clk);
	clk_disable_unprepare(priv->bus_clk);

	sdhci_pltfm_free(pdev);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int dwcmshc_suspend(struct device *dev)
{
	struct sdhci_host *host = dev_get_drvdata(dev);
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct dwcmshc_priv *priv = sdhci_pltfm_priv(pltfm_host);
	int ret;

	ret = sdhci_suspend_host(host);
	if (ret)
		return ret;

	clk_disable_unprepare(pltfm_host->clk);
	if (!IS_ERR(priv->bus_clk))
		clk_disable_unprepare(priv->bus_clk);

	return ret;
}

static int dwcmshc_resume(struct device *dev)
{
	struct sdhci_host *host = dev_get_drvdata(dev);
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct dwcmshc_priv *priv = sdhci_pltfm_priv(pltfm_host);
	int ret;

	ret = clk_prepare_enable(pltfm_host->clk);
	if (ret)
		return ret;

	if (!IS_ERR(priv->bus_clk)) {
		ret = clk_prepare_enable(priv->bus_clk);
		if (ret)
			return ret;
	}

	return sdhci_resume_host(host);
}
#endif

static SIMPLE_DEV_PM_OPS(dwcmshc_pmops, dwcmshc_suspend, dwcmshc_resume);

static const struct of_device_id sdhci_dwcmshc_dt_ids[] = {
	{ .compatible = "snps,dwcmshc-sdhci" },
	{ .compatible = "clourney, dwcmshc-sdhci"},
	{}
};
MODULE_DEVICE_TABLE(of, sdhci_dwcmshc_dt_ids);

static struct platform_driver sdhci_dwcmshc_driver = {
	.driver	= {
		.name	= "sdhci-dwcmshc",
		.probe_type = PROBE_PREFER_ASYNCHRONOUS,
		.of_match_table = sdhci_dwcmshc_dt_ids,
		.pm = &dwcmshc_pmops,
	},
	.probe	= dwcmshc_probe,
	.remove	= dwcmshc_remove,
};
module_platform_driver(sdhci_dwcmshc_driver);

MODULE_DESCRIPTION("SDHCI platform driver for Synopsys DWC MSHC");
MODULE_AUTHOR("Jisheng Zhang <jszhang@kernel.org>");
MODULE_LICENSE("GPL v2");
