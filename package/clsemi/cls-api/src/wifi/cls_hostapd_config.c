/*
 * Copyright (c) 2022- Clourney Semiconductor Limited and Contributors. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <fcntl.h>
#include "wpa_adapter.h"


#ifndef MAC2STR
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
#endif

#define MAX_BSSID 8
#define RADIO_ID_FIRST 0
#define MAX_RADIO_NUM  (RADIO_ID_FIRST + 3)

#define CLS_DEBUG 1
#define HOST_APD_CONF_FMT  "hostapd-phy%u.conf"
#define WPA_SUPPLICANT_CONF_FMT "wpa_supplicant-phy%u.conf"

#define HS20_OSU_SERVER_URI_PARAM	"osu_server_uri"

#define CLSAPI_SEC_FILE_LEN_MAX  128

#define CLSAPI_MAX_PARAM_NAME_LEN	24
#define CLSAPI_MAX_PARAM_VAL_LEN	600	/* for long values parameters - use malloc */
#define CLSAPI_SHORT_PARAM_VAL_LEN	128	/* suitable for most config parameters */
#define MAX_SECURITY_CONFIG_LENGTH \
		(CLSAPI_MAX_PARAM_NAME_LEN + CLSAPI_MAX_PARAM_VAL_LEN + 1)

#define CLS_DEF_HOSTAD_DIR  "/tmp/run/"

#define CLSAPI_SEC_FILE_TEMP_PREF ".clst."
#define CLSAPI_SEC_FILE_HIDDEN_PREF	 ".clsh."
#define SCRATCHPAD_FOLDER	"/tmp"

#ifndef IFNAMSIZ
#define IFNAMSIZ	16
#endif /* IFNAMSIZ */

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof((a))/sizeof((a)[0]))
#endif

char *auth_server_param_list[] = {"auth_server_addr", "auth_server_port",
					"auth_server_shared_secret"};

char *general_param_list[] = {"hw_mode", "supported_rates", "basic_rates", "beacon_int", "channel",
				"chanlist", "ieee80211n", "ht_coex", "ht_capab", "ieee80211ac",
				"vht_oper_chwidth", "vht_oper_centr_freq_seg0_idx", "vht_capab",
				"ieee80211ax", "he_oper_chwidth", "he_oper_centr_freq_seg0_idx",
				"he_default_pe_duration", NULL};

char *ht_option_list[] = {"ieee80211n", "ht_coex", "ht_capab", NULL};

char *vht_option_list[] = {"ieee80211ac", "vht_oper_chwidth", "vht_oper_centr_freq_seg0_idx",
				"vht_capab", NULL};

char *he_option_list[] = {"ieee80211ax", "he_oper_chwidth", "he_oper_centr_freq_seg0_idx",
				"he_default_pe_duration", NULL};

int chan_5g_list[] = {36, 40, 44, 48, 52, 56, 60, 64, 149, 153, 157, 161, 165};

char *default_ht_capab = "[SHORT-GI-20][SHORT-GI-40][DSSS_CCK-40]";

char *default_vht_capab = "[RXLDPC][SHORT-GI-80][SHORT-GI-160][TX-STBC-2BY1]"
				"[RX-STBC-1234][VHT160-80PLUS80][MAX-MPDU-11454]"
				"[MAX-A-MPDU-LEN-EXP7]";


char *auth_options[] = {"wpa", "wpa_passphrase", "wpa_pairwise", "wpa_key_mgmt", NULL};

struct vap_config {
	char *option_name;
	char *option_value;
};

struct vap_config def_vap_config[] = {
	{"ctrl_interface", "/var/run/hostapd"},
	{"ap_isolate", "1"},
	{"bss_load_update_period", "60"},
	{"chan_util_avg_period", "600"},
	{"disassoc_low_ack", "1"},
	{"skip_inactivity_poll", "0"},
	{"preamble", "1"},
	{"wmm_enabled", "1"},
	{"ignore_broadcast_ssid", "0"},
	{"uapsd_advertisement_enabled", "1"},
	{"utf8_ssid", "1"},
	{"multi_ap", "0"},
	{"auth_algs", "1"},
	{"bridge", "br-lan"},
	{"wds_bridge", ""},
	{"snoop_iface", "br-lan"},
	{NULL, NULL},
	};

enum cls_ap_mode {
	CLS_AP_MODE_A,
	CLS_AP_MODE_B,
	CLS_AP_MODE_G,
	CLS_AP_MODE_11N,
	CLS_AP_MODE_11AC,
	CLS_AP_MODE_11AX
};

struct cls_param_with_sub {
	const char *param_name;
	int		has_sub_param; //0: single param 1:has sub param
};

static struct cls_param_with_sub param_with_sub[] = {
	{.param_name = "auth_server_addr", .has_sub_param = 1},
	{.param_name = "osu_server_uri", .has_sub_param = 1},
	{.param_name = "acct_server_addr", .has_sub_param = 1},

};

static const char *osu_server_sub_params_list[] = {
	"osu_friendly_name",
	"osu_nai",
	"osu_method_list",
	"osu_icon",
	"osu_service_desc",
	NULL
};

static const char *auth_server_sub_params_list[] = {
	"auth_server_port",
	"auth_server_shared_secret",
	NULL
};

static const char *acct_server_sub_params_list[] = {
	"acct_server_port",
	"acct_server_shared_secret",
	NULL
};

static char cls_primary_interface[MAX_RADIO_NUM][IFNAMSIZ] = { {"wlan0"}, {"wlan1"}, {"wlan2"}};

#define HS20_ICON_PARAM			"hs20_icon"
#define IEEE_80211R_R0KH		"r0kh"
#define IEEE_80211R_R1KH		"r1kh"

#define MAX_RADIUS_SERVERS		3

#define CLSAPI_TRUE	1
#define CLSAPI_FALSE	0

enum {
	cls_bare_string = 0,
	cls_in_quotes = 1
};

enum {
	security_update_pending = 0,
	security_update_complete = 1
};

typedef enum {
	/**
	 * The SSID parameter is a printable string.
	 */
	cls_ssid_fmt_str = 0,
	/**
	 * The SSID parameter is a hexdump string.
	 */
	cls_ssid_fmt_hex_str = 1,
	/**
	 * Any type of SSID parameter is acceptable.
	 */
	cls_ssid_fmt_any = 2,
} cls_ssid_fmt;


/**
 * \brief The operational WiFi mode of a device.
 *
 * The operational WiFi mode of a device.
 */
typedef enum {
	/** The WiFi mode has not yet been configured. */
	cls_mode_not_defined = 1,

	/** The device is operating as an AP. */
	cls_mode_access_point,

	/** The device is operating as a STA. */
	cls_mode_station,

	/** Invalid mode. */
	cls_nosuch_mode = 0
} cls_wifi_mode;

typedef enum {
	e_searching_for_generic_param,
	e_searching_for_network,
	e_found_network_token,
	e_found_current_network,
	e_found_parent_value,
	e_parameter_not_found
} SSID_parsing_state;

enum conf_parsing_parameter_s {
	E_PARAMETER_INVALID = 0,
	E_PARAMETER_FOUND = 1,
	E_PARAMETER_NOT_FOUND = 2,
	E_PARAMETER_EXCEED_LIMIT = 3
};


static int cls_security_get_sec_dir(char *buf, int buflen)
{
	char *seca_str;

	assert(buflen == CLSAPI_SEC_FILE_LEN_MAX);
	seca_str = calloc(CLSAPI_MAX_PARAM_VAL_LEN, sizeof(char));
	if (!seca_str)
		return -ENOMEM;

	strcpy(buf, CLS_DEF_HOSTAD_DIR);
	if (seca_str)
		free(seca_str);
	return strlen(buf);
}

static int locate_config_file(unsigned int radio_id, const cls_wifi_mode wifi_mode,
		char *config_file_path, const unsigned int buflen, const char *prefix)
{
	int len;
	int namelen;

	if (!strcmp(prefix, CLSAPI_SEC_FILE_TEMP_PREF))
		len = snprintf(config_file_path, buflen, SCRATCHPAD_FOLDER "/");
	else
		len = cls_security_get_sec_dir(config_file_path, buflen);

	namelen = buflen - len;

	if (wifi_mode == cls_mode_access_point) {
		if (snprintf(&config_file_path[len], namelen, "%s"HOST_APD_CONF_FMT,
					prefix, radio_id) >= namelen)
			return -ENAMETOOLONG;
	} else {
		if (snprintf(&config_file_path[len], namelen, "%s"WPA_SUPPLICANT_CONF_FMT,
					prefix, radio_id) >= namelen)
			return -ENAMETOOLONG;
	}
	config_file_path[buflen - 1] = '\0';
	//printf("----the config file path is [%s]\n", config_file_path);

	return 0;
}

