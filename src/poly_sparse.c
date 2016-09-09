#include <stddef.h>

#include "randombytes.h"

#include "poly_sparse.h"
#include "config.h"
#include "poly.h"

void polsp_rand(index_t *f) {
    static const index_t MASK = ((INDEX_C(1) << POLY_INDEX_BITS) - 1);

    index_t buf[2 * POLY_WEIGHT];
    index_t cand;
    size_t i, j, weight;

	do {
		randombytes((unsigned char *)buf, 2 * POLY_WEIGHT * INDEX_BYTES);
		weight = 0;
		for (i = 0; i < 2 * POLY_WEIGHT && weight < POLY_WEIGHT; ++i) {
            cand = buf[i] & MASK;
            if (cand > POLY_BITS) {
                continue;
            }
			for (j = 0; j < weight; ++j) {
				if (cand == f[j]) {
					break;
				}
			}
            if (j != weight) {
                continue;
            }
            f[weight++] = cand;
		}
	} while (weight < POLY_WEIGHT);
}

void polsp_copy(index_t *f, const index_t *g) {
    size_t i;
    for (i = 0; i < POLY_WEIGHT; ++i) {
        f[i] = g[i];
    }
}

void polsp_to_dense(word_t *f, const index_t *g) {
    static const unsigned char WORD_INDEX_MASK = (1 << WORD_INDEX_BITS) - 1;

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

/* non-optimized: I could not think of a side-channel resistant optimization
 * of this function */
void polsp_mul(word_t *f, const word_t *g, const index_t *h) {
    word_t h_dense[POLY_WORDS];
    polsp_to_dense(h_dense, h);
    poly_mul(f, g, h_dense);
}

void polsp_transpose(index_t *f) {
    size_t i;
    index_t mask;

    for (i = 0; i < POLY_WEIGHT; ++i) {
        mask = ((index_t)(-f[i]) >> (INDEX_BITS - 1)) - 1;
        f[i] = POLY_BITS - f[i] - (POLY_BITS & mask);
    }
}
