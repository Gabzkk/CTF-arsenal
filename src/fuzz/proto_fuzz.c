/*
 * proto_fuzz.c — Generic protocol frame mutator
 * POSIX C99 | Public Domain (Unlicense)
 *
 * Mutation strategies: bit-flip, byte-replace, boundary values, format strings,
 * length field corruption, duplication, truncation.
 */
#define _POSIX_C_SOURCE 200809L
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../../src/core/base.h"
#include "../../src/core/utils.h"

/* ── Mutation result ──────────────────────────────────────────────────────── */
typedef struct {
    unsigned char *data;
    size_t         len;
    const char    *desc;
} fuzz_frame_t;

static const uint8_t BOUNDARY_BYTES[] = {
    0x00, 0x01, 0x7f, 0x80, 0xfe, 0xff
};
static const char *FORMAT_STRINGS[] = {
    "%s%s%s%s%s", "%n%n%n%n", "%x%x%x%x",
    "AAAA%p%p%p", "%1024d%n", NULL
};

/* ── Mutation engines ─────────────────────────────────────────────────────── */
fuzz_frame_t *fuzz_bit_flip(const unsigned char *data, size_t len) {
    fuzz_frame_t *f = malloc(sizeof(*f));
    f->data = malloc(len); f->len = len;
    memcpy(f->data, data, len);
    size_t pos = rand() % (len * 8);
    f->data[pos / 8] ^= (1 << (pos % 8));
    f->desc = "bit-flip";
    return f;
}

fuzz_frame_t *fuzz_boundary(const unsigned char *data, size_t len) {
    fuzz_frame_t *f = malloc(sizeof(*f));
    f->data = malloc(len); f->len = len;
    memcpy(f->data, data, len);
    size_t pos  = rand() % len;
    f->data[pos] = BOUNDARY_BYTES[rand() % ARRAY_SIZE(BOUNDARY_BYTES)];
    f->desc = "boundary-byte";
    return f;
}

fuzz_frame_t *fuzz_format_string(const unsigned char *data, size_t len) {
    const char *fstr = FORMAT_STRINGS[rand() % 5];
    size_t flen = strlen(fstr);
    size_t ins  = rand() % (len + 1);
    fuzz_frame_t *f = malloc(sizeof(*f));
    f->len  = len + flen;
    f->data = malloc(f->len);
    memcpy(f->data, data, ins);
    memcpy(f->data + ins, fstr, flen);
    memcpy(f->data + ins + flen, data + ins, len - ins);
    f->desc = "format-string";
    return f;
}

fuzz_frame_t *fuzz_truncate(const unsigned char *data, size_t len) {
    fuzz_frame_t *f = malloc(sizeof(*f));
    size_t new_len  = 1 + (rand() % MAX(1, len - 1));
    f->data = malloc(new_len); f->len = new_len;
    memcpy(f->data, data, new_len);
    f->desc = "truncate";
    return f;
}

fuzz_frame_t *fuzz_duplicate(const unsigned char *data, size_t len) {
    fuzz_frame_t *f = malloc(sizeof(*f));
    f->len  = len * 2;
    f->data = malloc(f->len);
    memcpy(f->data,       data, len);
    memcpy(f->data + len, data, len);
    f->desc = "duplicate";
    return f;
}

void fuzz_frame_free(fuzz_frame_t *f) { if (f) { free(f->data); free(f); } }

/* ── Dispatch table ───────────────────────────────────────────────────────── */
typedef fuzz_frame_t *(*mutator_fn)(const unsigned char*, size_t);
static const mutator_fn MUTATORS[] = {
    fuzz_bit_flip, fuzz_boundary, fuzz_format_string,
    fuzz_truncate, fuzz_duplicate
};

fuzz_frame_t *fuzz_random(const unsigned char *data, size_t len) {
    int idx = rand() % ARRAY_SIZE(MUTATORS);
    return MUTATORS[idx](data, len);
}
