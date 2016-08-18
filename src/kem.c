#include <stdlib.h>

#include "config.h"
#include "kem.h"
#include "poly.h"

static const unsigned int thresholds[ITERATIONS] = THRESHOLDS;

void systematic(word_t *sys_par_ch,
                const index_t *par_ch)
{
	size_t block;
	unsigned char inv_last_block[BLOCK_BYTES];
	index_t *last_block = par_ch + (NUMBER_OF_BLOCKS - 1) * BLOCK_ROW_WEIGHT;

	inverse(inv_last_block, last_block);
	for (block = 0; block < NUMBER_OF_BLOCKS - 1; ++block) {
		mat_mat_mult(sys_par_ch + block * BLOCK_WORDS,
		             inv_last_block,
		             par_ch + block * BLOCK_WORDS);
	}
}

void kem_keypair(
    word_t *pub_key,
    index_t *priv_key)
{
    size_t i;

    for (i = 0; i < NUMBER_OF_POLYS; ++i) {
        
    }
}

void kem_gen_error(index_t *error)
{
	poly_gen_sparse(error, ERROR_WEIGHT, ERROR_BITS, ERROR_INDEX_MASK);
}

void kem_encrypt(
          word_t *pub_syn,
    const index_t *error,
    const word_t *sys_par_ch)
{
	size_t i, j;
	word_t buf[BLOCK_WORDS];

	for (i = 0; i < NUMBER_OF_BLOCKS - 1; ++i) {
		mat_vec_mult(buf, &sys_par_ch[i * BLOCK_WORDS], &err[i * BLOCK_WORDS]);
		for (j = 0; j < BLOCK_WORDS; ++j) {
			pub_syn[j] ^= buf[j];
		}
	}
	for (j = 0; j < BLOCK_WORDS; ++j) {
		pub_syn[j] ^= err[i * BLOCK_WORDS + j];
	}
}

/* Check if the syndrome is zero in constant time */
int is_zero(word_t *syn)
{
	size_t i;
	word_t zero = 0;
	for (i = 0; i < SYNDROME_WORDS; ++i) {
		zero |= syn[i];
	}
	return zero == 0;
}

int decode(word_t *err,
           word_t *syn_cand,
           const index_t *par_ch)
{
	size_t i;
	unsigned int threshold;
	unsigned int upc; /* unsatisfied parity check */
	unsigned char *syn_update[SYNDROME_BYTES];
	unsigned char *err_cand[ERROR_BYTES] = {0};

	for (i = 0; i < ITERATIONS; ++i) {
		threshold = thresholds[i];
		memset(syn_update, 0, SYNDROME_BYTES);
		// TODO: from here on is pseudocode
		for (j = 0; j < MATRIX_ROW_BITS; ++j) {
			upc = hamming_weight(syn_cand ^ par_ch[j]);
			if (upc > threshold) { // TODO: no branch
				err_cand.flip(j);
				syn_update ^= par_ch[j];
			}
		}
		syn_cand ^= syn_update;
	}
	return 1 - is_zero(syn_cand);
}

void private_syndrome(
    unsigned char *priv_syn,
    const unsigned char *pub_syn,
    const index_t *par_ch)
{
	index_t *last_block = par_ch + (NUMBER_OF_BLOCKS - 1) * BLOCK_ROW_WEIGHT;
	mat_vec_mult(priv_syn, last_block, pub_syn);
}

/* Important: kem_decrypt must return -1 upon failure (matching the
 * error code from libnacl.crypto_onetimeauth_verify()), or else the
 * type of failure leaks through the error code and the implementation
 * is no longer IND-CCA secure.
 */
int kem_decrypt(
          word_t *error,
    const word_t *pub_syn,
    const index_t *priv_key)
{
	word_t priv_syn[SYNDROME];
	private_syndrome(priv_syn, pub_syn, priv_key);
	return decode(error, priv_syn, priv_key);
}
