#include <linux/string.h>
#include <linux/bitops.h>
#include <linux/phy.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/compat.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/mutex.h>

#include <asm/io.h>
#include <linux/io.h>

#include <linux/in.h>
#include <linux/debugfs.h>
#include <net/sock.h>
#include <linux/socket.h>
#include <linux/net.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#ifdef CONFIG_SWCONFIG
#include <linux/switch.h>
#include "swconfig_leds.c"
#endif

#include <linux/of_mdio.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>

#include "cal_bprofile.h"
#include "yt_error.h"
#include "yt_init.h"
#include "yt_vlan.h"
#include "yt_port.h"
#include "yt_nic.h"
#include "yt_port_isolation.h"
#include "yt_stat.h"

#define NETLINK_USR_ESW  30
#define MAX_PAYLOAD (2048)
#define PORT_MAX_GROUP 7
#define UNIT 		0
#define PORT0 		0
#define PORT1 		1
#define PORT2 		2
#define PORT3 		3
#define PORT4 		4
#define PORT5 		5
#define PORT6 		6
#define ENABLE 		1
#define DISABLE 	0
#define DEBUG 		0
#define MAX_NPRINTK_LEN	4096
MODULE_LICENSE("GPL");

struct yt_esw {
	struct device           *dev;
	struct mii_bus          *bus;
	int gpio_reset;
	enum of_gpio_flags      yt_reset_flags;
#ifdef CONFIG_SWCONFIG
	struct switch_dev sw_dev;
#endif
};

extern int yt_cmd_main(int argc,  char *argv[]);
extern int g_debug_switch;

static struct yt_esw *_esw;
static struct sock *nlsk;
static int ssv_usr_pid = 0;
static DEFINE_MUTEX(ssv_pid_mutex);

#if DEBUG
static void smi_mib_proc_test(void);
#endif

void nprintf(const char *fmt, ...)
{
	va_list args;
	char *buf;
	struct sk_buff *skb_out;
	struct nlmsghdr *nlh;
	int res, len;

	if (!nlsk || ssv_usr_pid == 0) {
		printk_ratelimited(KERN_WARNING "%s: Netlink not ready\n", __func__);
		return;
	}

	buf = (char *)vmalloc(MAX_NPRINTK_LEN);
	if (!buf) {
		printk_ratelimited(KERN_ERR "%s: vmalloc failed\n", __func__);
		return;
	}

	va_start(args, fmt);
	len = vsnprintf(buf, MAX_NPRINTK_LEN, fmt, args);
	va_end(args);

	if (len >= MAX_NPRINTK_LEN) {
		len = MAX_NPRINTK_LEN - 1;
		buf[len] = '\0';
	}

	skb_out = nlmsg_new(len + 1, GFP_KERNEL);
	if (!skb_out) {
		vfree(buf);
		printk_ratelimited(KERN_ERR "%s: nlmsg_new failed\n", __func__);
		return;
	}

	nlh = nlmsg_put(skb_out, 0, 0, NLMSG_NOOP, len + 1, 0);
	if (!nlh) {
		nlmsg_free(skb_out);
		vfree(buf);
		return;
	}

	strncpy(nlmsg_data(nlh), buf, len + 1);
	vfree(buf);

	NETLINK_CB(skb_out).dst_group = 0;
	res = nlmsg_unicast(nlsk, skb_out, ssv_usr_pid);

	if (res < 0)
		printk_ratelimited(KERN_ERR "%s: send failed (%d)\n", __func__, res);
}

void esw_nl_snd_done(void)
{
	struct sk_buff *skb_out;
	struct nlmsghdr *nlh;
	int res;

	if (!nlsk || ssv_usr_pid == 0) {
		printk_ratelimited(KERN_WARNING "%s: Netlink not ready\n", __func__);
		return;
	}

	skb_out = nlmsg_new(1, GFP_KERNEL);
	if (!skb_out) {
		printk_ratelimited(KERN_ERR "%s: nlmsg_new failed\n", __func__);
		return;
	}

	nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, 1, 0);
	if (!nlh) {
		nlmsg_free(skb_out);
		return;
	}

	NETLINK_CB(skb_out).dst_group = 0;
	res = nlmsg_unicast(nlsk, skb_out, ssv_usr_pid);

	if (res < 0)
		printk_ratelimited(KERN_ERR "%s: send failed (%d)\n", __func__, res);
}

