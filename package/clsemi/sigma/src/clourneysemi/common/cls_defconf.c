#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "cls/clsapi.h"
#include <linux/wireless.h>
#include <net80211/ieee80211.h>
#include <net80211/ieee80211_ioctl.h>
#include <sys/ioctl.h>

#include "csigma_log.h"
#include "cls_dut_common.h"
#include "cls_defconf.h"
#include "csigma_common.h"

/* this header should be included last */

int need_clear_reserve_all_slots;
static void reset_rts_cts_settings(struct cls_dut_config *conf, const char* ifname)
{
	conf->bws_enable = 0;
	conf->bws_dynamic = 0;
	conf->force_rts = 0;
	conf->update_settings = 0;

	cls_set_rts_settings(ifname, conf);
}

static void allow_mu_for_non_cls(const char* ifname)
{
	char tmpbuf[64];
	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s set_mu_noncls 1", ifname);
	system(tmpbuf);
}

int cls_defconf_vht_testbed_sta(const char* ifname)
{
	int ret;
	struct cls_dut_config *conf;
	char tmpbuf[64];

	cls_log("%s, ifname %s", __FUNCTION__, ifname);

	/*  Table 138: Testbed Default Mode STA
	 * ---------------------------------------------------------
	 * #  | Mode name                   | Default | Notes
	 * ---------------------------------------------------------
	 * 1  | Spatial streams             | 1       |
	 * 2  | Bandwidth                   | 80 MHz  |
	 * 3  | VHT MCS Set                 | 0-7     | MCS 8-9 off
	 * 4  | Short GI for 20 MHz         | Off     | for both Tx/Rx
	 * 5  | Short GI for 40 MHz         | Off     | for both Tx/Rx
	 * 6  | Short GI for 80 MHz         | Off     | for both Tx/Rx
	 * 7  | SU Transmit Beamforming     | Off     |
	 * 8  | SU Transmit Beamformee      | Off     |
	 * 9  | MU Transmit Beamformer      | Off     |
	 * 10 | MU Transmit Beamformee      | Off     |
	 * 11 | Transmit A-MSDU             | Off     |
	 * 12 | Receive A-MPDU with A-MSDU  | Off     |
	 * 13 | STBC 2x1 Transmit           | Off     |
	 * 14 | STBC 2x1 Receive            | Off     |
	 * 15 | LDPC                        | Off     |
	 * 16 | Operating Mode Notification | Off     | Transmit
	 * 17 | RTS with Bandwidth Signaling| Off     |
	 * 18 | Two-character Country Code  | Off     |
	 * 19 | Transmit Power Control      | Off     |
	 * 20 | Channel Switching           | Off     |
	 * ---------------------------------------------------------
	 */

	ret = cls_set_phy_mode(ifname , "11ac");
	if (ret < 0) {
		cls_error("error: cannot set 11ac, errcode %d", ret);
		return ret;
	}

	/* VHT mode */
	ret = clsapi_wifi_set_vht(ifname, 1);
	if (ret < 0) {
		cls_error("error: cannot enable vht, errcode %d", ret);
		return ret;
	}

	/* 1. Spatial streams, 1 */
	ret = cls_set_nss_cap(ifname, clsapi_mimo_vht, 1);

	if (ret < 0) {
		cls_error("error: cannot set NSS capability, errcode %d", ret);
		return ret;
	}

	/* 2. Bandwidth, 80Mhz */
	cls_set_fixed_bw(ifname, 0);

	ret = clsapi_wifi_set_bw(ifname, clsapi_bw_80MHz, "11ac");
	if (ret < 0) {
		cls_error("error: cannot set bw capability %d, errcode %d", clsapi_bw_80MHz, ret);
		return ret;
	}

	/* 3. VHT MCS Set, 0-7 */
	cls_set_mcs_cap(ifname, IEEE80211_VHT_MCS_0_7);

	/* 4. Short GI for 20 MHz, Off, for both Tx/Rx
	 * 5. Short GI for 40 MHz, Off, for both Tx/Rx
	 * 6. Short GI for 80 MHz, Off, for both Tx/Rx
	 */

	/* disable dynamic GI selection */
	ret = clsapi_wifi_set_option(ifname, clsapi_GI_probing, 0);
	if (ret < 0) {
		/* ignore error since clsapi_GI_probing does not work for RFIC6 */
		cls_error("error: disable dynamic GI selection, errcode %d", ret);
	}

	/* disable short GI */
	ret = clsapi_wifi_set_option(ifname, clsapi_short_GI, 0);
	if (ret < 0) {
		cls_error("error: disable short GI, errcode %d", ret);
		return ret;
	}

	/* 7. SU Transmit Beamforming, Off */
	/* 8. SU Transmit Beamformee, Off */
	ret = clsapi_wifi_set_option(ifname, clsapi_beamforming, 0);
	if (ret < 0) {
		cls_error("error: disable beamforming, errcode %d", ret);
		return ret;
	}

	/* 9. MU Transmit Beamformer, Off */
	/* 10. MU Transmit Beamformee, 0ff */
	allow_mu_for_non_cls(ifname);
	ret = cls_set_mu_enable(ifname, 0);

	if (ret < 0) {
		cls_error("error: disable MU beamforming, errcode %d", ret);
		return ret;
	}

	/* 11. Transmit A-MSDU, Off
	 * 12. Receive A-MPDU with A-MSDU, Off
	 */
	ret = cls_set_amsdu(ifname, 0);
	if (ret < 0) {
		cls_error("error: disable tx amsdu, errcode %d", ret);
		return ret;
	}

	/* 13. STBC 2x1 Transmit, Off
	 * 14. STBC 2x1 Receive, Off
	 */
	ret = clsapi_wifi_set_option(ifname, clsapi_stbc, 0);
	if (ret < 0) {
		cls_error("error: cannot set stbc, errcode %d", ret);
		return ret;
	}

	/* 15. LDPC, Off */
	snprintf(tmpbuf, sizeof(tmpbuf), "iw dev %s set_ldpc %d", ifname, 0);
	system(tmpbuf);

	/* 16. Operating Mode Notification, Off, Transmit */
	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s set_vht_opmntf %d",
			ifname,
			0xFFFF);
	system(tmpbuf);

	/* 17. RTS with Bandwidth Signaling, Off */
	conf = cls_dut_get_config(ifname);

	if (conf) {
		reset_rts_cts_settings(conf, ifname);

	} else {
		ret = -EFAULT;
		cls_error("error: cannot get config, errcode %d", ret);
		return ret;
	}

	/* 18. Two-character Country Code, Off */
	/* 19. Transmit Power Control, Off */
	/* 20. Channel Switching, Off */

	cls_log("END: cls_defconf_vht_testbed_sta");

	return 0;
}

int cls_defconf_vht_testbed_ap(const char* ifname)
{
	int ret;
	struct cls_dut_config *conf;
	char tmpbuf[64];

	cls_log("%s, ifname %s", __FUNCTION__, ifname);

	/*  Table 137: Testbed Default Mode AP
	 * ---------------------------------------------------------
	 * #  | Mode name                   | Default | Notes
	 * ---------------------------------------------------------
	 * 1  | Spatial streams             | 2/4     | 4 for R2
	 * 2  | Bandwidth                   | 80 MHz  |
	 * 3  | VHT MCS Set                 | 0-7     | MCS 8-9 off
	 * 4  | Short GI for 20 MHz         | Off     | for both Tx/Rx
	 * 5  | Short GI for 40 MHz         | Off     | for both Tx/Rx
	 * 6  | Short GI for 80 MHz         | Off     | for both Tx/Rx
	 * 7  | SU Transmit Beamforming     | Off     |
	 * 8  | SU Transmit Beamformee      | Off     |
	 * 9  | MU Transmit Beamformer      | Off     |
	 * 10 | MU Transmit Beamformee      | Off     |
	 * 11 | Transmit A-MSDU             | Off     |
	 * 12 | Receive A-MPDU with A-MSDU  | Off     |
	 * 13 | STBC 2x1 Transmit           | Off     |
	 * 14 | STBC 2x1 Receive            | Off     |
	 * 15 | LDPC                        | Off     |
	 * 16 | Operating Mode Notification | Off     | Transmit
	 * 17 | RTS with Bandwidth Signaling| Off     |
	 * 18 | Two-character Country Code  | Any     |
	 * 19 | Transmit Power Control      | Any     |
	 * 20 | Channel Switching           | Any     |
	 * ---------------------------------------------------------
	 */


	ret = clsapi_wifi_scs_enable(ifname, 0);
	if (ret < 0) {
		cls_error("error: cannot disable SCS, error %d", ret);
	}

	ret = cls_set_phy_mode(ifname , "11ac");
	if (ret < 0) {
		cls_error("error: cannot set 11ac, errcode %d", ret);
		return ret;
	}

	int ch = cls_get_sigma_default_channel(ifname);

	ret = clsapi_wifi_set_channel(ifname, ch);
	if (ret < 0) {
		cls_error("error: cannot set channel to %d, errcode %d", ch, ret);
		return ret;
	}

	/* VHT mode */
	ret = clsapi_wifi_set_vht(ifname, 1);
	if (ret < 0) {
		cls_error("error: cannot enable vht, errcode %d", ret);
		return ret;
	}

	/* 1. Spatial streams, 4 */
	ret = cls_set_nss_cap(ifname, clsapi_mimo_vht, 4);

	if (ret < 0) {
		cls_error("error: cannot set NSS capability, errcode %d", ret);
		return ret;
	}

	/* 2. Bandwidth, 80Mhz */
	cls_set_fixed_bw(ifname, 0);

	ret = clsapi_wifi_set_bw(ifname, clsapi_bw_80MHz, "11ac");
	if (ret < 0) {
		cls_error("error: cannot set bw capability %d, errcode %d", clsapi_bw_80MHz, ret);
		return ret;
	}

	/* 3. VHT MCS Set, 0-7 */
	cls_set_mcs_cap(ifname, IEEE80211_VHT_MCS_0_7);

	/* 4. Short GI for 20 MHz, Off, for both Tx/Rx
	 * 5. Short GI for 40 MHz, Off, for both Tx/Rx
	 * 6. Short GI for 80 MHz, Off, for both Tx/Rx
	 */

	/* disable dynamic GI selection */
	ret = clsapi_wifi_set_option(ifname, clsapi_GI_probing, 0);
	if (ret < 0) {
		/* ignore error since clsapi_GI_probing does not work for RFIC6 */
		cls_error("error: disable dynamic GI selection, errcode %d", ret);
	}

	/* disable short GI */
	ret = clsapi_wifi_set_option(ifname, clsapi_short_GI, 0);
	if (ret < 0) {
		cls_error("error: disable short GI, errcode %d", ret);
		return ret;
	}

	/* 7. SU Transmit Beamforming, Off */
	/* 8. SU Transmit Beamformee, Off */
	ret = clsapi_wifi_set_option(ifname, clsapi_beamforming, 0);
	if (ret < 0) {
		cls_error("error: disable beamforming, errcode %d", ret);
		return ret;
	}

	/* 9. MU Transmit Beamformer, Off */
	/* 10. MU Transmit Beamformee, 0ff */
	allow_mu_for_non_cls(ifname);
	ret = cls_set_mu_enable(ifname, 0);

	if (ret < 0) {
		cls_error("error: disable MU beamforming, errcode %d", ret);
		return ret;
	}

	/* 11. Transmit A-MSDU, Off
	 * 12. Receive A-MPDU with A-MSDU, Off
	 */
	ret = cls_set_amsdu(ifname, 0);
	if (ret < 0) {
		cls_error("error: disable tx amsdu, errcode %d", ret);
		return ret;
	}

	/* 13. STBC 2x1 Transmit, Off
	 * 14. STBC 2x1 Receive, Off
	 */
	ret = clsapi_wifi_set_option(ifname, clsapi_stbc, 0);
	if (ret < 0) {
		cls_error("error: cannot set stbc, errcode %d", ret);
		return ret;
	}

	/* 15. LDPC, Off */
	snprintf(tmpbuf, sizeof(tmpbuf), "iw dev %s set_ldpc %d", ifname, 0);
	system(tmpbuf);

	/* 16. Operating Mode Notification, Off, Transmit */
	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s set_vht_opmntf %d",
			ifname,
			0xFFFF);
	system(tmpbuf);

	/* 17. RTS with Bandwidth Signaling, Off */
	conf = cls_dut_get_config(ifname);

	if (conf) {
		reset_rts_cts_settings(conf, ifname);
	} else {
		ret = -EFAULT;
		cls_error("error: cannot get config, errcode %d", ret);
		return ret;
	}

	/* set MU NDPA format to default. */
	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s set_mu_non_ht %d", ifname, 0);
	system(tmpbuf);

	/* 18. Two-character Country Code, Any */
	/* 19. Transmit Power Control, Any */
	/* 20. Channel Switching, Any */

	return 0;
}

