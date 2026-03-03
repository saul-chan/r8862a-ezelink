/*
 *  Copyright (c) 2021-2024, Clourney Semiconductor. All rights reserved.
 *
 *  This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
 *  Reproduction and redistribution in binary or source form, with or without modification,
 *  for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
 *  must retain the above copyright notice.
 *
 *  By using this software and/or documentation, you agree to the limited terms and conditions.
 */

#include "cwmp/stun.h"

static long long unsigned transaction_id = 0x0026860000000000ull;

#define member_size(type, member) sizeof(((type *)0)->member)

#define get_msg_type(msg)	(ntohs(((stun_hdr_t *)msg)->msg_type))
#define get_msg_len(msg)	(ntohs(((stun_hdr_t *)msg)->msg_len))
#define get_msg_cookie(msg)	(ntohl(((stun_hdr_t *)msg)->msg_cookie))

#define get_attr_type(msg)	(ntohs(((stun_attr_t *)msg)->type))
#define get_attr_len(msg)	(ntohs(((stun_attr_t *)msg)->len))
#define get_attr_value(msg)	(&(((stun_attr_t *)msg)->value))

#define get_attr_tot_len(msg)	(member_size(stun_attr_t, type) + member_size(stun_attr_t, len) + get_attr_len(msg))


#define round_up_attr_len(attr_len)	((((attr_len)>>2)<<2) + 4*(attr_len&0x3))
#define attr_tot_len(attr_val_len)	(member_size(stun_attr_t, type) + member_size(stun_attr_t, len) \
	+ attr_val_len)


/*
 * Per spec, attribute length is real length of the "Value" part, prior to padding. If length is not multiple of 4bytes,
 * padding is expected.
 * Params:
 *	attr_len: is the real length of attr_value, if length is fixed for some attributes, attr_len will be ignored
 */
int build_stun_attribute(unsigned char **pkt, unsigned int buf_len, unsigned short attr_type,
	const unsigned short attr_len, unsigned char *attr_value)
{
	int	ret = 0;
	int attr_val_len = -1, asm_len;
	stun_attr_t	*attr = (stun_attr_t *)(*pkt);

	switch (attr_type) {
		case STUN_ATTR_TYPE_CONN_REQ_BINDING:
			attr_val_len = round_up_attr_len(attr_len);
			if (attr_val_len > buf_len) {
				ret = -1;
				goto LOCAL_RETURN;
			}
		
			memcpy(&(attr->value), attr_value, attr_len);	
			break;
		case CWMP_STUN_STATE_BINDING_CHANGED:
			attr_val_len = 0;
			break;
		case STUN_ATTR_TYPE_USERNAME:
			attr_val_len = round_up_attr_len(attr_len);
			if (attr_val_len > buf_len) {
				ret = -1;
				goto LOCAL_RETURN;
			}

			memcpy(&(attr->value), attr_value, attr_len);
			/* "The padding bits are ignored, and may be any value" */

			break;

		case STUN_ATTR_TYPE_RESPONSE_ADDR:
		{
			struct sockaddr_in *addr_val = (struct sockaddr_in *)attr_value;
			attr_ipv4_addr_t *addr_attr = (attr_ipv4_addr_t *)(&(attr->value));
			attr_val_len = sizeof(attr_ipv4_addr_t);
			if (attr_val_len > buf_len) {
				ret = -1;
				goto LOCAL_RETURN;
			}
			addr_attr->reserved	= 0;
			addr_attr->prot_family	= STUN_ATTR_IPV4_ADDR_FMLY;
			addr_attr->port	= addr_val->sin_port;
			addr_attr->ip	= addr_val->sin_addr.s_addr;

			break;
		}
#if 0
		case STUN_ATTR_TYPE_MSG_INTEGRITY:
			/* MUST be the last one */

			break;
#endif
		default:
			cwmp_log_debug("unsupported attribute 0x%d\n", attr_type);
			ret = -1;
			goto LOCAL_RETURN;
	}

	asm_len = attr_tot_len(attr_val_len);
	attr->type	= htons(attr_type);
	attr->len	= htons(attr_val_len);
	*pkt += asm_len;

LOCAL_RETURN:
	if (ret < 0)
		return ret;

	return asm_len;
}

