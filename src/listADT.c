#include "../include/listADT.h"

typedef struct node {
	void* value;
	struct node* next;
} node;


typedef struct listCDT{
	node* first;
    node* iteradorNext; 
    void (*elemFree) (void*); //funcion que borra los elemntos
    unsigned int size;
}listCDT;


//private:
node* listSearch(node** current, void* elem, int (*cmp)(void*, void*));
int listContains(node* first, void* elem, int (*cmp)(void*, void*));

listADT newList(void (*elemFree)(void*)){

    listADT list = malloc(sizeof(listCDT));
    if(list == NULL){
        return NULL;
    }

    list->first = NULL;
    list->elemFree = elemFree;
    list->size = 0;
    list->iteradorNext = NULL;
    
    return list;
}

int getListStructSize(){
    return sizeof(listCDT);
}

//inserta los elemos al principio de la lista
//se tiene que crear con malloc el elem
int insert(listADT list, void* elem){

    //creo el nuevo node
    node* newNode = malloc(sizeof(node));
    if(newNode == NULL)
        return -1;

    newNode->next = list->first;

    newNode->value = elem;

    //lo inserto en la lista
    list->first = newNode;
    list->size++;

    return 0;
}

//retorna 1 si lo elimino y 0 si no lo encontro 
//se compara el elm2 con elem1, cmp devulve 0 si eson iguales, 1 sino (como strcmp)
// int delete(listADT list, void* elem2, int (*cmp)(void* elem1, void* elem2)){
    
//     node* current = list->first;
//     node* last = listSearch(&current, elem2, cmp);
//     if(current == NULL)
//         return 0;

//     if(last == NULL){
//         //se elimina el primero
//         list->first = current->next;
//     }else{
//         last->next = current->next;
//     }

//     list->size --;
//     list->elemFree(current->value);
//     free(current);
//     return 1;
// }

int listIsEmpty(listADT list){
	return list->size == 0;
}

int elemBelongs(listADT list, void* elem, int (*cmp)(void*, void*) ){
	return listContains(list->first, elem, cmp);
}

void freeList(listADT list){
    node* current = list->first;
    node* aux;
	while (current != NULL) {
		aux = current->next;
		list->elemFree(current->value);
        free(current);
		current = aux;
	}
    free(list);
}

int listSize(const listADT list) {
	return list->size;
}

//iterator
void listToBegin(listADT list) {
	list->iteradorNext = list->first;
}

int listHasNext(const listADT list) {
	return list->iteradorNext != NULL;
}

void* listNext(listADT list) {
	if (!listHasNext(list))
		return NULL;
	void* result = list->iteradorNext->value;
	list->iteradorNext = list->iteradorNext->next;

	return result;
}

//devuelve el elem, si es nulo no lo encontro
void* getElem(listADT list, void* value, int(*cmp)(void*, void*)){
    node* current = list->first;
    listSearch(&current, value, cmp);
    if(current == NULL)
        return NULL;
    return current->value;
}

//private:
//si current es null no lo encotro
//si last es null el elemnto buscado es el primer elemnto
node* listSearch(node** current, void* elem, int (*cmp)(void*, void*)) {
	node* last = NULL;
    node* auxCurrent = *current;
    while (auxCurrent != NULL){
        if(cmp(auxCurrent->value, elem) == 0){
            return last;
        }
        last = auxCurrent;
        auxCurrent = auxCurrent->next;
        *current =  auxCurrent;
    }
    return last;
}

//devuelve 1 si lo encontro sino 0
int listContains(node* first, void* elem, int (*cmp)(void*, void*)){
    listSearch(&first, elem, cmp);
    if(first == NULL)
        return 0;
    return 1;
}
