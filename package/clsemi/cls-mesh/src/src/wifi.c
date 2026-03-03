#include <stdlib.h>
#include <arpa/inet.h>
#include "platform.h"
#include "wifi.h"
#include "1905_tlvs.h"

//start freqence segment table
static uint32_t freq_segs[] = {5945, 5735, 5490, 5170, 0};
//operating class per ieee80211 annex table e-4 (only for wifi b/a/g/n/ac/ax)
static struct opclass_channel_table opclass_channel_map[] = {
    {81,  0, NL80211_CHAN_WIDTH_20,
        {1,2,3,4,5,6,7,8,9,10,11,12,13,0}},
    {82,  0, NL80211_CHAN_WIDTH_20_NOHT,
        {14,0}},
    {83,  NL80211_CHAN_HT40PLUS, NL80211_CHAN_WIDTH_40,
        {1,2,3,4,5,6,7,8,9,0}},
    {84,  NL80211_CHAN_HT40MINUS, NL80211_CHAN_WIDTH_40,
        {5,6,7,8,9,10,11,12,13,0}},
    {115, 0, NL80211_CHAN_WIDTH_20,
        {36,40,44,48,0}},
    {116, NL80211_CHAN_HT40PLUS, NL80211_CHAN_WIDTH_40,
        {36,44,0}},
    {117, NL80211_CHAN_HT40MINUS, NL80211_CHAN_WIDTH_40,
        {40,48,0}},
    {118, 0, NL80211_CHAN_WIDTH_20,
        {52,56,60,64,0}},
    {119, NL80211_CHAN_HT40PLUS, NL80211_CHAN_WIDTH_40,
        {52,60,0}},
    {120, NL80211_CHAN_HT40MINUS, NL80211_CHAN_WIDTH_40,
        {56,64,0}},
    {121, 0, NL80211_CHAN_WIDTH_20,
        {100,104,108,112,116,120,124,128,132,136,140,144,0}},
    {122, NL80211_CHAN_HT40PLUS, NL80211_CHAN_WIDTH_40,
        {100,108,116,124,132,140,0}},
    {123, NL80211_CHAN_HT40MINUS, NL80211_CHAN_WIDTH_40,
        {104,112,120,128,136,144,0}},
    {124, 0, NL80211_CHAN_WIDTH_20,
        {149,153,157,161,0}},
    {125, 0, NL80211_CHAN_WIDTH_20,
        {149,153,157,161,165,169,173,177,0}},
    {126, NL80211_CHAN_HT40PLUS, NL80211_CHAN_WIDTH_40,
        {149,157,165,173,0}},
    {127, NL80211_CHAN_HT40MINUS, NL80211_CHAN_WIDTH_40,
        {153,161,169,177,0}},
    /* operting class 128 ~ 135 uses channel center frequency index */
    {128, 0, NL80211_CHAN_WIDTH_80,
        {42,58,106,122,138,155,171,0}},
    {129, 0, NL80211_CHAN_WIDTH_160,
        {50,114,163,0}},
    {130, 0, NL80211_CHAN_WIDTH_80,
        {42,58,106,122,138,155,171,0}},
    {131, 0, NL80211_CHAN_WIDTH_20,
        {1,5,9,13,17,21,25,29,33,37,41,45,49,53,57,61,65,69,73,77,81,85,89,93,97,101,105,109,113,
        117,121,125,129,133,137,141,145,149,153,157,161,165,169,173,177,181,185,189,193,197,201,
        205,209,213,217,221,225,229,233,0}},
    {132, 0, NL80211_CHAN_WIDTH_40,
        {3,11,19,27,35,43,51,59,67,75,83,91,99,107,115,123,131,139,147,155,163,171,179,187,195,203,
        211,219,227,0}},
    {133, 0, NL80211_CHAN_WIDTH_80,
        {7,23,39,55,71,87,103,119,135,151,167,183,199,215,0}},
    {134, 0, NL80211_CHAN_WIDTH_160,
        {15,47,79,111,143,175,207,0}},
    {135, 0, NL80211_CHAN_WIDTH_80,
        {7,23,39,55,71,87,103,119,135,151,167,183,199,215,0}},
    {136, 0, NL80211_CHAN_WIDTH_20,
        {2,0}},
    {0, 0, 0, {0}},
};

struct opclass_channel_table *findOperatingChannelTable(uint32_t opclass)
{
    struct opclass_channel_table *t = opclass_channel_map;

    while (t->opclass) {
        if (t->opclass==opclass)
            return t;
        t++;
    }
    return NULL;
}

uint32_t opclass2nlBandwidth(uint32_t opclass, enum nl80211_channel_type *ptype)
{
    struct opclass_channel_table *t;

    if ((t=findOperatingChannelTable(opclass))) {
        if (ptype) *ptype = t->type;
        return t->channelWidth;
    }

    return NL80211_CHAN_WIDTH_20;
}

#define CHANNEL_OFFSET(_bw) \
    ((_bw)==NL80211_CHAN_WIDTH_160?14:((_bw==NL80211_CHAN_WIDTH_80?6:2)))
int isOpclassChannelValid(uint8_t opclass, uint8_t channel)
{
    struct opclass_channel_table *t;
    uint8_t *pchan;

    if ((t=findOperatingChannelTable(opclass))) {
        pchan = t->chan_set;
        if ((t->channelWidth<=NL80211_CHAN_WIDTH_20)
            || (t->type)) {
            do {
                if ((*pchan)==channel)
                    return 1;
            } while (*(++pchan));
        } else {
            uint8_t chan_off = CHANNEL_OFFSET(t->channelWidth);
            do {
                if ((channel>=(*pchan)-chan_off) && (channel<=(*pchan)+chan_off))
                    return 1;
            } while (*(++pchan));
        }
    }

    return 0;
}

