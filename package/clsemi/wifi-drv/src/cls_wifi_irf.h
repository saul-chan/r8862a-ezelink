#ifndef __CLS_WIFI_IRF_H__
#define __CLS_WIFI_IRF_H__
#include <net/netlink.h>
#include <net/mac80211.h>
#include <net/cfg80211.h>
#include "cls_wifi_debugfs.h"
#include "cls_wifi_afe_com_top_config.h"
#include "cls_wifi_platform.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define RADIO2STR(radio)    (0==radio?"2G":"5G")

#ifndef NELEMENTS
#define NELEMENTS(array)	(sizeof(array)/sizeof(array[0]))
#endif

#define INVALID_POWER   (-256)

#define TS_NUM	  3

#define CAL_CMD_MAX_PARAM	   14

#define DIF_SET_CTRIM_TIMER_INTERVAL						10
#define DIF_SET_CTRIM_SCH_TIMER_INTERVAL_JIFFIES			(DIF_SET_CTRIM_TIMER_INTERVAL * HZ)

#if !defined(CFG_MERAK3000)
#define DPD_ONLINE_INTERVAL_ADJUST							60
#else
#if defined(CFG_M3K_FPGA)
#define DPD_ONLINE_INTERVAL_ADJUST							(2*160)
#else
#define DPD_ONLINE_INTERVAL_ADJUST							2
#endif
#endif


typedef int (*PF_IRF_CMD)(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw);

struct irf_cmd
{
	const char *name;
	PF_IRF_CMD pfunc_exec;
};

#define IRF_CMD_DEF(cmd_name,exec_func) {cmd_name, &exec_func}
#define IRF_XTAL_CALI_CTRIM 128

#define FS_PATH_MAX_LEN		256
#define CAL_INFO		"cal_info"

#define IRF_TBL_DATA_TOTAL_SIZE         (228 * 1024)
#define IRF_TBL_SIZE                    (100 * 1024)
#define IRF_DATA_SIZE                   (128 * 1024)
#define IRF_TBL_FW_BASE_ADDR            (0xc0801000)
#define IRF_TBL_DFT_IDX                 (0)

/*****************************************************************************
 * Addresses within CLS_WIFI_ADDR_IRF
 *****************************************************************************/
#define IRF_DATA_BASE            0xF80000
#define IRF_DATA_OFFSET          0x19000
#define TX_ZIF_ADDR              0x0
#define TX_ZIF_SIZE              65536
#define TX_ZIF_DATA              PATH_TYPE_IRF, "CFRIn_With_DataIndStartAt2000.dat"

#define TX_ZIF_1_ADDR            0x10000
#define TX_ZIF_1_SIZE            65536
#define TX_ZIF_1_DATA            PATH_TYPE_IRF, "CFRIn_With_DataIndStartAt2000_1.dat"

#define TX_EQ_160M_ADDR          0x10000
#define TX_EQ_160M_SIZE          32768
#define TX_EQ_160M_DATA          PATH_TYPE_IRF, "Tx_160M_PN_AddNoise-17dBfs.dat"

#define TX_EQ_80M_ADDR           0x18000
#define TX_EQ_80M_SIZE           32768
#define TX_EQ_80M_DATA           PATH_TYPE_IRF, "Tx_80M_PN_AddNoise-17dBfs.dat"

#define TX_EQ_20M_ADDR           0x20000
#define TX_EQ_20M_SIZE           32768
#define TX_EQ_20M_DATA           PATH_TYPE_IRF, "Tx_20M_PN_AddNoise-17dBfs.dat"

#define RX_ZIF_160M_ADDR         0x28000
#define RX_ZIF_160M_SIZE         65536
#define RX_ZIF_160M_DATA         PATH_TYPE_IRF, "Phymod160M_ZIF2.dat"

#define RX_ZIF_80M_ADDR          0x38000
#define RX_ZIF_80M_SIZE          65536
#define RX_ZIF_80M_DATA          PATH_TYPE_IRF, "Phymod80M_ZIF2.dat"

#define RX_ZIF_20M_ADDR          0x48000
#define RX_ZIF_20M_SIZE          65536
#define RX_ZIF_20M_DATA          PATH_TYPE_IRF, "Rx_20M_ZIF_test.dat"

#define RX_EQ_160M_ADDR          0x58000
#define RX_EQ_160M_SIZE          32768
#define RX_EQ_160M_DATA          PATH_TYPE_IRF, "Rx_160M_PN_AddNoise-17dBfs.dat"

#define RX_EQ_80M_ADDR           0x60000
#define RX_EQ_80M_SIZE           32768
#define RX_EQ_80M_DATA           PATH_TYPE_IRF, "Rx_80M_PN_AddNoise-17dBfs.dat"

#define RX_EQ_20M_ADDR           0x68000
#define RX_EQ_20M_SIZE           32768
#define RX_EQ_20M_DATA           PATH_TYPE_IRF, "Rx_20M_PN_AddNoise-17dBfs.dat"

#define RX_ZIF_40M_ADDR          0x70000
#define RX_ZIF_40M_SIZE          65536
#define RX_ZIF_40M_DATA          PATH_TYPE_IRF, "Phymod40M_ZIF.dat"

