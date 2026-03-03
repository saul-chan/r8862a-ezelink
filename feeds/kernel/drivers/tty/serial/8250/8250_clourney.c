// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022 Clourney Semiconductor - All Rights Reserved.
 * dubhe1000 16550 UART "driver".
 */

#include <linux/acpi.h>
#include <linux/serial_reg.h>
#include <linux/serial_8250.h>
#include "8250.h"

#define DW16550_UART_USR        0x1F /* UART Status Register */ /* Why 0x1F ? 0x7C >> 2 = 0x1F  */

int cls8250_handle_irq(struct uart_port *p)
{
	struct uart_8250_port *up = up_to_u8250p(p);
	unsigned int iir = p->serial_in(p, UART_IIR);
	unsigned int status;
	unsigned long flags;

	/*
	 * There are ways to get Designware-based UARTs into a state where
	 * they are asserting UART_IIR_RX_TIMEOUT but there is no actual
	 * data available.  If we see such a case then we'll do a bogus
	 * read.  If we don't do this then the "RX TIMEOUT" interrupt will
	 * fire forever.
	 *
	 * This problem has only been observed so far when not in DMA mode
	 * so we limit the workaround only to non-DMA mode.
	 */
	if (!up->dma && ((iir & 0x3f) == UART_IIR_RX_TIMEOUT)) {
		spin_lock_irqsave(&p->lock, flags);
		status = p->serial_in(p, UART_LSR);

		if (!(status & (UART_LSR_DR | UART_LSR_BI)))
			(void) p->serial_in(p, UART_RX);

		spin_unlock_irqrestore(&p->lock, flags);
	}

	if (serial8250_handle_irq(p, iir))
		return 1;

	if ((iir & UART_IIR_BUSY) == UART_IIR_BUSY) {
		/* Clear the USR */
		(void)p->serial_in(p, DW16550_UART_USR);
		return 1;
	}
	return 0;
}

EXPORT_SYMBOL_GPL(cls8250_handle_irq);

