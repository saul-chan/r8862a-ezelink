/*
 * Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */

#include "clsapi_base.h"
#include "clsapi_sys.h"
#include "sys/sysinfo.h"
#include "sys/statvfs.h"
#include "sys/utsname.h"

#include <sys/reboot.h>

static const char *cls_support_lang_tbl[] = {
	"auto",				// Switch the corresponding language based on the browser's language environment.
	"en",				// English
	"zh_cn"				// Chinese
};

static const char *cls_ssh_interface_tbl[] = {
	"all",
	"lan",
	"wan",
	"wan6"
};

/*****************************	Functions declarations	*************************/

/* \brief Calculate storage size.
 * \param block [in] filesystem block size.
 * \param fragment [in] fragment size.
 * \return the calculated result.
 */
static unsigned long calculate_storage_size(unsigned long block, unsigned long fragment)
{
	return (block * (unsigned long long) fragment + 1024/2) / 1024;
}

/* \brief Get system rootfs type.
 * \return rootfs type if it was found, otherwise, return NULL.
 */
static const char *get_system_rootfs_type(void)
{
	const char proc_mounts[] = "/proc/self/mounts";
	static char fstype[16] = { 0 };
	char *mp = "/", *pos, *tmp;
	string_128 mountstr = {0};
	FILE *mounts;
	bool found;

	if (fstype[0])
		return fstype;

	mounts = fopen(proc_mounts, "r");
	if (!mounts)
		return NULL;

	while ((fgets(mountstr, sizeof(mountstr), mounts)) != NULL) {
		found = false;

		pos = strchr(mountstr, ' ');
		if (!pos)
			continue;

		tmp = pos + 1;
		pos = strchr(tmp, ' ');
		if (!pos)
			continue;

		*pos = '\0';
		if (strcmp(tmp, mp))
			continue;

		tmp = pos + 1;
		pos = strchr(tmp, ' ');
		if (!pos)
			continue;

		*pos = '\0';

		if (!strcmp(tmp, "overlay")) {
			/*
			 * there is no point in parsing overlay option string for
			 * lowerdir, as that can point to "/" being a previous
			 * overlay mount (after firstboot or sysuprade config
			 * restore). Hence just assume the lowerdir is "/rom" and
			 * restart searching for that instead.
			 */
			mp = "/rom";
			fseek(mounts, 0, SEEK_SET);
			continue;
		}

		found = true;
		break;
	}

	if (found)
		strncpy(fstype, tmp, sizeof(fstype) - 1);

	fstype[sizeof(fstype) - 1]= '\0';
	fclose(mounts);

	if (found)
		return fstype;
	else
		return NULL;
}

int clsapi_sys_get_led(const uint8_t led_gpio, bool *onoff)
{
	string_256 path;
	char *line_value = NULL;
	int int_onoff = -1;

	/* GPIO validation, TODO */

	if (!onoff)
		return -CLSAPI_ERR_NULL_POINTER;

	/* Check if led is enabled */
	snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", led_gpio);
	line_value = get_one_line_from_file(path);
	if (!line_value || strcmp(line_value, "out"))
		return -CLSAPI_ERR_NOT_SUPPORTED;

	/* get on/off status */
	snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", led_gpio);
	line_value = get_one_line_from_file(path);
	if (!line_value)
		return -CLSAPI_ERR_NOT_SUPPORTED;

	int_onoff = atoi(line_value);
	*onoff = (bool)int_onoff;

	return CLSAPI_OK;
}

