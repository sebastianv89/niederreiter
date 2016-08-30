#ifndef NIEDERREITER_DEBUG_H
#define NIEDERREITER_DEBUG_H

#include <stdio.h>

#include "config.h"
#include "inttypes.h"

void poly_to_sparse(index_t *f, size_t *weight, const word_t *g);
void print_poly_dense(const word_t *f, size_t words);
void print_poly_sparse(const index_t *f, size_t weight);
void print_bytes(const unsigned char *buf, size_t bytes);

#endif /* NIEDERREITER_DEBUG_H */
