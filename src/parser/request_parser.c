/**
 * request_parser.c -- parser del request de SOCKS5
 */
#include <string.h>
#include <arpa/inet.h>

#include "request_parser.h"

static void 
remaining_set (struct request_parser *p, const int n){
    p->i = 0;
    p->n = n;
}

static int
remaining_is_done (struct request_parser *p) {
    return p->i >= p->n;
}

static enum request_state
version (const uint8_t c, struct request_parser *p) {
    enum request_state next;
    switch (c) {
        case 0x05:
            next = request_cmd;
            break;
        default:
            next = request_error_unsupported_version;
            break;
    }
    return next;
}

static enum request_state
cmd (const uint8_t c, struct request_parser *p){
    p->request->cmd = c;
    return request_rvs;
}

static enum request_state
rsv (const uint8_t c, struct request_parser *p){
    return request_atype;
}

static enum request_state
atype (const uint8_t c, struct request_parser * p) {
    enum request_state next;
    p->request->dest_addr_type = c;
    switch (p->request->dest_addr_type)
    {
    case socks_addr_type_ipv4:
        remaining_set(p, 4);
        memset(&(p->request->dest_addr.ipv4), 0, sizeof(p->request->dest_addr.ipv4));
        p->request->dest_addr.ipv4.sin_family = AF_INET;
        next = request_dstaddr;
        break;
    case socks_addr_type_ipv6:
        remaining_set(p, 16);
        memset(&(p->request->dest_addr.ipv6), 0, sizeof(p->request->dest_addr.ipv6));
        p->request->dest_addr.ipv6.sin6_addr = AF_INET6;
        next = request_dstaddr;
        break;
    case socks_addr_type_domain:
        next = request_dstaddr_fqdn;
        break;
    default:
        next = request_error_unsupported_atype;
        break;
    }
    return next;
}

static enum request_state
dstaddr_fqdn (const uint8_t c, struct request_parser *p){
    remaining_set(p, c);
    p->request->dest_addr.fdqn[p->n - 1] = 0;
    return request_dstaddr;
}

static enum request_state
dstaddr (const uint8_t c, struct request_parser *p){
    enum request_state next;

    switch (p->request->dest_addr_type) 
    {
    case socks_addr_type_ipv4:
        ((uint8_t *)&(p->request->dest_addr.ipv4.sin_addr))[p->i++] = c;
        break;
    case socks_addr_type_ipv6:
        ((uint8_t *)&(p->request->dest_addr.ipv6.sin6_addr))[p->i++] = c;
        break;
    case socks_addr_type_domain:
        p->request->dest_addr.fdqn[p->i++] = c;
        break;
    }
    if (remaining_is_done(p))
    {
        remaining_set(p, 2);
        p->request->dest_port = 0;
        next = request_dstport;
    } else {
        next = request_dstaddr;
    }

    return next;
    
}

static enum request_state
dstport (const uint8_tc, struct request_parser *p) {
    enum request_state next = request_dstport;
    *(((uint8_t *)&(p->request->dest_port)) + p->i) = c;
    p->i++;
    if(remaining_is_done(p)) {
        next = request_done;
    }
    return next;
}

extern enum request_state 
request_parser_feed (struct request_parser *p, const uint8_t c) {
    enum request_state next;

    switch (p->state)
    {
    case request_version:
        next = version(c, p);
        break;
    case request_cmd:
        next = cmd(c, p);
        break;
    case request_rvs:
        next = rsv(c, p);
        break;
    case request_atype:
        next = atype(c, p);
        break;
    case request_dstaddr_fqdn:
        next = dstaddr_fqdn(c, p);
        break;
    case request_dstaddr:
        next = dstaddr(c, p);
        break;
    case request_dstport:
        next = dstport(c, p);
        break;
    case request_done:
    case request_error:
    case request_error_unsupported_version:
    case request_error_unsupported_atype:
        next = p->state;
        break;
    default:
        next = request_error;
        break;
    }
    p->state = next;
    return p->state;
}

void 
request_parser_init(struct request_parser *p) {
    p->state = request_version;
    memset(p->request, 0, sizeof(*(p->request)));
}

