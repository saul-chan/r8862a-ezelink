#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#define TXPOWER_DB_BASE_FILENAME	"/usr/psbin/power_tables/"

#define TXPOWER_DB_DEFAULT_PROJECT	"default"

#define TXPOWER_ENTRY_PRE_FORMAT	"%hhd:%hhd:%hhd"

#define TXPOWER_DB_CONFIG_DEBUGFS_2G	"/sys/kernel/debug/ieee80211/phy0/cls_wifi/power_table"
#define TXPOWER_DB_CONFIG_DEBUGFS_5G	"/sys/kernel/debug/ieee80211/phy1/cls_wifi/power_table"

#define TXPOWER_DB_CHIP_ID_DEBUGFS_2G	"/sys/kernel/debug/ieee80211/phy0/cls_wifi/hw_plat"
#define TXPOWER_DB_CHIP_ID_DEBUGFS_5G	"/sys/kernel/debug/ieee80211/phy1/cls_wifi/hw_plat"

/* chan phymode bw mcs0 ------ mcs11 */
#define TXPOWER_DEBUGFS_FORMAT	"%d %d %d"

#define MAX_11N_MCS_NUM		8
#define MAX_11AC_MCS_NUM	10
#define MAX_11AX_MCS_NUM	12
#define MAX_MCS_NUM		MAX_11AX_MCS_NUM


enum cls_wifi_hw_rev {
	CLS_WIFI_HW_DUBHE2000,
	CLS_WIFI_HW_MERAK2000,
	CLS_WIFI_HW_MAX_INVALID,
};

typedef enum {
	RADIO_MIN = 0,
	RADIO_2G = RADIO_MIN,// 2.4GHz
	RADIO_5G,// 5GHz
	RADIO_MAX = RADIO_5G,
} RadioType;

typedef enum {
	PHYMODE_MIN = 0,
	PHYMODE_LEGACY = PHYMODE_MIN,
	PHYMODE_11N,
	PHYMODE_11AC,
	PHYMODE_11AX,
	PHYMODE_MAX = PHYMODE_11AX,
} PhymodType;

typedef enum {
	BW_MIN = 0,
	BW_20M = BW_MIN,
	BW_40M,
	BW_2G_MAX = BW_40M,
	BW_80M,
	BW_160M,
	BW_5G_MAX = BW_160M,
} BWType;

struct txpower_db_entry {
	uint8_t radio;
	uint8_t chan;
	uint8_t phymode;
	uint8_t bw;
	int8_t power_list[MAX_MCS_NUM];
};
