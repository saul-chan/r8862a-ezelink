// SPDX-License-Identifier: GPL-2.0
// Copyright(c) 2023 Clourneysemi Corporation.

#include "npe_switch.h"
#include <arpa/inet.h>

bool isHex(const char* str) {
    return (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'));
}

void switch_option_setup(struct option options[], struct switch_field_def *switch_options, int size)
{
	int i;

	for (i = 0; i < size; i++) {
		options[i].name = switch_options[i].name;
		options[i].val = switch_options[i].index;
		options[i].has_arg = required_argument;
		options[i].flag = NULL;
	}

	options[size].name = 0;
	options[size].has_arg = 0;
	options[size].flag = NULL;
	options[size].val = 0;
}

bool switch_mac_to_u64(const char *mac_address, u64 *val_64)
{
	struct ether_addr ea;

	if (ether_aton_r(mac_address, &ea)) {
		*val_64 = ea.ether_addr_octet[0];
		*val_64 = ea.ether_addr_octet[1] + (*val_64 << 8);
		*val_64 = ea.ether_addr_octet[2] + (*val_64 << 8);
		*val_64 = ea.ether_addr_octet[3] + (*val_64 << 8);
		*val_64 = ea.ether_addr_octet[4] + (*val_64 << 8);
		*val_64 = ea.ether_addr_octet[5] + (*val_64 << 8);

		return 1;
	}

	return 0;
}

bool switch_ipv4_to_u32(const char *ipv4, u32 *val)
{
	struct in_addr addr;

	if (inet_pton(AF_INET, ipv4, &(addr.s_addr)) != 1)
		return 0;

	*val = ntohl(addr.s_addr);

	return 1;
}

/* This function is Not a common address translation.
 * Only used for switch register field
 */
bool switch_ipv6_to_u32(const char *ipv6, u32 *val)
{
	struct in6_addr addr;

	if (inet_pton(AF_INET6, ipv6, &(addr.s6_addr)) != 1)
		return 0;

	memcpy(val, &(addr.s6_addr), sizeof(u32) * 4);

	val[0] = htons(val[0] & 0xFFFF) + ((htons((val[0] >> 16) & 0xFFFF)) << 16);
	val[1] = htons(val[1] & 0xFFFF) + ((htons((val[1] >> 16) & 0xFFFF)) << 16);
	val[2] = htons(val[2] & 0xFFFF) + ((htons((val[2] >> 16) & 0xFFFF)) << 16);
	val[3] = htons(val[3] & 0xFFFF) + ((htons((val[3] >> 16) & 0xFFFF)) << 16);


	return 1;
}

u32 get_zero_bits(u8 m, u8 n)
{
        u32 mask;

        if (m > n || m > 31 || n > 31) {
                npecmd_err("[%s] invalid params: %d %d\n", __func__, m, n);
                return 0;
        }

        if (m == 0 && n == 31)
                mask = 0x0;
        else
                mask = ~(((1U << (n - m + 1)) - 1) << m);

        return mask;
}

u32 get_one_bits(u8 m, u8 n)
{
	u32 mask;

        if (m > n || m > 31 || n > 31) {
                npecmd_err("[%s] invalid params: %d %d\n", __func__, m, n);
                return 0;
        }

        if (m == 0 && n == 31)
                mask = 0xFFFFFFFF;
        else
                mask = ((1U << (n - m + 1)) - 1) << m;

        return mask;
}

// hashkey = hashkey >> shift
void switch_u32_array_right_shift(u32 *hashkey, u8 size, u8 shift)
{
        int i;
        u16 base_value = get_one_bits(0, shift -1 );

        for (i = 0; i < size - 1; i++) {
                hashkey[i] = hashkey[i] >> shift;
                hashkey[i] |= ((hashkey[i + 1] & base_value) << (32 - shift));
        }

        hashkey[size - 1] = hashkey[size - 1] >> shift;
}

void switch_set_field(u32 *src, struct switch_field_def field, u32 *value)
{
        int start_bit = field.start;
        int end_bit = field.width + start_bit - 1;
        int start_m = start_bit / 32;
        int start_n = start_bit % 32;
        int end_m = end_bit / 32;
        int end_n = end_bit % 32;
        int over_word = end_m - start_m;

	// update value[] :from src[strat_m] start_n bits to src[end_m] end_n bits
        npecmd_dbg("[%s]start_bit=%d(%d %d) end_bit=%d(%d %d)\n", __func__, start_bit, start_m, start_n, end_bit, end_m ,end_n);

        if (!over_word) {
                //clear
                src[start_m] &= get_zero_bits(start_n, end_n);
                npecmd_dbg("[%s]flag1: src[%u]=0x%08x\n", __func__, start_m, src[start_m]);

                //update
                //src[end_m] |= (value[0] & get_one_bits(0, field.width - 1)) << start_n;
                src[end_m] |= value[0] << start_n;
                npecmd_dbg("[%s]flag2: src[%u]=0x%08x\n", __func__, start_m, src[start_m]);
        } else {
                int i;

                //clear
                src[start_m] &= get_zero_bits(start_n, 31);
                npecmd_dbg("[%s]flag3: src[%u]=0x%08x\n", __func__, start_m, src[start_m]);
                for (i = 0; i < over_word - 1; i++) {
                        src[start_m + i + 1] = 0;
                        npecmd_dbg("[%s]flag4: src[%u]=0x%08x\n", __func__, start_m + i + 1, src[start_m + i + 1]);
                }
                src[start_m + over_word] &= get_zero_bits(0, end_n);
                npecmd_dbg("[%s]flag5: src[%u]=0x%08x\n", __func__, start_m + over_word, src[start_m + over_word]);

                //update
                src[start_m] |= value[0] << start_n;
                npecmd_dbg("[%s]flag6: src[%u]=0x%08x\n", __func__, start_m, src[start_m]);
                for (i = 0; i < over_word - 1; i++) {
			if (start_n != 0)
	                        src[start_m + i + 1] |= value[i] >> (32 - start_n);

                        src[start_m + i + 1] |= value[i + 1] << start_n;
                        npecmd_dbg("[%s]flag7: src[%u]=0x%08x\n", __func__, start_m + i + 1, src[start_m + i + 1]);
                }

                if (end_n > start_n) {
			if (start_n != 0)
                        src[start_m + over_word] |= value[over_word - 1] >> (32 - start_n);
                        //src[start_m + over_word] |= (value[over_word] & get_one_bits(0, end_n - start_n - 1)) << start_n;
                        src[start_m + over_word] |= value[over_word] << start_n;

                        npecmd_dbg("[%s]flag8: src[%u]=0x%08x\n", __func__, start_m + over_word, src[start_m + over_word]);
                } else {
                        //src[start_m + over_word] |= (value[over_word - 1] >> (32 - start_n)) & get_one_bits(0, end_n);
			if (start_n != 0)
                        	src[start_m + over_word] |= value[over_word - 1] >> (32 - start_n);
                        npecmd_dbg("[%s]flag9: src[%u]=0x%08x\n", __func__, start_m + over_word, src[start_m + over_word]);
                }
        }

}

/* This function is used to clear unused bit field */
// Note: the val_size/vidth will not be checked.
static int switch_modify_value_by_width(u32 *value, int val_size, int width)
{
        int m = width / 32;
        int n = width % 32;
        int i, need_words;

        need_words = n ? (m + 1) : m;

	if (val_size < need_words) {
		npecmd_err("[%s] val_size is too samll: %d < %d\n", __func__, val_size, need_words);
		return 0;
	} else {
		if (n)
        		value[need_words - 1] &= get_one_bits(0, n - 1);

		if (val_size > need_words) {
			for (i = need_words; i < val_size; i++)
				value[i] = 0;
		}
	}

        return 1;
}

int switch_field_setup(int argc, char **argv,
			struct switch_field_def switch_table[], int size,
			u32 *src)
{
	int c = 0, option_index = 0;
	struct option options[size + 1];
	u64 value;
	u32 val[4];

	switch_option_setup(options, switch_table, size);

	while ((c = getopt_long_only(argc, argv, "", options, &option_index)) != -1) {
		if (c >= size || c < 0) {
			npecmd_err("[%s] invalid param index: %d\n", __func__, c);
			return 0;
		}

		memset(val, 0, sizeof(val));

		/*
			1. the width of most field is less than 64;
			2. ipv4/ipv6/macaddr/compareData(ACL) will not be processed here.
			3. ipv4/ipv6/macaddr/compareData(ACL) will be specially processed
		 */
		if (switch_table[c].flag == SWITCH_FIELD_TYPE_SHORT_VALUE) {
			if ((isHex(optarg) && sscanf(optarg, "%lx", &value) == 1)
				|| (!isHex(optarg) && sscanf(optarg, "%lu", &value) == 1)) {
				val[0] =  value & 0xFFFFFFFF;
				val[1] =  (value >> 32) & 0xFFFFFFFF;

			} else {
				npecmd_err("[%s] invalid SHORT_VALUE: %s\n", __func__, options[option_index].name);
				return 0;
			}
		} else if (switch_table[c].flag == SWITCH_FIELD_TYPE_IPV4) {
			if (!switch_ipv4_to_u32(optarg, &val[0])) {
				npecmd_err("[%s] Invalid IPV4: --%s=%s\n", __func__, options[option_index].name, optarg);
				return 0;
			}
		} else if (switch_table[c].flag == SWITCH_FIELD_TYPE_IPV6) {
			if (!switch_ipv6_to_u32(optarg, &val[0])) {
				npecmd_err("[%s] Invalid IPV6: --%s=%s\n", __func__, options[option_index].name, optarg);
				return 0;
			}
		} else if (switch_table[c].flag == SWITCH_FIELD_TYPE_MACADDR) {
			if (switch_mac_to_u64(optarg, &value)) {
				val[0] =  value & 0xFFFFFFFF;
				val[1] =  (value >> 32) & 0xFFFFFFFF;
			} else {
				npecmd_err("[%s] Invalid MACADDR: %s\n", __func__, options[option_index].name);
				return 0;
			}
		} else if (switch_table[c].flag == SWITCH_FIELD_TYPE_LONG_VALUE) {
			npecmd_err("[%s] unsupported LONG_VALUE type: %s\n", __func__, options[option_index].name);
			return 0;
		} else if (switch_table[c].flag == SWITCH_FIELD_TYPE_OTHER) {
			npecmd_err("[%s] unsupported OTHER type: %s\n", __func__, options[option_index].name);
			return 0;
		}

		if (switch_modify_value_by_width(val, 4, switch_table[c].width)) {
			npecmd_err("[%s] will update %s(type=%d) : 0x[%08x %08x %08x %08x]\n", __func__,
						options[option_index].name, switch_table[c].flag,
						val[3], val[2], val[1], val[0]);

			switch_set_field(src, switch_table[c], val);
		}
		else {
			npecmd_err("[%s] invalid param width: %s\n", __func__, options[option_index].name);
			return 0;
		}
	}

	if (optind < argc) {
		npecmd_err("[%s] non-option ARGV-elements: ", __func__);
		while (optind < argc)
			npecmd_err("%s ", argv[optind++]);
		npecmd_err("\n");

		return 0;
	}

	return 1;
}