static int get_config_file_path(unsigned int radio_id, const cls_wifi_mode wifi_mode,
		char *config_file_path, const unsigned int buflen, const char *prefix)
{
	return locate_config_file(radio_id, wifi_mode, config_file_path, buflen, prefix);
}


static int
lock_config_file(const char *config_file, FILE **p_config_fh)
{
	int fd;
	int retval = 0;
	struct flock fl = {0};

	fd = open(config_file, O_RDWR);
	if (fd < 0) {
		//local_generic_syslog("CLSAPI_SEC", LOG_ERR, "open failed for %s", config_file);
		printf("%s:%d:error\n", __func__, __LINE__);
		retval = -errno;
		goto out;
	}

	/*
	 * Lock entire file exclusively. l_len = 0 is a special value representing end of file
	 * (no matter how large the file grows in future)
	 */
	fl.l_type = F_WRLCK;
	fl.l_whence = SEEK_SET;
	fl.l_start = 0;
	fl.l_len = 0;

	/* Non-blocking */
	if (fcntl(fd, F_SETLK, &fl) < 0) {
		//local_generic_syslog("CLSAPI_SEC", LOG_ERR, "fcntl64 failed for %s", config_file);
		retval = -errno;
		goto out;
	}

	/* Note:
	 * - Closing fd would release the lock
	 * - Do not duplicate fd. Closing the duplicated fd would also release the lock
	 */

	*p_config_fh = fdopen(fd, "rw");
	if (!*p_config_fh) {
		//local_generic_syslog("CLSAPI_SEC", LOG_ERR, "fopen failed for %s", config_file);
		retval = -errno;
		if (retval >= 0)
			retval = -EACCES;
		goto out;
	}

	return retval;

out:
	if (fd >= 0)
		close(fd);

	return retval;
}


static int open_config_file(unsigned int radio_id, const cls_wifi_mode wifi_mode,
		FILE **p_config_fh)
{
	int retval;
	FILE *config_fh = NULL;
	char config_file[CLSAPI_SEC_FILE_LEN_MAX];
	const int filename_len = sizeof(config_file);

	assert(p_config_fh);

	retval = get_config_file_path(radio_id, wifi_mode, config_file, filename_len, "");
	if (retval != 0)
		return retval;

	retval = lock_config_file(config_file, &config_fh);
	if (retval) {
		//local_generic_syslog("CLSAPI_SEC", LOG_ERR, "lock failed for %s", config_file);
		printf("error!!!\n");
		return retval;
	}

	*p_config_fh = config_fh;

	return retval;
}

static int open_temp_file(unsigned int radio_id, const cls_wifi_mode wifi_mode,
		FILE **p_temp_fh, int rw)
{
	int retval = 0;
	FILE *temp_fh = NULL;
	char config_file_tmp[CLSAPI_SEC_FILE_LEN_MAX];
	const int filename_len = sizeof(config_file_tmp);

	retval = locate_config_file(radio_id, wifi_mode, config_file_tmp,
			filename_len, CLSAPI_SEC_FILE_TEMP_PREF);

	if (retval != 0)
		return retval;

	/*
	 * "wx" could be used for exclusive access, but does not appear to be supported,
	 * either in RAM (/tmp) or flash.
	 */
	if (rw)
		temp_fh = fopen(config_file_tmp, "w+");
	else
		temp_fh = fopen(config_file_tmp, "w");
	if (temp_fh == NULL) {
		retval = -errno;
		if (retval >= 0)
			retval = -EACCES;
	}

	if (p_temp_fh)
		*p_temp_fh = temp_fh;

	return retval;
}

static int
file_move(const char *src, const char *dest)
{
	int retval;
	char cmd[512];

	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd), "mv %s %s", src, dest);

	retval = system(cmd);
	if (retval < 0)
		retval = -errno;

	return retval;
}

static void
unlock_config_file(FILE **p_config)
{
	/* Closing the file releases the lock too => no need to explicitly unlock */
	fclose(*p_config);
	*p_config = NULL;
}

static int close_file(unsigned int radio_id, FILE **p_config_fh, FILE **p_temp_fh,
		const cls_wifi_mode wifi_mode, int update)
{
	char config_file[CLSAPI_SEC_FILE_LEN_MAX];
	char config_file_tmp[CLSAPI_SEC_FILE_LEN_MAX];
	char config_file_hidden[CLSAPI_SEC_FILE_LEN_MAX];
	const int filename_len = sizeof(config_file);
	int retval = 0;

	retval = get_config_file_path(radio_id, wifi_mode, config_file, filename_len, "");
	if (retval != 0)
		return retval;
	retval = get_config_file_path(radio_id, wifi_mode, config_file_hidden,
			filename_len, CLSAPI_SEC_FILE_HIDDEN_PREF);
	if (retval != 0)
		return retval;
	retval = get_config_file_path(radio_id, wifi_mode, config_file_tmp,
			filename_len, CLSAPI_SEC_FILE_TEMP_PREF);
	if (retval != 0)
		return retval;

	if (p_temp_fh && *p_temp_fh) {
		fclose(*p_temp_fh);
		*p_temp_fh = NULL;
	}

	/*
	 * If changes have been made, first copy the updated temporary
	 * file back to the hidden file.
	 */
	if (update) {
		retval = file_move(config_file_tmp, config_file_hidden);
		if (retval < 0) {
			//local_generic_syslog("CLSAPI_SEC", LOG_ERR,
			//		"move failed with %d for %s", retval, config_file_tmp);
			unlink(config_file_tmp);
		} else {
			/*
			 * Now restore the hidden file to the original file.
			 */
			retval = rename(config_file_hidden, config_file);
			if (retval < 0) {
				retval = -errno;
				//local_generic_syslog("CLSAPI_SEC", LOG_ERR,
				//		"rename failed with %d for %s", retval,
				//		config_file_hidden);
				unlink(config_file_hidden);
			}
		}
	} else if (p_temp_fh && *p_temp_fh) {
		unlink(config_file_tmp);
	}

	if (p_config_fh && *p_config_fh)
		unlock_config_file(p_config_fh);


	return retval;
}

static int open_file(unsigned int radio_id, FILE **p_config_fh, FILE **p_temp_fh,
		const cls_wifi_mode wifi_mode, int tmp_rw)
{
	int retval = 0;

	if (p_config_fh == NULL && p_temp_fh == NULL)
		return -EPERM;

	if (p_config_fh)
		retval = open_config_file(radio_id, wifi_mode, p_config_fh);

	if (retval >= 0 && p_temp_fh) {
		retval = open_temp_file(radio_id, wifi_mode, p_temp_fh, tmp_rw);
		if (retval < 0)
			close_file(radio_id, p_config_fh, p_temp_fh, wifi_mode, 0);
	}

	return retval;
}

static const char **local_get_sub_parameter_list(const char *parameter)
{

	if (!strcmp(parameter, HS20_OSU_SERVER_URI_PARAM))
		return osu_server_sub_params_list;
	else if (!strcmp(parameter, "auth_server_addr"))
		return auth_server_sub_params_list;
	else if (!strcmp(parameter, "acct_server_addr"))
		return acct_server_sub_params_list;

	return NULL;
}

int local_get_radio_id(const char *ifname, unsigned int *radio_id)
{
	const char *ifname_format = "wlan%u";
	int radio;

	if (sscanf(ifname, ifname_format, &radio) != 1)
		return -1;

	if (radio < 0 || radio >= MAX_RADIO_NUM)
		return -1;

	*radio_id = radio;

	return 0;
}

int local_verify_interface_is_primary(const char *ifname)
{
	int retval = 0;
	unsigned int radio_id = 0;

	local_get_radio_id(ifname, &radio_id);
	retval = strncmp(cls_primary_interface[radio_id], ifname, IFNAMSIZ);

	return retval;
}

