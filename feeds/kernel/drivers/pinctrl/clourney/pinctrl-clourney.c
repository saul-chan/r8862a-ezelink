/*
 * Copyright (c) 2024, Clourney Semiconductor Limited and Contributors. All rights reserved.
 *
 */
#include <linux/err.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/mfd/syscon.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/pinctrl/machine.h>
#include <linux/pinctrl/pinconf.h>
#include <linux/pinctrl/pinctrl.h>
#include <linux/pinctrl/pinmux.h>
#include <linux/slab.h>
#include <linux/regmap.h>
#include "../core.h"
#include "../pinconf.h"
#include "../pinmux.h"
#include "pinctrl-common.h"

enum clourney_pads {
	DUBHE_PINCTRL_L0 = 0,
	DUBHE_PINCTRL_L1 = 1,
	DUBHE_PINCTRL_L2 = 2,
	DUBHE_PINCTRL_L3 = 3,
	DUBHE_PINCTRL_L4 = 4,
	DUBHE_PINCTRL_L5 = 5,
	DUBHE_PINCTRL_L6 = 6,
	DUBHE_PINCTRL_L7 = 7,
	DUBHE_PINCTRL_L8 = 8,
	DUBHE_PINCTRL_L9 = 9,
	DUBHE_PINCTRL_L10 = 10,
	DUBHE_PINCTRL_L11 = 11,
	DUBHE_PINCTRL_L12 = 12,
	DUBHE_PINCTRL_L13 = 13,
	DUBHE_PINCTRL_L14 = 14,
	DUBHE_PINCTRL_L15 = 15,
	DUBHE_PINCTRL_L16 = 16,
	DUBHE_PINCTRL_L17 = 17,
	DUBHE_PINCTRL_L18 = 18,
	DUBHE_PINCTRL_L19 = 19,
	DUBHE_PINCTRL_L20 = 20,
	DUBHE_PINCTRL_L21 = 21,
	DUBHE_PINCTRL_L22 = 22,
	DUBHE_PINCTRL_L23 = 23,
	DUBHE_PINCTRL_L24 = 24,
	DUBHE_PINCTRL_L25 = 25,
	DUBHE_PINCTRL_L26 = 26,
	DUBHE_PINCTRL_L27 = 27,
	DUBHE_PINCTRL_L28 = 28,
	DUBHE_PINCTRL_L29 = 29,
	DUBHE_PINCTRL_L30 = 30,
	DUBHE_PINCTRL_L31 = 31,
	DUBHE_PINCTRL_L32 = 32,
	DUBHE_PINCTRL_L33 = 33,
	DUBHE_PINCTRL_L34 = 34,
	DUBHE_PINCTRL_L35 = 35,
	DUBHE_PINCTRL_R0 = 36,
	DUBHE_PINCTRL_R1 = 37,
	DUBHE_PINCTRL_R2 = 38,
	DUBHE_PINCTRL_R3 = 39,
	DUBHE_PINCTRL_R4 = 40,
	DUBHE_PINCTRL_R5 = 41,
	DUBHE_PINCTRL_R6 = 42,
	DUBHE_PINCTRL_R7 = 43,
	DUBHE_PINCTRL_R8 = 44,
	DUBHE_PINCTRL_R9 = 45,
	DUBHE_PINCTRL_R10 = 46,
	DUBHE_PINCTRL_R11 = 47,
	DUBHE_PINCTRL_R12 = 48,
	DUBHE_PINCTRL_R13 = 49,
	DUBHE_PINCTRL_R14 = 50,
	DUBHE_PINCTRL_R15 = 51,
	DUBHE_PINCTRL_R16 = 52,
	DUBHE_PINCTRL_R17 = 53,
	DUBHE_PINCTRL_R18 = 54,
	DUBHE_PINCTRL_R19 = 55,
	DUBHE_PINCTRL_R20 = 56,
	DUBHE_PINCTRL_R21 = 57,
	DUBHE_PINCTRL_R22 = 58,
	DUBHE_PINCTRL_R23 = 59,
	DUBHE_PINCTRL_R24 = 60,
	DUBHE_PINCTRL_DR0 = 61,
	DUBHE_PINCTRL_DR1 = 62,
	DUBHE_PINCTRL_DR2 = 63,
	DUBHE_PINCTRL_DR3 = 64,
	DUBHE_PINCTRL_DR4 = 65,
	DUBHE_PINCTRL_DR5 = 66,
	DUBHE_PINCTRL_DR6 = 67,
	DUBHE_PINCTRL_DR7 = 68,
	DUBHE_PINCTRL_DR8 = 69,
	DUBHE_PINCTRL_DR9 = 70,
	DUBHE_PINCTRL_DR10 = 71,
	DUBHE_PINCTRL_DR11 = 72,
	DUBHE_PINCTRL_DR12 = 73,
	DUBHE_PINCTRL_DR13 = 74,
	DUBHE_PINCTRL_DL0 = 75,
	DUBHE_PINCTRL_DL1 = 76,
	DUBHE_PINCTRL_DL2 = 77,
	DUBHE_PINCTRL_DL3 = 78,
	DUBHE_PINCTRL_DL4 = 79,
	DUBHE_PINCTRL_DL5 = 80,
};