uint8_t freq2Band(uint32_t freq)
{
    if (freq<=2484)
        return band_2g;
    else if (freq<5950)
        return band_5g;
    else if (freq<=45000)
        return band_6g;
    else
        return 0;
}

uint8_t freq2BandIdx(uint32_t freq)
{
    if (freq<=2484)
        return band_2g_idx;
    else if (freq<5950)
        return band_5g_idx;
    else if (freq<=45000)
        return band_6g_idx;
    else
        return band_unknown;
}

uint8_t opclass2BandIdx(uint8_t opclass)
{
    if ((opclass>=81) && (opclass<=84)) {
        return band_2g_idx;
    } else if ((opclass>=115) && (opclass<=130)) {
        return band_5g_idx;
    } else if ((opclass>=131) && (opclass<=135)) {
        return band_6g_idx;
    } else {
        return band_none;
    }
}

uint8_t opclass2Band(uint8_t opclass)
{
    if ((opclass>=81) && (opclass<=84)) {
        return band_2g;
    } else if ((opclass>=115) && (opclass<=130)) {
        return band_5g;
    } else if ((opclass>=131) && (opclass<=135)) {
        return band_6g;
    } else {
        return band_none;
    }
}

/* band_type: 0: 2412Mhz - 2484Mhz, 1: 5180Mhz - 5925Mhz,
 * 3: 5955Mhz - 7115Mhz, 4: 902Mhz - 927Mhz */
uint8_t bandType2Band(uint16_t band_type)
{
    if (band_type == 0)
        return band_2g;
    else if (band_type == 1)
        return band_5g;
    else if (band_type == 3)
        return band_6g;
    else
        return band_none;
}

uint8_t band2BandIdx(uint8_t band)
{
    if (band == band_2g)
        return band_2g_idx;
    else if (band == band_5g)
        return band_5g_idx;
    else if (band == band_6g)
        return band_6g_idx;
    else
        return 0;
}

uint8_t bandIdx2Band(uint8_t bandidx)
{
    if (bandidx==band_2g_idx)
        return band_2g;
    else if (bandidx==band_5g_idx)
        return band_5g;
    else if (bandidx==band_6g_idx)
        return band_6g;
    else
        return 0;
}

int freq2Channel(uint32_t freq)
{
    /* see 802.11-2007 17.3.8.3.2 and Annex J */
    if (freq == 2484)
        return 14;
    /* see 802.11ax D6.1 27.3.23.2 and Annex E */
    else if (freq == 5935)
        return 2;
    else if (freq < 2484)
        return (freq - 2407) / 5;
    else if (freq >= 4910 && freq <= 4980)
        return (freq - 4000) / 5;
    else if (freq < 5950)
        return (freq - 5000) / 5;
    else if (freq <= 45000) /* DMG band lower limit */
        /* see 802.11ax D6.1 27.3.23.2 */
        return (freq - 5950) / 5;
    else if (freq >= 58320 && freq <= 70200)
        return (freq - 56160) / 2160;
    else
        return 0;
}

uint8_t nlbw2Idx(uint32_t nlbw)
{
    switch (nlbw) {
        case NL80211_CHAN_WIDTH_20:
        case NL80211_CHAN_WIDTH_20_NOHT:
            return bw_idx_20MHz;
            break;
        case NL80211_CHAN_WIDTH_40:
            return bw_idx_40MHz;
            break;
        case NL80211_CHAN_WIDTH_80:
        case NL80211_CHAN_WIDTH_80P80:
            return bw_idx_80MHz;
            break;
        case NL80211_CHAN_WIDTH_160:
            return bw_idx_160MHz;
            break;
    }
    return bw_idx_unknown;
}

uint8_t primaryChannel2CenterChannel(uint8_t primary_channel, enum nl80211_chan_width chan_width)
{
    switch (chan_width) {
    case NL80211_CHAN_WIDTH_80:
        if (primary_channel >= 36 && primary_channel <= 48)
            return 42;
        if (primary_channel >= 52 && primary_channel <= 64)
            return 58;
        if (primary_channel >= 100 && primary_channel <= 112)
            return 106;
        if (primary_channel >= 116 && primary_channel <= 128)
            return 122;
        if (primary_channel >= 132 && primary_channel <= 144)
            return 138;
        if (primary_channel >= 149 && primary_channel <= 161)
            return 155;
        break;
    case NL80211_CHAN_WIDTH_160:
        if (primary_channel >= 36 && primary_channel <= 64)
            return 50;
        if (primary_channel >= 100 && primary_channel <= 128)
            return 114;
        if (primary_channel >= 149 && primary_channel <= 177)
            return 163;
        break;
    default:
        return primary_channel;
    }

    return primary_channel;
}

/**
 * from hostapd ieee80211_freq_to_channel_ext - Convert frequency into channel info
 * for HT40, VHT, and HE. DFS channels are not covered.
 * @freq: Frequency (MHz) to convert
 * @sec_channel: 0 = non-HT40, 1 = sec. channel above, -1 = sec. channel below
 * @chanwidth: VHT/EDMG channel width (enum nl80211_chan_width)
 * @op_class: Buffer for returning operating class
 * @channel: Buffer for returning channel number
 * Returns: 0 on success, -1 on failure
 */

