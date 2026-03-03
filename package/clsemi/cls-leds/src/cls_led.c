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

#include "cls_led.h"

static struct ubus_context *ctx;
static struct ubus_subscriber backhaul_subscriber;
static struct ubus_event_handler internet_ev;
static struct ubus_event_handler wps_event;
static struct uloop_timeout wps_timer;

static enum led_event_priority current_priority = __LED_EVENT_MAX;
static enum led_mode current_led_mode = __LED_MODE_MAX;
static enum led_mode before_wps_led_mode = __LED_MODE_MAX;

char board_info[128];

enum led_mode get_current_led_info(void)
{
	FILE *fp = NULL;
	char led_val[64] = {0};
	char buf[1024] = {0};
	int num, delay_on = 0;
	bool is_blink = false;
	enum led_mode cur_led_mode = __LED_MODE_MAX;
	const char *led_trigger_path[LED_NUMBER] = {
		RED_LED_TRIGGER_PATH,
		GREEN_LED_TRIGGER_PATH,
		BLUE_LED_TRIGGER_PATH
	};
	const char *led_path[LED_NUMBER] = {
		RED_LED_BRIGHTNESS_PATH,
		GREEN_LED_BRIGHTNESS_PATH,
		BLUE_LED_BRIGHTNESS_PATH
	};

	const char *led_delay_path[LED_NUMBER] = {
		RED_LED_DELAY_ON_PATH,
		GREEN_LED_DELAY_ON_PATH,
		BLUE_LED_DELAY_ON_PATH
	};

	for (num = 0; num < LED_NUMBER; num++) {
		fp = fopen(led_trigger_path[num], "r");
		if (!fp)
			return LED_ERR_FILE_OPERATION;

		while (fgets(buf, sizeof(buf), fp)) {
			if (strstr(buf, "[timer]")) {
				is_blink = true;
				fclose(fp);
				goto out;
			}
		}
		fclose(fp);
	}

out:
	if (is_blink) {
		fp = fopen(led_delay_path[num], "r");
		if (!fp)
			return LED_ERR_FILE_OPERATION;

		if (fscanf(fp, "%d", &delay_on) != 1) {
			fclose(fp);
			return LED_ERR_FILE_OPERATION;
		}

		fclose(fp);

		if (delay_on <= atoi(FAST_BLINK_DELAY)) {
			switch (num) {
			case 0:
				cur_led_mode = LED_MODE_RED_FAST_BLINK;
				break;
			case 1:
				cur_led_mode = LED_MODE_GREEN_FAST_BLINK;
				break;
			case 2:
				cur_led_mode = LED_MODE_BLUE_FAST_BLINK;
				break;
			}
		} else if (delay_on >= atoi(SLOW_BLINK_DELAY)) {
			switch (num) {
			case 0:
				cur_led_mode = LED_MODE_RED_SLOW_BLINK;
				break;
			case 1:
				cur_led_mode = LED_MODE_GREEN_SLOW_BLINK;
				break;
			case 2:
				cur_led_mode = LED_MODE_BLUE_SLOW_BLINK;
				break;
			}
		} else {
			switch (num) {
			case 0:
				cur_led_mode = LED_MODE_RED_NORMAL_BLINK;
				break;
			case 1:
				cur_led_mode = LED_MODE_GREEN_NORMAL_BLINK;
				break;
			case 2:
				cur_led_mode = LED_MODE_BLUE_NORMAL_BLINK;
				break;
			}
		}
	} else {
		for (int i = 0; i < LED_NUMBER; i++) {
			fp = fopen(led_path[i], "r");
			if (!fp)
				return LED_ERR_FILE_OPERATION;

			memset(led_val, 0, ARRAY_SIZE(led_val));

			if (fgets(led_val, ARRAY_SIZE(led_val), fp) == NULL) {
				fclose(fp);
				return LED_ERR_FILE_OPERATION;
			}

			if (atoi(led_val) == 255) {
				switch (i) {
				case 0:
					cur_led_mode = LED_MODE_RED_ON;
					break;
				case 1:
					cur_led_mode = LED_MODE_GREEN_ON;
					break;
				case 2:
					cur_led_mode = LED_MODE_BLUE_ON;
					break;
				}
				fclose(fp);
				break;
			}
			fclose(fp);
		}
	}

