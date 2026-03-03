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

#include <linux/module.h>
#include <linux/rtnetlink.h>

#include "cls_wifi_defs.h"
#include "cls_wifi_tx.h"
#include "hal_desc.h"
#include "cls_wifi_cfgfile.h"
#include "cls_wifi_compat.h"

#define COMMON_PARAM(name, default_softmac, default_fullmac)	\
	.name = default_fullmac,
#define SOFTMAC_PARAM(name, default)
#define FULLMAC_PARAM(name, default) .name = default,

struct cls_wifi_mod_params cls_wifi_mod_params = {
	/* common parameters */
	COMMON_PARAM(ht_on, true, true)
	COMMON_PARAM(vht_on, true, true)
	COMMON_PARAM(he_on, true, true)
	COMMON_PARAM(mcs_map, IEEE80211_VHT_MCS_SUPPORT_0_9, IEEE80211_VHT_MCS_SUPPORT_0_9)
	COMMON_PARAM(he_mcs_map, IEEE80211_HE_MCS_SUPPORT_0_11, IEEE80211_HE_MCS_SUPPORT_0_11)
	COMMON_PARAM(he_ul_on, false, false)
	COMMON_PARAM(he_dl_on, false, false)
	COMMON_PARAM(ldpc_on, true, true)
	COMMON_PARAM(stbc_on, true, true)
	COMMON_PARAM(gf_rx_on, true, true)
	COMMON_PARAM(phy_cfg, 0, 0)
	COMMON_PARAM(uapsd_timeout, 300, 300)
	COMMON_PARAM(ap_uapsd_on, true, true)
	COMMON_PARAM(sgi, true, true)
	COMMON_PARAM(sgi80, true, true)
	COMMON_PARAM(use_2040, 1, 1)
	COMMON_PARAM(nss, 2, 2)
	COMMON_PARAM(amsdu_rx_max, 2, 2)
	COMMON_PARAM(bfmee, true, true)
	COMMON_PARAM(bfmer, true, true)
	COMMON_PARAM(mesh, true, true)
	COMMON_PARAM(murx, true, true)
	COMMON_PARAM(mutx, true, true)
	COMMON_PARAM(mutx_on, true, true)
	COMMON_PARAM(use_80, true, true)
	COMMON_PARAM(use_160, true, true)
	COMMON_PARAM(custregd, false, false)
	COMMON_PARAM(custchan, false, false)
	COMMON_PARAM(roc_dur_max, 500, 500)
	COMMON_PARAM(listen_itv, 0, 0)
	COMMON_PARAM(listen_bcmc, true, true)
	COMMON_PARAM(lp_clk_ppm, 20, 20)
	COMMON_PARAM(ps_on, true, true)
	COMMON_PARAM(tx_lft, CLS_WIFI_TX_LIFETIME_MS, CLS_WIFI_TX_LIFETIME_MS)
	COMMON_PARAM(amsdu_maxnb, CLS_TX_PAYLOAD_MAX, CLS_TX_PAYLOAD_MAX)
	COMMON_PARAM(amsdu_agg_timeout, 1000, 1000)
	// By default, only enable UAPSD for Voice queue (see IEEE80211_DEFAULT_UAPSD_QUEUE comment)
	COMMON_PARAM(uapsd_queues, IEEE80211_WMM_IE_STA_QOSINFO_AC_VO, IEEE80211_WMM_IE_STA_QOSINFO_AC_VO)
	COMMON_PARAM(tdls, true, true)
	COMMON_PARAM(uf, true, true)
	COMMON_PARAM(ftl, "", "")
	COMMON_PARAM(dpsm, true, true)
	COMMON_PARAM(tx_to_bk, 0, 0)
	COMMON_PARAM(tx_to_be, 0, 0)
	COMMON_PARAM(tx_to_vi, 0, 0)
	COMMON_PARAM(tx_to_vo, 0, 0)
	COMMON_PARAM(amsdu_tx, 0, 0)

	/* SOFTMAC only parameters */
	SOFTMAC_PARAM(mfp_on, true)
	SOFTMAC_PARAM(agg_tx, true)

	/* FULLMAC only parameters */
	FULLMAC_PARAM(ant_div, false)
	FULLMAC_PARAM(amsdu_require_spp, false)

	COMMON_PARAM(debug_mode, 0, 0)
	COMMON_PARAM(afe_enable, true, true)
	COMMON_PARAM(boot_cali_enable, true, true)
	COMMON_PARAM(ba_buffer_size, 0, 0)
	COMMON_PARAM(density, 4, 4)

	COMMON_PARAM(xtal_ctrim_set, 0, 0)
	COMMON_PARAM(dfs_enable, true, true)

	COMMON_PARAM(bw_2g, 1, 1)
	COMMON_PARAM(bw_5g, 3, 3)
	COMMON_PARAM(chan_ieee_2g, 6, 6)
	COMMON_PARAM(chan_ieee_5g, 36, 36)

	COMMON_PARAM(ant_mask, 0x3, 0x3)
	COMMON_PARAM(afe_dig_ctrl, 1, 1)
	COMMON_PARAM(afe_fem_en, 1, 1)
	COMMON_PARAM(afe_feat_mask, 0xffffffff, 0xffffffff)
	COMMON_PARAM(ant_num_2g, 1, 1)
	COMMON_PARAM(ant_num_5g, 1, 1)
	COMMON_PARAM(dpd_tx_power_5g, 19, 19)
	COMMON_PARAM(dpd_tx_power_2g_20, 22, 22)
	COMMON_PARAM(dpd_tx_power_2g_40, 21, 21)

	COMMON_PARAM(dpd_tx_nss, 1, 1)
	COMMON_PARAM(dpd_online_en, 0, 0)
	COMMON_PARAM(dpd_disable_cca, 0, 0)
	COMMON_PARAM(heartbeat_en, 0, 0)
	COMMON_PARAM(heartbeat_skip, 10, 10)
	COMMON_PARAM(heartbeat_cnt, 30, 30)
	COMMON_PARAM(heartbeat_recover, 2, 2)
	COMMON_PARAM(debug_pci, 1, 1)
	COMMON_PARAM(gain_tcomp_en, 1, 1)
	COMMON_PARAM(zif_online_en, 1, 1)
	COMMON_PARAM(txq_max_len, 0, 0)
	COMMON_PARAM(txq_init_credits, 0, 0)
	COMMON_PARAM(irq_force_exit_time, 1000, 1000)
	COMMON_PARAM(rd_agc_war_en, false, false)
	COMMON_PARAM(low_pwr_en, false, false)
	COMMON_PARAM(cal_path_cs8862, "/mnt/ubifs_data/", "/mnt/ubifs_data/")
	COMMON_PARAM(cal_path_cs8662, "/mnt/ubifs_data/m2k/", "/mnt/ubifs_data/m2k/")
	COMMON_PARAM(cal_path_m3k, "/mnt/ubifs_data/m3k/", "/mnt/ubifs_data/m3k/")
	COMMON_PARAM(irf_path_cs8862, "/root/irf/", "/root/irf/")
	COMMON_PARAM(irf_path_cs8662, "/root/irf/m2k/", "/root/irf/m2k/")
	COMMON_PARAM(irf_path_m3k, "/root/irf/m3k/", "/root/irf/m3k/")
	COMMON_PARAM(high_temp_op, false, false)
	COMMON_PARAM(load_rx_cali_tbl, 0, 0)
	COMMON_PARAM(ibex_en, 0, 0)
	COMMON_PARAM(dynamic_vlan, CLS_WIFI_DYN_VLAN_PER_RADIO, CLS_WIFI_DYN_VLAN_PER_RADIO)
};

EXPORT_SYMBOL(cls_wifi_mod_params);

/* FULLMAC specific parameters*/
module_param_named(ant_div, cls_wifi_mod_params.ant_div, bool, S_IRUGO);
MODULE_PARM_DESC(ant_div, "Enable Antenna Diversity (Default: 0)");
module_param_named(amsdu_require_spp, cls_wifi_mod_params.amsdu_require_spp, bool, S_IRUGO);
MODULE_PARM_DESC(amsdu_require_spp, "Require usage of SPP A-MSDU (Default: 0)");