/* Pad names for the pinmux subsystem */
static const struct pinctrl_pin_desc clourney_pinctrl_pads[] = {
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_L0),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_L1),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_L2),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_L3),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_L4),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_L5),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_L6),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_L7),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_L8),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_L9),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_L10),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_L11),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_L12),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_L13),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_L14),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_L15),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_L16),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_L17),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_L18),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_L19),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_L20),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_L21),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_L22),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_L23),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_L24),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_L25),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_L26),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_L27),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_L28),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_L29),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_L30),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_L31),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_L32),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_L33),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_L34),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_L35),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_R0),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_R1),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_R2),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_R3),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_R4),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_R5),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_R6),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_R7),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_R8),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_R9),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_R10),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_R11),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_R12),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_R13),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_R14),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_R15),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_R16),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_R17),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_R18),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_R19),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_R20),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_R21),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_R22),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_R23),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_R24),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_DR0),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_DR1),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_DR2),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_DR3),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_DR4),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_DR5),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_DR6),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_DR7),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_DR8),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_DR9),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_DR10),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_DR11),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_DR12),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_DR13),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_DL0),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_DL1),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_DL2),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_DL3),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_DL4),
	DUBHE_PINCTRL_PIN(DUBHE_PINCTRL_DL5),
};

static int clourney_pinconf_get(struct pinctrl_dev *pctldev,
			unsigned pin_id, unsigned long *config)
{
	return 0;
}

static int clourney_pinconf_set(struct pinctrl_dev *pctldev,
		unsigned pin_id, unsigned long *configs,
		unsigned num_configs)
{
	return 0;
}

static void clourney_pinconf_dbg_show(struct pinctrl_dev *pctldev,
			struct seq_file *s, unsigned pin_id)
{
	;
}

static int clourney_pmx_set_one_pin_mem(struct clourney_pinctrl *ipctl, struct clourney_pin *pin)
{
	return 0;
}

static int clourney_pinctrl_parse_pin_mem(struct clourney_pinctrl_soc_info *info,
			unsigned int *grp_pin_id, struct clourney_pin *pin,
			const __be32 **list_p, u32 generic_config)
{
	return 0;
}

static inline const struct group_desc *clourney_pinctrl_find_group_by_name(
				struct pinctrl_dev *pctldev,
				const char *name)
{
	const struct group_desc *grp = NULL;
	int i;

	for (i = 0; i < pctldev->num_groups; i++) {
		grp = pinctrl_generic_get_group(pctldev, i);
		if (grp && !strcmp(grp->name, name))
			break;
	}

	return grp;
}

static void clourney_pin_dbg_show(struct pinctrl_dev *pctldev, struct seq_file *s,
			unsigned offset)
{
	seq_printf(s, "%s", dev_name(pctldev->dev));
}

static int clourney_dt_node_to_map(struct pinctrl_dev *pctldev,
			struct device_node *np,
			struct pinctrl_map **map, unsigned *num_maps)
{
	struct clourney_pinctrl *ipctl = pinctrl_dev_get_drvdata(pctldev);
	struct clourney_pinctrl_soc_info *info = ipctl->info;
	const struct group_desc *grp;
	struct pinctrl_map *new_map;
	struct device_node *parent;
	int map_num = 1;
	int i, j;

	/*
	 * first find the group of this node and check if we need create
	 * config maps for pins
	 */
	grp = clourney_pinctrl_find_group_by_name(pctldev, np->name);
	if (!grp) {
		dev_err(info->dev, "unable to find group for node %s\n",
			np->name);
		return -EINVAL;
	}

	for (i = 0; i < grp->num_pins; i++) {
		struct clourney_pin *pin = &((struct clourney_pin *)(grp->data))[i];

	if (pin->pin_memmap.config_need)
		map_num++;
	}

