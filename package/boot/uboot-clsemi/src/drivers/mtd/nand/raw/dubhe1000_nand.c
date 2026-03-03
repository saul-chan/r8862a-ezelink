/*
 * Copyright (C) 2022 Clourney Semiconductor. All rights reserved.
 */

#include <common.h>
#include <log.h>
#include <malloc.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <nand.h>
#include <linux/ioport.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/rawnand.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/dma-mapping.h>
#include <dm.h>
#include <dm/uclass.h>
#include <dm/device_compat.h>

#define MACRO_RECOMBINE(x, y, z, c)    (((uint32_t)x<<24u)|((uint32_t)y<<16u)|((uint32_t)z<<8u)|(uint32_t)c)

/* this follow seqs from pnand spec. */
#define SEQ_0     0x00u
#define SEQ_1     0x21u
#define SEQ_2     0x22u
#define SEQ_3     0x03u
#define SEQ_4     0x24u
#define SEQ_5     0x25u
#define SEQ_6     0x26u
#define SEQ_7     0x27u
#define SEQ_8     0x08u
#define SEQ_9     0x29u
#define SEQ_10    0x2au
#define SEQ_11    0x2bu
#define SEQ_12    0x0cu
#define SEQ_13    0x0du
#define SEQ_14    0x0eu
#define SEQ_15    0x2fu
#define SEQ_17    0x31u
#define SEQ_18    0x32u
#define SEQ_19    0x13u
#define SEQ_20    0x34u
#define SEQ_21    0x15u
#define SEQ_22    0x36u
#define SEQ_23    0x17u
#define SEQ_24    0x18u
#define SEQ_25    0x39u

/************ COMMAND values ************/
#define RESET_DEV_CMD       MACRO_RECOMBINE(0x00, 0x00, 0xff, SEQ_0)
#define FLASH_STATUS        MACRO_RECOMBINE(0x00, 0x00, 0x70, SEQ_4)
#define ERASE_BLOCK_CMD     MACRO_RECOMBINE(0x00, 0xd0, 0x60, SEQ_14)
#define READ_CMD            MACRO_RECOMBINE(0x30, 0x00, 0x00, SEQ_10)
#define WRITE_CMD           MACRO_RECOMBINE(0x00, 0x10, 0x80, SEQ_12)
#define RANDOM_DATA_READ    MACRO_RECOMBINE(0xe0, 0x00, 0x05, SEQ_6)
#define READ_ID             MACRO_RECOMBINE(0x00, 0x00, 0x90, SEQ_1)
#define READ_PARAM          MACRO_RECOMBINE(0x00, 0x00, 0xec, SEQ_2)
#define GET_FEATURES        MACRO_RECOMBINE(0x00, 0x00, 0xee, SEQ_2)
#define SET_FEATURES        MACRO_RECOMBINE(0x00, 0x00, 0xef, SEQ_3)

#define DUBHE1000_NAND_CMD_READ_ID_SIZE     8
#define DUBHE1000_NAND_CMD_READ_PARAM_SIZE  768


/************ COMMAND Register ************/
#define CMD_REG_OFFSET       (0x0)
#define CMD_2_BIT            (24)
#define CMD_2_SIZE           (8)
#define CMD_1_3_BIT          (16)
#define CMD_1_3_SIZE         (8)
#define CMD_0_BIT            (8)
#define CMD_0_SIZE           (8)

/**
 * 0 – the FIFO module selected
 * 1 – the DATA register selected
 */
#define CMD_DATA_SEL_BIT    (7)
#define CMD_DATA_SEL_SIZE   (1)
#define CMD_DATA_SEL_FIFO   (0 << CMD_DATA_SEL_BIT)
#define CMD_DATA_SEL_DATA   (1 << CMD_DATA_SEL_BIT)
/**
 * 0 – select the SIU module as input
 * 1 – select the DMA module as input
 */
#define CMD_INPUT_SEL_BIT    (6)
#define CMD_INPUT_SEL_SIZE   (1)
#define CMD_INPUT_SEL_SIU    (0 << CMD_INPUT_SEL_BIT)
#define CMD_INPUT_SEL_DMA    (1 << CMD_INPUT_SEL_BIT)
/* Command code */
#define CMD_SEQ_BIT          (0)
#define CMD_SEQ_SIZE         (6)

/************ CONTROL Register ************/
#define DEV_CTRL_REG_OFFSET      (0x4)

#define DEV_CTRL_WIDTH_16_VAL   0x1000

/**
 *0 – Multi LUN mode disabled
 *1 – Multi LUN mode enabled
*/
#define MLUN_EN_BIT              (22)
#define MLUN_EN_SIZE             (1)
/**
 *0 – big block mode enabled
 *1 – small block mode enabled
 */
#define SMALL_BLOCK_EN_BIT       (21)
#define SMALL_BLOCK_EN_SIZE      (1)
/**
 *00 – asynchronous mode
 *01 – source synchronous mode
 *10 – Toogle mode
 *11 – not available
 */
