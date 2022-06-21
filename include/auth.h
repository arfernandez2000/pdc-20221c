#ifndef AUTH_H
#define AUTH_H

#include "selector.h"
#include "stm.h"

state_definition auth_read_def(void);
state_definition auth_write_def(void);
void auth_arrival(unsigned int st, selector_key * event);
unsigned auth_read(selector_key* event);
unsigned auth_write(selector_key *event);
unsigned auth_process(const struct auth_st *state);


#endif