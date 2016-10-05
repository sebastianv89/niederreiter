#ifndef NIEDERREITER_TEST_UTIL_H
#define NIEDERREITER_TEST_UTIL_H

/**! \file util.h
 *
 * This file includes all those functions that are useful to have for debugging
 * and testing, but which are not necessary in the actual implementation.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>

#include "types.h"

void pack_sp_error(unsigned char *dest, sp_error_t src);
void unpack_sp_error(sp_error_t dest, unsigned char *src);

void poly_rand(poly_t f);
bool poly_eq(const poly_t f, const poly_t g);
int index_cmp(const void *x, const void *y);
void sp_poly_sort(sp_poly_t f);
bool sp_poly_eq(const sp_poly_t f, const sp_poly_t g);
bool err_eq(const error_t f, const error_t g);
index_t poly_degree(const poly_t f);
/** f := g * x^n */
void poly_mulxn(poly_t f, const poly_t g, index_t n);
/** f := x^n */
void poly_xn(poly_t f, index_t n);
/** Compute div/mod, such that: f == div * g + mod
 *
 * Precondition: div == 0
 */
void poly_divmod(poly_t div, poly_t mod, const poly_t f, const poly_t g);
/** f := g; Precondition: hw(g) == POLY_WEIGHT */
void poly_to_sp_poly(sp_poly_t f, const poly_t g);
/** f := e; Precondition: hw(g) == ERROR_WEIGHT */
void error_to_sp_error(sp_error_t f, const error_t e);

/* This buffer should be big enough for the string rep of any
 * object. Dense errors are probably the biggest we want to output,
 * with 2 hex chars per byte, plus some overhead of chars joining
 * polys and limbs (assumed <= 2 bytes overhead per byte).
 */
#define TEST_STR_CHARS (4 * POLY_COUNT * POLY_BYTES)
#define TEST_RES(out, in, to_str, n)                            \
    {                                                           \
        if (ret) {                                              \
            if (verbose >= 2) {                                 \
                printf("%s passed\n", __func__);                \
            }                                                   \
        } else {                                                \
            if (verbose >= 1) {                                 \
                char str[TEST_STR_CHARS];                       \
                int ch = to_str(str, in, n);                    \
                assert(0 <= ch && ch < TEST_STR_CHARS);         \
                printf("%s failed for: %s\n", __func__, str);   \
            }                                                   \
        }                                                       \
    }

int poly_to_str(char *str, const poly_t f);
int sp_poly_to_str(char *str, const sp_poly_t f);
int error_to_str(char *str, const error_t e);
int sp_error_to_str(char *str, const sp_error_t e);
int bytes_to_hex(char *str, const unsigned char *buf, int byte_count);
int sys_par_ch_to_str(char *str, const sys_par_ch_t k);
int par_ch_to_str(char *str, const par_ch_t k);

void fprintpoly(FILE *f, const poly_t p);
void fprintsppoly(FILE *f, const sp_poly_t p);
void fprinterror(FILE *f, const error_t e);
void fprintsperror(FILE *f, const sp_error_t e);
void fprintbytes(FILE *f, const unsigned char *buf, int byte_count);
void fprintsysparch(FILE *f, const sys_par_ch_t k);
void fprintparch(FILE *f, const par_ch_t k);

#endif /* NIEDERREITER_TEST_UTIL_H */
