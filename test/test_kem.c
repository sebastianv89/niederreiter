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
#include "sp_poly.h"
#include "error.h"
#include "util.h"

static unsigned int verbose = 0;

struct test_error {
    error_t err;
    poly_t pub_syn;
};

struct test_key {
    par_ch_t priv_key;
    sys_par_ch_t pub_key;
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
        pack_privkey(buf, key->priv_key);
        fwrite(buf, PRIVATE_KEY_BYTES, 1, fout);
        pack_pubkey(buf, key->pub_key);
        fwrite(buf, PUBLIC_KEY_BYTES, 1, fout);
        buf[0] = (unsigned char)(key->nr_of_errors >> 8);
        buf[1] = (unsigned char)(key->nr_of_errors);
        for (e = 0; e < key->nr_of_errors; ++e) {
            const struct test_error *te = &key->errors[e];
            sp_error_t sp_error;
            error_to_sp_error(sp_error, te->err);
            pack_sp_error(buf, sp_error);
            fwrite(buf, ERROR_WEIGHT * INDEX_BYTES, 1, fout);
            pack_poly(buf, te->pub_syn);
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
        struct test_key *tk= &data->keys[k];
        fread(buf, PRIVATE_KEY_BYTES, 1, fin);
        unpack_privkey(tk->priv_key, buf);
        fread(buf, PUBLIC_KEY_BYTES, 1, fin);
        unpack_pubkey(tk->pub_key, buf);
        fread(buf, INDEX_BYTES, 1, fin);
        tk->nr_of_errors = (uint16_t)(buf[0] << 8 | buf[1]);
        tk->errors = malloc(tk->nr_of_errors * sizeof(struct test_error));
        for (e = 0; e < tk->nr_of_errors; ++e) {
            struct test_error *te = &tk->errors[e];
            sp_error_t sp_error;
            fread(buf, ERROR_WEIGHT * INDEX_BYTES, 1, fin);
            unpack_sp_error(sp_error, buf);
            sp_error_to_error(te->err, sp_error);
            fread(buf, POLY_BYTES, 1, fin);
            unpack_poly(te->pub_syn, buf);
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
        struct test_key *tk = &data->keys[k];
        data->keys->nr_of_errors = nr_of_errs;
        data->keys->errors = malloc(nr_of_errs * sizeof(struct test_error));

        kem_keypair(tk->pub_key, tk->priv_key);
        for (e = 0; e < nr_of_errs; ++e) {
            struct test_error *te = &tk->errors[e];
            kem_gen_err(te->err);
            kem_encrypt(te->pub_syn, te->err, tk->pub_key);
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

bool spc_eq(const sys_par_ch_t a, const sys_par_ch_t b) {
    size_t i;
    for (i = 0; i < POLY_COUNT - 1; ++i) {
        if (!poly_eq(a[i], b[i])) {
            return false;
        }
    }
    return true;
}

void test_with_data(struct test_data *td) {
    size_t k, e;
    sys_par_ch_t pub_key;
    poly_t inv, pub_syn;
    error_t err;

    for (k = 0; k < td->nr_of_keys; ++k) {
        int inv_failed;
        struct test_key *tk = &td->keys[k];
        kem_transpose_par_ch(tk->priv_key);
        sp_poly_to_poly(inv, tk->priv_key[POLY_COUNT - 1]);
        inv_failed = poly_inv(inv);
        assert(!inv_failed);
        kem_to_systematic(pub_key, inv, tk->priv_key);
        assert(spc_eq(pub_key, tk->pub_key));
        kem_transpose_par_ch(tk->priv_key);
        for (e = 0; e < tk->nr_of_errors; ++e) {
            struct test_error *te = &tk->errors[e];
            kem_encrypt(pub_syn, te->err, pub_key);
            assert(poly_eq(pub_syn, te->pub_syn));
            kem_decrypt(err, pub_syn, tk->priv_key);
            assert(err_eq(err, te->err));
        }
    }
}

int main(int argc, char *argv[]) {
    int opt;
    bool random_data = true;
    bool gen_data = false;
    unsigned int gen_keys = 1, gen_errs = 1;
    struct test_data *data = NULL;

    while ((opt = getopt(argc, argv, "gk:e:dv")) != -1) {
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
