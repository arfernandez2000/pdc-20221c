
#include "../../include/auth_prawtos.h"
#include <string.h>
#include <sys/socket.h>

static unsigned auth_prawtos_process(auth_st * state);

void auth_prawtos_init(const unsigned int st, selector_key * key){
    auth_st *state = &((struct prawtos *) key->data)->client.auth;
    state->read_buff = &((struct prawtos *) key->data)->read_buffer;
    state->write_buff = &((struct prawtos *) key->data)->write_buffer;
    state->parser.auth = &state->auth;
    auth_parser_init(&state->parser);
}

uint8_t check_credentials(const auth_st *state){
    if(user_check_credentials((char*)state->auth.uname, (char*)state->auth.passwd) == 0)
        return AUTH_SUCCESS;
    return AUTH_BAD_CREDENTIALS;
}

unsigned auth_prawtos_process(auth_st * state){
    unsigned ret = AUTH_WRITE_PRAWTOS;
    uint8_t status = check_credentials(state);
    if(auth_marshal(state->write_buff,status) == -1){
        ret = ERROR_PRAWTOS;
    }
    state->status = status;
    return ret;
}

unsigned auth_prawtos_read(selector_key * key){
    unsigned ret = AUTH_READ_PRAWTOS;
    auth_st * state = &((struct prawtos *) key->data)->client.auth;
    bool error = false;
    size_t count;

    uint8_t * ptr = buffer_write_ptr(state->read_buff,&count);
    ssize_t n = recv(key->fd,ptr,count,0);
    if (n > 0){
        buffer_write_adv(state->read_buff,n);
        int st = auth_consume(state->read_buff,&state->parser,&error);
        if(auth_is_done(st,0)){
            if (SELECTOR_SUCCESS == selector_set_interest_key(key, OP_WRITE))
            {
                ret = auth_prawtos_process(state);
            }
            else{
                ret = ERROR_PRAWTOS;
            }
        }

    }
    else{
        ret = ERROR_PRAWTOS;
    }
    return error ? ERROR_PRAWTOS : ret;
}

unsigned auth_prawtos_write(selector_key *key){
    auth_st * state = &((struct prawtos *) key->data)->client.auth;
    unsigned ret = AUTH_WRITE_PRAWTOS;
    size_t count;
    uint8_t  * ptr = buffer_read_ptr(state->write_buff,&count);
    ssize_t n = send(key->fd,ptr,count,MSG_NOSIGNAL);
    if(state->status != AUTH_SUCCESS){
        ret = ERROR_PRAWTOS;
    }
    else if (n > 0){
        buffer_read_adv(state->write_buff,n);
        if(!buffer_can_read(state->write_buff)){
            if(selector_set_interest_key(key,OP_READ) == SELECTOR_SUCCESS){
                ret = CMD_READ_PRAWTOS;
            }
            else{
                ret = ERROR_PRAWTOS;
            }
        }
    }
    return ret;
