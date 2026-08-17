/*
 * pdf_cracker.c — PDF password cracker via poppler-utils (pdfinfo)
 * POSIX C99 | Public Domain (Unlicense)
 */
#define _POSIX_C_SOURCE 200809L
#include <getopt.h>
#include "../../src/core/base.h"
#include "../../src/core/utils.h"

int pdf_try_password(const char *pdf_path, const char *password) {
    char cmd[CTF_MAX_PATH + CTF_MAX_PASS + 64];
    snprintf(cmd, sizeof(cmd),
        "pdfinfo -upw '%s' '%s' > /dev/null 2>&1", password, pdf_path);
    return system(cmd) == 0;
}

#ifndef CTF_MULTI_BRUTE
int main(int argc, char **argv) {
    const char *pdf_path = NULL, *wordlist = NULL;
    int opt;
    while ((opt = getopt(argc, argv, "p:w:v")) != -1) {
        switch (opt) {
            case 'p': pdf_path = optarg; break;
            case 'w': wordlist = optarg; break;
            case 'v': ctf_verbosity = 2; break;
            default:
                fprintf(stderr, "Usage: %s -p archive.pdf -w wordlist [-v]\n", argv[0]);
                return 1;
        }
    }
    if (!pdf_path || !wordlist) { fprintf(stderr, "pdf_cracker: -p and -w required\n"); return 1; }

    FILE *f = fopen(wordlist, "r");
    if (!f) { PRINT_FAIL("Cannot open wordlist"); return 1; }
    char line[CTF_MAX_PASS];
    long tried = 0;
    double t0 = ctf_time_now();
    while (fgets(line, sizeof(line), f)) {
        char *p = ctf_strtrim(line);
        if (!*p) continue;
        tried++;
        if (pdf_try_password(pdf_path, p)) {
            PRINT_SUCCESS("PDF password: %s  (tried %ld, %.2fs)", p, tried, ctf_time_now()-t0);
            fclose(f);
            return 0;
        }
    }
    fclose(f);
    PRINT_FAIL("Not found. Tried %ld in %.2fs", tried, ctf_time_now()-t0);
    return 1;
}
#endif /* CTF_MULTI_BRUTE */
