#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>

#include "randombytes.h"

#include "debug.h"
#include "config.h"

void unpack_error_sparse(index_t *dest, unsigned char *src) {
    unpack_indices(dest, src, ERROR_WEIGHT);
}

void pack_error_sparse(unsigned char *dest, index_t *src) {
    pack_indices(dest, src, ERROR_WEIGHT);
}

void poly_rand_dense(word_t *f) {
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

bool poly_sparse_eq(const index_t *f, const index_t *g) {
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

/* f := g
 * assumes f has enough memory allocated. sets weight.
 */
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

/* f := g
 * assumes f has enough memory allocated. sets weight.
 */
void error_to_sparse(index_t *f, size_t *weight, const word_t (*g)[POLY_WORDS]) {
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

void print_poly_dense(const word_t *f) {
    size_t i;
    for (i = 0; i < POLY_WORDS-1; ++i) {
        printf("%016" PRIx64 ", ", f[i]);
    }
    printf("%016" PRIx64, f[i]);
}

void print_poly_sparse(const index_t *f) {
    size_t i;

    for (i = 0; i < POLY_WEIGHT-1; ++i) {
        printf("%" PRIu16 ", ", f[i]);
    }
    printf("%" PRIu16, f[i]);
}

void print_bytes(const unsigned char *buf, size_t bytes) {
    size_t i;
    for (i = 0; i < bytes; ++i) {
        printf("%02hhx", buf[i]);
    }
}
