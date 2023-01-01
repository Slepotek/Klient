#include <arpa/inet.h> //htonl, htons, ntohl, nthos, inet_addr etc.
#include <stdio.h> //printf, etc.
#include <stdlib.h> //malloc, realloc, free, exit etc.
#include <string.h> //memcpy, memset, strcpy, etc.
#include <sys/socket.h> //socklen_t, sockaddr, msghdr, etc.
#include "struktury.h"
#include <unistd.h>//socket closing
#include <linux/if.h>// this one supposignly gets the mac address TODO: compleate the reference
#include <sys/ioctl.h>// this one contains macro SIOCGIFHWADDR that does TODO: compleate what this macro does

#define NUMER_STUDENTA 1 //numer studenta ( nie wiem o ktory konkretnie numer chodzi ale chyba sobie rozkminicie )

void operacja1(unsigned char *buff, int *sock_descr);//deklaracja operacji pierwszej (przeslanie w polu danych MAC, IP i 10 liczb)
void operacja2(unsigned char *buff, int *sock_descr);//deklaracja operacji drugiej (przeslanie w polu opcji naglowka PWP MAC, IP i 10 liczb)
void operacja3(unsigned char *buff, int *sock_descr);//deklaracja operacji trzeciej (przeslanie w polu danych tylko adresu MAC)
void operacja4(unsigned char *buff, int *sock_descr);//deklaracja operacji czwartej (przeslanie w polu danych tylko adresu IP)
void operacja5(unsigned char *buff, int *sock_descr);//deklaracja operacji piatej (przeslanie w polu danych tylko 10 liczb)
void Odbierz(int *sock_descr, unsigned char *bufor);//deklaracja funkcji do odbierania danych z gniazda do bufora
void Wyslij(int *sock_descr, unsigned char *bufor);//deklaracja funkcji do wysylania danych do gniazda z bufora
void PobierzAdresMac (int *sock_descr, unsigned char* mac_addr);//deklaracja funkcji do pobrania adresu mac interfejsu internetowego na ktorym pracuje aplikacja klienta
struct sockaddr_in klient, serwer;//deklaracja struktury do przechowywania adresow gniazd sieciowych serwera oraz klienta
int sockaddr_len = 0;//zmienna do przechowywania rozmiaru struktury adresu (sockaddr_in) klienta i serwera (domyslnie to 16 bajtow)
int buffer_len = 0;//zmienna do przechowywania dlugosci bufora danych (rowniez sluzy do pozycjonowania odczytu)

