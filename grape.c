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
char msgs[] = "";
int col, row;

struct Keys {
    BIGNUM *n, *e, *d;
};

typedef struct Keys keys;

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
    k.e = e;
    k.n = n;
    // private key
    printBN("d =", d);
    k.d = d;
    printBN("n =", n);

    return k;
}

void encrypt_data(char res[], char input[]) {

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
        mvprintw(0,0,"%s",msgs);
    }

    return NULL;
}

// function for chat between server and client
void *sen(void *vargp) {
    int sockfd = *((int *)vargp);
    char buff[MAX];
    int n;

    // inf loop for chat
    while (1) {
        bzero(buff, MAX);
        n = 0;

        printf(">");
        //mvprintw(row/2,(col-strlen(">"))/2,"%s",">");
        //getstr(buff);
        while ((buff[n++] = getchar()) != '\n') // goes until \n
            ;

        write(sockfd, buff, sizeof(buff));
        strcat(msgs, buff);

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
    keys k;
    pthread_t tid;
    // initscr();				/* start the curses mode */
    // getmaxyx(stdscr,row,col);		/* get the number of rows and columns */

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
    pthread_create(&tid, NULL, sen, arg);
    pthread_create(&tid, NULL, rec, arg);
    pthread_exit(NULL);

    close(sockfd);
    
    return 0;
}