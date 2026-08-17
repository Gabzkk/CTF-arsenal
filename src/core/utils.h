/*
 * utils.h — CTF Arsenal utility declarations
 * POSIX C99 | Public Domain (Unlicense)
 */
#ifndef CTF_UTILS_H
#define CTF_UTILS_H

#include "base.h"

/* File helpers */
int     ctf_file_exists(const char *path);
long    ctf_file_size(const char *path);
char   *ctf_read_file(const char *path, size_t *out_len);  /* caller frees */
int     ctf_write_file(const char *path, const void *data, size_t len);

/* String helpers */
char   *ctf_strdup(const char *s);
char   *ctf_strtrim(char *s);           /* in-place, returns s */
int     ctf_starts_with(const char *s, const char *prefix);
int     ctf_ends_with(const char *s, const char *suffix);
void    ctf_str_lower(char *s);

/* Hex encoding */
void    ctf_bin2hex(const unsigned char *bin, size_t len, char *hex_out); /* hex_out must be 2*len+1 */
int     ctf_hex2bin(const char *hex, unsigned char *bin_out, size_t max_out, size_t *out_len);

/* Config (INI-style key=value, # comments) */
typedef struct ctf_config ctf_config_t;
ctf_config_t  *ctf_config_load(const char *path);
const char    *ctf_config_get(const ctf_config_t *cfg, const char *key, const char *def);
long           ctf_config_get_long(const ctf_config_t *cfg, const char *key, long def);
void           ctf_config_free(ctf_config_t *cfg);

/* Timing */
double  ctf_time_now(void);   /* seconds since epoch, double precision */

/* Progress bar (stderr) */
void    ctf_progress(size_t done, size_t total, const char *label);

#endif /* CTF_UTILS_H */