int cls_defconf_vht_dut_sta(const char* ifname)
{
	int ret;
	struct cls_dut_config *conf;
	char tmpbuf[64];

	cls_log("%s, ifname %s", __FUNCTION__, ifname);

	/*  Table 142: STAUT Default Mode
	 * ---------------------------------------------------------
	 * #  | Mode name                   | Default | Notes
	 * ---------------------------------------------------------
	 * 1  | Spatial streams             | 4       | use 4 SS for VHT R2
	 * 2  | Bandwidth                   | 80 MHz  |
	 * 3  | VHT MCS Set                 | 0-9     |
	 * 4  | Short GI for 20 MHz         | On      | for both Tx/Rx
	 * 5  | Short GI for 40 MHz         | On      | for both Tx/Rx
	 * 6  | Short GI for 80 MHz         | On      | for both Tx/Rx
	 * 7  | SU Transmit Beamformer      | On      |
	 * 8  | SU Transmit Beamformee      | On      |
	 * 9  | MU Transmit Beamformer      | Off     |
	 * 10 | MU Transmit Beamformee      | Off     |
	 * 11 | Transmit A-MSDU             | On      |
	 * 12 | Receive A-MPDU with A-MSDU  | On      |
	 * 13 | Tx STBC 2x1                 | On      |
	 * 14 | Rx STBC 2x1                 | On      |
	 * 15 | Tx LDPC                     | On      |
	 * 16 | Rx LDPC                     | On      |
	 * 17 | Operating Mode Notification | On      | Transmit
	 * 18 | RTS with Bandwidth Signaling| On      |
	 * 19 | Two-character Country Code  | On      |
	 * 20 | Transmit Power Control      | On      |
	 * 21 | Channel Switching           | On      |
	 * ---------------------------------------------------------
	 */

	ret = clsapi_wifi_scs_enable(ifname, 0);
	if (ret < 0) {
		cls_error("error: cannot disable SCS, error %d", ret);
	}

/*	ret = clsapi_wifi_cancel_scan(ifname, 0);
	if (ret < 0) {
		cls_error("error: can't cancel scan, error %d", ret);
	}
*/
	ret = cls_set_phy_mode(ifname , "11ac");
	if (ret < 0) {
		cls_error("error: cannot set 11ac, errcode %d", ret);
		return ret;
	}

	int ch = cls_get_sigma_default_channel(ifname);

	ret = clsapi_wifi_set_channel(ifname, ch);
	if (ret < 0) {
		cls_error("error: can't set channel to %d, error %d", ch, ret);
	}

	/* VHT mode */
	ret = clsapi_wifi_set_vht(ifname, 1);
	if (ret < 0) {
		cls_error("error: cannot enable vht, errcode %d", ret);
		return ret;
	}

	/* 1. Spatial streams, 4 */
	ret = cls_set_nss_cap(ifname, clsapi_mimo_vht, 4);

	if (ret < 0) {
		cls_error("error: cannot set NSS capability, errcode %d", ret);
		return ret;
	}

	/* 2. Bandwidth, 80Mhz */
	cls_set_fixed_bw(ifname, 0);

	ret = clsapi_wifi_set_bw(ifname, clsapi_bw_80MHz, "11ac");
	if (ret < 0) {
		cls_error("error: cannot set bw capability %d, errcode %d", clsapi_bw_80MHz, ret);
		return ret;
	}

	/* 3. VHT MCS Set, 0-9 */
	cls_set_mcs_cap(ifname, IEEE80211_VHT_MCS_0_9);

	/* 4. Short GI for 20 MHz, Off, for both Tx/Rx
	 * 5. Short GI for 40 MHz, Off, for both Tx/Rx
	 * 6. Short GI for 80 MHz, Off, for both Tx/Rx
	 */

	/* enable dynamic GI selection */
	ret = clsapi_wifi_set_option(ifname, clsapi_GI_probing, 1);
	if (ret < 0) {
		/* not supported on RFIC6, ignore error for now. */
		cls_error("error: enable dynamic GI selection, errcode %d", ret);
	}

	/* enable short GI */
	ret = clsapi_wifi_set_option(ifname, clsapi_short_GI, 1);
	if (ret < 0) {
		cls_error("error: enable short GI, errcode %d", ret);
		return ret;
	}

	/* 7. SU Transmit Beamformer, On */
	/* 8. SU Transmit Beamformee, On */
	ret = clsapi_wifi_set_option(ifname, clsapi_beamforming, 1);
	if (ret < 0) {
		cls_error("error: enable beamforming, errcode %d", ret);
		return ret;
	}

	/* 9. MU Transmit Beamformer, Off */
	/* 10. MU Transmit Beamformee, 0ff */
	allow_mu_for_non_cls(ifname);
	ret = cls_set_mu_enable(ifname, 0);

	if (ret < 0) {
		cls_error("error: disable MU beamforming, errcode %d", ret);
		return ret;
	}

	/* 11. Transmit A-MSDU, On
	 * 12. Receive A-MPDU with A-MSDU, On
	 */
	ret = cls_set_amsdu(ifname, 1);
	if (ret < 0) {
		cls_error("error: disable tx amsdu, errcode %d", ret);
		return ret;
	}

	/* 13. Tx STBC 2x1, On
	 * 14. Rx STBC 2x1, On
	 */
	ret = clsapi_wifi_set_option(ifname, clsapi_stbc, 1);
	if (ret < 0) {
		cls_error("error: cannot set stbc, errcode %d", ret);
		return ret;
	}

	/* 15. Tx LDPC, On
	 * 16. Rx LDPC, On
	 */
	snprintf(tmpbuf, sizeof(tmpbuf), "iw dev %s set_ldpc %d", ifname, 1);
	system(tmpbuf);

	/* 17. Operating Mode Notification, On (if supported) */
	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s set_vht_opmntf %d",
			ifname,
			0xFFFF);
	system(tmpbuf);

	/* 18. RTS with Bandwidth Signaling, On (if supported) */
	conf = cls_dut_get_config(ifname);

	if (conf) {
		reset_rts_cts_settings(conf, ifname);
	} else {
		ret = -EFAULT;
		cls_error("error: cannot get config, errcode %d", ret);
		return ret;
	}

	/* 19. Two-character Country Code, On (if supported) */
	/* 20. Transmit Power Control, On (if supported) */
	/* 21. Channel Switching, On (if supported) */

	return 0;
}

int cls_defconf_vht_dut_ap(const char* ifname)
{
	cls_log("%s, ifname %s", __FUNCTION__, ifname);

	/*  Table 141: APUT Default Mode
	 * ---------------------------------------------------------
	 * #  | Mode name                   | Default | Notes
	 * ---------------------------------------------------------
	 * 1  | Spatial streams             | 2/4     | use 4 SS for VHT R2
	 * 2  | Bandwidth                   | 80 MHz  |
	 * 3  | VHT MCS Set                 | 0-9     |
	 * 4  | Short GI for 20 MHz         | On      | for both Tx/Rx
	 * 5  | Short GI for 40 MHz         | On      | for both Tx/Rx
	 * 6  | Short GI for 80 MHz         | On      | for both Tx/Rx
	 * 7  | SU Transmit Beamformer      | On      |
	 * 8  | SU Transmit Beamformee      | On      |
	 * 9  | MU Transmit Beamformer      | Off     |
	 * 10 | MU Transmit Beamformee      | Off     |
	 * 11 | Transmit A-MSDU             | On      |
	 * 12 | Receive A-MPDU with A-MSDU  | On      |
	 * 13 | Tx STBC 2x1                 | On      |
	 * 14 | Rx STBC 2x1                 | On      |
	 * 15 | Tx LDPC                     | On      |
	 * 16 | Rx LDPC                     | On      |
	 * 17 | Operating Mode Notification | On      | Transmit
	 * 18 | RTS with Bandwidth Signaling| On      |
	 * 19 | Two-character Country Code  | On      |
	 * 20 | Transmit Power Control      | On      |
	 * 21 | Channel Switching           | On      |
	 * ---------------------------------------------------------
	 */

	return cls_defconf_vht_dut_sta(ifname);
}

