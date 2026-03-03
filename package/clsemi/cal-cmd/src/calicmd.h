#ifndef _CALICMD_H
#define _CALICMD_H

#define ARRAY_SIZE(a) (sizeof((a))/sizeof((a)[0]))
char *filepath_set = "/sys/kernel/debug/calidir/calicmd";
char *filepath_show = "/sys/kernel/debug/calidir/show";

char cmd_type[][16] = {"radio", "bss", "ptk", "gtk", "su", "mu", "mac-phy", "mu-info",
                       "sounding", "wmm", "xtal_cal_status", "rx-stats", "igtk", "csi",
			"pppc", "mld", "ap-mld", "sta-mld", "tdma"};

//0: 2.4G, 1: 5G
char *radio_list[] = {"2G", "5G", NULL};

//0: "AP" / 1:"STA"
char *mode_list[] = {"AP", "STA", NULL};

// 0: "11b" / 1: "11a" / 2: "11g" / 3: "11n_2g" / 4: "11an_5g" / 5: "11ac" / 6: "11be"
char *phymode_list[] = {"11b", "11a", "11g", "11n_2g", "11n_5g", "11ac", "11be", NULL};

//0: none / 1: WEP / 2: TKIP / 3: CCMP / 4: WAPI / 5: GCMP
char *ciper_list[] = {"none", "wep", "tkip", "ccmp", "wapi", "gcmp", "bip-cmac-128", "bip-cmac-256", "wapi-gcm", "sft-gcmp", NULL};

//0: NON-HT / 1: NON-HT-DUP-OFDM / 2: HT-MF / 3: HT-GF
//4: VHT / 5: HE-SU / 6: HE-MU / 7: HE-ER-SU / 8: HE-TB / 9: EHT-MU
char *ppdu_type_list[] = {"NON-HT", "NON-HT-DUP-OFDM", "HT-MF", "HT-GF",
				"VHT", "HE-SU", "HE-MU", "HE-ER-SU", "HE-TB",
				"EHT-MU", "EHT-TB", NULL};

//0: EHT-MU-OFDMA / 1: EHT-MU-SU / 2: EHT-MU-MIMO
char *mac_ppdu_type_list[] = {"EHT-MU-OFDMA", "EHT-MU-SU", "EHT-MU-MIMO", NULL};

//0: "long" / 1: "short" / 2: "0.8" / 3: "1.6" / 4: "3.2"
char *gi_list[] = {"long", "short", "0.8", "1.6", "3.2", NULL};

// "prot_nav_frm_ex=string". Valid string: 0: "none" / 1: "self_cts" / 2: "rts_cts" / 3: "rts_cts_qap" / 4: "stbc"
char *nav_frm_list[] = {"none", "self_cts", "rts_cts", "rts_cts_qap", "stbc", NULL};

// Valid string: 0:"0"/1: "0.25" / 2: "0.5" /3: "1"/ 4: "2" / 5: "4" / 6: "8" / 7: "16"
char *mpdu_space_list[] ={"0", "0.25", "0.5", "1", "2", "4", "8", "16", NULL};

//Valid string: 0: no_ack / 1: normal / 2: implicit
char *ba_ack_list[] = {"no_ack", "normal", "implicit", NULL};

char *bw_list[] = {"20", "40", "80", "160", "320", "8080", "242", "106", NULL};

char *txpower_list[] = {"0x80", "0xFF", "0x00", "0x0", "0x1", "0x01", "0x3F", NULL};

char *he_ltf_list[] = {"1", "2", "4", NULL};

char *num_he_ltf_list[] = {"1", "2", "4", "6", "8", NULL};

char *num_eht_ltf_list[] = {"1", "2", "4", NULL};

char *packet_ext_list[] = {"0", "8", "16", "20", NULL};

//Format : For legacy-rate, it's 1/2/5/11/6/9/12/18/24/36/48/54.
char *mcs_legacy_list[] = {"1", "2", "5", "11", "6", "9", "12", "18", "24", "36", "48", "54", NULL};

//Valid string: 0: round down / 1: round up
char *nsymb_choose_list[] = {"down", "up", "0", "1", NULL};

//Valid string: 0:enable/1:disable
char *both_value_list[] = {"0", "1", "on", "off", NULL};

char *radio_name_list[] = {"radio", NULL};

char *bss_name_list[] = {"mode", "phy-mode", "bw", "channel", "mu-user", "current-ac", "local-addr", "tx-ppdu-cnt",
                         "tx-interval", "bw_change", "mfp", NULL};

char *ptk_name_list[] = {"mac-addr", "cipher", "key-len", "key", "hw-index", "wep-index",
			"bw-sig-force", NULL};

