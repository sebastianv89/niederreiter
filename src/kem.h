#ifndef NIEDERREITER_KEM_H
#define NIEDERREITER_KEM_H

#include "config.h"

void kem_keypair(
    word_t  *pub_key,
    index_t *priv_key);

/** Generate a random error vector of weight ERROR_WEIGHT */
void kem_gen_error(
    word_t *error);

/** Encrypt the error.
 * 
 * \param[out] pub_syn  Resulting public syndrome (ciphertext)
 * \param[in]  error    (Dense) error polynomial (plaintext)
 * \param[in]  pub_key  Systematic QC-MPDC parity-check matrix,
 *                      represented with polynomials (public key)
 */
void kem_encrypt(
          word_t *pub_syn,
    const word_t *error,
    const word_t *pub_key);

/** Decrypt the error.
 * 
 * Important: kem_decrypt returns -1 upon failure (matching the error
 * code from libnacl.crypto_onetimeauth_verify()), so that the type of
 * failure does not leak through the error code and the implementation
 * is IND-CCA secure.
 *
 * \param[out] error     Resulting error (plaintext)
 * \param[in]  pub_syn   Public syndrome (ciphertext)
 * \param[in]  priv_key  QC-MDPC parity-check matrix, represented with
 *                       polynomials (private key)
 * 
 * \return error code: (decoding successful: 0 : -1)
 */
int kem_decrypt(
          word_t  *error,
    const word_t  *pub_syn,
    const index_t *priv_key);

#endif /* NIEDERREITER_KEM_H */
