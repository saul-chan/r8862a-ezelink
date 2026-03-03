#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/gpio_keys.h>
#include <linux/ioport.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>

int irq_num = 0;
int key_gpio = 0;

static irqreturn_t key_interrupt(int irq, void *dev_id) {
    return IRQ_HANDLED;
}

static int drProbe(struct platform_device *pdev) {
	struct device *dev = &pdev->dev;	
	struct fwnode_handle *fwnode = &dev->of_node->fwnode;

	key_gpio = of_get_named_gpio(to_of_node(fwnode), "key-gpio", 0);
	pr_info("1_key_gpio_num = %d\n", key_gpio);

	key_gpio = of_get_named_gpio(to_of_node(fwnode), "key-gpio", 1);
	pr_info("2_key_gpio_num = %d\n", key_gpio);

	key_gpio = of_get_named_gpio(to_of_node(fwnode), "key-gpio", 2);
	pr_info("3_key_gpio_num = %d\n", key_gpio);
	
	irq_num = irq_of_parse_and_map(to_of_node(fwnode), 0);
	pr_info("gpio_key_irq_num = %d\n", irq_num);

	int ret = devm_request_irq(dev, irq_num, key_interrupt, IRQ_TYPE_LEVEL_HIGH, "gpio_key", pdev);

	if (ret < 0) {
        pr_info("request gpio_key_interrupt failed\n");
        return ret;
	}

    return 0;
}

static int drRemove(struct platform_device *pdev){
    pr_info("clourney-dubhe-gpio-key-test driver remove\n");
    return 0;
}

static const struct of_device_id of_interr_match[] = {
    {.compatible = "clourney,clourney-dubhe-gpio-key-test"},
    {},
};

static struct platform_driver pdrv = {
    .probe = drProbe,
    .remove = drRemove,
    .driver = {
        .name = "clourney,clourney-dubhe-gpio-key-test",
        .owner = THIS_MODULE,
		.of_match_table = of_match_ptr(of_interr_match),
    }
	
	//.id_table = of_interr_match,
};

static int driver_init_interr(void){
    int ret=0;
    ret = platform_driver_register(&pdrv);
    if(ret < 0){
        pr_info("platform driver regist failed\n");
        return -1;
    }
    return 0;
}

static void driver_exit_interr(void){
   platform_driver_unregister(&pdrv);
   pr_info("platform driver exit!\n");
}

module_init(driver_init_interr);
module_exit(driver_exit_interr);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("clourney");
MODULE_DESCRIPTION("clourney GPIO test driver");
