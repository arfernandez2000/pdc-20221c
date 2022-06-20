#include "prawtos.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include "prawtosutils.h"
#include "auth_prawtos.h"
#include "../parser/prawtos_parser.h"
#include "cmd_prawtos.h"
#include "../stadistics/stadistics.h"

static const struct state_definition prawtos_init_states[]={
    {
        .state = AUTH_READ,
        .on_arrival = auth_prawtos_init,
        .on_read_ready = auth_prawtos_read,
    },
    {
        .state = AUTH_WRITE,
        .on_write_ready = auth_prawtos_write,
    },
    {
        .state = CMD_READ,
        .on_arrival = cmd_init,
        .on_read_ready = cmd_read,
    },
    {
        .state = CMD_WRITE,
        .on_write_ready = cmd_write,
    },
    {
        .state = DONE,
    },
    {
        .state = ERROR,
    }
};

static struct prawtos * prawtos_arrival(int client_fd){
    struct prawtos * ret;
    ret = malloc(sizeof(*ret));
    if(ret == NULL){
        return NULL;
    }
    memset(ret,0x00,sizeof(*ret));
    ret->client_fd = client_fd;
    ret->client_addr_len = sizeof(ret->client_addr);
    ret->stm.states = prawtos_init_states;
    ret->stm.initial = CMD_READ;
    ret->stm.max_state = ERROR;
    stm_init(&(ret->stm));
    buffer_init(&ret->read_buffer, sizeof(ret->raw_buff_a)/sizeof(((ret->raw_buff_a)[0])), ret->raw_buff_a);
    buffer_init(&ret->write_buffer, sizeof(ret->raw_buff_b)/sizeof(((ret->raw_buff_b)[0])), ret->raw_buff_b);
    buffer_init(&aux, sizeof(ret->raw_buff_a)/sizeof(((ret->raw_buff_a)[0])), ret->raw_buff_a);
    
    return ret;
}

static void prawtos_done(struct selector_key * key){
    int fd = ((struct prawtos*)key->data)->client_fd;
    if (fd!= -1){
        if (SELECTOR_SUCCESS != selector_unregister_fd(key->s, fd))
        {
            abort();
        }
        close(fd);
    }
}

static void prawtos_write(struct selector_key *key){
    state_machine * stm = &((struct prawtos*)key->data)->stm;
    const enum prawtos_state st = stm_handler_write(stm, key);

    if (ERROR == st || DONE == st)
    {
        prawtos_done(key);
    }
}

static void prawtos_read(struct selector_key *key){
    state_machine *stm = &((struct prawtos*)key->data)->stm;
    const enum prawtos_state st = stm_handler_read(stm, key);
    if (ERROR == st || DONE == st){
        prawtos_done(key);
    }   
}

static void prawtos_close(struct selector_key *key){
    if(key->data != NULL) {
        free(key->data);
        key->data = NULL;
    }
    if(get_concurrent_connections() > 0){
        stadistics_decrease_concurrent();
    }
}

const struct fd_handler prawtos_handler = {
    .handle_read = prawtos_read,
    .handle_write = prawtos_write,
    .handle_close = prawtos_close,
};

void prawtos_passive_accept(selector_key * key) {
    fprintf(stdout, "Esoty en prawtos_passive_accept!\n");
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    struct prawtos *state = NULL;
    const int client = accept(key->fd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (selector_fd_set_nio(client) == -1)
    {
        goto error;
    }
    state = prawtos_arrival(client);
    if (state == NULL)
    {
        goto error;
    }

    memcpy(&state->client_addr, &client_addr, client_addr_len);
    state->client_addr_len = client_addr_len;
    if (SELECTOR_SUCCESS != selector_register(key->s, client, &prawtos_handler, OP_READ, state))
    {
        goto error;
    }
    stadistics_increase_concurrent();
    return;

error:
    if (client != -1)
    {
        close(client);
    }
    prawtos_close(key);
}
