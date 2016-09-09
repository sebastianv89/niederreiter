/**! \file util.h
 *
 * This file includes all those functions that are useful to have for debugging
 * and testing, but which are not necessary in the actual implementation.
 */
#ifndef NIEDERREITER_TEST_UTIL_H
#define NIEDERREITER_TEST_UTIL_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "config.h"

void pack_error_sparse(unsigned char *dest, index_t *src);
void unpack_error_sparse(index_t *dest, unsigned char *src);

void poly_rand(word_t *f);
bool poly_eq(const word_t *f, const word_t *g);
bool polsp_eq(const index_t *f, const index_t *g);
index_t poly_degree(const word_t *f);
/** f = g * x^n */
void poly_mulxn(word_t *f, const word_t *g, index_t n);
void poly_divmod(word_t *div, word_t *rem, const word_t *f, const word_t *g);
/** f := g; weight := Hamming weight */
void poly_to_sparse(index_t *f, size_t *weight, const word_t *g);
/** f := g; weight := Hamming weight */
void err_to_sparse(index_t *f, size_t *weight, const word_t (*g)[POLY_WORDS]);

void print_poly_sz(const word_t *f, size_t size);
void print_poly(const word_t *f);
void print_polsp_wg(const index_t *f, size_t weight);
void print_polsp(const index_t *f);
void print_bytes(const unsigned char *buf, size_t bytes);

#endif /* NIEDERREITER_TEST_UTIL_H */
