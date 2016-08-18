#ifndef NIEDERREITER_DEM_H
#define NIEDERREITER_DEM_H

#include "crypto_stream_salsa20.h"
#include "crypto_onetimeauth_poly1305.h"

#define SECRET_KEY_BYTES \
    (crypto_stream_salsa20_KEYBYTES + \
     crypto_onetimeauth_poly1305_KEYBYTES)

void dem_encrypt(
    unsigned char *ct,
    unsigned long long *ct_len,
    const unsigned char *msg,
    unsigned long long msg_len,
    const unsigned char *sec_key);

int dem_decrypt(
    unsigned char *msg,
    unsigned long long *msg_len,
    const unsigned char *ct,
    unsigned long long ct_len,
    const unsigned char *sec_key);

#endif /* INCLUDED_DEM_H */
