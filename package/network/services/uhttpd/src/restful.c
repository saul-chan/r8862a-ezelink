/*
 * Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
*/

#include "restful.h"
#include "restapi.h"

//macro definition zone
#define UH_UBUS_MAX_POST_SIZE	65536
#define conf (*_conf)
#define UBUS_INVOKE_TIMEOUT 60*60

static char* special_urls[] = {
	"ssh/enable",
	"ssh/disable",
	"vbss_2g/enable",
	"vbss_2g/disable",
	"vbss_5g/enable",
	"vbss_5g/disable"
};
//static global parameters
//The operations of uhttpd
static const struct uhttpd_ops *ops;
static struct config *_conf;

//The global buffer parameter which is from client to server
struct blob_buf c2s_buf;

//The global buffer parameter which is from server to client
struct blob_buf s2c_buf;

//The global avl_trees save all the nodes of restapi
static struct avl_tree avl_trees;

//The global context used to communicate with ubus bus
struct ubus_context *ctx;

//The particular ressource node will be executed
static restapi *res_node;

//The global url resource list save all the url information
static char url_res[3][128] = {};

enum cors_hdr {
	HDR_ORIGIN,
	HDR_ACCESS_CONTROL_REQUEST_METHOD,
	HDR_ACCESS_CONTROL_REQUEST_HEADERS,
	__HDR_MAX
};

enum common_policy {
	UBUS_VALUE,
	UBUS_VALUES,
	__COMMON_POLICY_MAX,
};

extern struct blobmsg_policy common_policy[];

/*!
 * @brief Control ssh enable or disable
 * @param[in] enable enable or disable
 * @return error code if fail
 */
static int uh_control_ssh(char* enable)
{
	int ret = 0;

	if (!strcmp(enable, "enable")) {
		ret = system("/etc/init.d/dropbear start");
	} else {
		ret = system("/etc/init.d/dropbear killclients");
		ret = system("/etc/init.d/dropbear stop");
	}

	if (ret == 0)
		return UBUS_STATUS_OK;
	else
		return UBUS_STATUS_SYSTEM_ERROR;
}

/*!
 * @brief Control ssh enable or disable
 * @param[in] enable enable or disable
 * @return error code if fail
 */
static int uh_control_vbss_2g(char* enable)
{
	int ret = 0;

	if (!strcmp(enable, "enable")) {
		ret = system("ubus call clsapi.wifi set_vbss_enabled '{\"phyname\":\"wlan0\",\"onoff\":true}'");
	} else {
		ret = system("ubus call clsapi.wifi set_vbss_enabled '{\"phyname\":\"wlan0\",\"onoff\":false}'");
	}

	if (ret == 0)
		return UBUS_STATUS_OK;
	else
		return UBUS_STATUS_SYSTEM_ERROR;
}

/*!
 * @brief Control ssh enable or disable
 * @param[in] enable enable or disable
 * @return error code if fail
 */
static int uh_control_vbss_5g(char* enable)
{
	int ret = 0;

	if (!strcmp(enable, "enable")) {
		ret = system("ubus call clsapi.wifi set_vbss_enabled '{\"phyname\":\"wlan1\",\"onoff\":true}'");
	} else {
		ret = system("ubus call clsapi.wifi set_vbss_enabled '{\"phyname\":\"wlan1\",\"onoff\":false}'");
	}

	if (ret == 0)
		return UBUS_STATUS_OK;
	else
		return UBUS_STATUS_SYSTEM_ERROR;
}

/*!
 * @brief The callback of handling the client ubus post request, get the result of check
 * @param[in] type The type of ubus request
 * @param[in] msg The message from the ubu object
 * @param[out] req Request of ubus
 * @return None
 */
static void uh_restf_handle_check_sid_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
	bool *allow = (bool *)req->priv;

	if (!msg)
		*allow = false;
	else
		*allow = true;
}

