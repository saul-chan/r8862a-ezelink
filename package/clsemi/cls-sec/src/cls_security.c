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
#include <stdint.h>

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

#define ETH_MACADDR_LEN 6

#define ARRAY_SIZE(a) (sizeof((a))/sizeof((a)[0]))

char *auth_server_param_list[] = {"auth_server_addr", "auth_server_port",
					"auth_server_shared_secret"};

char *general_param_list[] = {"hw_mode", "supported_rates", "basic_rates", "beacon_int", "channel",
				"chanlist", "ieee80211n", "ht_coex", "ht_capab", "ieee80211ac",
				"vht_oper_chwidth", "vht_oper_centr_freq_seg0_idx", "vht_capab",
				"ieee80211ax", "he_oper_chwidth", "he_oper_centr_freq_seg0_idx",
				"he_default_pe_duration", NULL};

char *ht_option_list[] = {"ieee80211n", "ht_coex", "obss_interval", "ht_capab", NULL};

char *vht_option_list[] = {"ieee80211ac", "vht_oper_chwidth", "vht_oper_centr_freq_seg0_idx",
				"vht_capab", NULL};

char *he_option_list[] = {"ieee80211ax", "he_oper_chwidth", "he_oper_centr_freq_seg0_idx",
				"he_default_pe_duration", NULL};

int chan_5g_list[] = {36, 40, 44, 48, 52, 56, 60, 64, 149, 153, 157, 161, 165};

char *default_ht_capab = "[LDPC][SHORT-GI-20][SHORT-GI-40][TX-STBC][RX-STBC1][MAX-AMSDU-7935]";

char *default_vht_capab = "[RXLDPC][SHORT-GI-80][SHORT-GI-160][TX-STBC-2BY1][SU-BEAMFORMER]"
			"[MU-BEAMFORMER][RX-STBC-1][SOUNDING-DIMENSION-2][VHT160]"
			"[MAX-MPDU-7991][MAX-A-MPDU-LEN-EXP7]";

char *bcc_ht_capab = "[SHORT-GI-20][SHORT-GI-40][TX-STBC][RX-STBC1][MAX-AMSDU-7935]";
char *bcc_vht_capab = "[SHORT-GI-80][SHORT-GI-160][TX-STBC-2BY1][SU-BEAMFORMER]"
			"[MU-BEAMFORMER][RX-STBC-1][SOUNDING-DIMENSION-2][VHT160]"
			"[MAX-MPDU-7991][MAX-A-MPDU-LEN-EXP7]";

char *special_config_list[] = {"mode", "bw", "channel", "payload_code", "sgi20", "sideband", NULL};

char *sgi20_disable_ht_vacap = "[LDPC][SHORT-GI-40][TX-STBC][RX-STBC1][MAX-AMSDU-7935]";

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
	{"wpa", "0"},
	{"ssid", "new"},
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

enum cls_ap_radio {
	CLS_AP_RADIO_24G,
	CLS_AP_RADIO_5G,
	CLS_AP_RADIO_6G,
	CLS_AP_RADIO_MAX
};

struct cls_param_with_sub {
	const char *param_name;
	int has_sub_param;//0: single param 1:has sub param
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

int get_interface_mac(char *ifname, uint8_t *mac);
int found_ap_radio_param_value(const char *ifname, char *para_name, char *value);

/*
 * check whether the param is in the special list
 * (mode, channel, bw, payload_code, sgi20)
 */
int is_special_parameter(const char *param)
{
	int i = 0;

	while (special_config_list[i] != NULL) {
		if (!strcasecmp(param, special_config_list[i]))
			return 1;
		i++;
	}
	return 0;
}

int local_get_radio_id(const char *ifname, unsigned int *radio_id)
{
	const char *ifname_format = "wlan%u";
	int radio;
	FILE *fp = NULL;
	char path[64];

	if (sscanf(ifname, ifname_format, &radio) != 1)
		return -1;

	if (radio < 0 || radio >= MAX_RADIO_NUM) {
		snprintf(path, sizeof(path) - 1, "/sys/class/net/%s/phy80211/index", ifname);
		fp = fopen(path, "r");
		if (!fp)
			return -1;
		if (fscanf(fp, "%d", &radio) != 1) {
			fclose(fp);
			return -1;
		}
		fclose(fp);
		*radio_id = radio;
		return 0;
	}

	*radio_id = radio;

	return 0;
}

static int cls_security_get_sec_dir(char *buf, int buflen)
{
	int retval;
	char *seca_str;
	int seca_enabled = 0;
	char tmp_dir[CLSAPI_SEC_FILE_LEN_MAX];

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
		retval = -errno;
		goto out;
	}

