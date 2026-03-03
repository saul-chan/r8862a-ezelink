/*
 *  Copyright (c) 2021-2024, Clourney Semiconductor. All rights reserved.
 *
 *  This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
 *  Reproduction and redistribution in binary or source form, with or without modification,
 *  for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
 *  must retain the above copyright notice.
 *
 *  By using this software and/or documentation, you agree to the limited terms and conditions.
 *
 *  Clourney Semiconductor Embedded NAND flash controller driver.
 *
 */

#include <linux/bitfield.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>
#include <linux/interrupt.h>
 #include <linux/iopoll.h>
#include <linux/slab.h>

#include <linux/module.h>
#include <linux/moduleparam.h>

#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/rawnand.h>
#include <linux/mtd/partitions.h>

#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#endif

#define MAX_ADDRESS_CYC		    5
#define COL_ADDRESS_CYC		    2
#define MAX_ERASE_ADDRESS_CYC	3

#define MAX_DATA_PAGE_SIZE	    SZ_2K
#define MAX_FIFO_SIZE		    512
#define DMA_DATA_SIZE_ALIGN	    8
#define NAND_WAIT_RB_TIMEBASE   60000
#define CLNY_ENFC_FIFO_MODE     0
#define CLNY_ENFC_SDMA_MODE     1

/* Register definition. */
/*
 * Command register
 * Writing data to this register will initiate a new transaction
 * of the NF controller.
 */
#define CLNY_NFC_CMD             0x0000
#define CMD_0_SHIFT              8u
#define CMD_1_3_SHIFT            16u
#define CMD_2_SHIFT              24u
#define CMD_SEQ_SHIFT            0u

#define CMD_2_CODE               GENMASK(31, 24)
#define CMD_1_3_CODE             GENMASK(23, 16)
#define CMD_0_CODE               GENMASK(15,  8)
#define CMD_SEQ_CODE             GENMASK( 5,  0)

/**
* 0 – the FIFO module selected
* 1 – the DATA register selected
*/
#define CMD_DATA_SEL_BIT         (7)
#define CMD_DATA_SEL_SIZE        (1)
#define CMD_DATA_SEL_FIFO        (0 << CMD_DATA_SEL_BIT)
#define CMD_DATA_SEL_DATA        (1 << CMD_DATA_SEL_BIT)
/**
* 0 – select the SIU module as input
* 1 – select the DMA module as input
*/
#define CMD_INPUT_SEL_BIT        (6)
#define CMD_INPUT_SEL_SIZE       (1)
#define CMD_INPUT_SEL_SIU        (0 << CMD_INPUT_SEL_BIT)
#define CMD_INPUT_SEL_DMA        (1 << CMD_INPUT_SEL_BIT)

#define MACRO_RECOMBINE(cmd_0, cmd_1_3, cmd_2, cmd_seq)    \
	(((u32)cmd_2<<CMD_2_SHIFT)|((u32)cmd_1_3<<CMD_1_3_SHIFT)|((u32)cmd_0<<CMD_0_SHIFT)|((u32)cmd_seq<<CMD_SEQ_SHIFT))

/* this follow command seqs from pnand spec 
 * ONFI 1.x and 2.2 standards. Command Sequence Encoding 
 */
#define CMD_SEQ_0     0x00u
#define CMD_SEQ_1     0x21u
#define CMD_SEQ_2     0x22u
#define CMD_SEQ_3     0x03u
#define CMD_SEQ_4     0x24u
#define CMD_SEQ_5     0x25u
#define CMD_SEQ_6     0x26u
#define CMD_SEQ_7     0x27u
#define CMD_SEQ_8     0x08u
#define CMD_SEQ_9     0x29u
#define CMD_SEQ_10    0x2au
#define CMD_SEQ_11    0x2bu
#define CMD_SEQ_12    0x0cu
#define CMD_SEQ_13    0x0du
#define CMD_SEQ_14    0x0eu
#define CMD_SEQ_15    0x2fu
#define CMD_SEQ_17    0x31u
#define CMD_SEQ_18    0x32u
#define CMD_SEQ_19    0x13u
#define CMD_SEQ_20    0x34u
#define CMD_SEQ_21    0x15u
#define CMD_SEQ_22    0x36u
#define CMD_SEQ_23    0x17u
#define CMD_SEQ_24    0x18u
#define CMD_SEQ_25    0x39u

/* Reset Commands */
#define RESET_DEV_CMD            MACRO_RECOMBINE(0xff, 0x00, 0x00, CMD_SEQ_0)

/* Identification Operations */
#define READ_ID_CMD              MACRO_RECOMBINE(0x90, 0x00, 0x00, CMD_SEQ_1)
#define READ_PARA_PAGE_CMD       MACRO_RECOMBINE(0xec, 0x00, 0x00, CMD_SEQ_2)
#define READ_UNIQUE_ID_CMD       MACRO_RECOMBINE(0xed, 0x00, 0x00, CMD_SEQ_2)

/* Configuration Operations */
#define GET_FEATURES_CMD         MACRO_RECOMBINE(0xee, 0x00, 0x00, CMD_SEQ_2)
#define SET_FEATURES_CMD         MACRO_RECOMBINE(0xef, 0x00, 0x00, CMD_SEQ_3)

/* Status Operations */
#define READ_STATUS_CMD          MACRO_RECOMBINE(0x70, 0x00, 0x00, CMD_SEQ_4)
#define READ_DEV_STA_CMD         MACRO_RECOMBINE(0x72, 0x00, 0x00, CMD_SEQ_4)

/* Program Operations */
#define PROG_PGAE_CMD            MACRO_RECOMBINE(0x80, 0x10, 0x00, CMD_SEQ_12)
#define PROG_PGAE_CACHE_CMD      MACRO_RECOMBINE(0x80, 0x15, 0x00, CMD_SEQ_12)
#define PROG_MULTIPLANE_CMD      MACRO_RECOMBINE(0x80, 0x11, 0x00, CMD_SEQ_12)
#define WRITE_PGAE_CMD           MACRO_RECOMBINE(0x10, 0x00, 0x00, CMD_SEQ_0)
#define WRITE_PGAE_CACHE_CMD     MACRO_RECOMBINE(0x15, 0x00, 0x00, CMD_SEQ_0)
#define WRITE_MULTIPLANE_CMD     MACRO_RECOMBINE(0x11, 0x00, 0x00, CMD_SEQ_0)

/* Read Operations */
#define READ_PAGE_CMD            MACRO_RECOMBINE(0x00, 0x00, 0x30, CMD_SEQ_10)
#define READ_PAGE_CACHE_CMD      MACRO_RECOMBINE(0x31, 0x00, 0x30, CMD_SEQ_10)
#define READ_MULTIPLANE_CMD      MACRO_RECOMBINE(0x00, 0x32, 0x00, CMD_SEQ_9)


/* Column Address Operations */
#define CHANGE_READ_COLUMN       MACRO_RECOMBINE(0x05, 0x00, 0xe0, CMD_SEQ_6)
#define CHANGE_WRITE_COLUMN      MACRO_RECOMBINE(0x85, 0x00, 0x00, CMD_SEQ_8)

/* Erase Operations */
#define ERASE_BLOCK_CMD          MACRO_RECOMBINE(0x60, 0xd0, 0x00, CMD_SEQ_14)
#define ERASE_MULTIPLANE_CMD     MACRO_RECOMBINE(0x60, 0xd1, 0x00, CMD_SEQ_14)

/* OTP Operations */
#define PROG_OTP_CMD             MACRO_RECOMBINE(0xa0, 0x10, 0x00, CMD_SEQ_12)
#define DATA_PROTECT_OTP_CMD     MACRO_RECOMBINE(0xa5, 0x10, 0x00, CMD_SEQ_9)
#define READ_PAGE_OTP_CMD        MACRO_RECOMBINE(0xaf, 0x00, 0x30, CMD_SEQ_10)


/*
 * Configurations register
 * Interrupt enable
 */
#define CLNY_NFC_CTRL       0x0004

/* Auto Read Status mode enable  */
#define	CTRL_ATUO_READ_STA_EN    BIT(23)
/* Multi LUN mode enable */
#define	CTRL_MULTI_LUN_EN        BIT(22)
/* Small block mode enable */
#define	CTRL_SMALL_BLOCK_EN      BIT(21)
/* Controller work mode */
#define	CTRL_WORK_MODE           GENMASK(20, 19)
/* asynchronous mode*/
#define	CTRL_WORK_MODE_ASYNC     0uL
/* source synchronous mode */
#define	CTRL_WORK_MODE_SSYNC     1uL
/* Toogle mode*/
#define	CTRL_WORK_MODE_TOOGLE    2uL
/* Clear NAND Enable */
#define	CTRL_CLRN_EN             BIT(18)
/* Address auto incrememnt for row address register1 enable */
#define	CTRL_ADDR1_AUTO_INCR_EN  BIT(17)
/* Address auto incrememnt for row address register0 enable */
#define	CTRL_ADDR0_AUTO_INCR_EN  BIT(16)
/* Protect mechanism enable */
#define	CTRL_PROT_EN             BIT(14)
/* Bad Block Management enable */
#define	CTRL_BBM_EN              BIT(13)
/* Nand Flash I/O width */
#define	CTRL_IO_WIDTH_16BIT      BIT(12)
/* Device Stacking enable */
#define	CTRL_DEV_STACK_EN        BIT(11)
/* The Block Size */
#define	CTRL_BLOCK_SIZE          GENMASK(7, 6)
/* 32 pages per block */
#define	CTRL_BLOCK_32P           0uL
/* 64 pages per block */
#define	CTRL_BLOCK_64P           1uL
/* 128 pages per block */
#define	CTRL_BLOCK_128P          2uL
/* 256 pages per block */
#define	CTRL_BLOCK_256P          3uL
/* Hardware ECC Support enable */
#define	CTRL_HW_ECC_EN           BIT(5)
/* Global Interrupt enable */
#define	CTRL_GBL_INT_EN          BIT(4)
/* Retry response enable */
#define	CTRL_RETRY_RSP_EN        BIT(3)
/* The ECC Block Size */
#define	CTRL_ECC_BLOCK_SIZE      GENMASK(2, 1)
/* 256 bytes */
#define	CTRL_ECC_BLK_256B        0uL
/* 512 bytes */
#define	CTRL_ECC_BLK_512B        1uL
/* 1024 bytes */
#define	CTRL_ECC_BLK_1024B       2uL
/* Automatically READ STATUS / Check RNB Lines */
#define	CTRL_READ_STATUS_EN      BIT(0)

/*
 * The Controller status register
 * Interrupt enable
 */
#define CLNY_NFC_STATUS			 0x0008
#define	DATA_REG_ST_AVAILABLE	 BIT(10)
#define	DATASIZE_ST_INCORRECT  	 BIT(9)
#define	CTRL_STATUS_IS_BUSY  	 BIT(8)
#define	MEM7_STATUS_IS_READY  	 BIT(7)
#define	MEM6_STATUS_IS_READY  	 BIT(6)
#define	MEM5_STATUS_IS_READY  	 BIT(5)
#define	MEM4_STATUS_IS_READY  	 BIT(4)
#define	MEM3_STATUS_IS_READY  	 BIT(3)
#define	MEM2_STATUS_IS_READY  	 BIT(2)
#define	MEM1_STATUS_IS_READY  	 BIT(1)
#define	MEM0_STATUS_IS_READY  	 BIT(0)

/*
 * The Controller status mask register
 * Interrupt enable
 */
#define CLNY_NFC_STATUS_MASK     0x000C


/*
 * The interrupt mask register
 *
 */