char *read_config_line(char *address, int size, FILE *fh)
{
	int current_char = 0;
	int read_complete = 0;
	char *p_dst_addr = address;
	char *retaddr = address;
	int dst_char_count = 0;

	memset(address, 0, size);
	if (address != NULL && size >= 2) {
		while (dst_char_count < size - 1 && read_complete == 0) {
			current_char = fgetc(fh);
			if (current_char == EOF || current_char == '\n')
				read_complete = 1;


			if (current_char != EOF) {
				*p_dst_addr = current_char;
				p_dst_addr++;
				dst_char_count++;
			}
		}
	}

	if (p_dst_addr != NULL)
		*p_dst_addr = '\0';

	while (read_complete == 0) {
		current_char = fgetc(fh);
		if (current_char != EOF)
			dst_char_count++;

		if (current_char == EOF || current_char == '\n')
			read_complete = 1;
	}

	if (dst_char_count == 0)
		retaddr = NULL;

	return retaddr;
}

int local_isspace(char str)
{
	if ((str == ' ') || (str == '\t'))
		return 1;

	return 0;
}

static const char *locate_parameter_line(const char *parameter, const char *config_line)
{
	const char *config_addr;
	char first_non_wchar;

	config_addr = config_line;

	while (local_isspace(*config_addr))
		config_addr++;

	/* eliminate comment lines */
	first_non_wchar = *config_addr;
	if ((first_non_wchar == '\0') || (first_non_wchar == '#'))
		return NULL;

	/* does this line define the parameter? */
	unsigned int length_of_parameter = strlen(parameter);

	if (strncmp(parameter, config_addr, length_of_parameter) == 0) {
		char current_char;
		/*
		 * Parameter "wpa" matches "wpa_passphrase"
		 * But that of course is not a match.
		 * Eliminate situation here.
		 */
		config_addr += length_of_parameter;
		current_char = *config_addr;
		if (local_isspace(current_char) || (current_char == '='))
			return config_addr;
		else
			return NULL;
	}

	return NULL;
}

static int local_is_sub_parameter(const char *config_buffer, const char **sub_parameter_list)
{
	while (*sub_parameter_list) {
		if ((locate_parameter_line(*sub_parameter_list, config_buffer)))
			return 1;
		sub_parameter_list++;
	}
	return 0;
}

static int local_line_is_empty(const char *config_buffer)
{
	while (isspace(*config_buffer))
		config_buffer++;

	return *config_buffer == '\0' || *config_buffer == '#';
}

static int get_param_len(const char *value, char ch)
{
	int count = 0;

	if (value == NULL)
		return 0;

	while (*value) {
		if (*value == ch)
			return count;
		count++;
		value++;
	}

	return count;
}

static const char *local_skip_char(const char *buf, const char c, const int count)
{
	int n = 0;

	while (n < count && (buf = strchr(buf, c))) {
		buf++;
		n++;
	}

	return buf;
}

static int local_configured_value_equal(const char *config_line, const char *parameter,
					const char *value)
{
	int config_len;
	int value_len;
	const char *config_start = local_skip_char(config_line, '=', 1);

	if (!config_start)
		return 0;

	/*
	 * hs20_icon parameter format is
	 * <Icon Width>:<Icon Height>:<Language code>:<Icon Type>:<Name>:<file path>
	 * We consider icons with the same name to be equal.
	 */
	if (!strcmp(parameter, HS20_ICON_PARAM)) {
		const char *value_tmp;

		config_start = local_skip_char(config_start, ':', 4);
		if (!config_start)
			return 0;

		config_len = get_param_len(config_start, ':');

		value_tmp = local_skip_char(value, ':', 4);
		if (value_tmp) {
			value = value_tmp;
			value_len = get_param_len(value, ':');
		} else {
			/* When icon is deleted only icon name is passed */
			value_len = strlen(value);
		}
	} else if ((strcmp(parameter, IEEE_80211R_R0KH) == 0) ||
			(strcmp(parameter, IEEE_80211R_R1KH) == 0)) {
		config_len = get_param_len(config_start, ' ');
		value_len = get_param_len(value, ' ');
	} else {
		config_len = get_param_len(config_start, '\n');
		value_len = strlen(value);
	}

	return (value_len == config_len) && !strncmp(config_start, value, config_len);
}

static int parse_ap_config_with_sub_parameter(const char *ifname,
		SSID_parsing_state *p_parse_state,
		const char *parameter,
		char *config_line,
		const char *parameter_value,
		const char *sub_parameter, const char **sub_parameter_list)
{
	const char *config_addr = config_line;

	switch (*p_parse_state) {
	case e_searching_for_network:
		config_addr = locate_parameter_line("interface", config_line);
		if (!config_addr)
			config_addr = locate_parameter_line("bss", config_line);
		if (config_addr) {
			if (*config_addr == '=')
				config_addr++;
			if (strncmp(parameter, "interface", 9) == 0)
				return 1;	/* Looking up "interface" without interface name */
			if (strncmp(config_addr, ifname, strlen(ifname)) == 0) {
				char check_char = *(config_addr + strlen(ifname));

				if (isspace(check_char) || check_char == '\0') {
					*p_parse_state = e_found_current_network;
					if (strcmp(parameter, "bss") == 0) {
						/*
						 * Looking up parameter "bss" with given interface
						 * name.
						 */
						return 1;
					}
				}
			}
		}
		break;
	case e_found_current_network:
		config_addr = locate_parameter_line(parameter, config_line);
		if (config_addr != NULL) {
			if (sub_parameter) {
				if (local_configured_value_equal(config_addr, parameter,
								parameter_value))
					*p_parse_state = e_found_parent_value;
			} else {
				return 1;
			}
		} else if (locate_parameter_line("bss", config_line) != NULL) {
			*p_parse_state = e_parameter_not_found;
		}
		break;
	case e_found_parent_value:
		if (sub_parameter && locate_parameter_line(sub_parameter, config_line) != NULL)
			return 1;
		if (!(local_line_is_empty(config_line) ||
					(sub_parameter_list && local_is_sub_parameter(config_line,
									sub_parameter_list))))
			*p_parse_state = e_parameter_not_found;
		break;
	case e_searching_for_generic_param:
		config_addr = locate_parameter_line(parameter, config_line);
		if (config_addr != NULL) {
			if (sub_parameter) {
				if (local_configured_value_equal(config_addr, parameter,
								parameter_value))
					*p_parse_state = e_found_parent_value;
			} else {
				//printf("e_searching_for_generic_param found");
				return 1;
			}
		} else if (locate_parameter_line("interface", config_line) != NULL) {
			*p_parse_state = e_found_network_token;
		}
		break;
	default:
		break;
	}

	return 0;
}

int Is_general_parameter(const char *param)
{
	int i = 0;

	while (general_param_list[i] != NULL) {
		if (!strcmp(param, general_param_list[i]))
			return 1;
		i++;
	}
	return 0;
}

static int locate_ap_parameter_with_xfer(const char *ifname, const char *parameter, FILE *config_fh,
		FILE *temp_fh, char *config_buffer, const unsigned int sizeof_buffer,
		const char *parameter_value, const char *sub_parameter)
{
	int retval = E_PARAMETER_INVALID;
	int complete = 0;
	SSID_parsing_state e_parse_state = e_searching_for_network;
	const char **sub_parameter_list = NULL;

	if (ifname == NULL)
		return -EFAULT;

	if (sub_parameter)
		sub_parameter_list = local_get_sub_parameter_list(parameter);

	if ((strcmp(parameter, "interface") != 0) &&
			local_verify_interface_is_primary(ifname) == 0) {
		if (Is_general_parameter(parameter))
			e_parse_state = e_searching_for_generic_param;
		else {
			if (strcmp(parameter, "bss"))
				e_parse_state = e_found_current_network;
		}
	}
	//printf("before while complete:%d, e_parse_state=%d\n", complete, e_parse_state);

	while (complete == 0 &&
		read_config_line(config_buffer, sizeof_buffer, config_fh) != NULL) {
		complete = parse_ap_config_with_sub_parameter(ifname, &e_parse_state,
				parameter, config_buffer, parameter_value,
				sub_parameter, sub_parameter_list);
		//printf("complete:%d, e_parse_state=%d,", complete, e_parse_state);

		if (complete == 0) {
			if (!sub_parameter && e_parse_state == e_found_current_network)
				retval = E_PARAMETER_NOT_FOUND;
			else if (sub_parameter && e_parse_state == e_found_parent_value)
				retval = E_PARAMETER_NOT_FOUND;

			if (!sub_parameter && e_parse_state == e_searching_for_generic_param)
				retval = E_PARAMETER_NOT_FOUND;
			else if (sub_parameter && e_parse_state == e_found_parent_value)
				retval = E_PARAMETER_NOT_FOUND;
			//printf("retval = %d\n", retval);
			if (e_parse_state == e_parameter_not_found ||
				e_parse_state == e_found_network_token)
				complete = 1;
			else
				fprintf(temp_fh, "%s", config_buffer);
		} else {
			//printf("\nelse retval = %d, config_buff=[%s]\n", retval, config_buffer);
			retval = E_PARAMETER_FOUND;
		}
	}

	return retval;
}

