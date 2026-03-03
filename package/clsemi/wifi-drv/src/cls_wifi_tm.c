#include <linux/time_namespace.h>
#include "cls_wifi_defs.h"

#ifdef CONFIG_DEBUG_FS

#define NEXT_IDX(idx)	((idx + 1) & (WBM_STATS_NUM - 1))
#define PREV_IDX(idx)	((idx - 1) & (WBM_STATS_NUM - 1))
#define TS_TO_IDX(ts)	(ts & (WBM_STATS_NUM - 1))

enum wtm_obj {
	WBM_OBJ_PHY,
	WBM_OBJ_VIF,
	WBM_OBJ_STA,
};

struct wtm_iter {
	unsigned long jiffies_ts;
	unsigned long uptime;
	struct wtm_phy *cur_phy;
	enum wtm_obj cur_obj;
	union {
		int vif_idx;
		int sta_idx;
	};
};

struct wtm_conf {
	u8 enable_wtm;
	u8 watch_all_flags;
	u8 watch_format;
	u8 watch_period;
	u8 show_peak;
};

enum {
	WATCH_ALL_PHY = BIT(0),
	WATCH_ALL_VIF = BIT(1),
	WATCH_ALL_STA = BIT(2),
};

enum {
	PSS_FORMAT_BYTES,
	PSS_FORMAT_BITRATE,
};

struct wtm_watch_node {
	struct list_head node;
	union {
		char name[IFNAMSIZ];
		u8 mac_addr[ETH_ALEN];
	};
};

struct wtm_phy {
	struct list_head node;
	struct cls_wifi_hw *cls_wifi_hw;
};

static LIST_HEAD(wtm_phy_list);
static LIST_HEAD(phy_watch_list);
static LIST_HEAD(vif_watch_list);
static LIST_HEAD(sta_watch_list);
static struct dentry *wtm_debugfs;
static struct wtm_conf wtm_conf;

static void wtm_usage(void)
{
	pr_info("traffic monitor is %s\n", wtm_conf.enable_wtm ? "enabled" : "disabled");
	pr_info("available command:\n");
	pr_info("    enable                           : enable traffic monitor\n");
	pr_info("    disable                          : disable traffic monitor\n");
	pr_info("    watch all [phy|vif|sta]          : show stats of all phy, vif and sta\n");
	pr_info("    watch phy <name of phy>          : add specified phy to watch list\n");
	pr_info("    watch vif <name of vif>          : add specified vif to watch list\n");
	pr_info("    watch sta <macaddr of sta>       : add specified sta to watch list\n");
	pr_info("    watch none                       : clean watch list\n");
	pr_info("    watch period <1~7>               : show last n seconds stats\n");
	pr_info("    format <bytes | bitrate>         : show in bytes / bits\n");
	pr_info("    peak <hide | show | reset>       : hide / show / reset peak data\n");
}

void cls_wtm_add_phy(struct cls_wifi_hw *cls_wifi_hw)
{
	struct wtm_phy *entry;

	entry = kzalloc(sizeof(*entry), GFP_KERNEL);
	if (!entry)
		return;

	entry->cls_wifi_hw = cls_wifi_hw;
	list_add_tail(&entry->node, &wtm_phy_list);
}

void cls_wtm_del_phy(struct cls_wifi_hw *cls_wifi_hw)
{
	struct wtm_phy *cur, *next;

	list_for_each_entry_safe(cur, next, &wtm_phy_list, node) {
		list_del(&cur->node);
		kfree(cur);
	}
}

static void wtm_reset_peak_data(struct wtm_stats *stats)
{
	stats->peak_rx_pkts = 0;
	stats->peak_tx_pkts = 0;
	stats->peak_rx_bytes = 0;
	stats->peak_tx_bytes = 0;
	stats->peak_rssi = SHRT_MIN;
}

void cls_wtm_reset_sta_peak_data(struct cls_wifi_sta *sta)
{
	wtm_reset_peak_data(&sta->wtm_stats);
}

