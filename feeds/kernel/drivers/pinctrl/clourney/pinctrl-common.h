/*
 * Copyright (c) 2022, Clourney Semiconductor Limited and Contributors. All rights reserved.
 *
 */

#ifndef __DRIVERS_PINCTRL_COMMON_H
#define __DRIVERS_PINCTRL_COMMON_H

#include <linux/pinctrl/pinconf-generic.h>
#include <linux/pinctrl/pinmux.h>

struct platform_device;

extern struct pinmux_ops clourney_pmx_ops;

/**
 * struct clourney_pin - describes a single kunlun pin
 * @mux_mode: the mux mode for this pin.
 * @config: the config for this pin.
 * @direct_val: output direction for this pin.
 * @config_need: whether config_reg setup needed.
 */
struct clourney_pin_memmap {
	unsigned int mux_mode;
    unsigned long config;
    unsigned int direct_val;
    unsigned int config_need;
};

struct clourney_pin {
	unsigned int pin;
	struct clourney_pin_memmap pin_memmap;
};

/**
 * struct clourney_pin_reg - describe a pin reg map
 * @mux_reg: mux register offset
 * @conf_reg: config register offset
 */
struct clourney_pin_reg {
	s16 mux_reg;
	s16 conf_reg;
};

/* decode a generic config into raw register value */
struct clourney_cfg_params_decode {
	enum pin_config_param param;
	u32 mask;
	u8 shift;
	bool invert;
};

struct clourney_pinctrl_soc_info {
	struct device *dev;
	const struct pinctrl_pin_desc *pins;
	unsigned int npins;
	struct clourney_pin_reg *pin_regs;
	unsigned int group_index;
	unsigned int flags;
	const char *gpr_compatible;
	struct mutex mutex;

	/* MUX_MODE shift and mask in case SHARE_MUX_CONF_REG */
	unsigned int mux_mask;
	u8 mux_shift;
	u32 ibe_bit;
	u32 obe_bit;

	/* generic pinconf */
	bool generic_pinconf;
	const struct pinconf_generic_params *custom_params;
	unsigned int num_custom_params;
	struct clourney_cfg_params_decode *decodes;
	unsigned int num_decodes;
	void (*fixup)(unsigned long *configs, unsigned int num_configs,
		      u32 *raw_config);

	int (*gpio_set_direction)(struct pinctrl_dev *pctldev,
				  struct pinctrl_gpio_range *range,
				  unsigned offset,
				  bool input);
};

/**
 * @dev: a pointer back to containing device
 * @base: the offset to the controller in virtual memory
 */
struct clourney_pinctrl {
	struct device *dev;
	struct pinctrl_dev *pctl;
	void __iomem *base;
	void __iomem *input_sel_base;
	struct clourney_pinctrl_soc_info *info;
};

#define DUBHE_PINCTRL_PIN(pin) PINCTRL_PIN(pin, #pin)
#endif /* __DRIVERS_PINCTRL_COMMON_H */
