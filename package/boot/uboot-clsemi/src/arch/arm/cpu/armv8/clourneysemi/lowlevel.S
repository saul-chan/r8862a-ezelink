/*
 * Copyright (C) 2022 Clourney Semiconductor. All rights reserved.
 *
 * SPDX-License-Identifier:     GPL-2.0+
 *
 * Extracted from armv8/start.S
 */

#include <config.h>
#include <linux/linkage.h>
#include <asm/macro.h>

ENTRY(apply_core_errata)
        mov     x29, lr                 /* Save LR */

#ifdef CONFIG_ARM_ERRATA_855873
        mrs     x0, midr_el1
        tst     x0, #(0xf << 20)
        b.ne    0f

        mrs     x0, midr_el1
        and     x0, x0, #0xf
        cmp     x0, #3
        b.lt    0f

        mrs     x0, S3_1_c15_c2_0       /* cpuactlr_el1 */
        /* Enable data cache clean as data cache clean/invalidate */
        orr     x0, x0, #1 << 44
        msr     S3_1_c15_c2_0, x0       /* cpuactlr_el1 */
        isb
#endif
0:
        mrs     x0, S3_1_C15_C2_0
        tst     x0, #0xE000
        b.ne    1f
        b       2f

.align 6
1:
        dsb     sy
        isb
        bic     x0, x0, #0xE000
        msr     S3_1_C15_C2_0, x0
        isb
2:
        mov     lr, x29                 /* Restore LR */
        ret
ENDPROC(apply_core_errata)

