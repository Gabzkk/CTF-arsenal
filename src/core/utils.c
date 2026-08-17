/*
 * utils.c — CTF Arsenal utility implementations
 * POSIX C99 | Public Domain (Unlicense)
 */
#define _POSIX_C_SOURCE 200809L
#include "utils.h"
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>
#include <math.h>

int ctf_verbosity = 1;

/* ── File helpers ─────────────────────────────────────────────────────────── */
int ctf_file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0;
}

long ctf_file_size(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) return -1;
    return (long)st.st_size;
}

char *ctf_read_file(const char *path, size_t *out_len) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    rewind(f);
    char *buf = malloc(sz + 1);
    if (!buf) { fclose(f); return NULL; }
    size_t rd = fread(buf, 1, sz, f);
    buf[rd] = '\0';
    fclose(f);
    if (out_len) *out_len = rd;
    return buf;
}

int ctf_write_file(const char *path, const void *data, size_t len) {
    FILE *f = fopen(path, "wb");
    if (!f) return CTF_ERR_IO;
    size_t wr = fwrite(data, 1, len, f);
    fclose(f);
    return (wr == len) ? CTF_OK : CTF_ERR_IO;
}

/* ── String helpers ───────────────────────────────────────────────────────── */
char *ctf_strdup(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char *d = malloc(n);
    if (d) memcpy(d, s, n);
    return d;
}

char *ctf_strtrim(char *s) {
    if (!s) return s;
    while (isspace((unsigned char)*s)) s++;
    char *end = s + strlen(s);
    while (end > s && isspace((unsigned char)*(end-1))) end--;
    *end = '\0';
    return s;
}

int ctf_starts_with(const char *s, const char *prefix) {
    return strncmp(s, prefix, strlen(prefix)) == 0;
}

int ctf_ends_with(const char *s, const char *suffix) {
    size_t sl = strlen(s), pl = strlen(suffix);
    if (pl > sl) return 0;
    return strcmp(s + sl - pl, suffix) == 0;
}

void ctf_str_lower(char *s) {
    for (; *s; s++) *s = (char)tolower((unsigned char)*s);
}

/* ── Hex encoding ─────────────────────────────────────────────────────────── */
static const char HEX[] = "0123456789abcdef";

void ctf_bin2hex(const unsigned char *bin, size_t len, char *out) {
    for (size_t i = 0; i < len; i++) {
        out[2*i]   = HEX[bin[i] >> 4];
        out[2*i+1] = HEX[bin[i] & 0xf];
    }
    out[2*len] = '\0';
}

int ctf_hex2bin(const char *hex, unsigned char *out, size_t max, size_t *out_len) {
    size_t hlen = strlen(hex);
    if (hlen % 2 != 0) return CTF_ERR_BADARG;
    size_t blen = hlen / 2;
    if (blen > max) return CTF_ERR_BADARG;
    for (size_t i = 0; i < blen; i++) {
        char hi = hex[2*i], lo = hex[2*i+1];
        unsigned char v = 0;
        if      (hi >= '0' && hi <= '9') v = (hi - '0') << 4;
        else if (hi >= 'a' && hi <= 'f') v = (hi - 'a' + 10) << 4;
        else if (hi >= 'A' && hi <= 'F') v = (hi - 'A' + 10) << 4;
        else return CTF_ERR_BADARG;
        if      (lo >= '0' && lo <= '9') v |= lo - '0';
        else if (lo >= 'a' && lo <= 'f') v |= lo - 'a' + 10;
        else if (lo >= 'A' && lo <= 'F') v |= lo - 'A' + 10;
        else return CTF_ERR_BADARG;
        out[i] = v;
    }
    if (out_len) *out_len = blen;
    return CTF_OK;
}

/* ── Config ───────────────────────────────────────────────────────────────── */
#define MAX_ENTRIES 256

typedef struct { char key[128]; char val[512]; } kv_t;
struct ctf_config { kv_t entries[MAX_ENTRIES]; int count; };

ctf_config_t *ctf_config_load(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return NULL;
    ctf_config_t *cfg = calloc(1, sizeof(*cfg));
    if (!cfg) { fclose(f); return NULL; }
    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        char *p = ctf_strtrim(line);
        if (!p || *p == '#' || *p == ';' || *p == '\0') continue;
        char *eq = strchr(p, '=');
        if (!eq) continue;
        *eq = '\0';
        char *k = ctf_strtrim(p);
        char *v = ctf_strtrim(eq + 1);
        if (cfg->count >= MAX_ENTRIES) break;
        snprintf(cfg->entries[cfg->count].key, 128, "%s", k);
        snprintf(cfg->entries[cfg->count].val, 512, "%s", v);
        cfg->count++;
    }
    fclose(f);
    return cfg;
}

const char *ctf_config_get(const ctf_config_t *cfg, const char *key, const char *def) {
    if (!cfg) return def;
    for (int i = 0; i < cfg->count; i++)
        if (strcmp(cfg->entries[i].key, key) == 0)
            return cfg->entries[i].val;
    return def;
}

long ctf_config_get_long(const ctf_config_t *cfg, const char *key, long def) {
    const char *v = ctf_config_get(cfg, key, NULL);
    if (!v) return def;
    return strtol(v, NULL, 0);
}

void ctf_config_free(ctf_config_t *cfg) { free(cfg); }

/* ── Timing ───────────────────────────────────────────────────────────────── */
double ctf_time_now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

/* ── Progress ─────────────────────────────────────────────────────────────── */
void ctf_progress(size_t done, size_t total, const char *label) {
    if (ctf_verbosity < 1) return;
    int pct = (total > 0) ? (int)(100.0 * done / total) : 0;
    int filled = pct / 5;
    fprintf(stderr, "\r" COL_CYAN "%-12s" COL_RESET " [", label ? label : "");
    for (int i = 0; i < 20; i++) fprintf(stderr, "%c", i < filled ? '#' : '.');
    fprintf(stderr, "] %3d%% (%zu/%zu)", pct, done, total);
    if (done == total) fprintf(stderr, "\n");
    fflush(stderr);
}