#define WORK_MODE_BIT            (19)
#define WORK_MODE_SIZE           (2)
/**
 *0 – disabled
 *1 – enabled
 */
#define CLN_EN_BIT               (18)
#define CLN_EN_SIZE              (1)
/**
 *0 – auto increment disabled (ADDR1_ROW)
 *1 – auto increment enabled  (ADDR1_ROW)
 */
#define ADDR1_AUTO_INCR_BIT      (17)
#define ADDR1_AUTO_INCR_SIZE     (1)
/**
 *0 – auto increment disabled (ADDR0_ROW)
 *1 – auto increment enabled  (ADDR0_ROW)
 */
#define ADDR0_AUTO_INCR_BIT      (16)
#define ADDR0_AUTO_INCR_SIZE     (1)
/**
 *0 - protect disabled
 *1 – protect enabled
 */
#define PROT_EN_BIT              (14)
#define PROT_EN_SIZE             (1)
/* Bad Block Management enable flag */
#define BBM_EN_BIT               (13)
#define BBM_EN_SIZE              (1)
/**
 *0 – 8 bits
 *1 – 16 bits
 */
#define IO_WIDTH_BIT             (12)
#define IO_WIDTH_SIZE            (1)
/**
 *0 – Single 16-bit device is selected.
 *1 – Two 8-bit devices are selected
 */
#define DEV_STACK_BIT            (11)
#define DEV_STACK_SIZE           (1)
/**
 *00 – 32 pages per block
 *01 – 64 pages per block
 *10 – 128 pages per block
 *11 – 256 pages per block
 */
#define BLOCK_SIZE_BIT           (6)
#define BLOCK_SIZE_SIZE          (2)
/**
 *0 – ECC disabled
 *1 – ECC enabled
 */
#define ECC_EN_BIT               (5)
#define ECC_EN_SIZE              (1)
/**
 *0 – Interrupts disabled
 *1 – Interrupts enabled
 */
#define INT_EN_BIT               (4)
#define INT_EN_SIZE              (1)
/**
 *0 – Retry response disabled
 *1 – Retry response enabled
 */
#define RETRY_EN_BIT             (3)
#define RETRY_EN_SIZE            (1)
/**
 *00 – 256 bytes
 *01 – 512 bytes
 *10 – 1024 bytes
 *11 – not available
 */
#define ECC_BLOCK_SIZE_BIT       (1)
#define ECC_BLOCK_SIZE_SIZE      (2)

/**
 *0 – The controller checks RNB lines
 *1 – The Controller sends READ STATUS commands
 */
#define READ_STATUS_EN_BIT       (0)
#define READ_STATUS_EN_SIZE      (1)

/************ STATUS Register ************/
#define STATUS_REG_OFFSET        (0x8)
/**
 *1 – data in DATA_REG is available
 *0 – data in DATA_REG is not available
 */
#define DATA_REG_ST_BIT           (10)
#define DATA_REG_ST_SIZE           (1)

/************ INT_MASK Register ************/
#define INT_MASK_REG_OFFSET    0x10
/**
 *0 – interrupt disabled
 *1 – interrupt enabled
 */
#define DMA_INT_EN_BIT        (3)
#define DMA_INT_EN_SIZE       (1)
/**
 *0 – interrupt disabled
 *1 – interrupt enabled
 */
#define DATA_REG_INT_EN_BIT   (2)
#define DATA_REG_INT_EN_SIZE  (1)

/************ interrupt status Register ************/
#define INT_STATUS_OFFSET     0x14

/************ ECC Register ************/
#define ECC_CTRL                0x18
#define ECC_OFFSET              0x1C
#define ECC_STAT                0x20

#define ECC_SIZE_DEFAULE_VAL    512
#define ECC_OFFSET_DEFAULE_VAL  8

/**
 * A15-A0 address bits.
 */
#define ADDR0_COL_REG_OFFSET  0x24

/************ Page address Register ************/
/* !<A39-A16 address bits. */
#define ADDR0_ROW_REG_OFFSET   0x28
#define WRITER_FIFO_REG_OFFSET 0x38
#define READ_DATA_REG_OFFSET   0x3C

/* data register size */
#define DATA_REG_SIZE_REG_OFFSET 0x40

#define DATA_REG_SIZE_BYTE_1  0
#define DATA_REG_SIZE_BYTE_2  1
#define DATA_REG_SIZE_BYTE_3  2
#define DATA_REG_SIZE_BYTE_4  3

#define DMA_ADDRL_REG_OFFSET  0x64

/************ DMA count Register ************/
/* !<The number of the bytes has to be divided by 4> */
#define DMA_CNT_REG_OFFSET    0x6c

/************ DMA controller Register ************/
#define DMA_CTRL_REG_OFFSET   0x70
#define DMA_START_BIT         (7)
#define DMA_START_SIZE        (1)
#define DMA_READY_BIT         (0)
#define DMA_READY_SIZE        (1)

