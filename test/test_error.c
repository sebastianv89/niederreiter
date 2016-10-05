#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "config.h"
#include "error.h"
#include "test/util.h"

void test_to_dense() {
    index_t f0[ERROR_WEIGHT], f1[ERROR_WEIGHT];
    word_t g[NUMBER_OF_POLYS][POLY_WORDS];
    index_t weight;

    polsp_rand(f0);
    err_to_dense(g, f0);
    err_to_sparse(f1, &weight, g);
    assert(weight == ERROR_WEIGHT);
    assert(poly_sparse_eq(f0, f1, ERROR_WEIGHT));
}

int main(int argc, char *argv[]) {
    test_to_dense();

    return EXIT_SUCCESS;
}
