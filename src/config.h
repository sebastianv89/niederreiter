#ifndef NIEDERREITER_CONFIG_H
#define NIEDERREITER_CONFIG_H

#include "endian.h"

#define NUMBER_OF_POLYS 2    /**< n_0   */
#define POLY_BITS       4801 /**< r     */
#define POLY_WEIGHT     45   /**< w/n_0 */
#define ERROR_WEIGHT    84   /**< t     */

#define THRESHOLDS {29, 27, 25, 24, 23, 23} /**< T */
#define ITERATIONS 6                        /**< length(THRESHOLDS) */

#define RAND_BUFFER_SIZE 120

/* TODO: dynamic source of randomness */

#define POLY_INDEX_BITS  13 /**< ceil(log_2(POLY_BITS)) */
#define ERROR_INDEX_BITS 14 /**< ceil(log_2(ERROR_BITS)) */
#define INDEX_BITS 16

/* Check the index bits values */
#if   1 << POLY_INDEX_BITS < POLY_BITS || POLY_BITS < 1 << (POLY_INDEX_BITS - 1)
#error Invalid value for POLY_INDEX_BITS
#elif 1 << ERROR_INDEX_BITS < (NUMBER_OF_POLYS * POLY_BITS) \
    || (NUMBER_OF_POLYS * POLY_BITS) < 1 << (ERROR_INDEX_BITS - 1)
#error Invalid value for ERROR_INDEX_BITS
#elif INDEX_BITS < POLY_INDEX_BITS || INDEX_BITS < ERROR_INDEX_BITS
#error Invalid value for INDEX_BITS
#endif

#define ERROR_BITS (NUMBER_OF_POLYS * POLY_BITS)

#define TO_BYTES(bits) (((bits) + 7) / 8)

#define INDEX_BYTES TO_BYTES(INDEX_BITS)
#define POLY_BYTES TO_BYTES(POLY_BITS)

#if defined(SUPERCOP_BUILD)

#if   WORD_BITS == 64
#include "crypto_uint64.h"
#define WORD_C(w) w##ULL
#elif WORD_BITS == 32
#include "crypto_uint32.h"
#define WORD_C(w) w##UL
#elif WORD_BITS == 16
#include "crypto_uint16.h"
#define WORD_C(w) w##U
#elif WORD_BITS == 8
#include "crypto_uint8.h"
#define WORD_C(w) w##U
#else
#error Specified invalid size for WORD_BITS
#endif

#if INDEX_BITS == 16
#include "crypto_uint16.h"
#define INDEX_C(w) w##U
#else
#error No implementation (yet) for INDEX_BITS != 16
#endif

#define TYPE_T(b) crypto_uint##b

#else /* ! defined(SUPERCOP_BUILD) */

#include <stdint.h>
#define TYPE_T(b) uint##b##_t
#define TYPE_T_C(b, w) UINT##b##_C(w)
#define TYPE_C(b, w) TYPE_T_C(b, w)
#define WORD_C(w) TYPE_C(WORD_BITS, w)

#if INDEX_BITS == 16
#define INDEX_C(w) UINT16_C(w)
#else
#error No implementation (yet) for INDEX_BITS != 16
#endif

#endif /* defined(SUPERCOP_BUILD) */

#define TYPE(b) TYPE_T(b)
#define TO_WORDS(bits) (((bits) + WORD_BITS - 1) / WORD_BITS)

typedef TYPE(WORD_BITS) word_t;
typedef TYPE(INDEX_BITS) index_t;

/* Derived word parameters */

#if   WORD_BITS == 64
#define WORD_INDEX_BITS 6
#elif WORD_BITS == 32
#define WORD_INDEX_BITS 5
#elif WORD_BITS == 16
#define WORD_INDEX_BITS 4
#elif WORD_BITS == 8
#define WORD_INDEX_BITS 3
#else
#error Specified invalid size for WORD_BITS
#endif

#if 1 << WORD_INDEX_BITS != WORD_BITS
#error Incorrect value for WORD_INDEX_BITS
#endif
#define WORD_INDEX_MASK ((INDEX_C(1) << WORD_INDEX_BITS) - 1)

#define TAIL_BITS (POLY_BITS % WORD_BITS)
#define TAIL_BYTES TO_BYTES(TAIL_BITS)
#define TAIL_MASK ((WORD_C(1) << TAIL_BITS) - 1)

#define WORD_BYTES TO_BYTES(WORD_BITS)
#define POLY_WORDS TO_WORDS(POLY_BITS)

#define ERROR_BYTES TO_BYTES(ERROR_BITS)
#define ERROR_WORDS TO_WORDS(ERROR_BITS)

#define SYNDROME_WORDS POLY_WORDS
#define SYNDROME_BYTES POLY_BYTES

#define PUBLIC_KEY_WORDS ((NUMBER_OF_POLYS - 1) * POLY_WORDS)
#define PUBLIC_KEY_BYTES  ((NUMBER_OF_POLYS - 1) * POLY_BYTES)

#define PRIVATE_KEY_WEIGHT (NUMBER_OF_POLYS * POLY_WEIGHT)
#define PRIVATE_KEY_BYTES (PRIVATE_KEY_WEIGHT * INDEX_BYTES)

#include "debug.h" /* TODO: remove after debugging */

#endif /* NIEDERREITER_CONFIG_H */
