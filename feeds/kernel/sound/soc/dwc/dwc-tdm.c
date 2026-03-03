/*
 * ALSA SoC Synopsys I2S Audio Layer
 *
 * sound/soc/dwc/dwc_tdm.c
 *
 */

#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/clk.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/pm_runtime.h>
#include <sound/designware_i2s.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/dmaengine_pcm.h>
#include "local.h"

static inline void i2s_write_reg(void __iomem *io_base, int reg, u32 val)
{
	writel(val, io_base + reg);
}

static inline u32 i2s_read_reg(void __iomem *io_base, int reg)
{
	return readl(io_base + reg);
}

static inline void tdm_disable_channels(struct dw_i2s_dev *dev, u32 stream)
{
	u32 xer_val;

	if (stream == SNDRV_PCM_STREAM_PLAYBACK) {
		xer_val = i2s_read_reg(dev->i2s_base, TER(0));
		i2s_write_reg(dev->i2s_base, TER(0), xer_val & ~0x1);

	} else {
		xer_val = i2s_read_reg(dev->i2s_base, RER(0));
		i2s_write_reg(dev->i2s_base, RER(0), xer_val & ~0x1);
	}
}

static inline void tdm_clear_irqs(struct dw_i2s_dev *dev, u32 stream)
{
	if (stream == SNDRV_PCM_STREAM_PLAYBACK)
		i2s_read_reg(dev->i2s_base, TOR(0));
	else
		i2s_read_reg(dev->i2s_base, ROR(0));
}

static inline void tdm_disable_irqs(struct dw_i2s_dev *dev, u32 stream,
				    int chan_nr)
{
	u32 irq;

	if (stream == SNDRV_PCM_STREAM_PLAYBACK) {
		irq = i2s_read_reg(dev->i2s_base, IMR(0));
		i2s_write_reg(dev->i2s_base, IMR(0), irq | 0x30);
	} else {
		irq = i2s_read_reg(dev->i2s_base, IMR(0));
		i2s_write_reg(dev->i2s_base, IMR(0), irq | 0x03);
	}
}

static inline void tdm_enable_irqs(struct dw_i2s_dev *dev, u32 stream,
				   int chan_nr)
{
	u32 irq;

	if (stream == SNDRV_PCM_STREAM_PLAYBACK) {
		irq = i2s_read_reg(dev->i2s_base, IMR(0));
		i2s_write_reg(dev->i2s_base, IMR(0), irq & ~0x30);
	} else {
		irq = i2s_read_reg(dev->i2s_base, IMR(0));
		i2s_write_reg(dev->i2s_base, IMR(0), irq & ~0x03);
	}
}

static int tdm_slots_mask_matched(u32 chan_nr, u32 slots_mask)
{
	int count = 0;

	if (!slots_mask)
		return 0;

	while (slots_mask) {
		count += slots_mask & 0x1;
		slots_mask >>= 1;
	}

	if (chan_nr != count)
		return -EINVAL;

	return 0;
}

static irqreturn_t tdm_irq_handler(int irq, void *dev_id)
{
	struct dw_i2s_dev *dev = dev_id;
	bool irq_valid = false;
	u32 isr;

	isr = i2s_read_reg(dev->i2s_base, ISR(0));

	tdm_clear_irqs(dev, SNDRV_PCM_STREAM_PLAYBACK);
	tdm_clear_irqs(dev, SNDRV_PCM_STREAM_CAPTURE);

	/*
	 * Check if TX fifo is empty. If empty fill FIFO with samples
	 * NOTE: Only two channels supported
	 */
	if ((isr & ISR_TXFE) && dev->use_pio) {
		dw_pcm_push_tx(dev);
		irq_valid = true;
	}

	/*
	 * Data available. Retrieve samples from FIFO
	 * NOTE: Only two channels supported
	 */
	if ((isr & ISR_RXDA) && dev->use_pio) {
		dw_pcm_pop_rx(dev);
		irq_valid = true;
	}

	/* Error Handling: TX */
	if (isr & ISR_TXFO) {
		dev_err(dev->dev, "TX overrun\n");
		irq_valid = true;
	}

	/* Error Handling: TX */
	if (isr & ISR_RXFO) {
		dev_err(dev->dev, "RX overrun\n");
		irq_valid = true;
	}

	if (irq_valid)
		return IRQ_HANDLED;
	else
		return IRQ_NONE;
}

