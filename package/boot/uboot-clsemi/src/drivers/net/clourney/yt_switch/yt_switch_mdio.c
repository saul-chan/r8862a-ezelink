#include <common.h>
#include <command.h>
#include <phy.h>
#include <linux/compat.h>
#include <malloc.h>
#include <string.h>

#include "cal_bprofile.h"
#include "yt_error.h"
#include "yt_init.h"
#include "yt_vlan.h"
#include "yt_port.h"
#include "yt_nic.h"
#include "../npe/cls_npe.h"
#include "yt_port_isolation.h"
#define GPIO_IN     0
#define GPIO_OUT    1

#define GPIO_SET_DIR(addr, i, dir)  ({ uint32_t __val = readl(addr + 4); \
		__val &= ~BIT(i); \
		__val |= BIT(i); \
		writel(__val, addr + 4); \
		if (g_debug_cls) \
			printf("GPIO DIR [%#x] i[%d] SET %#x\n", addr + 4, i, __val); \
		})

#define GPIO_SET_VAILE(addr, i, val) ({ uint32_t __val = readl(addr); \
		__val &= ~BIT(i); \
		if (val) \
			__val |= BIT(i); \
		writel(__val, addr); \
		if (g_debug_cls) \
			printf("GPIO VALUE [%#x] i[%d] SET %#x\n", addr, i, __val); \
		})

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
#define FLAGS_GET_CONFIG  0
struct yt_esw {
	struct cls_eth_priv * adapter;
	struct device           *dev;
	struct mii_dev          *bus;
	ofnode bus_ofnode;
	uint32_t rxc_delay;
	uint32_t txc_delay;
	uint32_t txc_2ns_en;
	uint8_t  port_num;
	uint8_t  group_len;
	uint8_t  gpio_reset;
	uint8_t  bus_no;
	uint8_t  flags;
	uint8_t  group[PORT_MAX_GROUP];
	uint8_t  port[PORT_MAX_GROUP];
};

//extern int yt_cmd_main(int argc,  char *argv[]);
static struct yt_esw _esw;
extern struct mii_dev * mii_bus_list[5];
extern int cls_mdio_init(const char *name, ofnode mdio_node, void *priv);
static void __iomem  * gpio_addr_list[] = {
	0x90050000,
	0x90051000,
};

#if DEBUG
static void smi_mib_proc_test(void);
#endif

//ret = yt_cmd_main(argc, argv);
//printk(KERN_ERR"argc:%d", argc);

uint32_t yt_smi_cl22_read(uint8_t phy_addr, uint8_t phy_register, uint16_t *read_data)
{
	struct yt_esw *esw = &_esw;
	struct mii_dev *bus = esw->bus;
	int devad = MDIO_DEVAD_NONE;

	if(!bus) {
		printf("Can't find mdio dev!!\n");
		return 0;
		/* Save the chosen bus */
		//	miiphy_set_current_dev(bus->name);
	}

	if (read_data ) {
		*read_data = bus->read(bus, phy_addr, devad, phy_register);
		if (g_debug_cls)
			printf("%s phy[%#x] reg[%#x:%d] val[%#x:%d]\n",
					__FUNCTION__,
					phy_addr,
					phy_register,
					phy_register,
					*read_data,
					*read_data);
	}

	return 0;
}

uint32_t yt_smi_cl22_write(uint8_t phy_addr, uint8_t phy_register, uint16_t write_data)
{

	struct yt_esw *esw = &_esw;
	struct mii_dev *bus = esw->bus;
	int devad = MDIO_DEVAD_NONE;

	if (g_debug_cls)
		printf("%s phy[%#x] reg[%#x:%d] val[%#x:%d]\n",
				__FUNCTION__,
				phy_addr,
				phy_register,
				phy_register,
				write_data,
				write_data);

	if(!bus) {

		printf("Can't find mdio dev!!\n");
		return 0;
		/* Save the chosen bus */
		//miiphy_set_current_dev(bus->name);
	}

	bus->write(bus, phy_addr, devad,
			phy_register, write_data);

	return 0;
}

static int esw_hw_reset_gpio(int gpio_pin, int final_value, char* msg)
{
	u32 gpio_n, gpio_g, gpio_i;
	int gpio_gs = 0;
	void __iomem *ioaddr;

	if (gpio_pin < 0)
		return 0;

	printk("Reset YT SWITCH(%s) gpio:%d, final_value=%d\n", msg, gpio_pin, final_value);

	gpio_gs = sizeof(gpio_addr_list) / sizeof(gpio_addr_list[0]);
	gpio_n = gpio_pin;

	gpio_g = gpio_n / 32;
	gpio_i = gpio_n % 32;

	if (gpio_g > gpio_gs) {
		printf("%s: GPIO%d is too max\n", __func__, gpio_n);
		return 0;
	}

	ioaddr = gpio_addr_list[gpio_g];

	GPIO_SET_VAILE(ioaddr, gpio_i, !final_value);
	GPIO_SET_DIR(ioaddr, gpio_i, 1);
	mdelay(100);
	GPIO_SET_VAILE(ioaddr, gpio_i, final_value);
	mdelay(500);

	return 0;
}

