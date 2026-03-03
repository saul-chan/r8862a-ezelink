/*
 * Motorcomm YT921x DSA Switch driver
 * Copyright (c), 2022, Motorcomm Electronic Technology Co.,Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * APPLICABLE DEVICES:
 * YT9215 series
 * YT9218 series
 */

#include <linux/if_bridge.h>
#include <linux/phylink.h>
#include <linux/delay.h>
#include <net/dsa.h>
#include "motorcomm_mdio.h"
#include "yt921x.h"
#ifndef LINUX_VERSION_CODE
#include <linux/version.h>
#else
#define KERNEL_VERSION(a, b, c) (((a) << 16) + ((b) << 8) + (c))
#endif

/* register define -- start */
/* internal mdio controller */
#define YT921X_CHIP_ID_REG                                  (0x80008)
#define MAX_BUSYING_WAIT_TIME                       (10U)
#define YT921X_MDIOx_CTRL_BASE(n)                  (0x6a000 + (n) * 0x86000)
#define YT921X_MDIOx_OPT_CTRL_REG(n)         (YT921X_MDIOx_CTRL_BASE(n))
#define YT921X_MDIOx_ADDR_CTRL_REG(n)      (YT921X_MDIOx_CTRL_BASE(n) + 0x4)
#define YT921X_MDIOx_DATA_0_REG(n)               (YT921X_MDIOx_CTRL_BASE(n) + 0x8)
#define YT921X_MDIOx_DATA_1_REG(n)               (YT921X_MDIOx_CTRL_BASE(n) + 0xc)
#define YT921X_GLOBAL_CTRL1_REG                  0x80004
#define MIB_ENABLE_MASK                                     BIT(1)
#define CTRL1M_MIB_ENABLE                                BIT(1)
#define ACL_ENABLE_MASK                                    BIT(2)
#define CTRL1M_ACL_ENABLE                               BIT(2)
#define YT921X_EXT_CPU_PORT_CTRL_REG     (0x8000c)
#define EXT_CPU_PORT_NUM_MASK                    GENMASK(3,0)
#define EXT_CPU_PORT_EN                                     BIT(14)
#define EXT_CPU_TAG_EN                                        BIT(15)
#define YT921X_PORT_CTRL_REG(n)                    (0x80100 + (n) * 0x4)
#define TXMAC_EN_MASK                                        BIT(3)
#define RXMAC_EN_MASK                                        BIT(4)
#define YT921X_EXTIF_PORT_CTRL_REG(n)       (0x80120 + (n) * 4)
#define YT921X_INTERFACE_CTRL_REG              (0x80394)
#define YT921X_EXTIF_MODE_CTRL_REG(n)      (0x80400 + (n) * 8)
#define RGMII_RXC_DELAY_SEL_MASK               GENMASK(6,3)
#define RGMII_RXC_DELAY_SEL_OFFSET           (3U)
#define RGMII_RXC_DELAY_SEL_DEFAULT_VAL     (1U)
#define RGMII_TXC_DELAY_SEL_MASK               GENMASK(16,13)
#define RGMII_TXC_DELAY_SEL_OFFSET           (13U)
#define RGMII_TXC_DELAY_SEL_DEFAULT_VAL     (2U)
#define YT921X_MIB_OP_REG                                   (0xC0004)
#define MIB_OP_FLUSH_ALL                                     BIT(30)
#define YT921X_LED_GLB_CTRL_REG                   (0xd0000)
#define LED_GLB_CTRL_MODE_MASK                  GENMASK(1,0)
#define LED_GLB_CTRL_MODE_SERIAL               (0x2U)
#define LED_GLB_CTRL_MODE_PARALLEL        (0U)
#define LED_CFG_DONE_MASK                                BIT(21)
#define YT921X_LED_CTRL_REG                             (0xd0100)
#define LED_CTRL_ACT_MASK                                BIT(4)
#define LED_CTRL_ACT_LOW                                  BIT(4)
#define LED_CTRL_ACT_HIGH                                 (0U)
#define YT921X_PORT_MIB_BASE(n)                       (0xc0100 + (n) * 0x100)
#define YT921X_EGR_PORT_CTRL_REG(n)           (0x100000 + (n) * 0x4)
#define STAG_TPID_SEL_MASK                                GENMASK(3,2)
#define CTAG_TPID_SEL_MASK                               GENMASK(5,4)
#define YT921X_EGR_PORT_VLAN_CTRL_REG(n)    (0x100080 + (n) * 0x4)
#define STAG_MODE_MASK                                       GENMASK(29, 27)
#define STAG_MODE_OFFSET                                    (27U)
#define CTAG_MODE_MASK                                       GENMASK(14,12)
#define CTAG_MODE_OFFSET                                   (12U)
#define EGR_VLANTAG_MODE_ENTRY_BASE      (0x5U)
#define EGR_VLANTAG_MODE_TAGGED              (0x1)
#define YT921X_EGR_TPID_PROFILE_REG(n)        (0x100300 + (n) * 0x4)
#define YT921X_CVLAN_TPID                                     (0x8100)
#define YT921X_SVLAN_TPID                                      (0x88a8)
#define YT921X_CPU_PKT_BYPASS_CTRL_REG    (0x1004d4)
#define YT921X_L2_VLAN_INGR_FILTER_REG      (0x180280)
#define YT921X_L2_VLAN_EGR_FILTER_REG       (0x180598)
#define VLAN_FILTER_PORT_EN_MASK(n)            BIT(n)
#define YT921X_INTERFACE_SELECT_REG            (0x80028)
#define YT921X_INTERFACE_SG_PHY_REG(n)       (0x8008c + (n) * 4)
#define YT921X_PORT_ISOLATION_CTRL_BASE(n)  (0x180294 + (n) * 4)
#define YT921X_STP_STATE_REG(n)                         (0x18038c + (n) * 0x4)
#define PORT_STP_STATE_MASK(n)                         (0x3UL << ((n) * 0x2))
#define PORT_STP_STATE_OFFSET(n)                     ((n) * 0x2)
#define YT921X_LEARN_PER_PORT_CTRL_BASE(n)  (0x1803d0 + (n) * 4)
#define LEARN_DISABLE_MASK                                BIT(17)

/* FDB */
#define YT921X_FDB_TBL_CTRL0_REG                    (0x180454)
#define MACDA_0_3_MASK                                          GENMASK(31,0)
#define YT921X_FDB_TBL_CTRL1_REG                    (0x180458)
#define MACDA_4_5_MASK                                          GENMASK(15,0)
#define FID_MASK                                                          GENMASK(27,16)
#define FID_OFFSET                                                       (16U)
#define STATUS_MASK                                                  GENMASK(30,28)
#define STATUS_VALUE_STATIC                               (0x70000000U)
#define YT921X_FDB_TBL_CTRL2_REG                    (0x18045c)
#define DST_PORTMASK_MASK                                 GENMASK(28,18)
#define DST_PORTMASK_OFFSET                             (18U)
#define YT921X_FDB_TBL_OP_REG                           (0x180460)
#define FDB_OP_START_MASK                                   BIT(0)
#define FDB_OP_START_EN                                         (0x1U)
#define FDB_OP_CMD_MASK                                       GENMASK(3,1)
#define FDB_OP_CMD_ADD                                          (0x0U)
#define FDB_OP_CMD_DEL                                          (0x2U)
#define FDB_OP_MODE_MASK                                    BIT(10)
#define FDB_OP_MODE_HASH                                    (0x0U)
#define FDB_OP_ENTRYID_MASK                              GENMASK(22,11)
#define YT921X_FDB_TBL_CFG_REG                        (0x180464)
#define FDB_OP_DONE_MASK                                    BIT(15)
#define YT921X_COPY_DST_CTRL_REG                  (0x180690)
#define COPY_TO_EXT_CPU_MASK                          BIT(0)
#define COPY_TO_EXT_CPU_EN                                BIT(0)
#define YT921X_UC_UNKNOWN_ACT_CTRL_REG      (0x180734)
#define YT921X_MC_UNKNOWN_ACT_CTRL_REG     (0x180738)
#define UNKNOWN_PKT_ACT_MASK                       GENMASK(21, 0)
#define UNKNOWN_PKT_ACT_TRAP                        (1U)
#define YT921X_L2_VLAN_TBL_REG0(n)                 (0x188000 + (n) * 0x8)
#define PORT_MEMBER_BITMAP_MASK                GENMASK(17,7)
#define PORT_MEMBER_BITMAP_OFFSET            (0x7UL)
#define YT921X_L2_VLAN_TBL_REG1(n)                (0x188000 + (n) * 0x8 + 0x4)
#define UNTAG_MEMBER_BITMAP_MASK            GENMASK(18,8)
#define UNTAG_MEMBER_BITMAP_OFFSET         (0x8UL)
#define YT921X_PORT_VLAN_CTRL_REG(n)         (0x230010 + (n) * 0x4)
#define DEFAULT_SVID_MASK                                  GENMASK(29,18)
#define DEFAULT_SVID_OFFSET                               (18U)
#define DEFAULT_CVID_MASK                                  GENMASK(17,6)
#define DEFAULT_CVID_OFFSET                              (6U)
#define DEFAULT_PORT_VID                                     (1U)
/*switch default cfg led*/
#define LED_CFG_DONE_REG                                   (0xd0000)
#define LED_CFG_DONE_OFFSET                             BIT(21)
#define DISABLE_LED_LINK_TRY_OFFSET            BIT(17)
#define LED_CTRL_REG(N)                                        (0xd0004 + (N) * 0x4)
/* switch default cfg ac */
#define AC_UNI_Q_CFG_TBL_REG0(N)                   (0x301004 + (N) * 0x8)
#define AC_UNI_Q_CFG_TBL_REG1(N)                   (0x301000 + (N) * 0x8)
#define MCAST_QUEUE_REG(macid, qid)                 (0x302000 + ((macid) * YT921X_MC_QUEUE_MAX * 4) + ((qid) * 4))
#define UCAST_QUEUE_REG(macid, qid)                  (0x301000 + ((macid) * YT921X_AC_QUEUE_MAX * 8) + ((qid) * 8))
/* switch default cfg fc */
#define PORT_FC_CFG_TBL0(N)                               (0x281000 + (N) * 0x8)
#define PORT_FC_CFG_TBL1(N)                               (0x281004 + (N) * 0x8)
#define PORT_FC_SHARED_GRP_CTRL(N)            (0x2801d0 + (N) * 0x4)
/* for xmii on low temp */
#define YT921X_EXTIF0_MODE                                 (0x80400)
/* acl en in global_ctrl1 */
#define ACL_EN_OFFSET                                            BIT(2)
/* ac en in global_ctrl1 */
#define AC_EN_OFFSET                                               BIT(3)
/* phy */
#define PHY_BASE_CTRL_REG                                 (0U)
/* vlan tag reg */
#define YT_PARSER_PORT_CTRLN(N)                   (0x210010 + (N) * 0x4)
#define YT_EGR_PORT_CTRLN(N)                          (0x100000 + (N) * 0x4)
#define YT_EGR_TPID_PROFILE(N)                        (0x100300 + (N) * 0x4)
#define YT_ING_TPID_PROFILE(N)                         (0x210000 + (N) * 0x4)
#define YT_L2_VLAN_TBL(N)    (0x188000 + (N) * 0x8)
#define DEFAULT_PORT_MEMBER_BITMAP    GENMASK(17, 7)
#define DEFAULT_PORT_FID0_BITMAP    GENMASK(31, 23)
#define DEFAULT_PORT_FID0_BITMAP_OFFSET             23
#define DEFAULT_PORT_MEMBER_BITMAP_OFFSET    7
#define DEFAULT_UNTAG_MEMBER_BITMAP    GENMASK(18, 8)
#define DEFAULT_UNTAG_MEMBER_BITMAP_OFFSET    8
#define DEFAULT_PORT_SVL_MODE_OFFSET    3

#define DEFAULT_PORT_VLAN_CTRLN(N)    (0x230010 + (N) * 0x4)
#define DEFAULT_CVID    GENMASK(17, 6)
#define DEFAULT_CVID_OFFSET    6

#define L2_VLAN_INGRESS_FILTER_EN    0x180280
#define L2_EGR_VLAN_FILTER_EN    0x180598

#define EGR_PORT_VLAN_CTRLN(N)    (0x100080 + (N) * 0x4)

typedef enum yt_stp_state_e
{
        STP_STATE_LEARN = 1,
        STP_STATE_DISCARD,
        STP_STATE_FORWARD
}yt_stp_state_t;

typedef struct yt_fdb_op_result_s
{
        u16    entry_idx;
        u8     op_result;
        u8     overwrite;
        u8     lookup_fail;
        u8     op_done;
} yt_fdb_fdb_op_result_t;

/* MIB counter -- start */
struct yt921x_mib_counter
{
        unsigned int size;
        unsigned int offset;
        const char *name;
};

static const struct yt921x_mib_counter yt921x_mib[] =
{
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
        { 1, 0x90, "TxAbortCollision"},
        { 1, 0x94, "TxMultiCollision"},
        { 1, 0x98, "TxSingleCollision"},
        /*{ 1, 0x9C, "TxExcDefer"},*/
        /*{ 1, 0xA0, "TxDefer"},*/
        { 1, 0xA4, "TxLateCollision"},
        /*{ 1, 0xA8, "RxOamCounter"},*/
        /*{ 1, 0xAC, "TxOamCounter"},*/
};
/* MIB counter -- start */

