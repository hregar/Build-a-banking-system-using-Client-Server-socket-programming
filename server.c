#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//netwoking related header files
#include <sys/types.h> 
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>

#include "message.h"
#include "base64.h"

//generic method to output error and exit
void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno;
     socklen_t clilen;
     char buffer[256];
     struct sockaddr_in serv_addr, cli_addr;
     int n;
     
     if (argc < 2) {
         fprintf(stderr,"ERROR: no port provided\n");
         exit(1);
     }
     
     portno = atoi(argv[1]);
     sockfd = socket(AF_INET, SOCK_STREAM, 0); //create socket
     if (sockfd < 0) //check if socket created
        error("Socket not created!\n");
        
     memset((void*)&serv_addr, 0, sizeof(serv_addr)); //clear serv_addr
   
     serv_addr.sin_family = AF_INET; //set family
     
     //memcpy((void*)&serv_addr.sin_addr.s_addr,(void*)INADDR_ANY);
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     
     serv_addr.sin_port = htons(portno); //handle network byte order(BIG ENDIAN)
     
     //binding the socket to the address and port number specified
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR in binding");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     
    //extracts the first connection request on the queue of pending connections for the listening socket, 
    //sockfd, creates a new connected socket, and returns a new file descriptor referring to that socket
    newsockfd = accept(sockfd, 
                 (struct sockaddr *) &cli_addr, 
                 &clilen);
                 
    if (newsockfd < 0) 
        error("ERROR on accept");
    struct Message msg;

    printf("%-5s\t%-10s\t%-10s\n", "Type", "Encoded", "Decoded");
    while(1)
    {
    
        n = readMsgFromSocket(newsockfd, &msg);
        
        if (n < 0) error("ERROR in reading from socket");
        
        //decode
        int len=0;
        char* decoded = decode(msg.message,&len);
        decoded[len] = '\0';

        printf("%d\t%s\t%s\n", msg.type, msg.message, decoded);
        free(decoded);

        if(msg.type == TYPE3)
            break;
        
        msg.type = TYPE2;
        strcpy(msg.message, "Message Received");
        n = writeMsgToSocket(newsockfd, &msg);
        
        //handling the error in writing to socket
        if (n < 0) error("ERROR in writing to socket");
      
    }
    close(newsockfd);
    close(sockfd);
     return 0;
}
