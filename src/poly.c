#include <stddef.h>

#include "config.h"
#include "types.h"
#include "poly.h"

void poly_zero(poly_t f) {
    size_t i;
    for (i = 0; i < POLY_LIMBS; ++i) {
        f[i] = 0;
    }
}

void poly_copy(poly_t f, const poly_t g) {
    size_t i;
    for (i = 0; i < POLY_LIMBS; ++i) {
        f[i] = g[i];
    }
}

int poly_verify_zero(const poly_t f) {
    size_t i, shift;
    unsigned char nonzero_bits = 0;
    for (i = 0; i < POLY_LIMBS - 1; ++i) {
        for (shift = 0; shift < LIMB_BYTES; shift += 8) {
            nonzero_bits |= f[i] >> shift;
        }
    }
    for (shift = 0; shift < TAIL_BYTES; shift += 8) {
        nonzero_bits |= f[i] >> shift;
    }
    return (((nonzero_bits - 1) >> 8) & 1) - 1;
}

int poly_verify_one(const poly_t f) {
    size_t i;
    if (f[0] != 1) {
        return -1;
    }
    for (i = 1; i < POLY_LIMBS; ++i) {
        if (f[i] != 0) {
            return -1;
        }
    }
    return 0;
}

/* TODO: if the processor has a built-in popcount, that is probably
 * more efficient */
limb_t poly_hamming_weight(const poly_t f) {
#if __has_builtin(__builtin_popcountll)
    size_t i;
    int total = 0;
    for (i = 0; i < POLY_LIMBS; ++i) {
        total += __builtin_popcountll(f[i]);
    }
    return (limb_t)total;
#else
    static const limb_t MASK0 = (limb_t)(~0) / 3;
    static const limb_t MASK1 = (limb_t)(~0) / 15 * 3;
    static const limb_t MASK2 = (limb_t)(~0) / 255 * 15;
    static const limb_t MULT  = (limb_t)(~0) / 255;
    limb_t wc, total = 0;
    size_t i;

    for (i = 0; i < POLY_LIMBS; ++i) {
        wc   = f[i];
        wc  -= (wc >> 1) & MASK0;
        wc   = (wc & MASK1) + ((wc >> 2) & MASK1);
        wc  += wc >> 4;
        wc  &= MASK2;
        wc  *= MULT;
        wc >>= LIMB_BITS - 8;
        total += wc;
    }
    return total;
#endif
}

void poly_add(poly_t f, const poly_t g, const poly_t h) {
    size_t i;
    for (i = 0; i < POLY_LIMBS; ++i) {
        f[i] = g[i] ^ h[i];
    }
}

void poly_mask(poly_t f, const poly_t g, const poly_t mask) {
    size_t i;
    for (i = 0; i < POLY_LIMBS; ++i) {
        f[i] = g[i] & mask[i];
    }
}

void poly_inplace_add_masked(poly_t f, const poly_t g, limb_t mask) {
    size_t i;
    for (i = 0; i < POLY_LIMBS; ++i) {
        f[i] ^= g[i] & mask;
    }
}

void poly_inplace_mulx(poly_t f) {
    size_t i;
    limb_t carry, tmp;
    carry = f[0] >> (LIMB_BITS - 1);
    f[0] <<= 1;
    for (i = 1; i < POLY_LIMBS - 1; ++i) {
        tmp = f[i] >> (LIMB_BITS - 1);
        f[i] <<= 1;
        f[i] |= carry;
        carry = tmp;
    }
    f[i] <<= 1;
    f[i] |= carry;
}

/* TODO: multiple carry chains? */
void poly_mulx_modG(poly_t f, const poly_t g) {
    size_t i;
    f[0] = (g[0] << 1) | (g[POLY_LIMBS - 1] >> (TAIL_BITS - 1));
    for(i = 1; i < POLY_LIMBS - 1; ++i) {
        f[i] = (g[i] << 1) | (g[i - 1] >> (LIMB_BITS - 1));
    }
#if TAIL_BITS == 1
    f[i] = g[i - 1] >> (LIMB_BITS - 1);
#else
    f[i] = g[i] << 1;
    f[i] |= g[i - 1] >> (LIMB_BITS - 1);
    f[i] &= TAIL_MASK;
#endif
}

