#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "kem.h"
#include "error.h"
#include "poly.h"
#include "sp_poly.h"
#include "util.h"

int main(void) {
    sys_par_ch_t pub_key;
    par_ch_t priv_key;
    error_t error, error_decrypted = {{0}}, e = {{0}};
    poly_t pub_syn;
    int decode_failure;
    poly_t priv_last, priv_syn;

    /*
    esp_t esp;
    err_t err;
    esp_rand(esp);
    esp_to_err(err, esp);
    printf("\nesp:\n"); fprintesp(stdout, esp); putchar('\n');
    printf("\nerr:\n"); fprinterr(stdout, err); putchar('\n');
    */

    /*
    poly_t inv;
    sys_par_ch_t pubk;
    par_ch_t privk;
    kem_rand_pc(inv, privk);
    printf("\nprivk:\n"); fprintprivkey(stdout, privk); putchar('\n');
    printf("\ninv:\n"); fprintpoly(stdout, inv); putchar('\n');
    kem_to_systematic(pubk, inv, privk);
    printf("\npubk:\n"); fprintpubkey(stdout, pubk); putchar('\n');
    kem_transpose_pc(privk);
    printf("\nprivk:\n"); fprintprivkey(stdout, privk); putchar('\n');
    */
    
    kem_keypair(pub_key, priv_key);
//    printf("\npriv_key:\n"); fprintparch(stdout, priv_key); putchar('\n');
//    printf("\npub_key:\n"); fprintsysparch(stdout, pub_key); putchar('\n');
    kem_gen_err(error);
//    printf("\nerror:\n"); fprinterror(stdout, error); putchar('\n');
    kem_encrypt(pub_syn, error, pub_key);
//    printf("\npub_syn:\n"); fprintpoly(stdout, pub_syn); putchar('\n');
    
    sp_poly_to_poly(priv_last, priv_key[POLY_COUNT - 1]);
//    printf("\npriv_last:\n"); fprintpoly(stdout, priv_last); putchar('\n');
    poly_mul(priv_syn, pub_syn, priv_last);
//    printf("\npriv_syn:\n"); fprintpoly(stdout, priv_syn); putchar('\n');
    kem_decode(e, priv_syn, priv_key);
//    printf("\npriv_syn (decoded):\n"); fprintpoly(stdout, priv_syn); putchar('\n');
//    printf("\ne:\n"); fprinterror(stdout, e); putchar('\n');

    decode_failure = kem_decrypt(error_decrypted, pub_syn, priv_key);
    printf("\nfailure: "); fprintf(stdout, "%d\n", decode_failure);
    printf("\nerror:\n"); fprinterror(stdout, error_decrypted); putchar('\n');
    
    if (decode_failure) {
        fprintf(stderr, "Decoding failure\n");
    } else {
        assert(err_eq(error, error_decrypted));
    }

    return EXIT_SUCCESS;
}