#define CLNY_NFC_INTR_MASK       0x0010
#define	NFC_ECC_INT7_EN        	 BIT(31)
#define	NFC_ECC_INT6_EN        	 BIT(30)
#define	NFC_ECC_INT5_EN        	 BIT(29)
#define	NFC_ECC_INT4_EN        	 BIT(28)
#define	NFC_ECC_INT3_EN        	 BIT(27)
#define	NFC_ECC_INT2_EN        	 BIT(26)
#define	NFC_ECC_INT1_EN        	 BIT(25)
#define	NFC_ECC_INT0_EN        	 BIT(24)
/* memory operation fail */
#define	NFC_ERR_INT7_EN        	 BIT(23)
#define	NFC_ERR_INT6_EN        	 BIT(22)
#define	NFC_ERR_INT5_EN        	 BIT(21)
#define	NFC_ERR_INT4_EN        	 BIT(20)
#define	NFC_ERR_INT3_EN        	 BIT(19)
#define	NFC_ERR_INT2_EN        	 BIT(18)
#define	NFC_ERR_INT1_EN        	 BIT(17)
#define	NFC_ERR_INT0_EN        	 BIT(16)
/* memory device ready */
#define	NFC_MEM7_RDY_INT_EN    	 BIT(15)
#define	NFC_MEM6_RDY_INT_EN    	 BIT(14)
#define	NFC_MEM5_RDY_INT_EN    	 BIT(13)
#define	NFC_MEM4_RDY_INT_EN    	 BIT(12)
#define	NFC_MEM3_RDY_INT_EN    	 BIT(11)
#define	NFC_MEM2_RDY_INT_EN    	 BIT(10)
#define	NFC_MEM1_RDY_INT_EN    	 BIT(9)
#define	NFC_MEM0_RDY_INT_EN    	 BIT(8)

/* Data Size error occur */
#define	NFC_PG_SZ_ERR_INT_EN   	 BIT(6)
/* Super Sequence finished */
#define	NFC_SS_READY_INT_EN   	 BIT(5)
/* transfer on the slave interface error */
#define	NFC_TRANS_ERR_INT_EN   	 BIT(4)
/* DMA Transfer ended */
#define	NFC_DMA_INT_EN      	 BIT(3)
/* Data in DATA_REG is available */
#define	NFC_DATA_REG_INT_EN      BIT(2)
/* Command Sequence ended */
#define	NFC_CMD_END_INT_EN       BIT(1)
/* Erase/Write Protected area interrupt enable */
#define	NFC_PROT_INT_EN          BIT(0)

#define	NFC_INTR_DISABLE    	 0x00000000uL

/*
 * The interrupt status register
 *
 */
#define CLNY_NFC_INTR_STATUS     0x0014

#define	INTR_STA_ECC_INT7_FL	 BIT(31)
#define	INTR_STA_ECC_INT6_FL	 BIT(30)
#define	INTR_STA_ECC_INT5_FL	 BIT(29)
#define	INTR_STA_ECC_INT4_FL	 BIT(28)
#define	INTR_STA_ECC_INT3_FL	 BIT(27)
#define	INTR_STA_ECC_INT2_FL	 BIT(26)
#define	INTR_STA_ECC_INT1_FL	 BIT(25)
#define	INTR_STA_ECC_INT0_FL	 BIT(24)

#define	INTR_STA_ERR_INT7_FL	 BIT(23)
#define	INTR_STA_ERR_INT6_FL	 BIT(22)
#define	INTR_STA_ERR_INT5_FL	 BIT(21)
#define	INTR_STA_ERR_INT4_FL	 BIT(20)
#define	INTR_STA_ERR_INT3_FL	 BIT(19)
#define	INTR_STA_ERR_INT2_FL	 BIT(18)
#define	INTR_STA_ERR_INT1_FL	 BIT(17)
#define	INTR_STA_ERR_INT0_FL	 BIT(16)

#define	INTR_STA_MEM7_RDY_INT_FL BIT(15)
#define	INTR_STA_MEM6_RDY_INT_FL BIT(14)
#define	INTR_STA_MEM5_RDY_INT_FL BIT(13)
#define	INTR_STA_MEM4_RDY_INT_FL BIT(12)
#define	INTR_STA_MEM3_RDY_INT_FL BIT(11)
#define	INTR_STA_MEM2_RDY_INT_FL BIT(10)
#define	INTR_STA_MEM1_RDY_INT_FL BIT(9)
#define	INTR_STA_MEM0_RDY_INT_FL BIT(8)

/* Data Size error occur flag */
#define	INTR_STA_PG_SZ_ERR_INT_FL BIT(6)
/* Super Sequence finished flag */
#define	INTR_STA_SS_READY_INT_FL  BIT(5)
/* transfer on the slave interface error flag */
#define	INTR_STA_TRANS_ERR_INT_FL BIT(4)
/* DMA Transfer ended flag*/
#define	INTR_STA_DMA_INT_FL       BIT(3)
/* Data in DATA_REG is available flag */
#define	INTR_STA_DATA_REG_INT_FL  BIT(2)
/* Command Sequence ended flag */
#define	INTR_STA_CMD_END_INT_FL   BIT(1)
/* Erase/Write Protected area interrupt flag */
#define	INTR_STA_PROT_INT_FL      BIT(0)
#define	INTR_STA_CLEAR            0x00000000uL

/* Timings configuration. */
#define NFC_TIMINGS_ASYNC         0x0088
#define	TIMINGS_ASYNC_TRWH	      GENMASK(7, 4)
#define	TIMINGS_ASYNC_TRWP	      GENMASK(3, 0)

#define NFC_TIMINGS_SYNC          0x008C
#define	TIMINGS_SYNC_TCAD	      GENMASK(3, 0)

#define	NFC_TIMING_SEQ0			  0x0090
#define	NFC_TIMING_SEQ0_TWHR      GENMASK(29, 24)
#define	NFC_TIMING_SEQ0_TRHW	  GENMASK(21, 16)
#define	NFC_TIMING_SEQ0_TADL	  GENMASK(13,  8)
#define	NFC_TIMING_SEQ0_TCCS	  GENMASK( 5,  0)

#define	NFC_TIMING_SEQ1			  0x0094
/* Read High to Read low */
#define	TIMING_SEQ1_TRR			  GENMASK(13,  8)
#define	TIMING_SEQ1_TWB			  GENMASK( 5,  0)

#define	NFC_TIMING_GEN_SEQ0		  0x0098
#define	TIMING_GEN_SEQ0_t0_d3	  GENMASK(29, 24)
#define	TIMING_GEN_SEQ0_t0_d2	  GENMASK(21, 16)
#define	TIMING_GEN_SEQ0_t0_d1	  GENMASK(13,  8)
#define	TIMING_GEN_SEQ0_t0_d0	  GENMASK( 5,  0)

#define	NFC_TIMING_GEN_SEQ1		  0x009c
#define	TIMING_GEN_SEQ1_t0_d7	  GENMASK(29, 24)
#define	TIMING_GEN_SEQ1_t0_d6	  GENMASK(21, 16)
#define	TIMING_GEN_SEQ1_t0_d5	  GENMASK(13,  8)
#define	TIMING_GEN_SEQ1_t0_d4	  GENMASK( 5,  0)

#define	NFC_TIMING_GEN_SEQ2		  0x00A0
#define	TIMING_GEN_SEQ2_t0_d11    GENMASK(29, 24)
#define	TIMING_GEN_SEQ2_t0_d10    GENMASK(21, 16)
#define	TIMING_GEN_SEQ2_t0_d9	  GENMASK(13,  8)
#define	TIMING_GEN_SEQ2_t0_d8	  GENMASK( 5,  0)

#define	NFC_TIMING_GEN_SEQ3		  0x0134
#define	TIMING_GEN_SEQ2_t33_d12   GENMASK( 5,  0)

#define NFC_TIMINGS_TOGGLE        0x0130
#define	TIMINGS_TOGGLE_TPST	      GENMASK(7, 4)
#define	TIMINGS_TOGGLE_TPRE	      GENMASK(3, 0)

/* ECC engine configuration register */
#define	NFC_ECC_CTRL              0x0018
#define	ECC_CTRL_SEL    	      GENMASK(17, 16)
#define	ECC_CTRL_ERR_THRESHOLD    GENMASK(13,  8)
#define	ECC_CTRL_CAP    	      GENMASK( 2,  0)


#define	NFC_ECC_OFFSET            0x001c

#define	NFC_ECC_STAT              0x0020
#define	ECC_STAT_OVER_7           BIT(23)
#define	ECC_STAT_OVER_6           BIT(22)
#define	ECC_STAT_OVER_5           BIT(21)
#define	ECC_STAT_OVER_4           BIT(20)
#define	ECC_STAT_OVER_3           BIT(19)
#define	ECC_STAT_OVER_2           BIT(18)
#define	ECC_STAT_OVER_1           BIT(17)
#define	ECC_STAT_OVER_0           BIT(16)

#define	ECC_STAT_UNC_7            BIT(15)
#define	ECC_STAT_UNC_6            BIT(14)
#define	ECC_STAT_UNC_5            BIT(13)
#define	ECC_STAT_UNC_4            BIT(12)
#define	ECC_STAT_UNC_3            BIT(11)
#define	ECC_STAT_UNC_2            BIT(10)
#define	ECC_STAT_UNC_1            BIT(9)
#define	ECC_STAT_UNC_0            BIT(8)

#define	ECC_STAT_ERROR_7          BIT(7)
#define	ECC_STAT_ERROR_6          BIT(6)
#define	ECC_STAT_ERROR_5          BIT(5)
#define	ECC_STAT_ERROR_4          BIT(4)
#define	ECC_STAT_ERROR_3          BIT(3)
#define	ECC_STAT_ERROR_2          BIT(2)
#define	ECC_STAT_ERROR_1          BIT(1)
#define	ECC_STAT_ERROR_0          BIT(0)

/**
 * A15-A0 address bits.
 */
#define	NFC_ADDR0_COL_REG         0x0024
#define	NFC_ADDR1_COL_REG         0x002C

/************ Page address Register ************/

/* !<A39-A16 address bits. */
#define	NFC_ADDR0_ROW_REG         0x0028
#define	NFC_ADDR1_ROW_REG         0x0030

#define NFC_FIFO_DATA_REG         0x0038
#define NFC_READ_DATA_REG         0x003C

/* data register size */
#define NFC_DATA_REG_SIZE_REG     0x0040

#define NFC_FLASH_STATUS_REG      0x4040

#define NFC_DATA_SIZE_REG         0x0084

/* Memory Control register */
#define NFC_MEM_CTRL_REG          0x0080
#define	MEM_CTRL_BANK_SEL         GENMASK(18, 16)
#define	MEM_CTRL_MEMn_WP          GENMASK(15, 8)
#define	MEM_CTRL_MEM7_WP          BIT(15)
#define	MEM_CTRL_MEM6_WP          BIT(14)
#define	MEM_CTRL_MEM5_WP          BIT(13)
#define	MEM_CTRL_MEM4_WP          BIT(12)
#define	MEM_CTRL_MEM3_WP          BIT(11)
#define	MEM_CTRL_MEM2_WP          BIT(10)
#define	MEM_CTRL_MEM1_WP          BIT(9)
#define	MEM_CTRL_MEM0_WP          BIT(8)
#define	MEM_CTRL_MEM_CE           GENMASK(2, 0)

/* Interrupt status register. */
#define INTR_STATUS			0x0110
#define		INTR_STATUS_SDMA_ERR	BIT(22)
#define		INTR_STATUS_SDMA_TRIGG	BIT(21)
#define		INTR_STATUS_UNSUPP_CMD	BIT(19)
#define		INTR_STATUS_DDMA_TERR	BIT(18)
#define		INTR_STATUS_CDMA_TERR	BIT(17)
#define		INTR_STATUS_CDMA_IDL	BIT(16)

/* FIFO Initital Register */
#define NFC_FIFO_INIT_REG           0x00B0

/* FIFO STATUS Register */
#define NFC_FIFO_STATUS_REG         0x00B4
#define	DF_STATUS_W_EMPTY  	        BIT(7)
#define	DF_STATUS_R_FULL  	        BIT(6)

#define	DF_STATUS_W_FULL  	        BIT(1)
#define	DF_STATUS_R_EMPTY  	        BIT(0)

/* LUN STATUS Register */
#define NFC_LUN_STATUS0_REG         0x0128
#define NFC_LUN_STATUS1_REG         0x012C

/* INTERNAL STATUS Register */
#define NFC_INTERNAL_STATUS_REG     0x0148
#define NFC_PARAM_REG               0x0150

