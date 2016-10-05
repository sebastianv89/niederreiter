#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>

#include "config.h"
#include "sp_poly.h"
#include "util.h"

static unsigned int verbose = 0;

void test_to_dense() {
    poly_t f;
    sp_poly_t g0, g1;
    sp_poly_rand(g0);
    sp_poly_to_poly(f, g0);
    poly_to_sp_poly(g1, f);
    assert(sp_poly_eq(g0, g1));
}

void test_double_transpose() {
    sp_poly_t f, g;
    sp_poly_rand(f);
    sp_poly_copy(g, f);
    sp_poly_transpose(f, f);
    sp_poly_transpose(f, f);
    assert(sp_poly_eq(f, g));
}

void do_tests(unsigned int rounds) {
    unsigned int i;
    for (i = 0; i < rounds; ++i) {
        test_to_dense();
        test_double_transpose();
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