/* yt921x function api -- start */
static void
yt921x_write(struct dsa_switch *ds, u32 regAddr, u32 regValue)
{
        struct motorcomm_priv *priv = ds->priv;

        if ((NULL == priv) || (NULL == priv->ops) || (NULL == priv->ops->reg_write))
        {
            return;
        }

        priv->ops->reg_write(ds, regAddr, regValue);
}

static void
yt921x_read(struct dsa_switch *ds, u32 regAddr, u32 *pRegValue)
{
        struct motorcomm_priv *priv = ds->priv;

        if ((NULL == priv) || (NULL == priv->ops) || (NULL == priv->ops->reg_read))
        {
            return;
        }

        priv->ops->reg_read(ds, regAddr, pRegValue);
}

static void
yt921x_rmw(struct dsa_switch *ds, u32 reg, u32 mask, u32 set)
{
        struct motorcomm_priv *priv = ds->priv;

        if ((NULL == priv) || (NULL == priv->ops) || (NULL == priv->ops->reg_rmw))
        {
            return;
        }

        priv->ops->reg_rmw(ds, reg, mask, set);
}

static void
yt921x_led_init(struct dsa_switch *ds)
{
        yt921x_rmw(ds, YT921X_LED_GLB_CTRL_REG, LED_GLB_CTRL_MODE_MASK, LED_GLB_CTRL_MODE_PARALLEL);
        yt921x_rmw(ds, YT921X_LED_CTRL_REG, LED_CTRL_ACT_MASK, LED_CTRL_ACT_LOW);
        yt921x_rmw(ds, YT921X_LED_GLB_CTRL_REG, LED_CFG_DONE_MASK, LED_CFG_DONE_MASK);
}

static void
yt921x_mib_init(struct dsa_switch *ds)
{
        u32 regValue;

        yt921x_rmw(ds, YT921X_GLOBAL_CTRL1_REG, MIB_ENABLE_MASK, CTRL1M_MIB_ENABLE);
        yt921x_write(ds, YT921X_MIB_OP_REG, MIB_OP_FLUSH_ALL);
        yt921x_read(ds, YT921X_MIB_OP_REG, &regValue);
}

static void
yt921x_vlan_port_set(struct dsa_switch *ds, u16 vid, u32 memberMask, u32 utagMask)
{
        yt921x_rmw(ds, YT921X_L2_VLAN_TBL_REG0(vid), PORT_MEMBER_BITMAP_MASK, (memberMask << PORT_MEMBER_BITMAP_OFFSET));
        yt921x_rmw(ds, YT921X_L2_VLAN_TBL_REG1(vid), UNTAG_MEMBER_BITMAP_MASK, (utagMask << UNTAG_MEMBER_BITMAP_OFFSET));
}

static void
yt921x_vlan_port_get(struct dsa_switch *ds, u16 vid, u32 *pMemberMask, u32 *pUtagMask)
{
        u32 tmpValue;

        yt921x_read(ds, YT921X_L2_VLAN_TBL_REG0(vid), &tmpValue);
        *pMemberMask = (tmpValue & PORT_MEMBER_BITMAP_MASK) >> PORT_MEMBER_BITMAP_OFFSET;

        yt921x_read(ds, YT921X_L2_VLAN_TBL_REG1(vid), &tmpValue);
        *pUtagMask = (tmpValue & UNTAG_MEMBER_BITMAP_MASK) >> UNTAG_MEMBER_BITMAP_OFFSET;
}

static void
yt921x_vlan_pvid_set(struct dsa_switch *ds, int port, u16 vid)
{
        u32 tmpValue;

        tmpValue = vid << DEFAULT_SVID_OFFSET;
        yt921x_rmw(ds, YT921X_PORT_VLAN_CTRL_REG(port), DEFAULT_SVID_MASK, tmpValue);

        tmpValue = vid << DEFAULT_CVID_OFFSET;
        yt921x_rmw(ds, YT921X_PORT_VLAN_CTRL_REG(port), DEFAULT_CVID_MASK, tmpValue);
}

static void
yt921x_vlan_pvid_get(struct dsa_switch *ds, int port, u16 *pVid)
{
        u32 tmpValue;

        yt921x_read(ds, YT921X_PORT_VLAN_CTRL_REG(port), &tmpValue);
        *pVid = (tmpValue | DEFAULT_CVID_MASK) >> DEFAULT_CVID_OFFSET;
}

static void
yt921x_vlan_init(struct dsa_switch *ds)
{
        int port;

        /* tpid select */
        yt921x_write(ds, YT921X_EGR_TPID_PROFILE_REG(0), YT921X_CVLAN_TPID);
        yt921x_write(ds, YT921X_EGR_TPID_PROFILE_REG(1), YT921X_SVLAN_TPID);

        for(port = 0; port < DSA_MAX_PORTS; port++)
        {
                if (YT921X_ALL_PORTMASK & BIT(port))
                {
                        yt921x_rmw(ds, YT921X_EGR_PORT_CTRL_REG(port), CTAG_TPID_SEL_MASK | STAG_TPID_SEL_MASK, 0 | 0x4);
#if defined(YT921X_DSA_WITHOUT_TAG) ||defined(YT921X_DSA_WITH_VLAN_TAG)
                        yt921x_rmw(ds, YT921X_EGR_PORT_VLAN_CTRL_REG(port), STAG_MODE_MASK | CTAG_MODE_MASK,
                            (EGR_VLANTAG_MODE_ENTRY_BASE << STAG_MODE_OFFSET) | (EGR_VLANTAG_MODE_ENTRY_BASE << CTAG_MODE_OFFSET));
#endif
                }
        }
}

#define CLS_YT_DSA_PORT
#ifdef CLS_YT_DSA_PORT
static int test_call_esw_cli(void)
{
	int result = 0;

	char cmdPath[]="/usr/sbin/esw_cli";
	char* cmdEnvp[]={"HOME=/", "PATH=/sbin:/bin:/usr/bin", NULL};

	char* cmdArgv[]={cmdPath, "yt_port_enable_set 0 5 1", NULL};
	result=call_usermodehelper(cmdPath,cmdArgv,cmdEnvp,UMH_WAIT_PROC);
	msleep(1000);

	char* cmdArgv1[] ={cmdPath, "yt_port_extif_mode_set 0 5 YT_EXTIF_MODE_BX2500", NULL};
	result=call_usermodehelper(cmdPath,cmdArgv1,cmdEnvp,UMH_WAIT_PROC);
	msleep(1000);

	char* cmdArgv2[] ={cmdPath, "yt_port_mac_force_set 0 5 speed_dup PORT_SPEED_DUP_2500FULL rx_fc_en 1 tx_fc_en 1", NULL};
	result=call_usermodehelper(cmdPath,cmdArgv2,cmdEnvp,UMH_WAIT_PROC);
	msleep(1000);

	//pr_info("esw_cli set cpu port to 2.5G fix rate \"%s\",pid is %d\n",current->comm,current->pid);

	return 0;
}
#endif

static void
yt921x_cpu_port_enable(struct dsa_switch *ds)
{
        /* CPU Port */
#if defined(YT921X_DSA_WITHOUT_TAG) ||defined(YT921X_DSA_WITH_VLAN_TAG)
        yt921x_rmw(ds, YT921X_EXT_CPU_PORT_CTRL_REG, EXT_CPU_PORT_NUM_MASK | EXT_CPU_PORT_EN | EXT_CPU_TAG_EN, YT921X_CPU_PORT | EXT_CPU_PORT_EN);
#else
        yt921x_rmw(ds, YT921X_EXT_CPU_PORT_CTRL_REG, EXT_CPU_PORT_NUM_MASK | EXT_CPU_PORT_EN | EXT_CPU_TAG_EN, YT921X_CPU_PORT | EXT_CPU_PORT_EN | EXT_CPU_TAG_EN);
        yt921x_write(ds, YT921X_CPU_PKT_BYPASS_CTRL_REG, 0x0);
#endif
        yt921x_rmw(ds, YT921X_COPY_DST_CTRL_REG, COPY_TO_EXT_CPU_MASK, COPY_TO_EXT_CPU_EN);

        /* disable SA learning of CPU PORT */
        yt921x_rmw(ds, YT921X_LEARN_PER_PORT_CTRL_BASE(YT921X_CPU_PORT), LEARN_DISABLE_MASK, LEARN_DISABLE_MASK);
}

#ifdef YT921X_UNKNOWN_PKT_TO_CPU
static void
yt921x_ctrlpkt_init(struct dsa_switch *ds)
{
        int port;
        u32 unknownPktAct;

        /* unknown pkt trap to CPU Port */
        unknownPktAct = 0;
        for (port = 0; port < DSA_MAX_PORTS; port++)
        {
                if (YT921X_USER_PORTMASK & BIT(port))
                {
                        unknownPktAct |= UNKNOWN_PKT_ACT_TRAP << ( 2 * port);
                }
        }
        yt921x_rmw(ds, YT921X_UC_UNKNOWN_ACT_CTRL_REG, UNKNOWN_PKT_ACT_MASK, unknownPktAct);
        yt921x_rmw(ds, YT921X_MC_UNKNOWN_ACT_CTRL_REG, UNKNOWN_PKT_ACT_MASK, unknownPktAct);

        /* bcast pkt trap to CPU Port */
        yt921x_rmw(ds, YT921X_GLOBAL_CTRL1_REG, ACL_ENABLE_MASK, CTRL1M_ACL_ENABLE);
        yt921x_write(ds, 0x1806a0, YT921X_USER_PORTMASK);
        yt921x_write(ds, 0x202000, YT921X_USER_PORTMASK);
        yt921x_write(ds, 0x202004, 0x7f81);
        yt921x_write(ds, 0x204000, 0xffffffff);
        yt921x_write(ds, 0x204004, 0x2c7c10);
        yt921x_write(ds, 0x205000, 0xffffffff);
        yt921x_write(ds, 0x205004, 0x0);
        yt921x_write(ds, 0x204200, 0xffff0000);
        yt921x_write(ds, 0x204204, 0x2c7c30);
        yt921x_write(ds, 0x205200, 0xffff0000);
        yt921x_write(ds, 0x205204, 0x0);
        yt921x_write(ds, 0x201000, 0x11111100);
        yt921x_write(ds, 0x203000, 0x11);
        yt921x_write(ds, 0x202004, 0x7f80);
        yt921x_write(ds, 0x1c0000, 0x0);
        yt921x_write(ds, 0x1c0004, 0x0);
        yt921x_write(ds, 0x1c0008, 0x600200);
}
#endif

static void
yt921x_port_enable_set(struct dsa_switch *ds, int port, u8 enable)
{
        if (enable)
        {
                yt921x_rmw(ds, YT921X_PORT_CTRL_REG(port), TXMAC_EN_MASK | RXMAC_EN_MASK, TXMAC_EN_MASK | RXMAC_EN_MASK);
        }
        else
        {
                yt921x_rmw(ds, YT921X_PORT_CTRL_REG(port), TXMAC_EN_MASK | RXMAC_EN_MASK, 0);
        }
}

static void
yt921x_vlan_filter_set(struct dsa_switch *ds, int port, bool vlanFiltering)
{
        u32 regValue = 0;

        if (vlanFiltering)
        {
                regValue = VLAN_FILTER_PORT_EN_MASK(port);
        }

        /* ingress filter */
        yt921x_rmw(ds, YT921X_L2_VLAN_INGR_FILTER_REG, VLAN_FILTER_PORT_EN_MASK(port), regValue);
        /* egress filter */
        yt921x_rmw(ds, YT921X_L2_VLAN_EGR_FILTER_REG, VLAN_FILTER_PORT_EN_MASK(port), regValue);
}

static int
yt921x_slave_mdio_write(struct dsa_switch *ds, int port, int regNum, u16 data)
{
        u32 baseData;
        u32 opData = 0;
        u32 phyAddr = port;
        u32 regAddr = regNum;
        u32 waitCount = 0;
        u8  mdioId = 1;

        yt921x_read(ds, YT921X_MDIOx_ADDR_CTRL_REG(mdioId), &baseData);
        baseData &= 0xFC00FFF1;
        baseData |= ((phyAddr&0x1f) << 21 | (regAddr&0x1f) << 16 | 1 << 2);
        yt921x_write(ds, YT921X_MDIOx_ADDR_CTRL_REG(mdioId), baseData);

        yt921x_write(ds, YT921X_MDIOx_DATA_0_REG(mdioId), data);

        opData = 1;
        yt921x_write(ds, YT921X_MDIOx_OPT_CTRL_REG(mdioId), opData);

        while(MAX_BUSYING_WAIT_TIME > waitCount)
        {
                yt921x_read(ds, YT921X_MDIOx_OPT_CTRL_REG(mdioId), &opData);
                if(!opData)
                {
                        return 0;
                }

                waitCount ++;
        }

        return -1;
}

