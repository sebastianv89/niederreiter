#include "crypto_hash_sha512.h"
#include "crypto_encrypt.h"

int crypto_encrypt_keypair(
    unsigned char *pub_key,
    unsigned char *priv_key)
{
    kem_keypair(pub_key, priv_key);
    return 0;
}

int crypto_encrypt(
          unsigned char *ct,  unsigned long long *ct_len,
    const unsigned char *msg, unsigned long long  msg_len,
    const unsigned char *pub_key)
{
    unsigned char error[ERROR_BYTES];
    unsigned char sec_key[/* TODO: size */];
    unsigned long long dem_ct_len;
    unsigned char *dem_ct = ct + BLOCK_BYTES;

    kem_rand_err(error);
    kem_encrypt(ct, error, pub_key);
    crypto_hash_sha512(sec_key, error, ERROR_BYTES);
    dem_encrypt(dem_ct, dem_ct_len, msg, msg_len, sec_key);
    *ct_len = BLOCK_BYTES + dem_ct_len;

    return 0;
}

int crypto_encrypt_open(
          unsigned char *msg, unsigned long long *msg_len,
    const unsigned char *ct,  unsigned long long  ct_len,
    const unsigned char *priv_key)
{
    int ret_val;
    unsigned char error[ERROR_BYTES];
    unsigned char sec_key[SECRET_KEY_BYTES]
    unsigned long long dem_ct_len = ct_len - BLOCK_BYTES;
    unsigned char dem_ct = ct + BLOCK_BYTES;
    
    ret_val = kem_decrypt(error, ct, priv_key);
    crypto_hash_sha512(sec_key, error, ERROR_BYTES);
    ret_val |= dem_decrypt(msg, *msg_len, dem_ct, dem_ct_len, sec_key);
    return ret_val;
}
