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

unsigned char* encrypt_data(unsigned char input[], BIGNUM* s) {
    BN_CTX *ctx = BN_CTX_new();
    BIGNUM *m = BN_new();
    BIGNUM *c = BN_new();
    unsigned char newinput[strlen(input) + 1];
    unsigned char secret[BLOCK];
    BN_bn2bin(s, secret);
    printf("%d", strlen(input));
    newinput[strlen(input)] = 0x7F;

    strcat(newinput, input);
    // while (strlen(newinput) % BLOCK != 0) { // do i even need padding
    //     strcat(newinput, "0"); //this was "\0"
    // }

    unsigned char *cipher = malloc(MAX);
    int i;
    int j; // set up here so it runs each one
    for (i=0; i<strlen(input); i+=strlen(secret)) {
        for(int j=0; j<strlen(secret); j++) {
            cipher[i+j] = input[i+j] ^ secret[j];
        }
    }
    cipher[i] = '\0';\

    return cipher;
}

unsigned char* decrypt_data(unsigned char input[], BIGNUM *s) {
    unsigned char secret[BLOCK];
    BN_bn2bin(s, secret);

    unsigned char *decrypted = malloc(MAX);
    int i;
    int j; // set up here so it runs each one
    for (i=0; i<strlen(input); i+=strlen(secret)) {
        for(j=0; j<strlen(secret); j++) {
            decrypted[i+j] = input[i+j] ^ secret[j];
        }
    } 
    decrypted[i] = '\0';

    return decrypted;
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

int main() {
    unsigned char *cipher = malloc(MAX);
    unsigned char *decrypted;
    char message[300];
    unsigned char buff[MAX] = "send hello";
    BIGNUM *s = BN_new();
    BN_hex2bn(&s, "2A");

    subString(buff, 5, strlen(buff)-5, message);
    cipher = encrypt_data(message, s);

    printf("message: %s %d\n", message, strlen(message));
    printf("buff: %s\n", buff);
    printf("cipher: %s\n", cipher);
    char send[MAX];
    int len = strlen(message);
    sprintf(send, "server send 4 6 %s", cipher);
    strtok(send, " ");
    strtok(NULL, " ");
    char *id = strtok(NULL, " ");

    char enc[300];
    printf("send: %s %d\n", send,strlen(send));
    memcpy(enc, &send[9+7], len);
    printf("enc: %s %d\n", enc, strlen(enc));
    decrypted = decrypt_data(enc, s);

    printf("decrypted: %d \"%s\"\n", atoi(id), decrypt_data(cipher, s));

    return 0;
}