/*
 * dns_fuzz.c — DNS query/response fuzzer using raw UDP
 * POSIX C99 | Public Domain (Unlicense)
 */
#define _POSIX_C_SOURCE 200809L
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include "../../src/core/base.h"
#include "../../src/core/utils.h"
#include "proto_fuzz.c"

static unsigned char DNS_A_QUERY[] = {
    0xab, 0xcd, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x07,'e','x','a','m','p','l','e',
    0x03,'c','o','m', 0x00,
    0x00, 0x01, 0x00, 0x01
};

int main(int argc, char **argv) {
    const char *host = "127.0.0.1";
    int port = 53, count = 50;
    int opt;
    srand((unsigned)time(NULL));

    while ((opt = getopt(argc, argv, "H:p:n:v")) != -1) {
        switch(opt) {
            case 'H': host  = optarg; break;
            case 'p': port  = atoi(optarg); break;
            case 'n': count = atoi(optarg); break;
            case 'v': ctf_verbosity = 2; break;
            default: break;
        }
    }

    PRINT_INFO("DNS Fuzzer: %s:%d  count=%d", host, port, count);

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) { perror("socket"); return 1; }
    struct timeval tv; tv.tv_sec = 1; tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    struct sockaddr_in srv;
    memset(&srv, 0, sizeof(srv));
    srv.sin_family = AF_INET;
    srv.sin_port   = htons(port);
    inet_pton(AF_INET, host, &srv.sin_addr);

    int ok = 0, noresp = 0;
    for (int i = 0; i < count; i++) {
        fuzz_frame_t *frame = fuzz_random(DNS_A_QUERY, sizeof(DNS_A_QUERY));
        sendto(fd, frame->data, frame->len, 0,
               (struct sockaddr*)&srv, sizeof(srv));
        unsigned char resp[512];
        socklen_t slen = sizeof(srv);
        int n = recvfrom(fd, resp, sizeof(resp), 0, (struct sockaddr*)&srv, &slen);
        if (n > 0) ok++; else noresp++;
        LOG_VERB("[%3d] %s -> %d bytes", i, frame->desc, n);
        fuzz_frame_free(frame);
    }
    close(fd);
    PRINT_INFO("Done: %d responses, %d no-response", ok, noresp);
    return 0;
}
