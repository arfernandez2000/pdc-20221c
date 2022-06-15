#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "auth_parser.h"

void auth_parser_init (auth_parser *p)
{
    p->state = auth_version;
    memset(&p->auth, 0, sizeof(p->auth));
    p->n = 0;
    p->i = 0;
}


static void remaining_set(auth_parser *p, const int n) {
    p->i = 0;
    p->n = n;
}


static int remaining_is_done(auth_parser *p) {
    return p->i >= p->n;
}

static enum auth_state
version (const uint8_t c, auth_parser *p) {
    enum auth_state next;
    switch (c) {
        case 0x01:
            next = auth_ulen;
            break;
        default:
            next = auth_error_unsupported_version;
            break;
    }
    return next;
}

static enum auth_state 
ulen (const uint8_t c, auth_parser *p) {
    if(c <= 0) {
        p->state = auth_error;
        return p->state;
    }

    remaining_set(p, c);
    p->auth->ulen = c;

    p->state = auth_uname;
    return p->state;
}

static enum auth_state 
uname (const uint8_t c, auth_parser *p) {
    *((p->auth->uname) + p->i) = c;
    p->i++;
    
    if (remaining_is_done(p)){
        *((p->auth->uname) + p->i) = '\0';
        p->state = auth_plen;
    } else {
        p->state = auth_uname;
    }
    return p->state;
}

static enum auth_state 
plen (const uint8_t c, auth_parser *p) {
    if(c <= 0) {
        p->state = auth_error;
        return p->state;
    }
    
    remaining_set(p, c);
    p->auth->plen = c;

    p->state = auth_passwd;
    return p->state;
}

static enum auth_state 
passwd (const uint8_t c, auth_parser *p) {
    *((p->auth->passwd) + p->i) = c;
    p->i++;
    
    if (remaining_is_done(p)){
        *((p->auth->passwd) + p->i) = '\0';
        p->state = auth_done;
    } else {
        p->state = auth_passwd;
    }
    return p->state;
}

enum auth_state auth_parser_feed(auth_parser *p, uint8_t c) {
    enum auth_state next;

    switch (p->state)
    {
    case auth_version:
        next = version(c, p);
        break;
    case auth_ulen:
        next = ulen(c, p);
        break;
    case auth_uname:
        next = uname(c, p);
        break;
    case auth_plen:
        next = plen(c, p);
        break;
    case auth_passwd:
        next = passwd(c, p);
        break;
    case auth_done:
    case auth_error:
    case auth_error_unsupported_version:
    default:
        next = auth_error;
        break;
    }

    p->state = next;
    return p->state;

}



enum auth_state 
auth_consume(buffer *b, auth_parser *p, bool *error) {
    enum auth_state st = p->state;
    bool finished = false;
    while (buffer_can_read(b) && !finished)
    {
        uint8_t byte = buffer_read(b);
        st = auth_parser_feed(p, byte);
        if (auth_is_done(st, error)) {
            finished = true;
        }
    }
    return st;
}



bool 
auth_is_done(const enum auth_state state, bool *error) {
    bool ret = false;
    if (state == auth_error ||
        state == auth_error_unsupported_version)
    {
        if (error != 0)
        {
            *error = true;
        }
        ret = true;
    }
    else if (state == auth_done)
    {
        ret = true;
    }
    return ret;
}


int 
auth_marshal(buffer *b, const uint8_t status) {
    size_t n, len = 2;
    uint8_t *buff = buffer_write_ptr(b, &n);
    if (n < len)
    {
        return -1;
    }
    buff[0] = 0x01;
    buff[2] = status;
   
    buffer_write_adv(b, len);
    return len; 
}