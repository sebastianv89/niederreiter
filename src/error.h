#ifndef NIEDERREITER_ERROR_H
#define NIEDERREITER_ERROR_H

#include "config.h"
#include "types.h"

/** \def ERROR_BITS Error size */
#define ERROR_BITS (POLY_COUNT * POLY_BITS)

/** \def ERROR_INDEX_BITS ceil(log_2(ERROR_BITS)) */
#define ERROR_INDEX_BITS 14

/** err $= polynomial
 *
 * Length (in bits) = POLY_COUNT * POLY_BITS
 * Hamming weight = ERROR_WEIGHT
 * Implemented with rejection sampling.
 */
void sp_gen_error(sp_error_t e);

/** f := g */
void sp_error_copy(sp_error_t f, const sp_error_t g);

/** Subtract `POLY_BITS` from all error indices.
 * Subtraction saturates to -1.
 */
void sp_error_align(sp_error_t err);

/** f := g
 * 
 * Indices > POLY_BITS are ignored
 */
void sp_error_to_poly(poly_t f, const sp_error_t g);

/** f := g */
void sp_error_to_error(error_t f, const sp_error_t g);

#endif /* NIEDERREITER_ERROR_H */