static void reset_peak_data(void)
{
	struct wtm_phy *entry;
	struct cls_wifi_hw *phy;
	struct cls_wifi_vif *vif;
	struct cls_wifi_sta *sta;
	int i;

	list_for_each_entry(entry, &wtm_phy_list, node) {
		phy = entry->cls_wifi_hw;
		wtm_reset_peak_data(&phy->wtm_stats);
		for (i = 0; i < hw_vdev_max(phy); ++i) {
			vif = cls_wifi_get_vif(phy, i);
			if (vif)
				wtm_reset_peak_data(&vif->wtm_stats);
		}
		for (i = 0; i < hw_remote_sta_max(phy); ++i) {
			sta = cls_wifi_get_sta(phy, i, NULL, false);
			if (sta)
				wtm_reset_peak_data(&sta->wtm_stats);
		}
	}
}

static void wtm_update_stats(struct wtm_stats *stats, unsigned long ts, u32 bytes, s16 rssi, bool is_rx)
{
	u32 idx = TS_TO_IDX(ts);

	if (!stats)
		return;

	if (stats->cur_idx == idx && stats->stats[idx].ts == ts) {
		if (is_rx) {
			stats->stats[idx].rx_pkts	+= 1;
			stats->stats[idx].rx_bytes	+= bytes;
			stats->stats[idx].rssi_sum	+= rssi;
		} else {
			stats->stats[idx].tx_pkts	+= 1;
			stats->stats[idx].tx_bytes	+= bytes;
		}
	} else {
		if (is_rx) {
			stats->stats[idx].rx_pkts	= 1;
			stats->stats[idx].rx_bytes	= bytes;
			stats->stats[idx].rssi_sum	= rssi;
			stats->stats[idx].tx_pkts	= 0;
			stats->stats[idx].tx_bytes	= 0;
		} else {
			stats->stats[idx].rx_pkts	= 0;
			stats->stats[idx].rx_bytes	= 0;
			stats->stats[idx].rssi_sum	= 0;
			stats->stats[idx].tx_pkts	= 1;
			stats->stats[idx].tx_bytes	= bytes;
		}

		stats->stats[idx].ts = ts;
		stats->cur_idx = idx;
	}

	if (is_rx) {
		if (stats->peak_rx_pkts < stats->stats[idx].rx_pkts)
			stats->peak_rx_pkts = stats->stats[idx].rx_pkts;
		if (stats->peak_rx_bytes < stats->stats[idx].rx_bytes)
			stats->peak_rx_bytes = stats->stats[idx].rx_bytes;
		if (stats->peak_rssi < rssi)
			stats->peak_rssi = rssi;
	} else {
		if (stats->peak_tx_pkts < stats->stats[idx].tx_pkts)
			stats->peak_tx_pkts = stats->stats[idx].tx_pkts;
		if (stats->peak_tx_bytes < stats->stats[idx].tx_bytes)
			stats->peak_tx_bytes = stats->stats[idx].tx_bytes;
	}
}

void cls_wtm_update_stats(struct cls_wifi_hw *phy, struct cls_wifi_vif *vif, struct cls_wifi_sta *sta,
			  u32 bytes, s16 rssi, bool is_rx)
{
	unsigned long ts;

	if (!wtm_conf.enable_wtm)
		return;

	if (!phy || !vif || !sta)
		return;

	ts = jiffies / HZ;
	wtm_update_stats(&phy->wtm_stats, ts, bytes, rssi, is_rx);
	wtm_update_stats(&vif->wtm_stats, ts, bytes, rssi, is_rx);
	wtm_update_stats(&sta->wtm_stats, ts, bytes, rssi, is_rx);
}

