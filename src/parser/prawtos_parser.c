
/** 
 * prawtos_parser.c -- parser de prawtosv5
 */

#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#include "../../include/parser/prawtos_parser.h"

static void 
remaining_set (prawtos_parser *p, const int n){
    p->i = 0;
    p->n = n;
}

static int
remaining_is_done (prawtos_parser *p) {
    return p->i >= p->n;
}

static enum prawtos_parser_state
type (const uint8_t c, prawtos_parser *p) {
    enum prawtos_parser_state next;
    switch (c) {
        case 0x00:
            next = prawtos_cmd_get;
            break;
        case 0x01:
            next = prawtos_cmd_user;
            break;
        case 0x02:
            next = prawtos_cmd_sniff;
            break;
        case 0x03:
            next = prawtos_parser_done;
            break;
        default:
            next = prawtos_error_unsupported_type;
            break;
    }
    p->type = c;
    return next;
}

static enum prawtos_parser_state
cmd_get(const uint8_t c, struct prawtos_parser *p){
    if(c >= 4)
        return prawtos_error_unsupported_cmd;
    p->get->cmd = c;
    return prawtos_parser_done;
}

static enum prawtos_parser_state
cmd_user(const uint8_t c, struct prawtos_parser *p){
    enum prawtos_parser_state next;

    switch(c){
        case 0x00:
            next = prawtos_admin;
            break;
        case 0x01:
            next = prawtos_ulen;
            break;
        case 0x02:
            next = prawtos_ulen;
            break;
        default:
            next = prawtos_error_unsupported_cmd;
            break;
    }
    p->user->cmd = c;
    return next;
}

static enum prawtos_parser_state
cmd_sniff(const uint8_t c, struct prawtos_parser *p){
    if(c >= 2)
        return prawtos_error_unsupported_cmd;
    p->get->cmd = c;
    return prawtos_parser_done;
}

static enum prawtos_parser_state
is_admin (const uint8_t c, struct prawtos_parser *p){
    enum prawtos_parser_state next;

    switch(c){
        case 0x00:
            next = prawtos_ulen;
            break;
        case 0x01:
            next = prawtos_ulen;
            break;
        default:
            next = prawtos_error_unsupported_admin;
            break;
    }

    p->user->admin = c;
    return next;
}

static enum prawtos_parser_state 
ulen (const uint8_t c, prawtos_parser *p) {
    if(c <= 0) {
        p->state = prawtos_error;
        return p->state;
    }

    remaining_set(p, c);
    p->user->ulen = c;

    p->state = prawtos_uname;
    return p->state;
}

static enum prawtos_parser_state 
uname (const uint8_t c, prawtos_parser *p) {
    *((p->user->uname) + p->i) = c;
    p->i++;
    
    if (remaining_is_done(p)){
        *((p->user->uname) + p->i) = '\0';
        if(p->user->cmd == 0x01)
            p->state = prawtos_parser_done;
        else
            p->state = prawtos_plen;
    } else {
        p->state = prawtos_uname;
    }
    return p->state;
}

static enum prawtos_parser_state 
plen (const uint8_t c, prawtos_parser *p) {
    if(c <= 0) {
        p->state = prawtos_error;
        return p->state;
    }
    
    remaining_set(p, c);
    p->user->plen = c;

    p->state = prawtos_passwd;
    return p->state;
}

static enum prawtos_parser_state 
passwd (const uint8_t c, prawtos_parser *p) {
    *((p->user->passwd) + p->i) = c;
    p->i++;
    
    if (remaining_is_done(p)){
        *((p->user->passwd) + p->i) = '\0';
        p->state = prawtos_parser_done;
    } else {
        p->state = prawtos_passwd;
    }
    return p->state;
}


extern enum prawtos_parser_state 
prawtos_parser_feed (prawtos_parser *p, const uint8_t c) {
    enum prawtos_parser_state next;

    switch (p->state)
    {
    case prawtos_type:
        next = type(c, p);
        break;
    case prawtos_cmd_get:
        next = cmd_get(c, p);
        break;
    case prawtos_cmd_user:
        next = cmd_user(c, p);
        break;
    case prawtos_cmd_sniff:
        next = cmd_sniff(c, p);
        break;
    case prawtos_admin:
        next = is_admin(c, p);
        break;
    case prawtos_ulen:
        next = ulen(c, p);
        break;
    case prawtos_uname:
        next = uname(c, p);
        break;
    case prawtos_plen:
        next = plen(c, p);
        break;
    case prawtos_passwd:
        next = passwd(c, p);
        break;
    case prawtos_parser_done:
    case prawtos_error:
    case prawtos_error_unsupported_type:
    case prawtos_error_unsupported_cmd:
        next = p->state;
        break;
    default:
        next = prawtos_error;
        break;
    }
    p->state = next;
    return p->state;
}    

void 
prawtos_parser_init(prawtos_parser *p) {
    p->state = prawtos_type;
    memset(p->get, 0, sizeof(*(p->get)));
    memset(p->user, 0, sizeof(*(p->user)));
}

bool prawtos_is_done(const enum prawtos_parser_state state, bool *errored)
{
    bool ret = false;
    if (state == prawtos_error ||
        state == prawtos_error_unsupported_type ||
        state == prawtos_error_unsupported_cmd  || 
        state == prawtos_error_unsupported_admin)
    {
        if (errored != 0)
        {
            *errored = true;
        }
        ret = true;
    }
    else if (state == prawtos_parser_done)
    {
        ret = true;
    }
    return ret;
}

enum prawtos_parser_state
prawtos_consume (buffer *b, prawtos_parser *p, bool *errored) {
    enum prawtos_parser_state st = p->state;
    bool finished = false;
    while (buffer_can_read(b) && !finished)
    {
        uint8_t byte = buffer_read(b);
        st = prawtos_parser_feed(p, byte);
        if(prawtos_is_done(st, errored)) {
            finished = true;
        }
    }
    return st;
}

int 
get_marshal(buffer *b, const enum prawtos_response_status status, const enum get_cmd cmd, uint8_t nargs, uint8_t* args, int args_len){
    size_t count, len = 3 + args_len;
    uint8_t * ptr =  buffer_write_ptr(b, &count);

//SI SE ROMPE ES ESTO
    if((int)count < args[0] + 1){
        return -1;
    }

    ptr[0] = status;
    ptr[1] = cmd;
    ptr[2] = nargs;
    buffer_write_adv(b, 3);


    // memcpy(ptr + 3, args, args[0]+1);
    memcpy(ptr + 3, args, args_len);

    // buffer_write_adv(b, args[0]+1);
    buffer_write_adv(b, args_len);

    return len;
}

int 
user_marshal(buffer *b, const enum prawtos_response_status status){
    size_t count, len = 1;
    uint8_t * ptr = buffer_write_ptr(b, &count);
    
    if(count < len){
        return -1;
    }

    ptr[0] = status;
    buffer_write_adv(b, len);

    return len;
}

int 
quit_marshal(buffer *b, const enum prawtos_response_status status){
    size_t count, len = 1;
    uint8_t * ptr = buffer_write_ptr(b, &count);
    
    if(count < len){
        return -1;
    }

    ptr[0] = status;
    buffer_write_adv(b, len);

    return len;
}

int 
sniff_marshal(buffer *b, const enum prawtos_response_status status){
    size_t count, len = 1;
    uint8_t * ptr = buffer_write_ptr(b, &count);
    
    if(count < len){
        return -1;
    }

    ptr[0] = status;
    buffer_write_adv(b, len);

    return len;
}