int main (void)
{
    int sock_r = 0;//deskryptor gniazda
    sockaddr_len = sizeof(klient);//pobranie rozmiaru struktury adresu klienta
    printf("Otwieram gniazdo Klienta:\n");
    sock_r = socket(AF_INET, SOCK_RAW, 144+NUMER_STUDENTA);//otwarcie gniazda
    //UWAGA uzycie flagi AF_INET inaczej PF_INET pozwala nam nie zajmowac sie warstwa porotokolu IP, caly naglowek IP jest uzupelniany poza nasza kontrola, automatycznie przez wewnetrzne funkcje biblioteki socket.h
    if (sock_r == -1)
    {
        printf("Blad podczas otwierania gniazda klienta, przerywam program.\n");
        printf("Sprawdz czy program uruchomiono jako root.\n");
        return 0;
    }
    klient.sin_addr.s_addr = inet_addr("127.0.0.12");//zdefiniowanie adresu IP gniazda sieciowego klienta
    klient.sin_family = AF_INET;//zdefiniowanie rodziny protokolow
    klient.sin_port = htons(7777);//zdefiniowanie numeru portu

    if(bind(sock_r, (struct sockaddr*)&klient, sizeof(klient)) < 0)//przypisanie ustawionego wyzej adresu do gniazda
    {
        printf("Problem z dowiazaniem gniazda do adresu\n");
    }

    serwer.sin_family = AF_INET;//zdefiniowanie rodziny protokolu dla strony serwerea
    serwer.sin_port = htons(7777);//zdefiniowanie numeru portu
    serwer.sin_addr.s_addr = inet_addr("127.0.0.13");//zdefiniowanie adresu ip gniazda po stronie serwera
    printf("Wybierz typ operacji jaka chcesz wykonac: \n");
    printf("    |- 1 -> Przygotowac i przeslac w polu danych adres MAC, IP oraz 10 cyfr\n");
    printf("    |- 2 -> Przygotowac i przeslac w polu opcji adres MAC, IP oraz 10 cyfr\n");
    printf("    |- 3 -> Przygotowac i przeslac w polu danych tylko adres MAC\n");
    printf("    |- 4 -> Przygotowac i przeslac w polu danych tylko adres IP\n");
    printf("    |- 5 -> Przygotowac i przeslac w polu danych tylko 10 cyfr\n");
    int wyb = 0;//zmienna wyboru operacji
    scanf("%d", &wyb);//pobranie wyboru uzytkownika
    printf("Wybrales %d\n", wyb);

    unsigned char* buff = (unsigned char*)malloc(65536);//zadeklarowanie i zdefiniowanie buforu pamieci do przechowywania ramek (65536 to maksymalny rozmiar pakietu IP)

    switch (wyb) //stardy dobry switch do sterowania przypadkami uzycia (wybor odpowiedniej procedury do pozodanej operacji)
    {
        struct pwphead naglowek;//struktura naglowka wlasnego protokolu (wiecej info w pliku struktury.h)
        case 1:
            operacja1(buff, &sock_r);//wywolanie funkcji realizujacej procedure operacji pierwszej
            Wyslij(&sock_r, buff);//wywolanie funkcji do wyslania danych z bufora
            do//petla czekajaca na potwierdzenie poprawnego odbioru danych po stronie serwera
            {
                struct iphead ipHead;//zadeklarowanie struktury naglowka IP
                int poz = 0;//zadeklarowanie zmiennej pozycji 
                Odbierz(&sock_r, buff);//wywolanie procedury odbioru ramki z informacja zwrotna od serwera (program przejdzie dalej dopiero gdy otrzyma jaka kolwiek wiadomosc ze strony serwera - moze to byc nawet bledna wiadomosc)
                memcpy(&ipHead, buff, sizeof(ipHead));//skopiuj do struktury adresu ip - adres ip ktory zostal wyslany przez serwer(robimy to glownie po to zeby nie bawic sie w liczenie bitow)
                poz = poz + sizeof(ipHead);//zwieksz zmienna pozycji odczytu o rozmiar naglowka ip
                memcpy(&naglowek, buff+poz, sizeof(naglowek));//skopiuj do struktury naglowka PWP - naglowek PWP przeslany przez serwer
                poz = poz + sizeof(naglowek);//przesun zmienna odczytu o rozmiar naglowka pwp
            } while (naglowek.typDanych != TYP4);//sprawdz czy w naglowku pwp przeslano potwierdzenie odebrania danych prawidlowo (ustawienie wartosci TYP4, w polu "typDanych" to flaga potwierdzenia)  
            printf("Wiadomosc przeslano i odebrano przez serwer.\n");
            break;
        case 2:
            //tu jest tam sama procedura co powyzej, zmienia sie tylko funkcja operacji dlatego nie bede opisywal
            operacja2(buff, &sock_r);//numer operacji sie zmienil
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
        case 3:
            //jak wyzej
            operacja3(buff, &sock_r);
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
        case 4:
            //jak wyzej
            operacja4(buff, &sock_r);
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
        case 5:
            //jak wyzej
            operacja5(buff, &sock_r);
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
        default:
        //domyslnie gdy uzytkownik wpisze podczas wyboru operacji jakies bzdury lub numer ktorego program nie obsluzy - program sie zakonczy
            printf("Zamykam aplikacje\n");
            return EXIT_SUCCESS;
            break;
    }
}

