#ifndef NIEDERREITER_KEX_H
#define NIEDERREITER_KEX_H

#include "config.h"

/** Generate random key pair */
void crypto_kex_keypair(
          unsigned char *pub_key,
          index_t       (*priv_key)[POLY_WEIGHT]);

/** Generate random secret key and encrypt using */
void crypto_kex_encrypt(
          unsigned char *ct,
          unsigned char *sec_key,
    const unsigned char *pub_key);

int crypto_kex_encrypt_open(
          unsigned char *sec_key,
    const unsigned char *ct,
    const index_t       (*priv_key)[POLY_WEIGHT]);

#endif /* NIEDERREITER_KEX_H */
