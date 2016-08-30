#include <assert.h>

#include "config.h"
#include "randombytes.h"
#include "poly.h"

#define TEST_ROUNDS 10

void poly_rand(word_t *f)
{
    randombytes((unsigned char *)f, POLY_WORDS * WORD_BYTES);
    f[POLY_WORDS - 1] &= TAIL_MASK;
}

/* f == g */
int poly_eq(const word_t *f, const word_t *g, size_t words)
{
    size_t i;
    for (i = 0; i < words; ++i) {
        if (f[i] != g[i]) {
            return 0;
        }
    }
    return 1;
}

/* f == g */
int poly_sparse_eq(const index_t *f, const index_t *g, size_t indices)
{
    size_t i;
    for (i = 0; i < indices; ++i) {
        if (f[i] != g[i]) {
            return 0;
        }
    }
    return 1;
}

/* f (sparse) := g (dense) */
void poly_to_sparse(index_t *f, size_t *indices, const word_t g, size_t words)
{
    size_t i, j = 0;
    word_t bit;
    *indices = 0;

    for (i = 0; i < words; ++i) {
        for (j = 0, bit = 1; j < WORD_BITS; ++j, bit <<= 1)
            if (g[j] & bit) {
                f[indices++] = i * WORD_BITS + j;
            }
        }
    }
}

void test_poly_to_dense()
{
    word_t f[POLY_WORDS];
    index_t g0[POLY_WEIGHT], g1[POLY_WEIGHT];
    index_t h0[ERROR_WEIGHT], h1[ERROR_WEIGHT];
    size_t weight;
    
    poly_gen_sparse(g0, GEN_POLY);
    poly_to_dense(f, POLY_WORDS, g0, POLY_WEIGHT);
    poly_to_sparse(g1, &weight, f, POLY_WORDS);
    assert(weight == POLY_WEIGHT);
    assert(poly_sparse_eq(g0, g1, POLY_WEIGHT));

    poly_gen_sparse(h0, GEN_ERROR);
    poly_to_dense(f, POLY_WORDS, h0, ERROR_WEIGHT);
    poly_to_sparse(h1, &weight, f, POLY_WORDS);
    assert(weight == ERROR_WEIGHT);
    assert(poly_sparse_eq(h0, h1, ERROR_WEIGHT));
}

int main(int argc, char *argv[])
{
    unsigned int test_rounds;
    unsigned int i;
    
    if (argc < 2) {
        test_rounds = TEST_ROUNDS;
    } else {
        test_rounds = atoi(argv[1]);
    }
    
    for (i = 0; i < test_rounds; ++i) {
        test_poly_to_dense();
    }

    return 0;
}
