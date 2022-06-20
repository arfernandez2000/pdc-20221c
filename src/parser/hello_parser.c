#include "../../include/parser/hello_parser.h"

void hello_parser_init(struct hello_parser *p)
{
    p->state = hello_version;
    p->remaining = 0;
}

enum hello_state hello_parser_feed(struct hello_parser *p, uint8_t b)
{
    switch (p->state)
    {
    case hello_version:
        if (b == 0x05)
        {
            p->state = hello_nmethods;
        }
        else
        {
            p->state = hello_error_unsupported_version;
        }
        break;
    case hello_nmethods:
        p->remaining = b;
        p->state = hello_methods;
        if (p->remaining <= 0)
        {
            p->state = hello_done;
        }
        break;
    case hello_methods:
        if (p->on_authentication_method != NULL)
        {
            p->on_authentication_method(p->data, b);
            //p->on_authentication_method(p, b);
        }
        p->remaining--;
        if (p->remaining <= 0)
        {
            p->state = hello_done;
        }
        break;
    case hello_done:
        //nada que hacer
        break;
    case hello_error_unsupported_version:
        //nada que hacer
        break;
    default:
        fprintf(stderr, "unknown state %d\n", p->state);
        abort();
    }
    return p->state;
}

bool hello_is_done(const enum hello_state state, bool *error)
{
    bool ret = false;
    switch (state)
    {
    case hello_error_unsupported_version:
        if (error != 0)
        {
            *error = true;
        }
        ret = true;
        break;
    case hello_done:
        ret = true;
        break;
    default:
        ret = false;
        break;
    }
    return ret;
}

enum hello_state hello_consume(buffer *b, struct hello_parser *p, bool *error)
{
    enum hello_state st = p->state;
    bool finished = false;
    while (buffer_can_read(b) && !finished)
    {
        uint8_t byte = buffer_read(b);
        st = hello_parser_feed(p, byte);
        if (hello_is_done(st, error))
        {
            finished = true;
        }
    }
    return st;
}

int hello_marshall(buffer *b, const uint8_t method)
{
    size_t n;
    uint8_t *buf = buffer_write_ptr(b, &n);
    if (n < 2)
    {
        return -1;
    }
    buf[0] = 0x05;
    buf[1] = method;
    buffer_write_adv(b, 2);
    return 2;
}

extern const char * hello_error(const struct hello_parser *p){
    char * ret;
    switch(p->state){
        case hello_error_unsupported_version:
            ret = "unsupported version";
            break;
        default:
            ret="";
            break;
    }
    return ret;
}

void hello_parser_close(){
    // no hay nada que liberar
}