/*!
 * @brief The callback of handling the client ubus post request, add blobmsg to s2c_buf
 * @param[in] req Request of ubus
 * @param[in] type The type of ubus request
 * @param[in] msg The message from the ubu object
 * @return None
 */
static void uh_restf_handle_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
	if (!msg)
		return;

	struct blob_attr *cur;
	int rem;

	blob_buf_init(&s2c_buf, 0);
	blob_for_each_attr(cur, msg, rem)
		blobmsg_add_blob(&s2c_buf, cur);
}

/*!
 * @brief Check the session id and the ubus object/function is matched
 * @param[in] sid Session id
 * @param[in] obj Ubus object
 * @param[in] func The function of ubus object
 * @return True or false
 */
static bool uh_restf_handle_check_session_id(char *sid, char *obj, char *func)
{
	uint32_t id;
	int ret;
	bool result = false;

	blob_buf_init(&c2s_buf, 0);
	blobmsg_add_string(&c2s_buf, "ubus_rpc_session", sid);
	blobmsg_add_string(&c2s_buf, "object", obj);
	blobmsg_add_string(&c2s_buf, "function", func);
	ret = ubus_lookup_id(ctx, "session", &id);
	if (ret == 0) {
		ret = ubus_invoke(ctx, id, "access", c2s_buf.head, uh_restf_handle_check_sid_cb, &result, UBUS_INVOKE_TIMEOUT);
		if (ret)
			fprintf(stderr, "%s, line:%d, ubus_invoke failed, ret:%s\n", __func__, __LINE__, ubus_strerror(ret));
	} else
		fprintf(stderr, "%s, line:%d, ubus_invoke failed, ret:%s\n", __func__, __LINE__, ubus_strerror(ret));

	return result;

}
/*!
 * @brief Function called to add core headers
 * @param[in] cl HTTP client
 * @return None
 */
static void uh_restf_add_cors_headers(struct client *cl)
{
	struct blob_attr *tb[__HDR_MAX];
	static const struct blobmsg_policy hdr_policy[__HDR_MAX] = {
		[HDR_ORIGIN] = { "origin", BLOBMSG_TYPE_STRING },
		[HDR_ACCESS_CONTROL_REQUEST_METHOD] = { "access-control-request-method", BLOBMSG_TYPE_STRING },
		[HDR_ACCESS_CONTROL_REQUEST_HEADERS] = { "access-control-request-headers", BLOBMSG_TYPE_STRING },
	};

	blobmsg_parse(hdr_policy, __HDR_MAX, tb, blob_data(cl->hdr.head), blob_len(cl->hdr.head));

	if (!tb[HDR_ORIGIN])
		return;

	if (tb[HDR_ACCESS_CONTROL_REQUEST_METHOD]) {
		char *hdr = (char *) blobmsg_data(tb[HDR_ACCESS_CONTROL_REQUEST_METHOD]);

		if (strcmp(hdr, "GET") && strcmp(hdr, "POST") && strcmp(hdr, "PUT") && strcmp(hdr, "PATCH") && strcmp(hdr, "DELETE"))
			return;
	}

	ustream_printf(cl->us, "Access-Control-Allow-Origin: %s\r\n",
			blobmsg_get_string(tb[HDR_ORIGIN]));

	if (tb[HDR_ACCESS_CONTROL_REQUEST_HEADERS])
		ustream_printf(cl->us, "Access-Control-Allow-Headers: %s\r\n",
				blobmsg_get_string(tb[HDR_ACCESS_CONTROL_REQUEST_HEADERS]));

	ustream_printf(cl->us, "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n");
	ustream_printf(cl->us, "Access-Control-Allow-Credentials: true\r\n");
}

/*!
 * @brief Function called to send HTTP header
 * @param[in] cl HTTP client
 * @param[in] code HTTP response code
 * @param[in] summary HTTP response string
 * @param[in] content_type Response's type
 * @return None
 */
