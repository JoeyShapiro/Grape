#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <netdb.h>
#include <arpa/inet.h>
// for bool
#include <ncurses.h>

#define TRUE 1
#define FALSE 0
#define PORT 8787
#define MAX 1024

bool startsWith(unsigned char *str, unsigned char *exp) {
    if (strncmp(str, exp, strlen(exp)) == 0) return 1;
    return 0;
}

int main(int argc, char *argv[]) {
    int opt = TRUE;
    int master_socket, addrlen, new_socket, client_socket[30], max_clients = 30, activity, i, valread, sd;
    int max_sd;
    struct sockaddr_in address;
    char publicKeys[30][256];

    char buffer[1025];
    fd_set readfds;
    char *message = "server: ECHO Daemon v1.0 \r\n";

    for (i=0; i<max_clients; i++) {
        client_socket[i] = 0;
    }

    if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 ) {
        perror("setsocketopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", PORT);

    if (listen(master_socket, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    addrlen = sizeof(address);
    puts("Waiting for connections ...");

    while(TRUE) {
        FD_ZERO(&readfds);

        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        for (i=0; i<max_clients; i++) {
            sd = client_socket[i];

            if(sd > 0)
                FD_SET(sd, &readfds);

            if(sd > max_sd)
                max_sd = sd;
        }

        activity = select(max_sd+1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            printf("select error");
        }

        if (FD_ISSET(master_socket, &readfds))  
        {  
            if ((new_socket = accept(master_socket, 
                    (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)  
            {  
                perror("accept");  
                exit(EXIT_FAILURE);  
            }  
             
            //inform user of socket number - used in send and receive commands 
            printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));  
           
            //send new connection greeting message 
            if( send(new_socket, message, strlen(message), 0) != strlen(message) )  
            {  
                perror("send");  
            }  
                 
            puts("Welcome message sent successfully");
            for (i = 0; i < max_clients; i++)  
            {  
                //if position is empty 
                if( client_socket[i] == 0 )  
                {  
                    client_socket[i] = new_socket;  
                    printf("Adding to list of sockets as %d\n" , i);  
                         
                    break;  
                }  
            }  
        }  
        //else its some IO operation on some other socket
        for (i = 0; i < max_clients; i++)  
        {  
            sd = client_socket[i];  
                 
            if (FD_ISSET( sd , &readfds))  
            {  
                //Check if it was for closing , and also read the 
                //incoming message 
                valread = read( sd , buffer, MAX);
                printf("\nA user sent something\n");
                if (startsWith(buffer, "chat ")) {
                    char *user = strtok(buffer, " ");
                    user = strtok(NULL, " ");
                    int id = atoi(user);
                    char *pub = publicKeys[id];
                    printf("user %d asked for %d's pubkey\n", sd, id);
                    char pubid[500];
                    sprintf(pubid, "system pubid %d %s", id, pub);
                    send(sd , pubid , strlen(pubid) , 0 );
                    continue;
                }
                if (startsWith(buffer, "user ")) {
                    char *pub = strtok(buffer, " ");
                    pub = strtok(NULL, " ");
                    strcpy(publicKeys[sd], pub);
                    printf("user %s\n", pub);
                    continue;
                }
                if (startsWith(buffer, "system")) {
                    char *arg = strtok(buffer, " ");
                    char *subarg = strtok(NULL, " ");
                    if (startsWith(subarg, "secret")) {
                        int id = atoi(strtok(NULL, " "));
                        char *enc_sec = strtok(NULL, " ");
                        char msg[400];
                        sprintf(msg, "%s %s %d %s", arg, subarg, sd, enc_sec);
                        printf("user %d sent a key to user %d\n", sd, id);
                        send(id, msg, strlen(msg), 0);
                        continue;
                    }
                    if (startsWith(subarg, "send")) {
                        char *id = strtok(NULL, " ");
                        char *len = strtok(NULL, " ");
                        char enc_msg[300]; // = strtok(NULL, " ");
                        char msg[300];
                        char sig[256];
                        memcpy(enc_msg, &buffer[9+7], atoi(len));
                        memcpy(sig, &buffer[17+atoi(len)], 122);

                        sprintf(msg, "%s %s %d %s %s %s", arg, subarg, sd, len, enc_msg, sig);
                        printf("message; %s\n",msg);
                        send(atoi(id), msg, strlen(msg), 0);
                        send(sd, msg, strlen(msg), 0);
                        printf("user %d sent a message to user %s of length %d %s\n", sd, id, strlen(enc_msg), enc_msg);
                        continue;
                    }
                }
                if (startsWith(buffer, "list")) {

                }
                for (int b = 0; b < sizeof(&buffer); b ++) {
                        printf(" %02x", buffer[b]);
                }


                if (valread == 0)  
                {  
                    //Somebody disconnected , get his details and print 
                    getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);  
                    printf("Host disconnected , ip %s , port %d \n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));  
                         
                    //Close the socket and mark as 0 in list for reuse 
                    close( sd );  
                    client_socket[i] = 0;  
                }  
                //Echo back the message that came in 
                else 
                {  
                    //set the string terminating NULL byte on the end 
                    //of the data read 
                    
                    // for (int b = 0; b < sizeof(&buffer); b ++) {
                    //     printf(" %02x", buffer[b]);
                    // }
                    //printf("User %d sent %x", sd, buffer);
                    //for (int j=0; j<max_clients; j++)
                    //{
                    send(sd , buffer , strlen(buffer) , 0 ); // also sends to sender (makes sense)
                    //}
                }  
                
            }  
        }  

    }

    return 0;
}