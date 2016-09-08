#include "crypto_hash_sha256.h" /* TODO: use SHA-3 (Keccak) */

#include "kex.h"
#include "config.h"
#include "kem.h"
#include "pack.h"

void crypto_kex_keypair(unsigned char *pub_key,
                        index_t (*priv_key)[POLY_WEIGHT]) {
    word_t sys_par_ch[NUMBER_OF_POLYS - 1][POLY_WORDS];
    kem_keypair(sys_par_ch, priv_key);
    pack_pub_key(pub_key, sys_par_ch);
}

void crypto_kex_encrypt(unsigned char *ct,
                        unsigned char *sec_key,
                        const unsigned char *pub_key)
{
    word_t error[NUMBER_OF_POLYS][POLY_WORDS];
    unsigned char error_bytes[ERROR_BYTES];
    word_t syndrome[POLY_WORDS];
    word_t sys_par_ch[NUMBER_OF_POLYS - 1][POLY_WORDS];
    
    unpack_pub_key(sys_par_ch, pub_key);

    kem_gen_error(error);
    kem_encrypt(syndrome, error, sys_par_ch);
    pack_error(error_bytes, error);
    crypto_hash_sha256(sec_key, error_bytes, ERROR_BYTES);

    pack_syndrome(ct, syndrome);
}

int crypto_kex_encrypt_open(unsigned char *sec_key,
                            const unsigned char *ct, 
                            const index_t (*priv_key)[POLY_WEIGHT])
{
    int ret_val;
    word_t error[NUMBER_OF_POLYS][POLY_WORDS];
    unsigned char error_bytes[ERROR_BYTES];
    word_t syndrome[POLY_WORDS];
    
    unpack_syndrome(syndrome, ct);
    
    ret_val = kem_decrypt(error, syndrome, priv_key);
    pack_error(error_bytes, error);
    crypto_hash_sha256(sec_key, error_bytes, ERROR_BYTES);

    return ret_val;
}


#if defined(KEX_MAIN)

/* TODO: move to test */

#include <stdio.h>
#include "debug.h"

#define SECRET_KEY_BYTES crypto_hash_sha256_BYTES

int main(void) {
    unsigned char pub_key[PUBLIC_KEY_BYTES];
    index_t priv_key[NUMBER_OF_POLYS][POLY_WEIGHT];
    unsigned char secret_key[SECRET_KEY_BYTES];
    unsigned char pt[SECRET_KEY_BYTES];
    unsigned char ct[POLY_BYTES];
    int ret_val;
    
    crypto_kex_keypair(pub_key, priv_key);
    crypto_kex_encrypt(ct, secret_key, pub_key);
    ret_val = crypto_kex_encrypt_open(pt, ct, priv_key);
    
    printf("session key:\n");
    print_bytes(secret_key, SECRET_KEY_BYTES);
    printf("\nct:\n");
    print_bytes(ct, POLY_BYTES);
    printf("\npt (%s):\n", ret_val ? "failed" : "succeeded");
    print_bytes(pt, SECRET_KEY_BYTES);
    printf("\n");
    
    return 0;
}

#endif /* ENCRYPT_MAIN */