static void tdm_start(struct dw_i2s_dev *dev,
		      struct snd_pcm_substream *substream)
{
	struct i2s_clk_config_data *config = &dev->config;
	u32 ier_val;

	ier_val = i2s_read_reg(dev->i2s_base, IER);
	i2s_write_reg(dev->i2s_base, IER, ier_val | IER_IEN_MASK);

	tdm_enable_irqs(dev, substream->stream, config->chan_nr);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		i2s_write_reg(dev->i2s_base, ITER, 1);
	else
		i2s_write_reg(dev->i2s_base, IRER, 1);

	i2s_write_reg(dev->i2s_base, CER, 1);
}

static void tdm_stop(struct dw_i2s_dev *dev,
		struct snd_pcm_substream *substream)
{
	struct i2s_clk_config_data *config = &dev->config;
	u32 ier_val;

	tdm_clear_irqs(dev, substream->stream);
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		i2s_write_reg(dev->i2s_base, ITER, 0);
	else
		i2s_write_reg(dev->i2s_base, IRER, 0);

	tdm_disable_irqs(dev, substream->stream, config->chan_nr);

	if (!dev->active) {
		i2s_write_reg(dev->i2s_base, CER, 0);
		ier_val = i2s_read_reg(dev->i2s_base, IER);
		i2s_write_reg(dev->i2s_base, IER,
			      ier_val | (~IER_IEN_MASK));
	}
}

static int dw_tdm_startup(struct snd_pcm_substream *substream,
		struct snd_soc_dai *cpu_dai)
{
	struct dw_i2s_dev *dev = snd_soc_dai_get_drvdata(cpu_dai);
	union dw_i2s_snd_dma_data *dma_data = NULL;

	if (!(dev->capability & DWC_I2S_RECORD) &&
			(substream->stream == SNDRV_PCM_STREAM_CAPTURE))
		return -EINVAL;

	if (!(dev->capability & DWC_I2S_PLAY) &&
			(substream->stream == SNDRV_PCM_STREAM_PLAYBACK))
		return -EINVAL;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		dma_data = &dev->play_dma_data;
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		dma_data = &dev->capture_dma_data;

	snd_soc_dai_set_dma_data(cpu_dai, substream, (void *)dma_data);

	return 0;
}

static void dw_tdm_config(struct dw_i2s_dev *dev, int stream)
{
	struct i2s_clk_config_data *config = &dev->config;
	u32 slot_en_mask;

	tdm_disable_channels(dev, stream);

	// 启用对应Slot（bit8-bit15对应Slot0-7）
	// 生成如0x0300（2ch）、0x0F00（4ch）
	if (stream == SNDRV_PCM_STREAM_PLAYBACK) {
		if (!dev->tx_slots_mask)
			slot_en_mask = (GENMASK(15, 8) >> (8 - config->chan_nr)) & GENMASK(15, 8);
		else
			slot_en_mask = (dev->tx_slots_mask << IER_TDM_SLOTS_SHIFT) & GENMASK(15, 8);

		i2s_write_reg(dev->i2s_base, TCR(0),
			      dev->xfer_resolution);
		i2s_write_reg(dev->i2s_base, TFCR(0),
			      dev->fifo_th - 1);
		i2s_write_reg(dev->i2s_base, TER(0),
			      slot_en_mask | 0x1);
	} else {
		if (!dev->rx_slots_mask)
			slot_en_mask = (GENMASK(15, 8) >> (8 - config->chan_nr)) & GENMASK(15, 8);
		else
			slot_en_mask = (dev->rx_slots_mask << IER_TDM_SLOTS_SHIFT) & GENMASK(15, 8);

		i2s_write_reg(dev->i2s_base, RCR(0),
			      dev->xfer_resolution);
		i2s_write_reg(dev->i2s_base, RFCR(0),
			      dev->fifo_th - 1);
		i2s_write_reg(dev->i2s_base, RER(0),
			      slot_en_mask | 0x1);
	}
}