	/* Note:
	 * - Closing fd would release the lock
	 * - Do not duplicate fd. Closing the duplicated fd would also release the lock
	 */

	*p_config_fh = fdopen(fd, "rw");
	if (!*p_config_fh) {
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
	char cmd[256];

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

int local_verify_interface_is_primary(const char *ifname)
{
	int retval = 1;
	int radio_id = -1;

	local_get_radio_id(ifname, &radio_id);
	if (radio_id < 0 || radio_id >= MAX_RADIO_NUM)
		return retval;

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

static int get_local_configured_value(const char *config_line, const char *parameter,
					char *value)
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

		config_start = local_skip_char(config_start, ':', 4);
		if (!config_start)
			return 0;

		config_len = get_param_len(config_start, ':');

	} else if ((strcmp(parameter, IEEE_80211R_R0KH) == 0) ||
			(strcmp(parameter, IEEE_80211R_R1KH) == 0)) {
		config_len = get_param_len(config_start, ' ');
	} else {
		config_len = get_param_len(config_start, '\n');
	}

	strncpy(value, config_start, config_len);
	return config_len;
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
		} else if (locate_parameter_line("radio_config_id", config_line) != NULL) {
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
	int i = 0;

	// remove parameter
	update_parameter(ifname, NULL, cls_ssid_fmt_any,
	param_name, NULL, cls_mode_access_point, CLSAPI_TRUE,
	cls_bare_string, security_update_complete, 1, 0, NULL, NULL);

	// remove sub parameter
	while (sublist[i] != NULL) {
		update_parameter(ifname, NULL, cls_ssid_fmt_any,
		sublist[i], NULL, cls_mode_access_point, CLSAPI_TRUE,
		cls_bare_string, security_update_complete, 1, 0, NULL, NULL);
		i++;
	}

	return 0;
}

int is_interface_up(const char *interface)
{
	int sockfd;
	struct ifreq ifr;


	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1) {
		perror("socket");
		return -1;
	}


	strncpy(ifr.ifr_name, interface, IFNAMSIZ);

	if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) == -1) {
		perror("ioctl");
		close(sockfd);
		return -1;
	}

	close(sockfd);
	return (ifr.ifr_flags & IFF_UP) ? 1 : 0;
}

void reconfig_interface_config(char *ifname)
{
	char cmd[128];
	if (!ifname)
		return;
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "hostapd_cli -i %s reconfigure &", ifname);
	system(cmd);
	if (!strcmp(ifname, "wlan1"))
		sleep(50);
}

void del_mboconf_file(char *path)
{
	char cmd[128] = {0};

	sprintf(cmd, "rm -f %s", path);
	system(cmd);
}

struct vap_config mbo_params[] = {
	/*** MBO TestBed - case4.2.1 need the below configs as OOB conf ***/
	{"bss_transition", "1"},
	{"interworking", "1"},
	{"internet", "1"},
	{"rrm_neighbor_report", "1"},
	{"rrm_beacon_report", "1"},
	{"mbo", "1"},
	{"country3", "4"},
	/* content of neighbor report sub IE in GAS anqp RSP */
	/* info id=272(ANQP_NEIGHBOR_REPORT) , sub-elem id=52(0x34, neighbor report element) */
	/* the bssid of neighbor is set to broadcast mac, it will be changed to the bssid of sending interface */
	/* other fields(opc/ch/phytype) are copied from the frame which is belong to the other MBO passed dev */
	/* pls do NOT change the "3410" fields and the whole length of the frame, testbed will not check the other fields. */
	{"anqp_elem", "272:3410FFFFFFFFFFFF1806B3BA747324"},
};

int add_params(struct vap_config *params, int num_params, FILE *config_file)
{
	char new_param[64];
	int i;

	if (!params || !config_file)
		return -1;

	for (i = 0; i < num_params; i++) {
		memset(new_param, 0, 64);
		sprintf(new_param, "%s=%s\n", params[i].option_name, params[i].option_value);
		fputs(new_param, config_file);
	}
	return 0;
}