int build_stun_msg(unsigned char *msg_buf, unsigned int buf_len, unsigned short msg_type,
	 stun_req_attrs_t *req_attrs, int state)
{
	int	ret = 0;
	stun_hdr_t	*stun_hdr = (stun_hdr_t *)msg_buf;
	unsigned char	*pkt = (unsigned char *)(stun_hdr + 1);
	int	rest_len = buf_len - sizeof(stun_hdr_t);

	if (msg_buf == NULL || buf_len < sizeof(stun_hdr_t))
		return -1;

	ret = build_stun_attribute(&pkt, rest_len, STUN_ATTR_TYPE_CONN_REQ_BINDING, strlen(STUN_ATTR_VALUE_CONN_REQ_BINDING),
		(unsigned char *)STUN_ATTR_VALUE_CONN_REQ_BINDING);
	if (ret > 0)
		rest_len -= ret;

	if (state == CWMP_STUN_STATE_BINDING_CHANGED) {
		ret = build_stun_attribute(&pkt, rest_len, STUN_ATTR_TYPE_BINDING_CHANGE, 0, NULL);
		if (ret > 0)
			rest_len -= ret;
	}

	if (req_attrs->username && req_attrs->password) {
		ret = build_stun_attribute(&pkt, rest_len, STUN_ATTR_TYPE_USERNAME, strlen(req_attrs->username),
			(unsigned char *)req_attrs->username);
		if (ret > 0)
			rest_len -= ret;
	}

	/* 0x0002: RESPONSE-ADDRESS */
	if (req_attrs->resp_addr.sin_port != 0) {
		ret = build_stun_attribute(&pkt, rest_len, STUN_ATTR_TYPE_RESPONSE_ADDR,
			sizeof(req_attrs->resp_addr), (unsigned char *)(&(req_attrs->resp_addr)));
		if (ret > 0)
			rest_len -= ret;
	}

#if 0
	/* MESSAGE-INTEGRITY MUST be the last attribute */
	if (username && password) {
		attr = build_stun_attribute(STUN_ATTR_TYPE_MSG_INTEGRITY, strlen(password), password);
	}
#endif

	stun_hdr->msg_type	= htons(msg_type);
	stun_hdr->msg_len	= htons(buf_len - sizeof(stun_hdr_t) - rest_len);
	stun_hdr->msg_cookie= htonl(STUN_MSG_COOKIE_RFC5389);
	memset(stun_hdr->trans_id, 0, sizeof(stun_hdr->trans_id));
	*((long long unsigned int*)(stun_hdr->trans_id)) = transaction_id++;

	return buf_len - rest_len;
}


int tx_binding_request(int sock, struct sockaddr_in *server_addr, stun_req_attrs_t *req_attrs, int state)
{
	int ret = 0;
	unsigned char	msg_buf[STUN_MAX_MSG_BUF_LEN];

	ret = build_stun_msg(msg_buf, sizeof(msg_buf), STUN_MSG_TYPE_BINDING_REQUEST, req_attrs, state);
	if (ret < 0) {
		cwmp_log_debug("build_stun_msg() error %d\n", ret);
		return ret;
	}

	ret = sendto(sock, msg_buf, ret, 0, (struct sockaddr *)server_addr, sizeof(struct sockaddr_in));
	if (ret < 0) {
		cwmp_log_debug("sendto() error %d\n", ret);
		return ret;
	}

	return ret;
}


short get_stun_msg_type(const unsigned char *msg, const unsigned int msg_len)
{
	short msg_type = -1;
	char	*msg_str = (char *)msg;

	if (strncmp(msg_str, TR069_UDP_CONN_REQUEST_MAGIC, strlen(TR069_UDP_CONN_REQUEST_MAGIC)) == 0) {
		msg_type = STUN_MSG_TYPE_UDP_CONN_REQUEST;
	}
	else if (get_msg_type(msg) == STUN_MSG_TYPE_BINDING_REQUEST)
		msg_type = STUN_MSG_TYPE_BINDING_REQUEST;
	else if (get_msg_type(msg) == STUN_MSG_TYPE_BINDING_RESPONSE)
		msg_type = STUN_MSG_TYPE_BINDING_RESPONSE;
	else if (get_msg_type(msg) == STUN_MSG_TYPE_BINDING_ERR_RESP)
		msg_type = STUN_MSG_TYPE_BINDING_ERR_RESP;
	else
		return -1;

	/* validation check */

	cwmp_log_debug("%s() msg_type=0x%04x\n", __func__, msg_type);
	return msg_type;
}


int get_mapped_addr(const unsigned char *msg, const unsigned int buf_len,
	const char *username, const unsigned char *password, struct sockaddr_in *mapped_addr)
{
	unsigned char	*pkt	= (unsigned char *)msg;
	unsigned short	msg_type	= get_msg_type(msg);
	unsigned short	msg_len		= get_msg_len(msg);

	/* validation check */
	if (msg_type != STUN_MSG_TYPE_BINDING_RESPONSE) {
		cwmp_log_debug("Ignore pkt since %d is not Binding Response\n", msg_type);
		return -1;
	}
	if (msg_len > buf_len)
		return -1;

	/* get info from attributes */
	pkt += sizeof(stun_hdr_t);
	while (msg_len > 0) {
		unsigned short attr_type = get_attr_type(pkt);
		unsigned short	attr_tot_len = get_attr_tot_len(pkt);
		if (attr_type != STUN_ATTR_TYPE_MAPPED_ADDR) {
			pkt += attr_tot_len;
			msg_len -= attr_tot_len;
			cwmp_log_debug("Ignore attr: T-%x L-%x\n", attr_type, get_attr_len(pkt));
		}
		else {
			attr_ipv4_addr_t *attr_mapped_addr = (attr_ipv4_addr_t *)(get_attr_value(pkt));
			if (attr_mapped_addr->prot_family != STUN_ATTR_IPV4_ADDR_FMLY) {
				cwmp_log_debug("Ignore Mapped Address since it's NOT IPv4\n");
				return -1;
			}

			mapped_addr->sin_family	= AF_INET;
			mapped_addr->sin_port	= attr_mapped_addr->port;
			mapped_addr->sin_addr.s_addr = attr_mapped_addr->ip;
			cwmp_log_debug("Mapped addr: %s:%d\n\n", inet_ntoa(mapped_addr->sin_addr), ntohs(mapped_addr->sin_port));
			return 0;
		}
	}

	return -1;
}