static int
yt921x_slave_mdio_read(struct dsa_switch *ds, int port, int regNum)
{
        u32 baseData;
        u32 opData = 0;
        u32 phyAddr = port;
        u32 regAddr = regNum;
        u32 waitCount = 0;
        u8  mdioId = 1;

        yt921x_read(ds, YT921X_MDIOx_ADDR_CTRL_REG(mdioId), &baseData);
        baseData &= 0xFC00FFF1;
        baseData |= ((phyAddr&0x1f) << 21 | (regAddr&0x1f) << 16 | 2 << 2);
        yt921x_write(ds, YT921X_MDIOx_ADDR_CTRL_REG(mdioId), baseData);

        opData = 1;
        yt921x_write(ds, YT921X_MDIOx_OPT_CTRL_REG(mdioId), opData);

        while(MAX_BUSYING_WAIT_TIME > waitCount)
        {
                yt921x_read(ds, YT921X_MDIOx_OPT_CTRL_REG(mdioId), &opData);
                if(!opData)
                {
                        yt921x_read(ds, YT921X_MDIOx_DATA_1_REG(mdioId), &baseData);

                        return baseData;
                }

                waitCount ++;
        }

        return -1;
}

static int
yt921x_fdb_op_result_get(struct dsa_switch *ds)
{
    u32 regValue;
    u16 l2_fdb_tbl_op_busy_cnt = MAX_BUSYING_WAIT_TIME;

    while (l2_fdb_tbl_op_busy_cnt)
    {
        yt921x_read(ds, YT921X_FDB_TBL_CFG_REG, &regValue);

        if(regValue & FDB_OP_DONE_MASK)
        {
            break;
        }

        l2_fdb_tbl_op_busy_cnt--;

        if(0 == l2_fdb_tbl_op_busy_cnt)
        {
            return -ETIMEDOUT;
        }

    }

    return 0;
}

static int yt921x_patch_default_init(struct dsa_switch *ds)
{
    yt921x_write(ds, 0x80028, 0x0);
    mdelay(1000);
    return 0;
}

/* yt921x function api -- end */

#if (KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE && KERNEL_VERSION(4, 20, 0) > LINUX_VERSION_CODE)
/* kernel version 4.19 dsa_switch_ops -- start */
static enum dsa_tag_protocol
yt921x_get_tag_protocol(struct dsa_switch *ds, int port)
{
#if defined(YT921X_DSA_WITHOUT_TAG)
    return DSA_TAG_PROTO_NONE;
#elif defined (YT921X_DSA_WITH_VLAN_TAG)
    return DSA_TAG_PROTO_MOTORCOMM_VLAN;
#else
    return DSA_TAG_PROTO_MOTORCOMM;
#endif
}

static int yt921x_setup(struct dsa_switch *ds)
{
    int i;

    yt921x_patch_default_init(ds);

    /* LED init */
    yt921x_led_init(ds);

    /* vlan init */
    yt921x_vlan_init(ds);

    /* Enable and reset MIB counters */
    yt921x_mib_init(ds);

    /* enable cpu_port */
    yt921x_cpu_port_enable(ds);

    /* ctrl pkt */
#ifdef YT921X_UNKNOWN_PKT_TO_CPU
    yt921x_ctrlpkt_init(ds);
#endif

    /* port isolation */
    for (i = 0; i < DSA_MAX_PORTS; i++)
    {
        if (YT921X_USER_PORTMASK & BIT(i))
        {
            /* Disable forwarding by default on all user ports */
            yt921x_rmw(ds, YT921X_PORT_ISOLATION_CTRL_BASE(i), YT921X_USER_PORTMASK, YT921X_USER_PORTMASK & (~BIT(i)));
        }
    }

	pr_err("%s %d end of yt921x_setup\n", __FUNCTION__, __LINE__);
    return 0;
}

static void
yt921x_adjust_link(struct dsa_switch *ds, int port,
                  struct phy_device *phydev)
{
    int i;
    const struct dsa_port *dp = dsa_to_port(ds, port);
    unsigned int regData;

    if (port < DSA_MAX_PORTS && dp->type == DSA_PORT_TYPE_CPU)
    {
        switch (phydev->interface)
        {
                case PHY_INTERFACE_MODE_SGMII:
                    {
                        switch (phydev->speed)
                        {
                            case 10:
                                printk(KERN_WARNING "the mode is unsupported now \n");
                                break;
                            case 100:
                                printk(KERN_WARNING "the mode is unsupported now \n");
                                break;
                            case 1000:
                                {
                                    /* P8/P9 sgmii 1G full duplex*/
                                    yt921x_write(ds, YT921X_INTERFACE_CTRL_REG, 0x0);
                                    yt921x_read(ds, YT921X_INTERFACE_SELECT_REG, &regData);
                                    regData &= ~(0xf << 0);
                                    regData |= 0x3;
                                    yt921x_write(ds, YT921X_INTERFACE_SELECT_REG, regData);
                                    yt921x_write(ds, YT921X_EXTIF_PORT_CTRL_REG(0), 0x1fa);
                                    yt921x_write(ds, YT921X_INTERFACE_SG_PHY_REG(0), 0xfa);
                                }
                                break;
                            default:
                                printk(KERN_WARNING "the speed is unsupported in SGMII \n");
                                break;
                        }
                    }
                    break;
            case PHY_INTERFACE_MODE_RGMII:
                {
                    switch (phydev->speed)
                    {
                        case 10:
                            printk(KERN_WARNING "the mode is unsupported\n");
                            break;
                        case 100:
                            printk(KERN_WARNING "the mode is unsupported\n");
                            break;
                        case 1000:
                            {
                                /* P8/P9 rgmii 1G full duplex*/
                                yt921x_write(ds, YT921X_INTERFACE_CTRL_REG, 0x3);
                                for (i = 0; i < 2; i++)
                                {
                                    yt921x_write(ds, YT921X_EXTIF_PORT_CTRL_REG(i), 0x1fa);
                                    yt921x_write(ds, YT921X_EXTIF_MODE_CTRL_REG(i), 0x841c0000);
                                    /* tx/rx delay */
                                    yt921x_rmw(ds, YT921X_EXTIF_MODE_CTRL_REG(i), RGMII_RXC_DELAY_SEL_MASK | RGMII_TXC_DELAY_SEL_MASK ,
                                        (RGMII_RXC_DELAY_SEL_DEFAULT_VAL << RGMII_RXC_DELAY_SEL_OFFSET) | (RGMII_TXC_DELAY_SEL_DEFAULT_VAL << RGMII_TXC_DELAY_SEL_OFFSET));
                                }
                            }
                            break;
                        default:
                            printk(KERN_WARNING "the speed is unsupported in RGMII \n");
                            break;
                    }
                }
                break;
            case PHY_INTERFACE_MODE_2500BASEX:
                {
                    int j;
                    for (j = 0; j < 2; j++)
                    {
                        yt921x_write(ds, YT921X_INTERFACE_CTRL_REG, 0x0);
                        yt921x_write(ds, YT921X_INTERFACE_SELECT_REG, 0x3);
                        yt921x_write(ds, YT921X_INTERFACE_SG_PHY_REG(j), 0x2fc);
                    }
                }
                break;
            default:
                printk(KERN_WARNING "the mode is unsupported\n");
                break;
        }
    }
    return;
}

static void
yt921x_get_strings(struct dsa_switch *ds, int port, u32 stringset,
             u8 *data)
{
    int i;

    if (stringset != ETH_SS_STATS)
        return;

    for (i = 0; i < ARRAY_SIZE(yt921x_mib); i++)
    {
        strncpy(data + i * ETH_GSTRING_LEN, yt921x_mib[i].name,
            ETH_GSTRING_LEN);
    }
}

static void
yt921x_get_ethtool_stats(struct dsa_switch *ds, int port, u64 *data)
{
    int i;
    u32 lowData = 0;
    u32 highData = 0;
    u64 resultData = 0;
    int mibCount;
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->cfg_mutex);

    mibCount = ARRAY_SIZE(yt921x_mib);
    for (i = 0; i < mibCount; i++)
    {
        yt921x_read(ds, YT921X_PORT_MIB_BASE(port) + yt921x_mib[i].offset, &lowData);
        data[i] = lowData;

        if (yt921x_mib[i].size == 2)
        {
            yt921x_read(ds, YT921X_PORT_MIB_BASE(port) + yt921x_mib[i].offset + 4, &highData);
            resultData = highData;

            data[i] |= resultData << 32;
        }
    }

    mutex_unlock(&priv->cfg_mutex);
}

static int
yt921x_get_sset_count(struct dsa_switch *ds, int port, int sset)
{
    if (sset != ETH_SS_STATS)
        return 0;

    return ARRAY_SIZE(yt921x_mib);
}

static int
yt921x_vlan_filtering(struct dsa_switch *ds, int port, bool vlan_filtering)
{
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->cfg_mutex);
    yt921x_vlan_filter_set(ds, port, vlan_filtering);
    mutex_unlock(&priv->cfg_mutex);

    return 0;
}

static int
yt921x_vlan_prepare(struct dsa_switch *ds, int port,
             const struct switchdev_obj_port_vlan *vlan)
{
    return 0;
}

static void
yt921x_vlan_add(struct dsa_switch *ds, int port,
            const struct switchdev_obj_port_vlan *vlan)
{
    bool untagged = vlan->flags & BRIDGE_VLAN_INFO_UNTAGGED;
    bool pvid = vlan->flags & BRIDGE_VLAN_INFO_PVID;
    u32 member = 0;
    u32 untag = 0;
    u16 vid;
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->cfg_mutex);

    for (vid = vlan->vid_begin; vid <= vlan->vid_end; ++vid) {
        yt921x_vlan_port_get(ds, vid, &member, &untag);

        member |= BIT(port);
        if (untagged)
        {
            untag |= BIT(port);
        }
        else
        {
            untag &= ~(BIT(port));
        }

        /* add cpu port to all vlan */
        member |= BIT(YT921X_CPU_PORT);
        yt921x_vlan_port_set(ds, vid, member, untag);
    }

    /* vid_end as pvid */
    if(pvid)
    {
        yt921x_vlan_pvid_set(ds, port, vlan->vid_end);
    }

    mutex_unlock(&priv->cfg_mutex);
}

static int
yt921x_vlan_del(struct dsa_switch *ds, int port, const struct switchdev_obj_port_vlan *vlan)
{
    bool untagged = vlan->flags & BRIDGE_VLAN_INFO_UNTAGGED;
    u32 member = 0;
    u32 untag = 0;
    u16 vid;
    u16 pvid;
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->cfg_mutex);

    for (vid = vlan->vid_begin; vid <= vlan->vid_end; ++vid) {
        yt921x_vlan_port_get(ds, vid, &member, &untag);

        member &= ~(BIT(port));
        if (untagged)
        {
            untag &= ~(BIT(port));
        }
        yt921x_vlan_port_set(ds, vid, member, untag);
    }

    yt921x_vlan_pvid_get(ds, port, &pvid);
    if ((vlan->vid_begin <= pvid) && (pvid <= vlan->vid_end))
    {
        yt921x_vlan_pvid_set(ds, port, DEFAULT_PORT_VID);
    }

    mutex_unlock(&priv->cfg_mutex);

    return 0;
}

static int
yt921x_port_enable(struct dsa_switch *ds, int port,
              struct phy_device *phy)
{
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->cfg_mutex);
    yt921x_port_enable_set(ds, port, 1);
    mutex_unlock(&priv->cfg_mutex);

    return 0;
}

static void
yt921x_port_disable(struct dsa_switch *ds, int port,
                struct phy_device *phy)
{
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->cfg_mutex);
    yt921x_port_enable_set(ds, port, 0);
    mutex_unlock(&priv->cfg_mutex);
}

static void
yt921x_stp_state_set(struct dsa_switch *ds, int port, u8 state)
{
    u32 stp_state;

    switch (state) {
        case BR_STATE_DISABLED:
            stp_state = STP_STATE_DISCARD;
            break;

        case BR_STATE_BLOCKING:
            stp_state = STP_STATE_DISCARD;
            break;

        case BR_STATE_LISTENING:
            stp_state = STP_STATE_DISCARD;
            break;

        case BR_STATE_LEARNING:
            stp_state = STP_STATE_LEARN;
            break;

        case BR_STATE_FORWARDING:
        default:
            stp_state = STP_STATE_FORWARD;
            break;
    }

    yt921x_rmw(ds, YT921X_STP_STATE_REG(port), PORT_STP_STATE_MASK(port), stp_state << PORT_STP_STATE_OFFSET(port));
}

static int
yt921x_port_bridge_join(struct dsa_switch *ds, int port, struct net_device *bridge)
{
    unsigned int port_bitmap = 0;
    int i;
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->cfg_mutex);

    for (i = 0; i < DSA_MAX_PORTS; i++)
    {
        if ((YT921X_USER_PORTMASK & BIT(i)) && i != port)
        {
            if (dsa_to_port(ds, i)->bridge_dev != bridge)
                continue;

            yt921x_rmw(ds, YT921X_PORT_ISOLATION_CTRL_BASE(i), BIT(port), 0);

            port_bitmap |= BIT(i);
        }
    }

    yt921x_rmw(ds, YT921X_PORT_ISOLATION_CTRL_BASE(port), port_bitmap, 0);
    mutex_unlock(&priv->cfg_mutex);

    return 0;
}

static void
yt921x_port_bridge_leave(struct dsa_switch *ds, int port, struct net_device *bridge)
{
    unsigned int port_bitmap = 0;
    int i;
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->cfg_mutex);
    for (i = 0; i < DSA_MAX_PORTS; i++)
    {
        if ((YT921X_USER_PORTMASK & BIT(i)) && i != port)
        {
            if (dsa_to_port(ds, i)->bridge_dev != bridge)
                continue;

            yt921x_rmw(ds, YT921X_PORT_ISOLATION_CTRL_BASE(i), BIT(port), BIT(port));

            port_bitmap |= BIT(i);
        }
    }

    yt921x_rmw(ds, YT921X_PORT_ISOLATION_CTRL_BASE(port), port_bitmap, port_bitmap);
    mutex_unlock(&priv->cfg_mutex);
}

