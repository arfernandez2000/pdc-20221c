#include "../buffer/buffer.h"
#include "../stm/stm.h"

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
} Connection;

typedef struct Session {

    state_machine s_machine;

    buffer input;
    buffer output;

    Connection client;

    time_t lastModified;
} Session;
