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
    unsigned char *priv_key) {
    sys_par_ch_t spc;
    par_ch_t pc;

    kem_keypair(spc, pc);

    pack_pubkey(pub_key, spc);
    pack_privkey(priv_key, pc); 
    return 0;
}

int crypto_encrypt(
          unsigned char *ct,  unsigned long long *ct_len,
    const unsigned char *msg, unsigned long long  msg_len,
    const unsigned char *pub_key) {
    error_t err;
    unsigned char err_bytes[ERROR_BYTES], sec_key[SECRET_KEY_BYTES];
    unsigned char *dem_ct = ct + POLY_BYTES;
    unsigned long long dem_ct_len;
    poly_t pub_syn;
    sys_par_ch_t spc;

    unpack_pubkey(spc, pub_key);

    kem_gen_err(err);
    kem_encrypt(pub_syn, err, spc);

    pack_err(err_bytes, err);

    crypto_hash_sha512(sec_key, err_bytes, ERROR_BYTES);
    dem_encrypt(dem_ct, &dem_ct_len, msg, msg_len, sec_key);
    *ct_len = POLY_BYTES + dem_ct_len;

    pack_poly(ct, pub_syn);

    return 0;
}

int crypto_encrypt_open(
          unsigned char *msg, unsigned long long *msg_len,
    const unsigned char *ct,  unsigned long long  ct_len,
    const unsigned char *priv_key) {
    int ret_val;
    error_t err = {{0}};
    poly_t pub_syn;
    par_ch_t pc;
    unsigned char err_bytes[ERROR_BYTES], sec_key[SECRET_KEY_BYTES];
    const unsigned char *dem_ct = ct + POLY_BYTES;
    unsigned long long dem_ct_len = ct_len - POLY_BYTES;

    unpack_poly(pub_syn, ct);
    unpack_privkey(pc, priv_key);

    ret_val = kem_decrypt(err, pub_syn, pc);

    pack_err(err_bytes, err);

    crypto_hash_sha512(sec_key, err_bytes, ERROR_BYTES);

    ret_val |= dem_decrypt(msg, msg_len, dem_ct, dem_ct_len, sec_key);

    return ret_val;
}