static int
yt921x_port_fdb_add(struct dsa_switch *ds, int port, const unsigned char *addr, u16 vid)
{
    int ret;
    u32 regValue;
    u32 tmpValue;
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->cfg_mutex);
    regValue = (addr[0] << 24) | (addr[1] << 16) | (addr[2] << 8) | (addr[3]);
    yt921x_write(ds, YT921X_FDB_TBL_CTRL0_REG, regValue);

    tmpValue = vid;
    regValue = ((tmpValue << FID_OFFSET) & FID_MASK) | STATUS_VALUE_STATIC | (addr[4] << 8) | addr[5];
    yt921x_write(ds, YT921X_FDB_TBL_CTRL1_REG, regValue);

    regValue = (BIT(port)) << DST_PORTMASK_OFFSET;
    yt921x_write(ds, YT921X_FDB_TBL_CTRL2_REG, regValue);

    regValue = FDB_OP_START_EN | FDB_OP_CMD_ADD | FDB_OP_MODE_HASH;
    yt921x_write(ds, YT921X_FDB_TBL_OP_REG, regValue);

    ret = yt921x_fdb_op_result_get(ds);
    mutex_unlock(&priv->cfg_mutex);

    return ret;
}

static int
yt921x_port_fdb_del(struct dsa_switch *ds, int port, const unsigned char *addr, u16 vid)
{
    int ret;
    u32 regValue;
    u32 tmpValue;
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->cfg_mutex);
    regValue = (addr[0] << 24) | (addr[1] << 16) | (addr[2] << 8) | (addr[3]);
    yt921x_write(ds, YT921X_FDB_TBL_CTRL0_REG, regValue);

    tmpValue = vid;
    regValue = ((tmpValue << FID_OFFSET) & FID_MASK) | (addr[4] << 8) | addr[5];
    yt921x_write(ds, YT921X_FDB_TBL_CTRL1_REG, regValue);

    regValue = 0;
    yt921x_write(ds, YT921X_FDB_TBL_CTRL2_REG, regValue);

    regValue = FDB_OP_START_EN | FDB_OP_CMD_DEL | FDB_OP_MODE_HASH;
    yt921x_write(ds, YT921X_FDB_TBL_OP_REG, regValue);

    ret = yt921x_fdb_op_result_get(ds);
    mutex_unlock(&priv->cfg_mutex);

    return ret;
}

static int
yt921x_phy_read(struct dsa_switch *ds, int port, int regnum)
{
    int ret;
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->cfg_mutex);
    ret =  yt921x_slave_mdio_read(ds, port, regnum);
    mutex_unlock(&priv->cfg_mutex);

    return ret;
}

static int
yt921x_phy_write(struct dsa_switch *ds, int port, int regnum,
                u16 val)
{
    int ret;
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->cfg_mutex);
    ret =  yt921x_slave_mdio_write(ds, port, regnum, val);
    mutex_unlock(&priv->cfg_mutex);

    return ret;
}

static void
yt921x_mac_link_up(struct dsa_switch *ds, int port, unsigned int mode,
                phy_interface_t interface, struct phy_device *phydev)
{
}

static void
yt921x_mac_link_down(struct dsa_switch *ds, int port, unsigned int mode,
            phy_interface_t interface)
{
}

static int
yt921x_resume(struct dsa_switch *ds)
{
    yt921x_setup(ds);
    yt921x_init(ds->priv);
    return 0;
}

#if 0
static int
yt921x_port_pre_bridge_flags(struct dsa_switch *ds, int port,
                struct switchdev_brport_flags flags,
                struct netlink_ext_ack *extack)
{
    if (flags.mask & ~(BR_LEARNING))
        return -EINVAL;

    return 0;
}

static int
yt921x_port_bridge_flags(struct dsa_switch *ds, int port,
                struct switchdev_brport_flags flags,
                struct netlink_ext_ack *extack)
{
    int ret;

    if (flags.mask & BR_LEARNING) {
        yt921x_rmw(ds, YT921X_LEARN_PER_PORT_CTRL_BASE(port), LEARN_DISABLE_MASK, (flags.val & BR_LEARNING) ? 0 : LEARN_DISABLE_MASK);
    }

    return 0;
}
#endif
#elif (KERNEL_VERSION(5, 10, 0) <= LINUX_VERSION_CODE && KERNEL_VERSION(5, 11, 0) > LINUX_VERSION_CODE)
static enum dsa_tag_protocol
yt921x_get_tag_protocol(struct dsa_switch *ds,
						  int port,
						  enum dsa_tag_protocol mprot)
{
#if defined(YT921X_DSA_WITHOUT_TAG)
    return DSA_TAG_PROTO_NONE;
#elif defined (YT921X_DSA_WITH_VLAN_TAG)
    return DSA_TAG_PROTO_MOTORCOMM_VLAN;
#else
    return DSA_TAG_PROTO_MOTORCOMM;
#endif
}

static int yt921x_setup(struct dsa_switch *ds)
{
    int i;

    yt921x_patch_default_init(ds);

    /* LED init */
    yt921x_led_init(ds);

    /* vlan init */
    yt921x_vlan_init(ds);

    /* Enable and reset MIB counters */
    yt921x_mib_init(ds);

    /* enable cpu_port */
    yt921x_cpu_port_enable(ds);

    /* ctrl pkt */
#ifdef YT921X_UNKNOWN_PKT_TO_CPU
    yt921x_ctrlpkt_init(ds);
#endif

    /* port isolation */
    for (i = 0; i < DSA_MAX_PORTS; i++)
    {
        if (YT921X_USER_PORTMASK & BIT(i))
        {
            /* Disable forwarding by default on all user ports */
            yt921x_rmw(ds, YT921X_PORT_ISOLATION_CTRL_BASE(i), YT921X_USER_PORTMASK, YT921X_USER_PORTMASK & (~BIT(i)));
        }
    }

    return 0;
}

static void
yt921x_adjust_link(struct dsa_switch *ds, int port,
                  struct phy_device *phydev)
{
    int i;
    unsigned int regData;
    const struct dsa_port *dp = dsa_to_port(ds, port);

    if (port < DSA_MAX_PORTS && dp->type == DSA_PORT_TYPE_CPU)
    {
        switch (phydev->interface)
        {
                case PHY_INTERFACE_MODE_SGMII:
                    {
                        switch (phydev->speed)
                        {
                            case 10:
                                printk(KERN_WARNING "the mode is unsupported now \n");
                                break;
                            case 100:
                                printk(KERN_WARNING "the mode is unsupported now \n");
                                break;
                            case 1000:
                                {
                                    /* P8/P9 sgmii 1G full duplex*/
                                    yt921x_write(ds, YT921X_INTERFACE_CTRL_REG, 0x0);
                                    yt921x_read(ds, YT921X_INTERFACE_SELECT_REG, &regData);
                                    regData &= ~(0xf << 0);
                                    regData |= 0x3;
                                    yt921x_write(ds, YT921X_INTERFACE_SELECT_REG, regData);
                                    yt921x_write(ds, YT921X_EXTIF_PORT_CTRL_REG(0), 0x1fa);
                                    yt921x_write(ds, YT921X_INTERFACE_SG_PHY_REG(0), 0xfa);
                                }
                                break;
                            default:
                                printk(KERN_WARNING "the speed is unsupported in SGMII \n");
                                break;
                        }
                    }
                    break;
            case PHY_INTERFACE_MODE_RGMII:
                {
                    switch (phydev->speed)
                    {
                        case 10:
                            printk(KERN_WARNING "the mode is unsupported\n");
                            break;
                        case 100:
                            printk(KERN_WARNING "the mode is unsupported\n");
                            break;
                        case 1000:
                            {
                                /* P8/P9 rgmii 1G full duplex*/
                                yt921x_write(ds, YT921X_INTERFACE_CTRL_REG, 0x3);
                                for (i = 0; i < 2; i++)
                                {
                                    yt921x_write(ds, YT921X_EXTIF_PORT_CTRL_REG(i), 0x1fa);
                                    yt921x_write(ds, YT921X_EXTIF_MODE_CTRL_REG(i), 0x841c0000);
                                    /* tx/rx delay */
                                    yt921x_rmw(ds, YT921X_EXTIF_MODE_CTRL_REG(i), RGMII_RXC_DELAY_SEL_MASK | RGMII_TXC_DELAY_SEL_MASK ,
                                        (RGMII_RXC_DELAY_SEL_DEFAULT_VAL << RGMII_RXC_DELAY_SEL_OFFSET) | (RGMII_TXC_DELAY_SEL_DEFAULT_VAL << RGMII_TXC_DELAY_SEL_OFFSET));
                                }
                            }
                            break;
                        default:
                            printk(KERN_WARNING "the speed is unsupported in RGMII \n");
                            break;
                    }
                }
                break;
            case PHY_INTERFACE_MODE_2500BASEX:
                {
                    int j;
                    for (j = 0; j < 2; j++)
                    {
                        yt921x_write(ds, YT921X_INTERFACE_CTRL_REG, 0x0);
                        yt921x_write(ds, YT921X_INTERFACE_SELECT_REG, 0x3);
                        yt921x_write(ds, YT921X_INTERFACE_SG_PHY_REG(j), 0x2fc);
                    }
                }
                break;
            default:
                printk(KERN_WARNING "the mode is unsupported\n");
                break;
        }
    }
    return;
}

static void
yt921x_get_strings(struct dsa_switch *ds, int port, u32 stringset,
             u8 *data)
{
    int i;

    if (stringset != ETH_SS_STATS)
        return;

    for (i = 0; i < ARRAY_SIZE(yt921x_mib); i++)
    {
        strncpy(data + i * ETH_GSTRING_LEN, yt921x_mib[i].name,
            ETH_GSTRING_LEN);
    }
}

static void
yt921x_get_ethtool_stats(struct dsa_switch *ds, int port, u64 *data)
{
    int i;
    u32 lowData = 0;
    u32 highData = 0;
    u64 resultData = 0;
    int mibCount;
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->cfg_mutex);

    mibCount = ARRAY_SIZE(yt921x_mib);
    for (i = 0; i < mibCount; i++)
    {
        yt921x_read(ds, YT921X_PORT_MIB_BASE(port) + yt921x_mib[i].offset, &lowData);
        data[i] = lowData;

        if (yt921x_mib[i].size == 2)
        {
            yt921x_read(ds, YT921X_PORT_MIB_BASE(port) + yt921x_mib[i].offset + 4, &highData);
            resultData = highData;

            data[i] |= resultData << 32;
        }
    }

    mutex_unlock(&priv->cfg_mutex);
}

static int
yt921x_vlan_filtering(struct dsa_switch *ds, int port,
    bool vlan_filtering,
    struct switchdev_trans *trans)
{
    struct motorcomm_priv *priv = ds->priv;
    if (switchdev_trans_ph_prepare(trans))
    {
        return 0;
    }

    mutex_lock(&priv->cfg_mutex);
    yt921x_vlan_filter_set(ds, port, vlan_filtering);
    mutex_unlock(&priv->cfg_mutex);

    return 0;
}

static int
yt921x_vlan_prepare(struct dsa_switch *ds, int port,
    const struct switchdev_obj_port_vlan *vlan)
{
    return 0;
}

static void
yt921x_vlan_add(struct dsa_switch *ds, int port,
            const struct switchdev_obj_port_vlan *vlan)
{
    bool untagged = vlan->flags & BRIDGE_VLAN_INFO_UNTAGGED;
    bool pvid = vlan->flags & BRIDGE_VLAN_INFO_PVID;
    u32 member = 0;
    u32 untag = 0;
    u16 vid;
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->cfg_mutex);

    for (vid = vlan->vid_begin; vid <= vlan->vid_end; ++vid) {
        yt921x_vlan_port_get(ds, vid, &member, &untag);

        member |= BIT(port);
        if (untagged)
        {
            untag |= BIT(port);
        }
        else
        {
            untag &= ~(BIT(port));
        }

        /* add cpu port to all vlan */
        member |= BIT(YT921X_CPU_PORT);
        yt921x_vlan_port_set(ds, vid, member, untag);
    }

    /* vid_end as pvid */
    if(pvid)
    {
        yt921x_vlan_pvid_set(ds, port, vlan->vid_end);
    }

    mutex_unlock(&priv->cfg_mutex);
}

static int
yt921x_vlan_del(struct dsa_switch *ds, int port, const struct switchdev_obj_port_vlan *vlan)
{
    bool untagged = vlan->flags & BRIDGE_VLAN_INFO_UNTAGGED;
    u32 member = 0;
    u32 untag = 0;
    u16 vid;
    u16 pvid;
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->cfg_mutex);

    for (vid = vlan->vid_begin; vid <= vlan->vid_end; ++vid) {
        yt921x_vlan_port_get(ds, vid, &member, &untag);

        member &= ~(BIT(port));
        if (untagged)
        {
            untag &= ~(BIT(port));
        }
        yt921x_vlan_port_set(ds, vid, member, untag);
    }

    yt921x_vlan_pvid_get(ds, port, &pvid);
    if ((vlan->vid_begin <= pvid) && (pvid <= vlan->vid_end))
    {
        yt921x_vlan_pvid_set(ds, port, DEFAULT_PORT_VID);
    }

    mutex_unlock(&priv->cfg_mutex);

    return 0;
}