int ieee80211Freq2ChannelExt(uint32_t freq, int sec_channel, int chanwidth,
                           uint8_t *opclass, uint8_t *channel)
{
    uint8_t vht_opclass;

    /* TODO: more operating classes */

    if (sec_channel > 1 || sec_channel < -1)
        return -1;

    if (freq >= 2412 && freq <= 2472) {
        if ((freq - 2407) % 5)
            return -1;

        /* 2.407 GHz, channels 1..13 */
        if (sec_channel == 1)
            *opclass = 83;
        else if (sec_channel == -1)
            *opclass = 84;
        else
            *opclass = 81;

        *channel = (freq - 2407) / 5;

        return 0;
    }

    if (freq == 2484) {
        if (sec_channel)
            return -1;

        *opclass = 82; /* channel 14 */
        *channel = 14;

        return 0;
    }

    if (freq >= 4900 && freq < 5000) {
        if ((freq - 4000) % 5)
            return -1;
        *channel = (freq - 4000) / 5;
        *opclass = 0; /* TODO */
        return 0;
    }

    switch (chanwidth) {
    case NL80211_CHAN_WIDTH_80:
        vht_opclass = 128;
        break;
    case NL80211_CHAN_WIDTH_160:
        vht_opclass = 129;
        break;
    case NL80211_CHAN_WIDTH_80P80:
        vht_opclass = 130;
        break;
    default:
        vht_opclass = 0;
        break;
    }

    /* 5 GHz, channels 36..48 */
    if (freq >= 5180 && freq <= 5240) {
        if ((freq - 5000) % 5)
            return -1;

        if (vht_opclass)
            *opclass = vht_opclass;
        else if (sec_channel == 1)
            *opclass = 116;
        else if (sec_channel == -1)
            *opclass = 117;
        else
            *opclass = 115;

        *channel = (freq - 5000) / 5;

        return 0;
    }

    /* 5 GHz, channels 52..64 */
    if (freq >= 5260 && freq <= 5320) {
        if ((freq - 5000) % 5)
            return -1;

        if (vht_opclass)
            *opclass = vht_opclass;
        else if (sec_channel == 1)
            *opclass = 119;
        else if (sec_channel == -1)
            *opclass = 120;
        else
            *opclass = 118;

        *channel = (freq - 5000) / 5;

        return 0;
    }

    /* 5 GHz, channels 149..177 */
    if (freq >= 5745 && freq <= 5885) {
        if ((freq - 5000) % 5)
            return -1;

        if (vht_opclass)
            *opclass = vht_opclass;
        else if (sec_channel == 1)
            *opclass = 126;
        else if (sec_channel == -1)
            *opclass = 127;
        else if (freq <= 5805)
            *opclass = 124;
        else
            *opclass = 125;

        *channel = (freq - 5000) / 5;

        return 0;
    }

    /* 5 GHz, channels 100..144 */
    if (freq >= 5500 && freq <= 5720) {
        if ((freq - 5000) % 5)
            return -1;

        if (vht_opclass)
            *opclass = vht_opclass;
        else if (sec_channel == 1)
            *opclass = 122;
        else if (sec_channel == -1)
            *opclass = 123;
        else
            *opclass = 121;

        *channel = (freq - 5000) / 5;

        return 0;
    }

    if (freq >= 5000 && freq < 5900) {
        if ((freq - 5000) % 5)
            return -1;
        *channel = (freq - 5000) / 5;
        *opclass = 0; /* TODO */
        return 0;
    }

    if (freq > 5950 && freq <= 7115) {
        if ((freq - 5950) % 5)
            return -1;

        switch (chanwidth) {
        case NL80211_CHAN_WIDTH_80:
            *opclass = 133;
            break;
        case NL80211_CHAN_WIDTH_160:
            *opclass = 134;
            break;
        case NL80211_CHAN_WIDTH_80P80:
            *opclass = 135;
            break;
        default:
            if (sec_channel)
                *opclass = 132;
            else
                *opclass = 131;
            break;
        }

        *channel = (freq - 5950) / 5;
        return 0;
    }

    if (freq == 5935) {
        *opclass = 136;
        *channel = (freq - 5925) / 5;
        return 0;
    }

    return -1;
}

int channel2Freq(uint8_t chan, uint8_t opclass)
{
    /* see 802.11 17.3.8.3.2 and Annex J
     * there are overlapping channel numbers in 5GHz and 2GHz bands */
    if (chan <= 0)
        return 0; /* not supported */
    if (opclass<=84) {
        if (chan == 14)
            return 2484;
        else if (chan < 14)
            return 2407 + chan * 5;
    } else if (opclass<=130) {
        if (chan >= 182 && chan <= 196)
            return 4000 + chan * 5;
        else
            return 5000 + chan * 5;
    } else {
        /* see 802.11ax D6.1 27.3.23.2 */
        if (chan == 2)
            return 5935;
        if (chan <= 253)
            return 5950 + chan * 5;
    }
    return 0; /* not supported */
}

uint8_t freq2SpecChannel(uint32_t freq, uint8_t opclass)
{
    if (opclass<128)
        return freq2Channel(freq);
    else
        return freq2Channel(getCf1(opclass2nlBandwidth(opclass, NULL), freq, 0));
}

uint32_t getCf1(enum nl80211_chan_width width, uint32_t freq, enum nl80211_channel_type type)
{
    uint32_t factor, start_freq;
    int i = 0;

    switch (width) {
        case NL80211_CHAN_WIDTH_40:
            factor = 40;
            break;
        case NL80211_CHAN_WIDTH_80:
            factor = 80;
            break;
        case NL80211_CHAN_WIDTH_160:
            factor = 160;
            break;
        default:
            return freq;
            break;
    }
    //2ghz
    if (freq<5170) {
        if (type==NL80211_CHAN_HT40MINUS)
            return (freq-10);
        else
            return (freq+10);
    }

    //5/6ghz
    while ((start_freq = freq_segs[i]) && (start_freq>freq))
        i++;

    if (start_freq) {
        while (freq>start_freq+factor)
            start_freq+=factor;
        return (start_freq+(factor>>1));
    } else {
        return freq;
    }
}

