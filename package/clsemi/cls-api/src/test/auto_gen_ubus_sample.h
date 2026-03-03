/* Automatically generated file; DO NOT EDIT. */
/*
 * Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */

#include <libubus.h>
#include <syslog.h>
#include <stdarg.h>
#include <stdlib.h>
#include <libubox/blobmsg.h>
#include <libubox/blobmsg_json.h>
#include "auto_gen_sample.h"

static char **result_str;
static uint8_t *result_bool;
static uint32_t *result_int;
static struct blob_buf buf;
static uint8_t *macint;

extern int blobmsg_check_array(const struct blob_attr *attr, int type);

static int get_dec_from_hex(char a)
{
    char *c = &a;
    switch (a){
    case '1' :
    case '2' :
    case '3' :
    case '4' :
    case '5' :
    case '6' :
    case '7' :
    case '8' :
    case '9' :
        return atoi(c);
    case 'a' :
        return 10;
    case 'b' :
        return 11;
    case 'c' :
        return 12;
    case 'd' :
        return 13;
    case 'e' :
        return 14;
    case 'f' :
        return 15;
    default:
        return 256;
    }
}

static void ubus_log(const char* format, ...)
{
    va_list vl;
    va_start(vl,format);
    vsyslog(4,format,vl);//warn level
    va_end(vl);
}

static uint8_t* handle_get_mac_string_to_int(char *macstr)
{
    char *temp = NULL;
    char token[2] = ":";
    int count = 1;

    macint = calloc(sizeof(int),6);
    temp = strtok(macstr, token);
    macint[0] = get_dec_from_hex(*temp)*16 + get_dec_from_hex(*(temp+1));
    while(temp != NULL) {
        temp = strtok(NULL, token);
        if ( temp !=  NULL) {
            macint[count] = get_dec_from_hex(*temp)*16 + get_dec_from_hex(*(temp+1));
            count++;
        }
    }
    return macint;
}

static char *handle_get_mac_int_to_string(uint8_t mac_addr[])
{
    char temp[3];
    char *mac_str = calloc(sizeof(char),18);
    memset(mac_str, 0, 18);
    for(int i = 0; i < 6; i++){
        sprintf(temp, "%x", mac_addr[i]);
        if ( i == 0)
            strcpy(mac_str, temp);
        else {
            strcat(mac_str,":");
            strcat(mac_str, temp);
            }
    }
    strcat(mac_str,"\0");
    return mac_str;
}

static void* blobmsg_get_string_array(struct blob_attr *msg)
{
    int type, rem, length, i;
    struct blob_attr *attr;

    length = blobmsg_check_array(msg, type);
    result_str = calloc(sizeof(1), length);
    type = blobmsg_type(msg);

    blobmsg_for_each_attr(attr, msg, rem )
    {
        result_str[i++] = blobmsg_get_string(attr);
    }
    return result_str;
}

static void* blobmsg_get_bool_array(struct blob_attr *msg)
{
    int type, rem, length, i;
    struct blob_attr *attr;

    length = blobmsg_check_array(msg, type);
    result_bool = (uint8_t *)calloc(sizeof(1), length);
    type = blobmsg_type(msg);

    blobmsg_for_each_attr(attr, msg, rem )
    {
        result_bool[i++] = blobmsg_get_u8(attr);
    }
    return result_bool;
}

static void* blobmsg_get_integer_array(struct blob_attr *msg)
{
    int type, rem, length, i;
    struct blob_attr *attr;

    length = blobmsg_check_array(msg, type);
    result_int = calloc(sizeof(4), length);
    type = blobmsg_type(msg);

    blobmsg_for_each_attr(attr, msg, rem )
    {
        result_int[i++] = blobmsg_get_u32(attr);
    }
    return result_int;
}

static void blobmsg_add_string_array(char *array_name, char *array[], int array_len)
{
    void *p;

    p = blobmsg_open_array(&buf, array_name);
    for (int i = 0; i < array_len; i++) {
        blobmsg_add_string(&buf, NULL, array[i]);
    }
    blobmsg_close_array(&buf, p);
}

static void blobmsg_add_uint8_t_array(char *array_name, uint8_t array[], int array_len)
{
    void *p;

    p = blobmsg_open_array(&buf, array_name);
    for (int i = 0; i < array_len; i++) {
        blobmsg_add_u32(&buf, NULL, array[i]);
    }
    blobmsg_close_array(&buf, p);
}

static void blobmsg_add_uint16_t_array(char *array_name, uint16_t array[], int array_len)
{
    void *p;

    p = blobmsg_open_array(&buf, array_name);
    for (int i = 0; i < array_len; i++) {
        blobmsg_add_u32(&buf, NULL, array[i]);
    }
    blobmsg_close_array(&buf, p);
}

static void blobmsg_add_uint32_t_array(char *array_name, uint32_t array[], int array_len)
{
    void *p;

    p = blobmsg_open_array(&buf, array_name);
    for (int i = 0; i < array_len; i++) {
        blobmsg_add_u32(&buf, NULL, array[i]);
    }
    blobmsg_close_array(&buf, p);
}

static void blobmsg_add_uint64_t_array(char *array_name, uint64_t array[], int array_len)
{
    void *p;

    p = blobmsg_open_array(&buf, array_name);
    for (int i = 0; i < array_len; i++) {
        blobmsg_add_u64(&buf, NULL, array[i]);
    }
    blobmsg_close_array(&buf, p);
}

enum {
	GET_NET_CLIENT_NAME_MAC,
	__GET_NET_CLIENT_NAME_MAX,
};

static const struct blobmsg_policy get_net_client_name_policy[__GET_NET_CLIENT_NAME_MAX] = {
	[GET_NET_CLIENT_NAME_MAC] = { .name = "mac", .type = BLOBMSG_TYPE_STRING },
};

static int ubus_get_net_client_name(struct ubus_context *ctx, struct ubus_object *obj,
                    struct ubus_request_data *req, const char *method,
                    struct blob_attr *msg)
{
    int ret = CLSAPI_OK;
    struct blob_attr *tb[__GET_NET_CLIENT_NAME_MAX];
	char name[128];

	blobmsg_parse(get_net_client_name_policy, __GET_NET_CLIENT_NAME_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[GET_NET_CLIENT_NAME_MAC])
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = clsapi_get_net_client_name(blobmsg_get_string(tb[GET_NET_CLIENT_NAME_MAC]), name, sizeof(name));

	if (ret < 0)
		goto out;

	cls_ubus_send_basic_result(ctx, req, ret, BLOBMSG_TYPE_STRING, name);

out:
	return ret;
}

struct ubus_method auto_gen_sample_methods[] = {
	UBUS_METHOD("get_client_name",	ubus_get_net_client_name,	get_net_client_name_policy),
};

