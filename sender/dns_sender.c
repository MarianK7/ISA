#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include "dns_sender_events.h"

int main(int argc, char **argv)
{

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

    return 0;
}
