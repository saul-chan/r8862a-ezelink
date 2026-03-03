/*
 * Copyright (c) 2022, Clourney Semiconductor Limited and Contributors. All rights reserved.
 *
 */
#include <linux/acpi.h>
#include <linux/gpio/driver.h>
#include <linux/gpio.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/irq.h>
#include <linux/irqdomain.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/property.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include "gpiolib.h"

#define GPIO_DIR_INPUT 		1
#define GPIO_DIR_OUTPUT 	0

/*
 * gpio out data register
 */
#define GPIO_OUT_DATA_PORT_1		    0x00
#define GPIO_OUT_DATA_PORT_SIZE	        0x0c
#define GPIO_OUT_DATA_PORT_X(n) \	
	(GPIO_OUT_DATA_PORT_1 + ((n) * GPIO_OUT_DATA_PORT_SIZE))

#define GPIO_DIR_PORT_1			        0x4
#define GPIO_DIR_PORT_SIZE  	        0x0c
#define GPIO_DIR_PORT_X(n) \
	(GPIO_DIR_PORT_1 + ((n) * GPIO_DIR_PORT_SIZE))

#define GPIO_DATA_SOURCE_PORT_1	        0x8
#define GPIO_DATA_SOURCE_PORT_SIZE      0x0c
#define GPIO_DATA_SOURCE_PORT_X(n) \
	(GPIO_DATA_SOURCE_PORT_1 + ((n) * GPIO_DATA_SOURCE_PORT_SIZE))

#define GPIO_INT_EN_PORT_1			    0x30
#define GPIO_INT_EN_PORT_SIZE  	 	    0x0
#define GPIO_INT_EN_PORT_X(n) \
	(GPIO_INT_EN_PORT_1 + ((n) * GPIO_INT_EN_PORT_SIZE))

#define GPIO_INT_MASK_PORT_1		    0x34
#define GPIO_INT_MASK_PORT_SIZE 	    0x0
#define GPIO_INT_MASK_PORT_X(n) \
	(GPIO_INT_MASK_PORT_1 + ((n) * GPIO_INT_MASK_PORT_SIZE))

#define GPIO_INT_TYPE_PORT_1		    0x38
#define GPIO_INT_TYPE_PORT_SIZE		    0x0
#define GPIO_INT_TYPE_PORT_X(n) \
	(GPIO_INT_TYPE_PORT_1 + ((n) * GPIO_INT_TYPE_PORT_SIZE))

#define GPIO_INT_POL_PORT_1			    0x3c
#define GPIO_INT_POL_PORT_SIZE          0x0
#define GPIO_INT_POL_PORT_X(n) \
	(GPIO_INT_POL_PORT_1 + ((n) * GPIO_INT_POL_PORT_SIZE))

#define GPIO_INT_STATUS_PORT_1			0x40
#define GPIO_INT_STATUS_PORT_SIZE   	0x0
#define GPIO_INT_STATUS_PORT_X(n) \
	(GPIO_INT_STATUS_PORT_1 + ((n) * GPIO_INT_STATUS_PORT_SIZE))

#define GPIO_INT_EDGE_CLR_PORT_1	    0x4c	
#define GPIO_INT_EDGE_CLR_PORT_SIZE		0x0
#define GPIO_INT_EDGE_CLR_PORT_X(n) \
	(GPIO_INT_EDGE_CLR_PORT_1 + ((n) * GPIO_INT_EDGE_CLR_PORT_SIZE))

#define GPIO_INT_DEB_EN_PORT_1		    0x48
#define GPIO_INT_DEB_EN_PORT_SIZE       0x0
#define GPIO_INT_DEB_EN_PORT_X(n) \
	(GPIO_INT_DEB_EN_PORT_1 + ((n) * GPIO_INT_DEB_EN_PORT_SIZE))

#define GPIO_IN_DATA_PORT_1		        0x50
#define GPIO_IN_DATA_PORT_SIZE		    0x0c
#define GPIO_IN_DATA_PORT_X(n) \	
	(GPIO_IN_DATA_PORT_1 + ((n) * GPIO_IN_DATA_PORT_SIZE))

