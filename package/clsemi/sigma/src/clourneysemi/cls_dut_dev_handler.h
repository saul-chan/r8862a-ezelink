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

#ifndef CLS_DUT_DEV_HANDLER_H_
#define CLS_DUT_DEV_HANDLER_H_

#define CLS_UBUS_CLI_PREFIX		"ubus call"

void cls_handle_dev_set_config(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out);

void cls_handle_dev_send_1905(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out);

void cls_handle_dev_reset_default(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out);

void cls_handle_dev_get_parameter(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out);

void cls_handle_dev_set_rfeature(int cmd_tag, int len, unsigned char *params, int *out_len,
		unsigned char *out);

void cls_handle_start_wps_registration(int cmd_tag, int len, unsigned char *params, int *out_len,
	unsigned char *out);

#endif

