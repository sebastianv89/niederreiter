/* TODO (this comes from src/kex.c

#define SECRET_KEY_BYTES crypto_hash_sha256_BYTES

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
*/

#include <stdio.h>
#include <stdlib.h>

#include "test/util.h"

int main(int argc, char *argv[]) {

    return EXIT_SUCCESS;
}
