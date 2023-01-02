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

#define NUMERSTUDENTA 1 //pamietac zeby zmienic na swoj numer, tak jak to tam ustali prowadzacy 

/*Mozna zmienic nastepujace nazwy
    PrintIpHddr
    PrintPwpHddr
    wstaw
    print
    usun
    PrintMacAddr
    PrintIpAddr
    WyslijZwrot
    LiczbyDoListy
    source
    dest
    serwer
    klient
    sockaddr_len
    sock_r
    rozmiar_danych
    bufor
    iphddr
    naglowek
    lista
    data
    Size
    element
    header
    hddr
    buff
    sock_descr
    wsk_nagl
    pozycja
    poz

    Mozna zmienic ale nie polecam ( trzeba zmienic tez w pliku struktury.h)
    Element_listy
    liczba
    poprzedni
    nastepny
    pierwszy
    iphead
    dlugosc
    wersja
    uslug_ecn
    calk_dlug
    identy
    frag_flagi
    czas_zy
    protok
    suma_kontr
    zrodlo
    destyn
    pwpopcje
    optKod
    optDlug
    optDane
    pwphead
    kodOper
    typDanych
    headDlug
    rozmDanych
    opcje

    Mozna tez zmienic nazwy makr
    KOD1
    KOD2
    KOD3
    itd...
    TYP1
    TYP2
    TYP3
    itd...
*/

void PrintIpHddr(unsigned char *data, int Size); //funkcja do wyswietlenia naglowka IP + wpisanie danych do wlasnej struktury z naglowkiem
void PrintPwpHddr(struct pwphead* data, int size);//funkcja do wyswietlania naglowka PWP
//funkcje listy wiazanej
void wstaw(Element_listy **wsk_nagl);
void print(Element_listy *element);
void usun(Element_listy **header, int pozycja);
//koniec funkcji listy wiazanej
void PrintMacAddr(unsigned char * buff, int *poz);//funkcja do pobrania adresu MAC
void PrintIpAddr(unsigned char *buff, int *poz);//funkcja do pobrania adresu IP
void WyslijZwrot(struct pwphead *hddr, unsigned char* buff, int* sock_descr);//funkcja do wysylania wiadomosci zwrotneej (naglowka PWP z potwierdzeniem (TYP4))
void LiczbyDoListy(unsigned char* buff, int *poz, Element_listy *lista);//funkcja wpisujaca kolejne liczby z bufora otrzymanego od klienta do listy wiazanej
struct sockaddr_in source, dest;//struktury adresu IP (uzywane w funkcji PrintIpAddr)
struct sockaddr_in serwer, klient;//rowniez struktury adresu IP uzywane ogolnie w programie, mozna by bylo zastapic je powyzszymi

int sockaddr_len = 0;//rozmiar struktury adresu IP (domyslnie jest to 16 bajtow)