void esw_nl_recv_msg(struct sk_buff *skb)
{
#define PARA_MAX_NUM                30
	struct nlmsghdr *nlh;

	int ret=0;
	u8 *pInBuf = NULL;
	u32 inBufLen=0;
	int argc = 0;
	bool is_para_left = true;
	char *argv[PARA_MAX_NUM] = {0};
	u8 *str_ptr;

	nlh = (struct nlmsghdr *)skb->data;
	pInBuf = (u8 *)nlmsg_data(nlh);
	inBufLen = nlmsg_len(nlh);
	str_ptr = pInBuf;

	while ( '\0' != *str_ptr && 
			argc < PARA_MAX_NUM &&
			str_ptr - pInBuf < inBufLen) {
		if ('\t' == *str_ptr ||
				' ' == *str_ptr ||
				'\n' == *str_ptr) {
			*str_ptr = '\0';
			is_para_left = true;
		} else if (is_para_left) {
			is_para_left = false;
			argv[argc++] = str_ptr;
		}
		str_ptr++;
	}

	/* IMPROVE ME: Please don't define the ssv_usr_pid as global var and pass its value
	 * to nprintf() and esw_nl_snd_done() to avoid lock.
	 */
	mutex_lock(&ssv_pid_mutex);
	ssv_usr_pid = nlh->nlmsg_pid;
	ret = yt_cmd_main(argc, argv);
	esw_nl_snd_done();
	ssv_usr_pid = 0;
	mutex_unlock(&ssv_pid_mutex);

	if (g_debug_switch & 0x10)
		printk(KERN_ERR"argc:%d", argc);	

	return;
}

static int _esw_netlink_init(void)
{
	struct netlink_kernel_cfg cfg = {
		.groups	= 0,
		.input	= esw_nl_recv_msg,
	};

	nlsk = netlink_kernel_create(&init_net, NETLINK_USR_ESW, &cfg);

	if (nlsk == NULL) {
		printk(KERN_ERR"Can't create netlink esw\n");
		return -ENOMEM;
	}

	return 0;
}

static int _esw_netlink_exit(void)
{
	netlink_kernel_release(nlsk);
	nlsk = NULL;
	ssv_usr_pid = 0;

	return 0;
}

uint32_t yt_smi_cl22_read(uint8_t phy_addr, uint8_t phy_register, uint16_t *read_data)
{
	struct mii_bus *bus = _esw->bus;

	if (unlikely(!bus))
		return 0;

	mutex_lock_nested(&bus->mdio_lock, MDIO_MUTEX_NESTED);

	*read_data = bus->read(bus, phy_addr, phy_register);

	mutex_unlock(&bus->mdio_lock);

	if (g_debug_switch & 2)
		printk(KERN_ERR "%s phy_addr%#x Read reg:%#x V:%#x",__FUNCTION__, phy_addr, phy_register, *read_data);

	return 0;
}

uint32_t yt_smi_cl22_write(uint8_t phy_addr, uint8_t phy_register, uint16_t write_data)
{
	struct mii_bus *bus = _esw->bus;

	if (unlikely(!bus)) 
		return 0;

	if (g_debug_switch & 4)
		printk(KERN_ERR "%s phy_addr%#x Write Reg:%#x V:%#x", __FUNCTION__, phy_addr, phy_register, write_data);

	mutex_lock_nested(&bus->mdio_lock, MDIO_MUTEX_NESTED);

	bus->write(bus, phy_addr, phy_register, write_data);

	mutex_unlock(&bus->mdio_lock);

	return 0;  
}

static int esw_hw_reset(void)
{
	struct yt_esw *esw = _esw;

	if (esw->gpio_reset < 0) {
		return 0;
	}

	gpio_direction_output(esw->gpio_reset, 0);
	gpio_set_value(esw->gpio_reset, 0);
	mdelay(500);
	gpio_set_value(esw->gpio_reset, 1);

	return 0;
}

