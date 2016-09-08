#ifndef NIEDERREITER_KEM_H
#define NIEDERREITER_KEM_H

#include "config.h"

/** Generate a random keypair */
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
