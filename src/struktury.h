#ifndef STRUKTURY_H
#define STRUKTURY_H

/*Mozna zmienic nazwy takie jak:
ALE UWAGA!!!! TRZEBA JE ZMIENIC TEZ W PLIKU Klient.c i Serwer.c

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

ALE TEZ TRZEBA PAMIETAC ZEBY ZMIENIC W PLIKACH Serwer.c i Klient.c
*/

//struktura listy wiazanej
typedef struct {
	int *liczba;
	struct Element_listy *poprzedni;
	struct Element_listy *nastepny;
	struct Element_listy *pierwszy;
}Element_listy;

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

//struktura opcji protokolu PWP (jak opcje nie sa przekazywane to struktura w naglowku i tak jest obecna)
struct pwpopcje //16 bajtow wraz z "dodatkowa pamiecia od kompilatora"
{
    unsigned int optKod:8;
    unsigned int optDlug;
    unsigned char *optDane;//8 bajtow (bo to wskaznik) ***lub 4 bajty na 32 bit maszynie
};

//struktura naglowka protokolu PWP
struct pwphead //8 bajtow wraz z "dodatkowa pamiecia od kompilatora"
{
    unsigned int wersja:4;
    unsigned int kodOper:4;
    unsigned int typDanych:4;
    unsigned int headDlug:4;
    unsigned short rozmDanych;
    struct pwpopcje opcje;
};
/*UWAGA UWAGA UWAGA - zapytacie czemy domyslnie naglowek ma 24 bajty kiedy ja naliczylem tylko 17 ????? Otoz dzieje sie tak ze wzgledu na to 
ze kompilator ktorego sie uzywa sobie roznie te zmienne uklada a generalnie probuje je przyrownac do najwiekszej zmiennej ze wszystkich dostepnych
czyli de-facto do rozmiaru wskaznika i tak np z dwoch 4 bitowych zmiennych zrobi 2 dwu bajtowe, a z jednobajtowej zrobi sobie 4 bajtowa. Jak sie wszystko
doda wychodzi 24. Mozna sobie o tym poczytac tutaj https://www.includehelp.com/c/size-of-struct-in-c-padding-alignment-in-struct.aspx

Dodatkowo specjalnie dla was zostawiam tutaj specjalny programik nazwany "testing123.c", dzieki ktoremu sprawdzicie sobie jak wygladaja rozmiary 
poszczegolnych zmiennych.*/

//Makra kodow operacji i typow danych
#define KOD1 1 //Brak operacji
#define KOD2 2 //Dodawanie
#define KOD3 3 //Odejmowanie
#define KOD4 4 //Mnozenie
#define KOD5 5 //Dzielenie
#define KOD6 6 //Wynik operacji 
#define KOD7 7 //Poprawny odbior

#define TYP1 1 /*Tylko adres MAC*/
#define TYP2 2 //Tylko adres IP
#define TYP3 3 //Tylko liczby
#define TYP4 4 //Potwierdzenie
#define TYP5 5 //MAC, IP, liczby


#endif 