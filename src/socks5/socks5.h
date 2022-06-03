#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include "../selector/selector.h"
#include "../args/args.h"

#ifndef SOCKS5_H
#define SOCKS5_H

#define INPUT_BUFFER_SIZE 100
#define OUTPUT_BUFFER_SIZE 100

void initialize_socks5();
void new_connection_ipv4();

#endif