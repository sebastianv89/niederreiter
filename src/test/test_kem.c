#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <inttypes.h>
#include <getopt.h>

#include "kem.h"
#include "config.h"
#include "pack.h"
#include "poly.h"
#include "poly_sparse.h"
#include "error.h"
#include "util.h"

static bool verbose = false;

struct test_error {
    word_t error[NUMBER_OF_POLYS][POLY_WORDS];
    word_t pub_syn[POLY_WORDS];
};

struct test_key {
    index_t priv_key[NUMBER_OF_POLYS][POLY_WEIGHT];
    word_t pub_key[NUMBER_OF_POLYS - 1][POLY_WORDS];
    size_t nr_of_errors;
    struct test_error *errors;
};

struct test_data {
    size_t nr_of_keys;
    struct test_key *keys;
};

/* test data format:
 *   - nr_of_keys (short)
 *     - packed private key, transposed (bytes)
 *     - packed public key (bytes)
 *     - nr_of_errors (short)
 *       - packed sparse error (bytes)
 *       - public syndrome (bytes)
 */

void write_data(FILE *fout, const struct test_data *data) {
    unsigned char buf[PUBLIC_KEY_BYTES];
    uint16_t k, e;

    buf[0] = (unsigned char)(data->nr_of_keys >> 8);
    buf[1] = (unsigned char)(data->nr_of_keys);
    fwrite(buf, INDEX_BYTES, 1, fout);
    for (k = 0; k < data->nr_of_keys; ++k) {
        const struct test_key *key = &data->keys[k];
        pack_priv_key(buf, key->priv_key);
        fwrite(buf, PRIVATE_KEY_BYTES, 1, fout);
        pack_pub_key(buf, key->pub_key);
        fwrite(buf, PUBLIC_KEY_BYTES, 1, fout);
        buf[0] = (unsigned char)(key->nr_of_errors >> 8);
        buf[1] = (unsigned char)(key->nr_of_errors);
        for (e = 0; e < key->nr_of_errors; ++e) {
            const struct test_error *err = &key->errors[e];
            index_t error_sparse[ERROR_WEIGHT];
            size_t weight;
            error_to_sparse(error_sparse, &weight, err->error);
            pack_error_sparse(buf, error_sparse);
            fwrite(buf, ERROR_WEIGHT * INDEX_BYTES, 1, fout);
            pack_syndrome(buf, err->pub_syn);
            fwrite(buf, POLY_BYTES, 1, fout);
        }
    }
}

struct test_data *parse_data(FILE *fin) {
    unsigned char buf[PUBLIC_KEY_BYTES];
    uint16_t k, e;
    struct test_data *data;

    data = malloc(sizeof(struct test_data));
    fread(buf, INDEX_BYTES, 1, fin);
    data->nr_of_keys = (uint16_t)(buf[0] << 8 | buf[1]);
    data->keys = malloc(data->nr_of_keys * sizeof(struct test_key));
    for (k = 0; k < data->nr_of_keys; ++k) {
        struct test_key *key = &data->keys[k];
        fread(buf, PRIVATE_KEY_BYTES, 1, fin);
        unpack_priv_key(key->priv_key, buf);
        fread(buf, PUBLIC_KEY_BYTES, 1, fin);
        unpack_pub_key(key->pub_key, buf);
        fread(buf, INDEX_BYTES, 1, fin);
        key->nr_of_errors = (uint16_t)(buf[0] << 8 | buf[1]);
        key->errors = malloc(key->nr_of_errors * sizeof(struct test_error));
        for (e = 0; e < key->nr_of_errors; ++e) {
            struct test_error *err = &key->errors[e];
            index_t error_sparse[ERROR_WEIGHT];
            fread(buf, ERROR_WEIGHT * INDEX_BYTES, 1, fin);
            unpack_error_sparse(error_sparse, buf);
            err_to_dense(err->error, error_sparse);
            fread(buf, POLY_BYTES, 1, fin);
            unpack_syndrome(err->pub_syn, buf);
        }
    }