#define GPIO_INT_LEV_SYNC_PORT_1		0x60
#define GPIO_INT_LEV_SYNC_PORT_SIZE     0x0
#define GPIO_INT_LEV_SYNC_PORT_X(n) \
	(GPIO_INT_LEV_SYNC_PORT_1 + ((n) * GPIO_INT_LEV_SYNC_PORT_SIZE))

#define GPIO_INT_BOTH_EDGE_PORT_1		0x68
#define GPIO_INT_BOTH_EDGE_PORT_SIZE    0x0
#define GPIO_INT_BOTH_EDGE_PORT_X(n) \
	(GPIO_INT_BOTH_EDGE_PORT_1 + ((n) * GPIO_INT_BOTH_EDGE_PORT_SIZE))

#define CLOURNERY_MAX_PORTS		3

struct clourney_port_property {
	struct fwnode_handle *fwnode;
	unsigned int	idx;
	unsigned int	ngpio;
	unsigned int	gpio_base;
	unsigned int	irq;
	bool		    irq_shared;
	unsigned int	gpio_ranges[4];
};

struct clourney_gpio;

#ifdef CONFIG_PM_SLEEP
/* Store GPIO context across system-wide suspend/resume transitions */
struct clourney_context {
	u32 data;
	u32 dir;
	u32 ext;
	u32 int_en;
	u32 int_mask;
	u32 int_type;
	u32 int_pol;
	u32 int_deb;
};
#endif

struct clourney_gpio_port {
	struct gpio_chip	gc;
	bool			is_registered;
	struct clourney_gpio	*gpio;
#ifdef CONFIG_PM_SLEEP
	struct clourney_context	*ctx;
#endif
	unsigned int		idx;
	struct irq_domain	*domain;  /* each port has one domain */
};

struct clourney_gpio {
	struct	device		*dev;
	void __iomem		*regs;
	struct clourney_gpio_port	*ports;
	unsigned int		nr_ports;
};

static inline u32 clourney_read(struct clourney_gpio *gpio, unsigned int offset)
{
	void __iomem *reg_base	= gpio->regs;
	return readl(reg_base + offset);
}

static inline void clourney_write(struct clourney_gpio *gpio, unsigned int offset,
			       u32 val)
{
	void __iomem *reg_base	= gpio->regs;
	writel(val, reg_base + offset);
}

static int clourney_gpio_to_irq(struct gpio_chip *gc, unsigned offset)
{
	struct clourney_gpio_port *port = gpiochip_get_data(gc);
	int irq;

	irq = irq_find_mapping(port->domain, offset);

	return irq;
}

static u32 clourney_do_irq(struct clourney_gpio_port *port)
{
	unsigned int port_idx = 0;
	int bit = 0;
	unsigned long irq_status = 0;
	unsigned long clear_int, reg_mask_read, ret = 1;
	int gpio_irq;
	struct clourney_gpio *gpio = port->gpio;

	port_idx = port->idx;

	irq_status = clourney_read(gpio, GPIO_INT_STATUS_PORT_X(port_idx));
	reg_mask_read = clourney_read(gpio, GPIO_INT_MASK_PORT_X(port_idx));

	/* mask the port INT */
	clourney_write(gpio,  GPIO_INT_MASK_PORT_X(port_idx), 0xffffffff);

	for_each_set_bit(bit, &irq_status, 32) {
		gpio_irq = irq_find_mapping(port->domain, bit);
		clear_int = clourney_read(gpio, GPIO_INT_EDGE_CLR_PORT_X(port_idx));
		/* clear INT */
		clourney_write(gpio, GPIO_INT_EDGE_CLR_PORT_X(port_idx), clear_int | BIT(bit));
		/* Invoke interrupt handler */
		if (generic_handle_irq(gpio_irq))
			ret = 0;
	}
	/* restore irq mask */
	clourney_write(gpio,  GPIO_INT_MASK_PORT_X(port_idx), reg_mask_read);
	return ret;
}

static void clourney_irq_handler(struct irq_desc *desc)
{
	struct clourney_gpio_port *port = irq_desc_get_handler_data(desc);
	struct irq_chip *chip = irq_desc_get_chip(desc);
	clourney_do_irq(port);
	if (chip->irq_eoi)
		chip->irq_eoi(irq_desc_get_irq_data(desc));
}

