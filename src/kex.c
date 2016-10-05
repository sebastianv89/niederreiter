#include "crypto_hash_sha256.h" /* TODO: use SHA-3 (Keccak) */

#include "kex.h"
#include "config.h"
#include "kem.h"
#include "pack.h"

int crypto_kex_keypair(
        unsigned char *pub_key,
        unsigned char *priv_key) {
    sys_par_ch_t spc;
    par_ch_t pc;

    kem_keypair(spc, pc);

    pack_pubkey(pub_key, spc);
    pack_privkey(priv_key, pc);
    
    return 0;
}

int crypto_kex_encrypt(
          unsigned char *sec_key,
          unsigned char *ct,
    const unsigned char *pub_key) {
    unsigned char err_bytes[ERROR_BYTES];
    error_t err;
    poly_t pub_syn;
    sys_par_ch_t spc;

    unpack_pubkey(spc, pub_key);

    kem_gen_err(err);
    kem_encrypt(pub_syn, err, spc);
    pack_poly(ct, pub_syn);

    pack_err(err_bytes, err);
    crypto_hash_sha256(sec_key, err_bytes, ERROR_BYTES);
    
    return 0;
}

int crypto_kex_encrypt_open(
          unsigned char *sec_key,
    const unsigned char *ct,
    const unsigned char *priv_key) {
    int ret_val;
    unsigned char err_bytes[ERROR_BYTES];
    error_t err = {{0}};
    poly_t pub_syn;
    par_ch_t pc;

    unpack_poly(pub_syn, ct);
    unpack_privkey(pc, priv_key);

    ret_val = kem_decrypt(err, pub_syn, pc);

    pack_err(err_bytes, err);
    crypto_hash_sha256(sec_key, err_bytes, ERROR_BYTES);

    return ret_val;
}
