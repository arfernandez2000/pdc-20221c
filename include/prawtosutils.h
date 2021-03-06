#ifndef PRAWTOSUTILS_H
#define PRAWTOSUTILS_H

#include "buffer.h"
#include "stm.h"
#include <sys/socket.h>
#include "parser/auth_parser.h"
#include "cmd_prawtos.h"
#include "parser/prawtos_parser.h"
#include "socks5utils.h"

#define BUFFER_SIZE 4096

enum prawtos_state
{
    AUTH_READ_PRAWTOS = 0,
    AUTH_WRITE_PRAWTOS,
    CMD_READ_PRAWTOS,
    CMD_WRITE_PRAWTOS,
    DONE_PRAWTOS,
    ERROR_PRAWTOS,
};

// typedef struct auth_prawtos_st{
//     buffer *read_buff, *write_buff;
//     auth_parser parser;
//     uint8_t ulen;
//     uint8_t uname[MAX_LEN];
//     uint8_t plen;
//     uint8_t passwd[MAX_LEN];
//     uint8_t status;
// } auth_prawtos_st;

typedef struct cmd_prawtos_st {
    buffer *read_buff, *write_buff;
    prawtos_parser parser;
    get get;
    user_st user;
    enum prawtos_response_status status;
    uint8_t * args;
    uint8_t nargs;
} cmd_prawtos_st;

struct prawtos {
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len;
    int client_fd;

    state_machine stm;

    union{
        auth_st auth;
        cmd_prawtos_st cmd;
    } client;

    /** buffers para write y read **/
    uint8_t raw_buff_a[BUFFER_SIZE], raw_buff_b[BUFFER_SIZE];
    buffer read_buffer, write_buffer;
};

#endif