/**
 *0 – the SFR-s managed mode
 *1 – the Scatter-Gather mode
 */
#define DMA_MODE_BIT          (5)
#define DMA_MODE_SIZE         (1)
#define DMA_BURST_BIT         (2)
#define DMA_BURST_SIZE        (3)
/* DMA error flag */
#define ERR_FLAG_BIT          (1)
#define ERR_FLAG_SIZE         (1)
/* DMA ready flag */
#define DMA_READY_BIT         (0)
#define DMA_READY_SIZE        (1)

/************ MEM controller Register ************/
#define MEM_CTRL_REG_OFFSET       0x80
/* The value of this field defines data size. [14:0] */
#define DATA_SIZE_REG_OFFSET      0x84
#define TIMINGS_ASYN_REG_OFFSET   0x88
#define TIMINGS_SYN_REG_OFFSET    0x8c
#define TIME_SEQ_0_REG_OFFSET     0x90
#define TIME_SEQ_1_REG_OFFSET     0x94

#define CONTROLLER_DEVICE0_STATUS      (1<<0)
#define DMA_READY_FLAG                 (1<<DMA_READY_BIT)
#define DEV_RDY_STATUS                 (1<<6)

#if (CONFIG_BRD_PLAT_RUN_CLK_VARIANT == BRD_PLAT_RUN_SOC_CLK)
// tWHR 15 tRHW 25 tADL 25 tCCS 3
#define TIMER_SEQ_0_VAL         0x3F3F3F3F  //tWHR bit29-24 tRHW bit21-16 tADL bit13-8 tCCS bit5-0
// tRR 5 tWB 25
#define TIMER_SEQ_1_VAL         0x00003F3F  //tRR bit13-8  tWB bit5-0
// tRWH 4 tRWP 7
#define TIMER_ASYN_VAL          0x000000FF  //tRWH bit7-4  tRWP bit3-0
#else
#define TIMER_SEQ_0_VAL         0x01010101
#define TIMER_SEQ_1_VAL         0x00000101
#define TIMER_ASYN_VAL          0x00000011
#endif

#define CONTROL_VAL             0x0003004A
#define MEM_CTRL_VAL            0x0000FF00

/************ FIFO Register ************/
#define FIFO_STATE_REG_OFFSET       0xB4

#define CF_EMPTY_STATUS                 (1<<2)

struct dubhe1000_nand_buf {
    u8 *dmabuf;
    dma_addr_t dmaaddr;
    int dmabuflen;
    int data_len;
    int data_index;
};

struct nand_drv {
    phys_addr_t base;
    struct dubhe1000_nand_buf nand_buf;
    bool using_dma;
    bool set_feature;
};

struct dubhe1000_nand_timings {
	u32 t_async;
	u32 t0_sync;
	u32 t1_sync;
	u32 t2_sync;
};

struct dubhe1000_nand_info {
    struct udevice *dev;
    struct dubhe1000_nand_timings timings;
    struct nand_drv nand_ctrl;
    struct nand_chip nand_chip;
};

static u32 dubhe1000_nand_reg_read(struct nand_drv *nand_ctrl, u32 offset)
{
    return readl(nand_ctrl->base + offset);
}

static void dubhe1000_nand_reg_write(struct nand_drv *nand_ctrl, u32 offset, u32 value)
{
    writel(value, nand_ctrl->base + offset);
}

static void dubhe1000_nand_cmd_write(struct nand_drv *nand_ctrl, u32 value, u32 mode)
{
    dubhe1000_nand_reg_write(nand_ctrl, CMD_REG_OFFSET, value | mode);
}

static void dubhe1000_nand_addr_set(struct nand_drv *nand_ctrl, u32 row, u32 column)
{
    dubhe1000_nand_reg_write(nand_ctrl, ADDR0_ROW_REG_OFFSET, row);
    dubhe1000_nand_reg_write(nand_ctrl, ADDR0_COL_REG_OFFSET, column);
}

static void dubhe1000_nand_size_set(struct nand_drv *nand_ctrl, u32 size)
{
    dubhe1000_nand_reg_write(nand_ctrl, DATA_SIZE_REG_OFFSET, size);
}

static void dubhe1000_nand_dma_set(struct nand_drv *nand_ctrl, u32 size)
{
    dubhe1000_nand_reg_write(nand_ctrl, DMA_ADDRL_REG_OFFSET, nand_ctrl->nand_buf.dmaaddr);
    dubhe1000_nand_reg_write(nand_ctrl, DATA_SIZE_REG_OFFSET, size);
    dubhe1000_nand_reg_write(nand_ctrl, DMA_CNT_REG_OFFSET, size);
    dubhe1000_nand_reg_write(nand_ctrl, DMA_CTRL_REG_OFFSET, 
        dubhe1000_nand_reg_read(nand_ctrl, DMA_CTRL_REG_OFFSET) | (1 << DMA_START_BIT));
}

