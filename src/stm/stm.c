/**
 * stm.c - pequeño motor de maquina de estados donde los eventos son los
 *         del selector.c
 */
#include <stdlib.h>
#include "../../include/stm.h"

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
handle_first(state_machine *stm, selector_key *s_key) {
    if(stm->current == NULL) {
        stm->current = stm->states + stm->initial;
        if(NULL != stm->current->on_arrival) {
            stm->current->on_arrival(stm->current->state, s_key);
        }
    }
    
}

inline static
void jump(state_machine *stm, unsigned next, selector_key *s_key) {
    if(next > stm->max_state) {
        abort();
    }
    if(stm->current != stm->states + next) {
        if(stm->current != NULL && stm->current->on_departure != NULL) {
            stm->current->on_departure(stm->current->state, s_key);
        }
        stm->current = stm->states + next;

        if(NULL != stm->current->on_arrival) {
            stm->current->on_arrival(stm->current->state, s_key);
        }
    }
}

unsigned
stm_handler_read(state_machine *stm, selector_key *s_key) {
    handle_first(stm, s_key);
    if(stm->current->on_read_ready == 0) {
        abort();
    }
    const unsigned int ret = stm->current->on_read_ready(s_key);
    jump(stm, ret, s_key);

    return ret;
}

unsigned
stm_handler_write(state_machine *stm, selector_key *s_key) {
    handle_first(stm, s_key);
    if(stm->current->on_write_ready == 0) {
        abort();
    }
    const unsigned int ret = stm->current->on_write_ready(s_key);
    jump(stm, ret, s_key);

    return ret;
}

unsigned
stm_handler_block(state_machine *stm, selector_key *s_key) {
    handle_first(stm, s_key);
    if(stm->current->on_block_ready == 0) {
        abort();
    }
    const unsigned int ret = stm->current->on_block_ready(s_key);
    jump(stm, ret, s_key);

    return ret;
}

void
stm_handler_close(state_machine *stm, selector_key *s_key) {
    if(stm->current != NULL && stm->current->on_departure != NULL) {
        stm->current->on_departure(stm->current->state, s_key);
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
