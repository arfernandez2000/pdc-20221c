#ifndef LISTADT_H_
#define LISTADT_H_

#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>

/*
 * lista generica, se puede guardar cualquier tipo de elem y de cualquier tamaño
 * los elentos se crean con un malloc y se le pasa a insert el puntero al elem.
 * para poder hacer un free de los elemnto se debe pasar la funcion freeElem al crar los la
 * lista.
 * */


typedef struct listCDT* listADT;

/*
static int compare(elemType elem1, elemType elem2 ){
	return elem1 - elem2;		
}
*/


listADT newList(void (*elemFree)(void*));

//devulve la el tamano de listCDT
int getListStructSize();

//simpre inserta al pricipio de la lista
int insert(listADT list, void* elem);

//retorna 1 si lo elimino y 0 si no lo encontro
//se compara el elm2 con elem1, cmp devulve 0 si eson iguales, 1 sino (como strcmp)
//int delete(listADT list, void* elem2, int (*cmp)(void* elem1, void* elem2));

int listIsEmpty(listADT list);

//devuelve 1 si lo encontro sino 0
int elemBelongs(listADT list, void* elem, int (*cmp)(void*, void*));

void* getElem(listADT list, void* value, int(*cmp)(void*, void*));

void freeList(listADT list);

int listSize(const listADT list);

/**
 * Funciones para poder iterar sobre la lista
 */
void listToBegin(listADT list);

int listHasNext(const listADT list);

void* listNext(listADT list);


// void* freeElem(void * elem){
// 	free(elem);
// }

/*
int main(int argc, char const *argv[])
{
	listADT lista = newList(freeElem);

	int newElem = malloc(sizeof(int));
	insert(lista, newElem);

	listToBegin(lista);
	while(listHasNext(lista)){
		int elem = *((int*) listNext(lista));
	}

	return 0;
}
*/

#endif 
