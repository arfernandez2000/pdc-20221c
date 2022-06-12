#include "stm_initialize.h"
#include "../socks5/socks5utils.h"
#include "states/hello/hello.h"

static state_definition session_state_def[HELLO_WRITE + 1];

void stm_map() {
    session_state_def[HELLO_READ] = hello_state_def();
    session_state_def[HELLO_WRITE] = hello_write_state_def();
    //session_state_def[AUTH_READ] = auth_read_def();
    //session_state_def[AUTH_WRITE] = auth_write_state_def();
    //session_state_def[REQUEST_READ] = request_read_state_def();
    //session_state_def[REQUEST_RESOLVE] = request_resolve_state_def();
    //session_state_def[REQUEST_CONNECTING] = request_connecting_def();
    //session_state_def[REQUEST_WRITE] = request_write_state_def();
    //session_state_def[ERROR] = error_state_def();
    //session_state_def[DONE] = done_state_def();
}

void stm_create(state_machine* s_machine) {
    s_machine->initial = HELLO_READ;
    s_machine->max_state = HELLO_WRITE;
    s_machine->states = session_state_def;
    // s_machine.current->state = HELLO_READ;
    
    stm_init(s_machine);
}