	return cur_led_mode;
}

int get_board_info(char board_info[128])
{
	int ret = LED_OK;
	FILE *fp = fopen(BOARD_INFO_PATH, "r");

	if (!fp) {
		ret = LED_ERR_FILE_OPERATION;
		return LED_ERR_FILE_OPERATION;
	}

	if (fgets(board_info, 128, fp) == NULL)
		ret = LED_ERR_FILE_OPERATION;

	fclose(fp);

	return ret;
}

int write_to_file(const char *path, const char *value)
{
	FILE *fp = fopen(path, "w");
	int ret;

	if (!fp)
		return LED_ERR_FILE_OPERATION;

	ret = fprintf(fp, "%s", value);
	if (ret <= 0)
		ret = LED_ERR_FILE_OPERATION;

	fclose(fp);

	return LED_OK;
}

int set_led(enum led_mode mode)
{
	int ret = LED_OK;

	if (mode == current_led_mode)
		return ret;

	switch (current_led_mode) {
	case LED_MODE_GREEN_ON:
	case LED_MODE_GREEN_FAST_BLINK:
	case LED_MODE_GREEN_NORMAL_BLINK:
	case LED_MODE_GREEN_SLOW_BLINK:
		ret = write_to_file(GREEN_LED_TRIGGER_PATH, "none");
		if (ret < 0)
			return ret;
		ret = write_to_file(GREEN_LED_BRIGHTNESS_PATH, "0");
		if (ret < 0)
			return ret;
		break;

	case LED_MODE_RED_ON:
	case LED_MODE_RED_SLOW_BLINK:
	case LED_MODE_RED_FAST_BLINK:
	case LED_MODE_RED_NORMAL_BLINK:
		ret = write_to_file(RED_LED_TRIGGER_PATH, "none");
		if (ret < 0)
			return ret;
		ret = write_to_file(RED_LED_BRIGHTNESS_PATH, "0");
		if (ret < 0)
			return ret;
		break;

	case LED_MODE_BLUE_ON:
	case LED_MODE_BLUE_SLOW_BLINK:
	case LED_MODE_BLUE_FAST_BLINK:
	case LED_MODE_BLUE_NORMAL_BLINK:
		ret = write_to_file(BLUE_LED_TRIGGER_PATH, "none");
		if (ret < 0)
			return ret;
		ret = write_to_file(BLUE_LED_BRIGHTNESS_PATH, "0");
		if (ret < 0)
			return ret;
		break;
	case LED_MODE_GREEN_ON_DEMO_B:
		ret = write_to_file(GREEN_LED_TRIGGER_PATH_DEMO_B, "none");
		if (ret < 0)
			return ret;
		ret = write_to_file(GREEN_LED_BRIGHTNESS_PATH_DEMO_B, "0");
		if (ret < 0)
			return ret;
		break;

	case LED_MODE_GREEN_OFF:
	case LED_MODE_RED_OFF:
	case LED_MODE_BLUE_OFF:
	case LED_MODE_GREEN_OFF_DEMO_B:
	case __LED_MODE_MAX:
		break;
	}

	current_led_mode = mode;
	switch (mode) {
	case LED_MODE_GREEN_ON:
			ret = write_to_file(GREEN_LED_TRIGGER_PATH, "none");
			if (ret < 0)
				return ret;
			ret = write_to_file(GREEN_LED_BRIGHTNESS_PATH, "255");
			if (ret < 0)
				return ret;
			break;
	case LED_MODE_GREEN_OFF:
			ret = write_to_file(GREEN_LED_TRIGGER_PATH, "none");
			if (ret < 0)
				return ret;
			ret = write_to_file(GREEN_LED_BRIGHTNESS_PATH, "0");
			if (ret < 0)
				return ret;
			break;
	case LED_MODE_GREEN_FAST_BLINK:
			ret = write_to_file(GREEN_LED_TRIGGER_PATH, "timer");
			if (ret < 0)
				return ret;
			ret = write_to_file(GREEN_LED_DELAY_ON_PATH, FAST_BLINK_DELAY);
			if (ret < 0)
				return ret;
			ret = write_to_file(GREEN_LED_DELAY_OFF_PATH, FAST_BLINK_DELAY);
			if (ret < 0)
				return ret;
			break;
	case LED_MODE_GREEN_NORMAL_BLINK:
			ret = write_to_file(GREEN_LED_TRIGGER_PATH, "timer");
			if (ret < 0)
				return ret;
			ret = write_to_file(GREEN_LED_DELAY_ON_PATH, NORMAL_BLINK_DELAY);
			if (ret < 0)
				return ret;
			ret = write_to_file(GREEN_LED_DELAY_OFF_PATH, NORMAL_BLINK_DELAY);
			if (ret < 0)
				return ret;
			break;
	case LED_MODE_GREEN_SLOW_BLINK:
			ret = write_to_file(GREEN_LED_TRIGGER_PATH, "timer");
			if (ret < 0)
				return ret;
			ret = write_to_file(GREEN_LED_DELAY_ON_PATH, SLOW_BLINK_DELAY);
			if (ret < 0)
				return ret;
			ret = write_to_file(GREEN_LED_DELAY_OFF_PATH, SLOW_BLINK_DELAY);
			if (ret < 0)
				return ret;
			break;
	case LED_MODE_RED_ON:
			ret = write_to_file(RED_LED_TRIGGER_PATH, "none");
			if (ret < 0)
				return ret;
			ret = write_to_file(RED_LED_BRIGHTNESS_PATH, "255");
			if (ret < 0)
				return ret;
			break;
	case LED_MODE_RED_OFF:
			ret = write_to_file(RED_LED_TRIGGER_PATH, "none");
			if (ret < 0)
				return ret;
			ret = write_to_file(RED_LED_BRIGHTNESS_PATH, "0");
			if (ret < 0)
				return ret;
			break;
	case LED_MODE_RED_SLOW_BLINK:
			ret = write_to_file(RED_LED_TRIGGER_PATH, "timer");
			if (ret < 0)
				return ret;
			ret = write_to_file(RED_LED_DELAY_ON_PATH, SLOW_BLINK_DELAY);
			if (ret < 0)
				return ret;
			ret = write_to_file(RED_LED_DELAY_OFF_PATH, SLOW_BLINK_DELAY);
			if (ret < 0)
				return ret;
			break;
	case LED_MODE_RED_FAST_BLINK:
			ret = write_to_file(RED_LED_TRIGGER_PATH, "timer");
			if (ret < 0)
				return ret;
			ret = write_to_file(RED_LED_DELAY_ON_PATH, FAST_BLINK_DELAY);
			if (ret < 0)
				return ret;
			ret = write_to_file(RED_LED_DELAY_OFF_PATH, FAST_BLINK_DELAY);
			if (ret < 0)
				return ret;
			break;
	case LED_MODE_RED_NORMAL_BLINK:
			ret = write_to_file(RED_LED_TRIGGER_PATH, "timer");
			if (ret < 0)
				return ret;
			ret = write_to_file(RED_LED_DELAY_ON_PATH, NORMAL_BLINK_DELAY);
			if (ret < 0)
				return ret;
			ret = write_to_file(RED_LED_DELAY_OFF_PATH, NORMAL_BLINK_DELAY);
			if (ret < 0)
				return ret;
			break;
	case LED_MODE_BLUE_ON:
			ret = write_to_file(BLUE_LED_TRIGGER_PATH, "none");
			if (ret < 0)
				return ret;
			ret = write_to_file(BLUE_LED_BRIGHTNESS_PATH, "255");
			if (ret < 0)
				return ret;
			break;
	case LED_MODE_BLUE_OFF:
			ret = write_to_file(BLUE_LED_TRIGGER_PATH, "none");
			if (ret < 0)
				return ret;
			ret = write_to_file(BLUE_LED_BRIGHTNESS_PATH, "0");
			if (ret < 0)
				return ret;
			break;
	case LED_MODE_BLUE_SLOW_BLINK:
			ret = write_to_file(BLUE_LED_TRIGGER_PATH, "timer");
			if (ret < 0)
				return ret;
			ret = write_to_file(BLUE_LED_DELAY_ON_PATH, SLOW_BLINK_DELAY);
			if (ret < 0)
				return ret;
			ret = write_to_file(BLUE_LED_DELAY_OFF_PATH, SLOW_BLINK_DELAY);
			if (ret < 0)
				return ret;
			break;
	case LED_MODE_BLUE_FAST_BLINK:
			ret = write_to_file(BLUE_LED_TRIGGER_PATH, "timer");
			if (ret < 0)
				return ret;
			ret = write_to_file(BLUE_LED_DELAY_ON_PATH, FAST_BLINK_DELAY);
			if (ret < 0)
				return ret;
			ret = write_to_file(BLUE_LED_DELAY_OFF_PATH, FAST_BLINK_DELAY);
			if (ret < 0)
				return ret;
			break;
	case LED_MODE_BLUE_NORMAL_BLINK:
			ret = write_to_file(BLUE_LED_TRIGGER_PATH, "timer");
			if (ret < 0)
				return ret;
			ret = write_to_file(BLUE_LED_DELAY_ON_PATH, NORMAL_BLINK_DELAY);
			if (ret < 0)
				return ret;
			ret = write_to_file(BLUE_LED_DELAY_OFF_PATH, NORMAL_BLINK_DELAY);
			if (ret < 0)
				return ret;
			break;
	case LED_MODE_GREEN_ON_DEMO_B:
			ret = write_to_file(GREEN_LED_TRIGGER_PATH_DEMO_B, "none");
			if (ret < 0)
				return ret;
			ret = write_to_file(GREEN_LED_BRIGHTNESS_PATH_DEMO_B, "255");
			if (ret < 0)
				return ret;
			break;
	case LED_MODE_GREEN_OFF_DEMO_B:
			ret = write_to_file(GREEN_LED_TRIGGER_PATH_DEMO_B, "none");
			if (ret < 0)
				return ret;
			ret = write_to_file(GREEN_LED_BRIGHTNESS_PATH_DEMO_B, "0");
			if (ret < 0)
				return ret;
			break;
	default:
			return ret;
	}

	return ret;
}

