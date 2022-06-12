#include "../buffer/buffer.h"
#include "../stm/stm.h"
#include "../defs.h"
#include "../user/user_utils.h"
#include "../stm/states/hello/hello.h"
#include "../parser/hello_parser.h"
#include <netinet/in.h>

typedef struct hello_st
{
    buffer *read_buff, *write_buff;
    struct hello_parser parser;
    uint8_t method;
} hello_st;


typedef enum socket_state{
    INVALID,
    OPEN,
    CLOSING,
    CLOSED,
}socket_state;

enum session_state{
    HELLO_READ = 0,
    HELLO_WRITE,
    AUTH_READ,
    AUTH_WRITE,
    REQUEST_READ,
    REQUEST_RESOLVE,
    REQUEST_CONNECTING,
    REQUEST_WRITE,
    COPY,
    ERROR,
    DONE,
};


typedef struct Connection {
    int fd;
    struct sockaddr_storage address;
    socket_state state;
} Connection;


typedef union Socks_header {
    hello_st hello;    
//    auth_st auth;
//    request_st request;
//    copy copy;
} SocksHeaders;

typedef struct Client{
    uint8_t authentication;
    User * user;
    address_type address;
    in_port_t port;
    char * domain;
} Client;

typedef struct Session {

    state_machine s_machine;

    buffer input;
    buffer output;

    Connection client;

    Client client_information;
    union Socks_header socks;

    time_t lastModified;
} Session;