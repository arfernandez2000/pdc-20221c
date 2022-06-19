#ifndef PRAWTOSUTILS_H
#define PRAWTOSUTILS_H

#include "../buffer/buffer.h"
#include "../parser/prawtos_typ_parser.h"
#include "../stm/stm.h"
#include <sys/socket.h>
#include "../parser/auth_parser.h"
#define BUFFER_SIZE 4096

enum prawtos_state
{
    AUTH_READ = 0,
    AUTH_WRITE,
    TYP_READ,
    TYP_WRITE,
    DONE,
    ERROR,
    GET_READ,
    GET_WRITE,
    USER_READ,
    USER_WRITE
};

typedef struct auth_prawtos_st{
    buffer *read_buff, *write_buff;
    auth_parser parser;
    uint8_t ulen;
    uint8_t uname[MAX_LEN];
    uint8_t plen;
    uint8_t passwd[MAX_LEN];
    uint8_t status;
} auth_prawtos_st;

typedef struct typ_prawtos_st {
    buffer *read_buff, *write_buff;
    typ_parser parser;
    enum typ_response_status status;
} typ_prawtos_st;


struct prawtos {
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len;
    int client_fd;

    state_machine stm;

    union{
        auth_prawtos_st auth;
        typ_prawtos_st typ;
    } client;

    /** buffers para write y read **/
    uint8_t raw_buff_a[BUFFER_SIZE], raw_buff_b[BUFFER_SIZE];
    buffer read_buffer, write_buffer;
};

#endif