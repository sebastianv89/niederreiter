#include "kem.h"
#include "config.h"
#include "poly.h"
#include "poly_sparse.h"
#include "error.h"

/* Convert the partiycheck matrix into a systematic one. */
void kem_to_systematic(word_t (*sys_par_ch)[POLY_WORDS],
                       const word_t *inv,
                       const index_t (*par_ch)[POLY_WEIGHT]) {
    size_t i;
    for (i = 0; i < NUMBER_OF_POLYS - 1; ++i) {
        poly_sparse_mul(sys_par_ch[i], inv, par_ch[i]);
    }
}

/* Transpose the parity check matrix (except for the last block), so
 * that it becomes possible to iterate over the columns (instead of
 * the rows).
 */ 
void kem_transpose_privkey(index_t (*par_ch)[POLY_WEIGHT]) {
    size_t i;
    for (i = 0; i < NUMBER_OF_POLYS - 1; ++i) {
        poly_sparse_transpose(par_ch[i]);
    }
}

/* generate the private key and compute the inverse of the last block */
void kem_rand_par_ch(word_t *inv, index_t (*par_ch)[POLY_WEIGHT]) {
    size_t i;
    for (i = 0; i < NUMBER_OF_POLYS - 1; ++i) {
        poly_sparse_rand(par_ch[i]);
    }
    do {
        poly_sparse_rand(par_ch[i]);
        poly_sparse_to_dense(inv, par_ch[i]);
    } while (poly_inv(inv));
}

/* generate a random keypair */
void kem_keypair(word_t (*pub_key)[POLY_WORDS], index_t (*priv_key)[POLY_WEIGHT]) {
    word_t inv[POLY_WORDS];

    kem_rand_par_ch(inv, priv_key);
    kem_to_systematic(pub_key, inv, priv_key);
    kem_transpose_privkey(priv_key);
}

/* generate a random error of weight `ERROR_WEIGHT` */
void kem_gen_error(word_t (*error)[POLY_WORDS]) {
    index_t error_sparse[ERROR_WEIGHT];
	error_rand(error_sparse);
    error_to_dense(error, error_sparse);
}

/* Possible optimization (if sparse multiplication can be optimized):
 * use sparse error */
void kem_encrypt(word_t *pub_syn,
                 const word_t (*error)[POLY_WORDS],
                 const word_t (*sys_par_ch)[POLY_WORDS]) {
    word_t buf[POLY_WORDS];
    size_t i;
    
    poly_mul(pub_syn, sys_par_ch[0], error[0]);
    for (i = 1; i < NUMBER_OF_POLYS - 1; ++i) {
        poly_mul(buf, sys_par_ch[i], error[i]);
        poly_inplace_add(pub_syn, buf);
    }
    poly_inplace_add(pub_syn, error[NUMBER_OF_POLYS - 1]);
}

/* Decode the provided (private) syndrome to find the corresponding
 * error vector.
 * 
 * \param[out] error     Resulting error vector
 * \param[out] syn_cand  Syndrome of resulting error vector: is zero upon success
 * \param[in]  syn_cand  Private syndrome
 * \param[in]  par_ch    Parity check polynomial
 */
void decode(word_t (*error)[POLY_WORDS],
            word_t *syn_cand,
            const index_t (*par_ch)[POLY_WEIGHT]) {
    static const word_t threshold[ITERATIONS] = THRESHOLDS;

	size_t i, poly, word;
	word_t upc; /* unsatisfied parity check counter */
	word_t syn_update[POLY_WORDS];
    word_t par_ch_dense[NUMBER_OF_POLYS - 1][POLY_WORDS];
    word_t masked[POLY_WORDS];
    word_t bit, mask;
    
    error_zero(error);
    for (i = 0; i < NUMBER_OF_POLYS - 1; ++i) {
        poly_sparse_to_dense(par_ch_dense[i], par_ch[i]);
    }

	for (i = 0; i < ITERATIONS; ++i) {
        poly_zero(syn_update);
        for (poly = 0; poly < NUMBER_OF_POLYS - 1; ++poly) {
            for (word = 0; word < POLY_WORDS; ++word) {
                for (bit = 1; bit != 0; bit <<= 1) {
                    poly_mask(masked, syn_cand, par_ch_dense[poly]);
                    upc = poly_hamming_weight(masked);
                    mask = -(((threshold[i] - upc - 1) >> (WORD_BITS - 1)) & 1);
                    error[poly][word] ^= bit & mask;
                    poly_inplace_add_masked(syn_update, par_ch_dense[poly], mask);
                    poly_inplace_mulx_modG(par_ch_dense[poly]);
                }
            }
        }
        poly_inplace_add(syn_cand, syn_update);
	}
}

int kem_decrypt(word_t (*error)[POLY_WORDS],
                const word_t *pub_syn,
                const index_t (*priv_key)[POLY_WEIGHT]) {
	word_t priv_syn[POLY_WORDS];
	poly_sparse_mul(priv_syn, pub_syn, priv_key[NUMBER_OF_POLYS - 1]);
	decode(error, priv_syn, priv_key);
    return poly_verify_zero(priv_syn);
}
