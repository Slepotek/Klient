#include <arpa/inet.h> //htonl, htons, ntohl, nthos, inet_addr etc.
#include <stdio.h> //printf, etc.
#include <stdlib.h> //malloc, realloc, free, exit etc.
#include <string.h> //memcpy, memset, strcpy, etc.
#include <sys/socket.h> //socklen_t, sockaddr, msghdr, etc.
#include "struktury.h"

#define NUMER_STUDENTA 1

struct sockaddr_in socketAddres; //inserted for training but will be needed
struct sockaddr socketAddresNative;
void operacja1 (int *sock_descr, struct sockaddr* saddr, int *addr_len, unsigned char* buff, int *dane, int rozm_dane);

int main (void)
{
    int sock_r = 0;
    int sockaddr_len = 0;
    int buffer_len = 0;
    struct sockaddr_in klient;
    unsigned char *buffer = (unsigned char*) malloc(65536);
    memset(buffer, 0, sizeof(*buffer));
    printf("Otwieram gniazdo Klienta:/n");
    sock_r = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sock_r == -1)
    {
        printf("Blad podczas otwierania gniazda klienta, przerywam program./n");
        printf("Sprawdz czy program uruchomiono jako root./n");
        return 0;
    }
    klient.sin_addr.s_addr = inet_addr("127.0.0.12");
    klient.sin_family = AF_INET;
    klient.sin_port = htons(7777);

    if(bind(sock_r, (struct sockaddr*)&klient, sizeof(klient)) < 0)
    {
        printf("Problem z dowiazaniem gniazda do adresu");
    }


    return EXIT_SUCCESS;
}

void operacja1 (int *sock_descr, struct sockaddr* saddr, int *addr_len, unsigned char* buff, int *dane, int dane_rozm)
{
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

    //lookup ARP header to obtain mac address, there is solution on github the comand is like ic*something*
    //TODO: get the MAC address of current interface
    sendto(*sock_descr, &buff, sizeof(buff), 0, &saddr, *addr_len);
}