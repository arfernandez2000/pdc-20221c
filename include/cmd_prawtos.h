#ifndef CMD_PRAWTOS_H
#define CMD_PRAWTOS_H

#include "selector.h"

void cmd_init(const unsigned int st, selector_key * key);
unsigned cmd_read(selector_key * key);
unsigned cmd_write(selector_key *key);

#endif