void fill_mbo_conf_in_runconf(int radio, char *mboconf_path)
{
	char runconf_path[64] = {0};
	char defconf_path[64] = {0};
	char cmd[128] = {0};
	char ifname[8] = {0};
	FILE *mboconf_file = NULL;

	sprintf(runconf_path, "/tmp/run/hostapd-phy%u.conf", radio);
	sprintf(ifname, "wlan%d", radio);

	if (radio == 1) /* MBO testplan will set radio-5G mode to 11ac */
		sprintf(defconf_path, "/etc/hostapd-phy%u_VHT_default.conf", radio);
	else if (radio == 2)
		sprintf(defconf_path, "/etc/hostapd-phy%u_HE_default.conf", radio);
	else  /* MBO testplan will set radio-2G mode to 11n */
		sprintf(defconf_path, "/etc/hostapd-phy%u_HT_default.conf", radio);

	sprintf(cmd, "cp -f %s %s", defconf_path, mboconf_path);
	system(cmd);

	mboconf_file = fopen(mboconf_path, "a+");
	if (mboconf_file < 0)
		return;
	add_params(mbo_params, (int)ARRAY_SIZE(mbo_params), mboconf_file);
	fflush(mboconf_file);
	fclose(mboconf_file);

	memset(cmd, 0, 128);
	sprintf(cmd, "cp -f %s %s", mboconf_path, runconf_path);
	system(cmd);
}

int disable_interface_dl_ofdma(char *ifname)
{
	char cmd[128];

	if (!ifname)
		return -1;
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "iw dev %s vendor send 0xD04433 0x01 0x07 0x03", ifname);
	system(cmd);
	return 0;
}