#define SAWTOOTH_ADDR             0x0
#define SAWTOOTH_SIZE            131072
#define TX_SAWTOOTH_DATA         "tx_sawtooth.dat"
#define RX_SAWTOOTH_160M_DATA    "rx_sawtooth_160M.dat"
#define RX_SAWTOOTH_80M_DATA     "rx_sawtooth_80M.dat"
#define RX_SAWTOOTH_20M_DATA     "rx_sawtooth_20M.dat"


/*****************************************************************************
 * Addresses within CLS_WIFI_ADDR_IRF_TBL
 *****************************************************************************/
#define IRF_TABLE_BASE			0x1780000

#define DIF_EQ_2G_DATA			PATH_TYPE_CAL, "dif_eq_2G.bin"
#define DIF_EQ_2G_ADDR_M2K		0x0
#define DIF_EQ_2G_SIZE_M2K		7168
#define DIF_EQ_2G_ADDR_D2K		0x0
#define DIF_EQ_2G_SIZE_D2K		20480

#define TX_LEVEL_2G_DATA		PATH_TYPE_IRF, "tx_gain_level_2G.bin"
#define TX_LEVEL_2G_ADDR_M3K		0
#define TX_LEVEL_2G_SIZE_M3K		1024
#define TX_LEVEL_2G_ADDR_M2K		0x1c00
#define TX_LEVEL_2G_SIZE_M2K		1024
#define TX_LEVEL_2G_ADDR_D2K		0x5000
#define TX_LEVEL_2G_SIZE_D2K		1024

#define FB_LEVEL_2G_DATA		PATH_TYPE_IRF, "fb_gain_level_2G.bin"
#define FB_LEVEL_2G_ADDR_M3K		0x400
#define FB_LEVEL_2G_SIZE_M3K		1024
#define FB_LEVEL_2G_ADDR_M2K		0x2000
#define FB_LEVEL_2G_SIZE_M2K		1024
#define FB_LEVEL_2G_ADDR_D2K		0x5400
#define FB_LEVEL_2G_SIZE_D2K		1024

#define RX_LEVEL_2G_DATA		PATH_TYPE_IRF, "rx_gain_level_2G.bin"
#define RX_LEVEL_2G_CALI_DATA	PATH_TYPE_CAL, "rx_gain_level_2G.bin"
#define RX_LEVEL_2G_ADDR_M3K		0x800
#define RX_LEVEL_2G_SIZE_M3K		3072
#define RX_LEVEL_2G_ADDR_M2K		0x2400
#define RX_LEVEL_2G_SIZE_M2K		3072
#define RX_LEVEL_2G_ADDR_D2K		0x5800
#define RX_LEVEL_2G_SIZE_D2K		3072
#define RX_LEVEL_2G_DATA_FIL_NAM	"rx_gain_level_2G.bin"

#define TX_FCOMP_2G_DATA		PATH_TYPE_CAL, "tx_gain_fcomp_2G.bin"
#define TX_FCOMP_2G_ADDR_M3K		0x1400
#define TX_FCOMP_2G_SIZE_M3K		1024
#define TX_FCOMP_2G_ADDR_M2K		0x3000
#define TX_FCOMP_2G_SIZE_M2K		1024
#define TX_FCOMP_2G_ADDR_D2K		0x6400
#define TX_FCOMP_2G_SIZE_D2K		1024

#define FB_FCOMP_2G_DATA		PATH_TYPE_CAL, "fb_gain_fcomp_2G.bin"
#define FB_FCOMP_2G_ADDR_M3K		0x1800
#define FB_FCOMP_2G_SIZE_M3K		1024
#define FB_FCOMP_2G_ADDR_M2K		0x3400
#define FB_FCOMP_2G_SIZE_M2K		1024
#define FB_FCOMP_2G_ADDR_D2K		0x6800
#define FB_FCOMP_2G_SIZE_D2K		1024

#define RX_FCOMP_2G_DATA		PATH_TYPE_CAL, "rx_gain_fcomp_2G.bin"
#define RX_FCOMP_2G_ADDR_M3K		0x1c00
#define RX_FCOMP_2G_SIZE_M3K		1024
#define RX_FCOMP_2G_ADDR_M2K		0x3800
#define RX_FCOMP_2G_SIZE_M2K		1024
#define RX_FCOMP_2G_ADDR_D2K		0x6c00
#define RX_FCOMP_2G_SIZE_D2K		1024

/*
#define RX_LEVEL_COMP_2G_DATA		PATH_TYPE_CAL, "rx_gain_level_comp_2G.bin"
#define RX_LEVEL_COMP_2G_ADDR_M3K	0x2000
#define RX_LEVEL_COMP_2G_SIZE_M3K	768
#define RX_LEVEL_COMP_2G_ADDR_M2K	0x3c00
#define RX_LEVEL_COMP_2G_SIZE_M2K	768
#define RX_LEVEL_COMP_2G_ADDR_D2K	0x7000
#define RX_LEVEL_COMP_2G_SIZE_D2K	768

#define RX_FEM_COMP_2G_DATA		PATH_TYPE_CAL, "rx_gain_fem_comp_2G.bin"
#define RX_FEM_COMP_2G_ADDR_M3K		0x2300
#define RX_FEM_COMP_2G_SIZE_M3K		256
#define RX_FEM_COMP_2G_ADDR_M2K		0x3f00
#define RX_FEM_COMP_2G_SIZE_M2K		256
#define RX_FEM_COMP_2G_ADDR_D2K		0x7300
#define RX_FEM_COMP_2G_SIZE_D2K		256
*/