char *macphy_name_list[] = {"ppdu-type", "mac-ppdu-type", "stbc", "gi", "gid", "bw-ppdu", "spatial-reuse", "n-tx-prot",
                            "midable", "doppler", "num-ext-nss", "bss-color","antenna-set", "tx-power-level",
                            "he-ltf", "preamble-type", "prot-nav-frm-ex", "prot-tx-power", "prot-format-mod",
                            "prot-preamble-type", "prot-bw", "prot-mcs", "dyn-pre-punc-type", "num-he-ltf", "num-eht-ltf",
			"center-26tone-ru", "sigb-comp", "sigb-dcm", "sigb-mcs", "sib-ru-alloc1",
			"sib-ru-alloc2", "beamformed", "inact-subchan-bitmap", "dropbwentx", "txopdurationen",
                            "txopduration", "smm-index", "smart-ant-en", "smart-ant-param0", "smart-ant-param1", "hw-retry-en",
			"sr-ppdu-min-mcs", "non-standard-ta-func", "non-standard-ta",
			"mpdu-retry-limit", "no-just-hard-retry-limit", "prot-retry-limit",
			"sr-ppdu-min-mcs", "non-standard-ta-func", "non-standard-ta", "dyn-bw",
			"mpdu-retry-limit", "no-just-hard-retry-limit", "prot-retry-limit",
			"txbf-filter-en", "txbf-alpha", "txbf-smooth", NULL};

char *su_name_list[] = {"mac-addr", "mpdu-type", "mpdu-subtype", "mpdu-protect", "mpdu-htc", "mpdu-ackpolicy",
                        "mpdu-body", "mpdu-ssn", "mpdu-subframe", "mpdu-body-len", "apep-len", "ppdu-protection",
                        "mpdu-duration", "mpdu-seq-ctrl", "mpdu-qos-ctrl", "mpdu-amsdu-num", "ppdu-bf",
                        "mpdu-ignore-cca", "aid", "paid", "sta-index", "tx-tid", "ba-rx-size", "ba-tx-size",
                        "ba-ack-policy", "ba-ssn", "fec", "mcs", "mcs-legacy", "packet-ext", "dcm",
                        "min-mpdu-space", "cbf-host-addr", "cbf-report-len", "user-pos", "start-sts", "ss-num",
                        "ru-index", "ru-size", "ru-offset", "ssn-reset", "non-qos-ack", "bw-signal-en", "mpdu-fc-flags",
                        "eof-padding", "retry-rate-dec", "sr-disallow", "user-bf", "mfp", "lock-csi",
                        "ru-pwr-factor", "dropMCSEnTx",  "pow_split_en", "sr-drop-pwr-en",
                        "sr-adj-mcs-delta", "spp", "is-eht", NULL};

char *muinfo_name_list[] = {"mu-type", "mu-ack", "mu-prot", "tb-hw", "ul-duration", "en-sample",
				"pre-fec-pad-factor", "ldpc-extra-sym", "pe-disambiguity",
				"ul-mu-start-num", "trig-ppdu-id", "nsymb-choose", NULL};

char *wmm_name_list[] = {"aifsn", "cw-min", "cw-max", NULL};

char *sounding_name_list[] = {"type", "nb-sta", "feedback-type", "nc-idx", "ng", "sequence", NULL};

char *rx_stats_list[] = {"rx-stats", NULL};

char *radar_detect_list[] = {"enable", NULL};

char *interference_detect_list[] = {"enable", NULL};

char *rssi_list[] = {"enable", "debug", "max-num", NULL};

char *csi_name_list[] = {"csi-on", "format-on", "base-addr", "bank-size", "bank-num",
		"smp-mode-sel", "smp-cont-sel0", "smp-cont-sel1", "smpout-send-wait",
		"rx-ppdu-symb-thresh", "rx-time-thresh", "l-ltf-gran", "non-he-ltf-gran",
		"he-ltf-gran", "csi_flag", NULL};
char *pppc_name_list[] = {"enable", "template", NULL};

char *tdma_name_list[] = {"tdma-en", "mode", "ack-duration", NULL};

