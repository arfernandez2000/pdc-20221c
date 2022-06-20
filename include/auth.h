#ifndef AUTH_H
#define AUTH_H

#include "../../selector/selector.h"
#include "../../stm.h"

state_definition auth_read_def(void);
state_definition auth_write_def(void);
static void auth_arrival(unsigned int st, selector_key * event);
static unsigned auth_read(selector_key* event);
static unsigned auth_write(selector_key *event);
static unsigned auth_process(const struct auth_st *state);


#endif