/* ECC engine configuration register 0. */
#define ECC_CONFIG_0				0x0428
/* Correction strength. */
#define		ECC_CONFIG_0_CORR_STR		GENMASK(10, 8)
/* Enable erased pages detection mechanism. */
#define		ECC_CONFIG_0_ERASE_DET_EN	BIT(1)
/* Enable controller ECC check bits generation and correction. */
#define		ECC_CONFIG_0_ECC_EN		BIT(0)

/* ECC engine configuration register 1. */
#define ECC_CONFIG_1				0x042C

/* Multiplane settings register. */
#define MULTIPLANE_CFG				0x0434
/* Cache operation settings. */
#define CACHE_CFG				0x0438

/* DMA settings register. */
#define DMA_SETINGS				0x043C
/* Enable SDMA error report on access unprepared slave DMA interface. */
#define		DMA_SETINGS_SDMA_ERR_RSP	BIT(17)

/* Transferred data block size for the slave DMA module. */
#define SDMA_SIZE				0x0440

/* Thread number associated with transferred data block
 * for the slave DMA module.
 */
#define SDMA_TRD_NUM				0x0444
/* Thread number mask. */
#define		SDMA_TRD_NUM_SDMA_TRD		GENMASK(2, 0)

#define CONTROL_DATA_CTRL			0x0494
/* Thread number mask. */
#define		CONTROL_DATA_CTRL_SIZE		GENMASK(15, 0)

#define CTRL_VERSION				0x800
#define		CTRL_VERSION_REV		GENMASK(7, 0)

/* Available hardware features of the controller. */
#define CTRL_FEATURES				0x804
/* Support for NV-DDR2/3 work mode. */
#define		CTRL_FEATURES_NVDDR_2_3		BIT(28)
/* Support for NV-DDR work mode. */
#define		CTRL_FEATURES_NVDDR		BIT(27)
/* Support for asynchronous work mode. */
#define		CTRL_FEATURES_ASYNC		BIT(26)
/* Support for asynchronous work mode. */
#define		CTRL_FEATURES_N_BANKS		GENMASK(25, 24)
/* Slave and Master DMA data width. */
#define		CTRL_FEATURES_DMA_DWITH64	BIT(21)
/* Availability of Control Data feature.*/
#define		CTRL_FEATURES_CONTROL_DATA	BIT(10)

/* BCH Engine identification register 0 - correction strengths. */
#define BCH_CFG_0				0x838
#define		BCH_CFG_0_CORR_CAP_0		GENMASK(7, 0)
#define		BCH_CFG_0_CORR_CAP_1		GENMASK(15, 8)
#define		BCH_CFG_0_CORR_CAP_2		GENMASK(23, 16)
#define		BCH_CFG_0_CORR_CAP_3		GENMASK(31, 24)

/* BCH Engine identification register 1 - correction strengths. */
#define BCH_CFG_1				0x83C
#define		BCH_CFG_1_CORR_CAP_4		GENMASK(7, 0)
#define		BCH_CFG_1_CORR_CAP_5		GENMASK(15, 8)
#define		BCH_CFG_1_CORR_CAP_6		GENMASK(23, 16)
#define		BCH_CFG_1_CORR_CAP_7		GENMASK(31, 24)

/* BCH Engine identification register 2 - sector sizes. */
#define BCH_CFG_2				0x840
#define		BCH_CFG_2_SECT_0		GENMASK(15, 0)
#define		BCH_CFG_2_SECT_1		GENMASK(31, 16)

/* BCH Engine identification register 3. */
#define BCH_CFG_3				0x844
#define		BCH_CFG_3_METADATA_SIZE		GENMASK(23, 16)

/* Ready/Busy# line status. */
#define RBN_SETINGS				0x1004

/* Common settings. */
#define COMMON_SET				0x1008
/* 16 bit device connected to the NAND Flash interface. */
#define		COMMON_SET_DEVICE_16BIT		BIT(8)

/* Skip_bytes registers. */
#define SKIP_BYTES_CONF				0x100C
#define		SKIP_BYTES_MARKER_VALUE		GENMASK(31, 16)
#define		SKIP_BYTES_NUM_OF_BYTES		GENMASK(7, 0)

#define SKIP_BYTES_OFFSET			0x1010
#define		 SKIP_BYTES_OFFSET_VALUE	GENMASK(23, 0)

/* Generic command layout. */
#define GCMD_LAY_CS			GENMASK_ULL(11, 8)
/*
 * This bit informs the minicotroller if it has to wait for tWB
 * after sending the last CMD/ADDR/DATA in the sequence.
 */
#define GCMD_LAY_TWB			BIT_ULL(6)
/* Type of generic instruction. */
#define GCMD_LAY_INSTR			GENMASK_ULL(5, 0)

/* Generic CMD sequence type. */
#define		GCMD_LAY_INSTR_CMD	0
/* Generic ADDR sequence type. */
#define		GCMD_LAY_INSTR_ADDR	1
/* Generic data transfer sequence type. */
#define		GCMD_LAY_INSTR_DATA	2

/* Input part of generic command type of input is command. */
#define GCMD_LAY_INPUT_CMD		GENMASK_ULL(23, 16)

/* Generic command address sequence - address fields. */
#define GCMD_LAY_INPUT_ADDR		GENMASK_ULL(63, 16)
/* Generic command address sequence - address size. */
#define GCMD_LAY_INPUT_ADDR_SIZE	GENMASK_ULL(13, 11)

/* Transfer direction field of generic command data sequence. */
#define GCMD_DIR			BIT_ULL(11)
/* Read transfer direction of generic command data sequence. */
#define		GCMD_DIR_READ		0
/* Write transfer direction of generic command data sequence. */
#define		GCMD_DIR_WRITE		1

/* ECC enabled flag of generic command data sequence - ECC enabled. */
#define GCMD_ECC_EN			BIT_ULL(12)
/* Generic command data sequence - sector size. */
#define GCMD_SECT_SIZE			GENMASK_ULL(31, 16)
/* Generic command data sequence - sector count. */
#define GCMD_SECT_CNT			GENMASK_ULL(39, 32)
/* Generic command data sequence - last sector size. */
#define GCMD_LAST_SIZE			GENMASK_ULL(55, 40)

/* CDMA descriptor fields. */
/* Erase command type of CDMA descriptor. */
#define CDMA_CT_ERASE		0x1000
/* Program page command type of CDMA descriptor. */
#define CDMA_CT_WR		0x2100
/* Read page command type of CDMA descriptor. */
#define CDMA_CT_RD		0x2200

/* Flash pointer memory shift. */
#define CDMA_CFPTR_MEM_SHIFT	24
/* Flash pointer memory mask. */
#define CDMA_CFPTR_MEM		GENMASK(26, 24)

/*
 * Command DMA descriptor flags. If set causes issue interrupt after
 * the completion of descriptor processing.
 */
#define CDMA_CF_INT		BIT(8)
/*
 * Command DMA descriptor flags - the next descriptor
 * address field is valid and descriptor processing should continue.
 */
#define CDMA_CF_CONT		BIT(9)
/* DMA master flag of command DMA descriptor. */
#define CDMA_CF_DMA_MASTER	BIT(10)

/* Operation complete status of command descriptor. */
#define CDMA_CS_COMP		BIT(15)
/* Operation complete status of command descriptor. */
/* Command descriptor status - operation fail. */
#define CDMA_CS_FAIL		BIT(14)
/* Command descriptor status - page erased. */
#define CDMA_CS_ERP		BIT(11)
/* Command descriptor status - timeout occurred. */
#define CDMA_CS_TOUT		BIT(10)
/*
 * Maximum amount of correction applied to one ECC sector.
 * It is part of command descriptor status.
 */
#define CDMA_CS_MAXERR		GENMASK(9, 2)
/* Command descriptor status - uncorrectable ECC error. */
#define CDMA_CS_UNCE		BIT(1)
/* Command descriptor status - descriptor error. */
#define CDMA_CS_ERR		BIT(0)

/* Status of operation - OK. */
#define STAT_OK			0
/* Status of operation - FAIL. */
#define STAT_FAIL		2
/* Status of operation - uncorrectable ECC error. */
#define STAT_ECC_UNCORR		3
/* Status of operation - page erased. */
#define STAT_ERASED		5
/* Status of operation - correctable ECC error. */
#define STAT_ECC_CORR		6
/* Status of operation - unsuspected state. */
#define STAT_UNKNOWN		7
/* Status of operation - operation is not completed yet. */
#define STAT_BUSY		0xFF

#define BCH_MAX_NUM_CORR_CAPS		6
#define BCH_MAX_NUM_SECTOR_SIZES	2

#define NANOSECONDS_CYCLE_TICK  1000000000
#define NANDCTROLLER_TIMING_NUM         8

#define tWHR_BIT                24
#define tWHR_MASK               (0x3F << tWHR_BIT)
#define tRHW_BIT                16
#define tRHW_MASK               (0x3F << tRHW_BIT)
#define tADL_BIT                8
#define tADL_MASK               (0x3F << tADL_BIT)
#define tCCS_BIT                0
#define tCCS_MASK               (0x3F << tCCS_BIT)
#define tRR_BIT                 8
#define tRR_MASK                (0x3F << tRR_BIT)
#define tWB_BIT                 0
#define tWB_MASK                (0x3F << tWB_BIT)
#define tRWH_BIT                4
#define tRWH_MASK               (0x0F << tRWH_BIT)
#define tRWP_BIT                0
#define tRWP_MASK               (0x0F << tRWP_BIT)

// tWHR 15 tRHW 25 tADL 25 tCCS 3
#define TIMER_SEQ_0_VAL         0x3F3F3F3F  //tWHR bit29-24 tRHW bit21-16 tADL bit13-8 tCCS bit5-0
// tRR 5 tWB 25
#define TIMER_SEQ_1_VAL         0x00003F3F  //tRR bit13-8  tWB bit5-0
// tRWH 4 tRWP 7
#define TIMER_ASYN_VAL          0x000000FF  //tRWH bit7-4  tRWP bit3-0

struct clourney_enfc_timings {
	u32 tWHR;
	u32 tRHW;
	u32 tADL;
	u32 tCCS;
	u32 tRR;
	u32 tWB;
	u32 tRWH;
	u32 tRWP;
};

struct clourney_enfc_op {
	u32 addrx_col;
	u32 addrx_row;
	u32 cmd_reg;
	unsigned int rdy_timeout_ms;
	unsigned int len;
	u8  opcode;
	bool skip_adds_op;
	bool read;
	bool need_cmd_wait;
	bool need_data_wait;
	u8 *buf;
};

/* Interrupt status. */
struct clourney_enfc_irq_status {
	/* Thread operation complete status. */
	u32 trd_status;
	/* Thread operation error. */
	u32 trd_error;
	/* Controller status. */
	u32 status;
};

/* Cadence NAND flash controller capabilities read from registers. */
struct clourney_enfc_caps {
	/* Maximum number of banks supported by hardware. */
	u8 max_banks;
	/* Slave and Master DMA data width in bytes (4 or 8). */
	u8 data_dma_width;
	/* Control Data feature supported. */
	bool data_control_supp;
	u32 is_phy_type_dll;
};

struct clourney_enfc {
	struct device *dev;
	struct nand_controller controller;
	/* IP capability. */
	struct clourney_enfc_caps caps2;
	/* Timings Parameter*/
	struct clourney_enfc_timings timings;
	u8 ctrl_rev;
	dma_addr_t dma_cdma_desc;
	u8 *buf;
	u32 buf_size;
	u8 curr_corr_str_idx;

	/* Register interface. */
	void __iomem *reg;

	struct {
		void __iomem *virt;
		dma_addr_t dma;
	} io;

	bool sdma_enable;
	u32 clk;
	u32 clk_cycle;   // cycle unit is ns
	int irq;
	/* Interrupts that have happened. */
	struct clourney_enfc_irq_status irq_status;
	/* Interrupts we are waiting for. */
	struct clourney_enfc_irq_status irq_mask;
	struct completion complete;
	/* Protect irq_mask and irq_status. */
	spinlock_t irq_lock;

