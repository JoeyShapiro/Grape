#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <openssl/bn.h>
#include <arpa/inet.h>
// for multi-threading
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
// for TUI
#include <ncurses.h>
// for hash
#include <openssl/sha.h>

void hashAndSign(char sig[], char input[]) {
    unsigned char hash[256];
    BIGNUM *sign = BN_new();
    BIGNUM *h = BN_new();

    BIGNUM *d = BN_new();
    BIGNUM *n = BN_new();
    BIGNUM *ctx = BN_CTX_new();

    SHA256(input, strlen(input), hash);
    BN_bin2bn(hash, 256, h);

    BN_mod_exp(sign, h, d, n, ctx);

    sig = BN_bn2hex(sign);
}

bool verify(char sig[], char msg) {
    unsigned char hash[256];
    BIGNUM *sign = BN_new();
    BIGNUM *v = BN_new();
    BIGNUM *h = BN_new();

    BIGNUM *e = BN_new();
    BIGNUM *n = BN_new();
    BIGNUM *ctx = BN_CTX_new();

    SHA256(msg, strlen(msg), hash);
    BN_bin2bn(hash, 256, h);
    BN_hex2bn(sign, sig);

    BN_mod_exp(v, sign, e, n, ctx);

    if (BN_cmp(v, h) == 0 ) return 1;
    return 0;
}

int main() {

    return 0;
}