#define TX_TCOMP_2G_DATA		PATH_TYPE_IRF, "tx_gain_tcomp_2G.bin"
#define TX_TCOMP_2G_ADDR_M3K		0x2400
#define TX_TCOMP_2G_SIZE_M3K		2048
#define TX_TCOMP_2G_ADDR_M2K		0x4000
#define TX_TCOMP_2G_SIZE_M2K		2048
#define TX_TCOMP_2G_ADDR_D2K		0x7400
#define TX_TCOMP_2G_SIZE_D2K		2048

#define FB_TCOMP_2G_DATA		PATH_TYPE_IRF, "fb_gain_tcomp_2G.bin"
#define FB_TCOMP_2G_ADDR_M3K		0x2c00
#define FB_TCOMP_2G_SIZE_M3K		2048
#define FB_TCOMP_2G_ADDR_M2K		0x4800
#define FB_TCOMP_2G_SIZE_M2K		2048
#define FB_TCOMP_2G_ADDR_D2K		0x7c00
#define FB_TCOMP_2G_SIZE_D2K		2048

#define RX_TCOMP_2G_DATA		PATH_TYPE_IRF, "rx_gain_tcomp_2G.bin"
#define RX_TCOMP_2G_ADDR_M3K		0x3400
#define RX_TCOMP_2G_SIZE_M3K		2048
#define RX_TCOMP_2G_ADDR_M2K		0x5000
#define RX_TCOMP_2G_SIZE_M2K		2048
#define RX_TCOMP_2G_ADDR_D2K		0x8400
#define RX_TCOMP_2G_SIZE_D2K		2048

#define DIF_EQ_5G_DATA			PATH_TYPE_CAL, "dif_eq_5G.bin"
#define DIF_EQ_5G_ADDR_M2K		0x5800
#define DIF_EQ_5G_SIZE_M2K		14336
#define DIF_EQ_5G_ADDR_D2K		0x8c00
#define DIF_EQ_5G_SIZE_D2K		163840

#define TX_LEVEL_5G_DATA		PATH_TYPE_IRF, "tx_gain_level_5G.bin"
#define TX_LEVEL_5G_ADDR_M3K		0x3c00
#define TX_LEVEL_5G_SIZE_M3K		1024
#define TX_LEVEL_5G_ADDR_M2K		0x9000
#define TX_LEVEL_5G_SIZE_M2K		1024
#define TX_LEVEL_5G_ADDR_D2K		0x30c00
#define TX_LEVEL_5G_SIZE_D2K		1024

#define FB_LEVEL_5G_DATA		PATH_TYPE_IRF, "fb_gain_level_5G.bin"
#define FB_LEVEL_5G_ADDR_M3K		0x4000
#define FB_LEVEL_5G_SIZE_M3K		1024
#define FB_LEVEL_5G_ADDR_M2K		0x9400
#define FB_LEVEL_5G_SIZE_M2K		1024
#define FB_LEVEL_5G_ADDR_D2K		0x31000
#define FB_LEVEL_5G_SIZE_D2K		1024

#define RX_LEVEL_5G_DATA		PATH_TYPE_IRF, "rx_gain_level_5G.bin"
#define RX_LEVEL_5G_CALI_DATA	PATH_TYPE_CAL, "rx_gain_level_5G.bin"
#define RX_LEVEL_5G_ADDR_M3K		0x4400
#define RX_LEVEL_5G_SIZE_M3K		3072
#define RX_LEVEL_5G_ADDR_M2K		0x9800
#define RX_LEVEL_5G_SIZE_M2K		3072
#define RX_LEVEL_5G_ADDR_D2K		0x31400
#define RX_LEVEL_5G_SIZE_D2K		3072
#define RX_LEVEL_5G_DATA_FIL_NAM	"rx_gain_level_5G.bin"

#define TX_FCOMP_5G_DATA		PATH_TYPE_CAL, "tx_gain_fcomp_5G.bin"
#define TX_FCOMP_5G_ADDR_M3K		0x5000
#define TX_FCOMP_5G_SIZE_M3K		1024
#define TX_FCOMP_5G_ADDR_M2K		0xa400
#define TX_FCOMP_5G_SIZE_M2K		1024
#define TX_FCOMP_5G_ADDR_D2K		0x32000
#define TX_FCOMP_5G_SIZE_D2K		1024

#define TX_GAIN_ERR_5G_DATA		PATH_TYPE_CAL, "tx_gain_err_5G.bin"
#define TX_GAIN_ERR_5G_ADDR_M3K		0x5400
#define TX_GAIN_ERR_5G_SIZE_M3K		1024
#define TX_GAIN_ERR_5G_ADDR_M2K		0xa800
#define TX_GAIN_ERR_5G_SIZE_M2K		1024
#define TX_GAIN_ERR_5G_ADDR_D2K		0x32400
#define TX_GAIN_ERR_5G_SIZE_D2K		1024

#define TX_GAIN_ERR_2G_DATA		PATH_TYPE_CAL, "tx_gain_err_2G.bin"
#define TX_GAIN_ERR_2G_ADDR_M3K		0x5800
#define TX_GAIN_ERR_2G_SIZE_M3K		1024
#define TX_GAIN_ERR_2G_ADDR_M2K		0xac00
#define TX_GAIN_ERR_2G_SIZE_M2K		1024
#define TX_GAIN_ERR_2G_ADDR_D2K		0x32800
#define TX_GAIN_ERR_2G_SIZE_D2K		1024

