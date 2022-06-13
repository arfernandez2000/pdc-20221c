#include "../../../socks5/socks5utils.h"
#include "../../stm_initialize.h"

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

static void auth_arrival(selector_key * event)
{
    auth_st* state = &((Session *) (event->date))->socks.auth;
    state->read_buff = &((Session *) (event->data)) ->input;
    state->write_buff =  &((Session *) (event->data)) ->output;
    // auth_parser_init(&state->parser);
    // &((Session *) (event->date))->client_information->user->username = &state->parser.username;
    // &((Session *) (event->date))->client_information->user->password = &state->parser.password;
}

static unsigned auth_read(selector_key* event) {
    struct auth_st * state = &((Session *) (event->data))->socks.auth;
    unsigned ret = AUTH_READ;
    bool error = false;
    uint8_t *ptr;
    size_t count;

    ptr = buffer_write_ptr(state->read_buff,&count);
    ssize_t n = recv(key->fd,ptr,count,0);
    if (n > 0)
    {
        buffer_write_adv(state->read_buff,n);
        const enum auth_state curr_state = auth_consume(state->read_buff,&state->parser,&error);
        if(auth_is_done(curr_state,0)){
            if (SELECTOR_SUCCESS == selector_set_interest_key(event, OP_WRITE))
            {
                ret = auth_process(state);
                //memcpy(&ATTACHMENT(key)->socks_info.user_info,&d->parser.usr,sizeof(d->parser.usr));
                
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

static unsigned auth_write(selector_key *event){
    struct auth_st * state = &((Session *) (event->data))->socks.auth;
    unsigned ret = AUTH_WRITE;
    uint8_t *ptr;
    size_t count;
    ssize_t n;

    ptr = buffer_read_ptr(state->write_buff,&count);
    n = send(event->fd,ptr,count,MSG_NOSIGNAL);
    if(state->method != AUTH_SUCCESS){
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

static unsigned auth_process(const struct auth_st *state) {
    unsigned ret = AUTH_WRITE;
    
    //TODO: habria que validar que las credentials sean validas
    // Esta escrito por el coda
    uint8_t credential_status = validate_credentials(state);

    uint8_t method = state->method;
    if (-1 == auth_marshall(state->write_buff, method)) {
        ret = ERROR;
    }
    if (method == METHOD_NO_ACCEPTABLE_METHODS) {
        ret = ERROR;
    }

    return ret;
}