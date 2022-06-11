#include "../buffer/buffer.h"
#include "../stm/stm.h"
#include "../../defs.h"
#include "../user/user_utils.h"

typedef enum socket_state{
    INVALID,
    OPEN,
    CLOSING,
    CLOSED,
}socket_state;

typedef enum session_state{
    HELLO = 0;
    HELLO_ERROR,
    AUTH_ANNOUNCEMENT,
    AUTH_REQUEST,
    AUTH_ERROR,
    AUTH_SUCCESFUL,
    REQUEST,
    REQUEST_ERROR,
    IP_CONNECT,
    DNS_QUERY,
    RESPONSE_DNS,
    DNS_CONNECT,
    REQUEST_SUCCESSFUL,
    FORWARDING,
    CLOSER,
    CLOSING,
    FINISH,
} session_state;


typedef struct Connection {
    int fd;
    struct sockaddr_storage address;
    socket_state state;
} Connection;

typedef struct hello_header
{
    //TODO MIRAR LA ESTRUCTURA DE LAS CHICAS
    hello_parser parser;
    sizer_t bytes;
};

typedef union Socks_header {
    HelloHeader helloHeader;    
    AuthRequestHeader authRequestHeader;
    RequestHeader requestHeader;
    SpoofingHeader spoofingHeader;
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
    Socks_header socks;

    time_t lastModified;
} Session;