uint8_t rssi2Rcpi(int rssi)
{
    if (!rssi)
        return 255; /* not available */
    if (rssi < -110)
        return 0;
    if (rssi > 0)
        return 220;
    return (rssi + 110) * 2;
}

int rcpi2Rssi(uint8_t rcpi)
{
    if (rcpi >= 220)
        return 0;
    return ((rcpi >> 1) - 110);
}

/*
 1. IN easymesh spec HT capa param:
    bits 7-6 Maximum number of supported Tx spatial streams.
     00: 1 Tx spatial stream
     01: 2 Tx spatial stream
     10: 3 Tx spatial stream
     11: 4 Tx spatial stream

    bits 5-4 Maximum number of supported Rx spatial streams.
     00: 1 Rx spatial stream
     01: 2 Rx spatial stream
     10: 3 Rx spatial stream
     11: 4 Rx spatial stream

    bit 3    Short GI support for 20 MHz
    bit 2    Short GI support for 40 MHz
    bit 1    HT support for 40 MHz
    bit 0    Reserved

 2. we receive from nl80211:
    ht_capa:
     bit 0     LDPC coding cpability
     bit 1     Support channel width set
     bits 2-3  SM power save
     bit 4     HT-greenfield
     bit 5     Short GI for 20 MHz
     bit 6     Short GI for 40 MHz
     bit 7     Tx STBC
     bits 8-9  Rx STBC
     bit 10    HT-delayed block ack
     bit 11    maximum A-MSDU length
     bit 12    DSSS/CCK mode in 40 MHz
     bit 13    PSMP support
     bit 14    Forty MHz intolerant
     bit 15    L-SIG TXOP protection support
    ht_mcsset:
     bits 0-76  Rx MCS Bitmask
     bits 77-79 reserved
     bits 80-89 Rx highest supported data rate
     bits 90-95 reserved
     bit 96     Tx MCS set defined
     bit 97     Tx Rx MCS set not equal
     bits 98-99 Tx maximum number spatial streams supported
     bit 100    Tx unequal modulation supported
     bits 101-127 reserved

 3. transfer to easymesh ht para
    ht_mcsset[12] & BIT(2-3)  -- supported Tx spatial streams.
    ht_capa & BIT(5)  -- Short GI support for 20 MHz
    ht_capa & BIT(6)  -- Short GI support for 40 MHz
    ht_capa & BIT(1)  -- HT support for 40 MHz
 */

uint8_t transfer2htcapa(uint16_t ht_capa, uint8_t *ht_mcsset)
{
    uint8_t tx_nss = 0;
    uint8_t rx_nss = 4;
    uint8_t ht_cap0 = 0;
    uint8_t capa = 0;

    while ((rx_nss > 1) && (0 == IEEE80211_HTCAP_MCS_VALUE(ht_mcsset, rx_nss - 1)))
        rx_nss--;
    rx_nss--;

    if (IEEE80211_HTCAP_MCS_TXRX_NOT_EQUAL(ht_mcsset)
        && IEEE80211_HTCAP_MCS_TX_DEFINED(ht_mcsset))
        tx_nss = IEEE80211_HTCAP_MCS_STREAMS(ht_mcsset);
    else
        tx_nss = rx_nss;

    capa |= (tx_nss << HT_CAPA_TXSS_MASK);
    capa |= (rx_nss << HT_CAPA_RXSS_MASK);
    ht_cap0 = ht_capa & 0x00ff;
    if (ht_cap0 & IEEE80211_HTCAP_C_SHORTGI20)
        capa |= HT_CAPA_SGI_20M;
    if (ht_cap0 & IEEE80211_HTCAP_C_SHORTGI40)
        capa |= HT_CAPA_SGI_40M;
    if (ht_cap0 & IEEE80211_HTCAP_C_CHWIDTH40)
        capa |= HT_CAPA_BW_40M;
    return capa;
}