/* TODO: multiple carry chains? */
void poly_inplace_mulx_modG(poly_t f) {
    size_t i;
    limb_t carry, tmp;
    carry = f[POLY_LIMBS - 1] >> (TAIL_BITS - 1);
    for (i = 0; i < POLY_LIMBS - 1; ++i) {
        tmp = f[i] >> (LIMB_BITS - 1);
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

/* TODO: multiple carry chains? */
void poly_divx(poly_t f, const poly_t g) {
    size_t i;
    for (i = 0; i < POLY_LIMBS - 1; ++i) {
        f[i] = (g[i] >> 1) | (g[i + 1] << (LIMB_BITS - 1));
    }
    f[i] = g[i] >> 1;
}

/* TODO: multiple carry chains? */
void poly_divx_modG(poly_t f, const poly_t g) {
    size_t i;
    limb_t carry;
    carry = (g[0] & 1) << (TAIL_BITS - 1);
    for (i = 0; i < POLY_LIMBS - 1; ++i) {
        f[i] = (g[i] >> 1) | (g[i + 1] << (LIMB_BITS - 1));
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
 * g is the result of a non-reduced multiplication: it is assumed to
 * have 2*`POLY_LIMBS`-1 words.
 */
void poly_reduce(poly_t f, const limb_t *g) {
    size_t i;
    for (i = 0; i < POLY_LIMBS - 1; ++i) {
        f[i] = g[i];
        f[i] ^= g[i + POLY_LIMBS - 1] >> TAIL_BITS;
        f[i] ^= g[i + POLY_LIMBS] << (LIMB_BITS - TAIL_BITS);
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
void poly_mul(poly_t f, const poly_t g, const poly_t h) {
    unsigned char bit_index;
    size_t i;
    limb_t mask, res[2 * POLY_LIMBS - 1] = {0};
    poly_t lhs;

    poly_copy(lhs, g);
    for (i = 0; i < POLY_LIMBS; ++i) {
        mask = -(h[i] & 1);
        poly_inplace_add_masked(res + i, lhs, mask);
    }
    for (bit_index = 1; bit_index < LIMB_BITS; ++bit_index) {
        poly_inplace_mulx_modG(lhs);
        for (i = 0; i < POLY_LIMBS; ++i) {
            mask = -((h[i] >> bit_index) & 1);
            poly_inplace_add_masked(res + i, lhs, mask);
        }
    }
    poly_reduce(f, res);
}

void poly_compare(limb_t *lt, const poly_t f, const poly_t g) {
    size_t word = POLY_LIMBS - 1, shift;
    unsigned char f_byte, g_byte;
    unsigned char eq = 1;
    *lt = 0;
    shift = 8 * TAIL_BYTES;
    while (shift > 0) {
        shift -= 8;
        f_byte = (unsigned char)(f[word] >> shift);
        g_byte = (unsigned char)(g[word] >> shift);
        *lt |= eq & ((f_byte - g_byte) >> 8);
        eq &= ((f_byte ^ g_byte) - 1) >> 8;
    }
    while (word > 0) {
        --word;
        shift = LIMB_BITS;
        while (shift > 0) {
            shift -= 8;
            f_byte = (unsigned char)(f[word] >> shift);
            g_byte = (unsigned char)(g[word] >> shift);
            *lt |= eq & ((f_byte - g_byte) >> 8);
            eq &= ((f_byte ^ g_byte) - 1) >> 8;
        }
    }
    *lt = -(*lt);
}

void poly_xgcd(poly_t f, poly_t a) {
    size_t i, j;
    poly_t g = {1}, b = {0};
    g[POLY_LIMBS - 1] = 1 << TAIL_BITS; /* g := G */

    /* Compute every iteration:
     *      if (even(f)) { f = f_div_x;  a = a_div_x; }
     * else if (even(g)) { g = g_div_x;  b = b_div_x; }
     * else if (f > g)   { g = fg_div_x; b = ab_div_x; }
     * else              { f = fg_div_x; a = ab_div_x; }
     */
    for (i = 0; i < 2 * POLY_BITS - 1; ++i) {
        poly_t f_divx, g_divx, a_divx, b_divx, fg_divx, ab_divx;
        limb_t f_odd, g_odd, g_lt_f;
        limb_t f2f, f2fx, f2fgx, g2g, g2gx, g2fgx;

        /* Possible room for optimization (parallellization) */
        poly_divx(f_divx, f);
        poly_divx(g_divx, g);
        poly_add(fg_divx, f_divx, g_divx);
        poly_divx_modG(a_divx, a);
        poly_divx_modG(b_divx, b);
        poly_add(ab_divx, a_divx, b_divx);

        poly_compare(&g_lt_f, g, f);
        f_odd = -(f[0] & 1);
        g_odd = -(g[0] & 1);

#ifndef ALT_XGCD_MASKS
        f2f   =  f_odd & (~g_odd | ~g_lt_f);
        f2fx  = ~f_odd;
        f2fgx =  f_odd &   g_odd &  g_lt_f;
        g2g   = ~f_odd | ( g_odd &  g_lt_f);
        g2gx  =  f_odd &  ~g_odd;
        g2fgx =  f_odd &   g_odd & ~g_lt_f;
#else
        f2fx  = ~f_odd;
        f2fgx = f_odd & g_odd & g_lt_f;
        g2g   = f2fx | f2fgx;
        f2f   = ~g2g;
        g2gx  = f2f & g_odd;
        g2fgx = ~(g2g | g2gx);
# endif

        for (j = 0; j < POLY_LIMBS; ++j) {
            f[j] = (f[j] & f2f) | (f_divx[j] & f2fx) | (fg_divx[j] & f2fgx);
            a[j] = (a[j] & f2f) | (a_divx[j] & f2fx) | (ab_divx[j] & f2fgx);
            g[j] = (g[j] & g2g) | (g_divx[j] & g2gx) | (fg_divx[j] & g2fgx);
            b[j] = (b[j] & g2g) | (b_divx[j] & g2gx) | (ab_divx[j] & g2fgx);
        }
    }
}

int poly_inv(poly_t f) {
    int ret;
    poly_t inv = {1};
    poly_xgcd(f, inv);
    if ((ret = poly_verify_one(f))) {
        return ret;
    }
    poly_copy(f, inv);
    return ret;
}
