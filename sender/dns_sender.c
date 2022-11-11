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
#include <netdb.h>
#include <err.h>
#include <stdbool.h>
#include <unistd.h>
#include "dns_sender_events.h"

#define BUFFER 1024 // buffer length
#define PORT 20004   // port number

int main(int argc, char **argv)
{

    int sockfd;
    int msg_size, i;
    struct sockaddr_in server, cliaddr; // address structures of the server and the client
    socklen_t len, cliaddrLen;
    char buffer[BUFFER];

    char *DNS_Server;
    bool ns = false;    // control variable if the nameserver was set by parameter or not
    bool input = false; // control variable if the input was set by parameter or not
    char *Base_Host;
    char *DST_Filepath;
    char *SRC_Filepath;
    FILE* fp;

    if (argc < 3)
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
            Base_Host = argv[1];
            DST_Filepath = argv[2];
            printf("Base_Host: %s \n DST_Filepath: %s \n", Base_Host, DST_Filepath);
        }
        else if (argc == 4)
        {
            Base_Host = argv[1];
            DST_Filepath = argv[2];
            SRC_Filepath = argv[3];
            input = true;
        }
        else if (argc == 5)
        {
            if (strcmp(argv[1], "-u") == 0)
            {
                ns = true;
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
                ns = true;
                input = true;
                DNS_Server = argv[2];
                Base_Host = argv[3];
                DST_Filepath = argv[4];
                SRC_Filepath = argv[5];
                printf("DNS Server: %s \n Base_Host: %s \n Filepath: %s \n", DNS_Server, Base_Host, DST_Filepath);
            }
            else
            {
                printf("Wrong parameters");
                return 1;
            }
        }
    }

    if (ns == false)
    {
        char Nameserver[1024];
        FILE *fpp;
        fpp = popen("cat /etc/resolv.conf | grep 'nameserver' | awk -F ' ' '{print $2}'", "r");
        fgets(Nameserver, sizeof(Nameserver), fpp);
        fclose(fpp);
        printf("Nameserver: %s \n", Nameserver);
        DNS_Server = Nameserver;
        printf("DNS Server: %s \n", DNS_Server);
    }

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("* Server socket created %i\n", sockfd);

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(DNS_Server);

    len = sizeof(server);
    cliaddrLen = sizeof(cliaddr);

    if (input == true)
    {
        fp = fopen(SRC_Filepath, "r");
        if (fp == NULL)
        {
            printf("Could not open file %s", SRC_Filepath);
            return 1;
        }
    }else
    {
        fp = stdin;
    }

    while ((msg_size = read(STDIN_FILENO, buffer, BUFFER)) > 0)
    // read input data from STDIN (console) until end-of-line (Enter) is pressed
    // when end-of-file (CTRL-D) is received, n == 0
    {
        printf("* Sending message to server\n");
        i = sendto(sockfd, buffer, msg_size, MSG_CONFIRM, (const struct sockaddr *)&server, len); // send data to the server
        printf("* Data sent from %s, port %d (%d) to %s, port %d (%d)\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), cliaddr.sin_port, inet_ntoa(server.sin_addr), ntohs(server.sin_port), server.sin_port);

        // read the answer from the server)
        i = recvfrom(sockfd, (char *)buffer, BUFFER, MSG_WAITALL, (struct sockaddr *)&cliaddr, &len);
        buffer[i] = '\0';
        // printf("Client : %s\n", buffer);

        printf("* UDP packet received from %s, port %d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
        printf("%.*s", i, buffer); // print the answer
    }
    // reading data until end-of-file (CTRL-D)

    if (msg_size == -1)
        err(1, "reading failed");
    if (input == true)
        fclose(fp);
    close(sockfd);
    printf("* Closing the client socket ...\n");
    return 0;
}
