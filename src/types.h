#ifndef NIEDERREITER_TYPES_H
#define NIEDERREITER_TYPES_H

/** \file types.h
 * Defines types that are used in the implementation
 */

#include "config.h"

/* TODO: allow for indices > 2^16 */
#ifndef INDEX_BITS
#define INDEX_BITS 16
#endif

/** \typedef limb_t  Chunks of the polynomial that fit in a register */
/** \typedef index_t Index of set bit in a sparse polynomial */

#ifdef SUPERCOP_BUILD

#ifndef LIMB_BITS
#define LIMB_BITS 64
#endif

#if   LIMB_BITS == 64
#include "crypto_uint64.h"
typedef crypto_uint64 limb_t;
#define LIMB_INDEX_BITS 6
#elif LIMB_BITS == 32
#include "crypto_uint32.h"
typedef crypto_uint32 limb_t;
#define LIMB_INDEX_BITS 5
#elif LIMB_BITS == 16
#include "crypto_uint16.h"
typedef crypto_uint16 limb_t;
#define LIMB_INDEX_BITS 4
#elif LIMB_BITS == 8
#include "crypto_uint8.h"
typedef crypto_uint8 limb_t;
#define LIMB_INDEX_BITS 3
#endif

#if INDEX_BITS == 16
#include "crypto_uint16.h"
typedef crypto_uint16 index_t;
#endif

#else /* !SUPERCOP_BUILD */

/* TODO: replace source of randomness */

#include <stdint.h>

#if   LIMB_BITS == 64
typedef uint64_t limb_t;
#define LIMB_INDEX_BITS 6
#elif LIMB_BITS == 32
typedef uint32_t limb_t;
#define LIMB_INDEX_BITS 5
#elif LIMB_BITS == 16
typedef uint16_t limb_t;
#define LIMB_INDEX_BITS 4
#elif LIMB_BITS == 8
typedef uint8_t limb_t;
#define LIMB_INDEX_BITS 3
#endif

#if INDEX_BITS == 16
typedef uint16_t index_t;
#endif

#endif /* SUPERCOP_BUILD */


#define TO_BYTES(bits) (((bits) + 7) / 8)
#define TO_LIMBS(bits) (((bits) + LIMB_BITS - 1) / LIMB_BITS)
#define TO_MASK(bits)  ((limb_t)((1 << bits) - 1))

#define TAIL_BITS (POLY_BITS % LIMB_BITS)
#define TAIL_MASK TO_MASK(TAIL_BITS)
#define TAIL_BYTES TO_BYTES(TAIL_BITS)

#define LIMB_BYTES TO_BYTES(LIMB_BITS)
#define INDEX_BYTES TO_BYTES(INDEX_BITS)

#define POLY_BYTES TO_BYTES(POLY_BITS)
#define POLY_LIMBS TO_LIMBS(POLY_BITS)


/** Dense polynomial type (fixed size array of limbs) */
typedef limb_t poly_t[POLY_LIMBS];

/** Sparse polynomial type (fixed weight array of indices) */
typedef index_t sp_poly_t[POLY_WEIGHT];

/** Systematic parity-check type (fixed size array of dense polynomials) */
typedef poly_t sys_par_ch_t[POLY_COUNT - 1];

/** Sparse error type (fixed weight array of indices) */
typedef index_t sp_error_t[ERROR_WEIGHT];

/** Error type (fixed size array of dense polynomials) */
typedef poly_t error_t[POLY_COUNT];

/** Syndrome type (alias for poly_t) */
typedef poly_t syn_t;

/** Parity-check type (fixed size array of sparse polynomials) */
typedef sp_poly_t par_ch_t[POLY_COUNT];

/** Dense parity-check type (fixed size array of dense polynomials) */
typedef poly_t dense_par_ch_t[POLY_COUNT];

#endif /* NIEDERREITER_TYPES_H */
