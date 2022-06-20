#include "cmd_prawtos.h"
#include "../buffer/buffer.h"
#include "prawtosutils.h"
#include "../stadistics/stadistics.h"
#include "../user/user_utils.h"

typedef void (*cmd_options)(cmd_prawtos_st *state);

static void bytes_sent(cmd_prawtos_st *state){
    fprintf(stdout, "Estoy en bytes_sent!\n");
    int args_len = 5;
    state->args = malloc(args_len * sizeof(uint8_t));
    uint32_t bytes = get_bytes_sent();
    fprintf(stdout, "bytes_sent: %d\n", bytes);
    state->nargs = 0x01;
    state->args[0] = 0x04;
    state->args[1] = bytes >> 24;
    state->args[2] = (bytes >> 16) & 255;
    state->args[3] = (bytes >> 8) & 255;
    state->args[4] = bytes & 255;
   
    state->status = success;
}

static void total_connections(cmd_prawtos_st *state){
    fprintf(stdout, "Estoy en total_connections!\n");
    int args_len = 3;
    state->args = malloc(args_len * sizeof(uint8_t));
    uint16_t connections = get_total_connections();
    fprintf(stdout, "bytes_sent: %d\n", connections);
    state->nargs = 0x01;
    state->args[0] = 0x02;
    state->args[1] = connections >> 8;
    state->args[2] = connections & 255;
    state->status = success;
}

static void concurrent_connections(cmd_prawtos_st *state){
    int args_len = 3;
    state->args = malloc(args_len * sizeof(uint8_t));
    uint16_t connections = get_concurrent_connections();
    fprintf(stdout, "bytes_sent: %d\n", connections);
    state->nargs = 0x01;
    state->args[0] = 0x02;
    state->args[1] = connections >> 8;
    state->args[2] = connections & 255;
    state->status = success;
}

static void get_users_func(cmd_prawtos_st *state){
    size_t nwrite;
    char * users = get_all_users();
    if(users == NULL){
        perror("error en get_all_user");
        return;
    }
    state->nargs = get_nusers();
    state->args = malloc(strlen(users) * sizeof(uint8_t));
    memcpy(state->args, (uint8_t *) users, strlen(users));
    state-> status = success;
}

cmd_options get_handlers[] = {bytes_sent, total_connections, concurrent_connections, get_users_func};

static void create_user(cmd_prawtos_st *state){
    bool ans = add_user((char*)state->parser.user->uname, state->parser.user->ulen, (char*)state->parser.user->passwd, state->parser.user->plen, false);
    if(ans)
        state->status = success;
    state->status = user_credentia;
}

static void del_user(cmd_prawtos_st *state){
    bool ans = delete_user((char*)state->parser.user->uname, state->parser.user->ulen);
    if(ans)
        state->status = success;
    state->status = user_credentia;
}

static void modify_user(cmd_prawtos_st *state){
    bool ans = edit_user((char*) state->parser.user->uname, state->parser.user->ulen, (char*) state->parser.user->passwd,  state->parser.user->plen);
    if(ans)
        state->status = success;
    state->status = user_credentia;
}

//static void set_sniffer()

cmd_options user_handlers[] = {del_user, modify_user,create_user};

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
        fprintf(stdout, "get!\n");
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
    fprintf(stdout, "key->fd: %d\n", key->fd);
    fprintf(stdout, "ptr: %s\n", ptr);
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