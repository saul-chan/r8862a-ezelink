#if defined(__ENABLE_FWD_TEST__)
#ifndef __DUBHE1000_FWD_TEST_H__
#define __DUBHE1000_FWD_TEST_H__
#include "dubhe2000.h"

#define WIFI40M_FIFO_DEPTH_BIT	8
#define WIFI160M_FIFO_DEPTH_BIT	16
#define PCIE_FIFO_DEPTH_BIT	24

#define FWD_CMD_STR	    "fwd"
#define FWD_START	    ('b')
#define FWD_STOP	    ('e')
#define FWD_ROUTE	    ('r')
#define FWD_TOKEN	    ('t')
#define DEL_ROUTE	    ('r' | ('d' << 8))
#define ADD_ROUTE	    ('r' | ('a' << 8))
#define QUERY_ROUTE	    ('r' | ('q' << 8))
#define READALL_ROUTE	    ('r' | ('r' << 8))
#define EN_TOKEN	    ('t' | ('e' << 8))
#define INIT_TOKEN	    ('t' | ('i' << 8))
#define PUT_TOKEN	    ('t' | ('p' << 8))
#define FREE_TOKEN	    ('t' | ('f' << 8))
#define GET_TOKEN	    ('t' | ('g' << 8))
#define RECV_DATA	    ('i')
#define FREE_PKT	    ('f')
#define FWD_PRINT	    ('p')
#define UNKNOWN_CMD	    "Unknown cmd"
#define FWD_HELP_PRINT(CMD) dev_info(adapter->dev, "%s: %s\n", get_fwdCmd_name(NULL, CMD), get_fwdCmd_help(NULL, CMD))
#define FWD_USAGE_HELP() do {                                                                                          \
	FWD_HELP_PRINT(FWD_START);                                                                                     \
	FWD_HELP_PRINT(FWD_STOP);                                                                                      \
	FWD_HELP_PRINT(ADD_ROUTE);                                                                                     \
	FWD_HELP_PRINT(DEL_ROUTE);                                                                                     \
	FWD_HELP_PRINT(QUERY_ROUTE);                                                                                   \
	FWD_HELP_PRINT(READALL_ROUTE);                                                                                 \
	FWD_HELP_PRINT(EN_TOKEN);                                                                                      \
	FWD_HELP_PRINT(INIT_TOKEN);                                                                                    \
	FWD_HELP_PRINT(PUT_TOKEN);                                                                                     \
	FWD_HELP_PRINT(GET_TOKEN);                                                                                     \
	FWD_HELP_PRINT(FREE_TOKEN);                                                                                    \
	FWD_HELP_PRINT(FREE_PKT);                                                                                      \
} while (0)

struct fwd_cmd_def_obj {
	uint16_t type;
	char *name;
	char *help;
	int min_argc;
};

char *get_fwdCmd_name(struct fwd_cmd_def_obj *cmd_def, uint16_t type);
char *get_fwdCmd_help(struct fwd_cmd_def_obj *cmd_def, uint16_t type);
void fwd_test_cmd(struct dubhe1000_adapter *adapter, char *cmd_buf);

extern struct dubhe1000_from_cpu_tag g_from_cpu_tag[DUBHE1000_MAC_COUNT];
extern int g_from_cpuTag_bitmap;

#endif /*__DUBHE1000_FWD_TEST_H__*/
#endif
