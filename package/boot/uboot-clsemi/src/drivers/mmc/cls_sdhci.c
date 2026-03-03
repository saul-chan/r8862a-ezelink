/*
 *  Copyright (c) 2021-2024, Clourney Semiconductor. All rights reserved.
 *
 *  This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
 *  Reproduction and redistribution in binary or source form, with or without modification,
 *  for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
 *  must retain the above copyright notice.
 *
 *  By using this software and/or documentation, you agree to the limited terms and conditions.
 *
 *  Clourney Semiconductor SD Host Controller Interface
 *
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <dm/ofnode.h>
#include <dm/device_compat.h>
#include <dt-structs.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/libfdt.h>
#include <linux/iopoll.h>
#include <malloc.h>
#include <mapmem.h>
#include "mmc_private.h"
#include <sdhci.h>
#include <syscon.h>

#define CPU_TOP_SYS_BASE	0x90000000
#define SDEMMC_DELAY_LV2	(0x500 + CPU_TOP_SYS_BASE)
#define SDEMMC_DELAY_LV1	(0x504 + CPU_TOP_SYS_BASE)
#define TAP_MID(start, len)	((start) + ((len - 1) / 2))

/* 400KHz is max freq for card ID etc. Use that as min */
#define EMMC_MIN_FREQ	400000
#define KHz	(1000)
#define MHz	(1000 * KHz)
#define SDHCI_TUNING_LOOP_COUNT		40

struct clourney_sdhc_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

struct clourney_sdhc {
	struct sdhci_host host;
	struct udevice *dev;
	u32 f_max;
};

static int clourney_sdhc_clk_setup(struct udevice *dev)
{
        struct clourney_sdhc *priv = dev_get_priv(dev);
        struct sdhci_host *host = &priv->host;

        struct clk clk_bus;
        int ret;

        ret = clk_get_by_name(dev, "bus", &clk_bus);
        if (ret)
                goto clk_err;

        host->max_clk = clk_get_rate(&clk_bus);
        if (host->max_clk < EMMC_MIN_FREQ) {
                ret = -EINVAL;
                goto clk_err;
        }

        return 0;

clk_err:
        dev_err(dev, "failed to setup clocks, ret %d\n", ret);

        return ret;
}

static int clourney_sdhci_set_ios_post(struct sdhci_host *host)
{
	struct clourney_sdhc *priv = container_of(host, struct clourney_sdhc, host);
	struct mmc *mmc = host->mmc;
	uint clock = mmc->tran_speed;
	u32 reg;

	if (!clock)
		clock = mmc->clock;

	if (mmc->selected_mode == MMC_HS_400 || mmc->selected_mode == MMC_HS_400_ES) {
		reg = sdhci_readw(host, SDHCI_HOST_CONTROL2);
		reg &= ~SDHCI_CTRL_UHS_MASK;
		reg |= SDHCI_CTRL_HS400;
		sdhci_writew(host, reg, SDHCI_HOST_CONTROL2);
	} else {
		sdhci_set_uhs_timing(host);
	}

	return 0;
}

static void set_tap_value(uint32_t tap)
{
    uint32_t val_lv2 = (tap - 1) / 8;
    uint32_t val_lv1 = (tap - 1) % 8;

    writel(val_lv2, SDEMMC_DELAY_LV2);  // 设置区间
    writel(val_lv1, SDEMMC_DELAY_LV1);  // 设置具体拍数
}

static int clourney_sdhci_execute_tuning(struct mmc *mmc, u8 opcode)
{
	struct sdhci_host *host = dev_get_priv(mmc->dev);
	int best_start = -1, best_len = 0;
	int start = -1, len = 0;
	char tuning_loop = SDHCI_TUNING_LOOP_COUNT;

	// 2. 扫描所有可能的tap值
	for (int tap = 1; tap <= tuning_loop; tap++) {
		// 设置当前tap值
		set_tap_value(tap);

		// 发送CMD21 tuning命令
		if (mmc_send_tuning(mmc, opcode, NULL)) {
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
			if (start == -1)
				start = tap;
			len++;
		}
	}

	// 3. 确定最佳窗口
	if (best_len == 0 && start != -1) {
		best_start = start;
		best_len = len;
	}

	if (best_len <= 0) {
		printf("%s failed: no valid window found\n", __func__);
		return -EIO;
	}

	// 4. 选择窗口中间值
	int best_tap = TAP_MID(best_start,best_len);
	set_tap_value(best_tap);

	// 5. 验证最终结果
	return mmc_send_tuning(mmc, opcode, NULL);
}

/* Clounrney SDHCI Controler not support execute_tuning */
static struct sdhci_ops clourney_sdhci_ops = {
	.set_ios_post	= clourney_sdhci_set_ios_post,
	.platform_execute_tuning = &clourney_sdhci_execute_tuning,
};

static int clourney_sdhci_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct clourney_sdhc_plat *plat = dev_get_plat(dev);
	struct clourney_sdhc *prv = dev_get_priv(dev);
	struct mmc_config *cfg = &plat->cfg;
	struct sdhci_host *host = &prv->host;
	unsigned int clock_max;
	int ret;

	ret = clourney_sdhc_clk_setup(dev);
	if (ret)
		return ret;

	if (!prv->f_max)
		clock_max = host->max_clk;
	else
		clock_max = min_t(unsigned int, host->max_clk, prv->f_max);

	prv->dev = dev;

	host->ops = &clourney_sdhci_ops;
	host->quirks = SDHCI_QUIRK_WAIT_SEND_CMD;

	host->mmc = &plat->mmc;
	host->mmc->priv = &prv->host;
	host->mmc->dev = dev;
	upriv->mmc = host->mmc;

	ret = sdhci_setup_cfg(cfg, host, clock_max, EMMC_MIN_FREQ);
	if (ret)
		return ret;

	return sdhci_probe(dev);
}

static int clourney_sdhci_of_to_plat(struct udevice *dev)
{
        struct clourney_sdhc *priv = dev_get_priv(dev);
        struct sdhci_host *host = &priv->host;
	struct clourney_sdhc_plat *plat = dev_get_plat(dev);
	struct mmc_config *cfg = &plat->cfg;
	int ret;

	host->name = dev->name;
	host->ioaddr = dev_read_addr_ptr(dev);

	/*
	 * If max-frequency is unset don't set cfg->f_max - we will use
	 * host->max_clk in probe() instead.
	 */
	ret = dev_read_u32(dev, "max-frequency", &priv->f_max);
	if (!ret && priv->f_max < EMMC_MIN_FREQ)
		return -EINVAL;

	ret = mmc_of_parse(dev, cfg);
	if (ret)
		return ret;

	return 0;
}

static int clourney_sdhci_bind(struct udevice *dev)
{
	struct clourney_sdhc_plat *plat = dev_get_plat(dev);

	return sdhci_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct udevice_id sdhci_ids[] = {
	{
		.compatible = "clourney,dwcmshc-sdhci",
	},
	{ }
};

U_BOOT_DRIVER(sdhci_dwcmshc_driver) = {
	.name		= "sdhci-dwcmshc",
	.id		= UCLASS_MMC,
	.of_match	= sdhci_ids,
	.of_to_plat	= clourney_sdhci_of_to_plat,
	.ops		= &sdhci_ops,
	.bind		= clourney_sdhci_bind,
	.probe		= clourney_sdhci_probe,
	.priv_auto	= sizeof(struct clourney_sdhc),
	.plat_auto	= sizeof(struct clourney_sdhc_plat),
};
