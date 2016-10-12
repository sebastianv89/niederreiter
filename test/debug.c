#include <stdio.h>

#include "config.h"
#include "types.h"
#include "util.h"
#include "kem.h"
#include "poly.h"
#include "sp_poly.h"
#include "error.h"

#include "api.h"
#include "encrypt.h"

#define DEBUG_DIR "/home/sebastian/oqs/niederreiter/data/"

#define PRINT(fname, print_func, obj) {           \
        f = fopen(DEBUG_DIR fname, "w");          \
        print_func(f, obj);                       \
        fclose(f);                                \
    }

int main(void) {
    FILE *f;
    poly_t inv;
    par_ch_t privkey;
    poly_t privkey_dense[2];
    sys_par_ch_t pubkey;
    sp_error_t sp_error;
    error_t error = {{0}}, error2 = {{0}};
    syn_t pub_syn, priv_syn;
    int dec_fail = 0;

    /* kem_gen_keypair */
    kem_gen_par_ch(inv, privkey);
    PRINT("privkey.txt",  fprintparch,    privkey);
    sp_poly_to_poly(privkey_dense[0], privkey[0]);
    sp_poly_to_poly(privkey_dense[1], privkey[1]);
    PRINT("privkey_0.txt",fprintpoly, privkey_dense[0]);
    PRINT("privkey_1.txt",fprintpoly, privkey_dense[1]);
    PRINT("inv.txt",      fprintpoly,     inv);
    kem_to_systematic(pubkey, inv, privkey);
    PRINT("pubkey.txt",   fprintsysparch, pubkey);
    
    // kem_gen_err
    sp_gen_error(sp_error);
    sp_error_to_error(error, sp_error);
    PRINT("sp_error.txt", fprintsperror,  sp_error);
    PRINT("error.txt",    fprinterror,    error);
    
    kem_encrypt(pub_syn, error, pubkey);
    PRINT("pubsyn.txt",   fprintpoly,     pub_syn);
    
    // kem_decrypt
    poly_mul(priv_syn, privkey_dense[1], pub_syn);
    PRINT("privsyn.txt",  fprintpoly,     priv_syn);
    kem_decode(error2, priv_syn, privkey);
    PRINT("error2.txt",fprinterror,    error2);
    dec_fail = poly_verify_zero(priv_syn);

    if (dec_fail) {
        puts("Decoding failure");
    } else if (err_eq(error, error2)) {
        puts("YEAH!!!!");
    } else {
        puts("nope");
    }
    
    {
        unsigned char pub_key[CRYPTO_PUBLICKEYBYTES];
        unsigned char priv_key[CRYPTO_SECRETKEYBYTES];
        unsigned char msg[14] = "Hello world!\n";
        unsigned char ct[14 + CRYPTO_BYTES];
        unsigned char pt[14];
        unsigned long long mlen = 14, ctlen, ptlen, i;

        crypto_encrypt_keypair(pub_key, priv_key);
        crypto_encrypt(ct, &ctlen, msg, mlen, pub_key);
        crypto_encrypt_open(pt, &ptlen, ct, ctlen, priv_key);
        for (i = 0; i < ptlen; ++i) {
            putchar(pt[i]);
        }
    }
    
    return EXIT_SUCCESS;
}
