#ifndef NIEDERREITER_KEM_H
#define NIEDERREITER_KEM_H

#include "config.h"

/* TODO: documentation */

void kem_keypair(
    word_t *pub_key,
    index_t *priv_key);

void kem_gen_error(
    index_t *error);

void kem_encrypt(
          word_t *pub_syn,
    const index_t *error,
    const word_t *pub_key);

int kem_decrypt(
          word_t *error,
    const word_t *pub_syn,
    const index_t *priv_key);

#endif /* NIEDERREITER_KEM_H */
