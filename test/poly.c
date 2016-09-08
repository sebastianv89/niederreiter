#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "randombytes.h"

#include "poly.h"
#include "config.h"
#include "debug.h"

void test_to_dense()
{
    word_t f[POLY_WORDS];
    index_t g0[POLY_WEIGHT], g1[POLY_WEIGHT];
    index_t h0[ERROR_WEIGHT], h1[ERROR_WEIGHT];
    size_t weight;
    
    poly_gen_sparse(g0, TYPE_POLY);
    poly_to_dense(f, g0);
    poly_to_sparse(g1, &weight, f);
    assert(weight == POLY_WEIGHT);
    assert(poly_sparse_eq(g0, g1, POLY_WEIGHT));

    poly_gen_sparse(h0, TYPE_ERROR);
    error_to_dense(f, h0);
    error_to_sparse(h1, &weight, f);
    print_poly_dense(f + , ERROR_);
    print_poly_dense(f, ERROR_);
    print_poly_sparse(h0, ERROR_WEIGHT);
    printf("\nweight: %"PRIu64"\n",weight);
    assert(weight == ERROR_WEIGHT);
    assert(poly_sparse_eq(h0, h1, ERROR_WEIGHT));
}

void test_mul_inv()
{
    word_t f[POLY_WORDS], g[POLY_WORDS], h[POLY_WORDS];
    index_t g_sparse[POLY_WEIGHT];
    
    poly_gen_sparse(g_sparse, TYPE_POLY);
    poly_to_dense(g, g_sparse);
    poly_copy(g, h, POLY_WORDS);
    if (poly_inv(g)) {
        fprintf(stderr, "Unlucky test-case: g was not invertible\n");
        return;
    }
    poly_mul(f, g, h);
    assert(poly_is_one_nonconst(f));
}

void test_compare()
{
    word_t f[POLY_WORDS], g[POLY_WORDS];
    word_t eq, lt;
    bool is_eq;
    index_t deg_f, deg_g;
    
    poly_rand(f);
    poly_rand(g);
    is_eq = poly_eq(f, g, POLY_WORDS);
    deg_f = poly_degree(f);
    deg_g = poly_degree(g);
    
    poly_compare(&eq, &lt, f, g);
    assert(is_eq ? eq == (word_t)(~WORD_C(0)) : eq == 0);
    if (deg_f < deg_g) {
        assert(lt == (word_t)(~WORD_C(0)));
    }

    poly_compare(&eq, &lt, f, f);
    assert(eq == (word_t)(~WORD_C(0)));
}

int main(int argc, char *argv[])
{
    const unsigned int TEST_ROUNDS = 10;
    unsigned int i, test_rounds;
    
    if (argc < 2) {
        test_rounds = TEST_ROUNDS;
    } else {
        test_rounds = (unsigned int)(atoi(argv[1]));
    }
    
    for (i = 0; i < test_rounds; ++i) {
        test_to_dense();
        test_mul_inv();
        test_compare();
    }

    return EXIT_SUCCESS;
}
