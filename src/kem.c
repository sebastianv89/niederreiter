#include <stdlib.h>

#include "kem.h"
#include "config.h"
#include "poly.h"

/* Decode the provided (private) syndrome to find the corresponding
 * error vector.
 * 
 * \param[out] error     Resulting error vector
 * \param[out] syn_cand  Syndrome of resulting error vector: is zero upon success
 * \param[in]  syn_cand  Private syndrome
 * \param[in]  par_ch    Parity check polynomial
 */
void decode(
          word_t  *error,
          word_t  *syn_cand,
    const index_t *par_ch)
{
    static const word_t threshold[ITERATIONS] = THRESHOLDS;

	size_t i, j, k;
	word_t upc; /* unsatisfied parity check */
	word_t syn_update[POLY_WORDS];
    word_t par_ch_dense[(NUMBER_OF_POLYS - 1) * POLY_WORDS];
    word_t *block;
    word_t masked[POLY_WORDS];
    word_t mask;
    
    poly_zero(error, POLY_WORDS);

    for (j = 0; j < NUMBER_OF_POLYS - 1; ++j) {
        poly_to_dense(par_ch_dense + j * POLY_WORDS, par_ch + j * POLY_WEIGHT);
    }

    /*
    printf("parity check matrix (sparse):\n");
    print_poly_sparse(par_ch, NUMBER_OF_POLYS * POLY_WEIGHT);
    printf("\nparity check matrix (dense):\n");
    for (i = 0; i < NUMBER_OF_POLYS; ++i) {
        print_poly_dense(par_ch_dense, POLY_WORDS);
        printf("\n");
    }
    */

	for (i = 0; i < ITERATIONS; ++i) {
        unsigned int count = 0;
        printf("threshold %"PRIu64":\n", threshold[i]);
        poly_zero(syn_update, POLY_WORDS);
        for (j = 0; j < NUMBER_OF_POLYS - 1; ++j) {
            block = par_ch_dense + j * POLY_WORDS;
            for (k = 0; k < POLY_BITS; ++k) {
                poly_mask(masked, syn_cand, block);
                upc = poly_hamming_weight(masked); //debug
                if (upc >= threshold[i]) count++;
                //printf("col %zu; hw %2"PRIu64"; ", j*POLY_BITS + k, upc);
                mask = ((threshold[i] - upc - 1) >> (WORD_BITS - 1)) & 1;
                //printf("flip %016"PRIx64"; ", mask);
                poly_flip(error, (index_t)(j * POLY_WORDS + k), mask);
                mask = -mask;
                //printf("mask %016"PRIx64"; ", mask);
                poly_add_masked(syn_update, block, mask);
                poly_inplace_mul_x_modG(block);
                //printf("\n");
            }
        }
        printf("potential erros found: %u\n", count);
        poly_add(syn_cand, syn_cand, syn_update, POLY_WORDS);

        /*
        printf("syndrome update (hw %"PRIu64")\n", poly_hamming_weight(syn_update));
        print_poly_dense(syn_update, POLY_WORDS);
        printf("\nsyndrome (hw %"PRIu64"):\n", poly_hamming_weight(syn_cand));
        print_poly_dense(syn_cand, POLY_WORDS);
        printf("\nerror:\n");
        print_poly_dense(error, ERROR_WORDS);
        printf("\n");
        */
	}
}

void kem_keypair(
    word_t  *pub_key,
    index_t *priv_key)
{
    size_t i;
    word_t inverse[POLY_WORDS];

    for (i = 0; i < NUMBER_OF_POLYS - 1; ++i) {
        poly_gen_sparse(priv_key + i * POLY_WEIGHT, TYPE_POLY);
    }

    do {
        poly_gen_sparse(priv_key + i * POLY_WEIGHT, TYPE_POLY);
        poly_to_dense(inverse, priv_key + i * POLY_WEIGHT);
    } while (poly_inv(inverse));
    
    /*
    {
        word_t one[POLY_WORDS];
        word_t last_block[POLY_WORDS];
        poly_to_dense(last_block, priv_key + i * POLY_WEIGHT);
        poly_mul(one, inverse, last_block);
        
        printf("last block:\n");
        print_poly_sparse(priv_key + i * POLY_WEIGHT, POLY_WEIGHT);
        printf("\ninverse:\n");
        print_poly_dense(inverse, POLY_WORDS);
        printf("\none:\n");
        print_poly_dense(one, POLY_WORDS);
        printf("\n");
    }
    */

    for (i = 0; i < NUMBER_OF_POLYS - 1; ++i) {
        poly_sparse_mul(pub_key + i * POLY_WORDS,
                        inverse,
                        priv_key + i * POLY_WEIGHT);
    }

    for (i = 0; i < NUMBER_OF_POLYS - 1; ++i) {
        poly_transpose(priv_key + i * POLY_WEIGHT);
    }
}

void kem_gen_error(word_t *error)
{
    index_t error_sparse[ERROR_WEIGHT];
	poly_gen_sparse(error_sparse, TYPE_ERROR);
    error_to_dense(error, error_sparse);
}

/* Possible optimization: use sparse error and multiply using that */
void kem_encrypt(
          word_t *pub_syn,
    const word_t *error,
    const word_t *sys_par_ch)
{
    size_t i;
    word_t buf[POLY_WORDS];
    
    /*
    printf("public syndrome = systematic parity check * error\n");
    printf("systematic parity check:\n");
    print_poly_dense(sys_par_ch, POLY_WORDS);
    printf("\nerror:\n");
    print_poly_dense(error, ERROR_WORDS);
    printf("\n");
    */

    poly_mul(pub_syn, sys_par_ch, error);

    /*
    printf("first block:\n");
    print_poly_dense(pub_syn, POLY_WORDS);
    */

    for (i = 1; i < NUMBER_OF_POLYS - 1; ++i) {
        poly_mul(buf,
                 sys_par_ch + i * POLY_WORDS,
                 error + i * POLY_WORDS);
        poly_add(pub_syn, pub_syn, buf, POLY_WORDS);

        /*
        printf("\nadding block:\n");
        print_poly_dense(buf, POLY_WORDS);
        printf("\nsum:\n");
        print_poly_dense(pub_syn, POLY_WORDS);
        */
    }

    poly_add(pub_syn,
             pub_syn,
             error + i * POLY_WORDS, POLY_WORDS);

    /*
    printf("\nadding (last block):\n");
    print_poly_dense(error + i * POLY_WORDS, POLY_WORDS);
    printf("public syndrome (ciphertext, hw %"PRIu64")\n", poly_hamming_weight(pub_syn));
    print_poly_dense(pub_syn, POLY_WORDS);
    printf("\n");
    */
}


int kem_decrypt(
          word_t  *error,
    const word_t  *pub_syn,
    const index_t *priv_key)
{
	word_t priv_syn[POLY_WORDS];

	poly_sparse_mul(priv_syn,
                    pub_syn,
                    priv_key + (NUMBER_OF_POLYS - 1) * POLY_WEIGHT);
    printf("pub syn hw: %"PRIu64"\n", poly_hamming_weight(pub_syn));
    printf("priv syn hw: %"PRIu64"\n", poly_hamming_weight(priv_syn));
	decode(error, priv_syn, priv_key);
    return poly_is_zero(priv_syn, POLY_WORDS);
}
