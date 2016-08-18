#ifndef NIEDERREITER_ENCRYPT_H
#define NIEDERREITER_ENCRYPT_H

int crypto_encrypt_keypair(
    unsigned char *pub_key,
    unsigned char *priv_key);

int crypto_encrypt(
          unsigned char *ct,  unsigned long long *ct_len,
    const unsigned char *msg, unsigned long long  msg_len,
    const unsigned char *pub_key);

int crypto_encrypt_open(
          unsigned char *msg, unsigned long long *msg_len,
    const unsigned char *ct,  unsigned long long ct_len,
    const unsigned char *priv_key);

#endif /* NIEDERREITER_ENCRYPT_H */
