#include "hello.h"
#include <stdio.h>


static int hello_arrival(selector_key* event) {
    Session* session = (Session *) event->data;
    session -> client.authentication = NO_ACCEPTABLE_METHODS;
    
    // TODO inicializar el parser
    hello_parser h_parser;
    hello_parser_init(h_parser);


    selector_set_interest(event->s, session->client.fd, OP_READ);
}

static unsigned hello_read(selector_key * event){
    Session * session = (Session *) event->data;
    bool error;
    hello_header * hello = &session->socks.helloHeader;

    //TODO va cosas del parser;
}

state_definition hello_state_def(void) {

    state_definition state_def = {
        .state = HELLO,
        .on_arrival = hello_arrival,
        .on_read = hello_read,
        .on_write = NULL,
        .on_block_ready = NULL,
        .on_departure = NULL,
    };

    return state_def;
}