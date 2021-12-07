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

#define MAX 1024
#define PORT 8787
#define SA struct sockaddr
#define NBITS 256
#define BLOCK 8
int col, row;
WINDOW * sender;
WINDOW * receiver;

struct Keys {
    BIGNUM *n, *e, *d, *s;
    BN_CTX *ctx;
    int others_id;
};

struct Queue {
    int capacity;
    int size;
    int front;
    int back;
    unsigned char elements[10][80]; // 10 strings of size 80
};

typedef struct Keys keys;
typedef struct Queue Queue;
keys k;
Queue *msgs;

Queue * createQueue(int s) {
    Queue *Q;
    Q = (Queue *) malloc(sizeof(Queue));
    Q->size = 0;
    Q->capacity = s;
    Q->front = 0;
    Q->back = -1;
    return Q;
}

void Enqueue(Queue *Q, unsigned char *element) {
    // if Queue is full remove front
    if (Q->size == Q->capacity) {
        Q->size--; // decrese size
        Q->front++; // move front up one
        if (Q->front == Q->capacity) {
            Q->front = 0;
        }
    }
    // add to back
    Q->size++; // increase size
    Q->back = Q->back+1; // move back up one
    if (Q->back == Q->capacity) {
        Q->back = 0;
    }
    strcpy(Q->elements[Q->back], element);
}

unsigned char* getQueue(Queue *Q, int i) {
    return Q->elements[i];
}

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
    //printBN("e =", e);
    //printBN("n =", n);
    k.e = e;
    k.n = n;
    // private key
    //printBN("d =", d);
    k.d = d;
    //printBN("n =", n);

    return k;
}

BIGNUM* gen_sec() {
    BIGNUM *s = BN_new();
    BN_rand(s, BLOCK, 0, 0);
    return s;
}

BIGNUM* encrypt_sec(BIGNUM *pub) {
    // convert string to binary
    // do math for enc
    BIGNUM *c = BN_new();
    // c = m^e mod n
    BN_mod_exp(c, k.s, k.e, pub, k.ctx);
    return c;
}

BIGNUM* decrypt_sec(char *secret) {
    // convert string to binary
    // do math for enc
    BIGNUM *m = BN_new();
    BIGNUM *s = BN_new();
    BN_hex2bn(&s, secret);
    // m = c^d mod n
    BN_mod_exp(m, s, k.d, k.n, k.ctx);
    return m;
}

void printHex(unsigned char str[]) {
    printf("the string (in hex) is\n");
    for (int b = 0; b < sizeof(&str); b ++) {
        printf(" %02x", str[b]);
    }
}

bool startsWith(unsigned char *str, unsigned char *exp) {
    if (strncmp(str, exp, strlen(exp)) == 0) return 1;
    return 0;
}

char* subString (const char* input, int offset, int len, char* dest)
{
  int input_len = strlen (input);

  if (offset + len > input_len)
  {
     return NULL;
  }

  strncpy (dest, input + offset, len);
  return dest;
}

unsigned char* encrypt_data(unsigned char input[], BIGNUM* s) {
    BN_CTX *ctx = BN_CTX_new();
    BIGNUM *m = BN_new();
    BIGNUM *c = BN_new();
    unsigned char newinput[strlen(input) + BLOCK];
    unsigned char secret[BLOCK];
    BN_bn2bin(s, secret);

    strcat(newinput, input);
    while (strlen(newinput) % BLOCK != 0) { // do i even need padding
        strcat(newinput, "0"); //this was "\0"
    }

    unsigned char *cipher = malloc(MAX);
    int i;
    int j; // set up here so it runs each one
    for (i=0; i<strlen(input); i+=strlen(secret)) { // dont reinit i or j!!
        for(j=0; j<strlen(secret); j++) {
            cipher[i+j] = input[i+j] ^ secret[j];
        }
    }
    cipher[i] = '\0';

    return cipher;
}

unsigned char* decrypt_data(unsigned char input[], BIGNUM *s) {
    unsigned char secret[BLOCK];
    bzero(secret, BLOCK);
    BN_bn2bin(s, secret);

    unsigned char *decrypted = malloc(MAX);
    bzero(decrypted, sizeof(decrypted)); // ZERO OUT ALL THE THINGS
    int i = 0;
    int j = 0; // set up here so it runs each one
    for (i=0; i<strlen(input); i+=strlen(secret)) {
        for(j=0; j<strlen(secret); j++) {
            decrypted[i+j] = input[i+j] ^ secret[j];
        }
    } 
    decrypted[i] = '\0';

    return decrypted;
}

void hashAndSign(char sig[], char input[]) {
    unsigned char hash[256];
    BIGNUM *sign = BN_new();
    BIGNUM *h = BN_new();

    SHA256(input, strlen(input), hash);
    BN_bin2bn(hash, 256, h);

    BN_mod_exp(sign, h, k.d, k.n, k.ctx);

    sig = BN_bn2hex(sign);
}

bool verify(char sig[], char msg[], BIGNUM *pub) {
    unsigned char hash[256];
    BIGNUM *sign = BN_new();
    BIGNUM *v = BN_new();
    BIGNUM *h = BN_new();

    SHA256(msg, strlen(msg), hash);
    BN_bin2bn(hash, 256, h);
    BN_hex2bn(&sign, sig);

    BN_mod_exp(v, sign, k.e, pub, k.ctx);

    if (BN_cmp(v, h) == 0 ) return 1;
    return 0;
}