int cls_defconf_vht_dut_ap_all()
{
	int ret = -EINVAL;
	static char ifname[IFNAMSIZ] = {0};
	enum clsapi_freq_band band_info = clsapi_freq_band_unknown;

	for (int radio_id = 0; radio_id < CLS_MAX_RADIO_ID; ++radio_id) {
		int chipid;
		if (cls_wifi_get_rf_chipid(radio_id, &chipid) < 0)
			chipid = CHIPID_5_GHZ;

		ret = clsapi_radio_get_primary_interface(radio_id, ifname,
				sizeof(ifname));
		if (ret < 0)
			continue;

		band_info = cls_get_sigma_band_info_from_interface(ifname);

		if (band_info == clsapi_freq_band_6_ghz)
			continue;

		char status[32] = {0};
		if (clsapi_interface_get_status(ifname, status) < 0)
			continue;

		if (chipid == CHIPID_2_4_GHZ) {
			if (cls_defconf_11n_dut(ifname, "11ng") < 0)
				cls_error("warning: failed to set default settings for 2.4G interface");
		} else {
			ret = cls_defconf_vht_dut_ap(ifname);
			if (ret < 0)
				return ret;
		}
	}

	return ret;
}

int cls_defconf_pmf_dut(const char* ifname)
{
	int ret;

	cls_log("%s, ifname %s", __FUNCTION__, ifname);


	ret = cls_set_phy_mode(ifname , "11na");
	if (ret < 0) {
		cls_error("error: cannot set 11an, errcode %d", ret);
		return ret;
	}


	/* 1. Spatial streams, 2 */
	ret = cls_set_nss_cap(ifname, clsapi_mimo_ht, 2);
	if (ret < 0) {
		cls_error("error: cannot set NSS capability, errcode %d", ret);
		return ret;
	}

	/* 2. Bandwidth, 20Mhz */
	ret = clsapi_wifi_set_bw(ifname, clsapi_bw_20MHz, "11na");
	if (ret < 0) {
		cls_error("error: cannot set bw capability %d, errcode %d", clsapi_bw_20MHz, ret);
		return ret;
	}

	/* enable dynamic GI selection */
	ret = clsapi_wifi_set_option(ifname, clsapi_GI_probing, 1);
	if (ret < 0) {
		/* not supported on RFIC6, ignore error for now. */
		cls_error("error: enable dynamic GI selection, errcode %d", ret);
	}

	/* enable short GI */
	ret = clsapi_wifi_set_option(ifname, clsapi_short_GI, 1);
	if (ret < 0) {
		cls_error("error: enable short GI, errcode %d", ret);
		return ret;
	}

	return 0;
}

/* restore default hostapd config */
static void cls_defconf_hostapd_conf(int reconfigure)
{
	char cmdbuf[CLS_DEFCONF_CMDBUF_LEN] = {0};

	if (snprintf(cmdbuf, CLS_DEFCONF_CMDBUF_LEN,
			"/scripts/restore_default_config -nr -m ap -d") <= 0) {
		cls_error("error: cannot format cmd, errno = %d", errno);
		return;
	}

	system(cmdbuf);

	if (!reconfigure)
		return;

	if (snprintf(cmdbuf, CLS_DEFCONF_CMDBUF_LEN,
			"test -e /scripts/hostapd.conf && "
			"cat /scripts/hostapd.conf > /mnt/jffs2/hostapd.conf && "
			"hostapd_cli reconfigure") <= 0) {
		cls_error("error: cannot format cmd, errno = %d", errno);
		return;
	}

	system(cmdbuf);
}


/* restore default wpa_supplicant config */
static void cls_defconf_wpa_supplicant_conf(int reconfigure)
{
	char cmdbuf[CLS_DEFCONF_CMDBUF_LEN] = {0};

	if (snprintf(cmdbuf, CLS_DEFCONF_CMDBUF_LEN,
			"/scripts/restore_default_config -nr -m sta -d") <= 0) {
		cls_error("error: cannot format cmd, errno = %d", errno);
		return;
	}

	system(cmdbuf);

	if (!reconfigure)
		return;

	if (snprintf(cmdbuf, CLS_DEFCONF_CMDBUF_LEN,
			"test -e /scripts/wpa_supplicant.conf && "
			"cat /scripts/wpa_supplicant.conf > /mnt/jffs2/wpa_supplicant.conf && "
			"wpa_cli reconfigure") <= 0) {
		cls_error("error: cannot format cmd, errno = %d", errno);
		return;
	}

	system(cmdbuf);
}

int cls_defconf_hs2_dut(const char* ifname)
{
	int ret;
	unsigned int radio_id = 0;

	cls_log("%s, ifname %s", __FUNCTION__, ifname);

	ret = clsapi_get_radio_from_ifname(ifname, &radio_id);
	if (ret < 0) {
		cls_error("error: cannot get radio id from ifname, errcode %d", ret);
		return ret;
	}

	ret = clsapi_wifi_scs_enable(ifname, 0);
	if (ret < 0) {
		cls_error("error: cannot disable SCS, error %d", ret);
		return ret;
	}

	/* VHT mode */
	ret = clsapi_wifi_set_vht(ifname, 0);
	if (ret < 0) {
		cls_error("error: cannot disable vht, errcode %d", ret);
		return ret;
	}

	/* 1. Spatial streams, 2 */
	ret = cls_set_nss_cap(ifname, clsapi_mimo_ht, 2);

	if (ret < 0) {
		cls_error("error: cannot set NSS capability, errcode %d", ret);
		return ret;
	}

	/* 2. Bandwidth, 20Mhz */
	ret = clsapi_wifi_set_bw(ifname, clsapi_bw_20MHz, "11ac");
	if (ret < 0) {
		cls_error("error: cannot set bw capability %d, errcode %d", clsapi_bw_20MHz, ret);
		return ret;
	}

	/* 4. Short GI for 20 MHz, Off, for both Tx/Rx
	 * 5. Short GI for 40 MHz, Off, for both Tx/Rx
	 * 6. Short GI for 80 MHz, Off, for both Tx/Rx
	 */

	/* enable dynamic GI selection */
	ret = clsapi_wifi_set_option(ifname, clsapi_GI_probing, 1);
	if (ret < 0) {
		/* not supported on RFIC6, ignore error for now. */
		cls_error("error: enable dynamic GI selection, errcode %d", ret);
	}

	/* enable short GI */
	ret = clsapi_wifi_set_option(ifname, clsapi_short_GI, 1);
	if (ret < 0) {
		cls_error("error: enable short GI, errcode %d", ret);
		return ret;
	}

	ret = clsapi_wifi_set_interworking(ifname, "1");
	if (ret < 0) {
		cls_error("error: enable interworking, errcode %d", ret);
		return ret;
	}

	ret = clsapi_wifi_set_80211u_params(ifname, "access_network_type", "2", NULL);
	if (ret < 0) {
		cls_error("error: set 80211u access_network_type, errcode %d", ret);
		return ret;
	}

	ret = clsapi_wifi_set_80211u_params(ifname, "internet", "0", NULL);
	if (ret < 0) {
		cls_error("error: set 80211u internet, errcode %d", ret);
		return ret;
	}

	ret = clsapi_wifi_set_80211u_params(ifname, "venue_group", "2", NULL);
	if (ret < 0) {
		cls_error("error: set 80211u venue_group, errcode %d", ret);
		return ret;
	}

	ret = clsapi_wifi_set_80211u_params(ifname, "venue_type", "8", NULL);
	if (ret < 0) {
		cls_error("error: set 80211u venue_type, errcode %d", ret);
		return ret;
	}

	ret = clsapi_wifi_set_80211u_params(ifname, "hessid", "50:6f:9a:00:11:22", NULL);
	if (ret < 0) {
		cls_error("error: set 80211u hessid, errcode %d", ret);
		return ret;
	}

	ret = clsapi_wifi_set_80211u_params(ifname, "network_auth_type", "01", NULL);
	if (ret < 0) {
		cls_error("error: set 80211u network_auth_type, errcode %d", ret);
		return ret;
	}

	ret = clsapi_wifi_set_80211u_params(ifname, "domain_name", "wi-fi.org", NULL);
	if (ret < 0) {
		cls_error("error: set 80211u domain_name, errcode %d", ret);
		return ret;
	}

	ret = clsapi_wifi_set_hs20_status(ifname, "1");
	if (ret < 0) {
		cls_error("error: enable hs20, errcode %d", ret);
		return ret;
	}

	ret = clsapi_security_add_oper_friendly_name(ifname, "eng", "Wi-Fi Alliance");
	if (ret < 0) {
		cls_error("error: add_oper_friendly_name, errcode %d", ret);
		return ret;
	}

	ret = clsapi_security_add_oper_friendly_name(ifname, "chi", "Wi-Fi联盟");
	if (ret < 0) {
		cls_error("error: add_oper_friendly_name, errcode %d", ret);
		return ret;
	}

	ret = clsapi_security_add_roaming_consortium(ifname, "506F9A");
	if (ret < 0) {
		cls_error("error: add_roaming_consortium, errcode %d", ret);
		return ret;
	}

	ret = clsapi_security_add_roaming_consortium(ifname, "001BC504BD");
	if (ret < 0) {
		cls_error("error: add_roaming_consortium, errcode %d", ret);
		return ret;
	}

	ret = clsapi_wifi_set_hs20_params(ifname, "disable_dgaf", "1",
			       NULL, NULL, NULL, NULL, NULL);
	if (ret < 0) {
		cls_error("error: disable DGAF, errcode %d", ret);
		return ret;
	}

	ret = clsapi_wifi_set_hs20_params(ifname, "hs20_deauth_req_timeout", "20",
			       NULL, NULL, NULL, NULL, NULL);
	if (ret < 0) {
		cls_error("error: hs20_deauth_req_timeout, errcode %d", ret);
		return ret;
	}

	ret = clsapi_wifi_set_pmf(ifname, 1);
	if (ret < 0) {
		cls_error("error: set PMF, errcode %d", ret);
		return ret;
	}

	ret = clsapi_wifi_disable_wps(ifname, 1);
	if (ret < 0) {
		cls_error("error: disable WPS, errcode %d", ret);
		return ret;
	}

	return 0;
}

