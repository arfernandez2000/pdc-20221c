#ifndef AUTH_PRAWTOS_H
#define AUTH_PRAWTOS_H

#include "prawtosutils.h"

void auth_prawtos_init(const unsigned int st, selector_key * key);
unsigned auth_prawtos_read(selector_key * key);
unsigned auth_prawtos_write(selector_key *key);
uint8_t check_credentials(const auth_st *state);


#endif
