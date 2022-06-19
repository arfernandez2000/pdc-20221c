#include "cmd_prawtos.h"
#include "../buffer/buffer.h"
#include "prawtosutils.h"
#include "../stadistics/stadistics.h"


typedef void (*cmd_options)(cmd_prawtos_st *state);

static void bytes_sent(cmd_prawtos_st *state){
    fprintf(stdout, "Estoy en bytes_sent!\n");
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
    state->status = success;
}

cmd_options get_handlers[] = {bytes_sent};

cmd_options user_handlers[] = {};

void cmd_init(const unsigned int st, selector_key * key){
    fprintf(stdout, "Estoy en cmd_init!\n");
    cmd_prawtos_st *state = &((struct prawtos *) (key->data))->client.cmd;
    state->read_buff = &((struct prawtos *) (key->data))->read_buffer;
    state->write_buff = &((struct prawtos *) (key->data))->write_buffer;
    state->parser.get = &state->get;
    state->parser.user = &state->user;
    prawtos_parser_init(&state->parser);
    fprintf(stdout, "Estoy al final de cmd_init!\n");
}

static unsigned cmd_process(cmd_prawtos_st * state){
    fprintf(stdout, "Estoy al final de cmd_process!\n");
    unsigned ret = CMD_WRITE;
    switch (state->parser.type) {
    case 0x00:
        fprintf(stdout, "bytes!\n");
        fprintf(stdout, "cmd: %d\n", state->get.cmd);
        get_handlers[state->get.cmd](state);
        if(get_marshal(state->write_buff,state->status, state->get.cmd, state->nargs, state->args) == -1){
            ret = ERROR;
        }
        fprintf(stdout, "Despues de marshall!\n");
        break;
    case 0x01:
        user_handlers[state->user.cmd](state);
        if(user_marshal(state->write_buff,state->status) == -1){
            ret = ERROR;
        }
        break;
    default:
        ret = ERROR;
        break;
    }
    
    return ret;
}

unsigned cmd_read(selector_key * key){
    fprintf(stdout, "Estoy en cmd_read!\n");
    unsigned ret = CMD_READ;
    cmd_prawtos_st * state = &((struct prawtos *) key->data)->client.cmd;
    bool error = false;
    size_t count;

    fprintf(stdout, "Estoy en cmd_read 2!\n");

    uint8_t * ptr = buffer_write_ptr(state->read_buff, &count);
    ssize_t n = recv(key->fd, ptr, count, 0);
    fprintf(stdout, "Estoy en cmd_read 3!\n");
    if (n > 0){
        buffer_write_adv(state->read_buff,n);
        int st = prawtos_consume(state->read_buff,&state->parser,&error);
        fprintf(stdout, "Despues del prawtos_consume!\n");
        if(prawtos_is_done(st,0)){
            fprintf(stdout, "Adntro del if prawtos_is_done!\n");
            if (SELECTOR_SUCCESS == selector_set_interest_key(key, OP_WRITE))
            {
                ret = cmd_process(state);
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

unsigned cmd_write(selector_key *key) {
    cmd_prawtos_st * state = &((struct prawtos *) key->data)->client.cmd;
    unsigned ret = CMD_WRITE;
    size_t count;
    uint8_t  * ptr = buffer_read_ptr(state->write_buff, &count);
    ssize_t n = send(key->fd, ptr, count, MSG_NOSIGNAL);
    if(state->status == cmd_not_supported){
        fprintf(stdout, "Lrpmqmp\n");
        ret = ERROR;
    }
    else if (n > 0){
        buffer_read_adv(state->write_buff, n);
        if(!buffer_can_read(state->write_buff)){
            if(selector_set_interest_key(key,OP_READ) == SELECTOR_SUCCESS){
                ret = CMD_READ;
            }
            else{
                fprintf(stdout, "Lrpmqmp 1\n");
                ret = ERROR;
            }
        }
    }
    return ret;
}