static void clourney_irq_enable(struct irq_data *d)
{
	struct irq_chip_generic *igc = irq_data_get_irq_chip_data(d);
	struct clourney_gpio_port *port = igc->private;
	struct clourney_gpio *gpio = port->gpio;
	struct gpio_chip *gc;
	unsigned long flags;
	u32 val;
	unsigned int reg_idx;
	unsigned int bit_idx;

	reg_idx = port->idx;
	bit_idx = d->hwirq;

	gc = &port->gc;

	spin_lock_irqsave(&gc->bgpio_lock, flags);

	/* unmask */
	val = clourney_read(gpio, GPIO_INT_MASK_PORT_X(reg_idx));
	val &= ~BIT(bit_idx);
	clourney_write(gpio,  GPIO_INT_MASK_PORT_X(reg_idx), val);

	/* INT EN */
	val = clourney_read(gpio, GPIO_INT_EN_PORT_X(reg_idx));
	val |= BIT(bit_idx);
	clourney_write(gpio,  GPIO_INT_EN_PORT_X(reg_idx), val);

	spin_unlock_irqrestore(&gc->bgpio_lock, flags);
}

static void clourney_irq_disable(struct irq_data *d)
{
	struct irq_chip_generic *igc = irq_data_get_irq_chip_data(d);
	struct clourney_gpio_port *port = igc->private;
	struct clourney_gpio *gpio = port->gpio;
	struct gpio_chip *gc;
	unsigned long flags;
	u32 val;
	unsigned int reg_idx = 0;
	unsigned int bit_idx;

	reg_idx = port->idx;
	bit_idx = d->hwirq;

	gc = &port->gc;

	spin_lock_irqsave(&gc->bgpio_lock, flags);
	val = clourney_read(gpio, GPIO_INT_EN_PORT_X(reg_idx));
	val &= ~BIT(bit_idx);
	clourney_write(gpio, GPIO_INT_EN_PORT_X(reg_idx), val);
	spin_unlock_irqrestore(&gc->bgpio_lock, flags);
}

static int clourney_irq_reqres(struct irq_data *d)
{
	struct irq_chip_generic *igc = irq_data_get_irq_chip_data(d);
	struct clourney_gpio_port *port = igc->private;
	struct clourney_gpio *gpio = port->gpio;
	struct gpio_chip *gc;

	gc = &port->gc;
	if (gpiochip_lock_as_irq(gc, irqd_to_hwirq(d))) {
		dev_err(gpio->dev, "unable to lock HW IRQ %lu for IRQ\n",
			irqd_to_hwirq(d));
		return -EINVAL;
	}
	return 0;
}

static void clourney_irq_relres(struct irq_data *d)
{
	struct irq_chip_generic *igc = irq_data_get_irq_chip_data(d);
	struct clourney_gpio_port *port = igc->private;
	struct gpio_chip *gc;

	gc = &port->gc;
	gpiochip_unlock_as_irq(gc, irqd_to_hwirq(d));
}

