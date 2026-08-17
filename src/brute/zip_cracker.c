/*
 * zip_cracker.c — ZIP password cracker using libzip
 * POSIX C99 | Public Domain (Unlicense)
 *
 * Standalone: zip_cracker -z <archive.zip> -w <wordlist>
 * Shared:     int zip_try_password(const char*, const char*)
 */
#define _POSIX_C_SOURCE 200809L
#include <zip.h>
#include <getopt.h>
#include "../../src/core/base.h"
#include "../../src/core/utils.h"

int zip_try_password(const char *zip_path, const char *password) {
    int err = 0;
    zip_t *za = zip_open(zip_path, ZIP_RDONLY, &err);
    if (!za) return 0;
    zip_set_default_password(za, password);
    zip_int64_t num = zip_get_num_entries(za, 0);
    int success = 0;
    for (zip_int64_t i = 0; i < num && !success; i++) {
        zip_file_t *zf = zip_fopen_index(za, i, 0);
        if (!zf) continue;
        char buf[512];
        zip_int64_t rd;
        int read_ok = 1;
        while ((rd = zip_fread(zf, buf, sizeof(buf))) > 0);
        if (rd < 0) read_ok = 0;
        if (zip_fclose(zf) != 0) read_ok = 0;
        if (read_ok) success = 1;
    }
    zip_close(za);
    return success;
}

#ifndef CTF_MULTI_BRUTE
int main(int argc, char **argv) {
    const char *zip_path = NULL, *wordlist = NULL;
    int opt;
    while ((opt = getopt(argc, argv, "z:w:v")) != -1) {
        switch (opt) {
            case 'z': zip_path = optarg; break;
            case 'w': wordlist = optarg; break;
            case 'v': ctf_verbosity = 2; break;
            default:
                fprintf(stderr, "Usage: %s -z archive.zip -w wordlist [-v]\n", argv[0]);
                return 1;
        }
    }
    if (!zip_path || !wordlist) {
        fprintf(stderr, "zip_cracker: -z and -w required\n");
        return 1;
    }
    if (!ctf_file_exists(zip_path)) { PRINT_FAIL("ZIP not found: %s", zip_path); return 1; }

    PRINT_INFO("Cracking: %s", zip_path);
    FILE *f = fopen(wordlist, "r");
    if (!f) { PRINT_FAIL("Cannot open wordlist"); return 1; }

    char line[CTF_MAX_PASS];
    long tried = 0;
    double t0 = ctf_time_now();
    while (fgets(line, sizeof(line), f)) {
        char *p = ctf_strtrim(line);
        if (!*p) continue;
        tried++;
        LOG_VERB("Trying: %s", p);
        if (zip_try_password(zip_path, p)) {
            PRINT_SUCCESS("ZIP password: %s  (tried %ld in %.2fs)", p, tried, ctf_time_now()-t0);
            fclose(f);
            return 0;
        }
    }
    fclose(f);
    PRINT_FAIL("Not found. Tried %ld passwords in %.2fs", tried, ctf_time_now()-t0);
    return 1;
}
#endif /* CTF_MULTI_BRUTE */
