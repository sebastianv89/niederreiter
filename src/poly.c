#include <stdlib.h>
#include "poly.h"
#include "randombytes.h"

/*
 * The polynomial operations in this file are in the ring
 * GF(2)[x]/(x^r-1), where r = POLY_BITS.  From here on, we refer to
 * this modulus as the polynomial G.
 *
 * Polynomial representation: The lsb of each word holds the
 * coefficient of the lowest degree. Words with a higher index hold
 * represent higher degrees. For example, the two 8-bit word f with
 * f[0] = 0b00010011; f[1] = 0b00100000 represents the polynomial 1 +
 * x + x^4 + x^13.
 * 
 * There's quite some room for improvements in assembly, unless the
 * compiler is already very clever. Having access to the carry flag,
 * could speed up multiplications and divisions by x. Being able to
 * have multiple of these carry chains in parallel could speed up
 * the multiplication.
 */


/* Generate a sparse polynomial
 * Non-constant time, but it only leaks how many random numbers collided.
 */
void poly_gen_sparse(
    index_t *f, size_t weight,
    index_t max_index,
    index_t index_mask)
{
	/* TODO: compute how small the candidate buffer could be */
	index_t cand[2 * weight];
	size_t bits_set, i, j;
	int collision;

	do {
        /* TODO: PRNG (now it's just /dev/urandom) */
		randombytes((unsigned char *)cand, 2 * weight * POLY_INDEX_BYTES);

		bits_set = 0;
		for (i = 0; i < 2 * weight * POLY_INDEX_BYTES; ++i) {
			cand[i] &= index_mask;
			if (cand[i] > max_index) {
				continue;
			}

			/* TODO: improve collision detection (sorting?) */
			collision = 0;
			for (j = 0; j < i; ++j) {
				if (cand[i] == f[j]) {
					collision = 1;
					break;
				}
			}
			if (collision) {
				continue;
			}

			f[bits_set++] = cand[i];
			if (bits_set == weight) {
				break;
			}
		}
	} while (bits_set < weight);
}

/* Could be more efficient with a sorted representation. */
void poly_to_dense(
          word_t  *f, size_t words,
    const index_t *g, size_t indices)
{
	size_t i, j;
	size_t word_index[indices];
	word_t bit[indices];
    word_t mask;

	/* split sparse indices into word index and "bit in word" */
	for (i = 0; i < indices; ++i) {
		word_index[i] = g[i] >> WORD_INDEX_BITS;
		bit[i] = WORD_C(1) << (g[i] & POLY_INDEX_MASK);
	}

	for (i = 0; i < words; ++i) {
		f[i] = 0;
		for (j = 0; j < indices; ++j) {
			/* if (word_index[j] == i) f[i] |= bit[j]; */
            mask = word_index[j] ^ i;
            mask = -((mask - 1) >> 63);
            f[i] |= bit[j] & mask;
		}
	}
}

/* f == g
 * 
 * \return a mask (not a boolean): (f == g ? ~0 : 0)
 */
word_t poly_eq(
    const word_t *f,
    const word_t *g, size_t words)
{
    size_t i;
    word_t different_bits = 0;
    for (i = 0; i < words; ++i) {
        different_bits |= f[i] ^ g[i];
    }
    different_bits |= different_bits >> (WORD_BITS / 2);
    different_bits &= (WORD_C(1) << (WORD_BITS / 2)) - 1;
    return -((different_bits - 1) >> (WORD_BITS - 1));
}

/* *eq := f == g; *lt := f < g
 *
 * This function is used in the context of the xgcd.  For equality,
 * the polynomials need to be exactly the same.  To find out which
 * polynomial is greater, only the degree matters.  If the degrees
 * are equal, it does not matter what is returned in \p lt.
 * 
 * TODO: I think this is assuming a little-endian representation.
 */
void poly_compare(word_t *eq, word_t *lt, const word_t *f, const word_t *g)
{
    size_t word = POLY_WORDS - 1, shift;
    unsigned char f_byte, g_byte;

    *lt = 0;
    *eq = 1;
    shift = 8 * TAIL_BYTES;
    while (shift > 0) {
        shift -= 8;
        f_byte = f[word] >> shift;
        g_byte = g[word] >> shift;
        *lt |= *eq & ((f_byte - g_byte) >> 8);
        *eq &= ((f_byte ^ g_byte) - 1) >> 8;
    }
    while (word > 0) {
        --word;
        shift = WORD_BITS;
        while (shift > 0) {
            shift -= 8;
            f_byte = f[word] >> shift;
            g_byte = g[word] >> shift;
            *lt |= *eq & ((f_byte - g_byte) >> 8);
            *eq &= ((f_byte ^ g_byte) - 1) >> 8;
        }
    }
    *eq = -(*eq);
    *lt = -(*lt);
}

