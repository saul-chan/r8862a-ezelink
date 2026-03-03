#include "common.h"

//ipv4[] = {1, 1, 168, 192} to indicate 192.168.1.1 and val will be 0xc0a80101
void switch_ipv4_u8_to_u32(const u8 *ipv4, u32 *val)
{
	*val = ipv4[3];
	*val = ((*val) << 8) + ipv4[2];
	*val = ((*val) << 8) + ipv4[1];
	*val = ((*val) << 8) + ipv4[0];
}

u32 get_one_bits(const char *func, int line, u8 m, u8 n)
{
	u32 mask;

        if (m > n || m > 31 || n > 31) {
                pr_err("[%s] invalid params: %d %d in [%s line%d]\n", __func__, m, n, func, line);
                return 0;
        }

        if (m == 0 && n == 31)
                mask = 0xFFFFFFFF;
        else
                mask = ((1U << (n - m + 1)) - 1) << m;

        return mask;
}

/* return 1 when data[i] are zero */
int dubhe1000_check_u8_array_is_zero(const u8 *data, u32 size)
{
	int i;

	for (i = 0; i < size; i++) {
		if (*(data + i))
			return 0;
	}

	return 1;
}

// hashkey = hashkey >> shift
void switch_u32_array_right_shift(u32 *hashkey, u8 size, u8 shift)
{
        int i;
        u16 base_value = BITS_32(0, shift -1 );

        for (i = 0; i < size - 1; i++) {
                hashkey[i] = hashkey[i] >> shift;
                hashkey[i] |= ((hashkey[i + 1] & base_value) << (32 - shift));
        }

        hashkey[size - 1] = hashkey[size - 1] >> shift;
}
