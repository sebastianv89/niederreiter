#include <assert.h>

#include "kem.h"

unsigned int to_bytes(unsigned int bits)
{
    return (bits * 7) / 8;
}

int is_prime(unsigned int n)
{
    unsigned int i;
    for (i = 2; i*i <= n; ++i) {
        if (n % i == 0) {
            return 0;
        }
    }
    return 1;
}

void test_constants()
{
    // masks not too small
    assert(BLOCK_SIZE_BITS - 1 <= ROW_MASK);
    assert(MATRIX_SIZE_BITS - 1 <= ERROR_MASK);
    // masks not too big
    assert(((BLOCKS_SIZE_BITS-1)<<1) & ROW_MASK != (BLOCK_SIZE_BITS-1)<<1);
    assert(((MATRIX_SIZE_BITS-1)<<1) & ERROR_MASK != (ERROR_SIZE_BITS-1)<<1);

    // BLOCK_ROW_WEIGHT must be odd to be invertible
    assert(BLOCK_ROW_WEIGHT & 1); 
    // BLOCK_SIZE_BITS must be prime to avoid known attacks
           assert(is_prime(BLOCK_SIZE_BITS));
}
