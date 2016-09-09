#ifndef NIEDERREITER_KEM_H
#define NIEDERREITER_KEM_H

#include "config.h"

/** Convert the parity-check matrix into a systematic one. */
void kem_to_systematic(word_t (*sys_par_ch)[POLY_WORDS],
                       const word_t *inv,
                       const index_t (*par_ch)[POLY_WEIGHT]);

/** Transpose the parity check matrix (except for the last block), so
 * that it becomes possible to iterate over the columns (instead of
 * the rows).
 */
void kem_transpose_privkey(index_t (*par_ch)[POLY_WEIGHT]);

/** generate the private key and compute the inverse of the last block */
void kem_rand_par_ch(word_t *inv, index_t (*par_ch)[POLY_WEIGHT]);

/** Generate a random key-pair */
void kem_keypair(word_t (*pub_key)[POLY_WORDS],
                 index_t (*priv_key)[POLY_WEIGHT]);

/** Generate a random error vector */
void kem_gen_error(word_t (*error)[POLY_WORDS]);

/** Encrypt the error
 *
 * \param[out] pub_syn  Public syndrome (ciphertext)
 * \param[in]  error    Error (plaintext)
 * \param[in]  pub_key  Systematic QC-MDPC code (public key)
 */
void kem_encrypt(word_t *pub_syn,
                 const word_t (*error)[POLY_WORDS],
                 const word_t (*pub_key)[POLY_WORDS]);

/** Decode the provided (private) syndrome to find the corresponding
 * error vector.
 *
 * @param[out]    error     Resulting error vector
 * @param[in,out] syn_cand  in:  Private syndrome
 *                          out: Syndrome of resulting error vector:
 *                               zero upon success
 * @param[in]     par_ch    Parity check polynomial
 */
void decode(word_t (*error)[POLY_WORDS],
            word_t *syn_cand,
            const index_t (*par_ch)[POLY_WEIGHT]);

/** Decrypt the error.
 *
 * Important: kem_decrypt returns -1 upon failure (matching the error
 * code from libnacl.crypto_onetimeauth_verify()), so that the type of
 * failure does not leak through the error code and the implementation
 * is IND-CCA secure.
 *
 * \param[out] error     Error (plaintext)
 * \param[in]  pub_syn   Public syndrome (ciphertext)
 * \param[in]  priv_key  QC-MDPC code (private key)
 *
 * \return error code    (decoding successful ? 0 : -1)
 */
int kem_decrypt(word_t (*error)[POLY_WORDS],
                const word_t *pub_syn,
                const index_t (*priv_key)[POLY_WEIGHT]);

#endif /* NIEDERREITER_KEM_H */
