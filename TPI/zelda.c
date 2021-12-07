#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <openssl/bn.h>
#include <arpa/inet.h>
// for sockerts
#include <unistd.h>
// for hash
#include <openssl/sha.h>

#define MAX 1024
#define PORT 8787
#define SA struct sockaddr
#define NBITS 256
#define BLOCK 8

int main(int argc, char *argv[]) {
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;

    if (argc<2 || argc>4) {
        printf("ERROR: wrong amount of args\nEXAMPLE: ./zelda id msg\n");
        exit(0);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket create failed...\n");
        exit(0);
    } else
        printf("Socket successfully created...\n");
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);

    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    } else
        printf("conneted to the server...\n");

    char dummy[MAX];
    bzero(dummy, MAX);
    char *spf = argv[1];
    int len = strlen(argv[2]);
    char *msg = argv[2];

    unsigned char hash[256];
    BIGNUM *h = BN_new();
    char *sig = malloc(122);
    bzero(sig, 122);
    BIGNUM *sign = BN_new();

    BN_CTX *ctx = BN_CTX_new();
    BIGNUM *n = BN_new();
    BIGNUM *d = BN_new();

    BN_hex2bn(&n, "DCBFFE3E51F62E09CE7032E2677A78946A849DC4CDDE3A4D0CB81629242FB1A5");
    BN_hex2bn(&d, "74D806F9F3A62BAE331FFE3F0A68AFE35B3D2E4794148AACBC26AA381CD7D30D");
    
    printf("creating packet with message \"%s\" by user %s\n", msg, spf);

    SHA256((unsigned char*) msg, strlen(msg), hash);
    BN_bin2bn(hash, 256, h);
    BN_mod_exp(sign, h, d, n, ctx);
    sig = BN_bn2hex(sign);
    printf("got hash \"%s\"\n", sig);

    sprintf(dummy, "system send %s %d %s %s", spf, len, msg, sig);
    printf("dummy: %s\n", dummy);

    write(sockfd, dummy, sizeof(dummy));

    // ending
    close(sockfd);
    
    return 0;
}