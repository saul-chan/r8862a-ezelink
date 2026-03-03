#ifndef _NL80211_HELPER_H
#define _NL80211_HELPER_H


#include <linux/nl80211.h>
#include <linux/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/msg.h>
#include <netlink/attr.h>

void dumpNlMsg(struct nl_msg *msg, const char *title);
char *getNlCmdName(int cmd);

#endif