static int clourney_irq_set_type(struct irq_data *d, u32 type)
{
	struct irq_chip_generic *igc = irq_data_get_irq_chip_data(d);
	struct clourney_gpio_port *port = igc->private;
	struct clourney_gpio *gpio = port->gpio;
	struct gpio_chip *gc;
	unsigned long flags;
	unsigned int reg_idx = 0, val;
	unsigned int bit_idx;

	reg_idx = port->idx;

	gc = &port->gc;

	if (type & ~(IRQ_TYPE_EDGE_RISING | IRQ_TYPE_EDGE_FALLING |
		     IRQ_TYPE_LEVEL_HIGH | IRQ_TYPE_LEVEL_LOW | IRQ_TYPE_EDGE_BOTH))
		return -EINVAL;

	spin_lock_irqsave(&gc->bgpio_lock, flags);

	bit_idx = d->hwirq;
	
	switch (type) {
	case IRQ_TYPE_EDGE_BOTH:
		/* EDGE_BOTH */
		val = clourney_read(gpio, GPIO_INT_TYPE_PORT_X(reg_idx));
		val |= BIT(bit_idx); //1:edge sensitive
		clourney_write(gpio, GPIO_INT_TYPE_PORT_X(reg_idx), val);
		val = clourney_read(gpio, GPIO_INT_POL_PORT_X(reg_idx));
		val &= ~BIT(bit_idx); //0:Active Low polarity
		clourney_write(gpio, GPIO_INT_POL_PORT_X(reg_idx), val);
		val = clourney_read(gpio, GPIO_INT_BOTH_EDGE_PORT_X(reg_idx));
		val |= BIT(bit_idx); //1:both edge sensitive
		clourney_write(gpio, GPIO_INT_BOTH_EDGE_PORT_X(reg_idx), val);
		break;

	case IRQ_TYPE_EDGE_RISING:
		/* EDGE + HIGH active */
		val = clourney_read(gpio, GPIO_INT_TYPE_PORT_X(reg_idx));
		val |= BIT(bit_idx); //1:edge sensitive
		clourney_write(gpio, GPIO_INT_TYPE_PORT_X(reg_idx), val);
		val = clourney_read(gpio, GPIO_INT_POL_PORT_X(reg_idx));
		val |= BIT(bit_idx); //1:Active High polarity
		clourney_write(gpio, GPIO_INT_POL_PORT_X(reg_idx), val);
		val = clourney_read(gpio, GPIO_INT_BOTH_EDGE_PORT_X(reg_idx));
		val &= ~BIT(bit_idx); //0:single edge sensitive
		clourney_write(gpio, GPIO_INT_BOTH_EDGE_PORT_X(reg_idx), val);
		break;

	case IRQ_TYPE_EDGE_FALLING:
		/* EDGE + LOW active */
		val = clourney_read(gpio, GPIO_INT_TYPE_PORT_X(reg_idx));
		val |= BIT(bit_idx); //1:edge sensitive
		clourney_write(gpio, GPIO_INT_TYPE_PORT_X(reg_idx), val);
		val = clourney_read(gpio, GPIO_INT_POL_PORT_X(reg_idx));
		val &= ~BIT(bit_idx); //0:Active Low polarity
		clourney_write(gpio, GPIO_INT_POL_PORT_X(reg_idx), val);
		val = clourney_read(gpio, GPIO_INT_BOTH_EDGE_PORT_X(reg_idx));
		val &= ~BIT(bit_idx); //0:single edge sensitive
		clourney_write(gpio, GPIO_INT_BOTH_EDGE_PORT_X(reg_idx), val);
		break;

	case IRQ_TYPE_LEVEL_HIGH:
		/* LEVEL + HIGH active */
		val = clourney_read(gpio, GPIO_INT_TYPE_PORT_X(reg_idx));
		val &= ~BIT(bit_idx); //0:level sensitive
		clourney_write(gpio, GPIO_INT_TYPE_PORT_X(reg_idx), val);
		val = clourney_read(gpio, GPIO_INT_POL_PORT_X(reg_idx));
		val |= BIT(bit_idx); //1:Active High polarity
		clourney_write(gpio, GPIO_INT_POL_PORT_X(reg_idx), val);
		val = clourney_read(gpio, GPIO_INT_BOTH_EDGE_PORT_X(reg_idx));
		val &= ~BIT(bit_idx); //0:single edge sensitive
		clourney_write(gpio, GPIO_INT_BOTH_EDGE_PORT_X(reg_idx), val);
		break;

	case IRQ_TYPE_LEVEL_LOW:
		/* LEVEL + LOW active */
		val = clourney_read(gpio, GPIO_INT_TYPE_PORT_X(reg_idx));
		val &= ~BIT(bit_idx); //0:level sensitive
		clourney_write(gpio, GPIO_INT_TYPE_PORT_X(reg_idx), val);
		val = clourney_read(gpio, GPIO_INT_POL_PORT_X(reg_idx));
		val &= ~BIT(bit_idx); //0:Active Low polarity
		clourney_write(gpio, GPIO_INT_POL_PORT_X(reg_idx), val);
		val = clourney_read(gpio, GPIO_INT_BOTH_EDGE_PORT_X(reg_idx));
		val &= ~BIT(bit_idx); //0:single edge sensitive
		clourney_write(gpio, GPIO_INT_BOTH_EDGE_PORT_X(reg_idx), val);
		break;
	default:
		break;
	}

	irq_setup_alt_chip(d, type);

	spin_unlock_irqrestore(&gc->bgpio_lock, flags);

	return 0;
}

