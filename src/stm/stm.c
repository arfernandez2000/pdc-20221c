/**
 * stm.c - pequeño motor de maquina de estados donde los eventos son los
 *         del selector.c
 */
#include <stdlib.h>
#include "stm.h"

#define N(x) (sizeof(x)/sizeof((x)[0]))

void
stm_init(state_machine *stm) {
    // verificamos que los estados son correlativos, y que están bien asignados.
    for(unsigned i = 0 ; i <= stm->max_state; i++) {
        if(i != stm->states[i].state) {
            abort();
        }
    }

    if(stm->initial < stm->max_state) {
        stm->current = NULL;
    } else {
        abort();
    }
}

inline static void
handle_first(state_machine *stm, selector_key *key) {
    if(stm->current == NULL) {
        stm->current = stm->states + stm->initial;
        if(NULL != stm->current->on_arrival) {
            fprintf(stdout,"Estoy en el if de handle_first!\n");
            stm->current->on_arrival(stm->current->state, key);
        }
    }
    
}

inline static
void jump(state_machine *stm, unsigned next, selector_key *key) {
    if(next > stm->max_state) {
        abort();
    }
    fprintf(stdout, "en jump\n");
    fprintf(stdout, "next: %d\n", next);
    if(stm->current != stm->states + next) {
        fprintf(stdout, "if 1 en jump\n");
        if(stm->current != NULL && stm->current->on_departure != NULL) {
            fprintf(stdout, "if 1.1\n");
            stm->current->on_departure(stm->current->state, key);
        }
        stm->current = stm->states + next;

        if(NULL != stm->current->on_arrival) {
            fprintf(stdout, "if 1.2\n");
            stm->current->on_arrival(stm->current->state, key);
        }
    }
}

unsigned
stm_handler_read(state_machine *stm, selector_key *key) {
    fprintf(stdout,"Estoy en stm_handler_read!\n");
    handle_first(stm, key);
    if(stm->current->on_read_ready == 0) {
        abort();
    }
    const unsigned int ret = stm->current->on_read_ready(key);
    jump(stm, ret, key);

    return ret;
}

unsigned
stm_handler_write(state_machine *stm, selector_key *key) {
    handle_first(stm, key);
    if(stm->current->on_write_ready == 0) {
        abort();
    }
    const unsigned int ret = stm->current->on_write_ready(key);
    jump(stm, ret, key);

    return ret;
}

unsigned
stm_handler_block(state_machine *stm, selector_key *key) {
    handle_first(stm, key);
    if(stm->current->on_block_ready == 0) {
        abort();
    }
    const unsigned int ret = stm->current->on_block_ready(key);
    jump(stm, ret, key);

    return ret;
}

void
stm_handler_close(state_machine *stm, selector_key *key) {
    if(stm->current != NULL && stm->current->on_departure != NULL) {
        stm->current->on_departure(stm->current->state, key);
    }
}

unsigned
stm_state(state_machine *stm) {
    unsigned ret = stm->initial;
    if(stm->current != NULL) {
        ret= stm->current->state;
    }
    return ret;
}