	int ecc_strengths[BCH_MAX_NUM_CORR_CAPS];
	struct nand_ecc_step_info ecc_stepinfos[BCH_MAX_NUM_SECTOR_SIZES];
	struct nand_ecc_caps ecc_caps;

	struct dma_chan *dmac;

	u32 nf_clk_rate;

	struct nand_chip *selected_chip;

	unsigned long assigned_cs;
	struct list_head chips;
	u8 bch_metadata_size;

#ifdef CONFIG_DEBUG_FS
	struct dentry *debugfs;
	struct debugfs_regset32 regset;
#endif

};

struct clourney_enfc_chip {
	struct nand_chip chip;
	u8 nsels;
	struct list_head node;

	/*
	 * part of oob area of NAND flash memory page.
	 * This part is available for user to read or write.
	 */
	u32 avail_oob_size;

	/* Sector size. There are few sectors per mtd->writesize */
	u32 sector_size;
	u32 sector_count;

	/* Offset of BBM. */
	u8 bbm_offs;
	/* Number of bytes reserved for BBM. */
	u8 bbm_len;
	/* ECC strength index. */
	u8 corr_str_idx;

	u8 cs[];
};

struct ecc_info {
	int (*calc_ecc_bytes)(int step_size, int strength);
	int max_step_size;
};

static inline struct
clourney_enfc_chip *to_clourney_enfc_chip(struct nand_chip *chip)
{
	return container_of(chip, struct clourney_enfc_chip, chip);
}

static inline struct
clourney_enfc *to_clourney_enfc(struct nand_controller *controller)
{
	return container_of(controller, struct clourney_enfc, controller);
}

static bool
clourney_enfc_dma_buf_ok(struct clourney_enfc *clny_ctrl, const void *buf,
			u32 buf_len)
{
	u8 data_dma_width = clny_ctrl->caps2.data_dma_width;

	return buf && virt_addr_valid(buf) &&
		likely(IS_ALIGNED((uintptr_t)buf, data_dma_width)) &&
		likely(IS_ALIGNED(buf_len, DMA_DATA_SIZE_ALIGN));
}


#ifdef CONFIG_DEBUG_FS

#define CLNY_ENFC_DBGFS_REG(_name, _off)	\
{					\
	.name = _name,			\
	.offset = _off,			\
}

static const struct debugfs_reg32 clny_enfc_dbgfs_regs[] = {
	CLNY_ENFC_DBGFS_REG("COMMAND", CLNY_NFC_CMD),
	CLNY_ENFC_DBGFS_REG("CONTROL", CLNY_NFC_CTRL),
	CLNY_ENFC_DBGFS_REG("STATUS", CLNY_NFC_STATUS),
	CLNY_ENFC_DBGFS_REG("STATUS_MASK", CLNY_NFC_STATUS_MASK),
	CLNY_ENFC_DBGFS_REG("INT_MASK", CLNY_NFC_INTR_MASK),
	CLNY_ENFC_DBGFS_REG("INT_STATUS", CLNY_NFC_INTR_STATUS),
	CLNY_ENFC_DBGFS_REG("ADDR0_COL", NFC_ADDR0_COL_REG),
	CLNY_ENFC_DBGFS_REG("ADDR1_COL", NFC_ADDR1_COL_REG),
	CLNY_ENFC_DBGFS_REG("ADDR0_ROW", NFC_ADDR0_ROW_REG),
	CLNY_ENFC_DBGFS_REG("ADDR1_ROW", NFC_ADDR1_ROW_REG),
	CLNY_ENFC_DBGFS_REG("DATA_REG", NFC_READ_DATA_REG),
	CLNY_ENFC_DBGFS_REG("DATA_REG_SIZE", NFC_DATA_REG_SIZE_REG),
	CLNY_ENFC_DBGFS_REG("MEM_CTRL", NFC_MEM_CTRL_REG),
	CLNY_ENFC_DBGFS_REG("DATA_SIZE", NFC_DATA_SIZE_REG),
	CLNY_ENFC_DBGFS_REG("TIMINGS_ASYN", NFC_TIMINGS_ASYNC),
	CLNY_ENFC_DBGFS_REG("TIMINGS_SYN", NFC_TIMINGS_SYNC),
	CLNY_ENFC_DBGFS_REG("TIME_SEQ_0", NFC_TIMING_SEQ0),
	CLNY_ENFC_DBGFS_REG("TIME_SEQ_1", NFC_TIMING_SEQ1),
	CLNY_ENFC_DBGFS_REG("FIFO_INIT", NFC_FIFO_INIT_REG),
	CLNY_ENFC_DBGFS_REG("FIFO_STATE", NFC_FIFO_STATUS_REG),
	CLNY_ENFC_DBGFS_REG("INTERNAL_STATUS", NFC_INTERNAL_STATUS_REG),
	CLNY_ENFC_DBGFS_REG("PARAM", NFC_PARAM_REG),
};

static int clny_enfc_debugfs_init(struct clourney_enfc *clny_ctrl)
{
	char name[32];

	snprintf(name, 32, "clny_enfc%d", clny_ctrl->selected_chip->cur_cs);
	clny_ctrl->debugfs = debugfs_create_dir(name, NULL);
	if (!clny_ctrl->debugfs)
		return -ENOMEM;

	clny_ctrl->regset.regs = clny_enfc_dbgfs_regs;
	clny_ctrl->regset.nregs = ARRAY_SIZE(clny_enfc_dbgfs_regs);
	clny_ctrl->regset.base = clny_ctrl->reg;
	debugfs_create_regset32("registers", 0400, clny_ctrl->debugfs, &clny_ctrl->regset);

	return 0;
}

static void clny_enfc_debugfs_remove(struct clourney_enfc *clny_ctrl)
{
	debugfs_remove_recursive(clny_ctrl->debugfs);
}

#else
static inline int clny_enfc_debugfs_init(struct clourney_enfc *clny_ctrl)
{
	return 0;
}

static inline void dw_spi_debugfs_remove(struct clourney_enfc *clny_ctrl)
{
}
#endif /* CONFIG_DEBUG_FS */



static u32 clourney_enfc_generic_cmd_parse(u8 cmd_opcode)
{
	u32 cmd_regval;

	switch(cmd_opcode)
	{
		case NAND_CMD_ERASE1:
		case NAND_CMD_ERASE2:
			cmd_regval = ERASE_BLOCK_CMD;
		break;

		case NAND_CMD_PARAM:
			cmd_regval = READ_PARA_PAGE_CMD;
		break;

		case NAND_CMD_READ0:
		case NAND_CMD_READOOB:
			cmd_regval = READ_PAGE_CMD;
		break;

		case NAND_CMD_RNDOUT:
			cmd_regval = CHANGE_READ_COLUMN;
		break;

		case NAND_CMD_STATUS:
			cmd_regval = READ_STATUS_CMD;
		break;

		case NAND_CMD_SEQIN:
			cmd_regval = PROG_PGAE_CMD;
		break;

		case NAND_CMD_PAGEPROG:
			cmd_regval = WRITE_PGAE_CMD;
		break;

		case NAND_CMD_RNDIN:
			cmd_regval = CHANGE_WRITE_COLUMN;
		break;

		case NAND_CMD_READID:
			cmd_regval = READ_ID_CMD;
		break;

		case NAND_CMD_GET_FEATURES:
			cmd_regval = GET_FEATURES_CMD;
		break;

		case NAND_CMD_SET_FEATURES:
			cmd_regval = SET_FEATURES_CMD;
		break;

		case NAND_CMD_RESET:
			cmd_regval = RESET_DEV_CMD;
		break;

		default:
			cmd_regval = NAND_CMD_RESET;
			pr_info("%s:error cmd_opcode = %x cmd_regval = %x\r\n",__func__,cmd_opcode,cmd_regval);
		break;
	}

	return cmd_regval;
}

static int clourney_enfc_wait_for_value(struct clourney_enfc *clny_ctrl,
				       u32 reg_offset, u32 timeout_us,
				       u32 mask, bool is_clear)
{
	u32 val;
	int ret;

	ret = readl_relaxed_poll_timeout(clny_ctrl->reg + reg_offset,
					 val, !(val & mask) == is_clear,
					 10, timeout_us);

	#if 0
	if (ret < 0) {
		dev_err(clny_ctrl->dev,
			"Timeout while waiting for reg %x with mask %x is clear %d\n",
			reg_offset, mask, is_clear);
	}
	#endif

	return ret;
}

static int clourney_enfc_set_access_width16(struct clourney_enfc *clny_ctrl,
					   bool bit_bus16)
{
	u32 reg;

	if (clourney_enfc_wait_for_value(clny_ctrl, CLNY_NFC_STATUS,
					NAND_WAIT_RB_TIMEBASE,
					CTRL_STATUS_IS_BUSY, true))
		return -ETIMEDOUT;

	reg = readl_relaxed(clny_ctrl->reg + CLNY_NFC_CTRL);

	if (!bit_bus16)
		reg &= ~CTRL_IO_WIDTH_16BIT;
	else
		reg |= CTRL_IO_WIDTH_16BIT;
	writel_relaxed(reg, clny_ctrl->reg + CLNY_NFC_CTRL);

	return 0;
}

static void
clourney_enfc_clear_interrupt(struct clourney_enfc *clny_ctrl,
			     struct clourney_enfc_irq_status *irq_status)
{
	writel_relaxed(INTR_STA_CLEAR, clny_ctrl->reg + CLNY_NFC_INTR_STATUS);
}

static void
clourney_enfc_read_int_status(struct clourney_enfc *clny_ctrl,
			     struct clourney_enfc_irq_status *irq_status)
{
	irq_status->status = readl_relaxed(clny_ctrl->reg + CLNY_NFC_INTR_STATUS);
}

static u32 irq_detected(struct clourney_enfc *clny_ctrl,
			struct clourney_enfc_irq_status *irq_status)
{
	clourney_enfc_read_int_status(clny_ctrl, irq_status);

	return irq_status->status;
}

static void clourney_enfc_reset_irq(struct clourney_enfc *clny_ctrl)
{
	unsigned long flags;

	spin_lock_irqsave(&clny_ctrl->irq_lock, flags);
	memset(&clny_ctrl->irq_status, 0, sizeof(clny_ctrl->irq_status));
	memset(&clny_ctrl->irq_mask, 0, sizeof(clny_ctrl->irq_mask));
	spin_unlock_irqrestore(&clny_ctrl->irq_lock, flags);
}

/*
 * This is the interrupt service routine. It handles all interrupts
 * sent to this device.
 */
static irqreturn_t clourney_enfc_isr(int irq, void *dev_id)
{
	struct clourney_enfc *clny_ctrl = dev_id;
	struct clourney_enfc_irq_status irq_status;
	irqreturn_t result = IRQ_NONE;

	spin_lock(&clny_ctrl->irq_lock);

	if (irq_detected(clny_ctrl, &irq_status)) {
		/* Handle interrupt. */
		/* First acknowledge it. */
		clourney_enfc_clear_interrupt(clny_ctrl, &irq_status);
		/* Status in the device context for someone to read. */
		clny_ctrl->irq_status.status |= irq_status.status;
		/* Notify anyone who cares that it happened. */
		complete(&clny_ctrl->complete);
		/* Tell the OS that we've handled this. */
		result = IRQ_HANDLED;
	}
	spin_unlock(&clny_ctrl->irq_lock);

	return result;
}

static void clourney_enfc_set_irq_mask(struct clourney_enfc *clny_ctrl,
				      struct clourney_enfc_irq_status *irq_mask)
{
	u32 regval = 0;

	writel_relaxed(irq_mask->status,
		       clny_ctrl->reg + CLNY_NFC_INTR_MASK);

	regval = readl_relaxed(clny_ctrl->reg + CLNY_NFC_CTRL) | CTRL_GBL_INT_EN;

	writel_relaxed( regval ,
		       clny_ctrl->reg + CLNY_NFC_CTRL);
}

