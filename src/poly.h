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
#ifndef NIEDERREITER_POLY_H
#define NIEDERREITER_POLY_H

#include "config.h"

/** f := 0 */
void poly_zero(word_t *f);

/** f := 1 */
void poly_one(word_t *f);

/** f := G */
void poly_G(word_t *f);

/** f := g */
void poly_copy(word_t *f, const word_t *g);

/** \return f == 0 ? 0 : -1 */
int poly_verify_zero(const word_t *f);

/** \return f == 1 ? 1 : 0
 *
 * \warning Not in constant time.
 */
int poly_is_one(const word_t *f);

/** \return hamming weight of f */
word_t poly_hamming_weight(const word_t *f);

/** f := g + h */
void poly_add(word_t *f, const word_t *g, const word_t *h);

/** f += g */
void poly_inplace_add(word_t *f, const word_t *g);

/** f := g & mask */
void poly_mask(word_t *f, const word_t *g, const word_t *mask);

/** f += g & mask
 *
 * Mask is extended to the length of \p g by repeating.
 */
void poly_inplace_add_masked(word_t *f, const word_t *g, word_t mask);

/** f *= x */
void poly_inplace_mulx(word_t *f);

/** f := g * x mod G */
void poly_mulx_modG(word_t *f, const word_t *g);

/** f := f * x mod G */
void poly_inplace_mulx_modG(word_t *f);

/** f := g / x */
void poly_divx(word_t *f, const word_t *g);

/** f := g / x mod G */
void poly_divx_modG(word_t *f, const word_t *g);

/** f := g mod G
 *
 * \param g The result of non-reduced multiplication.
 */
void poly_reduce(word_t *f, const word_t *g);

/** f := g * h */
void poly_mul(word_t *f, const word_t *g, const word_t *h);


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
            word_t *eq,
            word_t *lt,
      const word_t *f,
      const word_t *g);

/** Compute xgcd(f, G)
 *
 * Constant time implementation of the binary polynomial XGCD algorithm
 * The starting polynomial for g is constant: namely G.
 *
 * \param f  in: polynomial (POLY_WORDS);
 *           out: gcd(f, G)
 * \param a  in: allocated memory (POLY_WORDS);
 *           out: Bézout coefficient of f
 */
void poly_xgcd(word_t *f, word_t *a);

/** f := 1/f
 *
 * \return inverse exists ? 0 : -1
 */
int poly_inv(word_t *f);

#endif /* NIEDERREITER_POLY_H */
