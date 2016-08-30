#ifndef NIEDERREITER_PACK_H
#define NIEDERREITER_PACK_H

#include "config.h"

void pack_pub_key(unsigned char *bytes, const word_t *words);
void unpack_pub_key(word_t *words, const unsigned char *bytes);

void pack_priv_key(unsigned char *bytes, const index_t *indices);
void unpack_priv_key(index_t *indices, const unsigned char *bytes);

void pack_syndrome(unsigned char *bytes, const word_t *words);
void unpack_syndrome(word_t *words, const unsigned char *bytes);

void pack_error(unsigned char *bytes, const word_t *words);

#endif /* NIEDERREITER_PACK_H */
