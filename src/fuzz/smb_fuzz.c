/*
 * smb_fuzz.c — SMB/CIFS basic frame fuzzer
 * POSIX C99 | Public Domain (Unlicense)
 * WARNING: Only use against systems you own or have permission to test.
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

static unsigned char SMB_NEGOTIATE[] = {
    0x00, 0x00, 0x00, 0x54,
    0xff, 0x53, 0x4d, 0x42, 0x72,
    0x00, 0x00, 0x00, 0x00, 0x18,
    0x01, 0x28,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x31, 0x00,
    0x02, 'S','M','B',' ','2','.','0','0','2','\0',
    0x02, 'S','M','B',' ','2','.','?','?','?','\0',
    0x02, 'N','T',' ','L','M',' ','0','.','1','2','\0',
    0x02, 'L','A','N','M','A','N','1','.','0','\0',
};

static int tcp_connect_smb(const char *host, int port) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    struct hostent *he = gethostbyname(host);
    if (!he) return -1;
    memcpy(&addr.sin_addr, he->h_addr_list[0], he->h_length);
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;
    struct timeval tv; tv.tv_sec = 2; tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { close(fd); return -1; }
    return fd;
}

int main(int argc, char **argv) {
    const char *host = "127.0.0.1";
    int port = 445, count = 30;
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

    PRINT_INFO("SMB Fuzzer: %s:%d  count=%d", host, port, count);
    fprintf(stderr, COL_YELLOW "[!] Only test systems you have explicit permission to fuzz!" COL_RESET "\n");

    int ok = 0, fail = 0;
    for (int i = 0; i < count; i++) {
        fuzz_frame_t *frame = fuzz_random(SMB_NEGOTIATE, sizeof(SMB_NEGOTIATE));
        int fd = tcp_connect_smb(host, port);
        if (fd < 0) { fail++; fuzz_frame_free(frame); continue; }
        send(fd, frame->data, frame->len, 0);
        char resp[256] = {0};
        int n = recv(fd, resp, sizeof(resp)-1, 0);
        close(fd);
        if (n > 0) ok++; else fail++;
        LOG_VERB("[%3d] %s -> %d bytes", i, frame->desc, n);
        fuzz_frame_free(frame);
    }
    PRINT_INFO("Done: %d got response, %d failures", ok, fail);
    return 0;
}
