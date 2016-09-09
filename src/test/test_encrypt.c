#if defined(ENCRYPT_MAIN)

#include "debug.h"

#define MSG_BYTES 13
#define CT_LEN (MSG_BYTES + crypto_hash_sha512_BYTES + SYNDROME_BYTES)

int main(void) {
    unsigned char pub_key[PUBLIC_KEY_BYTES];
    unsigned char priv_key[PRIVATE_KEY_BYTES];
    unsigned char msg[MSG_BYTES] = "Hello World!";
    unsigned long long ct_len = CT_LEN;
    unsigned char ct[CT_LEN];
    unsigned long long pt_len;
    unsigned char pt[MSG_BYTES];

    crypto_encrypt_keypair(pub_key, priv_key);
    crypto_encrypt(ct, &ct_len, msg, MSG_BYTES, pub_key);
    crypto_encrypt_open(pt, &pt_len, ct, ct_len, priv_key);

    printf("msg: %s (", msg);
    print_bytes(msg, MSG_BYTES);
    printf(")\nct(%llu): ", ct_len);
    print_bytes(ct, ct_len);
    printf("\npt(%llu): %s (", pt_len, pt);
    print_bytes(pt, pt_len);
    printf(")\n");

    return 0;
}

#endif /* ENCRYPT_MAIN */