/* f == 1, in non-constant time */
int poly_is_one_nonconst(const word_t *f)
{
    size_t i;

    if (f[0] != 1) {
        return 0;
    }
    for (i = 1; i < POLY_WORDS; ++i) {
        if (f[i] != 0) {
            return 0;
        }
    }
    return 1;
}

/* dest := src */
void poly_copy(word_t *dest, const word_t *src, size_t words)
{
    size_t i;
    for (i = 0; i < words; ++i) {
        dest[i] = src[i];
    }
}

/* f := 0 */
void poly_zero(word_t *f, size_t words)
{
    size_t i;
    for (i = 0; i < words; ++i) {
        f[i] = 0;
    }
}

/* f := 1 */
void poly_one(word_t *f, size_t words)
{
    f[0] = 1;
    poly_zero(f + 1, words - 1);
}

/* f := G */
void poly_G(word_t *f)
{
    poly_one(f, POLY_WORDS - 1);
    f[POLY_WORDS - 1] = WORD_C(1) << TAIL_BITS;
}

/* f = g + h */
void poly_add(word_t *f, const word_t *g, const word_t *h, size_t words)
{
    size_t i;
    for (i = 0; i < words; ++i) {
        f[i] = g[i] ^ h[i];
    }
}

/* f *= x */
void poly_mul_x(word_t *f, size_t words)
{
    size_t i;
    word_t carry, tmp;
    
    carry = f[0] >> (WORD_BITS - 1);
    f[0] <<= 1;
    for (i = 1; i < words - 1; ++i) {
        tmp = f[i] >> (WORD_BITS - 1);
        f[i] <<= 1;
        f[i] |= carry;
        carry = tmp;
    }
    f[i] <<= 1;
    f[i] |= carry;
}

/* f *= x mod G */
void poly_inplace_mul_x_modG(word_t *f)
{
    size_t i;
    word_t carry, tmp;
    
    carry = f[POLY_WORDS - 1] >> (TAIL_BITS - 1);
    for (i = 0; i < POLY_WORDS - 1; ++i) {
        tmp = f[i] >> (WORD_BITS - 1);
        f[i] <<= 1;
        f[i] |= carry;
        carry = tmp;
    }
#if TAIL_BITS == 1
    f[i] = carry;
#else
    f[i] <<= 1;
    f[i] |= carry;
    f[i] &= TAIL_MASK;
#endif
}

/* f = x * g mod G */
void poly_mul_x_modG(word_t *f, const word_t *g)
{
    size_t i;

    f[0] = (g[0] << 1) | (g[POLY_WORDS - 1] >> (TAIL_BITS - 1));
    for(i = 1; i < POLY_WORDS - 1; ++i) {
        f[i] = (g[i] << 1) | (g[i - 1] >> (WORD_BITS - 1));
    }
#if TAIL_BITS == 1
    f[i] = g[i - 1] >> (WORD_BITS - 1);
#else
    f[i] = g[i] << 1;
    f[i] |= g[i - 1] >> (WORD_BITS - 1);
    f[i] &= TAIL_MASK;
#endif
}

/* f = g / x */
void poly_div_x(word_t *f, const word_t *g)
{
    size_t i;

    for (i = 0; i < POLY_WORDS - 1; ++i) {
        f[i] = (g[i] >> 1) | (g[i + 1] << (WORD_BITS - 1));
    }
    f[i] = g[i] >> 1;
}

/* f = g / x mod G */
void poly_div_x_modG(word_t *f, const word_t *g)
{
    size_t i;
    word_t carry;

    carry = (g[0] & 1) << (TAIL_BITS - 1);
    for (i = 0; i < POLY_WORDS - 1; ++i) {
        f[i] = (g[i] >> 1) | (g[i + 1] << (WORD_BITS - 1));
    }
#if TAIL_BITS == 1
    f[i] = carry;
#else
    f[i] = g[i] >> 1;
    f[i] |= carry;
#endif
}


