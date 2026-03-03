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

#ifndef _CLS_WIFI_PCI_H_
#define _CLS_WIFI_PCI_H_


#define PCI_VENDOR_ID_CLS			0x1fec
#define PCI_DEVICE_ID_MERAK2000_EP0	0x2002
#define PCI_DEVICE_ID_MERAK2000_EP1	0x2001
#define PCI_DEVICE_ID_MERAK3000		0x3000

/* EP BAR */
#define CLS_WIFI_BAR_IDX_SYS		1
#define CLS_WIFI_BAR_IDX_MSGQ		2
#define CLS_WIFI_BAR_IDX_MEM		3
#define CLS_WIFI_BAR_IDX_REG		4

#define CLS_WIFI_IRFTABLE_OFFSET	0xC000	/* Based on SYS BAR */
#define CLS_WIFI_AGCBIN_OFFSET		0x19000 /* Based on IRF table */

#endif /* _CLS_WIFI_PCI_H_ */
