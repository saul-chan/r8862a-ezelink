/*
 * Copyright (c) 2021-2024, Clourney Semiconductor. All rights reserved.
 *
 * This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
 * Reproduction and redistribution in binary or source form, with or without modification,
 * for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
 * must retain the above copyright notice.
 *
 * By using this software and/or documentation, you agree to the limited terms and conditions.
 */

/* dubhe1000_hw.h
 * Structures, enums, and macros for the DUBHE1000
 */

#ifndef _DUBHE1000_HW_H_
#define _DUBHE1000_HW_H_

#include "dubhe2000_osdep.h"

#define NODE_ADDRESS_SIZE 6

/* Forward declarations of structures used by the shared code */

/* Enumerated types specific to the dubhe1000 hardware */

/* Error Codes */
#define DUBHE1000_SUCCESS      0

/* The sizes (in bytes) of a ethernet packet */
#define ENET_HEADER_SIZE             14
#define MINIMUM_ETHERNET_FRAME_SIZE  64	/* With FCS */
#define ETHERNET_FCS_SIZE            4
#define MINIMUM_ETHERNET_PACKET_SIZE (MINIMUM_ETHERNET_FRAME_SIZE - ETHERNET_FCS_SIZE)
#define CRC_LENGTH                   ETHERNET_FCS_SIZE
#define MAX_JUMBO_FRAME_SIZE         0x3F00


/* Receive bit definitions */
#define DUBHE1000_RXD_STAT_EOP      0x02	/* End of Packet */
#define DUBHE1000_RXD_STAT_IXSM     0x04	/* Ignore checksum */
#define DUBHE1000_RXD_STAT_VLAN     0x08	/* IEEE VLAN Packet */
#define DUBHE1000_RXD_STAT_UDPCS    0x10	/* UDP xsum calculated */
#define DUBHE1000_RXD_STAT_TCPCS    0x20	/* TCP xsum calculated */
#define DUBHE1000_RXD_STAT_IPCS     0x40	/* IP xsum calculated */
#define DUBHE1000_RXD_STAT_PIF      0x80	/* passed in-exact filter */
#define DUBHE1000_RXD_STAT_IPIDV    0x200	/* IP identification valid */
#define DUBHE1000_RXD_STAT_UDPV     0x400	/* Valid UDP checksum */
#define DUBHE1000_RXD_STAT_ACK      0x8000	/* ACK Packet indication */
#define DUBHE1000_RXD_ERR_CE        0x01	/* CRC Error */
#define DUBHE1000_RXD_ERR_SE        0x02	/* Symbol Error */
#define DUBHE1000_RXD_ERR_SEQ       0x04	/* Sequence Error */
#define DUBHE1000_RXD_ERR_CXE       0x10	/* Carrier Extension Error */
#define DUBHE1000_RXD_ERR_TCPE      0x20	/* TCP/UDP Checksum Error */
#define DUBHE1000_RXD_ERR_IPE       0x40	/* IP Checksum Error */
#define DUBHE1000_RXD_ERR_RXE       0x80	/* Rx Data Error */
#define DUBHE1000_RXD_SPC_VLAN_MASK 0x0FFF	/* VLAN ID is in lower 12 bits */
#define DUBHE1000_RXD_SPC_PRI_MASK  0xE000	/* Priority is in upper 3 bits */
#define DUBHE1000_RXD_SPC_PRI_SHIFT 13
#define DUBHE1000_RXD_SPC_CFI_MASK  0x1000	/* CFI is bit 12 */
#define DUBHE1000_RXD_SPC_CFI_SHIFT 12

#define DUBHE1000_RXDEXT_STATERR_CE    0x01000000
#define DUBHE1000_RXDEXT_STATERR_SE    0x02000000
#define DUBHE1000_RXDEXT_STATERR_SEQ   0x04000000
#define DUBHE1000_RXDEXT_STATERR_CXE   0x10000000
#define DUBHE1000_RXDEXT_STATERR_TCPE  0x20000000
#define DUBHE1000_RXDEXT_STATERR_IPE   0x40000000
#define DUBHE1000_RXDEXT_STATERR_RXE   0x80000000

#define DUBHE1000_RXDPS_HDRSTAT_HDRSP        0x00008000
#define DUBHE1000_RXDPS_HDRSTAT_HDRLEN_MASK  0x000003FF


/* Default values for the transmit IPG register */
#define DUBHE1000_TIPG_IPGR1_SHIFT  10

#define DUBHE1000_TIPG_IPGR2_SHIFT  20


#endif /* _DUBHE1000_HW_H_ */
