/*************************************************************************************
*******************************READ ME************************************************
*************************************************************************************/

Auto_gen rules:
(1)Please use this structure like below
struct a {
	int b;
	struct {
		int c;
		} d;
	struct e f;
}

(2)The use of array must follow below:
	[a]func(char *array_name, int array_name_len);
	[b]func(char array_name[], int array_name_len);
	[c]func(int array_name[], int array_name_len);

(3)Attention about functions :
	(a)One function must have comments like above
	(b)The parameter in comments must be the same with those are in the declaration
	(c)The api's name must be like clsapi_xxxx
	(d)If the api is related with "wifi",there must be "wifi" in the name,so as "net/sys"

(4) Parse uint8_t/int8_t/unsigned char/char parameters in C-Call APIs to BLOBMSG_TYPE_INT32 blobmsg_type, bool to BLOBMSG_TYPE_BOOL, since BLOBMSG_TYPE_INT8 is printed as Boolean value (true/false) in ubus CLI.


Known issues:
(1)3d array has not be supported yet
(2)2d structure array is not supported


All the code must be like this structure below
—————————————————————————————————————————————————————————————————————————————————————————
								HEADERS and MACRO

#ifndef _CLSAPI_WIFI_H
#define _CLSAPI_WIFI_H

#include <linux/if_ether.h>
#include "clsapi_common.h"

#define TID_MAX 9

#define cls_ubus_send_status(ctx, req, status) \
	do { \
		blob_buf_init(&buf, 0); \
		blobmsg_add_u32(&buf, CLSAPI_STR_STATUS, status); \
		ret = ubus_send_reply(ctx, req, buf.head); \
	} while (0)

/*Attention:
(1)The macro definition can be single line or multiple lines
(2)The macro definition must be uppercase*/
—————————————————————————————————————————————————————————————————————————————————————————


—————————————————————————————————————————————————————————————————————————————————————————
									ENUMERATEs

enum WPA_AUTHORIZE_TYPE {
	WPA_AUTHORIZE_TYPE_NONE = 0,
	WPA_AUTHORIZE_TYPE_WPA,
	WPA_AUTHORIZE_TYPE_WPA2,
	WPA_AUTHORIZE_TYPE_SAE
};
—————————————————————————————————————————————————————————————————————————————————————————

—————————————————————————————————————————————————————————————————————————————————————————
									STRUCTUREs

struct vbss_vap_info {
	uint8_t bssid[ETH_ALEN];
	string_32 ssid;
	string_32 ifname;
	enum WPA_AUTHORIZE_TYPE auth_type;
	string_32 pwd;
};
/*Attention: structure parameters must be written like above*/
—————————————————————————————————————————————————————————————————————————————————————————

—————————————————————————————————————————————————————————————————————————————————————————
									FUNCTIONs

/**
 * @brief Get on/off status of Wi-Fi VBSS.
 * \param phyname [in] The phy name of Wi-Fi radio,phy0, phy1, ...
 * \param onoff [out] on/off status of Wi-Fi VBSS.
 */
extern int clsapi_wifi_get_vbss_enabled(const char *phyname, bool *onoff);
—————————————————————————————————————————————————————————————————————————————————————————
