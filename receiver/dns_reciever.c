#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include "dns_receiver_events.h"

int main(int argc, char **argv)
{

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
    
    
}