static int clourney_gpio_set_debounce(struct gpio_chip *gc,
				   unsigned offset, unsigned debounce)
{
	struct clourney_gpio_port *port = gpiochip_get_data(gc);
	struct clourney_gpio *gpio = port->gpio;
	unsigned long flags, val_deb, mask;
	unsigned int reg_idx = port->idx;

	spin_lock_irqsave(&gc->bgpio_lock, flags);

	mask = BIT(offset);

	val_deb = clourney_read(gpio, GPIO_INT_DEB_EN_PORT_X(reg_idx));
	if (debounce)
		clourney_write(gpio,
			GPIO_INT_DEB_EN_PORT_X(reg_idx), val_deb | mask);
	else
		clourney_write(gpio,
			GPIO_INT_DEB_EN_PORT_X(reg_idx), val_deb & ~mask);

	spin_unlock_irqrestore(&gc->bgpio_lock, flags);

	return 0;
}

static int clourney_gpio_set_config(struct gpio_chip *gc, unsigned offset,
				 unsigned long config)
{
	u32 debounce;

	if (pinconf_to_config_param(config) != PIN_CONFIG_INPUT_DEBOUNCE) {
		return -ENOTSUPP;
	}

	debounce = pinconf_to_config_argument(config);
	return clourney_gpio_set_debounce(gc, offset, debounce);
}

static irqreturn_t clourney_irq_handler_mfd(int irq, void *dev_id)
{
	u32 worked = 0;
	struct clourney_gpio_port *port = dev_id;

	worked = clourney_do_irq(port);

	return worked ? IRQ_HANDLED : IRQ_NONE;
}

static void clourney_configure_irqs(struct clourney_gpio *gpio,
				 struct clourney_gpio_port *port,
				 struct clourney_port_property *pp)
{
	struct gpio_chip *gc = &port->gc;
	struct fwnode_handle  *fwnode = pp->fwnode;
	struct irq_chip_generic	*irq_gc = NULL;
	unsigned int hwirq, ngpio = gc->ngpio;
	struct irq_chip_type *ct;
	int err, i;

	port->domain = irq_domain_create_linear(fwnode, ngpio,
						 &irq_generic_chip_ops, gpio);
	if (!port->domain)
		return;

	err = irq_alloc_domain_generic_chips(port->domain, ngpio, 2,
					     "gpio-clourney", handle_level_irq,
					     IRQ_NOREQUEST, 0,
					     IRQ_GC_INIT_NESTED_LOCK);
	if (err) {
		dev_info(gpio->dev, "irq_alloc_domain_generic_chips failed\n");
		irq_domain_remove(port->domain);
		port->domain = NULL;
		return;
	}

	irq_gc = irq_get_domain_generic_chip(port->domain, 0);
	if (!irq_gc) {
		irq_domain_remove(port->domain);
		port->domain = NULL;
		return;
	}

	irq_gc->reg_base = gpio->regs;
	irq_gc->private = port;

	for (i = 0; i < 2; i++) {
		ct = &irq_gc->chip_types[i];
		ct->chip.irq_ack = irq_gc_ack_set_bit;
		ct->chip.irq_mask = irq_gc_mask_set_bit;
		ct->chip.irq_unmask = irq_gc_mask_clr_bit;
		ct->chip.irq_set_type = clourney_irq_set_type;
		ct->chip.irq_enable = clourney_irq_enable;
		ct->chip.irq_disable = clourney_irq_disable;
		ct->chip.irq_request_resources = clourney_irq_reqres;
		ct->chip.irq_release_resources = clourney_irq_relres;
		ct->chip.flags |= IRQCHIP_SKIP_SET_WAKE;

		ct->regs.ack = GPIO_INT_EDGE_CLR_PORT_X(port->idx);
		ct->regs.mask = GPIO_INT_MASK_PORT_X(port->idx);
		ct->type = IRQ_TYPE_LEVEL_MASK;
	}

	irq_gc->chip_types[0].type = IRQ_TYPE_LEVEL_HIGH | IRQ_TYPE_LEVEL_LOW;
	irq_gc->chip_types[1].type = IRQ_TYPE_EDGE_RISING | IRQ_TYPE_EDGE_FALLING;
	irq_gc->chip_types[1].handler = handle_edge_irq;