static int
yt921x_port_enable(struct dsa_switch *ds, int port,
    struct phy_device *phy)
{
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->cfg_mutex);
    yt921x_port_enable_set(ds, port, 1);
    mutex_unlock(&priv->cfg_mutex);

    return 0;
}

static void
yt921x_port_disable(struct dsa_switch *ds, int port)
{
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->cfg_mutex);
    yt921x_port_enable_set(ds, port, 0);
    mutex_unlock(&priv->cfg_mutex);
}

static void
yt921x_stp_state_set(struct dsa_switch *ds, int port, u8 state)
{
    u32 stp_state;

    switch (state) {
        case BR_STATE_DISABLED:
            stp_state = STP_STATE_DISCARD;
            break;

        case BR_STATE_BLOCKING:
            stp_state = STP_STATE_DISCARD;
            break;

        case BR_STATE_LISTENING:
            stp_state = STP_STATE_DISCARD;
            break;

        case BR_STATE_LEARNING:
            stp_state = STP_STATE_LEARN;
            break;

        case BR_STATE_FORWARDING:
        default:
            stp_state = STP_STATE_FORWARD;
            break;
    }

    yt921x_rmw(ds, YT921X_STP_STATE_REG(port), PORT_STP_STATE_MASK(port), stp_state << PORT_STP_STATE_OFFSET(port));
}
#if 1
static int
yt921x_port_bridge_join(struct dsa_switch *ds, int port, struct net_device *bridge)
{
    unsigned int port_bitmap = 0;
    int i;
    int fid_mask = 0;
    int index = 0;
    int tmpData;
    int tmpData1;
    int fid = 0;
    int count = 0;
    int fid_range[DSA_MAX_PORTS] = {0x64, 0x6e, 0x78, 0x82, 0x8c, 0x96, 0xa0, 0xaa, 0xb4, 0xbe, 0xc8, 0xd2};
    unsigned int memberPort = 0;
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->cfg_mutex);
    for (i = 0; i < DSA_MAX_PORTS; i++) 
    {
        if ((YT921X_USER_PORTMASK & BIT(i)) && i != port)
        {
            if (dsa_to_port(ds, i)->bridge_dev != bridge)
            {
                if (dsa_to_port(ds, i)->bridge_dev)
                {
                    yt921x_read(ds, YT_L2_VLAN_TBL(i + YT_PORT_VLAN_MAP), &tmpData1);
                    tmpData1 &= (0x1FF << DEFAULT_PORT_FID0_BITMAP_OFFSET);
                    tmpData1 = tmpData1 >> DEFAULT_PORT_FID0_BITMAP_OFFSET;
                    for (i = 0; i < DSA_MAX_PORTS; i++)
                    {
                        if (fid_range[i] == tmpData1)
                        {
                            fid_mask |= BIT(i);
                        }
                    }
                    count++;
                }
                continue;
            }
            
            index++;
            yt921x_rmw(ds, YT921X_PORT_ISOLATION_CTRL_BASE(i), BIT(port), 0);
#if defined (YT921X_DSA_WITH_VLAN_TAG)
            yt921x_read(ds, YT_L2_VLAN_TBL(i + YT_PORT_VLAN_MAP), &tmpData);
            tmpData |= BIT(port) << DEFAULT_PORT_MEMBER_BITMAP_OFFSET;
            yt921x_write(ds, YT_L2_VLAN_TBL(i + YT_PORT_VLAN_MAP), tmpData);
    
            yt921x_read(ds, YT_L2_VLAN_TBL(i + YT_PORT_VLAN_MAP) + 0x4, &tmpData);
            tmpData |= BIT(port) << DEFAULT_UNTAG_MEMBER_BITMAP_OFFSET;
            yt921x_write(ds, YT_L2_VLAN_TBL(i + YT_PORT_VLAN_MAP) + 0x4, tmpData);
      
            yt921x_read(ds, YT_L2_VLAN_TBL(i + YT_PORT_VLAN_MAP), &tmpData);
            fid = (tmpData >> DEFAULT_PORT_FID0_BITMAP_OFFSET) & (0x1FF);
            memberPort = (tmpData >> DEFAULT_PORT_MEMBER_BITMAP_OFFSET) & (0x7FF);
    #endif
            port_bitmap |= BIT(i);
        }
    }

    yt921x_rmw(ds, YT921X_PORT_ISOLATION_CTRL_BASE(port), port_bitmap, 0);
#if defined (YT921X_DSA_WITH_VLAN_TAG)
    
    yt921x_read(ds, YT921X_LEARN_PER_PORT_CTRL_BASE(port), &tmpData1);
    tmpData1 = tmpData1 & (~LEARN_DISABLE_MASK);
    yt921x_write(ds, YT921X_LEARN_PER_PORT_CTRL_BASE(port), tmpData1);

    if (index == 0)
    {
        if (count == 0)
        {
            fid = fid_range[0];
        }
        else
        {
            for (i = 0; i < DSA_MAX_PORTS; i++)
            {
                if (!(fid_mask & BIT(i)))
                {
                     fid = fid_range[i];
                     break;
                }
            }
        }
        yt921x_read(ds, YT_L2_VLAN_TBL(port + YT_PORT_VLAN_MAP), &tmpData1);
        tmpData1 &= ~(0x1FF << DEFAULT_PORT_FID0_BITMAP_OFFSET);
        tmpData1 |= (fid) << DEFAULT_PORT_FID0_BITMAP_OFFSET;
        yt921x_write(ds, YT_L2_VLAN_TBL(port + YT_PORT_VLAN_MAP), tmpData1);

        yt921x_read(ds, YT_L2_VLAN_TBL(port + YT_PORT_VLAN_MAP) + 0x4, &tmpData1);
        tmpData1 |= BIT(DEFAULT_PORT_SVL_MODE_OFFSET);
        yt921x_write(ds, YT_L2_VLAN_TBL(port + YT_PORT_VLAN_MAP) + 0x4, tmpData1);
        
    }
    else
    {
        yt921x_read(ds, YT_L2_VLAN_TBL(port + YT_PORT_VLAN_MAP), &tmpData1);
        tmpData1 &= ~(0x1FF << DEFAULT_PORT_FID0_BITMAP_OFFSET);
        tmpData1 |= (fid) << DEFAULT_PORT_FID0_BITMAP_OFFSET;
        tmpData1 &= ~(0x7FF << DEFAULT_PORT_MEMBER_BITMAP_OFFSET);
        tmpData1 |= memberPort << DEFAULT_PORT_MEMBER_BITMAP_OFFSET;
        yt921x_write(ds, YT_L2_VLAN_TBL(port + YT_PORT_VLAN_MAP), tmpData1);

        yt921x_read(ds, YT_L2_VLAN_TBL(port + YT_PORT_VLAN_MAP) + 0x4, &tmpData1);
        tmpData1 |= BIT(DEFAULT_PORT_SVL_MODE_OFFSET);
        tmpData1 &= ~(0x7FF << DEFAULT_UNTAG_MEMBER_BITMAP_OFFSET);
        tmpData1 |= memberPort << DEFAULT_UNTAG_MEMBER_BITMAP_OFFSET;
        tmpData1 &= ~(BIT(YT921X_CPU_PORT) << DEFAULT_UNTAG_MEMBER_BITMAP_OFFSET);
        yt921x_write(ds, YT_L2_VLAN_TBL(port + YT_PORT_VLAN_MAP) + 0x4, tmpData1);
    }
#endif

    mutex_unlock(&priv->cfg_mutex);

    return 0;
}

#else
static int
yt921x_port_bridge_join(struct dsa_switch *ds, int port, struct net_device *bridge)
{
    unsigned int port_bitmap = 0;
    int i;
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->cfg_mutex);

    for (i = 0; i < DSA_MAX_PORTS; i++)
    {
        if ((YT921X_USER_PORTMASK & BIT(i)) && i != port)
        {
            if (dsa_to_port(ds, i)->bridge_dev != bridge)
                continue;

            yt921x_rmw(ds, YT921X_PORT_ISOLATION_CTRL_BASE(i), BIT(port), 0);

            port_bitmap |= BIT(i);
        }
    }

    yt921x_rmw(ds, YT921X_PORT_ISOLATION_CTRL_BASE(port), port_bitmap, 0);
    mutex_unlock(&priv->cfg_mutex);

    return 0;
}
#endif

#if 1
static void
yt921x_port_bridge_leave(struct dsa_switch *ds, int port, struct net_device *bridge)
{
    unsigned int port_bitmap = 0;
    int i;
    int index = 0;
    int tmpData1;
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->cfg_mutex);
    for (i = 0; i < DSA_MAX_PORTS; i++) 
    {
        if ((YT921X_USER_PORTMASK & BIT(i)) && i != port)
        {
            if (dsa_to_port(ds, i)->bridge_dev != bridge)
                continue;
            index++;
            yt921x_rmw(ds, YT921X_PORT_ISOLATION_CTRL_BASE(i), BIT(port), BIT(port));
#if defined (YT921X_DSA_WITH_VLAN_TAG)

            yt921x_read(ds, YT_L2_VLAN_TBL(i + YT_PORT_VLAN_MAP), &tmpData1);
            tmpData1 &= ~(BIT(port) << DEFAULT_PORT_MEMBER_BITMAP_OFFSET);
            yt921x_write(ds, YT_L2_VLAN_TBL(i + YT_PORT_VLAN_MAP), tmpData1);

            yt921x_read(ds, YT_L2_VLAN_TBL(i + YT_PORT_VLAN_MAP) + 0x4, &tmpData1);
            tmpData1 &= ~(BIT(port) << DEFAULT_UNTAG_MEMBER_BITMAP_OFFSET);
            yt921x_write(ds, YT_L2_VLAN_TBL(i + YT_PORT_VLAN_MAP) + 0x4, tmpData1);
       
#endif            
            port_bitmap |= BIT(i);
        }
    }
    yt921x_rmw(ds, YT921X_PORT_ISOLATION_CTRL_BASE(port), port_bitmap, port_bitmap);
#if defined (YT921X_DSA_WITH_VLAN_TAG)
    yt921x_read(ds, YT921X_LEARN_PER_PORT_CTRL_BASE(port), &tmpData1);
    tmpData1 = tmpData1 | LEARN_DISABLE_MASK;
    yt921x_write(ds, YT921X_LEARN_PER_PORT_CTRL_BASE(port), tmpData1);


    if (index == 0)
    {
        yt921x_read(ds, YT_L2_VLAN_TBL(port + YT_PORT_VLAN_MAP), &tmpData1);
        tmpData1 &= ~(0x1FF << DEFAULT_PORT_FID0_BITMAP_OFFSET);
        yt921x_write(ds, YT_L2_VLAN_TBL(port + YT_PORT_VLAN_MAP), tmpData1);
    
        yt921x_read(ds, YT_L2_VLAN_TBL(port + YT_PORT_VLAN_MAP) + 0x4, &tmpData1);
        tmpData1 &= ~(BIT(DEFAULT_PORT_SVL_MODE_OFFSET));
        yt921x_write(ds,  YT_L2_VLAN_TBL(port + YT_PORT_VLAN_MAP) + 0x4, tmpData1);

    }
    else
    {
        yt921x_read(ds, YT_L2_VLAN_TBL(port + YT_PORT_VLAN_MAP), &tmpData1);
        tmpData1 &= ~(port_bitmap << DEFAULT_PORT_MEMBER_BITMAP_OFFSET);
        tmpData1 &= ~(0x1FF << DEFAULT_PORT_FID0_BITMAP_OFFSET);
        yt921x_write(ds, YT_L2_VLAN_TBL(port + YT_PORT_VLAN_MAP), tmpData1);

        yt921x_read(ds, YT_L2_VLAN_TBL(port + YT_PORT_VLAN_MAP) + 0x4, &tmpData1);
        tmpData1 &= ~(BIT(DEFAULT_PORT_SVL_MODE_OFFSET));
        tmpData1 &= ~(port_bitmap << DEFAULT_UNTAG_MEMBER_BITMAP_OFFSET);
        yt921x_write(ds, YT_L2_VLAN_TBL(port + YT_PORT_VLAN_MAP) + 0x4, tmpData1);
    }
#endif
    mutex_unlock(&priv->cfg_mutex);
}
#else
static void
yt921x_port_bridge_leave(struct dsa_switch *ds, int port, struct net_device *bridge)
{
    unsigned int port_bitmap = 0;
    int i;
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->cfg_mutex);
    for (i = 0; i < DSA_MAX_PORTS; i++)
    {
        if ((YT921X_USER_PORTMASK & BIT(i)) && i != port)
        {
            if (dsa_to_port(ds, i)->bridge_dev != bridge)
                continue;

            yt921x_rmw(ds, YT921X_PORT_ISOLATION_CTRL_BASE(i), BIT(port), BIT(port));

            port_bitmap |= BIT(i);
        }
    }

    yt921x_rmw(ds, YT921X_PORT_ISOLATION_CTRL_BASE(port), port_bitmap, port_bitmap);
    mutex_unlock(&priv->cfg_mutex);
}
#endif