void reset_interface_config(char *ifname, char *type, char *program)
{
	char cmd[128], mboconf_path[64] = {0};
	int radio = 0;
	uint8_t mac[6];

	// to act according to type
	if (!ifname || !type || !program)
		return;

	if (-1 == local_get_radio_id(ifname, &radio))
		return;

	if (is_interface_up(ifname) <= 0)
		return;

	if (disable_interface_dl_ofdma(ifname) < 0) {
		printf("disable %s dl ofdma failed\n", ifname);
		return;
	}
	system("uci set system.@system[0].log_size=512");
	system("uci commit");
	system("/etc/init.d/log restart");

	memset(cmd, 0, sizeof(cmd));
	if (!strcasecmp(program, "HE")) {
		sprintf(cmd, "cp /etc/hostapd-phy%u_HE_default.conf %s/hostapd-phy%u.conf",
				radio, CLS_DEF_HOSTAD_DIR, radio);
		system(cmd);
	} else if (!strcasecmp(program, "VHT") || !strcasecmp(program, "WPA3")) {
		sprintf(cmd, "cp /etc/hostapd-phy%u_VHT_default.conf %s/hostapd-phy%u.conf",
				radio, CLS_DEF_HOSTAD_DIR, radio);
		system(cmd);
	} else if (!strcasecmp(program, "MBO")) {
		sprintf(mboconf_path, "/tmp/mbo-hostapd-phy%u.conf", radio);
		fill_mbo_conf_in_runconf(radio, mboconf_path);
	} else {
		sprintf(cmd, "cp /etc/hostapd-phy%u_HT_default.conf %s/hostapd-phy%u.conf",
				radio, CLS_DEF_HOSTAD_DIR, radio);
		system(cmd);
	}

	if (!strcasecmp(program, "HE") || !strcasecmp(program, "VHT") || !strcasecmp(program, "HT")
		|| !strcasecmp(program, "WPA3") || !strcasecmp(program, "FFD") || !strcasecmp(program, "MBO")) {
		update_parameter(ifname, NULL, cls_ssid_fmt_any,
					"country_code", "US", cls_mode_access_point, CLSAPI_TRUE,
					cls_bare_string, security_update_complete, 0, 0, NULL, NULL);
	}

	if (get_interface_mac(ifname, mac) > 0) {
		sprintf(cmd, "cls set %s bssid " MACSTR, ifname, MAC2STR(mac));
		system(cmd);
	}

	if (!strcasecmp(program, "MBO"))
		del_mboconf_file(mboconf_path);

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
				else if (!strcmp(option_name, "obss_interval"))
					sprintf(option_value, "%s", "0");
			} else if (!strcmp(width, "40")) {
				if (!strcmp(option_name, "ieee80211n")) {
					sprintf(option_value, "%s", "1");
				} else if (!strcmp(option_name, "ht_coex")) {
					sprintf(option_value, "%s", "1");
				} else if (!strcmp(option_name, "obss_interval")) {
					sprintf(option_value, "%s", "10");
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
				else if (!strcmp(option_name, "obss_interval"))
					sprintf(option_value, "%s", "0");
			} else if (!strcmp(width, "40")) {
				if (!strcmp(option_name, "ieee80211n")) {
					sprintf(option_value, "%s", "1");
				} else if (!strcmp(option_name, "ht_coex")) {
					sprintf(option_value, "%s", "1");
				} else if (!strcmp(option_name, "obss_interval")) {
					sprintf(option_value, "%s", "10");
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
					sprintf(option_value, "%s", default_vht_capab);
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
					sprintf(option_value, "%s", default_vht_capab);
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
					sprintf(option_value, "%s", default_vht_capab);
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
	int  wid = 0;
	int  chan = 0;

	wid = atoi(width);
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

int is_valid_channel(int radio, int chan)
{
	int ret = CLSAPI_FALSE;

	if (radio == CLS_AP_RADIO_24G) {
		if (chan >= 1 && chan <= 14)
			ret = CLSAPI_TRUE;
	} else if (radio == CLS_AP_RADIO_5G) {
		if (chan >= 36 && chan <= 64 && !((chan - 36) % 4))
			ret = CLSAPI_TRUE;
		else if (chan >= 100 && chan <= 144 && !((chan - 100) % 4))
			ret = CLSAPI_TRUE;
		else if (chan >= 149 && chan <= 177 && !((chan - 149) % 4))
			ret = CLSAPI_TRUE;
		else
			ret = CLSAPI_FALSE;
	} else {
		;
	}
	return ret;
}

int set_interface_channel(const char *ifname, char *channel)
{
	int  radio = -1;
	int chan = 0;

	if (!ifname || !channel)
		return -1;
	local_get_radio_id(ifname, &radio);
	if (radio < 0 || radio >= MAX_RADIO_NUM)
		return -1;

	chan = strtol(channel, NULL, 0);
	if (!is_valid_channel(radio, chan)) {
		printf("invalid channel for current radio\n");
		return -1;
	}
	update_interface_param(ifname, "channel", channel,
				cls_mode_access_point, 0);
	update_interface_param(ifname, "chanlist", channel,
				cls_mode_access_point, 0);

	return 0;
}

int set_interface_payloadcode(const char *ifname, char *code)
{
	if (!ifname || !code)
		return -1;

	if (!strcasecmp(code, "bcc")) {
		update_interface_param(ifname, "ht_capab", bcc_ht_capab,
				cls_mode_access_point, 0);
		update_interface_param(ifname, "vht_capab", bcc_vht_capab,
				cls_mode_access_point, 0);
	}
	return 0;
}

int set_interface_sgi20(const char *ifname, char *enabled)
{
	if (!ifname)
		return -1;
	if (!strcasecmp(enabled, "disable")) {
		update_interface_param(ifname, "ht_capab", sgi20_disable_ht_vacap,
				cls_mode_access_point, 0);
	}
	return 0;
}

int set_ht_options(const int radio, const char *ifname, char *channel, char *width)
{
	int i = 0;
	char opt_val[512];

	while (ht_option_list[i] != NULL) {
		memset(opt_val, 0, sizeof(opt_val));
		get_option_value(radio, "11n", width, channel,
			ht_option_list[i], opt_val);
		update_interface_param(ifname, ht_option_list[i],
			opt_val, cls_mode_access_point, 0);
		i++;
	}
	return 0;
}

int set_vht_options(const int radio, const char *ifname, char *channel, char *width)
{
	int i = 0;
	char opt_val[512];

	while (vht_option_list[i] != NULL) {
		memset(opt_val, 0, sizeof(opt_val));
		get_option_value(radio, "11ac", width, channel,
			vht_option_list[i], opt_val);
		update_interface_param(ifname, vht_option_list[i],
			opt_val, cls_mode_access_point, 0);
		i++;
	}
	return 0;
}

int set_he_options(const int radio, const char *ifname, char *channel, char *width)
{
	int i = 0;
	char opt_val[512];

	while (he_option_list[i] != NULL) {
		memset(opt_val, 0, sizeof(opt_val));
		get_option_value(radio, "11ax", width, channel,
			he_option_list[i], opt_val);
		update_interface_param(ifname, he_option_list[i],
			opt_val, cls_mode_access_point, 0);
		i++;
	}
	return 0;
}

int set_radio_options(const int radio, const char *ifname, char *channel,
							char *width, enum cls_ap_mode mode)
{
	if (!ifname || !channel || !width)
		return -1;
	int i = 0;
	char opt_val[512];

	if (radio == CLS_AP_RADIO_24G) {
		switch (mode) {
		case CLS_AP_MODE_11AX:
			if (!strcmp(width, "20") || !strcmp(width, "40")) {
				set_ht_options(radio, ifname, channel, width);
				set_he_options(radio, ifname, channel, width);
			}
		break;
		case CLS_AP_MODE_11N:
			if (!strcmp(width, "20") || !strcmp(width, "40"))
				set_ht_options(radio, ifname, channel, width);
		break;
		default:
			;
		}
	} else if (radio == CLS_AP_RADIO_5G) {
		switch (mode) {
		case CLS_AP_MODE_11AX:
			if (!strcmp(width, "20")) {
				//11n,20M
				set_ht_options(radio, ifname, channel, width);
				//11ac,20M
				set_vht_options(radio, ifname, channel, width);
				//11ax
				set_he_options(radio, ifname, channel, width);
			} else if (!strcmp(width, "40") || !strcmp(width, "80") ||
				!strcmp(width, "160")) {
				//11n,40M
				set_ht_options(radio, ifname, channel, "40");
				//11ac,width
				set_vht_options(radio, ifname, channel, width);
				//11ax
				set_he_options(radio, ifname, channel, width);
			}
		break;
		case CLS_AP_MODE_11AC:
			if (!strcmp(width, "20")) {
				//11n,20M
				set_ht_options(radio, ifname, channel, width);
				//11ac,20M
				set_vht_options(radio, ifname, channel, width);

			} else if (!strcmp(width, "40") || !strcmp(width, "80") ||
						!strcmp(width, "160")) {
				//11n,40M
				set_ht_options(radio, ifname, channel, "40");
				//11ac,width
				set_vht_options(radio, ifname, channel, width);
			} else {
				;
			}
		break;
		case CLS_AP_MODE_11N:
			set_ht_options(radio, ifname, channel, width);
		break;
		default:
			;
		}
	} else {
		;
	}

	return 0;
}

int is_valid_bw(int radio, int wd, enum cls_ap_mode mode)
{
	int ret = CLSAPI_TRUE;

	if (radio == CLS_AP_RADIO_24G) {
		if (mode == CLS_AP_MODE_11N) {
			if (wd != 20 && wd != 40)
				ret = CLSAPI_FALSE;
		} else if (mode == CLS_AP_MODE_11AX) {
			if (wd != 20 && wd != 40)
				ret = CLSAPI_FALSE;
		} else {
			ret = CLSAPI_FALSE;
		}
	} else if (radio == CLS_AP_RADIO_5G) {
		if (mode == CLS_AP_MODE_11N) {
			if (wd != 20 && wd != 40)
				ret = CLSAPI_FALSE;
		} else if (mode == CLS_AP_MODE_11AX || mode == CLS_AP_MODE_11AC) {
			if (wd != 20 && wd != 40 && wd != 80 && wd != 160)
				ret  = CLSAPI_FALSE;
		} else {
			ret = CLSAPI_FALSE;
		}
	} else {
		ret = CLSAPI_FALSE;
	}

	return ret;
}

int set_interface_sideband(const char *ifname, char *sideband)
{
	enum cls_ap_mode ap_mode;
	char ht_enable[8];
	int radio = -1;
	int i = 0;
	char ht_capb[128] = {0};

	if (!ifname || !sideband)
		return -1;

	local_get_radio_id(ifname, &radio);
	if (radio == -1) {
		printf("%s: get band error ", __func__);
		return -1;
	}

	memset(ht_enable, 0, sizeof(ht_enable));
	found_ap_radio_param_value(ifname, "ieee80211n", ht_enable);

	if (strcmp(ht_enable, "0") == 0) {
		printf("legacy mode, has NO sideband\n");
		return -1;
	}

	if (strcmp(sideband, "above") == 0)
		sprintf(ht_capb, "%s", "[HT40+]");
	else
		sprintf(ht_capb, "%s", "[HT40-]");
	strcat(ht_capb, default_ht_capab);
	update_parameter(ifname, NULL, cls_ssid_fmt_any,
				"ht_capab", ht_capb, cls_mode_access_point, CLSAPI_TRUE,
				cls_bare_string, security_update_complete, 0, 0, NULL, NULL);

	return 0;
}

int set_interface_bandwidth(const char *ifname, char *width)
{
	enum cls_ap_mode ap_mode;
	char he_enable[8];
	char vht_enable[8];
	char ht_enable[8];
	char channel[8];
	int radio = -1;
	char opt_val[512];
	int i = 0;
	char *mode[CLS_AP_MODE_11AX + 1] = {"11a", "11b", "11g", "11n", "11ac", "11ax"};
	int wd = 0;

	if (!ifname || !width)
		return -1;

	local_get_radio_id(ifname, &radio);
	if (radio == -1) {
		printf("%s: get band error ", __func__);
		return -1;
	}
	wd = strtol(width, NULL, 10);
	if (wd == 0)
		return -1;

	memset(he_enable, 0, sizeof(he_enable));
	memset(vht_enable, 0, sizeof(he_enable));
	memset(ht_enable, 0, sizeof(ht_enable));
	found_ap_radio_param_value(ifname, "ieee80211ax", he_enable);
	found_ap_radio_param_value(ifname, "ieee80211ac", vht_enable);
	found_ap_radio_param_value(ifname, "ieee80211n", ht_enable);

	if (!strcmp(he_enable, "1")) {
		ap_mode = CLS_AP_MODE_11AX;
	} else if (!strcmp(vht_enable, "1")) {
		ap_mode = CLS_AP_MODE_11AC;
	} else if (!strcmp(ht_enable, "1")) {
		ap_mode = CLS_AP_MODE_11N;
	} else {
		if (radio == 0)
			ap_mode = CLS_AP_MODE_G;
		else if (radio == 1)
			ap_mode = CLS_AP_MODE_A;
	}

	if (!is_valid_bw(radio, wd, ap_mode)) {
		printf(" invalid bw for current radio and mode\n");
		return -1;
	}
	memset(channel, 0, sizeof(channel));
	found_ap_radio_param_value(ifname, "channel", channel);
	printf("---%s : get channel[%s], mode[%s]\n", __func__, channel, mode[ap_mode]);

	return set_radio_options(radio, ifname, channel, width, ap_mode);
}

int set_interface_mode_width_channel(char *ifname, char *mode, char *orig_width, char *orig_channel)
{
	int radio = -1;
	int chan = 0;
	int i = 0;
	char opt_val[512];
	char width[8];
	char channel[8];


	local_get_radio_id(ifname, &radio);
	if (radio == -1) {
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
		if (radio == CLS_AP_RADIO_24G) {
			if (!strcmp(mode, "11b") || !strcmp(mode, "11g")) {
				sprintf(width, "%s", "20");
				sprintf(channel, "%s", "6");
			} else if (!strcmp(mode, "11n")) {
				sprintf(width, "%s", "20");
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
		if (radio == CLS_AP_RADIO_24G)
			sprintf(channel, "%s", "6");
		else
			sprintf(channel, "%s", "36");
	} else if (!orig_width) {
		if (radio == CLS_AP_RADIO_24G) {
			if (!strcmp(mode, "11b") || !strcmp(mode, "11g"))
				sprintf(width, "%s", "20");
			else if (!strcmp(mode, "11n"))
				sprintf(width, "%s", "20");
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
	case CLS_AP_RADIO_24G: //2.4G
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
		} else {
			;
		}
		break;
	case CLS_AP_RADIO_5G://5G
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
		} else {
			;
		}
		break;
	default:
		;
	}
	return 0;
}

int get_primary_vap_name(int radio, char *name, int len)
{
	snprintf(name, len, "wlan%d", radio);
	return 0;
}

int check_ap_config(char *ifname, SSID_parsing_state *parse_state, char *param, char *configline)
{
	parse_ap_config_with_sub_parameter(ifname, parse_state, "bss",
			configline, ifname, NULL, NULL);
	return 0;
}

int check_radio_param_config(char *param, char *configline)
{
	if (!strncasecmp(configline, param, strlen(param)))
		return CLSAPI_TRUE;
	return CLSAPI_FALSE;
}

int found_radio_param_value(int radio, char *param_name, char *param_value, int *found)
{
	int retval = 0;
	char config_file[CLSAPI_SEC_FILE_LEN_MAX];
	char config_line[1024];
	const int filename_len = sizeof(config_file);
	FILE *config_fh = NULL;
	const char *config_addr = config_line;

	retval = get_config_file_path(radio, cls_mode_access_point, config_file, filename_len, "");

	if (retval >= 0)
		config_fh = fopen(config_file, "r");

	if (config_fh == NULL) {
		retval = -1;
		return retval;
	} else {
		while (fgets(config_line, sizeof(config_line), config_fh) != NULL) {
			*found = check_radio_param_config(param_name, config_line);
			if (*found == CLSAPI_TRUE) {
				config_addr = locate_parameter_line(param_name, config_line);
				get_local_configured_value(config_addr, param_name,
					param_value);
				break;
			}
		}
		*found = CLSAPI_FALSE;
	}

	if (config_fh != NULL)
		fclose(config_fh);

	if (*found == CLSAPI_FALSE)
		retval = -1;

	return retval;
}

int found_ap_radio_param_value(const char *ifname, char *para_name, char *value)
{
	int found = 0;
	int radio_id = -1;
	int ret = 0;

	local_get_radio_id(ifname, &radio_id);
	if (radio_id < 0 || radio_id > 2) {
		printf("can't get radio\n");
		ret = -1;
		return ret;
	}
	ret = found_radio_param_value(radio_id, para_name, value, &found);
	return ret;

}

int found_bss_ifname(int radio, char *ifname, int *found)
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
	}

	while (fgets(config_line, sizeof(config_line), config_fh) != NULL) {
		check_ap_config(ifname, &e_parse_state, "bss", config_line);
		if (e_parse_state == e_found_current_network) {
			*found = CLSAPI_TRUE;
			break;
		}
	}
	if (e_parse_state != e_found_current_network)
		*found = CLSAPI_FALSE;

	if (config_fh != NULL)
		fclose(config_fh);

	return retval;
}

int add_default_bss_config(int radio,char *ifname, char *mac)
{
	int retval = 0;
	FILE *config_fh = NULL;
	FILE *temp_fh = NULL;
	int ival = 0;
	char *config_buffer;
	struct vap_config *pconfig = def_vap_config;
	int update = 0;

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
		while (pconfig->option_name != NULL)
		{
			fprintf(temp_fh, "%s=%s\n", pconfig->option_name, pconfig->option_value);
			pconfig++;
		}
		fprintf(temp_fh, "bssid=%s\n", mac);
		update = 1;
	}

	close_file(radio, &config_fh, &temp_fh, cls_mode_access_point, 1);

	return 0;
}

int get_interface_idx(char *ifname, int *index)
{
	char *p = NULL;

	p = strrchr(ifname, '_');
	if (!p) {
		if (*(ifname + 4) != 0)
			*index = *(ifname + 4) - '0';
		return 0;
	}
	*index = *(p + 1) - '0';
	return 0;
}

int get_interface_mac(char *ifname, uint8_t *mac)
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
	memcpy(mac, (unsigned char *)ifr.ifr_hwaddr.sa_data, ETH_MACADDR_LEN);
	return 1;
}

