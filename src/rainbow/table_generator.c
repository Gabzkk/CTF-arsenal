/*
 * table_generator.c — Generate rainbow table and save to binary file
 * POSIX C99 | Public Domain (Unlicense)
 *
 * Usage: table_generator -a <md5|sha1|sha256> -c <chain_len> -n <num_chains>
 *                         -l <plain_len> -o <output.rt>
 */
#define _POSIX_C_SOURCE 200809L
#include <getopt.h>
#include <time.h>
#include "../../src/core/base.h"
#include "../../src/core/utils.h"

/* ── Inline the chain API ─────────────────────────────────────────────────── */
#include "hash_rainbow.c"

static void rand_plain(char *out, int len) {
    static const char cs[] = "abcdefghijklmnopqrstuvwxyz0123456789";
    for (int i = 0; i < len; i++) out[i] = cs[rand() % (sizeof(cs)-1)];
    out[len] = '\0';
}

/* Binary table format:
 * Header: "CTFRT\0" + uint8 alg + uint8 plain_len + uint32 chain_len + uint64 num_chains
 * Entries: num_chains * (plain_len+1 start + plain_len+1 end)
 */
int main(int argc, char **argv) {
    int alg_id = ALG_MD5, chain_len = 100, num_chains = 10000, plain_len = 6;
    const char *outfile = "rainbow.rt";
    int opt;
    while ((opt = getopt(argc, argv, "a:c:n:l:o:v")) != -1) {
        switch (opt) {
            case 'a':
                if (strcmp(optarg,"sha1")  == 0) alg_id = ALG_SHA1;
                else if (strcmp(optarg,"sha256") == 0) alg_id = ALG_SHA256;
                break;
            case 'c': chain_len  = atoi(optarg); break;
            case 'n': num_chains = atoi(optarg); break;
            case 'l': plain_len  = atoi(optarg); break;
            case 'o': outfile    = optarg; break;
            case 'v': ctf_verbosity = 2; break;
        }
    }

    srand((unsigned)time(NULL));
    PRINT_INFO("Generating %d chains, len=%d, plain_len=%d", num_chains, chain_len, plain_len);
    double t0 = ctf_time_now();

    FILE *f = fopen(outfile, "wb");
    if (!f) { PRINT_FAIL("Cannot open output: %s", outfile); return 1; }

    /* Header */
    fwrite("CTFRT\0", 6, 1, f);
    uint8_t a = (uint8_t)alg_id, pl = (uint8_t)plain_len;
    uint32_t cl = (uint32_t)chain_len;
    uint64_t nc = (uint64_t)num_chains;
    fwrite(&a, 1, 1, f); fwrite(&pl, 1, 1, f);
    fwrite(&cl, 4, 1, f); fwrite(&nc, 8, 1, f);

    char start[MAX_PLAIN+1], end[MAX_PLAIN+1];
    for (int i = 0; i < num_chains; i++) {
        rand_plain(start, plain_len);
        rainbow_gen_chain((hash_alg_t)alg_id, start, plain_len, chain_len, end);
        fwrite(start, plain_len+1, 1, f);
        fwrite(end,   plain_len+1, 1, f);
        if ((i+1) % 1000 == 0)
            ctf_progress(i+1, num_chains, "Generating");
    }
    fclose(f);
    PRINT_SUCCESS("Table written: %s  (%.2fs)", outfile, ctf_time_now()-t0);
    return 0;
}
