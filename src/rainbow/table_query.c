/*
 * table_query.c — Query a rainbow table for a given hash
 * POSIX C99 | Public Domain (Unlicense)
 *
 * Usage: table_query -t <table.rt> -H <hex_hash>
 */
#define _POSIX_C_SOURCE 200809L
#include <getopt.h>
#include "../../src/core/base.h"
#include "../../src/core/utils.h"
#include "hash_rainbow.c"

int main(int argc, char **argv) {
    const char *table_path = NULL, *hex_hash = NULL;
    int opt;
    while ((opt = getopt(argc, argv, "t:H:v")) != -1) {
        switch (opt) {
            case 't': table_path = optarg; break;
            case 'H': hex_hash   = optarg; break;
            case 'v': ctf_verbosity = 2; break;
        }
    }
    if (!table_path || !hex_hash) {
        fprintf(stderr, "Usage: table_query -t table.rt -H <hexhash> [-v]\n");
        return 1;
    }

    unsigned char target[HASH_MAX_LEN];
    size_t hash_len = 0;
    if (ctf_hex2bin(hex_hash, target, HASH_MAX_LEN, &hash_len) != CTF_OK) {
        PRINT_FAIL("Invalid hex hash");
        return 1;
    }

    FILE *f = fopen(table_path, "rb");
    if (!f) { PRINT_FAIL("Cannot open table: %s", table_path); return 1; }

    /* Read header */
    char magic[6]; fread(magic, 6, 1, f);
    uint8_t alg_id, plain_len_u; uint32_t chain_len; uint64_t num_chains;
    fread(&alg_id, 1, 1, f); fread(&plain_len_u, 1, 1, f);
    fread(&chain_len, 4, 1, f); fread(&num_chains, 8, 1, f);

    int plain_len = (int)plain_len_u;
    PRINT_INFO("Table: alg=%d  plain_len=%d  chain_len=%u  chains=%llu",
               alg_id, plain_len, chain_len, (unsigned long long)num_chains);

    chain_entry_t *table = malloc(num_chains * sizeof(chain_entry_t));
    if (!table) { PRINT_FAIL("OOM"); fclose(f); return 1; }

    for (uint64_t i = 0; i < num_chains; i++) {
        fread(table[i].start, plain_len+1, 1, f);
        fread(table[i].end,   plain_len+1, 1, f);
    }
    fclose(f);

    char result[MAX_PLAIN+1] = {0};
    double t0 = ctf_time_now();
    int found = rainbow_crack((hash_alg_t)alg_id, target, hash_len,
                               table, (size_t)num_chains, plain_len,
                               (int)chain_len, result);
    free(table);

    if (found) {
        PRINT_SUCCESS("Cracked: %s  (%.3fs)", result, ctf_time_now()-t0);
        return 0;
    }
    PRINT_FAIL("Not found in table (%.3fs)", ctf_time_now()-t0);
    return 1;
}