static int wtm_watch_list_add(const char *type_str, const char *value)
{
	struct wtm_watch_node *entry;

	entry = kzalloc(sizeof(*entry), GFP_KERNEL);
	if (!entry)
		return -ENOMEM;

	if (!strcmp(type_str, "phy")) {
		strncpy(entry->name, value, IFNAMSIZ);
		list_add(&entry->node, &phy_watch_list);
	} else if (!strcmp(type_str, "vif")) {
		strncpy(entry->name, value, IFNAMSIZ);
		list_add(&entry->node, &vif_watch_list);
	} else if (!strcmp(type_str, "sta")) {
		if (!mac_pton(value, entry->mac_addr)) {
			kfree(entry);
			return -EINVAL;
		}
		list_add(&entry->node, &sta_watch_list);
	}

	return 0;
}

static void wtm_watch_list_clean(struct list_head *list)
{
	struct wtm_watch_node *cur, *next;

	list_for_each_entry_safe(cur, next, list, node)
		kfree(cur);
	INIT_LIST_HEAD(list);
}

static bool wtm_watch_list_find_phy(const char *name)
{
	struct wtm_watch_node *entry;

	if (wtm_conf.watch_all_flags & WATCH_ALL_PHY)
		return true;

	list_for_each_entry(entry, &phy_watch_list, node) {
		if (!strcmp(entry->name, name))
			return true;
	}

	return false;
}

static bool wtm_watch_list_find_vif(const char *name)
{
	struct wtm_watch_node *entry;

	if (wtm_conf.watch_all_flags & WATCH_ALL_VIF)
		return true;

	list_for_each_entry(entry, &vif_watch_list, node) {
		if (!strcmp(entry->name, name))
			return true;
	}

	return false;
}

static bool wtm_watch_list_find_sta(const u8 *mac_addr)
{
	struct wtm_watch_node *entry;

	if (wtm_conf.watch_all_flags & WATCH_ALL_STA)
		return true;

	list_for_each_entry(entry, &sta_watch_list, node) {
		if (ether_addr_equal_64bits(entry->mac_addr, mac_addr))
			return true;
	}

	return false;
}

