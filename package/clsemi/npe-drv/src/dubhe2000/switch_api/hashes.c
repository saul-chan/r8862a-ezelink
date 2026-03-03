#include "hashes.h"

uint32_t calc_l2_hash( uint64_t key ) {
  // key: 60 bits hash key
  //     key[59:48] = GID
  //     key[47:0] = MAC
  // fold count = 6
  // returns: 10 bits hash value
  //
  uint32_t hashval;
  hashval = key & 0x3ff;
  hashval = hashval ^ (key>>10);
  hashval = hashval & 0x3ff;
  hashval = hashval ^ (key>>20);
  hashval = hashval & 0x3ff;
  hashval = hashval ^ (key>>30);
  hashval = hashval & 0x3ff;
  hashval = hashval ^ (key>>40);
  hashval = hashval & 0x3ff;
  hashval = hashval ^ (key>>50);
  hashval = hashval & 0x3ff;
  return hashval;
}

uint32_t
l2_hash( uint32_t gid, uint64_t mac ) {
  uint64_t key;
  key = (uint64_t)(gid & 0xfff) << 48;
  key |= mac & 0xffffffffffffull;
  return calc_l2_hash( key );
}

uint32_t calc_l3_ipv4_hash( uint64_t key ) {
  // key: 34 bits hash key
  //     key[33:32] = VRF
  //     key[31:0] = IP address
  // fold count = 4
  // returns: 9 bits hash value

  uint32_t hashval;
  hashval = key & 0x1ff;
  hashval = hashval ^ (key>>9);
  hashval = hashval & 0x1ff;
  hashval = hashval ^ (key>>18);
  hashval = hashval & 0x1ff;
  hashval = hashval ^ (key>>27);
  hashval = hashval & 0x1ff;
  return hashval;
}

uint32_t
l3_ipv4_hash( uint32_t vrf, uint32_t ip_addr ) {
  uint64_t key;
  key = (uint64_t)(vrf & 0x3) << 32;
  key |= ip_addr;
  return calc_l3_ipv4_hash( key );
}

// 2001:1234::1 --- u32 ip_addr[4] = {0x1, 0x0, 0x0, 0x20011234};
uint32_t
l3_ipv6_hash( uint32_t vrf, const uint32_t *ip_addr ) {
	uint32_t tmp[5] = {0};
	uint16_t hashval = 0;
	uint8_t shift = 9;
	uint32_t base_value = BITS_32(0, shift -1 );
	int i;

	memcpy(tmp, ip_addr, sizeof(u32) * 4);
	tmp[4] = vrf;

	hashval = tmp[0] & base_value;

	for (i = 1; i < 15; i++) {
		switch_u32_array_right_shift(tmp, ARRAY_SIZE(tmp), shift);
		hashval = hashval ^ tmp[0];
		hashval = hashval & base_value;
	}

	return hashval;
}

uint32_t calc_l3_mpls_hash( uint64_t key ) {
  // key: 25 bits hash key
  //     key[24:23] = VRF
  //     key[22:20] = source port
  //     key[19:0]    = MPLS label
  // fold count = 3
  // returns: 9 bits hash value

  uint32_t hashval;
  hashval = key & 0x1ff;
  hashval = hashval ^ (key>>9);
  hashval = hashval & 0x1ff;
  hashval = hashval ^ (key>>18);
  hashval = hashval & 0x1ff;
  return hashval;
}

uint32_t
l3_mpls_hash( uint32_t vrf, uint32_t source_port, uint32_t label ) {
  uint64_t key;
  key = (vrf & 0xfff) << 23;
  key |= ( source_port & 0x7 ) << 20;
  key |= label & 0xfffff;
  return calc_l3_mpls_hash( key );
}


