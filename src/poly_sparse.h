#ifndef NIEDERREITER_POLY_SPARSE_H
#define NIEDERREITER_POLY_SPARSE_H

#include "config.h"

/** \def POLY_INDEX_BITS ceil(log_2(POLY_BITS)) */
#define POLY_INDEX_BITS 13

/** f $= polynomial
 *
 * Length (in bits) = POLY_BITS
 * Hamming weight = ERROR_WEIGHT
 * Implemented with rejection sampling.
 */
void polsp_rand(index_t *f);

/** f := g */
void polsp_copy(index_t *f, const index_t *g);

/** f := g */
void polsp_to_dense(word_t *f, const index_t *g);

/** f := g * h */
void polsp_mul(word_t *f, const word_t *g, const index_t *h);

/** f := f^T
 *
 * A polynomial does not really have a transpose, but this operation
 * corresponds to transposing the circular matrix that is represented
 * by the polynomial.
 */
void polsp_transpose(index_t *f);

#endif /* NIEDERREITER_POLY_SPARSE_H */