void operacja1 (unsigned char * buff, int *sock_descr) //funkcja do skonstruowania bufora z odpowiednim ukladem danych do przeslania
{
    memset(buff, 0, sizeof(*buff));//wyzerowanie bufora
    //budujemy naglowek wlasnego protokolu
    struct pwphead naglowek;
    naglowek.headDlug = 6; //domyslnie dlugosc naglowka to 24 bajty
    naglowek.kodOper = KOD1;//kod 1 mial oznaczac brak operacji wedlug instrukcji, tu operacji na danyhc zadnej nie wykonujemy tylko przesylamy (tak bylo na ocene 3)
    naglowek.rozmDanych = sizeof(struct in_addr)+(sizeof(int)*10)+6; //rozmiar danych zawierajacych adres IP, 10 liczb (typ int) oraz adres MAC (6 bajtow)
    naglowek.typDanych = TYP5;//typ danych nr 5 oznacza ze przesylamy wszystkie przewidziane dane czyli adres MAC, IP i 10 liczb
    naglowek.wersja = 1;//wersja protokolu - miala byc zawsze 1
    naglowek.opcje.optKod = 0;//kod opcji - w tym polu przesylam flage wskazujaca czy w opcjach sa jakies dane czy przesylamy naglowek bez opcji (0 - oznacza bez opcji)

    unsigned char *mac_addr = (unsigned char*)malloc(sizeof(unsigned char)*6);//zadeklaruj i zaalokuj pamiec dla adresu MAC
    PobierzAdresMac(sock_descr, mac_addr);//wywolaj funkcje do pobrania adresu mac stacji

    //wpisz do struktury adresu ip adres ip klienta (tak lepiej jest go przeslac)
    struct sockaddr_in ipAddres;//struktura adresu ip 
    int ipAddres_len = sizeof(struct sockaddr);//dlugosc struktury adresu ip (jest tu taka mala )
    getsockname(*sock_descr, (struct sockaddr*) &ipAddres, (socklen_t*) &ipAddres_len);//funkcja do pobrania adresu IP gniazda o deskryptorze sock_descr
    printf("Adres IP gniazda klienta :%s\n", inet_ntoa(ipAddres.sin_addr));

    //pobierz 10 liczb od uzytkownika
    int *wektor_liczb = malloc(sizeof(int)*10);//dynamiczna tablica do przechowywania wektora liczb podanych przez uzytkownika
    printf("Podaj 10 liczb: \n");
    for(int i = 0; i < 10; i++)//petla pobierajaca wartosci
    {
        scanf("%d", &wektor_liczb[i]);
    }

    //Budowanie bufora z danymi
    memcpy(buff+buffer_len, &naglowek, sizeof(naglowek));//najpierw kopiujemy naglowek PWP
    buffer_len =buffer_len+sizeof(naglowek);//zwiekszamy rozmiar bufora o rozmiar naglowka
    memcpy(buff+buffer_len, mac_addr, 6);//teraz kopiujemy adres MAC
    buffer_len =buffer_len+6;//zwiekszamy rozmiar bufora o rozmiar adresu MAC
    memcpy(buff+buffer_len, &ipAddres.sin_addr, sizeof(struct in_addr));//teraz kopiujemy adres IP
    buffer_len =buffer_len+sizeof(struct in_addr);//zwiekszamy rozmiar bufora o rozmiar adresu ip
    memcpy(buff+buffer_len, wektor_liczb, sizeof(int)*10);//kopiujemy do bufora wektor 10 liczb 
    buffer_len = buffer_len+(sizeof(int)*10);//zwiekszamy rozmiar bufora o rozmiar wekotra 10 liczb
}

void Odbierz(int * sock_descr, unsigned char * bufor)//funkcja odbierania danych z gniazda
{
    int rec = recvfrom(*sock_descr, bufor, 65536, 0, (struct sockaddr*)&serwer, (socklen_t*)&sockaddr_len); //rec to zmienna w ktorej przechowany bedzie rozmiar odebranych danych - jak byl jakis blad to jej wartosc bedzie -1
    //funckja recvfrom sluzy do odebrania danych z gniazda
    if (rec < 0)
    {
        printf("Blad podczas odbierania danych\n");
    }
    memcpy(&buffer_len, &rec, sizeof(rec));//kopiujemy odebrane dane do buofra
}

void Wyslij(int *sock_descr, unsigned char *bufor)//funkcja wysylajaca dane do gniazda ( i dalej do gniazda odbierajacego)
{
    int rec = sendto(*sock_descr, bufor, buffer_len, 0, (struct sockaddr*)&serwer, sockaddr_len);// funkcja bardzo podobna do tej wyzej, dziala tak samo jak -1 to znaczy ze blad w wysylaniu ( jest tutaj tez maly myk z rozmiarami, ale zostawie to dla dociekliwych ;-))
    if (rec < 0)
    {
        printf("Blad podczas wysylania\n");
    }
    memset(bufor, 0, 65536);//wyzerowanie bufora danych - przygotowanie go do nastepnych operacji
    buffer_len = 0;//wyzerowanie rozmiaru bufora danych (przesuniecie pozycji odczytu na 0)
}