int cls_defconf_hs2_dut_all(void)
{
	int ret = 0;

	clsapi_wifi_remove_bss("wifi0_1");
	clsapi_wifi_remove_bss("wifi2_1");

	cls_defconf_hostapd_conf(0);

	ret = cls_defconf_hs2_dut("wifi0_0");
	if (ret < 0)
		return ret;

	ret = cls_defconf_hs2_dut("wifi2_0");
	if (ret < 0)
		return ret;

	return 0;
}

int cls_defconf_11n_dut(const char* ifname, const char *phy_mode)
{
	int ret;

	cls_log("cls_defconf_11n_dut, ifname %s, phy_mode %s", ifname, phy_mode);

	ret = cls_set_phy_mode(ifname, phy_mode);
	if (ret < 0) {
		cls_error("error: cannot set %s, errcode %d", phy_mode, ret);
		return ret;
	}

	/* Spatial streams, 4 */
	ret = cls_set_nss_cap(ifname, clsapi_mimo_ht, 4);
	if (ret < 0) {
		cls_error("error: cannot set NSS capability, errcode %d", ret);
		return ret;
	}

	/* restore automatic bandwidth selection */
	cls_set_fixed_bw(ifname, 0);

	/* Bandwidth, 40Mhz */
	ret = clsapi_wifi_set_bw(ifname, clsapi_bw_40MHz, "11n");
	if (ret < 0) {
		cls_error("error: cannot set bw capability %d, errcode %d", clsapi_bw_40MHz, ret);
		return ret;
	}

	/* enable dynamic GI selection */
	ret = clsapi_wifi_set_option(ifname, clsapi_GI_probing, 1);
	if (ret < 0) {
		/* not supported on RFIC6, ignore error for now. */
		cls_error("error: enable dynamic GI selection, errcode %d", ret);
	}

	/* enable short GI */
	ret = clsapi_wifi_set_option(ifname, clsapi_short_GI, 1);
	if (ret < 0) {
		cls_error("error: enable short GI, errcode %d", ret);
		return ret;
	}

	/* SU Transmit Beamformer, Off */
	ret = clsapi_wifi_set_option(ifname, clsapi_beamforming, 0);
	if (ret < 0) {
		cls_error("error: enable beamforming, errcode %d", ret);
		return ret;
	}

	/* Transmit A-MSDU, On
	 * Receive A-MPDU with A-MSDU, On
	 */
	ret = cls_set_amsdu(ifname, 1);
	if (ret < 0) {
		cls_error("error: enable tx amsdu, errcode %d", ret);
		return ret;
	}

	/* Tx/Rx STBC 2x1, On */
	ret = clsapi_wifi_set_option(ifname, clsapi_stbc, 1);
	if (ret < 0) {
		cls_error("error: cannot set stbc, errcode %d", ret);
		return ret;
	}

	return 0;
}

int cls_defconf_11n_testbed(const char* ifname)
{
	int ret;
	char tmpbuf[128];

	cls_log("%s, ifname %s", __FUNCTION__, ifname);

	ret = cls_set_phy_mode(ifname , "11na");
	if (ret < 0) {
		cls_error("error: cannot set 11na, errcode %d", ret);
		return ret;
	}

	/* Spatial streams, 2 */
	ret = cls_set_nss_cap(ifname, clsapi_mimo_ht, 2);
	if (ret < 0) {
		cls_error("error: cannot set NSS capability, errcode %d", ret);
		return ret;
	}

	/* Bandwidth, 40Mhz */
	cls_set_fixed_bw(ifname, 0);

	ret = clsapi_wifi_set_bw(ifname, clsapi_bw_40MHz, "11na");
	if (ret < 0) {
		cls_error("error: cannot set bw capability %d, errcode %d", clsapi_bw_40MHz, ret);
		return ret;
	}

	/* disable dynamic GI selection */
	ret = clsapi_wifi_set_option(ifname, clsapi_GI_probing, 0);
	if (ret < 0) {
		/* not supported on RFIC6, ignore error for now. */
		cls_error("error: enable dynamic GI selection, errcode %d", ret);
	}

	/* disable short GI */
	ret = clsapi_wifi_set_option(ifname, clsapi_short_GI, 0);
	if (ret < 0) {
		cls_error("error: enable short GI, errcode %d", ret);
		return ret;
	}

	/* SU Transmit Beamformer, Off */
	ret = clsapi_wifi_set_option(ifname, clsapi_beamforming, 0);
	if (ret < 0) {
		cls_error("error: enable beamforming, errcode %d", ret);
		return ret;
	}

	/* Transmit A-MSDU, Off
	 * Receive A-MPDU with A-MSDU, Off
	 */
	ret = cls_set_amsdu(ifname, 0);
	if (ret < 0) {
		cls_error("error: enable tx amsdu, errcode %d", ret);
		return ret;
	}

	/* Tx/Rx STBC 2x1, Off*/
	ret = clsapi_wifi_set_option(ifname, clsapi_stbc, 0);
	if (ret < 0) {
		cls_error("error: cannot set stbc, errcode %d", ret);
		return ret;
	}

	/* 15. Tx LDPC, On
	 * 16. Rx LDPC, On
	 */
	snprintf(tmpbuf, sizeof(tmpbuf), "iw dev %s set_ldpc %d", ifname, 1);
	system(tmpbuf);

	return 0;
}

int cls_defconf_he_all(const char* ifname)
{
	char tmpbuf[64];
	struct cls_dut_config *conf;

	conf = cls_dut_get_config(ifname);
	if (!conf) {
		cls_error("error: cannot get config");
		return -EFAULT;
	}

	/* AMPDU, On */
	cls_set_ampdu(ifname, 1);

	/* WDS, off */
	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s wds %d", ifname, 0);
	system(tmpbuf);

	/* disable implicit BA */
	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s implicit_ba 0", ifname);
	system(tmpbuf);

	/* enable LDPC since in HE BCC works only for 20Mhz bw */
	snprintf(tmpbuf, sizeof(tmpbuf), "iw dev %s set_ldpc %d", ifname, 1);
	system(tmpbuf);

	/* disable STBC since BBIC5 C0 does not support it */
	if (clsapi_wifi_set_option(ifname, clsapi_stbc, 0) < 0) {
		/* just report and ignore since it is not so important */
		cls_error("can't disable STBC on HE program");
	}

	/* disable VHT/HE caps for TKIP */
	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s set_vht_tkip %d", ifname, 0);
	system(tmpbuf);

//	Need set rfs_thresthold
//	clsapi_wifi_set_rts_threshold(ifname, IEEE80211_RTS_THRESH_OFF);

	/* set HE NDP GI+LFT to 0.8 2xLTF */
	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s he_ndp_gi_ltf 1", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "ofdma dbg_flg %s clear mu_bar", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s swba 0", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s swba 0x0700", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s mu_8sts_snd 0", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "ofdma set_mcs %s all all 0xff", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "ofdma set_group_size %s 0", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s he_txbf_bit_ctl %d",
						ifname, CLS_HE_SIG_TXBF_ENABLE_CBF_RECEIVED);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "ofdma set_ul_period %s 0", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "ofdma set_bsrp_period %s 0", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "ofdma auto_grp %s 0", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "ofdma set_grouping_mode %s 0", ifname);
	system(tmpbuf);

	/* only enable TWT for non testbed config */
	if (!conf->testbed_enable) {
		snprintf(tmpbuf, sizeof(tmpbuf), "twt %s enable 1", ifname);
		system(tmpbuf);

		snprintf(tmpbuf, sizeof(tmpbuf), "twt %s set_cap broadcast 0", ifname);
		system(tmpbuf);
	}

	snprintf(tmpbuf, sizeof(tmpbuf), "ofdma ul_ignore_nav %s 0", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "ofdma ul_ignore_ps %s 0", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "ofdma ul_duration_field %s -1", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s he_mtid_tx 0", ifname);
	system(tmpbuf);

	return 0;
}

