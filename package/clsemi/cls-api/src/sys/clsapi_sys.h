/*
 * Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */

#ifndef _CLSAPI_SYS_H
#define _CLSAPI_SYS_H

#include "clsapi_common.h"

/** System memory usage information, the unit of all data in struct is kbytes */
struct clsapi_meminfo {
	/** Total usable main memory size */
	unsigned long total;

	/** Available memory size */
	unsigned long free;

	/** Amount of shared memory */
	unsigned long shared;

	/** Memory used by buffers */
	unsigned long buffered;

	/** System available memory */
	unsigned long long available;

	/** Amount of memory cached */
	unsigned long long cached;
};

/** System storage usage information, the unit of all data in struct is kbytes */
struct clsapi_storage_info {
	/** The total capacity of storage devices */
	unsigned long total;

	/** Remaining unallocated space */
	unsigned long free;

	/** used space */
	unsigned long used;

	/** Actual available space for users */
	unsigned long avail;
};

/** System board information */
struct clsapi_board_info {
	/** Linux kernel version number */
	string_64 kernel;

	/** Hostname, used to identify a computer */
	string_64 hostname;

	/** CPU architecture information */
	string_64 system;

	/** Device model */
	string_64 model;

	/** Board name */
	string_64 board_name;

	/** Root file system type */
	string_16 rootfs_type;

	/** The distribution name of the release */
	string_64 release_distribution;

	/** The release version number of the release */
	string_64 release_version;

	/** The revision number of the release */
	string_64 release_revision;

	/** The target platform of the release */
	string_64 release_target;

	/** The description of the release */
	string_64 release_description;
};

/*******************************	Macro definitions	**************************/
#define CLS_DEV_FEATURE_VPN			BIT(0)

#define CLS_DEV_FEATURE_COMMON		(CLS_DEV_FEATURE_VPN)

/******************	Configurations	******************/
#define CLSCONF_CFG_RPCD			"rpcd"
#define CLSCONF_CFG_SYSTEM		"system"
#define CLSCONF_CFG_LUCI        "luci"
#define CLSCONF_CFG_UBOOT_EVN	"ubootenv"
#define CLSCONF_CFG_UCI_TRACK	"ucitrack"
#define CLSCONF_CFG_DROPBEAR	"dropbear"

/*****************************	Data type definitions	**************************/


/*****************************	Variables definitions	**************************/


/*!\addtogroup system
 *  @{
*/

/*****************************	Functions declarations	*************************/

/**
 * \brief Get the status of led.
 * \details Get led on/off status by GPIO value. This is based on "/sys/class/gpio/".
 * \param led_gpio [In] The GPIO number of this led.
 * \param onoff [Out] The status of led (on/off).
 * \return 0 on success or others on error.
 */
int clsapi_sys_get_led(const uint8_t led_gpio, bool *onoff);

/**
 * \brief Turn on/off led.
 * \details Turn on/off led by controlling GPIO. This API is based on "/sys/class/gpio/".
 * \note
 *	There are several ways to control led and they are mutually exclusive.Please disable other
 * ways of led controlling before call this API. Other commands of disabling leds:
 * \n	/etc/init.d/cls-led stop
 * \n	rmmod leds_gpio
 * \param led_gpio [In] GPIO number of this led.
 * \param onoff [In] The status of this led(on/off).
 * \return 0 on success or others on error.
 */
int clsapi_sys_set_led(const uint8_t led_gpio, const bool onoff);

/**
 * \brief Get hostname of the device.
 * \details Get hostname of the device.
 * \param hostname [Out] The hostname of device.
 * \return 0 on success or others on error.
 */
int clsapi_sys_get_hostname(string_1024 hostname);

/**
 * \brief Set hostname of the device.
 * \details Set hostname of the device.
 * \param hostname [In] The hostname of device.
 * \return 0 on success or others on error.
 */
int clsapi_sys_set_hostname(const char *hostname);

/**
 * \brief Restore the machine to factory settings.
 * \details Restore the machine to factory settings. The device will reboot immediately.
 * \return 0 on success or others on error.
 */
int clsapi_sys_restore_factory(void);

/**
 * \brief Get the language of system GUI.
 * \details Get the language of system GUI.
 * \param lang [Out] The language of system GUI.
 * \return 0 on success or others on error.
 */
int clsapi_sys_get_lang(string_1024 lang);

/**
 * \brief Set the language of the system GUI.
 * \details Set the language of the system GUI.
 * \param lang [In] The language of system GUI will be set.
 * \return 0 on success or others on error.
 */
int clsapi_sys_set_lang(const char *lang);

/**
 * \brief Get the interface information which the current SSH server is listening.
 * \details Get the interface information which is attached to current SSH server.
 * \param idx [In] The index of the SSH access policy(default 0). Only one policy is supported now.
 * \param interface [Out] The interface which the SSH server is listening.
 * \return 0 on success or others on error.
 */
int clsapi_sys_get_ssh_iface(const uint8_t idx, string_32 interface);

/**
 * \brief Set the interface information which is attached to current SSH server.
 * \details Set the interface information which is attached to current SSH server.
 * \param idx [In] The index of the SSH access policy(default 0). Only one policy is supported now.
 * \param interface [In] The new interface. Currently only four interfaces are supported: all, lan, wan and wan6. "all" means listening all interfaces.
 * \return 0 on success or others on error.
 */
