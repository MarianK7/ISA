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
#include "../base32.h"
#include "../base32.c"
#include "../dns_packet.h"

#define BUFFER 1024 // buffer length
#define DATA 30
#define PORT 53 // port number

packet create_packet(char *base_host)
{
    // base_host[0] = 9;
    packet packet;
    packet.header.id = (unsigned short)htons(getpid());
    packet.header.qr = 0;
    packet.header.opcode = 0;
    packet.header.aa = 0;
    packet.header.tc = 0;
    packet.header.rd = 1;
    packet.header.ra = 0;
    packet.header.z = 0;
    packet.header.rcode = 0;
    packet.header.q_count = htons(1);
    packet.header.ans_count = 0;
    packet.header.auth_count = 0;
    packet.header.add_count = 0;
    packet.question.qname = base_host;
    packet.question.qdaco.qtype = htons(1);
    packet.question.qdaco.qclass = htons(1);

    return packet;
}

packet create_custom_packet(char *base_host, unsigned short id)
{
    // base_host[0] = 9;
    packet p;
    p.header.id = (unsigned short)htons(id);
    p.header.qr = 0;
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
    struct sockaddr_in server, cliaddr; // address structures of the server and the client
    int sockfd;
    int msg_size = 0, i;
    socklen_t len, cliaddrLen;
    char buffer[BUFFER] = {0};
    char data[256] = {0};

    char *DNS_Server;
    bool ns = false;    // control variable if the nameserver was set by parameter or not
    bool input = false; // control variable if the input was set by parameter or not
    char *Base_Host;
    char *DST_Filepath;
    char *SRC_Filepath;
    FILE *fp;

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
            }
            else
            {
                printf("Wrong parameters");
                return 1;
            }
        }
    }

    // change the base host to DNS format
    char path_dnsFormat[256] = {0};
    char formatedFirst[256] = {0};
    int sizeofpacket;
    char formated_encoded[256];
    // send filepath to the receiver
    strcat(path_dnsFormat, DST_Filepath); // copy the filepath to the path_dnsFormat becouse bad things are happening
    base32_encode((unsigned char *)path_dnsFormat, strlen(path_dnsFormat), (unsigned char *)formated_encoded);
    for (int i = 0; i < strlen(formated_encoded); i++)
    {
        if (formated_encoded[i] != '=' && (formated_encoded[i] < 'A' || formated_encoded[i] > 'Z') && (formated_encoded[i] < '2' || formated_encoded[i] > '7'))
        {
            formated_encoded[i] = '\0';
        }
    }
    memset(path_dnsFormat, 0, 256);
    strcat(formated_encoded, ".");
    strcat(formated_encoded, Base_Host);
    ChangetoDnsNameFormat((unsigned char *)path_dnsFormat, (unsigned char *)formated_encoded);

    if (ns == false)
    {
        char Nameserver[1024];
        FILE *fpp;
        fpp = popen("cat /etc/resolv.conf | grep 'nameserver' | awk -F ' ' '{print $2}'", "r");
        fgets(Nameserver, sizeof(Nameserver), fpp);
        fclose(fpp);
        DNS_Server = Nameserver;
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
    }
    else
    {
        fp = stdin;
    }

    packet p = create_custom_packet(path_dnsFormat, 10);                                                           // create the packet
    memcpy(buffer, &p, sizeof(p.header));                                                                          // copy the packet to the buffer
    memcpy(buffer + sizeof(p.header), p.question.qname, strlen(p.question.qname) + 1);                             // copy the question to the buffer
    memcpy(buffer + sizeof(p.header) + strlen(p.question.qname) + 1, &p.question.qdaco, sizeof(p.question.qdaco)); // copy the question to the buffer
    sizeofpacket = sizeof(p.header) + strlen(p.question.qname) + 1 + sizeof(p.question.qdaco);                     // calculate the size of the packet
    printf("id: %i \n", p.header.id);

    printf("* Sending message to server\n");
    i = sendto(sockfd, buffer, sizeofpacket, MSG_CONFIRM, (const struct sockaddr *)&server, len); // send data to the server
    printf("* Data sent from %s, port %d (%d) to %s, port %d (%d)\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), cliaddr.sin_port, inet_ntoa(server.sin_addr), ntohs(server.sin_port), server.sin_port);

    // read the answer from the server
    i = recvfrom(sockfd, (char *)buffer, BUFFER, MSG_WAITALL, (struct sockaddr *)&cliaddr, &len);
    buffer[i] = '\0';
    printf("Answer: %s\n", buffer);
    memset(buffer, 0, 1024);

    printf("* UDP packet received from %s, port %d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

    char encoded[256];
    char formated[256];
    packet packet;

    while ((msg_size = fread(data, 1, DATA, fp)) != 0)
    // read input data from STDIN (console) until end-of-line (Enter) is pressed
    // when end-of-file (CTRL-D) is received, n == 0
    {

        base32_encode((unsigned char *)data, msg_size, (unsigned char *)encoded);
        for (int i = 0; i < strlen(encoded); i++)
        {
            if (encoded[i] != '=' && (encoded[i] < 'A' || encoded[i] > 'Z') && (encoded[i] < '2' || encoded[i] > '7'))
            {
                encoded[i] = '\0';
            }
        }
        memset(data, 0, 256);
        // printf("Data encoded: %s \n", encoded);
        strcat(encoded, ".");
        // printf("Data encoded + dot: %s \n", encoded);
        // printf("Lenghth encoded: %li \n", strlen(encoded));
        strcat(encoded, Base_Host);
        printf("Data encoded + basehost: %s \n", encoded);
        ChangetoDnsNameFormat((unsigned char *)formated, (unsigned char *)encoded);
        memset(encoded, 0, 256);

        packet = create_packet(formated);
        memcpy(buffer, &packet, sizeof(packet.header));                                                                                    // copy the packet to the buffer
        memcpy(buffer + sizeof(packet.header), packet.question.qname, strlen(packet.question.qname) + 1);                                  // copy the question to the buffer
        memcpy(buffer + sizeof(packet.header) + strlen(packet.question.qname) + 1, &packet.question.qdaco, sizeof(packet.question.qdaco)); // copy the question to the buffer
        sizeofpacket = sizeof(packet.header) + strlen(packet.question.qname) + 1 + sizeof(packet.question.qdaco);

        printf("* Sending message to server\n");
        i = sendto(sockfd, buffer, sizeofpacket, MSG_CONFIRM, (const struct sockaddr *)&server, len); // send data to the server
        printf("* Data sent from %s, port %d (%d) to %s, port %d (%d)\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), cliaddr.sin_port, inet_ntoa(server.sin_addr), ntohs(server.sin_port), server.sin_port);
        memset(buffer, 0, 1024);
        memset(formated, 0, 256);

        // read the answer from the server
        i = recvfrom(sockfd, (char *)buffer, BUFFER, MSG_WAITALL, (struct sockaddr *)&cliaddr, &len);
        buffer[i] = '\0';
        printf("Answer: %s\n", buffer);
        memset(buffer, 0, 1024);
        memset(formated, 0, 256);

        printf("* UDP packet received from %s, port %d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
    }
    char formatedBaseHost[256];
    memset(data, 0, 256);
    memset(encoded, 0, 256);
    strcat(data, "Finished");
    base32_encode((unsigned char *)data, strlen(data), (unsigned char *)encoded);
    for (int i = 0; i < strlen(encoded); i++)
    {
        if (encoded[i] != '=' && (encoded[i] < 'A' || encoded[i] > 'Z') && (encoded[i] < '2' || encoded[i] > '7'))
        {
            encoded[i] = '\0';
        }
    }
    memset(data, 0, 256);
    // printf("Data encoded: %s \n", encoded);
    strcat(encoded, ".");
    // printf("Data encoded + dot: %s \n", encoded);
    // printf("Lenghth encoded: %li \n", strlen(encoded));
    strcat(encoded, Base_Host);
    printf("Data encoded + basehost: %s \n", encoded);
    ChangetoDnsNameFormat((unsigned char *)formated, (unsigned char *)encoded);
    memset(encoded, 0, 256);

    packet = create_custom_packet(formated, 20);
    memcpy(buffer, &packet, sizeof(packet.header));                                                                                    // copy the packet to the buffer
    memcpy(buffer + sizeof(packet.header), packet.question.qname, strlen(packet.question.qname) + 1);                                  // copy the question to the buffer
    memcpy(buffer + sizeof(packet.header) + strlen(packet.question.qname) + 1, &packet.question.qdaco, sizeof(packet.question.qdaco)); // copy the question to the buffer
    sizeofpacket = sizeof(packet.header) + strlen(packet.question.qname) + 1 + sizeof(packet.question.qdaco);

    printf("* Sending final message to server\n");
    i = sendto(sockfd, buffer, sizeofpacket, MSG_CONFIRM, (const struct sockaddr *)&server, len); // send data to the server
    printf("* Data sent from %s, port %d (%d) to %s, port %d (%d)\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), cliaddr.sin_port, inet_ntoa(server.sin_addr), ntohs(server.sin_port), server.sin_port);
    memset(buffer, 0, 1024);
    memset(formated, 0, 256);

    // read the answer from the server
    i = recvfrom(sockfd, (char *)buffer, BUFFER, MSG_WAITALL, (struct sockaddr *)&cliaddr, &len);
    buffer[i] = '\0';
    printf("Answer: %s\n", buffer);
    memset(buffer, 0, 1024);
    memset(formated, 0, 256);

    if (input == true)
        fclose(fp);
    close(sockfd);
    printf("* Closing the client socket ...\n");
    return 0;
}