void uh_restf_send_header(struct client *cl, int code, const char *summary, const char *content_type)
{
	ops->http_header(cl, code, summary);

	if (conf.ubus_cors)
		uh_restf_add_cors_headers(cl);

	ustream_printf(cl->us, "Content-Type: %s\r\n", content_type);

	if (cl->request.method == UH_HTTP_MSG_OPTIONS)
		ustream_printf(cl->us, "Content-Length: 0\r\n");

	ustream_printf(cl->us, "\r\n");
}

/*!
 * @brief Complete the batch request
 * @param[in] cl The HTTP client
 * @return None
 */
static void uh_restf_complete_batch(struct client *cl)
{
	ops->chunk_printf(cl, "]");
	ops->request_done(cl);
}

/*!
 * @brief Free all the resource
 * @param[in] cl The HTTP client
 * @return None
 */
static void uh_restf_request_free(struct client *cl)
{
	struct dispatch_ubus *du = &cl->dispatch.ubus;

	blob_buf_free(&du->buf);
	uloop_timeout_cancel(&du->timeout);

	if (du->jsobj)
		json_object_put(du->jsobj);

	if (du->jstok)
		json_tokener_free(du->jstok);

	if (du->req_pending)
		ubus_abort_request(ctx, &du->req);
	if (du->url_path)
		free(du->url_path);
	du->url_path = NULL;
}

/*!
 * @brief Set next batch request
 * @param[in] timeout The uloop_timeout
 * @return None
 */
static void __uh_restf_next_batched_request(struct uloop_timeout *timeout)
{
	struct dispatch_ubus *du = container_of(timeout, struct dispatch_ubus, timeout);
	struct client *cl = container_of(du, struct client, dispatch.ubus);
	struct json_object *obj = du->jsobj;
	int len;

	len = json_object_array_length(obj);
	if (du->array_idx >= len)
		return uh_restf_complete_batch(cl);
}

/*!
 * @brief Set the callback function to the timeout.cb
 * @param[in] cl The HTTP client
 * @return None
 */
static void uh_restf_next_batched_request(struct client *cl)
{
	struct dispatch_ubus *du = &cl->dispatch.ubus;

	du->timeout.cb = __uh_restf_next_batched_request;
	uloop_timeout_set(&du->timeout, 1);
}

/*!
 * @brief Function called to send HTTP response
 * @param[in] cl HTTP client
 * @param[in] buf HTTP response buf
 * @return None
 */
void uh_restf_send_response(struct client *cl, struct blob_buf *buf)
{
	struct dispatch_ubus *du = &cl->dispatch.ubus;
	const char *sep = "";
	char *str;

	if (du->array && du->array_idx > 1)
		sep = ", ";

	str = blobmsg_format_json(buf->head, true);
	ops->chunk_printf(cl, "%s%s", sep, str);
	free(str);

	du->jsobj_cur = NULL;
	if (du->array)
		uh_restf_next_batched_request(cl);
	else
		return ops->request_done(cl);
}

/*!
 * @brief Function called to close all the file descriptor
 * @param[in] cl The HTTP client
 * @return None
 */
static void uh_restf_close_fds(struct client *cl)
{
	if (ctx->sock.fd < 0)
		return;

	close(ctx->sock.fd);
	ctx->sock.fd = -1;
}

/*!
 * @brief Function called to get HTTP request
 * @param[in] cl HTTP client
 * @param[in] data HTTP client send data
 * @param[in] len Length of send data
 * @return The length which server get
 */
static int uh_restf_data_send(struct client *cl, const char *data, int len)
{
	struct dispatch_ubus *du = &cl->dispatch.ubus;
	char *temp = NULL;

	if (du->jsobj || !du->jstok)
		goto error;

	du->post_len += len;
	if (du->post_len > UH_UBUS_MAX_POST_SIZE)
		goto error;

	du->jsobj = json_tokener_parse_ex(du->jstok, data, len);
	return len;

error:
	temp = "REQ_PRECONDITION_FAILED";
	blobmsg_add_string(&c2s_buf, "result", temp);
	uh_restf_send_header(cl, REQ_PRECONDITION_FAILED, temp, "application/json");
	uh_restf_send_response(cl, &c2s_buf);
	return 0;
}