int clsapi_sys_set_led(const uint8_t led_gpio, const bool onoff)
{
	string_256 cmd;

	/* GPIO validation, TODO */

	/* enable GPIO of led */
	snprintf(cmd, sizeof(cmd), "echo %d > /sys/class/gpio/export 2>/dev/null", led_gpio);
	if( system(cmd) == -1)
		return -CLSAPI_ERR_INVALID_PARAM;
	snprintf(cmd, sizeof(cmd), "echo out > /sys/class/gpio/gpio%d/direction 2>/dev/null", led_gpio);
	if( system(cmd) == -1)
		return -CLSAPI_ERR_INTERNAL_ERR;

	/* turn on/off this led */
	snprintf(cmd, sizeof(cmd), "echo %d > /sys/class/gpio/gpio%d/value 2>/dev/null", onoff, led_gpio);
	if( system(cmd) == -1)
		return -CLSAPI_ERR_INTERNAL_ERR;

	return CLSAPI_OK;
}

int clsapi_sys_get_hostname(string_1024 hostname)
{
	int ret = CLSAPI_OK;

	if (!hostname)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_get_param(CLSCONF_CFG_SYSTEM, "@system[0]", "hostname", hostname);

	return ret;
}

int clsapi_sys_set_hostname(const char *hostname)
{
	int ret = CLSAPI_OK;

	if (!hostname)
		return -CLSAPI_ERR_INVALID_PARAM;

	clsconf_defer_apply_param(CLSCONF_CFG_SYSTEM, "@system[0]", "hostname", hostname);

	return ret;
}

int clsapi_sys_restore_factory(void)
{
	int ret = system("/sbin/firstboot -r -y");

	if (ret == -1)
		return -CLSAPI_ERR_SYSTEM_ERROR;

	return CLSAPI_OK;
}

int clsapi_sys_get_lang(string_1024 lang)
{
	int ret = CLSAPI_OK;

	if (!lang)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = clsconf_get_param(CLSCONF_CFG_LUCI, "main", "lang", lang);

	return ret;
}

int clsapi_sys_set_lang(const char *lang)
{
	int ret = CLSAPI_OK;

	for (int i = 0; i < ARRAY_SIZE(cls_support_lang_tbl); i++) {
		if (strcmp(lang, cls_support_lang_tbl[i]) == 0) {
			clsconf_defer_apply_param(CLSCONF_CFG_LUCI, "main", "lang", lang);
			return ret;
		}
	}

	return -CLSAPI_ERR_INVALID_PARAM;
}

int clsapi_sys_get_ssh_iface(const uint8_t idx, string_32 interface)
{
	int ret = CLSAPI_OK;
	string_32 section_id;
	string_1024 str_value;

	if (idx != 0)
		return -CLSAPI_ERR_NOT_SUPPORTED;

	if (!interface)
		return -CLSAPI_ERR_INVALID_PARAM;

	snprintf(section_id, sizeof(string_32), "@%s[%d]", CLSCONF_CFG_DROPBEAR, idx);

	ret = clsconf_get_param(CLSCONF_CFG_DROPBEAR, section_id, "Interface", str_value);
	if (ret == -CLSAPI_ERR_NOT_FOUND) {
		cls_strncpy(interface, "all", sizeof(string_32));
		return CLSAPI_OK;
	}

	cls_strncpy(interface, str_value, sizeof(string_32));

	return ret;
}

int clsapi_sys_set_ssh_iface(const uint8_t idx, const char *interface)
{
	int ret = CLSAPI_OK;
	string_32 section_id;

	if (idx != 0)
		return -CLSAPI_ERR_NOT_SUPPORTED;

	if (!interface)
		return -CLSAPI_ERR_INVALID_PARAM;

	snprintf(section_id, sizeof(string_32), "@%s[%d]", CLSCONF_CFG_DROPBEAR, idx);

	for (int i = 0; i < ARRAY_SIZE(cls_ssh_interface_tbl); i++) {
		if (strcmp(interface, cls_ssh_interface_tbl[i]) == 0) {
			if (strcmp(interface, "all") != 0)
				clsconf_defer_apply_param(CLSCONF_CFG_DROPBEAR, section_id, "Interface", interface);
			else
				clsconf_defer_apply_param(CLSCONF_CFG_DROPBEAR, section_id, "Interface", "");

			return ret;
		}
	}

	return -CLSAPI_ERR_INVALID_PARAM;
}

