#ifndef NIEDERREITER_KEX_H
#define NIEDERREITER_KEX_H

/** Generate key-pair
 *
 * @param[out] pub_key   Public key
 * @param[out] priv_key  Private key
 * @return 0
 */
int crypto_kex_keypair(
          unsigned char *pub_key,
          unsigned char *priv_key);

/** Generate random secret key and encrypt using KEM encryption
 *
 * @param[out] sec_key   Secret key (plaintext)
 * @param[out] ct        Ciphertext
 * @param[in]  pub_key   Public key
 * @return 0
 */
int crypto_kex_encrypt(
          unsigned char *sec_key,
          unsigned char *ct,
    const unsigned char *pub_key);

/** Verify/decrypt a secret key
 *
 * @param[out] sec_key   Secret key (plaintext)
 * @param[in]  ct        Ciphertext
 * @param[in]  priv_key  Private key
 * @return 0 upon successful decryption, -1 otherwise
 */
int crypto_kex_encrypt_open(
          unsigned char *sec_key,
    const unsigned char *ct,
    const unsigned char *priv_key);

#endif /* NIEDERREITER_KEX_H */
