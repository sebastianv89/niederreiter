#include "crypto_hash_sha512.h"

#include "pack.h"

#define CRYPTO_SECRETKEYBYTES PRIVATE_KEY_BYTES
#define CRYPTO_PUBLICKEYBYTES PUBLIC_KEY_BYTES
#define CRYPTO_BYTES (SYNDROME_BYTES + crypto_hash_sha512_BYTES)

#define CRYPTO_VERSION 0.1.0
