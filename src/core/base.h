/*
 * base.h — CTF Arsenal core definitions
 * POSIX C99 | Public Domain (Unlicense)
 */
#ifndef CTF_BASE_H
#define CTF_BASE_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* ── Version ───────────────────────────────────────────────────────────── */
#define CTF_VERSION_MAJOR 1
#define CTF_VERSION_MINOR 0
#define CTF_VERSION_PATCH 0
#define CTF_VERSION_STR   "1.0.0"

/* ── Boolean ────────────────────────────────────────────────────────────── */
#ifndef __cplusplus
# include <stdbool.h>
#endif

/* ── Error Codes ────────────────────────────────────────────────────────── */
typedef enum {
    CTF_OK            = 0,
    CTF_ERR_GENERIC   = -1,
    CTF_ERR_NOMEM     = -2,
    CTF_ERR_IO        = -3,
    CTF_ERR_NOTFOUND  = -4,
    CTF_ERR_BADARG    = -5,
    CTF_ERR_TIMEOUT   = -6,
    CTF_ERR_CRACKED   = 1,
    CTF_ERR_NOCRACK   = 2,
} ctf_result_t;

/* ── Logging ────────────────────────────────────────────────────────────── */
extern int ctf_verbosity;

#define CTF_LOG(level, fmt, ...)  \
    do { if (ctf_verbosity >= (level)) fprintf(stderr, "[CTF] " fmt "\n", ##__VA_ARGS__); } while (0)

#define LOG_INFO(fmt, ...)   CTF_LOG(1, "INFO  " fmt, ##__VA_ARGS__)
#define LOG_VERB(fmt, ...)   CTF_LOG(2, "VERB  " fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)   CTF_LOG(0, "WARN  " fmt, ##__VA_ARGS__)
#define LOG_ERR(fmt, ...)    CTF_LOG(0, "ERR   " fmt, ##__VA_ARGS__)

/* ── Utility macros ─────────────────────────────────────────────────────── */
#define ARRAY_SIZE(a)  (sizeof(a) / sizeof((a)[0]))
#define MIN(a,b)       ((a) < (b) ? (a) : (b))
#define MAX(a,b)       ((a) > (b) ? (a) : (b))
#define CLAMP(v,lo,hi) (MIN(MAX((v),(lo)),(hi)))
#define CTF_STRNLEN(s, max) strnlen((s), (max))

/* ── Colour output ──────────────────────────────────────────────────────── */
#define COL_RESET  "\033[0m"
#define COL_RED    "\033[31m"
#define COL_GREEN  "\033[32m"
#define COL_YELLOW "\033[33m"
#define COL_CYAN   "\033[36m"
#define COL_BOLD   "\033[1m"

#define PRINT_SUCCESS(fmt, ...) printf(COL_GREEN COL_BOLD "[+] " fmt COL_RESET "\n", ##__VA_ARGS__)
#define PRINT_FAIL(fmt, ...)    printf(COL_RED    "[!] " fmt COL_RESET "\n", ##__VA_ARGS__)
#define PRINT_INFO(fmt, ...)    printf(COL_CYAN   "[-] " fmt COL_RESET "\n", ##__VA_ARGS__)

/* ── Common limits ──────────────────────────────────────────────────────── */
#define CTF_MAX_PATH    4096
#define CTF_MAX_PASS    512
#define CTF_MAX_LINE    8192

#endif /* CTF_BASE_H */
