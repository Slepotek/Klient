#include <arpa/inet.h> //htonl, htons, ntohl, nthos, inet_addr etc.
#include <stdio.h> //printf, etc.
#include <stdlib.h> //malloc, realloc, free, exit etc.
#include <string.h> //memcpy, memset, strcpy, etc.
#include <sys/socket.h> //socklen_t, sockaddr, msghdr, etc.
#include "struktury.h"
#include <linux/if.h>// this one supposignly gets the mac address TODO: compleate the reference
#include <sys/ioctl.h>// this one contains macro SIOCGIFHWADDR that does TODO: compleate what this macro does
#include <linux/if_ether.h>
#include <unistd.h>

#define NUMERSTUDENTA 1
void PrintIpHddr(unsigned char *data, int Size); //funkcja do wyswietlenia naglowka IP + wpisanie danych do wlasnej struktury z naglowkiem
void PrintPwpHddr(struct pwphead* data, int size);
void wstaw(Element_listy **wsk_nagl);
void print(Element_listy *element);
void usun(Element_listy **header, int pozycja);
void PrintMacAddr(unsigned char * buff, int *poz);
void PrintIpAddr(unsigned char *buff, int *poz);
void WyslijZwrot(struct pwphead *hddr, unsigned char* buff, int* sock_descr);
void LiczbyDoListy(unsigned char* buff, int *poz, Element_listy *lista);
struct sockaddr_in source, dest;
struct sockaddr_in serwer, klient;

int sockaddr_len = 0;

int main (void)
{
    int sock_r = 0;
    int rozmiar_danych = 0;
    unsigned char* bufor = (unsigned char*) malloc (65536);
    memset(bufor, 0, 65536);
    sockaddr_len = sizeof(serwer);
    printf("Otwieram gniazdo serwerowe:\n");
    sock_r = socket(AF_INET, SOCK_RAW, 144+NUMERSTUDENTA);
    if (sock_r < 0)
    {
        printf("Blad podczas otwieraniu gniazda serwerowego. Przerywam program.\n");
        printf("Sprawdz czy program uruchomiono jako root...");
        return 0;
    }
    printf("Gniazdo otwarte\n");
    serwer.sin_addr.s_addr = inet_addr("127.0.0.13");
    serwer.sin_family = AF_INET;
    serwer.sin_port = htons(7777);

    printf("Probuje dowiazac adres IP do gniazda: ");
    if(bind(sock_r, (struct sockaddr*)&serwer, sockaddr_len) < 0)
    {
        printf("problem z dowiazaniem gniazda do adresu\n");
        close(sock_r);
        return 0;
    }
    printf("adres dowiazany\n");
    while(1)
    {
        printf("Uruchamiam nasluch...\n");
        rozmiar_danych = recvfrom(sock_r, bufor, 65536, 0, (struct sockaddr*)&serwer, (socklen_t*)&sockaddr_len);
        if (rozmiar_danych < 0)
        {
            printf("Blad podczas odbierania danych: blad lub timeout\n");
        }
        int j = 0;
        printf("Odebralem ramke protokolu PWP: \n");
        for (int i = 0; i <1000 ; i++) 
        {
            printf("%.2x ", bufor[i]);
            j++;
            if (0 == j%20)
            { // wypisywanie po 20 oktetów
                printf ("\n");
            }
        }
        printf("\n");
        printf("Naglowki przenoszonych protokolow: ");
        int bufor_poz = 0;
        struct iphead iphddr;
        memcpy(&iphddr, bufor, sizeof(iphddr));
        bufor_poz = bufor_poz + sizeof(iphddr);
        printf("\nNaglowek IP wyglada tak: \n");
        PrintIpHddr(&iphddr, sizeof(iphddr));
        klient.sin_addr.s_addr = iphddr.zrodlo;
        klient.sin_family = AF_INET;
        klient.sin_port = htons(7777);
        struct pwphead naglowek;
        memcpy(&naglowek, bufor+bufor_poz, sizeof(naglowek));
        bufor_poz = bufor_poz+sizeof(naglowek);
        printf("Naglowek PWP wyglada tak:\n");
        PrintPwpHddr(&naglowek, sizeof(naglowek));
        if (naglowek.opcje.optKod == 1)
        {
            bufor = naglowek.opcje.optDane;
        }
        Element_listy *lista = (Element_listy*) malloc(sizeof(Element_listy));
        switch(naglowek.typDanych)
        {
            case TYP1:
                printf("Przeslano tylko adres MAC\n");
                PrintMacAddr(bufor, &bufor_poz);
                //Uzupelnij bufor pamieci prawidlowymi danymi
                //zerowanie naglowka PWP
                memset(&naglowek, 0, sizeof(naglowek));
                naglowek.kodOper = KOD7;
                naglowek.typDanych = TYP4;
                naglowek.rozmDanych = 0;
                naglowek.wersja = 1;
                naglowek.opcje.optKod = 0;
                naglowek.headDlug = 6;
                WyslijZwrot(&naglowek, bufor, &sock_r);
                break;
            case TYP2:
                printf("Przeslano tylko adres IP\n");
                PrintIpAddr(bufor, &bufor_poz);
                memset(&naglowek, 0, sizeof(naglowek));
                naglowek.kodOper = KOD7;
                naglowek.typDanych = TYP4;
                naglowek.rozmDanych = 0;
                naglowek.wersja = 1;
                naglowek.opcje.optKod = 0;
                naglowek.headDlug = 6;
                WyslijZwrot(&naglowek, bufor, &sock_r);
                break;
            case TYP3:
                printf("Przeslano tylko liczby\n");
                LiczbyDoListy(bufor, &bufor_poz, lista);//wpisz liczby do listy wiazanej
                memset(&naglowek, 0, sizeof(naglowek));
                naglowek.kodOper = KOD6;
                naglowek.typDanych = TYP4;
                naglowek.rozmDanych = 0;
                naglowek.wersja = 1;
                naglowek.opcje.optKod = 0;
                naglowek.headDlug = 6;
                WyslijZwrot(&naglowek, bufor, &sock_r);
                break;
            case TYP5:
                printf("Przeslano komplet danych\n");
                PrintMacAddr(bufor, &bufor_poz);
                PrintIpAddr(bufor, &bufor_poz);
                LiczbyDoListy(bufor, &bufor_poz, lista);
                memset(&naglowek, 0, sizeof(naglowek));
                naglowek.kodOper = KOD6;
                naglowek.typDanych = TYP4;
                naglowek.rozmDanych = 0;
                naglowek.wersja = 1;
                naglowek.opcje.optKod = 0;
                naglowek.headDlug = 6;
                WyslijZwrot(&naglowek, bufor, &sock_r);
                break;
            default:
                printf("Nieprawidlowa operacja\n");
                break;
        }
        memset(bufor, 0, 65536);
        printf("Poprawnie wyslano potwierdzenie odebrania danych do klienta.\n");
    }
}

