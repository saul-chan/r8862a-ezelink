/*
 * Copyright (C) 2023 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */

#ifndef __CAPWAP_WTPUCI_HEADER__
#define __CAPWAP_WTPUCI_HEADER__

#include "CWCommon.h"

CWBool WTPUciAddAPInterface(int radioIndex, int wlanIndex, WTPInterfaceInfo * interfaceInfo);
CWBool WTPUciDelAPInterface(char *ifname);

#endif