	if (!pp->irq_shared) {
		dev_info(gpio->dev, "clourney_gpio irq Not shared...\n");
		irq_set_chained_handler_and_data(pp->irq,
			clourney_irq_handler, port);
	} else {
		/*
		 * Request a shared IRQ since where MFD would have devices
		 * using the same irq pin
		 */
		dev_info(gpio->dev, "clourney_gpio irq shared...\n");
		err = devm_request_irq(gpio->dev, pp->irq,
			clourney_irq_handler_mfd,
			IRQF_SHARED, "gpio-clourney-mfd", port);
		if (err) {
			dev_err(gpio->dev, "error requesting IRQ\n");
			irq_domain_remove(port->domain);
			port->domain = NULL;
			return;
		}
	}

	for (hwirq = 0 ; hwirq < ngpio ; hwirq++)
		irq_create_mapping(port->domain, hwirq);

	port->gc.to_irq = clourney_gpio_to_irq;

}

static void clourney_irq_teardown(struct clourney_gpio *gpio)
{
	struct clourney_gpio_port *port = &gpio->ports[0];
	struct gpio_chip *gc = &port->gc;
	unsigned int ngpio = gc->ngpio;
	irq_hw_number_t hwirq;
	int idx = 0;

	for (idx = 0; idx < gpio->nr_ports; idx++) {
		dev_info(gpio->dev, "port idx[%d] removed!\n", idx);
		port = &gpio->ports[idx];
		gc = &port->gc;
		ngpio = gc->ngpio;

		if (!port->domain)
			return;

		for (hwirq = 0 ; hwirq < ngpio ; hwirq++)
			irq_dispose_mapping(
				irq_find_mapping(port->domain, hwirq));

		irq_domain_remove(port->domain);
		port->domain = NULL;
	}
}

static int clourney_gpio_get_direction(struct gpio_chip *gc, unsigned offset)
{
	struct clourney_gpio_port *port = gpiochip_get_data(gc);
	struct clourney_gpio *gpio = port->gpio;
	unsigned long flags;
	unsigned int gpiodir;
	unsigned int reg_idx = port->idx;

	spin_lock_irqsave(&gc->bgpio_lock, flags);
	u32 dir = clourney_read(gpio, GPIO_DIR_PORT_X(reg_idx));
	spin_unlock_irqrestore(&gc->bgpio_lock, flags);
		
	dir &= BIT(offset);
	
	if(dir)
		return GPIO_DIR_OUTPUT; // dug_sys out:1
	else
		return GPIO_DIR_INPUT; // dug_sys in:0
}

static int clourney_gpio_direction_set_input(struct gpio_chip *gc, unsigned int offset)
{
	struct clourney_gpio_port *port = gpiochip_get_data(gc);
	struct clourney_gpio *gpio = port->gpio;
	unsigned long flags;
	unsigned int gpiodir;
	unsigned int reg_idx = port->idx;
	
	spin_lock_irqsave(&gc->bgpio_lock, flags);
	u32 val = clourney_read(gpio, GPIO_DIR_PORT_X(reg_idx));
	spin_unlock_irqrestore(&gc->bgpio_lock, flags);
	
	val &= ~BIT(offset);
	clourney_write(gpio, GPIO_DIR_PORT_X(reg_idx), val);

	return 0;
}

static int clourney_gpio_direction_set_output(struct gpio_chip *gc, unsigned int offset,
		int value)
{
	struct clourney_gpio_port *port = gpiochip_get_data(gc);
	struct clourney_gpio *gpio = port->gpio;
	unsigned long flags;
	unsigned int gpiodir;
	unsigned int reg_idx = port->idx;

	spin_lock_irqsave(&gc->bgpio_lock, flags);
	u32 val = clourney_read(gpio, GPIO_DIR_PORT_X(reg_idx));
	spin_unlock_irqrestore(&gc->bgpio_lock, flags);

	val |= BIT(offset);
	clourney_write(gpio, GPIO_DIR_PORT_X(reg_idx), val);

	return 0;
}