#define FB_FCOMP_5G_DATA		PATH_TYPE_CAL, "fb_gain_fcomp_5G.bin"
#define FB_FCOMP_5G_ADDR_M3K		0x5c00
#define FB_FCOMP_5G_SIZE_M3K		1024
#define FB_FCOMP_5G_ADDR_M2K		0xb000
#define FB_FCOMP_5G_SIZE_M2K		1024
#define FB_FCOMP_5G_ADDR_D2K		0x32c00
#define FB_FCOMP_5G_SIZE_D2K		1024

#define FB_GAIN_ERR_5G_DATA		PATH_TYPE_CAL, "fb_gain_err_5G.bin"
#define FB_GAIN_ERR_5G_ADDR_M3K		0x6000
#define FB_GAIN_ERR_5G_SIZE_M3K		1024
#define FB_GAIN_ERR_5G_ADDR_M2K		0xb400
#define FB_GAIN_ERR_5G_SIZE_M2K		1024
#define FB_GAIN_ERR_5G_ADDR_D2K		0x33000
#define FB_GAIN_ERR_5G_SIZE_D2K		1024

#define FB_GAIN_ERR_2G_DATA		PATH_TYPE_CAL, "fb_gain_err_2G.bin"
#define FB_GAIN_ERR_2G_ADDR_M3K		0x6400
#define FB_GAIN_ERR_2G_SIZE_M3K		1024
#define FB_GAIN_ERR_2G_ADDR_M2K		0xb800
#define FB_GAIN_ERR_2G_SIZE_M2K		1024
#define FB_GAIN_ERR_2G_ADDR_D2K		0x33400
#define FB_GAIN_ERR_2G_SIZE_D2K		1024

#define RX_FCOMP_5G_DATA		PATH_TYPE_CAL, "rx_gain_fcomp_5G.bin"
#define RX_FCOMP_5G_ADDR_M3K		0x6800
#define RX_FCOMP_5G_SIZE_M3K		2048
#define RX_FCOMP_5G_ADDR_M2K		0xbc00
#define RX_FCOMP_5G_SIZE_M2K		2048
#define RX_FCOMP_5G_ADDR_D2K		0x33800
#define RX_FCOMP_5G_SIZE_D2K		5120

/*
#define RX_LEVEL_COMP_5G_DATA		PATH_TYPE_CAL, "rx_gain_level_comp_5G.bin"
#define RX_LEVEL_COMP_5G_ADDR_M3K	0x7000
#define RX_LEVEL_COMP_5G_SIZE_M3K	768
#define RX_LEVEL_COMP_5G_ADDR_M2K	0xc400
#define RX_LEVEL_COMP_5G_SIZE_M2K	768
#define RX_LEVEL_COMP_5G_ADDR_D2K	0x34c00
#define RX_LEVEL_COMP_5G_SIZE_D2K	768

#define RX_FEM_COMP_5G_DATA		PATH_TYPE_CAL, "rx_gain_fem_comp_5G.bin"
#define RX_FEM_COMP_5G_ADDR_M3K		0x7300
#define RX_FEM_COMP_5G_SIZE_M3K		256
#define RX_FEM_COMP_5G_ADDR_M2K		0xc700
#define RX_FEM_COMP_5G_SIZE_M2K		256
#define RX_FEM_COMP_5G_ADDR_D2K		0x34f00
#define RX_FEM_COMP_5G_SIZE_D2K		256
*/

#define TX_TCOMP_5G_DATA		PATH_TYPE_IRF, "tx_gain_tcomp_5G.bin"
#define TX_TCOMP_5G_ADDR_M3K		0x7400
#define TX_TCOMP_5G_SIZE_M3K		2048
#define TX_TCOMP_5G_ADDR_M2K		0xc800
#define TX_TCOMP_5G_SIZE_M2K		2048
#define TX_TCOMP_5G_ADDR_D2K		0x35000
#define TX_TCOMP_5G_SIZE_D2K		2048

#define FB_TCOMP_5G_DATA		PATH_TYPE_IRF, "fb_gain_tcomp_5G.bin"
#define FB_TCOMP_5G_ADDR_M3K		0x7c00
#define FB_TCOMP_5G_SIZE_M3K		2048
#define FB_TCOMP_5G_ADDR_M2K		0xd000
#define FB_TCOMP_5G_SIZE_M2K		2048
#define FB_TCOMP_5G_ADDR_D2K		0x35800
#define FB_TCOMP_5G_SIZE_D2K		2048

#define RX_TCOMP_5G_DATA		PATH_TYPE_IRF, "rx_gain_tcomp_5G.bin"
#define RX_TCOMP_5G_ADDR_M3K		0x8400
#define RX_TCOMP_5G_SIZE_M3K		2048
#define RX_TCOMP_5G_ADDR_M2K		0xd800
#define RX_TCOMP_5G_SIZE_M2K		2048
#define RX_TCOMP_5G_ADDR_D2K		0x36000
#define RX_TCOMP_5G_SIZE_D2K		2048

