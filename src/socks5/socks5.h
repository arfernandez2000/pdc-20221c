#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <time.h>

#include "../selector/selector.h"
#include "../buffer/buffer.h"
#include "../args/args.h"
#include "socks5utils.h"

#ifndef SOCKS5_H
#define SOCKS5_H

#define INPUT_BUFFER_SIZE 100
#define OUTPUT_BUFFER_SIZE 100

void initialize_socks5();
void new_connection_ipv4();
static void initialize_state_machine(state_machine * stm);
static void server_write(selector_key * event);
static void server_read(selector_key * event);
static void server_close(selector_key * event);
static void close_session(selector_key * event);
static void client_close(selector_key *event);
static void client_read(selector_key  *event);
static void client_write(selector_key * event);
static Session * initialize_session();


#endif