/** 
 * prawtos_get_parser.c -- parser del get request de prawtosv5
 */

#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#include "prawtos_get_parser.h"

static void 
remaining_set (get_parser *p, const int n){
    p->i = 0;
    p->n = n;
}

static int
remaining_is_done (get_parser *p) {
    return p->i >= p->n;
}

// static enum get_state
// type (const uint8_t c, get_parser *p) {
//     enum get_state next;
//     switch (c) {
//         case 0x00:
//             next = get_cmd;
//             break;
//         default:
//             next = get_error_unsupported_type;
//             break;
//     }
//     return next;
// }

static enum get_state
cmd (const uint8_t c, struct get_parser *p){
    if(c >= 4)
        return get_error_unsupported_cmd;
    p->get->cmd = c;
    return get_done;
}

extern enum get_state 
get_parser_feed (get_parser *p, const uint8_t c) {
    enum get_state next;

    switch (p->state)
    {
    // case get_type:
    //     next = type(c, p);
    //     break;
    case get_cmd:
        next = cmd(c, p);
        break;
    case get_done:
    case get_error:
    case get_error_unsupported_type:
    case get_error_unsupported_cmd:
        next = p->state;
        break;
    default:
        next = get_error;
        break;
    }
    p->state = next;
    return p->state;
}    

void 
get_parser_init(get_parser *p) {
    fprintf(stdout, "Al principio del get_parser_init\n");
    p->state = get_cmd;
    memset(&(p->get), 0, sizeof(*(p->get)));
    fprintf(stdout, "Despues del memset\n");
}

bool get_is_done(const enum get_state state, bool *errored)
{
    bool ret = false;
    if (state == get_error ||
        state == get_error_unsupported_type ||
        state == get_error_unsupported_cmd)
    {
        if (errored != 0)
        {
            *errored = true;
        }
        ret = true;
    }
    else if (state == get_done)
    {
        ret = true;
    }
    return ret;
}

enum get_state
get_consume (buffer *b, get_parser *p, bool *errored) {
    fprintf(stdout, "Estoy en get_consume!\n");
    enum get_state st = p->state;
    bool finished = false;
    while (buffer_can_read(b) && !finished)
    {
        uint8_t byte = buffer_read(b);
        st = get_parser_feed(p, byte);
        if(get_is_done(st, errored)) {
            finished = true;
        }
    }
    return st;
}

int 
get_marshal(buffer *b, const enum get_response_status status, const enum get_cmd cmd, uint8_t nargs, uint8_t* args){
    size_t args_len =  sizeof(args);
    size_t count, len = 3 + args_len;
    uint8_t * ptr = buffer_write_ptr(b, &count);
    
    if(count < len){
        return -1;
    }

    ptr[0] = status;
    ptr[1] = cmd;
    ptr[2] = nargs;
    buffer_write_adv(b, 3);

    memcpy(ptr + 1, args, args_len);
    buffer_write_adv(b, args_len);
    free(args);

    return len;
}