static int
yt921x_port_fdb_add(struct dsa_switch *ds, int port, const unsigned char *addr, u16 vid)
{
    int ret;
    u32 regValue;
    u32 tmpValue;
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->cfg_mutex);
    regValue = (addr[0] << 24) | (addr[1] << 16) | (addr[2] << 8) | (addr[3]);
    yt921x_write(ds, YT921X_FDB_TBL_CTRL0_REG, regValue);

    tmpValue = vid;
    regValue = ((tmpValue << FID_OFFSET) & FID_MASK) | STATUS_VALUE_STATIC | (addr[4] << 8) | addr[5];
    yt921x_write(ds, YT921X_FDB_TBL_CTRL1_REG, regValue);

    regValue = (BIT(port)) << DST_PORTMASK_OFFSET;
    yt921x_write(ds, YT921X_FDB_TBL_CTRL2_REG, regValue);

    regValue = FDB_OP_START_EN | FDB_OP_CMD_ADD | FDB_OP_MODE_HASH;
    yt921x_write(ds, YT921X_FDB_TBL_OP_REG, regValue);

    ret = yt921x_fdb_op_result_get(ds);
    mutex_unlock(&priv->cfg_mutex);

    return ret;
}

static int
yt921x_port_fdb_del(struct dsa_switch *ds, int port, const unsigned char *addr, u16 vid)
{
    int ret;
    u32 regValue;
    u32 tmpValue;
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->cfg_mutex);
    regValue = (addr[0] << 24) | (addr[1] << 16) | (addr[2] << 8) | (addr[3]);
    yt921x_write(ds, YT921X_FDB_TBL_CTRL0_REG, regValue);

    tmpValue = vid;
    regValue = ((tmpValue << FID_OFFSET) & FID_MASK) | (addr[4] << 8) | addr[5];
    yt921x_write(ds, YT921X_FDB_TBL_CTRL1_REG, regValue);

    regValue = 0;
    yt921x_write(ds, YT921X_FDB_TBL_CTRL2_REG, regValue);

    regValue = FDB_OP_START_EN | FDB_OP_CMD_DEL | FDB_OP_MODE_HASH;
    yt921x_write(ds, YT921X_FDB_TBL_OP_REG, regValue);

    ret = yt921x_fdb_op_result_get(ds);
    mutex_unlock(&priv->cfg_mutex);

    return ret;
}

static int
yt921x_phy_read(struct dsa_switch *ds, int port, int regnum)
{
    int ret;
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->cfg_mutex);
    ret =  yt921x_slave_mdio_read(ds, port, regnum);
    mutex_unlock(&priv->cfg_mutex);

    return ret;
}

static int
yt921x_phy_write(struct dsa_switch *ds, int port, int regnum,
                u16 val)
{
    int ret;
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->cfg_mutex);
    ret =  yt921x_slave_mdio_write(ds, port, regnum, val);
    mutex_unlock(&priv->cfg_mutex);

    return ret;
}

static void
yt921x_mac_link_up(struct dsa_switch *ds, int port,
    unsigned int mode,
    phy_interface_t interface,
    struct phy_device *phydev,
    int speed, int duplex,
    bool tx_pause, bool rx_pause)
{
}

static void
yt921x_mac_link_down(struct dsa_switch *ds, int port,
    unsigned int mode,
    phy_interface_t interface)
{
}

static int
yt921x_get_sset_count(struct dsa_switch *ds, int port, int sset)
{
    return ARRAY_SIZE(yt921x_mib);
}

static int
yt921x_resume(struct dsa_switch *ds)
{
    yt921x_setup(ds);
    yt921x_init(ds->priv);
    return 0;
}

#elif (KERNEL_VERSION(4, 9, 0) <= LINUX_VERSION_CODE && KERNEL_VERSION(4, 10, 0) > LINUX_VERSION_CODE)

static enum dsa_tag_protocol
yt921x_get_tag_protocol(struct dsa_switch *ds)
{
#if defined(YT921X_DSA_WITHOUT_TAG)
    return DSA_TAG_PROTO_NONE;
#elif defined (YT921X_DSA_WITH_VLAN_TAG)
    return DSA_TAG_PROTO_MOTORCOMM_VLAN;
#else
    return DSA_TAG_PROTO_MOTORCOMM;
#endif
}

static int yt921x_setup(struct dsa_switch *ds)
{
    int i;

    yt921x_patch_default_init(ds);

    /* LED init */
    yt921x_led_init(ds);

    /* vlan init */
    yt921x_vlan_init(ds);

    /* Enable and reset MIB counters */
    yt921x_mib_init(ds);

    /* enable cpu_port */
    yt921x_cpu_port_enable(ds);

    /* ctrl pkt */
#ifdef YT921X_UNKNOWN_PKT_TO_CPU
    yt921x_ctrlpkt_init(ds);
#endif

    /* port isolation */
    for (i = 0; i < DSA_MAX_PORTS; i++)
    {
        if (YT921X_USER_PORTMASK & BIT(i))
        {
            /* Disable forwarding by default on all user ports */
            yt921x_rmw(ds, YT921X_PORT_ISOLATION_CTRL_BASE(i), YT921X_USER_PORTMASK, YT921X_USER_PORTMASK & (~BIT(i)));
        }
    }

    return 0;
}

static void
yt921x_adjust_link(struct dsa_switch *ds, int port,
                  struct phy_device *phydev)
{
    int i;
    unsigned int regData;

    if (port < DSA_MAX_PORTS && ds->ports[port].type == DSA_PORT_TYPE_CPU)
    {
        switch (phydev->interface)
        {
                case PHY_INTERFACE_MODE_SGMII:
                    {
                        switch (phydev->speed)
                        {
                            case 10:
                                printk(KERN_WARNING "the mode is unsupported now \n");
                                break;
                            case 100:
                                printk(KERN_WARNING "the mode is unsupported now \n");
                                break;
                            case 1000:
                                {
                                    /* P8/P9 sgmii 1G full duplex*/
                                    yt921x_write(ds, YT921X_INTERFACE_CTRL_REG, 0x0);
                                    yt921x_read(ds, YT921X_INTERFACE_SELECT_REG, &regData);
                                    regData &= ~(0xf << 0);
                                    regData |= 0x3;
                                    yt921x_write(ds, YT921X_INTERFACE_SELECT_REG, regData);
                                    yt921x_write(ds, YT921X_EXTIF_PORT_CTRL_REG(0), 0x1fa);
                                    yt921x_write(ds, YT921X_INTERFACE_SG_PHY_REG(0), 0xfa);
                                }
                                break;
                            default:
                                printk(KERN_WARNING "the speed is unsupported in SGMII \n");
                                break;
                        }
                    }
                    break;
            case PHY_INTERFACE_MODE_RGMII:
                {
                    switch (phydev->speed)
                    {
                        case 10:
                            printk(KERN_WARNING "the mode is unsupported\n");
                            break;
                        case 100:
                            printk(KERN_WARNING "the mode is unsupported\n");
                            break;
                        case 1000:
                            {
                                /* P8/P9 rgmii 1G full duplex*/
                                yt921x_write(ds, YT921X_INTERFACE_CTRL_REG, 0x3);
                                for (i = 0; i < 2; i++)
                                {
                                    yt921x_write(ds, YT921X_EXTIF_PORT_CTRL_REG(i), 0x1fa);
                                    yt921x_write(ds, YT921X_EXTIF_MODE_CTRL_REG(i), 0x841c0000);
                                    /* tx/rx delay */
                                    yt921x_rmw(ds, YT921X_EXTIF_MODE_CTRL_REG(i), RGMII_RXC_DELAY_SEL_MASK | RGMII_TXC_DELAY_SEL_MASK ,
                                        (RGMII_RXC_DELAY_SEL_DEFAULT_VAL << RGMII_RXC_DELAY_SEL_OFFSET) | (RGMII_TXC_DELAY_SEL_DEFAULT_VAL << RGMII_TXC_DELAY_SEL_OFFSET));
                                }
                            }
                            break;
                        default:
                            printk(KERN_WARNING "the speed is unsupported in RGMII \n");
                            break;
                    }
                }
                break;
            case PHY_INTERFACE_MODE_2500BASEX:
                {
                    int j;
                    for (j = 0; j < 2; j++)
                    {
                        yt921x_write(ds, YT921X_INTERFACE_CTRL_REG, 0x0);
                        yt921x_write(ds, YT921X_INTERFACE_SELECT_REG, 0x3);
                        yt921x_write(ds, YT921X_INTERFACE_SG_PHY_REG(j), 0x2fc);
                    }
                }
                break;
            default:
                printk(KERN_WARNING "the mode is unsupported\n");
                break;
        }
    }
    return;
}

static void
yt921x_get_strings(struct dsa_switch *ds, int port, u8 *data)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(yt921x_mib); i++)
    {
        strncpy(data + i * ETH_GSTRING_LEN, yt921x_mib[i].name,
            ETH_GSTRING_LEN);
    }
}

static void
yt921x_get_ethtool_stats(struct dsa_switch *ds, int port, u64 *data)
{
    int i;
    u32 lowData = 0;
    u32 highData = 0;
    u64 resultData = 0;
    int mibCount;
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->cfg_mutex);

    mibCount = ARRAY_SIZE(yt921x_mib);
    for (i = 0; i < mibCount; i++)
    {
        yt921x_read(ds, YT921X_PORT_MIB_BASE(port) + yt921x_mib[i].offset, &lowData);
        data[i] = lowData;

        if (yt921x_mib[i].size == 2)
        {
            yt921x_read(ds, YT921X_PORT_MIB_BASE(port) + yt921x_mib[i].offset + 4, &highData);
            resultData = highData;

            data[i] |= resultData << 32;
        }
    }

    mutex_unlock(&priv->cfg_mutex);
}

static int
yt921x_get_sset_count(struct dsa_switch *ds)
{
    return ARRAY_SIZE(yt921x_mib);
}

static int
yt921x_vlan_filtering(struct dsa_switch *ds, int port, bool vlan_filtering)
{
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->cfg_mutex);
    yt921x_vlan_filter_set(ds, port, vlan_filtering);
    mutex_unlock(&priv->cfg_mutex);

    return 0;
}

static int
yt921x_vlan_prepare(struct dsa_switch *ds, int port,
             const struct switchdev_obj_port_vlan *vlan)
{
    return 0;
}

static void
yt921x_vlan_add(struct dsa_switch *ds, int port,
    const struct switchdev_obj_port_vlan *vlan,
    struct switchdev_trans *trans)
{
    bool untagged = vlan->flags & BRIDGE_VLAN_INFO_UNTAGGED;
    bool pvid = vlan->flags & BRIDGE_VLAN_INFO_PVID;
    u32 member = 0;
    u32 untag = 0;
    u16 vid;
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->cfg_mutex);

    for (vid = vlan->vid_begin; vid <= vlan->vid_end; ++vid) {
        yt921x_vlan_port_get(ds, vid, &member, &untag);

        member |= BIT(port);
        if (untagged)
        {
            untag |= BIT(port);
        }
        else
        {
            untag &= ~(BIT(port));
        }

        /* add cpu port to all vlan */
        member |= BIT(YT921X_CPU_PORT);
        yt921x_vlan_port_set(ds, vid, member, untag);
    }

    /* vid_end as pvid */
    if(pvid)
    {
        yt921x_vlan_pvid_set(ds, port, vlan->vid_end);
    }

    mutex_unlock(&priv->cfg_mutex);
}

static int
yt921x_vlan_del(struct dsa_switch *ds, int port, const struct switchdev_obj_port_vlan *vlan)
{
    bool untagged = vlan->flags & BRIDGE_VLAN_INFO_UNTAGGED;
    u32 member = 0;
    u32 untag = 0;
    u16 vid;
    u16 pvid;
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->cfg_mutex);

    for (vid = vlan->vid_begin; vid <= vlan->vid_end; ++vid) {
        yt921x_vlan_port_get(ds, vid, &member, &untag);

        member &= ~(BIT(port));
        if (untagged)
        {
            untag &= ~(BIT(port));
        }
        yt921x_vlan_port_set(ds, vid, member, untag);
    }

    yt921x_vlan_pvid_get(ds, port, &pvid);
    if ((vlan->vid_begin <= pvid) && (pvid <= vlan->vid_end))
    {
        yt921x_vlan_pvid_set(ds, port, DEFAULT_PORT_VID);
    }

    mutex_unlock(&priv->cfg_mutex);

    return 0;
}

static int
yt921x_port_enable(struct dsa_switch *ds, int port,
              struct phy_device *phy)
{
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->cfg_mutex);
    yt921x_port_enable_set(ds, port, 1);
    mutex_unlock(&priv->cfg_mutex);

    return 0;
}

static void
yt921x_port_disable(struct dsa_switch *ds, int port,
                struct phy_device *phy)
{
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->cfg_mutex);
    yt921x_port_enable_set(ds, port, 0);
    mutex_unlock(&priv->cfg_mutex);
}

static void
yt921x_stp_state_set(struct dsa_switch *ds, int port, u8 state)
{
    u32 stp_state;

    switch (state) {
        case BR_STATE_DISABLED:
            stp_state = STP_STATE_DISCARD;
            break;

        case BR_STATE_BLOCKING:
            stp_state = STP_STATE_DISCARD;
            break;

        case BR_STATE_LISTENING:
            stp_state = STP_STATE_DISCARD;
            break;

        case BR_STATE_LEARNING:
            stp_state = STP_STATE_LEARN;
            break;

        case BR_STATE_FORWARDING:
        default:
            stp_state = STP_STATE_FORWARD;
            break;
    }

    yt921x_rmw(ds, YT921X_STP_STATE_REG(port), PORT_STP_STATE_MASK(port), stp_state << PORT_STP_STATE_OFFSET(port));
}

