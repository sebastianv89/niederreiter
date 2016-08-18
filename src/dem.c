#include <string.h>

#include "crypto_stream_salsa20.h"
#include "crypto_onetimeauth_poly1305.h"

#include "dem.h"

static const unsigned char nonce[crypto_stream_salsa20_NONCEBYTES] = {0};

void dem_encrypt(
          unsigned char *ct,  unsigned long long *ct_len,
    const unsigned char *msg, unsigned long long  msg_len,
    const unsigned char *sec_key)
{
    unsigned char *auth = ct + msg_len;
    const unsigned char *auth_key = sec_key + crypto_stream_salsa20_KEYBYTES;

    *ct_len = msg_len + crypto_onetimeauth_poly1305_BYTES;
    crypto_stream_xor_salsa20(ct, msg, msg_len, nonce, sec_key);
    crypto_onetimeauth_poly1305(auth, ct, msg_len, auth_key);
}

int dem_decrypt(
          unsigned char *msg, unsigned long long *msg_len,
    const unsigned char *ct,  unsigned long long  ct_len,
    const unsigned char *sec_key)
{
    int ret_val;
    const unsigned char *auth;
    const unsigned char *auth_key = sec_key + crypto_stream_salsa20_KEYBYTES;

    *msg_len = ct_len - crypto_onetimeauth_poly1305_BYTES;
    auth = ct + *msg_len;
    ret_val = crypto_onetimeauth_verify_poly1305(auth, ct, msg_len, auth_key);
    if (ret_val) {
        return ret_val;
    }
    return crypto_stream_xor_salsa20(msg, ct, ct_len, nonce, sec_key);
}