char *help = "Usable command List:\n"
            "\n\tcalcmd set radio 2G|5G"
            "\n"
            "\n\tcalcmd set bss [mode sta|ap] [bw 20|40|80|160|320|8080] [channel 36|40|...]"
            "\n\t\t [bw_change 0|1|2][mu-user 1-16] [current-ac 1-4]"
            "\n"
            "\n\tcalcmd set ptk|gtk mac-addr xx:xx:xx:xx:xx:xx [cipher none|wep|tkip|ccmp|wapi|gcmp]"
            "\n\t\t [key-len x] [key xxxxx] [hw-index xx] [wep-index 0-3]"
            "\n"
            "\n\tcalcmd set mac-phy [ppdu-type non-ht|non-ht-dup-ofdm|ht-mf|ht-gf|vht|he-su|he-mu|he-er-su|he-tb]"
            "\n\t\t [stbc 0|1] [gi long|short|0.8|1.6|3.2] [gid 0-63] [bw-ppdu 20|40|80|160|320|8080]"
            "\n\t\t [spatial-reuse xx] [n-tx-prot xx] [midable 0|1] [doppler 0|1] [num-ext-nss xx]"
            "\n\t\t [bss-color xx] [antenna-set xx] [tx-power-level -128-127]"
            "\n\t\t [he-ltf 1|2|4] [preamble-type 0|1]"
            "\n\t\t [prot-nav-frm-ex none|self_cts|rts_cts|rts_cts_qap|stbc]"
            "\n\t\t [prot-tx-power -128-127]"
            "\n\t\t [prot-format-mod non-ht|non-ht-dup-ofdm|ht-mf|ht-gf|vht|he-su|he-mu|he-er-su|he-tb]"
            "\n\t\t [prot-preamble-type 0|1] [prot-bw  20|40|80|160|320|8080]"
            "\n\t\t [prot-mcs xx] [dyn-pre-punc-type xx][num-he-ltf 1|2|4|6|8] [center-26tone-ru xx][sigb-comp 0|1]"
            "\n\t\t [sigb-dcm 0|1] [sigb-mcs xx] [sib-ru-alloc1 0xXX] [sib-ru-alloc2 0xXX] [beamformed 0|1]"
            "\n\t\t [dropbwentx 0|1] [txopdurationen 0|1] [txopduration xx] [hw-retry-en 0|1] [sr-ppdu-min-mcs xx]"
            "\n"
            "\n\tcalcmd set su mac-addr xx:xx:xx:xx:xx:xx [mpdu-type 0-2] [mpdu-subtype xx] [mpdu-protect 0|1]"
            "\n\t\t [mpdu-htc 0|1] [mpdu-ackpolicy 0|1] [mpdu-body XX] [mpdu-ssn xxx] [mpdu-subframe xx]"
            "\n\t\t [mpdu-body-len xx] [ppdu-protection 0|1] [mpdu-duration xx] [mpdu-seq-ctrl 0xXX]"
            "\n\t\t [mpdu-qos-ctrl 0xXX] [mpdu-amsdu-num xx] [ppdu-bf 0|1] [mpdu-ignore-cca 0|1]"
            "\n\t\t [aid xx] [paid xx] [sta-index xx] [tx-id xx] [ba-rx-size xx] [ba-tx-size xx]"
            "\n\t\t [ba-ack-policy no_ack|normal|implicit] [ba-ssn xx]"
            "\n\t\t [fec 0|1] [mcs nxx] [mcs-legacy xxx] [packet-ext 0|8|16] [dcm 0|1]"
            "\n\t\t [min-mpdu-space 0|0.25|0.5|1|2|4|8|16] [cbf-host-addr 0xXX]"
            "\n\t\t [cbf-report-len xx] [user-pos xx] [start-sts xx] [ss-num xx]"
            "\n\t\t [ru-index xx] [ru-size xx] [ru-offset xx] [non-qos-ack 0|1] [bw-signal-en 0|1] [mpdu-fc-flags xx]"
            "\n\t\t [eof-padding xx] [retry-rate-dec 0|1] [sr-disallow 0|1] [sr-drop-pwr-en 0|1] [ru-pwr-factor xx]"
            "\n\t\t [dropMCSEnTx 0|1] [pow_split_en 0-7] [sr-adj-mcs-delta xx] [is-eht 0|1]"
            "\n"
            "\n\tcalcmd set mu 1-16 mac-addr xx:xx:xx:xx:xx:xx [aid xx] [paid xx] [fec 0|1] [mcs nxx] [mcs-legacy xxx]"
            "\n"
            "\n\tcalcmd set mu-info [mu-type 0-3] [mu-ack 0-3] [mu-prot 0|1] [ul-duration xx] [en-sample 0|1]"
            "\n"
            "\n\tcalcmd set wmm 1-4 [aifsn xx] [cw-min xx] [cw-max xx]"
            "\n"
            "\n\tcalcmd set sounding [type 0|1] [nb-sta xx] [feedback-type 0|1] [nc-idx xx] [ng 0|1]"
            "\n"
            "\n\tcalcmd set rx-stats on|off"
            "\n"
            "\n\tcalcmd set pppc [enable 0|1] [template txpower1,cnt1,txpower2,cnt2....txpowern,cntn]"
            "\n"
            "\n\tcalcmd show rx-stats|rx-last|tx-stats|rssi-stats|rssi-stats-avg|leg-rssi-stats|leg-rssi-stats-avg"
            "\n"
            "\n\tcalcmd show radio|bss|ptk|gtk|mac-phy|su|mu|sounding|wmm|mu-info|pppc [all]"
            "\n"
            "\n\tcalcmd read-mem regaddr num"
            "\n"
            "\n\tcalcmd write-mem regaddr val1 val2...valn"
            "\n"
            "\n\tcalcmd update"
            "\n"
            "\n\tcalcmd tx-mu"
            "\n"
            "\n\tcalcmd reset"
            "\n"
            "\n\tcalcmd sounding"
            "\n"
            "\n\tcalcmd tx-su [en-sample 0|1] [sample-num xx]"
            "\n"
            "\n\tcalcmd dif-sample [sample-num xx] [chan 1|2]"
            "\n"
            "\n\tcalcmd radar-detect [enable 0|1]"
            "\n"
            "\n\tcalcmd interference-detect [enable 0|1]"
            "\n"
            "\n\tcalcmd rssi [enable 0|1] [debug 0|1] [max-num] xx\n";


