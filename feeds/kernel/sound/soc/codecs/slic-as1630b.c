// SPDX-License-Identifier: GPL-2.0-only
/*
 * Driver for the SaiSi SLIC AS1630B
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>

#include <sound/soc.h>

static struct snd_soc_dai_driver as1630b_dai = {
	.name = "slic-pcm",
	.playback = {
		.stream_name = "Playback",
		.rates = SNDRV_PCM_RATE_8000_192000,
		.formats = SNDRV_PCM_FMTBIT_S16_LE |
			   SNDRV_PCM_FMTBIT_S24_LE |
			   SNDRV_PCM_FMTBIT_S32_LE,
		.rate_min = 8000,
		.rate_max = 192000,
		.channels_min = 1,
		.channels_max = 8,
	},
	.capture = {
		.stream_name = "Capture",
		.rates = SNDRV_PCM_RATE_8000_192000,
		.formats = SNDRV_PCM_FMTBIT_S16_LE |
			   SNDRV_PCM_FMTBIT_S24_LE |
			   SNDRV_PCM_FMTBIT_S32_LE,
		.rate_min = 8000,
		.rate_max = 192000,
		.channels_min = 1,
		.channels_max = 8,
	},
};

static const struct snd_soc_component_driver soc_component_dev_as1630b = {
	.idle_bias_on		= 1,
	.use_pmdown_time	= 1,
	.endianness		= 1,
	.non_legacy_dai_naming	= 1,
};

static int as1630b_probe(struct platform_device *pdev)
{
	return devm_snd_soc_register_component(&pdev->dev, &soc_component_dev_as1630b,
			&as1630b_dai, 1);
}

static const struct of_device_id as1630b_of_match[] = {
	{ .compatible = "saisi,slic_as1630b", },
	{ }
};
MODULE_DEVICE_TABLE(of, as1630b_of_match);

static struct platform_driver as1630b_codec_driver = {
	.probe		= as1630b_probe,
	.driver		= {
		.name	= "slic-codec",
		.of_match_table = as1630b_of_match,
	},
};

module_platform_driver(as1630b_codec_driver);

MODULE_DESCRIPTION("ASoC SLIC as1630b codec driver");
MODULE_AUTHOR("Weston Zhu <wez057@clourneysemi.com>");
MODULE_LICENSE("GPL v2");