int cls_defconf_he_testbed_sta(const char* ifname)
{
	int ret;
	struct cls_dut_config *conf;
	char tmpbuf[64];

	cls_log("%s, ifname %s", __FUNCTION__, ifname);

	const int is_2p4g = cls_is_2p4_interface(ifname);

	const char *phy_mode = is_2p4g ? "11axng" : "11ax";
	const int bw = is_2p4g ? clsapi_bw_20MHz : clsapi_bw_80MHz;


	ret = cls_set_phy_mode(ifname , phy_mode);
	if (ret < 0) {
		cls_error("error: cannot set 11ax, errcode %d", ret);
		return ret;
	}

	ret = cls_set_nss_cap(ifname, clsapi_mimo_he, 1);
	if (ret < 0) {
		cls_error("error: cannot set NSS capability, errcode %d", ret);
		return ret;
	}

	cls_set_fixed_bw(ifname, 0);
	ret = clsapi_wifi_set_bw(ifname, bw, phy_mode);
	if (ret < 0) {
		cls_error("error: cannot set bw capability %d, errcode %d", bw, ret);
		return ret;
	}

	/* 3. VHT MCS Set, 0-7 */
	cls_set_mcs_cap(ifname, IEEE80211_HE_MCS_0_7);

	/* disable dynamic GI selection */
	ret = clsapi_wifi_set_option(ifname, clsapi_GI_probing, 0);
	if (ret < 0) {
		/* ignore error since clsapi_GI_probing does not work for RFIC6 */
		cls_error("error: disable dynamic GI selection, errcode %d", ret);
	}

	/* disable short GI */
	ret = clsapi_wifi_set_option(ifname, clsapi_short_GI, 0);
	if (ret < 0) {
		cls_error("error: disable short GI, errcode %d", ret);
		return ret;
	}

	ret = clsapi_wifi_set_option(ifname, clsapi_beamforming, 0);
	if (ret < 0) {
		cls_error("error: disable beamforming, errcode %d", ret);
		return ret;
	}

	allow_mu_for_non_cls(ifname);
	ret = cls_set_mu_enable(ifname, 0);
	if (ret < 0) {
		cls_error("error: disable MU beamforming, errcode %d", ret);
		return ret;
	}

	ret = cls_set_amsdu(ifname, 0);
	if (ret < 0) {
		cls_error("error: disable tx amsdu, errcode %d", ret);
		return ret;
	}

	ret = clsapi_wifi_set_option(ifname, clsapi_stbc, 0);
	if (ret < 0) {
		cls_error("error: cannot set stbc, errcode %d", ret);
		return ret;
	}

	if (!is_2p4g) {
		/* Operating Mode Notification, Off, Transmit */
		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s set_vht_opmntf %d",
				ifname,
				0xFFFF);
		system(tmpbuf);
	}

	/* RTS with Bandwidth Signaling, Off */
	conf = cls_dut_get_config(ifname);
	if (conf) {
		reset_rts_cts_settings(conf, ifname);

	} else {
		ret = -EFAULT;
		cls_error("error: cannot get config, errcode %d", ret);
		return ret;
	}

	/* 18. Two-character Country Code, Off */
	/* 19. Transmit Power Control, Off */
	/* 20. Channel Switching, Off */

	cls_log("END: %s", __FUNCTION__);

	return cls_defconf_he_all(ifname);
}

static void cls_defconf_6ghz_security(const char *ifname)
{
	int ret;
	struct clsapi_set_parameters set_params;

	ret = clsapi_wifi_set_beacon_type(ifname, "11i");
	if (ret < 0)
		cls_error("can't set beacon_type to %s, error %d", "11i", ret);

	ret = clsapi_wifi_set_WPA_authentication_mode(ifname, "SAEAuthentication");
	if (ret < 0)
		cls_error("can't set authentication to %s, error %d", "SAEAuthentication", ret);

	ret = clsapi_wifi_set_WPA_encryption_modes(ifname, "AESEncryption");
	if (ret < 0)
		cls_error("can't set encryption to %s, error %d", "AESEncryption", ret);

	strncpy(set_params.param[0].key, "sae_pwe",
			sizeof(set_params.param[0].key) - 1);
// Need set SAE PWE H2E
/*	snprintf(set_params.param[0].value,
			sizeof(set_params.param[0].value), "%d", CLSAPI_SAE_PWE_H2E); */
	ret = clsapi_set_params(ifname, NULL, &set_params);
	if (ret < 0)
		cls_error("can't set rsnxe for keymgmt %s", "sae");
}

int cls_defconf_he_ap(const char *ifname)
{
	int ret = 0;
	char tmpbuf[128];
	int i;
	unsigned int radio_id = 0;
	enum clsapi_freq_band band_info;
	struct cls_dut_config *conf;

	/* NoAck policy, Off */
	const int stream_classes[] = { WME_AC_BE, WME_AC_BK, WME_AC_VI, WME_AC_VO };
	const int noackpolicy = 0;

	conf = cls_dut_get_config(ifname);
	if (!conf) {
		cls_error("error: cannot get config, errcode %d", ret);
		return -EFAULT;
	}

	for (int i = 0; i < ARRAY_SIZE(stream_classes); ++i) {
		ret = clsapi_wifi_qos_set_param(ifname, 2,
							stream_classes[i],
							IEEE80211_WMMPARAMS_NOACKPOLICY,
							0,
							noackpolicy);
		if (ret < 0) {
			cls_error("error: can't set noackpolicy to %d, error %d",
					noackpolicy,
					ret);
		}
	}

	snprintf(tmpbuf, sizeof(tmpbuf), "ofdma set_group_size %s 0", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "ofdma set_grouping_mode %s 0", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "ofdma set_group_ru_size %s 255", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "ofdma set_gi_ltf %s all 2", ifname);
	system(tmpbuf);

// Need set HE triger
/*	snprintf(tmpbuf, sizeof(tmpbuf), "ofdma set_ul_gi_ltf %s %d", ifname,
							HE_TRIGGER_CMNINFO_2LTF_SGI);  */
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "ofdma set_ul_period %s 0", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s muedca_to 0x000ff", ifname);
	system(tmpbuf);
	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s muedca_to 0x100ff", ifname);
	system(tmpbuf);
	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s muedca_to 0x200ff", ifname);
	system(tmpbuf);
	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s muedca_to 0x300ff", ifname);
	system(tmpbuf);
	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s mu_cwmax 0x0000f", ifname);
	system(tmpbuf);
	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s mu_cwmax 0x1000f", ifname);
	system(tmpbuf);
	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s mu_cwmax 0x2000f", ifname);
	system(tmpbuf);
	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s mu_cwmax 0x3000f", ifname);
	system(tmpbuf);
	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s mu_cwmin 0x0000f", ifname);
	system(tmpbuf);
	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s mu_cwmin 0x1000f", ifname);
	system(tmpbuf);
	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s mu_cwmin 0x2000f", ifname);
	system(tmpbuf);
	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s mu_cwmin 0x3000f", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s mu_aifs 0x00000", ifname);
	system(tmpbuf);
	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s mu_aifs 0x10000", ifname);
	system(tmpbuf);
	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s mu_aifs 0x20000", ifname);
	system(tmpbuf);
	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s mu_aifs 0x30000", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s txop_rtsthr %d",
						ifname, IEEE80211_IE_HEOP_RTS_THRESHOLD_DISABLED);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s he_mu_rts 0", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s 11v_mbssid 0", ifname);
	system(tmpbuf);

	if (!conf->testbed_enable) {
		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s omctl_umdrx 1", ifname);
		system(tmpbuf);

		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s ersudisable 0", ifname);
		system(tmpbuf);

		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s fullbw_ulmumimo 1", ifname);
		system(tmpbuf);
	}


	clsapi_get_radio_from_ifname(ifname, &radio_id);

	for (i = QNT_HE_MBSSID_IDX_START; i < QNT_HE_MBSSID_IDX_MAX; i++) {
		snprintf(tmpbuf, sizeof(tmpbuf), "wifi%d_%d", radio_id, i);
		clsapi_wifi_remove_bss(tmpbuf);
	}

	if (need_clear_reserve_all_slots) {
		need_clear_reserve_all_slots = 0;
		snprintf(tmpbuf, sizeof(tmpbuf), "mu %d dbg_flg clear reserve_all_slots",
										radio_id);
		system(tmpbuf);
	}
// Need set HTCAP value
/*	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s set_ampdu_dens %d",
							ifname, IEEE80211_HTCAP_MPDUSPACING_4); 
	system(tmpbuf); */

	snprintf(tmpbuf, sizeof(tmpbuf), "ofdma set_ul_trig_ppdu %s 0", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "ofdma set_lsig_len %s 0", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "ofdma set_mmsf %s 0", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s mu_use_su_prd 0", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "mu 2 dbg_flg clear mu_better");
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s mu_rts_cts 3", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "twt %s set_cap broadcast 0", ifname);
	system(tmpbuf);

	/* Reset number of subframes in A-MPDU to 64 by default */
	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s setparam 74 64", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "he_tx_cfg %d mu_ack bar", radio_id);
	system(tmpbuf);

	band_info = cls_get_sigma_band_info_from_interface(ifname);
	if (band_info == clsapi_freq_band_6_ghz)
		cls_defconf_6ghz_security(ifname);

	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s prembl_punc 0x%x", ifname, 0);
	system(tmpbuf);

	return ret;
}