	new_map = kmalloc(sizeof(struct pinctrl_map) * map_num, GFP_KERNEL);
	if (!new_map)
		return -ENOMEM;

	*map = new_map;
	*num_maps = map_num;

	/* create mux map */
	parent = of_get_parent(np);
	if (!parent) {
		kfree(new_map);
		return -EINVAL;
	}
	new_map[0].type = PIN_MAP_TYPE_MUX_GROUP;
	new_map[0].data.mux.function = parent->name;
	new_map[0].data.mux.group = np->name;
	of_node_put(parent);

	/* create config map */
	new_map++;
	for (i = j = 0; i < grp->num_pins; i++) {
		struct clourney_pin *pin = &((struct clourney_pin *)(grp->data))[i];

	if (pin->pin_memmap.config_need) {
			new_map[j].type = PIN_MAP_TYPE_CONFIGS_PIN;
			new_map[j].data.configs.group_or_pin =
					pin_get_name(pctldev, pin->pin);
			new_map[j].data.configs.configs =
					&pin->pin_memmap.config;
			new_map[j].data.configs.num_configs = 1;
			j++;
		}
	}

	return 0;
}

static void clourney_dt_free_map(struct pinctrl_dev *pctldev,
				struct pinctrl_map *map, unsigned num_maps)
{
	kfree(map);
}

static const struct pinctrl_ops clourney_pctrl_ops = {
	.get_groups_count = pinctrl_generic_get_group_count,
	.get_group_name = pinctrl_generic_get_group_name,
	.get_group_pins = pinctrl_generic_get_group_pins,
	.pin_dbg_show = clourney_pin_dbg_show,
	.dt_node_to_map = clourney_dt_node_to_map,
	.dt_free_map = clourney_dt_free_map,
};

static int clourney_pmx_set(struct pinctrl_dev *pctldev, unsigned selector,
		       unsigned group)
{
	struct clourney_pinctrl *ipctl = pinctrl_dev_get_drvdata(pctldev);
	unsigned int npins;
	int i, err;
	struct group_desc *grp = NULL;
	struct function_desc *func = NULL;

	/*
	 * Configure the mux mode for each pin in the group for a specific
	 * function.
	 */
	grp = pinctrl_generic_get_group(pctldev, group);
	if (!grp)
		return -EINVAL;

	func = pinmux_generic_get_function(pctldev, selector);
	if (!func)
		return -EINVAL;

	npins = grp->num_pins;

	for (i = 0; i < npins; i++) {
		struct clourney_pin *pin = &((struct clourney_pin *)(grp->data))[i];

		err = clourney_pmx_set_one_pin_mem(ipctl, pin);
		if (err)
			return err;
	}

	return 0;
}

struct pinmux_ops clourney_pmx_ops = {
	.get_functions_count = pinmux_generic_get_function_count,
	.get_function_name = pinmux_generic_get_function_name,
	.get_function_groups = pinmux_generic_get_function_groups,
	.set_mux = clourney_pmx_set,
};

/* decode generic config into raw register values */
static u32 clourney_pinconf_decode_generic_config(struct clourney_pinctrl *ipctl,
					      unsigned long *configs,
					      unsigned int num_configs)
{
	struct clourney_pinctrl_soc_info *info = ipctl->info;
	struct clourney_cfg_params_decode *decode;
	enum pin_config_param param;
	u32 raw_config = 0;
	u32 param_val;
	int i, j;

	WARN_ON(num_configs > info->num_decodes);

	for (i = 0; i < num_configs; i++) {
		param = pinconf_to_config_param(configs[i]);
		param_val = pinconf_to_config_argument(configs[i]);
		decode = info->decodes;
		for (j = 0; j < info->num_decodes; j++) {
			if (param == decode->param) {
				if (decode->invert)
					param_val = !param_val;
				raw_config |= (param_val << decode->shift)
					      & decode->mask;
				break;
			}
			decode++;
		}
	}

	if (info->fixup)
		info->fixup(configs, num_configs, &raw_config);

	return raw_config;
}

static u32 clourney_pinconf_parse_generic_config(struct device_node *np,
					    struct clourney_pinctrl *ipctl)
{
	struct clourney_pinctrl_soc_info *info = ipctl->info;
	struct pinctrl_dev *pctl = ipctl->pctl;
	unsigned int num_configs;
	unsigned long *configs;
	int ret;

	if (!info->generic_pinconf)
		return 0;