/*
 * dubhe1000_nand_init_nand_flash - Initialize NAND controller
 * @option:    Device property flags
 *
 * This function initializes the NAND flash interface on the NAND controller.
 *
 * returns:    0 on success or error value on failure
 */
static int dubhe1000_nand_init_nand_flash(struct nand_drv *nand_ctrl, struct dubhe1000_nand_timings *t)
{
    dubhe1000_nand_reg_write(nand_ctrl, TIME_SEQ_0_REG_OFFSET, t->t0_sync);
    dubhe1000_nand_reg_write(nand_ctrl, TIME_SEQ_1_REG_OFFSET, t->t1_sync);
    dubhe1000_nand_reg_write(nand_ctrl, TIMINGS_ASYN_REG_OFFSET, t->t_async);
    /* 64 pages per block, en_clean. */
    dubhe1000_nand_reg_write(nand_ctrl, DEV_CTRL_REG_OFFSET, CONTROL_VAL);
    /* change mem_dev0 as lun device default. */
    dubhe1000_nand_reg_write(nand_ctrl, MEM_CTRL_REG_OFFSET, MEM_CTRL_VAL);
    return 0;
}

void dubhe1000_nand_bch_hwctl(struct mtd_info *mtd, int mode)
{
    /* Do nothing. */
}

/*
 * dubhe1000_nand_calculate_hwecc - Calculate Hardware ECC
 * @mtd:    Pointer to the mtd_info structure
 * @data:    Pointer to the page data
 * @ecc_code:    Pointer to the ECC buffer where ECC data needs to be stored
 *
 * This function retrieves the Hardware ECC data from the controller and returns
 * ECC data back to the MTD subsystem.
 *
 * returns:    0 on success or error value on failure
 */
static int dubhe1000_nand_calculate_hwecc(struct mtd_info *mtd, const u8 *data,
        u8 *ecc_code)
{
    /* Do nothing. */
    return 0;
}

/*
 * dubhe1000_nand_correct_data - ECC correction function
 * @mtd:    Pointer to the mtd_info structure
 * @buf:    Pointer to the page data
 * @read_ecc:    Pointer to the ECC value read from spare data area
 * @calc_ecc:    Pointer to the calculated ECC value
 *
 * This function corrects the ECC single bit errors & detects 2-bit errors.
 *
 * returns:    0 if no ECC errors found
 *        1 if single bit error found and corrected.
 *        -1 if multiple ECC errors found.
 */
static int dubhe1000_nand_correct_data(struct mtd_info *mtd, unsigned char *buf,
            unsigned char *read_ecc, unsigned char *calc_ecc)
{
    /* Do nothing. */
    return 0; /* Uncorrectable error */
}

/*
 * dubhe1000_nand_select_chip - Select the flash device
 * @mtd:    Pointer to the mtd_info structure
 * @chip:    Chip number to be selected
 *
 * This function is empty as the NAND controller handles chip select line
 * internally based on the chip address passed in command and data phase.
 */
static void dubhe1000_nand_select_chip(struct mtd_info *mtd, int chip)
{
    /* Not support multiple chips yet */
}

static void dubhe1000_nand_read_from_fifo(struct nand_drv *nand_ctrl, u32 size)
{
    u32 i;

    for(i = 0; i < size / 4; i++)
    {
        *(u32*)(nand_ctrl->nand_buf.dmabuf + nand_ctrl->nand_buf.data_index + 4 * i) = \
            dubhe1000_nand_reg_read(nand_ctrl, WRITER_FIFO_REG_OFFSET);
    }
}

static void dubhe1000_nand_write_to_fifo(struct nand_drv *nand_ctrl, u32 size)
{
    u32 i;

    for(i = 0; i < size / 4; i++)
    {
        dubhe1000_nand_reg_write(nand_ctrl, WRITER_FIFO_REG_OFFSET, *(u32*)(nand_ctrl->nand_buf.dmabuf + 4 * i));
    }
}

/*
 * Read a single byte from the temporary buffer. Used after READID
 * to get the NAND information and for STATUS.
 */
static u8 dubhe1000_nand_read_byte(struct mtd_info *mtd)
{
    struct nand_chip *nand_chip = mtd_to_nand(mtd);
    struct nand_drv *nand_ctrl = nand_get_controller_data(nand_chip);

    if (nand_ctrl->nand_buf.data_index < nand_ctrl->nand_buf.data_len)
        return nand_ctrl->nand_buf.dmabuf[nand_ctrl->nand_buf.data_index++];

    printf("No data to read, idx: 0x%x, len: 0x%x\n",
        nand_ctrl->nand_buf.data_index, nand_ctrl->nand_buf.data_len);

	return 0xff;
}

/*
 * dubhe1000_nand_read_buf - read chip data into buffer
 * @mtd:        MTD device structure
 * @buf:        buffer to store date
 * @len:        number of bytes to read
 */
