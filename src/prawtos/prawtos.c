#include "prawtos.h"
#include "../buffer/buffer.h"
#include <stdio.h>
#include "../parser/auth_parser.h"
#include "../parser/prawtos_get_parser.h"
#include "../parser/prawtos_user_parser.h"
#include "../parser/prawtos_typ_parser.h"
#include <sys/socket.h>
#include "../stm/stm.h"
#include <string.h>

#define BUFFER_SIZE 4096


enum prawtos_state
{
    AUTH_READ = 0,
    AUTH_WRITE,
    TYP_READ,
    TYP_WRITE,
    GET_READ,
    GET_WRITE,
    USER_READ,
    USER_WRITE,
    DONE,
    ERROR
};

typedef struct auth_prawtos_st{
    buffer *read_buff, *write_buff;
    auth_parser parser;
    uint8_t ulen;
    uint8_t uname[MAX_LEN];
    uint8_t plen;
    uint8_t passwd[MAX_LEN];
    uint8_t status;
} auth_prawtos_st;

typedef struct typ_prawtos_st {
    buffer *read_buff, *write_buff;
    typ_parser parser;
    enum typ_response_status status;
} typ_prawtos_st;


struct prawtos {
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len;
    int client_fd;

    state_machine stm;

    union{
        auth_prawtos_st auth;
        typ_prawtos_st typ;
    } client;

    /** buffers para write y read **/
    uint8_t raw_buff_a[BUFFER_SIZE], raw_buff_b[BUFFER_SIZE];
    buffer read_buffer, write_buffer;
};

static void auth_prawtos_init(const unsigned int st, selector_key * key);
static unsigned auth_prawtos_read(selector_key * key);
static unsigned auth_prawtos_write(selector_key *key);
static unsigned auth_prawtos_process(auth_prawtos_st * state);
static uint8_t check_credentials(const auth_prawtos_st *state);
static void type_init(const unsigned int st, selector_key * key);
static enum typ_response_status check_type(const typ_prawtos_st *state);
static unsigned type_process(typ_prawtos_st * state);
static unsigned type_read(selector_key * key);
static unsigned type_write(selector_key *key);

static const struct state_definition prawtos_init_states[]={
    {
        .state = AUTH_READ,
        .on_arrival = auth_prawtos_init,
        .on_read_ready = auth_prawtos_read,
    },
    {
        .state = AUTH_WRITE,
        .on_write_ready = auth_prawtos_write,
    },
    {
        .state = TYP_READ,
        .on_arrival = type_init,
        .on_read_ready = type_read,
    },
    {
        .state = TYP_WRITE,
        .on_write_ready = type_write,
    },
    {
        .state = DONE,
    },
    {
        .state = ERROR,
    }
};

static void auth_prawtos_init(const unsigned int st, selector_key * key){
    auth_prawtos_st *state = &((struct prawtos *) key->data)->client.auth;
    state->read_buff = &((struct prawtos *) key->data)->read_buffer;
    state->write_buff = &((struct prawtos *) key->data)->write_buffer;
    auth_parser_init(&state->parser);
    memcpy(state->uname,state->parser.auth->uname, state->parser.auth->ulen);
    state->ulen = state->parser.auth->ulen;
    memcpy(state->passwd,state->parser.auth->passwd, state->parser.auth->plen);
    state->plen = state->parser.auth->plen;
}

static uint8_t check_credentials(const auth_prawtos_st *state){
    return AUTH_SUCCESS;
}

static unsigned auth_prawtos_process(auth_prawtos_st * state){
    unsigned ret = AUTH_WRITE;
    uint8_t status = check_credentials(state);
    if(auth_marshal(state->write_buff,status) == -1){
        ret = ERROR;
    }
    state->status = status;
    return ret;
}

