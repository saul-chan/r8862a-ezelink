/*
 * Copyright (C) 2024 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */

#ifndef _CLSAPI_BASE_H
#define _CLSAPI_BASE_H

#include "clsapi_common.h"

/*******************************	Macro definitions	**************************/


/******************	Configurations	******************/


/*****************************	Data type definitions	**************************/


/*****************************	Variables definitions	**************************/

/*!\addtogroup base
 *  @{
*/

/*****************************	Functions declarations	*************************/

/**
 * \brief Get the status of configuration defer mode.
 * \details Get enabled/disabled status of configuration defer mode.
 * \param defer [Out] defer mode enabled/disabled status.
 * \return 0 on success or others on error.
 */
int clsapi_base_get_defer_mode(int *defer);

/**
 * \brief Set the status of the configuration defer mode.
 * \details Enable or disable "Configuration defer mode". When defer mode enabled, configurations are
 * saved to file but not apply. This feature is expected to reduce the configuration reload
 * time when multiple parameters are modified.
 * Example to using defer mode:
 *
 * 1. Enable defer mode
 * @code
 * clsapi set defer_mode 1
 * @endcode
 *
 * 2. Configure parameters
 * @code
 * clsapi set ssid wlan0 new_ssid
 * clsapi set passphrase wlan0 12345678
 * clsapi add bss wlan1-1 00:11:22:33:44:55
 * clsapi set ssid wlan1-1 Clourney_wlan1-1
 * clsapi set passphrase wlan0 12345678
 * @endcode
 *
 * 3. Disable defer mode
 * @code
 * clsapi set defer_mode 0
 * @endcode
 *
 * 4. Apply configurations to hostapd and/or driver/FW.
 * @code
 * clsapi apply conf_cfg wireless
 * @endcode
 *
 * \param defer [In] defer mode enabled/disabled status.
 * \return 0 on success or others on error.
 */
int clsapi_base_set_defer_mode(const int defer);

/**
 * \brief Get param value by (cfg, section, param). It's an independent API from platform.
 * \details Get param value by (cfg, section, param). It's an independent API from platform.
 * \param cfg [In] config file or module
 * \param section [In] section @ cfg
 * \param param [In] param @ section
 * \param value [Out] value of the param
 * \return 0 on success or others on error.
 */
int clsapi_base_get_conf_param(const char *cfg, const char *section, const char *param, string_1024 value);

/**
 * \brief Set param value of (cfg, section, param). It's an independent API from platform.
 * \details Set param value of (cfg, section, param). It's an independent API from platform.
 * \param cfg [In] config file or module
 * \param section [In] section @ cfg
 * \param param [In] param @ section
 * \param value [In] value of the param
 * \return 0 on success or others on error.
 */
int clsapi_base_set_conf_param(const char *cfg, const char *section, const char *param, const char *value);

/**
 * \brief Make configuration applied. It's an independent API from platform.
 * \details Make configuration applied. It's an independent API from platform.
 * \param cfg [In] config file or module
 * \return 0 on success or others on error.
 */
int clsapi_base_apply_conf_cfg(const char *cfg);

/**
 * \brief Set param value of (cfg, section, param) and make it applied. It's an independent API from platform.
 * \details Set param value of (cfg, section, param) and make it applied. It's an independent API from platform.
 * \param cfg [In] config file or module
 * \param section [In] section @ cfg
 * \param param [In] param @ section
 * \param value [In] value of the param
 * \return 0 on success or others on error.
 */
int clsapi_base_set_apply_conf_param(const char *cfg, const char *section, const char *param, const char *value);

/* !@} */

#endif /* _CLSAPI_BASE_H */
