/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2023 Clourneysemi Corporation. */

#ifndef _NPE_SWITCH_H_
#define _NPE_SWITCH_H_

#include "npe.h"
#include <net/ethernet.h>

#define ETH_ALEN	6

struct switch_getopt_option {
	char *name;
	int val;
};

struct switch_field_def {
	int index; // used in getopt()
        char *name;// Field Name
        u32 width;
        u32 start;//Field start bit
#define SWITCH_FIELD_TYPE_SHORT_VALUE		0//u64 (default)
#define SWITCH_FIELD_TYPE_IPV4			1
#define SWITCH_FIELD_TYPE_IPV6			2
#define SWITCH_FIELD_TYPE_MACADDR		3
#define SWITCH_FIELD_TYPE_LONG_VALUE		4// over 64 bits value
#define SWITCH_FIELD_TYPE_OTHER			5// unsupported
	u8 flag;
};

struct switch_register_info {
	u64 address;
	u32 entries;
	u32 addr_per_entry;
};

void switch_option_setup(struct option options[], struct switch_field_def *switch_options, int size);
struct ether_addr *ether_aton_r(const char *asc, struct ether_addr *addr);
bool switch_mac_to_u64(const char *mac_address, u64 *val_64);
int switch_field_setup(int argc, char **argv,
			struct switch_field_def switch_table[], int size, u32 *src);

bool switch_ipv4_to_u32(const char *ipv4, u32 *val);
bool switch_ipv6_to_u32(const char *ipv6, u32 *val);
bool switch_mac_to_u64(const char *mac_address, u64 *val_64);
u32 get_zero_bits(u8 m, u8 n);
u32 get_one_bits(u8 m, u8 n);
bool isHex(const char* str);
void switch_u32_array_right_shift(u32 *hashkey, u8 size, u8 shift);
#endif /* _NPE_SWITCH_H_ */