static void
clourney_enfc_wait_for_irq(struct clourney_enfc *clny_ctrl,
			  struct clourney_enfc_irq_status *irq_mask,
			  struct clourney_enfc_irq_status *irq_status)
{
	unsigned long timeout = msecs_to_jiffies(10000);
	unsigned long time_left;

	time_left = wait_for_completion_timeout(&clny_ctrl->complete,
						timeout);

	*irq_status = clny_ctrl->irq_status;
	if (time_left == 0) {
		/* Timeout error. */
		dev_err(clny_ctrl->dev, "timeout occurred:\n");
		dev_err(clny_ctrl->dev, "\tstatus = 0x%x, mask = 0x%x\n",
			irq_status->status, irq_mask->status);
	}
}

/* Wait for data on slave DMA interface. */
static int clourney_enfc_wait_on_sdma(struct clourney_enfc *clny_ctrl,
				     u8 *out_sdma_trd,
				     u32 *out_sdma_size)
{
	struct clourney_enfc_irq_status irq_mask, irq_status;

	irq_mask.trd_status = 0;
	irq_mask.trd_error = 0;
	irq_mask.status = INTR_STATUS_SDMA_TRIGG
		| INTR_STATUS_SDMA_ERR
		| INTR_STATUS_UNSUPP_CMD;

	clourney_enfc_set_irq_mask(clny_ctrl, &irq_mask);
	clourney_enfc_wait_for_irq(clny_ctrl, &irq_mask, &irq_status);
	if (irq_status.status == 0) {
		dev_err(clny_ctrl->dev, "Timeout while waiting for SDMA\n");
		return -ETIMEDOUT;
	}

	if (irq_status.status & INTR_STATUS_SDMA_TRIGG) {
		*out_sdma_size = readl_relaxed(clny_ctrl->reg + SDMA_SIZE);
		*out_sdma_trd  = readl_relaxed(clny_ctrl->reg + SDMA_TRD_NUM);
		*out_sdma_trd =
			FIELD_GET(SDMA_TRD_NUM_SDMA_TRD, *out_sdma_trd);
	} else {
		dev_err(clny_ctrl->dev, "SDMA error - irq_status %x\n",
			irq_status.status);
		return -EIO;
	}

	return 0;
}

static void clourney_enfc_get_caps(struct clourney_enfc *clny_ctrl)
{
	clny_ctrl->caps2.max_banks = 8;
	clny_ctrl->caps2.data_dma_width = 8;
	clny_ctrl->caps2.data_control_supp = true;
}

/* Hardware initialization. */
static int clourney_enfc_hw_init(struct clourney_enfc *clny_ctrl)
{
	int status;
	u32 regval = 0;

	status = clourney_enfc_wait_for_value(clny_ctrl, CLNY_NFC_STATUS,
					     NAND_WAIT_RB_TIMEBASE,
					     CTRL_STATUS_IS_BUSY, true);
	if (status)
		return status;

	/* Clear all interrupts. */
	writel_relaxed(INTR_STA_CLEAR, clny_ctrl->reg + CLNY_NFC_INTR_STATUS);

	clourney_enfc_get_caps(clny_ctrl);

	/* 64 pages per block, en_clean. */
	regval  = FIELD_PREP(CTRL_BLOCK_SIZE,
			  CTRL_BLOCK_64P);
	regval |= FIELD_PREP(CTRL_ECC_BLOCK_SIZE,
			  CTRL_ECC_BLK_512B);
	regval |= (CTRL_RETRY_RSP_EN | CTRL_CLRN_EN | CTRL_ADDR1_AUTO_INCR_EN | CTRL_ADDR0_AUTO_INCR_EN);

	writel_relaxed(regval, clny_ctrl->reg + CLNY_NFC_CTRL);

	/* change mem_dev0 as lun device default. */
	regval  = FIELD_PREP(MEM_CTRL_MEMn_WP,
			  0xFF);
	regval |= FIELD_PREP(MEM_CTRL_BANK_SEL,
			  0);
	regval |= FIELD_PREP(MEM_CTRL_MEM_CE,
			  0);
	writel_relaxed(regval, clny_ctrl->reg + NFC_MEM_CTRL_REG);
	/*
	 * Set IO width access to 8.
	 * It is because during SW device discovering width access
	 * is expected to be 8.
	 */
	status = clourney_enfc_set_access_width16(clny_ctrl, false);

	return status;
}

#define TT_MAIN_OOB_AREAS	2
#define TT_RAW_PAGE		3
#define TT_BBM			4
#define TT_MAIN_OOB_AREA_EXT	5

static u8 calc_timing_setvalue(u32 cycle_uint, u32 val)
{
	int tmpval;

	tmpval = (val * 10) / cycle_uint;

	if (tmpval == 0)
		return 1;

	if (tmpval % 10)
		return (u8)((tmpval / 10) + 1);
	else
		return (u8)(tmpval / 10);
}

static void clourney_enfc_set_timings(struct clourney_enfc *clny_ctrl,
				     struct clourney_enfc_timings *t)
{
	u32 t_async, t0_sync, t1_sync;

	t0_sync = (calc_timing_setvalue(clny_ctrl->clk_cycle, t->tWHR) << tWHR_BIT) & tWHR_MASK \
		| (calc_timing_setvalue(clny_ctrl->clk_cycle, t->tRHW) << tRHW_BIT) & tRHW_MASK \
		| (calc_timing_setvalue(clny_ctrl->clk_cycle, t->tADL) << tADL_BIT) & tADL_MASK \
		| (calc_timing_setvalue(clny_ctrl->clk_cycle, t->tCCS) << tCCS_BIT) & tCCS_MASK;

	t1_sync = (calc_timing_setvalue(clny_ctrl->clk_cycle, t->tRR) << tRR_BIT) & tRR_MASK \
		| (calc_timing_setvalue(clny_ctrl->clk_cycle, t->tWB) << tWB_BIT) & tWB_MASK;

	t_async = (calc_timing_setvalue(clny_ctrl->clk_cycle, t->tRWH) << tRWH_BIT) & tRWH_MASK \
		| (calc_timing_setvalue(clny_ctrl->clk_cycle, t->tRWP) << tRWP_BIT) & tRWP_MASK;

#if 0
	dev_info(clny_ctrl->dev, "t0_sync = 0x%08x t1_sync = 0x%08x t_async = 0x%08x\r\n", t0_sync, t1_sync, t_async);
#endif
	writel_relaxed(t_async, clny_ctrl->reg + NFC_TIMINGS_ASYNC);
	writel_relaxed(t0_sync, clny_ctrl->reg + NFC_TIMING_SEQ0);
	writel_relaxed(t1_sync, clny_ctrl->reg + NFC_TIMING_SEQ1);
}

static int clourney_enfc_select_target(struct nand_chip *chip)
{
	struct clourney_enfc *clny_ctrl = to_clourney_enfc(chip->controller);
	struct clourney_enfc_chip *clny_chip = to_clourney_enfc_chip(chip);

	if (chip == clny_ctrl->selected_chip)
		return 0;

	if (clourney_enfc_wait_for_value(clny_ctrl, CLNY_NFC_STATUS,
					NAND_WAIT_RB_TIMEBASE,
					CTRL_STATUS_IS_BUSY, true))
		return -ETIMEDOUT;

	clourney_enfc_set_timings(clny_ctrl, &clny_ctrl->timings);

	clny_ctrl->selected_chip = chip;

	return 0;
}

static int clourney_enfc_write_page(struct nand_chip *chip,
				   const u8 *buf, int oob_required,
				   int page)
{
	int status = 0;

	return status;

}

static int clourney_enfc_write_oob(struct nand_chip *chip, int page)
{
	struct clourney_enfc *clny_ctrl = to_clourney_enfc(chip->controller);
	struct mtd_info *mtd = nand_to_mtd(chip);

	memset(clny_ctrl->buf, 0xFF, mtd->writesize);

	return clourney_enfc_write_page(chip, clny_ctrl->buf, 1, page);
}

static int clourney_enfc_write_page_raw(struct nand_chip *chip,
				       const u8 *buf, int oob_required,
				       int page)
{
	int status = 0;

	return status;

}

static int clourney_enfc_write_oob_raw(struct nand_chip *chip,
				      int page)
{
	return clourney_enfc_write_page_raw(chip, NULL, true, page);
}

static int clourney_enfc_read_page(struct nand_chip *chip,
				  u8 *buf, int oob_required, int page)
{
	int status = 0;

	return status;
}

/* Reads OOB data from the device. */
static int clourney_enfc_read_oob(struct nand_chip *chip, int page)
{
	struct clourney_enfc *clny_ctrl = to_clourney_enfc(chip->controller);

	return clourney_enfc_read_page(chip, clny_ctrl->buf, 1, page);
}

static int clourney_enfc_read_page_raw(struct nand_chip *chip,
				      u8 *buf, int oob_required, int page)
{
	int status = 0;

	return status;
}

static int clourney_enfc_read_oob_raw(struct nand_chip *chip,
				     int page)
{
	return clourney_enfc_read_page_raw(chip, NULL, true, page);
}

static int clourney_enfc_rw_op(struct clourney_enfc *clny_ctrl, struct clourney_enfc_op *enfc_op)
{
	u8 *buf = enfc_op->buf;
	unsigned int dwords = (enfc_op->len / sizeof(u32));
	unsigned int last_len = enfc_op->len % sizeof(u32);
	unsigned int offset=0;
	u32 remainder;

	if(enfc_op->read) {
		for(offset = 0; offset < dwords; offset++)
		{
			remainder = readl_relaxed(clny_ctrl->reg + NFC_FIFO_DATA_REG);
			memcpy(&buf[offset * sizeof(u32)], &remainder, sizeof(u32));
		}
	} else {
		for(offset = 0; offset < dwords; offset++)
		{
			memcpy(&remainder, &buf[offset * sizeof(u32)], sizeof(u32));
			writel_relaxed(remainder, clny_ctrl->reg + NFC_FIFO_DATA_REG);
		}
	}

	if (last_len) {
		offset = enfc_op->len - last_len;

		if (enfc_op->read) {
			remainder = readl_relaxed(clny_ctrl->reg + NFC_FIFO_DATA_REG);
			memcpy(&buf[offset], &remainder, last_len);
		} else {
			memcpy(&remainder, &buf[offset], last_len);
			writel_relaxed(remainder, clny_ctrl->reg + NFC_FIFO_DATA_REG);
		}
	}

	#if 0
	pr_info("%s:opcode:%x cmd:%x row:%x col:%x len = %d\r\n",__func__,enfc_op->opcode,enfc_op->cmd_reg, \
					enfc_op->addrx_row,enfc_op->addrx_col,enfc_op->len);
	for(offset = 0; offset < enfc_op->len; offset++)
	{
		pr_info("%02x ",*(buf + offset));
	}
	pr_info("\r\n");
	#endif
	return 0;
}

static int clourney_enfc_wait_for_rb(struct clourney_enfc *clny_ctrl, struct nand_chip *chip,
			    unsigned int timeout_ms)
{
	struct clourney_enfc_chip *clny_chip = to_clourney_enfc_chip(chip);
	int status;

	/* There is no R/B interrupt, we must poll a register */
	status = clourney_enfc_wait_for_value(clny_ctrl, CLNY_NFC_STATUS,
					     timeout_ms * NAND_WAIT_RB_TIMEBASE,
					     CTRL_STATUS_IS_BUSY, true);

	if (status) {
		dev_err(clny_ctrl->dev, "Timeout %d waiting for enfc R/B %s 0x%x\n", timeout_ms, \
			(status ? "Busy" : "Ready"), readl_relaxed(clny_ctrl->reg + CLNY_NFC_STATUS));
		return status;
	}

	status = clourney_enfc_wait_for_value(clny_ctrl, CLNY_NFC_STATUS,
					     timeout_ms * NAND_WAIT_RB_TIMEBASE,
					     BIT(clny_chip->cs[chip->cur_cs]), false);

	if (status) {
		dev_err(clny_ctrl->dev, "Timeout %d waiting for device%d R/B %s 0x%x\n",timeout_ms, clny_chip->cs[chip->cur_cs],
			(!status ? "Busy" : "Ready"), readl_relaxed(clny_ctrl->reg + CLNY_NFC_STATUS));
		return status;
	}


	return 0;
}


