#ifndef CLIENT_H
#define CLIENT_H

#include "../args/args.h"

void transfered_bytes(int fd);
void create_user(int fd);
void create_admin(int fd);

static int connect_by_ipv4(struct in_addr ip, in_port_t port);
static int connect_by_ipv6(struct in6_addr ip, in_port_t port);
void first_message(int fd, socks5args *args);
static void clean_buffer();
void options(int fd);
void add_new_user(int fd, bool admin);


#endif