/*!
 * @brief Function called to transfer code to msg
 * @param[in] code HTTP code
 * @return The msg which the code represent
 */
static int uh_restf_parse2msg(int code)
{
	switch (code) {
	case UBUS_STATUS_OK:
		return REQ_SUCCESS;
	case UBUS_STATUS_INVALID_COMMAND:
		return REQ_PRECONDITION_FAILED;
	case UBUS_STATUS_INVALID_ARGUMENT:
		return REQ_PRECONDITION_FAILED;
	case UBUS_STATUS_METHOD_NOT_FOUND:
		return REQ_RESOURCE_NOT_FOUND;
	case UBUS_STATUS_NOT_FOUND:
		return REQ_RESOURCE_NOT_FOUND;
	case UBUS_STATUS_NO_DATA:
		return REQ_RESOURCE_BAD;
	case UBUS_STATUS_PERMISSION_DENIED:
		return REQ_UNSUPPORTED;
	case UBUS_STATUS_TIMEOUT:
		return REQ_INTERNAL_SERVER_ERROR;
	case UBUS_STATUS_NOT_SUPPORTED:
		return REQ_SERVICE_UNAVALIABLE;
	case UBUS_STATUS_UNKNOWN_ERROR:
		return REQ_OTHERS;
	case UBUS_STATUS_CONNECTION_FAILED:
		return REQ_SERVICE_UNAVALIABLE;
	case UBUS_STATUS_NO_MEMORY:
		return REQ_INTERNAL_SERVER_ERROR;
	case UBUS_STATUS_PARSE_ERROR:
		return REQ_INTERNAL_SERVER_ERROR;
	case UBUS_STATUS_SYSTEM_ERROR:
		return REQ_INTERNAL_SERVER_ERROR;
	default:
		return REQ_OTHERS;
	}
	return code;
}

/*!
 * @brief Insert restapi node into avl_trees
 * @param[in] r Restapi node
 * @return None
 */
static void uh_restf_insert_avl_tree(restapi *r)
{
	r->avl.key = r->name;

	if (avl_insert(&avl_trees, &r->avl))
		fprintf(stderr, "%s %s insert failed \n", __func__, r->name);
}

/*!
 * @brief Judge if the url is ubus, parse url into url_res
 * @param[in] url The url is passed in
 * @return If url is ubus return true or false
 *	URL: 192.168.1.1/restful/wifi/get/radio_enabled
 */
static void uh_restf_parse_url(char *url)
{
	char *s = "/";
	char *token = strtok(url, s);
	int i = 0;

	while (token != NULL && i < 3) {
		strcpy(url_res[i], token);
		token = strtok(NULL, s);
		i++;
	}
}

/*!
 * @brief Handle the rest get request
 * @param[in] cl The client input
 * @return None
 */
static void uh_restf_handle_get(struct client *cl)
{
	int ret = 0;

	blob_buf_init(&s2c_buf, 0);
	if (res_node->uh_restf_get) {
		ret = res_node->uh_restf_get();
		uh_restf_send_header(cl, uh_restf_parse2msg(ret), ubus_strerror(ret), "application/json");
		uh_restf_send_response(cl, &s2c_buf);
	} else {
		uh_restf_send_header(cl, 404, "Method Not Found", "application/json");
		uh_restf_send_response(cl, &s2c_buf);
	}
}

/*!
 * @brief Handle the client ubus get request
 * @param[in] None
 * @return Error code from ubus
 */