static void ap_fprintf(FILE *temp_fh, const char *param, const char *value,
					int quote_flag)
{
	if (quote_flag)
		fprintf(temp_fh, "%s=\"%s\"\n", param, value);
	else
		fprintf(temp_fh, "%s=%s\n", param, value);
}

static int is_radius_server_param(const char *parameter,
		const char **port_param, const char **shared_secret_param)
{
	if (!strcmp(parameter, "auth_server_addr")) {
		*port_param = "auth_server_port";
		*shared_secret_param = "auth_server_shared_secret";
		return 1;
	} else if (!strcmp(parameter, "acct_server_addr")) {
		*port_param = "acct_server_port";
		*shared_secret_param = "acct_server_shared_secret";
		return 1;
	} else {
		return 0;
	}
}

static int value_equal(const char *config_val, const char *val)
{
	size_t config_len = get_param_len(config_val, '\n');
	size_t val_len = get_param_len(val, '\n');

	return (config_len == val_len) && !strncmp(config_val, val, config_len);
}

static void flush_buffer(FILE *fh, char *config_buffer)
{
	fprintf(fh, "%s", config_buffer);
	config_buffer[0] = '\0';
}

static int traverse_radius_config(FILE *config_fh, FILE *temp_fh,
		char *config_buffer, const unsigned int sizeof_buffer, const char *addr_param,
		const char *port_param, const char *shared_secret_param, const char *ipaddr,
		const char *port)
{
	int retval = 0;
	const char *config_addr;
	int count = 0;
	char *temp_config_buffer;

	temp_config_buffer = calloc(MAX_SECURITY_CONFIG_LENGTH, sizeof(char));
	if (!temp_config_buffer)
		return -ENOMEM;

	do {
		config_addr = locate_parameter_line(addr_param, config_buffer);
		if (config_addr) {
			config_addr++;
			count++;
			if (value_equal(config_addr, ipaddr)) {
				strncpy(temp_config_buffer, config_buffer,
						MAX_SECURITY_CONFIG_LENGTH - 1);
				read_config_line(config_buffer, sizeof_buffer, config_fh);
				config_addr = locate_parameter_line(port_param, config_buffer);
				if (config_addr) {
					config_addr++;
					if (value_equal(config_addr, port)) {
						retval = E_PARAMETER_FOUND;
					} else {
						if (count >= MAX_RADIUS_SERVERS)
							retval = E_PARAMETER_EXCEED_LIMIT;
						flush_buffer(temp_fh,
								temp_config_buffer);
						flush_buffer(temp_fh, config_buffer);
					}
				}
			} else {
				if (count >= MAX_RADIUS_SERVERS)
					retval = E_PARAMETER_EXCEED_LIMIT;
				flush_buffer(temp_fh, config_buffer);
			}
		} else if (locate_parameter_line(port_param, config_buffer) ||
				locate_parameter_line(shared_secret_param, config_buffer)) {
			fprintf(temp_fh, "%s", config_buffer);
		} else {
			retval = E_PARAMETER_NOT_FOUND;
		}
	} while (!retval && read_config_line(config_buffer, sizeof_buffer, config_fh));

	if (retval == E_PARAMETER_FOUND && read_config_line(config_buffer, sizeof_buffer, config_fh)
			&& locate_parameter_line(shared_secret_param, config_buffer)) {
		config_buffer[0] = '\0';
	}

	if (temp_config_buffer)
		free(temp_config_buffer);
	return retval;
}

static int update_radius_parameter(const char *parameter, const char *value_str,
		const int remove_param, FILE *config_fh, FILE *temp_fh, char *config_buffer,
		const unsigned int sizeof_buffer, const char *port_param,
		const char *shared_secret_param)
{
	int val;
	char *addr = NULL;
	char *port = NULL;
	char *shared_secret = NULL;
	char val_buf[256];

	strncpy(val_buf, value_str, sizeof(val_buf));
	addr = strtok(val_buf, ",");
	port = strtok(NULL, ",");
	shared_secret = strtok(NULL, ",");
	val_buf[sizeof(val_buf) - 1] = '\0';

	val = traverse_radius_config(config_fh, temp_fh,
			config_buffer,
			sizeof_buffer, parameter, port_param, shared_secret_param, addr, port);

	if (!remove_param && val != E_PARAMETER_EXCEED_LIMIT) {
		fprintf(temp_fh, "%s=%s\n", parameter, addr);
		fprintf(temp_fh, "%s=%s\n", port_param, port);
		fprintf(temp_fh, "%s=%s\n", shared_secret_param, shared_secret);
	}

	flush_buffer(temp_fh, config_buffer);

	if (remove_param && val == E_PARAMETER_EXCEED_LIMIT)
		val = E_PARAMETER_NOT_FOUND;

	return val;
}

static int traverse_hs20_conn_capab(FILE *config_fh, FILE *temp_fh,
		char *config_buffer, const unsigned int sizeof_buffer, const char *value)
{
	int retval = 0;
	const char *config_addr;
	int buf_len = 0;
	int value_len;
	char *pch;

	/* Config_buffer has starting line of hs20_conn_capab */
	if (config_buffer == NULL)
		return retval;
	if (value == NULL)
		return E_PARAMETER_NOT_FOUND;
	pch = strrchr(value, ':');
	if (!pch)
		return -EINVAL;

	value_len = pch - value;

	do {
		config_addr = locate_parameter_line("hs20_conn_capab", config_buffer);
		if (config_addr) {
			config_addr++;
			pch = strrchr(config_addr, ':');
			if (pch == NULL)
				continue;
			buf_len = pch - config_addr;
		} else {
			/* hs20_conn_capab param was not present in current line */
			return E_PARAMETER_NOT_FOUND;
		}

		if (!((value_len == buf_len) && (strncmp(config_addr, value, buf_len) == 0)))
			fprintf(temp_fh, "%s", config_buffer);
		else
			return E_PARAMETER_FOUND;
	} while (!retval && read_config_line(config_buffer, sizeof_buffer, config_fh));

	return retval;
}

static int traverse_param(FILE *config_fh, FILE *temp_fh,
		char *config_buffer, const unsigned int sizeof_buffer,
		const char *parameter, const char *value, const char **sub_parameter_list)
{
	const char *config_addr;

	/* Config_buffer has starting line of Param */
	if (config_buffer == NULL)
		return 0;

	do {
		config_addr = locate_parameter_line(parameter, config_buffer);
		if (config_addr) {
			if (!local_configured_value_equal(config_addr, parameter, value))
				fprintf(temp_fh, "%s", config_buffer);
			else
				return E_PARAMETER_FOUND;
		} else if ((sub_parameter_list &&
						local_is_sub_parameter(config_buffer,
								sub_parameter_list))
				|| local_line_is_empty(config_buffer))
			fprintf(temp_fh, "%s", config_buffer);
		else
			return E_PARAMETER_NOT_FOUND;
	} while (read_config_line(config_buffer, sizeof_buffer, config_fh));

	return 0;
}

static void local_skip_sub_parameters(FILE *config_fh, FILE *temp_fh, char *config_buffer,
		const unsigned int sizeof_buffer, const char **sub_parameter_list)
{
	if (!sub_parameter_list)
		return;

	while (read_config_line(config_buffer, sizeof_buffer, config_fh)) {
		if (local_line_is_empty(config_buffer)) {
			fprintf(temp_fh, "%s", config_buffer);
		} else if (!local_is_sub_parameter(config_buffer, sub_parameter_list)) {
			fprintf(temp_fh, "%s", config_buffer);
			break;
		}
	}
}


