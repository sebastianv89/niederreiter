#ifndef NIEDERREITER_KEX_H
#define NIEDERREITER_KEX_H

#include "config.h"

/* TODO: documentation */
/* TODO: add lengths to interface? */

void crypto_kex_keypair(
          unsigned char *pub_key,
          index_t       *priv_key);

void crypto_kex_encrypt(
          unsigned char *ct,
          unsigned char *sec_key,
    const unsigned char *pub_key);

int crypto_kex_encrypt_open(
          unsigned char *sec_key,
    const unsigned char *ct,
    const index_t       *priv_key);

#endif /* NIEDERREITER_KEX_H */