int clsapi_sys_get_ssh_port(const uint8_t idx, uint16_t *port)
{
	int ret = CLSAPI_OK;
	string_32 section_id;

	if (idx != 0)
		return -CLSAPI_ERR_NOT_SUPPORTED;

	if (!port)
		return -CLSAPI_ERR_INVALID_PARAM;

	snprintf(section_id, sizeof(string_32), "@%s[%d]", CLSCONF_CFG_DROPBEAR, idx);
	clsconf_get_int_param(CLSCONF_CFG_DROPBEAR, section_id, "Port", *port);

	return ret;
}

int clsapi_sys_set_ssh_port(const uint8_t idx, const uint16_t port)
{
	int ret = CLSAPI_OK;
	string_32 section_id;

	if (idx != 0)
		return -CLSAPI_ERR_NOT_SUPPORTED;

	snprintf(section_id, sizeof(string_32), "@%s[%d]", CLSCONF_CFG_DROPBEAR, idx);
	clsconf_defer_apply_uint_param(CLSCONF_CFG_DROPBEAR, section_id, "Port", port);

	return ret;
}

int clsapi_sys_get_ssh_password_auth(const uint8_t idx, bool *onoff)
{
	int ret = CLSAPI_OK;
	string_32 section_id;
	string_1024 status;

	if (idx != 0)
		return -CLSAPI_ERR_NOT_SUPPORTED;

	if (!onoff)
		return -CLSAPI_ERR_INVALID_PARAM;

	snprintf(section_id, sizeof(section_id), "@%s[%d]", CLSCONF_CFG_DROPBEAR, idx);

	clsconf_get_param(CLSCONF_CFG_DROPBEAR, section_id, "PasswordAuth", status);

	if (strcmp(status, "1") == 0 || strcmp(status, "on") == 0)
		*onoff = 1;
	else if (strcmp(status, "0") == 0 || strcmp(status, "off") == 0)
		*onoff = 0;

	return ret;
}

int clsapi_sys_set_ssh_password_auth(const uint8_t idx, const bool onoff)
{
	int ret = CLSAPI_OK;
	string_32 section_id;

	if (idx != 0)
		return -CLSAPI_ERR_NOT_SUPPORTED;

	snprintf(section_id, sizeof(section_id), "@%s[%d]", CLSCONF_CFG_DROPBEAR, idx);

	clsconf_defer_apply_param(CLSCONF_CFG_DROPBEAR, section_id, "PasswordAuth", onoff ? "on" : "off");

	return ret;
}

int clsapi_sys_get_ssh_root_login(const uint8_t idx, bool *onoff)
{
    int ret = CLSAPI_OK;
	string_32 section_id;
	string_1024 status;

	if (idx != 0)
		return -CLSAPI_ERR_NOT_SUPPORTED;

	if (!onoff)
		return -CLSAPI_ERR_INVALID_PARAM;

	snprintf(section_id, sizeof(section_id), "@%s[%d]", CLSCONF_CFG_DROPBEAR, idx);

	ret = clsconf_get_param(CLSCONF_CFG_DROPBEAR, section_id, "RootPasswordAuth", status);

	if (strcmp(status, "1") == 0 || strcmp(status, "on") == 0)
		*onoff = 1;
	else if (strcmp(status, "0") == 0 || strcmp(status, "off") == 0)
		*onoff = 0;

	return ret;
}

int clsapi_sys_set_ssh_root_login(const uint8_t idx, const bool onoff)
{
	int ret = CLSAPI_OK;
	string_32 section_id;

	if (idx != 0)
		return -CLSAPI_ERR_NOT_SUPPORTED;

	snprintf(section_id, sizeof(section_id), "@%s[%d]", CLSCONF_CFG_DROPBEAR, idx);

	clsconf_defer_apply_param(CLSCONF_CFG_DROPBEAR, section_id, "RootPasswordAuth", onoff ? "on" : "off");

	return ret;
}