void PobierzAdresMac (int *sock_descr, unsigned char* mac_addrr)
{
    /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      + Procedura wyluskania adresu MAC przypisanego do gniazda +
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    struct ifreq s; //struct for socket ioctl's, it can contain among others the MAC address of the interface attached to the socket
    char *if_name = "enp0s3"; //tak dziala. W przypadku zwyklego srodowiska (jakim jest zwykly komputer PC lub wirtualna maszyna) wystarczy dowolny interfejs
    int if_name_len = IFNAMSIZ;

    memset(&s, 0, sizeof(struct ifreq));

    s.ifr_addr.sa_family = AF_INET;
    strncpy(s.ifr_name, if_name, IFNAMSIZ-1);

    if(0 == ioctl(*sock_descr, SIOCGIFHWADDR, &s))
    {
        memcpy(mac_addrr,(unsigned char*)s.ifr_hwaddr.sa_data, sizeof(unsigned char)*6);
        //display mac address
        printf("Mac : %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n" , mac_addrr[0], mac_addrr[1], mac_addrr[2], mac_addrr[3], mac_addrr[4], mac_addrr[5]);
    }
    /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      +           Koniec procedury wyluskania adresu MAC                +
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
}

void operacja2(unsigned char* buff, int * sock_descr)//przeslac w opcjach adres mac, IP i 10 liczb
{
    //druga operacja jest bardzo podobna do pierwszej ale ze wzgledu na przesylanie danych w opcjach troszeczke sie rozni
    memset(buff, 0, sizeof(*buff));
    //budujemy naglowek wlasnego protokolu tak jak poprzednio
    struct pwphead naglowek;
    naglowek.headDlug = 6; // domyslnie dlugosc naglowka to 24 bajty
    naglowek.kodOper = KOD1;
    naglowek.rozmDanych = 0; //nie przesylamy w polu danych nic, dane przesylamy w tym przypadku w polu opcji
    naglowek.typDanych = TYP5;
    naglowek.wersja = 1;
    naglowek.opcje.optKod = 1;//ustawiamy flage opcji na 1, po stronie serwera ta zmienna jest odczytywana i uruchamiany jest odpowiedni kod wskaujacy ze dane sa w opcjach
    naglowek.opcje.optDlug = 0;//wstepnie dlugosc opcji ustawiamy na 0 ( taka inicjalizacja - pozniej zmienimy na faktyczny rozmiar)
    naglowek.opcje.optDane = (unsigned char*)malloc(sizeof(struct in_addr)+(sizeof(int)*10)+6);//rozmiar danych jest taki jak wczesniej rozmiar adresu MAC + IP + 10 liczb

    unsigned char *mac_addr = (unsigned char*)malloc(sizeof(unsigned char)*6);//zaalokowanie pamieci dla adresu MAC
    PobierzAdresMac(sock_descr, mac_addr);//pobranie adresu MAC

    //wpisz do struktury adresu ip adres ip klienta (tak lepiej jest go przeslac)
    struct sockaddr_in ipAddres;//struktura adresu IP
    int ipAddres_len = sizeof(struct sockaddr);//dlugosc adresu (ten myk z konwersja sockaddr_in na sockaddr zostawiam dociekliwym, dla tych mniej dociekliwych tak musi byc :-D)
    getsockname(*sock_descr, (struct sockaddr*) &ipAddres, (socklen_t*) &ipAddres_len);//pobieramy adres ip gniazda
    printf("Adres IP gniazda klienta :%s\n", inet_ntoa(ipAddres.sin_addr));

    //pobierz 10 liczb od uzytkownika tak jak i w przypadku operacji 1
    int *wektor_liczb = malloc(sizeof(int)*10);
    printf("Podaj 10 liczb: \n");
    for(int i = 0; i < 10; i++)
    {
        scanf("%d", &wektor_liczb[i]);
    }

    //Budowanie bufora z danymi i tu sa zmiany wzgledem operacji 1
    memcpy(naglowek.opcje.optDane, mac_addr, 6);//najpierw pod adres wskazujacy na dane opcji przesylamy bity adresu MAC
    buffer_len =buffer_len+6;//zwiekszamy pozycje w buforze o rozmiar adresu MAC
    memcpy(naglowek.opcje.optDane+buffer_len, &ipAddres.sin_addr, sizeof(struct in_addr)); //teraz leci adres IP
    buffer_len =buffer_len+sizeof(struct in_addr);// i pozycje przesowamy o rozmiar adresu aip
    memcpy(naglowek.opcje.optDane+buffer_len, wektor_liczb, sizeof(int)*10);//teraz wektor 10 liczb
    buffer_len = buffer_len+(sizeof(int)*10);//i znow przesuwamy pozycje w buforze - tym samym trzymamy wartosc rzeczywistego rozmiaru bufora
    naglowek.opcje.optDlug = buffer_len;//teraz dopiero wpisujemy rzeczywisty rozmiar danych w polu opcji w pole przypisane do rozmiaru danych pola opcji :-P 
    memcpy(buff, &naglowek, sizeof(naglowek));//i kopiujemy po kolei do bufora, najpierw strukture naglowka protokolu PWP
    memcpy(buff+sizeof(naglowek), naglowek.opcje.optDane, buffer_len);// potem dane ktore sa w opcjach
    buffer_len = buffer_len + sizeof(naglowek);// no i zwiekszamy rozmiar bufora o rozmiar naglowka ( bo teraz w buforze sa i naglowek PWP i dane w opcjach, lacznie jest bodaj 74 bajty - albo cos okolo, nie liczylem :-P)
}