void *rec(void *vargp) { // TODO can i just pass int, or must it be void*
    int sockfd = *((int *)vargp);
    unsigned char buff[MAX];
    unsigned char tmp[MAX];
    BIGNUM *chatter = BN_new();
    unsigned char decrypted[MAX];
    Enqueue(msgs, "https://github.com/JoeyShapiro/grape");

    while(1) {
        int i;
        bzero(buff, MAX);
        bzero(tmp, MAX);
        bzero(decrypted, strlen(decrypted));
        read(sockfd, buff, sizeof(buff));

        if (startsWith(buff, "system")) {
            memcpy(tmp, buff, sizeof(buff));
            char *arg = strtok(buff, " "); // remove system
            arg = strtok(NULL, " "); // get second arg
            int id;
            if (startsWith(arg, "pubid")) { // getting pubid of another
                k.s = gen_sec();
                id = atoi(strtok(NULL, " "));
                k.others_id = id;
                arg = strtok(NULL, " ");
                BN_hex2bn(&chatter, arg);
                BIGNUM *enc_key = encrypt_sec(chatter);
                char secret[400]; // this cant be *
                char *str_key = BN_bn2hex(enc_key);
                sprintf(secret, "system secret %d %s ", id, str_key);
                write(sockfd, secret, strlen(secret));
                sprintf(decrypted, "server: now chatting with user %d %s", k.others_id, BN_bn2hex(k.s));
            } else if (startsWith(arg, "secret")) { // getting secret of another
                int oid = atoi(strtok(NULL, " "));
                k.others_id = oid;
                char *got_key = strtok(NULL, " "); // but this has to be * (same data)?
                k.s = decrypt_sec(got_key); // i never decrypted
                sprintf(decrypted, "server: now chatting with user %d %s", k.others_id, BN_bn2hex(k.s));
            } else if (startsWith(arg, "send")) {
                char *oid = strtok(NULL, " ");
                char *len = strtok(NULL, " ");
                char enc_msg[300];
                char sig[256];
                bzero(enc_msg, 300);
                bzero(sig, 256);
                memcpy(enc_msg, &tmp[16], atoi(len));
                memcpy(sig, &tmp[17+atoi(len)], 122);
                unsigned char *dec_msg = decrypt_data(enc_msg, k.s);
                if(!verify(sig, dec_msg, chatter)) {
                    sprintf(decrypted, "%d: %s is not verified", atoi(oid), dec_msg);
                } else {
                    sprintf(decrypted, "%d: %s", atoi(oid), dec_msg);
                }
            }
        }
        // if (startsWith(buff, "server: ")) {
        //     decrypted = buff;
        // }
        // else {
        //     decrypted = decrypt_data(buff, k.s);
        // }
        Enqueue(msgs, decrypted);

        for (i=0; i<10; i++) {
            mvwprintw(receiver, i+1, 1, getQueue(msgs, i));
        }
        wrefresh(receiver);
    }

    return NULL;
}

// function for chat between server and client
void *sen(void *vargp) {
    int sockfd = *((int *)vargp);
    char buff[MAX];
    int n;
    unsigned char *cipher;
    char str[80];
    echo();

    // inf loop for chat
    while (1) {
        bzero(buff, MAX);
        bzero(str, 80);
        n = 0;

        box(sender, 105, 105);

        // allow user input
        mvwprintw(sender, 1, 1, "                                                  "); // TODO find better way
        mvwprintw(sender, 1, 1, "P:");
        wrefresh(sender);
        wgetstr(sender, str);

        // check not to long
        if (strlen(str) > 80) {
            continue;
        }
        strcat(buff, str);
        if (startsWith(buff, "chat ")) {
            write(sockfd, buff, sizeof(buff));
            continue;
        }
        if (startsWith(buff, "list")) {
            //write(sockfd, buff, sizeof(buff));
            continue;
        }
        if (startsWith(buff, "send")) {
            char message[300];
            subString(buff, 5, strlen(buff)-5, message);
            cipher = encrypt_data(message, k.s);
            int len = strlen(message);
            char toSend[400];
            char sig[256];
            hashAndSign(sig, message);
            sprintf(toSend, "system send %d %d %s %s", k.others_id, len, cipher, sig);
            write(sockfd, toSend, strlen(toSend));
            continue;
        }

        cipher = encrypt_data(buff, k.s);
        char number_str[MAX];
        
        write(sockfd, cipher, sizeof(cipher));

        if (startsWith(buff, "bye")) {
            printf("Server Exit...\n");
            break;
        }
    }

    return NULL;
}

// void *reDraw(void *vargp) {


//     return NULL;
// }

int main() {
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;
    pthread_t tid;
    msgs = createQueue(10);

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
    
    // CONNECTION ALL GOOD
    // start ncurses mode
    initscr();
    cbreak();
    noecho(); // disable input (password mode)

    // check and add color
    if(has_colors() == FALSE)
	{	endwin();
		printf("Your terminal does not support color\n");
		exit(1);
	}
	start_color();
    assume_default_colors(COLOR_GREEN,COLOR_MAGENTA);

    // init ncurses variables
    int row, col, y, x;
    getmaxyx(stdscr,row,col);

    // more init
    y = x = 0;
    receiver = newwin(row-3, col, y, x);
    y = row-3;
    sender = newwin(row/8, col, y, x);

    // create the windows
    refresh();
    box(receiver, 104, 104);
    box(sender, 105, 105);
    wrefresh(receiver);
    wrefresh(sender);

    k = gen_keys();
    int *arg = malloc(sizeof(*arg));
    *arg = sockfd;
    char init[400];
    strcat(init, "user ");
    strcat(init, BN_bn2hex(k.n)); // send public key of this user (I PUT K.N!!)
    write(sockfd, init, sizeof(init));

    pthread_create(&tid, NULL, rec, arg);
    pthread_create(&tid, NULL, sen, arg); // must be second
    //pthread_create(&tid, NULL, reDraw, arg); // must be third
    pthread_exit(NULL);

    // ending
    getch();
    close(sockfd);
    endwin();
    
    return 0;
}