static void dubhe1000_nand_read_buf(struct mtd_info *mtd, u8 *buf, int len)
{
    struct nand_chip *nand_chip = mtd_to_nand(mtd);
    struct nand_drv *nand_ctrl = nand_get_controller_data(nand_chip);

    if (len > nand_ctrl->nand_buf.data_len - nand_ctrl->nand_buf.data_index) 
    {
        printf("No enough data for read of %d bytes\n", len);
        return;
    }

    memcpy(buf, nand_ctrl->nand_buf.dmabuf + nand_ctrl->nand_buf.data_index, len);
    nand_ctrl->nand_buf.data_index += len;
}

/*
 * dubhe1000_nand_write_buf - write buffer to chip
 * @mtd:        MTD device structure
 * @buf:        data buffer
 * @len:        number of bytes to write
 */
static void dubhe1000_nand_write_buf(struct mtd_info *mtd, const u8 *buf, int len)
{
    struct nand_chip *nand_chip = mtd_to_nand(mtd);
    struct nand_drv *nand_ctrl = nand_get_controller_data(nand_chip);

    memcpy(nand_ctrl->nand_buf.dmabuf + nand_ctrl->nand_buf.data_len, buf, len);
    nand_ctrl->nand_buf.data_len += len;

    if(nand_ctrl->set_feature)
    {
        nand_ctrl->set_feature = false;
        dubhe1000_nand_write_to_fifo(nand_ctrl, nand_ctrl->nand_buf.data_len);
    }
}

/*
 * dubhe1000_nand_wait_fifo_ready - Check fifo ready/busy status
 * @mtd:    Pointer to the mtd_info structure
 *
 * returns:    0 on busy or 1 on ready state
 */
static void dubhe1000_nand_wait_fifo_ready(struct mtd_info *mtd)
{
    struct nand_chip *nand_chip = mtd_to_nand(mtd);
    struct nand_drv *nand_ctrl = nand_get_controller_data(nand_chip);
    u32 value;

    do
    {
        value = dubhe1000_nand_reg_read(nand_ctrl, FIFO_STATE_REG_OFFSET);
        //printf("%s %d value %x\n", __func__, __LINE__, value);
    }while(!(value & CF_EMPTY_STATUS));
}

/*
 * dubhe1000_nand_device_ready - Check device ready/busy line
 * @mtd:    Pointer to the mtd_info structure
 *
 * returns:    0 on busy or 1 on ready state
 */
static int dubhe1000_nand_device_ready(struct mtd_info *mtd)
{
    struct nand_chip *nand_chip = mtd_to_nand(mtd);
    struct nand_drv *nand_ctrl = nand_get_controller_data(nand_chip);
    u32 value;

    value = dubhe1000_nand_reg_read(nand_ctrl, STATUS_REG_OFFSET) & CONTROLLER_DEVICE0_STATUS;
    if(!value)
    {
        return 0;
    }

    value = dubhe1000_nand_reg_read(nand_ctrl, DMA_CTRL_REG_OFFSET) & DMA_READY_FLAG;
    if(!value)
    {
        return 0;
    }

    return 1;
}

/*
 * dubhe1000_nand_cmd_function - Send command to NAND device
 * @mtd:    Pointer to the mtd_info structure
 * @command:    The command to be sent to the flash device
 * @column:    The column address for this command, -1 if none
 * @page_addr:    The page address for this command, -1 if none
 */