static ssize_t wtm_write(struct file *filp, const char __user *userbuf, size_t count, loff_t *ppos)
{
	char *buf;
	char **argv;
	int ret, argc, i;
	u8 watch_period;

	buf = kzalloc(count + 1, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	ret = simple_write_to_buffer(buf, count, ppos, userbuf, count);
	if (ret < 0) {
		kfree(buf);
		return ret;
	}

	argv = argv_split(GFP_KERNEL, buf, &argc);
	if (!argv) {
		kfree(buf);
		return -ENOMEM;
	}
	kfree(buf);

	if (!strcmp(argv[0], "enable")) {
		wtm_conf.enable_wtm = 1;
	} else if (!strcmp(argv[0], "disable")) {
		wtm_conf.enable_wtm = 0;
	} else if (!strcmp(argv[0], "watch")) {
		if (argc < 2) {
			ret = -EINVAL;
			goto out;
		}
		if (!strcmp(argv[1], "all")) {
			if (argc == 2) {
				wtm_conf.watch_all_flags = WATCH_ALL_PHY | WATCH_ALL_VIF | WATCH_ALL_STA;
			} else if (argc == 3) {
				if (!strcmp(argv[2], "phy"))
					wtm_conf.watch_all_flags |= WATCH_ALL_PHY;
				else if (!strcmp(argv[2], "vif"))
					wtm_conf.watch_all_flags |= WATCH_ALL_VIF;
				else if (!strcmp(argv[2], "sta"))
					wtm_conf.watch_all_flags |= WATCH_ALL_STA;
				else
					ret = -EINVAL;
			} else {
				ret = -EINVAL;
			}
		} else if (!strcmp(argv[1], "none")) {
			wtm_conf.watch_all_flags = 0;
			wtm_watch_list_clean(&phy_watch_list);
			wtm_watch_list_clean(&vif_watch_list);
			wtm_watch_list_clean(&sta_watch_list);
		} else if (!strcmp(argv[1], "phy") || !strcmp(argv[1], "vif") || !strcmp(argv[1], "sta")) {
			if (argc < 3) {
				ret = -EINVAL;
				goto out;
			}
			for (i = 2; i < argc; ++i) {
				ret = wtm_watch_list_add(argv[1], argv[i]);
				if (ret)
					goto out;
			}
		} else if (!strcmp(argv[1], "period")) {
			if (!kstrtou8(argv[1], 10, &watch_period)) {
				ret = -EINVAL;
				goto out;
			}
			if (watch_period > 0 && watch_period < WBM_STATS_NUM)
				wtm_conf.watch_period = watch_period;
			else
				ret = -EINVAL;
		} else {
			ret = -EINVAL;
		}
	} else if (!strcmp(argv[0], "format")) {
		if (argc != 2) {
			ret = -EINVAL;
			goto out;
		}
		if (!strcmp(argv[1], "bytes"))
			wtm_conf.watch_format = PSS_FORMAT_BYTES;
		else if (!strcmp(argv[1], "bitrate"))
			wtm_conf.watch_format = PSS_FORMAT_BITRATE;
		else
			ret = -EINVAL;
	} else if (!strcmp(argv[0], "peak")) {
		if (argc != 2) {
			ret = -EINVAL;
			goto out;
		}
		if (!strcmp(argv[1], "hide"))
			wtm_conf.show_peak = 0;
		else if (!strcmp(argv[1], "show"))
			wtm_conf.show_peak = 1;
		else if (!strcmp(argv[1], "reset"))
			reset_peak_data();
		else
			ret = -EINVAL;
	} else
		ret = -EINVAL;

out:
	if (ret == -EINVAL)
		wtm_usage();

	argv_free(argv);
	return count;
}

static struct wtm_phy *wtm_get_next_phy(struct wtm_phy *start)
{
	struct wtm_phy *phy;

	phy = list_prepare_entry(start, &wtm_phy_list, node);
	list_for_each_entry_continue(phy, &wtm_phy_list, node)
		return phy;
	return NULL;
}

static struct wtm_iter *wtm_get_next_obj(struct wtm_iter *iter)
{
	if (iter->cur_obj == WBM_OBJ_PHY) {
		iter->cur_phy = wtm_get_next_phy(iter->cur_phy);
		if (iter->cur_phy)
			return iter;
		/* all phy have been iterated, change to iterate vif */
		iter->cur_obj = WBM_OBJ_VIF;
		iter->cur_phy = wtm_get_next_phy(NULL);
		iter->vif_idx = -1;
	}

	if (iter->cur_obj == WBM_OBJ_VIF) {
		if (iter->cur_phy) {
			iter->vif_idx++;
			if (iter->vif_idx < hw_vdev_max(iter->cur_phy->cls_wifi_hw))
				return iter;
			iter->cur_phy = wtm_get_next_phy(iter->cur_phy);
			iter->vif_idx = -1;
			return wtm_get_next_obj(iter);
		}
		/* all vif have been iterated, change to iterate sta */
		iter->cur_obj = WBM_OBJ_STA;
		iter->cur_phy = wtm_get_next_phy(NULL);
		iter->sta_idx = -1;
	}

	if (iter->cur_obj == WBM_OBJ_STA) {
		if (iter->cur_phy) {
			iter->sta_idx++;
			if (iter->sta_idx < hw_remote_sta_max(iter->cur_phy->cls_wifi_hw))
				return iter;
			iter->cur_phy = wtm_get_next_phy(iter->cur_phy);
			iter->sta_idx = -1;
			return wtm_get_next_obj(iter);
		}
	}
	return NULL;
}

static void *wtm_seq_start(struct seq_file *seq, loff_t *ppos)
{
	struct wtm_iter *iter = seq->private;
	loff_t pos = *ppos;

	if (!pos) {
		iter->cur_obj = WBM_OBJ_PHY;
		iter->cur_phy = NULL;
		iter = wtm_get_next_obj(iter);
	} else {
		if (!iter->cur_phy)
			return NULL;
	}

	return iter;
}

static void *wtm_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
	++*pos;
	return wtm_get_next_obj(v);
}

