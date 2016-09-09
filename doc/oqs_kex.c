#include <stdlib.h>

#include <oqs/kex.h>
#include <oqs/rand.h>

#include "config.h"
#include "kex.h"

#define UNUSED __attribute__((unused))

/* TODO: change the order of the API parameters.  A more common
 * order is to put output parameters first, then input parameters.
 */

/* TODO: change the return booleans to error codes.  Right now we have
 * 2^16-2 ways to tell everything went correct and only one way to say
 * that something went wrong.  For example, assume I want to let the
 * user know if decryption failed because of a memory allocation
 * failure or a decryption failure.  With a boolean there is no way of
 * doing that, but with an error code there is.
 */ 

/* TODO: remove seed_len const qualifier (it's call-by-value anyway)? */

/* TODO: the current API will likely lead to memory leaks.  An unaware
 * user might suspect that everything is cleaned up after calling
 * OQS_KEX_free(), but that leaves `alice_priv`, `alice_msg`,
 * `bob_msg` and `key` (twice).  When that user notices that we (only)
 * provide `alice_priv_free`, they might call this and think they're
 * done with it.
 * 
 * The alternative is to go with something similar to NaCl: we assume
 * that the user has pre-allocated memory when calling the function
 * (and we provide macro's (or something similar) to let them know
 * exactly how much memory they should provide.  The big advantage is
 * that the user needs to allocate the memory themselves and is thus
 * less likely to forget to free the memory (and if they do, it is
 * more obvious that this really is a user error, not a library
 * error).
 */

/* TODO: implicit conversion between uint8_t and unsigned char */

/* TODO: init \p rand with seed */
OQS_KEX *OQS_KEX_niederreiter_new(
          OQS_RAND *rand,
    const uint8_t   seed, const size_t seed_len)
{
    OQS_KEX *k = malloc(sizeof(struct OQS_KEX));
    if (k == NULL) {
        return NULL;
    }
    k->method_name = "qc-mdpc-niederreiter";
    k->estimated_classical_security = 80;
    k->estimated_quantum_security = 0; /* unknown */
    k->rand = rand;
    k->params = NULL;
    k->alice_0 = &OQS_KEX_niederreiter_alice0;
    k->bob = &OQS_KEX_niederreiter_bob;
    k->alice_1 = &OQS_KEX_niederreiter_alice1;
    k->alice_priv_free = &OQS_KEX_niederreiter_priv_free;
    k->free = &OQS_KEX_niederreiter_free;
    return k;
}

/* TODO: pass k->rand to keygen */
int OQS_KEX_niederreiter_alice_0(
          OQS_KEX  *k,
          void    **alice_priv,
          uint8_t **alice_msg, size_t *alice_msg_len)
{
    /* allocate private key */
    *alice_priv = malloc(PRIVATE_KEY_BYTES);
    if (priv_key == NULL) {
        return 0;
    }
    
    /* allocate public key */
    *alice_msg_len = PUBLIC_KEY_BYTES;
    *alice_msg = malloc(*alice_msg_len);
    if (*alice_msg == NULL) {
        free(priv_key);
        return 0;
    }
    
    /* generate keypair */
    crypto_kex_keypair(*alice_msg, (index_t *)(*alice_priv));
    
    return 1;
}

/* TODO: pass k->rand to encrypt */ 
int OQS_KEX_niederreiter_bob(
          OQS_KEX  *k,
    const uint8_t **alice_msg, const size_t  alice_msg_len,
          uint8_t **bob_msg,         size_t *bob_msg_len,
          uint8_t **key,             size_t *key_len)
{
    /* allocate session key */
    *key_len = SECRET_KEY_BYTES;
    *key = malloc(*key_len);
    if (*key == NULL) {
        return 0;
    }

    /* allocate ciphertext */
    *bob_msg_len = SYNDROME_BYTES;
    *bob_msg = malloc(*bob_msg_len);
    if (*bob_msg == NULL) {
        free(*key);
        return 0;
    }
    
    /* generate random session key and encrypt to alice */
    crypto_kex_encrypt(*bob_msg, *key, *alice_msg);
    
    return 1;
}

int OQS_KEX_niederreiter_alice1(
    UNUSED OQS_KEX  *k,
     const void     *alice_priv,
     const uint8_t **bob_msg,    const size_t  bob_msg_len,
           uint8_t **key,              size_t *key_len)
{
    /* allocate session key */
    *key_len = SECRET_KEY_BYTES;
    *key = malloc(*key_len);
    if (*key == NULL) {
        return 0;
    }
    
    /* decrypt session key */
    if (crypto_kex_encrypt_open(*key,
                                *bob_msg,
                                (index_t *)alice_priv)) {
        free(key);
        return 0;
    }
    
    return 1;
}

void OQS_KEX_niederreiter_alice_priv_free(
    UNUSED OQS_KEX  *k,
           void     *alice_priv)
{
    free(alice_priv);
}

void OQS_KEX_niederreiter_free(
          OQS_KEX  *k)
{
    free(k);
}
