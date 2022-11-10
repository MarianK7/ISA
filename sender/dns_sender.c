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
#include "dns_sender_events.h"

int main(int argc, char **argv)
{

    int sockfd;
    struct sockaddr_in servaddr;
    char buffer[1024];
    char *hello = "Hello from client";

    char *DNS_Server;
    char *Base_Host;
    char *DST_Filepath;
    char *SRC_Filepath;

    if (argc < 2)
    {
        printf("Wrong parameters");
        return 1;
    }
    else if (argc > 6)
    {
        printf("Too many parameters");
        return 1;
    }
    else
    {
        if (argc == 3)
        {
            Base_Host = argv[2];
            DST_Filepath = argv[3];
        }
        else if (argc == 4)
        {
            Base_Host = argv[2];
            DST_Filepath = argv[3];
            SRC_Filepath = argv[4];
        }
        else if (argc == 5)
        {
            if (strcmp(argv[1], "-u") == 0)
            {
                DNS_Server = argv[2];
                Base_Host = argv[3];
                DST_Filepath = argv[4];
                printf("DNS Server: %s \n Base_Host: %s \n Filepath: %s \n", DNS_Server, Base_Host, DST_Filepath);
            }
            else
            {
                printf("Wrong parameters");
                return 1;
            }
        }
        else if (argc == 6)
        {
            if (strcmp(argv[1], "-u") == 0)
            {
                DNS_Server = argv[2];
                Base_Host = argv[3];
                DST_Filepath = argv[4];
                printf("DNS Server: %s \n Base_Host: %s \n Filepath: %s \n", DNS_Server, Base_Host, DST_Filepath);
            }
            else
            {
                printf("Wrong parameters");
                return 1;
            }
        }
    }

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(8080);

    int n, len;

    sendto(sockfd, (const char *)hello, strlen(hello), MSG_CONFIRM, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    printf("Hello message sent.\n");

    n = recvfrom(sockfd, (char *)buffer, 1024, MSG_WAITALL, (struct sockaddr *)&servaddr, &len);
    buffer[n] = '\0';
    printf("Server : %s\n", buffer);

    close(sockfd);

    return 0;
}
