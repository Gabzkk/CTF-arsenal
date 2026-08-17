/*
 * http_fuzz.c — HTTP/1.1 fuzzer
 * POSIX C99 | Public Domain (Unlicense)
 *
 * Usage: http_fuzz -H <host> -p <port> -P <path> [-n <count>]
 */
#define _POSIX_C_SOURCE 200809L
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include "../../src/core/base.h"
#include "../../src/core/utils.h"
#include "proto_fuzz.c"

#define HTTP_TIMEOUT_MS 2000

static int tcp_connect(const char *host, int port) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    struct hostent *he = gethostbyname(host);
    if (!he) return -1;
    memcpy(&addr.sin_addr, he->h_addr_list[0], he->h_length);
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;
    struct timeval tv;
    tv.tv_sec  = HTTP_TIMEOUT_MS / 1000;
    tv.tv_usec = (HTTP_TIMEOUT_MS % 1000) * 1000;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(fd); return -1;
    }
    return fd;
}

static const char *SEED_PATHS[] = { "/", "/admin", "/api", "/login", "/../etc/passwd" };
static const char *SEED_METHODS[] = { "GET", "POST", "PUT", "DELETE", "OPTIONS",
                                       "TRACE", "HEAD", "CONNECT" };
static const char *SEED_HDRS[] = {
    "X-Forwarded-For: 127.0.0.1\r\n",
    "Content-Length: 99999999\r\n",
    "Transfer-Encoding: chunked\r\nTransfer-Encoding: identity\r\n",
    "Connection: keep-alive\r\n"
};

int main(int argc, char **argv) {
    const char *host = "127.0.0.1", *path = "/";
    int port = 80, count = 100;
    int opt;
    srand((unsigned)time(NULL));

    while ((opt = getopt(argc, argv, "H:p:P:n:v")) != -1) {
        switch(opt) {
            case 'H': host  = optarg; break;
            case 'p': port  = atoi(optarg); break;
            case 'P': path  = optarg; break;
            case 'n': count = atoi(optarg); break;
            case 'v': ctf_verbosity = 2; break;
            default: break;
        }
    }

    PRINT_INFO("HTTP Fuzzer: %s:%d%s  count=%d", host, port, path, count);

    int timeouts = 0, ok = 0;
    for (int i = 0; i < count; i++) {
        char req[4096];
        const char *method = SEED_METHODS[rand() % ARRAY_SIZE(SEED_METHODS)];
        const char *fpath  = (rand() % 3 == 0)
            ? SEED_PATHS[rand() % ARRAY_SIZE(SEED_PATHS)] : path;
        const char *hdr    = SEED_HDRS[rand() % ARRAY_SIZE(SEED_HDRS)];
        int rlen = snprintf(req, sizeof(req),
            "%s %s HTTP/1.1\r\nHost: %s\r\n%sConnection: close\r\n\r\n",
            method, fpath, host, hdr);

        fuzz_frame_t *frame = fuzz_random((unsigned char*)req, rlen);
        int fd = tcp_connect(host, port);
        if (fd < 0) { timeouts++; fuzz_frame_free(frame); continue; }

        send(fd, frame->data, frame->len, 0);
        char resp[512] = {0};
        int n = recv(fd, resp, sizeof(resp)-1, 0);
        close(fd);

        LOG_VERB("[%4d] %s -> %d bytes", i, frame->desc, n);
        if (n > 0) ok++; else timeouts++;
        fuzz_frame_free(frame);
        ctf_progress(i+1, count, "HTTP-fuzz");
    }
    fprintf(stderr, "\n");
    PRINT_INFO("Done: %d ok, %d timeouts", ok, timeouts);
    return 0;
}