int set_led_temp(enum led_mode mode, long time)
{
	int ret = LED_OK;
	enum led_mode before_led_mode;

	before_led_mode = current_led_mode;
	ret = set_led(mode);
	sleep(time);
	ret = set_led(before_led_mode);
	current_led_mode = before_led_mode;

	return ret;
}

int handle_event(enum led_mode new_mode, enum led_event_priority priority, long timeout)
{
	int ret = LED_OK;
	enum led_event_priority before_priority = current_priority;

	if (priority <= current_priority) {
		current_priority = priority;

		if (strncmp(board_info, "clsemi,dubhe2000-demo-b", ARRAY_SIZE(board_info)) == 0) {
			switch (new_mode) {
			case LED_MODE_GREEN_ON:
			case LED_MODE_RED_ON:
			case LED_MODE_BLUE_ON:
				new_mode = LED_MODE_GREEN_ON_DEMO_B;
				break;

			case LED_MODE_GREEN_OFF:
			case LED_MODE_RED_OFF:
			case LED_MODE_BLUE_OFF:
				new_mode = LED_MODE_GREEN_OFF_DEMO_B;
				break;
			default:
				break;
			}
		}
		if (timeout > 0) {
			ret = set_led_temp(new_mode, timeout);
			current_priority = before_priority;
		} else
			ret = set_led(new_mode);

	} else
		ret = LED_OK;

	return ret;
}

