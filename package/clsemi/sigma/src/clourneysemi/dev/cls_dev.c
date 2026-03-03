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

#include "cls_dev.h"
#include "common/csigma_log.h"
#include "common/csigma_tags.h"
#include "common/csigma_common.h"
#include "common/cls_ca_common.h"
#include "wfa_types.h"
#include "wfa_tlv.h"
#include "wfa_sock.h"

#include <cls/clsapi.h>

extern char gRespStr[];
extern int gCaSockfd;


int cls_ca_encode_dev_reset_default(char *cmd_str, unsigned char *tlv_buf, int *tlv_len)
{
	cls_log("%s", __func__);

	return cls_ca_encode_cmd(CSIGMA_DEV_RESET_DEFAULT_TAG, cmd_str, tlv_buf, tlv_len);
}

int cls_ca_encode_dev_get_parameter(char *cmd_str, unsigned char *tlv_buf, int *tlv_len)
{
	cls_log("%s", __func__);

	return cls_ca_encode_cmd(CSIGMA_DEV_GET_PARAMETER_TAG, cmd_str, tlv_buf, tlv_len);
}

int cls_ca_encode_dev_set_config(char *cmd_str, unsigned char *tlv_buf, int *tlv_len)
{
	cls_log("%s", __func__);

	return cls_ca_encode_cmd(CSIGMA_DEV_SET_CONFIG_TAG, cmd_str, tlv_buf, tlv_len);
}

int cls_ca_encode_dev_send_1905(char *cmd_str, unsigned char *tlv_buf, int *tlv_len)
{
	cls_log("%s", __func__);

	return cls_ca_encode_cmd(CSIGMA_DEV_SEND_1905_TAG, cmd_str, tlv_buf, tlv_len);
}

int cls_ca_encode_dev_set_rfeature(char *cmd_str, unsigned char *tlv_buf, int *tlv_len)
{
	cls_log("%s", __func__);

	return cls_ca_encode_cmd(CSIGMA_DEV_SET_RFEATURE_TAG, cmd_str, tlv_buf, tlv_len);
}

int cls_ca_respond_dev(unsigned char *tlv_buf)
{
	int ret;
	char out_buf[512];

	cls_log("%s", __FUNCTION__);

	ret = cls_ca_make_response(tlv_buf, out_buf, sizeof(out_buf));

	if (ret != WFA_SUCCESS)
		return ret;

	wfaCtrlSend(gCaSockfd, (unsigned char *)out_buf, strlen(out_buf));

	return WFA_SUCCESS;
}
