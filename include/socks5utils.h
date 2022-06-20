#ifndef SOCKS5UTILS_H
#define SOCKS5UTILS_H

#include "buffer.h"
#include "stm.h"
#include "defs.h"
#include "user_utils.h"
#include "hello.h"
#include "parser/hello_parser.h"
#include "parser/request_parser.h"
#include "parser/auth_parser.h"
#include "selector.h"
#include "args.h"
#include <netinet/in.h>

typedef enum socket_state{
    INVALID,
    OPEN,
    CLOSING,
    CLOSED,
}socket_state;

typedef struct Connection {
    int fd;
    struct sockaddr_storage address;
    socket_state state;
    int domain;
    socklen_t address_len;

} Connection;

typedef struct hello_st
{
    buffer *read_buff, *write_buff;
    struct hello_parser parser;
    uint8_t method;
    
} hello_st;

typedef struct auth_st {
    buffer *read_buff, *write_buff;
    auth_parser parser;
    auth auth;
    uint8_t status;
    struct users user;
    uint8_t method;
} auth_st;

typedef struct request_st
{
    buffer *read_buff, *write_buff;
    struct request_parser parser;
    struct request request;

    enum socks_response_status status;

    Connection client;
    Connection server;
    
} request_st;

typedef struct connect_st
{
    buffer *write_buff;
    const int *client_fd;
    int *origin_fd;
    enum socks_response_status *status;

} connect_st;



enum session_state{
    HELLO_READ = 0,
    HELLO_WRITE,
    REQUEST_READ,
    REQUEST_CONNECTING,
    //REQUEST_RESOLVE,
    REQUEST_WRITE,
    COPY,
    ERROR,
    DONE,
    AUTH_READ,
    AUTH_WRITE,
};

typedef struct copy_st
{
    int *fd;
    buffer *read_buff, *write_buff;
    fd_interest duplex;

    struct copy_st *other;
}copy_st;

union client_header {
    hello_st hello;    
    auth_st auth;
    request_st request;
    copy_st copy;
};

union server_header {
    connect_st conect;
    copy_st copy;
};

typedef struct register_st{
    uint8_t method;
    User user_info;
    enum socks_response_status status;
    enum socks_addr_type atyp;
    struct sockaddr_storage client_addr;
    union socks_addr dest_addr;
    in_port_t dest_port;

    //Sniffer
    char *user;
    char *passwd;
} register_st;

typedef struct Session {

    state_machine s_machine;

    buffer input;
    buffer output;

    Connection client;
    Connection server;

    register_st register_info;

    union client_header client_header;
    union server_header server_header;



    time_t lastModified;
} Session;

#endif
