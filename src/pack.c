#include <string.h>

#include "pack.h"
#include "config.h"

void pack_words(unsigned char *bytes, const word_t *words, size_t len)
{
#if defined(LITTLE_ENDIAN)
    size_t byte;
    size_t word = 0;
    unsigned int shift = 0;

    /* TODO: unroll for specific word sizes */
    for (byte = 0; byte < len; ++byte) {
        bytes[byte] = (unsigned char)(words[word] >> shift);
        shift += 8;
        if (shift == WORD_BITS) {
            shift = 0;
            ++word;
        }
    }
#else
    memcpy(bytes, words, len);
#endif
}
void unpack_words(word_t *words, const unsigned char *bytes, size_t len)
{
#if defined(LITTLE_ENDIAN)
    size_t byte;
    size_t word = 0;
    unsigned int shift = 0;

    /* TODO: unroll for specific word sizes */
    for (byte = 0; byte < len; ++byte) {
        if (shift == 0) {
            words[word] = bytes[byte];
        } else {
            words[word] |= (word_t)bytes[byte] << shift;
        }
        shift += 8;
        if (shift == WORD_BITS) {
            shift = 0;
            ++word;
        }
    }
#else
    memcpy(words, bytes, len); 
#endif
}

void pack_pub_key(unsigned char *bytes, const word_t *words)
{
    pack_words(bytes, words, PUBLIC_KEY_BYTES);
}

void unpack_pub_key(word_t *words, const unsigned char *bytes)
{
    unpack_words(words, bytes, PUBLIC_KEY_BYTES);
    words[PUBLIC_KEY_WORDS - 1] &= TAIL_MASK;
}

void pack_syndrome(unsigned char *bytes, const word_t *words)
{
    pack_words(bytes, words, POLY_BYTES);
}

void unpack_syndrome(word_t *words, const unsigned char *bytes)
{
    unpack_words(words, bytes, POLY_BYTES);
    words[SYNDROME_WORDS - 1] &= TAIL_MASK;
}

/* Hardcoded for INDEX_BYTES == 2 */
void pack_indices(unsigned char *bytes, const index_t *indices, size_t len)
{
#if defined(LITTLE_ENDIAN)
    size_t i;
    
    for (i = 0; i < len/2; ++i) {
        bytes[2*i]   = (unsigned char)(indices[i]);
        bytes[2*i+1] = (unsigned char)(indices[i] >> 8);
    }
#else
    memcpy(bytes, indices, len);
#endif
}

/* Hardcoded for INDEX_BYTES == 2 */
void unpack_indices(index_t *indices, const unsigned char *bytes, size_t len)
{
#if defined(LITTLE_ENDIAN)
    size_t i;
    
    for (i = 0; i < len/2; ++i) {
        indices[i] = (index_t)(bytes[2*i] | ((index_t)bytes[2*i+1] << 8));
    }
#else
    memcpy(indices, bytes, len);
#endif
}

void pack_priv_key(unsigned char *bytes, const index_t *indices)
{
    pack_indices(bytes, indices, PRIVATE_KEY_BYTES);
}

void unpack_priv_key(index_t *indices, const unsigned char *bytes)
{
    unpack_indices(indices, bytes, PRIVATE_KEY_BYTES);
}

void pack_error(unsigned char *bytes, const word_t *words)
{
    pack_words(bytes, words, ERROR_BYTES);
}