#define PLL_PLAN_2G_DATA		PATH_TYPE_IRF, "frequency_plan_config_2G.bin"
#define PLL_PLAN_2G_ADDR_M3K		0x8c00
#define PLL_PLAN_2G_SIZE_M3K		2048
#define PLL_PLAN_2G_ADDR_M2K		0xe000
#define PLL_PLAN_2G_SIZE_M2K		2048
#define PLL_PLAN_2G_ADDR_D2K		0x36800
#define PLL_PLAN_2G_SIZE_D2K		2048

#define PLL_PLAN_5G_DATA		PATH_TYPE_IRF, "frequency_plan_config_5G.bin"
#define PLL_PLAN_5G_ADDR_M3K		0x9400
#define PLL_PLAN_5G_SIZE_M3K		20480
#define PLL_PLAN_5G_ADDR_M2K		0xe800
#define PLL_PLAN_5G_SIZE_M2K		20480
#define PLL_PLAN_5G_ADDR_D2K		0x37000
#define PLL_PLAN_5G_SIZE_D2K		18432

#define RX_DC_OFFSET_2G_LOW_DATA	PATH_TYPE_CAL, "rx_dcoc_2G_low.bin"
#define RX_DC_OFFSET_2G_LOW_ADDR_M3K	0xe400
#define RX_DC_OFFSET_2G_LOW_SIZE_M3K	1024
#define RX_DC_OFFSET_2G_LOW_ADDR_M2K	0x13800
#define RX_DC_OFFSET_2G_LOW_SIZE_M2K	1024
#define RX_DC_OFFSET_2G_LOW_ADDR_D2K	0x3cb40
/* two bandwiths and two antennas */
#define RX_DC_OFFSET_2G_LOW_SIZE_D2K	(48 + 4*50*2*2)

#define RX_DC_OFFSET_2G_HIGH_DATA	PATH_TYPE_CAL, "rx_dcoc_2G_high.bin"
#define RX_DC_OFFSET_2G_HIGH_ADDR_M3K	0xe800
#define RX_DC_OFFSET_2G_HIGH_SIZE_M3K	1024
#define RX_DC_OFFSET_2G_HIGH_ADDR_M2K	(RX_DC_OFFSET_2G_LOW_ADDR_M2K + RX_DC_OFFSET_2G_LOW_SIZE_M2K)
#define RX_DC_OFFSET_2G_HIGH_SIZE_M2K	1024
#define RX_DC_OFFSET_2G_HIGH_ADDR_D2K	(RX_DC_OFFSET_2G_LOW_ADDR_D2K + RX_DC_OFFSET_2G_LOW_SIZE_D2K)
/* two bandwiths and two antennas */
#define RX_DC_OFFSET_2G_HIGH_SIZE_D2K	(48 + 4*50*2*2)

#define RX_DC_OFFSET_5G_LOW_DATA	PATH_TYPE_CAL, "rx_dcoc_5G_low.bin"
#define RX_DC_OFFSET_5G_LOW_ADDR_M3K	0xec00
#define RX_DC_OFFSET_5G_LOW_SIZE_M3K	5120
#define RX_DC_OFFSET_5G_LOW_ADDR_M2K	0x14000
#define RX_DC_OFFSET_5G_LOW_SIZE_M2K	5120
#define RX_DC_OFFSET_5G_LOW_ADDR_D2K	0x3d1e0
/* four bandwiths and two antennas and three freq range*/
#define RX_DC_OFFSET_5G_LOW_SIZE_D2K	(48 + 4*50*4*2*3)

#define RX_DC_OFFSET_5G_HIGH_DATA	PATH_TYPE_CAL, "rx_dcoc_5G_high.bin"
#define RX_DC_OFFSET_5G_HIGH_ADDR_M3K	0x10000
#define RX_DC_OFFSET_5G_HIGH_SIZE_M3K	5120
#define RX_DC_OFFSET_5G_HIGH_ADDR_M2K	(RX_DC_OFFSET_5G_LOW_ADDR_M2K + RX_DC_OFFSET_5G_LOW_SIZE_M2K)
#define RX_DC_OFFSET_5G_HIGH_SIZE_M2K	5120
/* four bandwiths and two antennas and three freq range*/
#define RX_DC_OFFSET_5G_HIGH_ADDR_D2K	(RX_DC_OFFSET_5G_LOW_ADDR_D2K + RX_DC_OFFSET_5G_LOW_SIZE_D2K)
#define RX_DC_OFFSET_5G_HIGH_SIZE_D2K	(48 + 4*50*4*2*3)

#define EQ_CALI_XTAL_CTRIM_DATA		PATH_TYPE_CAL, "eq_cali_xtal_ctrim.bin"
#define EQ_CALI_XTAL_CTRIM_ADDR_M3K	0x11400
#define EQ_CALI_XTAL_CTRIM_SIZE_M3K	49
#define EQ_CALI_XTAL_CTRIM_ADDR_M2K	0x16800
#define EQ_CALI_XTAL_CTRIM_SIZE_M2K	49
#define EQ_CALI_XTAL_CTRIM_ADDR_D2K	0x3f7c0
#define EQ_CALI_XTAL_CTRIM_SIZE_D2K	49

