#ifndef PRAWTOSUTILS_H
#define PRAWTOSUTILS_H

#include "../buffer/buffer.h"
#include "../parser/prawtos_typ_parser.h"
#include "../stm/stm.h"
#include <sys/socket.h>
#include "../parser/auth_parser.h"

#include "../parser/prawtos_parser.h"

#define BUFFER_SIZE 4096

enum prawtos_state
{
    AUTH_READ = 0,
    AUTH_WRITE,
    CMD_READ,
    CMD_WRITE,
    // GET_READ,
    // GET_WRITE,
    // USER_READ,
    // USER_WRITE,
    DONE,
    ERROR,
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
        auth_prawtos_st auth;
        cmd_prawtos_st cmd;
    } client;

    /** buffers para write y read **/
    uint8_t raw_buff_a[BUFFER_SIZE], raw_buff_b[BUFFER_SIZE];
    buffer read_buffer, write_buffer;
};

buffer aux;

#endif