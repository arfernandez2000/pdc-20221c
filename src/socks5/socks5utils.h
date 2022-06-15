#include "../buffer/buffer.h"
#include "../stm/stm.h"
#include "../defs.h"
#include "../user/user_utils.h"
#include "../stm/states/hello/hello.h"
#include "../parser/hello_parser.h"
#include "../parser/request_parser.h"
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

} Connection;

typedef struct hello_st
{
    buffer *read_buff, *write_buff;
    struct hello_parser parser;
    uint8_t method;
    
} hello_st;


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
    ERROR,
    DONE,
    AUTH_READ,
    AUTH_WRITE,
    COPY,
};



union client_header {
    hello_st hello;    
//    auth_st auth;
   request_st request;
//    copy copy;
};

union server_header {
    connect_st conect;
};

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
    Connection server;

    Client client_information;
    union client_header client_header;
    union server_header server_header;

    time_t lastModified;
} Session;