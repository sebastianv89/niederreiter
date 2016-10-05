#ifndef NIEDERREITER_POLY_H
#define NIEDERREITER_POLY_H

/**! \file poly.h
 * Polynomial representation: The least significant bit of each word holds the
 * coefficient of the lowest degree. Words with a higher index hold
 * represent higher degrees. For example, the two 8-bit word f with
 * f[0] = 0b00010011 and f[1] = 0b00100000 represents the polynomial
 * 1 + x + x^4 + x^13.
 *
 * Polynomials are defined modulo G, where
 *   G = x^POLY_BITS - 1
 */

#include "types.h"

/** f := 0 */
void poly_zero(poly_t f);

/** f := g */
void poly_copy(poly_t f, const poly_t g);

/** \return f == 0 ? 0 : -1 */
int poly_verify_zero(const poly_t f);

/** \return f == 0 ? 0 : -1
 *
 * \warning Not in constant time.
 */
int poly_verify_one(const poly_t f);

/** \return hamming weight of \p f */
limb_t poly_hamming_weight(const poly_t f);

/** f := g + h 
 *
 * Any subset of {f, g, h} may point to the same memory. Misaligned
 * overlap in memory leads to erroneous behaviour.
 */
void poly_add(poly_t f, const poly_t g, const poly_t h);

/** f := g & mask */
void poly_mask(poly_t f, const poly_t g, const poly_t mask);

/** f += g & mask
 *
 * Mask is extended to the length of \p g by repeating.
 */
void poly_inplace_add_masked(poly_t f, const poly_t g, limb_t mask);

/** f *= x */
void poly_inplace_mulx(poly_t f);

/** f := g * x mod G
 * 
 * Equivalent to rotating the matrix row.
 */
void poly_mulx_modG(poly_t f, const poly_t g);

/** f := f * x mod G */
void poly_inplace_mulx_modG(poly_t f);

/** f := g / x */
void poly_divx(poly_t f, const poly_t g);

/** f := g / x mod G */
void poly_divx_modG(poly_t f, const poly_t g);

/** f := g mod G
 *
 * \param g The result of non-reduced multiplication.
 */
void poly_reduce(poly_t f, const limb_t *g);

/** f := g * h */
void poly_mul(poly_t f, const poly_t g, const poly_t h);

/** Compare polynomials
 *
 * This function is used in the context of the xgcd.  For equality,
 * the polynomials need to be exactly the same.  To find out which
 * polynomial is greater, only the degree matters.  If the degrees
 * are equal, it does not matter what is returned in \p lt.
 *
 * \param[out] eq f == g ? -1 : 0
 * \param[out] lt f < g ? -1 : 0
 */
void poly_compare(
            poly_t eq,
            poly_t lt,
      const poly_t f,
      const poly_t g);

/** Compute xgcd(f, G)
 *
 * Constant time implementation of the binary polynomial XGCD algorithm
 * The starting polynomial for g is constant: namely G.
 *
 * \param[in,out] f  in:  Polynomial (POLY_LIMBS);
 *                   out: GCD(f, G)
 * \param[in,out] a  in:  One
 *                   out: Bézout coefficient of f
 */
void poly_xgcd(poly_t f, poly_t a);

/** f := 1/f
 *
 * \return inverse exists ? 0 : -1
 */
int poly_inv(poly_t f);

#endif /* NIEDERREITER_POLY_H */
