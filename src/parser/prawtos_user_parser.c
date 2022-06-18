/** 
 * prawtos_user_parser.c -- parser del user request de prawtosv5
 */

#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>

#include "prawtos_user_parser.h"

static void 
remaining_set (user_parser *p, const int n){
    p->i = 0;
    p->n = n;
}

static int
remaining_is_done (user_parser *p) {
    return p->i >= p->n;
}

static enum user_state
type (const uint8_t c, user_parser *p) {
    enum user_state next;
    switch (c) {
        case 0x01:
            next = user_cmd;
            break;
        default:
            next = user_error_unsupported_type;
            break;
    }
    return next;
}

static enum user_state
cmd (const uint8_t c, struct user_parser *p){
    enum user_state next;

    switch(c){
        case 0x00:
            next = user_admin;
            break;
        case 0x01:
            next = user_ulen;
            break;
        case 0x02:
            next = user_ulen;
            break;
        default:
            next = user_error_unsupported_cmd;
            break;
    }
    p->user->cmd = c;
    return next;
}

static enum user_state
is_admin (const uint8_t c, struct user_parser *p){
    enum user_state next;

    switch(c){
        case 0x00:
            next = user_ulen;
            break;
        case 0x01:
            next = user_ulen;
            break;
        default:
            next = user_error_unsupported_admin;
            break;
    }

    p->user->admin = c;
    return next;
}

static enum user_state 
ulen (const uint8_t c, user_parser *p) {
    if(c <= 0) {
        p->state = user_error;
        return p->state;
    }

    remaining_set(p, c);
    p->user->ulen = c;

    p->state = user_uname;
    return p->state;
}

static enum user_state 
uname (const uint8_t c, user_parser *p) {
    *((p->user->uname) + p->i) = c;
    p->i++;
    
    if (remaining_is_done(p)){
        *((p->user->uname) + p->i) = '\0';
        if(p->user->cmd == 0x01)
            p->state = user_done;
        else
            p->state = user_plen;
    } else {
        p->state = user_uname;
    }
    return p->state;
}

static enum user_state 
plen (const uint8_t c, user_parser *p) {
    if(c <= 0) {
        p->state = user_error;
        return p->state;
    }
    
    remaining_set(p, c);
    p->user->plen = c;

    p->state = user_passwd;
    return p->state;
}

static enum user_state 
passwd (const uint8_t c, user_parser *p) {
    *((p->user->passwd) + p->i) = c;
    p->i++;
    
    if (remaining_is_done(p)){
        *((p->user->passwd) + p->i) = '\0';
        p->state = user_done;
    } else {
        p->state = user_passwd;
    }
    return p->state;
}

extern enum user_state 
user_parser_feed (user_parser *p, const uint8_t c) {
    enum user_state next;

    switch (p->state)
    {
    case user_type:
        next = type(c, p);
        break;
    case user_cmd:
        next = cmd(c, p);
        break;
    case user_admin:
        next = is_admin(c, p);
        break;
    case user_ulen:
        next = ulen(c, p);
        break;
    case user_uname:
        next = uname(c, p);
        break;
    case user_plen:
        next = plen(c, p);
        break;
    case user_passwd:
        next = passwd(c, p);
        break;
    case user_done:
    case user_error:
    case user_error_unsupported_type:
    case user_error_unsupported_admin:
    case user_error_unsupported_cmd:
        next = p->state;
        break;
    default:
        next = user_error;
        break;
    }
    p->state = next;
    return p->state;
}    

void 
user_parser_init(user_parser *p) {
    p->state = user_type;
    memset(p->user, 0, sizeof(*(p->user)));
}

bool user_is_done(const enum user_state state, bool *errored)
{
    bool ret = false;
    if (state == user_error ||
        state == user_error_unsupported_type    ||
        state == user_error_unsupported_cmd     || 
        state == user_error_unsupported_admin)
    {
        if (errored != 0)
        {
            *errored = true;
        }
        ret = true;
    }
    else if (state == user_done)
    {
        ret = true;
    }
    return ret;
}

enum user_state
user_consume (buffer *b, user_parser *p, bool *errored) {
    enum user_state st = p->state;
    bool finished = false;
    while (buffer_can_read(b) && !finished)
    {
        uint8_t byte = buffer_read(b);
        st = user_parser_feed(p, byte);
        if(user_is_done(st, errored)) {
            finished = true;
        }
    }
    return st;
}

int 
user_marshal(buffer *b, const enum user_response_status status){
    size_t count, len = 1;
    uint8_t * ptr = buffer_write_ptr(b, &count);
    
    if(count < len){
        return -1;
    }

    ptr[0] = status;
    buffer_write_adv(b, len);

    return len;
}
