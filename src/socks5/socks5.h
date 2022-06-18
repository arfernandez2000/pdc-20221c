
#ifndef SOCKS5_H
#define SOCKS5_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <time.h>

#include "../stm/stm.h"
#include "../selector/selector.h"

#define INPUT_BUFFER_SIZE 100
#define OUTPUT_BUFFER_SIZE 100

void initialize_socks5();
void new_connection_ipv4();
void new_connection_ipv6();
static void initialize_state_machine(state_machine * stm);
static void server_write(selector_key * event);
static void server_read(selector_key * event);
static void server_close(selector_key * event);
static void close_session(selector_key * event);
void client_close(selector_key *event);
void client_read(selector_key  *event);
void client_write(selector_key * event);


// const struct fd_handler server_handler = {
//     .handle_read   = server_read,
//     .handle_write  = server_write,
//     .handle_close  = server_close,
// };


#endif