#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h> 
#include "dns_receiver_events.h"

int main(int argc, char **argv)
{
    int sockfd;
    char buffer[1024];
    char *hello = "Hello from server";
    struct sockaddr_in servaddr, cliaddr;

    char *Base_Host;
    char *DST_Dirpath;

    if(argc < 3){
        printf("Wrong parameters \n");
        return 1;
    }else if (argc > 4){
        printf("Too many parameters \n");
        return 1;
    }else
    {
        Base_Host = argv[1];
        DST_Dirpath = argv[2];
        printf("Base_Host: %s \n Filepath: %s \n", Base_Host, DST_Dirpath);
    }
    
    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Filling server information
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(8080);

    // Bind the socket with the server address
    if (bind(sockfd, (const struct sockaddr *)&servaddr,
             sizeof(servaddr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    int n, len;

    len = sizeof(cliaddr); //len is value/resuslt

    n = recvfrom(sockfd, (char *)buffer, 1024,
                 MSG_WAITALL, (struct sockaddr *)&cliaddr,
                 &len);
    buffer[n] = '\0';
    printf("Client : %s\n", buffer);

    sendto(sockfd, (const char *)hello, strlen(hello),
           MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
           len);
    printf("Hello message sent.\n");

    return 0;

}