#define EQ_CALI_XTAL_TEMP_DATA		PATH_TYPE_CAL, "eq_cali_xtal_temp.bin"
#define EQ_CALI_XTAL_TEMP_ADDR_M3K	0x11431
#define EQ_CALI_XTAL_TEMP_SIZE_M3K	(48 + 100)
#define EQ_CALI_XTAL_TEMP_ADDR_M2K	(EQ_CALI_XTAL_CTRIM_ADDR_M2K + EQ_CALI_XTAL_CTRIM_SIZE_M2K)
#define EQ_CALI_XTAL_TEMP_SIZE_M2K	(48 + 100)
#define EQ_CALI_XTAL_TEMP_ADDR_D2K	(EQ_CALI_XTAL_CTRIM_ADDR_D2K + EQ_CALI_XTAL_CTRIM_SIZE_D2K)
#define EQ_CALI_XTAL_TEMP_SIZE_D2K	(48 + 100)

#define IRF_RESERVED_ADDR_M3K		0x114c5
#define IRF_RESERVED_SIZE_M3K		27451
#define IRF_RESERVED_ADDR_M2K		0x168c5
#define IRF_RESERVED_SIZE_M2K		5947
#define IRF_RESERVED_ADDR_D2K		0x3f880
#define IRF_RESERVED_SIZE_D2K		(241 * 1024)

#define IRF_SHARE_MEM_ADDR_M3K		0x18000
#define IRF_SHARE_MEM_SIZE_M3K		(4 * 1024)
#define IRF_SHARE_MEM_ADDR_M2K		0x18000
#define IRF_SHARE_MEM_SIZE_M2K		(4 * 1024)
#define IRF_SHARE_MEM_ADDR_D2K		0x7c000
#define IRF_SHARE_MEM_SIZE_D2K		(16 * 1024)

#define DATA_OK		0x5aa5
#define DATA_NOK	0x1234

#define DETRESULT_9CFG                      0x5c
#define DET_READY_WIDTH                     1
#define DET_READY_LSB                       0
#define DET_READY_MASK                      (((1<<DET_READY_WIDTH) - 1)<<DET_READY_LSB)
#define DET_PULSE_NUM_RECORD_WIDTH          7
#define DET_PULSE_NUM_RECORD_LSB            1
#define DET_PULSE_NUM_RECORD_MASK           (((1<<DET_PULSE_NUM_RECORD_WIDTH) - 1)<<DET_PULSE_NUM_RECORD_LSB)

#define RADARDET_INTERRUPT_RPT              0x68

#define DETMEM_OPT_1CFG                     0x84
#define RESULT_MEM_ACCESS_MODE_WIDTH        1
#define RESULT_MEM_ACCESS_MODE_LSB          0
#define RESULT_MEM_ACCESS_MODE_MASK         (((1<<RESULT_MEM_ACCESS_MODE_WIDTH) - 1)<<RESULT_MEM_ACCESS_MODE_LSB)


typedef enum
{
	IRF_DATA_DIF_EQ = 0,
	IRF_DATA_TX_LEVEL,
	IRF_DATA_RX_LEVEL,
	IRF_DATA_FB_LEVEL,
	IRF_DATA_PLL_PLAN,
	IRF_DATA_TX_FCOMP,	 //Frequency compensation
	IRF_DATA_RX_FCOMP,	 //Frequency compensation
	IRF_DATA_FB_FCOMP,	 //Frequency compensation
	IRF_DATA_TX_TCOMP,	 //Temperature compensation
	IRF_DATA_RX_TCOMP,	 //Temperature compensation
	IRF_DATA_FB_TCOMP,	 //Temperature compensation
	IRF_DATA_TX_ZIF,
	IRF_DATA_TX_EQ_160M,
	IRF_DATA_TX_EQ_80M,
	IRF_DATA_TX_EQ_20M,
	IRF_DATA_RX_ZIF_160M,
	IRF_DATA_RX_ZIF_80M,
	IRF_DATA_RX_ZIF_20M,
	IRF_DATA_RX_EQ_160M,
	IRF_DATA_RX_EQ_80M,
	IRF_DATA_RX_EQ_20M,
	IRF_DATA_FB_DC_OFFSET_2G_CH0_LOW,
	IRF_DATA_FB_DC_OFFSET_2G_CH0_HIGH,
	IRF_DATA_FB_DC_OFFSET_2G_CH1_LOW,
	IRF_DATA_FB_DC_OFFSET_2G_CH1_HIGH,
	IRF_DATA_FB_DC_OFFSET_5G_CH0_LOW,
	IRF_DATA_FB_DC_OFFSET_5G_CH0_HIGH,
	IRF_DATA_FB_DC_OFFSET_5G_CH1_LOW,
	IRF_DATA_FB_DC_OFFSET_5G_CH1_HIGH,
	IRF_DATA_RX_DC_OFFSET_2G_LOW,
	IRF_DATA_RX_DC_OFFSET_2G_HIGH,
	IRF_DATA_RX_DC_OFFSET_5G_LOW,
	IRF_DATA_RX_DC_OFFSET_5G_HIGH,
	IRF_DATA_EQ_CALI_XTAL_CTRIM,
	IRF_DATA_EQ_CALI_XTAL_TEMP,
	IRF_DATA_FB_GAIN_ERR,
	IRF_DATA_RX_ZIF_40M,
	IRF_DATA_TX_GAIN_ERR,
#if defined(CFG_MERAK3000)
	IRF_DATA_TX_1_ZIF,
#endif
	IRF_DATA_RESERVED,
	IRF_DATA_TYPE_MAX
}EN_IRF_DATA;

