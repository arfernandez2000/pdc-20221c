#include "../../include/socks5utils.h"
#include "../../include/stm_initialize.h"
#include <string.h>

void auth_arrival(unsigned int st, selector_key * event)
{
    auth_st* state = &((Session *) (event->data))->client_header.auth;
    state->read_buff = &((Session *) (event->data)) ->input;
    state->write_buff =  &((Session *) (event->data)) ->output;
    state->parser.auth = &state->auth;
    auth_parser_init(&state->parser);
}

uint8_t check_credentials(const auth_st *state){
    return user_check_credentials((char*)state->auth.uname, (char*)state->auth.passwd, 1);
}

unsigned auth_process(const struct auth_st *state) {
    unsigned ret = AUTH_WRITE;

    uint8_t status = check_credentials(state);

    uint8_t method = state->method;
    if (-1 == auth_marshal(state->write_buff, status)) {
        ret = ERROR;
    }
    if (method == METHOD_NO_ACCEPTABLE_METHODS) {
        ret = ERROR;
    }

    return ret;
}

unsigned auth_read(selector_key* event) {
    struct auth_st * state = &((Session *) (event->data))->client_header.auth;
    unsigned ret = AUTH_READ;
    bool error = false;
    uint8_t *ptr;
    size_t count;

    ptr = buffer_write_ptr(state->read_buff,&count);
    ssize_t n = recv(event->fd,ptr,count,0);
    if (n > 0)
    {
        buffer_write_adv(state->read_buff,n);
        const enum auth_state curr_state = auth_consume(state->read_buff,&state->parser,&error);
        if(auth_is_done(curr_state,0)){
            if (SELECTOR_SUCCESS == selector_set_interest_key(event, OP_WRITE))
            {
                ret = auth_process(state);
                memcpy(&((Session *) (event->data))->register_info.user_info, &state->parser.auth, sizeof(state->parser.auth));
                
            }
            else{
                error = true;
                ret = ERROR;
            }
        }

    }
    else{
        error = true;
        ret = ERROR;
    }
    return error ? ERROR : ret;
}

unsigned auth_write(selector_key *event) {
    struct auth_st * state = &((Session *) (event->data))->client_header.auth;
    unsigned ret = AUTH_WRITE;
    uint8_t *ptr;
    size_t count;
    ssize_t n;

    ptr = buffer_read_ptr(state->write_buff,&count);
    n = send(event->fd,ptr,count,MSG_NOSIGNAL);
    if(n == -1){
        ret = ERROR;
    }
    else if (n > 0){
        buffer_read_adv(state->write_buff,n);
        if(!buffer_can_read(state->write_buff)){
            if(selector_set_interest_key(event,OP_READ) == SELECTOR_SUCCESS){
                ret = REQUEST_READ;
            }
            else{
                ret = ERROR;
            }
        }
    }
    return ret;
}

state_definition auth_read_def(void) {

    state_definition state_def = {
        .state = AUTH_READ,
        .on_arrival = auth_arrival,
        .on_read_ready = auth_read,
        .on_write_ready = NULL,
        .on_block_ready = NULL,
        .on_departure = NULL,
    };

    return state_def;
}

state_definition auth_write_def(void) {

    state_definition state_def = {
        .state = AUTH_WRITE,
        .on_arrival = NULL,
        .on_read_ready = NULL,
        .on_write_ready = auth_write,
        .on_block_ready = NULL,
        .on_departure = NULL,
    };

    return state_def;
}