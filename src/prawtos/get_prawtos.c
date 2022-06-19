#include "get_prawtos.h"
#include "../buffer/buffer.h"
#include "prawtosutils.h"
#include "../stadistics/stadistics.h"

typedef void (*get_options)(get_prawtos_st *state);

static void bytes_sent(get_prawtos_st *state){
    int args_len = 3;
    state->args = malloc(args_len * sizeof(uint8_t));
    uint32_t bytes = get_bytes_sent();
    state->nargs = 1;
    state->args[0] = bytes >> 24;
    // state->resp[0] = state->parser.get;
    // state->resp[1] = 0x01;
    // state->resp[2] = 0x04;
    // state->resp[3] = bytes >> 24;
    for(int i = 1; i < args_len;i++){
        state->args[i] = (bytes >> (8*(6-i))) & 255;
    }
    state->status = get_success;
}

get_options handlers[] = {bytes_sent};

void get_init(const unsigned int st, selector_key * key){
    fprintf(stdout, "Estoy en get_init!\n");
    //get_prawtos_st *state = &((struct prawtos *) key->data)->client.get;
    // state->read_buff = &aux;
    // state->write_buff = &((struct prawtos *) key->data)->write_buffer;
    // get_parser_init(&state->parser);
    fprintf(stdout, "Estoy al final de get_init!\n");
}

static unsigned get_process(get_prawtos_st * state){
    unsigned ret = GET_WRITE;
    handlers[state->get->cmd](state);
    if(get_marshal(state->write_buff,state->status, state->get->cmd, state->nargs, state->args) == -1){
        ret = ERROR;
    }
    return ret;
}

unsigned get_read(selector_key * key){
    fprintf(stdout, "Estoy en get_read!\n");
    unsigned ret = GET_READ;
    get_prawtos_st * state = &((struct prawtos *) key->data)->client.get;
    bool error = false;
    size_t count;

    uint8_t * ptr = buffer_write_ptr(state->read_buff,&count);
    ssize_t n = recv(key->fd,ptr,count,0);
    if (n > 0){
        buffer_write_adv(state->read_buff,n);
        int st = get_consume(state->read_buff,&state->parser,&error);
        if(get_is_done(st,0)){
            if (SELECTOR_SUCCESS == selector_set_interest_key(key, OP_WRITE))
            {
                ret = get_process(state);
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

unsigned get_write(selector_key *key) {
    get_prawtos_st * state = &((struct prawtos *) key->data)->client.get;
    unsigned ret = GET_WRITE;
    size_t count;
    uint8_t  * ptr = buffer_read_ptr(state->write_buff, &count);
    ssize_t n = send(key->fd, ptr, count, MSG_NOSIGNAL);
    if(state->status == get_cmd_not_supported){
        fprintf(stdout, "Lrpmqmp\n");
        ret = ERROR;
    }
    else if (n > 0){
        buffer_read_adv(state->write_buff, n);
        if(!buffer_can_read(state->write_buff)){
            if(selector_set_interest_key(key,OP_READ) == SELECTOR_SUCCESS){
                ret = GET_WRITE;
            }
            else{
                fprintf(stdout, "Lrpmqmp 1\n");
                ret = ERROR;
            }
        }
    }
    return ret;
}