int cls_defconf_he_testbed_ap(const char* ifname)
{
	int ret;
	struct cls_dut_config *conf;
	char tmpbuf[64];
	enum cls_dut_band_index band_idx;

	const int is_2p4g = cls_is_2p4_interface(ifname);
	const char *phy_mode = is_2p4g ? "11axng" : "11ax";
	const int bw = is_2p4g ? clsapi_bw_20MHz : clsapi_bw_80MHz;
	unsigned int radioid;

	clsapi_get_radio_from_ifname(ifname, &radioid);

	cls_log("%s, ifname %s", __FUNCTION__, ifname);

	ret = cls_set_phy_mode(ifname , phy_mode);
	if (ret < 0) {
		cls_error("error: cannot set 11ac, errcode %d", ret);
		return ret;
	}

	/* Spatial streams, 4 */
	ret = cls_set_nss_cap(ifname, clsapi_mimo_he, 4);
	if (ret < 0) {
		cls_error("error: cannot set NSS capability, errcode %d", ret);
		return ret;
	}

	/* Bandwidth, 80Mhz */
	cls_set_fixed_bw(ifname, 0);

	ret = clsapi_wifi_set_bw(ifname, bw, phy_mode);
	if (ret < 0) {
		cls_error("error: cannot set bw capability %d, errcode %d", bw, ret);
		return ret;
	}

	/* 3. VHT MCS Set, 0-7 */
	cls_set_mcs_cap(ifname, IEEE80211_HE_MCS_0_7);

	/* 4. Short GI for 20 MHz, Off, for both Tx/Rx
	 * 5. Short GI for 40 MHz, Off, for both Tx/Rx
	 * 6. Short GI for 80 MHz, Off, for both Tx/Rx
	 */

	/* disable dynamic GI selection */
	ret = clsapi_wifi_set_option(ifname, clsapi_GI_probing, 0);
	if (ret < 0) {
		/* ignore error since clsapi_GI_probing does not work for RFIC6 */
		cls_error("error: disable dynamic GI selection, errcode %d", ret);
	}

	/* disable short GI */
	ret = clsapi_wifi_set_option(ifname, clsapi_short_GI, 0);
	if (ret < 0) {
		cls_error("error: disable short GI, errcode %d", ret);
		return ret;
	}

	/* SU Transmit Beamforming, Off */
	/* SU Transmit Beamformee, Off */
	ret = clsapi_wifi_set_option(ifname, clsapi_beamforming, 0);
	if (ret < 0) {
		cls_error("error: disable beamforming, errcode %d", ret);
		return ret;
	}

	/* MU Transmit Beamformer, Off */
	/* MU Transmit Beamformee, 0ff */
	allow_mu_for_non_cls(ifname);
	ret = cls_set_mu_enable(ifname, 0);

	if (ret < 0) {
		cls_error("error: disable MU beamforming, errcode %d", ret);
		return ret;
	}

	/* Transmit A-MSDU, Off
	 * Receive A-MPDU with A-MSDU, Off
	 */
	ret = cls_set_amsdu(ifname, 0);
	if (ret < 0) {
		cls_error("error: disable tx amsdu, errcode %d", ret);
		return ret;
	}

	/* STBC 2x1 Transmit, Off
	 * STBC 2x1 Receive, Off
	 */
	ret = clsapi_wifi_set_option(ifname, clsapi_stbc, 0);
	if (ret < 0) {
		cls_error("error: cannot set stbc, errcode %d", ret);
		return ret;
	}

	if (!is_2p4g) {
		/* Operating Mode Notification, Off, Transmit */
		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s set_vht_opmntf %d",
				ifname,
				0xFFFF);
		system(tmpbuf);
	}

	/* RTS with Bandwidth Signaling, Off */
	conf = cls_dut_get_config(ifname);
	if (conf) {
		reset_rts_cts_settings(conf, ifname);
	} else {
		ret = -EFAULT;
		cls_error("error: cannot get config, errcode %d", ret);
		return ret;
	}

	/* set MU NDPA format to default. */
	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s set_mu_non_ht %d", ifname, 3);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "mu %d enable", radioid);
	system(tmpbuf);

	/* PMF should be disabled of testbed device by default */
	clsapi_wifi_set_pmf(ifname, 0);

	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s ppe_thresh 0", ifname);
	system(tmpbuf);

	cls_dut_disable_mu_bf(ifname);

	band_idx = cls_get_sigma_band_info_from_interface(ifname);
	cls_dut_backup_num_snd_dim(0, band_idx);

	snprintf(tmpbuf, sizeof(tmpbuf), "twt %s set_cap broadcast 0", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "twt %s enable 0", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s bsscolor 1", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s wfa_6e_ap_dflt 1", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s mu_edca 0", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s ersudisable 1", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s omctl_umdrx 0", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s su_bfmr 0", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s mu_bfmr 0", ifname);
	system(tmpbuf);

	/* 256 bit BA should be disabled by default for testbed AP */
	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s he_256ba_txrx 0x0", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s fullbw_ulmumimo 0", ifname);
	system(tmpbuf);

	ret = cls_defconf_he_ap(ifname);
	if (ret < 0)
		return ret;

	return cls_defconf_he_all(ifname);
}

static int local_get_snd_dim_from_rx_chains(const char *ifname)
{
	unsigned int rx_chains;
	int ret;

	ret = clsapi_wifi_get_rx_chains(ifname, &rx_chains);
	if (ret < 0)
		return 3;

	return ((rx_chains == 8) ? 7 : 3);
}

int cls_defconf_he_dut_sta(const char* ifname)
{
	int ret;
	struct cls_dut_config *conf;
	char tmpbuf[64];

	const int is_2p4g = cls_is_2p4_interface(ifname);
	const char *phy_mode = is_2p4g ? "11axng" : "11ax";
	const int bw = is_2p4g ? clsapi_bw_40MHz : clsapi_bw_80MHz;
	int snd_dim;

	cls_log("%s, ifname %s", __FUNCTION__, ifname);

	ret = cls_set_phy_mode(ifname , phy_mode);
	if (ret < 0) {
		cls_error("error: cannot set 11ac, errcode %d", ret);
		return ret;
	}

	/* 1. Spatial streams, 4 */
	ret = cls_set_nss_cap(ifname, clsapi_mimo_he, 4);

	if (ret < 0) {
		cls_error("error: cannot set NSS capability, errcode %d", ret);
		return ret;
	}

	/* 2. Bandwidth, 80Mhz */
	cls_set_fixed_bw(ifname, 0);

	ret = clsapi_wifi_set_bw(ifname, bw, phy_mode);
	if (ret < 0) {
		cls_error("error: cannot set bw capability %d, errcode %d", bw, ret);
		return ret;
	}

	/* 3. HE MCS Set, 0-11 */
	cls_set_mcs_cap(ifname, IEEE80211_HE_MCS_0_11);

	/* 4. Short GI for 20 MHz, Off, for both Tx/Rx
	 * 5. Short GI for 40 MHz, Off, for both Tx/Rx
	 * 6. Short GI for 80 MHz, Off, for both Tx/Rx
	 */

	/* enable dynamic GI selection */
	ret = clsapi_wifi_set_option(ifname, clsapi_GI_probing, 1);
	if (ret < 0) {
		/* not supported on RFIC6, ignore error for now. */
		cls_error("error: enable dynamic GI selection, errcode %d", ret);
	}

	/* enable short GI */
	ret = clsapi_wifi_set_option(ifname, clsapi_short_GI, 1);
	if (ret < 0) {
		cls_error("error: enable short GI, errcode %d", ret);
		return ret;
	}

	/* 7. SU Transmit Beamformer, On */
	/* 8. SU Transmit Beamformee, On */
	ret = clsapi_wifi_set_option(ifname, clsapi_beamforming, 1);
	if (ret < 0) {
		cls_error("error: enable beamforming, errcode %d", ret);
		return ret;
	}

	/* 9. MU Transmit Beamformer, Off */
	/* 10. MU Transmit Beamformee, 0ff */
	allow_mu_for_non_cls(ifname);
	ret = cls_set_mu_enable(ifname, 0);

	if (ret < 0) {
		cls_error("error: disable MU beamforming, errcode %d", ret);
		return ret;
	}

	/* 11. Transmit A-MSDU, On
	 * 12. Receive A-MPDU with A-MSDU, On
	 */
	ret = cls_set_amsdu(ifname, 1);
	if (ret < 0) {
		cls_error("error: disable tx amsdu, errcode %d", ret);
		return ret;
	}

	/* 13. Tx STBC 2x1, On
	 * 14. Rx STBC 2x1, On
	 */
	ret = clsapi_wifi_set_option(ifname, clsapi_stbc, 0);
	if (ret < 0) {
		cls_error("error: cannot set stbc, errcode %d", ret);
		return ret;
	}

	if (!is_2p4g) {
		/* 17. Operating Mode Notification, On (if supported) */
		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s set_vht_opmntf %d",
				ifname,
				0xFFFF);
		system(tmpbuf);
	}

	/* 18. RTS with Bandwidth Signaling, On (if supported) */
	conf = cls_dut_get_config(ifname);

	if (conf) {
		reset_rts_cts_settings(conf, ifname);
	} else {
		ret = -EFAULT;
		cls_error("error: cannot get config, errcode %d", ret);
		return ret;
	}

	snd_dim = local_get_snd_dim_from_rx_chains(ifname);

	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s set_snd_dim %d", ifname, snd_dim);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "mu %d enable", is_2p4g ? 2 : 0);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s ppe_thresh 63", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s wfa_6e_ap_dflt 0", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s mu_edca 0", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s ersudisable 0", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s su_bfmr 1", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s mu_bfmr 1", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "twt %s set_cap broadcast 0", ifname);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "twt %s enable 0", ifname);
	system(tmpbuf);

	/* 19. Two-character Country Code, On (if supported) */
	/* 20. Transmit Power Control, On (if supported) */
	/* 21. Channel Switching, On (if supported) */

	return cls_defconf_he_all(ifname);
}