static int update_multi_parameter(const char *parameter, const char *value_str,
		const int remove_param, FILE *config_fh, FILE *temp_fh,
		char *config_buffer, const unsigned int sizeof_buffer)
{
	int retval = 0;
	int val;
	const char *config_addr;
	const char **sub_parameter_list = local_get_sub_parameter_list(parameter);
	const char *radius_port_param;
	const char *radius_shared_secret_param;

	if (is_radius_server_param(parameter,
					&radius_port_param, &radius_shared_secret_param)) {
		val = update_radius_parameter(parameter, value_str, remove_param,
				config_fh, temp_fh, config_buffer, sizeof_buffer,
				radius_port_param, radius_shared_secret_param);
		if (val == E_PARAMETER_EXCEED_LIMIT)
			retval = -1;
	} else if (strcmp(parameter, "hs20_conn_capab") == 0) {
		val = traverse_hs20_conn_capab(config_fh, temp_fh,
				&config_buffer[0], sizeof_buffer, value_str);
		if (remove_param == 0) {
			if (val == E_PARAMETER_FOUND) {
				config_addr = locate_parameter_line("hs20_conn_capab",
						config_buffer);
				if (config_addr) {
					config_addr++;
					if (!strncmp(config_addr, value_str, strlen(value_str)))
						retval = -1;
				}
			}
			fprintf(temp_fh, "%s=%s\n", parameter, value_str);
		}
	} else {
		val = traverse_param(config_fh, temp_fh,
				&config_buffer[0], sizeof_buffer, parameter, value_str,
				sub_parameter_list);

		if (remove_param == 0) {
			if (val == E_PARAMETER_FOUND)
				retval = -1;
			fprintf(temp_fh, "%s=%s\n", parameter, value_str);
		} else if (val == E_PARAMETER_FOUND) {
			local_skip_sub_parameters(config_fh, temp_fh, config_buffer,
					sizeof_buffer, sub_parameter_list);
		}
	}
	if (remove_param && val != E_PARAMETER_FOUND)
		retval = -1;

	/*
	 * Flush config_buffer before it is lost by complete_security_file_xfer.
	 */
	if (val == E_PARAMETER_NOT_FOUND)
		fprintf(temp_fh, "%s", config_buffer);

	return retval;
}

static int complete_security_file_xfer(FILE *config_fh, FILE *temp_fh, char *config_buffer,
		const unsigned int sizeof_buffer, int remove_null_line)
{
	int retval = 0;

	while (read_config_line(config_buffer, sizeof_buffer, config_fh) != NULL) {
		if (remove_null_line == 0 || strcmp(config_buffer, "\n"))
			fprintf(temp_fh, "%s", config_buffer);
	}

	return retval;
}

static int ap_update_param_ext(const char *ifname, FILE *config_fh,
		FILE *temp_fh, const char *param, const char *value, const int update_flag,
		const int quote_flag, const int remove_param, const int multi_flag,
		const char *sub_param, const char *sub_value)
{
	int retval = 0;
	int ival;
	char *config_buffer;

	config_buffer = calloc(MAX_SECURITY_CONFIG_LENGTH, sizeof(char));
	if (!config_buffer)
		return -ENOMEM;

	ival = locate_ap_parameter_with_xfer(ifname, param, config_fh,
			temp_fh, config_buffer, MAX_SECURITY_CONFIG_LENGTH, value, sub_param);

	if (ival == E_PARAMETER_FOUND || ival == E_PARAMETER_NOT_FOUND) {
		if (update_flag) {
			if (sub_param) {
				param = sub_param;
				value = sub_value;
			}

			if (!multi_flag) {
				if (!remove_param)
					ap_fprintf(temp_fh, param, value,
							quote_flag);
			} else {
				retval = update_multi_parameter(param, value,
						remove_param, config_fh, temp_fh, config_buffer,
						MAX_SECURITY_CONFIG_LENGTH);
			}
		}

		/*
		 * Need to flush config_buffer before it is lost by
		 * complete_security_file_xfer. If end of file has been
		 * reached config_buffer will have 0 length.
		 */
		if (!multi_flag && (ival == E_PARAMETER_NOT_FOUND))
			fprintf(temp_fh, "%s", &config_buffer[0]);
	} else {
		/* Network block or parent parameter was not found */
		retval = -EINVAL;
	}

	if (ival > 0)
		complete_security_file_xfer(config_fh, temp_fh,
			config_buffer, MAX_SECURITY_CONFIG_LENGTH, 0);
	if (config_buffer)
		free(config_buffer);

	return retval;
}

static int update_parameter(const char *ifname, const char *station_SSID,
		cls_ssid_fmt fmt, const char *parameter, const char *parameter_value,
		const cls_wifi_mode wifi_mode, const int update_flag, const int quote_flag,
		const int comp_update, const int remove_param, const int multi_flag,
		const char *sub_parameter, const char *sub_parameter_value)
{
	int retval = 0;
	FILE *config_fh = NULL;
	FILE *temp_fh = NULL;
	unsigned int radio_id;

	/*
	 * If the real User ID is not root, then abort now.
	 */
	if (getuid() != 0)
		retval = -EPERM;

	if (retval >= 0)
		retval = local_get_radio_id(ifname, &radio_id);

	if (retval >= 0)
		retval = open_file(radio_id, &config_fh, &temp_fh, wifi_mode, 0);

	if (retval >= 0) {
		if (wifi_mode == cls_mode_access_point)
			retval = ap_update_param_ext(ifname,
					config_fh, temp_fh, parameter,
					parameter_value, update_flag, quote_flag,
					remove_param, multi_flag,
					sub_parameter, sub_parameter_value);
		close_file(radio_id, &config_fh, &temp_fh, wifi_mode, 1);
	}

	return retval;
}

void update_interface_param(const char *ifname, char *param, char *value,
			cls_wifi_mode wifi_mode, int action)
{
	if (action == 0) {
		update_parameter(ifname, NULL, cls_ssid_fmt_any,
		param, value, wifi_mode, CLSAPI_TRUE,
		cls_bare_string, security_update_complete, 0, 0, NULL, NULL);
	} else {
		update_parameter(ifname, NULL, cls_ssid_fmt_any,
		param, NULL, wifi_mode, CLSAPI_TRUE,
		cls_bare_string, security_update_complete, 1, 0, NULL, NULL);
	}
}

int is_param_with_sub(char *param_name)
{
	int i = 0;

	for (; i < ARRAY_SIZE(param_with_sub); i++) {
		if (!strcmp(param_name, param_with_sub[i].param_name))
			return 1;
	}
	return 0;
}

int remove_parameter_with_sub(const char *ifname, const char *param_name)
{
	const char **sublist = local_get_sub_parameter_list(param_name);

	// remove parameter
	update_parameter(ifname, NULL, cls_ssid_fmt_any,
	param_name, NULL, cls_mode_access_point, CLSAPI_TRUE,
	cls_bare_string, security_update_complete, 1, 0, NULL, NULL);

	// remove sub parameter
	int i = 0;

	while (sublist[i] != NULL) {
		update_parameter(ifname, NULL, cls_ssid_fmt_any,
		sublist[i], NULL, cls_mode_access_point, CLSAPI_TRUE,
		cls_bare_string, security_update_complete, 1, 0, NULL, NULL);
		i++;
	}

	return 0;
}

void reconfig_interface_config(char *ifname)
{
	char cmd[128];

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "hostapd_cli -i %s reconfigure &", ifname);
	if (system(cmd) == -1)
		return;

	return;
}

void reset_interface_config(char *ifname, char *type, char *program)
{
	char cmd[128];
	unsigned int radio = 0;

	// to act according to type

	if (-1 == local_get_radio_id(ifname, &radio))
		return;
	memset(cmd, 0, sizeof(cmd));

	sprintf(cmd, "cp /etc/hostapd-phy%u_default.conf %s/hostapd-phy%u.conf",
			radio, CLS_DEF_HOSTAD_DIR, radio);
	if (system(cmd) == -1)
		return;

	reconfig_interface_config(ifname);
}