//*************************
//     UWAGA - dalej juz nie opisuje operacji sa one takie same jak operacja 1 zmienia sie tylko to co kopiujemy do bufora czyli np tylko adres MAC, albo tylko adres IP. Wszystkie zmienne sa opisane tak samo, takze mozna sobie przesledzic. 
//*************************

void operacja3 (unsigned char * buff, int *sock_descr)
{
    memset(buff, 0, sizeof(*buff));
    //budujemy naglowek wlasnego protokolu
    struct pwphead naglowek;
    naglowek.headDlug = 6; // domyslnie dlugosc naglowka to 24 bajty
    naglowek.kodOper = KOD1;
    naglowek.rozmDanych = 6;
    naglowek.typDanych = TYP1;
    naglowek.wersja = 1;
    naglowek.opcje.optKod = 0;

    unsigned char *mac_addr = (unsigned char*)malloc(sizeof(unsigned char)*6);
    PobierzAdresMac(sock_descr, mac_addr);

    //Budowanie bufora z danymi
    memcpy(buff+buffer_len, &naglowek, sizeof(naglowek));
    buffer_len =buffer_len+sizeof(naglowek);
    memcpy(buff+buffer_len, mac_addr, 6);
    buffer_len =buffer_len+6;
}

void operacja4 (unsigned char * buff, int *sock_descr)
{
    memset(buff, 0, sizeof(*buff));
    //budujemy naglowek wlasnego protokolu
    struct pwphead naglowek;
    naglowek.headDlug = 6; // domyslnie dlugosc naglowka to 4 bajty
    naglowek.kodOper = KOD1;
    naglowek.rozmDanych = sizeof(struct in_addr); //TODO: Uzupelnic tutaj
    naglowek.typDanych = TYP2;
    naglowek.wersja = 1;
    naglowek.opcje.optKod = 0;

    //wpisz do struktury adresu ip adres ip klienta (tak lepiej jest go przeslac)
    struct sockaddr_in ipAddres;
    int ipAddres_len = sizeof(struct sockaddr);
    getsockname(*sock_descr, (struct sockaddr*) &ipAddres, (socklen_t*) &ipAddres_len);
    printf("Adres IP gniazda klienta :%s\n", inet_ntoa(ipAddres.sin_addr));

    //Budowanie bufora z danymi
    memcpy(buff+buffer_len, &naglowek, sizeof(naglowek));
    buffer_len =buffer_len+sizeof(naglowek);
    memcpy(buff+buffer_len, &ipAddres.sin_addr, sizeof(struct in_addr));
    buffer_len =buffer_len+sizeof(struct in_addr);
}

void operacja5 (unsigned char * buff, int *sock_descr)
{
    memset(buff, 0, sizeof(*buff));
    //budujemy naglowek wlasnego protokolu
    struct pwphead naglowek;
    naglowek.headDlug = 6; // domyslnie dlugosc naglowka to 4 bajty
    naglowek.kodOper = KOD1;
    naglowek.rozmDanych = (sizeof(int)*10); //TODO: Uzupelnic tutaj
    naglowek.typDanych = TYP3;
    naglowek.wersja = 1;
    naglowek.opcje.optKod = 0;

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
    memcpy(buff+buffer_len, wektor_liczb, sizeof(int)*10);
    buffer_len = buffer_len+(sizeof(int)*10);
}