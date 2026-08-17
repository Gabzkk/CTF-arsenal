/*
 * multi_brute.c — Multithreaded wordlist + mutation brute-force engine
 * POSIX C99 + pthreads | Public Domain (Unlicense)
 */
#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <unistd.h>
#include <getopt.h>
#include "../../src/core/base.h"
#include "../../src/core/utils.h"

typedef struct {
    int             found;
    char            result[CTF_MAX_PASS];
    pthread_mutex_t lock;
} shared_state_t;

typedef struct {
    const char     *target;
    char          **passwords;
    size_t          start;
    size_t          end;
    shared_state_t *shared;
} worker_args_t;

int zip_try_password(const char *zip_path, const char *password);
int rar_try_password(const char *rar_path, const char *password);
int pdf_try_password(const char *pdf_path, const char *password);

static int (*picker(const char *path))(const char*, const char*) {
    if (ctf_ends_with(path, ".zip")) return zip_try_password;
    if (ctf_ends_with(path, ".rar")) return rar_try_password;
    if (ctf_ends_with(path, ".pdf")) return pdf_try_password;
    return NULL;
}

static void *worker(void *arg) {
    worker_args_t *wa = (worker_args_t *)arg;
    int (*try_fn)(const char*, const char*) = picker(wa->target);
    if (!try_fn) return NULL;

    for (size_t i = wa->start; i < wa->end; i++) {
        pthread_mutex_lock(&wa->shared->lock);
        int done = wa->shared->found;
        pthread_mutex_unlock(&wa->shared->lock);
        if (done) break;

        if (try_fn(wa->target, wa->passwords[i])) {
            pthread_mutex_lock(&wa->shared->lock);
            if (!wa->shared->found) {
                wa->shared->found = 1;
                snprintf(wa->shared->result, CTF_MAX_PASS, "%s", wa->passwords[i]);
            }
            pthread_mutex_unlock(&wa->shared->lock);
            break;
        }
    }
    return NULL;
}

static char **load_passwords(const char *wl_path, const char *mut_path, size_t *out_count) {
    size_t cap = 65536, count = 0;
    char **list = malloc(cap * sizeof(char*));
    if (!list) return NULL;

    FILE *f = fopen(wl_path, "r");
    if (!f) { free(list); return NULL; }
    char line[CTF_MAX_PASS];
    while (fgets(line, sizeof(line), f)) {
        char *p = ctf_strtrim(line);
        if (!*p) continue;
        if (count >= cap) {
            cap *= 2;
            list = realloc(list, cap * sizeof(char*));
        }
        list[count++] = ctf_strdup(p);
    }
    fclose(f);

    if (mut_path && ctf_file_exists(mut_path)) {
        size_t base_count = count;
        FILE *mf = fopen(mut_path, "r");
        while (fgets(line, sizeof(line), mf)) {
            char *rule = ctf_strtrim(line);
            if (!*rule || *rule == '#') continue;
            int is_suffix = ctf_starts_with(rule, "suffix:");
            int is_prefix = ctf_starts_with(rule, "prefix:");
            const char *val = is_suffix ? rule+7 : (is_prefix ? rule+7 : rule);
            for (size_t b = 0; b < base_count; b++) {
                char mutated[CTF_MAX_PASS];
                if (is_suffix)      snprintf(mutated, CTF_MAX_PASS, "%s%s", list[b], val);
                else if (is_prefix) snprintf(mutated, CTF_MAX_PASS, "%s%s", val, list[b]);
                else                snprintf(mutated, CTF_MAX_PASS, "%s%s", list[b], val);
                if (count >= cap) { cap *= 2; list = realloc(list, cap * sizeof(char*)); }
                list[count++] = ctf_strdup(mutated);
            }
        }
        fclose(mf);
    }

    *out_count = count;
    return list;
}

int main(int argc, char **argv) {
    const char *wordlist = NULL, *target = NULL, *mutations = NULL;
    int threads = 4;
    int opt;

    while ((opt = getopt(argc, argv, "w:t:m:j:v")) != -1) {
        switch (opt) {
            case 'w': wordlist  = optarg; break;
            case 't': target    = optarg; break;
            case 'm': mutations = optarg; break;
            case 'j': threads   = atoi(optarg); break;
            case 'v': ctf_verbosity = 2; break;
            default:
                fprintf(stderr, "Usage: %s -w wordlist -t target [-m mutations] [-j threads] [-v]\n", argv[0]);
                return 1;
        }
    }

    if (!wordlist || !target) {
        fprintf(stderr, "multi_brute: -w and -t are required\n");
        return 1;
    }
    if (!picker(target)) {
        fprintf(stderr, "multi_brute: unsupported target type (need .zip/.rar/.pdf)\n");
        return 1;
    }

    PRINT_INFO("Loading wordlist: %s", wordlist);
    size_t total = 0;
    char **passwords = load_passwords(wordlist, mutations, &total);
    if (!passwords || total == 0) {
        PRINT_FAIL("Failed to load wordlist");
        return 1;
    }
    PRINT_INFO("Loaded %zu candidate passwords (mutations included)", total);
    PRINT_INFO("Target: %s  Threads: %d", target, threads);

    shared_state_t shared;
    shared.found = 0;
    memset(shared.result, 0, sizeof(shared.result));
    pthread_mutex_init(&shared.lock, NULL);

    worker_args_t *args = calloc(threads, sizeof(worker_args_t));
    pthread_t *tids = malloc(threads * sizeof(pthread_t));
    size_t chunk = (total + threads - 1) / threads;

    for (int i = 0; i < threads; i++) {
        args[i].target    = target;
        args[i].passwords = passwords;
        args[i].start     = i * chunk;
        args[i].end       = ((i + 1) * chunk > total) ? total : (i + 1) * chunk;
        args[i].shared    = &shared;
        pthread_create(&tids[i], NULL, worker, &args[i]);
    }

    for (int i = 0; i < threads; i++) pthread_join(tids[i], NULL);

    int cracked = shared.found;
    if (cracked) {
        PRINT_SUCCESS("Password found: %s", shared.result);
    } else {
        PRINT_FAIL("Password not found in wordlist");
    }

    pthread_mutex_destroy(&shared.lock);
    for (size_t i = 0; i < total; i++) free(passwords[i]);
    free(passwords); free(args); free(tids);
    return cracked ? 0 : 1;
}
