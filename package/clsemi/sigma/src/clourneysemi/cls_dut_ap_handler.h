/****************************************************************************
*
* Copyright (c) 2023  Clourney Semiconductor Co.,Ltd.
*
* Permission to use, copy, modify, and/or distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
* WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
* SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
* RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
* NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
* USE OR PERFORMANCE OF THIS SOFTWARE.
*
*****************************************************************************/

#ifndef CLS_DUT_AP_HANDLER_H_
#define CLS_DUT_AP_HANDLER_H_

#define CLS_SIGMA_TEST_IF_5G		"wifi0_0"
#define CLS_SIGMA_TEST_IF_24G		"wifi2_0"
#define CLS_CHAN_2GHZ_FIRST		1
#define CLS_CHAN_2GHZ_LAST		14      /* Chan 14 has non-std offset */
#define CLS_CHAN_4GHZ_FIRST		184
#define CLS_CHAN_4GHZ_LAST		196
#define CLS_CHAN_5GHZ_FIRST		36
#define CLS_CHAN_5GHZ_LAST		169
#define CLS_CHAN_6GHZ_FIRST		1
#define CLS_CHAN_6GHZ_LAST		253

#define	IEEE80211_TU_TO_USEC(x)		((x) * 1024)


/*
 * For 6G band, we need to use freq intead of channel number to determine the band,
 * Suppose the alliance will change the WFA test script to meet this requirement.
 */
#define CLS_CHAN_IS_IN_2G_BAND(_chan) \
	(((_chan) >= CLS_CHAN_2GHZ_FIRST) && ((_chan) <= CLS_CHAN_2GHZ_LAST))
#define CLS_CHAN_IS_IN_5G_BAND(_chan) \
	(((_chan) >= CLS_CHAN_5GHZ_FIRST) && ((_chan) <= CLS_CHAN_5GHZ_LAST))


void cls_handle_ap_get_info(int len, unsigned char *params, int *out_len, unsigned char *out);
void cls_handle_ap_set_radius(int len, unsigned char *params, int *out_len, unsigned char *out);
void cls_handle_ap_set_wireless(int len, unsigned char *params, int *out_len, unsigned char *out);
void cls_handle_ap_set_security(int len, unsigned char *params, int *out_len, unsigned char *out);
void cls_handle_ap_reset(int len, unsigned char *params, int *out_len, unsigned char *out);
void cls_handle_ca_version(int tag, int len, unsigned char *params, int *out_len,
	unsigned char *out);
void cls_handle_unknown_command(int tag, int len, unsigned char *params, int *out_len,
	unsigned char *out);
void cls_handle_ap_reset_default(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out);
void cls_handle_ap_set_11n_wireless(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out);
void cls_handle_ap_set_qos(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out);
void cls_handle_ap_config_commit(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out);
void cls_handle_ap_get_mac_address(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out);
#if 1
void cls_handle_ap_get_parameter(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out);
void cls_handle_ap_set_hs2(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out);
void cls_handle_ap_deauth_sta(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out);
void cls_handle_ap_set_11d(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out);
void cls_handle_ap_set_11h(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out);
void cls_handle_ap_set_rfeature(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out);
void cls_handle_ap_set_pmf(int len, unsigned char *params, int *out_len, unsigned char *out);
void cls_handle_ap_send_addba_req(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out);
void cls_handle_ap_preset_testparameters(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out);
void cls_handle_device_get_info(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out);
#endif
#endif				/* CLS_DUT_AP_HANDLER_H_ */
