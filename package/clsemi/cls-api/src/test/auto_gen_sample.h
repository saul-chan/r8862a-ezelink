/*
 * Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */


#include "clsapi_common.h"

/**
 * @brief Get Wi-Fi name of the device
 * \param mac [in] The MAC address of device phy1, ...
 * \param name [out] The name of the device.
 * \param name_len [out] The length of the name.
 */
extern int clsapi_get_net_client_name(const char *mac, char *name, int name_len);