/*
 1. IN easymesh spec VHT capa param:
    bits 7-5 Maximum number of supported Tx spatial streams.
     000: 1 Tx spatial stream
     001: 2 Tx spatial stream
     010: 3 Tx spatial stream
     011: 4 Tx spatial stream
     100: 5 Tx spatial stream
     101: 6 Tx spatial stream
     110: 7 Tx spatial stream
     111: 8 Tx spatial stream

    bits 4-2 Maximum number of supported Rx spatial streams.
     000: 1 Rx spatial stream
     001: 2 Rx spatial stream
     010: 3 Rx spatial stream
     011: 4 Rx spatial stream
     100: 5 Rx spatial stream
     101: 6 Rx spatial stream
     110: 7 Rx spatial stream
     111: 8 Rx spatial stream

    bit 1    Short GI support for 80 MHz.
    bit 0    Short GI support for 160 MHz and 80+80 MHz.
    bit 7    VHT support for 80+80 MHz.
    bit 6    VHT support for 160 MHz.
    bit 5    SU beamformer capable.
    bit 4    MU beamformer capable.
    bit 3-0  reserved.

 2. we receive from nl80211:
    vht_capa:
     bits 0-1   Maximum MPDU length
     bits 2-3   Supported channel width set
     bit 4      Rx LDPC
     bit 5      Short GI for 80 MHz
     bit 6      Short GI for 160 and 80+80 MHz
     bit 7      Tx STBC
     bits 8-10  Rx STBC
     bit 11     SU beamformer capable
     bit 12     SU beamformee capable
     bit 13-15  Beamformee STS capability
     bit 14     Forty MHz intolerant
     bit 15     L-SIG TXOP protection support
     bits 16-18 Number of sounding dimensions
     bit 19     MU beamformer capable
     bit 20     MU beamformee capable
     bit 21     VHT TXOP PS
     bit 22     +HTC-VHT capable
     bits 23-25 Maximum A-MPDU length exponent
     bits 26-27 VHT link adaptation capable
     bit 28     Rx antenna pattern consistency
     bit 29     Tx antenna pattern consistency
     bits 30-31 Extended NSS BW Support
    vht_rx_mss
     bits 0-15  Rx VHT-MCS map
        B0   B1    B2   B3    B4   B5    B6   B7    B8   B9    B10   B11    B12   B13    B14   B15
        Max VHT-   Max VHT-   Max VHT-   Max VHT-   Max VHT-   Max VHT-     Max VHT-     Max VHT-
        MCS For    MCS For    MCS For    MCS For    MCS For    MCS For      MCS For      MCS For
         1 SS        2 SS       3 SS       4 SS       5 SS       6 SS         7 SS         8 SS
    vht_tx_mss
     bits 0-15 Tx VHT-MCS map
        B0   B1    B2   B3    B4   B5    B6   B7    B8   B9    B10   B11    B12   B13    B14   B15
        Max VHT-   Max VHT-   Max VHT-   Max VHT-   Max VHT-   Max VHT-     Max VHT-     Max VHT-
        MCS For    MCS For    MCS For    MCS For    MCS For    MCS For      MCS For      MCS For
         1 SS        2 SS       3 SS       4 SS       5 SS       6 SS         7 SS         8 SS

     The Max VHT-MCS For n SS subfield (where n = 1, ..., 8) is encoded as follows:
        0 indicates support for VHT-MCS 0-7 for n spatial streams
        1 indicates support for VHT-MCS 0-8 for n spatial streams
        2 indicates support for VHT-MCS 0-9 for n spatial streams
        3 indicates that n spatial streams is not supported

 3. transfer to easymesh ht para
    capa[0]
     vht_tx_mss & BIT(0-15)  -- supported Tx spatial streams.
     vht_rx_mss & BIT(0-15)  -- supported Rx spatial streams.
     vht_capa & BIT(5)  -- support short GI for 80 MHz
     vht_capa & BIT(6)  -- support short GI for 160 MHz and 80+80 MHz
    capa[1]
 */
static uint8_t _getNssCodeFromMCS(uint16_t mcs)
{
    uint8_t nss = 8;
    while ((nss > 1) && ((mcs & IEEE80211_VHT_HE_MCSMAP_MASK) == IEEE80211_VHT_HE_MCSMAP_MASK)) {
        nss--;
        mcs <<= 2;
    }
    nss--;
    return nss;
}
uint16_t transfer2vhtcapa(uint8_t *vht_capa, uint16_t vht_rx_mss, uint16_t vht_tx_mss)
{
    uint8_t tx_nss = 0;
    uint8_t rx_nss = 0;
    uint8_t bw = 0;
    uint8_t ext_bw = 0;
    uint8_t value[2] = {0};
    uint16_t capa = 0;

    value[0] = 0x0;
    rx_nss = _getNssCodeFromMCS(vht_rx_mss);
    tx_nss = _getNssCodeFromMCS(vht_tx_mss);
    value[0] |= tx_nss << VHT_CAPA_TXSS_SHIFT;
    value[0] |= rx_nss << VHT_CAPA_RXSS_SHIFT;

    if (IEEE80211_VHTCAP_GET_SGI_80MHZ(vht_capa))
        value[0] |= VHT_CAPA_SGI_80M;
    if (IEEE80211_VHTCAP_GET_SGI_160MHZ(vht_capa))
        value[0] |= VHT_CAPA_SGI_160M;

    value[1] = 0x0;
    bw = IEEE80211_VHTCAP_GET_CHANWIDTH(vht_capa);
    ext_bw = IEEE80211_VHTCAP_GET_EXTENDED_NSS_BW_SUPPORT(vht_capa);
    if (0 == ext_bw && 0x02 == bw)
        value[1] |= VHT_CAPA_BW_8080M;
    if (0 == ext_bw && bw > 0x00 && bw <= 0x02)
        value[1] |= VHT_CAPA_BW_160M;
    if (IEEE80211_VHTCAP_GET_SU_BEAM_FORMER(vht_capa))
        value[1] |= VHT_CAPA_SU_BF;
    if (IEEE80211_VHTCAP_GET_MU_BEAM_FORMER(vht_capa))
        value[1] |= VHT_CAPA_MU_BF;
    capa = (uint16_t)(((value[1] << 8) | (value[0])));
    return capa;
}