/* f := g mod G
 *
 * g is the result of a non-reduced multiplication
 */ 
void poly_reduce(word_t *f, const word_t *g)
{
    size_t i;

    for (i = 0; i < POLY_WORDS - 1; ++i) {
        f[i] = g[i];
        f[i] ^= g[i + POLY_WORDS - 1] >> TAIL_BITS;
        f[i] ^= g[i + POLY_WORDS] << (WORD_BITS - TAIL_BITS);
    }
    f[i] = g[i] & TAIL_MASK;
}

/* This function implements schoolbook multiplication, with reduction
 * mod G postponed until the very end.  There is much room for
 * optimizations, for example, Karatsuba multiplication should achieve
 * a great speedup, as would FFT multiplication.  Using the CLMUL
 * instruction (where available) is another option (but less
 * portable).
 */
void poly_mul(word_t *f, const word_t *g, const word_t *h)
{
    static const size_t LHS_WORDS = TAIL_BITS == 1 ? POLY_WORDS : POLY_WORDS + 1;

    size_t bit_index, i, j;
    word_t lhs[LHS_WORDS];
    word_t res[2 * POLY_WORDS - 1];
    word_t lhs_mask;

    poly_copy(lhs, g, POLY_WORDS);
#if TAIL_BITS > 1
    lhs[POLY_WORDS] = 0;
#endif
    poly_zero(res, 2 * POLY_WORDS - 1);

    /* bit_index == 0 */
    for (i = 0; i < POLY_WORDS; ++i) {
        lhs_mask = -(h[i] & WORD_C(1));
        for (j = 0; j < LHS_WORDS; ++j) {
            res[i + j] ^= lhs[j] & lhs_mask;
        }
    }
    for (bit_index = 1; bit_index < WORD_BITS; ++bit_index) {
        poly_inplace_mul_x_modG(lhs);
        for (i = 0; i < POLY_WORDS; ++i) {
            lhs_mask = -((h[i] >> bit_index) & WORD_C(1));
            for (j = 0; j < LHS_WORDS; ++j) {
                res[i + j] ^= lhs[j] & lhs_mask;
            }
        }
    }
    
    poly_reduce(f, res);
}

/* TODO: this can be optimized */
void poly_sparse_mul(word_t *f, const word_t *g, const index_t *h)
{
    word_t h_dense[POLY_WORDS];
    poly_to_dense(h_dense, POLY_WORDS, h, POLY_SPARSE_WEIGHT); // TODO: hardcoded
    poly_mul(f, g, h_dense);
}


/* Compute xgcd(f, G)
 *
 * Constant time implementation of the binary polynomial XGCD algorithm
 * The starting polynomial for g is constant: namely G.
 * 
 * \param[in]  f  polynomial (POLY_WORDS)
 * \param[in]  a  allocated memory (POLY_WORDS)
 * \param[out] f  gcd(f, G)
 * \param[out] a  parameter a in Bézout identity "gcd(x,y) = ax + by"
 */
