#include <arpa/inet.h> //htonl, htons, ntohl, nthos, inet_addr etc.
#include <stdio.h> //printf, etc.
#include <stdlib.h> //malloc, realloc, free, exit etc.
#include <string.h> //memcpy, memset, strcpy, etc.
#include <sys/socket.h> //socklen_t, sockaddr, msghdr, etc.
#include "struktury.h"
#include <unistd.h>//socket closing
#include <linux/if.h>// this one supposignly gets the mac address TODO: compleate the reference
#include <sys/ioctl.h>// this one contains macro SIOCGIFHWADDR that does TODO: compleate what this macro does

#define NUMER_STUDENTA 1

void operacja1 (unsigned char * buff, int *sock_descr);
void Odbierz(int * sock_descr, unsigned char * bufor);
void Wyslij(int *sock_descr, unsigned char *bufor);
struct sockaddr_in klient, serwer;
int sockaddr_len = 0;
int buffer_len = 0;

int main (void)
{
    int sock_r = 0;
    sockaddr_len = sizeof(klient);
    printf("Otwieram gniazdo Klienta:\n");
    sock_r = socket(AF_INET, SOCK_RAW, 144+NUMER_STUDENTA);
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

    serwer.sin_family = AF_INET;
    serwer.sin_port = htons(7777);
    serwer.sin_addr.s_addr = inet_addr("127.0.0.13");
    printf("Wybierz typ operacji jaka chcesz wykonac: \n");
    printf("    |- 1 -> Przygotowac i przeslac w polu danych adres MAC, IP oraz 10 cyfr\n");
    printf("    |- 2 -> Przygotowac i przeslac w polu opcji adres MAC, IP oraz 10 cyfr\n");
    printf("    |- 3 -> Przygotowac i przeslac w polu danych tylko adres MAC\n");
    printf("    |- 4 -> Przygotowac i przeslac w polu danych tylko adres IP\n");
    printf("    |- 5 -> Przygotowac i przeslac w polu danych tylko 10 cyfr\n");
    int wyb = 0;
    scanf("%d", &wyb);
    printf("Wybrales %d\n", wyb);

    unsigned char* buff = (unsigned char*)malloc(65536);

    switch (wyb)
    {
        case 1:
            operacja1(buff, &sock_r);
            struct pwphead naglowek;
            Wyslij(&sock_r, buff);
            do
            {
                struct iphead ipHead;
                int poz = 0;
                Odbierz(&sock_r, buff);
                memcpy(&ipHead, buff, sizeof(ipHead));
                poz = poz + sizeof(ipHead);
                memcpy(&naglowek, buff+poz, sizeof(naglowek));
                poz = poz + sizeof(naglowek);
            } while (naglowek.typDanych != TYP4);  
            printf("Wiadomosc przeslano i odebrano przez serwer.\n");
            break;
        case 2:
            //TODO: Zaimplementowac operacje 2
            break;
        case 3:
            //TODO: Zaimplementowac operacje 3
            break;
        case 4:
            //TODO: Zaimplementowac operacje 4
            break;
        case 5:
            //TODO: Zaimplementowac operacje 5
            break;
        default:
            printf("Zamykam aplikacje\n");
            return EXIT_SUCCESS;
            break;
    }
}

void operacja1 (unsigned char * buff, int *sock_descr)
{
    memset(buff, 0, sizeof(*buff));
    //budujemy naglowek wlasnego protokolu
    struct pwphead naglowek;
    naglowek.headDlug = 4; // domyslnie dlugosc naglowka to 4 bajty
    naglowek.kodOper = KOD1;
    naglowek.rozmDanych = sizeof(struct in_addr)+(sizeof(int)*10)+6; //TODO: Uzupelnic tutaj
    naglowek.typDanych = TYP5;
    naglowek.wersja = 1;
    naglowek.opcje.optKod = 0;

    /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      + Procedura wyluskania adresu MAC przypisanego do gniazda +
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    struct ifreq s; //struct for socket ioctl's, it can contain among others the MAC address of the interface attached to the socket
    char *if_name = "enp0s3"; //tak dziala, w przypadku zwyklego srodowiska wystarczy dowolny interfejs
    unsigned char *mac_addr = NULL;
    int if_name_len = IFNAMSIZ;

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

    //wpisz do struktury adresu ip adres ip klienta (tak lepiej jest go przeslac)
    struct sockaddr_in ipAddres;
    int ipAddres_len = sizeof(struct sockaddr);
    getsockname(*sock_descr, (struct sockaddr*) &ipAddres, (socklen_t*) &ipAddres_len);
    printf("Adres IP gniazda klienta :%s\n", inet_ntoa(ipAddres.sin_addr));

    //pobierz 10 liczb od uzytkownika
    int *wektor_liczb = malloc(sizeof(int)*10);
    printf("Podaj 10 liczb: \n");
    for(int i = 0; i < 10; i++)
    {
        scanf("%d", &wektor_liczb[i]);
    }

    //Budowanie bufora z danymi
    memcpy(buff+buffer_len, &naglowek, sizeof(naglowek));
    buffer_len =buffer_len+sizeof(naglowek);
    memcpy(buff+buffer_len, mac_addr, 6);
    buffer_len =buffer_len+6;
    memcpy(buff+buffer_len, &ipAddres.sin_addr, sizeof(struct in_addr));
    buffer_len =buffer_len+sizeof(struct in_addr);
    memcpy(buff+buffer_len, wektor_liczb, sizeof(int)*10);
    buffer_len = buffer_len+(sizeof(int)*10);
}

void Odbierz(int * sock_descr, unsigned char * bufor)
{
    int rec = recvfrom(*sock_descr, bufor, 65536, 0, (struct sockaddr*)&serwer, (socklen_t*)&sockaddr_len);
    if (rec < 0)
    {
        printf("Blad podczas odbierania danych\n");
    }
    memcpy(&buffer_len, &rec, sizeof(rec));
}

void Wyslij(int *sock_descr, unsigned char *bufor)
{
    int* addr_len = &sockaddr_len;
    int rec = sendto(*sock_descr, bufor, buffer_len, 0, (struct sockaddr*)&serwer, sockaddr_len);
    if (rec < 0)
    {
        printf("Blad podczas wysylania\n");
    }
    memset(bufor, 0, 65536);
    buffer_len = 0;
}