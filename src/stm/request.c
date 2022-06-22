#include "../../include/socks5utils.h"
#include "../../include/parser/request_parser.h"
#include "../../include/socks5.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <string.h>
#include "../../include/register.h"
//#include <pthread.h>

static void request_arrival(const unsigned st, selector_key * event);
static unsigned request_process(selector_key * event, request_st * state);
static unsigned request_read(selector_key * event);
static unsigned request_write(selector_key *event);
static void request_connecting_arrival(const unsigned int st, selector_key *event);
static unsigned request_connect(selector_key *event, request_st *state);
static unsigned request_connecting(selector_key *event);
//static unsigned request_resolve_done(selector_key *key);


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

// state_definition request_resolve_state_def(void){

//     state_definition state_def = {
//         .state = REQUEST_RESOLVE,
//         .on_arrival = NULL,
//         .on_read_ready = NULL,
//         .on_write_ready = NULL,
//         .on_block_ready = request_resolve_done,
//         .on_departure = NULL,
//     };

//     return state_def;
    
// }

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

const fd_handler client_handler_request = {
    .handle_read   = client_read,
    .handle_write  = client_write,
    .handle_close  = client_close,
};

state_definition request_connecting_state_def(void) {

    state_definition state_def = {
        .state = REQUEST_CONNECTING,
        .on_arrival = request_connecting_arrival,
        .on_read_ready = NULL,
        .on_write_ready = request_connecting,
        .on_block_ready = NULL,
        .on_departure = NULL,
    };

    return state_def;
}

// static void *request_resolv_blocking(void *data) {
//     selector_key *key = (struct selector_key *)data;
//     Session * state = ((Session *) (data));

//     pthread_detach(pthread_self());
//     state->origin_resolution = 0;

//     struct addrinfo hints = {
//         .ai_family = AF_UNSPEC,
//         .ai_socktype = SOCK_STREAM,
//         .ai_flags = AI_PASSIVE,
//         .ai_protocol = 0,
//         .ai_canonname = NULL,
//         .ai_addr = NULL,
//         .ai_next = NULL,
//     };

//     char buff[7]; 
//     snprintf(buff, sizeof(buff), "%d",
//              ntohs(state->client_header.request.request.dest_port));

//     if(getaddrinfo(state->client_header.request.request.dest_addr.fdqn, buff, &hints,&state->origin_resolution)==1){
//                     perror("Error making getaddrinfo\n");
//     }
//     state->origin_resolution_current = state->origin_resolution;

//     selector_notify_block(key->s, key->fd);

//     free(data);

//     return 0;
// }

static void request_arrival(const unsigned st, selector_key * event)
{
    request_st * state = &((Session *) (event->data))->client_header.request;
    state->read_buff = &((Session *) (event->data))->input;
    state->write_buff =  &((Session *) (event->data))->output;
    state->parser.request = &state->request;
    state->status = status_general_SOCKS_server_failure;
    request_parser_init(&state->parser);
    
    state->client.fd = ((Session *) (event->data))->client.fd;
    state->server.fd =  ((Session *) (event->data))->server.fd;
    state->server.address = ((Session *) (event->data)) ->server.address;
    state->server.address_len = ((Session *) (event->data)) ->server.address_len;
    state->server.domain = ((Session *) (event->data)) ->server.domain;
}

// static unsigned request_resolve_done(selector_key *key) {
//     struct request_st *d =  &((Session *) (key->data))->client_header.request;
//     struct Session *s = ((Session *) (key->data));
//     if (s->origin_resolution == 0) {
//         d->status = status_host_unreachable;
//         s->register_info.status = d->status;
//         if (-1 != request_marshal(d->write_buff, d->status, d->request.dest_addr_type,
//                                    d->request.dest_addr,
//                                    d->request.dest_port)) {
//             selector_set_interest(key->s, d->client.fd, OP_WRITE);
//             return REQUEST_WRITE;
//         } else {
//             return ERROR;
//         }

//     } else {
//         s->server.domain = s->origin_resolution_current->ai_family;
//         s->server.address_len = s->origin_resolution_current->ai_addrlen;
//         memcpy(&s->server.address, s->origin_resolution_current->ai_addr,
//                s->origin_resolution_current->ai_addrlen);
//     }