int get_option_value(int radio, char *mode, char *width,  char *chan,
				char *option_name, char *option_value)
{
	int channel = atoi(chan);
	int chan_ofs = 0;
	int idx = channel;
	int tmp  = 0;

	switch (radio) {
	case 0:
		if (!strcmp(mode, "11g")) {
			;
		} else if (!strcmp(mode, "11n")) {
			if (!strcmp(width, "20")) {
				if (!strcmp(option_name, "ieee80211n"))
					sprintf(option_value, "%s", "1");
				else if (!strcmp(option_name, "ht_coex"))
					sprintf(option_value, "%s", "0");
				else if (!strcmp(option_name, "ht_capab"))
					sprintf(option_value, "%s", default_ht_capab);
			} else if (!strcmp(width, "40")) {
				if (!strcmp(option_name, "ieee80211n")) {
					sprintf(option_value, "%s", "1");
				} else if (!strcmp(option_name, "ht_coex")) {
					sprintf(option_value, "%s", "0");
				} else if (!strcmp(option_name, "ht_capab")) {
					sprintf(option_value, "[HT40+]");
					strcat(option_value, default_ht_capab);
				}
			}
		} else if (!strcmp(mode, "11ax")) {
			if (!strcmp(option_name, "ieee80211ax"))
				sprintf(option_value, "%s", "1");
			else if (!strcmp(option_name, "he_default_pe_duration"))
				sprintf(option_value, "%s", "4");
		}
		break;

	case 1:
		if (!strcmp(mode, "11a")) {
			;
		} else if (!strcmp(mode, "11n")) {
			if (!strcmp(width, "20")) {
				if (!strcmp(option_name, "ieee80211n"))
					sprintf(option_value, "%s", "1");
				else if (!strcmp(option_name, "ht_coex"))
					sprintf(option_value, "%s", "0");
				else if (!strcmp(option_name, "ht_capab"))
					sprintf(option_value, "%s", default_ht_capab);
			} else if (!strcmp(width, "40")) {
				if (!strcmp(option_name, "ieee80211n")) {
					sprintf(option_value, "%s", "1");
				} else if (!strcmp(option_name, "ht_coex")) {
					sprintf(option_value, "%s", "0");
				} else if (!strcmp(option_name, "ht_capab")) {
					sprintf(option_value, "[HT40+]");
					strcat(option_value, default_ht_capab);
				}
			}
		} else if (!strcmp(mode, "11ac")) {
			if (!strcmp(width, "20")) {
				if (!strcmp(option_name, "ieee80211ac"))
					sprintf(option_value, "%s", "1");
				else if (!strcmp(option_name, "vht_oper_chwidth"))
					sprintf(option_value, "%s", "0");
				else if (!strcmp(option_name, "vht_capab"))
					sprintf(option_value, "%s", default_vht_capab);
			} else if (!strcmp(width, "40")) {
				if (!strcmp(option_name, "ieee80211ac")) {
					sprintf(option_value, "%s", "1");
				} else if (!strcmp(option_name, "vht_oper_chwidth")) {
					sprintf(option_value, "%s", "0");
				} else if (!strcmp(option_name, "vht_oper_centr_freq_seg0_idx")) {
					tmp = (channel / 4 + chan_ofs) % 2;
					if (tmp == 0)
						idx = channel - 2;
					else
						idx = channel + 2;
					sprintf(option_value, "%d", idx);
				} else if (!strcmp(option_name, "vht_capab")) {
					sprintf(option_value, "%s", default_ht_capab);
				}
			} else if (!strcmp(width, "80")) {
				if (!strcmp(option_name, "ieee80211ac")) {
					sprintf(option_value, "%s", "1");
				} else if (!strcmp(option_name, "vht_oper_chwidth")) {
					sprintf(option_value, "%s", "1");
				} else if (!strcmp(option_name, "vht_oper_centr_freq_seg0_idx")) {
					tmp = (channel / 4 + chan_ofs) % 4;
					switch (tmp) {
					case 0:
						idx =  channel - 6;
						break;
					case 1:
						idx =  channel + 6;
						break;
					case 2:
						idx =  channel + 2;
						break;
					case 3:
						idx =  channel - 2;
						break;
					}
					sprintf(option_value, "%d", idx);
				} else if (!strcmp(option_name, "vht_capab")) {
					sprintf(option_value, "%s", default_ht_capab);
				}
			} else if (!strcmp(width, "160")) {
				if (!strcmp(option_name, "ieee80211ac")) {
					sprintf(option_value, "%s", "1");
				} else if (!strcmp(option_name, "vht_oper_chwidth")) {
					sprintf(option_value, "%s", "2");
				} else if (!strcmp(option_name, "vht_oper_centr_freq_seg0_idx")) {
					if (channel == 36 || channel == 40 || channel == 44 ||
						channel == 48 || channel == 52 || channel == 56 ||
						channel == 60 || channel == 64) {
						idx  = 50;
						sprintf(option_value, "%d", idx);
					} else if (channel == 100 || channel == 104 ||
						channel == 108 || channel == 112 ||
						channel == 116 || channel == 120 ||
						channel == 124 || channel == 128) {
						idx = 114;
						sprintf(option_value, "%d", idx);
					}
				} else if (!strcmp(option_name, "vht_capab")) {
					sprintf(option_value, "%s", default_ht_capab);
				}
			}
		} else if (!strcmp(mode, "11ax")) {
			if (!strcmp(width, "20")) {
				if (!strcmp(option_name, "ieee80211ax"))
					sprintf(option_value, "%s", "1");
				else if (!strcmp(option_name, "he_oper_chwidth"))
					sprintf(option_value, "%s", "0");
				else if (!strcmp(option_name, "he_default_pe_duration"))
					sprintf(option_value, "%s", "4");
			} else if (!strcmp(width, "40")) {
				if (!strcmp(option_name, "ieee80211ax")) {
					sprintf(option_value, "%s", "1");
				} else if (!strcmp(option_name, "he_oper_chwidth")) {
					sprintf(option_value, "%s", "0");
				} else if (!strcmp(option_name, "he_oper_centr_freq_seg0_idx")) {
					tmp = (channel / 4 + chan_ofs) % 2;
					if (tmp == 0)
						idx = channel - 2;
					else
						idx = channel + 2;
					sprintf(option_value, "%d", idx);
				} else if (!strcmp(option_name, "he_default_pe_duration")) {
					sprintf(option_value, "%s", "4");
				}
			} else if (!strcmp(width, "80")) {
				if (!strcmp(option_name, "ieee80211ax")) {
					sprintf(option_value, "%s", "1");
				} else if (!strcmp(option_name, "he_oper_chwidth")) {
					sprintf(option_value, "%s", "1");
				} else if (!strcmp(option_name, "he_oper_centr_freq_seg0_idx")) {
					tmp = (channel / 4 + chan_ofs) % 4;
					switch (tmp) {
					case 0:
						idx =  channel - 6;
						break;
					case 1:
						idx =  channel + 6;
						break;
					case 2:
						idx =  channel + 2;
						break;
					case 3:
						idx =  channel - 2;
						break;
					}
					sprintf(option_value, "%d", idx);
				} else if (!strcmp(option_name, "he_default_pe_duration")) {
					sprintf(option_value, "%s", "4");
				}
			} else if (!strcmp(width, "160")) {
				if (!strcmp(option_name, "ieee80211ax")) {
					sprintf(option_value, "%s", "1");
				} else if (!strcmp(option_name, "he_oper_chwidth")) {
					sprintf(option_value, "%s", "2");
				} else if (!strcmp(option_name, "he_oper_centr_freq_seg0_idx")) {
					if (channel == 36 || channel == 40 || channel == 44 ||
						channel == 48 || channel == 52 || channel == 56 ||
						channel == 60 || channel == 64) {
						idx  = 50;
						sprintf(option_value, "%d", idx);
					} else if (channel == 100 || channel == 104 ||
						channel == 108 || channel == 112 ||
						channel == 116 || channel == 120 ||
						channel == 124 || channel == 128) {
						idx = 114;
						sprintf(option_value, "%d", idx);
					}
				} else if (!strcmp(option_name, "he_default_pe_duration")) {
					sprintf(option_value, "%s", "4");
				}
			}

		}
		break;

	default:
		;
	}

	return 0;
}

int check_band_mode_channel_match(int band, char *mode, char *width, char *channel)
{
	int  chan = 0;

	chan = atoi(channel);

	if (band == 0) {//2.4G
		if (strcmp(mode, "11b") && strcmp(mode, "11g") && strcmp(mode, "11n")
			&& strcmp(mode, "11ax"))
			return 0;
		if (chan < 0 || chan > 14)
			return 0;

	} else if (band == 1) {
		if (strcmp(mode, "11a") && strcmp(mode, "11n") &&
			strcmp(mode, "11ac") && strcmp(mode, "11ax"))
			return 0;
	}
		return 1;

}