enum dif_online_cali_type{
	DIF_ZIF_TASK = 0,
	DIF_FBDELAY_TASK,
	DIF_PD_TASK,
};

enum XTAL_CAL_STAUS
{
	IRF_XTAL_CAL_STATUS_IN_PROGRESS = 0,
	IRF_XTAL_CAL_STATUS_FAIL,
	IRF_XTAL_CAL_STATUS_DONE,
	IRF_XTAL_CAL_STATUS_MAX
};

enum XTAL_CAL_MODE {
	IRF_XTAL_CAL_START = 0,
	IRF_XTAL_CAL_FREQ_MODE,
	IRF_XTAL_CAL_PPM_MODE,
	IRF_XTAL_CAL_MODE_MAX
};

typedef enum
{
	IRF_AFE_INIT = 0,
	IRF_AFE_COM_TOP,
	IRF_AFE_PLL,
	IRF_AFE_LO_GEN,
	IRF_AFE_ABB_CLK,
	IRF_AFE_IQ_GEN,
	IRF_AFE_RF,
	IRF_AFE_ABB,
	IRF_AFE_ALL_CALI,
	IRF_AFE_CH_BW_SWITCH,
	IRF_AFE_DIG_CTRL,
} IRF_AFE_MODULE_LST;

typedef enum
{
	IRF_AFE_INIT_RESET,
	IRF_AFE_INIT_MAIN_PROC,
	IRF_AFE_INIT_CLK_2_DIF,
	IRF_AFE_INIT_CLOSE,
	IRF_AFE_INIT_RESUME,
	IRF_AFE_COM_TOP_BANDGAP,
	IRF_AFE_COM_TOP_RCTUNE,
	IRF_AFE_COM_TOP_RTUNE,
	IRF_AFE_COM_TOP_PSENSOR,
	IRF_AFE_COM_TOP_IPTAT,
	IRF_AFE_PLL_SA,
	IRF_AFE_PLL_2G,
	IRF_AFE_PLL_5G,
	IRF_AFE_LO_GEN_INIT,
	IRF_AFE_LO_GEN_CALI,
	IRF_AFE_ABB_CLK_RX,
	IRF_AFE_ABB_CLK_TX,
	IRF_AFE_ABB_CLK_FB,
	IRF_AFE_IQ_GEN_CALI,
	IRF_AFE_RF_RX,
	IRF_AFE_RF_TX,
	IRF_AFE_RF_FB,
	IRF_AFE_RF_IQCAL,
	IRF_AFE_RF_DPD,
	IRF_AFE_ABB_RX,
	IRF_AFE_ABB_TX,
	IRF_AFE_ABB_FB,
	IRF_AFE_ABB_RX_SFT,
	IRF_AFE_ABB_TX_SFT,
	IRF_AFE_ABB_FB_SFT
} IRF_AFE_SUB_MODULE_LST;

enum
{
	IRF_RX_CALI_GAIN_LEVEL = 0,
	IRF_RX_CALI_GAIN_FREQ_COMP,
	IRF_RX_CALI_PREP,
	// MAX number of types
	IRF_RX_CALI_MAX
};

enum IRF_FILE_PATH_TYPE {
	PATH_TYPE_CAL = 0,
	PATH_TYPE_IRF,
};

struct irf_file_list
{
	enum IRF_FILE_PATH_TYPE path_type;
	char *file_name;
	uint32_t data_type;
	uint32_t load_addr;
	uint32_t load_size;
};

struct irf_data{
	uint32_t data_addr;
	uint32_t data_size;
	uint32_t load_flag;
};

struct irf_share_data
{
	volatile int8_t cos_sin_table_init;
	volatile int8_t temperature[TS_NUM];
	uint8_t tcomp_en;
	uint32_t rctune;
	uint32_t rtune;
	psensor_info_T psensor_type[PS_ARRAY_TYPE_NUM];
	uint8_t afe_2g_enable;
	uint8_t afe_5g_enable;

	int bw_2g;
	int bw_5g;
	int chan_ieee_2g;
	int chan_ieee_5g;
	int ant_mask;
	int afe_dig_ctrl;
	int afe_fem_en;
	int afe_feat_mask;
	bool dpd_cca_disable;
	bool low_pwr_en;

	int dpd_tx_power_5g;
	int dpd_tx_power_2g_20;
	int dpd_tx_power_2g_40;

	uint8_t ant_num_2g;
	uint8_t ant_num_5g;
	bool ibex_en;

	float cos_table[256];
	float sin_table[256];

	struct irf_data irf_data_2G[IRF_DATA_TYPE_MAX];
	struct irf_data irf_data_5G[IRF_DATA_TYPE_MAX];

};

#define IRF_MAX_NODE    4
enum IRF_SND_SMP_MOD
{
	IRF_DEF_SND_SMP_MOD = 0,
	IRF_COM_SND_MOD,
	IRF_DDR_SND_MOD,
	IRF_COM_SMP_MOD,
	IRF_DDR_SMP_MOD,
};

struct irf_node_ram_map
{
	uint32_t ram_bitmap;
	uint32_t snd_smp_mod;
};

struct temp_ctrim {
	int8_t temp;
	int8_t ctrim_offset;
};

