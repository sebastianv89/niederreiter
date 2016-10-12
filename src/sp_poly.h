#ifndef NIEDERREITER_SP_POLY_H
#define NIEDERREITER_SP_POLY_H

#include "types.h"

/** \def POLY_INDEX_BITS ceil(log_2(POLY_BITS)) */
#define POLY_INDEX_BITS 13

/** f $= polynomial
 *
 * Length (in bits) = POLY_BITS
 * Hamming weight = POLY_WEIGHT
 * Implemented with rejection sampling.
 */
void sp_poly_rand(sp_poly_t f);

/** f := g */
void sp_poly_copy(sp_poly_t f, const sp_poly_t g);

/** f := g */
void sp_poly_to_poly(poly_t f, const sp_poly_t g);

#endif /* NIEDERREITER_SP_POLY_H */