int clsapi_sys_get_sys_uptime(uint64_t *uptime)
{
	struct sysinfo info;
	int ret = CLSAPI_OK;

	if (!uptime)
		return -CLSAPI_ERR_INVALID_PARAM;

	if (sysinfo(&info))
		return -CLSAPI_ERR_UNKNOWN_ERROR;

	*uptime = info.uptime;

	return ret;
}

int clsapi_sys_get_meminfo(struct clsapi_meminfo *meminfo)
{
	int ret = CLSAPI_OK;
	struct sysinfo info;
	unsigned long long available = 0;
	unsigned long long cached = 0;
	FILE *file;
	char line[256];
	char *key, *val;

	if (!meminfo)
		return -CLSAPI_ERR_INVALID_PARAM;

	if ((file = fopen("/proc/meminfo", "r")) == NULL)
		return -CLSAPI_ERR_UNKNOWN_ERROR;

	while (fgets(line, sizeof(line), file)) {
		key = strtok(line, " :");
		val = strtok(NULL, " ");

		if (!key || !val)
			continue;

		if (!strcasecmp(key, "MemAvailable"))
			available = atoll(val);
		else if (!strcasecmp(key, "Cached"))
			cached = atoll(val);
	}

	fclose(file);

	if (sysinfo(&info))
		return -CLSAPI_ERR_UNKNOWN_ERROR;

	meminfo->total = info.totalram / 1024;
	meminfo->free = info.freeram / 1024;
	meminfo->shared = info.sharedram / 1024;
	meminfo->buffered = info.bufferram / 1024;
	meminfo->available = available;
	meminfo->cached = cached;

	return ret;
}

int clsapi_sys_get_storage(struct clsapi_storage_info *storage_info)
{
	int ret = CLSAPI_OK;
	struct statvfs s;

	if (!storage_info)
		return -CLSAPI_ERR_INVALID_PARAM;

	memset(storage_info, 0, sizeof(struct clsapi_storage_info));

	if (statvfs("/", &s))
		return -CLSAPI_ERR_UNKNOWN_ERROR;

	storage_info->total = calculate_storage_size(s.f_blocks, s.f_frsize);
	storage_info->free = calculate_storage_size(s.f_bfree, s.f_frsize);
	storage_info->used = calculate_storage_size(s.f_blocks - s.f_bfree, s.f_frsize);
	storage_info->avail = calculate_storage_size(s.f_bavail, s.f_frsize);

	return ret;
}