static int clourney_gpio_get_value(struct gpio_chip *gc, unsigned int offset)
{	
	struct clourney_gpio_port *port = gpiochip_get_data(gc);
	struct clourney_gpio *gpio = port->gpio;
	unsigned long flags;
	unsigned int gpiodir;
	unsigned int reg_idx = port->idx;

	clourney_write(gpio, GPIO_IN_DATA_PORT_X(reg_idx), 0xffffffff);
	
	spin_lock_irqsave(&gc->bgpio_lock, flags);
	u32 val = clourney_read(gpio, GPIO_IN_DATA_PORT_X(reg_idx));
	spin_unlock_irqrestore(&gc->bgpio_lock, flags);

	val &= BIT(offset);
	if(val)
		return 0x1;
	else
		return 0;
}

static void clourney_gpio_set_value(struct gpio_chip *gc, unsigned int offset, int value)
{
	struct clourney_gpio_port *port = gpiochip_get_data(gc);
	struct clourney_gpio *gpio = port->gpio;
	unsigned long flags;
	unsigned int gpiodir;
	unsigned int reg_idx = port->idx;

	clourney_write(gpio, GPIO_DATA_SOURCE_PORT_X(reg_idx), 0xffffffff);
	
	spin_lock_irqsave(&gc->bgpio_lock, flags);
	u32 val = clourney_read(gpio, GPIO_OUT_DATA_PORT_X(reg_idx));
	spin_unlock_irqrestore(&gc->bgpio_lock, flags);
	if(value) {
		val |= BIT(offset);
		clourney_write(gpio, GPIO_OUT_DATA_PORT_X(reg_idx), val);
	}
	else {
		val &= ~BIT(offset);
		clourney_write(gpio, GPIO_OUT_DATA_PORT_X(reg_idx), val);
	}
}

static int clourney_gpio_add_port(struct clourney_gpio *gpio,
			       struct clourney_port_property *pp,
			       unsigned int offs)
{
	struct clourney_gpio_port *port;
	void __iomem *dat, *set, *dirout;
	int err;

	port = &gpio->ports[offs];
	port->gpio = gpio;
	port->idx = pp->idx;

#ifdef CONFIG_PM_SLEEP
	port->ctx = devm_kzalloc(gpio->dev, sizeof(*port->ctx), GFP_KERNEL);
	if (!port->ctx)
		return -ENOMEM;
#endif

#ifdef CONFIG_OF_GPIO
	port->gc.of_node = to_of_node(pp->fwnode);
#endif
	port->gc.ngpio = pp->ngpio;
	port->gc.base = pp->gpio_base;
	port->gc.get_direction = clourney_gpio_get_direction;
	port->gc.direction_input = clourney_gpio_direction_set_input;
	port->gc.direction_output = clourney_gpio_direction_set_output;
	port->gc.get = clourney_gpio_get_value;
	port->gc.set = clourney_gpio_set_value;

	port->gc.set_config = clourney_gpio_set_config;

	if (pp->irq)
		clourney_configure_irqs(gpio, port, pp);

	err = gpiochip_add_data(&port->gc, port);
	if (err)
		dev_err(gpio->dev, "failed to register gpiochip for port%d\n",
			port->idx);
	else
		port->is_registered = true;

	return err;
}

static void clourney_gpio_unregister(struct clourney_gpio *gpio)
{
	unsigned int m;

	for (m = 0; m < gpio->nr_ports; ++m)
		if (gpio->ports[m].is_registered)
			gpiochip_remove(&gpio->ports[m].gc);
}