static void clourney_enfc_trigger_op(struct clourney_enfc *clny_enfc, struct clourney_enfc_op *enfc_op)
{
	if (!(enfc_op->skip_adds_op)) {
		writel_relaxed(enfc_op->addrx_col, clny_enfc->reg + NFC_ADDR0_COL_REG);
		writel_relaxed(enfc_op->addrx_row, clny_enfc->reg + NFC_ADDR0_ROW_REG);
	}

	writel_relaxed(enfc_op->len, clny_enfc->reg + NFC_DATA_SIZE_REG);
	writel_relaxed(enfc_op->cmd_reg, clny_enfc->reg + CLNY_NFC_CMD);

	pr_debug("%s:line%d addrx_col =%x addrx_row = %x len = %d cmd_reg = %x ",__func__,__LINE__,\
		enfc_op->addrx_col, enfc_op->addrx_row, enfc_op->len, enfc_op->cmd_reg);
}

/* NAND framework ->exec_op() hooks and related helpers */
static int clourney_enfc_parse_instructions(struct nand_chip *chip,
				   const struct nand_subop *subop,
				   struct clourney_enfc_op *enfc_op)
{
	const struct nand_op_instr *instr = NULL;
	unsigned int op_id;
	int i;

	memset(enfc_op, 0, sizeof(*enfc_op));

	for (op_id = 0; op_id < subop->ninstrs; op_id++) {
		unsigned int offset, naddrs;
		const u8 *addrs;
		u8 *buf;

		instr = &subop->instrs[op_id];

		switch (instr->type) {
		case NAND_OP_CMD_INSTR:
			// only parse the first cmd instr
			if (op_id == 0){
				enfc_op->opcode = instr->ctx.cmd.opcode;
				enfc_op->cmd_reg = clourney_enfc_generic_cmd_parse(instr->ctx.cmd.opcode);
			}
			// prog page op need parse the second cmd instr
			if (((subop->instrs[0].ctx.cmd.opcode == NAND_CMD_READ0) && (subop->instrs[1].ctx.cmd.opcode == NAND_CMD_SEQIN)) && (op_id == 1))
			{
				enfc_op->opcode = instr->ctx.cmd.opcode;
				enfc_op->cmd_reg = clourney_enfc_generic_cmd_parse(instr->ctx.cmd.opcode);
			}
			break;

		case NAND_OP_ADDR_INSTR:
			offset = nand_subop_get_addr_start_off(subop, op_id);
			naddrs = nand_subop_get_num_addr_cyc(subop, op_id);
			addrs = &instr->ctx.addr.addrs[offset];

			pr_debug("%s:line%d naddrs = %x ",__func__,__LINE__,naddrs);

			for (i = 0; i < min(MAX_ADDRESS_CYC, naddrs); i++) {
				pr_debug("addrs[%d] = %x ",i,addrs[i]);

				if (subop->instrs[0].ctx.cmd.opcode == NAND_CMD_ERASE1) {
					enfc_op->addrx_row  |= (u32)addrs[i] << i * 8;
					enfc_op->addrx_col  = 0;
				} else {
					if (i < COL_ADDRESS_CYC)
						enfc_op->addrx_col  |= (u32)addrs[i] << i * 8;
					else
						enfc_op->addrx_row|= (u32)addrs[i] << (i - COL_ADDRESS_CYC) * 8;
				}
			}
			break;
		case NAND_OP_DATA_IN_INSTR:
			enfc_op->read = true;
			enfc_op->need_cmd_wait = true;
			offset = nand_subop_get_data_start_off(subop, op_id);
			buf = instr->ctx.data.buf.in;
			enfc_op->buf = &buf[offset];
			enfc_op->len = nand_subop_get_data_len(subop, op_id);

			if ((enfc_op->opcode == NAND_CMD_READID) || (enfc_op->opcode == NAND_CMD_STATUS))
				enfc_op->rdy_timeout_ms = 1;

			break;
		case NAND_OP_DATA_OUT_INSTR:
			enfc_op->read = false;
			enfc_op->need_cmd_wait = false;
			offset = nand_subop_get_data_start_off(subop, op_id);
			buf = instr->ctx.data.buf.out;
			enfc_op->buf = &buf[offset];
			enfc_op->len = nand_subop_get_data_len(subop, op_id);

			if (enfc_op->opcode == NAND_CMD_SEQIN)
				enfc_op->rdy_timeout_ms = 1;

			break;
		case NAND_OP_WAITRDY_INSTR:
			enfc_op->rdy_timeout_ms = instr->ctx.waitrdy.timeout_ms;
			break;
		}
	}

	if ( !enfc_op->need_cmd_wait && enfc_op->rdy_timeout_ms)
		enfc_op->need_data_wait = true;

	if ( !enfc_op->need_data_wait && enfc_op->rdy_timeout_ms)
		enfc_op->need_cmd_wait = true;

	return 0;
}

static int clourney_enfc_zerolen_cmd_exec(struct nand_chip *chip,
				       const struct nand_subop *subop,
				       bool skip_cmd_op)
{
	struct clourney_enfc *clny_ctrl = to_clourney_enfc(chip->controller);
	struct clourney_enfc_op enfc_op = {};
	int ret;

	ret = clourney_enfc_parse_instructions(chip, subop, &enfc_op);
	if (ret)
		return ret;

	if (!skip_cmd_op)
		clourney_enfc_trigger_op(clny_ctrl, &enfc_op);

	enfc_op.rdy_timeout_ms = 1;

	ret = clourney_enfc_wait_for_rb(clny_ctrl, chip, enfc_op.rdy_timeout_ms);

	return ret;
}

static int clourney_enfc_cmd_data_exec(struct nand_chip *chip,
				       const struct nand_subop *subop,
				       bool skip_adds_op)
{
	struct clourney_enfc *clny_ctrl = to_clourney_enfc(chip->controller);
	struct clourney_enfc_op enfc_op = {};
	int ret = 0;

	ret = clourney_enfc_parse_instructions(chip, subop, &enfc_op);
	if (ret)
		return ret;

	enfc_op.skip_adds_op = skip_adds_op;

	clourney_enfc_trigger_op(clny_ctrl, &enfc_op);

	if ( enfc_op.need_cmd_wait ) {
		ret = clourney_enfc_wait_for_rb(clny_ctrl, chip, enfc_op.rdy_timeout_ms);
		if (ret)
			return ret;
	}

	ret = clourney_enfc_rw_op(clny_ctrl, &enfc_op);

	if (ret)
		return ret;

	if ( enfc_op.need_data_wait ) {
		ret = clourney_enfc_wait_for_rb(clny_ctrl, chip, enfc_op.rdy_timeout_ms);
		if (ret)
			return ret;
	}

	return ret;
}

static void clourney_enfc_slave_dma_transfer_finished(void *data)
{
	struct completion *finished = data;

	complete(finished);
}

static int clourney_enfc_slave_dma_transfer(struct clourney_enfc *clny_ctrl,
					   void *buf,
					   dma_addr_t dev_dma, size_t len,
					   enum dma_data_direction dir)
{
	DECLARE_COMPLETION_ONSTACK(finished);
	struct dma_chan *chan;
	struct dma_device *dma_dev;
	dma_addr_t src_dma, dst_dma, buf_dma;
	struct dma_async_tx_descriptor *tx;
	dma_cookie_t cookie;

	chan = clny_ctrl->dmac;
	dma_dev = chan->device;

	buf_dma = dma_map_single(dma_dev->dev, buf, len, dir);
	if (dma_mapping_error(dma_dev->dev, buf_dma)) {
		dev_err(clny_ctrl->dev, "Failed to map DMA buffer\n");
		goto err;
	}

	if (dir == DMA_FROM_DEVICE) {
		src_dma = clny_ctrl->io.dma;
		dst_dma = buf_dma;
	} else {
		src_dma = buf_dma;
		dst_dma = clny_ctrl->io.dma;
	}

	tx = dmaengine_prep_dma_memcpy(clny_ctrl->dmac, dst_dma, src_dma, len,
				       DMA_CTRL_ACK | DMA_PREP_INTERRUPT);
	if (!tx) {
		dev_err(clny_ctrl->dev, "Failed to prepare DMA memcpy\n");
		goto err_unmap;
	}

	tx->callback = clourney_enfc_slave_dma_transfer_finished;
	tx->callback_param = &finished;

	cookie = dmaengine_submit(tx);
	if (dma_submit_error(cookie)) {
		dev_err(clny_ctrl->dev, "Failed to do DMA tx_submit\n");
		goto err_unmap;
	}

	dma_async_issue_pending(clny_ctrl->dmac);
	wait_for_completion(&finished);

	dma_unmap_single(clny_ctrl->dev, buf_dma, len, dir);

	return 0;

err_unmap:
	dma_unmap_single(clny_ctrl->dev, buf_dma, len, dir);

err:
	dev_dbg(clny_ctrl->dev, "Fall back to CPU I/O\n");

	return -EIO;
}

static int clourney_enfc_read_buf(struct clourney_enfc *clny_ctrl,
				 u8 *buf, int len)
{
	u8 thread_nr = 0;
	u32 sdma_size;
	int status;

	/* Wait until slave DMA interface is ready to data transfer. */
	status = clourney_enfc_wait_on_sdma(clny_ctrl, &thread_nr, &sdma_size);
	if (status)
		return status;

	if (clourney_enfc_dma_buf_ok(clny_ctrl, buf, len)) {
		status = clourney_enfc_slave_dma_transfer(clny_ctrl, buf,
							 clny_ctrl->io.dma,
							 len, DMA_FROM_DEVICE);
		if (status == 0)
			return 0;

		dev_warn(clny_ctrl->dev,
			 "Slave DMA transfer failed. Try again using bounce buffer.");
	}

	/* If DMA transfer is not possible or failed then use bounce buffer. */
	status = clourney_enfc_slave_dma_transfer(clny_ctrl, clny_ctrl->buf,
						 clny_ctrl->io.dma,
						 sdma_size, DMA_FROM_DEVICE);

	if (status) {
		dev_err(clny_ctrl->dev, "Slave DMA transfer failed");
		return status;
	}

	memcpy(buf, clny_ctrl->buf, len);

	return 0;
}

static int clourney_enfc_write_buf(struct clourney_enfc *clny_ctrl,
				  const u8 *buf, int len)
{
	u8 thread_nr = 0;
	u32 sdma_size;
	int status;

	/* Wait until slave DMA interface is ready to data transfer. */
	status = clourney_enfc_wait_on_sdma(clny_ctrl, &thread_nr, &sdma_size);
	if (status)
		return status;

	if (clourney_enfc_dma_buf_ok(clny_ctrl, buf, len)) {
		status = clourney_enfc_slave_dma_transfer(clny_ctrl, (void *)buf,
							 clny_ctrl->io.dma,
							 len, DMA_TO_DEVICE);
		if (status == 0)
			return 0;

		dev_warn(clny_ctrl->dev,
			 "Slave DMA transfer failed. Try again using bounce buffer.");
	}

	/* If DMA transfer is not possible or failed then use bounce buffer. */
	memcpy(clny_ctrl->buf, buf, len);

	status = clourney_enfc_slave_dma_transfer(clny_ctrl, clny_ctrl->buf,
						 clny_ctrl->io.dma,
						 sdma_size, DMA_TO_DEVICE);

	if (status)
		dev_err(clny_ctrl->dev, "Slave DMA transfer failed");

	return status;
}

static int clourney_enfc_data_in(struct nand_chip *chip,
				       const struct nand_subop *subop)
{
	struct clourney_enfc *clny_ctrl = to_clourney_enfc(chip->controller);
	struct clourney_enfc_op enfc_op = {};
	int ret;

	ret = clourney_enfc_parse_instructions(chip, subop, &enfc_op);
	if (ret)
		return ret;

	enfc_op.rdy_timeout_ms = 3;

	ret = clourney_enfc_wait_for_rb(clny_ctrl, chip, enfc_op.rdy_timeout_ms);
	if (ret)
		return ret;

	return clourney_enfc_rw_op(clny_ctrl, &enfc_op);
}