static void wtm_seq_stop(struct seq_file *seq, void *v)
{
}

static char *format_bitrate(unsigned long bytes, char *buf)
{
	unsigned long bits = bytes << 3;

	if (bits >= 1000000000)
		sprintf(buf, "%lu.%02luG", bits / 1000000000, bits % 1000000000 / 10000000);
	else if (bits >= 1000000)
		sprintf(buf, "%lu.%02luM", bits / 1000000, bits % 1000000 / 10000);
	else if (bits >= 1000)
		sprintf(buf, "%lu.%02luK", bits / 1000, bits % 1000 / 10);
	else
		sprintf(buf, "%lu", bits);

	return buf;
}

static int wtm_seq_show(struct seq_file *seq, void *v)
{
	struct wtm_iter *iter = v;
	struct cls_wifi_hw *phy = iter->cur_phy->cls_wifi_hw;
	struct cls_wifi_vif *vif;
	struct cls_wifi_sta *sta;
	struct wtm_stats *stats;
	unsigned long uptime = iter->uptime;
	unsigned long ts, now = iter->jiffies_ts;
	u32 idx, cur_idx = TS_TO_IDX(now);
	unsigned int watch_period = wtm_conf.watch_period;
	char tx_bitrate[16], rx_bitrate[16];
	s32 avg_rssi;

	if (iter->cur_obj == WBM_OBJ_PHY) {
		if (!wtm_watch_list_find_phy(wiphy_name(phy->wiphy)))
			return 0;
		stats = &phy->wtm_stats;
		seq_printf(seq, "%s:\n", wiphy_name(phy->wiphy));
	} else if (iter->cur_obj == WBM_OBJ_VIF) {
		vif = cls_wifi_get_vif(phy, iter->vif_idx);
		if (!vif)
			return 0;
		if (!wtm_watch_list_find_vif(vif->ndev->name))
			return 0;
		stats = &vif->wtm_stats;
		seq_printf(seq, "%s on %s:\n", vif->ndev->name, wiphy_name(phy->wiphy));
	} else if (iter->cur_obj == WBM_OBJ_STA) {
		sta = cls_wifi_get_sta(phy, iter->sta_idx, NULL, false);
		if (!sta)
			return 0;
		if (!wtm_watch_list_find_sta(sta->mac_addr))
			return 0;
		vif = cls_wifi_get_vif(phy, sta->vif_idx);
		if (!vif)
			return 0;
		stats = &sta->wtm_stats;
		seq_printf(seq, "%pM associated to %s\n", sta->mac_addr, vif->ndev->name);
	}

	seq_printf(seq, "%-10s %10s %10s %10s %10s",
		   "uptime", "rx_pkts", "tx_pkts",
		   wtm_conf.watch_format == PSS_FORMAT_BYTES ? "rx_bytes" : "rx_bps",
		   wtm_conf.watch_format == PSS_FORMAT_BYTES ? "tx_bytes" : "tx_bps");
	if (iter->cur_obj == WBM_OBJ_STA)
		seq_printf(seq, " %10s\n", "avg_rssi");
	else
		seq_puts(seq, "\n");

	ts = now - 1;
	uptime = uptime - 1;
	idx = PREV_IDX(cur_idx);
	for (; watch_period && idx != cur_idx; watch_period--, ts--, uptime--, idx = PREV_IDX(idx)) {
		if (stats->stats[idx].ts == ts) {
			if (wtm_conf.watch_format == PSS_FORMAT_BYTES)
				seq_printf(seq, "%-10lu %10u %10u %10u %10u",
					   uptime,
					   stats->stats[idx].rx_pkts, stats->stats[idx].tx_pkts,
					   stats->stats[idx].rx_bytes, stats->stats[idx].tx_bytes);
			else
				seq_printf(seq, "%-10lu %10u %10u %10s %10s",
					   uptime,
					   stats->stats[idx].rx_pkts, stats->stats[idx].tx_pkts,
					   format_bitrate(stats->stats[idx].rx_bytes, rx_bitrate),
					   format_bitrate(stats->stats[idx].tx_bytes, tx_bitrate));
		} else {
			seq_printf(seq, "%-10lu %10u %10u %10u %10u",
				   uptime, 0, 0, 0, 0);
		}

		if (iter->cur_obj == WBM_OBJ_STA) {
			if (stats->stats[idx].ts == ts && stats->stats[idx].rx_pkts != 0) {
				avg_rssi = stats->stats[idx].rssi_sum / (s32)stats->stats[idx].rx_pkts;
				seq_printf(seq, " %10d\n", avg_rssi);
			} else
				seq_printf(seq, " %10s\n", "N/A");
		} else
			seq_puts(seq, "\n");
	}

	if (wtm_conf.show_peak) {
		if (wtm_conf.watch_format == PSS_FORMAT_BYTES)
			seq_printf(seq, "%-10s %10u %10u %10u %10u",
				   "peak",
				   stats->peak_rx_pkts, stats->peak_tx_pkts,
				   stats->peak_rx_bytes, stats->peak_tx_bytes);
		else
			seq_printf(seq, "%-10s %10u %10u %10s %10s",
				   "peak",
				   stats->peak_rx_pkts, stats->peak_tx_pkts,
				   format_bitrate(stats->peak_rx_bytes, rx_bitrate),
				   format_bitrate(stats->peak_tx_bytes, tx_bitrate));
		if (iter->cur_obj == WBM_OBJ_STA) {
			if (stats->peak_rssi != SHRT_MIN)
				seq_printf(seq, " %10d\n", stats->peak_rssi);
			else
				seq_printf(seq, " %10s\n", "N/A");
		} else
			seq_puts(seq, "\n");
	}

	return 0;
}

