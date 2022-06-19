#include "user_prawtos.h"
#include "prawtosutils.h"

void user_init(const unsigned int st, selector_key * key){
    // user_prawtos_st *state = &((struct prawtos *) key->data)->client.user; //chequear
    // state->read_buff = &((struct prawtos *) key->data)->read_buffer;
    // state->write_buff = &((struct prawtos *) key->data)->write_buffer;
    // user_parser_init(&state->parser);
}

static unsigned user_process(user_prawtos_st * state){
    unsigned ret = USER_WRITE;
    // enum user_response_status status = check_user(state);
    // if(auth_marshal(state->write_buff,status) == -1){
    //     ret = ERROR;
    // }
    // state->status = status;
    return ret;
}

unsigned user_read(selector_key * key){
    unsigned ret = USER_READ;
    user_prawtos_st * state = &((struct prawtos *) key->data)->client.user; //chck
    bool error = false;
    size_t count;

    uint8_t * ptr = buffer_write_ptr(state->read_buff,&count);
    ssize_t n = recv(key->fd,ptr,count,0);
    if (n > 0){
        buffer_write_adv(state->read_buff,n);
        int st = user_consume(state->read_buff,&state->parser,&error);
        if(typ_is_done(st,0)){
            if (SELECTOR_SUCCESS == selector_set_interest_key(key, OP_WRITE))
            {
                //ret = user_process(state);
                ret = DONE;
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

unsigned user_write(selector_key *key) {
    user_prawtos_st * state = &((struct prawtos *) key->data)->client.user;
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