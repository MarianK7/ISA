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
#include "../base32.h"
#include "../base32.c"
#include "../dns_packet.h"

#define BUFFER 1024 // buffer length
#define MAX 100
#define PORT 53 // port number

packet create_response(char *base_host, char *buffer)
{
    // base_host[0] = 9;
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

    for (i = 0; i < strlen((char *)host); i++)
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

int main(int argc, char **argv)
{
    int sockfd;
    char buffer[BUFFER] = {0};
    char respond[BUFFER] = {0};
    char DST_Filepath[MAX] = {0};
    struct sockaddr_in servaddr, cliaddr;
    socklen_t length;
    char *received = "DST_Filepath received\n";

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
        printf("Base_Host: %s \n Dirpath: %s \n", Base_Host, DST_Dirpath);
    }

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

    int n, r;

    length = sizeof(cliaddr);

    char decoded[1024];
    char data[1024];
    int sizeofpacket;
    int size;
    packet p;
    FILE *fp = NULL;

    while ((n = recvfrom(sockfd, buffer, BUFFER, 0, (struct sockaddr *)&cliaddr, &length)) >= 0)
    {
        printf("data received from %s, port %d\n", inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port));
        p = create_response(dnsFormat, buffer);
        if (ntohs(p.header.id) == 10)
        {
            size = buffer[12];
            memcpy(data, &(buffer[13]), size);
            data[size] = '\0';
            printf("DATA: %s\n", data);
            base32_decode((unsigned char *)data, (unsigned char *)decoded);
            printf("Decoded: %s\n", decoded);
            memcpy(DST_Filepath, &(decoded), strlen(decoded));
            printf("Filepath: %s\n", DST_Filepath);
            
            memset(decoded, 0, 1024);
            memset(data, 0, 1024);
            fp = fopen(DST_Filepath, "w");
            if (fp == NULL)
            {
                printf("Error opening file \n");
                return 1;
            }
            
            
            memcpy(respond, &p, sizeof(p.header));                                                                          // copy the packet to the buffer
            memcpy(respond + sizeof(p.header), p.question.qname, strlen(p.question.qname) + 1);                             // copy the question to the buffer
            memcpy(respond + sizeof(p.header) + strlen(p.question.qname) + 1, &p.question.qdaco, sizeof(p.question.qdaco)); // copy the question to the buffer
            sizeofpacket = sizeof(p.header) + strlen(p.question.qname) + 1 + sizeof(p.question.qdaco);                      // calculate the size of the packet

            r = sendto(sockfd, respond, sizeofpacket, 0, (struct sockaddr *)&cliaddr, length); // send the answer

            printf("data \"%.*s\" sent to %s, port %d\n", r - 1, buffer, inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
            continue;
        }
        else if (ntohs(p.header.id) == 20)
        {
            fclose(fp);
            memcpy(respond, &p, sizeof(p.header));                                                                          // copy the packet to the buffer
            memcpy(respond + sizeof(p.header), p.question.qname, strlen(p.question.qname) + 1);                             // copy the question to the buffer
            memcpy(respond + sizeof(p.header) + strlen(p.question.qname) + 1, &p.question.qdaco, sizeof(p.question.qdaco)); // copy the question to the buffer
            sizeofpacket = sizeof(p.header) + strlen(p.question.qname) + 1 + sizeof(p.question.qdaco);                      // calculate the size of the packet

            r = sendto(sockfd, respond, sizeofpacket, 0, (struct sockaddr *)&cliaddr, length); // send the answer

            printf("data \"%.*s\" sent to %s, port %d\n", r - 1, buffer, inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
            continue;
        }
        else
        {
            size = buffer[12];
            memcpy(data, &(buffer[13]), size);
            data[size] = '\0';
            printf("DATA: %s\n", data);
            base32_decode((unsigned char *)data, (unsigned char *)decoded);
            printf("Decoded: %s\n", decoded);
            printf("Decoded len: %d\n", strlen(decoded));

            fputs(decoded, fp);

            memset(decoded, 0, 1024);
            memset(data, 0, 1024);

            memcpy(respond, &p, sizeof(p.header));                                                                          // copy the packet to the buffer
            memcpy(respond + sizeof(p.header), p.question.qname, strlen(p.question.qname) + 1);                             // copy the question to the buffer
            memcpy(respond + sizeof(p.header) + strlen(p.question.qname) + 1, &p.question.qdaco, sizeof(p.question.qdaco)); // copy the question to the buffer
            sizeofpacket = sizeof(p.header) + strlen(p.question.qname) + 1 + sizeof(p.question.qdaco);                      // calculate the size of the packet

            r = sendto(sockfd, respond, sizeofpacket, 0, (struct sockaddr *)&cliaddr, length); // send the answer

            printf("data \"%.*s\" sent to %s, port %d\n", r - 1, buffer, inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
            continue;
        }

    }

    return 0;
}