static int dw_tdm_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	struct dw_i2s_dev *dev = snd_soc_dai_get_drvdata(dai);
	struct i2s_clk_config_data *config = &dev->config;
	int ret;
	u32 ier_val;

	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S16_LE:
		config->data_width = 16;
		dev->ccr = 0x10;
		dev->xfer_resolution = 0x02;
		break;

	case SNDRV_PCM_FORMAT_S24_LE:
		config->data_width = 24;
		dev->ccr = 0x10;
		dev->xfer_resolution = 0x04;
		break;

	case SNDRV_PCM_FORMAT_S32_LE:
		config->data_width = 32;
		dev->ccr = 0x10;
		dev->xfer_resolution = 0x05;
		break;

	default:
		dev_err(dev->dev, "designware-tdm: unsupported PCM fmt");
		return -EINVAL;
	}

	config->chan_nr = params_channels(params);

	if (config->chan_nr > dev->tdm_slots) {
		dev_err(dev->dev,
			"designware-tdm: channels(%d) is larger than max slots(%d)",
			config->chan_nr, dev->tdm_slots);
		return -EINVAL;
	}

	switch (config->chan_nr) {
	case EIGHT_CHANNEL_SUPPORT:
	case SIX_CHANNEL_SUPPORT:
	case FOUR_CHANNEL_SUPPORT:
	case TWO_CHANNEL_SUPPORT:
	case ONE_CHANNEL_SUPPORT:
		break;
	default:
		dev_err(dev->dev, "channel(%d) not supported\n", config->chan_nr);
		return -EINVAL;
	}

	ret = tdm_slots_mask_matched(config->chan_nr, dev->tx_slots_mask);
	if (ret) {
		dev_err(dev->dev,
			"designware-tdm: channels(%d) is not matched with tx-slots(%2X)\n",
			config->chan_nr, dev->tx_slots_mask);
		return -EINVAL;
	}

	ret = tdm_slots_mask_matched(config->chan_nr, dev->rx_slots_mask);
	if (ret) {
		dev_err(dev->dev,
			"designware-tdm: channels(%d) is not matched with rx-slots(%2X)\n",
			config->chan_nr, dev->rx_slots_mask);
		return -EINVAL;
	}

	// Set IER Register, Disable IEN Bit
	ier_val = i2s_read_reg(dev->i2s_base, IER);
	i2s_write_reg(dev->i2s_base, IER,
		      ier_val & (~IER_IEN_MASK));

	ier_val &= ~(IER_TDM_SLOTS_MASK | IER_FRAME_OFF_MASK | IER_INTF_TYPE_MASK);
	ier_val |= (((dev->tdm_slots - 1) << IER_TDM_SLOTS_SHIFT) |
		   (dev->tdm_fmt << IER_FRAME_OFF_SHIFT) |
		   (dev->mode << IER_INTF_TYPE_SHIFT));
	i2s_write_reg(dev->i2s_base, IER, ier_val);

	dw_tdm_config(dev, substream->stream);

	config->sample_rate = params_rate(params);

	if (dev->capability & DW_I2S_MASTER) {
		if (dev->i2s_clk_cfg) {
			ret = dev->i2s_clk_cfg(config);
			if (ret < 0) {
				dev_err(dev->dev, "runtime audio clk config fail\n");
				return ret;
			}
		} else {
			// tdm_slots parameter select the frame total clock
			// config->chan_nr select valid tdm slot number
			u32 bitclk = config->sample_rate *
				TDM_SLOT_WIDTH * dev->tdm_slots;
			u32 top_sysclk_frequence;
			struct device_node *np = dev->dev->of_node;

			if (of_property_read_u32(np, "top_sysclk_frequence", &top_sysclk_frequence)) {
				pr_err("Failed to read top_sysclk_frequence from device tree\n");
				return -EINVAL;
			}

			writel(DIV_ROUND_CLOSEST(top_sysclk_frequence, bitclk),
				dev->i2s_clk_base + TOP_SYSCLK_DIV_OFFSET);

			ret = clk_set_rate(dev->clk, bitclk);
			if (ret) {
				dev_err(dev->dev, "Can't set I2S clock rate: %d\n",
					ret);
				return ret;
			}
		}
	}
	return 0;
}

