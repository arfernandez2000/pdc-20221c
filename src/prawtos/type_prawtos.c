#include "type_prawtos.h"
#include "../parser/prawtos_typ_parser.h"
#include <sys/socket.h>

void type_init(const unsigned int st, selector_key * key){
    typ_prawtos_st *state = &((struct prawtos *) key->data)->client.typ;
    state->read_buff = &((struct prawtos *) key->data)->read_buffer;
    aux = ((struct prawtos *) key->data)->read_buffer;
    state->write_buff = &((struct prawtos *) key->data)->write_buffer;
    get_prawtos_st * get_state = &((struct prawtos *) key->data)->client.get;
    get_state->read_buff = &((struct prawtos *) key->data)->read_buffer + 1;
    get_state->write_buff = &((struct prawtos *) key->data)->write_buffer;
    get_state->args = NULL;
    get_state->nargs = 0;
    user_prawtos_st * user_state = &((struct prawtos *) key->data)->client.user;
    user_state->read_buff = &((struct prawtos *) key->data)->read_buffer + 1;
    user_state->write_buff = &((struct prawtos *) key->data)->write_buffer;
    typ_parser_init(&state->parser);
    get_parser_init(&get_state->parser);
    user_parser_init(&user_state->parser);
}

enum typ_response_status check_type(const typ_prawtos_st *state){
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

unsigned type_read(selector_key * key){
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

unsigned type_write(selector_key *key) {
    typ_prawtos_st * state = &((struct prawtos *) key->data)->client.typ;
    unsigned ret = TYP_WRITE;
    size_t count;
    uint8_t  * ptr = buffer_read_ptr(state->write_buff,&count);
    ssize_t n = send(key->fd,ptr,count,MSG_NOSIGNAL);
    if(state->status == cmd_unsupported){
        fprintf(stdout, "Lrpmqmp\n");
        ret = ERROR;
    }
    else if (n > 0){
        buffer_read_adv(state->write_buff,n);
        if(!buffer_can_read(state->write_buff)){
            if(selector_set_interest_key(key,OP_READ) == SELECTOR_SUCCESS){
                switch (state->status)
                {
                case cmd_get:
                    fprintf(stdout, "Llegue al get read\n");
                    ret = GET_READ;
                    break;
                
                case cmd_user:
                    fprintf(stdout, "Llegue al user read\n");
                    ret = USER_READ;
                    break;
                default:
                    fprintf(stdout, "Lrpmqmp 1\n");
                    ret = ERROR;
                    break;    
                }
            }
            else{
                fprintf(stdout, "Lrpmqmp 2\n");
                ret = ERROR;
            }
        }
    }
    return ret;
}
