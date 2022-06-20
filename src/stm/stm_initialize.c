#include "../../include/stm_initialize.h"
#include "../../include/socks5utils.h"
#include "../../include/hello.h"
#include "../../include/request.h"
#include "../../include/copy.h"

static state_definition session_state_def[DONE + 1];

state_definition done_state_def(void) {

    state_definition state_def = {
        .state = DONE,
        .on_arrival = NULL,
        .on_read_ready = NULL,
        .on_write_ready = NULL,
        .on_block_ready = NULL,
        .on_departure = NULL,
    };

    return state_def;
}

state_definition error_state_def(void) {

    state_definition state_def = {
        .state = ERROR,
        .on_arrival = NULL,
        .on_read_ready = NULL,
        .on_write_ready = NULL,
        .on_block_ready = NULL,
        .on_departure = NULL,
    };

    return state_def;
}

void stm_map() {
    session_state_def[HELLO_READ] = hello_state_def();
    session_state_def[HELLO_WRITE] = hello_write_state_def();
    //session_state_def[AUTH_READ] = auth_read_def();
    //session_state_def[AUTH_WRITE] = auth_write_state_def();
    session_state_def[REQUEST_READ] = request_read_state_def();
    //session_state_def[REQUEST_RESOLVE] = request_resolve_state_def();
    session_state_def[REQUEST_CONNECTING] = request_connecting_state_def();
    session_state_def[REQUEST_WRITE] = request_write_state_def();
    session_state_def[COPY] = copy_state_def();
    session_state_def[ERROR] = error_state_def();
    session_state_def[DONE] = done_state_def();
}

void stm_create(state_machine* s_machine) {
    s_machine->initial = HELLO_READ;
    s_machine->max_state = DONE;
    s_machine->states = session_state_def;
    // s_machine.current->state = HELLO_READ;
    
    stm_init(s_machine);
}

