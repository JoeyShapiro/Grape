#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <openssl/bn.h>

#define MAX 128
#define PORT 8787
#define SA struct sockaddr
#define NBITS 256

void printBN(char *msg, BIGNUM * a)
{
    /* Use BN_bn2hex(a) for hex string
     * Use BN_bn2dec(a) for decimal string */
    char * number_str = BN_bn2hex(a);
    printf("%s %s\n", msg, number_str);
    OPENSSL_free(number_str);
}

void gen_keys() {
    BN_CTX *ctx = BN_CTX_new();

    BIGNUM *p = BN_new();
    BIGNUM *q = BN_new();
    BIGNUM *n = BN_new();
    BIGNUM *phi = BN_new();
    BIGNUM *e = BN_new();
    BIGNUM *d = BN_new();

    // init a, b, n
    BN_generate_prime_ex(p, NBITS, 1, NULL, NULL, NULL);
    BN_generate_prime_ex(q, NBITS, 1, NULL, NULL, NULL);
    BN_rand(n, NBITS, 0, 0);

    // n = p * q
    BN_mul(n, p, q, ctx);

    // phi = (p-1) * (q-1)
    BIGNUM *p1 = BN_new();
    BIGNUM *q1 = BN_new();
    BN_sub(p1,p, BN_value_one());
    BN_sub(q1,q, BN_value_one());
    BN_mul(phi, p1, q1, ctx);

    // e = 65537 TODO check if valid
    BN_dec2bn(&e, "65537");

    // d * e % phi = 1 (d = e^-1 % phi)?
    BN_mod_inverse(d, e, phi, ctx);

    // public key
    printBN("e =", e);
    printBN("n =", n);
    // private key
    printBN("e =", e);
    printBN("d =", d);
}

void encrypt_data(char res[], char input[]) {

}

void decrypt_data(char res[], char input[]) {

}

void func(int sockfd) {
    char buff[MAX];
    int n;
    char enc[MAX];

    while (1) {
        bzero(buff, sizeof(buff));
        printf("Enter the string: ");
        n = 0;

        // get user input
        while ((buff[n++] = getchar()) != '\n') // goes until \n
            ;

        // encrypt
        encrypt_data(enc, buff);

        // send to server
        write(sockfd, buff, sizeof(buff));
        bzero(buff, sizeof(buff));
        read(sockfd, buff, sizeof(buff));
        printf("From Server: %s", buff);
        if ((strncmp(buff, "bye", 4)) == 0) {
            printf("Client Exit...\n");
            break;
        }
    }
}

int main() {
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;

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
    
    gen_keys();
    func(sockfd);
    close(sockfd);
}