/* IRF reserved DDR memory management for send and sample data */
#if defined(CFG_M3K_FPGA)
#define IRF_RAM_BASE_ADDR_M3K          0xa8700000
#define IRF_RAM_BLOCK_SIZE_M3K         0x80000
#else
#define IRF_RAM_BASE_ADDR_M3K		0
#define IRF_RAM_BLOCK_SIZE_M3K		0x32000
#endif
#define IRF_RAM_BASE_ADDR_M2K		0
#define IRF_RAM_BLOCK_SIZE_M2K		0x32000
#define IRF_RAM_BASE_ADDR_D2K		(0x8000000)
#define IRF_RAM_BLOCK_SIZE_D2K		0x800000
#define IRF_RAM_IDLE        0
#define IRF_RAM_BUSY        1

struct irf_ram_block
{
	uint32_t ram_base;
	uint32_t ram_size;
	uint32_t status;
	uint32_t snd_smp_mod;
};

enum IRF_MEM_TYP
{
	IRF_RESV_MEM = 0,
	IRF_SND_SMP_MEM,
#if defined(CFG_M3K_FPGA)
	IRF_SND_SMP_MEM_FPGA,
#endif
};

enum IRF_CALI_STAUS {
	IRF_CALI_STATUS_IDLE = 0,
	IRF_CALI_STATUS_BUSY,
	IRF_CALI_STATUS_DONE,
	IRF_CALI_STATUS_FAIL,
	IRF_CALI_STATUS_MAX
};



extern int cls_tsens_get_value(int *rdata);

extern int irf_cmd_distribute(struct cls_wifi_hw *cls_wifi_hw,char *cmd_str);
extern int irf_dif_eq(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw);
int irf_get_fullname(struct cls_wifi_plat *plat, char *full_name,
		enum IRF_FILE_PATH_TYPE path_type, char *file_name);
extern int irf_load_data(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw);
extern int irf_run_task(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw);
extern int irf_set_mode(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw);
extern int irf_show_table(int argc, char *argv[], struct cls_wifi_hw *cls_wifi_hw);
extern int irf_str2val(char *str);
extern int cls_wifi_irf_init(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx);
extern void cls_wifi_irf_deinit(struct cls_wifi_hw *cls_wifi_hw);
#ifdef CFG_PCIE_SHM
extern int cls_wifi_irf_outbound_init(struct cls_wifi_plat *cls_wifi_plat);
#endif
extern int32_t irf_smp_send_ram_init(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx);
extern int32_t irf_smp_send_ram_malloc(struct cls_wifi_plat *cls_wifi_plat,
		uint32_t node, uint32_t snd_smp_mod, uint32_t length, uint32_t *phy_addr);
extern int32_t irf_smp_send_ram_free(struct cls_wifi_plat *cls_wifi_plat, uint32_t node_msk);
int32_t irf_set_snd_smp_mod(uint32_t node, uint32_t snd_smp_mod);
int32_t irf_get_snd_smp_mod(uint32_t node, uint32_t *snd_smp_mod);
extern int cls_wifi_afe_xtal_ctrim_set(struct cls_wifi_plat *cls_wifi_plat);
extern void cls_wifi_afe_xtal_ctrim_unset(struct cls_wifi_plat *cls_wifi_plat);
extern int cls_wifi_dpd_online_schedule_init(struct cls_wifi_hw *cls_wifi_hw);
extern void cls_wifi_dpd_online_schedule_deinit(struct cls_wifi_hw *cls_wifi_hw);
extern int irf_load_table(uint32_t radio_id, uint32_t addr, struct cls_wifi_plat *cls_wifi_plat, int msg_flag);
extern int irf_load_cfr_data(uint32_t radio_id, uint32_t addr, struct cls_wifi_hw *cls_wifi_hw);
void cls_wifi_start_dpd_fbdelay_cali(struct cls_wifi_hw *cls_wifi_hw);
void cls_wifi_start_dpd_cali(struct cls_wifi_hw *cls_wifi_hw);


extern uint8_t dcoc_status[2];
extern uint8_t dif_cali_status;
extern uint8_t fb_err_cali_status;
extern uint8_t rx_cali_status;
extern uint8_t tx_err_cali_status;
extern u32 *cfr_data_buf;

int clsemi_vndr_cmds_set_cca_cs_thr(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int len);
int clsemi_vndr_cmds_get_cca_cs_thr(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int len);
int clsemi_vndr_cmds_set_cca_ed_thr(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int len);
int clsemi_vndr_cmds_get_cca_ed_thr(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int len);

extern int irf_load_boot_cali_data(struct cls_wifi_hw *cls_wifi_hw);
extern void cls_wifi_irf_save_work_init(struct cls_wifi_hw *cls_wifi_hw);
extern void cls_wifi_irf_save_work_deinit(struct cls_wifi_hw *cls_wifi_hw);
extern u8 irf_get_curr_bw(struct cls_wifi_hw *cls_wifi_hw);
extern void cls_wifi_csa_delay_cali_init(struct cls_wifi_hw *cls_wifi_hw);
extern void cls_wifi_csa_delay_cali_deinit(struct cls_wifi_hw *cls_wifi_hw);
extern int cls_wifi_irf_smp_send_ram_init(struct cls_wifi_plat *cls_wifi_plat, u8 radio_idx);
extern int irf_load_rx_gain_dcoc_tbl(uint32_t radio_id, uint32_t addr,
		struct cls_wifi_plat *cls_wifi_plat, int msg_flag);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __CLS_WIFI_IRF_H__ */
