#include <arpa/inet.h> //htonl, htons, ntohl, nthos, inet_addr etc.
#include <stdio.h> //printf, etc.
#include <stdlib.h> //malloc, realloc, free, exit etc.
#include <string.h> //memcpy, memset, strcpy, etc.
#include <sys/socket.h> //socklen_t, sockaddr, msghdr, etc.
#include "struktury.h"
#include <linux/if.h>// this one supposignly gets the mac address TODO: compleate the reference
#include <sys/ioctl.h>// this one contains macro SIOCGIFHWADDR that does TODO: compleate what this macro does

int main (void)
{
    int sock_r = 0;
    int sockaddr_len = 0;
    struct sockaddr_in serwer;
    sockaddr_len = sizeof(serwer);
    printf("Otwieram gniazdo serwerowe:\n");
    sock_r = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sock_r < 0)
    {
        printf("Blad podczas otwieraniu gniazda serwerowego. Przerywam program.\n");
        printf("Sprawdz czy program uruchomiono jako root...");
        return 0;
    }
    serwer.sin_addr.s_addr = inet_addr("127.0.0.13");
    serwer.sin_family = AF_INET;
    serwer.sin_port = htons(7777);

    if(bind(sock_r, (struct sockaddr*)&serwer, sockaddr_len) < 0)
    {
        
    }

}