int clsapi_sys_set_ssh_iface(const uint8_t idx, const char *interface);

/**
 * \brief Get SSH server port.
 * \details Get SSH server port.
 * \param idx [In] The index of the SSH access policy(default 0). Only one policy is supported now.
 * \param port [Out] The port of the SSH server.
 */
int clsapi_sys_get_ssh_port(const uint8_t idx, uint16_t *port);

/**
 * \brief Set port of the SSH server.
 * \details Set port of the SSH server.
 * \param idx [In] The index of the SSH access policy(default 0). Only one policy is supported now.
 * \param port [In] The new port of the SSH server, between 0~65535.
 */
int clsapi_sys_set_ssh_port(const uint8_t idx, const uint16_t port);

/**
 * \brief Get the authentication status of the SSH server.
 * \details Get the authentication status of the SSH server.
 * \param idx [In] The index of the SSH access policy(default 0). Only one policy is supported now.
 * \param onoff [Out] The status of authentication(enable or disable).
 * \return 0 on success or others on error.
 */
int clsapi_sys_get_ssh_password_auth(const uint8_t idx, bool *onoff);

/**
 * \brief Set the authentication status of the SSH server.
 * \details Set the authentication status of the SSH server.
 * \param idx [In] The index of the SSH access policy(default 0). Only one policy is supported now.
 * \param onoff [In] Set 0/1 to disable/enable authenticating with passwords.
 * \return 0 on success or others on error.
 */
int clsapi_sys_set_ssh_password_auth(const uint8_t idx, const bool onoff);

/**
 * \brief Get if the user needs to login as root to access the SSH server.
 * \details Get if the user needs to login as root to access the SSH server.
 * \param idx [In] The index of the SSH access policy(default 0). Only one policy is supported now.
 * \param onoff [Out] Set 0/1 to disable/enable logining as root.
 * \return 0 on success or others on error.
 */
int clsapi_sys_get_ssh_root_login(const uint8_t idx, bool *onoff);

/**
 * \brief Set if the user needs to login as root to access the SSH server.
 * \details Set if the user needs to login as root to access the SSH server.
 * \param idx [In] The index of the SSH access policy(default 0). Only one policy is supported now.
 * \param onoff [In] Enable or disable authenticating as root with passwords.
 * \return 0 on success or others on error.
 */
int clsapi_sys_set_ssh_root_login(const uint8_t idx, const bool onoff);

/**
 * \brief Get system uptime.
 * \details Get system uptime.
 * \param uptime [Out] The uptime of system.
 * \return 0 on success or others on error.
 */
int clsapi_sys_get_sys_uptime(uint64_t *uptime);

/**
 * \brief Get the system memory information.
 * \details Get the system memory information.
 * \param meminfo [Out] The meminfo of the system.
 * \return 0 on success or others on error.
 */
int clsapi_sys_get_meminfo(struct clsapi_meminfo *meminfo);

/**
 * \brief Get the device storage information.
 * \details Get the device storage information.
 * \param storage_info [Out] The storage information of the system.
 * \return 0 on success or others on error.
 */
int clsapi_sys_get_storage(struct clsapi_storage_info *storage_info);

/**
 * \brief Get the board information.
 * \details Get the board information.
 * \param board_info [Out] The board information of system.
 * \return 0 on success or others on error.
 */
int clsapi_sys_get_board_info(struct clsapi_board_info *board_info);

/**
 * \brief Add a new backup file.
 * \details Add a new backup file which is needed to be backed up.
 * \param filepath [In] The path of the backup file which will be added.
 * \return 0 on success or others on error.
 */
int clsapi_sys_add_backup_file(const char *filepath);

/**
 * \brief Delete a new backup file.
 * \details Delete a new backup file which is needed to be backed up.
 * \param filepath [In] The path of the backup file which will be deleted.
 * \return 0 on success or others on error.
 */
int clsapi_sys_del_backup_file(const char *filepath);

/**
 * \brief Back up all configurations into a tar file.
 * \details Back up all system configurations and custom configurations into a tar file.
 * \param filepath [In] Absolute path for storing backup files, the file format is xxx.tar.gz.
 * \return 0 on success or others on error.
 */
int clsapi_sys_backup_conf(const char *filepath);

/**
 * \brief Restore previous saved configurations from specific filepath.
 * \details Restore previous saved configurations from specific filepath.
 * \param filepath [In] Absolute path for backup files, the file format is xxx.tar.gz.
 * \return 0 on success or others on error.
 */
int clsapi_sys_restore_backup_conf(const char *filepath);

/**
 * \brief Upgrade the device firmware.
 * \details Upgrade the device firmware.
 * \param filepath [In] The firmware file path which will be upgraded.
 * \return 0 on success or others on error.
 */
int clsapi_sys_upgrade_firmware(const char *filepath);

/**
 * \brief Reboot the whole system.
 * \details Reboot the whole system.
 * \return 0 on success or others on error.
 */
int clsapi_sys_trigger_system_reboot(void);

/*!@} */

#endif /* _CLSAPI_SYS_H */

