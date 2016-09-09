#include "crypto_hash_sha512.h"

#ifdef SUPERCOP_BUILD
#include "crypto_encrypt.h"
#else
#include "encrypt.h"
#endif

#include "config.h"
#include "pack.h"
#include "kem.h"
#include "dem.h"

int crypto_encrypt_keypair(
    unsigned char *pub_key,
    unsigned char *priv_key)
{
    word_t sys_par_ch[NUMBER_OF_POLYS - 1][POLY_WORDS];
    index_t par_ch[NUMBER_OF_POLYS][POLY_WEIGHT];

    kem_keypair(sys_par_ch, par_ch);

    pack_pub_key(pub_key, sys_par_ch);
    pack_priv_key(priv_key, par_ch);

    return 0;
}

int crypto_encrypt(
          unsigned char *ct,  unsigned long long *ct_len,
    const unsigned char *msg, unsigned long long  msg_len,
    const unsigned char *pub_key)
{
    word_t error[NUMBER_OF_POLYS][POLY_WORDS];
    unsigned char error_bytes[ERROR_BYTES];
    unsigned char sec_key[SECRET_KEY_BYTES];
    unsigned char *dem_ct = ct + POLY_BYTES;
    unsigned long long dem_ct_len;
    word_t syndrome[POLY_WORDS];
    word_t sys_par_ch[NUMBER_OF_POLYS - 1][POLY_WORDS];

    unpack_pub_key(sys_par_ch, pub_key);

    kem_gen_error(error);
    kem_encrypt(syndrome, error, sys_par_ch);
    pack_error(error_bytes, error);
    crypto_hash_sha512(sec_key, error_bytes, ERROR_BYTES);
    dem_encrypt(dem_ct, &dem_ct_len, msg, msg_len, sec_key);

    pack_syndrome(ct, syndrome);
    *ct_len = POLY_BYTES + dem_ct_len;

    return 0;
}

int crypto_encrypt_open(
          unsigned char *msg, unsigned long long *msg_len,
    const unsigned char *ct,  unsigned long long  ct_len,
    const unsigned char *priv_key)
{
    int ret_val;
    word_t error[NUMBER_OF_POLYS][POLY_WORDS];
    word_t syndrome[POLY_WORDS];
    index_t par_ch[NUMBER_OF_POLYS][POLY_WEIGHT];
    unsigned char error_bytes[ERROR_BYTES];
    unsigned char sec_key[SECRET_KEY_BYTES];
    const unsigned char *dem_ct = ct + POLY_BYTES;
    unsigned long long dem_ct_len = ct_len - POLY_BYTES;

    unpack_syndrome(syndrome, ct);
    unpack_priv_key(par_ch, priv_key);

    ret_val = kem_decrypt(error, syndrome, par_ch);
    pack_error(error_bytes, error);
    crypto_hash_sha512(sec_key, error_bytes, ERROR_BYTES);
    ret_val |= dem_decrypt(msg, msg_len, dem_ct, dem_ct_len, sec_key);

    return ret_val;
}
