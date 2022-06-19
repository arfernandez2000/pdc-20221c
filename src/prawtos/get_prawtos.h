#ifndef GET_PRAWTOS_H
#define GET_PRAWTOS_H

#include "../selector/selector.h"

void get_init(const unsigned int st, selector_key * key);
unsigned get_read(selector_key * key);
unsigned get_write(selector_key *key);

#endif