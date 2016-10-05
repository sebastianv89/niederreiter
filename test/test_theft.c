#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "greatest.h"
#include "theft.h"

#define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))

typedef uint8_t byte_t;

static void *
byte_alloc(struct theft *UNUSED(t), 
           theft_seed s,
           void *UNUSED(env)) {
    byte_t *byte = calloc(1, sizeof(byte));
    if (byte == NULL) { return THEFT_ERROR; }
    *byte = (byte_t)(s & 0xff);
    return byte;
}

static void
byte_free(void *byte,
          void *UNUSED(env)) {
    free(byte);
}

static void
byte_print(FILE *f,
           void *byte,
           void *UNUSED(env)) {
    fprintf(f, "%02" PRIx8, (byte_t)byte);
}

static struct theft_type_info byte_type_info = {
    .alloc = byte_alloc,
    .free = byte_free,
    .print = byte_print,
};

static theft_progress_callback_res
guiap_pcb(struct theft_trial_info *UNUSED(info),
          void *UNUSED(env)) {
    return THEFT_PROGRESS_CONTINUE;
}

static theft_trial_res 
prop_true(byte_t *UNUSED(byte)) {
    return THEFT_TRIAL_PASS;
}

TEST
generated_bytes_true() {
    struct theft *t = theft_init(0);
    theft_run_res res = theft_run(t, &(struct theft_cfg){
            .name = __func__,
            .fun = prop_true,
            .type_info = { &byte_type_info },
            .progress_cb = guiap_pcb,
        });
    ASSERT_EQm(__func__, THEFT_RUN_PASS, res);
    theft_free(t);
    PASS();
}

static theft_trial_res 
prop_false(byte_t *UNUSED(byte)) {
    return THEFT_TRIAL_FAIL;
}

TEST
generated_bytes_false() {
    struct theft *t = theft_init(0);
    theft_run_res res = theft_run(t, &(struct theft_cfg){
            .name = __func__,
            .fun = prop_false,
            .type_info = { &byte_type_info },
            .progress_cb = guiap_pcb,
        });
    ASSERT_EQm(__func__, THEFT_RUN_PASS, res);
    theft_free(t);
    PASS();
}

SUITE(suite) {
    RUN_TEST(generated_bytes_true);
    RUN_TEST(generated_bytes_false);
}

GREATEST_MAIN_DEFS();

int main(int argc, char *argv[]) {
    GREATEST_MAIN_BEGIN();
    RUN_SUITE(suite);
    GREATEST_MAIN_END();
}