static int esw_hw_init(void)
{
	yt_ret_t ret = 0;

	esw_hw_reset();

	ret = yt_init();
	if (ret)
		goto END;	

	ret = yt_modules_init();
	if(ret)
		goto END;

	mdelay(500);

END:
	printk("YT switch init completed (%d)\n", ret);

	return ret;
}

static void esw_set_hsgmii(void)
{
	yt_port_force_ctrl_t port_ctrl = {0};

	/*set Port5 HSGMII Mode，Force LinkRate 2.5G Full */
	/*use logic Port5 as physical GMac8, use logic Port6 as physical GMac9,*/
	yt_port_extif_mode_set(UNIT, PORT5, YT_EXTIF_MODE_BX2500);
	port_ctrl.speed_dup = PORT_SPEED_DUP_2500FULL;
	port_ctrl.rx_fc_en = ENABLE;
	port_ctrl.tx_fc_en = ENABLE;

	yt_port_mac_force_set(UNIT, PORT5, port_ctrl);
	yt_port_enable_set(UNIT, PORT5, ENABLE);
}

static void esw_set_rgmii(uint32_t rxc_delay, uint32_t txc_delay, uint32_t txc_2ns_en)
{
	yt_port_force_ctrl_t port_ctrl = {0};

	yt_port_extif_mode_set(UNIT, PORT6, YT_EXTIF_MODE_RGMII);
	port_ctrl.speed_dup = PORT_SPEED_DUP_1000FULL;
	port_ctrl.rx_fc_en = ENABLE;
	port_ctrl.tx_fc_en = ENABLE;

	yt_port_mac_force_set(UNIT, PORT6, port_ctrl);

	yt_port_extif_rgmii_delay_set(UNIT, PORT6, rxc_delay, txc_delay, txc_2ns_en);

	yt_port_enable_set(UNIT, PORT6, ENABLE);
}

int gen_port_map(const char *str, int len, uint8_t * port, uint8_t * group)
{
	int i;
	uint8_t group_len = 0;

	for (i = 0; i < len; i++) {
		int key_i, key_v;

		key_i = strchr(str, str[i]) - str;

		if (key_i == i) {
			group_len++;
			key_v = group_len;
		} else
			key_v = port[key_i];

		group[key_v] |= BIT(i);

		port[i] = key_v;
	}

	return group_len;
}

#ifdef CONFIG_SWCONFIG
static int yt_sw_get_port_link(struct switch_dev *dev, int port,
			     struct switch_port_link *link)
{
	yt_port_link_status_t pLinkStatus;
	yt_ret_t ret;

	ret = yt_port_link_status_get(UNIT, port, &pLinkStatus);
	if (ret == CMM_ERR_FAIL)
		return -EOPNOTSUPP;
	if (pLinkStatus == PORT_LINK_DOWN)
		link->link = false;
	else
		link->link = true;

	return 0;
}

static int yt_sw_get_port_stats(struct switch_dev *dev, int port,
			struct switch_port_stats *stats)
{
	yt_stat_mib_port_cnt_t pCnt;
	yt_ret_t ret;

	ret = yt_stat_mib_port_get(UNIT, port, &pCnt);
	if (ret == CMM_ERR_FAIL)
		return -EOPNOTSUPP;

	stats->tx_bytes = pCnt.TX_OKBYTE;
	stats->rx_bytes = pCnt.RX_OKBYTE;

	return 0;
}

static const struct switch_dev_ops yt_sw_ops = {
	.get_port_link = yt_sw_get_port_link,
	.get_port_stats = yt_sw_get_port_stats,
};

void init_sw_led(void)
{
	struct switch_dev *dev;
	yt_enable_t pEnable;
	yt_ret_t ret;

	dev = &_esw->sw_dev;
	dev->ops = &yt_sw_ops;
	/* We don't support two external switches in one board.So
	 * just hard code its name as "switch0".
	 */
	strcpy(dev->devname, "switch0");

	/* Enable mib for switch led trigger to use */
	ret = yt_stat_mib_enable_get(UNIT, &pEnable);
	if (ret == CMM_ERR_FAIL)
		return;
	if (pEnable == YT_DISABLE)
		ret = yt_stat_mib_enable_set(UNIT, YT_ENABLE);
	if (ret == CMM_ERR_FAIL)
		return;

	if (swconfig_create_led_trigger(dev))
		pr_err("Failed to create swconfig led trigger\n");
}
#endif