int cls_defconf_he_dut_ap(const char* ifname)
{
	int ret;
	struct cls_dut_config *conf;
	char tmpbuf[64];

	const int is_2p4g = cls_is_2p4_interface(ifname);
	const char *phy_mode = is_2p4g ? "11axng" : "11ax";
	const int bw = is_2p4g ? clsapi_bw_20MHz : clsapi_bw_80MHz;
	int snd_dim;
	unsigned int radioid;

	clsapi_get_radio_from_ifname(ifname, &radioid);

	cls_log("%s, ifname %s", __FUNCTION__, ifname);

	ret = cls_set_phy_mode(ifname , phy_mode);
	if (ret < 0) {
		cls_error("error: cannot set 11ac, errcode %d", ret);
		return ret;
	}

	/* Spatial streams, 4 */
	ret = cls_set_nss_cap(ifname, clsapi_mimo_he, 4);

	if (ret < 0) {
		cls_error("error: cannot set NSS capability, errcode %d", ret);
		return ret;
	}

	/* Bandwidth, 80Mhz */
	cls_set_fixed_bw(ifname, 0);

	ret = clsapi_wifi_set_bw(ifname, bw, phy_mode);
	if (ret < 0) {
		cls_error("error: cannot set bw capability %d, errcode %d", bw, ret);
		return ret;
	}

	/* HE MCS Set, 0-11 */
	cls_set_mcs_cap(ifname, IEEE80211_HE_MCS_0_11);

	/* enable dynamic GI selection */
	ret = clsapi_wifi_set_option(ifname, clsapi_GI_probing, 1);
	if (ret < 0) {
		/* not supported on RFIC6, ignore error for now. */
		cls_error("error: enable dynamic GI selection, errcode %d", ret);
	}

	/* enable short GI */
	ret = clsapi_wifi_set_option(ifname, clsapi_short_GI, 1);
	if (ret < 0) {
		cls_error("error: enable short GI, errcode %d", ret);
		return ret;
	}

	/* SU Transmit Beamformer, Off */
	/* SU Transmit Beamformee, Off */
	ret = clsapi_wifi_set_option(ifname, clsapi_beamforming, 0);
	if (ret < 0) {
		cls_error("error: enable beamforming, errcode %d", ret);
		return ret;
	}

	/* MU Transmit Beamformer, Off */
	/* MU Transmit Beamformee, 0ff */
	allow_mu_for_non_cls(ifname);
	ret = cls_set_mu_enable(ifname, 0);

	if (ret < 0) {
		cls_error("error: disable MU beamforming, errcode %d", ret);
		return ret;
	}

	/* Transmit A-MSDU, On
	 * Receive A-MPDU with A-MSDU, On
	 */
	ret = cls_set_amsdu(ifname, 1);
	if (ret < 0) {
		cls_error("error: disable tx amsdu, errcode %d", ret);
		return ret;
	}

	ret = clsapi_wifi_set_option(ifname, clsapi_stbc, 0);
	if (ret < 0) {
		cls_error("error: cannot set stbc, errcode %d", ret);
		return ret;
	}

	if (!is_2p4g) {
		/* Operating Mode Notification, On (if supported) */
		snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s set_vht_opmntf %d",
				ifname,
				0xFFFF);
		system(tmpbuf);
	}

	/* RTS with Bandwidth Signaling, On (if supported) */
	conf = cls_dut_get_config(ifname);
	if (conf) {
		reset_rts_cts_settings(conf, ifname);
	} else {
		ret = -EFAULT;
		cls_error("error: cannot get config, errcode %d", ret);
		return ret;
	}

	snd_dim = local_get_snd_dim_from_rx_chains(ifname);
	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s set_snd_dim %d", ifname, snd_dim);
	system(tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "mu %d enable", radioid);
	system(tmpbuf);

	/* 256 bit BA should be enabled by default for APUT */
	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s he_256ba_txrx 0x03", ifname);
	system(tmpbuf);

	ret = cls_defconf_he_ap(ifname);
	if (ret < 0)
		return ret;

	return cls_defconf_he_all(ifname);
}

