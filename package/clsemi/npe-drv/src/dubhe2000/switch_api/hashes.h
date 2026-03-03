
#ifndef __HASHES_H__
#define __HASHES_H__
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include "common.h"

uint32_t l2_hash( uint32_t gid, uint64_t mac );
uint32_t l3_ipv4_hash( uint32_t vrf, uint32_t ip_addr );
uint32_t l3_ipv6_hash( uint32_t vrf, const uint32_t *ip_addr );
uint32_t l3_mpls_hash( uint32_t vrf, uint32_t source_port, uint32_t label );
uint32_t calc_l2_hash( uint64_t key );
uint32_t calc_l3_ipv4_hash( uint64_t key );
uint32_t calc_l3_mpls_hash( uint64_t key );

#endif

