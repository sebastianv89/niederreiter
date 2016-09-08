#ifndef NIEDERREITER_ERROR_H
#define NIEDERREITER_ERROR_H

#include "config.h"

#define ERROR_BITS (NUMBER_OF_POLYS * POLY_BITS)

/** \def ERROR_INDEX_BITS ceil(log_2(ERROR_BITS)) */
#define ERROR_INDEX_BITS 14

/** err := 0 */
void error_zero(word_t (*err)[POLY_WORDS]);

/** dense := sparse */
void error_to_dense(word_t (*dense)[POLY_WORDS], const index_t *sparse);

/** err $= polynomial
 * 
 * Length (in bits) = NUMBER_OF_POLYS * POLY_BITS
 * Hamming weight = ERROR_WEIGHT
 * Implemented with rejection sampling.
 */
void error_rand(index_t *err);

#endif /* NIEDERREITER_ERROR_H */
