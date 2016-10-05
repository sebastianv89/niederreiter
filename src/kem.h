#ifndef NIEDERREITER_KEM_H
#define NIEDERREITER_KEM_H

#include "types.h"

/** Generate the private key and compute the inverse of the last block */
void kem_rand_par_ch(poly_t inv, par_ch_t priv_key);

/** Convert the parity-check matrix into a systematic one. */
void kem_to_systematic(
          sys_par_ch_t pub_key,
    const poly_t       inv,
    const par_ch_t     priv_key);

/** Transpose the parity check matrix (except for the last block), so
 * that it becomes efficient to iterate over the columns (instead of
 * the rows).
 */
void kem_transpose_par_ch(par_ch_t priv_key);

/** Generate a random key-pair */
void kem_keypair(sys_par_ch_t pub_key, par_ch_t priv_key);

/** Generate a random error vector */
void kem_gen_err(error_t err);

/** Encrypt the error
 *
 * \param[out] pub_syn  Public syndrome (ciphertext)
 * \param[in]  err      Error (plaintext)
 * \param[in]  pub_key  Systematic QC-MDPC code (public key)
 */
void kem_encrypt(syn_t pub_syn, error_t err, const sys_par_ch_t pub_key);

/** Decode the provided (private) syndrome to find the corresponding
 * error vector.
 *
 * @param[in,out] err       in:  Zero
 *                          out: Resulting error vector
 * @param[in,out] priv_syn  in:  Private syndrome
 *                          out: Syndrome of resulting error vector
 *                               (zero upon success)
 * @param[in]     priv_key  Parity check polynomial
 */
void kem_decode(error_t err, syn_t priv_syn, const par_ch_t priv_key);

/* TODO: Tranposing the last block of the private key can be done inplace,
 *       saving a small bit of memory.  The const qualifier will have to
 *       be removed from the priv_key parameter in kem_decode and kem_decrypt
 */

/** Decrypt the error.
 *
 * Important: kem_decrypt returns -1 upon failure (matching the error
 * code from libnacl.crypto_onetimeauth_verify()), so that the type of
 * failure does not leak through the error code and the implementation
 * is IND-CCA secure.
 *
 * \param[in,out] err       in:  Zero
 *                          out: Error (plaintext)
 * \param[in]     pub_syn   Public syndrome (ciphertext)
 * \param[in]     priv_key  QC-MDPC code (private key)
 *
 * \return failure code    (decoding successful ? 0 : -1)
 */
int kem_decrypt(error_t err, syn_t pub_syn, const par_ch_t priv_key);

#endif /* NIEDERREITER_KEM_H */