int get_not_primary_interface_mac(char *primaryvap, char *vapname, char *mac_str)
{
	char mac[ETH_MACADDR_LEN];
	int idex = 0;

	if (get_interface_mac(primaryvap, mac) < 0)
		return 0;

	get_interface_idx(vapname, &idex);
	if (idex == 0)
		return 0;

	mac[5] += idex & 0xff;
	sprintf(mac_str, MACSTR, MAC2STR(mac));

	return 1;
}

int create_new_bss_config(char *ifname, char *mac, int phy_id)
{
	int radio_id;
	char primary_name[32];
	int found = 0;
	char mac_str[18];

	memset(mac_str, 0, sizeof(mac_str));
	memset(primary_name, 0, sizeof(primary_name));
	if (phy_id == -1) {
		local_get_radio_id(ifname, &radio_id);
	} else {
		if (phy_id >= 0 && phy_id <= 2)
			radio_id = phy_id;
	}
	get_primary_vap_name(radio_id, primary_name, sizeof(primary_name) - 1);

	// to find bss which name is ifname
	found_bss_ifname(radio_id, ifname, &found);

	if (found) {
		printf("the bss exists!!\n");
		return 0;
	}

	if (mac == NULL) {
		get_not_primary_interface_mac(primary_name, ifname, mac_str);
		printf(">>> mac == NULL, primary vap[%s], mac_str[%s]\n",
			primary_name, mac_str);
		add_default_bss_config(radio_id, ifname, mac_str);
	} else {
		add_default_bss_config(radio_id, ifname, mac);
	}

	return 0;
}

