#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>

#include "randombytes.h"

#include "config.h"
#include "test/util.h"


void pack_error_sparse(unsigned char *dest, index_t *src) {
    pack_indices(dest, src, ERROR_WEIGHT);
}

void unpack_error_sparse(index_t *dest, unsigned char *src) {
    unpack_indices(dest, src, ERROR_WEIGHT);
}

void poly_rand(word_t *f) {
    randombytes((unsigned char *)f, POLY_WORDS * WORD_BYTES);
    f[POLY_WORDS - 1] &= TAIL_MASK;
}

bool poly_eq(const word_t *f, const word_t *g) {
    size_t i;
    for (i = 0; i < POLY_WORDS; ++i) {
        if (f[i] != g[i]) {
            return false;
        }
    }
    return true;
}

bool polsp_eq(const index_t *f, const index_t *g) {
    size_t i;
    for (i = 0; i < POLY_WEIGHT; ++i) {
        if (f[i] != g[i]) {
            return false;
        }
    }
    return true;
}

index_t poly_degree(const word_t *f) {
    index_t shift, word = POLY_WORDS - 1;
    for (shift = 0; shift < TAIL_BITS; ++shift) {
        if ((f[word] >> shift) & 1) {
            return word * WORD_BITS + shift;
        }
    }
    while (word > 0) {
        --word;
        for (shift = 0; shift < WORD_BITS; ++shift) {
            if ((f[word] >> shift) & 1) {
                return word * WORD_BITS + shift;
            }
        }
    }
    return (index_t)(-1);
}


void poly_mulxn(word_t *f, index_t n) {
    static const unsigned char WORD_INDEX_MASK = (1 << WORD_INDEX_BITS) - 1;
    word_t bit_shift, carry = 0;
    index_t i;

    bit_shift = WORD_C(1) << (n & WORD_INDEX_MASK);
    n >>= WORD_INDEX_BITS;

    for (i = n; i < POLY_BITS - 1; ++i) {
        word_t carry_next = f[i] >> (WORD_BITS - bit_shift);
        f[i] <<= bit_shift;
        f[i] |= carry;
        carry = carry_next;
    }
    f[POLY_BITS - 1] <<= bit_shift;
    f[POLY_BITS - 1] |= carry;
}

void poly_xn(word_t *f, index_t n) {
    poly_one(f);
    poly_mulxn(f, n);
}

void poly_divmod(word_t *div, word_t *rem, const word_t *f, const word_t *g) {
    index_t deg_rem, deg_g, deg_diff;
    word_t g_shifted[POLY_WORDS];
    word_t one_shifted[POLY_WORDS];

    printf("f: "); print_poly(f); printf("\n");
    printf("g: "); print_poly(g); printf("\n");

    poly_zero(div);
    poly_copy(rem, f);
    deg_g = poly_degree(g);
    deg_rem = poly_degree(rem);
    while (deg_rem <= deg_g) {
        poly_xn(one_shifted, deg_g - deg_rem);
        poly_mulxn(g_shifted, g, deg_g - deg_rem);
        poly_add_inplace(rem, g_shifted);
        poly_add_inplace(div, one_shifted);
        deg_rem = poly_degree(rem);
    }

    printf("div: "); print_poly(div); printf("\n");
    printf("rem: "); print_poly(rem); printf("\n");
}

void poly_to_sparse(index_t *f, size_t *weight, const word_t *g) {
    size_t word;
    index_t bit;
    word_t g_word;

    *weight = 0;
    for (word = 0; word < POLY_WORDS; ++word) {
        bit = (index_t)(word * WORD_BITS);
        g_word = g[word];
        while (g_word > 0) {
            if (g_word & 1) {
                f[*weight] = bit;
                *weight += 1;
            }
            g_word >>= 1;
            ++bit;
        }
    }
}

void err_to_sparse(index_t *f, size_t *weight, const word_t (*g)[POLY_WORDS]) {
    size_t poly, word;
    index_t bit;
    word_t g_word;

    *weight = 0;
    for (poly = 0; poly < NUMBER_OF_POLYS; ++poly) {
        for (word = 0; word < POLY_WORDS; ++word) {
            bit = (index_t)(word * WORD_BITS);
            g_word = g[poly][word];
            while (g_word > 0) {
                if (g_word & 1) {
                    f[*weight] = bit;
                    *weight += 1;
                }
                g_word >>= 1;
                ++bit;
            }
        }
    }
}

void print_poly_sz(const word_t *f, size_t size) {
    size_t i;
    for (i = 0; i < size-1; ++i) {
        printf("%016" PRIx64 ", ", f[i]);
    }
    printf("%016" PRIx64, f[i]);
}

void print_poly(const word_t *f) {
    print_poly_sz(f, POLY_WORDS);
}
void print_polsp_wg(const index_t *f, size_t weight) {
    size_t i;

    for (i = 0; i < weight-1; ++i) {
        printf("%" PRIu16 ", ", f[i]);
    }
    printf("%" PRIu16, f[i]);
}

void print_polsp(const index_t *f) {
    print_polsp_wg(f, POLY_WEIGHT);
}

void print_bytes(const unsigned char *buf, size_t bytes) {
    size_t i;
    for (i = 0; i < bytes; ++i) {
        printf("%02hhx", buf[i]);
    }
}
