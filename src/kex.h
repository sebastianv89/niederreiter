#ifndef NIEDERREITER_KEX_H
#define NIEDERREITER_KEX_H

void crypto_kex_keypair(
    unsigned char *pub_key,
    unsigned char *priv_key);

void crypto_kex_encrypt(
          unsigned char *ct,
          unsigned char *sec_key,
    const unsigned char *pub_key);

void crypto_kex_encrypt_open(
          unsigned char *msg,
          unsigned char *sec_key,
    const unsigned char *priv_key);

#endif /* NIEDERREITER_KEX_H */
