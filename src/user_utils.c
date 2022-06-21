
#include "../include/user_utils.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static user_list * list = NULL;

void init_user_list( void ){
	list = calloc(1, sizeof(user_list));
    add_user("admin2", 7, "alggo2", 7, true);
    add_user("admin", 6, "alggo", 6, true);
    add_user("pruebita", 9, "pruebita", 9, true);
}

static User * add_user_rec(User * first, char* username, uint8_t ulen, char* password, uint8_t plen, bool admin, bool * added) {
    if(first == NULL){
        User * aux = malloc(sizeof(User));
        aux->username = malloc(ulen);
        aux->password = malloc(plen);
        if(aux == NULL){
            perror("Fallo malloc de aux user");
            return first;
        }
        aux->next = first;
        memcpy(aux->username, username, ulen);
        memcpy(aux->password, password, plen);
        aux->ulen = ulen;
        aux->upass = plen;
        aux->is_admin = admin;
        *added = true;
        return aux;
    } else if (strcmp(first->username, username) == 0) {
        return first;
    } else {
        first->next = add_user_rec(first->next, username, ulen, password, plen, admin, added);
    }
    return first;
}

bool add_user(char* username, uint8_t ulen, char* password, uint8_t plen, bool admin){
    bool added = false;
    list->first = add_user_rec(list->first, username, ulen, password, plen, admin, &added);
    if(added)
        list->size++;
    return added;
}


bool delete_user(char* username, uint8_t ulen){
   User * prev = list->first;
   User * curr = list->first;
	int c = -1;
     // Avanzamos hasta encontrar el elemento a borrar o detectar que no esta
	while( curr != NULL && c != 0) {
		c = strcmp(curr->username, username);
		if ( c != 0) {
			prev = curr;
			curr = curr->next;
		}
	}
	// curr apunta al elemento encontrado, al primero que es mayor a element o a NULL
	if (c != 0) {
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


bool edit_user(char* username, uint8_t ulen, char* password, uint8_t upass){
    User * curr = list->first;
    int c = -1;
    while( curr != NULL && c != 0) {
		c = strcmp(curr->username, username);
		if (c != 0) {
			curr = curr->next;
		}
	}

    if(c != 0) {
        return false;
    }

    if(strcmp(curr->username, username) == 0){
        curr->password = realloc(curr->password, upass);
        strcpy(curr->password, password);
        curr->upass = upass;
        return true;
    }

    return false;
}

char * get_all_users(int *nwrite){
    int init = 255;
    char * all_users = malloc(255);
    char * aux;
    if(all_users == NULL){
        return all_users;
    }
    User *user = list->first;
    int cont = 0;
    for (int k = 0; k < list->size; ++k){
        if(cont >= init || cont + user->ulen >= init){
            init *= 2;
            if ((aux = realloc(all_users,init)) == NULL){
                free(all_users);
                return NULL;
            }
            all_users = aux;
        }
        all_users[cont++] = user->ulen;
        strcpy(all_users + cont, user->username);
        cont += user->ulen;
        user = user->next;
    }   
    *nwrite = cont;
    return all_users;
}

int get_nusers() {
    return list->size;
}

int user_check_credentials(char* uname, char* passwd){
    User * current = list->first;

    while (current != NULL) {
        if(strcmp(uname, current->username) == 0 && strcmp(passwd, current->password) == 0 && current->is_admin == true){
            return 0;
        }
        current = current->next;
    }
    return 1;   
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