static struct clourney_port_property *
clourney_gpio_get_pdata(struct device *dev, struct clourney_gpio *gpio)
{
	struct fwnode_handle *fwnode;
	struct clourney_port_property *properties;
	struct clourney_port_property *pp;
	int nports = 1;
	int i;
	int gpio_count = 0;
	static int pre_gpio_count = 0;

	struct device_node *np = dev->of_node;

	properties = devm_kcalloc(dev, nports, sizeof(*pp), GFP_KERNEL);
	if (!properties)
		return ERR_PTR(-ENOMEM);

	gpio->nr_ports = nports;

	fwnode = &dev->of_node->fwnode;
	pp = properties;
	pp->fwnode = fwnode;

	if (fwnode_property_read_u32(fwnode, "reg", &pp->idx) ||
	    pp->idx >= CLOURNERY_MAX_PORTS) {
		dev_err(dev,
			"missing/invalid port index for port%d\n", i);
		fwnode_handle_put(fwnode);
		return ERR_PTR(-EINVAL);
	}

	if (fwnode_property_read_u32_array(fwnode, "gpio-ranges",
		pp->gpio_ranges, ARRAY_SIZE(pp->gpio_ranges))) {
		dev_err(dev,
			 "failed to get gpio-ranges for port%d\n", i);
		fwnode_handle_put(fwnode);
		return ERR_PTR(-EINVAL);
	} else {
		dev_info(dev,
			 "Got gpio-ranges[%d, %d, %d, %d] for port%d\n",
			pp->gpio_ranges[0], pp->gpio_ranges[1],
			pp->gpio_ranges[2], pp->gpio_ranges[3], i);
	}

	if (fwnode_property_read_bool(fwnode, "interrupt-controller")) {
		pp->irq = irq_of_parse_and_map(to_of_node(fwnode), 0);
		if (!pp->irq)
				dev_warn(dev, "no irq for port%d\n", pp->idx);
		dev_err(dev, "clourney_gpio: irq [%d]\n", pp->irq);
	}
	
	pp->ngpio = pp->gpio_ranges[3];
	gpio_count += pp->ngpio;

	pp->irq_shared	= true;
	pp->gpio_base	= pre_gpio_count;

	pre_gpio_count += gpio_count;
	
	return properties;
}

static const struct of_device_id clourney_of_match[] = {
	{ .compatible = "clourney,clourney-dubhe-gpio",},
	{ /* Sentinel */ }
};
MODULE_DEVICE_TABLE(of, clourney_of_match);

static int clourney_gpio_probe(struct platform_device *pdev)
{
	unsigned int i;
	struct resource *res;
	struct clourney_gpio *gpio;
	int err;
	struct device *dev = &pdev->dev;
	struct clourney_port_property *properties;

	gpio = devm_kzalloc(&pdev->dev, sizeof(*gpio), GFP_KERNEL);
	if (!gpio)
		return -ENOMEM;

	properties = clourney_gpio_get_pdata(dev, gpio);
	if (!properties) {
		dev_err(dev, "clourney_gpio clourney_gpio_get_pdata error!\n");
		return PTR_ERR(properties);
	}
	
	if (!gpio->nr_ports) {
		dev_err(dev, "clourney_gpio No ports found!\n");
		return -ENODEV;
	}

	gpio->dev = &pdev->dev;

	gpio->ports = devm_kcalloc(&pdev->dev, gpio->nr_ports,
				   sizeof(*gpio->ports), GFP_KERNEL);
	if (!gpio->ports)
		return -ENOMEM;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	gpio->regs = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(gpio->regs)) {
		dev_err(dev, "clourney_gpio regs devm_ioremap_resource failed !\n");
		return PTR_ERR(gpio->regs);
	}
	
	for (i = 0; i < gpio->nr_ports; i++) {
		err = clourney_gpio_add_port(gpio, &properties[i], i);
		if (err) {
			dev_err(dev, "clourney_gpio clourney_gpio_add_port failed !\n");
			goto out_unregister;
		}		
	}
	platform_set_drvdata(pdev, gpio);

	return 0;

out_unregister:
	clourney_gpio_unregister(gpio);
	clourney_irq_teardown(gpio);

	return err;
}

static int clourney_gpio_remove(struct platform_device *pdev)
{
	struct clourney_gpio *gpio = platform_get_drvdata(pdev);

	clourney_gpio_unregister(gpio);
	clourney_irq_teardown(gpio);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int clourney_gpio_suspend(struct device *dev)
{
	return 0;
}

static int clourney_gpio_resume(struct device *dev)
{
	return 0;
}
#endif

static SIMPLE_DEV_PM_OPS(clourney_gpio_pm_ops, clourney_gpio_suspend,
			 clourney_gpio_resume);

static struct platform_driver clourney_gpio_driver = {
	.driver		= {
		.name	= "clourney,clourney-dubhe-gpio",
		.pm	= &clourney_gpio_pm_ops,
		.of_match_table = of_match_ptr(clourney_of_match),
	},
	.probe		= clourney_gpio_probe,
	.remove		= clourney_gpio_remove,
};

module_platform_driver(clourney_gpio_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("clourney");
MODULE_DESCRIPTION("clourney GPIO driver");
