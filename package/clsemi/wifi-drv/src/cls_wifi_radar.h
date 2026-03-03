/*
 * Copyright (c) 2021-2024, Clourney Semiconductor. All rights reserved.
 *
 * This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
 * Reproduction and redistribution in binary or source form, with or without modification,
 * for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
 * must retain the above copyright notice.
 *
 * By using this software and/or documentation, you agree to the limited terms and conditions.
 */

#ifndef _CLS_WIFI_RADAR_H_
#define _CLS_WIFI_RADAR_H_

#include <linux/nl80211.h>
#include "ipc_shared.h"

struct cls_wifi_vif;
struct cls_wifi_hw;

extern u32 g_radar_rebuild_num;
extern u32 g_radar_rebuild_thresh;
extern bool g_radar_force_cac;
extern bool g_radar_debug;
extern u8 g_radar_ratio;
extern u8 g_radar_rebuild_num_thresh;
extern u8 g_radar_min_num_det;

enum cls_wifi_radar_chain {
	CLS_WIFI_RADAR_RIU = 0,
	CLS_WIFI_RADAR_FCU,
	CLS_WIFI_RADAR_LAST
};

enum cls_wifi_radar_detector {
	CLS_WIFI_RADAR_DETECT_DISABLE = 0, /* Ignore radar pulses */
	CLS_WIFI_RADAR_DETECT_ENABLE  = 1, /* Process pattern detection but do not
									  report radar to upper layer (for test) */
	CLS_WIFI_RADAR_DETECT_REPORT  = 2  /* Process pattern detection and report
									  radar to upper layer. */
};

#ifdef CONFIG_CLS_WIFI_RADAR
#include <linux/workqueue.h>
#include <linux/spinlock.h>

#define CLS_WIFI_RADAR_PULSE_MAX  16
#define CLS_WIFI_RADAR_PULSE_TIMEOUT_WIDTH 100

/* To unit of us */
#define PRI_ROUND(pri) ((((pri) * 128) + 5) / 10)
/* To unit of MHz */
#define FREQ_ROUND(freq) ((((freq) * 15625) + 500000) / 1000000)

/**
 * struct cls_wifi_radar_pulses - List of pulses reported by HW
 * @index: write index
 * @count: number of valid pulses
 * @buffer: buffer of pulses
 */
struct cls_wifi_radar_pulses {
	/* Last radar interrupt received */
	int index;
	int count;
	struct radar_pulse_array_desc buffer[CLS_WIFI_RADAR_PULSE_MAX];
};

/**
 * struct dfs_pattern_detector - DFS pattern detector
 * @region: active DFS region, NL80211_DFS_UNSET until set
 * @num_radar_types: number of different radar types
 * @last_pulse_ts: time stamp of last valid pulse in usecs
 * @prev_jiffies:
 * @radar_detector_specs: array of radar detection specs
 * @channel_detectors: list connecting channel_detector elements
 */
struct dfs_pattern_detector {
	u8 enabled;
	enum nl80211_dfs_regions region;
	u8 num_radar_types;
	u64 last_pulse_ts;
	u32 prev_jiffies;
	const struct radar_detector_specs *radar_spec;
	struct list_head detectors[];
};

#define CLS_NB_RADAR_DETECTED 4

/**
 * struct cls_wifi_radar_detected - List of radar detected
 */
struct cls_wifi_radar_detected {
	u16 index;
	u32 count;
	s64 time[CLS_NB_RADAR_DETECTED];
	s16 freq[CLS_NB_RADAR_DETECTED];
	u32 pulses_cnt;
};

struct cls_wifi_radar {
	struct cls_wifi_radar_pulses pulses;
	struct dfs_pattern_detector *dpd;
	struct cls_wifi_radar_detected detected;
	struct work_struct detection_work; /* Work used to process radar pulses */
	spinlock_t lock; /* lock for pulses processing */
	struct delayed_work cac_work; /* Work used to handle CAC */
	struct cls_wifi_vif *cac_vif; /* vif on which we started CAC */
	struct cfg80211_chan_def skip_chandef; /*only valid for skip dfs_cac(skip_fw = 1)*/
	bool skip_fw;
};

bool cls_wifi_radar_detection_init(struct cls_wifi_radar *radar);
void cls_wifi_radar_detection_deinit(struct cls_wifi_radar *radar);
bool cls_wifi_radar_set_domain(struct cls_wifi_radar *radar,
						   enum nl80211_dfs_regions region, char *alpha2);
void cls_wifi_radar_detection_enable(struct cls_wifi_radar *radar, u8 enable, u8 chain);
bool cls_wifi_radar_detection_is_enable(struct cls_wifi_radar *radar);
void cls_wifi_radar_start_cac(struct cls_wifi_radar *radar, u32 cac_time_ms,
						  struct cls_wifi_vif *vif);
void cls_wifi_radar_cancel_cac(struct cls_wifi_radar *radar);
void cls_wifi_radar_detection_enable_on_cur_channel(struct cls_wifi_hw *cls_wifi_hw,
							struct cls_wifi_vif *vif);
int  cls_wifi_radar_dump_pattern_detector(char *buf, size_t len,
									  struct cls_wifi_radar *radar, u8 chain);
int  cls_wifi_radar_dump_radar_detected(char *buf, size_t len,
									struct cls_wifi_radar *radar, u8 chain);
void dfs_pattern_detector_reset(struct dfs_pattern_detector *dpd);
void cls_wifi_radar_detected(struct cls_wifi_hw *cls_wifi_hw);
#else

struct cls_wifi_radar {
};

static inline bool cls_wifi_radar_detection_init(struct cls_wifi_radar *radar)
{return true;}

static inline void cls_wifi_radar_detection_deinit(struct cls_wifi_radar *radar)
{}

static inline bool cls_wifi_radar_set_domain(struct cls_wifi_radar *radar,
										 enum nl80211_dfs_regions region, char *alpha2)
{return true;}

static inline void cls_wifi_radar_detection_enable(struct cls_wifi_radar *radar,
											   u8 enable, u8 chain)
{}

static inline bool cls_wifi_radar_detection_is_enable(struct cls_wifi_radar *radar)
{return false;}

static inline void cls_wifi_radar_start_cac(struct cls_wifi_radar *radar,
										u32 cac_time_ms, struct cls_wifi_vif *vif)
{}

static inline void cls_wifi_radar_cancel_cac(struct cls_wifi_radar *radar)
{}

static inline void cls_wifi_radar_detection_enable_on_cur_channel(struct cls_wifi_hw *cls_wifi_hw,
										struct cls_wifi_vif *vif)
{}

static inline int cls_wifi_radar_dump_pattern_detector(char *buf, size_t len,
												   struct cls_wifi_radar *radar,
												   u8 chain)
{return 0;}

static inline int cls_wifi_radar_dump_radar_detected(char *buf, size_t len,
												 struct cls_wifi_radar *radar,
												 u8 chain)
{return 0;}

#endif /* CONFIG_CLS_WIFI_RADAR */

#endif // _CLS_WIFI_RADAR_H_
