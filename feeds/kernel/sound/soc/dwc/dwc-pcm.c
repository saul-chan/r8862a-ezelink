/*
 * ALSA SoC Synopsys PIO PCM for I2S driver
 *
 * sound/soc/dwc/designware_pcm.c
 *
 * Copyright (C) 2016 Synopsys
 * Jose Abreu <joabreu@synopsys.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/io.h>
#include <linux/rcupdate.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include "local.h"

#define BUFFER_BYTES_MAX	(3 * 2 * 8 * PERIOD_BYTES_MIN)
#define PERIOD_BYTES_MIN	4096
#define PERIODS_MIN		2

#ifdef CONFIG_I2S_DMA_REPLACE_FIFO
#define dw_pcm_tx_fn(sample_bits) \
static unsigned int dw_pcm_tx_##sample_bits(struct dw_i2s_dev *dev, \
		struct snd_pcm_runtime *runtime, unsigned int tx_ptr, \
		bool *period_elapsed) \
{ \
	const u##sample_bits (*p)[2] = (void *)runtime->dma_area; \
	unsigned int period_pos = tx_ptr % runtime->period_size; \
	int i; \
\
	for (i = 0; i < dev->fifo_th; i++) { \
		iowrite32(p[tx_ptr][0], dev->i2s_base + I2S_TXDMA); \
		iowrite32(p[tx_ptr][1], dev->i2s_base + I2S_TXDMA); \
		period_pos++; \
		if (++tx_ptr >= runtime->buffer_size) \
			tx_ptr = 0; \
	} \
	*period_elapsed = period_pos >= runtime->period_size; \
	return tx_ptr; \
}
#else
#define dw_pcm_tx_fn(sample_bits) \
static unsigned int dw_pcm_tx_##sample_bits(struct dw_i2s_dev *dev, \
		struct snd_pcm_runtime *runtime, unsigned int tx_ptr, \
		bool *period_elapsed) \
{ \
	const u##sample_bits (*p)[2] = (void *)runtime->dma_area; \
	unsigned int period_pos = tx_ptr % runtime->period_size; \
	int i; \
\
	for (i = 0; i < dev->fifo_th; i++) { \
		iowrite32(p[tx_ptr][0], dev->i2s_base + LRBR_LTHR(0)); \
		iowrite32(p[tx_ptr][1], dev->i2s_base + RRBR_RTHR(0)); \
		period_pos++; \
		if (++tx_ptr >= runtime->buffer_size) \
			tx_ptr = 0; \
	} \
	*period_elapsed = period_pos >= runtime->period_size; \
	return tx_ptr; \
}
#endif

#ifdef CONFIG_I2S_DMA_REPLACE_FIFO
#define dw_pcm_rx_fn(sample_bits) \
static unsigned int dw_pcm_rx_##sample_bits(struct dw_i2s_dev *dev, \
		struct snd_pcm_runtime *runtime, unsigned int rx_ptr, \
		bool *period_elapsed) \
{ \
	u##sample_bits (*p)[2] = (void *)runtime->dma_area; \
	unsigned int period_pos = rx_ptr % runtime->period_size; \
	int i; \
\
	for (i = 0; i < dev->fifo_th; i++) { \
		p[rx_ptr][0] = ioread32(dev->i2s_base + I2S_RXDMA); \
		p[rx_ptr][1] = ioread32(dev->i2s_base + I2S_RXDMA); \
		period_pos++; \
		if (++rx_ptr >= runtime->buffer_size) \
			rx_ptr = 0; \
	} \
	*period_elapsed = period_pos >= runtime->period_size; \
	return rx_ptr; \
}
#else
#define dw_pcm_rx_fn(sample_bits) \
static unsigned int dw_pcm_rx_##sample_bits(struct dw_i2s_dev *dev, \
		struct snd_pcm_runtime *runtime, unsigned int rx_ptr, \
		bool *period_elapsed) \
{ \
	u##sample_bits (*p)[2] = (void *)runtime->dma_area; \
	unsigned int period_pos = rx_ptr % runtime->period_size; \
	int i; \
\
	for (i = 0; i < dev->fifo_th; i++) { \
		p[rx_ptr][0] = ioread32(dev->i2s_base + LRBR_LTHR(0)); \
		p[rx_ptr][1] = ioread32(dev->i2s_base + RRBR_RTHR(0)); \
		period_pos++; \
		if (++rx_ptr >= runtime->buffer_size) \
			rx_ptr = 0; \
	} \
	*period_elapsed = period_pos >= runtime->period_size; \
	return rx_ptr; \
}
#endif

dw_pcm_tx_fn(16);
dw_pcm_tx_fn(32);
dw_pcm_rx_fn(16);
dw_pcm_rx_fn(32);

#undef dw_pcm_tx_fn
#undef dw_pcm_rx_fn

#ifndef CONFIG_TDM_DMA_REPLACE_FIFO
static void find_enable_slots(u8 *p, u8 ch_nr, u8 slots_mask)
{
	int i = 0, ch;

	for (ch = 0; ch < ch_nr; ch++) {
		if (slots_mask & (1 << ch))
			p[i++] = ch;
	}
}
#endif

#ifdef CONFIG_TDM_DMA_REPLACE_FIFO
#define dw_tdm_tx_fn(sample_bits) \
static unsigned int dw_tdm_tx_##sample_bits(struct dw_i2s_dev *dev, \
		struct snd_pcm_runtime *runtime, unsigned int tx_ptr, \
		bool *period_elapsed) \
{ \
	const u##sample_bits (*p)[runtime->channels] = (void *)runtime->dma_area; \
	unsigned int period_pos = tx_ptr % runtime->period_size; \
	int i, ch; \
\
	for (i = 0; i < dev->fifo_th; i++) { \
		for (ch = 0; ch < runtime->channels; ch++) \
			iowrite32(p[tx_ptr][ch], dev->i2s_base + I2S_TXDMA); \
		period_pos++; \
		if (++tx_ptr >= runtime->buffer_size) \
			tx_ptr = 0; \
	} \
	*period_elapsed = period_pos >= runtime->period_size; \
	return tx_ptr; \
}
#else
#define dw_tdm_tx_fn(sample_bits) \
static unsigned int dw_tdm_tx_##sample_bits(struct dw_i2s_dev *dev, \
		struct snd_pcm_runtime *runtime, unsigned int tx_ptr, \
		bool *period_elapsed) \
{ \
	u8 slot_ch[runtime->channels]; \
	const u##sample_bits (*p)[runtime->channels] = (void *)runtime->dma_area; \
	unsigned int period_pos = tx_ptr % runtime->period_size; \
	int i, ch; \
\
	if (dev->tx_slots_mask) \
		find_enable_slots((u8 *)&slot_ch[0], dev->tdm_slots, dev->tx_slots_mask); \
\
	for (i = 0; i < dev->fifo_th; i++) { \
		for (ch = 0; ch < runtime->channels; ch++) { \
			if (dev->tx_slots_mask) \
				iowrite32(p[tx_ptr][ch], dev->i2s_base + TSLOT(slot_ch[ch])); \
			else \
				iowrite32(p[tx_ptr][ch], dev->i2s_base + TSLOT(ch)); \
		} \
		period_pos++; \
		if (++tx_ptr >= runtime->buffer_size) \
			tx_ptr = 0; \
	} \
	*period_elapsed = period_pos >= runtime->period_size; \
	return tx_ptr; \
}
#endif

#ifdef CONFIG_TDM_DMA_REPLACE_FIFO
#define dw_tdm_rx_fn(sample_bits) \
static unsigned int dw_tdm_rx_##sample_bits(struct dw_i2s_dev *dev, \
		struct snd_pcm_runtime *runtime, unsigned int rx_ptr, \
		bool *period_elapsed) \
{ \
	u##sample_bits (*p)[runtime->channels] = (void *)runtime->dma_area; \
	unsigned int period_pos = rx_ptr % runtime->period_size; \
	int i, ch; \
\
	for (i = 0; i < dev->fifo_th; i++) { \
		for (ch = 0; ch < runtime->channels; ch++) \
			p[rx_ptr][ch] = ioread32(dev->i2s_base + I2S_RXDMA); \
		period_pos++; \
		if (++rx_ptr >= runtime->buffer_size) \
			rx_ptr = 0; \
	} \
	*period_elapsed = period_pos >= runtime->period_size; \
	return rx_ptr; \
}
#else
#define dw_tdm_rx_fn(sample_bits) \
static unsigned int dw_tdm_rx_##sample_bits(struct dw_i2s_dev *dev, \
		struct snd_pcm_runtime *runtime, unsigned int rx_ptr, \
		bool *period_elapsed) \
{ \
	u8 slot_ch[runtime->channels]; \
	u##sample_bits (*p)[runtime->channels] = (void *)runtime->dma_area; \
	unsigned int period_pos = rx_ptr % runtime->period_size; \
	int i, ch; \
\
	if (dev->rx_slots_mask) \
		find_enable_slots((u8 *)&slot_ch[0], dev->tdm_slots, dev->rx_slots_mask); \
\
	for (i = 0; i < dev->fifo_th; i++) { \
		for (ch = 0; ch < runtime->channels; ch++) { \
			if (dev->rx_slots_mask) \
				p[rx_ptr][ch] = ioread32(dev->i2s_base + RSLOT(slot_ch[ch])); \
			else \
				p[rx_ptr][ch] = ioread32(dev->i2s_base + RSLOT(ch)); \
		} \
		period_pos++; \
		if (++rx_ptr >= runtime->buffer_size) \
			rx_ptr = 0; \
	} \
	*period_elapsed = period_pos >= runtime->period_size; \
	return rx_ptr; \
}
#endif

dw_tdm_tx_fn(16);
dw_tdm_tx_fn(32);
dw_tdm_rx_fn(16);
dw_tdm_rx_fn(32);

#undef dw_tdm_tx_fn
#undef dw_tdm_rx_fn

static const struct snd_pcm_hardware dw_pcm_hardware = {
	.info = SNDRV_PCM_INFO_INTERLEAVED |
		SNDRV_PCM_INFO_MMAP |
		SNDRV_PCM_INFO_MMAP_VALID |
		SNDRV_PCM_INFO_BLOCK_TRANSFER,
	.rates = SNDRV_PCM_RATE_16000 |
		SNDRV_PCM_RATE_32000 |
		SNDRV_PCM_RATE_48000 |
		SNDRV_PCM_RATE_96000 |
		SNDRV_PCM_RATE_192000,
	.rate_min = 16000,
	.rate_max = 192000,
	.formats = SNDRV_PCM_FMTBIT_S16_LE |
		SNDRV_PCM_FMTBIT_S24_LE |
		SNDRV_PCM_FMTBIT_S32_LE,
	.channels_min = 2,
	.channels_max = 2,
	.buffer_bytes_max = BUFFER_BYTES_MAX,
	.period_bytes_min = PERIOD_BYTES_MIN,
	.period_bytes_max = BUFFER_BYTES_MAX / PERIODS_MIN,
	.periods_min = PERIODS_MIN,
	.periods_max = BUFFER_BYTES_MAX / PERIOD_BYTES_MIN,
	.fifo_size = 16,
};

static void dw_pcm_transfer(struct dw_i2s_dev *dev, bool push)
{
	struct snd_pcm_substream *substream;
	bool active, period_elapsed;

	rcu_read_lock();
	if (push)
		substream = rcu_dereference(dev->tx_substream);
	else
		substream = rcu_dereference(dev->rx_substream);
	active = substream && snd_pcm_running(substream);
	if (active) {
		unsigned int ptr;
		unsigned int new_ptr;

		if (push) {
			ptr = READ_ONCE(dev->tx_ptr);
			new_ptr = dev->tx_fn(dev, substream->runtime, ptr,
					&period_elapsed);
			cmpxchg(&dev->tx_ptr, ptr, new_ptr);
		} else {
			ptr = READ_ONCE(dev->rx_ptr);
			new_ptr = dev->rx_fn(dev, substream->runtime, ptr,
					&period_elapsed);
			cmpxchg(&dev->rx_ptr, ptr, new_ptr);
		}

		if (period_elapsed)
			snd_pcm_period_elapsed(substream);
	}
	rcu_read_unlock();
}

void dw_pcm_push_tx(struct dw_i2s_dev *dev)
{
	dw_pcm_transfer(dev, true);
}

void dw_pcm_pop_rx(struct dw_i2s_dev *dev)
{
	dw_pcm_transfer(dev, false);
}

static int dw_pcm_open(struct snd_soc_component *component,
		       struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct snd_soc_pcm_runtime *rtd = asoc_substream_to_rtd(substream);
	struct dw_i2s_dev *dev = snd_soc_dai_get_drvdata(asoc_rtd_to_cpu(rtd, 0));

	snd_soc_set_runtime_hwparams(substream, &dw_pcm_hardware);
	snd_pcm_hw_constraint_integer(runtime, SNDRV_PCM_HW_PARAM_PERIODS);
	runtime->private_data = dev;

	return 0;
}

static int dw_pcm_close(struct snd_soc_component *component,
			struct snd_pcm_substream *substream)
{
	synchronize_rcu();
	return 0;
}

static int dw_pcm_hw_params(struct snd_soc_component *component,
			    struct snd_pcm_substream *substream,
			    struct snd_pcm_hw_params *hw_params)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct dw_i2s_dev *dev = runtime->private_data;

	switch (params_channels(hw_params)) {
	case 2:
		break;
	default:
		dev_err(dev->dev, "invalid channels number\n");
		return -EINVAL;
	}

	switch (params_format(hw_params)) {
	case SNDRV_PCM_FORMAT_S16_LE:
		dev->tx_fn = dw_pcm_tx_16;
		dev->rx_fn = dw_pcm_rx_16;
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
	case SNDRV_PCM_FORMAT_S32_LE:
		dev->tx_fn = dw_pcm_tx_32;
		dev->rx_fn = dw_pcm_rx_32;
		break;
	default:
		dev_err(dev->dev, "invalid format\n");
		return -EINVAL;
	}

	return 0;
}

static int dw_pcm_trigger(struct snd_soc_component *component,
			  struct snd_pcm_substream *substream, int cmd)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct dw_i2s_dev *dev = runtime->private_data;
	int ret = 0;

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
			WRITE_ONCE(dev->tx_ptr, 0);
			rcu_assign_pointer(dev->tx_substream, substream);
		} else {
			WRITE_ONCE(dev->rx_ptr, 0);
			rcu_assign_pointer(dev->rx_substream, substream);
		}
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
			rcu_assign_pointer(dev->tx_substream, NULL);
		else
			rcu_assign_pointer(dev->rx_substream, NULL);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static snd_pcm_uframes_t dw_pcm_pointer(struct snd_soc_component *component,
					struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct dw_i2s_dev *dev = runtime->private_data;
	snd_pcm_uframes_t pos;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		pos = READ_ONCE(dev->tx_ptr);
	else
		pos = READ_ONCE(dev->rx_ptr);

	return pos < runtime->buffer_size ? pos : 0;
}

static int dw_pcm_new(struct snd_soc_component *component,
		      struct snd_soc_pcm_runtime *rtd)
{
	size_t size = dw_pcm_hardware.buffer_bytes_max;

	snd_pcm_set_managed_buffer_all(rtd->pcm,
			SNDRV_DMA_TYPE_CONTINUOUS,
			NULL, size, size);
	return 0;
}

static const struct snd_soc_component_driver dw_pcm_component = {
	.open		= dw_pcm_open,
	.close		= dw_pcm_close,
	.hw_params	= dw_pcm_hw_params,
	.trigger	= dw_pcm_trigger,
	.pointer	= dw_pcm_pointer,
	.pcm_construct	= dw_pcm_new,
};

int dw_pcm_register(struct platform_device *pdev)
{
	return devm_snd_soc_register_component(&pdev->dev, &dw_pcm_component,
					       NULL, 0);
}

static const struct snd_pcm_hardware dw_tdm_hardware = {
	.info = SNDRV_PCM_INFO_INTERLEAVED |
		SNDRV_PCM_INFO_MMAP |
		SNDRV_PCM_INFO_MMAP_VALID |
		SNDRV_PCM_INFO_BLOCK_TRANSFER,
	.rates = SNDRV_PCM_RATE_8000 |
		SNDRV_PCM_RATE_16000 |
		SNDRV_PCM_RATE_32000 |
		SNDRV_PCM_RATE_48000 |
		SNDRV_PCM_RATE_96000 |
		SNDRV_PCM_RATE_192000,
	.rate_min = 8000,
	.rate_max = 192000,
	.formats = SNDRV_PCM_FMTBIT_S16_LE |
		   SNDRV_PCM_FMTBIT_S24_LE |
		   SNDRV_PCM_FMTBIT_S32_LE,
	.channels_min = 1,
	.channels_max = 8,
	.buffer_bytes_max = 4915200,
	.period_bytes_min = 160,
	.period_bytes_max = 307200,
	.periods_min = 1,
	.periods_max = 16,
	.fifo_size = 16,
};

static int dw_tdm_open(struct snd_soc_component *component,
		       struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct snd_soc_pcm_runtime *rtd = asoc_substream_to_rtd(substream);
	struct dw_i2s_dev *dev = snd_soc_dai_get_drvdata(asoc_rtd_to_cpu(rtd, 0));

	snd_soc_set_runtime_hwparams(substream, &dw_tdm_hardware);
	snd_pcm_hw_constraint_integer(runtime, SNDRV_PCM_HW_PARAM_PERIODS);
	runtime->private_data = dev;

	return 0;
}

static int dw_tdm_close(struct snd_soc_component *component,
			struct snd_pcm_substream *substream)
{
	synchronize_rcu();
	return 0;
}

static int dw_tdm_hw_params(struct snd_soc_component *component,
			    struct snd_pcm_substream *substream,
			    struct snd_pcm_hw_params *hw_params)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct dw_i2s_dev *dev = runtime->private_data;
	unsigned int bytes_per_sample = snd_pcm_format_physical_width(params_format(hw_params)) / 8;
	unsigned int channels = params_channels(hw_params);
	unsigned int rate = params_rate(hw_params);

	switch (params_channels(hw_params)) {
	case 1:
	case 2:
	case 4:
	case 6:
	case 8:
		break;
	default:
		dev_err(dev->dev, "invalid channels number\n");
		return -EINVAL;
	}

	switch (params_format(hw_params)) {
	case SNDRV_PCM_FORMAT_S16_LE:
		dev->tx_fn = dw_tdm_tx_16;
		dev->rx_fn = dw_tdm_rx_16;
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
	case SNDRV_PCM_FORMAT_S32_LE:
		dev->tx_fn = dw_tdm_tx_32;
		dev->rx_fn = dw_tdm_rx_32;
		break;
	default:
		dev_err(dev->dev, "invalid format\n");
		return -EINVAL;
	}

	// 动态计算周期大小（目标周期时间 10ms）
	runtime->hw.period_bytes_min = (rate * bytes_per_sample * channels * 10000) / (8 * 1000000);

	// 限制在全局范围内
	runtime->hw.period_bytes_min = max(runtime->hw.period_bytes_min, 160UL);
	runtime->hw.period_bytes_max = min(runtime->hw.period_bytes_max, 307200UL);
	return 0;
}

static int dw_tdm_new(struct snd_soc_component *component,
		      struct snd_soc_pcm_runtime *rtd)
{
	size_t size = dw_tdm_hardware.buffer_bytes_max;

	snd_pcm_set_managed_buffer_all(rtd->pcm,
			SNDRV_DMA_TYPE_CONTINUOUS,
			NULL, size, size);
	return 0;
}

static const struct snd_soc_component_driver dw_tdm_component = {
	.open           = dw_tdm_open,
	.close          = dw_tdm_close,
	.hw_params      = dw_tdm_hw_params,
	.trigger        = dw_pcm_trigger,
	.pointer        = dw_pcm_pointer,
	.pcm_construct  = dw_tdm_new,
};

int dw_tdm_register(struct platform_device *pdev)
{
	return devm_snd_soc_register_component(&pdev->dev, &dw_tdm_component,
					       NULL, 0);
}
