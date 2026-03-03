#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>

void switch_mac_to_u64(const u8 *mac_address, u64 *val_64);
void switch_ipv4_u8_to_u32(const u8 *ipv4, u32 *val);
void switch_ipv6_to_u32(const u8 *ipv6, u32 *val);
int dubhe1000_check_u8_array_is_zero(const u8 *data, u32 size);
void switch_u32_array_right_shift(u32 *hashkey, u8 size, u8 shift);

u32 get_one_bits(const char *func, int line, u8 m, u8 n);
#define BIT_32(n)	(1 << (n))// u32
#define BITS_32(m, n)	get_one_bits(__func__, __LINE__, (m), (n))