static const struct of_device_id cls_esw_match[] = {
	{ .compatible = "clourney,yt_switch" },
	{},
};

MODULE_DEVICE_TABLE(of, cls_esw_match);

static int cls_esw_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct device_node *mdio;
	struct mii_bus *mdio_bus;
	struct yt_esw *esw;
	const char *pm;
	uint8_t group[PORT_MAX_GROUP] = {0};
	uint8_t port[PORT_MAX_GROUP] = {0};
	uint8_t group_len = 0;
	int port_num;
	int ret, i, j;
	uint32_t rxc_delay = 1;
	uint32_t txc_delay = 2;
	uint32_t txc_2ns_en = 1;
	unsigned long gpio_flags;

	mdio = of_parse_phandle(np, "mdio-bus", 0);

	if (!mdio)
		return -EINVAL;

	mdio_bus = of_mdio_find_bus(mdio);

	if (!mdio_bus)
		return -EPROBE_DEFER;

	esw = devm_kzalloc(&pdev->dev, sizeof(struct yt_esw), GFP_KERNEL);

	if (!esw)
		return -ENOMEM;

	esw->dev = &pdev->dev;

	esw->bus = mdio_bus;

	esw->gpio_reset = of_get_named_gpio_flags(np, "reset-gpio", 0, &esw->yt_reset_flags);
	if (esw->gpio_reset >= 0) {
		if (esw->yt_reset_flags & OF_GPIO_ACTIVE_LOW)
			gpio_flags = GPIOF_OUT_INIT_LOW;
		else
			gpio_flags = GPIOF_OUT_INIT_HIGH;

		ret = devm_gpio_request_one(esw->dev, esw->gpio_reset, gpio_flags, "reset-gpio");
		if (ret)
			pr_err("fail to devm_gpio_request for reset-gpio\n");
	}
	/*GPIO_ACTIVE_HIGH -> 0, GPIO_ACTIVE_LOW -> 1*/
	pr_info("yt gpio_reset=%d, flags=%d\n", esw->gpio_reset, esw->yt_reset_flags);

	_esw = esw;

	esw_hw_init();

	if(of_property_read_string(pdev->dev.of_node,
				"port-isolation", &pm)) {
		printk(KERN_ERR"!!!!!Can't find port-isolation DTS\n");
		pm = "yyyyyyx";
	}

	port_num = strlen(pm);
	group_len = gen_port_map(pm, port_num, port, group);
	for (i = 0; i < port_num; i++) {

		int offset = 0;
		char str_buf[PORT_MAX_GROUP * 2 + 1] = {0};

		yt_port_mask_t iso_portmask;
		iso_portmask.portbits[0] = (~group[port[i]]) & GENMASK(port_num - 1, 0);

		yt_port_isolation_set(UNIT, i, iso_portmask);

		for (j = 0; j < port_num; j++)
			offset += sprintf(str_buf + offset, "%d ", !!(group[port[i]] & BIT(j)));

		printk(KERN_ERR"Set YT_SWITCH isolation\n\tport%d mask[%#x] map %s\n",i, iso_portmask.portbits[0], str_buf);
	}

	ret = of_property_read_u32(np, "rgmii-rxc_delay", &rxc_delay);
	if(ret)
		rxc_delay = 1;

	ret = of_property_read_u32(np, "rgmii-txc_delay", &txc_delay);
	if(ret)
		txc_delay = 2;

	ret = of_property_read_u32(np, "rgmii-txc_2ns_en", &txc_2ns_en);
	if(ret)
		txc_2ns_en = 1;

	pr_err("YT_SW:set rgmii rxc_delay [%d] txc_delay [%d] txc_2ns_en [%d]\n",
			rxc_delay, txc_delay, txc_2ns_en);

	esw_set_hsgmii();

	esw_set_rgmii(rxc_delay, txc_delay, txc_2ns_en);

	platform_set_drvdata(pdev, esw);

	_esw_netlink_init();

#ifdef CONFIG_SWCONFIG
	init_sw_led();
