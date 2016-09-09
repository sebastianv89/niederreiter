#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "randombytes.h"

#include "poly_sparse.h"
#include "config.h"
#include "test/util.h"

void test_mul_inv() {
    word_t f[POLY_WORDS], g[POLY_WORDS], h[POLY_WORDS];
    index_t g_sparse[POLY_WEIGHT];

    polsp_rand(g_sparse);
    poly_to_dense(g, g_sparse);
    poly_copy(g, h, POLY_WORDS);
    if (poly_inv(g)) {
        fprintf(stderr, "Test passed trivially: g non-invertible\n");
        return;
    }
    poly_mul(f, g, h);
    assert(poly_is_one_nonconst(f));
}

void test_compare() {
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

void poly_xgcd_ref(word_t *f, word_t *g, word_t *a, word_t *b) {
    size_t i, j;
    word_t new_a[POLY_WORDS], new_b[POLY_WORDS];
    word_t quot[POLY_WORDS], rem[POLY_WORDS];
    word_t tmp0[POLY_WORDS], tmp1[POLY_WORDS];

    poly_G(g);
    poly_one(a);
    poly_zero(b);
    poly_zero(new_a);
    poly_one(new_b);
    while (poly_verify_zero()) {
        /* g, f := f, g % f */
        poly_divmod(quot, rem, g, f);
        poly_copy(g, f);
        poly_copy(f, rem);
        /* a, new_a := new_a, a - quot*new_a */
        poly_mul(tmp0, quot, new_a);
        poly_add(tmp1, a, tmp0);
        poly_copy(a, new_a);
        poly_copy(new_a, tmp1);
        /* b, new_b := new_b, b - quot*new_b */
        poly_mul(tmp0, quot, new_b);
        poly_add(tmp1, b, tmp0);
        poly_copy(b, new_b);
        poly_copy(new_b, tmp1);
    }
}

void test_xgcd() {
    word_t f0[POLY_WORDS], f1[POLY_WORDS], g[POLY_WORDS];
    word_t a0[POLY_WORDS], a1[POLY_WORDS], b[POLY_WORDS];

    poly_rand(f0);
    poly_copy(f1, f0);
    poly_xgcd(f0, a0);
    poly_G(g);
    poly_xgcd_ref(f1, g, a1, b);
    assert(poly_eq(f0, f1));
    assert(poly_eq(a0, a1));
}

/* TODO: (this comes from src/poly.c)
    word_t f[POLY_WORDS], g[POLY_WORDS], h[POLY_WORDS];
    word_t h_inv[POLY_WORDS];
    index_t f_sparse[POLY_BITS], g_sparse[POLY_WEIGHT], h_sparse[POLY_WEIGHT];
    size_t f_weight;

    poly_gen_sparse(g_sparse, TYPE_POLY);
    poly_gen_sparse(h_sparse, TYPE_POLY);
    poly_to_dense(g, g_sparse);
    poly_to_dense(h, h_sparse);
    poly_mul(f, g, h);
    poly_to_sparse(f_sparse, &f_weight, f);

    printf("g: ");
    print_poly_sparse(g_sparse, POLY_WEIGHT);
    printf("\n");
    print_poly_dense(g, POLY_WORDS);
    printf("\nh: ");
    print_poly_sparse(h_sparse, POLY_WEIGHT);
    printf("\n");
    print_poly_dense(h, POLY_WORDS);
    printf("\nf = g*h:\n");
    print_poly_sparse(f_sparse, f_weight);
    printf("\n");
    print_poly_dense(f, POLY_WORDS);
    printf("\n");

    poly_copy(h_inv, h, POLY_WORDS);
    if (poly_inv(h_inv)) {
        printf("oops, h not invertible\n");
        return -1;
    }
    poly_mul(f, h, h_inv);

    printf("1/h: ");
    poly_to_sparse(f_sparse, &f_weight, h_inv);
    print_poly_sparse(f_sparse, f_weight);
    printf("\n");
    print_poly_dense(h_inv, POLY_WORDS);
    printf("\nf = h/h:\n");
    print_poly_dense(f, POLY_WORDS);
    printf("\n");
*/

int main(int argc, char *argv[])
{
    const unsigned int TEST_ROUNDS = 10;
    unsigned int i, test_rounds;

    if (argc < 2) {
        test_rounds = TEST_ROUNDS;
    } else {
        test_rounds = (unsigned int)(atoi(argv[1]));
    }

    {
        word_t f[POLY_WORDS], g[POLY_WORDS];
        word_t div[POLY_WORDS], mod[POLY_WORDS];
        index_t sp[POLY_BITS], weight;
        poly_zero(f);
        f[20] = 0xa4;
        f[50] = 0x01;
        poly_to_sparse(sp, &weight, f);
        print("f: "); print_polsp_wg(sp, weight); printf("\n");
        poly_zero(g);
        g[0] = 0x03;
        g[48] = 0x22;
        poly_to_sparse(sp, &weight, g);
        print("g: "); print_polsp_wg(sp, weight); printf("\n");
        poly_divmod(div, mod, f, g);
        poly_to_sparse(sp, &weight, div);
        print("div: "); print_polsp_wg(sp, weight); printf("\n");
        poly_to_sparse(sp, &weight, div);
        print("mod: "); print_polsp_wg(sp, weight); printf("\n");
    }


    for (i = 0; i < test_rounds; ++i) {
        test_mul_inv();
        test_compare();
        test_xgcd();
    }

    return EXIT_SUCCESS;
}