	ret = pinconf_generic_parse_dt_config(np, pctl, &configs,
					      &num_configs);
	if (ret)
		return 0;

	return clourney_pinconf_decode_generic_config(ipctl, configs, num_configs);
}

static void clourney_pinconf_group_dbg_show(struct pinctrl_dev *pctldev,
					 struct seq_file *s, unsigned group)
{
	struct group_desc *grp;
	unsigned long config;
	const char *name;
	int i, ret;

	if (group >= pctldev->num_groups)
		return;

	seq_printf(s, "\n");
	grp = pinctrl_generic_get_group(pctldev, group);
	if (!grp)
		return;

	for (i = 0; i < grp->num_pins; i++) {
		struct clourney_pin *pin = &((struct clourney_pin *)(grp->data))[i];

		name = pin_get_name(pctldev, pin->pin);
		ret = clourney_pinconf_get(pctldev, pin->pin, &config);
		if (ret)
			return;
		seq_printf(s, "  %s: 0x%lx\n", name, config);
	}
}

static const struct pinconf_ops clourney_pinconf_ops = {
	.pin_config_get = clourney_pinconf_get,
	.pin_config_set = clourney_pinconf_set,
	.pin_config_dbg_show = clourney_pinconf_dbg_show,
	.pin_config_group_dbg_show = clourney_pinconf_group_dbg_show,
};


/*  5 u32 * 4 */
#define CLOURNEY_PIN_SIZE 20

static int clourney_pinctrl_parse_groups(struct device_node *np,
				    struct group_desc *grp,
				    struct clourney_pinctrl *ipctl,
				    u32 index)
{
	struct clourney_pinctrl_soc_info *info = ipctl->info;
	int size, pin_size;
	const __be32 *list, **list_p;
	u32 config;
	int i;

	pin_size = CLOURNEY_PIN_SIZE;

	if (info->generic_pinconf)
		pin_size -= 4;

	/* Initialise group */
	grp->name = np->name;

	/*
	 * the binding format is dubhe,pins = <PIN_FUNC_ID CONFIG ...>,
	 * do sanity check and calculate pins number
	 *
	 * First try legacy 'dubhe,pins' property, then fall back to the
	 * generic 'pinmux'.
	 *
	 * Note: for generic 'pinmux' case, there's no CONFIG part in
	 * the binding format.
	 */
	list = of_get_property(np, "dubhe,pins", &size);
	if (!list) {
		list = of_get_property(np, "pinmux", &size);
		if (!list) {
			dev_err(info->dev,
				"no dubhe,pins and pins property in node %pOF\n", np);
			return -EINVAL;
		}
	}

	list_p = &list;

	/* we do not check return since it's safe node passed down */
	if (!size || size % pin_size) {
		dev_err(info->dev, "Invalid dubhe,pins or pins property in node %pOF\n", np);
		return -EINVAL;
	}

	/* first try to parse the generic pin config */
	config = clourney_pinconf_parse_generic_config(np, ipctl);

	grp->num_pins = size / pin_size;
	grp->data = devm_kzalloc(info->dev, grp->num_pins *
				 sizeof(struct clourney_pin), GFP_KERNEL);
	grp->pins = devm_kzalloc(info->dev, grp->num_pins *
				 sizeof(unsigned int), GFP_KERNEL);
	if (!grp->pins || !grp->data)
		return -ENOMEM;

	for (i = 0; i < grp->num_pins; i++) {
		struct clourney_pin *pin = &((struct clourney_pin *)(grp->data))[i];

		clourney_pinctrl_parse_pin_mem(info, &grp->pins[i],
			pin, list_p, config);
	}

	return 0;
}

static int clourney_pinctrl_parse_functions(struct device_node *np,
				struct clourney_pinctrl *ipctl,
				u32 index)
{
	struct pinctrl_dev *pctl = ipctl->pctl;
	struct clourney_pinctrl_soc_info *info = ipctl->info;
	struct device_node *child;
	struct function_desc *func;
	struct group_desc *grp;
	u32 i = 0;

	func = pinmux_generic_get_function(pctl, index);
	if (!func)
		return -EINVAL;

	/* Initialise function */
	func->name = np->name;
	func->num_group_names = of_get_child_count(np);
	if (func->num_group_names == 0) {
		dev_err(info->dev, "no groups defined in %pOF\n", np);
		return -EINVAL;
	}
	func->group_names = devm_kcalloc(info->dev, func->num_group_names,
					 sizeof(char *), GFP_KERNEL);
	if (!func->group_names)
		return -ENOMEM;

	for_each_child_of_node(np, child) {
		func->group_names[i] = child->name;

		grp = devm_kzalloc(info->dev, sizeof(struct group_desc),
				   GFP_KERNEL);
		if (!grp)
			return -ENOMEM;

		mutex_lock(&info->mutex);
		radix_tree_insert(&pctl->pin_group_tree,
				  info->group_index++, grp);
		mutex_unlock(&info->mutex);

		clourney_pinctrl_parse_groups(child, grp, ipctl, i++);
	}

	return 0;
}

