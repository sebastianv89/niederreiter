#include "crypto_hash_sha256.h" /* TODO: use SHA-3 (Keccak) */

#include "kex.h"
#include "kem.h"
#include "word.h"

void crypto_kex_keypair(
    unsigned char *pub_key,
    unsigned char *priv_key)
{
    /* TODO: are *pub_key and *priv_key pre-allocated? */
    /* TODO unpack byte format to words */
    kem_keypair(pub_key, priv_key);
}

void crypto_kex_encrypt(
    unsigned char *ct,
    unsigned char *sec_key,
    const unsigned char *pub_key)
{
    // TODO: are *ct and *sec_key pre-allocated?
    index_t error[ERROR_WEIGHT];
    kem_rand_error(error);
    crypto_hash_sha256(sec_key, error, ERROR_SIZE);
    kem_encrypt(ct, error, pub_key);
}

void crypto_kex_encrypt_open(
          unsigned char *msg,
    const unsigned char *sec_key,
    const unsigned char *priv_key)
{
    // TODO: is *sec_key pre-allocated?
    unsigned char error[ERROR_SIZE];
    kem_decrypt(error, ct, priv_key);
    crypto_hash_sha256(sec_key, error, ERROR_SIZE);
}