//     return request_connect(key, d);
// }

static unsigned request_process(selector_key * event, struct request_st * state)
{
    unsigned ret = REQUEST_CONNECTING;

    switch (state->request.cmd)
    {
    case socks_req_cmd_connect:
        ((Session *) (event->data))->register_info.atyp = state->request.dest_addr_type;
        switch (state->request.dest_addr_type)
        {
        case socks_addr_type_ipv4:
        {
            ((Session *) (event->data))->server.domain = AF_INET;
            state->request.dest_addr.ipv4.sin_port = state->request.dest_port;
            ((Session *) (event->data))->server.address_len = sizeof(state->request.dest_addr.ipv4);
            memcpy(&((Session *) (event->data))->server.address, &state->request.dest_addr,
                   sizeof(state->request.dest_addr.ipv4));

            memcpy(&((Session *) (event->data))->register_info.dest_addr, &state->request.dest_addr,
                   sizeof(state->request.dest_addr));
            ((Session *) (event->data))->register_info.dest_port = state->request.dest_port;

            ret = request_connect(event, state);
            break;
        }
        case socks_addr_type_ipv6:
        {
            ((Session *) (event->data))->server.domain = AF_INET6;
            state->request.dest_addr.ipv6.sin6_port = state->request.dest_port;
            ((Session *) (event->data))->server.address_len = sizeof(state->request.dest_addr.ipv6);
            memcpy(&((Session *) (event->data))->server.address, &state->request.dest_addr,
                   sizeof(state->request.dest_addr.ipv6));

            memcpy(&((Session *) (event->data))->register_info.dest_addr, &state->request.dest_addr,
                   sizeof(state->request.dest_addr));
            ((Session *) (event->data))->register_info.dest_port = state->request.dest_port;


            ret = request_connect(event, state);
            break;
        }
        case socks_addr_type_domain:
        // {
        //     pthread_t thr;
        //     selector_key *k = malloc(sizeof(*k));
        //         if (k == NULL) {
        //            ret = REQUEST_WRITE;
        //            state->status = status_general_SOCKS_server_failure;
        //            selector_set_interest_key(event, OP_WRITE);
        //         } else {
        //             memcpy(k, event, sizeof(*k));
        //             if (-1 == pthread_create(&thr, 0, request_resolv_blocking, k)) {
        //                 ret = REQUEST_WRITE;
        //                 state->status = status_general_SOCKS_server_failure;
        //                 selector_set_interest_key(event, OP_WRITE);
        //                 ((Session *) (event->data))->register_info.status = state->status;
        //             } else {
        //                 ret = REQUEST_RESOLVE;
        //                 // hasta que no resuelva el nombre, no hay que hacer nada
        //                 selector_set_interest_key(event, OP_NOOP);
        //                 memcpy(((Session *) (event->data))->register_info.dest_addr.fdqn,state->request.dest_addr.fdqn,sizeof(state->request.dest_addr.fdqn));
        //                 ((Session *) (event->data))->register_info.dest_port = state->request.dest_port;
        //             }
        //         }
                break;
        // }
        default:
        //{
            ret = REQUEST_WRITE;
            state->status = status_address_type_not_supported;
            selector_set_interest_key(event, OP_WRITE);
        //}
        }
            break;
        break;
    case socks_req_cmd_bind:
    // Unsupported
    case socks_req_cmd_associate:
    // Unsupported
    default:
        state->status = status_command_not_supported;
        ret = REQUEST_WRITE;
        break;
    }

    return ret;
}


static unsigned request_read(selector_key * event) {
    request_st * state = &((Session *) (event->data))->client_header.request;
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

static unsigned request_write(selector_key *event) {
    request_st *state = &((Session *) (event->data))->client_header.request;

    buffer *b = state->write_buff;
    unsigned ret = REQUEST_WRITE;
    uint8_t *ptr;
    size_t count;
    
    ptr = buffer_read_ptr(state->write_buff, &count);
    ssize_t n = send(event->fd, ptr, count, MSG_NOSIGNAL);
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
            log_access(&((Session *) (event->data))->register_info);
        }
    }
    return ret;
}


