#include "prawtos.h"
#include "../buffer/buffer.h"
#include <stdio.h>
#include "../parser/auth_parser.h"
#include <sys/socket.h>
#include "../stm/stm.h"
#include <string.h>

#define BUFFER_SIZE 4096

static void auth_prawtos_init(const unsigned int st, selector_key * key);
static unsigned auth_prawtos_read(selector_key * key);
static unsigned auth_prawtos_write(selector_key *key);

enum prawtos_state
{
    AUTH_READ = 0,
    AUTH_WRITE,
    //CMD_READ,
    //CMD_WRITE,
    DONE,
    ERROR
};

struct auth_prawtos_st{
    buffer *read_buff, *write_buff;
    auth_parser parser;
    uint8_t ulen;
    uint8_t uname[MAX_LEN];
    uint8_t plen;
    uint8_t passwd[MAX_LEN];
    uint8_t status;
};

struct prawtos{
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len;
    int client_fd;

    state_machine stm;

    union{
        struct auth_prawtos_st auth;
        //struct cmd_prawtos_st cmd;
    }client;

    /** buffers para write y read **/
    uint8_t raw_buff_a[BUFFER_SIZE], raw_buff_b[BUFFER_SIZE];
    buffer read_buffer, write_buffer;
};

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
        .state = DONE,
    },
    {
        .state = ERROR,
    }
};

static void auth_prawtos_init(const unsigned int st, selector_key * key){
    struct auth_prawtos_st *state = &((struct prawtos *) key->data)->client.auth;
    state->read_buff = &((struct prawtos *) key->data)->read_buffer;
    state->write_buff = &((struct prawtos *) key->data)->write_buffer;
    auth_parser_init(&state->parser);
    memcpy(state->uname,state->parser.auth->uname, state->parser.auth->ulen);
    state->ulen = state->parser.auth->ulen;
    memcpy(state->passwd,state->parser.auth->passwd, state->parser.auth->plen);
    state->plen = state->parser.auth->plen;
}

static uint8_t check_credentials(const struct auth_prawtos_st *state){

    return AUTH_SUCCESS;
}

static unsigned auth_prawtos_process(struct auth_prawtos_st * state){
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
    struct auth_prawtos_st * state = &((struct prawtos *) key->data)->client.auth;
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

static unsigned auth_prawtos_write(struct selector_key *key){
    struct auth_prawtos_st * state = &((struct prawtos *) key->data)->client.auth;
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
                ret = DONE;
            }
            else{
                ret = ERROR;
            }
        }
    }
    return ret;
}

