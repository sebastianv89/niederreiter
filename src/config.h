#ifndef NIEDERREITER_CONFIG_H
#define NIEDERREITER_CONFIG_H

/**
 * \file config.h
 * \brief System parameters.
 * 
 * Defines the parameters for both the cryptosystem and the particular
 * implementation.
 * Some parameters can be changed and many will be
 * derived automatically, but one should check upon changing
 * parameters that the rest makes sense.
 */

#define NUMBER_OF_POLYS 2
#define POLY_BITS 4801
#define POLY_WEIGHT 45
#define ERROR_WEIGHT 84

#define THRESHOLDS {29, 27, 25, 24, 23, 23}
#define ITERATIONS 6


#define INDEX_BITS 16


#if defined(OQS_BUILD)
/* TODO: replace source of randomness */
#include <stdint.h>

#if   WORD_BITS == 64
typedef uint64_t word_t;
#define WORD_C(w) UINT64_C(w)
#define WORD_INDEX_BITS 6
#elif WORD_BITS == 32
typedef uint32_t word_t;
#define WORD_C(w) UINT32_C(w)
#define WORD_INDEX_BITS 5
#elif WORD_BITS == 16
typedef uint16_t word_t;
#define WORD_C(w) UINT16_C(w)
#define WORD_INDEX_BITS 4
#elif WORD_BITS == 8
typedef uint8_t word_t;
#define WORD_C(w) UINT8_C(w)
#define WORD_INDEX_BITS 3
#endif

#if INDEX_BITS == 16
typedef uint16_t index_t;
#define INDEX_C(w) UINT16_C(w)
#endif

#else /* !defined(OQS_BUILD) */

#if   WORD_BITS == 64
#include "crypto_uint64.h"
typedef crypto_uint64 word_t;
#define WORD_C(w) w##ULL
#define WORD_INDEX_BITS 6
#elif WORD_BITS == 32
#include "crypto_uint32.h"
typedef crypto_uint32 word_t;
#define WORD_C(w) w##UL
#define WORD_INDEX_BITS 5
#elif WORD_BITS == 16
#include "crypto_uint16.h"
typedef crypto_uint16 word_t;
#define WORD_C(w) w##U
#define WORD_INDEX_BITS 4
#elif WORD_BITS == 8
#include "crypto_uint8.h"
typedef crypto_uint8 word_t;
#define WORD_C(w) w##U
#define WORD_INDEX_BITS 3
#endif

#if INDEX_BITS == 16
#include "crypto_uint16.h"
typedef crypto_uint16 index_t;
#define INDEX_C(w) w##U
#endif

#endif


#define TO_BYTES(bits) (((bits) + 7) / 8)
#define TO_WORDS(bits) (((bits) + WORD_BITS - 1) / WORD_BITS)

#define TAIL_BITS (POLY_BITS % WORD_BITS)
#define TAIL_MASK ((word_t)((WORD_C(1) << TAIL_BITS) - 1))
#define TAIL_BYTES TO_BYTES(TAIL_BITS)

#define WORD_BYTES TO_BYTES(WORD_BITS)
#define INDEX_BYTES TO_BYTES(INDEX_BITS)

#define POLY_BYTES TO_BYTES(POLY_BITS)
#define POLY_WORDS TO_WORDS(POLY_BITS)

#include "debug.h" /* TODO: remove after debugging */

#endif /* NIEDERREITER_CONFIG_H */
