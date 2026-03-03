/*
 *  Copyright (c) 2021-2025, Clourney Semiconductor. All rights reserved.
 *
 *  This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
 *  Reproduction and redistribution in binary or source form, with or without modification,
 *  for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
 *  must retain the above copyright notice.
 *
 *  By using this software and/or documentation, you agree to the limited terms and conditions.
 */

#ifndef __CLS_LED_H__
#define __CLS_LED_H__

#include <stdio.h>
#include <uloop.h>
#include <stdlib.h>
#include <string.h>

#include <libubus.h>
#include <libubox/blobmsg_json.h>

#define LED_NUMBER 3
#define BOARD_INFO_PATH "/tmp/sysinfo/board_name"

#define RED_LED_TRIGGER_PATH "/sys/class/leds/red:status/trigger"
#define RED_LED_BRIGHTNESS_PATH "/sys/class/leds/red:status/brightness"
#define RED_LED_DELAY_OFF_PATH "/sys/class/leds/red:status/delay_off"
#define RED_LED_DELAY_ON_PATH "/sys/class/leds/red:status/delay_on"

#define GREEN_LED_TRIGGER_PATH "/sys/class/leds/green:status/trigger"
#define GREEN_LED_BRIGHTNESS_PATH "/sys/class/leds/green:status/brightness"
#define GREEN_LED_DELAY_OFF_PATH "/sys/class/leds/green:status/delay_off"
#define GREEN_LED_DELAY_ON_PATH "/sys/class/leds/green:status/delay_on"

#define BLUE_LED_TRIGGER_PATH "/sys/class/leds/blue:status/trigger"
#define BLUE_LED_BRIGHTNESS_PATH "/sys/class/leds/blue:status/brightness"
#define BLUE_LED_DELAY_OFF_PATH "/sys/class/leds/blue:status/delay_off"
#define BLUE_LED_DELAY_ON_PATH "/sys/class/leds/blue:status/delay_on"

//define only for dubhe2000-demo-b
#define GREEN_LED_TRIGGER_PATH_DEMO_B "/sys/class/leds/green:internet/trigger"
#define GREEN_LED_BRIGHTNESS_PATH_DEMO_B "/sys/class/leds/green:internet/brightness"

#define FAST_BLINK_DELAY "100"
#define NORMAL_BLINK_DELAY "500"
#define SLOW_BLINK_DELAY "1000"

#define SPECIFIC_STATE_DURATION 15000

enum led_error_msg {
	LED_OK,
	LED_ERR_FILE_OPERATION = -1,
	LED_ERR_UBUS_OPERATION = -2,
	LED_ERR_UBUS_CONNECTION = -3,
	LED_ERR_UBUS_PARSE_FAILED = -4,
	__LED_ERR_MAX = -5,

};

enum led_event_priority {
	LED_EVENT_WPS,
	LED_EVENT_BACKHAUL,
	LED_EVENT_INTERNET,
	LED_EVENT_RUNTIME,
	__LED_EVENT_MAX,
};

enum led_mode {
	LED_MODE_GREEN_ON,
	LED_MODE_GREEN_OFF,
	LED_MODE_GREEN_FAST_BLINK,
	LED_MODE_GREEN_SLOW_BLINK,
	LED_MODE_GREEN_NORMAL_BLINK,
	LED_MODE_RED_ON,
	LED_MODE_RED_OFF,
	LED_MODE_RED_FAST_BLINK,
	LED_MODE_RED_SLOW_BLINK,
	LED_MODE_RED_NORMAL_BLINK,
	LED_MODE_BLUE_ON,
	LED_MODE_BLUE_OFF,
	LED_MODE_BLUE_FAST_BLINK,
	LED_MODE_BLUE_SLOW_BLINK,
	LED_MODE_BLUE_NORMAL_BLINK,
	LED_MODE_GREEN_ON_DEMO_B,
	LED_MODE_GREEN_OFF_DEMO_B,
	__LED_MODE_MAX,
};

#endif