int clsapi_sys_get_board_info(struct clsapi_board_info *board_info)
{
	int ret = CLSAPI_OK;
	char line[256];
	char *val;
	char *key;
	int count = 0;
	FILE *file;
	char tmp[16][64];
	struct utsname utsname;
	const char *rootfs_type = get_system_rootfs_type();

	if (!board_info)
		return -CLSAPI_ERR_INVALID_PARAM;

	/* Get kernel version and hostname */
	if (uname(&utsname) < 0)
		return -CLSAPI_ERR_SYSTEM_ERROR;

	memset(board_info, 0, sizeof(struct clsapi_board_info));

	cls_strncpy(board_info->kernel, utsname.release, sizeof(board_info->kernel));
	cls_strncpy(board_info->hostname, utsname.nodename, sizeof(board_info->hostname));

	/* Get system architecture */
	if ((file = fopen("/proc/cpuinfo", "r")) == NULL) {
		DBG_ERROR("Failed to open file\n");
		return -CLSAPI_ERR_FILE_OPERATION;
	}

	while(fgets(line, sizeof(line), file)) {
		key = strtok(line, "\t:");
		val = strtok(NULL, "\t\n");

		if (!key || !val)
			continue;

		if (!strcasecmp(key, "CPU revision")) {
			snprintf(line, sizeof(line), "ARMv8 Processor rev %lu", strtoul(val + 2, NULL, 16));
			cls_strncpy(board_info->system, line, sizeof(board_info->system));
			break;
		}
	}

	fclose(file);

	/* Get system model */
	if ((file = fopen("/tmp/sysinfo/model", "r")) == NULL ||
	    (file = fopen("/proc/device-tree/model", "r")) == NULL) {
		DBG_ERROR("Failed to open file\n");
		return -CLSAPI_ERR_FILE_OPERATION;
	}

	if (fgets(line, sizeof(line), file)) {
		val = strtok(line, "\t\n");

		if (val)
			cls_strncpy(board_info->model, val, sizeof(board_info->model));
	}

	fclose(file);

	/* Get board name */
	if ((file = fopen("/tmp/sysinfo/board_name", "r")) == NULL) {

	}

	if (fgets(line, sizeof(line), file)) {
		val = strtok(line, "\t\n");

		if (val)
			cls_strncpy(board_info->board_name, val, sizeof(board_info->board_name));
	}

	fclose(file);

	/* Get rootfs type */
	if (!rootfs_type) {
		DBG_ERROR("Cannot found rootfs type!\n");
		return -CLSAPI_ERR_NOT_FOUND;
	}

	cls_strncpy(board_info->rootfs_type, rootfs_type, sizeof(board_info->rootfs_type));

	/* Get information of cls-openwrt release */
	if ((file = fopen("/etc/openwrt_release", "r")) == NULL) {
		DBG_ERROR("Failed to open file\n");
		return -CLSAPI_ERR_FILE_OPERATION;
	}

	while (fgets(line, sizeof(line), file)) {
		val = strchr(line, '=');
		if (!val)
			continue;

		val+=2;

		size_t len = strlen(val);
		if (len > 0 && val[len - 1] == '\n')
			val[len - 2] = '\0';

		cls_strncpy(tmp[count], val, sizeof(tmp[count]));
		count++;
	}

	cls_strncpy(board_info->release_distribution, tmp[0], sizeof(board_info->release_distribution));
	cls_strncpy(board_info->release_version, tmp[1], sizeof(board_info->release_version));
	cls_strncpy(board_info->release_revision, tmp[2], sizeof(board_info->release_revision));
	cls_strncpy(board_info->release_target, tmp[3], sizeof(board_info->release_target));
	cls_strncpy(board_info->release_description, tmp[5], sizeof(board_info->release_description));

	fclose(file);

	return ret;
}

