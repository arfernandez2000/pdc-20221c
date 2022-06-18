#ifndef CLIENT_H
#define CLIENT_H

#include <stdbool.h>

typedef struct arg_st {
    char *address;
    int port;
    bool sniff;
    char *user;
    char *pass;
} arg_st;


static int connect_by_ipv4(struct in_addr ip, in_port_t port);
static int connect_by_ipv6(struct in6_addr ip, in_port_t port);
void parse_args(int argc, char* argv[], arg_st *arguments);
void print_help();
void set_user(char *str, arg_st *arguments);
void print_version_info();
int set_port(char *port);


#endif
