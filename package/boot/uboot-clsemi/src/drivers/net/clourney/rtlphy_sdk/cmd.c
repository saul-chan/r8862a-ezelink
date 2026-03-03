#include <stdio.h>
#include <common.h>
#include <command.h>
#include <net.h>
#include <netdev.h>
#include <miiphy.h>
#include <phy.h>
#include <asm/io.h>
#include <linux/io.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <miiphy.h>
#include <phy.h>
#define uint32 uint32_t
#define int32 int32_t

typedef enum rt_error_common_e
{
    RT_ERR_FAILED = -1,                             /* General Error                                                                    */

    /* 0x0000xxxx for common error code */
    RT_ERR_OK = 0,                                  /* 0x00000000, OK                                                                   */
    RT_ERR_INPUT = 0xF001,                          /* 0x0000F001, invalid input parameter                                              */
    RT_ERR_UNIT_ID,                                 /* 0x0000F002, invalid unit id                                                      */
    RT_ERR_PORT_ID,                                 /* 0x0000F003, invalid port id                                                      */
    RT_ERR_PORT_MASK,                               /* 0x0000F004, invalid port mask                                                    */
    RT_ERR_PORT_LINKDOWN,                           /* 0x0000F005, link down port status                                                */
    RT_ERR_ENTRY_INDEX,                             /* 0x0000F006, invalid entry index                                                  */
    RT_ERR_NULL_POINTER,                            /* 0x0000F007, input parameter is null pointer                                      */
    RT_ERR_QUEUE_ID,                                /* 0x0000F008, invalid queue id                                                     */
    RT_ERR_QUEUE_NUM,                               /* 0x0000F009, invalid queue number                                                 */
    RT_ERR_BUSYWAIT_TIMEOUT,                        /* 0x0000F00a, busy watting time out                                                */
    RT_ERR_MAC,                                     /* 0x0000F00b, invalid mac address                                                  */
    RT_ERR_OUT_OF_RANGE,                            /* 0x0000F00c, input parameter out of range                                         */
    RT_ERR_CHIP_NOT_SUPPORTED,                      /* 0x0000F00d, functions not supported by this chip model                           */
    RT_ERR_SMI,                                     /* 0x0000F00e, SMI error                                                            */
    RT_ERR_NOT_INIT,                                /* 0x0000F00f, The module is not initial                                            */
    RT_ERR_CHIP_NOT_FOUND,                          /* 0x0000F010, The chip can not found                                               */
    RT_ERR_NOT_ALLOWED,                             /* 0x0000F011, actions not allowed by the function                                  */
    RT_ERR_DRIVER_NOT_FOUND,                        /* 0x0000F012, The driver can not found                                             */
    RT_ERR_SEM_LOCK_FAILED,                         /* 0x0000F013, Failed to lock semaphore                                             */
    RT_ERR_SEM_UNLOCK_FAILED,                       /* 0x0000F014, Failed to unlock semaphore                                           */
    RT_ERR_THREAD_EXIST,                            /* 0x0000F015, Thread exist                                                         */
    RT_ERR_THREAD_CREATE_FAILED,                    /* 0x0000F016, Thread create fail                                                   */
    RT_ERR_FWD_ACTION,                              /* 0x0000F017, Invalid forwarding Action                                            */
    RT_ERR_IPV4_ADDRESS,                            /* 0x0000F018, Invalid IPv4 address                                                 */
    RT_ERR_IPV6_ADDRESS,                            /* 0x0000F019, Invalid IPv6 address                                                 */
    RT_ERR_PRIORITY,                                /* 0x0000F01a, Invalid Priority value                                               */
    RT_ERR_FID,                                     /* 0x0000F01b, invalid fid                                                          */
    RT_ERR_ENTRY_NOTFOUND,                          /* 0x0000F01c, specified entry not found                                            */
    RT_ERR_DROP_PRECEDENCE,                         /* 0x0000F01d, invalid drop precedence                                              */
    RT_ERR_NOT_FINISH,                              /* 0x0000F01e, Action not finish, still need to wait                                */
    RT_ERR_TIMEOUT,                                 /* 0x0000F01f, Time out                                                             */
    RT_ERR_REG_ARRAY_INDEX_1,                       /* 0x0000F020, invalid index 1 of register array                                    */
    RT_ERR_REG_ARRAY_INDEX_2,                       /* 0x0000F021, invalid index 2 of register array                                    */
    RT_ERR_ETHER_TYPE,                              /* 0x0000F022, invalid ether type                                                   */
    RT_ERR_MBUF_PKT_NOT_AVAILABLE,                  /* 0x0000F023, mbuf->packet is not available                                        */
    RT_ERR_QOS_INVLD_RSN,                           /* 0x0000F024, invalid pkt to CPU reason                                            */
    RT_ERR_CB_FUNCTION_EXIST,                       /* 0x0000F025, Callback function exist                                              */
    RT_ERR_CB_FUNCTION_FULL,                        /* 0x0000F026, Callback function number is full                                     */
    RT_ERR_CB_FUNCTION_NOT_FOUND,                   /* 0x0000F027, Callback function can not found                                      */
    RT_ERR_TBL_FULL,                                /* 0x0000F028, The table is full                                                    */
    RT_ERR_TRUNK_ID,                                /* 0x0000F029, invalid trunk id                                                     */
    RT_ERR_TYPE,                                    /* 0x0000F02a, invalid type                                                         */
    RT_ERR_ENTRY_EXIST,                             /* 0x0000F02b, entry exists                                                         */
    RT_ERR_CHIP_UNDEFINED_VALUE,                    /* 0x0000F02c, chip returned an undefined value                                     */
    RT_ERR_EXCEEDS_CAPACITY,                        /* 0x0000F02d, exceeds the capacity of hardware                                     */
    RT_ERR_ENTRY_REFERRED,                          /* 0x0000F02e, entry is still being referred                                        */
    RT_ERR_OPER_DENIED,                             /* 0x0000F02f, operation denied                                                     */
    RT_ERR_PORT_NOT_SUPPORTED,                      /* 0x0000F030, functions not supported by this port                                 */
    RT_ERR_SOCKET,                                  /* 0x0000F031, socket error                                                         */
    RT_ERR_MEM_ALLOC,                               /* 0x0000F032, insufficient memory resource                                         */
    RT_ERR_ABORT,                                   /* 0x0000F033, operation aborted                                                    */
    RT_ERR_DEV_ID,                                  /* 0x0000F034, invalid device id                                                    */
    RT_ERR_DRIVER_NOT_SUPPORTED,                    /* 0x0000F035, functions not supported by this driver                               */
    RT_ERR_NOT_SUPPORTED,                           /* 0x0000F036, functions not supported                                              */
    RT_ERR_SER,                                     /* 0x0000F037, ECC or parity error                                                  */
    RT_ERR_MEM_NOT_ALIGN,                           /* 0x0000F038, memory address is not aligned                                        */
    RT_ERR_SEM_FAKELOCK_OK,                         /* 0x0000F039, attach thread lock a semaphore which was already locked              */
    RT_ERR_CHECK_FAILED,                            /* 0x0000F03a, check result is failed                                               */

    RT_ERR_COMMON_END = 0xFFFF                      /* The symbol is the latest symbol of common error                                  */
} rt_error_common_t;