enum cali_cmd_type {
    TYPE_RADIO,
    TYPE_BSS,
    TYPE_PTK,
    TYPE_GTK,
    TYPE_SU,
    TYPE_MU,
    TYPE_MACPHY,
    TYPE_MUINFO,
    TYPE_SOUNDING,
    TYPE_WMM,
    TYPE_RXSTATS,
    TYPE_RADAR,
    TYPE_INTERFERENCE,
    TYPE_RSSI,
    TYPE_CSI,
    TYPE_PPPC,
    TYPE_MLD,
    TYPE_AP_MLD,
    TYPE_STA_MLD,
    TYPE_TDMA,
    TYPE_MAX
};

struct type_name_list {
    enum cali_cmd_type type;
    char **name_list;
};

struct type_name_list type_name_config[] = {
    {TYPE_RADIO, radio_name_list},
    {TYPE_BSS, bss_name_list},
    {TYPE_PTK, ptk_name_list},
    {TYPE_GTK, ptk_name_list},
    {TYPE_SU, su_name_list},
    {TYPE_MU, su_name_list},
    {TYPE_MACPHY, macphy_name_list},
    {TYPE_MUINFO, muinfo_name_list},
    {TYPE_WMM, wmm_name_list},
    {TYPE_SOUNDING, sounding_name_list},
    {TYPE_RXSTATS, rx_stats_list},
    {TYPE_RADAR, radar_detect_list},
    {TYPE_INTERFERENCE, interference_detect_list},
    {TYPE_RSSI, rssi_list},
    {TYPE_CSI, csi_name_list},
    {TYPE_PPPC, pppc_name_list},
    {TYPE_TDMA, tdma_name_list},
};

struct name_int_value_pair {
    enum cali_cmd_type type;
    char *name;
    int   minval;
    int   maxval;
};

struct name_string_value_list {
    enum cali_cmd_type type;
    char *name;
    char **value_list;
};