#endif

	return 0;
}

static int cls_esw_remove(struct platform_device *pdev)
{
#ifdef CONFIG_SWCONFIG
	swconfig_destroy_led_trigger(&_esw->sw_dev);
#endif
	_esw_netlink_exit();
	platform_set_drvdata(pdev, NULL);

	return 0;
}

static struct platform_driver cls_esw_driver = {
	.probe = cls_esw_probe,
	.remove = cls_esw_remove,
	.driver = {
		.name = "esw_yc",
		.owner = THIS_MODULE,
		.of_match_table = cls_esw_match,
	},
};

module_platform_driver(cls_esw_driver);

#if DEBUG
#include <linux/proc_fs.h>
#include <linux/fs.h>

/* SMI format */
#define REG_ADDR_BIT1_ADDR 0
#define REG_ADDR_BIT1_DATA 1
#define REG_ADDR_BIT0_WRITE 0
#define REG_ADDR_BIT0_READ 1
#define PHYADDR 0x1D /*base on Hardware Switch Phyaddr*/
#define SWITCHID 0x0 /*base on Hardware Switch SwitchID*/
#define YT9215_PORT_MIB_BASE(n) (0xc0100 + (n) * 0x100)
static struct mutex smi_reg_mutex;

static u32 yt_smi_switch_write(u32 reg_addr, u32 reg_value)
{
	u8 phyAddr;
	u8 switchId;
	u8 regAddr;
	u16 regVal;

	mutex_lock(&smi_reg_mutex);
	phyAddr = PHYADDR;
	switchId = SWITCHID;
	regAddr = (switchId<<2)|(REG_ADDR_BIT1_ADDR<<1)|(REG_ADDR_BIT0_WRITE);

	/* Set reg_addr[31:16] */
	regVal = (reg_addr >> 16)&0xffff;
	yt_smi_cl22_write(phyAddr, regAddr, regVal);

	/* Set reg_addr[15:0] */ 
	regVal = reg_addr&0xffff;
	yt_smi_cl22_write(phyAddr, regAddr, regVal);

	/* Write Data [31:16] out */
	regAddr = (switchId<<2)|(REG_ADDR_BIT1_DATA<<1)|(REG_ADDR_BIT0_WRITE);
	regVal = (reg_value >> 16)&0xffff;
	yt_smi_cl22_write(phyAddr, regAddr, regVal);

	/* Write Data [15:0] out */
	regVal = reg_value&0xffff;
	yt_smi_cl22_write(phyAddr, regAddr, regVal);
	mutex_unlock(&smi_reg_mutex);
	return 0;
}

static u32 yt_smi_switch_read(u32 reg_addr, u32 *reg_value)
{ 
	u32 rData;
	u8 phyAddr;
	u8 switchId;
	u8 regAddr;
	u16 regVal;
	mutex_lock(&smi_reg_mutex);
	phyAddr = PHYADDR;
	switchId = SWITCHID;
	regAddr = (switchId<<2)|(REG_ADDR_BIT1_ADDR<<1)|(REG_ADDR_BIT0_READ);

	/* Set reg_addr[31:16] */
	regVal = (reg_addr >> 16)&0xffff;
	yt_smi_cl22_write(phyAddr, regAddr, regVal);

	/*change to platform smi write*/
	/* Set reg_addr[15:0] */
	regVal = reg_addr&0xffff;
	yt_smi_cl22_write(phyAddr, regAddr, regVal);
	regAddr = (switchId<<2)|(REG_ADDR_BIT1_DATA<<1)|(REG_ADDR_BIT0_READ);

	/* Read Data [31:16] */
	regVal = 0x0;
	yt_smi_cl22_read(phyAddr, regAddr, &regVal);
	/*change to platform smi read*/ rData = (uint32_t)(regVal<<16);

	/* Read Data [15:0] */
	regVal = 0x0;
	yt_smi_cl22_read(phyAddr, regAddr, &regVal);

	rData |= regVal;
	*reg_value = rData;
	mutex_unlock(&smi_reg_mutex);
	return 0;
}

