#include <malloc.h>
#include <stdint.h>
#include <sys_hal.h>
#include "dubhe1000_xpcs_serdes.h"

#define XGMAC_MDIO_C22P			0x00000220
#define XGMAC_MDIO_ADDR			0x00000200
#define XGMAC_MDIO_DATA			0x00000204
#define MDIO_CLK_CSR_SHIFT      (19)    
#define MDIO_CLK_CSR_MASK       (GENMASK(21, 19))     

#define MII_BUSY 0x00000001
#define MII_WRITE 0x00000002
#define MII_DATA_MASK (GENMASK(15, 0))

/* XGMAC defines */
#define MII_XGMAC_SADDR			BIT(18)
#define MII_XGMAC_CMD_SHIFT		16
#define MII_XGMAC_CMD_PSE       30
#define MII_XGMAC_WRITE			((1 << MII_XGMAC_CMD_SHIFT) | (1 << MII_XGMAC_CMD_PSE))
#define MII_XGMAC_READ			((3 << MII_XGMAC_CMD_SHIFT) | (1 << MII_XGMAC_CMD_PSE))
#define MII_XGMAC_BUSY			BIT(22)
#define MII_XGMAC_MAX_C22ADDR		4
#define MII_XGMAC_C22P_MASK		GENMASK(MII_XGMAC_MAX_C22ADDR, 0)
#define MII_XGMAC_PA_SHIFT		16
#define MII_XGMAC_DA_SHIFT		21

#define MII_ADDR_C45		(1<<30)
#define MII_DEVADDR_C45_SHIFT	16
#define MII_REGADDR_C45_MASK	GENMASK(15, 0)
#include <string.h>
#define simple_strtol strtol
#define simple_strtoul  strtoul
#ifndef GET_BIT
#define GET_BIT(_x, _pos) \
	(((_x) >> (_pos)) & 1)
#endif

#ifndef CLEAR_BIT
#define CLEAR_BIT(data, bit)		((data) & (~(0x1 << (bit))))
#endif
#define MDIO_PRTAD_NONE			(-1)
#define MDIO_DEVAD_NONE			(-1)
#ifndef ENODEV
#define ENODEV	19
#endif

#ifndef EINVAL
#define EINVAL	22
#endif


#ifndef EBUSY
#define EBUSY	16
#endif

#define	ETIMEDOUT	110	/* Connection timed out */

typedef uint32_t u32;
typedef uint16_t u16;
int g_clk_csr_h = 0;
int g_debug_cls = 0;
#define phy_write(phydev, reglo, data)   
#define phy_read(phydev, reglo)  0 
static int xgmac_c45_format(uint32_t ioaddr, int phyaddr,
		int phyreg, u32 *hw_addr)
{
	u32 tmp;

	/* Set port as Clause 45 */
	tmp = readl(ioaddr + XGMAC_MDIO_C22P);
	tmp &= ~BIT(phyaddr);
	writel(tmp, ioaddr + XGMAC_MDIO_C22P);

	*hw_addr = (phyaddr << MII_XGMAC_PA_SHIFT) | (phyreg & 0xffff);
	*hw_addr |= (phyreg >> MII_DEVADDR_C45_SHIFT) << MII_XGMAC_DA_SHIFT;
	return 0;
}

static int xgmac_c22_format(uint32_t ioaddr, int phyaddr,
		int phyreg, u32 *hw_addr)
{
	u32 tmp;

	if (phyaddr & ~MII_XGMAC_C22P_MASK)
		return -ENODEV;

	/* Set port as Clause 22 */
	tmp = readl(ioaddr + XGMAC_MDIO_C22P);
	tmp &= ~MII_XGMAC_C22P_MASK;
	tmp |= BIT(phyaddr);
	writel(tmp, ioaddr + XGMAC_MDIO_C22P);

	*hw_addr = (phyaddr << MII_XGMAC_PA_SHIFT) | (phyreg & 0x1f);
	return 0;
}

