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
 * Implementation of the multiple precision integer modular reduction
 * functions.
 *
 * @version $Id: relic_bn_mod.c 22 2009-05-03 06:48:50Z dfaranha $
 * @ingroup bn
 */

#include <string.h>

#include "relic_core.h"
#include "relic_bn.h"
#include "relic_bn_low.h"
#include "relic_error.h"
#include "relic_util.h"

/*============================================================================*/
/* Public definitions                                                         */
/*============================================================================*/

void bn_mod_2b(bn_t c, bn_t a, int b) {
	int i, first, d;

	if (b <= 0) {
		bn_zero(c);
		return;
	}

	if (b >= (int)(a->used * BN_DIGIT)) {
		bn_copy(c, a);
		return;
	}

	bn_copy(c, a);

	SPLIT(b, d, b, BN_DIG_LOG);

	first = (d) + (b == 0 ? 0 : 1);
	for (i = first; i < c->used; i++)
		c->dp[i] = 0;

	c->dp[d] &= MASK(b);

	bn_trim(c);
}

void bn_mod_dig(dig_t *c, bn_t a, dig_t b) {
	bn_div_dig(NULL, c, a, b);
}

void bn_mod_basic(bn_t c, bn_t a, bn_t m) {
	bn_div_basic(NULL, c, a, m);
}

#if BN_MOD == BARRT || !defined(STRIP)

void bn_mod_barrt_setup(bn_t u, bn_t m) {
	bn_set_2b(u, m->used * 2 * BN_DIGIT);
	bn_div_norem(u, u, m);
}

void bn_mod_barrt(bn_t c, bn_t a, bn_t m, bn_t u) {
	unsigned long mu;
	bn_t q = NULL, t = NULL;

	if (bn_cmp(a, m) == CMP_LT) {
		bn_copy(c, a);
		return;
	}
	TRY {
		bn_new(q);
		bn_new(t);
		bn_zero(t);

		mu = m->used;

		bn_rsh(q, a, (mu - 1) * BN_DIGIT);

		if (mu > ((dig_t)1) << (BN_DIGIT - 1)) {
			bn_mul(t, q, u);
		} else {
			if (q->used > u->used) {
				bn_muld_low(t->dp, q->dp, q->used, u->dp, u->used,
						mu, q->used + u->used);
			} else {
				bn_muld_low(t->dp, u->dp, u->used, q->dp, q->used,
						mu - (u->used - q->used), q->used + u->used);
			}
			t->used = q->used + u->used;
			bn_trim(t);
		}

		bn_rsh(q, t, (mu + 1) * BN_DIGIT);

		if (q->used > m->used) {
			bn_muld_low(t->dp, q->dp, q->used, m->dp, m->used, 0, q->used + 1);
		} else {
			bn_muld_low(t->dp, m->dp, m->used, q->dp, q->used, 0, mu + 1);
		}
		t->used = mu + 1;
		bn_trim(t);

		bn_mod_2b(q, t, BN_DIGIT * (mu + 1));
		bn_mod_2b(t, a, BN_DIGIT * (mu + 1));
		bn_sub(t, t, q);

		if (bn_sign(t) == BN_NEG) {
			bn_set_dig(q, (dig_t)1);
			bn_lsh(q, q, (mu + 1) * BN_DIGIT);
			bn_add(t, t, q);
		}

		while (bn_cmp(t, m) != CMP_LT) {
			bn_sub(t, t, m);
		}

		bn_copy(c, t);
	}
	CATCH_ANY {
		THROW(ERR_CAUGHT);
	}
	FINALLY {
		bn_free(q);
		bn_free(t);

	}
}

#endif /** BN_MOD == BARRT || !defined(STRIP) */

#if BN_MOD == MONTY || !defined(STRIP)

void bn_mod_monty_setup(bn_t u, bn_t m) {
	dig_t x, b;
	b = m->dp[0];

	if ((b & 0x01) == 0) {
		THROW(ERR_INVALID);
	}

	x = (((b + 2) & 4) << 1) + b;	/* here x*a==1 mod 2**4 */
	x *= 2 - b * x;				/* here x*a==1 mod 2**8 */
#if WORD != 8
	x *= 2 - b * x;				/* here x*a==1 mod 2**16 */
#endif
#if WORD == 64 || WORD != 8 || WORD == 16
	x *= 2 - b * x;				/* here x*a==1 mod 2**32 */
#endif
#if WORD == 64
	x *= 2 - b * x;				/* here x*a==1 mod 2**64 */
#endif
	/* u = -1/m0 (mod 2^BN_DIGIT) */
	bn_set_dig(u, -x);
}