static int
yt921x_port_bridge_join(struct dsa_switch *ds, int port, struct net_device *bridge)
{
    unsigned int port_bitmap = 0;
    int i;
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->cfg_mutex);

    for (i = 0; i < DSA_MAX_PORTS; i++)
    {
        if ((YT921X_USER_PORTMASK & BIT(i)) && i != port)
        {
            if (dsa_to_port(ds, i)->bridge_dev != bridge)
                continue;

            yt921x_rmw(ds, YT921X_PORT_ISOLATION_CTRL_BASE(i), BIT(port), 0);

            port_bitmap |= BIT(i);
        }
    }

    yt921x_rmw(ds, YT921X_PORT_ISOLATION_CTRL_BASE(port), port_bitmap, 0);
    mutex_unlock(&priv->cfg_mutex);

    return 0;
}

static void
yt921x_port_bridge_leave(struct dsa_switch *ds, int port)
{
    unsigned int port_bitmap = 0;
    int i;
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->cfg_mutex);
    for (i = 0; i < DSA_MAX_PORTS; i++)
    {
        if ((YT921X_USER_PORTMASK & BIT(i)) && i != port)
        {
            yt921x_rmw(ds, YT921X_PORT_ISOLATION_CTRL_BASE(i), BIT(port), BIT(port));

            port_bitmap |= BIT(i);
        }
    }

    yt921x_rmw(ds, YT921X_PORT_ISOLATION_CTRL_BASE(port), port_bitmap, port_bitmap);
    mutex_unlock(&priv->cfg_mutex);
}

static int
yt921x_port_fdb_add(struct dsa_switch *ds, int port,
    const struct switchdev_obj_port_fdb *fdb,
    struct switchdev_trans *trans)
{
    int ret;
    u32 regValue;
    u32 tmpValue;
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->cfg_mutex);
    regValue = (addr[0] << 24) | (addr[1] << 16) | (addr[2] << 8) | (addr[3]);
    yt921x_write(ds, YT921X_FDB_TBL_CTRL0_REG, regValue);

    tmpValue = fdb->vid;
    regValue = ((tmpValue << FID_OFFSET) & FID_MASK) | STATUS_VALUE_STATIC | (addr[4] << 8) | addr[5];
    yt921x_write(ds, YT921X_FDB_TBL_CTRL1_REG, regValue);

    regValue = (BIT(port)) << DST_PORTMASK_OFFSET;
    yt921x_write(ds, YT921X_FDB_TBL_CTRL2_REG, regValue);

    regValue = FDB_OP_START_EN | FDB_OP_CMD_ADD | FDB_OP_MODE_HASH;
    yt921x_write(ds, YT921X_FDB_TBL_OP_REG, regValue);

    ret = yt921x_fdb_op_result_get(ds);
    mutex_unlock(&priv->cfg_mutex);

    return ret;
}

static int
yt921x_port_fdb_del(struct dsa_switch *ds, int port,
    const struct switchdev_obj_port_fdb *fdb)
{
    int ret;
    u32 regValue;
    u32 tmpValue;
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->cfg_mutex);
    regValue = (addr[0] << 24) | (addr[1] << 16) | (addr[2] << 8) | (addr[3]);
    yt921x_write(ds, YT921X_FDB_TBL_CTRL0_REG, regValue);

    tmpValue = fdb->vid;
    regValue = ((tmpValue << FID_OFFSET) & FID_MASK) | (addr[4] << 8) | addr[5];
    yt921x_write(ds, YT921X_FDB_TBL_CTRL1_REG, regValue);

    regValue = 0;
    yt921x_write(ds, YT921X_FDB_TBL_CTRL2_REG, regValue);

    regValue = FDB_OP_START_EN | FDB_OP_CMD_DEL | FDB_OP_MODE_HASH;
    yt921x_write(ds, YT921X_FDB_TBL_OP_REG, regValue);

    ret = yt921x_fdb_op_result_get(ds);
    mutex_unlock(&priv->cfg_mutex);

    return ret;
}

static int
yt921x_phy_read(struct dsa_switch *ds, int port, int regnum)
{
    int ret;
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->cfg_mutex);
    ret =  yt921x_slave_mdio_read(ds, port, regnum);
    mutex_unlock(&priv->cfg_mutex);

    return ret;
}

static int
yt921x_phy_write(struct dsa_switch *ds, int port, int regnum,
                u16 val)
{
    int ret;
    struct motorcomm_priv *priv = ds->priv;

    mutex_lock(&priv->cfg_mutex);
    ret =  yt921x_slave_mdio_write(ds, port, regnum, val);
    mutex_unlock(&priv->cfg_mutex);

    return ret;
}

static void
yt921x_mac_link_up(struct dsa_switch *ds, int port,
    unsigned int mode,
    phy_interface_t interface,
    struct phy_device *phydev,
    int speed, int duplex,
    bool tx_pause, bool rx_pause)
{
}

static void
yt921x_mac_link_down(struct dsa_switch *ds, int port,
    unsigned int mode,
    phy_interface_t interface)
{
}

static int
yt921x_resume(struct dsa_switch *ds)
{
    yt921x_setup(ds);
    yt921x_init(ds->priv);
    return 0;
}

#endif // kernel 4.9

static void
yt921x_phy_disable(struct dsa_switch *ds, int port)
{
    u16 tmpValue;
    const struct dsa_port *dp = dsa_to_port(ds, port);

    if (dp->pl)
    {
        rtnl_lock();
        phylink_stop(dp->pl);
        rtnl_unlock();
    }

    /* phy linkdown */
    tmpValue = yt921x_phy_read(ds, port, PHY_BASE_CTRL_REG);
    tmpValue |= 0x800;
    yt921x_phy_write(ds, port, PHY_BASE_CTRL_REG, tmpValue);
}

static void
yt921x_led_default_init(struct dsa_switch *ds)
{
    int port = 0;

    yt921x_rmw(ds, LED_CFG_DONE_REG, LED_CFG_DONE_OFFSET, 1);
    for (port = 0; port < DSA_MAX_PORTS - 2; port++)
    {
        yt921x_rmw(ds, LED_CTRL_REG(port), DISABLE_LED_LINK_TRY_OFFSET, 1);
    }
}

static void
yt921x_ac_thresholds_init(struct dsa_switch *ds)
{
    unsigned int regdata;
    unsigned int regdata2;
    unsigned int regaddr;
    int i = 0;
    int j = 0;

    for (i = 0; i < DSA_MAX_PORTS - 1; i++)
    {
        for (j = 0; j < YT921X_AC_QUEUE_MAX; j++)
        {
            regaddr = UCAST_QUEUE_REG(i, j);
            yt921x_read(ds, regaddr, &regdata);
            yt921x_read(ds, regaddr + 0x4, &regdata2);
            regdata &= ~(0xff);
            regdata |= 0x20;
            yt921x_write(ds, regaddr, regdata);
            yt921x_write(ds, regaddr + 0x4, regdata2);

        }
        for (j = 0; j < YT921X_MC_QUEUE_MAX; j++)
        {
            regaddr = MCAST_QUEUE_REG(i, j);
            yt921x_read(ds, regaddr, &regdata);
            regdata &= ~(0xff);
            regdata |= 0xc;
            yt921x_write(ds, regaddr, regdata);
        }
    }
}

static void
yt921x_fc_cfg_init(struct dsa_switch *ds)
{
    int i = 0;
    unsigned int regChip;
    yt921x_read(ds, YT921X_CHIP_ID_REG, &regChip);

    for (i = 0; i < DSA_MAX_PORTS - 1; i++)
    {
            yt921x_write(ds, PORT_FC_CFG_TBL0(i), 0x8040284b);
            yt921x_write(ds, PORT_FC_CFG_TBL1(i), 0x26f1b);
    }

    if (0x9002 == (regChip>>16 & 0x0ffff))
    {
        for (i = 0; i < 4; i++)
        {
            yt921x_write(ds, PORT_FC_SHARED_GRP_CTRL(i), 0x14a);
        }
    }
    else
    {
        /* other chip version keep default value */
    }
}

static void
yt921x_patch_init(struct dsa_switch *ds)
{
    int phy_num;
    unsigned int regdata;
    unsigned int regChip;
    unsigned int phyAddr;
    unsigned int bit;
    u16 tmpValue;

    yt921x_read(ds, YT921X_CHIP_ID_REG, &regChip);

    /* for xmii on low temp */
    yt921x_read(ds, YT921X_EXTIF0_MODE, &regdata);
    regdata |= (1 << 11);
    yt921x_write(ds, YT921X_EXTIF0_MODE, regdata);

    /* ac */
    yt921x_read(ds, YT921X_GLOBAL_CTRL1_REG, &regdata);
    regdata |= ACL_EN_OFFSET;
    if (0x9001 == (regChip>>16 & 0x0ffff))
    {
        regdata |= AC_EN_OFFSET;
    }
    yt921x_write(ds, YT921X_GLOBAL_CTRL1_REG, regdata);

    /* mdio */
    yt921x_read(ds, YT921X_INTERFACE_SELECT_REG, &regdata);
    regdata &= ~(0x43<<0);
    yt921x_write(ds, YT921X_INTERFACE_SELECT_REG, regdata);

    /* vga */
    for (phy_num= 0; phy_num < 8; phy_num++)
    {
        /*vga init*/
        if (0x9001 == (regChip>>16 & 0x0ffff))
        {
            yt921x_phy_write(ds, phy_num, 0x1e, 0x50);
            tmpValue = yt921x_phy_read(ds, phy_num, 0x1f);
            tmpValue &= 0xF3FF;
            yt921x_phy_write(ds, phy_num, 0x1e, 0x50);
            yt921x_phy_write(ds, phy_num, 0x1f, tmpValue);
        }

        /* UTP init */
        yt921x_phy_write(ds, phy_num, 0x1e, 0x29);
        tmpValue = yt921x_phy_read(ds, phy_num, 0x1f);
        tmpValue &= (~(0x3F));
        tmpValue |= 0x8;
        yt921x_phy_write(ds, phy_num, 0x1e, 0x29);
        yt921x_phy_write(ds, phy_num, 0x1f, tmpValue);

        /* csd ca */
        yt921x_phy_write(ds, phy_num, 0x1e, 0x408);
        tmpValue = yt921x_phy_read(ds, phy_num, 0x1f);
        tmpValue &= 0x0FFFE;
        yt921x_phy_write(ds, phy_num, 0x1e, 0x408);
        yt921x_phy_write(ds, phy_num, 0x1f, tmpValue);

    }

    /*serdes init */
    for (phyAddr = 8; phyAddr <= 9; phyAddr++)
    {
        /*cp current */
        yt921x_phy_write(ds, phyAddr, 0x1e, 0xa);
        tmpValue = yt921x_phy_read(ds, phyAddr, 0xa);
        if (0x9002 == (regChip>>16 & 0x0ffff))
        {
            bit = 14;
        }
        else
        {
            bit = 13;
        }
        tmpValue &= ~(0x1 << bit);
        yt921x_phy_write(ds, phyAddr, 0x1e, 0xa);
        yt921x_phy_write(ds, phyAddr, 0x1f, tmpValue);

        /*vco band init */
        yt921x_phy_write(ds, phyAddr, 0x1e, 0x2e);
        tmpValue = yt921x_phy_read(ds, phyAddr, 0x1f);
        tmpValue |= (0x1 << 9);
        yt921x_phy_write(ds, phyAddr, 0x1e, 0x2e);
        yt921x_phy_write(ds, phyAddr, 0x1f, tmpValue);

        /*serdes as mode */
        yt921x_phy_write(ds, phyAddr, 0x1e, 0x1d0);
        tmpValue = yt921x_phy_read(ds, phyAddr, 0x1f);
        tmpValue &= ~(0xff << 0);
        tmpValue |= (0x80<<0);
        yt921x_phy_write(ds, phyAddr, 0x1e, 0x1d0);
        yt921x_phy_write(ds, phyAddr, 0x1f, tmpValue);
    }
}

