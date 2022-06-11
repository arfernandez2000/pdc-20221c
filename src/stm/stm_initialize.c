#include "stm_initialize.h"

static state_definition session_state_def[FINISH + 1];

void stm_map(){
    session_state_def[HELLO] = hello_state_def();
    session_state_def[HELLO_ERROR] = hello_error_state_def();
    session_state_def[AUTH_ANNOUNCEMENT] = auth_announcement_def();
    session_state_def[AUTH_REQUEST] = auth_request_state_def();
    session_state_def[AUTH_ERROR] = auth_error_state_def();
    session_state_def[AUTH_SUCCESSFUL] = auth_succesful_state_def();
    session_state_def[REQUEST] = request_state_def();
    session_state_def[REQUEST_ERROR] = request_error_state_def();
    session_state_def[IP_CONNECT] = ip_connect_state_def();
    session_state_def[DNS_QUERY] = dns_query_state_def();
    session_state_def[DNS_CONNECT] = dns_connect_state_def();
    session_state_def[REQUEST_SUCCESSFUL] = request_succesful_state_def();
    session_state_def[FORWARDING] = forwarding_state_def();
    session_state_def[CLOSING] = closing_state_def();
    session_state_def[CLOSER] = closer_state_def();
    session_state_def[FINISH] = finish_state_def();
}

void stm_create(state_machine s_machine) {
    s_machine->initial = HELLO;
    s_machine->max_states = FINISH;
    s_machine->states = session_state_def;
    s_machine->current = HELLO;

    stm_init(s_machine);
}