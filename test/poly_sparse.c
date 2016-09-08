#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "poly_sparse.h"
#include "config.h"

void test_double_transpose() {
    index_t f[POLY_WEIGHT], g[POLY_WEIGHT];
    
    poly_sparse_rand(f);
    poly_sparse_copy(g, f);
    poly_sparse_transpose(f);
    poly_sparse_transpose(f);
    assert(poly_sparse_eq(f, g));
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
        test_double_transpose();
    }

    return EXIT_SUCCESS;
}
