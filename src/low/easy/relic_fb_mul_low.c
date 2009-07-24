/*
 * Copyright 2007 Project RELIC
 *
 * This file is part of RELIC. RELIC is legal property of its developers,
 * whose names are not listed here. Please refer to the COPYRIGHT file.
 *
 * RELIC is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * RELIC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with RELIC. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file
 *
 * Implementation of the low-level binary field bit shifting functions.
 *
 * @version $Id$
 * @ingroup fb
 */

#include <stdlib.h>

#include "relic_fb.h"
#include "relic_fb_low.h"
#include "relic_bn_low.h"
#include "relic_util.h"

/*============================================================================*/
/* Public definitions                                                         */
/*============================================================================*/

void fb_mul1_low(dig_t *c, dig_t *a, dig_t digit) {
	dig_t carry;

	fb_zero(c);
	fb_zero(c + FB_DIGS);
	for (int i = FB_DIGIT - 1; i >= 0; i--) {
		if (digit & ((dig_t)1 << i)) {
			fb_addn_low(c, c, a);
		}
		if (i != 0) {
			carry = fb_lsh1_low(c, c);
			c[FB_DIGS] = (c[FB_DIGS] << 1) | carry;
		}
	}
}

void fb_muln_low(dig_t *c, dig_t *a, dig_t *b) {
	dig_t table[16][FB_DIGS + 1] = { { 0 } };
	dig_t r0, r1, r2, r4, r8, u, carry, *tmpa, *tmpc;
	int i, j;

	for (i = 0; i < 2 * FB_DIGS; i++) {
		c[i] = 0;
	}

	u = 0;
	for (i = 0; i < FB_DIGS; i++) {
		r1 = r0 = b[i];
		r2 = (r0 << 1) | (u >> (FB_DIGIT - 1));
		r4 = (r0 << 2) | (u >> (FB_DIGIT - 2));
		r8 = (r0 << 3) | (u >> (FB_DIGIT - 3));
		table[0][i] = 0;
		table[1][i] = r1;
		table[2][i] = r2;
		table[3][i] = r1 ^ r2;
		table[4][i] = r4;
		table[5][i] = r1 ^ r4;
		table[6][i] = r2 ^ r4;
		table[7][i] = r1 ^ r2 ^ r4;
		table[8][i] = r8;
		table[9][i] = r1 ^ r8;
		table[10][i] = r2 ^ r8;
		table[11][i] = r1 ^ r2 ^ r8;
		table[12][i] = r4 ^ r8;
		table[13][i] = r1 ^ r4 ^ r8;
		table[14][i] = r2 ^ r4 ^ r8;
		table[15][i] = r1 ^ r2 ^ r4 ^ r8;
		u = r1;
	}

	if (u > 0) {
		r1 = 0;
		r2 = u >> (FB_DIGIT - 1);
		r4 = u >> (FB_DIGIT - 2);
		r8 = u >> (FB_DIGIT - 3);
		table[0][FB_DIGS] = table[1][FB_DIGS] = 0;
		table[2][FB_DIGS] = table[3][FB_DIGS] = r2;
		table[4][FB_DIGS] = table[5][FB_DIGS] = r4;
		table[6][FB_DIGS] = table[7][FB_DIGS] = r2 ^ r4;
		table[8][FB_DIGS] = table[9][FB_DIGS] = r8;
		table[10][FB_DIGS] = table[11][FB_DIGS] = r2 ^ r8;
		table[12][FB_DIGS] = table[13][FB_DIGS] = r4 ^ r8;
		table[14][FB_DIGS] = table[15][FB_DIGS] = r2 ^ r4 ^ r8;
	}

	for (i = FB_DIGIT - 4; i >= 4; i -= 4) {
		tmpa = a;
		tmpc = c;
		for (j = 0; j < FB_DIGS; j++, tmpa++, tmpc++) {
			u = (*tmpa >> i) & 0x0F;
			fb_addn_low(tmpc, tmpc, table[u]);
			*(tmpc + FB_DIGS) ^= table[u][FB_DIGS];
		}
		carry = fb_lshb_low(c, c, 4);
		fb_lshb_low(c + FB_DIGS, c + FB_DIGS, 4);
		c[FB_DIGS] ^= carry;
	}
	for (j = 0; j < FB_DIGS; j++, a++, c++) {
		u = *a & 0x0F;
		fb_addn_low(c, c, table[u]);
		*(c + FB_DIGS) ^= table[u][FB_DIGS];
	}
}

void fb_mulm_low(dig_t *c, dig_t *t, dig_t *a, dig_t *b) {
	fb_muln_low(t, a, b);
	fb_rdc(c, t);
}
