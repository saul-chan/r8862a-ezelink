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

#ifndef CLS_DEV_H
#define CLS_DEV_H

#include <stdint.h>

int cls_ca_respond_dev(unsigned char *tlv_buf);
int cls_ca_encode_dev_reset_default(char *cmd_str, unsigned char *tlv_buf, int *tlv_len);
int cls_ca_encode_dev_get_parameter(char *cmd_str, unsigned char *tlv_buf, int *tlv_len);
int cls_ca_encode_dev_set_config(char *cmd_str, unsigned char *tlv_buf, int *tlv_len);
int cls_ca_encode_dev_send_1905(char *cmd_str, unsigned char *tlv_buf, int *tlv_len);
int cls_ca_encode_dev_set_rfeature(char *cmd_str, unsigned char *tlv_buf, int *tlv_len);
#endif
