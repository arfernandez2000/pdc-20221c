#ifndef USER_PRAWTOS_H
#define USER_PRAWTOS_H

#include "../selector/selector.h"

void user_init(const unsigned int st, selector_key * key);
unsigned user_read(selector_key * key);
unsigned user_write(selector_key *key);

#endif