static const struct seq_operations wtm_seq_ops = {
	.start	= wtm_seq_start,
	.next	= wtm_seq_next,
	.stop	= wtm_seq_stop,
	.show	= wtm_seq_show,
};

static int wtm_open(struct inode *inode, struct file *file)
{
	struct wtm_iter *iter;
	struct seq_file *seq;
	struct timespec64 tp;
	int rc = -ENOMEM;

	iter = kzalloc(sizeof(*iter), GFP_KERNEL);
	if (!iter)
		return -ENOMEM;

	rc = seq_open(file, &wtm_seq_ops);
	if (rc) {
		kfree(iter);
		return rc;
	}

	seq = file->private_data;
	seq->private = iter;

	iter->jiffies_ts = jiffies / HZ;
	ktime_get_boottime_ts64(&tp);
	timens_add_boottime(&tp);
	iter->uptime = tp.tv_sec + (tp.tv_nsec ? 1 : 0);

	return rc;
}

static int wtm_release(struct inode *inode, struct file *file)
{
	struct seq_file *seq = file->private_data;

	kfree(seq->private);
	seq_release(inode, file);

	return 0;
}

static const struct file_operations wtm_file_ops = {
	.owner		= THIS_MODULE,
	.open		= wtm_open,
	.read		= seq_read,
	.write		= wtm_write,
	.llseek		= seq_lseek,
	.release	= wtm_release,
};

void cls_wtm_init_debugfs(void)
{
	wtm_debugfs = debugfs_create_file("wifi_traffic_monitor", 0600, NULL, NULL, &wtm_file_ops);
	wtm_conf.enable_wtm = 1;
	wtm_conf.watch_all_flags = WATCH_ALL_PHY | WATCH_ALL_VIF | WATCH_ALL_STA;
	wtm_conf.watch_period = WBM_STATS_NUM - 1;
	wtm_conf.watch_format = PSS_FORMAT_BITRATE;
	wtm_conf.show_peak = 1;
}

void cls_wtm_deinit_debugfs(void)
{
	debugfs_remove(wtm_debugfs);
}
#endif