static void dw_tdm_shutdown(struct snd_pcm_substream *substream,
		struct snd_soc_dai *dai)
{
	snd_soc_dai_set_dma_data(dai, substream, NULL);
}

static int dw_tdm_prepare(struct snd_pcm_substream *substream,
			  struct snd_soc_dai *dai)
{
	struct dw_i2s_dev *dev = snd_soc_dai_get_drvdata(dai);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		i2s_write_reg(dev->i2s_base, TXFFR, 1);
	else
		i2s_write_reg(dev->i2s_base, RXFFR, 1);

	return 0;
}

static int dw_tdm_trigger(struct snd_pcm_substream *substream,
		int cmd, struct snd_soc_dai *dai)
{
	struct dw_i2s_dev *dev = snd_soc_dai_get_drvdata(dai);
	int ret = 0;

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		dev->active++;
		tdm_start(dev, substream);
		break;

	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		dev->active--;
		tdm_stop(dev, substream);
		break;
	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}

static int dw_tdm_set_fmt(struct snd_soc_dai *cpu_dai, unsigned int fmt)
{
	struct dw_i2s_dev *dev = snd_soc_dai_get_drvdata(cpu_dai);
	int ret = 0;

	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBM_CFM:
		if (dev->capability & DW_I2S_SLAVE)
			ret = 0;
		else
			ret = -EINVAL;
		break;
	case SND_SOC_DAIFMT_CBS_CFS:
		if (dev->capability & DW_I2S_MASTER)
			ret = 0;
		else
			ret = -EINVAL;
		break;
	case SND_SOC_DAIFMT_CBM_CFS:
	case SND_SOC_DAIFMT_CBS_CFM:
		ret = -EINVAL;
		break;
	default:
		dev_dbg(dev->dev, "dwc-tdm : Invalid master/slave fromat %d\n",
			(fmt & SND_SOC_DAIFMT_MASTER_MASK));
		ret = -EINVAL;
		break;
	}

	/* interface format */
	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_DSP_A:
		dev->mode = PCM_MODE;
		dev->tdm_fmt = TDM_FRAME_DSP_A;
		break;
	case SND_SOC_DAIFMT_DSP_B:
		dev->mode = PCM_MODE;
		dev->tdm_fmt = TDM_FRAME_DSP_B;
		break;
	default:
		dev_dbg(dev->dev, "dwc-tdm : Invalid frame format %d\n",
			(fmt & SND_SOC_DAIFMT_FORMAT_MASK));
		ret = -EINVAL;
		break;
	}

	return ret;
}

static const struct snd_soc_dai_ops dw_tdm_dai_ops = {
	.startup	= dw_tdm_startup,
	.shutdown	= dw_tdm_shutdown,
	.hw_params	= dw_tdm_hw_params,
	.prepare	= dw_tdm_prepare,
	.trigger	= dw_tdm_trigger,
	.set_fmt	= dw_tdm_set_fmt,
};