static void timer_cb(struct uloop_timeout *t)
{
	handle_event(before_wps_led_mode, LED_EVENT_BACKHAUL, 0);
}

enum {
	DATA_ATTR_CLSNETMANAGER_BACKHAUL,
	__DATA_ATTR_MAX,
};

static const struct blobmsg_policy data_policy[__DATA_ATTR_MAX] = {
	[DATA_ATTR_CLSNETMANAGER_BACKHAUL] = { .name = "clsnetmanager.backhaul", .type = BLOBMSG_TYPE_TABLE },
};

enum {
	BACKHAUL_ATTR_TYPE,
	BACKHAUL_ATTR_STATUS,
	__BACKHAUL_ATTR_MAX,
};

static const struct blobmsg_policy backhaul_policy[__BACKHAUL_ATTR_MAX] = {
	[BACKHAUL_ATTR_TYPE] = { .name = "type", .type = BLOBMSG_TYPE_STRING },
	[BACKHAUL_ATTR_STATUS] = { .name = "status", .type = BLOBMSG_TYPE_STRING },
};

static int event_backhaul_handler(struct ubus_context *ctx, struct ubus_object *obj,
	struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
	struct blob_attr *tb[__DATA_ATTR_MAX];
	struct blob_attr *backhaul_tb[__BACKHAUL_ATTR_MAX];
	struct blob_attr *data_attr;
	struct blob_attr *backhaul_attr;
	int ret = LED_OK;

	if (!msg) {
		fprintf(stderr, "No message to parse.\n");
		return LED_ERR_UBUS_PARSE_FAILED;
	}

	current_led_mode = get_current_led_info();

	// Parse the top-level 'data' field
	blobmsg_parse(data_policy, __DATA_ATTR_MAX, tb, blob_data(msg), blob_len(msg));
	data_attr = tb[DATA_ATTR_CLSNETMANAGER_BACKHAUL];
	if (!data_attr) {
		fprintf(stderr, "Missing 'clsnetmanager.backhaul' field.\n");
		return LED_ERR_UBUS_PARSE_FAILED;
	}

	// Parse the 'clsnetmanager.backhaul' table
	blobmsg_parse(backhaul_policy, __BACKHAUL_ATTR_MAX, backhaul_tb,
			blobmsg_data(data_attr), blobmsg_data_len(data_attr));

	if (!backhaul_tb[BACKHAUL_ATTR_TYPE] || !backhaul_tb[BACKHAUL_ATTR_STATUS]) {
		fprintf(stderr, "status or type field is missing.\n");
		return LED_ERR_UBUS_PARSE_FAILED;
	}

	backhaul_attr = backhaul_tb[BACKHAUL_ATTR_STATUS];
	if (backhaul_attr) {
		if (strcmp(blobmsg_get_string(backhaul_attr), "disconnect") == 0) {
			ret = handle_event(LED_MODE_RED_ON, LED_EVENT_BACKHAUL, 0);
			if (ret < 0)
				return ret;
		} else if (strcmp(blobmsg_get_string(backhaul_attr), "connect") == 0)
			ret = handle_event(LED_MODE_GREEN_ON, LED_EVENT_BACKHAUL, 0);
		if (ret < 0)
			return ret;

	} else {
		fprintf(stderr, "status: field is missing.\n");
		ret = handle_event(LED_MODE_RED_ON, LED_EVENT_BACKHAUL, 0);
		if (ret < 0)
			return ret;

		ret = LED_ERR_UBUS_PARSE_FAILED;
		return ret;
	}

	return ret;
}