void bn_mod_monty_conv(bn_t c, bn_t a, bn_t m) {
	bn_lsh(c, a, m->used * BN_DIGIT);
	bn_mod_basic(c, c, m);
}

void bn_mod_monty_back(bn_t c, bn_t a, bn_t m) {
	bn_t u = NULL;

	bn_new(u);

	bn_mod_monty_setup(u, m);
	bn_mod_monty(c, a, m, u);

	bn_free(u);
}

#if BN_MUL == BASIC || !defined(STRIP)

void bn_mod_monty_basic(bn_t c, bn_t a, bn_t m, bn_t u) {
	int digits, i;
	dig_t r, carry, u0, *tmp;
	bn_t t = NULL;

	digits = 2 * m->used + 1;
	bn_new_size(t, digits);
	bn_zero(t);
	bn_copy(t, a);
	t->used = digits;

	u0 = u->dp[0];
	tmp = t->dp;

	for (i = 0; i < m->used; i++, tmp++) {
		r = (dig_t)(*tmp * u0);
		carry = bn_muladd_low(tmp, m->dp, r, m->used);
		bn_add1_low(tmp + m->used, tmp + m->used, carry, m->used - i + 1);
	}
	bn_rsh(t, t, m->used * BN_DIGIT);
	bn_trim(t);

	if (bn_cmp_abs(t, m) != CMP_LT) {
		bn_sub(t, t, m);
	}

	bn_copy(c, t);

	bn_free(t);
}

#endif /* BN_MUL == BASIC || !defined(STRIP) */

#if BN_MUL == COMBA || !defined(STRIP)

void bn_mod_monty_comba(bn_t c, bn_t a, bn_t m, bn_t u) {
	int digits;
	bn_t t = NULL;

	digits = 2 * m->used + 1;
	bn_new_size(t, digits);
	bn_zero(t);

	bn_modn_low(t->dp, a->dp, a->used, m->dp, m->used, u->dp[0]);
	t->used = m->used + 1;

	bn_trim(t);
	if (bn_cmp_abs(t, m) != CMP_LT) {
		bn_sub(t, t, m);
	}

	bn_copy(c, t);

	bn_free(t);
}

#endif /* BN_MUL == COMBA || !defined(STRIP) */

#endif /* BN_MOD == MONTY || !defined(STRIP) */

#if BN_MOD == RADIX || !defined(STRIP)

void bn_mod_radix_setup(bn_t u, bn_t m) {
	bn_t t = NULL;
	int bits;

	if (bn_mod_radix_check(m) != 1) {
		THROW(ERR_INVALID);
	}

	bits = bn_bits(m);

	bn_new(t);

	bn_set_2b(t, bits);
	bn_sub(t, t, m);

	bn_set_dig(u, t->dp[0]);

	bn_free(t);
}

int bn_mod_radix_check(bn_t m) {
	int i, bits, d;
	dig_t mask;

	if (m->used == 0) {
		return 0;
	} else {
		if (m->used == 1) {
			return 1;
		} else {
			bits = bn_bits(m);
			mask = DMASK;

			for (i = 1; i < m->used - 1; i++) {
				if ((m->dp[i] & mask) != mask) {
					return 0;
				}
			}
			SPLIT(bits, d, bits, BN_DIG_LOG);
			if (bits != 0) {
				mask = MASK(bits);
			}
			return (m->dp[m->used - 1] & mask) == mask;
		}
	}
}

void bn_mod_radix(bn_t c, bn_t a, bn_t m, bn_t u) {
	bn_t q = NULL, t = NULL;
	int bits, done;

	bn_new(q);
	bn_new(t);

	bn_copy(t, a);

	bits = bn_bits(m);

	do {
		done = 1;

		bn_rsh(q, t, bits);
		bn_mod_2b(t, t, bits);

		if (u->dp[0] != 1) {
			bn_mul_dig(q, q, u->dp[0]);
		}
		bn_add(t, t, q);

		if (bn_cmp_abs(t, m) != CMP_LT) {
			bn_sub(t, t, m);
			done = 0;
		}
	} while (done == 0);

	bn_copy(c, t);
	bn_free(t);
	bn_free(q);
}

#endif /* BN_MOD == RADIX || !defined(STRIP) */
