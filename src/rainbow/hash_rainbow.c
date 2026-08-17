/*
 * hash_rainbow.c — Rainbow table chain: hash + reduction functions
 * POSIX C99 + OpenSSL EVP | Public Domain (Unlicense)
 */
#define _POSIX_C_SOURCE 200809L
#include <openssl/evp.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../src/core/base.h"
#include "../../src/core/utils.h"

static const char CHARSET[] =
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "0123456789"
    "!@#$%^&*";
#define CHARSET_LEN  (sizeof(CHARSET)-1)
#define MAX_PLAIN    8
#define HASH_MAX_LEN 32

typedef enum { ALG_MD5 = 0, ALG_SHA1, ALG_SHA256 } hash_alg_t;

static size_t do_hash(hash_alg_t alg, const char *plain, size_t plen,
                       unsigned char *out) {
    const EVP_MD *md;
    switch (alg) {
    case ALG_MD5:    md = EVP_md5();    break;
    case ALG_SHA1:   md = EVP_sha1();   break;
    case ALG_SHA256: md = EVP_sha256(); break;
    default:         md = EVP_md5();    break;
    }
    unsigned int len = 0;
    EVP_Digest(plain, plen, out, &len, md, NULL);
    return (size_t)len;
}

static void reduce(const unsigned char *hash_bytes, size_t hash_len,
                   uint32_t position, int plain_len, char *out) {
    uint64_t idx = 0;
    for (size_t i = 0; i < MIN(8, hash_len); i++)
        idx = (idx << 8) | hash_bytes[i];
    idx ^= (uint64_t)position * 0xdeadbeefULL;
    for (int i = 0; i < plain_len; i++) {
        out[i] = CHARSET[idx % CHARSET_LEN];
        idx /= CHARSET_LEN;
    }
    out[plain_len] = '\0';
}

void rainbow_gen_chain(hash_alg_t alg, const char *start_plain, int plain_len,
                        int chain_len, char *end_plain) {
    char current[MAX_PLAIN+1];
    snprintf(current, MAX_PLAIN+1, "%s", start_plain);
    unsigned char hash_buf[HASH_MAX_LEN];

    for (int pos = 0; pos < chain_len; pos++) {
        size_t hlen = do_hash(alg, current, strlen(current), hash_buf);
        reduce(hash_buf, hlen, pos, plain_len, current);
    }
    snprintf(end_plain, MAX_PLAIN+1, "%s", current);
}

typedef struct { char start[MAX_PLAIN+1]; char end[MAX_PLAIN+1]; } chain_entry_t;

int rainbow_crack(hash_alg_t alg, const unsigned char *target_hash, size_t hash_len,
                   const chain_entry_t *table, size_t table_size,
                   int plain_len, int chain_len, char *result) {
    unsigned char hash_buf[HASH_MAX_LEN];
    char rp[MAX_PLAIN+1], cand[MAX_PLAIN+1];

    for (int col = chain_len - 1; col >= 0; col--) {
        memcpy(hash_buf, target_hash, hash_len);
        reduce(hash_buf, hash_len, col, plain_len, rp);
        for (int pos = col + 1; pos < chain_len; pos++) {
            size_t hl = do_hash(alg, rp, strlen(rp), hash_buf);
            reduce(hash_buf, hl, pos, plain_len, rp);
        }

        for (size_t c = 0; c < table_size; c++) {
            if (strcmp(table[c].end, rp) == 0) {
                snprintf(cand, MAX_PLAIN+1, "%s", table[c].start);
                for (int pos2 = 0; pos2 < chain_len; pos2++) {
                    size_t hl = do_hash(alg, cand, strlen(cand), hash_buf);
                    if (memcmp(hash_buf, target_hash, hash_len) == 0) {
                        snprintf(result, MAX_PLAIN+1, "%s", cand);
                        return 1;
                    }
                    reduce(hash_buf, hl, pos2, plain_len, cand);
                }
            }
        }
    }
    return 0;
}
