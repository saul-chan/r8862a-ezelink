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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* STUN message types */
#define STUN_MSG_TYPE_BINDING_REQUEST	0x0001
#define STUN_MSG_TYPE_BINDING_RESPONSE	0x0101
#define STUN_MSG_TYPE_BINDING_ERR_RESP	0x0111
#define STUN_MSG_TYPE_UDP_CONN_REQUEST	0x7FF0

#define TR069_UDP_CONN_REQUEST_MAGIC	"GET"

#define STUN_MSG_COOKIE_RFC5389			0x2112A442
#define STUN_MSG_COOKIE_RFC3489			0x0

/* STUN attributes */
#define STUN_ATTR_TYPE_CONN_REQ_BINDING	0xc001
#define STUN_ATTR_TYPE_BINDING_CHANGE	0xc002
#define STUN_ATTR_TYPE_MAPPED_ADDR		0x0001
#define STUN_ATTR_TYPE_RESPONSE_ADDR	0x0002
#define STUN_ATTR_TYPE_USERNAME			0x0006
#define STUN_ATTR_TYPE_MSG_INTEGRITY	0x0008
#define STUN_ATTR_TYPE_ERROR_CODE		0x0009


#define STUN_MAX_MSG_BUF_LEN			1024
//#define STUN_MAX_ATRR_VALUE_LEN		1024
#define STUN_ATTR_IPV4_ADDR_FMLY	0x01

#define STUN_ATTR_VALUE_CONN_REQ_BINDING "dslforum.org/TR-111"

typedef enum {
	CWMP_STUN_STATE_INIT = 0,
	CWMP_STUN_STATE_NAT_DISCOVER,
	CWMP_STUN_STATE_NAT_TIMEOUT_DISCV,
	CWMP_STUN_STATE_RUNNING,
	CWMP_STUN_STATE_BINDING_CHANGED,
	CWMP_STUN_STATE_MAX
} stun_state_t;

struct stun_header {
	unsigned short msg_type;
	unsigned short msg_len;
	unsigned int msg_cookie;
	unsigned char trans_id[12];
} __attribute__ ((__packed__));
typedef struct stun_header stun_hdr_t;


struct stun_attr {
	unsigned short type;
	unsigned short len;
	unsigned char *value;
} __attribute__ ((__packed__));
typedef struct stun_attr stun_attr_t;

struct attr_ipv4_addr {
	unsigned char	reserved;
	unsigned char	prot_family;
	unsigned short	port;
	unsigned int	ip;
} __attribute__ ((__packed__));
typedef struct attr_ipv4_addr attr_ipv4_addr_t;

typedef struct stun_req_attrs {
	char *username;
	char *password;
	struct sockaddr_in resp_addr;
} stun_req_attrs_t;

int tx_binding_request(int sock, struct sockaddr_in *server_addr, stun_req_attrs_t *req_attrs, int state);
short get_stun_msg_type(const unsigned char *msg, const unsigned int msg_len);
int get_mapped_addr(const unsigned char *msg, const unsigned int buf_len,
	const char *username, const unsigned char *password, struct sockaddr_in *mapped_addr);


