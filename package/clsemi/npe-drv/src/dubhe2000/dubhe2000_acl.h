#ifndef _DUBHE2000_ACL_H_
#define _DUBHE2000_ACL_H_

/* Ingress ACLN: 0 -3: value '0' means inlvaid */
struct acl_hash_setting {
	u8 compareData_u8_size; //u8
	u16 compareData_width;	//bit
	u8 large_hash_size;
	u8 large_bucket_size;
	u8 small_hash_size;
	u8 small_bucket_size;
};

enum SWITCH_ACL_L4_TYPE {
	L4_TYPE_NOT_ANY = 0,
	L4_TYPE_SPECIAL_IP, /* IPv6 or IPv4 packet but  L4 protocol is not UDP, TCP, IGMP, ICMP, ICMPv6 or MLD */
	L4_TYPE_UDP,
	L4_TYPE_TCP,
	L4_TYPE_IGMP,
	L4_TYPE_ICMP,
	L4_TYPE_ICMPV6, // excluding MLD
	L4_TYPE_MLD,

	L4_TYPE_MASK = 0x7,
};

enum SWITCH_ACL_L3_TYPE {
	L3_TYPE_IPV4 = 0,
	L3_TYPE_IPV6,
	L3_TYPE_MPLS,
	L3_TYPE_OTHER,

	L3_TYPE_MASK = 0x3,
};

enum SWITCH_ACL_TCP_FLAGS {
	TCP_FLAGS_BIT_NS = 0,
	TCP_FLAGS_BIT_CWR = 1,
	TCP_FLAGS_BIT_ECE = 2,
	TCP_FLAGS_BIT_URG = 3,
	TCP_FLAGS_BIT_ACK = 4,
	TCP_FLAGS_BIT_PSH = 5,
	TCP_FLAGS_BIT_RST = 6,
	TCP_FLAGS_BIT_SYN = 7,
	TCP_FLAGS_BIT_FIN = 8,
};

struct dubhe1000_tc_ipv4_nat_hashkey {
	u32 valid:7;
	u32 src_addr_part0:25;

	u32 src_addr_part1:7;
	u32 dest_addr_part0:25;

	u32 dest_addr_part1:7;
	u32 src_port:16;
	u32 dest_port_part0:9;

	u32 dest_port_part1:7;
	u32 l4_proto:8;
	u32 source_port:3; //source switch port
	u32 reserved:14;
};

struct dubhe1000_tc_ipv6_router_hashkey {
	u32 valid:7;
	u32 src_ipv6_part0:25;

	u32 src_ipv6_part1;
	u32 src_ipv6_part2;
	u32 src_ipv6_part3;

	u32 src_ipv6_part4:7;
	u32 dest_ipv6_part0:25;

	u32 dest_ipv6_part1;
	u32 dest_ipv6_part2;
	u32 dest_ipv6_part3;

	u32 dest_ipv6_part4:7;
	u32 src_port:16;
	u32 dest_port_part0:9;

	u32 dest_port_part1:7;
	u32 l4_proto:8;
	u32 source_port:3; //source switch port
	u32 reserved:14;
};

struct dubhe2000_tc_acl1_ip_hashkey { // for non-nat ipv4
	u32 valid:7;
	u32 ipv4_sa_part0:25;

	u32 ipv4_sa_part1:7;
	u32 ipv4_da_part0:25;

	u32 ipv4_da_part1:7;
	u32 l4_sport:16;
	u32 l4_dport_part0:9;

	u32 l4_dport_part1:7;
	u32 l4_proto:8;
	u32 source_port:3;
	u32 reserved:14;
};

struct dubhe2000_tc_acl1_nonip_hashkey { // for non-IP
	u32 valid:7;
	u32 mac_da_part0:25;

	u32 mac_da_part1:23;
	u32 mac_sa_part0:9;

	u32 mac_sa_part1:32;

	u32 mac_sa_part2:7;
	u32 has_vlans:1;
	u32 outer_vlan_tag_type:1;
	u32 inner_vlan_tag_type:1;
	u32 etype:16;
	u32 source_port:3;
	u32 reserved:3;
};