    return data;
}

struct test_data *rand_data(unsigned int nr_of_keys, unsigned int nr_of_errs) {
    size_t k, e;
    struct test_data *data;

    data = malloc(sizeof(struct test_data));
    data->nr_of_keys = nr_of_keys;
    data->keys = malloc(nr_of_keys * sizeof(struct test_key));
    for (k = 0; k < nr_of_keys; ++k) {
        struct test_key *key = &data->keys[k];
        data->keys->nr_of_errors = nr_of_errs;
        data->keys->errors = malloc(nr_of_errs * sizeof(struct test_error));

        kem_keypair(key->pub_key, key->priv_key);
        for (e = 0; e < nr_of_errs; ++e) {
            struct test_error *err = &key->errors[e];
            kem_gen_error(err->error);
            kem_encrypt(err->pub_syn, err->error, key->pub_key);
        }
    }

    return data;
}

void free_data(struct test_data *data) {
    size_t k;
    for (k = 0; k < data->nr_of_keys; ++k) {
        free(data->keys[k].errors);
    }
    free(data->keys);
    free(data);
}

bool pub_key_eq(word_t (*a)[POLY_WORDS], word_t (*b)[POLY_WORDS]) {
    size_t i;
    for (i = 0; i < POLY_WORDS - 1; ++i) {
        if (!poly_eq(a[i], b[i])) {
            return false;
        }
    }
    return true;
}

bool err_eq(word_t (*a)[POLY_WORDS], word_t (*b)[POLY_WORDS]) {
    size_t i;
    for (i = 0; i < POLY_WORDS; ++i) {
        if (!poly_eq(a[i], b[i])) {
            return false;
        }
    }
    return true;
}

void test_with_data(struct test_data *data) {
    size_t k, e;
    word_t pub_key[NUMBER_OF_POLYS - 1][POLY_WORDS];
    word_t inv[POLY_WORDS];
    word_t pub_syn[POLY_WORDS];
    word_t error[NUMBER_OF_POLYS][POLY_WORDS];

    for (k = 0; k < data->nr_of_keys; ++k) {
        int inv_failed;
        struct test_key *key = &data->keys[k];
        kem_transpose_privkey(key->priv_key);
        polsp_to_dense(inv, key->priv_key[NUMBER_OF_POLYS - 1]);
        inv_failed = poly_inv(inv);
        assert(!inv_failed);
        kem_to_systematic(pub_key, inv, key->priv_key);
        assert(pub_key_eq(pub_key, key->pub_key));
        kem_transpose_privkey(key->priv_key);
        for (e = 0; e < key->nr_of_errors; ++e) {
            struct test_error *err = &key->errors[e];
            kem_encrypt(pub_syn, err->error, pub_key);
            assert(poly_eq(pub_syn, err->pub_syn));
            kem_decrypt(error, pub_syn, key->priv_key);
            assert(err_eq(error, err->error));
        }
    }
}

int main(int argc, char *argv[]) {
    int opt;
    bool random_data = true;
    bool gen_data = false;
    unsigned int gen_keys = 1, gen_errs = 1;
    struct test_data *data = NULL;

    while ((opt = getopt(argc, argv, "gkedv")) != -1) {
        switch (opt) {
        case 'g': gen_data = true; break;
        case 'k': gen_keys = (unsigned int)(atoi(optarg)); break;
        case 'e': gen_errs = (unsigned int)(atoi(optarg)); break;
        case 'd': random_data = false; break;
        case 'v': verbose = true; break;
        }
    }

    if (random_data) {
        data = rand_data(gen_keys, gen_errs);
        test_with_data(data);
    } else if (gen_data) {
        data = rand_data(gen_keys, gen_errs);
        write_data(stdout, data);
    } else {
        data = parse_data(stdin);
        test_with_data(data);
    }

    free_data(data);
    data = NULL;

    return EXIT_SUCCESS;
}
