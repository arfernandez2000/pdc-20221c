#ifndef USER_UTILS_H
#define USER_UTILS_H
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef struct User {
    bool is_admin;
    char * username;
    uint8_t ulen;
    char * password;
    uint8_t upass;
    struct User * next;
} User;

typedef struct user_list{
    User * first;
    int size;
}user_list;

static user_list * list = NULL;

void init_user_list();
bool add_user(user_list * list, char* username, uint8_t ulen, char* pass, uint8_t upass, bool admin);
bool delete_user(user_list * list, char* username, uint8_t ulen);
bool edit_user(user_list * list, char* username, uint8_t ulen, char* pass, uint8_t upass);
char * get_all_users(user_list * list);

#endif