enum {
	INTERNET_LINK_ATTR_STATUS,
	_INTERNET_LINK_ATTR_MAX,
};

static const struct blobmsg_policy internet_link_policy[_INTERNET_LINK_ATTR_MAX] = {
	[INTERNET_LINK_ATTR_STATUS] = { .name = "status", .type = BLOBMSG_TYPE_STRING },
};

static void internet_event_handler(struct ubus_context *ctx, struct ubus_event_handler *ev,
		const char *type, struct blob_attr *msg)
{
	struct blob_attr *tb[__DATA_ATTR_MAX];
	struct blob_attr *status_attr;

	if (!msg) {
		fprintf(stderr, "No message to parse.\n");
		return;
	}

	current_led_mode = get_current_led_info();
	// Parse the top-level 'status' field
	blobmsg_parse(internet_link_policy, _INTERNET_LINK_ATTR_MAX, tb, blob_data(msg), blob_len(msg));
	status_attr = tb[INTERNET_LINK_ATTR_STATUS];
	if (!status_attr) {
		fprintf(stderr, "Missing 'internet link' field.\n");
		return;
	}

	if (strcmp(blobmsg_get_string(status_attr), "disconnected") == 0)
		handle_event(LED_MODE_RED_ON, LED_EVENT_INTERNET, 0);
	else if (strcmp(blobmsg_get_string(status_attr), "connected") == 0)
		handle_event(LED_MODE_GREEN_ON, LED_EVENT_INTERNET, 0);
	else
		return;

	return;
}

enum {
	WPS_ATTR_EVENT,
	__WPS_ATTR_MAX
};