void PrintIpHddr(unsigned char *data, int Size) { //funkcja do wyswietlenia naglowka IP + wpisanie danych do wlasnej struktury z naglowkiem
	unsigned short iphdrlen;//zmienna z dlugoscia naglowka (ilosc danych w naglowku)
	struct iphead *iph = (struct iphead*) data;//inicjalizacja wlasnej struktury naglowka ip (po wiecej info patrz netstructs.h
	iphdrlen = iph->dlugosc * 4;//wylicz dlugosc danych naglowka
	memset(&source, 0, sizeof(source));//wyzerowanie struktury adresu zrodla (adresu czyli np. 127.0.0.1)
	source.sin_addr.s_addr = iph->zrodlo;// wpisz do struktury zrodla odebrany w ramce adres IP zrodla
	memset(&dest, 0, sizeof(dest));//jak wyzej tylko dla adresu przeznaczenia
	dest.sin_addr.s_addr = iph->destyn;// wpisz do struktury adresu przeznaczenia adres odebrany w ramce IP
	printf("\n");
	printf("IP Header\n");
	//i po kolei wypisujemy kolejne elementy ze struktury Przypominam funkcja ntohs odwraca bity, zeby system je prawidlowo odebral po przeslaniu, nie wszystkie pola trzeba w ten sposob traktowac, tylko te ktore nie sa wyswietlane w hexie
	printf(" |-IP wersja: %d\n", (unsigned int) iph->wersja);
	printf(" |-IP dlugosc naglowka : %d bajtow\n", ((unsigned int) (iph->dlugosc)) * 4);
	printf(" |-Typ uslugi : %d\n", (unsigned int) iph->uslug_ecn);
	printf(" |-IP calkowita dlugosc wiadomosci : %d bajtow\n", ntohs(iph->calk_dlug));
	printf(" |-Identification : %d\n", ntohs(iph->identy));
	printf(" |-TTL : %d\n", (unsigned int) iph->czas_zy);
	printf(" |-Protocol : %d\n", (unsigned int) iph->protok);
	printf(" |-Checksum : %d\n", ntohs(iph->suma_kontr));
	printf(" |-Source IP: %s\n", inet_ntoa(source.sin_addr));//tutaj funkcja inet_ntoa to wbudowana funkcja biblioteki bodaj netinet arpa oraz socket, sluzy do wyswietlenia adresu w postaci jaka wszyscy znamy czyli np. 127.0.0.1
	printf(" |-Destination IP : %s\n", inet_ntoa(dest.sin_addr));
	printf("\n");
}
void PrintPwpHddr(struct pwphead* data, int size)
{
    printf("\n");
    printf("PWP Header\n");
    printf("|-PWP wersja: %d\n", data->wersja);
    printf("|-PWP kod operacji: %d\n", data->kodOper);
    printf("|-PWP typ danych: %d\n", data->typDanych);
    printf("|-PWP dlugosc nagl: %d\n", ((unsigned int)(data->headDlug))*4);
    printf("|-PWP rozmiar danych: %d\n", data->rozmDanych);
    int optCheck = data->headDlug;
    if(optCheck == 4)
    {
        printf("Brak opcji\n");
    }
}

