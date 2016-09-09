#include <stddef.h>

#include "pack.h"
#include "config.h"

void pack_words(unsigned char *dest, const word_t *src, size_t bytes) {
    size_t byte;
    size_t word = 0;
    unsigned int shift = 0;

    /* TODO: unroll for specific word sizes */
    for (byte = 0; byte < bytes; ++byte) {
        dest[byte] = (unsigned char)(src[word] >> shift);
        shift += 8;
        if (shift == WORD_BITS) {
            shift = 0;
            ++word;
        }
    }
}

void unpack_words(word_t *dest, const unsigned char *src, size_t bytes) {
    size_t byte;
    size_t word = 0;
    unsigned int shift = 0;

    /* TODO: unroll for specific word sizes */
    for (byte = 0; byte < bytes; ++byte) {
        if (shift == 0) {
            dest[word] = (word_t)src[byte];
        } else {
            dest[word] |= (word_t)src[byte] << shift;
        }
        shift += 8;
        if (shift == WORD_BITS) {
            shift = 0;
            ++word;
        }
    }
}

void pack_pub_key(unsigned char *dest, const word_t (*src)[POLY_WORDS]) {
    size_t i;
    unsigned char *dest_poly;
    for (i = 0, dest_poly = dest; i < NUMBER_OF_POLYS - 1; ++i, dest_poly += POLY_BYTES) {
        pack_words(dest_poly, src[i], POLY_BYTES);
    }
}

void unpack_pub_key(word_t (*dest)[POLY_WORDS], const unsigned char *src) {
    size_t i;
    const unsigned char *src_poly;
    for (i = 0, src_poly = src; i < NUMBER_OF_POLYS - 1; ++i, src_poly += POLY_BYTES) {
        unpack_words(dest[i], src_poly, POLY_BYTES);
    }
}

void pack_syndrome(unsigned char *dest, const word_t *src) {
    pack_words(dest, src, POLY_BYTES);
}

void unpack_syndrome(word_t *dest, const unsigned char *src) {
    unpack_words(dest, src, POLY_BYTES);
}

void pack_error(unsigned char *dest, const word_t (*src)[POLY_WORDS]) {
    size_t i;
    unsigned char *dest_poly;
    for (i = 0, dest_poly = dest; i < NUMBER_OF_POLYS; ++i, dest_poly += POLY_BYTES) {
        pack_words(dest_poly, src[i], POLY_BYTES);
    }
}

/* Hard coded for INDEX_BYTES == 2 */
void pack_indices(unsigned char *dest, const index_t *src, size_t weight) {
    size_t i;
    for (i = 0; i < weight/2; ++i) {
        dest[2*i]   = (unsigned char)(src[i]);
        dest[2*i+1] = (unsigned char)(src[i] >> 8);
    }
}

/* Hard coded for INDEX_BYTES == 2 */
void unpack_indices(index_t *dest, const unsigned char *src, size_t weight) {
    size_t i;
    for (i = 0; i < weight/2; ++i) {
        dest[i] = (index_t)(src[2*i] | ((index_t)src[2*i+1] << 8));
    }
}

void pack_priv_key(unsigned char *dest, const index_t (*src)[POLY_WEIGHT]) {
    size_t i;
    unsigned char *dest_poly;
    for (i = 0, dest_poly = dest; i < NUMBER_OF_POLYS; ++i, dest_poly += POLY_WEIGHT) {
        pack_indices(dest_poly, src[i], POLY_WEIGHT);
    }
}

void unpack_priv_key(index_t (*dest)[POLY_WEIGHT], const unsigned char *src) {
    size_t i;
    const unsigned char *src_poly;
    for (i = 0, src_poly = src; i < NUMBER_OF_POLYS; ++i, src_poly += POLY_WEIGHT) {
        unpack_indices(dest[i], src_poly, POLY_WEIGHT);
    }
}
