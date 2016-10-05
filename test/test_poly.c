#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <assert.h>

#include "poly.h"
#include "sp_poly.h"
#include "config.h"
#include "util.h"

static unsigned int verbose = 0;

void test_mul_inv() {
    poly_t f, g, h;
    sp_poly_t gs;

    sp_poly_rand(gs);
    sp_poly_to_poly(g, gs);
    poly_copy(h, g);
    if (poly_inv(g)) {
        char str[TEST_STR_CHARS];
        sp_poly_to_str(str, gs);
        fprintf(stderr, "Trivial pass, g non-invertible: %s\n", str);
    }
    poly_mul(f, g, h);
    assert(poly_verify_one(f) == 0);
}

void test_compare() {
    poly_t f, g;
    limb_t eq, lt;
    bool ref_eq, res;
    index_t df, dg;

    poly_rand(f);
    poly_rand(g);
    ref_eq = poly_eq(f, g);
    df = poly_degree(f);
    dg = poly_degree(g);
    poly_compare(&eq, &lt, f, g);
    res = eq == (limb_t)(ref_eq ? -1 : 0);
    if (df < dg) {
        res &= (lt == (limb_t)(-1));
    }
    poly_compare(&eq, &lt, f, f);
    res &= (eq == (limb_t)(-1));
    assert(res);
}

void poly_xgcd_ref(poly_t f, poly_t g, poly_t a, poly_t b) {
    poly_t new_a = {0}, new_b = {1};
    poly_t div, mod, tmp0, tmp1;
    assert(!poly_verify_one(a) && !poly_verify_zero(b));

    while (poly_verify_zero(g)) {
        /* g, f := f, g % f */
        poly_divmod(div, mod, g, f);
        poly_copy(g, f);
        poly_copy(f, mod);
        /* a, new_a := new_a, a - quot*new_a */
        poly_mul(tmp0, div, new_a);
        poly_add(tmp1, a, tmp0);
        poly_copy(a, new_a);
        poly_copy(new_a, tmp1);
        /* b, new_b := new_b, b - quot*new_b */
        poly_mul(tmp0, div, new_b);
        poly_add(tmp1, b, tmp0);
        poly_copy(b, new_b);
        poly_copy(new_b, tmp1);
    }
}

void test_xgcd() {
    poly_t f0, f1, g = {1};
    poly_t a0 = {1}, a1 = {1}, b = {0};
    g[POLY_LIMBS - 1] = 1 << TAIL_BITS; /* g := G */

    poly_rand(f0);
    poly_copy(f1, f0);
    poly_xgcd(f0, a0);
    poly_xgcd_ref(f1, g, a1, b);

    assert(poly_eq(f0, f1) && poly_eq(a0, a1));
}

void do_tests(unsigned int rounds) {
    unsigned int i;
    for (i = 0; i < rounds; ++i) {
        test_mul_inv();
        test_compare();
        test_xgcd();
    }
}

int main(int argc, char *argv[]) {
    int opt;
    unsigned int rounds = 10;

    while ((opt = getopt(argc, argv, "vn:")) != -1) {
        switch (opt) {
        case 'n': rounds = (unsigned int)(atoi(optarg)); break;
        case 'v': verbose += 1; break;
        }
    }

    do_tests(rounds);

    return EXIT_SUCCESS;
}
