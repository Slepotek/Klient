#include <arpa/inet.h> //htonl, htons, ntohl, nthos, inet_addr etc.
#include <stdio.h> //printf, etc.
#include <stdlib.h> //malloc, realloc, free, exit etc.
#include <string.h> //memcpy, memset, strcpy, etc.
#include <sys/socket.h> //socklen_t, sockaddr, msghdr, etc.
#include "struktury.h"
#include <linux/if.h>// this one supposignly gets the mac address TODO: compleate the reference
#include <sys/ioctl.h>// this one contains macro SIOCGIFHWADDR that does TODO: compleate what this macro does

#define NUMER_STUDENTA 1

void operacja1 (int *sock_descr, struct sockaddr_in* saddr, int *addr_len, /*unsigned char* buff,*/ int *dane, int rozm_dane);

int main (void)
{
    int sock_r = 0;
    int sockaddr_len = 0;
    struct sockaddr_in klient;
    sockaddr_len = sizeof(klient);
    printf("Otwieram gniazdo Klienta:\n");
    sock_r = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sock_r == -1)
    {
        printf("Blad podczas otwierania gniazda klienta, przerywam program.\n");
        printf("Sprawdz czy program uruchomiono jako root.\n");
        return 0;
    }
    klient.sin_addr.s_addr = inet_addr("127.0.0.12");
    klient.sin_family = AF_INET;
    klient.sin_port = htons(7777);

    if(bind(sock_r, (struct sockaddr*)&klient, sizeof(klient)) < 0)
    {
        printf("Problem z dowiazaniem gniazda do adresu\n");
    }
    operacja1(&sock_r, &klient, &sockaddr_len, /*buffer,*/ NULL, 0);

    return EXIT_SUCCESS;
}

void operacja1 (int *sock_descr, struct sockaddr_in* saddr, int *addr_len, /*unsigned char* buff,*/ int *dane, int dane_rozm)
{
    unsigned char *buff = (unsigned char*) malloc(65536);
    memset(buff, 0, sizeof(*buff));
    int buffer_len = 0;
    //budujemy naglowek wlasnego protokolu
    struct pwphead naglowek;
    naglowek.headDlug = 4; // domyslnie dlugosc naglowka to 4 bajty
    naglowek.kodOper = 0;
    naglowek.rozmDanych = dane_rozm;
    naglowek.typDanych = 0;
    naglowek.wersja = 1;

    struct iphead ipheader; 
    ipheader.calk_dlug = sizeof(struct iphead)+dane_rozm+naglowek.headDlug;
    ipheader.czas_zy = 255;
    ipheader.destyn = inet_addr("127.0.0.13");
    ipheader.dlugosc = 5;
    ipheader.frag_flagi = 0;
    ipheader.identy = htonl(12345);
    ipheader.protok = 144 + NUMER_STUDENTA;
    ipheader.suma_kontr = 0;
    ipheader.uslug_ecn = 0xe0;
    ipheader.wersja = 4;
    ipheader.zrodlo = inet_addr("127.0.0.12");

    /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      + Procedura wyluskania adresu MAC przypisanego do gniazda +
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    struct ifreq s; //struct for socket ioctl's, it can contain among others the MAC address of the interface attached to the socket
    //unsigned char *if_name = (unsigned char*) malloc(IFNAMSIZ);//the name of the interface must be set
    char *if_name = "enp0s3";
    unsigned char *mac_addr = NULL;
    int if_name_len = IFNAMSIZ;

    //getsockopt(*sock_descr, SOL_SOCKET, SO_BINDTODEVICE, if_name, (socklen_t*)&if_name_len); //should get the socket eth interface name

    memset(&s, 0, sizeof(struct ifreq));

    s.ifr_addr.sa_family = AF_INET;
    strncpy(s.ifr_name, if_name, IFNAMSIZ-1);

    if(0 == ioctl(*sock_descr, SIOCGIFHWADDR, &s))
    {
        mac_addr = (unsigned char*)s.ifr_hwaddr.sa_data;
        //display mac address
        printf("Mac : %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n" , mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    }

    
    /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      +           Koniec procedury wyluskania adresu MAC                +
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

    struct sockaddr_in ipAddres;
    int ipAddres_len = sizeof(struct sockaddr);
    getsockname(*sock_descr, (struct sockaddr*) &ipAddres, (socklen_t*) &ipAddres_len);
    printf("Adres IP gniazda klienta :%s\n", inet_ntoa(ipAddres.sin_addr));

    int *wektor_liczb = malloc(sizeof(int)*10);
    printf("Podaj 10 liczb: ");
    for(int i = 0; i < 10; i++)
    {
        scanf("%d", &wektor_liczb[i]);
    }
    memcpy(buff, &ipheader, sizeof(ipheader));
    buffer_len = sizeof(ipheader);
    memcpy(buff+buffer_len, &naglowek, sizeof(naglowek));
    buffer_len =buffer_len+sizeof(naglowek);
    memcpy(buff+buffer_len, mac_addr, 6);
    buffer_len =buffer_len+6;
    memcpy(buff+buffer_len, &ipAddres.sin_addr, sizeof(struct in_addr));
    buffer_len =buffer_len+sizeof(struct in_addr);
    memcpy(buff+buffer_len, wektor_liczb, sizeof(int)*10);
    buffer_len = buffer_len+(sizeof(int)*10);

    int rec = sendto(*sock_descr, buff, buffer_len, 0, (struct sockaddr*)saddr, *addr_len);
    if (rec < 0)
    {
        printf("Blad podczas wysylania");
    }
}