static void dubhe1000_nand_cmd_function(struct mtd_info *mtd, unsigned int command,
                 int column, int page_addr)
{
    struct nand_chip *chip = mtd_to_nand(mtd);
    struct nand_drv *nand_ctrl = nand_get_controller_data(chip);

    nand_ctrl->set_feature = false;

    if(NAND_CMD_PAGEPROG != command && NAND_CMD_RNDOUT != command)
    {
        memset(nand_ctrl->nand_buf.dmabuf, 0xff, nand_ctrl->nand_buf.dmabuflen);
    }

    switch (command) {
    case NAND_CMD_READID:
        nand_ctrl->nand_buf.data_index = 0;
        nand_ctrl->nand_buf.data_len = DUBHE1000_NAND_CMD_READ_ID_SIZE;
        dubhe1000_nand_addr_set(nand_ctrl, 0, column);
        dubhe1000_nand_size_set(nand_ctrl, DUBHE1000_NAND_CMD_READ_ID_SIZE);
        dubhe1000_nand_cmd_write(nand_ctrl, READ_ID, CMD_INPUT_SEL_SIU | CMD_DATA_SEL_FIFO);
        dubhe1000_nand_wait_fifo_ready(mtd);
        nand_wait_ready(mtd);
        dubhe1000_nand_read_from_fifo(nand_ctrl, DUBHE1000_NAND_CMD_READ_ID_SIZE);
        break;
    case NAND_CMD_PARAM:
        nand_ctrl->nand_buf.data_index = 0;
        nand_ctrl->nand_buf.data_len = DUBHE1000_NAND_CMD_READ_PARAM_SIZE;
        dubhe1000_nand_addr_set(nand_ctrl, 0, column);
        dubhe1000_nand_size_set(nand_ctrl, DUBHE1000_NAND_CMD_READ_PARAM_SIZE);
        dubhe1000_nand_cmd_write(nand_ctrl, READ_PARAM, CMD_INPUT_SEL_SIU | CMD_DATA_SEL_FIFO);
        dubhe1000_nand_wait_fifo_ready(mtd);
        nand_wait_ready(mtd);
        dubhe1000_nand_read_from_fifo(nand_ctrl, DUBHE1000_NAND_CMD_READ_PARAM_SIZE);
        break;
    case NAND_CMD_READOOB:
        column += mtd->writesize;
        nand_ctrl->nand_buf.data_index = 0;
        nand_ctrl->nand_buf.data_len = mtd->oobsize;
        if(nand_ctrl->using_dma)
        {
            dubhe1000_nand_addr_set(nand_ctrl, page_addr, column);
            dubhe1000_nand_dma_set(nand_ctrl, mtd->oobsize);
            dubhe1000_nand_cmd_write(nand_ctrl, READ_CMD, CMD_INPUT_SEL_DMA | CMD_DATA_SEL_FIFO);
        }
        else
        {
            dubhe1000_nand_addr_set(nand_ctrl, page_addr, column);
            dubhe1000_nand_size_set(nand_ctrl, mtd->oobsize);
            dubhe1000_nand_cmd_write(nand_ctrl, READ_CMD, CMD_INPUT_SEL_SIU | CMD_DATA_SEL_FIFO);
            dubhe1000_nand_wait_fifo_ready(mtd);
            nand_wait_ready(mtd);
            dubhe1000_nand_read_from_fifo(nand_ctrl, mtd->oobsize);
        }
        break;
#if 1 //WAR for 2k+64 size read
    case NAND_CMD_READ0:
        nand_ctrl->nand_buf.data_index = 0;
        nand_ctrl->nand_buf.data_len = mtd->writesize;
        if(nand_ctrl->using_dma)
        {
            dubhe1000_nand_addr_set(nand_ctrl, page_addr, column);
            dubhe1000_nand_dma_set(nand_ctrl, mtd->writesize);
            dubhe1000_nand_cmd_write(nand_ctrl, READ_CMD, CMD_INPUT_SEL_DMA | CMD_DATA_SEL_FIFO);
        }
        else
        {
            dubhe1000_nand_addr_set(nand_ctrl, page_addr, column);
            dubhe1000_nand_size_set(nand_ctrl, mtd->writesize);
            dubhe1000_nand_cmd_write(nand_ctrl, READ_CMD, CMD_INPUT_SEL_SIU | CMD_DATA_SEL_FIFO);
            dubhe1000_nand_wait_fifo_ready(mtd);
            nand_wait_ready(mtd);
            dubhe1000_nand_read_from_fifo(nand_ctrl, mtd->writesize);
        }

        column = mtd->writesize;
        nand_ctrl->nand_buf.data_index = mtd->writesize;
        nand_ctrl->nand_buf.data_len = mtd->writesize + mtd->oobsize;
        if(nand_ctrl->using_dma)
        {
            dubhe1000_nand_addr_set(nand_ctrl, page_addr, column);
            dubhe1000_nand_dma_set(nand_ctrl, mtd->oobsize);
            dubhe1000_nand_cmd_write(nand_ctrl, READ_CMD, CMD_INPUT_SEL_DMA | CMD_DATA_SEL_FIFO);
        }
        else
        {
            dubhe1000_nand_addr_set(nand_ctrl, page_addr, column);
            dubhe1000_nand_size_set(nand_ctrl, mtd->oobsize);
            dubhe1000_nand_cmd_write(nand_ctrl, READ_CMD, CMD_INPUT_SEL_SIU | CMD_DATA_SEL_FIFO);
            dubhe1000_nand_wait_fifo_ready(mtd);
            nand_wait_ready(mtd);
            dubhe1000_nand_read_from_fifo(nand_ctrl, mtd->oobsize);
        }
        nand_ctrl->nand_buf.data_index = 0;
        break;
#else
    case NAND_CMD_READ0:
        nand_ctrl->nand_buf.data_index = 0;
        nand_ctrl->nand_buf.data_len = mtd->writesize + mtd->oobsize;
        if(nand_ctrl->using_dma)
        {
            dubhe1000_nand_addr_set(nand_ctrl, page_addr, column);
            dubhe1000_nand_dma_set(nand_ctrl, mtd->writesize + mtd->oobsize);
            dubhe1000_nand_cmd_write(nand_ctrl, READ_CMD, CMD_INPUT_SEL_DMA | CMD_DATA_SEL_FIFO);
        }
        else
        {
            dubhe1000_nand_addr_set(nand_ctrl, page_addr, column);
            dubhe1000_nand_size_set(nand_ctrl, mtd->writesize + mtd->oobsize);
            dubhe1000_nand_cmd_write(nand_ctrl, READ_CMD, CMD_INPUT_SEL_SIU | CMD_DATA_SEL_FIFO);
            dubhe1000_nand_wait_fifo_ready(mtd);
            nand_wait_ready(mtd);
            dubhe1000_nand_read_from_fifo(nand_ctrl, mtd->writesize + mtd->oobsize);
        }
        break;
#endif
    case NAND_CMD_SEQIN:
        nand_ctrl->nand_buf.data_index = 0;
        nand_ctrl->nand_buf.data_len = 0;
        dubhe1000_nand_addr_set(nand_ctrl, page_addr, column);
        break;
    case NAND_CMD_PAGEPROG:
        if(nand_ctrl->using_dma)
        {
            dubhe1000_nand_dma_set(nand_ctrl, nand_ctrl->nand_buf.data_len);
            dubhe1000_nand_cmd_write(nand_ctrl, WRITE_CMD, CMD_INPUT_SEL_DMA | CMD_DATA_SEL_FIFO);
        }
        else
        {
            dubhe1000_nand_size_set(nand_ctrl, nand_ctrl->nand_buf.data_len);
            dubhe1000_nand_cmd_write(nand_ctrl, WRITE_CMD, CMD_INPUT_SEL_SIU | CMD_DATA_SEL_FIFO);
            dubhe1000_nand_write_to_fifo(nand_ctrl, nand_ctrl->nand_buf.data_len);
        }
        break;
    case NAND_CMD_ERASE1:
        dubhe1000_nand_addr_set(nand_ctrl, page_addr, 0);
        dubhe1000_nand_cmd_write(nand_ctrl, ERASE_BLOCK_CMD, CMD_INPUT_SEL_SIU | CMD_DATA_SEL_FIFO);
        break;
    case NAND_CMD_ERASE2:
        break;
    case NAND_CMD_STATUS:
        nand_ctrl->nand_buf.data_index = 0;
        nand_ctrl->nand_buf.data_len = 1;
        dubhe1000_nand_reg_write(nand_ctrl, DATA_REG_SIZE_REG_OFFSET, DATA_REG_SIZE_BYTE_1);
        dubhe1000_nand_cmd_write(nand_ctrl, FLASH_STATUS, CMD_INPUT_SEL_SIU | CMD_DATA_SEL_DATA);
        nand_wait_ready(mtd);
        nand_ctrl->nand_buf.dmabuf[0] = dubhe1000_nand_reg_read(nand_ctrl, READ_DATA_REG_OFFSET);
        break;
    case NAND_CMD_RESET:
        dubhe1000_nand_cmd_write(nand_ctrl, RESET_DEV_CMD, CMD_INPUT_SEL_SIU | CMD_DATA_SEL_FIFO);
        break;
    case NAND_CMD_RNDOUT:
        nand_ctrl->nand_buf.data_index = column;
        break;
    case NAND_CMD_SET_FEATURES:
        nand_ctrl->nand_buf.data_index = 0;
        nand_ctrl->nand_buf.data_len = ONFI_SUBFEATURE_PARAM_LEN;
        dubhe1000_nand_addr_set(nand_ctrl, page_addr, 0);
        dubhe1000_nand_size_set(nand_ctrl, ONFI_SUBFEATURE_PARAM_LEN);
        dubhe1000_nand_cmd_write(nand_ctrl, SET_FEATURES, CMD_INPUT_SEL_SIU | CMD_DATA_SEL_FIFO);
        nand_ctrl->set_feature = true;
        return;
    case NAND_CMD_GET_FEATURES:
        nand_ctrl->nand_buf.data_index = 0;
        nand_ctrl->nand_buf.data_len = ONFI_SUBFEATURE_PARAM_LEN;
        dubhe1000_nand_addr_set(nand_ctrl, page_addr, 0);
        dubhe1000_nand_size_set(nand_ctrl, ONFI_SUBFEATURE_PARAM_LEN);
        dubhe1000_nand_cmd_write(nand_ctrl, GET_FEATURES, CMD_INPUT_SEL_SIU | CMD_DATA_SEL_FIFO);
        dubhe1000_nand_wait_fifo_ready(mtd);
        nand_wait_ready(mtd);
        dubhe1000_nand_read_from_fifo(nand_ctrl, ONFI_SUBFEATURE_PARAM_LEN);
        break;
    default:
        printf("%s: Unsupported command %d\n", __func__, command);
        break;
    }
    nand_wait_ready(mtd);
}