int cls_defconf_he_mbo_params(const char *ifname)
{
	int ret;
	char mstr[64], cmd[128];
	unsigned char addr[IEEE80211_ADDR_LEN];

	/* reset btm_delay to default 255 value */
	snprintf(cmd, sizeof(cmd), "iwpriv %s set_btm_delay 255", ifname);
	ret = system(cmd);
	if (ret != 0) {
		cls_error("error: cannot reset btm delay to default %d", ret);
		return ret;
	}

	/* reset assoc disallow to 0 */
	ret = clsapi_interface_get_mac_addr(ifname, addr);
	if (ret < 0) {
		cls_error("error: set assoc disallow: get macaddr failed");
		return ret;
	}
	snprintf(mstr, sizeof(mstr), "%02x:%02x:%02x:%02x:%02x:%02x",
		addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
/*	
#ifndef QDOCK2
	snprintf(cmd, sizeof(cmd), "%s test assoc_disallow %.17s 0", CLS_MBO_TEST_CLI, mstr);
	ret = system(cmd);
	if (ret != 0) {
		cls_error("error: failed to reset assoc disallow");
		return ret;
	}

	/* reset uns_btm_disassoc_imminent to 0 
	snprintf(cmd, sizeof(cmd), "%s set 0 uns_btm_disassoc_imminent 0", CLS_MBO_TEST_CLI);
	ret = system(cmd);
	if (ret != 0) {
		cls_error("error: failed to reset uns_btm_disassoc_imminent");
		return ret;
	}

	/* reset all steerable neighbor BSSes 
	snprintf(cmd, sizeof(cmd), "%s test wfa_neigh 0 del all", CLS_MBO_TEST_CLI);
	ret = system(cmd);
	if (ret != 0) {
		cls_error("error: failed to reset test neighbor BSSes");
		return ret;
	}
	cls_dut_set_mbo_auto_candidate_flag(1);
#endif
*/
	return 0;
}

int cls_defconf_he_dut_ap_radio(int radio_id)
{
	int ret = -EINVAL;
	static char ifname[IFNAMSIZ] = {0};

	if (clsapi_radio_get_primary_interface(radio_id, ifname, sizeof(ifname)) < 0)
		return ret;

	char status[32] = {0};

	if (clsapi_interface_get_status(ifname, status) < 0)
		return ret;

	ret = cls_defconf_he_dut_ap(ifname);
	if (ret < 0)
		return ret;

	ret = cls_defconf_he_mbo_params(ifname);
	if (ret < 0)
		return ret;

	return ret;
}

int cls_defconf_he_dut_ap_all()
{
	int ret = -EINVAL;
	static char ifname[IFNAMSIZ] = {0};

	for (int radio_id = 0; radio_id < CLS_MAX_RADIO_ID; ++radio_id) {
		if (clsapi_radio_get_primary_interface(radio_id, ifname, sizeof(ifname)) < 0)
			continue;

		char status[32] = {0};
		if (clsapi_interface_get_status(ifname, status) < 0)
			continue;

		ret = cls_defconf_he_dut_ap(ifname);
		if (ret < 0)
			return ret;

		ret = cls_defconf_he_mbo_params(ifname);
		if (ret < 0)
			return ret;
	}

	return ret;
}

int cls_defconf_wpa3_dut_all(const char *ifname)
{
	char phy_mode[32] = {0};
	int ret = clsapi_wifi_get_phy_mode(ifname, phy_mode);

	/* WAR: autorate_fallback should be invoked after set_phy_mode call */
	ret = cls_set_phy_mode(ifname, phy_mode);

	if (ret < 0)
		cls_error("error: cannot set %s, errcode %d", phy_mode, ret);

	return ret;
}

int cls_defconf_wpa3_dut_ap(const char *ifname)
{
	cls_log("%s, ifname %s", __func__, ifname);
	cls_defconf_wpa3_dut_all(ifname);
	cls_defconf_hostapd_conf(0);

	return 0;
}

int cls_defconf_wpa3_dut_sta(const char *ifname)
{
	cls_log("%s, ifname %s", __func__, ifname);

	cls_defconf_wpa_supplicant_conf(0);

	return 0;
}

static int cls_alloc_dpp_config(const char *ifname, struct cls_dut_config *conf)
{
	struct cls_dut_dpp_config *dpp_config;

	cls_log("%s, ifname %s, reset DPP config", __func__, ifname);

	if (conf->dpp_config)
		free(conf->dpp_config);

	dpp_config = calloc(1, sizeof(*dpp_config));
	if (!dpp_config) {
		cls_error("ifname %s: failed to allocate memory for dpp_config", ifname);
		return -ENOMEM;
	}

	conf->dpp_config = dpp_config;
	return 0;
}

int cls_defconf_dpp(const char *ifname)
{
	int ret;
	struct cls_dut_config *conf;
	clsapi_wifi_mode current_mode;

	cls_log("%s, ifname %s", __func__, ifname);

	ret = clsapi_wifi_get_mode(ifname, &current_mode);
	if (ret < 0) {
		cls_error("can't get mode, error %d", ret);
		return ret;
	}

	if (current_mode == clsapi_station)
		cls_defconf_wpa_supplicant_conf(1);
	else
		cls_defconf_hostapd_conf(1);

	conf = cls_dut_get_config(ifname);
	if (!conf)
		goto fail;

	if (cls_alloc_dpp_config(ifname, conf))
		goto fail;

	return 0;
fail:
	cls_error("error: cannot get config");
	return -EFAULT;
}

static void enable_bss_tm(const char *ifname)
{
	char tmpbuf[64];

	snprintf(tmpbuf, sizeof(tmpbuf), "iwpriv %s bss_tm 1", ifname);
	system(tmpbuf);
}

/* default mode configuration for MBO */
int cls_defconf_mbo_dut_ap(const char *ifname)
{
	int ret;
	unsigned int radio_id = 0;
	char region_name[16];
	char macstr[64], cmd[128];
	unsigned char macaddr[IEEE80211_ADDR_LEN];
	
	const int is_2p4g = cls_is_2p4_interface(ifname);
	const char *phy_mode = is_2p4g ? "11ng" : "11na";

	cls_log("%s, ifname %s", __func__, ifname);

	ret = clsapi_get_radio_from_ifname(ifname, &radio_id);
	if (ret < 0) {
		cls_error("error: cannot get radio id from ifname, errcode %d", ret);
		return ret;
	}

	ret = clsapi_wifi_scs_enable(ifname, 0);
	if (ret < 0) {
		cls_error("error: cannot disable SCS, error %d", ret);
		return ret;
	}

	/* spatial streams, 2 */
	ret = clsapi_wifi_set_nss_cap(ifname, clsapi_mimo_ht, 2);
	if (ret < 0) {
		cls_error("error: cannot set NSS capability, errcode %d", ret);
		return ret;
	}

	/* bandwidth, 20Mhz */
	ret = clsapi_wifi_set_bw(ifname, clsapi_bw_20MHz, phy_mode);
	if (ret < 0) {
		cls_error("error: cannot set bw capability %d, errcode %d", clsapi_bw_20MHz, ret);
		return ret;
	}

	/* enable short GI */
	ret = clsapi_wifi_set_option(ifname, clsapi_short_GI, 1);
	if (ret < 0) {
		cls_error("error: enable short GI, errcode %d", ret);
		return ret;
	}

	/* if region is none, reset to us as default */
	ret = clsapi_wifi_get_regulatory_region(ifname, region_name);
	if (ret < 0) {
		cls_error("error: cannot get regulatory region, errcode %d", ret);
		return ret;
	}
	if (!strcasecmp(region_name, "none")) {
		ret = clsapi_regulatory_set_regulatory_region(ifname, "us");
		if (ret < 0) {
			cls_error("error: cannot set regulatory region, errcode %d", ret);
			return ret;
		}
	}

	/* reset default channel */
	int ch = cls_get_sigma_default_channel(ifname);

	ret = clsapi_wifi_set_channel(ifname, ch);
	if (ret < 0)
		cls_error("error: can't set channel to %d, error %d", ch, ret);

	/* reset SSID to Wi-Fi */
	ret = clsapi_wifi_set_SSID(ifname, "Wi-Fi");
	if (ret < 0) {
		cls_error("error: cannot set SSID, errcode %d", ret);
		return ret;
	}

	/* reset passphrase to MBORocks */
	ret = clsapi_wifi_set_key_passphrase(ifname, "MBORocks");
	if (ret < 0) {
		cls_error("error: cannot set passphrase, errcode %d", ret);
		return ret;
	}

	/* reset PMF MFPC to 1, MFPR to 0 */
	ret = clsapi_wifi_set_pmf(ifname, 1);
	if (ret < 0) {
		cls_error("error: cannot enable pmf, errcode %d", ret);
		return ret;
	}

	/* reset btm_delay to 255 */
	snprintf(cmd, sizeof(cmd), "iwpriv %s set_btm_delay 255", ifname);
	ret = system(cmd);
	if (ret != 0) {
		cls_error("error: cannot reset btm delay to default %d", ret);
		return ret;
	}

	/* reset assoc disallow to 0 */
	ret = clsapi_interface_get_mac_addr(ifname, macaddr);
	if (ret < 0) {
		cls_error("error: set assoc disallow: get macaddr failed");
		return ret;
	}
	snprintf(macstr, sizeof(macstr), "%02x:%02x:%02x:%02x:%02x:%02x",
			macaddr[0], macaddr[1], macaddr[2],
			macaddr[3], macaddr[4], macaddr[5]);
/*
#ifndef QDOCK2
	snprintf(cmd, sizeof(cmd), "%s test assoc_disallow %.17s 0", CLS_MBO_TEST_CLI, macstr);
	ret = system(cmd);
	if (ret != 0) {
		cls_error("error: failed to reset assoc disallow");
		return ret;
	}

	/* reset uns_btm_disassoc_imminent to 0 
	snprintf(cmd, sizeof(cmd), "%s set 0 uns_btm_disassoc_imminent 0",
			CLS_MBO_TEST_CLI);
	ret = system(cmd);
	if (ret != 0) {
		cls_error("error: failed to reset uns_btm_disassoc_imminent");
		return ret;
	}

	/* reset all steerable neighbor BSSes 
	snprintf(cmd, sizeof(cmd), "%s test wfa_neigh 0 del all", CLS_MBO_TEST_CLI);
	ret = system(cmd);
	if (ret != 0) {
		cls_error("error: failed to reset test neighbor BSSes");
		return ret;
	}

	cls_dut_set_mbo_auto_candidate_flag(1);
#endif
*/
	/* enable bss tm */
	enable_bss_tm(ifname);

	return 0;
}

int cls_defconf_mbo_dut_ap_all(void)
{
	int ret = -EINVAL;

	cls_defconf_hostapd_conf(0);

	ret = cls_defconf_mbo_dut_ap("wifi0_0");
	if (ret < 0)
		return ret;

	ret = cls_defconf_mbo_dut_ap("wifi2_0");
	if (ret < 0)
		return ret;

	return 0;
}

static int cls_defconf_ffd_ap(const char *ifname)
{
	int ret = -EINVAL;
	char status[32] = {0};
	char phy_mode[32] = {0};
	enum clsapi_freq_band band_info = clsapi_freq_band_unknown;

	ret = clsapi_interface_get_status(ifname, status);
	if (ret < 0)
		return ret;

	ret = clsapi_wifi_get_phy_mode(ifname, phy_mode);
	if (ret < 0) {
		cls_error("can't get phy mode for interface %s, error %d", ifname, ret);
		return ret;
	}

	band_info = cls_get_sigma_band_info_from_interface(ifname);
	if (strstr(phy_mode, "11ax")) {
		ret = cls_defconf_he_dut_ap(ifname);
	} else {
		if (band_info == clsapi_freq_band_2pt4_ghz)
			ret = cls_defconf_11n_dut(ifname, "11ng");
		else
			ret = cls_defconf_vht_dut_ap(ifname);
	}

	return ret;
}

int cls_defconf_ffd(const char *ifname)
{
	int ret;
	clsapi_wifi_mode current_mode;

	cls_log("%s, ifname %s", __func__, ifname);

	ret = clsapi_wifi_get_mode(ifname, &current_mode);
	if (ret < 0) {
		cls_error("can't get mode, error %d", ret);
		return ret;
	}

	if (current_mode == clsapi_station)
		cls_defconf_wpa_supplicant_conf(1);
	else
		return cls_defconf_ffd_ap(ifname);

	/* FIXME: some configure maybe need to be done */
	return 0;
}

static int cls_easymesh_reset_supplicant_params(const char *ifname)
{
	char cmd[CLS_DEFCONF_CMDBUF_LEN];
	char conf[CLS_DEFCONF_CMDBUF_LEN];
	uint8_t ra_idx = atoi(&ifname[4]);

	snprintf(conf, CLS_DEFCONF_CMDBUF_LEN, "/mnt/jffs2/wpa_supplicant.conf.wifi%u", ra_idx);
	if (snprintf(cmd, CLS_DEFCONF_CMDBUF_LEN,
		"test -e /etc/default/wpa_supplicant.conf && "
		"cat /etc/default/wpa_supplicant.conf > %s && "
		"sed -i \'s/ssid=\"Clourneysemi\"/ssid=\"MAP-DEFAULT-STA\"/g' %s && "
		"wpa_cli -i%s reconfigure", conf, conf, ifname) <= 0) {
		cls_error("error: cannot format cmd, errno = %d", errno);
		return -1;
	}
	/* tweaks for check interface status and disable a-msdu */
	clsapi_interface_enable(ifname, 1);
	clsapi_wifi_set_tx_amsdu(ifname, 0);
	return system(cmd);
}

static int cls_easymesh_reset_hostapd_params(const char *ifname, uint8_t is_repeater)
{
	char cmd[CLS_DEFCONF_CMDBUF_LEN] = {0};
	char conf[40] = {0};
	uint8_t ra_idx = atoi(&ifname[4]);
	char interface[16] = {0};

	snprintf(interface, sizeof(interface), "wifi%u_%u", ra_idx, is_repeater);
	snprintf(conf, 40, "/mnt/jffs2/hostapd.conf.wifi%u", ra_idx);
	APPEND_CMD(cmd, sizeof(cmd),
			"test -e /etc/default/hostapd.conf && ");
	APPEND_CMD(cmd, sizeof(cmd),
			"cat /etc/default/hostapd.conf > %s && ", conf);
	APPEND_CMD(cmd, sizeof(cmd),
			"sed -i \'s/interface=wifi0/interface=%s/g' %s && ", interface, conf);
	APPEND_CMD(cmd, sizeof(cmd),
			"sed -i \'s/ssid=Clourneysemi/ssid=MAP-DEFAULT-AP/g' %s && ", conf);
	APPEND_CMD(cmd, sizeof(cmd),
			"sed -i \'s/ieee80211w=0/ieee80211w=1/g' %s && ", conf);
	APPEND_CMD(cmd, sizeof(cmd),
			"hostapd_cli -i%s reconfigure", interface);

	/* tweaks for check interface status and disable a-msdu */
	clsapi_interface_enable(interface, 1);
	clsapi_wifi_set_tx_amsdu(interface, 0);
	return system(cmd);
}

int cls_check_radio_available(uint8_t radio_idx, char *ifname, size_t ifname_len, uint32_t *p_band)
{
	uint32_t channel;
	int ret;

	if (clsapi_radio_get_primary_interface(radio_idx, ifname, ifname_len) < 0)
		return 0;

	ret = clsapi_wifi_get_chan(ifname, &channel, NULL/* bandwidth*/, p_band);
	if (ret < 0) {
		cls_error("error: get ifname(%s) band failed, ercode %d",
			ifname, ret);
		return 0;
	}
	cls_log("%s, ifname(%s) get band(%d) channel(%d)", __func__, ifname, *p_band, channel);

	return 1;
}

#define enable_restore_reset 0
int cls_defconf_easymesh(const char *ifname, int channel)
{
	char cmdbuf[CLS_DEFCONF_CMDBUF_LEN];
	uint8_t ra_idx = atoi(&ifname[4]);
	int is_repeater, ret;

	cls_log("%s, ifname %s", __func__, ifname);

	ret = clsapi_wifi_set_channel(ifname, channel);
	if (ret < 0) {
		cls_error("error: cannot set channel to %d, errcode %d",
			channel, ret);
		return ret;
	}

	/* WAR: when create MBSS, the beacon interval is scaled up, this will impact
	 * computes BTM request disassoc timer(WFA MAP-4.8.1),
	 * so disable dynamic beacon interval by default
	 */
	snprintf(cmdbuf, CLS_DEFCONF_CMDBUF_LEN, "iwpriv %s dyn_bcn_int 0", ifname);
	ret = system(cmdbuf);
	if (ret < 0)
		cls_error("error: can't disable dynamic beacon period for %s, errcode %d",
			ifname, ret);

	is_repeater = clsapi_radio_verify_repeater_mode(ra_idx);
	if (is_repeater != 1) {
#if enable_restore_reset
		if (snprintf(cmdbuf, CLS_DEFCONF_CMDBUF_LEN,
			"/scripts/restore_default_config -nr -i %s ap -d", ifname) <= 0) {
			cls_error("error: cannot format cmd, errno = %d", errno);
			return -1;
		}
		ret = system(cmdbuf);
#endif
		ret = cls_easymesh_reset_hostapd_params(ifname, 0);
		return ret;
	}
#if enable_restore_reset
	if (snprintf(cmdbuf, CLS_DEFCONF_CMDBUF_LEN,
		"/scripts/restore_default_config -nr -i %s repeater -d", ifname) <= 0) {
		cls_error("error: cannot format cmd, errno = %d", errno);
		return -1;
	}
	ret = system(cmdbuf);
#endif

	ret = cls_easymesh_reset_supplicant_params(ifname);
	if (!ret)
		ret = cls_easymesh_reset_hostapd_params(ifname, 1);

	return ret;
}
