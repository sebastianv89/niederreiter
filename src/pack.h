#ifndef NIEDERREITER_PACK_H
#define NIEDERREITER_PACK_H

/** \file pack.h
 * Convert internally typed data to byte arrays in network byte order
 * (big endian).
 */

#include "config.h"

#define ERROR_BYTES       (NUMBER_OF_POLYS * POLY_BYTES)
#define PUBLIC_KEY_BYTES  ((NUMBER_OF_POLYS - 1) * POLY_BYTES)
#define PRIVATE_KEY_BYTES (NUMBER_OF_POLYS * POLY_WEIGHT * INDEX_BYTES)

void pack_pub_key(unsigned char *dest, const word_t (*src)[POLY_WORDS]);
void unpack_pub_key(word_t (*dest)[POLY_WORDS], const unsigned char *src);

void pack_priv_key(unsigned char *dest, const index_t (*src)[POLY_WEIGHT]);
void unpack_priv_key(index_t (*dest)[POLY_WEIGHT], const unsigned char *src);

void pack_syndrome(unsigned char *dest, const word_t *src);
void unpack_syndrome(word_t *dest, const unsigned char *src);

void pack_error(unsigned char *dest, const word_t (*src)[POLY_WORDS]);

#endif /* NIEDERREITER_PACK_H */
