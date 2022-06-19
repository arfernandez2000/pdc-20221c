/** 
 * prawtos_typ_parser.c -- parser del get request de prawtosv5
 */

#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>

#include "prawtos_typ_parser.h"

static enum typ_state
type (const uint8_t c, typ_parser *p) {
    enum typ_state next;
    switch (c) {
        case 0x00:
            next = typ_done;
            break;
        case 0x01:
            next = typ_done;
            break;
        // case 0x02:
        //     next = typ_done;
        //     break;
        default:
            next = typ_error_unsupported_type;
            break;
    }
    p->type = c;
    return next;
}

extern enum typ_state 
typ_parser_feed (typ_parser *p, const uint8_t c) {
    enum typ_state next;

    switch (p->state)
    {
    case typ_type:
        next = type(c, p);
        break;
    case typ_error_unsupported_type:
        next = p->state;
        break;
    default:
        next = typ_error;
        break;
    }
    p->state = next;
    return p->state;
}    

void 
typ_parser_init(typ_parser *p) {
    p->state = typ_type;
}

bool typ_is_done(const enum typ_state state, bool *errored)
{
    bool ret = false;
    if (state == typ_error ||
        state == typ_error_unsupported_type)
    {
        if (errored != 0)
        {
            *errored = true;
        }
        ret = true;
    }
    else if (state == typ_done)
    {
        ret = true;
    }
    return ret;
}

enum typ_state
typ_consume (buffer *b, typ_parser *p, bool *errored) {
    enum typ_state st = p->state;
    bool finished = false;
    while (buffer_can_read(b) && !finished)
    {
        uint8_t byte = buffer_read(b);
        st = typ_parser_feed(p, byte);
        if(typ_is_done(st, errored)) {
            finished = true;
        }
    }
    return st;
}

int 
typ_marshal(buffer *b, const enum typ_response_status status){
    size_t count, len = 1;
    uint8_t * ptr = buffer_write_ptr(b, &count);

    if(count < len){
        return -1;
    }

    ptr[0] = status;
    buffer_write_adv(b, len);

    return len;
}