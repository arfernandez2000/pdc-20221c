#include "hello.h"
#include "../../../socks5/socks5utils.h"
#include "../../stm_initialize.h"
#include "../../../selector/selector.h"
#include "../../../buffer/buffer.h"
#include "../../../parser/hello_parser.h"

static unsigned hello_process(const struct hello_st *d);


// callback del parser utilizado en 'read_hello'
static void on_hello_method(void *data, const uint8_t method)
{
    uint8_t *selected = (uint8_t *)data;
    if(*selected != METHOD_USERNAME_PASSWORD){
        if ((method == METHOD_NO_AUTHENTICATION_REQUIRED) || (method == METHOD_USERNAME_PASSWORD))
        {
            *selected = method;
        }
    }
}

static void hello_arrival(unsigned int st, selector_key * event){
    hello_st * state  = &((Session *) (event->data)) ->socks.hello;
    state->read_buff = &((Session *) (event->data)) ->input;
    state->write_buff =  &((Session *) (event->data)) ->output;
    hello_parser_init(&state->parser);
    state->parser.data = &state->method;
    state->parser.on_authentication_method = on_hello_method, hello_parser_init(&state->parser);
}

static unsigned hello_read(selector_key * event){
    struct hello_st * state = &((Session *) (event->data))->socks.hello;
    unsigned ret = HELLO_READ;
    bool error = false;
    uint8_t * ptr;
    size_t count;

    ptr = buffer_write_ptr(state->read_buff, &count);
    ssize_t n = recv(event->fd, ptr, count, 0);
    if (n > 0)
    {
        buffer_write_adv(state->read_buff, n);
        const enum hello_state curr_state = hello_consume(state->read_buff, &state->parser, &error);
        if (hello_is_done(curr_state, 0))
        {
            if (SELECTOR_SUCCESS == selector_set_interest_key(event, OP_WRITE))
            {
                ret = hello_process(state);
                // ((Session *) (event->data))->socks_info.method = state->method;
            }
            else
            {
                ret = ERROR;
            }
        }
    }
    else
    {
        ret = ERROR;
    }

    return error ? ERROR : ret;
}

static unsigned hello_write(selector_key *event)
{
    fprintf(stdout, "En el hello_write\n");
    struct hello_st * state = &((Session *) (event->data))->socks.hello;

    unsigned ret = HELLO_WRITE;
    uint8_t *ptr;
    size_t count;
    ssize_t n;

    ptr = buffer_read_ptr(state->read_buff, &count);
    n = send(event->fd, ptr, count, MSG_NOSIGNAL);
    if (n == -1)
    {
        ret = ERROR;
    }
    else
    {
        buffer_read_adv(state->write_buff, n);
        fprintf(stdout, "write_buff: %s\n", state->write_buff->write);
        fprintf(stdout, "write_buff: %s\n", state->write_buff->read);
        if (!buffer_can_read(state->write_buff))
        {   
            if (SELECTOR_SUCCESS == selector_set_interest_key(event, OP_READ))
            {
                if(state->method == METHOD_USERNAME_PASSWORD){
                    ret = AUTH_READ;
                }
                else{
                    //TODO: cambiarlo
                    //ret = REQUEST_READ;
                    ret = DONE;
                }
            }
            else
            {
                ret = ERROR;
            }
        }
    }

    return ret;
}

static unsigned hello_process(const struct hello_st * state)
{
    unsigned ret = HELLO_WRITE;

    uint8_t method = state->method;
    if (-1 == hello_marshall(state->write_buff, method))
    {
        ret = ERROR;
    }
    if (method == METHOD_NO_ACCEPTABLE_METHODS)
    {
        ret = ERROR;
    }

    return ret;
}


state_definition hello_state_def(void) {

    state_definition state_def = {
        .state = HELLO_READ,
        .on_arrival = hello_arrival,
        .on_read_ready = hello_read,
        .on_write_ready = NULL,
        .on_block_ready = NULL,
        .on_departure = NULL,
    };

    return state_def;
}

state_definition hello_write_state_def(void) {

    state_definition state_def = {
        .state = HELLO_WRITE,
        .on_arrival = NULL,
        .on_read_ready = NULL,
        .on_write_ready = hello_write,
        .on_block_ready = NULL,
        .on_departure = NULL,
    };

    return state_def;
}