#ifdef CONFIG_PM
static int dw_tdm_runtime_suspend(struct device *dev)
{
	struct dw_i2s_dev *dw_dev = dev_get_drvdata(dev);

	if (dw_dev->capability & DW_I2S_MASTER)
		clk_disable(dw_dev->clk);
	return 0;
}

static int dw_tdm_runtime_resume(struct device *dev)
{
	struct dw_i2s_dev *dw_dev = dev_get_drvdata(dev);
	int ret;

	if (dw_dev->capability & DW_I2S_MASTER) {
		ret = clk_enable(dw_dev->clk);
		if (ret)
			return ret;
	}
	return 0;
}

static int dw_tdm_suspend(struct snd_soc_component *component)
{
	struct dw_i2s_dev *dev = snd_soc_component_get_drvdata(component);

	if (dev->capability & DW_I2S_MASTER)
		clk_disable(dev->clk);
	return 0;
}

static int dw_tdm_resume(struct snd_soc_component *component)
{
	struct dw_i2s_dev *dev = snd_soc_component_get_drvdata(component);
	struct snd_soc_dai *dai;
	int stream, ret;

	if (dev->capability & DW_I2S_MASTER) {
		ret = clk_enable(dev->clk);
		if (ret)
			return ret;
	}

	for_each_component_dais(component, dai) {
		for_each_pcm_streams(stream)
			if (snd_soc_dai_stream_active(dai, stream))
				dw_tdm_config(dev, stream);
	}

	return 0;
}

#else
#define dw_tdm_suspend	NULL
#define dw_tdm_resume	NULL
#endif

static const struct snd_soc_component_driver dw_tdm_component = {
	.name		= "dw-tdm",
	.suspend	= dw_tdm_suspend,
	.resume		= dw_tdm_resume,
};

/*
 * The following tables allow a direct lookup of various parameters
 * defined in the I2S block's configuration in terms of sound system
 * parameters.  Each table is sized to the number of entries possible
 * according to the number of configuration bits describing an I2S
 * block parameter.
 */

/* Maximum bit resolution of a channel - not uniformly spaced */
static const u32 fifo_width[COMP_MAX_WORDSIZE] = {
	12, 16, 20, 24, 32, 0, 0, 0
};

/* Width of (DMA) bus */
static const u32 bus_widths[COMP_MAX_DATA_WIDTH] = {
	DMA_SLAVE_BUSWIDTH_1_BYTE,
	DMA_SLAVE_BUSWIDTH_2_BYTES,
	DMA_SLAVE_BUSWIDTH_4_BYTES,
	DMA_SLAVE_BUSWIDTH_UNDEFINED
};

/* PCM format to support channel resolution */
static const u32 formats[COMP_MAX_WORDSIZE] = {
	SNDRV_PCM_FMTBIT_S16_LE,
	SNDRV_PCM_FMTBIT_S16_LE,
	SNDRV_PCM_FMTBIT_S24_LE,
	SNDRV_PCM_FMTBIT_S24_LE,
	SNDRV_PCM_FMTBIT_S32_LE|SNDRV_PCM_FMTBIT_S24_LE|SNDRV_PCM_FMTBIT_S16_LE,
	0,
	0,
	0
};

