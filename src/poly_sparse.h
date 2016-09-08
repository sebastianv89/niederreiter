#ifndef NIEDERREITER_POLY_SPARSE_H
#define NIEDERREITER_POLY_SPARSE_H

#include "config.h"

/** \def POLY_INDEX_BITS ceil(log_2(POLY_BITS)) */
#define POLY_INDEX_BITS 13

/** f := g */
void poly_sparse_copy(index_t *f, const index_t *g);

/** f := g */
void poly_sparse_to_dense(word_t *f, const index_t *g);

/** f := g * h */
void poly_sparse_mul(word_t *f, const word_t *g, const index_t *h);

/** f := f^T
 *
 * A polynomial does not really have a transpose, but this operation
 * corresponds to transposing the circular matrix that is represented
 * by the polynomial.
 */
void poly_sparse_transpose(index_t *f);

/** f $= polynomial
 *
 * Length (in bits) = POLY_BITS
 * Hamming weight = ERROR_WEIGHT
 * Implemented with rejection sampling.
 */
void poly_sparse_rand(index_t *f);

#endif /* NIEDERREITER_POLY_SPARSE_H */