enum wps_event {
	WPS_EV_FAIL = 1,
	WPS_EV_SUCCESS,
	WPS_EV_PBC_OVERLAP = 4,
	WPS_EV_PBC_TIMEOUT,
	WPS_EV_PBC_ACTIVE,
	WPS_EV_AP_PIN_SUCCESS = 14
};

const char *wps_event_type[] = {
	"hostapd.wps",
	"wpa_supplicant.wps"
};

static const struct blobmsg_policy wps_policy[__WPS_ATTR_MAX] = {
	[WPS_ATTR_EVENT] = { .name = "wps_event", .type = BLOBMSG_TYPE_INT16 },
};

static void event_wps_handler(struct ubus_context *ctx, struct ubus_event_handler *ev,
		const char *type, struct blob_attr *msg)
{
	struct blob_attr *tb[__WPS_ATTR_MAX];

	if (!msg) {
		fprintf(stderr, "No message to parse.\n");
		return;
	}

	blobmsg_parse(wps_policy, __WPS_ATTR_MAX, tb, blob_data(msg), blob_len(msg));

	if (tb[WPS_ATTR_EVENT]) {
		int evt = blobmsg_get_u16(tb[WPS_ATTR_EVENT]);

		switch (evt) {
		case WPS_EV_FAIL:
		case WPS_EV_PBC_OVERLAP:
		case WPS_EV_PBC_TIMEOUT:
			handle_event(LED_MODE_BLUE_FAST_BLINK, LED_EVENT_WPS, 0);
			current_priority = LED_EVENT_BACKHAUL;
			wps_timer.cb = timer_cb;
			uloop_timeout_set(&wps_timer, SPECIFIC_STATE_DURATION);
			break;
		case WPS_EV_AP_PIN_SUCCESS:
		case WPS_EV_SUCCESS:
			handle_event(LED_MODE_BLUE_ON, LED_EVENT_WPS, 0);
			current_priority = LED_EVENT_BACKHAUL;
			if (strcmp(type, "hostapd.wps") == 0) {
				wps_timer.cb = timer_cb;
				uloop_timeout_set(&wps_timer, SPECIFIC_STATE_DURATION);
			}
			break;
		case WPS_EV_PBC_ACTIVE:
			current_led_mode = get_current_led_info();
			before_wps_led_mode = current_led_mode;
			handle_event(LED_MODE_GREEN_FAST_BLINK, LED_EVENT_WPS, 0);
			break;
		default:
			break;
		}
	}

	return;
}

int main(int argc, char **argv)
{
	uint32_t id;
	int ret;

	ret = get_board_info(board_info);
	if (ret < 0)
		return ret;

	uloop_init();
	ctx = ubus_connect(NULL);
	if (!ctx) {
		fprintf(stderr, "Failed to connect to ubus\n");
		return LED_ERR_UBUS_CONNECTION;
	}

	//handler for wps status event
	wps_event.cb = event_wps_handler;
	for (int i = 0; i < ARRAY_SIZE(wps_event_type); i++) {
		if (ubus_register_event_handler(ctx, &wps_event, wps_event_type[i])) {
			fprintf(stderr, "Failed to register wps event: %s\n", wps_event_type[i]);
			return LED_ERR_UBUS_OPERATION;
		}
	}
	//handle internet event
	internet_ev.cb = internet_event_handler;
	if (ubus_register_event_handler(ctx, &internet_ev, "internet_event")) {
		fprintf(stderr, "Failed to register internet event\n");
		return LED_ERR_UBUS_OPERATION;
	}
	//handle backhaul event
	backhaul_subscriber.cb = event_backhaul_handler;
	if (ubus_register_subscriber(ctx, &backhaul_subscriber)) {
		fprintf(stderr, "Failed to register backhaul_subscriber\n");
		return LED_ERR_UBUS_OPERATION;
	}

	if (ubus_lookup_id(ctx, "network.clsnetmanager", &id) == 0) {
		if (ubus_subscribe(ctx, &backhaul_subscriber, id) != 0)
			fprintf(stderr, "Failed to subscribe to demo object\n");
	} else
		fprintf(stderr, "Failed to find network.clsnetmanager object\n");

	ubus_add_uloop(ctx);
	uloop_run();
	ubus_free(ctx);
	uloop_done();

	return LED_OK;
}