int set_interface_mode_width_channel(char *ifname, char *mode, char *orig_width, char *orig_channel)
{
	unsigned int radio = 255;
	int i = 0;
	char opt_val[128];
	char width[8];
	char channel[8];
	int ret;


	ret = local_get_radio_id(ifname, &radio);
	if (ret == -1) {
		printf("%s: get band error ", __func__);
		return -1;
	}

	if (orig_width && orig_channel &&
		!check_band_mode_channel_match(radio, mode, orig_width, orig_channel)) {
		printf("%s: band,mode,channel do not match", __func__);
		return -1;
	}

	memset(width, 0, sizeof(width));
	memset(channel, 0, sizeof(width));

	if (!orig_channel && !orig_width) {
		if (radio == 0) {
			if (!strcmp(mode, "11b") || !strcmp(mode, "11g")) {
				sprintf(width, "%s", "20");
				sprintf(channel, "%s", "6");
			} else if (!strcmp(mode, "11n")) {
				sprintf(width, "%s", "40");
				sprintf(channel, "%s", "6");
			} else if (!strcmp(mode, "11ax")) {
				sprintf(width, "%s", "80");
				sprintf(channel, "%s", "6");
			}
		} else {
			if (!strcmp(mode, "11a")) {
				sprintf(width, "%s", "20");
				sprintf(channel, "%s", "36");
			} else if (!strcmp(mode, "11n")) {
				sprintf(width, "%s", "40");
				sprintf(channel, "%s", "36");
			} else if (!strcmp(mode, "11ac") || !strcmp(mode, "11ax")) {
				sprintf(width, "%s", "80");
				sprintf(channel, "%s", "36");
			}
		}
	} else if (!orig_channel) {
		if (radio == 0)
			sprintf(channel, "%s", "6");
		else
			sprintf(channel, "%s", "36");
	} else if (!orig_width) {
		if (radio == 0) {
			if (!strcmp(mode, "11b") || !strcmp(mode, "11g"))
				sprintf(width, "%s", "20");
			else if (!strcmp(mode, "11n"))
				sprintf(width, "%s", "40");
			else if (!strcmp(mode, "11ax"))
				sprintf(width, "%s", "80");
		} else {
			if (!strcmp(mode, "11a"))
				sprintf(width, "%s", "20");
			else if (!strcmp(mode, "11n"))
				sprintf(width, "%s", "40");
			else if (!strcmp(mode, "11ac") || !strcmp(mode, "11ax"))
				sprintf(width, "%s", "80");
		}
	}
	if (orig_channel)
		sprintf(channel, "%s", orig_channel);
	if (orig_width)
		sprintf(width, "%s", orig_width);
	printf("%s:setd mode[%s], width[%s], channel[%s]\n", __func__, mode, width, channel);

	memset(opt_val, 0, sizeof(opt_val));
	switch (radio) {
	case 0: //2.4G
		if (!strcmp(mode, "11b")) {
			update_interface_param(ifname, "channel", channel,
				cls_mode_access_point, 0);
			update_interface_param(ifname, "chanlist", channel,
				cls_mode_access_point, 0);
			update_interface_param(ifname, "hw_mode", "b",
				cls_mode_access_point, 0);
			update_interface_param(ifname, "supported_rates", NULL,
				cls_mode_access_point, 1);
			update_interface_param(ifname, "basic_rates", NULL,
				cls_mode_access_point, 1);
		} else if (!strcmp(mode, "11g")) {
			update_interface_param(ifname, "channel", channel,
				cls_mode_access_point, 0);
			update_interface_param(ifname, "chanlist", channel,
				cls_mode_access_point, 0);
		} else if (!strcmp(mode, "11n")) {
			update_interface_param(ifname, "channel", channel,
				cls_mode_access_point, 0);
			update_interface_param(ifname, "chanlist", channel,
				cls_mode_access_point, 0);
			while (ht_option_list[i] != NULL) {
				memset(opt_val, 0, sizeof(opt_val));
				get_option_value(radio, mode, width, channel,
					ht_option_list[i], opt_val);
				update_interface_param(ifname, ht_option_list[i], opt_val,
					cls_mode_access_point, 0);
				i++;
			}
		} else if (!strcmp(mode, "11ax")) {
			update_interface_param(ifname, "channel", channel,
				cls_mode_access_point, 0);
			update_interface_param(ifname, "chanlist", channel,
				cls_mode_access_point, 0);
			if (!strcmp(width, "20")) {
				while (ht_option_list[i] != NULL) {
					memset(opt_val, 0, sizeof(opt_val));
					get_option_value(radio, "11n", "20", channel,
						ht_option_list[i], opt_val);
					update_interface_param(ifname, ht_option_list[i], opt_val,
						cls_mode_access_point, 0);
					i++;
				}
			} else if (!strcmp(width, "40")) {
				while (ht_option_list[i] != NULL) {
					memset(opt_val, 0, sizeof(opt_val));
					get_option_value(radio, "11n", "40", channel,
						ht_option_list[i], opt_val);
					update_interface_param(ifname, ht_option_list[i],
						opt_val, cls_mode_access_point, 0);
					i++;
				}
			}
			get_option_value(radio, mode, width, channel,
				"ieee80211ax", opt_val);
			update_interface_param(ifname, "ieee80211ax", opt_val,
				cls_mode_access_point, 0);
			get_option_value(radio, mode, width, channel,
				"he_default_pe_duration", opt_val);
			update_interface_param(ifname, "he_default_pe_duration",
				opt_val, cls_mode_access_point, 0);
		}
		break;
	case 1://5G
		if (!strcmp(mode, "11a")) {
			update_interface_param(ifname, "channel", channel,
				cls_mode_access_point, 0);
			update_interface_param(ifname, "chanlist", channel,
				cls_mode_access_point, 0);
		} else if (!strcmp(mode, "11n")) {
			update_interface_param(ifname, "channel", channel,
				cls_mode_access_point, 0);
			update_interface_param(ifname, "chanlist", channel,
				cls_mode_access_point, 0);
			while (ht_option_list[i] != NULL) {
				memset(opt_val, 0, sizeof(opt_val));
				get_option_value(radio, mode, width, channel,
					ht_option_list[i], opt_val);
				update_interface_param(ifname, ht_option_list[i],
					opt_val, cls_mode_access_point, 0);
				i++;
			}
		} else if (!strcmp(mode, "11ac")) {
			update_interface_param(ifname, "channel", channel,
				cls_mode_access_point, 0);
			update_interface_param(ifname, "chanlist", channel,
				cls_mode_access_point, 0);
			if (!strcmp(width, "20")) {
				//11n,20M
				while (ht_option_list[i] != NULL) {
					memset(opt_val, 0, sizeof(opt_val));
					get_option_value(radio, "11n", "20", channel,
						ht_option_list[i], opt_val);
					update_interface_param(ifname, ht_option_list[i], opt_val,
						cls_mode_access_point, 0);
					i++;
				}
				i = 0;
				//11ac,20M
				while (vht_option_list[i] != NULL) {
					memset(opt_val, 0, sizeof(opt_val));
					get_option_value(radio, mode, width, channel,
						vht_option_list[i], opt_val);
					update_interface_param(ifname, vht_option_list[i],
						opt_val, cls_mode_access_point, 0);
					i++;
				}

			}  else if (!strcmp(width, "40") || !strcmp(width, "80") ||
					!strcmp(width, "160")) {
				//11n,40M
				while (ht_option_list[i] != NULL) {
					memset(opt_val, 0, sizeof(opt_val));
					get_option_value(radio, "11n", "40", channel,
						ht_option_list[i], opt_val);
					update_interface_param(ifname, ht_option_list[i], opt_val,
						cls_mode_access_point, 0);
					i++;
				}
				i = 0;
				//11ac
				while (vht_option_list[i] != NULL) {
					memset(opt_val, 0, sizeof(opt_val));
					get_option_value(radio, mode, width, channel,
						vht_option_list[i], opt_val);
					update_interface_param(ifname, vht_option_list[i],
						opt_val, cls_mode_access_point, 0);
					i++;
				}
			}
		} else if (!strcmp(mode, "11ax")) {
			update_interface_param(ifname, "channel", channel,
				cls_mode_access_point, 0);
			update_interface_param(ifname, "chanlist", channel,
				cls_mode_access_point, 0);
			if (!strcmp(width, "20")) {
				//11n,20M
				while (ht_option_list[i] != NULL) {
					memset(opt_val, 0, sizeof(opt_val));
					get_option_value(radio, "11n", "20", channel,
						ht_option_list[i], opt_val);
					update_interface_param(ifname, ht_option_list[i],
						opt_val, cls_mode_access_point, 0);
					i++;
				}
				i = 0;
				//11ac,20M
				while (vht_option_list[i] != NULL) {
					memset(opt_val, 0, sizeof(opt_val));
					get_option_value(radio, "11ac", "20", channel,
						vht_option_list[i], opt_val);
					update_interface_param(ifname, vht_option_list[i],
						opt_val, cls_mode_access_point, 0);
					i++;
				}
				i = 0;
				//11ax
				while (he_option_list[i] != NULL) {
					memset(opt_val, 0, sizeof(opt_val));
					get_option_value(radio, mode, width, channel,
						he_option_list[i], opt_val);
					update_interface_param(ifname, he_option_list[i],
						opt_val, cls_mode_access_point, 0);
					i++;
				}
			} else if (!strcmp(width, "40") || !strcmp(width, "80") ||
				!strcmp(width, "160")) {
				//11n,40M
				while (ht_option_list[i] != NULL) {
					memset(opt_val, 0, sizeof(opt_val));
					get_option_value(radio, "11n", "40", channel,
						ht_option_list[i], opt_val);
					update_interface_param(ifname, ht_option_list[i],
						opt_val, cls_mode_access_point, 0);
					i++;
				}
				i = 0;
				//11ac,width
				while (vht_option_list[i] != NULL) {
					memset(opt_val, 0, sizeof(opt_val));
					get_option_value(radio, "11ac", width, channel,
						vht_option_list[i], opt_val);
					update_interface_param(ifname, vht_option_list[i],
						opt_val, cls_mode_access_point, 0);
					i++;
				}
				i = 0;
				//11ax
				while (he_option_list[i] != NULL) {
					memset(opt_val, 0, sizeof(opt_val));
					get_option_value(radio, mode, width, channel,
						he_option_list[i], opt_val);
					update_interface_param(ifname, he_option_list[i],
						opt_val, cls_mode_access_point, 0);
					i++;
				}
			}
		}
		break;
	default:
		;
	}
	return 0;
}


