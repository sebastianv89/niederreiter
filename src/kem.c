#include <stddef.h>

#include "kem.h"
#include "config.h"
#include "types.h"
#include "poly.h"
#include "sp_poly.h"
#include "error.h"

void kem_gen_par_ch(poly_t inv, par_ch_t priv_key) {
    size_t i;

    for (i = 0; i < POLY_COUNT - 1; ++i) {
        sp_poly_rand(priv_key[i]);
    }
    do {
        sp_poly_rand(priv_key[POLY_COUNT - 1]);
        sp_poly_to_poly(inv, priv_key[POLY_COUNT - 1]);
    } while (poly_inv(inv));
}

void kem_to_systematic(
          sys_par_ch_t pub_key,
    const poly_t inv,
    const par_ch_t priv_key) {
    size_t i;
    poly_t priv_poly;
    for (i = 0; i < POLY_COUNT - 1; ++i) {
        sp_poly_to_poly(priv_poly, priv_key[i]);
        poly_mul(pub_key[i], inv, priv_poly);
    }
}

void kem_gen_keypair(sys_par_ch_t pub_key, par_ch_t priv_key) {
    poly_t inv;
    kem_gen_par_ch(inv, priv_key);
    kem_to_systematic(pub_key, inv, priv_key);
}

void kem_gen_err(error_t err) {
    sp_error_t sp_error;
	sp_gen_error(sp_error);
    sp_error_to_error(err, sp_error);
}

void kem_encrypt(syn_t pub_syn, error_t err, const sys_par_ch_t pub_key) {
    syn_t buf;
    size_t i;
    poly_mul(pub_syn, pub_key[0], err[0]);
    for (i = 1; i < POLY_COUNT - 1; ++i) {
        poly_mul(buf, pub_key[i], err[i]);
        poly_add(pub_syn, pub_syn, buf);
    }
    poly_add(pub_syn, pub_syn, err[POLY_COUNT - 1]);
}

void kem_decode(error_t err, syn_t priv_syn, const par_ch_t priv_key) {
    static const limb_t threshold[] = THRESHOLDS;
    static const size_t ITERATIONS = sizeof(threshold)/sizeof(threshold[0]);

	size_t i, p, l;
	limb_t bit, mask, count;
    syn_t syn_update, masked;
    dense_par_ch_t priv_key_dense;

    for (i = 0; i < POLY_COUNT; ++i) {
        sp_poly_to_poly(priv_key_dense[i], priv_key[i]);
    }
	for (i = 0; i < ITERATIONS; ++i) {
        poly_zero(syn_update);
        for (p = 0; p < POLY_COUNT; ++p) {
            for (l = 0; l < POLY_LIMBS - 1; ++l) {
                for (bit = 1; bit != 0; bit <<= 1) {
                    poly_mask(masked, priv_key_dense[p], priv_syn);
                    count = poly_hamming_weight(masked);
                    mask = -((threshold[i] - count - 1) >> (LIMB_BITS - 1));
                    err[p][l] ^= bit & mask;
                    poly_inplace_add_masked(syn_update, priv_key_dense[p], mask);
                    poly_inplace_mulx_modG(priv_key_dense[p]);
                }
            }
            for (bit = 1; bit != (limb_t)(1) << TAIL_BITS; bit <<= 1) {
                poly_mask(masked, priv_key_dense[p], priv_syn);
                count = poly_hamming_weight(masked);
                mask = -(((threshold[i] - count - 1) >> (LIMB_BITS - 1)) & 1);
                err[p][POLY_LIMBS - 1] ^= bit & mask;
                poly_inplace_add_masked(syn_update, priv_key_dense[p], mask);
                poly_inplace_mulx_modG(priv_key_dense[p]);
            }
        }
        poly_add(priv_syn, priv_syn, syn_update);
	}
}

int kem_decrypt(error_t err, syn_t pub_syn, const par_ch_t priv_key) {
    poly_t priv_last;
    syn_t priv_syn;

    sp_poly_to_poly(priv_last, priv_key[POLY_COUNT - 1]);
    poly_mul(priv_syn, priv_last, pub_syn);
	kem_decode(err, priv_syn, priv_key);

    return poly_verify_zero(priv_syn);
}
