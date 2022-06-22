#ifndef REQUEST_H
#define REQUEST_H

#include "stm.h"

state_definition request_read_state_def(void);
state_definition request_write_state_def(void);
// state_definition request_resolve_state_def(void);
state_definition request_connecting_state_def(void);

#endif
