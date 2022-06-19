#ifndef PRAWTOS_TYP_PARSER_H
#define PRAWTOS_TYP_PARSER_H

#include <stdint.h>
#include <stdbool.h>
#include "../buffer/buffer.h"

enum typ_state {
    typ_type,

    typ_done,

    typ_error,
    typ_error_unsupported_type,
};

typedef struct typ_parser {
    enum typ_state state;
    uint8_t type;
} typ_parser;

enum typ_response_status {
    cmd_get,
    cmd_user,
    cmd_unsupported,
};

extern enum typ_state 
typ_parser_feed (typ_parser *p, const uint8_t c);

void 
typ_parser_init(typ_parser *p);

bool typ_is_done(const enum typ_state state, bool *errored);

enum typ_state
typ_consume (buffer *b, typ_parser *p, bool *errored);

int 
typ_marshal(buffer *b, const enum typ_response_status status);


#endif