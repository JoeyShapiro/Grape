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

int main() {
    unsigned char input[] = "helloWorld";
    unsigned char cipher[10];
    unsigned char output[10];
    unsigned char s = 'c';
    int i;

    printf("cipher: ");
    for (i = 0; i<strlen(input); i++) {
        cipher[i] = input[i] ^ s;
        printf(" %02x", cipher[i]);
    }

    cipher[i]= '\0';

    for (i = 0; i<strlen(cipher); i++) {
        output[i] = cipher[i] ^ s;
        printf(" %02x", output[i]);
    }
    output[i] = '\0';

    printf("\noutput: %s length: %d\n", output, strlen(output));

    printf("\nBLOCK\n");
    unsigned char secret[] = "cc";
    unsigned char input2[] = "helloWorld";
    unsigned char cipher2[10];
    unsigned char output2[10];

    printf("\ninput2: %s length: %d\n", input2, strlen(input2));
    int math = strlen(input2)/strlen(secret);
    printf("%d = %d / %d\n",math, strlen(input2), strlen(secret));

    printf("cipher2: ");
    for (int i=0; i<strlen(input2); i+=strlen(secret)) {
        for(int j=0; j<strlen(secret); j++) {
            cipher2[i+j] = input2[i+j] ^ secret[j];
            printf(" %02x", cipher2[i+j]);
        }
    }
    cipher2[i]= '\0';

    printf("\nsecret: %d\n", strlen(secret));

    for (int i=0; i<strlen(cipher2); i+=strlen(secret)) {
        for(int j=0; j<strlen(secret); j++) {
            output2[i+j] = cipher2[i+j] ^ secret[j];
            printf(" %02x", output2[i+j]);
        }
    } 
    output2[i] = '\0';

    printf("\n%s\n", output2);
}