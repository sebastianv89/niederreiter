#include "poly.h"
#include "config.h"
#include "randombytes.h"

/* 
 * There's quite some room for improvements in assembly, unless the
 * compiler is already very clever. Having access to the carry flag
 * could speed up multiplications and divisions by x. Being able to
 * have multiple of these carry chains in parallel could speed up
 * the multiplication.
 */

/* dest := src */
void poly_copy(word_t *dest, const word_t *src, size_t words)
{
    size_t i;
    for (i = 0; i < words; ++i) {
        dest[i] = src[i];
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

/* *eq := f == g; *lt := f < g
 *
 * This function is used in the context of the xgcd.  For equality,
 * the polynomials need to be exactly the same.  To find out which
 * polynomial is greater, only the degree matters.  If the degrees
 * are equal, it does not matter what is returned in \p lt.
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
        f_byte = (unsigned char)(f[word] >> shift);
        g_byte = (unsigned char)(g[word] >> shift);
        *lt |= *eq & ((f_byte - g_byte) >> 8);
        *eq &= ((f_byte ^ g_byte) - 1) >> 8;
    }
    while (word > 0) {
        --word;
        shift = WORD_BITS;
        while (shift > 0) {
            shift -= 8;
            f_byte = (unsigned char)(f[word] >> shift);
            g_byte = (unsigned char)(g[word] >> shift);
            *lt |= *eq & ((f_byte - g_byte) >> 8);
            *eq &= ((f_byte ^ g_byte) - 1) >> 8;
        }
    }
    *eq = -(*eq);
    *lt = -(*lt);
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
void poly_xgcd(word_t *f, word_t *a)
{
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
}


void poly_zero(word_t *f, size_t words)
{
    size_t i;
    for (i = 0; i < words; ++i) {
        f[i] = 0;
    }
}

void poly_gen_sparse(index_t *f, enum sparse_type type)
{
    index_t index_mask;
    size_t weight, max_index;
	size_t bits_set, i, j;
    index_t next, tmp, mask;
    index_t cand[RAND_BUFFER_SIZE];

    if (type == TYPE_POLY) {
        weight = POLY_WEIGHT;
        max_index = POLY_BITS;
        index_mask = (INDEX_C(1) << POLY_INDEX_BITS) - 1;
    } else {
        weight = ERROR_WEIGHT;
        max_index = ERROR_BITS;
        index_mask = (INDEX_C(1) << ERROR_INDEX_BITS) - 1;
    }

	do {
        /* TODO: PRNG (now it's just /dev/urandom) */
		randombytes((unsigned char *)cand, RAND_BUFFER_SIZE * INDEX_BYTES);

		bits_set = 0;
		for (i = 0; i < RAND_BUFFER_SIZE; ++i) {
			cand[i] &= index_mask;
			if (cand[i] > max_index) {
				continue;
			}

            /* constant time insertion sort */
            next = cand[i];
			for (j = 0; j < bits_set; ++j) {
				if (f[j] == cand[i]) {
					break;
				}
                mask = -(((index_t)(f[j] - next)) >> (INDEX_BITS - 1));
                tmp  = (f[j] &  mask) | (next & ~mask);
                next = (f[j] & ~mask) | (next &  mask);
                f[j] = tmp;
			}
            if (j != bits_set) {
                continue;
            }
            f[j] = next;

            ++bits_set;
			if (bits_set == weight) {
				break;
			}
		}
	} while (bits_set < weight);
}

void poly_to_dense(word_t *f, const index_t *g)
{
	size_t i, j;
	size_t word_index[POLY_WEIGHT];
	word_t bit[POLY_WEIGHT];
    word_t mask;

	/* split sparse indices into word index and "bit in word" */
	for (i = 0; i < POLY_WEIGHT; ++i) {
		word_index[i] = g[i] >> WORD_INDEX_BITS;
		bit[i] = WORD_C(1) << (g[i] & WORD_INDEX_MASK);
	}

	for (i = 0; i < POLY_WORDS; ++i) {
		f[i] = 0;
		for (j = 0; j < POLY_WEIGHT; ++j) {
            mask = word_index[j] ^ i;
            mask = -((mask - 1) >> (WORD_BITS - 1));
            f[i] |= bit[j] & mask;
		}
	}
}

