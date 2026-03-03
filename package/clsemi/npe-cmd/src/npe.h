/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2023 Clourneysemi Corporation. */

#ifndef _NPE_H_
#define _NPE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/types.h>

#define u8 u_int8_t
#define u16 u_int16_t
#define u32 u_int32_t
#define u64 u_int64_t
#define bool u_int8_t

#define NPE_MODULE_EDMA		"edma"
#define NPE_MODULE_EDMA_DBG	"edma_dbg"
#define NPE_MODULE_SWITCH	"switch"
#define NPE_MODULE_MAC		"mac"
#define NPE_MODULE_PHY		"phy"

#define ARRAY_SIZE(a) (sizeof((a))/sizeof((a)[0]))

#define NPECMD_DBG_PATH		"/tmp/npecmd_dbg"

#define npecmd_printf(...) \
    do { \
        if (access(NPECMD_DBG_PATH, F_OK) == 0) { \
            printf(__VA_ARGS__); \
        } \
    } while(0)

#define npecmd_err(args...)	printf(args)
#define npecmd_dbg(...)		npecmd_printf(__VA_ARGS__)

struct cmd_module {
	char name[64];
	int (*func)(int argc, char **argv);
	void (*usage)(void);
};


//char edma_dbg_cmd_list[][16] = {"tx_bmu", "tx_route", "dump", "stats"};
//char mac_cmd_list[][16] = {"tx_config", "rx_config", "packet_filter", "tx_flow_ctrl", "rx_flow_ctrl", "rx_queue_ctrl", "stats"};

int npe_module_config(int argc, char **argv, struct cmd_module *module, int size, void (*usage)(void));

int npe_edma_config(int argc, char **argv);
int npe_switch_config(int argc, char **argv);

void npe_edma_usage(void);
void npe_switch_usage(void);

int npe_read(char *module, u64 address, u32 *value);
int npe_write(char *module, u64 address, u32 value);

#define edma_read(address, pointer) npe_read(NPE_MODULE_EDMA, address, pointer)
#define edma_write(address, value) npe_write(NPE_MODULE_EDMA, address, value)

#define edma_dbg_read(address, pointer) npe_read(NPE_MODULE_EDMA_DBG, address, pointer)
#define edma_dbg_write(address, value) npe_write(NPE_MODULE_EDMA_DBG, address, value)

#define switch_read(address, pointer) npe_read(NPE_MODULE_SWITCH, address, pointer)
#define switch_write(address, value) npe_write(NPE_MODULE_SWITCH, address, value)

#define mac_read(address, pointer) npe_read(NPE_MODULE_MAC, address, pointer)
#define mac_write(address, value) npe_write(NPE_MODULE_MAC, address, value)

#define phy_read(address, pointer) npe_read(NPE_MODULE_PHY, address, pointer)
#define phy_write(address, value) npe_write(NPE_MODULE_PHY, address, value)

#endif /* _NPE_H_ */

