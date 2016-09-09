#ifndef NIEDERREITER_OQS_KEX_H
#define NIEDERREITER_OQS_KEX_H

#include <oqs/kex.h>
#include <oqs/rand.h>

OQS_KEX *OQS_KEX_niederreiter_new(
          OQS_RAND *rand,
    const uint8_t   seed, const size_t seed_len);

int OQS_KEX_niederreiter_alice_0(
          OQS_KEX  *k,
          void    **alice_priv,
          uint8_t **alice_msg, size_t *alice_msg_len);

int OQS_KEX_niederreiter_bob(
          OQS_KEX  *k,
    const uint8_t **alice_msg, const size_t  alice_msg_len,
          uint8_t **bob_msg,         size_t *bob_msg_len,
          uint8_t **key,             size_t *key_len);

int OQS_KEX_niederreiter_alice1(
          OQS_KEX  *k,
    const void     *alice_priv,
    const uint8_t **bob_msg,    const size_t  bob_msg_len,
          uint8_t **key,              size_t *key_len);

void OQS_KEX_niederreiter_alice_priv_free(
          OQS_KEX  *k,
          void     *alice_priv);

void OQS_KEX_niederreiter_free(
          OQS_KEX  *k);


#endif /* NIEDERREITER_OQS_KEX_H */
