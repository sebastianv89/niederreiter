#include "randombytes.h"

#include "error.h"
#include "config.h"
#include "poly.h"
#include "poly_sparse.h"

/* err := 0 */
void error_zero(word_t (*err)[POLY_WORDS]) {
    size_t i;
    for (i = 0; i < NUMBER_OF_POLYS; ++i) {
        poly_zero(err[i]);
    }
}

/* Subtract `POLY_BITS` from all error indices.
 * Subtraction saturates to (unsigned) value -1.
 */
void error_align(index_t *err) {
    size_t i;
    index_t mask;

    for (i = 0; i < ERROR_WEIGHT; ++i) {
        err[i] -= POLY_BITS;
        mask = -(err[i] >> (INDEX_BITS - 1));
        err[i] |= mask;
    }
}

/* dense := sparse */
void error_to_dense(word_t (*dense)[POLY_WORDS], const index_t *sparse) {
    size_t i;
    index_t aligned[ERROR_WEIGHT];

    poly_sparse_copy(aligned, sparse);
    poly_sparse_to_dense(dense[0], sparse);
    dense[0][POLY_WORDS - 1] &= TAIL_MASK;
    for (i = 1; i < NUMBER_OF_POLYS; ++i) {
        error_align(aligned);
        poly_sparse_to_dense(dense[i], aligned);
        dense[i][POLY_WORDS - 1] &= TAIL_MASK;
    }
}

/* rejection sampling */
void error_rand(index_t *err) {
    static const index_t MASK = ((INDEX_C(1) << ERROR_INDEX_BITS) - 1);
    
    index_t buf[2 * ERROR_WEIGHT];
    index_t cand;
    size_t i, j, weight;

    do {
        randombytes((unsigned char *)buf, 2 * ERROR_WEIGHT * INDEX_BYTES);
        weight = 0;
        for (i = 0; i < 2 * ERROR_WEIGHT && weight < ERROR_WEIGHT; ++i) {
            cand = buf[i] & MASK;
            if (cand > ERROR_BITS) {
                continue;
            }
            for (j = 0; j < weight; ++j) {
                if (cand == err[j]) {
                    break;
                }
            }
            if (j != weight) {
                continue;
            }
            err[weight++] = cand;
        }
    } while (weight < ERROR_WEIGHT);
}
