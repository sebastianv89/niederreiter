#ifndef NIEDERREITER_ENCRYPT_H
#define NIEDERREITER_ENCRYPT_H

/** Generate key-pair
 *
 * @param[out] pub_key   Public key
 * @param[out] priv_key  Private key
 * @return 0
 */
int crypto_encrypt_keypair(
    unsigned char *pub_key,
    unsigned char *priv_key);

/** Encrypt a message
 *
 * @param[out] ct        Ciphertext
 * @param[out] ct_len    Ciphertext length (in bytes)
 * @param[in]  msg       Plaintext
 * @param[in]  msg_len   Plaintext length (in bytes)
 * @param[in]  pub_key   Public key
 * @return 0
 */
int crypto_encrypt(
          unsigned char *ct,  unsigned long long *ct_len,
    const unsigned char *msg, unsigned long long  msg_len,
    const unsigned char *pub_key);

/** Verify/decrypt a message
 *
 * @param[out] msg       Plaintext
 * @param[out] msg_len   Plaintext length (in bytes)
 * @param[in]  ct        Ciphertext
 * @param[in]  ct_len    Ciphertext length (in bytes)
 * @param[in]  priv_key  Private key
 * @return 0 upon successful decryption, -1 otherwise
 */
int crypto_encrypt_open(
          unsigned char *msg, unsigned long long *msg_len,
    const unsigned char *ct,  unsigned long long  ct_len,
    const unsigned char *priv_key);

#endif /* NIEDERREITER_ENCRYPT_H */