//FUNKCJE LISTY WIAZANEJ
void wstaw(Element_listy **wsk_nagl) {
	Element_listy *nowy_wpis = (Element_listy*) malloc(sizeof(Element_listy)); //zajmij pamięć dla nowego elementu listy

	nowy_wpis->liczba = (int*) malloc(sizeof(int));
	nowy_wpis->nastepny = (*wsk_nagl); //wpisz pod adres elementu następnego pierwszy element z listy
	nowy_wpis->poprzedni = NULL; //adres poprzedniego elementu w nowym elemencie wyzeruj (elementy dodawane są na początek listy)

	if ((*wsk_nagl) != NULL) //jeżeli pierwszy element nie jest pusty (trzeba sprqawdzić, żeby nie pojawił się błąd)
	{
		(*wsk_nagl)->poprzedni = nowy_wpis; //wpisz w pole adresu elementu poprzedniego, adres poprzednio pierwszego elementu
	}
	(*wsk_nagl) = nowy_wpis; //przypisz pod adres pierwszego elementu(header) nowy element listy
}
void print(Element_listy *element) {
	printf("\nElementy listy: \n");
	while (element != NULL) //jeżeli pod adresem "element" nie ma pustej pamięci
	{
        printf("%d \n", *element->liczba);
		element = element->nastepny; //przenieś adres do następnego elementu z listy
	}
}

void usun(Element_listy **header, int pozycja) {
	Element_listy *temp; //tymczasowa zmienna, używana żeby zapewnić spójność listy
	Element_listy *pTemp; //tak samo jak wyżej
	temp = *header; //przypisz do tymczasowej zmiennej adres pierwszego elementu listy
	pTemp = *header; //to samo co wyżej (ale później przechowuje element poprzedzający temp)
	for (int i = 0; i < pozycja; i++) { //przechodź po kolejnych elementach listy aż do zadanego elementu
		if (i == 0 && pozycja == 1) { //jeżeli wskazany element to pierwszy element z listy
			*header = (*header)->nastepny; //pod adres pierwszego elementu przypisz adres następnego elementu z pierwszego elementu
			free(temp); // uwlonij pamięć zaalokowaną dla tymczasowych zmiennych
		} else {
			if (i == pozycja - 1 && temp) { //jeżeli wskazywana pozycja jest równa pozycji elementu - 1 i temp != NULL
				pTemp->nastepny = temp->nastepny; //przypisz następny element elementu temp jako następny element elementu poprzedzającego element temp
				free(temp);
			} else { //jeżeli iterator nie dotarł jeszcze do wymaganej pozycji
				pTemp = temp; //przepisz element ze zmiennej temp do zmienej pTemp

				if (pTemp == NULL) //sprawdź czy nie przekroczono zakresu listy
				{
					break; //jeżeli przekroczono - przerwij wykonywanie procedury
				}
				temp = temp->nastepny; // zmień adres zmiennej temp na kolejny element listy
				//teraz w zmiennych pomocniczych przechowywany jest
				/*temp - bierzący element w iteracji
				 pTemp - element poprzedzający element temp*/
			}
		}
	}
}

void PrintMacAddr(unsigned char * buff, int *poz)
{
    unsigned char *mac_addr = (unsigned char*)malloc(sizeof(unsigned char)*6);
    memcpy(mac_addr, buff+*poz, 6);
    *poz = *poz + 6;
    //display mac address
    printf("Mac: %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n" , mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
}

void PrintIpAddr(unsigned char *buff, int *poz)
{
    struct in_addr ipAdres;
    memcpy(&ipAdres, buff+*poz, sizeof(ipAdres));
    printf("Adres IP gniazda klienta %s\n", inet_ntoa(ipAdres));
    *poz = *poz + sizeof(ipAdres);
}
void LiczbyDoListy(unsigned char* buff, int *poz, Element_listy *lista)
{
    lista->nastepny = NULL;
    lista->poprzedni = NULL;
    lista->pierwszy = &lista;
    lista->liczba = (int*)malloc(sizeof(int));
    memcpy(lista->liczba, buff+*poz, sizeof(int));
    *poz = *poz+sizeof(int);
    for(int i = 0; i < 9; i++)
    {
        wstaw(&lista);
        memcpy(lista->liczba, buff+*poz, sizeof(int));
        *poz = *poz+sizeof(int);
    }
    print(lista);
}
//Wysyla wiadomosc zwrotna z serwera
//Wymaga wskaznika do naglowka oraz wskaznika do bufora
//deskryptor gniazda jest globalny, wiec korzystamy bezposrednio
void WyslijZwrot(struct pwphead *hddr, unsigned char* buff, int* sock_descr)
{
    memset(buff, 0, 65536);
    memcpy(buff, hddr, sizeof(*hddr));
    int sizeOfBuff = sizeof(*hddr);
    int *psockAddrLen = &sockaddr_len;
    int rozmiar_danych = sendto(*sock_descr, buff, sizeOfBuff, 0, (struct sockaddr*)&klient, *psockAddrLen);
    if(rozmiar_danych < 0)
    {
        printf("Nie udalo sie wyslac wiadomosci\n");
        printf("Mozliwe zablokowanie gniazda lub, blad konfiguracji\n");
    }
}