static int dw_configure_dai(struct dw_i2s_dev *dev,
				   struct snd_soc_dai_driver *dw_tdm_dai,
				   unsigned int rates)
{
	/*
	 * Read component parameter registers to extract
	 * the I2S block's configuration.
	 */
	u32 comp1 = i2s_read_reg(dev->i2s_base, dev->i2s_reg_comp1);
	u32 comp2 = i2s_read_reg(dev->i2s_base, dev->i2s_reg_comp2);
	u32 fifo_depth = 1 << (1 + COMP1_FIFO_DEPTH_GLOBAL(comp1));
	u32 idx;

	if (dev->capability & DWC_I2S_RECORD &&
			dev->quirks & DW_I2S_QUIRK_COMP_PARAM1)
		comp1 = comp1 & ~BIT(5);

	if (dev->capability & DWC_I2S_PLAY &&
			dev->quirks & DW_I2S_QUIRK_COMP_PARAM1)
		comp1 = comp1 & ~BIT(6);

	if (COMP1_TX_ENABLED(comp1)) {
		dev_dbg(dev->dev, " designware: play supported\n");
		idx = COMP1_TX_WORDSIZE_0(comp1);
		if (WARN_ON(idx >= ARRAY_SIZE(formats)))
			return -EINVAL;
		if (dev->quirks & DW_I2S_QUIRK_16BIT_IDX_OVERRIDE)
			idx = 1;
		dw_tdm_dai->playback.channels_min = 1;
		dw_tdm_dai->playback.channels_max = dev->tdm_slots;
		dw_tdm_dai->playback.formats = formats[idx];
		dw_tdm_dai->playback.rates = rates;
	}

	if (COMP1_RX_ENABLED(comp1)) {
		dev_dbg(dev->dev, "designware: record supported\n");
		idx = COMP2_RX_WORDSIZE_0(comp2);
		if (WARN_ON(idx >= ARRAY_SIZE(formats)))
			return -EINVAL;
		if (dev->quirks & DW_I2S_QUIRK_16BIT_IDX_OVERRIDE)
			idx = 1;
		dw_tdm_dai->capture.channels_min = 1;
		dw_tdm_dai->capture.channels_max = dev->tdm_slots;
		dw_tdm_dai->capture.formats = formats[idx];
		dw_tdm_dai->capture.rates = rates;
	}

	if (COMP1_MODE_EN(comp1)) {
		dev_dbg(dev->dev, "designware: i2s master mode supported\n");
		dev->capability |= DW_I2S_MASTER;
	} else {
		dev_dbg(dev->dev, "designware: i2s slave mode supported\n");
		dev->capability |= DW_I2S_SLAVE;
	}

	dev->fifo_th = fifo_depth / 2;
	return 0;
}

static int dw_configure_dai_by_pd(struct dw_i2s_dev *dev,
				   struct snd_soc_dai_driver *dw_tdm_dai,
				   struct resource *res,
				   const struct i2s_platform_data *pdata)
{
	u32 comp1 = i2s_read_reg(dev->i2s_base, dev->i2s_reg_comp1);
	u32 idx = COMP1_APB_DATA_WIDTH(comp1);
	int ret;

	if (WARN_ON(idx >= ARRAY_SIZE(bus_widths)))
		return -EINVAL;

	ret = dw_configure_dai(dev, dw_tdm_dai, pdata->snd_rates);
	if (ret < 0)
		return ret;

	if (dev->quirks & DW_I2S_QUIRK_16BIT_IDX_OVERRIDE)
		idx = 1;
	/* Set DMA slaves info */
	dev->play_dma_data.pd.data = pdata->play_dma_data;
	dev->capture_dma_data.pd.data = pdata->capture_dma_data;
	dev->play_dma_data.pd.addr = res->start + I2S_TXDMA;
	dev->capture_dma_data.pd.addr = res->start + I2S_RXDMA;
	dev->play_dma_data.pd.max_burst = 16;
	dev->capture_dma_data.pd.max_burst = 16;
	dev->play_dma_data.pd.addr_width = bus_widths[idx];
	dev->capture_dma_data.pd.addr_width = bus_widths[idx];
	dev->play_dma_data.pd.filter = pdata->filter;
	dev->capture_dma_data.pd.filter = pdata->filter;

	return 0;
}

static int dw_configure_dai_by_dt(struct dw_i2s_dev *dev,
				   struct snd_soc_dai_driver *dw_tdm_dai,
				   struct resource *res)
{
	u32 comp1 = i2s_read_reg(dev->i2s_base, I2S_COMP_PARAM_1);
	u32 comp2 = i2s_read_reg(dev->i2s_base, I2S_COMP_PARAM_2);
	u32 fifo_depth = 1 << (1 + COMP1_FIFO_DEPTH_GLOBAL(comp1));
	u32 idx = COMP1_APB_DATA_WIDTH(comp1);
	u32 idx2;
	int ret;