static int uh_restf_handle_ubus_get(void)
{
	uint32_t id;
	int ret;
	char section[256] = {0};
	char method[256] = {0};

	snprintf(section, sizeof(section), "clsapi.%s", url_res[0]);
	if (strcmp(url_res[2], "\0") == 0)
		snprintf(method, sizeof(method), "%s", url_res[1]);
	else
		snprintf(method, sizeof(method), "%s_%s", url_res[1], url_res[2]);

	ret = ubus_lookup_id(ctx, section, &id);
	if (ret == 0) {
		ret = ubus_invoke(ctx, id, method, c2s_buf.head, uh_restf_handle_cb, NULL, UBUS_INVOKE_TIMEOUT);
		if (ret)
			fprintf(stderr, "%s, line:%d, ubus_invoke failed, ret:%s\n", __func__, __LINE__, ubus_strerror(ret));
	} else
		fprintf(stderr, "%s, line:%d, ubus_invoke failed, ret:%s  %s\n", __func__, __LINE__, ubus_strerror(ret), url_res[1]);

	return ret;
}
/*!
 * @brief Handle the client ubus post request
 * @param[in] None
 * @return Error code from ubus
 */
static int uh_restf_handle_ubus_post(void)
{
	uint32_t id;
	int ret;
	char section[256] = {0};
	char method[256] = {0};

	if (strcmp(url_res[0], "session") == 0)
		snprintf(section, sizeof(section), "%s", url_res[0]);
	else
		snprintf(section, sizeof(section), "clsapi.%s", url_res[0]);

	if (strcmp(url_res[2], "\0") == 0)
		snprintf(method, sizeof(method), "%s", url_res[1]);
	else
		snprintf(method, sizeof(method), "%s_%s", url_res[1], url_res[2]);
	ret = ubus_lookup_id(ctx, section, &id);
	if (ret == 0) {
		ret = ubus_invoke(ctx, id, method, c2s_buf.head, uh_restf_handle_cb, NULL, UBUS_INVOKE_TIMEOUT);
		if (ret)
			fprintf(stderr, "%s, line:%d, ubus_invoke failed, ret:%s\n", __func__, __LINE__, ubus_strerror(ret));
	} else
		fprintf(stderr, "%s, line:%d, ubus_invoke failed, ret:%s\n", __func__, __LINE__, ubus_strerror(ret));

	return ret;
}

/*!
 * @brief Handle the client ubus delete request
 * @param[in] None
 * @return Error code from ubus
 */
static int uh_restf_handle_ubus_delete(void)
{
	uint32_t id;
	int ret;
	char section[256] = {0};
	char method[256] = {0};

	snprintf(section, sizeof(section), "clsapi.%s", url_res[0]);
	if (strcmp(url_res[2], "\0") == 0)
		snprintf(method, sizeof(method), "%s", url_res[1]);
	else
		snprintf(method, sizeof(method), "%s_%s", url_res[1], url_res[2]);
	ret = ubus_lookup_id(ctx, section, &id);
	if (ret == 0) {
		ret = ubus_invoke(ctx, id, method, c2s_buf.head, uh_restf_handle_cb, NULL, UBUS_INVOKE_TIMEOUT);
		if (ret)
			fprintf(stderr, "%s, line:%d, ubus_invoke failed, ret:%s\n", __func__, __LINE__, ubus_strerror(ret));
	} else
		fprintf(stderr, "%s, line:%d, ubus_invoke failed, ret:%s\n", __func__, __LINE__, ubus_strerror(ret));
	return ret;

}

/*!
 * @brief Handle the client UBUS request except get
 * @param[in] cl HTTP client
 * @return None
 */
