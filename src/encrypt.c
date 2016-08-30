#include "crypto_hash_sha512.h"
#include "crypto_encrypt.h"

#include "encrypt.h"
#include "config.h"
#include "pack.h"
#include "kem.h"
#include "dem.h"

int crypto_encrypt_keypair(
    unsigned char *pub_key,
    unsigned char *priv_key)
{
    word_t sys_par_ch[PUBLIC_KEY_WORDS];
    index_t par_ch[PRIVATE_KEY_WEIGHT];

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
    word_t error[ERROR_WORDS];
    unsigned char error_bytes[ERROR_BYTES];
    unsigned char sec_key[SECRET_KEY_BYTES];
    unsigned char *dem_ct = ct + POLY_BYTES;
    unsigned long long dem_ct_len;
    word_t syndrome[SYNDROME_WORDS];
    word_t sys_par_ch[PUBLIC_KEY_WORDS];

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
    word_t error[ERROR_WORDS];
    word_t syndrome[POLY_WORDS];
    index_t par_ch[PRIVATE_KEY_WEIGHT];
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


#if defined(ENCRYPT_MAIN)

#include "debug.h"

#define MSG_BYTES 13
#define CT_LEN (MSG_BYTES + crypto_hash_sha512_BYTES + SYNDROME_BYTES)

int main(void) {
    unsigned char pub_key[PUBLIC_KEY_BYTES];
    unsigned char priv_key[PRIVATE_KEY_BYTES];
    unsigned char msg[MSG_BYTES] = "Hello World!";
    unsigned long long ct_len = CT_LEN;
    unsigned char ct[CT_LEN];
    unsigned long long pt_len;
    unsigned char pt[MSG_BYTES];
    
    crypto_encrypt_keypair(pub_key, priv_key);
    crypto_encrypt(ct, &ct_len, msg, MSG_BYTES, pub_key);
    crypto_encrypt_open(pt, &pt_len, ct, ct_len, priv_key);
    
    printf("msg: %s (", msg);
    print_bytes(msg, MSG_BYTES);
    printf(")\nct(%llu): ", ct_len);
    print_bytes(ct, ct_len);
    printf("\npt(%llu): %s (", pt_len, pt);
    print_bytes(pt, pt_len);
    printf(")\n");
    
    return 0;
}

#endif /* ENCRYPT_MAIN */