int clsapi_sys_add_backup_file(const char *filepath)
{
	int ret = CLSAPI_OK;
	FILE *file;

	if (!filepath)
		return -CLSAPI_ERR_INVALID_PARAM;

	if (strlen(filepath) >= 256) {
		DBG_ERROR("filepath is too long!\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	file = fopen("/etc/sysupgrade.conf", "a");
	if (file == NULL) {
		DBG_ERROR("Failed to open file\n");
		return -CLSAPI_ERR_FILE_OPERATION;
	}

	fprintf(file, "%s\n", filepath);

	fclose(file);

	return ret;
}

int clsapi_sys_del_backup_file(const char *filepath)
{
	int ret = CLSAPI_OK;
	FILE *file, *tmpfile;
	char line[256], tmpfilepath[256];

	if (!filepath)
		return -CLSAPI_ERR_INVALID_PARAM;

	file = fopen("/etc/sysupgrade.conf", "r+");
	if (file == NULL) {
		DBG_ERROR("Failed to open file\n");
		return -CLSAPI_ERR_FILE_OPERATION;
	}

	tmpfile = fopen("/tmp/sysupgrade.conf", "w+");
	if (tmpfile == NULL) {
		fclose(file);
		DBG_ERROR("Failed to open file\n");
		return -CLSAPI_ERR_FILE_OPERATION;
	}

	ret = snprintf(tmpfilepath, sizeof(tmpfilepath), "%s\n", filepath);
	if (ret >= sizeof(tmpfilepath)) {
		fclose(file);
		fclose(tmpfile);
		DBG_ERROR("filepath is too long!\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	while (fgets(line, sizeof(line), file) != NULL) {
		if (strcmp(line, tmpfilepath) == 0)
			continue;

		fputs(line, tmpfile);
	}

	fclose(file);
	fclose(tmpfile);

	ret = system("mv /tmp/sysupgrade.conf /etc/sysupgrade.conf");
	if (!WIFEXITED(ret) || WEXITSTATUS(ret) != 0)
		return -CLSAPI_ERR_SYSTEM_ERROR;

	return ret;
}

int clsapi_sys_backup_conf(const char *filepath)
{
	int ret = CLSAPI_OK;
	char cmd[256];

	if (!filepath)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = snprintf(cmd, sizeof(cmd), "/sbin/sysupgrade --create-backup %s", filepath);
	if (ret >= sizeof(cmd)) {
		DBG_ERROR("filepath is too long!\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	ret = system(cmd);
	if (!WIFEXITED(ret) || WEXITSTATUS(ret) != 0)
		return -CLSAPI_ERR_SYSTEM_ERROR;

	return ret;
}

int clsapi_sys_restore_backup_conf(const char *filepath)
{
	int ret = CLSAPI_OK;
	char cmd[256];
	pid_t pid;

	if (!filepath)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = snprintf(cmd, sizeof(cmd), "/sbin/sysupgrade --restore-backup %s", filepath);
	if (ret >= sizeof(cmd)) {
		DBG_ERROR("filepath is too long!\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	ret = system(cmd);

	if (!WIFEXITED(ret) || WEXITSTATUS(ret) != 0)
		return -CLSAPI_ERR_SYSTEM_ERROR;

	/* Non blocking waiting for child processes to perform system reboot. */
	pid = fork();

	if (pid < 0) {
		DBG_ERROR("Failed in fork() child to restore backup!\n");
		return -CLSAPI_ERR_INTERNAL_ERR;
	} else if (pid == 0) {
		ret = system("reboot");
		if (ret == -1)
			return -CLSAPI_ERR_SYSTEM_ERROR;
		exit(0);
	} else {
		/* Parent process, non-blocking, return immediately */
		return CLSAPI_OK;
	}
}

int clsapi_sys_upgrade_firmware(const char *filepath)
{
	int ret = CLSAPI_OK;
	char cmd[256];
	pid_t pid;

	if (!filepath)
		return -CLSAPI_ERR_INVALID_PARAM;

	ret = snprintf(cmd, sizeof(cmd), "/sbin/sysupgrade --test %s", filepath);
	if (ret >= sizeof(cmd)) {
		DBG_ERROR("filepath is too long!\n");
		return -CLSAPI_ERR_INVALID_PARAM;
	}

	ret = system(cmd);
	if (!WIFEXITED(ret) || WEXITSTATUS(ret) != 0)
		return -CLSAPI_ERR_SYSTEM_ERROR;

	/* Non blocking waiting for child processes to perform system upgrades. */
	pid = fork();
	if (pid < 0) {
		DBG_ERROR("Failed in fork() child to upgrade firmware!\n");
		return -CLSAPI_ERR_INTERNAL_ERR;
	} else if (pid == 0) {
		ret = snprintf(cmd, sizeof(cmd), "/sbin/sysupgrade %s", filepath);
		if (ret >= sizeof(cmd)) {
			DBG_ERROR("filepath is too long!\n");
			return -CLSAPI_ERR_INVALID_PARAM;
		}
		ret = system(cmd);
		if (ret == -1)
			return -CLSAPI_ERR_SYSTEM_ERROR;

		exit(0);
	} else {
		 /* Parent process, non-blocking, return immediately */
		return CLSAPI_OK;
	}
}

int clsapi_sys_trigger_system_reboot(void)
{
	sync();

	if (reboot(RB_AUTOBOOT) == -1) {
		DBG_ERROR("Internal error-reboot failed\n");
		return -CLSAPI_ERR_SYSTEM_ERROR;
	}

	return CLSAPI_OK;
}