int main(int argc, char **argv)
{
	char cmd[128];
	int i = 0;

	memset(cmd, 0, sizeof(cmd));
	if (argc < 2) {
		printf("Usage: cls set|remove ifname paramname paramvalue\n"
				"          bw, mode, channel and payload_code are not included\n");
		printf("       cls set ifname mode [11ax|11ac|11n] [channel xx] [width xx]\n");
		printf("       cls set ifname channel xx\n");
		printf("       cls set ifname bw xx\n");
		printf("       cls set ifname payload_code bcc\n");
		printf("       cls set ifname sgi20 disable\n");
		printf("       cls reconfig ifname\n");
		printf("       cls get ifname paramname\n");
		printf("       cls create_bss new_ifname phyid [mac xx:xx:xx:xx:xx:xx]\n");
		return 0;
	}
#ifdef CLS_DEBUG
	for (i = 1; i < argc; i++)
		printf("argv[%d]=%s\n", i, argv[i]);
#endif
	if (!strcasecmp(argv[1], "set")) {
		if (argc < 5) {
			printf("not enough parameters\n");
			return 0;
		}
		if ((argc == 5) && !is_special_parameter(argv[3])) {
			if (is_param_with_sub(argv[3])) {
				// remove the multiparam
				remove_parameter_with_sub(argv[2], argv[3]);
				// to set every single param
				update_parameter(argv[2], NULL, cls_ssid_fmt_any,
				argv[3], argv[4], cls_mode_access_point, CLSAPI_TRUE,
				cls_bare_string, security_update_complete, 0, 1, NULL, NULL);
			} else {
				update_parameter(argv[2], NULL, cls_ssid_fmt_any,
				argv[3], argv[4], cls_mode_access_point, CLSAPI_TRUE,
				cls_bare_string, security_update_complete, 0, 0, NULL, NULL);
			}
		} else if ((argc == 5) && !strcasecmp(argv[3], "bw")) {
			set_interface_bandwidth(argv[2], argv[4]);
		} else if ((argc == 5) && !strcasecmp(argv[3], "channel")) {
			set_interface_channel(argv[2], argv[4]);
		} else if ((argc == 5) && !strcasecmp(argv[3], "payload_code")) {
			set_interface_payloadcode(argv[2], argv[4]);
		} else if ((argc == 5) && !strcasecmp(argv[3], "sgi20")) {
			set_interface_sgi20(argv[2], argv[4]);
		} else if ((argc == 5) && !strcasecmp(argv[3], "sideband")) {
			set_interface_sideband(argv[2], argv[4]);
		} else if (!strcasecmp(argv[3], "mode")) {
			if (argc == 9) {
				if (!strcmp(argv[3], "mode") && !strcmp(argv[5], "width")
					&& !strcmp(argv[7], "channel"))
					set_interface_mode_width_channel(argv[2],
							argv[4], argv[6], argv[8]);
				if (!strcmp(argv[3], "mode") && !strcmp(argv[5], "channel")
					&& !strcmp(argv[7], "width"))
					set_interface_mode_width_channel(argv[2],
							argv[4], argv[8], argv[6]);
			} else if ((argc == 5)) {
				set_interface_mode_width_channel(argv[2], argv[4], NULL, NULL);
			} else if (argc == 7) {
				if (!strcasecmp(argv[5], "channel"))
					set_interface_mode_width_channel(argv[2], argv[4], NULL, argv[6]);
				else if (!strcasecmp(argv[5], "width"))
					set_interface_mode_width_channel(argv[2], argv[4], argv[6], NULL);
			}
		}
	} else if (!strcasecmp(argv[1], "remove")) {
		if (argc >= 4) {
			// to get the value of param
			if (is_param_with_sub(argv[3]))
				remove_parameter_with_sub(argv[2], argv[3]);
			else
				update_interface_param(argv[2], argv[3], NULL, cls_mode_access_point, 1);
		}
	} else if (!strcasecmp(argv[1], "reconfig")) {
		//to do
		if (argc >= 3)
			reconfig_interface_config(argv[2]);
	} else if (!strcasecmp(argv[1], "reset")) {
		reset_interface_config(argv[2], argv[4], argv[6]);
	} else if (!strcasecmp(argv[1], "create_bss")) {
		if (argc == 4)
			create_new_bss_config(argv[2], NULL, atoi(argv[3]));
		else if (argc == 6 && !strcmp(argv[4], "mac"))
			create_new_bss_config(argv[2], argv[5], atoi(argv[3]));
	} else if (!strcasecmp(argv[1], "get")) {
		char value[1024];

		memset(value, 0, sizeof(value));
		found_ap_radio_param_value(argv[2], argv[3], value);
		printf("ifname[%s]:[%s]----[%s]\n", argv[2], argv[3], value);
	}
	return 0;
}

