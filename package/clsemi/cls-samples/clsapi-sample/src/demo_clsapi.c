/*
 * Copyright (c) 2021-2024, Clourney Semiconductor. All rights reserved.
 *
 * This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
 * Reproduction and redistribution in binary or source form, with or without modification,
 * for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
 * must retain the above copyright notice.
 *
 * By using this software and/or documentation, you agree to the limited terms and conditions.
 */

#include "clsapi_wifi.h"
#include "clsapi_base.h"

#define SET_SSID "set_ssid"
#define GET_SSID "get_ssid"
#define SET_CHANNEL "set_channel"
#define GET_CHANNEL "get_channel"

/* @brief Introduction to how to use demo_clsapi
 * @details Command usage like below:
 * demo_clsapi set_ssid <ifname> <XXX_ssid>
 * demo_clsapi get_ssid <ifname>
 * demo_clsapi set_channel <ifname> <XXX_channel>
 * demo_clsapi get_channel <ifname>
 */
static void show_usage(void)
{
	fprintf(stderr,
		"Usage:\n"
		"	demo_clsapi set_ssid <ifname> <XXXX_ssid>\n"
		"	demo_clsapi get_ssid <ifname>\n"
		"	demo_clsapi set_channel <ifname> <XXX_channel>\n"
		"	demo_clsapi get_channel <ifname>\n"
	);
}
/* @brief Entrance to the demo_clsapi
 * @details Be responsible for parameters parsing and analysis,
 * get in correct actions according to different parameters.
 * @return return 0 if success
 */
int main(int argc, char *argv[])
{
	int ret = CLSAPI_OK;
	string_32 error_msg = {0};
	enum clsapi_wifi_band band = CLSAPI_BAND_DEFAULT;
	enum clsapi_wifi_bw bw = CLSAPI_WIFI_BW_DEFAULT;

	if (argc == 4) {
		if (!strcmp(argv[1], SET_SSID))
			ret = clsapi_wifi_set_ssid(argv[2], argv[3]);
		else if (!strcmp(argv[1], SET_CHANNEL))
			ret = clsapi_wifi_set_channel(argv[2], atoi(argv[3]), band, bw);
		else {
			ret = -1;
			printf("ERROR: Unsupported option\n");
			goto error_out;
		}
		if (ret != CLSAPI_OK) {
			clsapi_base_get_str_err(ret, error_msg);
			printf("ERROR: %s\n", error_msg);
		} else
			printf("SUCCESS\n");

	} else if (argc == 3) {
		if (!strcmp(argv[1], GET_SSID)) {
			string_1024 ssid = {0};

			ret = clsapi_wifi_get_ssid(argv[2], ssid);
			if (ret == CLSAPI_OK)
				printf("ssid:\t%s\n", ssid);
			else {
				clsapi_base_get_str_err(ret, error_msg);
				printf("ERROR: %s\n", error_msg);
			}
		} else if (!strcmp(argv[1], GET_CHANNEL)) {
			uint8_t channel = 0;

			ret = clsapi_wifi_get_channel(argv[2], &channel);
			if (ret == CLSAPI_OK)
				printf("channel:\t%d\n", channel);
			else {
				clsapi_base_get_str_err(ret, error_msg);
				printf("ERROR: %s\n", error_msg);
			}
		} else {
			ret = -1;
			printf("ERROR: Unsupported option\n");
			goto error_out;
		}
	} else {
		ret = -1;
		printf("ERROR: Unsupported option\n");
		goto error_out;
	}

	return ret;

error_out:
	printf("ERROR: Wrong parameters\n");
	show_usage();
	return ret;
}