static void uh_restf_handle_ubus_method(struct client *cl)
{
	int num = REQ_PRECONDITION_FAILED;
	struct dispatch_ubus *du = &cl->dispatch.ubus;
	struct json_object *obj = du->jsobj;
	bool not_found = false;

	blob_buf_init(&s2c_buf, 0);
	if (json_type_object == json_object_get_type(obj)) {
		du->jsobj_cur = obj;
		blobmsg_add_object(&c2s_buf, obj);
	}
	switch (cl->request.method) {
		case UH_HTTP_MSG_POST:
			if (res_node->uh_restf_post)
				num = res_node->uh_restf_post();
			else
				not_found = true;
			break;
		case UH_HTTP_MSG_PUT:
			if (res_node->uh_restf_put)
				num = res_node->uh_restf_put();
			else
				not_found = true;
			break;
		case UH_HTTP_MSG_PATCH:
			if (res_node->uh_restf_patch)
				num = res_node->uh_restf_patch();
			else
				not_found = true;
			break;
		case UH_HTTP_MSG_DELETE:
			if (res_node->uh_restf_delete)
				num = res_node->uh_restf_delete();
			else
				not_found = true;
			break;
		default:
			break;
	}

	if (not_found == true) {
		uh_restf_send_header(cl, 404, "Method Not Found", "application/json");
		uh_restf_send_response(cl, &s2c_buf);
	} else {
		uh_restf_send_header(cl, uh_restf_parse2msg(num), ubus_strerror(num), "application/json");
		uh_restf_send_response(cl, &s2c_buf);
	}
}

/*!
 * @brief judge the URL if is special one
 * @param[in] the url string
 * @return true or false
 */
static bool uh_judge_special_url(char *url)
{
	int special_url_lens = 0;

	special_url_lens = sizeof(special_urls)/sizeof(special_urls[0]);
	for (int i = 0; i < special_url_lens; i++) {
		if (!strcmp(url, special_urls[i]))
			return true;
	}

	return false;
}

/*!
 * @brief the special URL special handle
 * @param[in] the url string
 * @return error if fail
 */
static int uh_special_url_handle(char *url)
{
	char *flag = "/";
	char *token = NULL;

	token = strtok(url, flag);
	url = strtok(NULL, flag);
	if (!strcmp(token, "ssh"))
		return uh_control_ssh(url);
	else if (!strcmp(token, "vbss_2g"))
		return uh_control_vbss_2g(url);
	else if (!strcmp(token, "vbss_5g"))
		return uh_control_vbss_5g(url);

	return UBUS_STATUS_SYSTEM_ERROR;
}
/*!
 * @brief Handle the client requests entrance
 * @param[in] cl HTTP client
 * @param[in] url The url client visit
 * @param[in] pi The path information
 * @return None
 */