	if (WARN_ON(idx >= ARRAY_SIZE(bus_widths)))
		return -EINVAL;

	ret = dw_configure_dai(dev, dw_tdm_dai, SNDRV_PCM_RATE_8000_192000);
	if (ret < 0)
		return ret;

	if (COMP1_TX_ENABLED(comp1)) {
		idx2 = COMP1_TX_WORDSIZE_0(comp1);

		dev->capability |= DWC_I2S_PLAY;
		dev->play_dma_data.dt.addr = res->start + I2S_TXDMA;
		dev->play_dma_data.dt.addr_width = bus_widths[idx];
		dev->play_dma_data.dt.fifo_size = fifo_depth *
			(fifo_width[idx2]) >> 8;
		dev->play_dma_data.dt.maxburst = 16;
	}
	if (COMP1_RX_ENABLED(comp1)) {
		idx2 = COMP2_RX_WORDSIZE_0(comp2);

		dev->capability |= DWC_I2S_RECORD;
		dev->capture_dma_data.dt.addr = res->start + I2S_RXDMA;
		dev->capture_dma_data.dt.addr_width = bus_widths[idx];
		dev->capture_dma_data.dt.fifo_size = fifo_depth *
			(fifo_width[idx2] >> 8);
		dev->capture_dma_data.dt.maxburst = 16;
	}

	return 0;

}

