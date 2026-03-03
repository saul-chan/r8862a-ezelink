/*
 * Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */

#ifndef __RESTF_H__
#define __RESTF_H__


#include <libubox/blobmsg.h>
#include <libubox/blobmsg_json.h>
#include <libubox/avl.h>
#include <libubox/avl-cmp.h>
#include <stdio.h>
#include <poll.h>
#include <uci.h>

#include "uhttpd.h"
#include "plugin.h"

/*!
 * @brief make up header
 * @param[in] cl HTTP client
 * @param[in] code HTTP response code
 * @param[in] summary Sumerry response string
 * @param[in] content_type Response type
 */
void uh_restf_send_header(struct client *cl, int code, const char *summary, const char *content_type);
void uh_restf_send_response(struct client *cl, struct blob_buf *buf);


/*Define restful_api structure
 */
typedef struct restful_api {
	/* @brief	name	api's name	*/
	char *name;
	/* @brief	uh_restf_get	handle get method	*/
	int (*uh_restf_get)(void);
	/* @brief	uh_restf_post	handle post method	*/
	int (*uh_restf_post)(void);
	/* @brief	uh_restf_put	handle put method	*/
	int (*uh_restf_put)(void);
	/* @brief	uh_restf_patch	handle patch method	*/
	int (*uh_restf_patch)(void);
	/* @brief	uh_restf_delete	handle delete method*/
	int (*uh_restf_delete)(void);
	/* @brief	avl	avl_node to store every resource's name	*/
	struct avl_node avl;
} restapi;

enum error_code {
	REQ_SUCCESS = 200,
	REQ_RESOURCE_CREATED = 201,
	REQ_RESOURCE_ACCEPTABLE = 202,
	REQ_RESOURCE_NULL = 204,
	REQ_RESOURCE_MOVED = 301,
	REQ_OTHERS = 303,
	REQ_RESOURCE_NOT_MODIFIED = 304,
	REQ_RESOURCE_BAD = 400,
	REQ_RESOURCE_NOT_FOUND = 404,
	REQ_NOT_ACCEPTABLE = 406,
	REQ_CONFLICT = 409,
	REQ_PRECONDITION_FAILED = 412,
	REQ_UNSUPPORTED = 415,
	REQ_INTERNAL_SERVER_ERROR = 500,
	REQ_SERVICE_UNAVALIABLE = 503,
};

enum {
	RPC_SESSION_ID,
	__RPC_SESSION_MAX,
};

static const struct blobmsg_policy rpc_sid_policy[__RPC_SESSION_MAX] = {
	[RPC_SESSION_ID] = { .name = "rest_token", .type = BLOBMSG_TYPE_STRING },
};

#endif
