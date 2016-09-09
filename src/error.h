#ifndef NIEDERREITER_ERROR_H
#define NIEDERREITER_ERROR_H

#include "config.h"

/** \def ERROR_BITS Error size */
#define ERROR_BITS (NUMBER_OF_POLYS * POLY_BITS)

/** \def ERROR_INDEX_BITS ceil(log_2(ERROR_BITS)) */
#define ERROR_INDEX_BITS 14

/** err := 0 */
void err_zero(word_t (*err)[POLY_WORDS]);

/** err $= polynomial
 *
 * Length (in bits) = NUMBER_OF_POLYS * POLY_BITS
 * Hamming weight = ERROR_WEIGHT
 * Implemented with rejection sampling.
 */
void err_rand(index_t *err);

/** Subtract `POLY_BITS` from all error indices.
 * Subtraction saturates to (unsigned) value -1.
 */
void err_align(index_t *err);

/** f := g */
void err_to_dense(word_t (*f)[POLY_WORDS], const index_t *g);

#endif /* NIEDERREITER_ERROR_H */