static int clourney_enfc_cmd_reset(struct nand_chip *chip,
				const struct nand_subop *subop)
{
	int ret = 0;

	if ((subop->instrs[0].ctx.cmd.opcode != NAND_CMD_RESET) && (subop->instrs[0].ctx.cmd.opcode != NAND_CMD_PAGEPROG))
		return -ENOTSUPP;

	if (subop->instrs[0].ctx.cmd.opcode == NAND_CMD_RESET)
		return clourney_enfc_zerolen_cmd_exec(chip, subop, false);


	if (subop->instrs[0].ctx.cmd.opcode == NAND_CMD_PAGEPROG)
		return clourney_enfc_zerolen_cmd_exec(chip, subop, true);

	return ret;
}

static int clourney_enfc_cmd_readid(struct nand_chip *chip,
				  const struct nand_subop *subop)
{
	if (subop->instrs[0].ctx.cmd.opcode != NAND_CMD_READID)
		return -ENOTSUPP;

	return clourney_enfc_cmd_data_exec(chip, subop, false);
}

static int clourney_enfc_cmd_erase(struct nand_chip *chip,
				  const struct nand_subop *subop)
{
	if ((subop->instrs[0].ctx.cmd.opcode != NAND_CMD_ERASE1) && (subop->instrs[0].ctx.cmd.opcode != NAND_CMD_ERASE2))
		return -ENOTSUPP;

	return clourney_enfc_zerolen_cmd_exec(chip, subop, false);
}

static int clourney_enfc_cmd_status(struct nand_chip *chip,
				  const struct nand_subop *subop)
{
	/* See anfc_check_op() for details about this constraint */
	if (subop->instrs[0].ctx.cmd.opcode != NAND_CMD_STATUS)
		return -ENOTSUPP;

	return clourney_enfc_cmd_data_exec(chip, subop, false);
}

static int clourney_enfc_cmd_get_param_features(struct nand_chip *chip,
				     const struct nand_subop *subop)
{
	if ((subop->instrs[0].ctx.cmd.opcode != NAND_CMD_PARAM) && (subop->instrs[0].ctx.cmd.opcode != NAND_CMD_GET_FEATURES))
		return -ENOTSUPP;

	return clourney_enfc_cmd_data_exec(chip, subop, false);
}

static int clourney_enfc_cmd_set_features(struct nand_chip *chip,
				     const struct nand_subop *subop)
{
	if (subop->instrs[0].ctx.cmd.opcode != NAND_CMD_SET_FEATURES)
		return -ENOTSUPP;

	return clourney_enfc_cmd_data_exec(chip, subop, false);
}

static int clourney_enfc_cmd_data_read(struct nand_chip *chip,
				    const struct nand_subop *subop)
{
	if ((subop->instrs[0].ctx.cmd.opcode != NAND_CMD_READ0) && (subop->instrs[0].ctx.cmd.opcode != NAND_CMD_READOOB))
		return -ENOTSUPP;

	return clourney_enfc_cmd_data_exec(chip, subop, false);
}

static int clourney_enfc_cmd_data_write(struct nand_chip *chip,
				     const struct nand_subop *subop)
{
	if (subop->instrs[0].ctx.cmd.opcode != NAND_CMD_SEQIN)
		return -ENOTSUPP;

	return clourney_enfc_cmd_data_exec(chip, subop, false);
}

static int clourney_enfc_cmd_waitrdy(struct nand_chip *chip,
				    const struct nand_subop *subop)
{
	struct clourney_enfc *clny_ctrl = to_clourney_enfc(chip->controller);
	struct clourney_enfc_op enfc_op = {};
	int ret;

	ret = clourney_enfc_parse_instructions(chip, subop, &enfc_op);
	if (ret)
		return ret;

	if (enfc_op.rdy_timeout_ms)
		ret = clourney_enfc_wait_for_rb(clny_ctrl, chip, enfc_op.rdy_timeout_ms);

	return ret;
}

static const struct nand_op_parser clourney_enfc_op_parser = NAND_OP_PARSER(
	NAND_OP_PARSER_PATTERN(
		clourney_enfc_cmd_reset,
		NAND_OP_PARSER_PAT_CMD_ELEM(false),
		NAND_OP_PARSER_PAT_WAITRDY_ELEM(false)),
	NAND_OP_PARSER_PATTERN(
		clourney_enfc_cmd_readid,
		NAND_OP_PARSER_PAT_CMD_ELEM(false),
		NAND_OP_PARSER_PAT_ADDR_ELEM(false, MAX_ADDRESS_CYC),
		NAND_OP_PARSER_PAT_DATA_IN_ELEM(false, MAX_DATA_PAGE_SIZE)),
	NAND_OP_PARSER_PATTERN(
		clourney_enfc_cmd_get_param_features,
		NAND_OP_PARSER_PAT_CMD_ELEM(false),
		NAND_OP_PARSER_PAT_ADDR_ELEM(false, MAX_ADDRESS_CYC),
		NAND_OP_PARSER_PAT_WAITRDY_ELEM(true),
		NAND_OP_PARSER_PAT_DATA_IN_ELEM(false, MAX_DATA_PAGE_SIZE)),
	NAND_OP_PARSER_PATTERN(
		clourney_enfc_cmd_data_read,
		NAND_OP_PARSER_PAT_CMD_ELEM(false),
		NAND_OP_PARSER_PAT_ADDR_ELEM(false, MAX_ADDRESS_CYC),
		NAND_OP_PARSER_PAT_CMD_ELEM(false),
		NAND_OP_PARSER_PAT_WAITRDY_ELEM(false),
		NAND_OP_PARSER_PAT_DATA_IN_ELEM(false, MAX_DATA_PAGE_SIZE)),
	NAND_OP_PARSER_PATTERN(
		clourney_enfc_data_in,
		NAND_OP_PARSER_PAT_DATA_IN_ELEM(false, MAX_DATA_PAGE_SIZE)),
	NAND_OP_PARSER_PATTERN(
		clourney_enfc_cmd_data_write,
		NAND_OP_PARSER_PAT_CMD_ELEM(false),
		NAND_OP_PARSER_PAT_ADDR_ELEM(false, MAX_ADDRESS_CYC),
		NAND_OP_PARSER_PAT_DATA_OUT_ELEM(false, MAX_DATA_PAGE_SIZE)),
	NAND_OP_PARSER_PATTERN(
		clourney_enfc_cmd_erase,
		NAND_OP_PARSER_PAT_CMD_ELEM(false),
		NAND_OP_PARSER_PAT_ADDR_ELEM(false, MAX_ERASE_ADDRESS_CYC),
		NAND_OP_PARSER_PAT_CMD_ELEM(false),
		NAND_OP_PARSER_PAT_WAITRDY_ELEM(false)),
	NAND_OP_PARSER_PATTERN(
		clourney_enfc_cmd_status,
		NAND_OP_PARSER_PAT_CMD_ELEM(false),
		NAND_OP_PARSER_PAT_DATA_IN_ELEM(false, MAX_DATA_PAGE_SIZE)),
	NAND_OP_PARSER_PATTERN(
		clourney_enfc_cmd_set_features,
		NAND_OP_PARSER_PAT_CMD_ELEM(false),
		NAND_OP_PARSER_PAT_ADDR_ELEM(false, MAX_ADDRESS_CYC),
		NAND_OP_PARSER_PAT_DATA_OUT_ELEM(false, MAX_DATA_PAGE_SIZE),
		NAND_OP_PARSER_PAT_WAITRDY_ELEM(false)),
	NAND_OP_PARSER_PATTERN(
		clourney_enfc_cmd_waitrdy,
		NAND_OP_PARSER_PAT_WAITRDY_ELEM(false))
	);

static int clourney_enfc_check_op(struct nand_chip *chip,
			 const struct nand_operation *op)
{
	const struct nand_op_instr *instr;
	int op_id;

	/*
	 * The controller abstracts all the NAND operations and do not support
	 * data only operations.
	 *
	 * TODO: The nand_op_parser framework should be extended to
	 * support custom checks on DATA instructions.
	 */
	for (op_id = 0; op_id < op->ninstrs; op_id++) {
		instr = &op->instrs[op_id];

		switch (instr->type) {
		case NAND_OP_ADDR_INSTR:
			if (instr->ctx.addr.naddrs > MAX_ADDRESS_CYC)
				return -ENOTSUPP;

			break;
		case NAND_OP_DATA_IN_INSTR:
		case NAND_OP_DATA_OUT_INSTR:
			if (instr->ctx.data.len > MAX_DATA_PAGE_SIZE)
				return -ENOTSUPP;

			break;
		default:
			break;
		}
	}

	/*
	 * The controller does not allow to proceed with a CMD+DATA_IN cycle
	 * manually on the bus by reading data from the data register. Instead,
	 * the controller abstract a status read operation with its own status
	 * register after ordering a read status operation. Hence, we cannot
	 * support any CMD+DATA_IN operation other than a READ STATUS.
	 *
	 * TODO: The nand_op_parser() framework should be extended to describe
	 * fixed patterns instead of open-coding this check here.
	 */
	if (op->ninstrs == 2 &&
	    op->instrs[0].type == NAND_OP_CMD_INSTR &&
	    op->instrs[0].ctx.cmd.opcode != NAND_CMD_STATUS &&
	    op->instrs[1].type == NAND_OP_DATA_IN_INSTR)
		return -ENOTSUPP;

	return nand_op_parser_exec_op(chip, &clourney_enfc_op_parser, op, true);
}

static int clourney_enfc_exec_op(struct nand_chip *chip,
				const struct nand_operation *op,
				bool check_only)
{
	int status;
	struct clourney_enfc *clny_ctrl = to_clourney_enfc(chip->controller);

	if (check_only)
		return clourney_enfc_check_op(chip, op);
	status = clourney_enfc_select_target(chip);

	if (status)
		return status;

	status = nand_op_parser_exec_op(chip, &clourney_enfc_op_parser, op,
			check_only);
	if (status)
		dev_info(clny_ctrl->dev, "%s:line=%d status=%d\n",__func__,__LINE__,status);
	return status;
}

static int clourney_enfc_ooblayout_free(struct mtd_info *mtd, int section,
				       struct mtd_oob_region *oobregion)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct clourney_enfc_chip *clny_chip = to_clourney_enfc_chip(chip);

	if (section)
		return -ERANGE;

	oobregion->offset = clny_chip->bbm_len;
	oobregion->length = clny_chip->avail_oob_size
		- clny_chip->bbm_len;

	return 0;
}

static int clourney_enfc_ooblayout_ecc(struct mtd_info *mtd, int section,
				      struct mtd_oob_region *oobregion)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct clourney_enfc_chip *clny_chip = to_clourney_enfc_chip(chip);

	if (section)
		return -ERANGE;

	oobregion->offset = clny_chip->avail_oob_size;
	oobregion->length = chip->ecc.total;

	return 0;
}

static const struct mtd_ooblayout_ops clourney_enfc_ooblayout_ops = {
	.free = clourney_enfc_ooblayout_free,
	.ecc = clourney_enfc_ooblayout_ecc,
};

static int
clourney_enfc_setup_interface(struct nand_chip *chip, int chipnr,
			     const struct nand_interface_config *conf)
{
	return 0;
}

static int clourney_enfc_attach_chip(struct nand_chip *chip)
{
	struct clourney_enfc *clny_ctrl = to_clourney_enfc(chip->controller);
	int ret;

	if (chip->options & NAND_BUSWIDTH_16) {
		ret = clourney_enfc_set_access_width16(clny_ctrl, true);
		if (ret)
			return ret;
	}

	chip->ecc.engine_type = NAND_ECC_ENGINE_TYPE_NONE;
	chip->bbt_options |= NAND_BBT_USE_FLASH;
	chip->bbt_options |= NAND_BBT_NO_OOB;
	chip->options |= NAND_SKIP_BBTSCAN | NAND_NO_BBM_QUIRK;

	switch (chip->ecc.engine_type) {
	case NAND_ECC_ENGINE_TYPE_NONE:
	case NAND_ECC_ENGINE_TYPE_SOFT:
	case NAND_ECC_ENGINE_TYPE_ON_DIE:
		break;
	case NAND_ECC_ENGINE_TYPE_ON_HOST:
		break;
	default:
		dev_err(clny_ctrl->dev, "Unsupported ECC mode: %d\n",
			chip->ecc.engine_type);
		return -EINVAL;
	}

	return 0;
}

