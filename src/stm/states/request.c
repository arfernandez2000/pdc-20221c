#include "../../../socks5/socks5utils.h"
#include "../../stm_initialize.h"
#include "../../../parser/request_parser.h"

state_definition request_read_state_def(void) {

    state_definition state_def = {
        .state = REQUEST_READ,
        .on_arrival = request_arrival,
        .on_read_ready = request_read,
        .on_write_ready = NULL,
        .on_block_ready = NULL,
        .on_departure = NULL,
    };

    return state_def;
}

state_definition request_write_state_def(void) {

    state_definition state_def = {
        .state = REQUEST_WRITE,
        .on_arrival = NULL,
        .on_read_ready = NULL,
        .on_write_ready = request_write,
        .on_block_ready = NULL,
        .on_departure = NULL,
    };

    return state_def;
}

state_definition request_resolve_state_def(void) {

    state_definition state_def = {
        .state = REQUEST_RESOLVE,
        .on_arrival = NULL,
        .on_read_ready = NULL,
        .on_write_ready = request_resolve,
        .on_block_ready = NULL,
        .on_departure = NULL,
    };

    return state_def;
}

state_definition request_connecting_state_def(void) {

    state_definition state_def = {
        .state = REQUEST_CONNECTING,
        .on_arrival = request_connecting_arrival,
        .on_read_ready = request_connecting,
        .on_write_ready = NULL,
        .on_block_ready = NULL,
        .on_departure = NULL,
    };

    return state_def;
}

static void request_arrival(selector_key * event)
{
    struct request_st * state = &((Session *) (event->data)) ->socks.request;
    state->read_buff = &((Session *) (event->data)) ->input;
    state->write_buff =  &((Session *) (event->data)) ->output;
    state->parser.request = state->request;
    state->method = status_general_socks_server_failure;
    request_parser_init(&state->parser);
    
    state->client.fd = &((Session *) (event->data)) ->client.fd;
    state->server.fd =  &((Session *) (event->data)) ->server.fd;

    state->server.address = &((Session *) (event->data)) ->server.address;
    state->server.domain = &((Session *) (event->data)) ->server.domain;
}

static unsigned request_process(selector_key * event, struct request_st * state)
{
    unsigned ret;

    switch (state->parser->request.cmd)
    {
    case socks_req_cmd_connect:
        switch (state->parser->request.dest_addr_type)
        {
        case socks_addr_type_ipv4:
        {
            &((Session *) (event->data))->server.domain = AF_INET;
            state->parser->request.dest_addr.ipv4.sin_port = state->parser->request.dest_port;
            memcpy(&((Session *) (event->data))->server.address, &state->parser->request.dest_addr,
                   sizeof(state->parser->request.dest_addr.ipv4));
            ret = request_connect(event, state);
            break;
        }
        case socks_addr_type_ipv6:
        {
             &((Session *) (event->data))->server.domain = AF_INET6;
            state->parser->request.dest_addr.ipv6.sin6_port = state->parser->request.dest_port;
            memcpy(&((Session *) (event->data))->server.address, &state->parser->request.dest_addr,
                   sizeof(state->parser->request.dest_addr.ipv6));
            ret = request_connect(key, d);
            break;
        }
        case socks_addr_type_domain:
        {
            break;
        }
        default:
        {
            ret = REQUEST_WRITE;
            state->method = status_address_type_not_supported;
            selector_set_interest_key(event, OP_WRITE);
        }
        }
        break;

    case socks_req_cmd_bind:
    // Unsupported
    case socks_req_cmd_associate:
    // Unsupported
    default:
        state->method = status_command_not_supported;
        ret = REQUEST_WRITE;
        break;
    }

    return ret;
}


static unsigned request_read(selector_key * event) {
    struct hello_st * state = &((Session *) (event->data))->socks.request;
    unsigned ret = REQUEST_READ;
    bool error = false;
    uint8_t *ptr;
    size_t count;

    ptr = buffer_write_ptr(state->read_buff, &count);
    ssize_t n = recv(event->fd, ptr, count, 0);
    if (n > 0)
    {
        buffer_write_adv(state->read_buff, n);
        const enum request_state curr_state = request_consume(state->read_buff, &state->parser, &error);
        if (request_is_done(curr_state, 0))
        {
            ret = request_process(event, state);
        }
    }
    else
    {
        ret = ERROR;
    }

    return error ? ERROR : ret;
}

static unsigned request_write(struct selector_key *key) {
    struct request_st *state = &((Session *) (event->data))->socks.request;

    buffer *b = d->wb;
    unsigned ret = REQUEST_WRITE;
    uint8_t *ptr;
    size_t count;
    ssize_t n;
    
    ptr = buffer_read_ptr(state->write_buff, &count);
    n = send(event->fd, ptr, count, MSG_NOSIGNAL);
    if (n == -1) {
        ret = ERROR;
    }
    else {
        buffer_read_adv(state->write_buff, n);
        if (!buffer_can_read(state->write_buff))
        {
            if (SELECTOR_SUCCESS == selector_set_interest_key(event, OP_READ)) {
                ret = COPY;
            }
            else {
                ret = ERROR;
            }
        }
    }
    return ret;
}