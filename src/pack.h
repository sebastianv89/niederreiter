#ifndef NIEDERREITER_PACK_H
#define NIEDERREITER_PACK_H

/** \file pack.h
 * Convert internally typed data to byte arrays (in network byte order).
 */

#include "config.h"
#include "types.h"

#define PUBLIC_KEY_BYTES  ((POLY_COUNT - 1) * POLY_BYTES)
#define PRIVATE_KEY_BYTES (POLY_COUNT * POLY_WEIGHT * INDEX_BYTES)
#define ERROR_BYTES       (POLY_COUNT * POLY_BYTES)
#define SYNDROME_BYTES    POLY_BYTES

void pack_poly(unsigned char *dest, const poly_t src);
void unpack_poly(poly_t dest, const unsigned char *src);

void pack_pubkey(unsigned char *dest, const sys_par_ch_t src);
void unpack_pubkey(sys_par_ch_t dest, const unsigned char *src);

void pack_err(unsigned char *dest, const error_t src);

void pack_privkey(unsigned char *dest, const par_ch_t src);
void unpack_privkey(par_ch_t dest, const unsigned char *src);


#endif /* NIEDERREITER_PACK_H */