struct dubhe2000_tc_acl2_ipv4_hashkey { // for non-nat ipv4
	u32 valid:20;
	u32 mac_da_part0:12;

	u32 mac_da_part1:32;

	u32 mac_da_part2:4;
	u32 mac_sa_part0:28;

	u32 mac_sa_part1:20;
	u32 outer_vid:12;

	u32 has_vlans:1;
	u32 outer_type:1;
	u32 inner_type:1;
	u32 outer_pcp:3;
	u32 outer_dei:1;
	u32 inner_vid:12;
	u32 inner_pcp:3;
	u32 inner_dei:1;
	u32 ipv4_sa_part0:9;

	u32 ipv4_sa_part1:23;
	u32 ipv4_da_part0:9;

	u32 ipv4_da_part1:23;
	u32 tos:8;
	u32 l4_sport_part0:1;

	u32 l4_sport_part1:15;
	u32 l4_dport:16;
	u32 l4_proto_part0:1;

	u32 l4_proto_part1:7;
	u32 l4_type:3;
	u32 source_port:3;
	u32 reserved:19;
};

struct dubhe2000_tc_acl2_ipv6_hashkey { // for non-routing ipv6
	u32 valid:20;
	u32 mac_da_part0:12;

	u32 mac_da_part1:32;

	u32 mac_da_part2:4;
	u32 mac_sa_part0:28;

	u32 mac_sa_part1:20;
	u32 outer_vid:12;

	u32 has_vlans:1;
	u32 outer_type:1;
	u32 inner_type:1;
	u32 outer_pcp:3;
	u32 outer_dei:1;
	u32 inner_vid:12;
	u32 inner_pcp:3;
	u32 inner_dei:1;
	u32 ipv6_sa_part0:9;

	u32 ipv6_sa_part1:32;
	u32 ipv6_sa_part2:32;
	u32 ipv6_sa_part3:32;

	u32 ipv6_sa_part4:23;
	u32 ipv6_da_part0:9;

	u32 ipv6_da_part1:32;
	u32 ipv6_da_part2:32;
	u32 ipv6_da_part3:32;

	u32 ipv6_da_part4:23;
	u32 tos:8;
	u32 l4_sport_part0:1;

	u32 l4_sport_part1:15;
	u32 l4_dport:16;
	u32 ipv6_flow_lable_part0:1;

	u32 ipv6_flow_lable_part1:19;
	u32 l4_proto:8;
	u32 l4_type:3;
	u32 source_port_part0:2;

	u32 source_port_part1:1;
	u32 reserved:31;
};

struct dubhe2000_tc_acl3_hashkey {
	u32 valid:10;
	u32 l2_packet_flags:4;
	u32 ipv4_options:5;
	u32 tcp_flags:9;
	u32 l4_proto_part0:4;

	u32 l4_proto_part1:4;
	u32 ethernet_type:16;
	u32 l4_type:3;
	u32 l3_type:2;
	u32 reserved:7;
};

enum { // acl0/1/2 counter
	SWITCH_INGRESS_ACL_COUNTER_MIN = 0,
	SWITCH_INGRESS_ACL_COUNTER_SVP = SWITCH_INGRESS_ACL_COUNTER_MIN,//SVP
	SWITCH_INGRESS_ACL_COUNTER_NONIP,				//NONIP
	SWITCH_INGRESS_ACL_COUNTER_FTP,					//FTP
	SWITCH_INGRESS_ACL_COUNTER_IPV6_FRAG,				//ipv6 frag
	SWITCH_INGRESS_ACL_COUNTER_IPERF_TCP,				//iperf:tcp
	SWITCH_INGRESS_ACL_COUNTER_IPERF_SPORT,			//iperf:tcp+sport
	SWITCH_INGRESS_ACL_COUNTER_IPERF_DPORT,			//iperf:tcp+dport

	SWITCH_INGRESS_ACL_COUNTER_MAX = 63,
};
#endif /* _DUBHE2000_ACL_H_ */