/*
 1. IN easymesh spec HE capa param:
    bits 7-5 Maximum number of supported Tx spatial streams.
     000: 1 Tx spatial stream
     001: 2 Tx spatial stream
     010: 3 Tx spatial stream
     011: 4 Tx spatial stream
     100: 5 Tx spatial stream
     101: 6 Tx spatial stream
     110: 7 Tx spatial stream
     111: 8 Tx spatial stream

    bits 4-2 Maximum number of supported Rx spatial streams.
     000: 1 Rx spatial stream
     001: 2 Rx spatial stream
     010: 3 Rx spatial stream
     011: 4 Rx spatial stream
     100: 5 Rx spatial stream
     101: 6 Rx spatial stream
     110: 7 Rx spatial stream
     111: 8 Rx spatial stream

    bit 1    HE support for 80+80 MHz.
    bit 0    HE support for 160 MHz.
    bit 7    SU beamformer capable.
    bit 6    MU beamformer capable.
    bit 5    UL MU-MIMO capable.
    bit 4    UL MU-MIMO + OFDMA capable.
    bit 3    DL MU-MIMO + OFDMA capable.
    bit 2    UL OFDMA capable.
    bit 1    DL OFDMA capable.
    bit 0    reserved.

 2. we receive from nl80211:
    phy_cap[11]
    mac_cap[6]
    mcs[12]
    he_6ghz_capa

 3. transfer to easymesh he para
    capa[0]
     phy_cap[0] & BIT(1) -- support HE40/2.4GHz
     phy_cap[0] & BIT(2) -- support HE40/HE80/5GHz
     phy_cap[0] & BIT(3) -- support HE160/5GHz
     phy_cap[0] & BIT(4) -- support HE160/HE80+80/5GHz
    capa[1]
     mac_cap[3] & BIT(2) -- OFDMA RA
     phy_cap[6] & BIT(6) -- Partial Bandwidth DL MU-MIMO
     phy_cap[2] & BIT(7) -- Partial Bandwidth UL MU-MIMO
     phy_cap[2] & BIT(6) -- Full Bandwidth UL MU-MIMO
     phy_cap[4] & BIT(2) -- MU Beamformer
     phy_cap[3] & BIT(7) -- SU Beamformer
 */
uint16_t transfer2hecapa(uint8_t *mac_capa, uint8_t *phy_capa, uint8_t *mcs_set)
{
    uint8_t tx_nss = 0;
    uint8_t rx_nss = 0;
    uint8_t value[2] = {0};
    uint16_t capa = 0;

    /* HE support for 160 MHz. */
    if (IEEE80211_HECAP_GET_160M_SUPPORTED(phy_capa))
        value[0] |= HE_CAPA_BW_160M;
    /* HE support for 80+80 MHz. */
    if (IEEE80211_HECAP_GET_8080M_SUPPORTED(phy_capa))
        value[0] |= HE_CAPA_BW_8080M;
    /* Rx streams: 9.4.2.242.4 HE-MCS and NSS set field: Bytes 2-3: Tx HE-MCS Map << 80Mhz*/
    tx_nss = _getNssCodeFromMCS((uint16_t)(IEEE80211_HECAP_GET_TX_MCS_NSS_80M(mcs_set)));
    /* Rx streams: 9.4.2.242.4 HE-MCS and NSS set field: Bytes 0-1: Rx HE-MCS Map << 80Mhz*/
    rx_nss = _getNssCodeFromMCS((uint16_t)(IEEE80211_HECAP_GET_RX_MCS_NSS_80M(mcs_set)));
    value[0] |= tx_nss << HE_CAPA_TXSS_SHIFT;
    value[0] |= rx_nss << HE_CAPA_RXSS_SHIFT;

    /* DL OFDMA capable. */
    value[1] |= HE_CAPA_DL_OFDMA;
    /* UL OFDMA capable. */
    value[1] |= HE_CAPA_UL_OFDMA;
    /* DL MU-MIMO + OFDMA capable. */
    if (IEEE80211_HECAP_GET_PART_BW_DL_MUMIMO(phy_capa))
        value[1] |= HE_CAPA_DL_MUMIMO_OFDMA;
    /* UL MU-MIMO + OFDMA capable. */
    if (IEEE80211_HECAP_GET_PART_BW_UL_MUMIMO(phy_capa))
        value[1] |= HE_CAPA_UL_MUMIMO_OFDMA;
    /* UL MU-MIMO capable. */
    if (IEEE80211_HECAP_GET_FULL_BW_UL_MUMIMO(phy_capa))
        value[1] |= HE_CAPA_UL_MUMIMO;
    /* MU Beamformer. */
    if (IEEE80211_HECAP_GET_MU_BEAM_FORMER(phy_capa))
        value[1] |= HE_CAPA_MU_BF;
    /* SU Beamformer. */
    if (IEEE80211_HECAP_GET_SU_BEAM_FORMER(phy_capa))
        value[1] |= HE_CAPA_SU_BF;
    capa = (uint16_t)(((value[1] << 8) | (value[0])));
    return capa;
}

uint8_t *addNeighborReport(uint8_t *p, uint8_t *mac, uint8_t opclass, uint8_t channel, uint32_t bss_info,
                            uint8_t phy_type, uint8_t *subelement, int subelement_len)
{
    struct ieee80211_neighbor_report_element *neighbor_report = (struct ieee80211_neighbor_report_element *)p;
    uint8_t *psub = neighbor_report->subelements;

    neighbor_report->eid = IEEE80211_ELEMID_NEIGHBOR_REPORT;
    neighbor_report->len = sizeof(struct ieee80211_neighbor_report_element) + subelement_len - 2; /* plus candidate pref subelement, minus the len of header */
    MACCPY(neighbor_report->bssid, mac);
    neighbor_report->opclass = opclass;
    neighbor_report->channel = channel;
    neighbor_report->bssid_info = htonl(bss_info);
    /* set phytype */
    neighbor_report->phy_type = phy_type;

    if ((subelement) && (subelement_len)) {
        memcpy(psub, subelement, subelement_len);
        psub += subelement_len;
    }

    return psub;
}

