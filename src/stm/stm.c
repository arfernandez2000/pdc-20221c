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
    fprintf(stdout,"Estoy en handle_first!\n");
    if(stm->current == NULL) {
        stm->current = stm->states + stm->initial;
        if(NULL != stm->current->on_arrival) {
            fprintf(stdout,"Estoy en el if de handle_first!\n");
            stm->current->on_arrival(stm->current->state, s_key);
        }
    }
    
}

inline static
void jump(state_machine *stm, unsigned next, selector_key *s_key) {
    fprintf(stdout,"next: %d\n", next);
    fprintf(stdout,"max_state: %d\n", stm->max_state);
    if(next > stm->max_state) {
        fprintf(stdout,"Estoy en abort de jump\n");
        abort();
    }
    fprintf(stdout, "en jump\n");
    fprintf(stdout, "next: %d\n", next);
    if(stm->current != stm->states + next) {
        fprintf(stdout, "if 1 en jump\n");
        if(stm->current != NULL && stm->current->on_departure != NULL) {
            fprintf(stdout, "if 1.1\n");
            stm->current->on_departure(stm->current->state, s_key);
        }
        stm->current = stm->states + next;

        if(NULL != stm->current->on_arrival) {
            fprintf(stdout, "if 1.2\n");
            fprintf(stdout, "current_state en jump: %d\n", stm->current->state);
            stm->current->on_arrival(stm->current->state, s_key);
        }
    }
}

unsigned
stm_handler_read(state_machine *stm, selector_key *s_key) {
    fprintf(stdout,"Estoy en stm_handler_read!\n");
    handle_first(stm, s_key);
    if(stm->current->on_read_ready == 0) {
        fprintf(stdout,"Abort de stm_handle_read\n");
        abort();
    }
    const unsigned int ret = stm->current->on_read_ready(s_key);
    fprintf(stdout,"Respuesta del on_read_ready: %d\n", ret);
    jump(stm, ret, s_key);

    return ret;
}

unsigned
stm_handler_write(state_machine *stm, selector_key *s_key) {
    fprintf(stdout, "Estoy en stm_handler_write\n");
    handle_first(stm, s_key);
    if(stm->current->on_write_ready == 0) {
        fprintf(stdout, "Abort de stm_handler_write\n");
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
