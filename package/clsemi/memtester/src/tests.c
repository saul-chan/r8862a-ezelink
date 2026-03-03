/*
 * Very simple but very effective user-space memory tester.
 * Originally by Simon Kirby <sim@stormix.com> <sim@neato.org>
 * Version 2 by Charles Cazabon <charlesc-memtester@pyropus.ca>
 * Version 3 not publicly released.
 * Version 4 rewrite:
 * Copyright (C) 2004-2020 Charles Cazabon <charlesc-memtester@pyropus.ca>
 * Licensed under the terms of the GNU General Public License version 2 (only).
 * See the file COPYING for details.
 *
 * This file contains the functions for the actual tests, called from the
 * main routine in memtester.c.  See other comments in that file.
 *
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "types.h"
#include "sizes.h"
#include "memtester.h"

char progress[] = "-\\|/";
#define PROGRESSLEN 4
#define PROGRESSOFTEN 2500
#define ONE 0x00000001L

union {
    unsigned char bytes[UL_LEN/8];
    ul val;
} mword8;

union {
    unsigned short u16s[UL_LEN/16];
    ul val;
} mword16;

extern int err_exit;
/* Function definitions. */

int compare_regions(ulv *bufa, ulv *bufb, size_t count) {
    int r = 0;
    size_t i;
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    ul tmp1 = 0, tmp2 = 0;
    off_t physaddr;

    for (i = 0; i < count; i++, p1++, p2++) {
	tmp1 = *p1;
	tmp2 = *p2;
        if (tmp1 != tmp2) {
            if (use_phys) {
                physaddr = physaddrbase + (i * sizeof(ul));
                fprintf(stderr,
			"1st FAILURE: 0x%08lx @bufa %p != 0x%08lx @bufb %p at physical address "
                        "0x%08lx.\n",
			(ul) tmp1, bufa, (ul) tmp2, bufb, physaddr);
                fprintf(stderr,
                        "2nd FAILURE: 0x%08lx @bufa %p != 0x%08lx @bufb %p at physical address "
                        "0x%08lx.\n",
                        (ul) *p1, bufa, (ul) *p2, bufb, physaddr);
                fprintf(stderr,
                        "3rd FAILURE: 0x%08lx @bufa %p != 0x%08lx @bufb %p at physical address "
                        "0x%08lx.\n",
                        (ul) *p1, bufa, (ul) *p2, bufb, physaddr);
		if (err_exit)
			return -1;
            } else {
                fprintf(stderr, 
                        "1st FAILURE: 0x%08lx @bufa %p != 0x%08lx @bufb %p at offset 0x%08lx.\n", 
                        (ul) *p1, bufa, (ul) *p2, bufb, (ul) (i * sizeof(ul)));
		fprintf(stderr,
                        "2nd FAILURE: 0x%08lx @bufa %p != 0x%08lx @bufb %p at offset 0x%08lx.\n", 
                        (ul) *p1, bufa, (ul) *p2, bufb, (ul) (i * sizeof(ul)));
		fprintf(stderr,
                        "3rd FAILURE: 0x%08lx @bufa %p != 0x%08lx @bufb %p at offset 0x%08lx.\n", 
                        (ul) *p1, bufa, (ul) *p2, bufb, (ul) (i * sizeof(ul)));
		if (err_exit)
			return -1;
            }
            /* printf("Skipping to next test..."); */
            r = -1;
        }
    }
    return r;
}

