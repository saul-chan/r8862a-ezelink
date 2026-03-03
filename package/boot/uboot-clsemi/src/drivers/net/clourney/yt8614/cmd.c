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
static int g_init_yt8614 = 0;

extern int yt_phy_register(void);

static int do_yt8614(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	if (!g_init_yt8614) {
		yt_phy_register();
		g_init_yt8614 = 1;
		printf("yt phy inited!\n");
	} 
	return 0;
}

U_BOOT_CMD(
		yt8614,	3,	1,	do_yt8614,
		"clourney yt8614 utility commands",
		""
		);