int main (void)
{
    int sock_r = 0;//deskryptor gniazda
    int rozmiar_danych = 0;//zmienna do kotrej wpisujemy wynikk dzialnia funkcji "sendto" i "recvfrom"
    unsigned char* bufor = (unsigned char*) malloc (65536);//zadeklarowanie i zaalokowanie pamieci dla bufora danych (65536 - maksymalny rozmiar ramki IP)
    memset(bufor, 0, 65536);//wyzerowanie bufora danych 
    sockaddr_len = sizeof(serwer);//wpisanie rozmiaru struktury adresu serwera do zmiennej przechowujacej rozmiar struktury adresu
    printf("Otwieram gniazdo serwerowe:\n");
    sock_r = socket(AF_INET, SOCK_RAW, 144+NUMERSTUDENTA);//otworzenie gniazda sieciowego
    if (sock_r < 0)
    {
        printf("Blad podczas otwieraniu gniazda serwerowego. Przerywam program.\n");
        printf("Sprawdz czy program uruchomiono jako root...\n");
        return 0;
    }
    printf("Gniazdo otwarte\n");
    serwer.sin_addr.s_addr = inet_addr("127.0.0.13");//wpisanie do zmiennej strukturalnej wartosci porzadanego przez nas adresu IP strony serwera (taki sam adres musi byc w programie klienta przy strukturze serwera)
    serwer.sin_family = AF_INET;//wpisanie do zmiennej strukturalnej rodziny protokolow
    serwer.sin_port = htons(7777);//wpisanie do zmiennej strukturalnej numeru portu

    printf("Probuje dowiazac adres IP do gniazda: ");
    if(bind(sock_r, (struct sockaddr*)&serwer, sockaddr_len) < 0)//dowiazanie ustawionych wyzej parametrow do gniazda sieciowego
    {
        printf("problem z dowiazaniem gniazda do adresu\n");
        close(sock_r);
        return 0;
    }
    printf("adres dowiazany\n");
    while(1)//rozpoczecie glownej petli programu
    {
        int bufor_poz = 0;//inicjalizacja zmiennej przechowujacej pozycje/rozmiar bufora
        printf("Uruchamiam nasluch...\n");
        rozmiar_danych = recvfrom(sock_r, bufor, 65536, 0, (struct sockaddr*)&serwer, (socklen_t*)&sockaddr_len);//oczekiwanie na ramke (funckja recvfrom w przypadku bledu odbioru zwraca -1)
        if (rozmiar_danych < 0)
        {
            printf("Blad podczas odbierania danych: blad lub timeout\n");
        }

        printf("Odebralem ramke protokolu PWP: \n");
        // Ponizsza procedura wyswietla pojedyncze bajty, przydatne podczas debugowania
        // [MOZNA USUNAC]
        // int j = 0; 
        // for (int i = 0; i <1000 ; i++) 
        // {
        //     printf("%.2x ", bufor[i]);
        //     j++;
        //     if (0 == j%20)
        //     { // wypisywanie po 20 oktetów
        //         printf ("\n");
        //     }
        // }
        printf("\n");
        printf("Naglowki przenoszonych protokolow: ");
        struct iphead iphddr;//zadeklarowanie struktury naglowka ip
        memcpy(&iphddr, bufor, sizeof(iphddr));//skopiowanie danych z bufora do struktruy naglowka IP
        bufor_poz = bufor_poz + sizeof(iphddr);//przesuniecie pozycji/zwiekszenie rozmiaru bufora o rozmiar naglowka ip
        printf("\nNaglowek IP wyglada tak: \n");
        PrintIpHddr(&iphddr, sizeof(iphddr));//wyswietl odebrany naglowek ip
        klient.sin_addr.s_addr = iphddr.zrodlo;//wpisz do struktury adresu ip klienta dane z odebranego naglowka (naglowek IP przenosi informacje takie jak adres zrodlowy i adres docelowy - tutaj z tego kozystamy)
        klient.sin_family = AF_INET;//ustaw rodzine protokolow ( przypominam AF_INET oznacza protokoly IP)
        klient.sin_port = htons(7777);//ustaw numer portu
        struct pwphead naglowek;//deklaracja struktury naglowka protokolu PWP
        memcpy(&naglowek, bufor+bufor_poz, sizeof(naglowek));//skopiuj dane na odopowiedniej pozycji z bufora do struktury naglowka)
        bufor_poz = bufor_poz+sizeof(naglowek);//przesun pozycje bufora/zwieksz rozmiar bufora o rozmiar struktury naglowka
        printf("Naglowek PWP wyglada tak:\n");
        PrintPwpHddr(&naglowek, sizeof(naglowek));//wyswietl odebrany naglowek PWP
        //PROCEDURA OBSLUGI OPCJI PROTOKOLU PWP
        if (naglowek.opcje.optKod == 1) //jesli dane w opcjach obecne wypisz na standardowym wyjsciu komunikat ze opcje obecne
        {
            printf("|-PWP Dane w opcjach:\n");
        }
        else//jesli nie ma danych w opcjach podpisz jako zwykle dane
        {
            printf("\nDane: \n");
        }
        Element_listy *lista = (Element_listy*) malloc(sizeof(Element_listy));//inicjalizacja wskaznika na liste wiazana
        switch(naglowek.typDanych) //stary dobry switch do sterowania programem
        {
            //jesli klient wyslal tylko adres MAC
            case TYP1:
                printf("Przeslano tylko adres MAC\n");
                PrintMacAddr(bufor, &bufor_poz);//wyswietl adres MAc z bufora
                //Uzupelnij bufor pamieci prawidlowymi danymi
                //zerowanie naglowka PWP
                memset(&naglowek, 0, sizeof(naglowek));//zerowanie struktury naglowka PWP
                naglowek.kodOper = KOD7;//wpisanie w naglowek flagi oznaczajacej poprawny odbior ramki
                naglowek.typDanych = TYP4;//wpisz flage oznaczajaca typ operacji jako - potwierdzenie odbioru
                naglowek.rozmDanych = 0;//danych nie przesylamy - tylko naglowek
                naglowek.wersja = 1;//zawsze 1 - wedlug instrukcji
                naglowek.opcje.optKod = 0;//flaga obecnosci opcji (teraz opcje nam nie potrzebne)
                naglowek.headDlug = 6;//domyslnie dlugosc to 24 bajty 
                WyslijZwrot(&naglowek, bufor, &sock_r);//wywolaj funkcje do wyslania potwierdzenia odebrania danych do klienta 
                break;
            //jesli klient wyslal adres IP
            //podobnie jak po stronie klienta tych samych procedur nie bede ponownie opisywal, roznica jest tylko w wywolywanych funkcjach wyswietlajacych kolejne typy danych
            case TYP2:
                printf("Przeslano tylko adres IP\n");
                PrintIpAddr(bufor, &bufor_poz);//wywolanie funkcji wyswietlajacej naglowek ip
                memset(&naglowek, 0, sizeof(naglowek));
                naglowek.kodOper = KOD7;
                naglowek.typDanych = TYP4;
                naglowek.rozmDanych = 0;
                naglowek.wersja = 1;
                naglowek.opcje.optKod = 0;
                naglowek.headDlug = 6;
                WyslijZwrot(&naglowek, bufor, &sock_r);
                break;
            //jesli klient wyslal tylko 10 liczb
            case TYP3:
                printf("Przeslano tylko liczby\n");
                LiczbyDoListy(bufor, &bufor_poz, lista);//wpisz liczby do listy wiazanej (rowniez wyswietla te liczby)
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
            //jesli klient wyslal komplet danych (NIE MA TYP4 bo to jest potwierdzenie - klient nie wysyla potwierdzenia, a przynajmniej nie mial wysylac wedlug instrukcji)
                printf("Przeslano komplet danych\n");
                PrintMacAddr(bufor, &bufor_poz);//wyswietl adres mac
                PrintIpAddr(bufor, &bufor_poz);//wyswietl adres IP
                LiczbyDoListy(bufor, &bufor_poz, lista);//wpisz liczby do listy wiazanej ( rowniez wystwietla te liczby )
                memset(&naglowek, 0, sizeof(naglowek));
                naglowek.kodOper = KOD6;
                naglowek.typDanych = TYP4;
                naglowek.rozmDanych = 0;
                naglowek.wersja = 1;
                naglowek.opcje.optKod = 0;
                naglowek.headDlug = 6;
                WyslijZwrot(&naglowek, bufor, &sock_r);
                break;
            //jesli przeslano jakies bzdury lub pojawily sie jakies bledy program wyswietli komunikat o nieprawidlowej operacji
            default:
                printf("Nieprawidlowa operacja\n");
                break;
        }
        memset(bufor, 0, 65536);//wyzeruj bufor pamieci
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
	printf(" |-IP dlugosc naglowka : %d bajtow\n", ((unsigned int) (iph->dlugosc)) * 4);//wartosc przenoszona w naglowku mnozymy razy 4 bajty ( czyli w naglowku jest przenoszona informacja ile razy wystapia 32 bity w naglowku)
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
//funkcja do wyswietlania naglowka PWP
void PrintPwpHddr(struct pwphead* data, int size)
{
    printf("\n");
    printf("PWP Header\n");
    printf("|-PWP wersja: %d\n", data->wersja);
    printf("|-PWP kod operacji: %d\n", data->kodOper);
    printf("|-PWP typ danych: %d\n", data->typDanych);
    printf("|-PWP dlugosc nagl: %d\n", ((unsigned int)(data->headDlug))*4);//przyjalem identycznie jak w naglowku IP ( po wiecej informacji odnosnie rozmiaru odsylam do struktury.h)
    printf("|-PWP rozmiar danych: %d\n", data->rozmDanych);
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
        printf("%d \n", *element->liczba);//wyswietl liczbe zawarta w danym elemencie listy wiazanej
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
//funkcja wyswietlajaca adres MAC
void PrintMacAddr(unsigned char * buff, int *poz)
{
    unsigned char *mac_addr = (unsigned char*)malloc(sizeof(unsigned char)*6);//zaalokuj pamiec dla 6 bajtow (tyle wlasnie ma adres MAC)
    memcpy(mac_addr, buff+*poz, 6);//skopiuj dane z bufora pod adres pamieci naszej zmiennej *mac_addr
    *poz = *poz + 6;//przesun pozycje bufora o 6 bajtow dalej
    //display mac address
    printf("Mac: %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n" , mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);//wyswietl poszczegolne bajty tak jak wyswietla sie adres MAC
}

//funkcja wyswietlania adresu ip
void PrintIpAddr(unsigned char *buff, int *poz)
{
    struct in_addr ipAdres;//struktura adresu ip - tutaj troszke inna niz wczesniej uzywalismy dlatego, ze w danych przesylamy TYLKO adres IP a nie caly naglowek
    memcpy(&ipAdres, buff+*poz, sizeof(ipAdres));//skopiuj dane z bufora pod adres wskazujacy na poczatek naszego adresu ip
    printf("Adres IP gniazda klienta %s\n", inet_ntoa(ipAdres));
    *poz = *poz + sizeof(ipAdres);//przesun pozycje bufora o rozmiar naglowka ip
}
//funkcja wpisujaca kolejne liczby z wektora liczb do listy wiazanej (wektor liczb jest przesylany w formie 10 int'ow w buforze)
void LiczbyDoListy(unsigned char* buff, int *poz, Element_listy *lista)
{
    //inicjalizacja listy
    lista->nastepny = NULL;
    lista->poprzedni = NULL;
    lista->pierwszy = &lista;
    lista->liczba = (int*)malloc(sizeof(int));
    memcpy(lista->liczba, buff+*poz, sizeof(int));//skopiuj pierwsza liczbe z bufora do listy
    *poz = *poz+sizeof(int);//zwieksz wskaznik pozycji bufora
    for(int i = 0; i < 9; i++)
    {
        wstaw(&lista);//wstaw nowy, PUSTY element listy
        memcpy(lista->liczba, buff+*poz, sizeof(int));// do nowego elementu skopiuj kolejna liczbe z bufora
        *poz = *poz+sizeof(int);//przesun pozycje odczytu bufora o rozmiar int'a
    }
    print(lista);//wywolaj funkcje wyswietlania calej listy
}
//Wysyla wiadomosc zwrotna z serwera
//Wymaga wskaznika do naglowka oraz wskaznika do bufora
//deskryptor gniazda jest globalny, wiec korzystamy bezposrednio
void WyslijZwrot(struct pwphead *hddr, unsigned char* buff, int* sock_descr)
{
    memset(buff, 0, 65536);//wyzeruj bufor pamieci
    memcpy(buff, hddr, sizeof(*hddr));//skopiuj strukture naglowka PWP do bufora pamieci
    int sizeOfBuff = sizeof(*hddr);//ustaw rozmiar bufora ( jak sma naglowek PWP to wiadomo ze 24 bajty)
    int *psockAddrLen = &sockaddr_len;//tak trzeba i tyle w temacie XD (funkcja sendto i recvfrom sa bardzo kaprysne pod tym wzgledem)
    int rozmiar_danych = sendto(*sock_descr, buff, sizeOfBuff, 0, (struct sockaddr*)&klient, *psockAddrLen);//wyslij bufor przez gniazdo sieciowe
    if(rozmiar_danych < 0)
    {
        printf("Nie udalo sie wyslac wiadomosci\n");
        printf("Mozliwe zablokowanie gniazda lub, blad konfiguracji\n");
    }
}