int xgmac_mdio_read(uint32_t ioaddr, int clk_csr, int first_pre, int phyaddr, int devad, int phyreg)
{
	u32 tmp, addr, value = MII_XGMAC_BUSY;
	int ret = 0;
	
	if (g_debug_cls&1)
		printf("mdio read phyaddr=%#x devad=%#x phyreg=%#x\n", phyaddr, devad, phyreg);

	/* Wait until any existing MII operation is complete */
	if (readl_poll_timeout(ioaddr + XGMAC_MDIO_DATA, tmp,
				!(tmp & MII_XGMAC_BUSY), 10000)) {
		ret = -EBUSY;
		goto err_disable_clks;
	}

	if (devad != MDIO_DEVAD_NONE) {
		phyreg |= devad << MII_DEVADDR_C45_SHIFT;
		phyreg |= MII_ADDR_C45;
	}

	if (phyreg & MII_ADDR_C45) {
		phyreg &= ~MII_ADDR_C45;

		ret = xgmac_c45_format(ioaddr, phyaddr, phyreg, &addr);
		if (ret)
			goto err_disable_clks;
	} else {
		ret = xgmac_c22_format(ioaddr, phyaddr, phyreg, &addr);
		if (ret)
			goto err_disable_clks;

		//value |= MII_XGMAC_SADDR;
	}
    if (g_clk_csr_h) 
	value |= BIT(31) | (clk_csr << MDIO_CLK_CSR_SHIFT) & MDIO_CLK_CSR_MASK;
	else
	value |= (clk_csr << MDIO_CLK_CSR_SHIFT) & MDIO_CLK_CSR_MASK;
	value |= MII_XGMAC_READ;

	/* Wait until any existing MII operation is complete */
	if (readl_poll_timeout(ioaddr + XGMAC_MDIO_DATA, tmp,
				!(tmp & MII_XGMAC_BUSY), 10000)) {
		ret = -EBUSY;
		goto err_disable_clks;
	}

	/* Set the MII address register to read */
	writel(addr, ioaddr + XGMAC_MDIO_ADDR);

	if (g_debug_cls&1)
	printf("=r=)write Address[%#x], value=[%#x]\n",ioaddr + XGMAC_MDIO_ADDR, addr);

	if (first_pre) {
		if (g_debug_cls&1)
			printf("Clear MII_XGMAC_CMD_PSE\n");	
		value = CLEAR_BIT(value, MII_XGMAC_CMD_PSE);
	}

	writel(value, ioaddr + XGMAC_MDIO_DATA);

	if (g_debug_cls&1)
	printf("=r=)write Data[%#x], value=[%#x]\n",ioaddr + XGMAC_MDIO_DATA, value);

	/* Wait until any existing MII operation is complete */
	if (readl_poll_timeout(ioaddr + XGMAC_MDIO_DATA, tmp,
				!(tmp & MII_XGMAC_BUSY), 10000)) {
		ret = -EBUSY;
		goto err_disable_clks;
	}

	/* Read the data from the MII data register */
	ret = (int)readl(ioaddr + XGMAC_MDIO_DATA) & GENMASK(15, 0);

#if 0
	if (ret != 0xffff) {
		printf("MDIO add pre form addr(%#x) ret=%#x\n",phyaddr, ret);
		first_pre = 0;
	}
#endif

	if (g_debug_cls&1)
		printf("=r=)read Data[%#x], value=[%#x]\n",ioaddr + XGMAC_MDIO_DATA, ret);

	if (g_debug_cls&2)
		printf("mdio read phyaddr=%#x phyreg=%d value=%#x\n", phyaddr, phyreg, ret);


err_disable_clks:

	return ret;

	return -ETIMEDOUT;
}

int xgmac_mdio_write(uint32_t ioaddr, int clk_csr, int first_pre, int phyaddr,int devad,
		int phyreg,  u16 phydata)
{
	u32 addr, tmp, value = MII_XGMAC_BUSY;
	int ret;

	if (g_debug_cls)
		printf("mdio write phyaddr=%#x devad=%#x phyreg=%x value=%#x\n", phyaddr, devad,  phyreg, phydata);

	/* Wait until any existing MII operation is complete */
	if (readl_poll_timeout(ioaddr + XGMAC_MDIO_DATA, tmp,
				!(tmp & MII_XGMAC_BUSY), 10000)) {
		ret = -EBUSY;
		goto err_disable_clks;
	}

	if (devad != MDIO_DEVAD_NONE) {
		phyreg |= devad << MII_DEVADDR_C45_SHIFT;
		phyreg |= MII_ADDR_C45;
	}

	if (phyreg & MII_ADDR_C45) {
		phyreg &= ~MII_ADDR_C45;

		ret = xgmac_c45_format(ioaddr, phyaddr, phyreg, &addr);
		if (ret)
			goto err_disable_clks;
	} else {
		ret = xgmac_c22_format(ioaddr, phyaddr, phyreg, &addr);
		if (ret)
			goto err_disable_clks;

	//	value |= MII_XGMAC_SADDR;
	}

    if (g_clk_csr_h) 
	value |= BIT(31) | (clk_csr << MDIO_CLK_CSR_SHIFT) & MDIO_CLK_CSR_MASK;
	else
	value |= (clk_csr << MDIO_CLK_CSR_SHIFT) & MDIO_CLK_CSR_MASK;
	value |= phydata;
	value |= MII_XGMAC_WRITE;

	if (first_pre) {
		if (g_debug_cls&1)
			printf("Clear MII_XGMAC_CMD_PSE\n");	
		value = CLEAR_BIT(value, MII_XGMAC_CMD_PSE);
	}

	/* Wait until any existing MII operation is complete */
	if (readl_poll_timeout(ioaddr + XGMAC_MDIO_DATA, tmp,
				!(tmp & MII_XGMAC_BUSY), 10000)) {
		ret = -EBUSY;
		goto err_disable_clks;
	}

	if (g_debug_cls&1) {
		printf("=w=)write Address[%#x], value=[%#x]\n",ioaddr + XGMAC_MDIO_ADDR, addr);
		printf("=w=)write Data[%#x], value=[%#x]\n",ioaddr + XGMAC_MDIO_DATA, value);
	}	

	/* Set the MII address register to write */
	writel(addr, ioaddr + XGMAC_MDIO_ADDR);
	writel(value, ioaddr + XGMAC_MDIO_DATA);

	/* Wait until any existing MII operation is complete */
	ret = readl_poll_timeout(ioaddr + XGMAC_MDIO_DATA, tmp,
			!(tmp & MII_XGMAC_BUSY), 10000);

err_disable_clks:

	return ret;
}