static void yt_smi_switch_rmw(u32 reg, u32 mask, u32 set)
{ 
	u32 val = 0;
	yt_smi_switch_read(reg, &val);
	val &= ~mask;
	val |= set;
	yt_smi_switch_write(reg, val);
}

static ssize_t smi_write_proc(struct file *filp, const char *buffer, size_t count, loff_t *offp)
{
	char *str, *cmd, *value;
	char tmpbuf[128] = {0};
	uint32_t regAddr = 0;
	uint32_t regData = 0;
	uint32_t rData = 0;

	if(count >= sizeof(tmpbuf)) goto error;

	if(!buffer || copy_from_user(tmpbuf, buffer, count) != 0)
		return 0;

	if (count > 0)
	{
		str = tmpbuf;
		cmd = strsep(&str, "\t \n");

		if (!cmd)
		{
			goto error;
		}

		if (strcmp(cmd, "write") == 0)
		{ 
			value = strsep(&str, "\t \n");

			if (!value)
			{
				goto error;
			}

			regAddr = simple_strtoul(value, &value, 16);
			value = strsep(&str, "\t \n");

			if (!value)
			{ 
				goto error;
			}

			regData = simple_strtoul(value, &value, 16);
			printk("write regAddr = 0x%x regData = 0x%x\n", regAddr, regData);
			yt_smi_switch_write(regAddr, regData);
		}
		else if (strcmp(cmd, "read") == 0)
		{

			value = strsep(&str, "\t \n");

			if (!value)
			{ 
				goto error;
			}
			regAddr = simple_strtoul(value, &value, 16);
			printk("read regAddr = 0x%x ", regAddr);
			yt_smi_switch_read(regAddr, &rData);
			printk("regData = 0x%x\n", rData);
		}
		else {
			goto error;
		}
	}
	return count;

error:
	printk("usage: \n");
	printk(" read regaddr: for example, echo read 0xd0004 > /proc/smi\n");
	printk(" write regaddr regdata: for example; \
			echo write 0xd0004 0x680 > /proc/smi\n");
	return -EFAULT;
}
static struct proc_dir_entry *smi_proc;
static const struct proc_ops smi_proc_fops = {
	.proc_read = NULL,
	.proc_write = smi_write_proc,
};
struct stat_mib_counter { 
	unsigned int size;
	unsigned int offset;
	const char *name;
};

static const struct stat_mib_counter stat_mib[] = {
	{ 1, 0x00, "RxBcast"},
	{ 1, 0x04, "RxPause"},
	{ 1, 0x08, "RxMcast"},
	{ 1, 0x0C, "RxCrcErr"},
	{ 1, 0x10, "RxAlignErr"},
	{ 1, 0x14, "RxRunt"},
	{ 1, 0x18, "RxFragment"},
	{ 1, 0x1C, "RxSz64"},
	{ 1, 0x20, "RxSz65To127"},
	{ 1, 0x24, "RxSz128To255"},
	{ 1, 0x28, "RxSz256To511"},
	{ 1, 0x2C, "RxSz512To1023"},
	{ 1, 0x30, "RxSz1024To1518"},
	{ 1, 0x34, "RxJumbo"},
	/*{ 1, 0x38, "RxMaxByte"},*/
	{ 2, 0x3C, "RxOkByte"},
	{ 2, 0x44, "RxNoOkByte"},
	{ 1, 0x4C, "RxOverFlow"},
	/*{ 1, 0x50, "QMFilter"},*/
	{ 1, 0x54, "TxBcast"},
	{ 1, 0x58, "TxPause"},
	{ 1, 0x5C, "TxMcast"}, 
	/*{ 1, 0x60, "TxUnderRun"},*/
	{ 1, 0x64, "TxSz64"},
	{ 1, 0x68, "TxSz65To127"},
	{ 1, 0x6C, "TxSz128To255"}, 
	{ 1, 0x70, "TxSz256To511"},
	{ 1, 0x74, "TxSz512To1023"}, 
	{ 1, 0x78, "TxSz1024To1518"}, 
	{ 1, 0x7C, "TxJumbo"},
	{ 1, 0x80, "TxOverSize"},
	{ 2, 0x84, "TxOkByte"},
	{ 1, 0x8C, "TxCollision"},
	/*{ 1, 0x90, "TxAbortCollision"},*/
	/*{ 1, 0x94, "TxMultiCollision"},*/
	/*{ 1, 0x98, "TxSingleCollision"},*/
	/*{ 1, 0x9C, "TxExcDefer"},*/
	/*{ 1, 0xA0, "TxDefer"},*/
	{ 1, 0xA4, "TxLateCollision"},
	/*{ 1, 0xA8, "RxOamCounter"},*/
	/*{ 1, 0xAC, "TxOamCounter"},*/
};