static void request_connecting_arrival(const unsigned int st, selector_key *event)
{
    connect_st *state = &((Session *) (event->data))->server_header.conect;
    state->client_fd = &((Session *) (event->data))->client.fd;
    state->origin_fd = &((Session *) (event->data))->server.fd;
    state->status = &((Session *) (event->data))->client_header.request.status;
    state->write_buff= &((Session *) (event->data))->output;
}


static unsigned request_connect(selector_key *event, request_st *state)
{
    bool error = false;
    bool fd_registered = false;
    struct Session *session = ((Session *) (event->data));
    int *fd = &state->server.fd;

    if(*fd != -1) {
        fd_registered = true;

        if(close(*fd) == -1) {
            error = true;
            goto finally;
        }
    }

    unsigned ret = REQUEST_CONNECTING;
    *fd = socket(session->server.domain, SOCK_STREAM, 0);
    session->server.fd = *fd;
    if (*fd == -1)
    {
        error = true;
        goto finally;
    }

    if (selector_fd_set_nio(*fd) == -1)
    {
        goto finally;
    }

    if (connect(*fd, (const struct sockaddr *)&session->server.address,
               session->server.address_len)  == -1)
    {
        if (errno == EINPROGRESS)
        {
            selector_status st = selector_set_interest_key(event, OP_NOOP);
            if (st != SELECTOR_SUCCESS)
            {
                error = true;
                goto finally;
            }
            if(!fd_registered) {
                st = selector_register(event->s, *fd, &client_handler_request, OP_WRITE, event->data);
            }
            else {
                st = selector_set_interest(event->s, *fd, OP_WRITE);
            }

            if (st != SELECTOR_SUCCESS)
            {
                error = true;
                goto finally;
            }
        }
        else
        {
            session->client_header.request.status = errno_to_socks(errno);
            if (-1 != request_marshal(session->client_header.request.write_buff, session->client_header.request.status, session->client_header.request.request.dest_addr_type, session->client_header.request.request.dest_addr, session->client_header.request.request.dest_port))
            {
                selector_set_interest(event->s, session->client.fd, OP_WRITE);
                selector_status st = selector_register(event->s, *fd, &client_handler_request, OP_NOOP, event->data); // registro el nuevo fd pero lo seteo en NOOP porque no se pudo establecer la conexión
                if (st != SELECTOR_SUCCESS)
                {
                    error = true;
                    goto finally;
                }
                
                ret = REQUEST_WRITE;
            }
            else {
                error = true;
            }
            ((Session *) (event->data))->register_info.status = session->client_header.request.status;   
            goto finally;
        }
    }

finally:
    return error ? ERROR : ret;
}

static unsigned request_connecting(selector_key *event)
{
    int error;
    socklen_t len = sizeof(error);
    unsigned ret = REQUEST_CONNECTING;

    struct Session *session = ((Session *) (event->data));
    int *fd = &session->server.fd;
    int ret_sock = getsockopt(*fd, SOL_SOCKET, SO_ERROR, &error, &len);
    if (ret_sock == 0)
    {
        selector_set_interest(event->s, *session->server_header.conect.client_fd, OP_WRITE);
        
        if (error == 0)
        {
            session->client_header.request.status = status_succeeded;
            //set_historical_conections(get_historical_conections() +1);
        // } else {
        //     session->client_header.request.status = errno_to_socks(error);

        //     if(SELECTOR_SUCCESS != selector_set_interest_key(event, OP_NOOP)) {
        //         return ERROR;
        //     }
        //     ((Session *) (event->data))->register_info.status = session->client_header.request.status;
        //     log_access(&((Session *) (event->data))->socks_info);

        //     return REQUEST_RESOLVE;
        }
        if (-1 != request_marshal(session->client_header.request.write_buff, session->client_header.request.status, session->client_header.request.request.dest_addr_type, session->client_header.request.request.dest_addr, session->client_header.request.request.dest_port))
        {
            selector_set_interest(event->s, *session->server_header.conect.origin_fd, OP_READ);
            
            ret = REQUEST_WRITE;
        }
        else {
            ret = ERROR;
        }
        ((Session *) (event->data))->register_info.status = session->client_header.request.status;    
    }
    return ret;
}