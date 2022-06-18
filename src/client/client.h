#ifndef CLIENT_H
#define CLIENT_H

#include "../args/args.h"

static int connect_by_ipv4(struct in_addr ip, in_port_t port);
static int connect_by_ipv6(struct in6_addr ip, in_port_t port);


#endif