enum request_state
request_consume (buffer *b, struct request_parser *p, bool *errored) {
    enum request_state st = p->state;
    bool finished = false;
    while (buffer_can_read(b) && !finished)
    {
        uint8_t byte = buffer_read(b);
        st = request_parser_feed(p, byte);
        if(request_is_done(st, errored)) {
            finished = true;
        }
    }
    return st;
}

int 
request_marshal(buffer *b, const enum socks_response_status status, const enum socks_addr_type atyp, const union socks_addr addr, const in_port_t dest_port) {
    size_t n, len = 6;
    uint8_t *buff = buffer_write_ptr(b, &n);
    uint8_t *aux = NULL;
    int addr_size = 0;
    switch (atyp)
    {
    case socks_addr_type_ipv4:
        addr_size = 4;
        len += addr_size;
        aux = (uint8_t *)malloc(4 * sizeof(uint8_t));
        memcpy(aux, &addr.ipv4.sin_addr, 4);
        break;
    case socks_addr_type_ipv6:
        addr_size = 16;
        len += addr_size;
        aux = (uint8_t *)malloc(16 * sizeof(uint8_t));
        memcpy(aux, &addr.ipv6.sin6_addr, 16);
        break;
    case socks_addr_type_domain:
        addr_size = strlen(addr.fqdn);
        aux = (uint8_t *)malloc((addr_size + 1) * sizeof(uint8_t));
        aux[0] = addr_size;
        memcpy(aux + 1, addr.fdqn, addr_size);
        addr_size++;
        len += addr_size;
        break;
    }
    if (n < len)
    {
        free(aux);
        return -1;
    }
    buff[0] = 0x05;
    buff[1] = status;
    buff[2] = 0x00;
    buff[3] = atyp;
    memcpy(&buff[4], aux, addr_size);
    free(aux);
    memcpy(&buff[4 + addr_size], &dest_port, 2);
    buffer_write_adv(b, len);
    return len;
}

bool request_is_done(const enum request_state state, bool *errored)
{
    bool ret = false;
    if (state == request_error ||
        state == request_error_unsupported_version ||
        state == request_error_unsupported_atype)
    {
        if (errored != 0)
        {
            *errored = true;
        }
        ret = true;
    }
    else if (state == request_done)
    {
        ret = true;
    }
    return ret;
}

enum socks_response_status errno_to_socks(int e)
{
    enum socks_response_status ret = status_general_SOCKS_server_failure;

    switch (e)
    {
    case 0:
        ret = status_succeeded;
        break;
    case ECONNREFUSED:
        ret = status_connection_refused;
        break;
    case EHOSTUNREACH:
        ret = status_host_unreachable;
        break;
    case ENETUNREACH:
        ret = status_network_unreachable;
        break;
    case ETIMEDOUT:
        ret = status_TTL_expired;
        break;
    }

    return ret;
}

enum socks_response_status cmd_resolve(struct request *request, struct sockaddr **originaddr, socklen_t *originlen, int *domain)
{
    enum socks_response_status ret = status_general_SOCKS_server_failure;

    *domain = AF_INET;
    struct sockaddr *addr = 0x00;
    socklen_t addrlen = 0;

    switch (request->dest_addr_type)
    {
    case socks_addr_type_domain:
    {
        struct hostent *hp = gethostbyname(request->dest_addr.fqdn);
        if (hp == 0)
        {
            memset(&request->dest_addr, 0x00, sizeof(request->dest_addr));
            break;
        }
        request->dest_addr.ipv4.sin_family = hp->h_addrtype;
        memcpy((char *)&request->dest_addr.ipv4.sin_addr, *hp->h_addr_list, hp->h_length);
    }
    // fall through
    case socks_addr_type_ipv4:
    {
        *domain = AF_INET;
        addr = (struct sockaddr *)&(request->dest_addr.ipv4);
        addrlen = sizeof(request->dest_addr.ipv4);
        request->dest_addr.ipv4.sin_port = request->dest_port;
        break;
    }
    case socks_addr_type_ipv6:
    {
        *domain = AF_INET6;
        addr = (struct sockaddr *)&(request->dest_addr.ipv6);
        addrlen = sizeof(request->dest_addr.ipv6);
        request->dest_addr.ipv6.sin6_port = request->dest_port;
        break;
    }
    default:
        return status_address_type_not_supported;
    }

    *originaddr = addr;
    *originlen = addrlen;

    return ret;
}