/*
 * Check if the DT contains pins in the direct child nodes. This indicates the
 * newer DT format to store pins. This function returns true if the first found
 * dubhe,pins property is in a child of np. Otherwise false is returned.
 */
static bool clourney_pinctrl_dt_is_flat_functions(struct device_node *np)
{
	struct device_node *function_np;
	struct device_node *pinctrl_np;

	for_each_child_of_node(np, function_np) {
		if (of_property_read_bool(function_np, "dubhe,pins"))
			return true;

		for_each_child_of_node(function_np, pinctrl_np) {
			if (of_property_read_bool(pinctrl_np, "dubhe,pins"))
				return false;
		}
	}

	return true;
}

static int clourney_pinctrl_probe_dt(struct platform_device *pdev,
				struct clourney_pinctrl *ipctl)
{
	struct device_node *np = pdev->dev.of_node;
	struct device_node *child;
	struct pinctrl_dev *pctl = ipctl->pctl;
	struct clourney_pinctrl_soc_info *info = ipctl->info;
	u32 nfuncs = 0;
	u32 i = 0;
	bool flat_funcs;

	if (!np)
		return -ENODEV;

	flat_funcs = clourney_pinctrl_dt_is_flat_functions(np);
	if (flat_funcs) {
		nfuncs = 1;
	} else {
		nfuncs = of_get_child_count(np);
		if (nfuncs <= 0) {
			dev_err(&pdev->dev, "no functions defined\n");
			return -EINVAL;
		}
	}

	for (i = 0; i < nfuncs; i++) {
		struct function_desc *function;

		function = devm_kzalloc(&pdev->dev, sizeof(*function),
					GFP_KERNEL);
		if (!function)
			return -ENOMEM;

		mutex_lock(&info->mutex);
		radix_tree_insert(&pctl->pin_function_tree, i, function);
		mutex_unlock(&info->mutex);
	}
	pctl->num_functions = nfuncs;

	info->group_index = 0;
	if (flat_funcs) {
		pctl->num_groups = of_get_child_count(np);
	} else {
		pctl->num_groups = 0;
		for_each_child_of_node(np, child)
			pctl->num_groups += of_get_child_count(child);
	}

	if (flat_funcs) {
		clourney_pinctrl_parse_functions(np, ipctl, 0);
	} else {
		i = 0;
		for_each_child_of_node(np, child)
			clourney_pinctrl_parse_functions(child, ipctl, i++);
	}

	return 0;
}

/*
 * clourney_free_resources() - free memory used by this driver
 * @info: info driver instance
 */
static void clourney_free_resources(struct clourney_pinctrl *ipctl)
{
	if (ipctl->pctl)
		pinctrl_unregister(ipctl->pctl);
}

static int __clourney_pinctrl_probe(struct platform_device *pdev,
		struct clourney_pinctrl_soc_info *info)
{
	struct regmap_config config = { .name = "gpr" };
	struct device_node *dev_np = pdev->dev.of_node;
	struct pinctrl_desc *clourney_pinctrl_desc;
	struct device_node *np;
	struct clourney_pinctrl *ipctl;

	struct regmap *gpr;
	int ret, i;

	if (!info || !info->pins || !info->npins) {
		dev_err(&pdev->dev, "wrong pinctrl info\n");
		return -EINVAL;
	}
	info->dev = &pdev->dev;

	if (info->gpr_compatible) {
		gpr = syscon_regmap_lookup_by_compatible(info->gpr_compatible);
		if (!IS_ERR(gpr))
			regmap_attach_dev(&pdev->dev, gpr, &config);
	}

	/* Create state holders etc for this driver */
	ipctl = devm_kzalloc(&pdev->dev, sizeof(*ipctl), GFP_KERNEL);
	if (!ipctl)
		return -ENOMEM;