static int dubhe1000_nand_probe(struct udevice *dev)
{
    struct dubhe1000_nand_info *dubhe1000 = dev_get_priv(dev);
    struct dubhe1000_nand_timings *t = &dubhe1000->timings;
    struct nand_chip *nand_chip = &dubhe1000->nand_chip;
    struct nand_drv *nand_ctrl = &dubhe1000->nand_ctrl;
    struct mtd_info *mtd;

    if (!dev_read_enabled(dev))
        return -1;

    nand_ctrl->base = dev_read_addr(dev);
    //dma mode is not supported now
    nand_ctrl->using_dma = false;
    nand_ctrl->nand_buf.dmabuflen = NAND_MAX_PAGESIZE + NAND_MAX_OOBSIZE;
    nand_ctrl->nand_buf.dmabuf = dma_alloc_coherent(nand_ctrl->nand_buf.dmabuflen,
                    (unsigned long *)&nand_ctrl->nand_buf.dmaaddr);
    if (!nand_ctrl->nand_buf.dmabuf) 
    {
        printf("%s: Could not allocate DMA buffer\n", __func__);
        return -ENOMEM;
    }

    mtd = nand_to_mtd(nand_chip);
    nand_set_controller_data(nand_chip, nand_ctrl);

    /* Set the driver entry points for MTD */
    nand_chip->options = 0;
    nand_chip->cmdfunc = dubhe1000_nand_cmd_function;
    nand_chip->dev_ready = dubhe1000_nand_device_ready;
    nand_chip->select_chip = dubhe1000_nand_select_chip;

    /* Buffer read/write routines */
    nand_chip->read_byte = dubhe1000_nand_read_byte;
    nand_chip->read_buf = dubhe1000_nand_read_buf;
    nand_chip->write_buf = dubhe1000_nand_write_buf;

    //cancel bbt scan
    nand_chip->options |= NAND_SKIP_BBTSCAN;
    nand_chip->options |= NAND_SUBPAGE_READ;
    nand_chip->options &= ~NAND_NO_SUBPAGE_WRITE;
    nand_chip->bbt_options = NAND_BBT_USE_FLASH;

    t->t0_sync = dev_read_u32_default(dev, "t0_sync", TIMER_SEQ_0_VAL);
    t->t1_sync = dev_read_u32_default(dev, "t1_sync", TIMER_SEQ_1_VAL);
    t->t_async = dev_read_u32_default(dev, "t_async", TIMER_ASYN_VAL);

    /* Initialize the NAND flash interface on NAND controller */
    if (dubhe1000_nand_init_nand_flash(nand_ctrl, t) < 0)
    {
        printf("%s: nand flash init failed\n", __func__);
        return -EINVAL;
    }

    /* first scan to find the device and get the page size */
    if (nand_scan_ident(mtd, 1, NULL)) 
    {
        printf("%s: nand_scan_ident failed\n", __func__);
        return -EINVAL;
    }

    dubhe1000_nand_reg_write(nand_ctrl, ECC_OFFSET, mtd->writesize + ECC_OFFSET_DEFAULE_VAL);

    if(nand_chip->options & NAND_BUSWIDTH_16)
    {
        dubhe1000_nand_reg_write(nand_ctrl, DEV_CTRL_REG_OFFSET, 
            dubhe1000_nand_reg_read(nand_ctrl, DEV_CTRL_REG_OFFSET) | DEV_CTRL_WIDTH_16_VAL);
    }

    nand_chip->ecc.size = ECC_SIZE_DEFAULE_VAL;
    nand_chip->ecc.steps = mtd->writesize / nand_chip->ecc.size;
    nand_chip->ecc.mode = NAND_ECC_HW;
    nand_chip->ecc.hwctl = dubhe1000_nand_bch_hwctl;
    nand_chip->ecc.calculate = dubhe1000_nand_calculate_hwecc;
    nand_chip->ecc.correct = dubhe1000_nand_correct_data;

    /* refer to ECC_CAP, [2:0] of register ECC_CTRL
    000 - 4
    001 - 8
    010 - 16
    011 - 24
    100 - 32
    101 – 48
    */
    nand_chip->ecc.strength = 4;

    /* Second phase scan */
    if (nand_scan_tail(mtd)) {
        printf("%s: nand_scan_tail failed\n", __func__);
        return -EINVAL;
    }
    if (nand_register(0, mtd))
        return -EINVAL;

    return 0;
}

static const struct udevice_id dubhe1000_nand_dt_ids[] = {
    { .compatible = "arm,dubhe1000-nandc" },
    { }
};

U_BOOT_DRIVER(dubhe1000_nand) = {
    .name = "dubhe1000-nand",
    .id = UCLASS_MTD,
    .of_match = dubhe1000_nand_dt_ids,
    .probe = dubhe1000_nand_probe,
    .priv_auto    = sizeof(struct dubhe1000_nand_info),
};

void board_nand_init(void)
{
    struct udevice *dev;
    int ret;

    ret = uclass_get_device_by_driver(UCLASS_MTD,
                        DM_DRIVER_GET(dubhe1000_nand), &dev);
    if (ret && ret != -ENODEV)
        pr_err("Failed to initialize %s. (error %d)\n", dev->name, ret);
}