static int extract_range(char *input, int *plo, int *phi)
{
	char *end;
	*plo = simple_strtol(input, &end, 16);
	if (end == input)
		return -1;

	if ((*end == '-') && *(++end))
		*phi = simple_strtol(end, NULL, 16);
	else if (*end == '\0')
		*phi = *plo;
	else
		return -1;

	return 0;
}

/* The register will be in the form [a[-b].]x[-y] */
static int extract_reg_range(char *input, int *devadlo, int *devadhi,
			     int *reglo, int *reghi)
{
	char *regstr;

	/* use strrchr to find the last string after a '.' */
	regstr = strrchr(input, '.');

	/* If it exists, extract the devad(s) */
	if (regstr) {
		char devadstr[32];

		strncpy(devadstr, input, regstr - input);
		devadstr[regstr - input] = '\0';

		if (extract_range(devadstr, devadlo, devadhi))
			return -1;

		regstr++;
	} else {
		/* Otherwise, we have no devad, and we just got regs */
		*devadlo = *devadhi = MDIO_DEVAD_NONE;

		regstr = input;
	}

	return extract_range(regstr, reglo, reghi);
}

uint32_t hextoul(const char *cp, char **endp)
{
	return simple_strtoul(cp, endp, 16);
}

int do_phy(int argc, char *argv[])
{
#define PHY_CMD_NAME   "" 
#define DUBHE1000_MAC_COUNT  5
	char op[2];
	int  reglo, reghi, devadlo, devadhi;
	unsigned short	data;
	int xgmac_index  = 0;
	uint32_t base_addr = 0;
	int first_pre = 1;
	int phyaddr  = 0;
	int pos;

	if (argc < 2) {
		printf( "%s ARG error", __FUNCTION__);
		return -1;
	}
	printf("argv[%s] \n", argv[0]);
	pos = argc - 1;
	//printf( "argc %d pos %d", pos, argc);
	
	op[0] = argv[1][0];
	xgmac_index = argv[0][strlen(PHY_CMD_NAME)] - '0';  
	if (xgmac_index >= DUBHE1000_MAC_COUNT || xgmac_index < 0) {
		printf( "%s err  phy index %s",__FUNCTION__,  argv[0]); 
		return -1;
	}

	base_addr = XGMAC_BASE_ADDR(xgmac_index);

	switch (op[0]) {
	case 'w':
		if (pos > 1)
			data = hextoul(argv[pos--], NULL);
		/* Intentional fall-through - Get reg for read and write */
	case 'r':
		if (pos > 1)
			if (extract_reg_range(argv[pos--], &devadlo, &devadhi,
					      &reglo, &reghi))
				return -1;
		/* Intentional fall-through - Get phy for all commands */
	default:
	
		break;
	}
	//mdio<id> read/write <flags> <addr> <reg> <value>
	if (pos > 1) {
		phyaddr = simple_strtol(argv[pos], NULL, 0);
		pos--;
	}

	while (pos > 1) {
		if ( 0 == strcasecmp("nopre", argv[pos])) 
			first_pre = 0;
		pos--;
	}

    //printf("phy[%#x] phyaddr[%#x] op[%c] devad[%#x] reg[%#x] data[%#x] pre[%d] \n",xgmac_index, phyaddr, op[0], devadlo, reglo, data, first_pre);

	switch (op[0]) {
	case 'w':
		xgmac_mdio_write(base_addr, 1, first_pre, phyaddr, devadlo, reglo, data);
		break;

	case 'r':
		data = xgmac_mdio_read(base_addr, 1, first_pre,  phyaddr, devadlo, reglo);
		if (devadlo == MDIO_DEVAD_NONE)
			 printf("%#x: %#x\n",reglo, data);
		 else 
			 printf("%#x.%#x: %#x\n", devadlo, reglo, data);

		 break;
	}

	return 0;
}
