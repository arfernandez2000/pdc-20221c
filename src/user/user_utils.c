#include "user_utils.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void init_user_list( void ){
	list = calloc(1, sizeof(user_list));
}

static User * add_user_rec(User * first, char* username, uint8_t ulen, char* password, uint8_t upass, bool admin, bool * added) {
    if(first == NULL){
        User * aux = malloc(sizeof(User));
        if(aux == NULL){
            perror("Fallo malloc de aux user");
            return first;
        }
        first->next = first;
        strcpy(first->username, username);
        strcpy(first->password, password);
        first->ulen = ulen;
        first->upass = upass;
        *added = true;
        return aux;
    } else if (strcmp(first->username, username) == 0) {
        return first;
    } else {
        first->next = add_user_rec(first->next, username, ulen, password, upass, admin, added);
    }
    return first;
}

bool add_user(user_list * list, char* username, uint8_t ulen, char* password, uint8_t upass, bool admin){
    bool added = false;
    list->first = add_user_rec(list->first, username, ulen, password, upass, admin, &added);
    if(added)
        list->size++;
    return added;
}


bool delete_user(user_list * list, char* username, uint8_t ulen){
   User * prev = list->first;
   User * curr = list->first;
	int c=-1;
     // Avanzamos hasta encontrar el elemento a borrar o detectar que no esta
	while( curr != NULL && c < 0) {
		c = strcmp(curr->username, username);
		if ( c < 0 ) {
			prev = curr;
			curr = curr->next;
		}
	}
	// curr apunta al elemento encontrado, al primero que es mayor a element o a NULL
	if (c!=0) {
		return false;	// no estaba
	}
	User * aux = curr->next;
	free(curr);
	if ( curr == list->first)
		list->first = aux;
	else
		prev->next= aux;
    
	list->size--;
	return true;
}


bool edit_user(user_list * list, char* username, uint8_t ulen, char* password, uint8_t upass){
    User * aux = list->first;
    while (aux->next != NULL || (strcmp(aux->username, username) != 0)){
        aux = aux->next;
    }
    if(strcmp(aux->username, username) == 0){
        aux->password = password;
        aux->upass = upass;
        return true;
    }
    return false;
}

char * get_all_users(user_list * list){
    int init = 255;
    char * all_users = malloc(255);
    char * aux;
    if(all_users == NULL){
        return all_users;
    }
    User *user = list->first;
    int cont = 0;
    for (int k = 0; k < list->size; ++k){
        all_users[cont++] = user->ulen;
        strcpy(all_users+cont,user->username);
        cont += user->ulen;
        if(cont >= init){
            init *= 2;
            if ((aux = realloc(all_users,init)) == NULL){
                free(all_users);
                return NULL;
            }
            all_users = aux;
        }
    }
    return all_users;
}


// char * get_all_users(user_list * list){
//     //0x02 0x67 0xff
//     User * user = list->first;
//     int init = 255;
//     char * user_string = malloc(255);
//     char * aux;
//     int k = 0;
//     for(int i=0; i < list->size; i++){
//         for (int j = 0; j < user->ulen; j++) {
//             if(k >= init) {
//                 init *= 2;
//                 if((aux = realloc(user_string, init)) == NULL){
//                     free(user_string);
//                     return NULL;
//                 }
//                 user_string = aux;
//             }
//             user_string[k++] = user->username[j];
//         }
//         if(k >= init) {
//             init *= 2;
//             if((aux = realloc(user_string, init)) == NULL){
//                 free(user_string);
//                 return NULL;                
//             }
//             user_string = aux;
//         }
//         user_string[k++] = ',';
//         user = user->next;
//     }
//     user_string[k--] = '\0';
//     return user_string;
// }