int compare_regions_with_expected(ulv *bufa, ulv *bufb, size_t count, ul expected) {
    int r = 0;
    size_t i;
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    ul tmp1 = 0, tmp2 = 0;
    off_t physaddr;

    for (i = 0; i < count; i++, p1++, p2++) {
	tmp1 = *p1;  tmp2 = *p2;
        if (tmp1 != tmp2) {
            if (use_phys) {
                physaddr = physaddrbase + (i * sizeof(ul));
                fprintf(stderr, 
                        "1st FAILURE: 0x%08lx @bufa %p != 0x%08lx @bufb %p expected 0x%08lx at physical address "
                        "0x%08lx.\n", 
                        (ul) tmp1, bufa, (ul) tmp2, bufb, (ul)expected, physaddr);
                fprintf(stderr,
			"2nd FAILURE: 0x%08lx @bufa %p != 0x%08lx @bufb %p expected 0x%08lx at physical address "
                        "0x%08lx.\n",
			(ul) *p1, bufa, (ul) *p2, bufb, (ul)expected, physaddr);
                fprintf(stderr,
			"3rd FAILURE: 0x%08lx @bufa %p != 0x%08lx @bufb %p expected 0x%08lx at physical address "
                        "0x%08lx.\n", 
                        (ul) *p1, bufa, (ul) *p2, bufb, (ul)expected, physaddr);
		if (err_exit)
			return -1;
            } else {
                fprintf(stderr, 
                        "1st FAILURE: 0x%08lx @bufa %p != 0x%08lx @bufb %p expected 0x%08lx at offset 0x%08lx.\n", 
			(ul) *p1, bufa, (ul) *p2, bufb, (ul)expected, (ul) (i * sizeof(ul)));
                fprintf(stderr,
			"2nd FAILURE: 0x%08lx @bufa %p != 0x%08lx @bufb %p expected 0x%08lx at offset 0x%08lx.\n",
                        (ul) *p1, bufa, (ul) *p2, bufb, (ul)expected, (ul) (i * sizeof(ul)));
                fprintf(stderr, 
			"3rd FAILURE: 0x%08lx @bufa %p != 0x%08lx @bufb %p expected 0x%08lx at offset 0x%08lx.\n",
			(ul) *p1, bufa, (ul) *p2, bufb, (ul)expected, (ul) (i * sizeof(ul)));
		if (err_exit)
			return -1;
            }
            /* printf("Skipping to next test..."); */
            r = -1;
        }
    }
    return r;
}

int test_stuck_address(ulv *bufa, size_t count) {
    ulv *p1 = bufa;
    unsigned int j;
    size_t i;
    off_t physaddr;

    //printf("           ");
    //fflush(stdout);
    for (j = 0; j < 16; j++) {
        //printf("\b\b\b\b\b\b\b\b\b\b\b");
        p1 = (ulv *) bufa;
        //printf("setting %3u", j);
        //fflush(stdout);
        for (i = 0; i < count; i++) {
            *p1 = ((j + i) % 2) == 0 ? (ul) p1 : ~((ul) p1);
            *p1++;
        }
        //printf("\b\b\b\b\b\b\b\b\b\b\b");
        //printf("testing %3u", j);
        //fflush(stdout);
        p1 = (ulv *) bufa;
        for (i = 0; i < count; i++, p1++) {
            if (*p1 != (((j + i) % 2) == 0 ? (ul) p1 : ~((ul) p1))) {
                if (use_phys) {
                    physaddr = physaddrbase + (i * sizeof(ul));
                    fprintf(stderr, 
                            "FAILURE: possible bad address line at physical "
                            "address 0x%08lx.\n", 
                            physaddr);
                } else {
                    fprintf(stderr, 
                            "FAILURE: possible bad address line at offset "
                            "0x%08lx.\n", 
                            (ul) (i * sizeof(ul)));
                }
                printf("Skipping to next test...\n");
                fflush(stdout);
                return -1;
            }
        }
    }
    //printf("\b\b\b\b\b\b\b\b\b\b\b           \b\b\b\b\b\b\b\b\b\b\b");
    //fflush(stdout);
    return 0;
}

int test_random_value(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    ul j = 0;
    size_t i;

    //putchar(' ');
    //fflush(stdout);
    for (i = 0; i < count; i++) {
        *p1++ = *p2++ = rand_ul();
        if (!(i % PROGRESSOFTEN)) {
            //putchar('\b');
            //putchar(progress[++j % PROGRESSLEN]);
            //fflush(stdout);
        }
    }
    //printf("\b \b");
    //fflush(stdout);
    return compare_regions(bufa, bufb, count);
}

int test_xor_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    size_t i;
    ul q = rand_ul();

    for (i = 0; i < count; i++) {
        *p1++ ^= q;
        *p2++ ^= q;
    }
    return compare_regions(bufa, bufb, count);
}

int test_sub_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    size_t i;
    ul q = rand_ul();

    for (i = 0; i < count; i++) {
        *p1++ -= q;
        *p2++ -= q;
    }
    return compare_regions(bufa, bufb, count);
}

int test_mul_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    size_t i;
    ul q = rand_ul();

    for (i = 0; i < count; i++) {
        *p1++ *= q;
        *p2++ *= q;
    }
    return compare_regions(bufa, bufb, count);
}