struct name_string_value_list  name_string_value_config[] = {
    {TYPE_RADIO, "radio", radio_list},
    {TYPE_BSS, "mode", mode_list},
    {TYPE_BSS, "bw", bw_list},
    {TYPE_PTK, "cipher", ciper_list},
    {TYPE_GTK, "cipher", ciper_list},
    {TYPE_MACPHY, "ppdu-type", ppdu_type_list},
	{TYPE_MACPHY, "mac-ppdu-type", mac_ppdu_type_list},
    {TYPE_MACPHY, "prot-format-mod", ppdu_type_list},
    {TYPE_MACPHY, "gi", gi_list},
    {TYPE_MACPHY, "prot-nav-frm-ex", nav_frm_list},
    {TYPE_MACPHY, "bw-ppdu", bw_list},
    {TYPE_MACPHY, "prot-bw", bw_list},
    {TYPE_MACPHY, "he-ltf", he_ltf_list},
    {TYPE_MACPHY, "num-he-ltf", num_he_ltf_list},
    {TYPE_MACPHY, "num-eht-ltf", num_eht_ltf_list},
    {TYPE_MACPHY, "stbc", both_value_list},
    {TYPE_MACPHY, "midable", both_value_list},
    {TYPE_MACPHY, "doppler", both_value_list},
    {TYPE_MACPHY, "preamble-type", both_value_list},
    {TYPE_MACPHY, "prot-preamble-type", both_value_list},
    {TYPE_MACPHY, "sigb-comp", both_value_list},
    {TYPE_MACPHY, "sigb-dcm", both_value_list},
    {TYPE_MACPHY, "beamformed", both_value_list},
    {TYPE_MACPHY, "dropbwentx", both_value_list},
    {TYPE_MACPHY, "txopdurationen", both_value_list},
    {TYPE_SU, "mcs-legacy", mcs_legacy_list},
    {TYPE_SU, "packet-ext", packet_ext_list},
    {TYPE_SU, "min-mpdu-space", mpdu_space_list},
    {TYPE_SU, "ba-ack-policy", ba_ack_list},
    {TYPE_SU, "mpdu-protect", both_value_list},
    {TYPE_SU, "mpdu-htc", both_value_list},
    {TYPE_SU, "ppdu-protection", both_value_list},
    {TYPE_SU, "ppdu-bf", both_value_list},
    {TYPE_SU, "mpdu-ignore-cca", both_value_list},
    {TYPE_SU, "fec", both_value_list},
    {TYPE_SU, "dcm", both_value_list},
    {TYPE_SU, "ssn-reset", both_value_list},
    {TYPE_SU, "dropMCSEnTx", both_value_list},
    {TYPE_SU, "sr-disallow", both_value_list},
    {TYPE_SU, "sr-drop-pwr-en", both_value_list},
    {TYPE_MU, "mcs-legacy", mcs_legacy_list},
    {TYPE_MU, "packet-ext", packet_ext_list},
    {TYPE_MU, "min-mpdu-space", mpdu_space_list},
    {TYPE_MU, "ba-ack-policy", ba_ack_list},
    {TYPE_MU, "mpdu-protect", both_value_list},
    {TYPE_MU, "mpdu-htc", both_value_list},
    {TYPE_MU, "ppdu-protection", both_value_list},
    {TYPE_MU, "ppdu-bf", both_value_list},
    {TYPE_MU, "mpdu-ignore-cca", both_value_list},
    {TYPE_MU, "fec", both_value_list},
    {TYPE_MU, "dcm", both_value_list},
    {TYPE_MU, "ssn-reset", both_value_list},
    {TYPE_MU, "dropMCSEnTx", both_value_list},
    {TYPE_RXSTATS, "rx-stats", both_value_list},
    {TYPE_MUINFO, "en-sample", both_value_list},
	{TYPE_MUINFO, "nsymb-choose", nsymb_choose_list},
    {TYPE_RADAR, "enable", both_value_list},
    {TYPE_INTERFERENCE, "enable", both_value_list},
    {TYPE_RSSI, "enable", both_value_list},
    {TYPE_RSSI, "debug", both_value_list},
    {TYPE_PPPC, "enable", both_value_list},
};

struct name_int_value_pair name_int_value_config[] = {
    {TYPE_BSS, "mu-user", 1, 16},
    {TYPE_BSS, "bw_change", 0, 2},
    {TYPE_BSS, "current-ac", 1, 4},
    {TYPE_MACPHY, "gid", 0, 63},
    {TYPE_MACPHY, "txopduration", 0, 127},
    {TYPE_SU, "mpdu-type", 0, 2},
    {TYPE_SU, "mpdu-subtype", 0, 15},
    {TYPE_MU, "mpdu-type", 0, 2},
    {TYPE_MU, "mpdu-subtype", 0, 15},
    {TYPE_MUINFO, "mu-type", 0, 3},
    {TYPE_MUINFO, "mu-ack", 0, 4},
    {TYPE_PTK, "wep-index", 0, 3},
    {TYPE_SU, "mpdu-ackpolicy", 0, 3},
    {TYPE_MU, "mpdu-ackpolicy", 0, 3},
    {TYPE_SU, "non-qos-ack", 0, 1},
    {TYPE_MU, "non-qos-ack", 0, 1},
    {TYPE_SU, "mpdu-fc-flags", 0, 255},
    {TYPE_MU, "mpdu-fc-flags", 0, 255},
    {TYPE_MU, "ru-pwr-factor", 0, 1000},
    {TYPE_SU, "pow_split_en", 0, 7},
    {TYPE_MU, "pow_split_en", 0, 7},
    {TYPE_RSSI, "max-num", 1, 10000},
    {TYPE_MACPHY, "tx-power-level", -128, 127},
    {TYPE_MACPHY, "prot-tx-power", -128, 127},
};
#endif

