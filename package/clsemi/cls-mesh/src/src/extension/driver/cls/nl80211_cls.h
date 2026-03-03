#ifndef _NL80211_CLS_H
#define _NL80211_CLS_H

#define GET_DEFAULT_IFNAME_BY_CHANNEL(channel) (channel < 36 ? "wlan0":"wlan1")

int nl80211GetInterfaceMode(const char *ifname);
int nl80211GetNacStaInfo(uint8_t channel, uint8_t *mac);


#endif