module_param_named(amsdu_tx, cls_wifi_mod_params.amsdu_tx, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(amsdu_tx, "Use A-MSDU in TX: 0-if advertised, 1-yes, 2-no (Default: 0)");

module_param_named(ht_on, cls_wifi_mod_params.ht_on, bool, S_IRUGO);
MODULE_PARM_DESC(ht_on, "Enable HT (Default: 1)");

module_param_named(vht_on, cls_wifi_mod_params.vht_on, bool, S_IRUGO);
MODULE_PARM_DESC(vht_on, "Enable VHT (Default: 1)");

module_param_named(he_on, cls_wifi_mod_params.he_on, bool, S_IRUGO);
MODULE_PARM_DESC(he_on, "Enable HE (Default: 1)");

module_param_named(mcs_map, cls_wifi_mod_params.mcs_map, int, S_IRUGO);
MODULE_PARM_DESC(mcs_map,  "VHT MCS map value  0: MCS0_7, 1: MCS0_8, 2: MCS0_9"
				 " (Default: 2)");

module_param_named(he_mcs_map, cls_wifi_mod_params.he_mcs_map, int, S_IRUGO);
MODULE_PARM_DESC(he_mcs_map,  "HE MCS map value  0: MCS0_7, 1: MCS0_9, 2: MCS0_11"
				 " (Default: 2)");

module_param_named(he_ul_on, cls_wifi_mod_params.he_ul_on, bool, S_IRUGO);
MODULE_PARM_DESC(he_ul_on, "Enable HE OFDMA UL (Default: 0)");

module_param_named(he_dl_on, cls_wifi_mod_params.he_dl_on, bool, S_IRUGO);
MODULE_PARM_DESC(he_dl_on, "Enable HE OFDMA DL (Default: 0)");

module_param_named(amsdu_maxnb, cls_wifi_mod_params.amsdu_maxnb, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(amsdu_maxnb, "Maximum number of MSDUs inside an A-MSDU in TX: (Default: CLS_TX_PAYLOAD_MAX)");

module_param_named(amsdu_agg_timeout, cls_wifi_mod_params.amsdu_agg_timeout, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(amsdu_agg_timeout, "amsdu agg timeout in us (default: 1000us)");

module_param_named(ps_on, cls_wifi_mod_params.ps_on, bool, S_IRUGO);
MODULE_PARM_DESC(ps_on, "Enable PowerSaving (Default: 1-Enabled)");

module_param_named(tx_lft, cls_wifi_mod_params.tx_lft, int, 0644);
MODULE_PARM_DESC(tx_lft, "Tx lifetime (ms) - setting it to 0 disables retries "
				 "(Default: "__stringify(CLS_WIFI_TX_LIFETIME_MS)")");

module_param_named(ldpc_on, cls_wifi_mod_params.ldpc_on, bool, S_IRUGO);
MODULE_PARM_DESC(ldpc_on, "Enable LDPC (Default: 1)");

module_param_named(stbc_on, cls_wifi_mod_params.stbc_on, bool, S_IRUGO);
MODULE_PARM_DESC(stbc_on, "Enable STBC in RX (Default: 1)");

module_param_named(gf_rx_on, cls_wifi_mod_params.gf_rx_on, bool, S_IRUGO);
MODULE_PARM_DESC(gf_rx_on, "Enable HT greenfield in reception (Default: 1)");

module_param_named(phycfg, cls_wifi_mod_params.phy_cfg, int, S_IRUGO);
MODULE_PARM_DESC(phycfg, "Main RF Path (Default: 0)");

module_param_named(uapsd_timeout, cls_wifi_mod_params.uapsd_timeout, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(uapsd_timeout,
				 "UAPSD Timer timeout, in ms (Default: 300). If 0, UAPSD is disabled");

module_param_named(uapsd_queues, cls_wifi_mod_params.uapsd_queues, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(uapsd_queues, "UAPSD Queues, integer value, must be seen as a bitfield\n"
				 "		Bit 0 = VO\n"
				 "		Bit 1 = VI\n"
				 "		Bit 2 = BK\n"
				 "		Bit 3 = BE\n"
				 "	 -> uapsd_queues=7 will enable uapsd for VO, VI and BK queues");

module_param_named(ap_uapsd_on, cls_wifi_mod_params.ap_uapsd_on, bool, S_IRUGO);
MODULE_PARM_DESC(ap_uapsd_on, "Enable UAPSD in AP mode (Default: 1)");

module_param_named(sgi, cls_wifi_mod_params.sgi, bool, S_IRUGO);
MODULE_PARM_DESC(sgi, "Advertise Short Guard Interval support (Default: 1)");

module_param_named(sgi80, cls_wifi_mod_params.sgi80, bool, S_IRUGO);
MODULE_PARM_DESC(sgi80, "Advertise Short Guard Interval support for 80MHz (Default: 1)");

module_param_named(use_2040, cls_wifi_mod_params.use_2040, bool, S_IRUGO);
MODULE_PARM_DESC(use_2040, "Enable 40MHz (Default: 1)");

module_param_named(use_80, cls_wifi_mod_params.use_80, bool, S_IRUGO);
MODULE_PARM_DESC(use_80, "Enable 80MHz (Default: 1)");

module_param_named(use_160, cls_wifi_mod_params.use_160, bool, S_IRUGO);
MODULE_PARM_DESC(use_160, "Enable 160MHz (Default: 1)");

module_param_named(custregd, cls_wifi_mod_params.custregd, bool, S_IRUGO);
MODULE_PARM_DESC(custregd,
				 "Use permissive custom regulatory rules (for testing ONLY) (Default: 0)");

module_param_named(custchan, cls_wifi_mod_params.custchan, bool, S_IRUGO);
MODULE_PARM_DESC(custchan,
				 "Extend channel set to non-standard channels (for testing ONLY) (Default: 0)");

module_param_named(nss, cls_wifi_mod_params.nss, int, S_IRUGO);
MODULE_PARM_DESC(nss, "1 <= nss <= 2 : Supported number of Spatial Streams (Default: 2)");

module_param_named(amsdu_rx_max, cls_wifi_mod_params.amsdu_rx_max, int, S_IRUGO);
MODULE_PARM_DESC(amsdu_rx_max, "0 <= amsdu_rx_max <= 2 : Maximum A-MSDU size supported in RX\n"
				 "		0: 3895 bytes\n"
				 "		1: 7991 bytes\n"
				 "		2: 11454 bytes\n"
				 "		This value might be reduced according to the FW capabilities.\n"
				 "		Default: 2");

module_param_named(bfmee, cls_wifi_mod_params.bfmee, bool, S_IRUGO);
MODULE_PARM_DESC(bfmee, "Enable Beamformee Capability (Default: 1-Enabled)");

module_param_named(bfmer, cls_wifi_mod_params.bfmer, bool, S_IRUGO);
MODULE_PARM_DESC(bfmer, "Enable Beamformer Capability (Default: 1-Enabled)");

module_param_named(mesh, cls_wifi_mod_params.mesh, bool, S_IRUGO);
MODULE_PARM_DESC(mesh, "Enable Meshing Capability (Default: 1-Enabled)");

module_param_named(murx, cls_wifi_mod_params.murx, bool, S_IRUGO);
MODULE_PARM_DESC(murx, "Enable MU-MIMO RX Capability (Default: 1-Enabled)");

module_param_named(mutx, cls_wifi_mod_params.mutx, bool, S_IRUGO);
MODULE_PARM_DESC(mutx, "Enable MU-MIMO TX Capability (Default: 1-Enabled)");

module_param_named(mutx_on, cls_wifi_mod_params.mutx_on, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(mutx_on, "Enable MU-MIMO transmissions (Default: 1-Enabled)");

module_param_named(roc_dur_max, cls_wifi_mod_params.roc_dur_max, int, S_IRUGO);
MODULE_PARM_DESC(roc_dur_max, "Maximum Remain on Channel duration");

module_param_named(listen_itv, cls_wifi_mod_params.listen_itv, int, S_IRUGO);
MODULE_PARM_DESC(listen_itv, "Maximum listen interval");

module_param_named(listen_bcmc, cls_wifi_mod_params.listen_bcmc, bool, S_IRUGO);
MODULE_PARM_DESC(listen_bcmc, "Wait for BC/MC traffic following DTIM beacon");

module_param_named(lp_clk_ppm, cls_wifi_mod_params.lp_clk_ppm, int, S_IRUGO);
MODULE_PARM_DESC(lp_clk_ppm, "Low Power Clock accuracy of the local device");

module_param_named(tdls, cls_wifi_mod_params.tdls, bool, S_IRUGO);
MODULE_PARM_DESC(tdls, "Enable TDLS (Default: 1-Enabled)");

module_param_named(uf, cls_wifi_mod_params.uf, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(uf, "Enable Unsupported HT Frame Logging (Default: 1-Enabled)");

module_param_named(ftl, cls_wifi_mod_params.ftl, charp, S_IRUGO);
MODULE_PARM_DESC(ftl, "Firmware trace level  (Default: \"\")");

module_param_named(dpsm, cls_wifi_mod_params.dpsm, bool, S_IRUGO);
MODULE_PARM_DESC(dpsm, "Enable Dynamic PowerSaving (Default: 1-Enabled)");

module_param_named(tx_to_bk, cls_wifi_mod_params.tx_to_bk, int, S_IRUGO);
MODULE_PARM_DESC(tx_to_bk,
	 "TX timeout for BK, in ms (Default: 0, Max: 65535). If 0, default value is applied");

module_param_named(tx_to_be, cls_wifi_mod_params.tx_to_be, int, S_IRUGO);
MODULE_PARM_DESC(tx_to_be,
	 "TX timeout for BE, in ms (Default: 0, Max: 65535). If 0, default value is applied");

module_param_named(tx_to_vi, cls_wifi_mod_params.tx_to_vi, int, S_IRUGO);
MODULE_PARM_DESC(tx_to_vi,
	 "TX timeout for VI, in ms (Default: 0, Max: 65535). If 0, default value is applied");

module_param_named(tx_to_vo, cls_wifi_mod_params.tx_to_vo, int, S_IRUGO);
MODULE_PARM_DESC(tx_to_vo,
	 "TX timeout for VO, in ms (Default: 0, Max: 65535). If 0, default value is applied");

module_param_named(debug_mode, cls_wifi_mod_params.debug_mode, int, S_IRUGO);
MODULE_PARM_DESC(debug_mode,
	 "Debug mode, default 0");

module_param_named(afe_enable, cls_wifi_mod_params.afe_enable, bool, S_IRUGO);
MODULE_PARM_DESC(afe_enable, "Enable AFE (Default: 1)");

module_param_named(dfs_enable, cls_wifi_mod_params.dfs_enable, bool, S_IRUGO);
MODULE_PARM_DESC(dfs_enable, "Enable DFS (Default: 1)");

module_param_named(boot_cali_enable, cls_wifi_mod_params.boot_cali_enable, bool, S_IRUGO);
MODULE_PARM_DESC(boot_cali_enable, "Enable DIF boot cali(Default: 1)");

module_param_named(ba_buffer_size, cls_wifi_mod_params.ba_buffer_size, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(ba_buffer_size, "BA buffer size, 0: 64, 1: 128, 2: 256");

module_param_named(density, cls_wifi_mod_params.density, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(density, "density, support 1/2/4/8/16, default 4us");

module_param_named(xtal_ctrim_set, cls_wifi_mod_params.xtal_ctrim_set, bool, S_IRUGO);
MODULE_PARM_DESC(xtal_ctrim_set, "xtal_ctrim_set (Default: 0)");

module_param_named(bw_2g, cls_wifi_mod_params.bw_2g, int, S_IRUGO);
MODULE_PARM_DESC(bw_2g, "bw_2g (Default: 1)");

module_param_named(bw_5g, cls_wifi_mod_params.bw_5g, int, S_IRUGO);
MODULE_PARM_DESC(bw_5g, "bw_5g (Default: 3)");

module_param_named(chan_ieee_2g, cls_wifi_mod_params.chan_ieee_2g, int, S_IRUGO);
MODULE_PARM_DESC(chan_ieee_2g, "chan_ieee_2g (Default: 6)");

module_param_named(chan_ieee_5g, cls_wifi_mod_params.chan_ieee_5g, int, S_IRUGO);
MODULE_PARM_DESC(chan_ieee_5g, "chan_ieee_5g (Default: 36)");

module_param_named(ant_mask, cls_wifi_mod_params.ant_mask, int, S_IRUGO);
MODULE_PARM_DESC(ant_mask, "ant_mask (Default: 3)");

module_param_named(afe_dig_ctrl, cls_wifi_mod_params.afe_dig_ctrl, int, S_IRUGO);
MODULE_PARM_DESC(afe_dig_ctrl, "afe_dig_ctrl (Default: 1)");

module_param_named(afe_fem_en, cls_wifi_mod_params.afe_fem_en, int, S_IRUGO);
MODULE_PARM_DESC(afe_fem_en, "afe_fem_en (Default: 1)");

module_param_named(afe_feat_mask, cls_wifi_mod_params.afe_feat_mask, int, S_IRUGO);
MODULE_PARM_DESC(afe_feat_mask, "afe_feat_mask (Default: 0xffffffff)");

module_param_named(ant_num_2g, cls_wifi_mod_params.ant_num_2g, int, 0644);
MODULE_PARM_DESC(ant_num_2g, "ant_num_2g (Default: 1)");

module_param_named(ant_num_5g, cls_wifi_mod_params.ant_num_5g, int, 0644);
MODULE_PARM_DESC(ant_num_5g, "ant_num_5g (Default: 1)");

module_param_named(dpd_tx_power_5g, cls_wifi_mod_params.dpd_tx_power_5g, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(dpd_tx_power_5g, "dpd_tx_power_5g (Default: 19)");

module_param_named(dpd_tx_power_2g_20, cls_wifi_mod_params.dpd_tx_power_2g_20, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(dpd_tx_power_2g_20, "dpd_tx_power_2g_20 (Default: 22)");

module_param_named(dpd_tx_power_2g_40, cls_wifi_mod_params.dpd_tx_power_2g_40, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(dpd_tx_power_2g_40, "dpd_tx_power_2g_40 (Default: 21)");

module_param_named(dpd_tx_nss, cls_wifi_mod_params.dpd_tx_nss, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(dpd_tx_nss, "dpd_tx_nss (Default: 1)");

module_param_named(dpd_online_en, cls_wifi_mod_params.dpd_online_en, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(dpd_online_en, "dpd_online_en (Default: 1)");

module_param_named(dpd_disable_cca, cls_wifi_mod_params.dpd_disable_cca, bool, 0644);
MODULE_PARM_DESC(dpd_disable_cca, "dpd_disable_cca (Default: 0)");

module_param_named(heartbeat_en, cls_wifi_mod_params.heartbeat_en, bool, 0644);
MODULE_PARM_DESC(heartbeat_en, "heartbeat_en (Default: 1)");

module_param_named(heartbeat_skip, cls_wifi_mod_params.heartbeat_skip, int, 0644);
MODULE_PARM_DESC(heartbeat_skip, "heartbeat_skip (Default: 10)");

module_param_named(debug_pci, cls_wifi_mod_params.debug_pci, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug_pci, "debug_pci (Default: 1)");

module_param_named(heartbeat_cnt, cls_wifi_mod_params.heartbeat_cnt, int, 0644);
MODULE_PARM_DESC(heartbeat_cnt, "heartbeat_cnt (Default: 30)");

module_param_named(heartbeat_recover, cls_wifi_mod_params.heartbeat_recover, int, 0644);
MODULE_PARM_DESC(heartbeat_recover, "heartbeat_recover (Default: 2)");

module_param_named(gain_tcomp_en, cls_wifi_mod_params.gain_tcomp_en, bool, 0644);
MODULE_PARM_DESC(gain_tcomp_en, "gain_tcomp_en (Default: 1)");

module_param_named(zif_online_en, cls_wifi_mod_params.zif_online_en, bool, 0644);
MODULE_PARM_DESC(zif_online_en, "zif_online_en (Default: 1)");

module_param_named(txq_max_len, cls_wifi_mod_params.txq_max_len, int, 0644);
MODULE_PARM_DESC(txq_max_len, "txq_max_len (Default: 2048)");

module_param_named(txq_init_credits, cls_wifi_mod_params.txq_init_credits, int, 0644);
MODULE_PARM_DESC(txq_init_credits, "txq init credit");

module_param_named(irq_force_exit_time, cls_wifi_mod_params.irq_force_exit_time, int, 0644);
MODULE_PARM_DESC(irq_force_exit_time, "max irq_task time per iteration (Default: 1000 ms)");

module_param_named(rd_agc_war_en, cls_wifi_mod_params.rd_agc_war_en, bool, 0644);
MODULE_PARM_DESC(rd_agc_war_en, "radar detection war (Default: 0)");

module_param_named(low_pwr_en, cls_wifi_mod_params.low_pwr_en, bool, 0644);
MODULE_PARM_DESC(low_pwr_en, "low_pwr_en (Default: 0)");

module_param_named(cal_path_cs8862, cls_wifi_mod_params.cal_path_cs8862, charp, 0644);
MODULE_PARM_DESC(cal_path_cs8862, "Cal path for cs8862(Default: \"/mnt/ubifs_data/\")");

module_param_named(cal_path_cs8662, cls_wifi_mod_params.cal_path_cs8662, charp, 0644);
MODULE_PARM_DESC(cal_path_cs8662, "Cal path for cs8662(Default: \"/mnt/ubifs_data/m2k/\")");

module_param_named(cal_path_m3k, cls_wifi_mod_params.cal_path_m3k, charp, 0644);
MODULE_PARM_DESC(cal_path_m3k, "Cal path for m3k(Default: \"/mnt/ubifs_data/m3k/\")");

module_param_named(irf_path_cs8862, cls_wifi_mod_params.irf_path_cs8862, charp, 0644);
MODULE_PARM_DESC(irf_path_cs8862, "Irf path for cs8862(Default: \"/root/irf/\")");

module_param_named(irf_path_cs8662, cls_wifi_mod_params.irf_path_cs8662, charp, 0644);
MODULE_PARM_DESC(irf_path_cs8662, "Irf path for cs8662(Default: \"/root/irf/m2k/\")");

module_param_named(irf_path_m3k, cls_wifi_mod_params.irf_path_m3k, charp, 0644);
MODULE_PARM_DESC(irf_path_m3k, "Irf path for m3k(Default: \"/root/irf/m3k/\")");

module_param_named(high_temp_op, cls_wifi_mod_params.high_temp_op, bool, 0644);
MODULE_PARM_DESC(high_temp_op, "high temp op(Default: 0)");

module_param_named(load_rx_cali_tbl, cls_wifi_mod_params.load_rx_cali_tbl, bool, 0644);
MODULE_PARM_DESC(load_rx_cali_tbl, "load_rx_cali_tbl (Default: 0)");

module_param_named(ibex_en, cls_wifi_mod_params.ibex_en, bool, 0644);
MODULE_PARM_DESC(ibex_en, "ibex_en (Default: 0)");

module_param_named(dynamic_vlan, cls_wifi_mod_params.dynamic_vlan, int, 0644);
MODULE_PARM_DESC(dynamic_vlan, "dynamic_vlan: 0 - disable, 1 - per_radio. 2 - per vif, 3 - globle (Default: 1)");

/* Regulatory rules */
static struct ieee80211_regdomain cls_wifi_regdom = {
	.n_reg_rules = 2,
	.alpha2 = "99",
	.reg_rules = {
		REG_RULE(2390 - 10, 2510 + 10, 40, 0, 1000, 0),
		REG_RULE(5150 - 10, 5970 + 10, 80, 0, 1000, 0),
	}
};

static const int mcs_map_to_rate[4][3] = {
	[PHY_CHNL_BW_20][IEEE80211_VHT_MCS_SUPPORT_0_7] = 65,
	[PHY_CHNL_BW_20][IEEE80211_VHT_MCS_SUPPORT_0_8] = 78,
	[PHY_CHNL_BW_20][IEEE80211_VHT_MCS_SUPPORT_0_9] = 78,
	[PHY_CHNL_BW_40][IEEE80211_VHT_MCS_SUPPORT_0_7] = 135,
	[PHY_CHNL_BW_40][IEEE80211_VHT_MCS_SUPPORT_0_8] = 162,
	[PHY_CHNL_BW_40][IEEE80211_VHT_MCS_SUPPORT_0_9] = 180,
	[PHY_CHNL_BW_80][IEEE80211_VHT_MCS_SUPPORT_0_7] = 292,
	[PHY_CHNL_BW_80][IEEE80211_VHT_MCS_SUPPORT_0_8] = 351,
	[PHY_CHNL_BW_80][IEEE80211_VHT_MCS_SUPPORT_0_9] = 390,
	[PHY_CHNL_BW_160][IEEE80211_VHT_MCS_SUPPORT_0_7] = 585,
	[PHY_CHNL_BW_160][IEEE80211_VHT_MCS_SUPPORT_0_8] = 702,
	[PHY_CHNL_BW_160][IEEE80211_VHT_MCS_SUPPORT_0_9] = 780,
};

#define MAX_VHT_RATE(map, nss, bw) (mcs_map_to_rate[bw][map] * (nss))

/**
 * Do some sanity check
 *
 */
static int cls_wifi_check_fw_hw_feature(struct cls_wifi_hw *cls_wifi_hw,
									struct wiphy *wiphy)
{
	u32_l sys_feat = cls_wifi_hw->version_cfm.features;
	u32_l mac_feat = cls_wifi_hw->version_cfm.version_machw_1;
	u32_l phy_feat = cls_wifi_hw->version_cfm.version_phy_1;
	u32_l phy_vers = cls_wifi_hw->version_cfm.version_phy_2;
	u16_l max_sta_nb = cls_wifi_hw->version_cfm.max_sta_nb;
	u8_l max_vif_nb = cls_wifi_hw->version_cfm.max_vif_nb;
	int bw, res = 0;
	int amsdu_rx;

	if (!cls_wifi_hw->radio_params->custregd)
		cls_wifi_hw->radio_params->custchan = false;

	if (cls_wifi_hw->radio_params->custchan) {
		cls_wifi_hw->radio_params->mesh = false;
		cls_wifi_hw->radio_params->tdls = false;
	}

	if (!(sys_feat & BIT(MM_FEAT_UMAC_BIT))) {
		wiphy_err(wiphy,
				  "Loading softmac firmware with fullmac driver\n");
		res = -1;
	}

	if (!(sys_feat & BIT(MM_FEAT_ANT_DIV_BIT))) {
		cls_wifi_hw->radio_params->ant_div = false;
	}

	if (!(sys_feat & BIT(MM_FEAT_VHT_BIT))) {
		cls_wifi_hw->radio_params->vht_on = false;
	}

	// Check if HE is supported
	if (!(sys_feat & BIT(MM_FEAT_HE_BIT))) {
		cls_wifi_hw->radio_params->he_on = false;
		cls_wifi_hw->radio_params->he_ul_on = false;
		cls_wifi_hw->radio_params->he_dl_on = false;
	}

	if (!(sys_feat & BIT(MM_FEAT_PS_BIT))) {
		cls_wifi_hw->radio_params->ps_on = false;
	}

	/* AMSDU (non)support implies different shared structure definition
	   so insure that fw and drv have consistent compilation option */
	if (sys_feat & BIT(MM_FEAT_AMSDU_BIT)) {
#ifndef CONFIG_CLS_WIFI_SPLIT_TX_BUF
		wiphy_err(wiphy,
				  "AMSDU enabled in firmware but support not compiled in driver\n");
		res = -1;
#else
		/* Adjust amsdu_maxnb so that it stays in allowed bounds */
		cls_wifi_adjust_amsdu_maxnb(cls_wifi_hw);
#endif /* CONFIG_CLS_WIFI_SPLIT_TX_BUF */
	} else {
#ifdef CONFIG_CLS_WIFI_SPLIT_TX_BUF
		wiphy_err(wiphy,
				  "AMSDU disabled in firmware but support compiled in driver\n");
		res = -1;
#endif /* CONFIG_CLS_WIFI_SPLIT_TX_BUF */
	}

	if (!(sys_feat & BIT(MM_FEAT_UAPSD_BIT))) {
		cls_wifi_hw->radio_params->uapsd_timeout = 0;
	}

	if (!(sys_feat & BIT(MM_FEAT_BFMEE_BIT))) {
		cls_wifi_hw->radio_params->bfmee = false;
	}

	if ((sys_feat & BIT(MM_FEAT_BFMER_BIT))) {
#ifndef CONFIG_CLS_WIFI_BFMER
		wiphy_err(wiphy,
				  "BFMER enabled in firmware but support not compiled in driver\n");
		res = -1;
#endif /* CONFIG_CLS_WIFI_BFMER */
		// Check PHY and MAC HW BFMER support and update parameter accordingly
		if (!(phy_feat & MDM_BFMER_BIT) || !(mac_feat & CLS_WIFI_MAC_BFMER_BIT)) {
			cls_wifi_hw->radio_params->bfmer = false;
			// Disable the feature in the bitfield so that it won't be displayed
			sys_feat &= ~BIT(MM_FEAT_BFMER_BIT);
		}
	} else {
#ifdef CONFIG_CLS_WIFI_BFMER
		wiphy_err(wiphy,
				  "BFMER disabled in firmware but support compiled in driver\n");
		res = -1;
#else
		cls_wifi_hw->radio_params->bfmer = false;
#endif /* CONFIG_CLS_WIFI_BFMER */
	}

	if (!(sys_feat & BIT(MM_FEAT_MESH_BIT))) {
		cls_wifi_hw->radio_params->mesh = false;
	}

	if (!(sys_feat & BIT(MM_FEAT_TDLS_BIT))) {
		cls_wifi_hw->radio_params->tdls = false;
	}

	if (!(sys_feat & BIT(MM_FEAT_UF_BIT))) {
		cls_wifi_hw->radio_params->uf = false;
	}

	if ((sys_feat & BIT(MM_FEAT_MON_DATA_BIT))) {
#ifndef CONFIG_CLS_WIFI_MON_DATA
		wiphy_err(wiphy,
				  "Monitor+Data interface support (MON_DATA) is enabled in firmware but support not compiled in driver\n");
		res = -1;
#endif /* CONFIG_CLS_WIFI_MON_DATA */
	} else {
#ifdef CONFIG_CLS_WIFI_MON_DATA
		wiphy_err(wiphy,
				  "Monitor+Data interface support (MON_DATA) disabled in firmware but support compiled in driver\n");
		res = -1;
#endif /* CONFIG_CLS_WIFI_MON_DATA */
	}

	// Check supported AMSDU RX size
	amsdu_rx = (sys_feat >> MM_AMSDU_MAX_SIZE_BIT0) & 0x03;
	if (amsdu_rx < cls_wifi_hw->radio_params->amsdu_rx_max)
		cls_wifi_hw->radio_params->amsdu_rx_max = amsdu_rx;

	// Check supported BW
	bw = (phy_feat & MDM_CHBW_MASK) >> MDM_CHBW_LSB;
	// Check if 160MHz BW is supported
	if (bw < PHY_CHNL_BW_160)
		cls_wifi_hw->radio_params->use_160 = false;

	// Check if 80MHz BW is supported
	if (bw < PHY_CHNL_BW_80)
		cls_wifi_hw->radio_params->use_80 = false;

	// Check if 40MHz BW is supported
	if (bw < PHY_CHNL_BW_40)
		cls_wifi_hw->radio_params->use_2040 = false;

	// Disable BW if lower BW are not enabled
	if (!cls_wifi_hw->radio_params->use_2040)
		cls_wifi_hw->radio_params->use_80 = false;

	if (!cls_wifi_hw->radio_params->use_80)
		cls_wifi_hw->radio_params->use_160 = false;

	// Check if HT is supposed to be supported. If not, disable VHT/HE too
	if (!cls_wifi_hw->radio_params->ht_on)
	{
		cls_wifi_hw->radio_params->vht_on = false;
		cls_wifi_hw->radio_params->he_on = false;
		cls_wifi_hw->radio_params->he_ul_on = false;
		cls_wifi_hw->radio_params->he_dl_on = false;
		cls_wifi_hw->radio_params->use_2040 = false;
		cls_wifi_hw->radio_params->use_80 = false;
		cls_wifi_hw->radio_params->use_160 = false;
	}

	// LDPC is mandatory for HE40 and above, so if LDPC is not supported, then disable
	// support for 40MHz and above
	if (cls_wifi_hw->radio_params->he_on && !cls_wifi_hw->radio_params->ldpc_on)
	{
		cls_wifi_hw->radio_params->use_160 = false;
		cls_wifi_hw->radio_params->use_80 = false;
		cls_wifi_hw->radio_params->use_2040 = false;
	}

	// HT greenfield is not supported in modem >= 3.0
	if (__MDM_MAJOR_VERSION(phy_vers) > 0) {
		cls_wifi_hw->radio_params->gf_rx_on = false;
	}

	if (!(sys_feat & BIT(MM_FEAT_MU_MIMO_RX_BIT)) ||
		!cls_wifi_hw->radio_params->bfmee) {
		cls_wifi_hw->radio_params->murx = false;
	}

	if ((sys_feat & BIT(MM_FEAT_MU_TX_BIT))) {
#ifndef CONFIG_CLS_WIFI_HEMU_TX
		wiphy_err(wiphy,
				  "MU TX compiled in firmware but support not compiled in driver\n");
		res = -1;
#endif /* CONFIG_CLS_WIFI_HEMU_TX */
	} else {
#ifdef CONFIG_CLS_WIFI_HEMU_TX
		wiphy_err(wiphy,
				  "MU TX not compiled in firmware but support compiled in driver\n");
		res = -1;
#else
		cls_wifi_hw->radio_params->mutx = false;
#endif /* CONFIG_CLS_WIFI_MUMIMO_TX */
	}

	if (sys_feat & BIT(MM_FEAT_WAPI_BIT)) {
		cls_wifi_enable_wapi(cls_wifi_hw);
	}

	if (sys_feat & BIT(MM_FEAT_MFP_BIT)) {
		cls_wifi_enable_mfp(cls_wifi_hw);
	}

	if (cls_wifi_hw->machw_type >= CLS_WIFI_MACHW_HE_AP) {
		cls_wifi_enable_ccmp_256(cls_wifi_hw);
	}

	if (mac_feat & CLS_WIFI_MAC_GCMP_BIT) {
		cls_wifi_enable_gcmp(cls_wifi_hw);
	}

	if (sys_feat & BIT(MM_FEAT_HW_LLCSNAP_INS_BIT)) {
		cls_wifi_hw->has_hw_llcsnap_insert = false;
	} else {
		cls_wifi_hw->has_hw_llcsnap_insert = false;
	}

#define QUEUE_NAME "Broadcast/Multicast queue "

	if (sys_feat & BIT(MM_FEAT_BCN_BIT)) {
#if CLS_TXQ_CNT == 4
		wiphy_err(wiphy, QUEUE_NAME
				  "enabled in firmware but support not compiled in driver\n");
		res = -1;
#endif /* CLS_TXQ_CNT == 4 */
	} else {
#if CLS_TXQ_CNT == 5
		wiphy_err(wiphy, QUEUE_NAME
				  "disabled in firmware but support compiled in driver\n");
		res = -1;
#endif /* CLS_TXQ_CNT == 5 */
	}
#undef QUEUE_NAME

#ifdef CONFIG_CLS_WIFI_RADAR
	if (sys_feat & BIT(MM_FEAT_RADAR_BIT)) {
		/* Enable combination with radar detection */
		wiphy->n_iface_combinations++;
	}
#endif /* CONFIG_CLS_WIFI_RADAR */

	switch (__MDM_PHYCFG_FROM_VERS(phy_feat)) {
		case MDM_PHY_CONFIG_VERSION:
			{
				int nss_supp = (phy_feat & MDM_NSS_MASK) >> MDM_NSS_LSB;
				if (cls_wifi_hw->radio_params->nss > nss_supp)
					cls_wifi_hw->radio_params->nss = nss_supp;
				if ((cls_wifi_hw->radio_params->phy_cfg < 0) ||
						(cls_wifi_hw->radio_params->phy_cfg > 1))
					cls_wifi_hw->radio_params->phy_cfg = 0;
			}
			break;
		default:
			WARN_ON(1);
			break;
	}

	if ((cls_wifi_hw->radio_params->nss < 1) || (cls_wifi_hw->radio_params->nss > 2))
		cls_wifi_hw->radio_params->nss = 1;

	if ((cls_wifi_hw->radio_params->mcs_map < 0) || (cls_wifi_hw->radio_params->mcs_map > 2))
		cls_wifi_hw->radio_params->mcs_map = 0;

#define PRINT_CLS_WIFI_PHY_FEAT(feat)								   \
	(phy_feat & MDM_##feat##_BIT ? "["#feat"]" : "")

	wiphy_info(wiphy, "PHY features: [NSS=%d][CHBW=%d]%s%s%s%s%s%s%s\n",
			   (phy_feat & MDM_NSS_MASK) >> MDM_NSS_LSB,
			   20 * (1 << ((phy_feat & MDM_CHBW_MASK) >> MDM_CHBW_LSB)),
			   (phy_feat & (MDM_LDPCDEC_BIT | MDM_LDPCENC_BIT)) ==
					   (MDM_LDPCDEC_BIT | MDM_LDPCENC_BIT) ? "[LDPC]" : "",
			   PRINT_CLS_WIFI_PHY_FEAT(VHT),
			   PRINT_CLS_WIFI_PHY_FEAT(HE),
			   PRINT_CLS_WIFI_PHY_FEAT(BFMER),
			   PRINT_CLS_WIFI_PHY_FEAT(BFMEE),
			   PRINT_CLS_WIFI_PHY_FEAT(MUMIMOTX),
			   PRINT_CLS_WIFI_PHY_FEAT(MUMIMORX)
			   );

#define PRINT_CLS_WIFI_FEAT(feat)								   \
	(sys_feat & BIT(MM_FEAT_##feat##_BIT) ? "["#feat"]" : "")

	wiphy_info(wiphy, "FW features: %s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n",
			   PRINT_CLS_WIFI_FEAT(BCN),
			   PRINT_CLS_WIFI_FEAT(RADAR),
			   PRINT_CLS_WIFI_FEAT(PS),
			   PRINT_CLS_WIFI_FEAT(UAPSD),
			   PRINT_CLS_WIFI_FEAT(AMPDU),
			   PRINT_CLS_WIFI_FEAT(AMSDU),
			   PRINT_CLS_WIFI_FEAT(P2P),
			   PRINT_CLS_WIFI_FEAT(P2P_GO),
			   PRINT_CLS_WIFI_FEAT(UMAC),
			   PRINT_CLS_WIFI_FEAT(VHT),
			   PRINT_CLS_WIFI_FEAT(HE),
			   PRINT_CLS_WIFI_FEAT(BFMEE),
			   PRINT_CLS_WIFI_FEAT(BFMER),
			   PRINT_CLS_WIFI_FEAT(WAPI),
			   PRINT_CLS_WIFI_FEAT(MFP),
			   PRINT_CLS_WIFI_FEAT(MU_MIMO_RX),
			   PRINT_CLS_WIFI_FEAT(MU_TX),
			   PRINT_CLS_WIFI_FEAT(MESH),
			   PRINT_CLS_WIFI_FEAT(TDLS),
			   PRINT_CLS_WIFI_FEAT(ANT_DIV),
			   PRINT_CLS_WIFI_FEAT(UF),
			   PRINT_CLS_WIFI_FEAT(TWT),
			   PRINT_CLS_WIFI_FEAT(FTM_INIT),
			   PRINT_CLS_WIFI_FEAT(FAKE_FTM_RSP),
			   PRINT_CLS_WIFI_FEAT(HW_LLCSNAP_INS));
#undef PRINT_CLS_WIFI_FEAT

	if(max_sta_nb != hw_remote_sta_max(cls_wifi_hw))
	{
		wiphy_err(wiphy, "Different number of supported stations between driver and FW (%d != %d)\n",
				  hw_remote_sta_max(cls_wifi_hw), max_sta_nb);
		res = -1;
	}

	if(max_vif_nb != hw_vdev_max(cls_wifi_hw))
	{
		wiphy_err(wiphy, "Different number of supported virtual interfaces between driver and FW (%d != %d)\n",
				  hw_vdev_max(cls_wifi_hw), max_vif_nb);
		res = -1;
	}

	return res;
}

static void cls_wifi_set_vht_capa(struct cls_wifi_hw *cls_wifi_hw, struct wiphy *wiphy)
{
	struct ieee80211_supported_band *sband;
	int i;
	int nss = cls_wifi_hw->radio_params->nss;
	int mcs_map;
	int bw_max;

	if (!(cls_wifi_hw->band_cap & CLS_WIFI_BAND_CAP_5G))
		return;
	//vht is available only for 5G
	sband = &cls_wifi_hw->if_cfg80211.sbands[NL80211_BAND_5GHZ];

	if (!cls_wifi_hw->radio_params->vht_on)
		return;

	sband->vht_cap.vht_supported = true;
	sband->vht_cap.cap = 7 << IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_SHIFT;
	if (cls_wifi_hw->radio_params->sgi80 && cls_wifi_hw->radio_params->use_80)
		sband->vht_cap.cap |= IEEE80211_VHT_CAP_SHORT_GI_80;
	if (cls_wifi_hw->radio_params->stbc_on)
		sband->vht_cap.cap |= IEEE80211_VHT_CAP_RXSTBC_1;
	if (cls_wifi_hw->radio_params->ldpc_on)
		sband->vht_cap.cap |= IEEE80211_VHT_CAP_RXLDPC;
	if (cls_wifi_hw->radio_params->bfmee) {
		sband->vht_cap.cap |= IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE;
		sband->vht_cap.cap |= 3 << IEEE80211_VHT_CAP_BEAMFORMEE_STS_SHIFT;
	}
	if (nss > 1)
		sband->vht_cap.cap |= IEEE80211_VHT_CAP_TXSTBC;
	if (cls_wifi_hw->radio_params->use_160) {
		sband->vht_cap.cap |= IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160MHZ;
		//sband->vht_cap.cap |= IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160_80PLUS80MHZ;
		sband->vht_cap.cap |= IEEE80211_VHT_CAP_SHORT_GI_160;
	}

	// Update the AMSDU max RX size (not shifted as located at offset 0 of the VHT cap)
	sband->vht_cap.cap |= cls_wifi_hw->radio_params->amsdu_rx_max;

	if (cls_wifi_hw->radio_params->bfmer) {
		sband->vht_cap.cap |= IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE;
		/* Set number of sounding dimensions */
		sband->vht_cap.cap |= (nss - 1) << IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_SHIFT;
	}
	if (cls_wifi_hw->radio_params->murx)
		sband->vht_cap.cap |= IEEE80211_VHT_CAP_MU_BEAMFORMEE_CAPABLE;
	if (cls_wifi_hw->radio_params->mutx)
		sband->vht_cap.cap |= IEEE80211_VHT_CAP_MU_BEAMFORMER_CAPABLE;

	// Get max supported BW
	if (cls_wifi_hw->radio_params->use_160)
		bw_max = PHY_CHNL_BW_160;
	else if (cls_wifi_hw->radio_params->use_80)
		bw_max = PHY_CHNL_BW_80;
	else if (cls_wifi_hw->radio_params->use_2040)
		bw_max = PHY_CHNL_BW_40;
	else
		bw_max = PHY_CHNL_BW_20;

	// 1 SS
	if (bw_max == PHY_CHNL_BW_20)
		// VHT spec doesn't allow MCS9 at 20MHz/1SS
		mcs_map = min_t(int, cls_wifi_hw->radio_params->mcs_map,
				IEEE80211_VHT_MCS_SUPPORT_0_8);
	else
		mcs_map = min_t(int, cls_wifi_hw->radio_params->mcs_map,
				IEEE80211_VHT_MCS_SUPPORT_0_9);
	sband->vht_cap.vht_mcs.tx_mcs_map = cpu_to_le16(mcs_map);
	sband->vht_cap.vht_mcs.tx_highest = MAX_VHT_RATE(mcs_map, nss, bw_max);
	sband->vht_cap.vht_mcs.rx_mcs_map = cpu_to_le16(mcs_map);
	sband->vht_cap.vht_mcs.rx_highest = MAX_VHT_RATE(mcs_map, nss, bw_max);

	if (nss > 1) {
		// 2 SS
		// Don't reset mcs_map as limitation on 20MHz is also valid for 2SS.
		// Moreover, For FPGA testing limit MCS for bw >= 80MHz:
		//   - in RX, 2SS, we support up to MCS7
		//   - in TX, 2SS, we support up to MCS8
		if (bw_max >= PHY_CHNL_BW_80)
			mcs_map = min_t(int, mcs_map, IEEE80211_VHT_MCS_SUPPORT_0_9);
		sband->vht_cap.vht_mcs.tx_mcs_map |= cpu_to_le16(mcs_map << 2);
		sband->vht_cap.vht_mcs.tx_highest = MAX_VHT_RATE(mcs_map, nss, bw_max);
		if (bw_max >= PHY_CHNL_BW_80)
			mcs_map = min_t(int, mcs_map, IEEE80211_VHT_MCS_SUPPORT_0_9);
		sband->vht_cap.vht_mcs.rx_mcs_map |= cpu_to_le16(mcs_map << 2);
		sband->vht_cap.vht_mcs.rx_highest = MAX_VHT_RATE(mcs_map, nss, bw_max);
	}

	if (nss > 2) {
		// > 2SS, no more limitation (neither in spec at 20MHz nor in FPGA)
		mcs_map = min_t(int, cls_wifi_hw->radio_params->mcs_map,
				IEEE80211_VHT_MCS_SUPPORT_0_9);
		sband->vht_cap.vht_mcs.tx_highest = MAX_VHT_RATE(mcs_map, nss, bw_max);
		sband->vht_cap.vht_mcs.rx_highest = MAX_VHT_RATE(mcs_map, nss, bw_max);
		for (i = 2 ; i < nss; i++ ) {
			sband->vht_cap.vht_mcs.tx_mcs_map |= cpu_to_le16(mcs_map << (i*2));
			sband->vht_cap.vht_mcs.rx_mcs_map |= cpu_to_le16(mcs_map << (i*2));
		}
	}

	// Unsupported SS
	for (i = nss; i < 8; i++ ) {
		sband->vht_cap.vht_mcs.tx_mcs_map |= cpu_to_le16(IEEE80211_VHT_MCS_NOT_SUPPORTED << (i*2));
		sband->vht_cap.vht_mcs.rx_mcs_map |= cpu_to_le16(IEEE80211_VHT_MCS_NOT_SUPPORTED << (i*2));
	}
}

static void cls_wifi_set_ht_capa(struct cls_wifi_hw *cls_wifi_hw, struct wiphy *wiphy)
{
	struct ieee80211_supported_band *sband;
	enum nl80211_band band;
	int nss = cls_wifi_hw->radio_params->nss;
	int i;

	for(band = NL80211_BAND_2GHZ; band <= NL80211_BAND_5GHZ; band++)
	{
		if (!(cls_wifi_hw->band_cap & (1 << band)))
			continue;
		sband = &cls_wifi_hw->if_cfg80211.sbands[band];

		if (!cls_wifi_hw->radio_params->ht_on) {
			sband->ht_cap.ht_supported = false;
			return;
		}

		if (cls_wifi_hw->radio_params->stbc_on)
			sband->ht_cap.cap |= 1 << IEEE80211_HT_CAP_RX_STBC_SHIFT;
		if (cls_wifi_hw->radio_params->ldpc_on)
			sband->ht_cap.cap |= IEEE80211_HT_CAP_LDPC_CODING;
		if (cls_wifi_hw->radio_params->use_2040) {
			sband->ht_cap.mcs.rx_mask[4] = 0x1; /* MCS32 */
			sband->ht_cap.cap |= IEEE80211_HT_CAP_SUP_WIDTH_20_40;
			sband->ht_cap.mcs.rx_highest = cpu_to_le16(135 * nss);
		} else {
			sband->ht_cap.mcs.rx_highest = cpu_to_le16(65 * nss);
		}
		if (nss > 1)
			sband->ht_cap.cap |= IEEE80211_HT_CAP_TX_STBC;

		// Update the AMSDU max RX size
		if (cls_wifi_hw->radio_params->amsdu_rx_max)
			sband->ht_cap.cap |= IEEE80211_HT_CAP_MAX_AMSDU;

		if (cls_wifi_hw->radio_params->sgi) {
			sband->ht_cap.cap |= IEEE80211_HT_CAP_SGI_20;
			if (cls_wifi_hw->radio_params->use_2040) {
				sband->ht_cap.cap |= IEEE80211_HT_CAP_SGI_40;
				sband->ht_cap.mcs.rx_highest = cpu_to_le16(150 * nss);
			} else
				sband->ht_cap.mcs.rx_highest = cpu_to_le16(72 * nss);
		}
		if (cls_wifi_hw->radio_params->gf_rx_on)
			sband->ht_cap.cap |= IEEE80211_HT_CAP_GRN_FLD;

		for (i = 0; i < nss; i++) {
			sband->ht_cap.mcs.rx_mask[i] = 0xFF;
		}

		/* SITS #2320 */
		switch (cls_wifi_hw->radio_params->density) {
			case 1:
				sband->ht_cap.ampdu_density = IEEE80211_HT_MPDU_DENSITY_1;
				break;
			case 2:
				sband->ht_cap.ampdu_density = IEEE80211_HT_MPDU_DENSITY_2;
				break;
			case 4:
				sband->ht_cap.ampdu_density = IEEE80211_HT_MPDU_DENSITY_4;
				break;
			case 8:
				sband->ht_cap.ampdu_density = IEEE80211_HT_MPDU_DENSITY_8;
				break;
			case 16:
				sband->ht_cap.ampdu_density = IEEE80211_HT_MPDU_DENSITY_16;
				break;
			default:
				sband->ht_cap.ampdu_density = IEEE80211_HT_MPDU_DENSITY_4;
				break;
		}
	}
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 20, 0)
static void cls_wifi_set_ppe_threshold(struct cls_wifi_hw *cls_wifi_hw,
								   struct ieee80211_sta_he_cap *he_cap)
{
	int nss = cls_wifi_hw->radio_params->nss;
	int ru_idx_bmp, ru_idx_cnt;
	int i, index, bit_oft, ppet;

	he_cap->ppe_thres[0] = ((nss - 1) << IEEE80211_PPE_THRES_NSS_POS) &
						   IEEE80211_PPE_THRES_NSS_MASK;
	if (cls_wifi_hw->radio_params->use_160) {
		ru_idx_bmp = 0xf;
		ru_idx_cnt = 4;
	} else if (cls_wifi_hw->radio_params->use_80) {
		ru_idx_bmp = 7;
		ru_idx_cnt = 3;
	} else if (cls_wifi_hw->radio_params->use_2040) {
		ru_idx_bmp = 3;
		ru_idx_cnt = 2;
	} else {
		ru_idx_bmp = 1;
		ru_idx_cnt = 1;
	}
	he_cap->ppe_thres[0] |= (ru_idx_bmp << IEEE80211_PPE_THRES_RU_INDEX_BITMASK_POS) &
							IEEE80211_PPE_THRES_RU_INDEX_BITMASK_MASK;

	bit_oft = 7;
	index = 0;
	// Use same Packet Padding Extension Threshold for all RU/NSS combination:
	// 16us for all constellation greater or equal to BPSK
	ppet = ((0 << 0) |  // PPET16 = BPSK
			(7 << 3));  // PPET8  = None
	for (i = 0; i < ru_idx_cnt * nss; i++) {
		int nb_bit = 8 - bit_oft;
		he_cap->ppe_thres[index] |= (ppet << bit_oft) & 0xFF;
		bit_oft += 6;
		if (bit_oft >= 8) {
			index++;
			if (bit_oft > 8) {
				he_cap->ppe_thres[index] |= (ppet >> nb_bit) & 0xFF;
			}
			bit_oft -= 8;
		}
	}
}

static void cls_wifi_set_sta_he_capa(struct cls_wifi_hw *cls_wifi_hw, struct ieee80211_sta_he_cap *he_cap)
{
	he_cap->he_cap_elem.phy_cap_info[3] |= IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_TX_16_QAM;
	if (cls_wifi_hw->radio_params->nss > 0)
		he_cap->he_cap_elem.phy_cap_info[3] |= IEEE80211_HE_PHY_CAP3_DCM_MAX_TX_NSS_2;
	else
		he_cap->he_cap_elem.phy_cap_info[3] |= IEEE80211_HE_PHY_CAP3_DCM_MAX_TX_NSS_1;

	he_cap->he_cap_elem.phy_cap_info[6] |= IEEE80211_HE_PHY_CAP6_PARTIAL_BANDWIDTH_DL_MUMIMO;
	he_cap->he_cap_elem.phy_cap_info[8] |= IEEE80211_HE_PHY_CAP8_20MHZ_IN_40MHZ_HE_PPDU_IN_2G;

	if (cls_wifi_hw->version_cfm.features & BIT(MM_FEAT_TWT_BIT)) {
		cls_wifi_hw->ext_capa[9] |= WLAN_EXT_CAPA10_TWT_REQUESTER_SUPPORT;
		he_cap->he_cap_elem.mac_cap_info[0] |= IEEE80211_HE_MAC_CAP0_TWT_REQ;
	}
}

static void cls_wifi_set_ap_he_capa(struct cls_wifi_hw *cls_wifi_hw, struct ieee80211_sta_he_cap *he_cap)
{
	if (cls_wifi_hw->radio_params->bfmer)
		he_cap->he_cap_elem.phy_cap_info[4] |= IEEE80211_HE_PHY_CAP4_MU_BEAMFORMER;

#ifdef CONFIG_CMCC_TEST
		cls_wifi_hw->ext_capa[9] |= WLAN_EXT_CAPA10_TWT_RESPONDER_SUPPORT;
		he_cap->he_cap_elem.mac_cap_info[0] |= IEEE80211_HE_MAC_CAP0_TWT_RES;
#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 12, 19)
		he_cap->he_cap_elem.phy_cap_info[7] |= IEEE80211_HE_PHY_CAP7_PSR_BASED_SR;
#else
		he_cap->he_cap_elem.phy_cap_info[7] |= IEEE80211_HE_PHY_CAP7_SRP_BASED_SR;
#endif
#else
	if (cls_wifi_hw->version_cfm.features & BIT(MM_FEAT_TWT_BIT)) {
		cls_wifi_hw->ext_capa[9] |= WLAN_EXT_CAPA10_TWT_RESPONDER_SUPPORT;
		he_cap->he_cap_elem.mac_cap_info[0] |= IEEE80211_HE_MAC_CAP0_TWT_RES;
	}
#endif
}
#endif // LINUX_VERSION_CODE >= 4.20.0

static void cls_wifi_set_he_capa(struct cls_wifi_hw *cls_wifi_hw, struct wiphy *wiphy)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 20, 0)
	struct ieee80211_supported_band *sband;
	enum nl80211_band band;
	int i;
	int nss = cls_wifi_hw->radio_params->nss;
	struct ieee80211_sta_he_cap *he_cap;
	int mcs_map, mcs_map_160, mcs_map_80p80, mcs_map_max_2ss = IEEE80211_HE_MCS_SUPPORT_0_11;
	u8 dcm_max_ru = IEEE80211_HE_PHY_CAP8_DCM_MAX_RU_242;
	u32_l phy_vers = cls_wifi_hw->version_cfm.version_phy_2;

	for(band = NL80211_BAND_2GHZ; band <= NL80211_BAND_5GHZ; band++)
	{
		if (!(cls_wifi_hw->band_cap & (1 << band)))
			continue;
		sband = &cls_wifi_hw->if_cfg80211.sbands[band];
		if (!cls_wifi_hw->radio_params->he_on) {
			sband->iftype_data = NULL;
			sband->n_iftype_data = 0;
			return;
		}

		// First configure Capabilities valid for all types of interfaces
		he_cap = (struct ieee80211_sta_he_cap *)&sband->iftype_data[0].he_cap;
		he_cap->has_he = true;

		he_cap->he_cap_elem.mac_cap_info[0] |= IEEE80211_HE_MAC_CAP0_HTC_HE;
		he_cap->he_cap_elem.mac_cap_info[3] |= IEEE80211_HE_MAC_CAP3_OMI_CONTROL;
		he_cap->he_cap_elem.mac_cap_info[3] |= IEEE80211_HE_MAC_CAP3_MAX_AMPDU_LEN_EXP_VHT_2;
		he_cap->he_cap_elem.mac_cap_info[2] |= IEEE80211_HE_MAC_CAP2_ALL_ACK;
		he_cap->he_cap_elem.mac_cap_info[2] |= IEEE80211_HE_MAC_CAP2_BSR;
		cls_wifi_set_ppe_threshold(cls_wifi_hw, he_cap);
		if (cls_wifi_hw->radio_params->use_2040) {
			he_cap->he_cap_elem.phy_cap_info[0] |=
							IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_40MHZ_IN_2G;
			dcm_max_ru = IEEE80211_HE_PHY_CAP8_DCM_MAX_RU_484;
		}
		if (cls_wifi_hw->radio_params->use_80) {
			he_cap->he_cap_elem.phy_cap_info[0] |=
							IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_40MHZ_80MHZ_IN_5G;
			//mcs_map_max_2ss = IEEE80211_HE_MCS_SUPPORT_0_7;
			dcm_max_ru = IEEE80211_HE_PHY_CAP8_DCM_MAX_RU_996;
		}
		if (cls_wifi_hw->radio_params->use_160) {
			he_cap->he_cap_elem.phy_cap_info[0] |= IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_160MHZ_IN_5G;
			he_cap->he_cap_elem.phy_cap_info[0] |= IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_80PLUS80_MHZ_IN_5G;
			dcm_max_ru = IEEE80211_HE_PHY_CAP8_DCM_MAX_RU_2x996;
		}

		if (cls_wifi_hw->radio_params->ldpc_on) {
			he_cap->he_cap_elem.phy_cap_info[1] |= IEEE80211_HE_PHY_CAP1_LDPC_CODING_IN_PAYLOAD;
		} else {
			// If no LDPC is supported, we have to limit to MCS0_9, as LDPC is mandatory
			// for MCS 10 and 11
			cls_wifi_hw->radio_params->he_mcs_map = min_t(int,
					cls_wifi_hw->radio_params->he_mcs_map,
					IEEE80211_HE_MCS_SUPPORT_0_9);
		}
		he_cap->he_cap_elem.phy_cap_info[1] |= IEEE80211_HE_PHY_CAP1_HE_LTF_AND_GI_FOR_HE_PPDUS_0_8US |
											   IEEE80211_HE_PHY_CAP1_MIDAMBLE_RX_TX_MAX_NSTS;
		he_cap->he_cap_elem.phy_cap_info[2] |= IEEE80211_HE_PHY_CAP2_MIDAMBLE_RX_TX_MAX_NSTS |
											   IEEE80211_HE_PHY_CAP2_NDP_4x_LTF_AND_3_2US |
											   IEEE80211_HE_PHY_CAP2_DOPPLER_RX;

		if (cls_wifi_hw->radio_params->stbc_on) {
			he_cap->he_cap_elem.phy_cap_info[2] |= IEEE80211_HE_PHY_CAP2_STBC_RX_UNDER_80MHZ;
			if (cls_wifi_hw->radio_params->use_160)
				he_cap->he_cap_elem.phy_cap_info[7] |= IEEE80211_HE_PHY_CAP7_STBC_RX_ABOVE_80MHZ;
		}

		he_cap->he_cap_elem.phy_cap_info[3] |= IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_RX_16_QAM;
		if (nss > 0)
			he_cap->he_cap_elem.phy_cap_info[3] |= IEEE80211_HE_PHY_CAP3_DCM_MAX_RX_NSS_2;
		else
			he_cap->he_cap_elem.phy_cap_info[3] |= IEEE80211_HE_PHY_CAP3_DCM_MAX_RX_NSS_1;

		he_cap->he_cap_elem.phy_cap_info[3] |= IEEE80211_HE_PHY_CAP3_RX_PARTIAL_BW_SU_IN_20MHZ_MU;

		if (cls_wifi_hw->radio_params->bfmer) {
			he_cap->he_cap_elem.phy_cap_info[3] |= IEEE80211_HE_PHY_CAP3_SU_BEAMFORMER;
			he_cap->he_cap_elem.phy_cap_info[5] |= IEEE80211_HE_PHY_CAP5_BEAMFORMEE_NUM_SND_DIM_UNDER_80MHZ_2;
			if (cls_wifi_hw->radio_params->use_160)
				he_cap->he_cap_elem.phy_cap_info[5] |= IEEE80211_HE_PHY_CAP5_BEAMFORMEE_NUM_SND_DIM_ABOVE_80MHZ_2;
		}

		if (cls_wifi_hw->radio_params->bfmee) {
			he_cap->he_cap_elem.phy_cap_info[4] |= IEEE80211_HE_PHY_CAP4_SU_BEAMFORMEE;
			he_cap->he_cap_elem.phy_cap_info[4] |= IEEE80211_HE_PHY_CAP4_BEAMFORMEE_MAX_STS_UNDER_80MHZ_4;
			if (cls_wifi_hw->radio_params->use_160)
				he_cap->he_cap_elem.phy_cap_info[4] |= IEEE80211_HE_PHY_CAP4_BEAMFORMEE_MAX_STS_ABOVE_80MHZ_4;
		}
		he_cap->he_cap_elem.phy_cap_info[5] |= IEEE80211_HE_PHY_CAP5_NG16_SU_FEEDBACK |
											   IEEE80211_HE_PHY_CAP5_NG16_MU_FEEDBACK;
		he_cap->he_cap_elem.phy_cap_info[6] |= IEEE80211_HE_PHY_CAP6_CODEBOOK_SIZE_42_SU |
											   IEEE80211_HE_PHY_CAP6_CODEBOOK_SIZE_75_MU |
											   IEEE80211_HE_PHY_CAP6_TRIG_SU_BEAMFORMING_FB |
											   IEEE80211_HE_PHY_CAP6_TRIG_MU_BEAMFORMING_PARTIAL_BW_FB |
											   IEEE80211_HE_PHY_CAP6_PPE_THRESHOLD_PRESENT;

		he_cap->he_cap_elem.phy_cap_info[7] |= IEEE80211_HE_PHY_CAP7_HE_SU_MU_PPDU_4XLTF_AND_08_US_GI;
		he_cap->he_cap_elem.phy_cap_info[8] |= dcm_max_ru;
		//since linux 5.17, this macro is replaced with IEEE80211_HE_PHY_CAP9_NOMI(N)AL_PKT_PADDING_16US with shift
#ifndef IEEE80211_HE_PHY_CAP9_NOMIMAL_PKT_PADDING_16US
#define IEEE80211_HE_PHY_CAP9_NOMIMAL_PKT_PADDING_16US 		0x80
#endif
		he_cap->he_cap_elem.phy_cap_info[9] |= IEEE80211_HE_PHY_CAP9_RX_FULL_BW_SU_USING_MU_WITH_COMP_SIGB |
											   IEEE80211_HE_PHY_CAP9_RX_FULL_BW_SU_USING_MU_WITH_NON_COMP_SIGB |
											   IEEE80211_HE_PHY_CAP9_NOMIMAL_PKT_PADDING_16US;

		// Starting from version v31 more HE_ER_SU modulations is supported
		if (__MDM_VERSION(phy_vers) > 30) {
			he_cap->he_cap_elem.phy_cap_info[6] |= IEEE80211_HE_PHY_CAP6_PARTIAL_BW_EXT_RANGE;
			he_cap->he_cap_elem.phy_cap_info[8] |= IEEE80211_HE_PHY_CAP8_HE_ER_SU_1XLTF_AND_08_US_GI |
												   IEEE80211_HE_PHY_CAP8_HE_ER_SU_PPDU_4XLTF_AND_08_US_GI;
		}

		mcs_map = cls_wifi_hw->radio_params->he_mcs_map;
		if (cls_wifi_hw->radio_params->use_160) {
			mcs_map_160 = cls_wifi_hw->radio_params->he_mcs_map;
			mcs_map_80p80 = cls_wifi_hw->radio_params->he_mcs_map;
		} else {
			mcs_map_160 = IEEE80211_HE_MCS_NOT_SUPPORTED;
			mcs_map_80p80 = IEEE80211_HE_MCS_NOT_SUPPORTED;
		}
		memset(&he_cap->he_mcs_nss_supp, 0, sizeof(he_cap->he_mcs_nss_supp));
		for (i = 0; i < nss; i++) {
			he_cap->he_mcs_nss_supp.rx_mcs_80 |= cpu_to_le16(mcs_map << (i*2));
			he_cap->he_mcs_nss_supp.rx_mcs_160 |= cpu_to_le16(mcs_map_160 << (i*2));
			he_cap->he_mcs_nss_supp.rx_mcs_80p80 |= cpu_to_le16(mcs_map_80p80 << (i*2));
			he_cap->he_mcs_nss_supp.tx_mcs_80 |= cpu_to_le16(mcs_map << (i*2));
			he_cap->he_mcs_nss_supp.tx_mcs_160 |= cpu_to_le16(mcs_map_160 << (i*2));
			he_cap->he_mcs_nss_supp.tx_mcs_80p80 |= cpu_to_le16(mcs_map_80p80 << (i*2));

			// update mcs_map for 2SS and above
			mcs_map = min_t(int, mcs_map, mcs_map_max_2ss);
			mcs_map_160 = min_t(int, mcs_map, mcs_map_max_2ss);
			mcs_map_80p80 = IEEE80211_HE_MCS_NOT_SUPPORTED;
		}
		for (i = nss; i < 8; i++) {
			__le16 unsup_for_ss = cpu_to_le16(IEEE80211_HE_MCS_NOT_SUPPORTED << (i*2));
			he_cap->he_mcs_nss_supp.rx_mcs_80 |= unsup_for_ss;
			he_cap->he_mcs_nss_supp.rx_mcs_160 |= unsup_for_ss;
			he_cap->he_mcs_nss_supp.rx_mcs_80p80 |= unsup_for_ss;
			he_cap->he_mcs_nss_supp.tx_mcs_80 |= unsup_for_ss;
			he_cap->he_mcs_nss_supp.tx_mcs_160 |= unsup_for_ss;
			he_cap->he_mcs_nss_supp.tx_mcs_80p80 |= unsup_for_ss;
		}

		// copy common HE capabilities to for all types of interfaces
		for ( i = 1; i < sband->n_iftype_data; i++) {
			memcpy((void *)&sband->iftype_data[i].he_cap, he_cap, sizeof(*he_cap));
		}

		// HE Capabilities only for AP interfaces
		cls_wifi_set_sta_he_capa(cls_wifi_hw, he_cap);

		// HE Capabilities only for AP interfaces
		he_cap = (struct ieee80211_sta_he_cap *)&sband->iftype_data[1].he_cap;
		cls_wifi_set_ap_he_capa(cls_wifi_hw, he_cap);
	}
#endif // LINUX >= 4.20.0
}

static void cls_wifi_set_wiphy_params(struct cls_wifi_hw *cls_wifi_hw, struct wiphy *wiphy)
{
	if (cls_wifi_hw->radio_params->tdls) {
		/* TDLS support */
		wiphy->flags |= WIPHY_FLAG_SUPPORTS_TDLS;
		wiphy->features |= NL80211_FEATURE_TDLS_CHANNEL_SWITCH;
		/* TDLS external setup support */
		wiphy->flags |= WIPHY_FLAG_TDLS_EXTERNAL_SETUP;
	}

	if (cls_wifi_hw->radio_params->ap_uapsd_on)
		wiphy->flags |= WIPHY_FLAG_AP_UAPSD;

	if (cls_wifi_hw->radio_params->ps_on)
		wiphy->flags |= WIPHY_FLAG_PS_ON_BY_DEFAULT;
	else
		wiphy->flags &= ~WIPHY_FLAG_PS_ON_BY_DEFAULT;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 20, 0)
	if (cls_wifi_hw->version_cfm.features & BIT(MM_FEAT_FAKE_FTM_RSP_BIT))
		wiphy_ext_feature_set(wiphy, NL80211_EXT_FEATURE_ENABLE_FTM_RESPONDER);
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)) || defined(CONFIG_ARCH_CLOURNEY)
	wiphy_ext_feature_set(wiphy, NL80211_EXT_FEATURE_BSS_COLOR);
#endif
	wiphy_ext_feature_set(wiphy, NL80211_EXT_FEATURE_SET_SCAN_DWELL);

	if (cls_wifi_hw->radio_params->custregd) {
		// Check if custom channel set shall be enabled. In such case only monitor mode is
		// supported
		if (cls_wifi_hw->radio_params->custchan) {
			wiphy->interface_modes = BIT(NL80211_IFTYPE_MONITOR);

			// Enable "extra" channels
			if (cls_wifi_hw->band_cap & (1 << NL80211_BAND_2GHZ))
				cls_wifi_hw->if_cfg80211.sbands[NL80211_BAND_2GHZ].n_channels += 13;
			if (cls_wifi_hw->band_cap & (1 << NL80211_BAND_5GHZ))
				cls_wifi_hw->if_cfg80211.sbands[NL80211_BAND_5GHZ].n_channels += 59;
		}
	}
}

static void cls_wifi_set_rf_params(struct cls_wifi_hw *cls_wifi_hw, struct wiphy *wiphy)
{
	struct ieee80211_supported_band *sband;
	u32 mdm_phy_cfg = __MDM_PHYCFG_FROM_VERS(cls_wifi_hw->version_cfm.version_phy_1);

	if (!(cls_wifi_hw->band_cap & CLS_WIFI_BAND_CAP_5G))
		return;
	sband = &cls_wifi_hw->if_cfg80211.sbands[NL80211_BAND_5GHZ];

	/*
	 * adjust caps depending on the RF
	 */
	switch (mdm_phy_cfg) {
		case MDM_PHY_CONFIG_VERSION:
		{
			wiphy_dbg(wiphy, "found CLS WIFI PHY version 2\n");
			break;
		}
		default:
			WARN_ON(1);
			break;
	}
}

int cls_wifi_handle_dynparams(struct cls_wifi_hw *cls_wifi_hw, struct wiphy *wiphy)
{
	int ret;

	/* Check compatibility between requested parameters and HW/SW features */
	ret = cls_wifi_check_fw_hw_feature(cls_wifi_hw, wiphy);
	if (ret)
		return ret;

	/* Allocate the RX buffers according to the maximum AMSDU RX size */
	ret = cls_wifi_ipc_rxbuf_init(cls_wifi_hw, (sizeof(struct hw_rxhdr) + MSDU_LENGTH));

	if (ret) {
		wiphy_err(wiphy, "Cannot allocate the RX buffers\n");
		return ret;
	}

	/* Set wiphy parameters */
	cls_wifi_set_wiphy_params(cls_wifi_hw, wiphy);

	/* Set VHT capabilities */
	cls_wifi_set_vht_capa(cls_wifi_hw, wiphy);

	/* Set HE capabilities */
	cls_wifi_set_he_capa(cls_wifi_hw, wiphy);

	/* Set HT capabilities */
	cls_wifi_set_ht_capa(cls_wifi_hw, wiphy);

	/* Set RF specific parameters (shall be done last as it might change some
	   capabilities previously set) */
	cls_wifi_set_rf_params(cls_wifi_hw, wiphy);

	return 0;
}

void cls_wifi_custregd(struct cls_wifi_hw *cls_wifi_hw, struct wiphy *wiphy)
{
	struct ieee80211_regdomain *ptr_cls_wifi_regdom;

	if (!cls_wifi_hw->radio_params->custregd)
		return;

	wiphy->regulatory_flags |= REGULATORY_IGNORE_STALE_KICKOFF;
	wiphy->regulatory_flags |= REGULATORY_WIPHY_SELF_MANAGED;
	ptr_cls_wifi_regdom = &cls_wifi_regdom;

	rtnl_lock();
	wiphy_lock(wiphy);
	if (regulatory_set_wiphy_regd_sync(wiphy, ptr_cls_wifi_regdom))
		wiphy_err(wiphy, "Failed to set custom regdomain\n");
	else
		wiphy_err(wiphy,"\n"
				  "*******************************************************\n"
				  "** CAUTION: USING PERMISSIVE CUSTOM REGULATORY RULES **\n"
				  "*******************************************************\n");
	wiphy_unlock(wiphy);
	rtnl_unlock();
}

void cls_wifi_adjust_amsdu_maxnb(struct cls_wifi_hw *cls_wifi_hw)
{
	if (cls_wifi_hw->radio_params->amsdu_maxnb > CLS_TX_PAYLOAD_MAX)
		cls_wifi_hw->radio_params->amsdu_maxnb = CLS_TX_PAYLOAD_MAX;
	else if (cls_wifi_hw->radio_params->amsdu_maxnb == 0)
		cls_wifi_hw->radio_params->amsdu_maxnb = 1;
}

void cls_wifi_limit_amsdu_maxnb(struct cls_wifi_hw *cls_wifi_hw, uint8_t amsdu_nb)
{
	pr_info("%s limit_amsdu_maxnb %u\n", __func__, amsdu_nb);
	cls_wifi_hw->radio_params->amsdu_maxnb = amsdu_nb;
	cls_wifi_adjust_amsdu_maxnb(cls_wifi_hw);
}

