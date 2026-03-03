#ifndef _SWITCH_CMD_H_
#define _SWITCH_CMD_H_

//#define LOCAL_TEST

#ifdef LOCAL_TEST
#define NPE_DEBUGFS_PATH	"./command"
#else
#define NPE_DEBUGFS_PATH	"/sys/kernel/debug/dubhe1000/command"
#endif

void npe_switch_usage(void);
int npe_switch_config(int argc, char **argv);

#endif /* _SWITCH_CMD_H_ */