/* Ignores indices of g that are larger than POLY_BITS */
void error_block_to_dense(word_t *f, const index_t *g)
{
    size_t i, j;
    size_t word_index[ERROR_WEIGHT];
    word_t bit[ERROR_WEIGHT];
    word_t mask;

	/* split sparse indices into word index and "bit in word" */
	for (i = 0; i < POLY_WEIGHT; ++i) {
		word_index[i] = g[i] >> WORD_INDEX_BITS;
		bit[i] = WORD_C(1) << (g[i] & WORD_INDEX_MASK);
	}

	for (i = 0; i < POLY_WORDS; ++i) {
		f[i] = 0;
		for (j = 0; j < ERROR_WEIGHT; ++j) {
            /* f[i] |= word_index[j] == i ? bit[j] : 0 */
            mask = word_index[j] ^ i;
            mask = -((mask - 1) >> (WORD_BITS - 1));
            f[i] |= bit[j] & mask;
		}
	}
    f[i] &= TAIL_MASK;
}

/* This function relies on the fact that POLY_BITS < 2^{INDEX_BITS-1}
 * and that ERROR_BITS < 2^{INDEX_BITS-1}.  Namely, the highest bit is
 * used to determine if subtraction resulted in underflow, and that
 * subtraction from -1 does not affect the highest bit.  This
 * assumption might not hold for a different parameter set.
 */
void error_to_dense(word_t *f, index_t *g)
{
    size_t i, j;
    index_t mask;
    poly_to_dense(f, g);
    for (i = 1; i < NUMBER_OF_POLYS; ++i) {
        for (j = 0; j < ERROR_WEIGHT; ++j) {
            /* g[j] = g[j] >= POLY_BITS ? g[j] - POLY_BITS : -1 */
            g[j] -= POLY_BITS;
            mask = -(g[j] >> (INDEX_BITS - 1));
            g[j] |= mask;
        }
        poly_to_dense(f + i * POLY_WORDS, g);
    }
}

/* TODO: if the processor has a built-in popcount, that is probably
 * more efficient */
word_t poly_hamming_weight(const word_t *f)
{
    static const word_t MASK0 = (word_t)(~WORD_C(0)) / 3;
    static const word_t MASK1 = (word_t)(~WORD_C(0)) / 15 * 3;
    static const word_t MASK2 = (word_t)(~WORD_C(0)) / 255 * 15;
    static const word_t MULT  = (word_t)(~WORD_C(0)) / 255;

    word_t total = 0;
    word_t wc;
    size_t i;
    
    for (i = 0; i < POLY_WORDS; ++i) {
        wc  = f[i];
        wc -= (wc >> 1) & MASK0;
        wc  = (wc & MASK1) + ((wc >> 2) & MASK1);
        wc  += wc >> 4;
        wc  &= MASK2;
        wc  *= MULT;
        wc >>= WORD_BITS - 8;
        total += wc;
    }
    return total;
}

void poly_mask(word_t *f, const word_t *g, const word_t *mask)
{
    size_t i;
    for (i = 0; i < POLY_WORDS; ++i) {
        f[i] = g[i] & mask[i];
    }
}

int poly_is_zero(const word_t *f, size_t words)
{
    size_t i, shift;
    unsigned char nonzero_bits = 0;
    for (i = 0; i < words - 1; ++i) {
        nonzero_bits |= f[i];
        for (shift = 8; shift < WORD_BYTES; shift += 8) {
            nonzero_bits |= f[i] >> shift;
        }
    }
    nonzero_bits |= f[i];
    for (shift = 8; shift < TAIL_BYTES; shift += 8) {
        nonzero_bits |= f[i] >> shift;
    }
    return (((nonzero_bits - 1) >> 8) & 1) - 1;
}

