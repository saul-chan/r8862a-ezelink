/*
 * Copyright (C) 2024 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */

#include "wifi_common.h"


/** Wi-Fi channel to frequency in unit of MHz.
 * Returns:
 *   = 0: Errors
 *   > 0: OK and return frequency in unit of MHz.
 */
uint32_t channel_to_freq_mhz(int chan, enum clsapi_wifi_band band)
{
	/* see 802.11 17.3.8.3.2 and Annex J
	 * there are overlapping channel numbers in 5GHz and 2GHz bands */
	if (chan <= 0)
		return 0; /* not supported */
	switch (band) {
	case CLSAPI_BAND_2GHZ:
		if (chan == 14)
			return (2484);
		else if (chan < 14)
			return (2407 + chan * 5);
		break;
	case CLSAPI_BAND_5GHZ:
		if (chan >= 182 && chan <= 196)
			return (4000 + chan * 5);
		else
			return (5000 + chan * 5);
		break;
	case CLSAPI_BAND_6GHZ:
		/* see 802.11ax D6.1 27.3.23.2 */
		if (chan == 2)
			return (5935);
		if (chan <= 233)
			return (5950 + chan * 5);
		break;
	default:
		;
	}

	return 0; /* not supported */
}

/** Wi-Fi frequency in unit of MHz to channel.
 * Returns:
 *   = 0: Errors
 *   > 0: OK and channel number is returned
 */
uint8_t freq_mhz_to_channel(uint32_t freq)
{
	/* see 802.11 17.3.8.3.2 and Annex J */
	if (freq == 2484)
		return 14;
	else if (freq < 2484)
		return (freq - 2407) / 5;
	else if (freq >= 4910 && freq <= 4980)
		return (freq - 4000) / 5;
	else if (freq < 5925)
		return (freq - 5000) / 5;
	else if (freq == 5935)
		return 2;
	else if (freq <= 45000) /* DMG band lower limit */
		/* see 802.11ax D6.1 27.3.22.2 */
		return (freq - 5950) / 5;
	else if (freq >= 58320 && freq <= 70200)
		return (freq - 56160) / 2160;
	else
		return 0;
}

/** Wi-Fi frequency (center freq of channel) in unit of MHz to enum clsapi_wifi_band.
 * Returns:
 *   ! CLSAPI_BAND_NOSUCH_BAND: valid band
 *   = CLSAPI_BAND_NOSUCH_BAND: Errors
 */
enum clsapi_wifi_band freq_mhz_to_band(uint32_t freq)
{
	if (freq <= 2484)
		return CLSAPI_BAND_2GHZ;
	else if (freq < 5925)
		return CLSAPI_BAND_5GHZ;
	else if (freq < 7115)
		return CLSAPI_BAND_6GHZ;
	else
		return CLSAPI_BAND_NOSUCH_BAND;
}


