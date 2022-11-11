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

#define BUFFER 1024 // buffer length
#define PORT 20004 // port number

int main(int argc, char **argv)
{
    int sockfd;
    char buffer[1024];
    struct sockaddr_in servaddr, cliaddr;
    socklen_t length;

    char *Base_Host;
    char *DST_Dirpath;

    if (argc < 3)
    {
        printf("Wrong parameters \n");
        return 1;
    }
    else if (argc > 4)
    {
        printf("Too many parameters \n");
        return 1;
    }
    else
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
    servaddr.sin_port = htons(PORT);

    // Bind the socket with the server address
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    int n, r;

    length = sizeof(cliaddr); // len is value/resuslt

    while ((n = recvfrom(sockfd, buffer, BUFFER, 0, (struct sockaddr *)&cliaddr, &length)) >= 0)
    {
        printf("data received from %s, port %d\n", inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port));

        for (r = 0; r < n; r++)
            if (islower(buffer[r]))
                buffer[r] = toupper(buffer[r]);
            else if (isupper(buffer[r]))
                buffer[r] = tolower(buffer[r]);

        r = sendto(sockfd, buffer, n, 0, (struct sockaddr *)&cliaddr, length); // send the answer
        if (r == -1)
            err(1, "sendto()");
        else if (r != n)
            errx(1, "sendto(): Buffer written just partially");
        else
            printf("data \"%.*s\" sent to %s, port %d\n", r - 1, buffer, inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
    }
    printf("closing socket\n");
    //close(sockfd);

    return 0;
}
