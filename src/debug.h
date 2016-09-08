#ifndef NIEDERREITER_DEBUG_H
#define NIEDERREITER_DEBUG_H

#include <stdlib.h>
#include <stdbool.h>

#include "config.h"

void unpack_error_sparse(index_t *dest, unsigned char *src);
void pack_error_sparse(unsigned char *dest, index_t *src);

void poly_rand_dense(word_t *f);
bool poly_eq(const word_t *f, const word_t *g);
index_t poly_degree(const word_t *f);
bool poly_sparse_eq(const index_t *f, const index_t *g);
void poly_to_sparse(index_t *f, size_t *weight, const word_t *g);
void error_to_sparse(index_t *f, size_t *weight, const word_t (*g)[POLY_WORDS]);

void print_poly_dense(const word_t *f);
void print_poly_sparse(const index_t *f);
void print_bytes(const unsigned char *buf, size_t bytes);


/* expose some "private" functions */

void kem_to_systematic(word_t (*sys_par_ch)[POLY_WORDS],
                       const word_t *inv,
                       const index_t (*par_ch)[POLY_WEIGHT]);
void kem_transpose_privkey(index_t (*par_ch)[POLY_WEIGHT]);

void pack_words(unsigned char *dest, const word_t *src, size_t bytes);
void unpack_words(word_t *dest, const unsigned char *src, size_t bytes);
void pack_indices(unsigned char *dest, const index_t *src, size_t weight);
void unpack_indices(index_t *dest, const unsigned char *src, size_t weight);

int poly_is_one(const word_t *f);
void poly_copy(word_t *f, const word_t *g);
void poly_compare(word_t *eq, word_t *lt, const word_t *f, const word_t *g);

#endif /* NIEDERREITER_DEBUG_H */
