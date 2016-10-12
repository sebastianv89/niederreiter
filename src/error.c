#include <stddef.h>

#include "randombytes.h"

#include "error.h"
#include "config.h"
#include "poly.h"
#include "sp_poly.h"

void sp_gen_error(sp_error_t err) {
    static const index_t MASK = ((1 << ERROR_INDEX_BITS) - 1);
    index_t cand, buf[3 * ERROR_WEIGHT];
    size_t i, j, weight = 0;
    do {
        randombytes((unsigned char *)buf, 3 * ERROR_WEIGHT * INDEX_BYTES);
        for (i = 0; i < 2 * ERROR_WEIGHT && weight < ERROR_WEIGHT; ++i) {
            cand = buf[i] & MASK;
            if (cand >= ERROR_BITS) {
                continue;
            }
            for (j = 0; j < weight; ++j) {
                if (cand == err[j]) {
                    break;
                }
            }
            if (j < weight) {
                continue;
            }
            err[weight++] = cand;
        }
    } while (weight < ERROR_WEIGHT);
}

void sp_error_copy(sp_error_t f, const sp_error_t g) {
    size_t i;
    for (i = 0; i < ERROR_WEIGHT; ++i) {
        f[i] = g[i];
    }
}

void sp_error_align(sp_error_t err) {
    size_t i;
    index_t mask;
    for (i = 0; i < ERROR_WEIGHT; ++i) {
        err[i] -= POLY_BITS;
        mask = -(err[i] >> (INDEX_BITS - 1));
        err[i] |= mask;
    }
}

void sp_error_to_poly(poly_t f, const sp_error_t g) {
    static const unsigned char LIMB_INDEX_MASK = (1 << LIMB_INDEX_BITS) - 1;

    size_t i, j, limb_index[ERROR_WEIGHT];
    limb_t mask, bit[ERROR_WEIGHT];
    for (j = 0; j < ERROR_WEIGHT; ++j) {
        limb_index[j] = g[j] >> LIMB_INDEX_BITS;
        bit[j] = (limb_t)(1) << (g[j] & LIMB_INDEX_MASK);
    }
    for (i = 0; i < POLY_LIMBS; ++i) {
        f[i] = 0;
        for (j = 0; j < ERROR_WEIGHT; ++j) {
            mask = limb_index[j] ^ i;
            mask = -((mask - 1) >> (LIMB_BITS - 1));
            f[i] |= bit[j] & mask;
        }
    }
    f[POLY_LIMBS - 1] &= TAIL_MASK;
}

void sp_error_to_error(error_t f, const sp_error_t g) {
    size_t i;
    sp_error_t g_copy;

    sp_error_copy(g_copy, g);
    sp_error_to_poly(f[0], g_copy);
    for (i = 1; i < POLY_COUNT; ++i) {
        sp_error_align(g_copy);
        sp_error_to_poly(f[i], g_copy);
    }
}