void poly_transpose(index_t *f) {
    size_t i;
    index_t mask;
    
    mask = ((index_t)(-f[0]) >> (INDEX_BITS - 1)) - 1;
    f[0] = POLY_BITS - f[0] - (POLY_BITS & mask);
    for (i = 1; i < POLY_WEIGHT; ++i) {
        f[i] = POLY_BITS - f[i];
    }
}

void poly_add_masked(word_t *f, const word_t *g, word_t mask)
{
    size_t i;
    for (i = 0; i < POLY_WORDS; ++i) {
        f[i] ^= g[i] & mask;
    }
}

void poly_add(word_t *f, const word_t *g, const word_t *h, size_t words)
{
    size_t i;
    for (i = 0; i < words; ++i) {
        f[i] = g[i] ^ h[i];
    }
}

void poly_flip(word_t *f, index_t bit_index, word_t bit)
{
    size_t word_index;
    word_t bit_mask;

    word_index = bit_index >> WORD_INDEX_BITS;
    bit_mask = bit << (bit_index & WORD_INDEX_MASK);
    f[word_index] ^= bit_mask;
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
    size_t bit_index, i, j;
    word_t lhs[POLY_WORDS];
    word_t res[2 * POLY_WORDS - 1];
    word_t lhs_mask;

    poly_copy(lhs, g, POLY_WORDS);
    poly_zero(res, 2 * POLY_WORDS - 1);

    /* bit_index == 0 */
    for (i = 0; i < POLY_WORDS; ++i) {
        lhs_mask = -(h[i] & WORD_C(1));
        for (j = 0; j < POLY_WORDS; ++j) {
            res[i + j] ^= lhs[j] & lhs_mask;
        }
    }
    for (bit_index = 1; bit_index < WORD_BITS; ++bit_index) {
        poly_inplace_mul_x_modG(lhs);
        for (i = 0; i < POLY_WORDS; ++i) {
            lhs_mask = -((h[i] >> bit_index) & WORD_C(1));
            for (j = 0; j < POLY_WORDS; ++j) {
                res[i + j] ^= lhs[j] & lhs_mask;
            }
        }
    }
    
    poly_reduce(f, res);
}

/* TODO: can this be optimized(?) */
void poly_sparse_mul(word_t *f, const word_t *g, const index_t *h)
{
    word_t h_dense[POLY_WORDS];
    poly_to_dense(h_dense, h);
    poly_mul(f, g, h_dense);
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

#include "debug.h"
#include "assert.h"

int main(void) {
    word_t f[POLY_WORDS], g[POLY_WORDS], h[POLY_WORDS];
    word_t h_inv[POLY_WORDS];
    index_t f_sparse[POLY_BITS], g_sparse[POLY_WEIGHT], h_sparse[POLY_WEIGHT];
    size_t f_weight;

    poly_gen_sparse(g_sparse, TYPE_POLY);
    poly_gen_sparse(h_sparse, TYPE_POLY);
    poly_to_dense(g, g_sparse);
    poly_to_dense(h, h_sparse);
    poly_mul(f, g, h);
    poly_to_sparse(f_sparse, &f_weight, f);
    
    printf("g: ");
    print_poly_sparse(g_sparse, POLY_WEIGHT);
    printf("\n");
    print_poly_dense(g, POLY_WORDS);
    printf("\nh: ");
    print_poly_sparse(h_sparse, POLY_WEIGHT);
    printf("\n");
    print_poly_dense(h, POLY_WORDS);
    printf("\nf = g*h:\n");
    print_poly_sparse(f_sparse, f_weight);
    printf("\n");
    print_poly_dense(f, POLY_WORDS);
    printf("\n");
    
    poly_copy(h_inv, h, POLY_WORDS);
    if (poly_inv(h_inv)) {
        printf("oops, h not invertible\n");
        return -1;
    }
    poly_mul(f, h, h_inv);

    printf("h^{-1}: ");
    poly_to_sparse(f_sparse, &f_weight, h_inv);
    print_poly_sparse(f_sparse, f_weight);
    printf("\n");
    print_poly_dense(h_inv, POLY_WORDS);
    printf("\nf = h/h:\n");
    print_poly_dense(f, POLY_WORDS);
    printf("\n");
    
    return 0;
}

#endif /* POLY_MAIN */