typedef uint32  rtk_port_t; 
extern int32 cmd_linkMedia_get();
extern int32 cmd_speedDuplex_get();
extern int32 cmd_adminEnable_set();
extern int32 cmd_phyAutoNegoEnable_get();
extern int32 cmd_phyAutoNegoEnable_set();
extern int32 cmd_phyAutoNegoAbilityLocal_get();
extern int32 cmd_phyAutoNegoAbility_get();
extern int32 cmd_phyAutoNegoAbility_set();
extern int32 cmd_phyForceModeAbility_get();
extern int32 cmd_phyForceModeAbility_set();
extern int32 cmd_phyForceFlowctrlMode_get();
extern int32 cmd_phyForceFlowctrlMode_set();
extern int32 cmd_phyMasterSlave_get();
extern int32 cmd_phyMasterSlave_set();
extern int32 cmd_phyReg_get();
extern int32 cmd_phyReg_set();
extern int32 cmd_phyExtParkPageReg_get();
extern int32 cmd_phyExtParkPageReg_set();
extern int32 cmd_phymaskExtParkPageReg_set();
extern int32 cmd_phyMmdReg_get();
extern int32 cmd_phyMmdReg_set();
extern int32 cmd_phymaskMmdReg_set();
extern int32 cmd_phyComboPortMedia_get();
extern int32 cmd_phyComboPortMedia_set();
extern int32 cmd_greenEnable_get();
extern int32 cmd_greenEnable_set();
extern int32 cmd_phyCrossOverMode_get();
extern int32 cmd_phyCrossOverMode_set();
extern int32 cmd_phyCrossOverStatus_get();
extern int32 cmd_linkDownPowerSavingEnable_get();
extern int32 cmd_linkDownPowerSavingEnable_set();
extern int32 cmd_gigaLiteEnable_get();
extern int32 cmd_gigaLiteEnable_set();
extern int32 cmd_phyReconfig_register();
extern int32 cmd_phyReconfig_unregister();
extern int32 cmd_downSpeedEnable_get();
extern int32 cmd_downSpeedEnable_set();
extern int32 cmd_downSpeedStatus_get();
extern int32 cmd_phyLoopBackEnable_get();
extern int32 cmd_phyLoopBackEnable_set();
extern int32 cmd_phyPolar_get();
extern int32 cmd_phyPolar_set();
extern int32 cmd_phySdsRxCaliStatus_get();
extern int32 cmd_phyReset_set();
extern int32 cmd_phyLinkStatus_get();
extern int32 cmd_phyPeerAutoNegoAbility_get();
extern int32 cmd_phyMacIntfSerdesMode_get();
extern int32 cmd_phyLedMode_set();
extern int32 cmd_phyLedCtrl_get();
extern int32 cmd_phyLedCtrl_set();
extern int32 cmd_phyMacIntfSerdesLinkStatus_get();
extern int32 cmd_phySdsEyeParam_get();
extern int32 cmd_phySdsEyeParam_set();
extern int32 cmd_phyMdiLoopbackEnable_get();
extern int32 cmd_phyMdiLoopbackEnable_set();
extern int32 cmd_phyIntr_init();
extern int32 cmd_phyIntrEnable_get();
extern int32 cmd_phyIntrEnable_set();
extern int32 cmd_phyIntrStatus_get();
extern int32 cmd_phyIntrMask_get();
extern int32 cmd_phyIntrMask_set();
extern int32 cmd_phySdsTestMode_set();
extern int32 cmd_phySdsTestModeCnt_get();
extern int32 cmd_phySdsLeq_get();
extern int32 cmd_phySdsLeq_set();
extern int32 cmd_phySds_get();
extern int32 cmd_phySds_set();
extern int32 cmd_phyCtrl_get();
extern int32 cmd_phyCtrl_set();
extern int32 cmd_phyDbgCounter_get();
extern int32 init_phy_api(void);