static u32 stat_mib_port_get(u8 unit, u32 port)
{
	int i = 0;
	u32 lowData = 0;
	u32 highData = 0;
	u64 resultData = 0;

	int mibCount;
	u64 count = 0;
	mibCount = ARRAY_SIZE(stat_mib);
	printk("%-20s %20d\n", "port", port);
	for (i = 0;
			i < mibCount;
			i++)
	{
		count = 0;
		yt_smi_switch_read(YT9215_PORT_MIB_BASE(port) + stat_mib[i].offset, &lowData);
		count = lowData;

		if (stat_mib[i].size == 2)
		{ yt_smi_switch_read(YT9215_PORT_MIB_BASE(port) + stat_mib[i].offset + 4, &highData);
			resultData = highData;
			count |= resultData << 32;
		}
		if (stat_mib[i].size == 1) printk("%-20s %20u\n", stat_mib[i].name, (u32)count);
		else printk("%-20s %20llu\n", stat_mib[i].name, count);
	}
	return 0;
}

static ssize_t mib_write_proc(struct file *filp, const char *buffer, size_t count, loff_t *offp)
{
	char *str, *cmd, *value;
	char tmpbuf[128] = {0};
	uint32_t port = 0;
	uint32_t ret = 0;

	uint8_t unit = 0;

	if(count >= sizeof(tmpbuf)) goto error;

	if(!buffer || copy_from_user(tmpbuf, buffer, count) != 0)
		return 0;

	if (count > 0)
	{
		str = tmpbuf;
		cmd = strsep(&str, "\t \n");

		if (!cmd)
		{ goto error;
		}
		if (strcmp(cmd, "mib") == 0)
		{
			cmd = strsep(&str, "\t \n");

			if (!cmd)
			{ goto error;
			}
			if (strcmp(cmd, "enable") == 0)
			{ 
				yt_smi_switch_rmw(0x80004, 1<<1, 1<<1);
			}
			else if (strcmp(cmd, "disable") == 0)
			{ 
				yt_smi_switch_rmw(0x80004, 1<<1, 0<<1);
			}
			else if (strcmp(cmd, "clear") == 0)
			{ 
				u32 ctrl_data = 0;
				yt_smi_switch_read(0xc0004, &ctrl_data);
				yt_smi_switch_write(0xc0004, 0<<0);
				yt_smi_switch_write(0xc0004, 1<<30);
			}
			else if (strcmp(cmd, "get") == 0)
			{ 
				value = strsep(&str, "\t \n");

				if (!value)
				{
					goto error;
				}

				port = simple_strtoul(value, &value, 10);

				if (port <= 9){
					stat_mib_port_get(unit, port);
				}
			}
			else { 
				goto error;
			}
		}
		else { 
			goto error;
		}
	}
	return count;
error: 
	printk("usage: \n");
	printk(" mib enable : for example, echo mib enable > /proc/mib\n");
	printk(" mib disable : for example, echo mib disable > /proc/mib\n");
	printk(" mib clear : for example, echo mib clear > /proc/mib\n");
	printk(" mib get port : for example, echo mib get 8 > /proc/mib\n");
	printk(" get mib 8/9 for extern RGMII counter \n");
	return -EFAULT;
}
static struct proc_dir_entry *mib_proc;
static const struct proc_ops mib_proc_fops = {
	.proc_read = NULL,
	.proc_write = mib_write_proc, 
};

static void smi_mib_proc_test(void)
{ 
	mutex_init(&smi_reg_mutex);
	smi_proc = proc_create("smi", 0666, NULL,&smi_proc_fops);
	mib_proc = proc_create("mib", 0666, NULL,&mib_proc_fops);
}
#endif
