#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//networking related header files
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#include "message.h"
#include "utils.h"
#include "base64.h"

int main(int argc, char const *argv[])
{
    
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent* server=NULL;
    
    char buffer[256];
    
    //need 2 args to work
    if(argc<3){
        fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
        exit(0);
    }

    
    portno = atoi(argv[2]); //get port from arg
    sockfd = socket(AF_INET, SOCK_STREAM, 0); //create socket
    if(sockfd < 0){ //check if created
        error("Socket not created!\n");
    }

    server= gethostbyname(argv[1]); //get information about the server
    if(server == NULL){ //check if succesful
        fprintf(stderr, "%s host not found\n", argv[1]);
        exit(0);
    }

    memset((void*)&serv_addr, 0, sizeof(serv_addr)); //clear serv_addr
    serv_addr.sin_family = AF_INET; //set family
    memcpy((void*)&serv_addr.sin_addr.s_addr,(void*)server->h_addr,
    server->h_length);//copy server's IP address to sock_addr
    serv_addr.sin_port = htons(portno); //handle network byte order(BIG ENDIAN)

    //check if successful connection possible
    if(connect(sockfd,(struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        error("Error connecting");
    
    int option = 0;
    struct Message msg;
    while(option != 2){

        printf("\n\n1. Send Message\n");
        printf("2. Quit\n");
        printf("Choice:");
        scanf("%d", &option);
        getchar(); //remove newline

        switch(option){
            case 1:
                printf("Please enter the message:");
        
                msg.type = TYPE1;
                readLine(msg.message,MESSAGE_LEN);
                
                char* encoded = encode(msg.message, strlen(msg.message));
                
                if(strlen(encoded) < MESSAGE_LEN)
                    strcpy(msg.message, encoded);
                else
                    printf("Error: Too Big to be sent\n");
                n = writeMsgToSocket(sockfd,&msg);
                free(encoded);

                if(n<0) //check if written
                    error("Error connecting");

                //while there is data to be read
                n = readMsgFromSocket(sockfd, &msg);
                if(n>0)
                    printf("Type:%d Message:%s\n", msg.type, msg.message);
                                
                break;

            case 2:
                //send TYPE3 message
                msg.type = TYPE3;
                strcpy(msg.message, "QUIT");
                n = writeMsgToSocket(sockfd,&msg);

                if(n<0) //check if written
                    error("Error connecting");
                break;
            default:
                printf("Invalid option\n");

        }        
    }
    
    close(sockfd);
    
    return 0;
}