static unsigned auth_prawtos_read(selector_key * key){
    unsigned ret = AUTH_READ;
    auth_prawtos_st * state = &((struct prawtos *) key->data)->client.auth;
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
                ret = ERROR;
            }
        }

    }
    else{
        ret = ERROR;
    }
    return error ? ERROR : ret;
}

static unsigned auth_prawtos_write(selector_key *key){
    auth_prawtos_st * state = &((struct prawtos *) key->data)->client.auth;
    unsigned ret = AUTH_WRITE;
    size_t count;
    uint8_t  * ptr = buffer_read_ptr(state->write_buff,&count);
    ssize_t n = send(key->fd,ptr,count,MSG_NOSIGNAL);
    if(state->status != AUTH_SUCCESS){
        ret = ERROR;
    }
    else if (n > 0){
        buffer_read_adv(state->write_buff,n);
        if(!buffer_can_read(state->write_buff)){
            if(selector_set_interest_key(key,OP_READ) == SELECTOR_SUCCESS){
                //ret = CMD_READ
                ret = USER_READ;
            }
            else{
                ret = ERROR;
            }
        }
    }
    return ret;
}

static void type_init(const unsigned int st, selector_key * key){
    typ_prawtos_st *state = &((struct prawtos *) key->data)->client.typ;
    state->read_buff = &((struct prawtos *) key->data)->read_buffer;
    state->write_buff = &((struct prawtos *) key->data)->write_buffer;
    typ_parser_init(&state->parser);
}

static enum typ_response_status check_type(const typ_prawtos_st *state){
    switch (state->parser.type) {
    case 0x00:
        return cmd_get; 
    case 0x01:
        return cmd_user; 
    default:
        return cmd_unsupported;
    }
}

static unsigned type_process(typ_prawtos_st * state){
    unsigned ret = TYP_WRITE;
    enum typ_response_status status = check_type(state);
    if(auth_marshal(state->write_buff,status) == -1){
        ret = ERROR;
    }
    state->status = status;
    return ret;
}

static unsigned type_read(selector_key * key){
    unsigned ret = TYP_READ;
    typ_prawtos_st * state = &((struct prawtos *) key->data)->client.typ;
    bool error = false;
    size_t count;

    uint8_t * ptr = buffer_write_ptr(state->read_buff,&count);
    ssize_t n = recv(key->fd,ptr,count,0);
    if (n > 0){
        buffer_write_adv(state->read_buff,n);
        int st = typ_consume(state->read_buff,&state->parser,&error);
        if(typ_is_done(st,0)){
            if (SELECTOR_SUCCESS == selector_set_interest_key(key, OP_WRITE))
            {
                ret = type_process(state);
            }
            else{
                ret = ERROR;
            }
        }

    }
    else{
        ret = ERROR;
    }
    return error ? ERROR : ret;
}

static unsigned type_write(selector_key *key){
    typ_prawtos_st * state = &((struct prawtos *) key->data)->client.typ;
    unsigned ret = TYP_WRITE;
    size_t count;
    uint8_t  * ptr = buffer_read_ptr(state->write_buff,&count);
    ssize_t n = send(key->fd,ptr,count,MSG_NOSIGNAL);
    if(state->status == cmd_unsupported){
        fprintf(stdout, "Lrpmqmp");
        ret = ERROR;
    }
    else if (n > 0){
        buffer_read_adv(state->write_buff,n);
        if(!buffer_can_read(state->write_buff)){
            if(selector_set_interest_key(key,OP_READ) == SELECTOR_SUCCESS){
                switch (state->status)
                {
                case cmd_get:
                    fprintf(stdout, "Llegue al get read");
                    ret = GET_READ;
                    break;
                
                case cmd_user:
                    fprintf(stdout, "Llegue al user read");
                    ret = USER_READ;
                    break;
                default:
                    fprintf(stdout, "Lrpmqmp 1");
                    ret = ERROR;
                    break;    
                }
            }
            else{
                fprintf(stdout, "Lrpmqmp 2");
                ret = ERROR;
            }
        }
    }
    return ret;
}

