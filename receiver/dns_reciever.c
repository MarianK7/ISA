#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/stat.h>
#include "dns_receiver_events.h"
#include "dns_receiver_events.c"
#include "../base32.h"
#include "../base32.c"
#include "../dns_packet.h"

#define BUFFER 1024 // buffer length
#define MAX 100
#define PORT 53 // port number

packet create_response(unsigned char *base_host, char *buffer)
{
    packet p;
    memcpy(&(p.header.id), buffer, 2);
    p.header.qr = 1;
    p.header.opcode = 0;
    p.header.aa = 0;
    p.header.tc = 0;
    p.header.rd = 1;
    p.header.ra = 0;
    p.header.z = 0;
    p.header.rcode = 0;
    p.header.q_count = htons(1);
    p.header.ans_count = 0;
    p.header.auth_count = 0;
    p.header.add_count = 0;
    p.question.qname = base_host;
    p.question.qdaco.qtype = htons(1);
    p.question.qdaco.qclass = htons(1);

    return p;
}

void ChangetoDnsNameFormat(unsigned char *dns, unsigned char *host)
{
    int lock = 0, i;
    strcat((char *)host, ".");

    for (i = 0; i < (int)strlen((char *)host); i++)
    {
        if (host[i] == '.')
        {
            *dns++ = i - lock;
            for (; lock < i; lock++)
            {
                *dns++ = host[lock];
            }
            lock++;
        }
    }
    *dns++ = '\0';
}

static volatile bool run = false;
FILE *fp;

void intHandler() 
{
    if (run == true)
    {
        fclose(fp);
    }
    printf("\nShutting down...\n");
    exit(0);  
}

int main(int argc, char **argv)
{
    int sockfd;
    char buffer[BUFFER] = {0};
    char respond[BUFFER] = {0};
    char DST_Filepath[MAX] = {0};
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
    }

    signal(SIGINT, intHandler);

    struct stat st = {0};
    if (stat(DST_Dirpath, &st) == -1)
    {
        mkdir(DST_Dirpath, 0777);
    }

    char dirPath[256] = {0};
    strcat(dirPath, DST_Dirpath);

    // Change Base_Host to DNS format
    char dnsFormat[strlen(Base_Host) + 1];
    ChangetoDnsNameFormat((unsigned char *)dnsFormat, (unsigned char *)Base_Host);

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

    int n;

    length = sizeof(cliaddr);

    char decoded[1024];
    char data[1024];
    char finalPath[1024] = {0};
    char hostID[256] = {0};
    char basehostFormated[256] = {0};
    char basehostPrint[256] = {0};
    char basehostDataPrint[256] = {0};
    int sizeofpacket;
    int msgSize = 0;
    int size;
    int sizeID;
    long int res;
    packet p;
    int chunkID = 0;
    strcat(basehostPrint, ".");
    strcat(basehostPrint, Base_Host);
    ChangetoDnsNameFormat((unsigned char *)basehostFormated, (unsigned char *)Base_Host);

    while ((n = recvfrom(sockfd, buffer, BUFFER, 0, (struct sockaddr *)&cliaddr, &length)) >= 0)
    {
        p = create_response((unsigned char *)dnsFormat, buffer);
        size = buffer[12];
        sizeID = size + 13;
        strcpy(hostID, &(buffer[sizeID]));

        if (strcmp(hostID, basehostFormated) == 0)
        {
            if (ntohs(p.header.id) == 10)
            {
                memcpy(data, &(buffer[13]), size);
                data[size] = '\0';
                base32_decode((unsigned char *)data, (unsigned char *)decoded);
                memcpy(DST_Filepath, &(decoded), strlen(decoded));
                strcat(finalPath, dirPath);
                strcat(finalPath, "/");
                strcat(finalPath, DST_Filepath);

                memset(decoded, 0, 1024);
                memset(data, 0, 1024);
                fp = fopen(finalPath, "w");
                run = true;
                if (fp == NULL)
                {
                    printf("Error opening file \n");
                    return 1;
                }

                dns_receiver__on_transfer_init(&(cliaddr.sin_addr));

                memcpy(respond, &p, sizeof(p.header));                                                                          // copy the packet to the buffer
                memcpy(respond + sizeof(p.header), p.question.qname, strlen((const char *)p.question.qname) + 1);                             // copy the question to the buffer
                memcpy(respond + sizeof(p.header) + strlen((const char *)p.question.qname) + 1, &p.question.qdaco, sizeof(p.question.qdaco)); // copy the question to the buffer
                sizeofpacket = sizeof(p.header) + strlen((const char *)p.question.qname) + 1 + sizeof(p.question.qdaco);                      // calculate the size of the packet

                sendto(sockfd, respond, sizeofpacket, 0, (struct sockaddr *)&cliaddr, length); // send the answer

                continue;
            }
            else if (ntohs(p.header.id) == 20)
            {
                fseek(fp, 0L, SEEK_END);
                res = ftell(fp);
                dns_receiver__on_transfer_completed(finalPath, (int)res);
                fclose(fp);
                run = false;

                memset(finalPath, 0, 1024);
                memcpy(respond, &p, sizeof(p.header));                                                                          // copy the packet to the buffer
                memcpy(respond + sizeof(p.header), p.question.qname, strlen((const char *)p.question.qname) + 1);                             // copy the question to the buffer
                memcpy(respond + sizeof(p.header) + strlen((const char *)p.question.qname) + 1, &p.question.qdaco, sizeof(p.question.qdaco)); // copy the question to the buffer
                sizeofpacket = sizeof(p.header) + strlen((const char *)p.question.qname) + 1 + sizeof(p.question.qdaco);                      // calculate the size of the packet

                sendto(sockfd, respond, sizeofpacket, 0, (struct sockaddr *)&cliaddr, length); // send the answer

                continue;
            }
            else
            {
                memcpy(data, &(buffer[13]), size);
                data[size] = '\0';
                strcat(basehostDataPrint, data);
                strcat(basehostDataPrint, basehostPrint);
                dns_receiver__on_query_parsed(finalPath, basehostDataPrint);
                memset(basehostDataPrint, 0, 256);
                msgSize = base32_decode((unsigned char *)data, (unsigned char *)decoded);
                
                fwrite(decoded, 1, msgSize, fp);

                memset(decoded, 0, 1024);
                memset(data, 0, 1024);

                dns_receiver__on_chunk_received(&(cliaddr.sin_addr), finalPath, chunkID, msgSize);
                chunkID++;

                memcpy(respond, &p, sizeof(p.header));                                                                          // copy the packet to the buffer
                memcpy(respond + sizeof(p.header), p.question.qname, strlen((const char *)p.question.qname) + 1);                             // copy the question to the buffer
                memcpy(respond + sizeof(p.header) + strlen((const char *)p.question.qname) + 1, &p.question.qdaco, sizeof(p.question.qdaco)); // copy the question to the buffer
                sizeofpacket = sizeof(p.header) + strlen((const char *)p.question.qname) + 1 + sizeof(p.question.qdaco);                      // calculate the size of the packet

                sendto(sockfd, respond, sizeofpacket, 0, (struct sockaddr *)&cliaddr, length); // send the answer

                continue;
            }
        }
        else
        {
            printf("Host ID is incorrect %s\n", hostID);
            continue;
        }
    }

    printf("Shutting down...\n");
    return 0;
}
