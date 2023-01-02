#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "struktury.h"

int main (void)
{
    unsigned char * pointer;
    int rozmiar = sizeof(pointer);
    printf("Rozmiar wskaznika jest rowny: %d\n", rozmiar);
    pointer = (unsigned char*)malloc(65536);
    rozmiar = sizeof(pointer);
    printf("Rozmiar wskaznika po alokacji pamieci rowny: %d\n", rozmiar);
    struct pwphead naglowek;
    rozmiar = sizeof(naglowek);
    printf("Rozmiar naglowka pwp jest rowny: %d\n", rozmiar);
    rozmiar = sizeof(naglowek.opcje);
    printf("Rozmiar struktury opcji w naglowku jest rowny: %d\n", rozmiar);
}