int check_ap_config(const char *ifname, SSID_parsing_state *parse_state,
			char *param, char *configline)
{
	parse_ap_config_with_sub_parameter(ifname, parse_state, "bss",
			configline, ifname, NULL, NULL);
	return 0;
}

int found_bss_ifname(int radio, const char *ifname, int *found)
{
	int retval = 0;
	char config_file[CLSAPI_SEC_FILE_LEN_MAX];
	char config_line[1024];
	const int filename_len = sizeof(config_file);
	FILE *config_fh = NULL;
	SSID_parsing_state e_parse_state = e_searching_for_network;

	retval = get_config_file_path(radio, cls_mode_access_point, config_file, filename_len, "");

	if (retval >= 0)
		config_fh = fopen(config_file, "r");

	if (config_fh == NULL) {
		retval = -1;
		return retval;
	} else {
		while (fgets(config_line, sizeof(config_line), config_fh) != NULL) {
			check_ap_config(ifname, &e_parse_state, "bss", config_line);
			if (e_parse_state == e_found_current_network) {
				*found = CLSAPI_TRUE;
				break;
			}
		}
		if (e_parse_state != e_found_current_network)
			*found = CLSAPI_FALSE;
	}

	if (config_fh != NULL)
		fclose(config_fh);

	return retval;
}

int add_default_bss_config(int radio, char *ifname, const struct vbss_vap_info *vap)
{
	int retval = 0;
	FILE *config_fh = NULL;
	FILE *temp_fh = NULL;
	int ival = 0;
	char *config_buffer;
	struct vap_config *pconfig = def_vap_config;

	config_buffer = calloc(MAX_SECURITY_CONFIG_LENGTH, sizeof(char));
	if (!config_buffer)
		return -ENOMEM;

	if (retval >= 0)
	retval = open_file(radio, &config_fh, &temp_fh, cls_mode_access_point, 0);

	ival = locate_ap_parameter_with_xfer(ifname, "bss", config_fh,
			temp_fh, config_buffer, MAX_SECURITY_CONFIG_LENGTH, NULL, NULL);
	if (ival != E_PARAMETER_FOUND) {
		fprintf(temp_fh, "\n");
		fprintf(temp_fh, "bss=%s\n", ifname);
		while (pconfig->option_name != NULL) {
			fprintf(temp_fh, "%s=%s\n", pconfig->option_name, pconfig->option_value);
			pconfig++;
		}
		if (vap->auth_type == WPA_AUTHORIZE_TYPE_WPA2) {
			fprintf(temp_fh, "wpa_passphrase=%s\n", vap->pwd);
			fprintf(temp_fh, "wpa=%d\n", vap->auth_type);
			fprintf(temp_fh, "wpa_pairwise=CCMP\n");
			fprintf(temp_fh, "wpa_key_mgmt=WPA-PSK\n");
		} else {
			fprintf(temp_fh, "wpa=0\n");
		}
		fprintf(temp_fh, "ssid=%s\n", vap->ssid);
		fprintf(temp_fh, "bssid=" MACSTR "\n", MAC2STR(vap->bssid));
	}

	close_file(radio, &config_fh, &temp_fh, cls_mode_access_point, 1);

	return 0;
}

int remove_bss(int radio, const char *ifname)
{
	int retval = 0;
	int found = 0;
	FILE *config_fh = NULL;
	FILE *temp_fh = NULL;
	char config_line[1024];
	int is_removed = 0;
	char dst_bss_line[32] = {0};
	int flag = 0;

	found_bss_ifname(radio, ifname, &found);

	if (!found) {
		printf("the bss doesn't exists!!\n");
		return -1;
	}
	retval = open_file(radio, &config_fh, &temp_fh, cls_mode_access_point, 0);
	if (retval < 0)
		return -1;
	snprintf(dst_bss_line, sizeof(dst_bss_line) - 1, "bss=%s", ifname);
	while (fgets(config_line, sizeof(config_line), config_fh) != NULL) {
		if (!is_removed && !flag) {
			if (!strncmp(config_line, dst_bss_line, strlen(dst_bss_line))) {
				is_removed = 1;
				flag = 1;
			}
		} else if (flag && is_removed) {
			if (!strncmp(config_line, "bss=", strlen("bss=")))
				is_removed = 0;
		}
		if (!is_removed)
			fputs(config_line, temp_fh);
	}
	close_file(radio, &config_fh, &temp_fh, cls_mode_access_point, 1);
	return 0;
}

int get_interface_idx(char *ifname, int *index)
{
	char *p = NULL;

	p = strrchr(ifname, '_');
	if (!p)
		return 0;
	*index = *(p + 1) - '0';
	return 0;
}

int get_interface_mac(const char *ifname, uint8_t *mac)
{
	int sock;
	struct ifreq ifr;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		printf("socket error");
		return 0;
	}
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);

	if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0) {
		printf("ioctl error");
		close(sock);
		return 0;
	}
	close(sock);
	memcpy(mac, (unsigned char *)ifr.ifr_hwaddr.sa_data, ETH_ALEN);
	return 1;
}

int get_primary_vap_name(int radio, char *name, int len)
{
	snprintf(name, len, "wlan%d", radio);
	return 0;
}

int create_new_bss_config(char *ifname, const struct vbss_vap_info *vap)
{
	unsigned int radio_id;
	char primary_name[32];
	int found = 0;

	memset(primary_name, 0, sizeof(primary_name));
	local_get_radio_id(ifname, &radio_id);
	get_primary_vap_name(radio_id, primary_name, sizeof(primary_name) - 1);

	// to find bss which name is ifname
	found_bss_ifname(radio_id, ifname, &found);

	if (found) {
		printf("the bss exists!!\n");
		return 0;
	}

	add_default_bss_config(radio_id, ifname, vap);

	return 0;
}

