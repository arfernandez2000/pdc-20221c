#include "socks5.h"

static int inputBufferSize;
static int outPutBufferSize;

void initialize_socks5() {
    
    inputBufferSize = INPUT_BUFFER_SIZE;
    outPutBufferSize = OUTPUT_BUFFER_SIZE;

    
    // Aca habria que inicializar la maquina de estados.
}


void new_connection_ipv4(selector_key *event) {

    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);

    // int fd = accept_socket(event->fd, (struct sockaddr *)&cli_addr, &clilen)
    int fd;

    do {
        fd = accept(event->fd, (struct sockaddr *)&cli_addr, &clilen);
    } while (fd < 0 && (errno == EINTR));

    fprintf(stdout,"Mi file descriptor es: %d\n", fd);

}