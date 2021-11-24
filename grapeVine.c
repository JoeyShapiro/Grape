#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
// for multi-threading
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define MAX 128 // TODO what do
#define PORT 8787 // like grapes on a vine :P
#define SA struct sockaddr // TODO what do

void *rec(void *vargp) {
    int *sockfd = vargp;
    char buff[MAX];
    int n;

    while(1) {
        bzero(buff, MAX);
        read(sockfd, buff, sizeof(buff));
        printf("%s\t", buff);
    }

    return NULL;
}

// function for chat between server and client
void *sen(void *vargp) {
    int *sockfd = vargp;
    char buff[MAX];
    int n;

    // inf loop for chat
    while (1) {
        bzero(buff, MAX);
        n = 0;

        while ((buff[n++] = getchar()) != '\n')
            ;

        write(sockfd, buff, sizeof(buff));

        if (strncmp("bye", buff, 3) == 0) {
            printf("Server Exit...\n");
            break;
        }
    }

    return NULL;
}

int main() {
    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;
    pthread_t tid;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    } else
        printf("Socket successfully created...\n");
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    } else
        printf("socket successfully binded...\n");
    
    if ((listen(sockfd, 5)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    } else
        printf("Server listening...\n");
    len = sizeof(cli);

    connfd = accept(sockfd, (SA*)&cli, &len);
    if (connfd < 0) {
        printf("Server accept failed...\n");
        exit(0);
    } else
        printf("server accepted the client...\n");
    
    pthread_create(&tid, NULL, sen, connfd);
    pthread_create(&tid, NULL, rec, connfd);
    pthread_exit(NULL);

    close(sockfd);
    
    return 0;
}