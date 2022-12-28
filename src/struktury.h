#ifndef STRUKTURY_H
#define STRUKTURY_H

//definicja struktury naglowka ip
struct iphead
{
	unsigned int dlugosc:4;
	unsigned int wersja:4;
	unsigned char uslug_ecn;
	unsigned short calk_dlug;
	unsigned short identy;
	unsigned short frag_flagi;
	unsigned char czas_zy;
	unsigned char protok;
	unsigned short suma_kontr;
	unsigned int zrodlo;
	unsigned int destyn;
};
//deklaracja struktury naglowka ethernet
struct ethhead
{
	unsigned char adresat[6];
	unsigned char zrodlo[6];
	unsigned short typ;
};
//deklaracja naglowka struktury tcp
struct tcphead
{
	unsigned short zrodlo;
	unsigned short adresat;
	unsigned int nr_sekw;
	unsigned int ack;
	unsigned short rozm_okn;
	unsigned short suma_kontr;
	unsigned short wsk_piln;
};
//deklaracja struktury naglowka udp
struct udphead
{
	unsigned short zrodlo;
	unsigned short adresat;
	unsigned short dlugosc;
	unsigned short suma_kontr;
};
//deklaracja struktury naglowka ipv6
struct ipv6head
{
	unsigned char wersja;
	unsigned char klasaRuchu:4;
	unsigned int etykieta_przep:20;
	unsigned short rozmiar_danych;
	unsigned char nastep_nagl;
	unsigned char limit_przesk;
	unsigned short zrodlo[8];
	unsigned short adresat[8];
};
//deklaracja struktury naglowka icmphead
struct icmphead
{
	unsigned char typ;
	unsigned char kod;
	unsigned short suma_kontr;
};

struct pwpopcje
{
    unsigned int optKod:4;
    unsigned int optDlug:4;
    unsigned int *optDane;
};

struct pwphead
{
    unsigned int wersja:4;
    unsigned int kodOper:4;
    unsigned int typDanych:4;
    unsigned int headDlug:4;
    unsigned short rozmDanych;
    struct pwpopcje opcje;
};
#define KOD1 1 //Brak operacji
#define KOD2 2 //Dodawanie
#define KOD3 3 //Odejmowanie
#define KOD4 4 //Mnozenie
#define KOD5 5 //Dzielenie
#define KOD7 7 //Poprawny odbior

#define TYP1 1 //Tylko adres MAC
#define TYP2 2 //Tylko adres IP
#define TYP3 3 //Tylko liczby

#endif 