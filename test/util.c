#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <inttypes.h>
#include <assert.h>

#include "randombytes.h"

#include "util.h"
#include "config.h"
#include "pack.h"
#include "sp_poly.h"
#include "error.h"
#include "poly.h"

void pack_sp_error(unsigned char *dest, sp_error_t src) {
    size_t i, b = 0;
    for (i = 0; i < ERROR_WEIGHT; ++i) {
        dest[b++] = (unsigned char)(src[i]);
        dest[b++] = (unsigned char)(src[i] >> 8);
    }
}

void unpack_sp_error(sp_error_t dest, unsigned char *src) {
    size_t i, b = 0;
    for (i = 0; i < ERROR_WEIGHT; ++i) {
        dest[i]  = (unsigned char)(src[b++]);
        dest[i] |= (unsigned char)(src[b++] << 8);
    }
}

void poly_rand(poly_t f) {
    randombytes((unsigned char *)f, POLY_LIMBS * LIMB_BYTES);
    f[POLY_LIMBS - 1] &= TAIL_MASK;
}

bool poly_eq(const poly_t f, const poly_t g) {
    size_t i;
    for (i = 0; i < POLY_LIMBS; ++i) {
        if (f[i] != g[i]) {
            return false;
        }
    }
    return true;
}

int index_cmp(const void *x, const void *y) {
    if (*(index_t *)x < *(index_t *)y) {
        return -1;
    } else if (*(index_t *)x == *(index_t *)y) {
        return 0;
    }
    return 1;
}

void sp_poly_sort(sp_poly_t f) {
    qsort(f, POLY_WEIGHT, INDEX_BYTES, index_cmp);
}

bool sp_poly_eq(const sp_poly_t f, const sp_poly_t g) {
    size_t i;
    sp_poly_t fs, gs;
    sp_poly_copy(fs, f);
    sp_poly_copy(gs, g);
    sp_poly_sort(fs);
    sp_poly_sort(gs);
    for (i = 0; i < POLY_WEIGHT; ++i) {
        if (fs[i] != gs[i]) {
            return false;
        }
    }
    return true;
}

bool err_eq(const error_t f, const error_t g) {
    size_t i;
    for (i = 0; i < POLY_COUNT; ++i) {
        if (!poly_eq(f[i], g[i])) {
            return false;
        }
    }
    return true;
}

index_t poly_degree(const poly_t f) {
    unsigned char bit;
    size_t i = POLY_LIMBS - 1;
    bit = TAIL_BITS + 1;
    while (bit > 0) {
        --bit;
        if ((f[i] >> bit) & 1) {
            return (index_t)(i * LIMB_BITS + bit);
        }
    }
    while (i > 0) {
        --i;
        bit = LIMB_BITS;
        while (bit > 0) {
            --bit;
            if ((f[i] >> bit) & 1) {
                return (index_t)(i * LIMB_BITS + bit);
            }
        }
    }
    return (index_t)(-1);
}

void poly_mulxn(poly_t f, const poly_t g, index_t n) {
    static const unsigned char LIMB_INDEX_MASK = (1 << LIMB_INDEX_BITS) - 1;
    unsigned char bit_shift;
    size_t i;

    bit_shift = n & LIMB_INDEX_MASK;
    n >>= LIMB_INDEX_BITS;

    for (i = 0; i < n; ++i) {
        f[i] = 0;
    }
    f[i++] = g[0] << bit_shift;
    for ( ; i < POLY_LIMBS; ++i) {
        index_t prev = (index_t)(i-n-1);
        index_t curr = (index_t)(i-n);
        if (bit_shift == 0) {
            f[i] = g[curr];
        } else {
            f[i] = (g[prev] >> (LIMB_BITS - bit_shift))
                 | (g[curr] << bit_shift);
        }
    }
}

void poly_xn(poly_t f, index_t n) {
    poly_t g = {1};
    poly_mulxn(f, g, n);
}

void poly_divmod(poly_t div, poly_t mod, const poly_t f, const poly_t g) {
    index_t dmod, dg;
    
    poly_zero(div);

    poly_copy(mod, f);
    dg = poly_degree(g);
    for (dmod = poly_degree(mod); dg <= dmod; dmod = poly_degree(mod)) {
        poly_t gs, xn;
        poly_xn(xn, dmod - dg);
        poly_mulxn(gs, g, dmod - dg);
        poly_add(mod, mod, gs);
        poly_add(div, div, xn);
    }
}

void poly_to_sp_poly(sp_poly_t f, const poly_t g) {
    size_t i;
    index_t wg = 0;

    assert(poly_hamming_weight(g) == POLY_WEIGHT);

    for (i = 0; i < POLY_LIMBS; ++i) {
        index_t bit = (index_t)(i * LIMB_BITS);
        limb_t gi;
        for (gi = g[i]; gi > 0; gi >>= 1) {
            if (gi & 1) {
                f[wg++] = bit;
            }
            ++bit;
        }
    }
}

