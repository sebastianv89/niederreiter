#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "config.h"
#include "poly_sparse.h"
#include "test/util.h"

void test_to_dense() {
    word_t f[POLY_WORDS];
    index_t g0[POLY_WEIGHT], g1[POLY_WEIGHT];
    size_t weight;

    polsp_rand(g0);
    polsp_to_dense(f, g0);
    poly_to_sparse(g1, &weight, f);
    assert(weight == POLY_WEIGHT);
    assert(polsp_eq(g0, g1));
}

void test_double_transpose() {
    index_t f[POLY_WEIGHT], g[POLY_WEIGHT];

    polsp_rand(f);
    polsp_copy(g, f);
    polsp_transpose(f);
    polsp_transpose(f);
    assert(polsp_eq(f, g));
}

int main(int argc, char *argv[]) {
    const unsigned int TEST_ROUNDS = 10;
    unsigned int i, test_rounds;

    if (argc < 2) {
        test_rounds = TEST_ROUNDS;
    } else {
        test_rounds = (unsigned int)(atoi(argv[1]));
    }

    for (i = 0; i < test_rounds; ++i) {
        test_to_dense();
        test_double_transpose();
    }

    return EXIT_SUCCESS;
}