struct  cmd_list_t{
	char * name;
	int32_t (*func)();
} cmd_list[] = {
	{"init",init_phy_api},
	{"cmd_linkMedia_get",cmd_linkMedia_get},

	{"cmd_speedDuplex_get",cmd_speedDuplex_get},

	{"cmd_adminEnable_set",cmd_adminEnable_set},

	{"cmd_phyAutoNegoEnable_get",cmd_phyAutoNegoEnable_get},

	{"cmd_phyAutoNegoEnable_set",cmd_phyAutoNegoEnable_set},

	{"cmd_phyAutoNegoAbilityLocal_get",cmd_phyAutoNegoAbilityLocal_get},

	{"cmd_phyAutoNegoAbility_get",cmd_phyAutoNegoAbility_get},

	{"cmd_phyAutoNegoAbility_set",cmd_phyAutoNegoAbility_set},

	{"cmd_phyForceModeAbility_get",cmd_phyForceModeAbility_get},

	{"cmd_phyForceModeAbility_set",cmd_phyForceModeAbility_set},

	{"cmd_phyForceFlowctrlMode_get",cmd_phyForceFlowctrlMode_get},

	{"cmd_phyForceFlowctrlMode_set",cmd_phyForceFlowctrlMode_set},

	{"cmd_phyMasterSlave_get",cmd_phyMasterSlave_get},

	{"cmd_phyMasterSlave_set",cmd_phyMasterSlave_set},

	{"cmd_phyReg_get",cmd_phyReg_get},

	{"cmd_phyReg_set",cmd_phyReg_set},

	{"cmd_phyExtParkPageReg_get",cmd_phyExtParkPageReg_get},

	{"cmd_phyExtParkPageReg_set",cmd_phyExtParkPageReg_set},

	{"cmd_phymaskExtParkPageReg_set",cmd_phymaskExtParkPageReg_set},

	{"cmd_phyMmdReg_get",cmd_phyMmdReg_get},

	{"cmd_phyMmdReg_set",cmd_phyMmdReg_set},

	{"cmd_phymaskMmdReg_set",cmd_phymaskMmdReg_set},

	{"cmd_phyComboPortMedia_get",cmd_phyComboPortMedia_get},

	{"cmd_phyComboPortMedia_set",cmd_phyComboPortMedia_set},

	{"cmd_greenEnable_get",cmd_greenEnable_get},

	{"cmd_greenEnable_set",cmd_greenEnable_set},

	{"cmd_phyCrossOverMode_get",cmd_phyCrossOverMode_get},

	{"cmd_phyCrossOverMode_set",cmd_phyCrossOverMode_set},

	{"cmd_phyCrossOverStatus_get",cmd_phyCrossOverStatus_get},

	{"cmd_linkDownPowerSavingEnable_get",cmd_linkDownPowerSavingEnable_get},

	{"cmd_linkDownPowerSavingEnable_set",cmd_linkDownPowerSavingEnable_set},

	{"cmd_gigaLiteEnable_get",cmd_gigaLiteEnable_get},

	{"cmd_gigaLiteEnable_set",cmd_gigaLiteEnable_set},

	{"cmd_phyReconfig_register",cmd_phyReconfig_register},

	{"cmd_phyReconfig_unregister",cmd_phyReconfig_unregister},

	{"cmd_downSpeedEnable_get",cmd_downSpeedEnable_get},

	{"cmd_downSpeedEnable_set",cmd_downSpeedEnable_set},

	{"cmd_downSpeedStatus_get",cmd_downSpeedStatus_get},

	{"cmd_phyLoopBackEnable_get",cmd_phyLoopBackEnable_get},

	{"cmd_phyLoopBackEnable_set",cmd_phyLoopBackEnable_set},

	{"cmd_phyPolar_get",cmd_phyPolar_get},

	{"cmd_phyPolar_set",cmd_phyPolar_set},

	{"cmd_phySdsRxCaliStatus_get",cmd_phySdsRxCaliStatus_get},

	{"cmd_phyReset_set",cmd_phyReset_set},

	{"cmd_phyLinkStatus_get",cmd_phyLinkStatus_get},

	{"cmd_phyPeerAutoNegoAbility_get",cmd_phyPeerAutoNegoAbility_get},

	{"cmd_phyMacIntfSerdesMode_get",cmd_phyMacIntfSerdesMode_get},

	{"cmd_phyLedMode_set",cmd_phyLedMode_set},

	{"cmd_phyLedCtrl_get",cmd_phyLedCtrl_get},

	{"cmd_phyLedCtrl_set",cmd_phyLedCtrl_set},

	{"cmd_phyMacIntfSerdesLinkStatus_get",cmd_phyMacIntfSerdesLinkStatus_get},

	{"cmd_phySdsEyeParam_get",cmd_phySdsEyeParam_get},

	{"cmd_phySdsEyeParam_set",cmd_phySdsEyeParam_set},

	{"cmd_phyMdiLoopbackEnable_get",cmd_phyMdiLoopbackEnable_get},

	{"cmd_phyMdiLoopbackEnable_set",cmd_phyMdiLoopbackEnable_set},

	{"cmd_phyIntr_init",cmd_phyIntr_init},

	{"cmd_phyIntrEnable_get",cmd_phyIntrEnable_get},

	{"cmd_phyIntrEnable_set",cmd_phyIntrEnable_set},

	{"cmd_phyIntrStatus_get",cmd_phyIntrStatus_get},

	{"cmd_phyIntrMask_get",cmd_phyIntrMask_get},

	{"cmd_phyIntrMask_set",cmd_phyIntrMask_set},

	{"cmd_phySdsTestMode_set",cmd_phySdsTestMode_set},

	{"cmd_phySdsTestModeCnt_get",cmd_phySdsTestModeCnt_get},

	{"cmd_phySdsLeq_get",cmd_phySdsLeq_get},

	{"cmd_phySdsLeq_set",cmd_phySdsLeq_set},

	{"cmd_phySds_get",cmd_phySds_get},

	{"cmd_phySds_set",cmd_phySds_set},

	{"cmd_phyCtrl_get",cmd_phyCtrl_get},

	{"cmd_phyCtrl_set",cmd_phyCtrl_set},

	{"cmd_phyDbgCounter_get",cmd_phyDbgCounter_get},
};

static int do_switch(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	char * name;
	int i;

	if (argc <= 1) {
		return 0;
	}

	name = argv[1];
	for (i = 0; i < sizeof(cmd_list)/sizeof(cmd_list[0]); i++) {
		if (strcasecmp(cmd_list[i].name, name) == 0)
		{
			printf("EXE FUN[%s] ret=%d\n", name, cmd_list[i].func());
		}
	}
	return 0;
}
U_BOOT_CMD(
		rtl8214fc,	3,	1,	do_switch,
		"clourney rtl8214fc utility commands",
		""
		);