uint8_t *addBeaconRequest(uint8_t *p, uint8_t token, uint8_t mode, uint8_t opclass, uint8_t chan_num, uint8_t *chan,
                            uint8_t *bssid, struct ssid *ssid, uint8_t detail, struct vvData *element_list)
{
    struct ieee80211_measure_req_report_element *measure_req = (struct ieee80211_measure_req_report_element *)p;
    uint8_t *psub = measure_req->u.beacon_req.subelements;

    measure_req->eid = IEEE80211_ELEMID_MEASREQ;
    measure_req->token = token;
    measure_req->mode = mode;
    measure_req->type = WLAN_MEASURE_TYPE_BEACON;

    measure_req->u.beacon_req.opclass = opclass;
    if (chan_num==1)
        measure_req->u.beacon_req.channel = chan[0];
    else
        measure_req->u.beacon_req.channel = 255;

    measure_req->u.beacon_req.random_interval = 0;
    measure_req->u.beacon_req.duration = htons(BEACON_REQUEST_DEFAULT_DURATION);
    measure_req->u.beacon_req.mode = mode;
    MACCPY(measure_req->u.beacon_req.bssid, bssid);

    {
        *psub++ = IEEE80211_BEACONREQ_SUBELEMID_DETAIL;
        *psub++ = 1;
        *psub++ = detail;
    }

    if ((ssid) && (ssid->len)) {
        *psub++ = IEEE80211_BEACONREQ_SUBELEMID_SSID;
        *psub++ = ssid->len;
        memcpy(psub, ssid->ssid, ssid->len);
        psub += ssid->len;
    }

    if (chan_num>1) {
        *psub++ = IEEE80211_BEACONREQ_SUBELEMID_AP_CHANNEL_RPT;
        *psub++ = chan_num+1;
        *psub++ = opclass;
        memcpy(psub, chan, chan_num);
        psub += chan_num;
    }

    if (element_list->len) {
        *psub++ = IEEE80211_BEACONREQ_SUBELEMID_REQUEST;
        *psub++ = element_list->len;
        memcpy(psub, element_list->datap, element_list->len);
        psub += element_list->len;
    }

    measure_req->len = psub-(uint8_t *)measure_req-2;

    return psub;
}

bool parse80211IEs(uint8_t *frm, uint32_t len,
                    uint8_t offset, struct ieee80211_ies *ies)
{
    uint8_t *pos = frm + offset, *efrm = frm + len;

    if (!frm || !ies || pos > efrm)
        return false;

    while (pos + 2 <= efrm) {
        if (pos + 2 + pos[1] > efrm)
            return false;

        switch (pos[0]) {
        case IEEE80211_ELEMID_SSID:
            ies->ssid = pos;
            break;
        case IEEE80211_ELEMID_MESHID:
            ies->meshid = pos;
            break;
        case IEEE80211_ELEMID_HTCAP:
            ies->htcap = pos;
            break;
        case IEEE80211_ELEMID_HTINFO:
            ies->htop = pos;
            break;
        case IEEE80211_ELEMID_VHTCAP:
            ies->vhtcap = pos;
            break;
        case IEEE80211_ELEMID_VHTOP:
            ies->vhtop = pos;
            break;
        case IEEE80211_ELEMID_BSS_LOAD:
            ies->bss_load = pos;
            break;
        case IEEE80211_ELEMID_EXT:
            if (pos[2] == IEEE80211_ELEMID_EXT_HECAP)
                ies->hecap = pos;
            else if (pos[2] == IEEE80211_ELEMID_EXT_HEOP)
                ies->heop = pos;
            break;
        default:
            break;
        }

        pos += pos[1] + 2;
    }

    return (pos == efrm);
}

bool parseSsidIE(uint8_t *ie, uint8_t *ssid, uint16_t *len)
{
    if (!ie || ie[0] != IEEE80211_ELEMID_SSID)
        return false;

    if (ssid) {
        *len = ie[1] < MAX_SSID_LEN ? ie[1] : MAX_SSID_LEN;
        memcpy(ssid, ie + 2, *len);
        ssid[*len] = 0;
    }

    return true;
}

bool parseMeshIdIE(uint8_t *ie, uint8_t *meshid, uint16_t *len)
{
    if (!ie ||ie[0] != IEEE80211_ELEMID_MESHID)
        return false;

    if (meshid) {
        *len = ie[1] < MAX_SSID_LEN ? ie[1] : MAX_SSID_LEN;
        memcpy(meshid, ie + 2, *len);
        meshid[*len] = 0;
    }

    return true;
}

bool parseBssLoadIE(uint8_t *ie, uint16_t *stas, uint8_t *util)
{
    if (!ie || ie[0] != IEEE80211_ELEMID_BSS_LOAD
            || ie[1] < 5)
        return false;

    if (stas)
        *stas = ntohs(*(uint16_t *)(ie + 1 + 1));
    if (util)
        *util = ie[1 + 1 + 2];
    return true;
}

uint8_t parseHeopInfoBssColor(uint8_t *ie)
{
    if (!ie || ie[2] != IEEE80211_ELEMID_EXT_HEOP)
        return 0;

    return ie[6];
}

uint8_t _parseVHTinfoBW(uint8_t *vhtop_info, uint8_t bw)
{
    if (vhtop_info[0] > IEEE80211_VHTOP_CHAN_WIDTH_80MHZ) {
        bw = bw_idx_80MHz;
    } else if (vhtop_info[0] == IEEE80211_VHTOP_CHAN_WIDTH_80MHZ) {
        uint8_t cf0 = vhtop_info[1];
        uint8_t cf1 = vhtop_info[2];
        if (cf1) {
            uint8_t gap = (cf0 > cf1) ? (cf0 - cf1) : (cf1 - cf0);
            if (gap == 8)
                bw = bw_idx_160MHz;
            else if (gap > 8)
                bw = bw_idx_80P80MHz;
        }
    }
    return bw;
}

