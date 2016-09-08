#ifndef NIEDERREITER_POLY_H
#define NIEDERREITER_POLY_H

#include "config.h"

/**!
 * Polynomial representation: The lsb of each word holds the
 * coefficient of the lowest degree. Words with a higher index hold
 * represent higher degrees. For example, the two 8-bit word f with
 * f[0] = 0b00010011; f[1] = 0b00100000 represents the polynomial 1 +
 * x + x^4 + x^13.
 */

/** f := 0 */
void poly_zero(word_t *f);

/** verify f == 0
 *
 * \return an error code: f == 0 ? 0 : -1
 */
int poly_verify_zero(const word_t *f);

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

/** f := f * x mod G */
void poly_inplace_mulx_modG(word_t *f);

/** f := g * h */
void poly_mul(word_t *f, const word_t *g, const word_t *h);

/** f := 1/f
 *
 * \return error code   (inverse exists ? 0 : -1)
 */
int poly_inv(word_t *f);

#endif /* NIEDERREITER_POLY_H */
