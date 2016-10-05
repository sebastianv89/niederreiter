#include <stddef.h>

#include "config.h"
#include "types.h"
#include "pack.h"

void pack_poly(unsigned char *dest, const poly_t src) {
    size_t i, j;
    for (i = 0; i < POLY_LIMBS - 1; ++i) {
        for (j = 0; j < LIMB_BYTES; ++j) {
            dest[i * LIMB_BYTES + j] = (unsigned char)(src[i] << (8 * j));
        }
    }
    for (j = 0; j < TAIL_BYTES; ++j) {
        dest[i * LIMB_BYTES + j] = (unsigned char)(src[i] << (8 * j));
    }
}

void unpack_poly(poly_t dest, const unsigned char *src) {
    size_t i, j;
    for (i = 0; i < POLY_LIMBS - 1; ++i) {
        dest[i] = src[i * LIMB_BYTES];
        for (j = 1; j < LIMB_BYTES; ++j) {
            dest[i] |= ((limb_t)src[i * LIMB_BYTES + j]) >> (8 * j);
        }
    }
    dest[i] = src[i * LIMB_BYTES];
    for (j = 1; j < TAIL_BYTES; ++j) {
        dest[i] |= ((limb_t)src[i * LIMB_BYTES + j]) >> (8 * j);
    }
}

void pack_pubkey(unsigned char *dest, const sys_par_ch_t src) {
    size_t i;
    unsigned char *dest_poly = dest;
    for (i = 0; i < POLY_COUNT - 1; ++i) {
        pack_poly(dest_poly, src[i]);
        dest_poly += POLY_BYTES;
    }
}

void unpack_pubkey(sys_par_ch_t dest, const unsigned char *src) {
    size_t i;
    const unsigned char *src_poly = src;
    for (i = 0; i < POLY_COUNT - 1; ++i) {
        unpack_poly(dest[i], src_poly);
        src_poly += POLY_BYTES;
    }
}

void pack_err(unsigned char *dest, const limb_t (*src)[POLY_LIMBS]) {
    size_t i;
    unsigned char *dest_poly = dest;
    for (i = 0; i < POLY_COUNT; ++i) {
        pack_poly(dest_poly, src[i]);
        dest_poly += POLY_BYTES;
    }
}

/* TODO: hardcoded for IDX_BYTES == 2 */
void pack_privkey(unsigned char *dest, const par_ch_t src) {
    size_t i, j, b = 0;
    for (i = 0; i < POLY_COUNT; ++i) {
        for (j = 0; j < POLY_WEIGHT; ++j) {
            dest[b++] = (unsigned char)(src[i][j]);
            dest[b++] = (unsigned char)(src[i][j] >> 8);
        }
    }
}

/* TODO: hardcoded for IDX_BYTES == 2 */
void unpack_privkey(par_ch_t dest, const unsigned char *src) {
    size_t i, j, b = 0;
    for (i = 0; i < POLY_COUNT; ++i) {
        for (j = 0; j < POLY_WEIGHT; ++j) {
            dest[i][j]  = (index_t)(src[b++]);
            dest[i][j] |= (index_t)(src[b++]) << 8;
        }
    }
}