uint8_t _parseHEcapBW(bool is5G, struct ieee80211_ie_hecap *hecap, uint8_t bw)
{
    uint8_t val = hecap->phy_cap[0] >> 1;

    if (is5G) {
        if (val & IEEE80211_HECAP_PHY_CHAN_WIDTH_5G_160MHZ)
            bw = bw_idx_160MHz;
        else if (val & IEEE80211_HECAP_PHY_CHAN_WIDTH_5G_80P80MHZ)
            bw = bw_idx_80P80MHz;
        else if (val & IEEE80211_HECAP_PHY_CHAN_WIDTH_5G_40P80MHZ)
            bw = bw_idx_80MHz;
    } else if (val & IEEE80211_HECAP_PHY_CHAN_WIDTH_2P4G_40MHZ) {
        bw = bw_idx_40MHz;
    }
    return bw;
}

uint8_t _parseVHTcapBW(struct ieee80211_ie_vhtcap *vhtcap, uint8_t bw)
{
    uint8_t val = IEEE80211_VHTCAP_GET_CHANWIDTH(vhtcap->cap);
    if (val == IEEE80211_VHTCAP_CW_80M_ONLY)
        bw = bw_idx_80MHz;
    else if (val == IEEE80211_VHTCAP_CW_160M)
        bw = bw_idx_160MHz;
    else if (val == IEEE80211_VHTCAP_CW_160_AND_80P80M)
        bw = bw_idx_80P80MHz;

    return bw;
}

uint8_t parseBWFromIEs(bool is5G, struct ieee80211_ies *ies)
{
    struct ieee80211_ie_htcap *htcap = (struct ieee80211_ie_htcap *)ies->htcap;
    struct ieee80211_ie_htinfo *htop = (struct ieee80211_ie_htinfo *)ies->htop;
    struct ieee80211_ie_vhtcap *vhtcap = (struct ieee80211_ie_vhtcap *)ies->vhtcap;
    struct ieee80211_ie_vhtop *vhtop = (struct ieee80211_ie_vhtop *)ies->vhtop;
    struct ieee80211_ie_hecap *hecap = (struct ieee80211_ie_hecap *)ies->hecap;
    struct ieee80211_ie_heop *heop = (struct ieee80211_ie_heop *)ies->heop;
    uint8_t *info = NULL, max_bw = bw_idx_20MHz;

    if (htop) {
        if (htop->bytes[0] & IEEE80211_HTINFO_B1_SEC_CHAN_OFFSET)
            max_bw = bw_idx_40MHz;
    } else if (htcap) {
        if (htcap->cap[0] & IEEE80211_HTCAP_C_CHWIDTH40)
            max_bw = bw_idx_40MHz;
    }

    if (vhtop)
        info = vhtop->info;
    else if (hecap && heop && IEEE80211_HEOP_GET_VHTOP_PRESENT(heop))
        info = ((uint8_t *)heop) + sizeof(heop);

    if (info)
        max_bw = _parseVHTinfoBW(info, max_bw);
    else if (hecap)
        max_bw = _parseHEcapBW(is5G, hecap, max_bw);
    else if (vhtcap)
        max_bw = _parseVHTcapBW(vhtcap, max_bw);

    return max_bw;
}

void parseHTCapaIE(struct band_capability *bands_capa, uint8_t *ie)
{
    struct ieee80211_ie_htcap *ht_cap = NULL;
    uint16_t capa = 0;

    if (!ie)
        return;

    ht_cap = (struct ieee80211_ie_htcap *)ie;
    capa = (uint16_t)((ht_cap->cap[1] << 8) | (ht_cap->cap[0]));
    bands_capa->ht_capa.capa = transfer2htcapa(capa, ht_cap->mcsset);
    DEBUG_DETAIL("HT capability: bands_capa->ht_capa.capa=%d\n", bands_capa->ht_capa.capa);
    bands_capa->ht_capa_valid = true;
}

void parseVHTCapaIE(struct band_capability *bands_capa, uint8_t *ie)
{
    struct ieee80211_ie_vhtcap *vht_capa = NULL;

    if (!ie)
        return;

    vht_capa = (struct ieee80211_ie_vhtcap *)ie;
    bands_capa->vht_capa.tx_mcs = IEEE80211_VHTCAP_GET_TX_MCS_NSS(vht_capa->mcs_nss_set);
    bands_capa->vht_capa.rx_mcs = IEEE80211_VHTCAP_GET_RX_MCS_NSS(vht_capa->mcs_nss_set);
    bands_capa->vht_capa.capa = transfer2vhtcapa(vht_capa->cap, bands_capa->vht_capa.rx_mcs, bands_capa->vht_capa.tx_mcs);
    DEBUG_DETAIL("VHT capability: bands_capa->vht_capa.capa=%d\n", bands_capa->vht_capa.capa);
    bands_capa->vht_capa_valid = true;
}

void parseHECapaIE(struct band_capability *bands_capa, uint8_t *ie)
{
    struct ieee80211_ie_hecap *he_capa = NULL;

    if (!ie)
        return;

    he_capa = (struct ieee80211_ie_hecap *)ie;
    bands_capa->he_capa.capa = transfer2hecapa(he_capa->mac_cap, he_capa->phy_cap, he_capa->mcs_map_le80);
    DEBUG_DETAIL("HE capability: bands_capa->he_capa.capa=%d\n", bands_capa->he_capa.capa);
    bands_capa->vht_capa_valid = true;
}

uint8_t chanSurveyFindOpclass(uint8_t chan)
{
    int i, k;

    for (i = 0; i < ARRAY_SIZE(opclass_channel_map); i++) {
        if (opclass_channel_map[i].channelWidth == NL80211_CHAN_WIDTH_20) { /* channel survey focused on primary channel */
            for (k = 0; k < MAX_CHANNEL_PER_OPCLASS && opclass_channel_map[i].chan_set[k]; k++) {
                if (opclass_channel_map[i].chan_set[k] == chan)
                   return opclass_channel_map[i].opclass;
            }
        }
    }
    return 0;
}