static void
yt921x_mac_speed_init(struct dsa_switch *ds)
{
    u32 regValue = 0;
    yt921x_write(ds, 0xf0004, 0x1e0904);
    yt921x_write(ds, 0xf0008, 0x0);
    yt921x_write(ds, 0xf0000, 0x1);
    yt921x_read(ds, 0xf0000, &regValue);
    while (regValue)
    {
        /* delay */
    }
    yt921x_write(ds, 0xf0004, 0x1f0904);
    yt921x_write(ds, 0xf0008, 0x1c0d);
    yt921x_write(ds, 0xf0000, 0x1);
    yt921x_read(ds, 0xf0000, &regValue);
    while (regValue)
    {
        /* delay */
    }

    yt921x_write(ds, 0xf0004, 0x3e0904);
    yt921x_write(ds, 0xf0008, 0x0);
    yt921x_write(ds, 0xf0000, 0x1);
    yt921x_read(ds, 0xf0000, &regValue);
    while (regValue)
    {
        /* delay */
    }
    yt921x_write(ds, 0xf0004, 0x3f0904);
    yt921x_write(ds, 0xf0008, 0x1c0d);
    yt921x_write(ds, 0xf0000, 0x1);
    yt921x_read(ds, 0xf0000, &regValue);
    while (regValue)
    {
        /* delay */
    }

    yt921x_write(ds, 0xf0004, 0x5e0904);
    yt921x_write(ds, 0xf0008, 0x0);
    yt921x_write(ds, 0xf0000, 0x1);
    yt921x_read(ds, 0xf0000, &regValue);
    while (regValue)
    {
        /* delay */
    }
    yt921x_write(ds, 0xf0004, 0x5f0904);
    yt921x_write(ds, 0xf0008, 0x1c0d);
    yt921x_write(ds, 0xf0000, 0x1);
    yt921x_read(ds, 0xf0000, &regValue);
    while (regValue)
    {
        /* delay */
    }

    yt921x_write(ds, 0xf0004, 0x7e0904);
    yt921x_write(ds, 0xf0008, 0x0);
    yt921x_write(ds, 0xf0000, 0x1);
    yt921x_read(ds, 0xf0000, &regValue);
    while (regValue)
    {
        /* delay */
    }
    yt921x_write(ds, 0xf0004, 0x7f0904);
    yt921x_write(ds, 0xf0008, 0x1c0d);
    yt921x_write(ds, 0xf0000, 0x1);
    yt921x_read(ds, 0xf0000, &regValue);
    while (regValue)
    {
        /* delay */
    }

    yt921x_write(ds, 0xf0004, 0x9e0904);
    yt921x_write(ds, 0xf0008, 0x0);
    yt921x_write(ds, 0xf0000, 0x1);
    yt921x_read(ds, 0xf0000, &regValue);
    while (regValue)
    {
        /* delay */
    }
    yt921x_write(ds, 0xf0004, 0x9f0904);
    yt921x_write(ds, 0xf0008, 0x1c0d);
    yt921x_write(ds, 0xf0000, 0x1);
    yt921x_read(ds, 0xf0000, &regValue);
    while (regValue)
    {
        /* delay */
    }
}
static void
yt921x_switch_default_cfg(struct dsa_switch *ds)
{
    yt921x_led_default_init(ds);
    yt921x_ac_thresholds_init(ds);
    yt921x_fc_cfg_init(ds);
    //yt921x_mac_speed_init(ds);
    yt921x_patch_init(ds);
}

#if 1
static void
yt_always_vlan_tag(struct dsa_switch *ds)
{
    int port = 0;
    int egr_tpid = 0;
    unsigned int regdata;

    for (port = 0; port < DSA_MAX_PORTS - 1; port++)
    {
        if (YT921X_ALL_PORTMASK & BIT(port))
        {
            yt921x_write(ds, YT_PARSER_PORT_CTRLN(port), 0x0);
        }
    }
#if defined(YT921X_DSA_WITHOUT_TAG)
	for (egr_tpid = 0; egr_tpid < 2; egr_tpid++)
	{
		yt921x_write(ds, YT_EGR_TPID_PROFILE(egr_tpid), 0x8100);
	}
	for (egr_tpid = 2; egr_tpid < 4; egr_tpid++)
	{
		yt921x_write(ds, YT_EGR_TPID_PROFILE(egr_tpid), 0x88a8);
	}
	yt921x_write(ds, YT_EGR_PORT_CTRLN(YT921X_CPU_PORT), 0x10);
#elif defined(YT921X_DSA_WITH_VLAN_TAG)
    for (egr_tpid = 0; egr_tpid < 4; egr_tpid++)
    {
        yt921x_write(ds, YT_EGR_TPID_PROFILE(egr_tpid), YT_ETH_VLAN_TPID);
        yt921x_write(ds, YT_ING_TPID_PROFILE(egr_tpid), YT_ETH_VLAN_TPID);
    }
    for (port = 0; port < DSA_MAX_PORTS - 1; port++)
    {
        if (BIT(port) & YT921X_ALL_PORTMASK)
        {
            yt921x_read(ds, YT_EGR_PORT_CTRLN(port), &regdata);
            regdata &= ~(0xf << 2);
            regdata |= (0x5 << 2);
            yt921x_write(ds, YT_EGR_PORT_CTRLN(port), regdata);
        }
    }
#endif
    yt921x_write(ds, YT_PARSER_PORT_CTRLN(YT921X_CPU_PORT), 0x1);
    for (port = 0; port < DSA_MAX_PORTS - 1; port++)
    {
        if (YT921X_USER_PORTMASK & BIT(port))
        {
            yt921x_rmw(ds, YT_L2_VLAN_TBL(port + YT_PORT_VLAN_MAP), DEFAULT_PORT_MEMBER_BITMAP, (BIT(port) | BIT(YT921X_CPU_PORT)) << DEFAULT_PORT_MEMBER_BITMAP_OFFSET);
            yt921x_rmw(ds, YT_L2_VLAN_TBL(port + YT_PORT_VLAN_MAP) + 0x4, DEFAULT_UNTAG_MEMBER_BITMAP, BIT(port) << DEFAULT_UNTAG_MEMBER_BITMAP_OFFSET);
            yt921x_rmw(ds, DEFAULT_PORT_VLAN_CTRLN(port), DEFAULT_CVID, (port + YT_PORT_VLAN_MAP) << DEFAULT_CVID_OFFSET);

            /* index 10 */
            yt921x_rmw(ds, YT_L2_VLAN_TBL(port + YT_PORT_XMIT_VLAN_MAP), DEFAULT_PORT_MEMBER_BITMAP, (BIT(port) | BIT(YT921X_CPU_PORT)) << DEFAULT_PORT_MEMBER_BITMAP_OFFSET);
            yt921x_rmw(ds, YT_L2_VLAN_TBL(port + YT_PORT_XMIT_VLAN_MAP) + 0x4, DEFAULT_UNTAG_MEMBER_BITMAP, BIT(port) << DEFAULT_UNTAG_MEMBER_BITMAP_OFFSET);
            yt921x_rmw(ds, DEFAULT_PORT_VLAN_CTRLN(port), DEFAULT_CVID, (port + YT_PORT_VLAN_MAP) << DEFAULT_CVID_OFFSET);
        }
    }
    yt921x_write(ds, L2_VLAN_INGRESS_FILTER_EN, YT921X_ALL_PORTMASK);
    yt921x_write(ds, L2_EGR_VLAN_FILTER_EN, YT921X_ALL_PORTMASK);
    for (port = 0; port < DSA_MAX_PORTS - 1; port++)
    {
        if (BIT(port) & YT921X_ALL_PORTMASK)
        {
            yt921x_write(ds, EGR_PORT_VLAN_CTRLN(port), 0x2000d001);
        }
    }
}


#else
static void
yt_always_vlan_tag(struct dsa_switch *ds)
{
    int port = 0;
    int egr_tpid = 0;
    unsigned int regdata;

    for (port = 0; port < DSA_MAX_PORTS - 1; port++)
    {
        if (YT921X_ALL_PORTMASK & BIT(port))
        {
            yt921x_write(ds, YT_PARSER_PORT_CTRLN(port), 0x0);
        }
    }
#if defined(YT921X_DSA_WITHOUT_TAG)
	for (egr_tpid = 0; egr_tpid < 2; egr_tpid++)
	{
		yt921x_write(ds, YT_EGR_TPID_PROFILE(egr_tpid), 0x8100);
	}
	for (egr_tpid = 2; egr_tpid < 4; egr_tpid++)
	{
		yt921x_write(ds, YT_EGR_TPID_PROFILE(egr_tpid), 0x88a8);
	}
	yt921x_write(ds, YT_EGR_PORT_CTRLN(YT921X_CPU_PORT), 0x10);
#elif defined(YT921X_DSA_WITH_VLAN_TAG)
    for (egr_tpid = 0; egr_tpid < 4; egr_tpid++)
    {
        yt921x_write(ds, YT_EGR_TPID_PROFILE(egr_tpid), YT_ETH_VLAN_TPID);
        yt921x_write(ds, YT_ING_TPID_PROFILE(egr_tpid), YT_ETH_VLAN_TPID);
    }
    for (port = 0; port < DSA_MAX_PORTS - 1; port++)
    {
        if (BIT(port) & YT921X_ALL_PORTMASK)
        {
            yt921x_read(ds, YT_EGR_PORT_CTRLN(port), &regdata);
            regdata &= ~(0xf << 2);
            regdata |= (0x5 << 2);
            yt921x_write(ds, YT_EGR_PORT_CTRLN(port), regdata);
        }
    }
#endif
    yt921x_write(ds, YT_PARSER_PORT_CTRLN(YT921X_CPU_PORT), 0x1);
    for (port = 0; port < DSA_MAX_PORTS - 1; port++)
    {
        if (YT921X_USER_PORTMASK & BIT(port))
        {
            yt921x_rmw(ds, YT_L2_VLAN_TBL(port + YT_PORT_VLAN_MAP), DEFAULT_PORT_MEMBER_BITMAP, (BIT(port) | BIT(YT921X_CPU_PORT)) << DEFAULT_PORT_MEMBER_BITMAP_OFFSET);
            yt921x_rmw(ds, YT_L2_VLAN_TBL(port + YT_PORT_VLAN_MAP) + 0x4, DEFAULT_UNTAG_MEMBER_BITMAP, BIT(port) << DEFAULT_UNTAG_MEMBER_BITMAP_OFFSET);
            yt921x_rmw(ds, DEFAULT_PORT_VLAN_CTRLN(port), DEFAULT_CVID, (port + YT_PORT_VLAN_MAP) << DEFAULT_CVID_OFFSET);
        }
    }
    yt921x_write(ds, L2_VLAN_INGRESS_FILTER_EN, YT921X_ALL_PORTMASK);
    yt921x_write(ds, L2_EGR_VLAN_FILTER_EN, YT921X_ALL_PORTMASK);
    for (port = 0; port < DSA_MAX_PORTS - 1; port++)
    {
        if (BIT(port) & YT921X_ALL_PORTMASK)
        {
            yt921x_write(ds, EGR_PORT_VLAN_CTRLN(port), 0x2000d001);
        }
    }
}
#endif

static void
yt921x_fdb_learning_disable(struct dsa_switch *ds, int port)
{
    yt921x_rmw(ds, YT921X_LEARN_PER_PORT_CTRL_BASE(port), LEARN_DISABLE_MASK, LEARN_DISABLE_MASK);
}

void yt921x_init(struct motorcomm_priv *priv)
{
    int port;
    struct dsa_switch *ds;
    const struct dsa_port *dp;

    ds = priv->ds;
    /* Disable ports */
    for (port = 0; port < DSA_MAX_PORTS; port++)
    {
        dp = dsa_to_port(ds, port);
        if (YT921X_USER_PORTMASK & BIT(port))
        {
            if (dp->pl && dp->pl->phydev)
            {
                if (dp->pl->phydev->state == PHY_DOWN ||
                    dp->pl->phydev->state == PHY_HALTED ||
                    dp->pl->phydev->state == PHY_NOLINK ||
                    dp->pl->phydev->state == PHY_READY)
                {
                    /* Disable mac tx/rx */
					pr_err("disable port %d phy\n", port);
                    yt921x_port_disable(priv->ds, port);

                    /* disable phy */
                    yt921x_phy_disable(priv->ds, port);
                }
            }
            /* disable fdb learning for user port */
            yt921x_fdb_learning_disable(priv->ds, port);
        }
    }
#if defined(YT921X_DSA_WITHOUT_TAG) ||defined(YT921X_DSA_WITH_VLAN_TAG)
    yt_always_vlan_tag(priv->ds);
#endif
	yt921x_switch_default_cfg(priv->ds);
#ifdef CLS_YT_DSA_PORT
	test_call_esw_cli();
#endif
}

static const struct dsa_switch_ops yt921x_switch_ops = {
    .get_tag_protocol = yt921x_get_tag_protocol,
    .setup = yt921x_setup,
    .adjust_link = yt921x_adjust_link,
    .get_strings = yt921x_get_strings,
    .get_ethtool_stats = yt921x_get_ethtool_stats,
    .get_sset_count = yt921x_get_sset_count,
    .port_vlan_filtering = yt921x_vlan_filtering,
    .port_vlan_prepare = yt921x_vlan_prepare,
    .port_vlan_add = yt921x_vlan_add,
    .port_vlan_del = yt921x_vlan_del,
    .port_enable = yt921x_port_enable,
    .port_disable = yt921x_port_disable,
    .port_stp_state_set = yt921x_stp_state_set,
    .port_bridge_join   = yt921x_port_bridge_join,
    .port_bridge_leave  = yt921x_port_bridge_leave,
    .port_fdb_add       = yt921x_port_fdb_add,
    .port_fdb_del       = yt921x_port_fdb_del,
    .phy_read       = yt921x_phy_read,
    .phy_write      = yt921x_phy_write,
    .phylink_mac_link_up = yt921x_mac_link_up,
    .phylink_mac_link_down = yt921x_mac_link_down,
    .resume = yt921x_resume,
    /*
    .port_pre_bridge_flags = yt921x_port_pre_bridge_flags,
    .port_bridge_flags = yt921x_port_bridge_flags,
     */
};

static const struct motorcomm_chip_ops yt921x_chip_ops = {
    .init = yt921x_init,
};

const struct motorcomm_mdio_variant yt921x_variant = {
    .switchId = YT921X_SWITCH_ID,
    .devAddr = YT921X_SWITCH_PHY_ADDR,
    .ds_ops = &yt921x_switch_ops,
    .ops = &yt921x_chip_ops,
};
EXPORT_SYMBOL_GPL(yt921x_variant);