void error_to_sp_error(sp_error_t f, const error_t err) {
    size_t i, j;
    index_t wg = 0;

    for (i = 0; i < POLY_COUNT; ++i) {
        wg += poly_hamming_weight(err[i]);
    }
    assert(wg == ERROR_WEIGHT);
    wg = 0;

    for (i = 0; i < POLY_COUNT; ++i) {
        for (j = 0; j < POLY_LIMBS; ++j) {
            index_t bit = (index_t)(i * POLY_LIMBS + j * LIMB_BITS);
            limb_t eij;
            for (eij = err[i][j]; eij > 0; eij >>= 1) {
                if (eij & 1) {
                    f[wg++] = bit;
                }
            }
            ++bit;
        }
    }
}

#if   LIMB_BITS == 8
#define PRILIMB PRIx8
#elif LIMB_BITS == 16
#define PRILIMB PRIx16
#elif LIMB_BITS == 32
#define PRILIMB PRIx32
#elif LIMB_BITS == 64
#define PRILIMB PRIx64
#endif

#define PRIINDEX PRIu16

#define CHARS_PER_BYTE 5
#define CHARS_PER_INDEX 8


int poly_to_str(char *str, const poly_t f) {
    int i, n = 0;
    for (i = 0; i < POLY_LIMBS - 1; ++i) {
        n += sprintf(str + n, "%0*" PRILIMB ", ", LIMB_BITS/4, f[i]);
    }
    n += sprintf(str + n, "%0*" PRILIMB " (hw = %lu)", LIMB_BITS/4, f[i], poly_hamming_weight(f));
    return n;
}

int sp_poly_to_str(char *str, const sp_poly_t f) {
    int i, n = 0;
    for (i = 0; i < POLY_WEIGHT - 1; ++i) {
        n += sprintf(str + n, "%" PRIINDEX ", ", f[i]);
    }
    n += sprintf(str + n, "%" PRIINDEX, f[i]);
    return n;
}

int error_to_str(char *str, const error_t err) {
    int i, n = 0;
    for (i = 0; i < POLY_COUNT - 1; ++i) {
        n += poly_to_str(str + n, err[i]);
        n += sprintf(str + n, "; ");
    }
    n += poly_to_str(str + n, err[i]);
    return n;
}

int sp_error_to_str(char *str, const sp_error_t err) {
    int i, n = 0;
    for (i = 0; i < ERROR_WEIGHT - 1; ++i) {
        n += sprintf(str + n, "%" PRIINDEX ", ", err[i]);
    }
    n += sprintf(str + n, "%" PRIINDEX, err[i]);
    return n;
}

int bytes_to_hex(char *str, const unsigned char *buf, int byte_count) {
    int i, n = 0;
    for (i = 0; i < byte_count; ++i) {
        n += sprintf(str, "%02hhx", buf[i]);
    }
    return n;
}

int sys_par_ch_to_str(char *str, const sys_par_ch_t k) {
    int i, n = 0;
    for (i = 0; i < POLY_COUNT - 2; ++i) {
        n += poly_to_str(str + n, k[i]);
        n += sprintf(str + n, "; ");
    }
    n += poly_to_str(str + n, k[POLY_COUNT - 2]);
    return n;
}

int par_ch_to_str(char *str, const par_ch_t k) {
    int i, n = 0;
    for (i = 0; i < POLY_COUNT - 1; ++i) {
        n += sp_poly_to_str(str + n, k[i]);
        n += sprintf(str + n, "; ");
    }
    n += sp_poly_to_str(str + n, k[POLY_COUNT - 1]);
    return n;
}

void fprintpoly(FILE *f, const poly_t p) {
    char buf[POLY_BYTES * CHARS_PER_BYTE] = {0};
    poly_to_str(buf, p);
    fprintf(f, "%s", buf);
}

void fprintsp_poly(FILE *f, const sp_poly_t p) {
    char buf[POLY_WEIGHT * CHARS_PER_INDEX] = {0};
    sp_poly_to_str(buf, p);
    fprintf(f, "%s", buf);
}

void fprinterror(FILE *f, const error_t err) {
    char buf[POLY_COUNT * POLY_BYTES * CHARS_PER_BYTE] = {0};
    error_to_str(buf, err);
    fprintf(f, "%s", buf);
}

void fprintsperror(FILE *f, const sp_error_t err) {
    char buf[ERROR_WEIGHT * CHARS_PER_INDEX] = {0};
    sp_error_to_str(buf, err);
    fprintf(f, "%s", buf);
}

void fprintbytes(FILE *f, const unsigned char *buf, int byte_count) {
    char str[byte_count * CHARS_PER_INDEX];
    for (int i = 0; i < byte_count * CHARS_PER_INDEX; ++i) {
        str[i] = 0;
    }
    bytes_to_hex(str, buf, byte_count);
    fprintf(f, "%s", str);
}

void fprintsysparch(FILE *f, const sys_par_ch_t k) {
    char buf[(POLY_COUNT - 1) * POLY_BYTES * CHARS_PER_BYTE] = {0};
    sys_par_ch_to_str(buf, k);
    fprintf(f, "%s", buf);
}

void fprintparch(FILE *f, const par_ch_t k) {
    char buf[POLY_COUNT * POLY_WEIGHT * CHARS_PER_INDEX] = {0};
    par_ch_to_str(buf, k);
    fprintf(f, "%s", buf);
}