int test_div_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    size_t i;
    ul q = rand_ul();

    for (i = 0; i < count; i++) {
        if (!q) {
            q++;
        }
        *p1++ /= q;
        *p2++ /= q;
    }
    return compare_regions(bufa, bufb, count);
}

int test_or_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    size_t i;
    ul q = rand_ul();

    for (i = 0; i < count; i++) {
        *p1++ |= q;
        *p2++ |= q;
    }
    return compare_regions(bufa, bufb, count);
}

int test_and_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    size_t i;
    ul q = rand_ul();

    for (i = 0; i < count; i++) {
        *p1++ &= q;
        *p2++ &= q;
    }
    return compare_regions(bufa, bufb, count);
}

int test_seqinc_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    size_t i;
    ul q = rand_ul();

    for (i = 0; i < count; i++) {
        *p1++ = *p2++ = (i + q);
    }
    return compare_regions(bufa, bufb, count);
}

unsigned long fixed_pattern[32] = {
				0x0f0f0f0f0f0f0f0f,
				0xf0f0f0f0f0f0f0f0,
				0x00000000ffffffff,
				0xffffffff00000000,
				0x00000000000000ff,
				0x000000000000ff00,
				0x0000000000ff0000,
				0x00000000ff000000,
				0xffffffffffffff00,
				0xffffffffffff00ff,
				0xffffffffff00ffff,
				0xffffffff00ffffff,
				0x0123456789abcdef,
				0xfedcba9876543210,
				0xfff000ff000fff00,
				0x000fff00fff000ff,
				0x1111222233334444,
				0x5555666677778888,
				0x9999aaaabbbbcccc,
				0xddddeeee11223344,
				0xFFFF00000000FFFF,
				0x0000FFFFFFFF0000,
				0xf0f00f0ff0f00f0f,
				0x0f0ff0f00f0ff0f0,
				0x0000000000000000,
				0xFFFFFFFFFFFFFFFF,
				0xFFFFFFFF88888888,
				0x88888888FFFFFFFF,
				0x5a5a5a5a5a5a5a5a,
				0x55555555aaaaaaaa,
				0x5a5aa5a55a5aa5a5,
				0xa5a55a5aa5a55a5a};

int test_fixed_pattern_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    unsigned int j;
    ul q;
    size_t i;

    //printf("           ");
    //fflush(stdout);
    for (j = 0; j < 32; j++) {
        //printf("\b\b\b\b\b\b\b\b\b\b\b");
        q = fixed_pattern[j];
        //printf("setting %3u", j);
        //fflush(stdout);
        p1 = (ulv *) bufa;
        p2 = (ulv *) bufb;
        for (i = 0; i < count; i++) {
            *p1++ = *p2++ = q;
        }
        //printf("\b\b\b\b\b\b\b\b\b\b\b");
        //printf("testing %3u", j);
        //fflush(stdout);
        if (compare_regions_with_expected(bufa, bufb, count, q)) {
            return -1;
        }
    }
    //printf("\b\b\b\b\b\b\b\b\b\b\b           \b\b\b\b\b\b\b\b\b\b\b");
    //fflush(stdout);
    return 0;
}

int test_solidbits_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    unsigned int j;
    ul q;
    size_t i;

    //printf("           ");
    //fflush(stdout);
    for (j = 0; j < 64; j++) {
        //printf("\b\b\b\b\b\b\b\b\b\b\b");
        q = (j % 2) == 0 ? UL_ONEBITS : 0;
        //printf("setting %3u", j);
        //fflush(stdout);
        p1 = (ulv *) bufa;
        p2 = (ulv *) bufb;
        for (i = 0; i < count; i++) {
            *p1++ = *p2++ = (i % 2) == 0 ? q : ~q;
        }
        //printf("\b\b\b\b\b\b\b\b\b\b\b");
        //printf("testing %3u", j);
        //fflush(stdout);
        if (compare_regions(bufa, bufb, count)) {
            return -1;
        }
    }
    //printf("\b\b\b\b\b\b\b\b\b\b\b           \b\b\b\b\b\b\b\b\b\b\b");
    //fflush(stdout);
    return 0;
}

