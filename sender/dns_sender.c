#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <arpa/inet.h>

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Wrong parameters");
    }else if (argc >= 3)
    {
        if (strcmp(argv[1],"-u") == 0)
        {
            char *DNS_Server = argv[2];
            printf("DNS Server: %s \n", DNS_Server);
        }
        
    }
    
    return 0;
}

