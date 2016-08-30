#include <inttypes.h>

#include "debug.h"
#include "config.h"

/* f := g
 * assumes f has enough memory allocated. sets weight.
 */
void poly_to_sparse(index_t *f, size_t *weight, const word_t *g) {
    size_t word;
    index_t bit;
    word_t g_word;

    *weight = 0;
    for (word = 0; word < POLY_WORDS; ++word) {
        bit = (index_t)(word * WORD_BITS);
        g_word = g[word];
        while (g_word > 0) {
            if (g_word & 1) {
                f[*weight] = bit;
                *weight += 1;
            }
            g_word >>= 1;
            bit++;
        }
    }
}

void print_poly_dense(const word_t *f, size_t words) {
    size_t i;
    for (i = 0; i < words-1; ++i) {
        printf("%016" PRIx64 ", ", f[i]);
    }
    printf("%016" PRIx64, f[i]);
}

void print_poly_sparse(const index_t *f, size_t indices) {
    size_t i;

    for (i = 0; i < indices-1; ++i) {
        printf("%" PRIu16 ", ", f[i]);
    }
    printf("%" PRIu16, f[i]);
}

void print_bytes(const unsigned char *buf, size_t bytes)
{
    size_t i;
    for (i = 0; i < bytes; ++i) {
        printf("%02hhx", buf[i]);
    }
}