static const struct nand_controller_ops clourney_enfc_controller_ops = {
	.attach_chip = clourney_enfc_attach_chip,
	.setup_interface = clourney_enfc_setup_interface,
	.exec_op = clourney_enfc_exec_op,
};

static int clourney_enfc_chip_init(struct clourney_enfc *clny_ctrl,
				  struct device_node *np)
{
	struct clourney_enfc_chip *clny_chip;
	struct mtd_info *mtd;
	struct nand_chip *chip;
	int nsels, ret, i;
	u32 cs;

	nsels = of_property_count_elems_of_size(np, "reg", sizeof(u32));
	if (nsels <= 0) {
		dev_err(clny_ctrl->dev, "missing/invalid reg property\n");
		return -EINVAL;
	}

	/* Allocate the nand chip structure. */
	clny_chip = devm_kzalloc(clny_ctrl->dev, sizeof(*clny_chip) +
				 (nsels * sizeof(u8)),
				 GFP_KERNEL);
	if (!clny_chip) {
		dev_err(clny_ctrl->dev, "could not allocate chip structure\n");
		return -ENOMEM;
	}

	clny_chip->nsels = nsels;

	for (i = 0; i < nsels; i++) {
		/* Retrieve CS id. */
		ret = of_property_read_u32_index(np, "reg", i, &cs);
		if (ret) {
			dev_err(clny_ctrl->dev,
				"could not retrieve reg property: %d\n",
				ret);
			return ret;
		}

		if (cs >= clny_ctrl->caps2.max_banks) {
			dev_err(clny_ctrl->dev,
				"invalid reg value: %u (max CS = %d)\n",
				cs, clny_ctrl->caps2.max_banks);
			return -EINVAL;
		}

		if (test_and_set_bit(cs, &clny_ctrl->assigned_cs)) {
			dev_err(clny_ctrl->dev,
				"CS %d already assigned\n", cs);
			return -EINVAL;
		}

		clny_chip->cs[i] = cs;
	}

	chip = &clny_chip->chip;
	chip->controller = &clny_ctrl->controller;
	nand_set_flash_node(chip, np);

	mtd = nand_to_mtd(chip);
	mtd->dev.parent = clny_ctrl->dev;

	ret = nand_scan(chip, clny_chip->nsels);
	if (ret) {
		dev_err(clny_ctrl->dev, "could not scan the nand chip error:%d\n",ret);
		return ret;
	}

	ret = mtd_device_register(mtd, NULL, 0);
	if (ret) {
		dev_err(clny_ctrl->dev,
			"failed to register mtd device: %d\n", ret);
		nand_cleanup(chip);
		return ret;
	}

	list_add_tail(&clny_chip->node, &clny_ctrl->chips);

	return 0;
}

static void clourney_enfc_chips_cleanup(struct clourney_enfc *clny_ctrl)
{
	struct clourney_enfc_chip *entry, *temp;
	struct nand_chip *chip;
	int ret;

	list_for_each_entry_safe(entry, temp, &clny_ctrl->chips, node) {
		chip = &entry->chip;
		ret = mtd_device_unregister(nand_to_mtd(chip));
		WARN_ON(ret);
		nand_cleanup(chip);
		list_del(&entry->node);
	}
}

static int clourney_enfc_chips_init(struct clourney_enfc *clny_ctrl)
{
	struct device_node *np = clny_ctrl->dev->of_node;
	struct device_node *nand_np;
	int max_cs = clny_ctrl->caps2.max_banks;
	int nchips, ret;

	nchips = of_get_child_count(np);

	if (nchips > max_cs) {
		dev_err(clny_ctrl->dev,
			"too many NAND chips: %d (max = %d CS)\n",
			nchips, max_cs);
		return -EINVAL;
	}

	for_each_child_of_node(np, nand_np) {
		ret = clourney_enfc_chip_init(clny_ctrl, nand_np);
		if (ret) {
			of_node_put(nand_np);
			clourney_enfc_chips_cleanup(clny_ctrl);
			return ret;
		}
	}

	return 0;
}

static void
clourney_enfc_irq_cleanup(int irqnum, struct clourney_enfc *clny_ctrl)
{
	u32 regval = 0;
	/* Disable interrupts. */
	writel_relaxed(NFC_INTR_DISABLE, clny_ctrl->reg + CLNY_NFC_INTR_MASK);
	/* Disable Global interrupts */
	regval = readl_relaxed(clny_ctrl->reg + CLNY_NFC_CTRL);
	regval = regval | (~CTRL_GBL_INT_EN);
	writel_relaxed(regval, clny_ctrl->reg + CLNY_NFC_CTRL);
}

static int clourney_enfc_init(struct clourney_enfc *clny_ctrl)
{
	int ret;

	clny_ctrl->buf_size = SZ_16K;
	clny_ctrl->buf = kmalloc(clny_ctrl->buf_size, GFP_KERNEL);
	if (!clny_ctrl->buf) {
		ret = -ENOMEM;
		return ret;
	}

	if(clny_ctrl->sdma_enable)
	{
		if (devm_request_irq(clny_ctrl->dev, clny_ctrl->irq, clourney_enfc_isr,
				     IRQF_SHARED, "clourney-enfc",
				     clny_ctrl)) {
			dev_err(clny_ctrl->dev, "Unable to allocate IRQ\n");
			ret = -ENODEV;
			goto free_buf;
		}
	}

	spin_lock_init(&clny_ctrl->irq_lock);
	init_completion(&clny_ctrl->complete);

	ret = clourney_enfc_hw_init(clny_ctrl);
	if (ret)
		goto disable_irq;

	nand_controller_init(&clny_ctrl->controller);
	INIT_LIST_HEAD(&clny_ctrl->chips);

	clny_ctrl->controller.ops = &clourney_enfc_controller_ops;
	clny_ctrl->curr_corr_str_idx = 0xFF;

	ret = clourney_enfc_chips_init(clny_ctrl);
	if (ret) {
		dev_err(clny_ctrl->dev, "Failed to register MTD: %d\n",
			ret);
		goto disable_irq;
	}

	kfree(clny_ctrl->buf);
	clny_ctrl->buf = kzalloc(clny_ctrl->buf_size, GFP_KERNEL);
	if (!clny_ctrl->buf) {
		ret = -ENOMEM;
		goto disable_irq;
	}

	return 0;

disable_irq:
	if(clny_ctrl->sdma_enable)
		clourney_enfc_irq_cleanup(clny_ctrl->irq, clny_ctrl);
free_buf:
	kfree(clny_ctrl->buf);

	return ret;
}

/* Driver exit point. */
static void clourney_enfc_remove(struct clourney_enfc *clny_ctrl)
{
	clourney_enfc_chips_cleanup(clny_ctrl);

	if(clny_ctrl->sdma_enable)
		clourney_enfc_irq_cleanup(clny_ctrl->irq, clny_ctrl);

	kfree(clny_ctrl->buf);
}

struct clourney_enfc_dt {
	struct clourney_enfc clny_ctrl;
	struct clk *clk;
};

static const struct of_device_id clourney_enfc_dt_ids[] = {
	{
		.compatible = "clourney,enfc",
	}, {}
};

MODULE_DEVICE_TABLE(of, clourney_enfc_dt_ids);

static int clourney_enfc_dt_probe(struct platform_device *ofdev)
{
	struct clourney_enfc_dt *dt;
	struct clourney_enfc *clny_ctrl;
	struct clourney_enfc_timings *t ;
	int ret;
	const struct of_device_id *of_id;
	const char *enfc_mode;

	of_id = of_match_device(clourney_enfc_dt_ids, &ofdev->dev);
	if (of_id) {
		ofdev->id_entry = of_id->data;
	} else {
		pr_err("Failed to find the right device id.\n");
		return -ENOMEM;
	}

	dt = devm_kzalloc(&ofdev->dev, sizeof(*dt), GFP_KERNEL);
	if (!dt)
		return -ENOMEM;

	clny_ctrl = &dt->clny_ctrl;
	clny_ctrl->dev = &ofdev->dev;
	clny_ctrl->sdma_enable = CLNY_ENFC_FIFO_MODE;

	clny_ctrl->reg = devm_platform_ioremap_resource(ofdev, 0);
	if (IS_ERR(clny_ctrl->reg))
		return PTR_ERR(clny_ctrl->reg);

	if (!of_property_read_string(ofdev->dev.of_node,
				"clny,enfc-mode", &enfc_mode))
	{
		if (!strncmp("fifo", enfc_mode, 4))
			clny_ctrl->sdma_enable = CLNY_ENFC_FIFO_MODE;
		if (!strncmp("sdma", enfc_mode, 4))
			clny_ctrl->sdma_enable = CLNY_ENFC_SDMA_MODE;
	}

	if(clny_ctrl->sdma_enable)
	{
		clny_ctrl->irq = platform_get_irq(ofdev, 0);
		if (clny_ctrl->irq < 0)
			return clny_ctrl->irq;
		dev_err(clny_ctrl->dev, "IRQ: nr %d\n", clny_ctrl->irq);
	}

	ret = of_property_read_u32(ofdev->dev.of_node, "clock-frequency", &clny_ctrl->clk);
	//dev_info(clny_ctrl->dev, "clny_ctrl->clk = %d\n ret(%d)\r\n", clny_ctrl->clk, ret);
	if (ret < 0)
		clny_ctrl->clk = 250000000;

	clny_ctrl->clk_cycle = NANOSECONDS_CYCLE_TICK / clny_ctrl->clk;

	// Setting Flash Timing Parameters
	t = &clny_ctrl->timings;
	ret = of_property_read_u32_array(ofdev->dev.of_node, "timings", &t->tWHR, NANDCTROLLER_TIMING_NUM);
	if (ret < 0) {
		t->tWHR = t->tRHW = t->tADL = t->tCCS = t->tRR = t->tWB = t->tRWH = t->tRWP = 0xFF;
	}
#if 1
	dev_info(clny_ctrl->dev, "t->tWHR = %d\n", t->tWHR);
	dev_info(clny_ctrl->dev, "t->tRHW = %d\n", t->tRHW);
	dev_info(clny_ctrl->dev, "t->tADL = %d\n", t->tADL);
	dev_info(clny_ctrl->dev, "t->tCCS = %d\n", t->tCCS);
	dev_info(clny_ctrl->dev, "t->tRR = %d\n", t->tRR);
	dev_info(clny_ctrl->dev, "t->tWB = %d\n", t->tWB);
	dev_info(clny_ctrl->dev, "t->tRWH = %d\n", t->tRWH);
	dev_info(clny_ctrl->dev, "t->tRWP = %d\n", t->tRWP);
#endif
	ret = clourney_enfc_init(clny_ctrl);
	if (ret)
		return ret;

	platform_set_drvdata(ofdev, dt);

	clny_enfc_debugfs_init(clny_ctrl);
	return 0;
}

static int clourney_enfc_dt_remove(struct platform_device *ofdev)
{
	struct clourney_enfc_dt *dt = platform_get_drvdata(ofdev);

	clourney_enfc_remove(&dt->clny_ctrl);
	clny_enfc_debugfs_remove(&dt->clny_ctrl);
	return 0;
}

static struct platform_driver clourney_enfc_dt_driver = {
	.probe		= clourney_enfc_dt_probe,
	.remove		= clourney_enfc_dt_remove,
	.driver		= {
		.name	= "clourney-enfc",
		.of_match_table = clourney_enfc_dt_ids,
	},
};

module_platform_driver(clourney_enfc_dt_driver);

MODULE_AUTHOR("Weston Zhu <wez057@clourneysemi.com>");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Driver for Clourney Embeded NAND flash controller");

