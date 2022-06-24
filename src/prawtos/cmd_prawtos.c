
#include "../../include/cmd_prawtos.h"
#include "../../include/buffer.h"
#include "../../include/prawtosutils.h"
#include "../../include/stadistics.h"
#include "../../include/user_utils.h"
#include "../../include/socks5utils.h"

typedef int (*cmd_options)(cmd_prawtos_st *state);

static int bytes_sent(cmd_prawtos_st *state){
    int args_len = 5;
    state->args = malloc(args_len * sizeof(uint8_t));
    uint32_t bytes = get_bytes_sent();
    state->nargs = 0x01;
    state->args[0] = 0x04;
    state->args[1] = bytes >> 24;
    state->args[2] = (bytes >> 16) & 255;
    state->args[3] = (bytes >> 8) & 255;
    state->args[4] = bytes & 255;
    state->status = success;
    return args_len;
}

static int total_connections(cmd_prawtos_st *state){
    int args_len = 3;
    state->args = malloc(args_len * sizeof(uint8_t));
    uint16_t connections = get_total_connections();
    state->nargs = 0x01;
    state->args[0] = 0x02;
    state->args[1] = connections >> 8;
    state->args[2] = connections & 255;
    state->status = success;
    return args_len;
}

static int concurrent_connections(cmd_prawtos_st *state){
    int args_len = 3;
    state->args = malloc(args_len * sizeof(uint8_t));
    uint16_t connections = get_concurrent_connections();
    state->nargs = 0x01;
    state->args[0] = 0x02;
    state->args[1] = connections >> 8;
    state->args[2] = connections & 255;
    state->status = success;
    return args_len;
}

static int get_users_func(cmd_prawtos_st *state){
    int nwrite = 0;
    char * users = get_all_users(&nwrite);
    if(users == NULL){
        perror("error en get_all_user");
        return 0;
    }
    state->nargs = get_nusers();
    state->args = malloc(nwrite * sizeof(uint8_t));
    memcpy(state->args, (uint8_t *) users, nwrite);
    state-> status = success;
    return nwrite;
}

cmd_options get_handlers[] = {bytes_sent, total_connections, concurrent_connections, get_users_func};

static int create_user(cmd_prawtos_st *state){
    bool ans = add_user((char*)state->parser.user->uname, state->parser.user->ulen, (char*)state->parser.user->passwd, state->parser.user->plen, state->parser.user->admin);
    if(ans)
        state->status = success;
    else
        state->status = user_credentia;
    return 1;
}

static int del_user(cmd_prawtos_st *state){
    bool ans = delete_user((char*)state->parser.user->uname, state->parser.user->ulen);
    if(ans)
        state->status = success;
    else
        state->status = user_credentia;
    return 1;
}

static int modify_user(cmd_prawtos_st *state){
    bool ans = edit_user((char*) state->parser.user->uname, state->parser.user->ulen, (char*) state->parser.user->passwd,  state->parser.user->plen);
    if(ans)
        state->status = success;
    else 
        state->status = user_credentia;
    return 1;
}

//static void set_sniffer()

cmd_options user_handlers[] = {create_user, del_user, modify_user};

void cmd_init(const unsigned int st, selector_key * key){
    cmd_prawtos_st *state = &((struct prawtos *) (key->data))->client.cmd;
    state->read_buff = &((struct prawtos *) (key->data))->read_buffer;
    state->write_buff = &((struct prawtos *) (key->data))->write_buffer;
    state->parser.get = &state->get;
    state->parser.user = &state->user;
    prawtos_parser_init(&state->parser);
}

static unsigned cmd_process(cmd_prawtos_st * state){
    unsigned ret = CMD_WRITE_PRAWTOS;
    switch (state->parser.type) {
    case 0x00: {
        int args_len = get_handlers[state->get.cmd](state);
        if(get_marshal(state->write_buff,state->status, state->get.cmd, state->nargs, state->args, args_len) == -1){
            ret = ERROR_PRAWTOS;
        }
        break;
    }
    case 0x01:
        user_handlers[state->user.cmd](state);
        if(user_marshal(state->write_buff,state->status) == -1){
            ret = ERROR_PRAWTOS;
        }
        break;
    case 0x02:
        {
        int status = set_enable(state->get.cmd);
        if(sniff_marshal(state->write_buff,status) == -1){
            ret = ERROR_PRAWTOS;
        }
        break;
        }
    case 0x03:
        state->status = success;
        if(quit_marshal(state->write_buff,state->status) == -1){
            ret = ERROR_PRAWTOS;
        }
        break;
    default:
        ret = ERROR_PRAWTOS;
        break;
    }
    
    return ret;
}

unsigned cmd_read(selector_key * key){
    unsigned ret = CMD_READ_PRAWTOS;
    cmd_prawtos_st * state = &((struct prawtos *) key->data)->client.cmd;
    bool error = false;
    size_t count;


    uint8_t * ptr = buffer_write_ptr(state->read_buff, &count);
    ssize_t n = recv(key->fd, ptr, count, 0);
    if (n > 0){
        buffer_write_adv(state->read_buff,n);
        int st = prawtos_consume(state->read_buff,&state->parser,&error);
        if(prawtos_is_done(st,0)){
            if (SELECTOR_SUCCESS == selector_set_interest_key(key, OP_WRITE))
            {
                ret = cmd_process(state);
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

unsigned cmd_write(selector_key *key) {
    cmd_prawtos_st * state = &((struct prawtos *) key->data)->client.cmd;
    unsigned ret = CMD_WRITE_PRAWTOS;
    size_t count;
    uint8_t  * ptr = buffer_read_ptr(state->write_buff, &count);
    ssize_t n = send(key->fd, ptr, count, MSG_NOSIGNAL);
    if(state->status == cmd_not_supported){
        ret = ERROR_PRAWTOS;
    }
    else if (n > 0){
        if(state->parser.type == 0x03){
            ret = DONE_PRAWTOS;
        } else {
            buffer_read_adv(state->write_buff, n);
            if(!buffer_can_read(state->write_buff)){
                if(selector_set_interest_key(key,OP_READ) == SELECTOR_SUCCESS){
                    ret = CMD_READ_PRAWTOS;
                }
                else{
                    ret = ERROR_PRAWTOS;
                }
            }
        }
    }
    return ret;
}