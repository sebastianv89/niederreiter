#include <stddef.h>

#include "randombytes.h"

#include "config.h"
#include "types.h"
#include "sp_poly.h"
#include "poly.h"

void sp_poly_rand(sp_poly_t f) {
    static const index_t MASK = ((1 << POLY_INDEX_BITS) - 1);
    index_t cand, buf[3 * POLY_WEIGHT];
    size_t i, j, weight = 0;
	do {
		randombytes((unsigned char *)buf, 3 * POLY_WEIGHT * INDEX_BYTES);
		for (i = 0; i < 2 * POLY_WEIGHT && weight < POLY_WEIGHT; ++i) {
            cand = buf[i] & MASK;
            if (cand >= POLY_BITS) {
                continue;
            }
			for (j = 0; j < weight; ++j) {
				if (cand == f[j]) {
					break;
				}
			}
            if (j < weight) {
                continue;
            }
            f[weight++] = cand;
		}
	} while (weight < POLY_WEIGHT);
}

void sp_poly_copy(sp_poly_t f, const sp_poly_t g) {
    size_t i;
    for (i = 0; i < POLY_WEIGHT; ++i) {
        f[i] = g[i];
    }
}

void sp_poly_to_poly(poly_t f, const sp_poly_t g) {
    static const unsigned char LIMB_INDEX_MASK = (1 << LIMB_INDEX_BITS) - 1;

	size_t i, j, limb_index[POLY_WEIGHT];
	limb_t mask, bit[POLY_WEIGHT];
    
	for (j = 0; j < POLY_WEIGHT; ++j) {
		limb_index[j] = g[j] >> LIMB_INDEX_BITS;
		bit[j] = (limb_t)(1) << (g[j] & LIMB_INDEX_MASK);
	}

	for (i = 0; i < POLY_LIMBS; ++i) {
		f[i] = 0;
		for (j = 0; j < POLY_WEIGHT; ++j) {
            mask = limb_index[j] ^ i;
            mask = -((mask - 1) >> (LIMB_BITS - 1));
            f[i] |= bit[j] & mask;
		}
	}
}
