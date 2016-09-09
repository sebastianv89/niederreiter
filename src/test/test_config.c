#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

#include "config.h"
#include "error.h"
#include "poly_sparse.h"
#include "test/util.h"

bool is_prime(uintmax_t n) {
    uintmax_t i;
    for (i = 2; i*i <= n; ++i) {
        if (n % i == 0) {
            return false;
        }
    }
    return true;
}

void test_index_bits() {
    assert(TO_BYTES(ERROR_INDEX_BITS) == INDEX_BITS);
    assert(TO_BYTES(POLY_INDEX_BITS) == INDEX_BITS);
}

int main(void)
{
    assert(is_prime(POLY_BITS));
    test_index_bits();

    return 0;
}