static int esw_hw_init(void)
{
	struct yt_esw *esw = &_esw;
	yt_ret_t ret = 0;

	esw_hw_reset_gpio(esw->gpio_reset, 1, "MAC");

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

static int gen_port_map(const char *str, int len, uint8_t * port, uint8_t * group)
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

int cls_yt_switch_create(struct cls_eth_priv *adapter, ofnode np)
{
	struct ofnode_phandle_args phandle;
	struct yt_esw *esw = &_esw;
	uint32_t rxc_delay = 1;
	uint32_t txc_delay = 2;
	uint32_t txc_2ns_en = 1;
	const char *pm;
	int ret;

	ret = ofnode_parse_phandle_with_args(np, "mdio-bus", NULL, 0, 0,
			&phandle);
	if (ret) {
		printk("fail to get mdio-bus\n");
		return -EINVAL;
	}

	ret = ofnode_read_u32(phandle.node, "id", &esw->bus_no);
	if (ret) {
		printk("fail to get mdio-bus no\n");
		return -EINVAL;
	}

    esw->bus_ofnode = phandle.node;
	esw->bus = NULL;

	ret = ofnode_read_u32(np, "reset-gpio", &esw->gpio_reset);
	if (ret) {
		esw->gpio_reset = -1;
		if (ret)
			printk("fail to devm_gpio_request\n");
	}

	pm = ofnode_read_string(np, "port-isolation");
	if (!pm) {
		printk(KERN_ERR"!!!!!Can't find port-isolation DTS\n");
		pm = "yyyyyyx";
	}

	esw->port_num = strlen(pm);
	esw->group_len = gen_port_map(pm, esw->port_num, esw->port, esw->group);

	ret = ofnode_read_u32(np, "rgmii-rxc_delay", &rxc_delay);
	if(ret)
		rxc_delay = 1;

	ret = ofnode_read_u32(np, "rgmii-txc_delay", &txc_delay);
	if(ret)
		txc_delay = 2;

	ret = ofnode_read_u32(np, "rgmii-txc_2ns_en", &txc_2ns_en);
	if(ret)
		txc_2ns_en = 1;

	esw->rxc_delay = rxc_delay;
	esw->txc_delay = txc_delay;
	esw->txc_2ns_en = txc_2ns_en;

	if (g_debug_cls)
		pr_err("YT_SW: RGMII rxc_delay [%d] txc_delay [%d] txc_2ns_en [%d]\n",
				rxc_delay, txc_delay, txc_2ns_en);

	esw->adapter = adapter;
	esw->flags |= BIT(FLAGS_GET_CONFIG);

	pr_err("YT_SW: PROBE Successfully\n");

	return 0;
}

int cls_yt_switch_init()
{
	int i,j;
	struct yt_esw *esw = &_esw;
	int port_num = esw->port_num;
	uint8_t *group = esw->group;
	uint8_t *port = esw->port;
	struct mii_dev *bus;

	if (!(esw->flags & BIT(FLAGS_GET_CONFIG)))
		return 0;

	bus = mii_bus_list[esw->bus_no];
	if (!bus) {
		if (cls_mdio_init("mdio_sw", esw->bus_ofnode, &esw->adapter->xgmac[esw->bus_no])) {
			printf("Can't create mdio dev!!\n");
			return -1;
		}
		bus = mii_bus_list[esw->bus_no];
	}

	esw->bus = bus;
	esw->adapter->xgmac[esw->bus_no].first_pre |= BIT(0x1b);

	esw_hw_init();

	for (i = 0; i < port_num; i++) {

		int offset = 0;
		char str_buf[PORT_MAX_GROUP * 2 + 1] = {0};
		yt_port_mask_t iso_portmask = {0};

		iso_portmask.portbits[0] = (~group[port[i]]) & GENMASK(port_num - 1, 0);

		yt_port_isolation_set(UNIT, i, iso_portmask);

		for (j = 0; j < port_num; j++)
			offset += sprintf(str_buf + offset, "%d ", !!(group[port[i]] & BIT(j)));

		if (g_debug_cls)
			printk("Set YT_SWITCH isolation\n\tport%d mask[%#x] map %s\n",
					i, iso_portmask.portbits[0], str_buf);
	}

	esw_set_hsgmii();

	//esw_set_rgmii(esw->rxc_delay, esw->txc_delay, esw->txc_2ns_en);
	return 0;
}

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