static int dw_tdm_probe(struct platform_device *pdev)
{
	const struct i2s_platform_data *pdata = pdev->dev.platform_data;
	struct dw_i2s_dev *dev;
	struct resource *res, *top_clk_div_res;
	int ret, irq;
	struct snd_soc_dai_driver *dw_tdm_dai;
	const char *clk_id;

	struct device_node *np = pdev->dev.of_node;

	dev = devm_kzalloc(&pdev->dev, sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	dw_tdm_dai = devm_kzalloc(&pdev->dev, sizeof(*dw_tdm_dai), GFP_KERNEL);
	if (!dw_tdm_dai)
		return -ENOMEM;

	// 读取 snps,tdm-slots 属性值
	dev->tdm_slots = 8;
	ret = of_property_read_u32(np, "snps,tdm-slots", &dev->tdm_slots);
	if (ret)
		dev_info(&pdev->dev, "Using default TDM slots: %u\n", dev->tdm_slots);
	if (dev->tdm_slots < 2 || dev->tdm_slots > 8) {
		dev_err(&pdev->dev, "Invalid TDM slots: %u\n", dev->tdm_slots);
		return -EINVAL;
	}

	// 读取 snps,tx-slots mask 属性值
	ret = of_property_read_u32(np, "snps,tx-slots", &dev->tx_slots_mask);
	if (ret)
		dev->tx_slots_mask = 0;
	else
		dev->tx_slots_mask &= 0xFF;

	// 读取 snps,rx-slots mask 属性值
	ret = of_property_read_u32(np, "snps,rx-slots", &dev->rx_slots_mask);
	if (ret)
		dev->rx_slots_mask = 0;
	else
		dev->rx_slots_mask &= 0xFF;

	dw_tdm_dai->ops = &dw_tdm_dai_ops;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	dev->i2s_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(dev->i2s_base))
		return PTR_ERR(dev->i2s_base);

	dev->dev = &pdev->dev;

	irq = platform_get_irq(pdev, 0);
	if (irq >= 0) {
		ret = devm_request_irq(&pdev->dev, irq, tdm_irq_handler, 0,
				pdev->name, dev);
		if (ret < 0) {
			dev_err(&pdev->dev, "failed to request irq\n");
			return ret;
		}
	}

	top_clk_div_res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	dev->i2s_clk_base = devm_ioremap_resource(&pdev->dev, top_clk_div_res);
	if (IS_ERR(dev->i2s_clk_base))
		return PTR_ERR(dev->i2s_clk_base);

	dev->i2s_reg_comp1 = I2S_COMP_PARAM_1;
	dev->i2s_reg_comp2 = I2S_COMP_PARAM_2;
	if (pdata) {
		dev->capability = pdata->cap;
		clk_id = NULL;
		dev->quirks = pdata->quirks;
		if (dev->quirks & DW_I2S_QUIRK_COMP_REG_OFFSET) {
			dev->i2s_reg_comp1 = pdata->i2s_reg_comp1;
			dev->i2s_reg_comp2 = pdata->i2s_reg_comp2;
		}
		ret = dw_configure_dai_by_pd(dev, dw_tdm_dai, res, pdata);
	} else {
		clk_id = "i2sclk";
		ret = dw_configure_dai_by_dt(dev, dw_tdm_dai, res);
	}
	if (ret < 0)
		return ret;

	if (dev->capability & DW_I2S_MASTER) {
		if (pdata) {
			dev->i2s_clk_cfg = pdata->i2s_clk_cfg;
			if (!dev->i2s_clk_cfg) {
				dev_err(&pdev->dev, "no clock configure method\n");
				return -ENODEV;
			}
		}
		dev->clk = devm_clk_get(&pdev->dev, clk_id);

		if (IS_ERR(dev->clk))
			return PTR_ERR(dev->clk);

		ret = clk_prepare_enable(dev->clk);
		if (ret < 0)
			return ret;
	}

	dev_set_drvdata(&pdev->dev, dev);
	ret = devm_snd_soc_register_component(&pdev->dev, &dw_tdm_component,
					 dw_tdm_dai, 1);
	if (ret != 0) {
		dev_err(&pdev->dev, "not able to register dai\n");
		goto err_clk_disable;
	}

	if (!pdata) {
		if (irq >= 0) {
			ret = dw_tdm_register(pdev);
			dev->use_pio = true;
		} else {
			ret = devm_snd_dmaengine_pcm_register(&pdev->dev, NULL,
					0);
			dev->use_pio = false;
		}

		if (ret) {
			dev_err(&pdev->dev, "could not register pcm: %d\n",
					ret);
			goto err_clk_disable;
		}
	}

	pm_runtime_enable(&pdev->dev);
	return 0;

err_clk_disable:
	if (dev->capability & DW_I2S_MASTER)
		clk_disable_unprepare(dev->clk);
	return ret;
}

static int dw_tdm_remove(struct platform_device *pdev)
{
	struct dw_i2s_dev *dev = dev_get_drvdata(&pdev->dev);

	if (dev->capability & DW_I2S_MASTER)
		clk_disable_unprepare(dev->clk);

	pm_runtime_disable(&pdev->dev);
	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id dw_tdm_of_match[] = {
	{ .compatible = "snps,designware-tdm",	 },
	{},
};

MODULE_DEVICE_TABLE(of, dw_tdm_of_match);
#endif

static const struct dev_pm_ops dwc_pm_ops = {
	SET_RUNTIME_PM_OPS(dw_tdm_runtime_suspend, dw_tdm_runtime_resume, NULL)
};

static struct platform_driver dw_tdm_driver = {
	.probe		= dw_tdm_probe,
	.remove		= dw_tdm_remove,
	.driver		= {
		.name	= "designware-tdm",
		.of_match_table = of_match_ptr(dw_tdm_of_match),
		.pm = &dwc_pm_ops,
	},
};

module_platform_driver(dw_tdm_driver);

MODULE_AUTHOR("Weston Zhu <wez057@clourneysemi.com>");
MODULE_DESCRIPTION("DESIGNWARE I2S SoC TDM Interface");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:designware_tdm");