static void uh_restf_handle_request(struct client *cl, char *url, struct path_info *pi)
{
	struct dispatch *d = &cl->dispatch;
	struct dispatch_ubus *du = &d->ubus;
	struct blob_attr *tb[__RPC_SESSION_MAX];
	char *chr;
	char url_comp[512];

	du->url_path = strdup(url);
	if (!du->url_path) {
		ops->client_error(cl, 500, "Internal Server Error", "Failed to allocate resources");
		return;
	}
	chr = strchr(du->url_path, '?');
	if (chr)
		chr[0] = '\0';

	du->legacy = false;
	url += strlen("/restful/");

	blob_buf_init(&c2s_buf, 0);
	blob_buf_init(&s2c_buf, 0);

	for (int i = 0; i < 3; i++) {
		if (url_res[i])
			memset(url_res[i], 0, sizeof(128));
	}

	//URL: 192.168.1.1/restful/wifi/get/radio_enabled
	char *temp_url = (char *)calloc(strlen(url), 1);
	strncpy(temp_url, url, strlen(url));
	uh_restf_parse_url(temp_url);
	free(temp_url);

	if (strcmp(url_res[2], "\0") == 0)
		sprintf(url_comp, "%s_%s", url_res[0], url_res[1]);
	else
		sprintf(url_comp, "%s_%s_%s", url_res[0], url_res[1], url_res[2]);

	res_node = avl_find_element(&avl_trees, url_comp, res_node, avl);
	//Judge if the request is login
	//URL : 192.168.1.1/restful/session/login
	if (!strcmp("session", url_res[0]) && !strcmp("login", url_res[1])) {
		if (res_node) {
			switch (cl->request.method) {
				case UH_HTTP_MSG_POST:
					d->data_send = uh_restf_data_send;
					d->data_done = uh_restf_handle_ubus_method;
					d->close_fds = uh_restf_close_fds;
					d->free = uh_restf_request_free;
					du->jstok = json_tokener_new();
					return;

				default:
					ops->client_error(cl, 400, "Bad Request", "Invalid Request");
			}
			free(du->url_path);
			du->url_path = NULL;
		} else {
			fprintf(stderr, "%s, not match any resource, please check again !\n", __func__);
			ops->client_error(cl, 404, "Not Found", "No Such Resource");
		}
		/*if something special*/
	} else if (uh_judge_special_url(url)) {
		int ret = uh_special_url_handle(url);

		blob_buf_init(&s2c_buf, 0);
		blobmsg_add_string(&s2c_buf, "result", ubus_strerror(ret));
		uh_restf_send_header(cl, 200, "OK", "application/json");
		uh_restf_send_response(cl, &s2c_buf);
	} else {
		blobmsg_parse(rpc_sid_policy, __RPC_SESSION_MAX, tb, blob_data(cl->hdr.head), blob_len(cl->hdr.head));
		if (tb[RPC_SESSION_ID]) {
			if (uh_restf_handle_check_session_id(blobmsg_get_string(tb[RPC_SESSION_ID]), "uci", "set")) {
				if (res_node) {
					blob_buf_init(&c2s_buf, 0);
					switch (cl->request.method) {
						case UH_HTTP_MSG_GET:
							uh_restf_handle_get(cl);
							break;
						case UH_HTTP_MSG_POST:
						case UH_HTTP_MSG_PUT:
						case UH_HTTP_MSG_PATCH:
						case UH_HTTP_MSG_DELETE:
							d->data_send = uh_restf_data_send;
							d->data_done = uh_restf_handle_ubus_method;
							d->close_fds = uh_restf_close_fds;
							d->free = uh_restf_request_free;
							du->jstok = json_tokener_new();
							return;

						default:
							ops->client_error(cl, 400, "Bad Request", "Invalid Request");
					}
					free(du->url_path);
					du->url_path = NULL;

				} else {
					fprintf(stderr, "%s, not match any resource, please check again !\n", __func__);
					ops->client_error(cl, 404, "Not Found", "No Such Resource");
				}
			} else {
				blob_buf_init(&s2c_buf, 0);
				blobmsg_add_string(&s2c_buf, "result", "Session id not match");
				uh_restf_send_header(cl, 403, "Forbidden", "application/json");
				uh_restf_send_response(cl, &s2c_buf);
			}
		} else {
			blob_buf_init(&s2c_buf, 0);
			blobmsg_add_string(&s2c_buf, "result", "Session id not exist");
			uh_restf_send_header(cl, 401, "Unauthorized", "application/json");
			uh_restf_send_response(cl, &s2c_buf);
		}
	}
}

static bool uh_restf_check_url(const char *url)
{
	return ops->path_match("/restful", url);
}

static int uh_restf_init(void)
{
	static struct dispatch_handler restf_dispatch = {
		.check_url = uh_restf_check_url,
		.handle_request = uh_restf_handle_request,
	};
	int i = 0;
	int len = 0;

	ctx = ubus_connect(conf.ubus_socket);
	if (!ctx) {
		fprintf(stderr, "Unable to connect to ubus socket\n");
		exit(1);
	}

	avl_init(&avl_trees, avl_strcmp, false, NULL);
	len = sizeof(resources)/sizeof(restapi);
	for (; i < len; i++)
		uh_restf_insert_avl_tree(&resources[i]);

	ops->dispatch_add(&restf_dispatch);
	uloop_done();
	return 0;
}

static void uh_restf_post_init(void)
{
	ubus_add_uloop(ctx);
}

static int uh_restf_plugin_init(const struct uhttpd_ops *o, struct config *c)
{
	ops = o;
	_conf = c;
	return uh_restf_init();
}

struct uhttpd_plugin uhttpd_plugin = {
	.init = uh_restf_plugin_init,
	.post_init = uh_restf_post_init,
};