	info->pin_regs = devm_kmalloc(&pdev->dev, sizeof(*info->pin_regs) *
				      info->npins, GFP_KERNEL);
	if (!info->pin_regs)
		return -ENOMEM;

	for (i = 0; i < info->npins; i++) {
		info->pin_regs[i].mux_reg = -1;
		info->pin_regs[i].conf_reg = -1;
	}

	if (of_property_read_bool(dev_np, "dubhe,input-sel")) {
		np = of_parse_phandle(dev_np, "dubhe,input-sel", 0);
		if (!np) {
			dev_err(&pdev->dev, "pinctrl dubhe,input-sel property not found\n");
			return -EINVAL;
		}

		ipctl->input_sel_base = of_iomap(np, 0);
		of_node_put(np);
		if (!ipctl->input_sel_base) {
			dev_err(&pdev->dev,
				"pinctrl input select base address not found\n");
			return -ENOMEM;
		}
	}

	clourney_pinctrl_desc = devm_kzalloc(&pdev->dev, sizeof(*clourney_pinctrl_desc),
					GFP_KERNEL);
	if (!clourney_pinctrl_desc)
		return -ENOMEM;

	clourney_pinctrl_desc->name = dev_name(&pdev->dev);
	clourney_pinctrl_desc->pins = info->pins;
	clourney_pinctrl_desc->npins = info->npins;
	clourney_pinctrl_desc->pctlops = &clourney_pctrl_ops;
	clourney_pinctrl_desc->pmxops = &clourney_pmx_ops;
	clourney_pinctrl_desc->confops = &clourney_pinconf_ops;
	clourney_pinctrl_desc->owner = THIS_MODULE;

	/* for generic pinconf */
	clourney_pinctrl_desc->custom_params = info->custom_params;
	clourney_pinctrl_desc->num_custom_params = info->num_custom_params;

	/* platform specific callback */
	clourney_pmx_ops.gpio_set_direction = info->gpio_set_direction;

	mutex_init(&info->mutex);

	ipctl->info = info;
	ipctl->dev = info->dev;
	platform_set_drvdata(pdev, ipctl);
	ret = devm_pinctrl_register_and_init(&pdev->dev,
					     clourney_pinctrl_desc, ipctl,
					     &ipctl->pctl);
	if (ret) {
		dev_err(&pdev->dev, "could not register clourney pinctrl driver\n");
		goto free;
	}

	ret = clourney_pinctrl_probe_dt(pdev, ipctl);
	if (ret) {
		dev_err(&pdev->dev, "fail to probe dt properties\n");
		goto free;
	}

	dev_info(&pdev->dev, "initialized clourney pinctrl driver\n");

	return pinctrl_enable(ipctl->pctl);

free:
	clourney_free_resources(ipctl);

	return ret;
}

static int clourney_pinctrl_suspend(struct device *dev)
{
	struct clourney_pinctrl *ipctl = dev_get_drvdata(dev);

	if (!ipctl)
		return -EINVAL;

	return pinctrl_force_sleep(ipctl->pctl);
}

static int clourney_pinctrl_resume(struct device *dev)
{
	struct clourney_pinctrl *ipctl = dev_get_drvdata(dev);

	if (!ipctl)
		return -EINVAL;

	return pinctrl_force_default(ipctl->pctl);
}

static struct clourney_pinctrl_soc_info clourney_pinctrl_info = {
	.pins = clourney_pinctrl_pads,
	.npins = ARRAY_SIZE(clourney_pinctrl_pads),
};

static int clourney_pinctrl_probe(struct platform_device *pdev)
{

	return __clourney_pinctrl_probe(pdev, &clourney_pinctrl_info);
}

static struct of_device_id clourney_pinctrl_of_match[] = {
	{ .compatible = "clourney,clourney-dubhe-pinctrl", },
};

static const struct dev_pm_ops clourney_pinctrl_pm_ops = {
		SET_LATE_SYSTEM_SLEEP_PM_OPS(clourney_pinctrl_suspend, clourney_pinctrl_resume)
};

static struct platform_driver clourney_pinctrl_driver = {
	.driver = {
		.name = "clourney,clourney-dubhe-pinctrl",
		.of_match_table = of_match_ptr(clourney_pinctrl_of_match),
		.pm = &clourney_pinctrl_pm_ops,
	},
	.probe = clourney_pinctrl_probe,
};

static int __init clourney_pinctrl_init(void)
{
	return platform_driver_register(&clourney_pinctrl_driver);
}
arch_initcall(clourney_pinctrl_init);
