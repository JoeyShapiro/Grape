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

#define MAX 1024
#define PORT 8787
#define SA struct sockaddr
#define NBITS 256
#define BLOCK 8
char msgs[] = "";
int col, row;

struct Keys {
    BIGNUM *n, *e, *d;
    BN_CTX *ctx;
};

typedef struct Keys keys;
keys k;

void printBN(char *msg, BIGNUM * a)
{
    /* Use BN_bn2hex(a) for hex string
     * Use BN_bn2dec(a) for decimal string */
    char * number_str = BN_bn2hex(a);
    printf("%s %s\n", msg, number_str);
    OPENSSL_free(number_str);
}

keys gen_keys() {
    keys k;
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
    BN_rand(n, NBITS, 0, 0); // redundant?

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
    k.ctx = ctx;
    printBN("e =", e);
    printBN("n =", n);
    k.e = e;
    k.n = n;
    // private key
    printBN("d =", d);
    k.d = d;
    printBN("n =", n);

    return k;
}

BIGNUM* gen_sec() {
    BIGNUM *s = BN_new();
    BN_rand(s, BLOCK, 0, 0);
    return s;
}

BIGNUM* encrypt_sec(BIGNUM *s) {
    // convert string to binary
    // do math for enc
    printf("good2");
    printf("good3");
    printf("good4");
    BIGNUM *c = BN_new();
    printf("good5");
    // c = m^e mod n
    BN_mod_exp(c, s, k.e, k.n, k.ctx);
    return c;
}

unsigned char* encrypt_data(unsigned char input[], BIGNUM* s) {
    BN_CTX *ctx = BN_CTX_new();
    BIGNUM *m = BN_new();
    BIGNUM *c = BN_new();
    unsigned char newinput[sizeof(&input) + BLOCK];
    printf("siez %d\n", sizeof(&input) + BLOCK);
    unsigned char secret[BLOCK];
    BN_bn2bin(s, secret);

    for (int b = 0; b < sizeof(&input); b ++) {
        printf(" %02x", input[b]);
    }
    strcat(newinput, input);
    while (sizeof(&newinput) % BLOCK != 0) { // do i even need padding
        strcat(newinput, "0"); //this was "\0"
    }

    for (int b = 0; b < sizeof(&newinput); b ++) {
        printf(" %02x", newinput[b]);
    }

    unsigned char *cipher = malloc(MAX);
    int i;
    int j; // set up here so it runs each one
    int blocks = sizeof(&newinput) / BLOCK;
    printf("%d / %d\n", sizeof(&newinput), BLOCK);
    printf("%s newinput (%x), %s secret (%x), newinput[7] = %d\n", newinput, newinput, secret, secret, newinput[7]);
    for (i=0; i<blocks; i++) {
        int blockStart = blocks*i;
        printf("blockStart = %d\n", blockStart);
        for (j=0; j<BLOCK; j++) {
            int cur = blockStart+j;
            printf("cur is %d on j %d and i %d\n", cur, j, i);
            cipher[cur] = (char)(newinput[cur] ^ secret[cur]);
            printf("%d (%x) = %x ^ %x\n", cipher[cur], cipher[cur], newinput[cur], secret[cur]);
        }
    }
    cipher[i] = '\n';
    BN_bin2bn(cipher, sizeof(&cipher), c);
    printf("the cipher string (in hex) is\n");
    for (int b = 0; b < sizeof(&cipher); b ++) {
        printf(" %02x", cipher[b]);
    }

    return cipher;
}

void decrypt_data(char res[], char input[]) {

}

void *rec(void *vargp) { // TODO can i just pass int, or must it be void*
    int sockfd = *((int *)vargp);
    char buff[MAX];
    int n;

    while(1) {
        bzero(buff, MAX);
        read(sockfd, buff, sizeof(buff));
        strcat(msgs, buff);
        printf("%s\t", buff);
        //mvprintw(0,0,"%s",msgs);
    }

    return NULL;
}

// function for chat between server and client
void *sen(void *vargp) {
    int sockfd = *((int *)vargp);
    char buff[MAX];
    int n;
    unsigned char *cipher;
    BIGNUM *secret = BN_new();
    secret = gen_sec();

    // inf loop for chat
    while (1) {
        bzero(buff, MAX);
        n = 0;

        //printf(">");
        //mvprintw(row/2,(col-strlen(">"))/2,"%s",">");
        //getstr(buff);
        printf("good-2");
        while ((buff[n++] = getchar()) != '\n') // goes until \n
            ;

        printf("good-1");
        cipher = encrypt_data(buff, secret);
        printf("chipher main is\n");
        for (int b = 0; b < sizeof(&cipher); b ++) {
            printf(" %02x", cipher[b]);
        }
        printf("good");
        char number_str[MAX];
        //BN_bn2bin(cipher, number_str);
        printf("good1\n");
        printf("numberstr %x\n", number_str);
        
        write(sockfd, cipher, sizeof(cipher));
        strcat(msgs, cipher);

        if (strncmp("bye", buff, 3) == 0) {
            printf("Server Exit...\n");
            break;
        }
    }

    return NULL;
}

int main() {
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;
    pthread_t tid;

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
    
    k = gen_keys();
    int *arg = malloc(sizeof(*arg));
    *arg = sockfd;

    //initscr();				/* start the curses mode */
    //getmaxyx(stdscr,row,col);		/* get the number of rows and columns */

    pthread_create(&tid, NULL, sen, arg);
    pthread_create(&tid, NULL, rec, arg);
    pthread_exit(NULL);

    close(sockfd);
    
    return 0;
}