int test_checkerboard_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    unsigned int j;
    ul q;
    size_t i;

    //printf("           ");
    //fflush(stdout);
    for (j = 0; j < 64; j++) {
        //printf("\b\b\b\b\b\b\b\b\b\b\b");
        q = (j % 2) == 0 ? CHECKERBOARD1 : CHECKERBOARD2;
        //printf("setting %3u", j);
        //fflush(stdout);
        p1 = (ulv *) bufa;
        p2 = (ulv *) bufb;
        for (i = 0; i < count; i++) {
            *p1++ = *p2++ = (i % 2) == 0 ? q : ~q;
        }
        //printf("\b\b\b\b\b\b\b\b\b\b\b");
        //printf("testing %3u", j);
        //fflush(stdout);
        if (compare_regions(bufa, bufb, count)) {
            return -1;
        }
    }
    //printf("\b\b\b\b\b\b\b\b\b\b\b           \b\b\b\b\b\b\b\b\b\b\b");
    //fflush(stdout);
    return 0;
}

int test_blockseq_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    unsigned int j;
    size_t i;

    //printf("           ");
    //fflush(stdout);
    for (j = 0; j < 256; j++) {
        //printf("\b\b\b\b\b\b\b\b\b\b\b");
        p1 = (ulv *) bufa;
        p2 = (ulv *) bufb;
        //printf("setting %3u", j);
        //fflush(stdout);
        for (i = 0; i < count; i++) {
            *p1++ = *p2++ = (ul) UL_BYTE(j);
        }
        //printf("\b\b\b\b\b\b\b\b\b\b\b");
        //printf("testing %3u", j);
        //fflush(stdout);
        if (compare_regions_with_expected(bufa, bufb, count, (ul) UL_BYTE(j))) {
            return -1;
        }
    }
    //printf("\b\b\b\b\b\b\b\b\b\b\b           \b\b\b\b\b\b\b\b\b\b\b");
    //fflush(stdout);
    return 0;
}

int test_walkbits0_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    unsigned int j;
    size_t i;

    //printf("           ");
    //fflush(stdout);
    for (j = 0; j < UL_LEN * 2; j++) {
        //printf("\b\b\b\b\b\b\b\b\b\b\b");
        p1 = (ulv *) bufa;
        p2 = (ulv *) bufb;
        //printf("setting %3u", j);
        //fflush(stdout);
        for (i = 0; i < count; i++) {
            if (j < UL_LEN) { /* Walk it up. */
                *p1++ = *p2++ = ONE << j;
            } else { /* Walk it back down. */
                *p1++ = *p2++ = ONE << (UL_LEN * 2 - j - 1);
            }
        }
        //printf("\b\b\b\b\b\b\b\b\b\b\b");
        //printf("testing %3u", j);
        //fflush(stdout);
        if (compare_regions(bufa, bufb, count)) {
            return -1;
        }
    }
    //printf("\b\b\b\b\b\b\b\b\b\b\b           \b\b\b\b\b\b\b\b\b\b\b");
    //fflush(stdout);
    return 0;
}

int test_walkbits1_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    unsigned int j;
    size_t i;

    //printf("           ");
    //fflush(stdout);
    for (j = 0; j < UL_LEN * 2; j++) {
        //printf("\b\b\b\b\b\b\b\b\b\b\b");
        p1 = (ulv *) bufa;
        p2 = (ulv *) bufb;
        //printf("setting %3u", j);
        //fflush(stdout);
        for (i = 0; i < count; i++) {
            if (j < UL_LEN) { /* Walk it up. */
                *p1++ = *p2++ = UL_ONEBITS ^ (ONE << j);
            } else { /* Walk it back down. */
                *p1++ = *p2++ = UL_ONEBITS ^ (ONE << (UL_LEN * 2 - j - 1));
            }
        }
        //printf("\b\b\b\b\b\b\b\b\b\b\b");
        //printf("testing %3u", j);
        //fflush(stdout);
        if (compare_regions(bufa, bufb, count)) {
            return -1;
        }
    }
    //printf("\b\b\b\b\b\b\b\b\b\b\b           \b\b\b\b\b\b\b\b\b\b\b");
    //fflush(stdout);
    return 0;
}