void poly_xgcd(word_t *f_, word_t *a_)
{
    // debug line
    word_t f[POLY_WORDS], a[POLY_WORDS]; poly_copy(f, f_, POLY_WORDS); poly_copy(a, a_, POLY_WORDS);

    size_t i, j;
    word_t g[POLY_WORDS], b[POLY_WORDS];
    word_t f_div_x[POLY_WORDS], g_div_x[POLY_WORDS];
    word_t a_div_x[POLY_WORDS], b_div_x[POLY_WORDS];
    word_t fg_div_x[POLY_WORDS], ab_div_x[POLY_WORDS];
    word_t mask_f_odd, mask_g_odd, mask_f_gt_g, mask_f_even;
    word_t mask_g_even, mask_f_lt_g;
    word_t mask_f2f, mask_f2fx, mask_f2fgx;
    word_t mask_g2g, mask_g2gx, mask_g2fgx;
    word_t mask_done, mask_not_done;

    poly_G(g);
    poly_one(a, POLY_WORDS);
    poly_zero(b, POLY_WORDS);

    for (i = 0; i < 2 * POLY_BITS - 1; ++i) {
        poly_div_x(f_div_x, f);
        poly_div_x(g_div_x, g);
        poly_add(fg_div_x, f_div_x, g_div_x, POLY_WORDS);
        poly_div_x_modG(a_div_x, a);
        poly_div_x_modG(b_div_x, b);
        poly_add(ab_div_x, a_div_x, b_div_x, POLY_WORDS);

        mask_f_odd = -(f[0] & WORD_C(1));
        mask_g_odd = -(g[0] & WORD_C(1));
        poly_compare(&mask_done, &mask_f_lt_g, f, g);
        mask_f_even = ~mask_f_odd;
        mask_g_even = ~mask_g_odd;
        mask_f_gt_g = ~mask_f_lt_g;
        mask_not_done = ~mask_done;
        
        mask_f2f   = mask_done     | (mask_f_odd  & (mask_g_even | mask_f_lt_g));
        mask_f2fx  = mask_not_done &  mask_f_even;
        mask_f2fgx = mask_not_done &  mask_f_odd  &  mask_g_odd  & mask_f_gt_g;
        mask_g2g   = mask_done     |  mask_f_even | (mask_g_odd  & mask_f_gt_g);
        mask_g2gx  = mask_not_done &  mask_f_odd  &  mask_g_even;
        mask_g2fgx = mask_not_done &  mask_f_odd  &  mask_g_odd  & mask_f_lt_g;

        /* Alternative. Minimizes bit operations, but it is probably
           faster to operate directly on the first masks to avoid
           dependency chains. 
        mask_f2fx  = ~mask_f_odd;                                // f := f/x
        mask_f2fgx = mask_f_odd & mask_g_odd & mask_f_gt_g;      // f := (f^g)/x
        mask_g2g   = mask_f2fx | mask_f2fgx;                     // g := g
        mask_f2f   = ~mask_g2g;                                  // f := f
        mask_g2gx  = mask_f2f & mask_g_odd;                      // g := g/x
        mask_g2fgx = ~(mask_g2g | mask_g2gx);                    // g := (f^g)/x
        */

        /* Compute:
         *      if (even(f)) { f = f_div_x;  a = a_div_x; }
         * else if (even(g)) { g = g_div_x;  b = b_div_x; }
         * else if (f > g)   { f = fg_div_x; a = ab_div_x; }
         * else              { g = fg_div_x; b = ab_div_x; }
         */
        for (j = 0; j < POLY_WORDS; ++j) { /* one loop per poly might be faster */
            f[j] = (f[j] & mask_f2f) | (f_div_x[j] & mask_f2fx) | (fg_div_x[j] & mask_f2fgx);
            a[j] = (a[j] & mask_f2f) | (a_div_x[j] & mask_f2fx) | (ab_div_x[j] & mask_f2fgx);
            g[j] = (g[j] & mask_g2g) | (g_div_x[j] & mask_g2gx) | (fg_div_x[j] & mask_g2fgx);
            b[j] = (b[j] & mask_g2g) | (b_div_x[j] & mask_g2gx) | (ab_div_x[j] & mask_g2fgx);
        }
    }
    // debug line
    poly_copy(f_, f, POLY_WORDS); poly_copy(a_, a, POLY_WORDS);
}

int poly_inv(word_t *f)
{
    word_t inv[POLY_WORDS];
    poly_xgcd(f, inv);
    if (poly_is_one_nonconst(f)) {
        poly_copy(f, inv, POLY_WORDS);
        return 0;
    }
    return -1;
}


#if defined(POLY_MAIN)

#include <stdio.h>
#include "randombytes.h"

void poly_rand(word_t *f)
{
    randombytes((unsigned char *)f, BLOCK_BYTES);
    f[POLY_WORDS - 1] &= TAIL_MASK;
}

int main(void)
{
    word_t f[POLY_WORDS], g[POLY_WORDS], h[POLY_WORDS], inv[POLY_WORDS];
    index_t g_sparse[45];

    printf("TAIL_BITS == %u\n", TAIL_BITS);
    
    do {
        poly_gen_sparse(g_sparse, BLOCK_WEIGHT, POLY_BITS - 1, POLY_INDEX_MASK);
        poly_to_dense(g, POLY_WORDS, g_sparse, POLY_WEIGHT);
        poly_copy(h, g, POLY_WORDS);
        poly_xgcd(g, inv);
    } while (!poly_is_one_nonconst(g));
    poly_mul(f, h, inv);

    return 0;
}

#endif /* POLY_MAIN */
