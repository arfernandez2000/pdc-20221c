#ifndef TYPE_PRAWTOS_H
#define TYPE_PRAWTOS_H

#include "prawtosutils.h"
#include "../selector/selector.h"

void type_init(const unsigned int st, selector_key * key);
enum typ_response_status check_type(const typ_prawtos_st *state);
unsigned type_read(selector_key * key);
unsigned type_write(selector_key *key);

#endif