int test_bitspread_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    unsigned int j;
    size_t i;

    //printf("           ");
    //fflush(stdout);
    for (j = 0; j < UL_LEN * 2; j++) {
        //printf("\b\b\b\b\b\b\b\b\b\b\b");
        p1 = (ulv *) bufa;
        p2 = (ulv *) bufb;
        //printf("setting %3u", j);
        //fflush(stdout);
        for (i = 0; i < count; i++) {
            if (j < UL_LEN) { /* Walk it up. */
                *p1++ = *p2++ = (i % 2 == 0)
                    ? (ONE << j) | (ONE << (j + 2))
                    : UL_ONEBITS ^ ((ONE << j)
                                    | (ONE << (j + 2)));
            } else { /* Walk it back down. */
                *p1++ = *p2++ = (i % 2 == 0)
                    ? (ONE << (UL_LEN * 2 - 1 - j)) | (ONE << (UL_LEN * 2 + 1 - j))
                    : UL_ONEBITS ^ (ONE << (UL_LEN * 2 - 1 - j)
                                    | (ONE << (UL_LEN * 2 + 1 - j)));
            }
        }
        //printf("\b\b\b\b\b\b\b\b\b\b\b");
        //printf("testing %3u", j);
        //fflush(stdout);
        if (compare_regions(bufa, bufb, count)) {
            return -1;
        }
    }
    //printf("\b\b\b\b\b\b\b\b\b\b\b           \b\b\b\b\b\b\b\b\b\b\b");
    //fflush(stdout);
    return 0;
}

int test_bitflip_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    unsigned int j, k;
    ul q;
    size_t i;

    //printf("           ");
    //fflush(stdout);
    for (k = 0; k < UL_LEN; k++) {
        q = ONE << k;
        for (j = 0; j < 8; j++) {
            //printf("\b\b\b\b\b\b\b\b\b\b\b");
            q = ~q;
            //printf("setting %3u", k * 8 + j);
            //fflush(stdout);
            p1 = (ulv *) bufa;
            p2 = (ulv *) bufb;
            for (i = 0; i < count; i++) {
                *p1++ = *p2++ = (i % 2) == 0 ? q : ~q;
            }
            //printf("\b\b\b\b\b\b\b\b\b\b\b");
            //printf("testing %3u", k * 8 + j);
            //fflush(stdout);
            if (compare_regions(bufa, bufb, count)) {
                return -1;
            }
        }
    }
    //printf("\b\b\b\b\b\b\b\b\b\b\b           \b\b\b\b\b\b\b\b\b\b\b");
    //fflush(stdout);
    return 0;
}

#ifdef TEST_NARROW_WRITES    
int test_8bit_wide_random(ulv* bufa, ulv* bufb, size_t count) {
    u8v *p1, *t;
    ulv *p2;
    int attempt;
    unsigned int b, j = 0;
    size_t i;

    //putchar(' ');
    fflush(stdout);
    for (attempt = 0; attempt < 2;  attempt++) {
        if (attempt & 1) {
            p1 = (u8v *) bufa;
            p2 = bufb;
        } else {
            p1 = (u8v *) bufb;
            p2 = bufa;
        }
        for (i = 0; i < count; i++) {
            t = mword8.bytes;
            *p2++ = mword8.val = rand_ul();
            for (b=0; b < UL_LEN/8; b++) {
                *p1++ = *t++;
            }
            if (!(i % PROGRESSOFTEN)) {
                //putchar('\b');
                //putchar(progress[++j % PROGRESSLEN]);
                fflush(stdout);
            }
        }
        if (compare_regions(bufa, bufb, count)) {
            return -1;
        }
    }
    //printf("\b \b");
    //fflush(stdout);
    return 0;
}

int test_16bit_wide_random(ulv* bufa, ulv* bufb, size_t count) {
    u16v *p1, *t;
    ulv *p2;
    int attempt;
    unsigned int b, j = 0;
    size_t i;

    //putchar( ' ' );
    fflush( stdout );
    for (attempt = 0; attempt < 2; attempt++) {
        if (attempt & 1) {
            p1 = (u16v *) bufa;
            p2 = bufb;
        } else {
            p1 = (u16v *) bufb;
            p2 = bufa;
        }
        for (i = 0; i < count; i++) {
            t = mword16.u16s;
            *p2++ = mword16.val = rand_ul();
            for (b = 0; b < UL_LEN/16; b++) {
                *p1++ = *t++;
            }
            if (!(i % PROGRESSOFTEN)) {
                //putchar('\b');
                //putchar(progress[++j % PROGRESSLEN]);
                fflush(stdout);
            }
        }
        if (compare_regions(bufa, bufb, count)) {
            return -1;
        }
    }
    //printf("\b \b");
    //fflush(stdout);
    return 0;
}
#endif
